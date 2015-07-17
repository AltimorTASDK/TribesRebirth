//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include <streams.h>
#include "grdEdgeTable.h"

#define GRID_SHADE_FILE "grdshade.dat"


//----------------------------------------------------------------------------

GridEdgeTable::GridEdgeTable()
{
	groundScale = -1;
	groundHeight.fMin = groundHeight.fMax = .0f;
}

GridEdgeTable::~GridEdgeTable()
{
}

void GridEdgeTable::setRasterData(int scale,int scanLevel,
	float visdist,GridRange<float> height)
{
	groundScale = scale;
	scanDetailLevel = scanLevel;
	visibleDistance = visdist;
	groundHeight = height;
}


//----------------------------------------------------------------------------

void GridEdgeTable::buildTables(const TSCamera& cam)
{
   const TSOrthographicCamera *camPtr = dynamic_cast<const TSOrthographicCamera *>(&cam);
   if(camPtr)
   {
      buildTablesOrtho(*camPtr);
      return;
   }
	TMat3F mat = cam.getTOC();
	mat.inverse();

	//
	Poly3F opoly,cpoly;
	Point3F opoints[8],cpoints[20];
	opoly.numPoints = 8;
	opoly.points = opoints;
	cpoly.numPoints = 0;
	cpoly.points = cpoints;

	Poly3I vpoly;
	Point3I vpoints[20];
	vpoly.numPoints = 0;
	vpoly.points = vpoints;

	// Build an octagon around the camera (in camera space)
	//
	float sd1 = visibleDistance;
   float height = visibleDistance * .41421356;

	// DMM - No longer Used?
   //float sd2 = sd1 * 0.707106f; // 1/sqrt(2)
	
   opoints[0].x = sd1; opoints[0].y = height;
   opoints[1].x = sd1; opoints[1].y = -height;
   opoints[2].x = height; opoints[2].y = -sd1;
   opoints[3].x = -height; opoints[3].y = -sd1;
   opoints[4].x = -sd1; opoints[4].y = -height;
   opoints[5].x = -sd1; opoints[5].y = height;
   opoints[6].x = -height; opoints[6].y = sd1;
   opoints[7].x = height; opoints[7].y = sd1;

	opoints[0].z = opoints[1].z = opoints[2].z = opoints[3].z =
	opoints[4].z = opoints[5].z = opoints[6].z = opoints[7].z = -mat.p.z;

	// Clip it against the camera view cone. It's then
	// converted into coors. of the highest detail and
	// rasterized into edge tables.
	//
	int shift = groundScale;
	float expand = float(1 << (shift + 1));
	ViewCone vcone(cam,Point3F(0.0f,0.0f,0.0f),float(visibleDistance),expand);
	vcone.setClipMask(VC_CLIP_ALL & ~VC_CLIP_BACK);
	if (vcone.clipGround(opoly,float(groundHeight.fMin),
			float(groundHeight.fMax),&cpoly)) {
		int round = (shift > 1)? 1 << (shift - 1): 0;
		for (int i = 0; i < cpoly.numPoints; ++i) {
			vpoints[i].x = (Int32(cpoints[i].x) + round) >> shift;
			vpoints[i].y = (Int32(cpoints[i].y) + round) >> shift;
		}
		vpoly.numPoints = cpoly.numPoints;
		setDetailCamera(mat.p);
		buildEdgeTables(vpoly);
	}
}


//----------------------------------------------------------------------------

void GridEdgeTable::buildTablesOrtho(const TSOrthographicCamera& cam)
{
	TMat3F mat = cam.getTOC();
	mat.inverse();
	setDetailCamera(mat.p);

	//
	Poly3I poly;
	Point3I points[4];
	poly.numPoints = 4;
	poly.points = points;

	// Use the camera worldView centered around the camera
	//
	int shift = groundScale;
	int expand = 1 << shift;
	RectF world = cam.getWorldViewport();
	float scale = 1.0f / (1 << shift);
	RectI iworld;
	iworld.upperL.x = Int32((world.upperL.x - expand) * scale);
	iworld.upperL.y = Int32((world.upperL.y + expand) * scale);
	iworld.lowerR.x = Int32((world.lowerR.x + expand) * scale);
	iworld.lowerR.y = Int32((world.lowerR.y - expand) * scale);

	points[0].set(iworld.upperL.x,iworld.upperL.y);
	points[1].set(iworld.lowerR.x,iworld.upperL.y);
	points[2].set(iworld.lowerR.x,iworld.lowerR.y);
	points[3].set(iworld.upperL.x,iworld.lowerR.y);

	buildEdgeTables(poly);
}


//----------------------------------------------------------------------------

void GridEdgeTable::setDetailCamera(Point3F& camPos)
{
	for (int i = 0; i <= scanDetailLevel; i++) {
		DetailData& d = details[i];
		int shift = groundScale + i;
		d.camera.x = int(camPos.x) >> shift;
		d.camera.y = int(camPos.y) >> shift;
	}
}


//----------------------------------------------------------------------------

void GridEdgeTable::buildEdgeTables(const Poly3I& poly)
{
	// Rasterize the poly into the first detail level
	rasterize(poly);

	// Build edge tables for the remaining details
	for (int i = 1; i <= scanDetailLevel; ++i) {
		DetailData& d1 = details[i - 1];
		DetailData& d2 = details[i];

		Point2I offset;
		offset.x = d1.camera.x - (d2.camera.x << 1);
		offset.y = d1.camera.y - (d2.camera.y << 1);

		d2.edgeTable.starty = (d1.edgeTable.starty + offset.y) >> 1;
		int endy = (d1.edgeTable.starty + offset.y + 
			d1.edgeTable.edges.size() - 1) >> 1;
		int size = endy - d2.edgeTable.starty + 1;

		EdgeVector& e1 = d1.edgeTable.edges;
		EdgeVector& e2 = d2.edgeTable.edges;
		e2.setSize(size);

		for (int e = 0; e < e2.size(); ++e) {
			int se = ((e + d2.edgeTable.starty) << 1) -
				d1.edgeTable.starty - offset.y;
			if (se >= 0) {
				AssertFatal(se < e1.size(),"Huh ?");
				e2[e].fMax = e1[se].fMax;
				e2[e].fMin = e1[se].fMin;
				if (++se < e1.size()) {
					e2[e].fMax = max(e2[e].fMax,e1[se].fMax);
					e2[e].fMin = min(e2[e].fMin,e1[se].fMin);
				}
			}
			else {
				AssertFatal(se >= -1,"Huh ?");
				e2[e].fMax = e1[se + 1].fMax;
				e2[e].fMin = e1[se + 1].fMin;
			}

			e2[e].fMin = (e2[e].fMin + offset.x) >> 1;
			e2[e].fMax = (e2[e].fMax + offset.x + 1) >> 1;
		}
	}
}


//----------------------------------------------------------------------------
// Polygon rasterizer
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

struct Raster
{
  int xp,xi,dy;

  void startLeft(const Point3I& tp,const Point3I& bp) {
    dy = bp.y - tp.y;
    xi = ((bp.x - tp.x) << 16) / dy;
    xp = (tp.x << 16) - xi + 0xffff;
  }
  void startRight(const Point3I& tp,const Point3I& bp) {
    dy = bp.y - tp.y;
    xi = ((bp.x - tp.x) << 16) / dy;
    xp = (tp.x << 16) - xi + 0xffff;
  }
  int pos() {
    return (xp += xi) >> 16;
  }
};


//----------------------------------------------------------------------------

static void findTop(const Poly3I& poly,Point3I** top,Point3I** bot)
{
	Point3I* tp,*bp;
	tp = bp = poly.points;
	Point3I* pp = &poly.points[1];
	for (int i = poly.numPoints - 1; i--; ++pp) {
		if (pp->y < tp->y)
			tp = pp;
		if (pp->y > bp->y)
			bp = pp;
	}
	*top = tp, *bot = bp;
}


static void nextLeft(const Poly3I& poly,Point3I** sp,Point3I** ep,Int32 cy)
{
	while ((*ep)->y <= cy) {
		*sp = *ep;
		if (--*ep < poly.points)
			*ep = &poly.points[poly.numPoints - 1];
	}
}

static void nextRight(const Poly3I& poly,Point3I** sp,Point3I** ep,Int32 cy)
{
	while ((*ep)->y <= cy) {
		*sp = *ep;
		if (++*ep >= &poly.points[poly.numPoints])
			*ep = poly.points;
	}
}


//----------------------------------------------------------------------------

void GridEdgeTable::trapezoid(int count,Raster& lx,Raster& rx)
{
	AssertFatal(count > 0,"GridEdgeTable::trapazoid: Internal error: Count <= 0");
	EdgeVector& edge = details[0].edgeTable.edges;
	do {
		// Edges include points on the poly boundry
		// Use temp to avoid MFC++ bug.
		Edge temp(rx.pos() - 1,lx.pos());
		edge.push_back(temp);
	} while (--count);
}


//----------------------------------------------------------------------------

void GridEdgeTable::rasterize(const Poly3I& poly)
{
	EdgeVector& edge = details[0].edgeTable.edges;
	edge.clear();

	if (poly.numPoints < 3)
		return;

	Point3I* tp,*bp;
	findTop(poly,&tp,&bp);
	details[0].edgeTable.starty = tp->y;
	if (tp->y == bp->y) {
		// Horizontal line fMin.x->fMax.x
		return;
	}

	Point3I* lp,*nl,*rp,*nr;
	lp = nl = rp = nr = tp;
	nextLeft(poly,&lp,&nl,lp->y);
	nextRight(poly,&rp,&nr,rp->y);
	int ycount = bp->y - tp->y;
	edge.reserve(ycount + 2);

	Raster lx,rx;
	lx.startLeft(*lp,*nl);
	rx.startRight(*rp,*nr);
	int dy,yy = rp->y;
	int by = bp->y;
	while (1) {
		if (nl->y > nr->y) {
			// Right edge ends first
			yy += dy = nr->y - yy;
			trapezoid(dy,lx,rx);
			nextRight(poly,&rp,&nr,yy);
			rx.startRight(*rp,*nr);
		}
		else
			if (nl->y < nr->y) {
				// Left edge ends first
				yy += dy = nl->y - yy;
				trapezoid(dy,lx,rx);
				nextLeft(poly,&lp,&nl,yy);
				lx.startLeft(*lp,*nl);
			}
			else
				if (nl->y < by) {
					// Edges end together
					yy += dy = nl->y - yy;
					trapezoid(dy,lx,rx);
					nextLeft(poly,&lp,&nl,yy);
					nextRight(poly,&rp,&nr,yy);
					lx.startLeft(*lp,*nl);
					rx.startRight(*rp,*nr);
				}
				else {
					// End of poly
					trapezoid(nl->y - yy + 1,lx,rx);
					break;
				}
	}
	AssertFatal(edge.size() != ycount,
		"GridEdgeTable::rasterize: Internal error: Incorrect scan count");
}

