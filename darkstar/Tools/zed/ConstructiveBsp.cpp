// ConstructiveBsp.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "THRED.h"
#include "ConstructiveBsp.h"
#include "3dmworld.h"
#include "BspManagerDialog.h"
#include "ThredBrush.h"
#include "ThredParser.h"
#include "THREDDoc.h"
#include "ts_PointArray.h"
#include "MainFrm.h"
#include "gfxmetrics.h"
#include "p_txcach.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static  GFXTextureHandle  KeyValue = {0,0};
static  DWORD             KeyVal = 0;

static  lmscanline  lmscans;
static  CMainFrame* ThepFrame;
static  int         showpcount;
static  int         pcount;
static  char        pcountbuf[80];
static  CBspNode*   currdelnode;
static  CBspNode*   topdelnode;
static  Point2F     tcenter;
static  int         tcount;

//-------------------------------------------------
// double point functions

Point3D& Point3D::normalize()
{  
   double len;
   
   len = sqrt( ( x * x ) + ( y * y ) + ( z * z ) );
   
   x /= len;
   y /= len;
   z /= len;
   
   return *this;
}

//-------------------------------------------------
// double math functions
inline Point3D& md_cross( const Point3D& a, const Point3D& b, Point3D * r )
{
   r->x = ( ( a.y * b.z ) - ( a.z * b.y ) );
   r->y = ( ( a.z * b.x ) - ( a.x * b.z ) );
   r->z = ( ( a.x * b.y ) - ( a.y * b.x ) );
   return *r;   
}

inline double md_dot( const Point3D& a, const Point3D& b )
{
   return( ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z ) );
}

// double plane stuff
// --------------------------------------------------------
TPlaneD::TPlaneD( const Point3D& p, const Point3D& n )
{
   x = n.x; y = n.y; z = n.z;
   normalize();

	d = -(p.x * x + p.y * y + p.z * z);
}

void TPlaneD::neg()
{
	Point3D::neg();
	d = -d;
}


void TPlaneD::set( const Point3D& k, const Point3D& j, const Point3D& l )
{
	Point3D	kj,lj,pv;

   kj = k;
   kj -= j;
   lj = l;
   lj -= j;
   md_cross( kj, lj, &pv );
	set( j, pv );
}

bool TPlaneD::intersect( const Point3D& start, const Point3D& end, double * value ) const
{
	Point3D dt = end;
	dt -= start;

	// If den == 0 then the line and plane area parallel.
	double den;
	if ((den = x * dt.x + y * dt.y + z * dt.z) == 0)
		return 0;
	double t = -(x * start.x + y * start.y + z * start.z + d);

	// Make sure intersection point is in between the line
	// endpoints.
	if (den > 0) {
		if (t < 0 || t > den)
			return false;
	}
	else
		if (t > 0 || t < den)
			return false;
	// Should correct this for fixed point math...
	*value = t / den;
	return true;
}

bool TPlaneD::set( int count, Point3D ** vl)
{
	double normalx,normaly,normalz;
	normalx = normaly = normalz = double(0);
	const Point3D *p0 = vl[count - 1];
	const Point3D *p1 = vl[0];

	for (int i = 0; i < count; i++) {
		normalx -= double(p0->y - p1->y) * double(p0->z + p1->z);
		normaly -= double(p0->z - p1->z) * double(p0->x + p1->x);
		normalz -= double(p0->x - p1->x) * double(p0->y + p1->y);
		p0 = p1;
		p1 = vl[i+1];
	}

	// Normalize
	double den = normalx * normalx + normaly * normaly + normalz * normalz;
	if (den != .0f) {
		double s = 1.0f / sqrt(den);
		x = normalx *= s;
		y = normaly *= s;
		z = normalz *= s;
		d = -(vl[0]->x * normalx + vl[0]->y * normaly + vl[0]->z * normalz);
		return true;
	}
	return false;
}


TPlaneD::Side TPlaneD::whichSide( const Point3D& cp ) const
{
   // get the distance precision from the app itself
   double DistancePrecision = theApp.m_planeDistancePrecision;
   
	double sr = md_dot(*this,cp);
	return ((sr - DistancePrecision) > -d)? Inside:
		((sr + DistancePrecision) < -d)? Outside:
		OnPlane;
}

coagscanline::coagscanline()
{
}

void coagscanline::clearlines()
{
  for (int i = 0; i < MAX_COAGLINES; i++)
  {
    line[i].isvertex_s = FALSE;
    line[i].isvertex_e = FALSE;
    line[i].startx = -9999999.9;
    line[i].endx = -9999999.9;
  }
}

void coagscanline::addpoint(int scanline, double xval, ThredPoint* vertex)
{
  if (line[scanline].startx > -9999999.9)
  {
    // This startx value is in use...
    if (xval < line[scanline].startx)
    {
      // Replace startx value because this one is smaller
      line[scanline].startx = xval;

      if (vertex)
      {
        // This is a vertex, so store it as such
        line[scanline].isvertex_s = TRUE;
        line[scanline].vertex_s.x = vertex->X;
        line[scanline].vertex_s.y = vertex->Y;
        line[scanline].vertex_s.z = vertex->Z;
      } else {
        // Not a vertex, so clear isvertex_s flag
        line[scanline].isvertex_s = FALSE;
      }
    } else {
      // Doesn't go in startx value, but maybe it goes in endx value
      if (line[scanline].endx > -9999999.9)
      {
        // This endx value is in use...
        if (xval > line[scanline].endx)
        {
          // Replace endx value because this one is larger
          line[scanline].endx = xval;

          if (vertex)
          {
            line[scanline].isvertex_e = TRUE;
            line[scanline].vertex_e.x = vertex->X;
            line[scanline].vertex_e.y = vertex->Y;
            line[scanline].vertex_e.z = vertex->Z;
          } else {
            // Not a vertex, so clear the isvertex_e flag
            line[scanline].isvertex_e = FALSE;
          }
        }
      } else {
        // endx value is not in use, so fill it in
        line[scanline].endx = xval;

        if (vertex)
        {
          // This is a vertex, so store it as such
          line[scanline].isvertex_e = TRUE;
          line[scanline].vertex_e.x = vertex->X;
          line[scanline].vertex_e.y = vertex->Y;
          line[scanline].vertex_e.z = vertex->Z;
        } else {
          // Not a vertex, so clear isvertex_s flag
          line[scanline].isvertex_e = FALSE;
        }
      }
    }
  } else {
    // startx value has not been filled yet, so fill it
    line[scanline].startx = xval;

    if (vertex)
    {
      // This is a vertex, so store it as such
      line[scanline].isvertex_s = TRUE;
      line[scanline].vertex_s.x = vertex->X;
      line[scanline].vertex_s.y = vertex->Y;
      line[scanline].vertex_s.z = vertex->Z;
    } else {
      // Not a vertex, so clear isvertex_s flag
      line[scanline].isvertex_s = FALSE;
    }
  }
}

lmscanline::lmscanline()
{
}

void lmscanline::clearlines()
{
  for (int i = 0; i < MAX_SLINES; i++)
  {
    line[i].startws.x = line[i].startws.y = line[i].startws.z = 0.0;
    line[i].endws.x = line[i].endws.y = line[i].endws.z = 0.0;
    line[i].startlm = -1;
    line[i].endlm = -1;
  }
}

void lmscanline::addpoint(int scanline, Point3D wspoint, int lmxpoint)
{
  if ((line[scanline].startlm > -1) && (line[scanline].endlm > -1))
  {
    return;
  }

  if (line[scanline].startlm == -1)
  {
    // Move new stuff into start
    line[scanline].startlm = lmxpoint;
    line[scanline].startws.x = wspoint.x;
    line[scanline].startws.y = wspoint.y;
    line[scanline].startws.z = wspoint.z;
  } else {
    if (line[scanline].startlm < lmxpoint)
    {
      // Move new stuff into end
      line[scanline].endlm = lmxpoint;
      line[scanline].endws.x = wspoint.x;
      line[scanline].endws.y = wspoint.y;
      line[scanline].endws.z = wspoint.z;
    } else {
      // Move start stuff to end stuff
      line[scanline].endlm = line[scanline].startlm;
      line[scanline].endws.x = line[scanline].startws.x;
      line[scanline].endws.y = line[scanline].startws.y;
      line[scanline].endws.z = line[scanline].startws.z;

      // Now put new stuff in the start position
      line[scanline].startlm = lmxpoint;
      line[scanline].startws.x = wspoint.x;
      line[scanline].startws.y = wspoint.y;
      line[scanline].startws.z = wspoint.z;
    }
  }
}


GFXTextureHandle CConstructiveBsp::GetNextCacheKey(CBspNode* node)
{
  (KeyValue.key[0])++;
  return KeyValue;
}

enum Constants {
		MaxTextureCoor = 256,
		MaxTextureMask = MaxTextureCoor - 1,
};

void PolyVertex::set(PolyVertex& start, PolyVertex& end,float scale)
 {
	TDPolyVertex::set(start,end,scale);
	texture.x = start.texture.x + (end.texture.x - start.texture.x) * scale;
	texture.y = start.texture.y + (end.texture.y - start.texture.y) * scale;
}

//----------------------------------------------------------------------------
// Rotate the poly so that the first three points are not colinear.
//
void Poly::rotate()
{
	// Find the best 3 points.
	if (vertexList.size() < 4)
		return;
	double bestLen = .0f;
	double bestVertex = 0;
	for (int v1 = 0; v1 < vertexList.size(); v1++) {
		int v2 = v1 + 1;
		if (v2 > vertexList.size() - 1)
			v2 = 0;
		int v3 = v2 + 1;
		if (v3 > vertexList.size() - 1)
			v3 = 0;
		Point3D vec1 = vertexList[v1].point;
		Point3D vec2 = vertexList[v3].point;
		vec1 -= vertexList[v2].point;
		vec2 -= vertexList[v2].point;

		// Should really normalize the vectors to get a 
		// correct sin value for the angle.
		Point3D vec3;
		md_cross(vec1,vec2,&vec3);
		double vl = vec3.len();
		if (vl > bestLen) {
			bestLen = vl;
			bestVertex = v1;
		}
	}

	// Rotate the best vertex to the start.
	while (bestVertex--) {
		PolyVertex t = vertexList[0];
		memmove(&vertexList[0],&vertexList[1],
			(vertexList.size() - 1) * sizeof(PolyVertex));
		vertexList.last() = t;
	}
}

//----------------------------------------------------------------------------
bool Poly::isPointPtr(Point3D* ptr)
{
	// Returns true if the pointer points to one of poly's points.
	for (int i = 0; i < vertexList.size(); i++)
		if (ptr == &vertexList[i].point)
			return true;
	return false;
}


//----------------------------------------------------------------------------
// CConstructiveBsp

CConstructiveBsp::CConstructiveBsp()
{
	TreeTop = NULL;
	mNodes = mCoplanarNodes = mOriginalPolys = mSplits = mTotalPolys = 0;
	mBrushList = NULL;
	mQuickBspNodes = NULL;
	mNumberOfQuickNodes = 0;

	poly_mat_list = NULL;
    found = false;

    localpBuffer = NULL;
    
   // generic lightmap...
   genericLightmap = gfxLightMapCache.allocate( 5300 );
   UInt16 * ptr = &(genericLightmap->data );
   memset( ptr, 0xff, 5300 * 2 );
}

CConstructiveBsp::~CConstructiveBsp()
{
	if(mQuickBspNodes)
		delete []mQuickBspNodes;
	ClearBsp();
}


//===============================================================
// This function will add a polygon to the into the tree.
// These polygons are just added in the order that they are 
// received so that there is no optimization of this tree.
//===============================================================
int CConstructiveBsp::AddPolygonToTree(CBspNode* Polygon)
{
	CBspNode* NewNode;
	// we basically want to take this polygon and add it into
	// the tree splitting it as necessary
	// This means that the polygon "filters" down the tree
	// possibly getting split as it goes.
	mOriginalPolys++;
	// if we have the special case of an empty tree.
	if(!TreeTop)
    {
		// add our polygon into our list
		NewNode = new CBspNode;
		*NewNode = *Polygon;
		TreeTop = NewNode;
        
        return 0;
	}

	// otherwise we want to filter the polygon down the tree
	// until it is added in
	  FilterPolygonThroughTree(TreeTop, Polygon, ADD_BRUSH);

	return 0;
}

//===============================================================
// This function draws the pre-transformed polygon
// onto the screen
//===============================================================
#define MAX_DRAW_VERTS	40
void CConstructiveBsp::DrawNode(CBspNode* Node, CRenderCamera* Camera)
{
	CDC* pDC = Camera->GetDC();
	CPoint PointList[MAX_DRAW_VERTS];

	// if we have no node then return
	if(!Node)
		return;

	// if we aren't supposed to draw this
	// then make it a different colour
	if(!Node->mValidated) {
		// now call ourselves to draw any coplanars
		if(Node->CoplanarChild)
			DrawNode(Node->CoplanarChild, Camera);
		return;
	}

	// make sure we don't frig
	_ASSERTE(Node->Polygon.NumberOfPoints < MAX_DRAW_VERTS);


	// copy the verts over to our pointlist
	for(int CurrentVert = 0; CurrentVert < Node->Polygon.NumberOfPoints; CurrentVert++) {

		// we have to transform these suckers
		ThredPoint TranPoint;
		TranPoint = Node->Polygon.Points[CurrentVert];
		Camera->DoPerspectiveTransform(TranPoint);
		
		// If any of the verts are behind us then drop this gon
		if(TranPoint.Z <= 0) {
			// now call ourselves to draw any coplanars
			if(Node->CoplanarChild) {
				DrawNode(Node->CoplanarChild, Camera);
			}
			return;
		}

		// else copy over the verts we need
		PointList[CurrentVert].x = (int)TranPoint.X;
		PointList[CurrentVert].y = (int)TranPoint.Y;
	}

	// now draw the bloody polygon
	pDC->Polygon(PointList, Node->Polygon.NumberOfPoints);

	// now call ourselves to draw any coplanars
	if(Node->CoplanarChild)
		DrawNode(Node->CoplanarChild, Camera);
}

// this function resets all of the new nodes in the tree
// so that we can add another brush to the tree.
void CConstructiveBsp::ResetNewNodes()
{
	ResetNewNodes(TreeTop);
}


// rescurse through this part of the tree and
// reset the mNewNode var
void CConstructiveBsp::ResetNewNodes(CBspNode* Node)
{
	// make sure we have something
	if(!Node)
		return;

	// reset the vars
	Node->mNewNode = FALSE;

	// recurse
	ResetNewNodes(Node->FrontChild);
	ResetNewNodes(Node->BackChild);
}

//=================================================================
// This function resets the bsp to nothing
//=================================================================
void CConstructiveBsp::ClearBsp()
{
    ThepFrame = (CMainFrame*)AfxGetMainWnd();
    pcount = 0;
    currdelnode = NULL;
    topdelnode = NULL;

	DeleteTree(TreeTop);
	TreeTop = NULL;

    CBspNode* ptr = topdelnode;
    CBspNode* lptr;

    while (ptr)
    {
      lptr = ptr;
      ptr = ptr->NextNode;
      delete lptr;

      if (showpcount)
      {
        pcount++;
        sprintf(pcountbuf, "Deleted %d Nodes", pcount);
        ThepFrame->UpdateGeneralStatus(pcountbuf);
      }
    }

	mNodes = mCoplanarNodes = mOriginalPolys = mSplits = mTotalPolys = 0;
}

void	CConstructiveBsp::FilterBrushBsp(CConstructiveBsp& BrushBsp)
{
	FilterBrushBsp(TreeTop, BrushBsp);
}
#if 0
void CConstructiveBsp::DeleteTree(CBspNode* Node)
{
	// get out of here if no node
	if(!Node)
    {
		return;
    }

    CBspNode* front;
    CBspNode* back;
    CBspNode* tempnode;

    front = Node->FrontChild;
    back = Node->BackChild;

    tempnode = Node->CoplanarChild;
    while(tempnode)
    {
      CBspNode* ptr = tempnode;
      tempnode = tempnode->CoplanarChild;
      if (topdelnode)
      {
        currdelnode->NextNode = ptr;
        currdelnode = ptr;
        currdelnode->NextNode = NULL;
      } else {
        topdelnode = ptr;
        currdelnode = topdelnode;
        currdelnode->NextNode = NULL;
      }
      //delete ptr;
    }

    if (currdelnode)
    {
      currdelnode->NextNode = Node;
      currdelnode = Node;
      currdelnode->NextNode = NULL;
    } else {
      topdelnode = Node;
      currdelnode = topdelnode;
      currdelnode->NextNode = NULL;
    }
    //delete Node;

    if (front)
    {
      DeleteTree(front);
    }

    if (back)
    {
      DeleteTree(back);
    }
}
#endif
//===========================================================
// This is a recursize function to delete the bsp.
//===========================================================
//#if 0
void CConstructiveBsp::DeleteTree(CBspNode* Node)
{
	// get out of here if no node
	if(!Node)
		return;

	// delete our subtree
	DeleteTree(Node->BackChild);
	DeleteTree(Node->FrontChild);

	// delete all of our co-planar gons
	do {
		CBspNode* NextNode = Node->CoplanarChild;
        
        //if ((Node->Polygon.lm) && (GFXMetrics.lightMapsInUse > 0))
        //{
        //  gfxLightMapCache.release(Node->Polygon.lm);
        //}

		delete Node;
		Node = NextNode;
	} while(Node);
}
//#endif
//====================================================================
// This function will basically add a polygon to the tree if it
// lands on an INSIDE node instead of an outside node.
//====================================================================
int CConstructiveBsp::SubtractPolygonFromTree(CBspNode* Polygon)
{
	// we basically want to take this polygon and add it into
	// the tree splitting it as necessary
	// This means that the polygon "filters" down the tree
	// possibly getting split as it goes.

	// if we have the special case of an empty tree.
	// obviously it ain't gunna be inside!!
	if(!TreeTop)
    {
		return 0;
	}
	mOriginalPolys++;

	// otherwise we want to filter the polygon down the tree
	// until it is added in
	  FilterPolygonThroughTreeSubtract(TreeTop, Polygon, SUBTRACT_BRUSH);

	return 0;
}

// This function will do the exporting
void CConstructiveBsp::Export(CString& Filename, CTHREDDoc* pDoc)
{
	// make the thing
	C3DMWorld *ThreeDeeEm;
	ThreeDeeEm = new C3DMWorld;

	// do the export
	ThreeDeeEm->Export3DM(TreeTop, Filename, pDoc);

	// get rid of it
	delete ThreeDeeEm;

}

//===================================================
// This function will bring up the bsp manager
//===================================================
void CConstructiveBsp::BspManager()
{
	CBspManagerDialog dialog;

	// set some stuff to nothing
	mNodes = mCoplanarNodes = mSplits = mTotalPolys = 0;

	// collect some information
	CollectBspStats(TreeTop);

	// our splits are our total nodes - original
	mSplits = (mCoplanarNodes+mNodes) - mOriginalPolys;

	dialog.m_CoplanarNodes = mCoplanarNodes;
	dialog.m_Nodes = mNodes;
	dialog.m_OriginalPolys = mOriginalPolys;
	dialog.m_Splits = mSplits;
	dialog.m_TotalPolys = mTotalPolys;

	dialog.DoDialog(this);
}

void CConstructiveBsp::CollectBspStats(CBspNode* Node)
{
	// get out of here if no node
	if(!Node)
		return;

	// delete our subtree
	CollectBspStats(Node->BackChild);
	CollectBspStats(Node->FrontChild);

	// inc our node count
	mNodes++;
	if(Node->mValidated)
		mTotalPolys++;

	// count all of our co-planar gons
	do {
		CBspNode* NextNode = Node->CoplanarChild;
		Node = NextNode;
		mCoplanarNodes++;
	} while(Node);
	mCoplanarNodes--;
}

void CConstructiveBsp::RebuildQuick(CProgressCtrl* Progress, int BuildWater, bool do_csg_and_tree)
{
	CThredBrush* CurrentBrush;
	int Brush = 0;

	// Clear the bsp
	ClearBsp();

	// Rebuild the bsp in order it was made.
	CurrentBrush = mBrushList->GetActiveBrush();
	while(CurrentBrush) {
			if(Progress)
				Progress->StepIt();
			if(BuildWater || CurrentBrush->mName[0] != '*')
				ProcessBrush(CurrentBrush, Brush, do_csg_and_tree);
		Brush++;
		CurrentBrush = CurrentBrush->NextActiveBrush();
	}
}

// Invalidates all polygons' cache keys so new lightmaps
// will be created at render time...
void CConstructiveBsp::SetLighting(CBspNode* Node)
{
    InvalidateNodeCacheKeys(Node);
//    AssignGenericLightMaps(Node, theApp.lightval);
}

void CConstructiveBsp::ValidateNodeTextureIndex(CBspNode* Node, int TMax)
{
    CBspNode*   tempnode;

	// we are at a terminating leaf.
	if(!Node)
	  return;

    tempnode = Node;

    while(tempnode)
    {
      if (tempnode->mBrushPtr)
      {
        if (tempnode->mBrushPtr->mTextureID[tempnode->mPolygonNumber].TextureID >= TMax)
        {
          tempnode->mBrushPtr->mTextureID[tempnode->mPolygonNumber].TextureID = 0;
          tempnode->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(tempnode);
//          AssignLightMap(tempnode, theApp.lightval);  // DPW - 9-26
        }
      }

      tempnode = tempnode->CoplanarChild;
    }

    if (Node->BackChild)
    {
      ValidateNodeTextureIndex(Node->BackChild, TMax);
    }

    if (Node->FrontChild)
    {
	  ValidateNodeTextureIndex(Node->FrontChild, TMax);
    }
}

void CConstructiveBsp::InvalidateNodeCacheKeys (CBspNode* Node)
{
    CBspNode*   tempnode;

	// we are at a terminating leaf.
	if(!Node)
	  return;

    tempnode = Node;

    while(tempnode)
    {
      tempnode->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(tempnode);
//      AssignLightMap(tempnode, theApp.lightval);  // DPW - 9-26
      tempnode = tempnode->CoplanarChild;
    }

    if (Node->BackChild)
    {
      InvalidateNodeCacheKeys(Node->BackChild);
    }

    if (Node->FrontChild)
    {
	  InvalidateNodeCacheKeys(Node->FrontChild);
    }
}

void CConstructiveBsp::ProcessCacheInfoHere(CBspNode* Node, bool do_poly_splitting)
{
    int         i;
    Poly        apolygon;
    PolyVertex  avert;
    PolyList    polyList;
    CBspNode*   last_coplanar;
    CBspNode*   tempnode;

    // Set up initial polygon
    for (i = 0; i < Node->Polygon.NumberOfPoints; i++)
    {
      avert.point.x = Node->Polygon.Points[i].X;
      avert.point.y = Node->Polygon.Points[i].Z;
      avert.point.z = Node->Polygon.Points[i].Y;
      avert.texture.x = Node->Polygon.mTextureUV[i].x;
      avert.texture.y = Node->Polygon.mTextureUV[i].y;
      apolygon.vertexList.push_back(avert);
    }

    apolygon.textureSize.x = Node->Polygon.mTextureSize.x;
    apolygon.textureSize.y = Node->Polygon.mTextureSize.y;

    if (Node->mBrushPtr)
    {
      apolygon.textureOffset.x = (Node->Polygon.mTextureOffset.x + Node->mBrushPtr->mTextureID[Node->mPolygonNumber].TextureShift.x) % 256;
      apolygon.textureOffset.y = (Node->Polygon.mTextureOffset.y + Node->mBrushPtr->mTextureID[Node->mPolygonNumber].TextureShift.y) % 256;
    } else {
      apolygon.textureOffset.x = Node->Polygon.mTextureOffset.x;
      apolygon.textureOffset.y = Node->Polygon.mTextureOffset.y;
    }

// old method for generating the plane equation ( not very acurate )
#if 0
    Point3D   p1, p2, p3;
    int       last = Node->Polygon.NumberOfPoints-1;
    p1.set(Node->Polygon.Points[0].X, Node->Polygon.Points[0].Z, Node->Polygon.Points[0].Y);
    p2.set(Node->Polygon.Points[1].X, Node->Polygon.Points[1].Z, Node->Polygon.Points[1].Y);
    p3.set(Node->Polygon.Points[last].X, Node->Polygon.Points[last].Z, Node->Polygon.Points[last].Y);
    apolygon.plane.set (p1, p2, p3);
#endif

// use all the points to generate the plane equation
#if 1
   Point3D * pointArray[ MAX_POLYGON_VERTS ];
   Point3D vertexList[ MAX_POLYGON_VERTS ];
   
   // add all the points into the point array
   for( i = 0; i < Node->Polygon.NumberOfPoints; i++ )
   {
      vertexList[i].set( Node->Polygon.Points[i].X, Node->Polygon.Points[i].Z, Node->Polygon.Points[i].Y );
      pointArray[i] = &vertexList[i];
   }
   
   apolygon.plane.set( ( int )Node->Polygon.NumberOfPoints, pointArray );
#endif       
    
    apolygon.handle = Node->Polygon.texture_handle;
    apolygon.material = Node->Polygon.material_index;
    polyList.push_back(&apolygon);

    if (do_poly_splitting)
    {
      for (int p = 0; p < polyList.size(); p++)
      {
        if (polyList[p]->textureSize.x > 256)
        {
          splitX(polyList[p], &polyList);
        }

        if (polyList[p]->textureSize.y > 256)
        {
          splitY(polyList[p], &polyList);
        }
      }
    }

    last_coplanar = Node->CoplanarChild;

    // Count through all the resultant polygons that we created & make 'em coplanars
    // (The node's new values are placed back in the original node so start loop at 1 not 0)
    for (int m = 1; m < polyList.size(); m++)
    {
      tempnode = new CBspNode;

      tempnode->BackChild = NULL;
      tempnode->FrontChild = NULL;
      tempnode->ConstantA = Node->ConstantA;
      tempnode->ConstantB = Node->ConstantB;
      tempnode->ConstantC = Node->ConstantC;
      tempnode->ConstantD = Node->ConstantD;
      tempnode->CoplanarChild = last_coplanar;
      last_coplanar = tempnode;
      tempnode->mBrushId = Node->mBrushId;
      tempnode->mBrushPtr = Node->mBrushPtr;
      tempnode->mNewNode = Node->mNewNode;
      tempnode->mPolygonNumber = Node->mPolygonNumber;
      tempnode->mReverseRef = Node->mReverseRef;
      tempnode->mTextureShift.x = Node->mTextureShift.x;
      tempnode->mTextureShift.y = Node->mTextureShift.y;
      tempnode->mValidated = Node->mValidated;
      tempnode->NextNode = Node->NextNode;  // Not sure if this is right...
      tempnode->Splitter = Node->Splitter;

      tempnode->Polygon.Colour = Node->Polygon.Colour;
      tempnode->Polygon.volumestate = Node->Polygon.volumestate;
      tempnode->Polygon.material_index = Node->Polygon.material_index;  // This is bogus right now...
      tempnode->Polygon.mTextureScaleShift = Node->Polygon.mTextureScaleShift;
      tempnode->Polygon.mApplyAmbient = Node->Polygon.mApplyAmbient;
      tempnode->Polygon.mTextureOffset.x = polyList[m]->textureOffset.x;
      tempnode->Polygon.mTextureOffset.y = polyList[m]->textureOffset.y;
      tempnode->Polygon.mTextureSize.x = polyList[m]->textureSize.x;
      tempnode->Polygon.mTextureSize.y = polyList[m]->textureSize.y;
      if ((tempnode->Polygon.mTextureSize.x > 256) ||
          (tempnode->Polygon.mTextureSize.y > 256))
      {
        int aaa;

        aaa += 1;
      }
      tempnode->Polygon.NumberOfPoints = polyList[m]->vertexList.size();
      tempnode->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(tempnode);
      //AssignLightMap(tempnode, theApp.lightval);  // DPW - 9-26

      // Loop through vertices and fill in necessary information...
      for (i = 0; i < polyList[m]->vertexList.size(); i++)
      {
        tempnode->Polygon.Points[i].X = polyList[m]->vertexList[i].point.x;
        tempnode->Polygon.Points[i].Y = polyList[m]->vertexList[i].point.z;
        tempnode->Polygon.Points[i].Z = polyList[m]->vertexList[i].point.y;

        tempnode->Polygon.mTextureUV[i].x = polyList[m]->vertexList[i].texture.x;
        tempnode->Polygon.mTextureUV[i].y = polyList[m]->vertexList[i].texture.y;
      }
    }

    // Fix up original node...
    for (i = 0; i < polyList[0]->vertexList.size(); i++)
    {
        Node->Polygon.Points[i].X = polyList[0]->vertexList[i].point.x;
        Node->Polygon.Points[i].Y = polyList[0]->vertexList[i].point.z;
        Node->Polygon.Points[i].Z = polyList[0]->vertexList[i].point.y;

        Node->Polygon.mTextureUV[i].x = polyList[0]->vertexList[i].texture.x;
        Node->Polygon.mTextureUV[i].y = polyList[0]->vertexList[i].texture.y;
    }

    Node->Polygon.mTextureOffset.x = polyList[0]->textureOffset.x;
    Node->Polygon.mTextureOffset.y = polyList[0]->textureOffset.y;
    Node->Polygon.mTextureSize.x = polyList[0]->textureSize.x;
    Node->Polygon.mTextureSize.y = polyList[0]->textureSize.y;

    if ((Node->Polygon.mTextureSize.x > 256) ||
        (Node->Polygon.mTextureSize.y > 256))
    {
      int aaa;

      aaa += 1;
      //Node->mValidated = FALSE;
      //Node->mValidated = FALSE;
    }// else {  // DPW - added else 10/1
      Node->Polygon.NumberOfPoints = polyList[0]->vertexList.size();
      Node->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(Node);
      //AssignLightMap(Node, theApp.lightval);  // DPW - 9-26
    //}

    Node->CoplanarChild = last_coplanar;

    // start with 1 since we locally allocated first poly
    for (i = 1; i < polyList.size(); i++)
    {
      Poly* tmp;
      tmp = polyList[i];
      delete tmp;
    }
}


void CConstructiveBsp::ProcessTreeCacheInfo(CBspNode* Node, bool do_poly_splitting)
{
    CBspNode*   tempnode;
    CBspNode*   tnode;

	// we are at a terminating leaf.
	if(!Node)
	  return;

    tempnode = Node->CoplanarChild;
    ProcessCacheInfoHere(Node, do_poly_splitting);

    while(tempnode)
    {
      tnode = tempnode;
      tempnode = tempnode->CoplanarChild;

	  ProcessCacheInfoHere(tnode, do_poly_splitting);
    }

    if (Node->BackChild)
    {
      ProcessTreeCacheInfo(Node->BackChild, do_poly_splitting);
    }

    if (Node->FrontChild)
    {
	  ProcessTreeCacheInfo(Node->FrontChild, do_poly_splitting);
    }
}

GFXLightMap* CConstructiveBsp::CopyLightMap(GFXLightMap* src)
{
  GFXLightMap*  returnlm;

  returnlm = gfxLightMapCache.allocate(src->size.x * src->size.y);
  returnlm->offset.x = src->offset.x;
  returnlm->offset.y = src->offset.y;
  returnlm->size.x = src->size.x;
  returnlm->size.y = src->size.y;

  UInt16* srcptr = &(src->data);
  UInt16* destptr = &(returnlm->data);

  for (int i = 0; i < (src->size.x * src->size.y); i++)
  {
    destptr[i] = srcptr[i];
  }

  return returnlm;
}

void CConstructiveBsp::AssignLightMap(CBspNode* Node, int inval)
{
  Point2I offset, size;
  int i;
  UInt16  lightval;
  Point2I temp(0,0);
  CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

  offset.x = Node->Polygon.mTextureOffset.x & 0xF;
  offset.y = Node->Polygon.mTextureOffset.y & 0xF;
  size.x = ((Node->Polygon.mTextureSize.x + 1 + offset.x + 8) >> 4) + 2;
  size.y = ((Node->Polygon.mTextureSize.y + 1 + offset.y + 8) >> 4) + 2;

  Node->Polygon.lm = gfxLightMapCache.allocate(size.x * size.y);
  Node->Polygon.lm->offset.x = offset.x;
  Node->Polygon.lm->offset.y = offset.y;
  Node->Polygon.lm->size.x = size.x;
  Node->Polygon.lm->size.y = size.y;

  lightval = ((inval << 12) + 0x0FFF) & 0xFFFF;
  UInt16* ptr = &(Node->Polygon.lm->data);
  for (i = 0; i < (size.x * size.y); i++)
  {
    ptr[i] = lightval;
  }
}

void CConstructiveBsp::AssignGenericLightMaps(CBspNode* Node, int inval)
{
    CBspNode*   tempnode;
    //CBspNode*   tnode;

	// we are at a terminating leaf.
	if(!Node)
	  return;

    //tempnode = Node->CoplanarChild;
    //AssignLightMap(Node, inval);

    tempnode = Node;

    while(tempnode)
    {
      //tnode = tempnode;
      //tempnode = tempnode->CoplanarChild;
	  //AssignLightMap(tnode, inval);
	  AssignLightMap(tempnode, inval);
      tempnode = tempnode->CoplanarChild;
    }

    if (Node->BackChild)
    {
      AssignGenericLightMaps(Node->BackChild, inval);
    }

    if (Node->FrontChild)
    {
	  AssignGenericLightMaps(Node->FrontChild, inval);
    }
}

void CConstructiveBsp::CalculateLightMaps(CBspNode* Node, 
   ThredPoint* lightpos,  double lightdist1, double lightdist2, 
   double maxintensity)
{
    CBspNode*   tempnode;

	// we are at a terminating leaf.
	if(!Node)
		return;
      
    // check which side of the light we are on
	if(Node->ClassifyPoint(lightpos) == POLYGON_IN_FRONT)
    {
		CalculateLightMaps(Node->FrontChild, lightpos, lightdist1, lightdist2, maxintensity);

        tempnode = Node;
        while (tempnode)
        {
          int         i, row, col;
          double      curru, denom;
          double      dx, dy, du, dz;
          double      currx, curry, currz;
          Point3D     line_o_sight, l1;
          Point3D     wp;
          ThredPoint  start, end;
          Point2F     startlmc, endlmc;

          lmscans.clearlines();

          double  xstep, ystep;
          Point2F tsize;

          xstep = double(tempnode->Polygon.lm->size.x);
          ystep = double(tempnode->Polygon.lm->size.y);
          tsize.x = (double)tempnode->Polygon.mTextureSize.x;
          tsize.y = (double)tempnode->Polygon.mTextureSize.y;

          for (i = 0; i < tempnode->Polygon.NumberOfPoints; i++)
          {
            int nextvertex = (i + 1) % tempnode->Polygon.NumberOfPoints;

            // This is where on the lightmap this point lands...
            startlmc.x = floor((tempnode->Polygon.mTextureUV[i].x / tsize.x) * xstep);
            startlmc.y = floor((tempnode->Polygon.mTextureUV[i].y / tsize.y) * ystep);
            endlmc.x = floor((tempnode->Polygon.mTextureUV[nextvertex].x / tsize.x) * xstep);
            endlmc.y = floor((tempnode->Polygon.mTextureUV[nextvertex].y / tsize.y) * ystep);

            if (startlmc.y < endlmc.y)
            {
              wp.x = tempnode->Polygon.Points[i].X;
              wp.y = tempnode->Polygon.Points[i].Y;
              wp.z = tempnode->Polygon.Points[i].Z;
              curru = startlmc.x;

              denom = (double)(endlmc.y - startlmc.y);

              dx = (tempnode->Polygon.Points[nextvertex].X - wp.x) / denom;
              dy = (tempnode->Polygon.Points[nextvertex].Y - wp.y) / denom;
              dz = (tempnode->Polygon.Points[nextvertex].Z - wp.z) / denom;
              du = (double)(endlmc.x - startlmc.x) / denom;

              for (row = (int)startlmc.y; row <= (int)endlmc.y; row++)
              {
                lmscans.addpoint(row, wp, (int)curru);
                wp.x += dx;
                wp.y += dy;
                wp.z += dz;
                curru += du;
              }
            } else {
              wp.x = tempnode->Polygon.Points[nextvertex].X;
              wp.y = tempnode->Polygon.Points[nextvertex].Y;
              wp.z = tempnode->Polygon.Points[nextvertex].Z;
              curru = endlmc.x;

              if (endlmc.y == startlmc.y)
              {
                // Horizontal scanline
                lmscans.addpoint((int)startlmc.y, wp, (int)endlmc.x);
                wp.x = tempnode->Polygon.Points[i].X;
                wp.y = tempnode->Polygon.Points[i].Y;
                wp.z = tempnode->Polygon.Points[i].Z;
                lmscans.addpoint((int)startlmc.y, wp, (int)startlmc.x);
              } else {
                denom = (double)(startlmc.y - endlmc.y);

                dx = (tempnode->Polygon.Points[i].X - wp.x) / denom;
                dy = (tempnode->Polygon.Points[i].Y - wp.y) / denom;
                dz = (tempnode->Polygon.Points[i].Z - wp.z) / denom;
                du = (double)(startlmc.x - endlmc.x) / denom;

                for (row = (int)endlmc.y; row <= (int)startlmc.y; row++)
                {
                  lmscans.addpoint(row, wp, (int)(curru));
                  wp.x += dx;
                  wp.y += dy;
                  wp.z += dz;
                  curru += du;
                }
              }
            }
          }

          for (row = 0; row < tempnode->Polygon.lm->size.y; row++)
          {
            if ((lmscans.line[row].startlm == -1) || (lmscans.line[row].endlm == -1))
            {
              continue;
            }

            denom = (double)(tempnode->Polygon.lm->size.x + 1.0);

            if (denom)
            {
              dx = (lmscans.line[row].endws.x - lmscans.line[row].startws.x) / denom;
              dy = (lmscans.line[row].endws.y - lmscans.line[row].startws.y) / denom;
              dz = (lmscans.line[row].endws.z - lmscans.line[row].startws.z) / denom;
            } else {
              dx = dy = dz = 0.0;
            }

            currx = lmscans.line[row].startws.x;
            curry = lmscans.line[row].startws.y;
            currz = lmscans.line[row].startws.z;

            for (col = 0; col < tempnode->Polygon.lm->size.x; col++)
            {
//#if 0
              if (col < lmscans.line[row].startlm)
              {
                currx = lmscans.line[row].startws.x;
                curry = lmscans.line[row].startws.y;
                currz = lmscans.line[row].startws.z;
              }

              if (col > lmscans.line[row].endlm)
              {
                currx = lmscans.line[row].endws.x - dx;
                curry = lmscans.line[row].endws.y - dy;
                currz = lmscans.line[row].endws.z - dz;
              }
//#endif
                l1.x = line_o_sight.x = currx - lightpos->X;
                l1.z = line_o_sight.z = curry - lightpos->Y;
                l1.y = line_o_sight.y = currz - lightpos->Z;
                line_o_sight.normalize();

                // Walk the bsp tree (front to back) to test if this vector directly reaches this node
                found = false;
                found_distance = 0.0;

                WalkSolidTreeFrontToBack(TreeTop, lightpos, line_o_sight);

                UInt16* ptr = &tempnode->Polygon.lm->data;
              
                if (found)
                {
                  found = FALSE;
                  CBspNode* nptr = Node;
                  while (nptr)
                  {
                    if (found_node == nptr)
                    {
                      found = TRUE;
                      nptr = NULL;
                    } else {
                      nptr = nptr->CoplanarChild;
                    }
                  }

                  if(found)
                  {
                    // vector reaches the lightmap, so fill in correct value
                    double distance = sqrt((l1.x * l1.x) + (l1.y * l1.y) + (l1.z * l1.z));
                
                    if (distance < lightdist2)
                    {
                      int intensity;
                      
                      if( distance < lightdist1 )
                         intensity = ( int )( maxintensity * 15 );
                      else
                      {
                        intensity = (int)(((((lightdist2 - lightdist1 ) - ( distance - lightdist1 ) ) / (lightdist2 - lightdist1)) * maxintensity) * 15);
                        int existingintensity = (ptr[(row*tempnode->Polygon.lm->size.x)+col] & 0xF000) >> 12;
                        intensity = intensity + existingintensity;
                      }
                      
                      if (intensity > 15)
                      {
                        intensity = 15;
                      }

                      ptr[(row*tempnode->Polygon.lm->size.x)+col] = ((intensity << 12) + 0x0FFF) & 0xFFFF;
                    }
                  } else {
                    // not found
                    double distance = sqrt((l1.x * l1.x) + (l1.y * l1.y) + (l1.z * l1.z));
                    double dnear = found_node->NearestVertexDistance(lightpos);

                    if ((found_distance + 10.0) > distance)
                    {
                      if (distance < lightdist2)
                      {
                        int intensity;
                        if( distance < lightdist1 )
                           intensity = (int)( maxintensity * 15 );
                        else
                        {
                           intensity = (int)(((((lightdist2 - lightdist1 ) - ( distance - lightdist1 ) ) / (lightdist2 - lightdist1 )) * maxintensity) * 15);
                           int existingintensity = (ptr[(row*tempnode->Polygon.lm->size.x)+col] & 0xF000) >> 12;
                           intensity = intensity + existingintensity;
                        }
                        if (intensity > 15)
                        {
                          intensity = 15;
                        }

                        ptr[(row*tempnode->Polygon.lm->size.x)+col] = ((intensity << 12) + 0x0FFF) & 0xFFFF;
                      }
                    }
                  }
                } else {
                  double distance = sqrt((l1.x * l1.x) + (l1.y * l1.y) + (l1.z * l1.z));
                
                  if (distance < lightdist2)
                  {
                     int intensity;
                     if( distance < lightdist1 )
                        intensity = (int)( maxintensity * 15 );
                     else
                     {
                       intensity = (int)(((((lightdist2 - lightdist1 ) - ( distance - lightdist1 ) ) / (lightdist2 - lightdist1 )) * maxintensity) * 15);
                       int existingintensity = (ptr[(row*tempnode->Polygon.lm->size.x)+col] & 0xF000) >> 12;
                       intensity = intensity + existingintensity;
                     }
                    if (intensity > 15)
                    {
                      intensity = 15;
                    }

                    ptr[(row*tempnode->Polygon.lm->size.x)+col] = ((intensity << 12) + 0x0FFF) & 0xFFFF;
                  } else {
                    int aaa;
                    aaa += 1;
                  }
                }

                currx += dx;
                currz += dz;
                curry += dy;
            }
          }
#if 0
          // Average lightmap adjacent points
          UInt16* ptr = &tempnode->Polygon.lm->data;
          for (row = 0; row < tempnode->Polygon.lm->size.y; row++)
          {
            for (col = 0; col < tempnode->Polygon.lm->size.x; col++)
            {
              int index;
              int intensity, count;

              count = intensity = 0;

              for (int subrow = row - 1; subrow < row + 1; subrow++)
              {
                for (int subcol = col - 1; subcol < col + 1; subcol++)
                {
                  if ((subcol >= 0) && (subcol < tempnode->Polygon.lm->size.x) &&
                      (subrow >= 0) && (subrow < tempnode->Polygon.lm->size.y))
                  {
                    intensity += (ptr[(subrow*tempnode->Polygon.lm->size.x) + subcol] & 0xF000) >> 12;
                    count++;
                  }
                }
              }

              intensity = (int)(((double)intensity) / ((double)count));
              ptr[(row * tempnode->Polygon.lm->size.x) + col] = ((intensity << 12) + 0x0FFF) & 0xFFFF;
            }
          }
#endif

          tempnode = tempnode->CoplanarChild;
        }

		CalculateLightMaps(Node->BackChild, lightpos, lightdist1, lightdist2, maxintensity);
	} else {
		CalculateLightMaps(Node->BackChild, lightpos, lightdist1, lightdist2, maxintensity);

        CalculateLightMaps(Node->FrontChild, lightpos, lightdist1, lightdist2, maxintensity);
	}

	// tree is not full
	return;
}

void CConstructiveBsp::FindTextureCenter(CBspNode* curr_node, CThredBrush* brush, int which_poly)
{
  CBspNode* tempnode;

  if (!curr_node)
    return;

  if ((curr_node->mBrushPtr == brush) && (curr_node->mPolygonNumber == which_poly))
  {
    tempnode = curr_node;

    while (tempnode)
    {
      for (int i = 0; i < tempnode->Polygon.NumberOfPoints; i++)
      {
        tcenter.x += tempnode->Polygon.mTextureUV[i].x;
        tcenter.y += tempnode->Polygon.mTextureUV[i].y;
        tcount++;
      }

      tempnode = tempnode->CoplanarChild;
    }
  }
    if (curr_node->FrontChild)
      FindTextureCenter(curr_node->FrontChild, brush, which_poly);

    if (curr_node->BackChild)
      FindTextureCenter(curr_node->BackChild, brush, which_poly);
}

int CConstructiveBsp::NormalizeTreePolyTextures(CBspNode* Node)
{
    CBspNode*     tempnode;
    CThredBrush*  thebrush;
    Point2F       center(0.0, 0.0);

	// we are at a terminating leaf.
	if(!Node)
	  return 0;

    if (!Node->mBrushPtr)
      return 0;

    thebrush = Node->mBrushPtr;
    tempnode = Node;

    tcenter.x = 0.0;
    tcenter.y = 0.0;
    tcount = 0;
    //FindTextureCenter(tempnode, thebrush, tempnode->mPolygonNumber);
    if (tcount)
    {
      center.x = tcenter.x / ((double)tcount);
      center.y = tcenter.y / ((double)tcount);
    }

    while(tempnode)
    {
      if (tempnode->mBrushPtr != thebrush)
        return 0;

      tempnode->mTextureShift.x = tempnode->mBrushPtr->mTextureID[tempnode->mPolygonNumber].TextureShift.x;
      tempnode->mTextureShift.y = tempnode->mBrushPtr->mTextureID[tempnode->mPolygonNumber].TextureShift.y;
	  tempnode->normalizeTexture(&tempnode->Polygon, tempnode->mBrushPtr);//, center);

      tempnode = tempnode->CoplanarChild;
    }

    if (NormalizeTreePolyTextures(Node->BackChild))
		return 1;

	if(NormalizeTreePolyTextures(Node->FrontChild))
		return 1;

    // tree is not full
	return 0;
}

CBspNode* CConstructiveBsp::GetBestNode(CBspNode *PolyList)
{
    return PolyList;  // DPW - Who cares?!?!?!
#if 0
	CBspNode *CurrentPoly;
	CBspNode *BestPoly = NULL;
	int BestScore = 5555555, CurrentScore;
	int FrontCount, BackCount, Splits;
	int BalancePercent = 0;

	// Go through all the polygons in the list
	for(CurrentPoly = PolyList;CurrentPoly != NULL; CurrentPoly = CurrentPoly->NextNode) {

		// Check how many splits we have
		CountSplits(CurrentPoly, PolyList, FrontCount,	BackCount, Splits);

		// Get the current score
		CurrentScore = (BalancePercent * (abs(FrontCount - BackCount)))
						 + ((100 - BalancePercent) * Splits);

		// Check our score
		if(CurrentScore < BestScore) {
			BestPoly = CurrentPoly;
			BestScore = CurrentScore;
		}
	}

	return BestPoly;
#endif
}

void CConstructiveBsp::CountSplits(CBspNode* Splitter, CBspNode* PolyList, 
								   int& FrontCount, int& BackCount, int& Splits) 
{
	CBspNode* CurrentPoly;
	CBspNode* CoplanarPoly;

	// set our stats to nothing
	FrontCount = BackCount = Splits = 0;

	// go through and count our splitteroos
	for(CurrentPoly = PolyList;CurrentPoly != NULL; CurrentPoly = CurrentPoly->NextNode) {
		// make sure we don't do ourself
		if(CurrentPoly == Splitter)
			continue;


		// do this poly and all it's coplanars
		CoplanarPoly = CurrentPoly;
		while(CoplanarPoly) {
//			_ASSERTE(CoplanarPoly->mDrawPoly);
			if(CoplanarPoly->mValidated) {
				// classify and check
				switch(Splitter->ClassifyPolygon(&CoplanarPoly->Polygon)) {
				case POLYGON_COPLANAR_SAME:
					// Since this goes to the front while building we will
					// put it here for now.
				case POLYGON_IN_FRONT:
					FrontCount++;
					break;
				case POLYGON_IN_BACK:
				case POLYGON_COPLANAR_REVERSE:
					BackCount++;
					break;
				case POLYGON_SPLIT:
					Splits++;
					break;
				}
			}
			// go to our next co
			CoplanarPoly = CoplanarPoly->CoplanarChild;
		}
	}
}

//=============================================================
// This function goes through the list of polygons that we have
// made by filtering them through all of the brushes and
// deletes the polygons that are not drawn fron the list.
//=============================================================
void	CConstructiveBsp::DeleteNonDrawnPolygons(CBspNode** PolygonList)
{
   CBspNode* CurrentNode;
   CBspNode* ParentNode = NULL;
   CBspNode* CoplanarNode;
   int nodecount = 0;
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   char  buf[80];

   // go through the list and assign our nextnodes
   for(CurrentNode = *PolygonList; CurrentNode; CurrentNode = CurrentNode->NextNode)
   {
      CoplanarNode = CurrentNode;
      while(CoplanarNode)
      {
         CoplanarNode->NextNode = CurrentNode->NextNode;
         CoplanarNode = CoplanarNode->CoplanarChild;
      }
   }

   // go through the list and do some deletions
   CurrentNode = *PolygonList; 

   while(CurrentNode)
   {
      // delete our current node
      if(!CurrentNode->mValidated)
      {
         // if we have a coplanar child make IT next
         if(CurrentNode->CoplanarChild)
         {
            if(ParentNode)
            {
               // if we have a parent set this stuff up
               ParentNode->NextNode = CurrentNode->CoplanarChild;
               delete CurrentNode;
               CurrentNode = ParentNode->NextNode;
            }
            else 
            {
               // if we DON'T have a parent then this is the
               // first node
               if(CurrentNode == *PolygonList) 
               {
                  *PolygonList = CurrentNode->CoplanarChild;
                  delete CurrentNode;
                  CurrentNode = *PolygonList;
               }
            }
         } 
         else 
         {
            // if we DON"T have a coplanar child make
            // our nextnode it
            // Give some feedback...
            sprintf(buf, "Found %d polygons", nodecount);
            pFrame->UpdateGeneralStatus(buf);

            nodecount++;

            if(ParentNode)
            {
               // if we have a parent set this stuff up
               ParentNode->NextNode = CurrentNode->NextNode;
               delete CurrentNode;
               CurrentNode = ParentNode->NextNode;
            } 
            else 
            {
               // if we DON'T have a parent then this is the
               // first node
               if(CurrentNode == *PolygonList)
               {
                  *PolygonList = CurrentNode->NextNode;
                  delete CurrentNode;
                  CurrentNode = *PolygonList;
               }
            }
         }
         continue;
      }

      // now go through our coplanar gon's!
      CoplanarNode = CurrentNode->CoplanarChild;
      ParentNode = CurrentNode;
      while(CoplanarNode) 
      {
         // if we want to delete
         if(!CoplanarNode->mValidated) 
         {
            ParentNode->CoplanarChild = CoplanarNode->CoplanarChild;
            delete CoplanarNode;
            CoplanarNode = ParentNode;
         }

         // go to the next
         ParentNode = CoplanarNode;
         CoplanarNode = CoplanarNode->CoplanarChild;
      }

      // reset and go to the beginning
      ParentNode = CurrentNode;
      CurrentNode = CurrentNode->NextNode;
   }
}

void CConstructiveBsp::CalculateSubLevel(CBspNode *Node, CBspNode *MainPolygonList,
CBspNode **FrontPolygonList, CBspNode **BackPolygonList, CProgressCtrl* Progess)
{
	CBspNode *CurrentStartListPolygon, *NextListPolygon, *CurrentListPolygon, *NextStartListPolygon;
	CBspNode *CurrentFront, *CurrentBack, *NewNode;

	if(Progess)
		Progess->StepIt();

	// Loop through the main polygon list and generate front and back
	// polygon lists, splitting if necessary

	CurrentStartListPolygon = MainPolygonList;

	while(CurrentStartListPolygon != NULL) {

		// Store the next polygon pointer
		NextStartListPolygon = CurrentStartListPolygon->NextNode;

		CurrentListPolygon = CurrentStartListPolygon;
		while(CurrentListPolygon != NULL) {
			// Store the next polygon pointer
			NextListPolygon = CurrentListPolygon->CoplanarChild;

			// if this is set our list will be SO freaking screwed it's
			// not even FUNNY.
			CurrentListPolygon->CoplanarChild = NULL;

			// Make sure we don't compare against the parent polygon
			if(CurrentListPolygon == Node) {
				// we have to set this to null so that
				// we won't be doing weird fucked up rendering
				CurrentListPolygon = NextListPolygon;
				continue;
			}


			ThredPolygon NewPolygon;
			switch(Node->ClassifyPolygon(&CurrentListPolygon->Polygon)) {
			case POLYGON_COPLANAR_SAME:
				// this sucker is COplanar which means we add it to the NODE
				// as a new coplanar gon
				// UNFORTUNATELY WE CAN'T BE SURE that the coplanar child
				// is correct because we are using it in our list!! bummer.
				// got to fix this later.
			case POLYGON_IN_FRONT:
				// Add our currentlist poly to the front list of polygons
				if(*FrontPolygonList == NULL) {
					*FrontPolygonList = CurrentListPolygon;
					(*FrontPolygonList)->NextNode = NULL;
					CurrentFront = *FrontPolygonList;
				}
				else {
					CurrentFront->NextNode = CurrentListPolygon;
					CurrentFront = CurrentFront->NextNode;
					CurrentFront->NextNode = NULL;
				}
				break;
			case POLYGON_IN_BACK:
			case POLYGON_COPLANAR_REVERSE:
				if(*BackPolygonList == NULL) {
					*BackPolygonList = CurrentListPolygon;
					(*BackPolygonList)->NextNode = NULL;
					CurrentBack = *BackPolygonList;
				}
				else {
					CurrentBack->NextNode = CurrentListPolygon;
					CurrentBack = CurrentBack->NextNode;
					CurrentBack->NextNode = NULL;
				}
				break;
			case POLYGON_SPLIT:
				// split the polygons
				Node->SplitPolygon(&CurrentListPolygon->Polygon, &NewPolygon);

				// The front half is stored in place of the original polygon
				if(*FrontPolygonList == NULL) {
					*FrontPolygonList = CurrentListPolygon;
					(*FrontPolygonList)->NextNode = NULL;
					CurrentFront = *FrontPolygonList;
				}
				else {
					CurrentFront->NextNode = CurrentListPolygon;
					CurrentFront = CurrentFront->NextNode;
					CurrentFront->NextNode = NULL;
				}

				NewNode = new CBspNode;
				NewNode->AssignPolygon(&NewPolygon);
				NewNode->mBrushId = CurrentListPolygon->mBrushId;
				NewNode->mPolygonNumber = CurrentListPolygon->mPolygonNumber;
				NewNode->mBrushPtr = CurrentListPolygon->mBrushPtr;

				// The back half is the most recent polygon added
				if(*BackPolygonList == NULL) {
					*BackPolygonList = NewNode;
					(*BackPolygonList)->NextNode = NULL;
					CurrentBack = *BackPolygonList;
				}
				else {
					CurrentBack->NextNode = NewNode;
					CurrentBack = CurrentBack->NextNode;
					CurrentBack->NextNode = NULL;
				}
				break;
			}

			// Assign the current polygon to the next one
			CurrentListPolygon = NextListPolygon;
		}
		// Assign the current polygon to the next one
		CurrentStartListPolygon = NextStartListPolygon;
	}

	// once we have calculated the sub-list we need to create
	// the bounding sphere for the front and back nodes of the list
//	bspGenerateBoundingSphere(Node, *FrontPolygonList, *BackPolygonList);
}

void CConstructiveBsp::DoSubTree(CBspNode *Node, CBspNode *FrontPolygonList,
CBspNode *BackPolygonList, CProgressCtrl* Progress)
{
	CBspNode *NewFrontList, *NewBackList;
	CBspNode *BestNode;

	NewFrontList = NewBackList = NULL;

	// Do front child if we have anything
	if(FrontPolygonList != NULL) {

		// Get the best split node from the list
		BestNode = GetBestNode(FrontPolygonList);

		Node->FrontChild = BestNode;

		CalculateSubLevel(BestNode, FrontPolygonList, &NewFrontList,
		&NewBackList, Progress);

		DoSubTree(BestNode, NewFrontList, NewBackList, Progress);
	}

	NewFrontList = NewBackList = NULL;

	// Do back child if we have anything
	if(BackPolygonList != NULL) {

		// Get the best split node from the list
		BestNode = GetBestNode(BackPolygonList);

		Node->BackChild = BestNode;

		CalculateSubLevel(BestNode, BackPolygonList, &NewFrontList,
		&NewBackList, Progress);

		DoSubTree(BestNode, NewFrontList, NewBackList, Progress);
	}
}

void	CConstructiveBsp::DoCSGFilterKeepOutsideNoCoplanars(CBspNode* Polygon)
{
	int FrontCount=0, BackCount=0;

	// do our sub-filter
	DoCSGFilterKeepOutsideNoCoplanars(TreeTop, Polygon, FrontCount, BackCount);
}

void	CConstructiveBsp::DoCSGFilterKeepOutsideCoplanars(CBspNode* Polygon)
{
	int FrontCount=0, BackCount=0;

	// do our sub-filter
	DoCSGFilterKeepOutsideCoplanars(TreeTop, Polygon, FrontCount, BackCount);
}

void	CConstructiveBsp::DoCSGFilterKeepOutside(CBspNode* Polygon)
{
	int FrontCount=0, BackCount=0;

	// do our sub-filter
	DoCSGFilterKeepOutside(TreeTop, Polygon, FrontCount, BackCount);
}

void	CConstructiveBsp::DoCSGFilterKeepInside(CBspNode* Polygon)
{
	int FrontCount=0, BackCount=0;

	// do our sub-filter
	DoCSGFilterKeepInside(TreeTop, Polygon, FrontCount, BackCount);
}

void	CConstructiveBsp::DoCSGFilterKeepInsideNoCoplanars(CBspNode* Polygon)
{
	int FrontCount=0, BackCount=0;

	// do our sub-filter
	DoCSGFilterKeepInsideNoCoplanars(TreeTop, Polygon, FrontCount, BackCount);
}

void	CConstructiveBsp::DoCSGFilterKeepInsideCoplanars(CBspNode* Polygon)
{
	int FrontCount=0, BackCount=0;

	// do our sub-filter
	DoCSGFilterKeepInsideCoplanars(TreeTop, Polygon, FrontCount, BackCount);
}

void	CConstructiveBsp::DoCSGFilterKeepInside(CBspNode* Node, 
	CBspNode* Polygon, int& NumberFront, int& NumberBack)
{

	// We will use whether the node is drawn
	// as the check to whether or not
	// we have had a backside polygon.
	if(!Node)
		return;

	// check where to go
	switch(Node->ClassifyPolygon(&Polygon->Polygon))
    {
	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
	case POLYGON_IN_FRONT:
	case POLYGON_COPLANAR_SAME:
		if(Node->FrontChild)
        {
			// do our sub-filter
			DoCSGFilterKeepInside(Node->FrontChild, Polygon, NumberFront, NumberBack);
		} else {
			// STUFF COMING TO THE FRONT SHOULD BE INVALIDATED
			// BUT ONLY IF IT'S ALREADY INVALID... if it's
			// VALID then it should stay alive
			NumberFront++;
		}
		break;
	// do the same as the front except for the back
	case POLYGON_IN_BACK:
	case POLYGON_COPLANAR_REVERSE:
		if(Node->BackChild)
        {
			// do our sub-filter
			DoCSGFilterKeepInside(Node->BackChild, Polygon, NumberFront, NumberBack);
		} else {
			Polygon->mValidated = TRUE;
			// do nothing here.
			NumberBack++;
		}
		break;
	// If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
			ThredPolygon UsagePolygon, SavePolygon;
			CBspNode* NewNode;
			int FrontCount=0, BackCount=0;

			// save our original gon
			SavePolygon = Polygon->Polygon;

			// split the gon
			Node->SplitPolygon(&Polygon->Polygon, &UsagePolygon);

			// make a new node and filter it through the tree
			NewNode = new CBspNode;
			NewNode->AssignPolygon(&UsagePolygon);
			NewNode->mValidated = Polygon->mValidated;
			NewNode->mBrushId = Polygon->mBrushId;
			NewNode->mPolygonNumber = Polygon->mPolygonNumber;
            NewNode->mBrushPtr = Polygon->mBrushPtr;  //????

            //FrontCount = BackCount = 0;
			// check for something in front
			if(Node->FrontChild)
            {
				// do our sub-filter
				DoCSGFilterKeepInside(Node->FrontChild, Polygon, FrontCount, BackCount);
			} else {
				FrontCount++;
			}

			// check for something in back
			if(Node->BackChild)
            {
				// do our sub-filter
				DoCSGFilterKeepInside(Node->BackChild, NewNode, FrontCount, BackCount);
			} else {
				// do nothing here.
				NewNode->mValidated = TRUE;
				BackCount++;
			}

			// Now we want to check whether or not to UNSPLIT
			// the polygon that we just split.  This would
			// happen if all of the nodes ended up on the front.
			// if we UNSPLIT then we return that all of the nodes ended up onthe front
			if(FrontCount == 0)
            {
				// delete the new node that we made
				delete NewNode;
				// tell them this one went on the front
				NumberFront++;
				// assign our old polygon.
				Polygon->AssignPolygon(&SavePolygon);
			} else {
				// add our newnode to the list of coplanar children.
				while(Polygon->CoplanarChild)
					Polygon = Polygon->CoplanarChild;
				Polygon->CoplanarChild = NewNode;

				// increment our counts
				NumberFront += FrontCount;
				NumberBack  += BackCount;
			}
		}
		break;
	}
}

void CConstructiveBsp::DoCSGFilterKeepInsideNoCoplanars(CBspNode* Node, 
	CBspNode* Polygon, int& NumberFront, int& NumberBack)
{
    int     l;
//    double  dotprod;

	// We will use whether the node is drawn
	// as the check to whether or not
	// we have had a backside polygon.
	if(!Node)
		return;

	// check where to go
	switch(Node->ClassifyPolygon(&Polygon->Polygon))
    {
	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
	case POLYGON_IN_FRONT:
	//case POLYGON_COPLANAR_SAME:
		if(Node->FrontChild)
        {
			// do our sub-filter
			DoCSGFilterKeepInsideNoCoplanars(Node->FrontChild, Polygon, NumberFront, NumberBack);
		} else {
			// STUFF COMING TO THE FRONT SHOULD BE INVALIDATED
			// BUT ONLY IF IT'S ALREADY INVALID... if it's
			// VALID then it should stay alive
			//Polygon->mValidated = FALSE;
			NumberFront++;
		}
		break;
	// do the same as the front except for the back
	case POLYGON_IN_BACK:
	//case POLYGON_COPLANAR_REVERSE:
		if(Node->BackChild)
        {
			// do our sub-filter
			DoCSGFilterKeepInsideNoCoplanars(Node->BackChild, Polygon, NumberFront, NumberBack);
		} else {
			Polygon->mValidated = TRUE;
			// do nothing here.
			NumberBack++;
		}
		break;
	case POLYGON_COPLANAR_REVERSE:
	case POLYGON_COPLANAR_SAME:
      {
          Point3D normal, nodenormal;

          // Perturb Polygon's position by some amount along its -normal
          // (This should cause splits to occur along our polygon)
          normal.x = Polygon->ConstantA;
          normal.y = Polygon->ConstantB;
          normal.z = Polygon->ConstantC;
          normal.normalize();

          nodenormal.x = Node->ConstantA;
          nodenormal.y = Node->ConstantB;
          nodenormal.z = Node->ConstantC;
          nodenormal.normalize();

          //dotprod = m_dot(nodenormal, normal);

          //if (dotprod < 1.0)
          //{
            // Perturb that poly!
            for (l = 0; l < Polygon->Polygon.NumberOfPoints; l++)
            {
              Polygon->Polygon.Points[l].X -= normal.x;
              Polygon->Polygon.Points[l].Y -= normal.y;
              Polygon->Polygon.Points[l].Z -= normal.z;
            }

            Polygon->perturbed = 1;

	        // do our sub-filter
		    DoCSGFilterKeepInsideNoCoplanars(TreeTop, Polygon, NumberFront, NumberBack);

            // Loop through any coplanars that might have been created by the split
            // and unperturb those polygons
            CBspNode* currnode = Polygon;

            while (currnode)
            {
              if (currnode->perturbed)
              {
                for (l = 0; l < currnode->Polygon.NumberOfPoints; l++)
                {
                  currnode->Polygon.Points[l].X += normal.x;
                  currnode->Polygon.Points[l].Y += normal.y;
                  currnode->Polygon.Points[l].Z += normal.z;
                }

                currnode->perturbed = 0;
              }

              currnode = currnode->CoplanarChild;
            }
          //} else {
		//	DoCSGFilterKeepInsideNoCoplanars(Node->BackChild, Polygon, NumberFront, NumberBack);
          //}
      }
      break;
	// If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
			ThredPolygon UsagePolygon, SavePolygon;
			CBspNode* NewNode;
			int FrontCount=0, BackCount=0;

			// save our original gon
			SavePolygon = Polygon->Polygon;

			// split the gon
			Node->SplitPolygon(&Polygon->Polygon, &UsagePolygon);

			// make a new node and filter it through the tree
			NewNode = new CBspNode;
			NewNode->AssignPolygon(&UsagePolygon);
			NewNode->mValidated = Polygon->mValidated;
			NewNode->mBrushId = Polygon->mBrushId;
			NewNode->mPolygonNumber = Polygon->mPolygonNumber;
            NewNode->mBrushPtr = Polygon->mBrushPtr;
            NewNode->perturbed = Polygon->perturbed;

            //FrontCount = BackCount = 0;
			// check for something in front
			if(Node->FrontChild)
            {
				// do our sub-filter
				DoCSGFilterKeepInsideNoCoplanars(Node->FrontChild, Polygon, FrontCount, BackCount);
			} else {
				//NewNode->mValidated = FALSE;
				FrontCount++;
			}

			// check for something in back
			if(Node->BackChild)
            {
				// do our sub-filter
				DoCSGFilterKeepInsideNoCoplanars(Node->BackChild, NewNode, FrontCount, BackCount);
			} else {
				// do nothing here.
				NewNode->mValidated = TRUE;
				BackCount++;
			}

			// Now we want to check whether or not to UNSPLIT
			// the polygon that we just split.  This would
			// happen if all of the nodes ended up on the front.
			// if we UNSPLIT then we return that all of the nodes ended up onthe front
			if(FrontCount == 0)
            {
				// delete the new node that we made
				delete NewNode;
				// tell them this one went on the front
				NumberFront++;
				// assign our old polygon.
				Polygon->AssignPolygon(&SavePolygon);
			} else {
				// add our newnode to the list of coplanar children.
				while(Polygon->CoplanarChild)
					Polygon = Polygon->CoplanarChild;
				Polygon->CoplanarChild = NewNode;
                //Polygon->CoplanarChild->perturbed = Polygon->perturbed;

				// increment our counts
				NumberFront += FrontCount;
				NumberBack  += BackCount;
			}
		}
		break;
	}
}

void CConstructiveBsp::DoCSGFilterKeepInsideCoplanars(CBspNode* Node, 
	CBspNode* Polygon, int& NumberFront, int& NumberBack)
{
    int     l;
    double  dotprod;

	// We will use whether the node is drawn
	// as the check to whether or not
	// we have had a backside polygon.
	if(!Node)
		return;

	// check where to go
	switch(Node->ClassifyPolygon(&Polygon->Polygon))
    {
	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
	case POLYGON_IN_FRONT:
	//case POLYGON_COPLANAR_SAME:
		if(Node->FrontChild)
        {
			// do our sub-filter
			DoCSGFilterKeepInsideCoplanars(Node->FrontChild, Polygon, NumberFront, NumberBack);
		} else {
			// STUFF COMING TO THE FRONT SHOULD BE INVALIDATED
			// BUT ONLY IF IT'S ALREADY INVALID... if it's
			// VALID then it should stay alive
			//Polygon->mValidated = FALSE;
			NumberFront++;
		}
		break;
	// do the same as the front except for the back
	case POLYGON_IN_BACK:
	//case POLYGON_COPLANAR_REVERSE:
		if(Node->BackChild)
        {
			// do our sub-filter
			DoCSGFilterKeepInsideCoplanars(Node->BackChild, Polygon, NumberFront, NumberBack);
		} else {
			Polygon->mValidated = TRUE;
			// do nothing here.
			NumberBack++;
		}
		break;

	case POLYGON_COPLANAR_REVERSE:
	case POLYGON_COPLANAR_SAME:
      {
          Point3D normal, nodenormal;

          // Perturb Polygon's position by some amount along its -normal
          // (This should cause splits to occur along our polygon)
          normal.x = Polygon->ConstantA;
          normal.y = Polygon->ConstantB;
          normal.z = Polygon->ConstantC;
          normal.normalize();

          nodenormal.x = Node->ConstantA;
          nodenormal.y = Node->ConstantB;
          nodenormal.z = Node->ConstantC;
          nodenormal.normalize();

          dotprod = md_dot(nodenormal, normal);

          if (dotprod < 1.0)
          {
            // Perturb that poly!
            for (l = 0; l < Polygon->Polygon.NumberOfPoints; l++)
            {
              Polygon->Polygon.Points[l].X += normal.x;
              Polygon->Polygon.Points[l].Y += normal.y;
              Polygon->Polygon.Points[l].Z += normal.z;
            }

            Polygon->perturbed = 1;

	        // do our sub-filter
		    DoCSGFilterKeepInsideCoplanars(TreeTop, Polygon, NumberFront, NumberBack);

            // Loop through any coplanars that might have been created by the split
            // and unperturb those polygons
            CBspNode* currnode = Polygon;

            while (currnode)
            {
              if (currnode->perturbed)
              {
                for (l = 0; l < currnode->Polygon.NumberOfPoints; l++)
                {
                  currnode->Polygon.Points[l].X -= normal.x;
                  currnode->Polygon.Points[l].Y -= normal.y;
                  currnode->Polygon.Points[l].Z -= normal.z;
                }

                currnode->perturbed = 0;
              }

              currnode = currnode->CoplanarChild;
            }
          } else {
			DoCSGFilterKeepInsideCoplanars(Node->BackChild, Polygon, NumberFront, NumberBack);
          }
      }
      break;

	// If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
			ThredPolygon UsagePolygon, SavePolygon;
			CBspNode* NewNode;
			int FrontCount=0, BackCount=0;

			// save our original gon
			SavePolygon = Polygon->Polygon;

			// split the gon
			Node->SplitPolygon(&Polygon->Polygon, &UsagePolygon);

			// make a new node and filter it through the tree
			NewNode = new CBspNode;
			NewNode->AssignPolygon(&UsagePolygon);
			NewNode->mValidated = Polygon->mValidated;
			NewNode->mBrushId = Polygon->mBrushId;
			NewNode->mPolygonNumber = Polygon->mPolygonNumber;
            NewNode->mBrushPtr = Polygon->mBrushPtr;
            NewNode->perturbed = Polygon->perturbed;

            //FrontCount = BackCount = 0;
			// check for something in front
			if(Node->FrontChild)
            {
				// do our sub-filter
				DoCSGFilterKeepInsideNoCoplanars(Node->FrontChild, Polygon, FrontCount, BackCount);
			} else {
				//NewNode->mValidated = FALSE;
				FrontCount++;
			}

			// check for something in back
			if(Node->BackChild)
            {
				// do our sub-filter
				DoCSGFilterKeepInsideNoCoplanars(Node->BackChild, NewNode, FrontCount, BackCount);
			} else {
				// do nothing here.
				NewNode->mValidated = TRUE;
				BackCount++;
			}

			// Now we want to check whether or not to UNSPLIT
			// the polygon that we just split.  This would
			// happen if all of the nodes ended up on the front.
			// if we UNSPLIT then we return that all of the nodes ended up onthe front
			if(FrontCount == 0)
            {
				// delete the new node that we made
				delete NewNode;
				// tell them this one went on the front
				NumberFront++;
				// assign our old polygon.
				Polygon->AssignPolygon(&SavePolygon);
			} 
			else {
				// add our newnode to the list of coplanar children.
				while(Polygon->CoplanarChild)
					Polygon = Polygon->CoplanarChild;
				Polygon->CoplanarChild = NewNode;
                //Polygon->CoplanarChild->perturbed = Polygon->perturbed;

				// increment our counts
				NumberFront += FrontCount;
				NumberBack  += BackCount;
			}
		}
		break;
	}
}

int CConstructiveBsp::InitializeQuickBspNodes(int NumberOfNodes)
{
	// we want to allocate a certain number of nodes
	if(mNumberOfQuickNodes < NumberOfNodes*2)
    {
		mQuickBspNodes = new CBspNode[NumberOfNodes*2];
		mNumberOfQuickNodes = NumberOfNodes*2;
	}
	// set the beginning to 0 and memset ourselves
	mCurrentQuickNode = 0;
	memset(mQuickBspNodes, 0, sizeof(CBspNode) * NumberOfNodes*2);
	return 1;
}

//===============================================================
// This function will add a polygon to the into the tree.
// These polygons are just added in the order that they are 
// received so that there is no optimization of this tree.
//===============================================================
int CConstructiveBsp::AddQuickPolygonToTree(ThredPolygon* Polygon, CBspNode* CheckPoly, CThredBrush* parentbrush)
{

	// we have to validate whether the Polygon crosses the plane
	// of checkpoly
	int ClassifyValue = CheckPoly->ClassifyPolygon(Polygon);
	if((ClassifyValue != POLYGON_SPLIT) && (ClassifyValue != POLYGON_COPLANAR_SAME) && (ClassifyValue != POLYGON_COPLANAR_REVERSE))
		return 0;

	// if we have the special case of an empty tree.
	if(!TreeTop) {
		// add our polygon into our list
		CBspNode* Node = GetNewQuickNode();
		Node->AssignPolygon(Polygon);
        Node->mBrushPtr = parentbrush;
		TreeTop = Node;
		return 1;
	}

	// otherwise we want to filter the polygon down the tree
	// until it is added in
	QuickFilterPoly(TreeTop, Polygon, parentbrush);

	return 1;
}

//===============================================================
// This is a recursize function which is called with a bsp node
// and with a polygon.  It trys to stick the polygon somewhere
// in the tree.
//===============================================================
void CConstructiveBsp::QuickFilterPoly(CBspNode* Node, ThredPolygon* Polygon, CThredBrush* parentbrush)
{
	CBspNode* NewNode;

	// if node is busted leave
	if(!Node)
		return;

	// check where to go
	switch(Node->ClassifyPolygon(Polygon)) {

	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
	case POLYGON_IN_FRONT:
	case POLYGON_COPLANAR_SAME:
		if(Node->FrontChild) {
			QuickFilterPoly(Node->FrontChild, Polygon, parentbrush);
		} else {
			NewNode = GetNewQuickNode();
			NewNode->AssignPolygon(Polygon);
            NewNode->mBrushPtr = parentbrush;
			Node->FrontChild = NewNode;
		}
		break;
	// do the same as the front except for the back
	case POLYGON_IN_BACK:
	case POLYGON_COPLANAR_REVERSE:
		if(Node->BackChild) {
			QuickFilterPoly(Node->BackChild, Polygon, parentbrush);
		} else {
			NewNode = GetNewQuickNode();
			NewNode->AssignPolygon(Polygon);
            NewNode->mBrushPtr = parentbrush;
			Node->BackChild = NewNode;
		}
		break;
	// If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
			ThredPolygon NewPolygon;

			// split the gon
			Node->SplitPolygon(Polygon, &NewPolygon);

			// check for something in front
			if(Node->FrontChild) {
				QuickFilterPoly(Node->FrontChild, Polygon, parentbrush);
			} else {
				NewNode = GetNewQuickNode();
				NewNode->AssignPolygon(Polygon);
                NewNode->mBrushPtr = parentbrush;
				Node->FrontChild = NewNode;
			}

			// check for something in back
			if(Node->BackChild) {
				QuickFilterPoly(Node->BackChild, &NewPolygon, parentbrush);
			} else {
				NewNode = GetNewQuickNode();
				NewNode->AssignPolygon(&NewPolygon);
                NewNode->mBrushPtr = parentbrush;
				Node->BackChild = NewNode;
			}
		}
		break;
	}
}

// Get a new node from the list of quick nodes
CBspNode* CConstructiveBsp::GetNewQuickNode()
{
	mCurrentQuickNode++;
	_ASSERTE(mCurrentQuickNode <= mNumberOfQuickNodes);
	return &mQuickBspNodes[mCurrentQuickNode-1];
}

void CConstructiveBsp::DeleteBrushIDNodes(CBspNode* Node, int BrushId)
{
    CBspNode*   tempnode;

	// we are at a terminating leaf.
	if(!Node)
		return;

    tempnode = Node;

    // Test for equivalent brush ids
    while(tempnode)
    {
      if (tempnode->mBrushId == BrushId)
        tempnode->mValidated = 0;

      // Invalidate that node...
      tempnode = tempnode->CoplanarChild;
    }

	DeleteBrushIDNodes(Node->BackChild, BrushId);

    DeleteBrushIDNodes(Node->FrontChild, BrushId);
}

void CConstructiveBsp::ProcessBrush(CThredBrush* Brush, int BrushId, bool do_csg_and_tree)
{
   int CurrentGon;
   CBspNode AddPolygon;
   ThredPolygon TempPolygon;

   switch(Brush->mTransactionType) 
   {
      case BRUSH_ADD_TO_WORLD:

         // transform all of the verts of all of the gons
         // so that they are ok.
         Brush->TransformBrushPolygons();	

         // reset all of the new nodes in the tree
         ResetNewNodes();

         // go through the list of gons in the brush
         // and send them into the bsp
         for(CurrentGon = 0; CurrentGon < Brush->mNumberOfPolygons; CurrentGon++)
         {
            AddPolygon.AssignPolygon(&Brush->mTransformedPolygons[CurrentGon]);
            AddPolygon.mBrushId = BrushId;
            AddPolygon.mPolygonNumber = CurrentGon;
            AddPolygon.mBrushPtr = Brush;
            AddPolygonToTree(&AddPolygon);
         }

         // make our brush bsp
         Brush->CreateBrushBsp();

         // filter our world through our brush
         FilterBrushBsp(Brush->mBrushBsp);

         // Normalize all your polygons...
         NormalizeTreePolyTextures(TreeTop);

         if (do_csg_and_tree)
         {
            ProcessTreeCacheInfo(TreeTop, TRUE);
         }

//         AssignGenericLightMaps(TreeTop, theApp.lightval);
         break;

      case BRUSH_SUBTRACT_FROM_WORLD:

         // transform all of the verts of all of the gons
         // so that they are ok.
         Brush->TransformBrushPolygons();	

         // reset all of the new nodes in the tree
         ResetNewNodes();
      
         // go through the list of gons in the brush
         // and send them into the bsp
         for(CurrentGon = 0; CurrentGon < Brush->mNumberOfPolygons; CurrentGon++)
         {
            TempPolygon = Brush->mTransformedPolygons[CurrentGon];

            // reverse the order of the verts
            int CurrentVert=0, MaxVert= TempPolygon.NumberOfPoints-1;
            for(; MaxVert >= 0; MaxVert--, CurrentVert++)
               TempPolygon.Points[CurrentVert] = Brush->mTransformedPolygons[CurrentGon].Points[MaxVert];

            // assign 'er
            AddPolygon.AssignPolygon(&TempPolygon);
            AddPolygon.mBrushId = BrushId;
            AddPolygon.mPolygonNumber = CurrentGon;
            AddPolygon.mBrushPtr = Brush;

            // add a subtraction polygon
            SubtractPolygonFromTree(&AddPolygon);
         }

         // make our brush bsp
         Brush->CreateBrushBsp();

         // filter our world through our brush
         FilterBrushBsp(Brush->mBrushBsp);

         // Normalize all your polygons...
         NormalizeTreePolyTextures(TreeTop);

         if (do_csg_and_tree)
         {
            ProcessTreeCacheInfo(TreeTop, TRUE);
         }

//         AssignGenericLightMaps(TreeTop, theApp.lightval);

      break;
   }
}

#define CCONSTRUCTIVEBSPSTART "CConstructiveBsp1"
#define CCONSTRUCTIVEBSPEND "End CConstructiveBsp"
void CConstructiveBsp::Serialize(CThredParser& Parser)
{
	//CThredParser Parser(&ar, CCONSTRUCTIVEBSPEND);
	Parser.SetEnd(CCONSTRUCTIVEBSPEND);

	if(Parser.IsStoring) {   // save code
		Parser.WriteString(CCONSTRUCTIVEBSPSTART);
		SerializeTree(Parser, TreeTop);
		Parser.WriteString(CCONSTRUCTIVEBSPEND);
	}
	else {					// load code

		// erase what we have
		ClearBsp();

		if(!Parser.GetKey(CCONSTRUCTIVEBSPSTART))
			return;
		// make a new top node
		CBspNode* TopNode = new CBspNode;

		// load up the tree
		if(SerializeTree(Parser, TopNode))
			TreeTop = TopNode;
		else
			delete TopNode;
// DPW - 10/23		Parser.GetKey(CCONSTRUCTIVEBSPEND);
		Parser.GotoEnd();

	}
}

//===========================================================
// This is a recursize function to delete the bsp.
//===========================================================
#define BSPCOPOLYCOUNT "CoCount"
#define BSPNODEVALIDATED "nValid"
#define BSPNODEBRUSHID	"nBid"
#define BSPNODEPOLYGONNUMBER "nPolNum"
#define BSPNODEHASBACK "nB"
#define BSPNODEHASFRONT "nF"

int CConstructiveBsp::SerializeTree(CThredParser& Parser, CBspNode* Node)
{
	//CThredParser Parser(&ar, NULL);

	Parser.SetEnd(NULL);

	// storing code
	if(Parser.IsStoring) {

		// get a count of how many co-planar gon's we have
		int PolygonCount = 0;
		CBspNode* CoNode = Node;
		while(CoNode) {
			PolygonCount++;
			CoNode = CoNode->CoplanarChild;
		}
		// write out he co count
		Parser.WriteInt(BSPCOPOLYCOUNT, PolygonCount);

		// we wrote our polycount
		// if it was zero get outta here
		if(!PolygonCount)
			return 0;

		// now write out our polygons
		CoNode = Node;
		while(PolygonCount--) {
			// write out the polygon we have here
			Parser.WritePolygon(CoNode->Polygon);

			// write out the validated status we have here
			Parser.WriteInt(BSPNODEVALIDATED, CoNode->mValidated);
			// write out the current brush ID number we have here
			Parser.WriteInt(BSPNODEBRUSHID, CoNode->mBrushId);
			Parser.WriteInt(BSPNODEPOLYGONNUMBER, CoNode->mPolygonNumber);

			CoNode = CoNode->CoplanarChild;
		}

		// now notify if we have a subtree
		// and write it out if we do
		if(Node->BackChild) {
			Parser.WriteInt(BSPNODEHASBACK, 1);
			SerializeTree(Parser, Node->BackChild);
		} 
		else  Parser.WriteInt(BSPNODEHASBACK, 0);

		// check for the front subtree
		if(Node->FrontChild) {
			Parser.WriteInt(BSPNODEHASFRONT, 1);
			SerializeTree(Parser, Node->FrontChild);
		}
		else Parser.WriteInt(BSPNODEHASFRONT, 0);

		return 1;
	}
	else {
		// read in a tree
		int PolygonCount=0;
		ThredPolygon InPoly;
		CBspNode* NewNode;
		CBspNode* ParentNode;

		// get cocount
		Parser.GetKeyInt(BSPCOPOLYCOUNT, PolygonCount);

		// if we have nothing then there is a problem
		if(!PolygonCount)
			return 0;

		// we want to read in the first polygon and
		// assign it to this node
		Parser.ReadPolygon(InPoly);
		Node->AssignPolygon(&InPoly);
		Parser.GetKeyInt(BSPNODEVALIDATED, Node->mValidated);
		Parser.GetKeyInt(BSPNODEBRUSHID, Node->mBrushId);

        // Get a pointer to the correct brush...
    //    Node->mBrushPtr = pDoc->GetBrushList()->GetBrushByID(Node->mBrushId);

		Parser.GetKeyInt(BSPNODEPOLYGONNUMBER, Node->mPolygonNumber);

		PolygonCount--;

		// we have more polys to read in
		ParentNode = Node;
		while(PolygonCount--) {
			NewNode = new CBspNode;
			Parser.ReadPolygon(InPoly);
			NewNode->AssignPolygon(&InPoly);
			Parser.GetKeyInt(BSPNODEVALIDATED, NewNode->mValidated);
			Parser.GetKeyInt(BSPNODEBRUSHID, NewNode->mBrushId);
			Parser.GetKeyInt(BSPNODEPOLYGONNUMBER, NewNode->mPolygonNumber);
			ParentNode->CoplanarChild = NewNode;
			ParentNode = NewNode;
		}

		int HasChild;

		// check for a back child tree
		Parser.GetKeyInt(BSPNODEHASBACK, HasChild);
		if(HasChild) {
			NewNode = new CBspNode;
			Node->BackChild = NewNode;
			SerializeTree(Parser, NewNode);
		}

		// check for a back child tree
		Parser.GetKeyInt(BSPNODEHASFRONT, HasChild);
		if(HasChild) {
			NewNode = new CBspNode;
			Node->FrontChild = NewNode;
			SerializeTree(Parser, NewNode);
		}
		return 1;
	}
	return 0;
}





void CConstructiveBsp::ConvexDoSubTree(CBspNode *Node, CBspNode *FrontPolygonList,
CBspNode *BackPolygonList)
{
	CBspNode *NewFrontList, *NewBackList;
	CBspNode *BestNode;

	NewFrontList = NewBackList = NULL;

	// Do front child if we have anything
	if(FrontPolygonList != NULL) {

		// Get the best split node from the list
		BestNode = ConvexGetBestNode(FrontPolygonList);
		if(BestNode == NULL) {
			Node->FrontChild = FrontPolygonList;
			FrontPolygonList->Splitter = 1;
		}
		else {
			Node->FrontChild = BestNode;

			ConvexCalculateSubLevel(BestNode, FrontPolygonList, &NewFrontList,
			&NewBackList);

			ConvexDoSubTree(BestNode, NewFrontList, NewBackList);
		}
	}

	NewFrontList = NewBackList = NULL;

	// Do back child if we have anything
	if(BackPolygonList != NULL) {

		// Get the best split node from the list
		BestNode = ConvexGetBestNode(BackPolygonList);

		if(BestNode == NULL) {
			Node->BackChild = BackPolygonList;
			BackPolygonList->Splitter = 1;
		}
		else {
			Node->BackChild = BestNode;

			ConvexCalculateSubLevel(BestNode, BackPolygonList, &NewFrontList,
			&NewBackList);

			ConvexDoSubTree(BestNode, NewFrontList, NewBackList);
		}
	}
}


int CConstructiveBsp::IsAllSplits(CBspNode *Node)
{
	CBspNode* Start = Node;
	while(Node) {
		if(Node->Splitter == 0)
			return 0;
		Node = Node->NextNode;
	}

	// we are all splitters so free us and leave
	Node = Start;
	while(Node) {
		Start = Node->NextNode;
		delete Node;
		Node = Start;
	}
	return 1;
}

// This function will go through the list of polygons and attempt to find
// an optimal partition plane.  An optimal plane consists of the best
// balance versus the best number of splits, based on bspBalancePercent
// Returns -1 if we have all partition planes in the list, in which
// case we can create a render node (this occurs because of some crepping
// precision errors!!!!  HELL!)
CBspNode* CConstructiveBsp::ConvexGetBestNode(CBspNode *PolyList)
{
	CBspNode *CurrentPoly;
	CBspNode *BestPoly = NULL;
	int BestScore = 5555555, CurrentScore;
	int FrontCount, BackCount, Splits;
	int BalancePercent = 0;

	// Go through all the polygons in the list
	for(CurrentPoly = PolyList;CurrentPoly != NULL; CurrentPoly = CurrentPoly->NextNode) {

		// Check how many splits we have
		CountSplits(CurrentPoly, PolyList, FrontCount,	BackCount, Splits);

		// if nothing in front and no splits
		// then this sucker is part of a convex
		// sub brush node so we can't
		// use this as a splitter.
		if(FrontCount == 0 && Splits == 0)
			continue;

		// Get the current score
		CurrentScore = (BalancePercent * (abs(FrontCount - BackCount)))
						 + ((100 - BalancePercent) * Splits);

		// Check our score
		if(CurrentScore < BestScore) {
			BestPoly = CurrentPoly;
			BestScore = CurrentScore;
		}
	}

	return BestPoly;
}

// This list has to be modified so that co-planar polygons are added
// to the list as co-planar gons.
void CConstructiveBsp::BuildPolygonListFromTree(CBspNode* List, CBspNode* Node)
{

	CBspNode* Coplanar;
	CBspNode* DelNode;

	// got to stop on sentinel.
	if(!Node)
		return;

	// depth first so we can delete the tree
	BuildPolygonListFromTree(List, Node->BackChild);
	BuildPolygonListFromTree(List, Node->FrontChild);


	// get the list to a point where we can us it
	while(List->NextNode)
		List = List->NextNode;

	// Do the FIRST node
	// we loop through until we have a validated then use it
	Coplanar = Node;
	while(Coplanar) {
		if(Coplanar->mValidated) {
			List->NextNode = new CBspNode;
			List = List->NextNode;
			*List = *Node;
			List->NextNode = List->CoplanarChild = List->FrontChild = List->BackChild = NULL;

			// go the next polygon and be done our first node
			DelNode = Coplanar;
			Coplanar = Coplanar->CoplanarChild;
			delete DelNode;
			break;

		}
		else {
			// go the next polygon
			DelNode = Coplanar;
			Coplanar = Coplanar->CoplanarChild;
			delete DelNode;
		}
	}


	// go through the rest and add them as coplanar gons
	while(Coplanar) {
		if(Coplanar->mValidated) {
			// get a new node
			List->CoplanarChild = new CBspNode;
			List = List->CoplanarChild;

			// copy over the stuff we need
			*List = *Coplanar;
			List->NextNode = List->CoplanarChild = List->FrontChild = List->BackChild = NULL;
		}
		// go the next polygon
		DelNode = Coplanar;
		Coplanar = Coplanar->CoplanarChild;
		delete DelNode;
	}

}


//=============================================================================
// This function has to be somewhat different from the other function.  This
// function has to process co-planar codes properly, making NEW co-planar node.
// This should be veeeery interesting.
//=============================================================================
void CConstructiveBsp::ConvexCalculateSubLevel(CBspNode *Node, CBspNode *MainPolygonList,
CBspNode **FrontPolygonList, CBspNode **BackPolygonList, CProgressCtrl* Progess)
{
	CBspNode *CurrentStartListPolygon, *NextListPolygon, *CurrentListPolygon, *NextStartListPolygon;
	CBspNode *CurrentFront, *CurrentBack, *NewNode;
	CBspNode *CoplanarPolygonList = NULL;


	if(Progess)
		Progess->StepIt();

	// Loop through the main polygon list and generate front and back
	// polygon lists, splitting if necessary

	CurrentStartListPolygon = MainPolygonList;

	while(CurrentStartListPolygon != NULL) {

		// Store the next polygon pointer
		NextStartListPolygon = CurrentStartListPolygon->NextNode;

		CurrentListPolygon = CurrentStartListPolygon;
		while(CurrentListPolygon != NULL) {
			// Store the next polygon pointer
			NextListPolygon = CurrentListPolygon->CoplanarChild;

			// if this is set our list will be SO freaking screwed it's
			// not even FUNNY.
			CurrentListPolygon->CoplanarChild = NULL;

			// Make sure we don't compare against the parent polygon
			if(CurrentListPolygon == Node) {
				// we have to set this to null so that
				// we won't be doing weird fucked up rendering
				CurrentListPolygon = NextListPolygon;
				continue;
			}


			ThredPolygon NewPolygon;
			switch(Node->ClassifyPolygon(&CurrentListPolygon->Polygon)) {
			case POLYGON_COPLANAR_SAME:
				// this sucker is COplanar which means we add it to the NODE
				// as a new coplanar gon
				// we basically make a list of these and then add them to the node
				// at the end of it all.
				if(CoplanarPolygonList) {
					CoplanarPolygonList->CoplanarChild = CurrentListPolygon;
					CoplanarPolygonList = CoplanarPolygonList->CoplanarChild;
				}
				else {
					CoplanarPolygonList = CurrentListPolygon;
				}
				break;
			case POLYGON_IN_FRONT:
				// Add our currentlist poly to the front list of polygons
				if(*FrontPolygonList == NULL) {
					*FrontPolygonList = CurrentListPolygon;
					(*FrontPolygonList)->NextNode = NULL;
					CurrentFront = *FrontPolygonList;
				}
				else {
					CurrentFront->NextNode = CurrentListPolygon;
					CurrentFront = CurrentFront->NextNode;
					CurrentFront->NextNode = NULL;
				}
				break;
			case POLYGON_COPLANAR_REVERSE:
			case POLYGON_IN_BACK:
				if(*BackPolygonList == NULL) {
					*BackPolygonList = CurrentListPolygon;
					(*BackPolygonList)->NextNode = NULL;
					CurrentBack = *BackPolygonList;
				}
				else {
					CurrentBack->NextNode = CurrentListPolygon;
					CurrentBack = CurrentBack->NextNode;
					CurrentBack->NextNode = NULL;
				}
				break;
			case POLYGON_SPLIT:
				// split the polygons
				Node->SplitPolygon(&CurrentListPolygon->Polygon, &NewPolygon);

				// The front half is stored in place of the original polygon
				if(*FrontPolygonList == NULL) {
					*FrontPolygonList = CurrentListPolygon;
					(*FrontPolygonList)->NextNode = NULL;
					CurrentFront = *FrontPolygonList;
				}
				else {
					CurrentFront->NextNode = CurrentListPolygon;
					CurrentFront = CurrentFront->NextNode;
					CurrentFront->NextNode = NULL;
				}

				NewNode = new CBspNode;
				NewNode->AssignPolygon(&NewPolygon);
				NewNode->mBrushId = CurrentListPolygon->mBrushId;
				NewNode->mPolygonNumber = CurrentListPolygon->mPolygonNumber;
				NewNode->mBrushPtr = CurrentListPolygon->mBrushPtr;

				// The back half is the most recent polygon added
				if(*BackPolygonList == NULL) {
					*BackPolygonList = NewNode;
					(*BackPolygonList)->NextNode = NULL;
					CurrentBack = *BackPolygonList;
				}
				else {
					CurrentBack->NextNode = NewNode;
					CurrentBack = CurrentBack->NextNode;
					CurrentBack->NextNode = NULL;
				}
				break;
			}

			// Assign the current polygon to the next one
			CurrentListPolygon = NextListPolygon;
		}
		// Assign the current polygon to the next one
		CurrentStartListPolygon = NextStartListPolygon;
	}


	Node->CoplanarChild = CoplanarPolygonList;

	// once we have calculated the sub-list we need to create
	// the bounding sphere for the front and back nodes of the list
//	bspGenerateBoundingSphere(Node, *FrontPolygonList, *BackPolygonList);
}

//=====================================================================
// This function will rasterize and send in pieces to the span
// buffer a nice polygon....
//=====================================================================
void CConstructiveBsp::DrawSolidNode(CBspNode* Node, CRenderCamera* Camera, TSRenderContext* rc, int TextureType, BOOL showvolumes)
{
    // DPW - Needed for hither clipping
    int             i;
	Point3F	        in_point;
    GFXSurface*     pBuffer;
    TSPointArray*   pointArray;

	// if we have no node then return
	if(!Node)
		return;

	// if we aren't supposed to draw this polygon then leave
	if(!Node->mValidated)
		return;

    if (!Node->mBrushPtr)
      return;

    pointArray = rc->getPointArray();
    pBuffer = rc->getSurface();

    pointArray->reset();
    pointArray->useTextures(texture_indices);

    TSMaterial mattest;
    
    if (Node->mPolygonNumber < Node->mBrushPtr->mNumberOfPolygons)
    {
      mattest = poly_mat_list->getMaterial(Node->mBrushPtr->mTextureID[Node->mPolygonNumber].TextureID);
    } else {
      return;
    }

    if ((TextureType != TEXTURE_NONE) &&
        ((mattest.fParams.fFlags & TSMaterial::MatFlags) == TSMaterial::MatTexture) &&
        (((Node->Polygon.volumestate) && (!showvolumes)) ||
        (!Node->Polygon.volumestate)))
    {
    
//        pBuffer->setFillMode(GFX_FILL_TEXTURE);
        pBuffer->setFillMode(GFX_FILL_TWOPASS);
        pointArray->useTextures(TRUE);
        pBuffer->setTexturePerspective(TRUE);
        pBuffer->setShadeSource(GFX_SHADE_CONSTANT);
        pBuffer->setHazeSource(GFX_HAZE_NONE);
        pBuffer->setAlphaSource(GFX_ALPHA_NONE);
        pBuffer->setConstantShade( 1.0f );
//        pBuffer->setTextureMap( poly_mat_list->getMaterial(Node->mBrushPtr->mTextureID[Node->mPolygonNumber].TextureID).getTextureMap() );

        if (pBuffer->setTextureHandle(Node->Polygon.texture_handle))
        {
          // This texture is valid...
          pBuffer->setTextureHandle(Node->Polygon.texture_handle);
        } else {
        
          pBuffer->registerTexture(Node->Polygon.texture_handle,
                                         Node->Polygon.mTextureSize.x,
                                         Node->Polygon.mTextureSize.y,
                                         Node->Polygon.mTextureOffset.x,
                                         Node->Polygon.mTextureOffset.y,
                                         4, genericLightmap, //Node->Polygon.lm,
                                         poly_mat_list->getMaterial(Node->mBrushPtr->mTextureID[Node->mPolygonNumber].TextureID).getTextureMap(),
                                         0);

          pBuffer->setTextureHandle(Node->Polygon.texture_handle);
//          pBuffer->setFillMode(GFX_FILL_TEXTURE);
          pBuffer->setFillMode(GFX_FILL_TWOPASS);
          pointArray->useTextures(TRUE);
          pBuffer->setTexturePerspective(TRUE);
        }
    } else {
      pBuffer->setFillMode(GFX_FILL_CONSTANT);
      pBuffer->setTexturePerspective(FALSE);
      pointArray->useTextures(false);
      if ((Node->Polygon.volumestate) && (showvolumes))
      {
        pBuffer->setFillColor(255);
      } else {
        pBuffer->setFillColor(Node->Polygon.Colour);
      }
    }

    float denomx = 1.0 / (float)Node->Polygon.mTextureSize.x;
    float denomy = 1.0 / (float)Node->Polygon.mTextureSize.y;

    // Loop thru all the vertices...
    for (i = 0; i < Node->Polygon.NumberOfPoints; i++)
    {
        in_point.x = Node->Polygon.Points[i].X;
        in_point.y = Node->Polygon.Points[i].Z;
        in_point.z = Node->Polygon.Points[i].Y;

        pairs[i].fVertexIndex = pointArray->addPoint(in_point);
        pairs[i].fTextureIndex = i;

        //texture_indices[i].x = Node->Polygon.mTextureUV[i].x / Node->Polygon.mTextureSize.x;
        //texture_indices[i].y = Node->Polygon.mTextureUV[i].y / Node->Polygon.mTextureSize.y;
        texture_indices[i].x = Node->Polygon.mTextureUV[i].x * denomx;
        texture_indices[i].y = Node->Polygon.mTextureUV[i].y * denomy;

        if ((texture_indices[i].x < 0.0) || (texture_indices[i].x > 1.0) ||
            (texture_indices[i].y < 0.0) || (texture_indices[i].y > 1.0))
        {
          return;
        }
    }

   // check for emission of too many pollies
   if( GFXMetrics.emittedPolys >= (MAX_POLYS-1) )
      return;
   
   pointArray->drawPoly(Node->Polygon.NumberOfPoints, pairs, 0);
}


void CConstructiveBsp::RenderSolidWorld(CRenderCamera* Camera, TSRenderContext* rc, int TextureType, CTHREDDoc* doc)
{
    // DPW - Grab a pointer to the material list
    poly_mat_list = doc->GetMaterialList();

	// draw all of the polys
	RenderSolidTreeBackToFront(TreeTop, Camera, rc, TextureType, doc->mFilledVolumeShow);
}

void CConstructiveBsp::RenderSolidTreeBackToFront(CBspNode* Node, CRenderCamera* Camera, TSRenderContext* rc, int TextureType, BOOL showvolumes)
{
    CBspNode*   tempnode;

	// we are at a terminating leaf.
	if(!Node)
		return;

    // check which side of the camera we are on
	if(Node->ClassifyPoint(&Camera->mCameraPosition) == POLYGON_IN_FRONT)
    {
		RenderSolidTreeBackToFront(Node->BackChild, Camera, rc, TextureType, showvolumes);

        tempnode = Node;
        while(tempnode)
        {
		  DrawSolidNode(tempnode, Camera, rc, TextureType, showvolumes);
          tempnode = tempnode->CoplanarChild;
        }

		RenderSolidTreeBackToFront(Node->FrontChild, Camera, rc, TextureType, showvolumes);
	} else {
		RenderSolidTreeBackToFront(Node->FrontChild, Camera, rc, TextureType, showvolumes);

		RenderSolidTreeBackToFront(Node->BackChild, Camera, rc, TextureType, showvolumes);
	}

	// tree is not full
	return;
}


#define FIXED_MUL 16384.0
#define FIXED_SHIFT 14

//===============================================================
// This is a recursize function which is called with a bsp node
// and with a polygon.  It trys to stick the polygon somewhere
// in the tree.
// NOTE: We allocate a new node when creating this sucker
// because the originals are created on the stack.  That
// means we need a copy
// MARKER
//===============================================================
void CConstructiveBsp::FilterPolygonThroughTree(CBspNode* Node, CBspNode* Polygon, FilterType Mode)
{

	CBspNode* NewNode;
    bool      didsplitadd;

    didsplitadd = FALSE;

	// check where to go
	switch(Node->ClassifyPolygon(&Polygon->Polygon)) {

	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
	case POLYGON_IN_FRONT:
	case POLYGON_COPLANAR_SAME:
		if(Node->FrontChild) {
			FilterPolygonThroughTree(Node->FrontChild, Polygon, Mode);
		} else {
			if(Mode != SUBTRACT_BRUSH || Node->mNewNode)
            {
				NewNode = new CBspNode;
				*NewNode = *Polygon;
				Node->FrontChild = NewNode;
			}
		}
		break;
	// do the same as the front except for the back
	case POLYGON_IN_BACK:
	case POLYGON_COPLANAR_REVERSE:
		if(Node->BackChild) {
			FilterPolygonThroughTree(Node->BackChild, Polygon, Mode);
		} else {
			// If we are ADDING polygons to the tree
			// and we want to add a polygon to the BACK
			// of a polygon that is already in the tree
			// if that polygon is not a NEW node then
			// don't add the polygon to the tree
			if(Mode != ADD_BRUSH || Node->mNewNode)
            {
				NewNode = new CBspNode;
				*NewNode = *Polygon;
				Node->BackChild = NewNode;
            }
		}
		break;
	// If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
			CBspNode TempNewNode;

			// copy over some information man.
			TempNewNode = *Polygon;
			// split the gon

			Node->SplitPolygon(&Polygon->Polygon, &TempNewNode.Polygon);
			// make sure we calc our constants.
			TempNewNode.AssignPolygon(&TempNewNode.Polygon);

			// check for something in front
			if(Node->FrontChild)
            {
				FilterPolygonThroughTree(Node->FrontChild, Polygon, Mode);
			} else {
				if(Mode != SUBTRACT_BRUSH || Node->mNewNode)
                {
					NewNode = new CBspNode;
					*NewNode = *Polygon;
					Node->FrontChild = NewNode;
                    didsplitadd = TRUE;
				}
			}

			// check for something in back
			if(Node->BackChild)
            {
				FilterPolygonThroughTree(Node->BackChild, &TempNewNode, Mode);
			} else {

				// When adding to the tree if the node isn't
				// a new node then we discard poly's that
				// we were to add to the back
				if(Mode != ADD_BRUSH || Node->mNewNode)
                {
					NewNode = new CBspNode;
					*NewNode = TempNewNode;
					Node->BackChild = NewNode;
				}
			}
	      break;
		}
	}
}

//===============================================================
// This is a recursize function which is called with a bsp node
// and with a polygon.  It trys to stick the polygon somewhere
// in the tree.
// NOTE: We allocate a new node when creating this sucker
// because the originals are created on the stack.  That
// means we need a copy
// MARKER
//===============================================================
void CConstructiveBsp::FilterPolygonThroughTreeSubtract(CBspNode* Node, CBspNode* Polygon, FilterType Mode)
{
	CBspNode* NewNode;
    bool      didsplitadd;

    didsplitadd = FALSE;

	// check where to go
	switch(Node->ClassifyPolygon(&Polygon->Polygon)) {

	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
	case POLYGON_IN_FRONT:
	case POLYGON_COPLANAR_REVERSE:
		if(Node->FrontChild) {
			FilterPolygonThroughTreeSubtract(Node->FrontChild, Polygon, Mode);
		} else {
			if(Mode != SUBTRACT_BRUSH || Node->mNewNode)
            {
				NewNode = new CBspNode;
				*NewNode = *Polygon;
				Node->FrontChild = NewNode;
			}
		}
		break;
	// do the same as the front except for the back
	case POLYGON_IN_BACK:
	case POLYGON_COPLANAR_SAME:
		if(Node->BackChild) {
			FilterPolygonThroughTreeSubtract(Node->BackChild, Polygon, Mode);
		} else {
			// If we are ADDING polygons to the tree
			// and we want to add a polygon to the BACK
			// of a polygon that is already in the tree
			// if that polygon is not a NEW node then
			// don't add the polygon to the tree
			if(Mode != ADD_BRUSH || Node->mNewNode)
            {
				NewNode = new CBspNode;
				*NewNode = *Polygon;
				Node->BackChild = NewNode;
			}
		}
		break;
	// If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
		  CBspNode TempNewNode;
			// copy over some information man.
			TempNewNode = *Polygon;
			// split the gon
			Node->SplitPolygon(&Polygon->Polygon, &TempNewNode.Polygon);
			// make sure we calc our constants.
			TempNewNode.AssignPolygon(&TempNewNode.Polygon);

			// check for something in front
			if(Node->FrontChild) {
				FilterPolygonThroughTreeSubtract(Node->FrontChild, Polygon, Mode);
			} else {
				if(Mode != SUBTRACT_BRUSH || Node->mNewNode)
                {
					NewNode = new CBspNode;
					*NewNode = *Polygon;
					Node->FrontChild = NewNode;
				}
			}

			// check for something in back
			if(Node->BackChild) {
				FilterPolygonThroughTreeSubtract(Node->BackChild, &TempNewNode, Mode);
			} else {

				// When adding to the tree if the node isn't
				// a new node then we discard poly's that
				// we were to add to the back
				if(Mode != ADD_BRUSH || Node->mNewNode)
                {
					NewNode = new CBspNode;
					*NewNode = TempNewNode;
					Node->BackChild = NewNode;
				}
			}
		  break;
		}
	}
}

void CConstructiveBsp::FilterCSGOnly(CThredBrush* Brush)
{
  CBspNode* temp;
  CBspNode* tempcoplanar;
  CBspNode* CoplanarNode;
  int       CoplanarCount;

  if (!TreeTop)
    return;

  temp = TreeTop;

  // Walk the list
  while(temp)
  {
    //if(!temp->mNewNode)
    //{
      tempcoplanar = temp;

      CoplanarCount = 0;
      CoplanarNode = temp;
	  while(CoplanarNode)
      {
		CoplanarCount++;
		CoplanarNode = CoplanarNode->CoplanarChild;
      }

      // Walk down coplanar polygons...
      while (CoplanarCount)
      {
        if(tempcoplanar->mValidated)
        {
          Brush->mBrushBsp.DoCSGFilterKeepOutside(tempcoplanar);
        }

        tempcoplanar = tempcoplanar->CoplanarChild;
        CoplanarCount--;
      }
    //}

    temp = temp->FrontChild;
  }
}

void CConstructiveBsp::FilterBrushBsp(CBspNode* Node, CConstructiveBsp& BrushBsp)
{
	CBspNode* CoplanarNode;
	int CoplanarCount =0;

	if(!Node)
		return;

	if(Node->mNewNode) 
		return;
	
	// go down the back
	FilterBrushBsp(Node->BackChild, BrushBsp);

	// go down the front
	FilterBrushBsp(Node->FrontChild, BrushBsp);

	// count the coplanar nodes
	CoplanarNode = Node;
	while(CoplanarNode) {
		CoplanarCount++;
		CoplanarNode = CoplanarNode->CoplanarChild;

	}

	// go through and filter them
	CoplanarNode = Node;
	while(CoplanarCount--) {
		// do the actual filtering of ourselves!
//		if(CoplanarNode->mValidated &&  (!CoplanarNode->mNewNode))
		if(CoplanarNode->mValidated)
			BrushBsp.DoCSGFilterKeepOutside(CoplanarNode);
		CoplanarNode = CoplanarNode->CoplanarChild;
	}
}

void	CConstructiveBsp::DoCSGFilterKeepOutsideNoCoplanars(CBspNode* Node, 
	CBspNode* Polygon, int& NumberFront, int& NumberBack)
{
    //int     l;
    //double  dotprod;
    Point3D normal, nodenormal;

	// We will use whether the node is drawn
	// as the check to whether or not
	// we have had a backside polygon.
	if(!Node)
		return;

	// check where to go
	switch(Node->ClassifyPolygon(&Polygon->Polygon)) {

	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
	case POLYGON_IN_FRONT:
	//case POLYGON_COPLANAR_SAME:
		if(Node->FrontChild) {
			// do our sub-filter
			DoCSGFilterKeepOutsideNoCoplanars(Node->FrontChild, Polygon, NumberFront, NumberBack);

		} else {
			Polygon->mValidated = TRUE;
			NumberFront++;
		}
		break;

    // do the same as the front except for the back
	case POLYGON_IN_BACK:
 	case POLYGON_COPLANAR_REVERSE:
	case POLYGON_COPLANAR_SAME:
 	//case POLYGON_COPLANAR_REVERSE:
		if(Node->BackChild) {
			// do our sub-filter
			DoCSGFilterKeepOutsideNoCoplanars(Node->BackChild, Polygon, NumberFront, NumberBack);
		} else {
			Polygon->mValidated = FALSE;
			// do nothing here.
			NumberBack++;
		}
		break;

    // If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
			// declare a new block here because we want to save stack if possible
			ThredPolygon UsagePolygon, SavePolygon;
			CBspNode* NewNode;
			int FrontCount, BackCount;

			// save our original gon
			SavePolygon = Polygon->Polygon;

			// split the gon
			Node->SplitPolygon(&Polygon->Polygon, &UsagePolygon);

			// make a new node and filter it through the tree
			NewNode = new CBspNode;
			NewNode->AssignPolygon(&UsagePolygon);
			NewNode->mValidated = Polygon->mValidated;
			NewNode->mBrushId = Polygon->mBrushId;
			NewNode->mPolygonNumber = Polygon->mPolygonNumber;
			NewNode->mBrushPtr = Polygon->mBrushPtr;
            NewNode->perturbed = Polygon->perturbed;

			FrontCount = BackCount = 0;
			// check for something in front
			if(Node->FrontChild) {
				// do our sub-filter
				DoCSGFilterKeepOutsideNoCoplanars(Node->FrontChild, Polygon, FrontCount, BackCount);
			} else {
				Polygon->mValidated = TRUE;
				FrontCount++;
			}

			//FrontCount = BackCount = 0;
			// check for something in back
			if(Node->BackChild) {
				// do our sub-filter
				DoCSGFilterKeepOutsideNoCoplanars(Node->BackChild, NewNode, FrontCount, BackCount);
			} else {
				// do nothing here.
				NewNode->mValidated = FALSE;
				BackCount++;
			}

			// Now we want to check whether or not to UNSPLIT
			// the polygon that we just split.  This would
			// happen if all of the nodes ended up on the front.
			// if we UNSPLIT then we return that all of the nodes ended up onthe front
			if(BackCount == 0) {

				// delete the new node that we made
				delete NewNode;
				// tell them this one went on the front
				NumberFront++;
				// assign our old polygon.
				Polygon->AssignPolygon(&SavePolygon);
			} 
			else {
				// add our newnode to the list of coplanar children.
				while(Polygon->CoplanarChild)
					Polygon = Polygon->CoplanarChild;
				Polygon->CoplanarChild = NewNode;

				// increment our counts
				NumberFront += FrontCount;
				NumberBack  += BackCount;
			}
		}
		break;
	}
}

void	CConstructiveBsp::DoCSGFilterKeepOutsideCoplanars(CBspNode* Node, 
	CBspNode* Polygon, int& NumberFront, int& NumberBack)
{
    //int     l;
    //double  dotprod;
    Point3D normal, nodenormal;

	// We will use whether the node is drawn
	// as the check to whether or not
	// we have had a backside polygon.
	if(!Node)
		return;

	// check where to go
	switch(Node->ClassifyPolygon(&Polygon->Polygon)) {

	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
	case POLYGON_IN_FRONT:
	//case POLYGON_COPLANAR_SAME:
	case POLYGON_COPLANAR_REVERSE:
	if(Node->FrontChild) {
			// do our sub-filter
			DoCSGFilterKeepOutsideCoplanars(Node->FrontChild, Polygon, NumberFront, NumberBack);

		} else {
			Polygon->mValidated = TRUE;
			NumberFront++;
		}
		break;

    // do the same as the front except for the back
	case POLYGON_IN_BACK:
	case POLYGON_COPLANAR_SAME:
		if(Node->BackChild) {
			// do our sub-filter
			DoCSGFilterKeepOutsideCoplanars(Node->BackChild, Polygon, NumberFront, NumberBack);
		} else {
			Polygon->mValidated = FALSE;
			// do nothing here.
			NumberBack++;
		}
		break;
#if 0
	case POLYGON_COPLANAR_SAME:
	case POLYGON_COPLANAR_REVERSE:
      {
          //Point3D normal, nodenormal;

          // Perturb Polygon's position by some amount along its normal
          // (This should cause splits to occur along our polygon)
          normal.x = Polygon->ConstantA;
          normal.y = Polygon->ConstantB;
          normal.z = Polygon->ConstantC;
          normal.normalize();

          nodenormal.x = Node->ConstantA;
          nodenormal.y = Node->ConstantB;
          nodenormal.z = Node->ConstantC;
          nodenormal.normalize();

          dotprod = md_dot(nodenormal, normal);

          if (dotprod == 1.0)
          {
            DoCSGFilterKeepOutsideNoCoplanars(Node, Polygon, NumberFront, NumberBack);
            return;
          } else {
              normal.x *= -1.0;
              normal.y *= -1.0;
              normal.z *= -1.0;
          }

          // Perturb that poly!
            for (l = 0; l < Polygon->Polygon.NumberOfPoints; l++)
            {
                Polygon->Polygon.Points[l].X += normal.x;
                Polygon->Polygon.Points[l].Y += normal.y;
                Polygon->Polygon.Points[l].Z += normal.z;
            }

            Polygon->perturbed = 1;

	        // do our sub-filter
		    DoCSGFilterKeepOutsideCoplanars(Node, Polygon, NumberFront, NumberBack);

            // Loop through any coplanars that might have been created by the split
            // and unperturb those polygons
            CBspNode* currnode = Polygon;

            while (currnode)
            {
              if (currnode->perturbed)
              {
                for (l = 0; l < currnode->Polygon.NumberOfPoints; l++)
                {
                  currnode->Polygon.Points[l].X -= normal.x;
                  currnode->Polygon.Points[l].Y -= normal.y;
                  currnode->Polygon.Points[l].Z -= normal.z;
                }

                currnode->perturbed = 0;
              }

              currnode = currnode->CoplanarChild;
            }
          //} else {
		//	DoCSGFilterKeepOutsideCoplanars(Node->BackChild, Polygon, NumberFront, NumberBack);
          //}
      }
		break;
#endif
    // If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
			// declare a new block here because we want to save stack if possible
			ThredPolygon UsagePolygon, SavePolygon;
			CBspNode* NewNode;
			int FrontCount, BackCount;

			// save our original gon
			SavePolygon = Polygon->Polygon;

			// split the gon
			Node->SplitPolygon(&Polygon->Polygon, &UsagePolygon);

			// make a new node and filter it through the tree
			NewNode = new CBspNode;
			NewNode->AssignPolygon(&UsagePolygon);
			NewNode->mValidated = Polygon->mValidated;
			NewNode->mBrushId = Polygon->mBrushId;
			NewNode->mPolygonNumber = Polygon->mPolygonNumber;
			NewNode->mBrushPtr = Polygon->mBrushPtr;
            NewNode->perturbed = Polygon->perturbed;

			FrontCount = BackCount = 0;
			// check for something in front
			if(Node->FrontChild) {
				// do our sub-filter
				DoCSGFilterKeepOutsideCoplanars(Node->FrontChild, Polygon, FrontCount, BackCount);
			} else {
				Polygon->mValidated = TRUE;
				FrontCount++;
			}

			//FrontCount = BackCount = 0;
			// check for something in back
			if(Node->BackChild) {
				// do our sub-filter
				DoCSGFilterKeepOutsideCoplanars(Node->BackChild, NewNode, FrontCount, BackCount);
			} else {
				// do nothing here.
				NewNode->mValidated = FALSE;
				BackCount++;
			}

			// Now we want to check whether or not to UNSPLIT
			// the polygon that we just split.  This would
			// happen if all of the nodes ended up on the front.
			// if we UNSPLIT then we return that all of the nodes ended up onthe front
			if(BackCount == 0) {

				// delete the new node that we made
				delete NewNode;
				// tell them this one went on the front
				NumberFront++;
				// assign our old polygon.
				Polygon->AssignPolygon(&SavePolygon);
			} 
			else {
				// add our newnode to the list of coplanar children.
				while(Polygon->CoplanarChild)
					Polygon = Polygon->CoplanarChild;
				Polygon->CoplanarChild = NewNode;
                //Polygon->CoplanarChild->perturbed = Polygon->perturbed;

				// increment our counts
				NumberFront += FrontCount;
				NumberBack  += BackCount;
			}
		}
		break;
	}
}

void	CConstructiveBsp::DoCSGFilterKeepOutside(CBspNode* Node, 
	CBspNode* Polygon, int& NumberFront, int& NumberBack)
{
	// We will use whether the node is drawn
	// as the check to whether or not
	// we have had a backside polygon.
	if(!Node)
		return;

	// check where to go
	switch(Node->ClassifyPolygon(&Polygon->Polygon)) {

	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
	case POLYGON_IN_FRONT:
	case POLYGON_COPLANAR_SAME:
		if(Node->FrontChild) {
			// do our sub-filter
			DoCSGFilterKeepOutside(Node->FrontChild, Polygon, NumberFront, NumberBack);

		} else {
			Polygon->mValidated = TRUE;
			NumberFront++;
		}
		break;

    // do the same as the front except for the back
	case POLYGON_IN_BACK:
	case POLYGON_COPLANAR_REVERSE:
		if(Node->BackChild) {
			// do our sub-filter
			DoCSGFilterKeepOutside(Node->BackChild, Polygon, NumberFront, NumberBack);
		} else {
			Polygon->mValidated = FALSE;
			// do nothing here.
			NumberBack++;
		}
		break;

	// If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
			// declare a new block here because we want to save stack if possible
			ThredPolygon UsagePolygon, SavePolygon;
			CBspNode* NewNode;
			int FrontCount, BackCount;

			// save our original gon
			SavePolygon = Polygon->Polygon;

			// split the gon
			Node->SplitPolygon(&Polygon->Polygon, &UsagePolygon);

			// make a new node and filter it through the tree
			NewNode = new CBspNode;
			NewNode->AssignPolygon(&UsagePolygon);
			NewNode->mValidated = Polygon->mValidated;
			NewNode->mBrushId = Polygon->mBrushId;
			NewNode->mPolygonNumber = Polygon->mPolygonNumber;
			NewNode->mBrushPtr = Polygon->mBrushPtr;

			FrontCount = BackCount = 0;
			// check for something in front
			if(Node->FrontChild) {
				// do our sub-filter
				DoCSGFilterKeepOutside(Node->FrontChild, Polygon, FrontCount, BackCount);

			} else {
				Polygon->mValidated = TRUE;
				FrontCount++;
			}


			//FrontCount = BackCount = 0;
			// check for something in back
			if(Node->BackChild) {
				// do our sub-filter
				DoCSGFilterKeepOutside(Node->BackChild, NewNode, FrontCount, BackCount);
			} else {
				// do nothing here.
				NewNode->mValidated = FALSE;
				BackCount++;
			}

			// Now we want to check whether or not to UNSPLIT
			// the polygon that we just split.  This would
			// happen if all of the nodes ended up on the front.
			// if we UNSPLIT then we return that all of the nodes ended up onthe front
			if(BackCount == 0) {

				// delete the new node that we made
				delete NewNode;
				// tell them this one went on the front
				NumberFront++;
				// assign our old polygon.
				Polygon->AssignPolygon(&SavePolygon);
			} 
			else {
				// add our newnode to the list of coplanar children.
				while(Polygon->CoplanarChild)
					Polygon = Polygon->CoplanarChild;
				Polygon->CoplanarChild = NewNode;

				// increment our counts
				NumberFront += FrontCount;
				NumberBack  += BackCount;
			}
		}
		break;
	}
}

//==============================================================
// This function will create a convex bsp.
// The brush that
// we pass in has pointers to the next brushes after it in
// the list so obviously we can do proper subtractions on
// it by using these brushes.
//==============================================================
void CConstructiveBsp::CreateConvexBsp(CThredBrush* Brush)
{
#if 0 //DPW - 02/20/97
	CBspNode Dummy;
	CBspNode* PolygonList = NULL;
	CThredBrush* CurrentBrush;


	// Clear out our bsp
	ClearBsp();

	// take our current brush and and make a world out of it
	ProcessBrush(Brush, 0);

	// now go through the list of brushes after this one
	// and process them into this brush
	CurrentBrush = Brush->NextActiveBrush();
	while(CurrentBrush) {

		// if this brush is a sub then check for sub
		if(CurrentBrush->mTransactionType == BRUSH_SUBTRACT_FROM_WORLD) {

			// make sure the brush is transformed
			CurrentBrush->TransformBrushPolygons();

			// check if our bounding boxes intersect
			if(Brush->mBoundingBox.CheckIntersection(CurrentBrush->mBoundingBox)) {
				ProcessBrush(CurrentBrush, 0);
			}
		}
	
		// go on to next
		CurrentBrush = CurrentBrush->NextActiveBrush();
	}


	// build a list from our tree
	BuildPolygonListFromTree(&Dummy, TreeTop);
	PolygonList = Dummy.NextNode;

	// now we should have a list of these bsp nodes
	// so we go through and partition the world
	// get the best node to partition with
	CBspNode* BestNode = ConvexGetBestNode(PolygonList);

	if(BestNode == NULL) {
		TreeTop = PolygonList;
		TreeTop->Splitter = 1;
		return;
	}

	// get our treetop
	TreeTop = BestNode;

	// make our front/back lists have nothing in them
	CBspNode *FrontPolygonList = NULL, *BackPolygonList = NULL;

	// calculate the new sublevel for our new tree
	ConvexCalculateSubLevel(TreeTop, PolygonList, &FrontPolygonList,
		&BackPolygonList);

	// recursively subdivide our nice tree
	ConvexDoSubTree(TreeTop, FrontPolygonList, BackPolygonList);

	// now we have to filter ourselves down our
	// own subtrees to create the "final" lists
	FilterDownSubtree(TreeTop);
#endif // DPW - 02/20/97
}


//===================================================================================
// This function goes through the sub-tree and filters all of the nodes down there
// children.  Since we can have co-planar nodes in the tree it will ONLY filter
// down coplanar nodes ONCE... e.g. it doesn't do anything with the coplanar list.
// NOTE: There is a bug because a backside polygon can be considered coplanar.  It
// should really just be backside I think.
//===================================================================================
void CConstructiveBsp::FilterDownSubtree(CBspNode* Node)
{
	ThredPolygon Polygon;
	CBspNode FilterNode;

	if(!Node)
		return;

	// if we are a splitter we don't filter
	if(Node->Splitter)
		return;

	// make a forward facing large polygon
	// to be culled into the tree.
	Node->CalcPolyFromPlane(Node, Polygon);
	FilterNode.AssignPolygon(&Polygon);
	FilterNode.mPolygonNumber = Node->mPolygonNumber;
	FilterNode.mBrushPtr = Node->mBrushPtr;
	ConvexFilterDown(Node->BackChild, &FilterNode);

	Node->CalcBackPolyFromPlane(Node, Polygon);
	FilterNode.AssignPolygon(&Polygon);
	FilterNode.mPolygonNumber = Node->mPolygonNumber;
	FilterNode.mReverseRef = 1;
	FilterNode.mBrushPtr = Node->mBrushPtr;
	ConvexFilterDown(Node->FrontChild, &FilterNode);

	// filter down our back and front.
	FilterDownSubtree(Node->BackChild);
	FilterDownSubtree(Node->FrontChild);
}



//===================================================================================
// Please not that the END of lists are ONLY fixed up if a node is filter through them
// This is why it might be a good idea to go through and "fix" leaf nodes before
// we export..... just an idea.
//===================================================================================
void CConstructiveBsp::ConvexFilterDown(CBspNode* Node, CBspNode* Poly)
{
	CBspNode* NewNode;
	int ReturnVal;

	// vee have a feck up
	if(!Node)
		return;

	// we have a list... we have to filter
	// the poly through the list keeping only
	// the stuff on the backside.
	if(Node->Splitter) {
		CBspNode* Start = Node;

		// we have to go through all of
		// these splitter nodes and
		// see if we end up with something
		// on the backside.  If we do then
		// we can keep it
		while(1) {

			ReturnVal = Node->ClassifyPolygon(&Poly->Polygon);
			if(ReturnVal == POLYGON_IN_FRONT || ReturnVal == POLYGON_COPLANAR_SAME || ReturnVal == POLYGON_COPLANAR_REVERSE) 
				return;

			if(Node->ClassifyPolygon(&Poly->Polygon) == POLYGON_SPLIT) {
				ThredPolygon NewPoly;
				// newpoly will hold the backside which is what we want
				Node->SplitPolygon(&Poly->Polygon, &NewPoly);
				Poly->Polygon = NewPoly;
			}

			if(Node->NextNode)
				Node = Node->NextNode;
			else
				break;
		}

		// we got through the process so
		// add the node
		NewNode = new CBspNode;
		NewNode->AssignPolygon(&Poly->Polygon);
		NewNode->Splitter = 1;
		NewNode->mPolygonNumber = Poly->mPolygonNumber;
		NewNode->mReverseRef = Poly->mReverseRef;
		NewNode->mBrushPtr = Poly->mBrushPtr;
		Node->NextNode = NewNode;


		// now we have to go through each node and clip it
		// against each other node to make the polygon's nice and small
		Node = Start;
		CBspNode* Cur;
		while(Node) {

			for(Cur = Start; Cur; Cur = Cur->NextNode) {
				// don't compare against ourselves
				if(Cur == Node)
					continue;

				// check if we are in front of node
				if(Node->ClassifyPolygon(&Cur->Polygon) == POLYGON_IN_FRONT)  {
					//_ASSERT(0);
					Cur->mValidated = 0;
					continue;
				}

				// if we aren't in front check if we are a split
				if(Node->ClassifyPolygon(&Cur->Polygon) == POLYGON_SPLIT) {
					ThredPolygon NewPoly;
					// newpoly will hold the backside which is what we want
					Node->SplitPolygon(&Cur->Polygon, &NewPoly);
					Cur->Polygon = NewPoly;
				}
			}

			Node = Node->NextNode;
		}
		return;
	}

	// check where to go
	switch(Node->ClassifyPolygon(&Poly->Polygon)) {

	// If the polygon is in front we go down that way
	// if we can.  Otherwise we make a new node and stick
	// the polygon there.
		// remove if coplanar... or should we... good question...
//		break;
	case POLYGON_COPLANAR_SAME:
	case POLYGON_IN_FRONT:
		if(Node->FrontChild) {
			//if(!Node->FrontChild->Splitter)
				ConvexFilterDown(Node->FrontChild, Poly);
		} else {
			//NewNode = new CBspNode;
			//NewNode->AssignPolygon(&Poly->Polygon);
			//NewNode->Splitter = 1;
			//NewNode->mPolygonNumber = Poly->mPolygonNumber;
			//NewNode->mReverseRef = Poly->mReverseRef;
			//NewNode->mBrushPtr = Poly->mBrushPtr;
			//Node->FrontChild = NewNode;
		}
		break;
	// do the same as the front except for the back
	case POLYGON_IN_BACK:
	case POLYGON_COPLANAR_REVERSE:
		if(Node->BackChild) {
			ConvexFilterDown(Node->BackChild, Poly);
		} else {
			NewNode = new CBspNode;
			NewNode->AssignPolygon(&Poly->Polygon);
			NewNode->Splitter = 1;
			NewNode->mPolygonNumber = Poly->mPolygonNumber;
			NewNode->mReverseRef = Poly->mReverseRef;
			NewNode->mBrushPtr = Poly->mBrushPtr;
			Node->BackChild = NewNode;
		}
		break;
	// If the polygon is split by the node then we
	// send it down both sides of the tree as new
	// polygons.
	case POLYGON_SPLIT:
		{
			CBspNode NewPoly;
			NewPoly.mPolygonNumber = Poly->mPolygonNumber;
			NewPoly.mReverseRef = Poly->mReverseRef;
			NewPoly.mBrushPtr = Poly->mBrushPtr;

			// split the gon
			Node->SplitPolygon(&Poly->Polygon, &NewPoly.Polygon);

			// check for something in front
			if(Node->FrontChild) {
				//if(!Node->FrontChild->Splitter)
					ConvexFilterDown(Node->FrontChild, Poly);
			} else {
				//NewNode = new CBspNode;
				//NewNode->AssignPolygon(&Poly->Polygon);
				//NewNode->Splitter = 1;
				//NewNode->mPolygonNumber = Poly->mPolygonNumber;
				//NewNode->mReverseRef = Poly->mReverseRef;
				//NewNode->mBrushPtr = Poly->mBrushPtr;
				//Node->FrontChild = NewNode;
			}

			// check for something in back
			if(Node->BackChild) {
				ConvexFilterDown(Node->BackChild, &NewPoly);
			} else {
				NewNode = new CBspNode;
				NewNode->AssignPolygon(&NewPoly.Polygon);
				NewNode->Splitter = 1;
				NewNode->mPolygonNumber = Poly->mPolygonNumber;
				NewNode->mReverseRef = Poly->mReverseRef;
				NewNode->mBrushPtr = Poly->mBrushPtr;
				Node->BackChild = NewNode;
			}
		}
		break;
	}

}

//=========================================================================
// This function makes sure that this polygon is either IN or ON the surface
// of our geometry.
//=========================================================================
int CConstructiveBsp::MakeSurePolygonIn(CBspNode* Node, ThredPolygon* Polygon)
{
	// vee have a feck up
	if(!Node)
		return 1;

	// check where to go
	switch(Node->ClassifyPolygon(Polygon)) {

	// if we are in front we are fucked
	case POLYGON_IN_FRONT:

		Node->ClassifyPolygon(Polygon);
		if(Node->FrontChild) {
			return MakeSurePolygonIn(Node->FrontChild, Polygon);
		} else {
			return 0;
		}
		break;

	// if we are in the back or on we are ok
	case POLYGON_IN_BACK:
	case POLYGON_COPLANAR_REVERSE:
	case POLYGON_COPLANAR_SAME:
		if(Node->BackChild) {
			return MakeSurePolygonIn(Node->BackChild, Polygon);
		} else {
			return 1;
		}
		break;
	case POLYGON_SPLIT:
		{
			// if we don't have a front child don't even bother splitting
			// because we may just return
			if(!Node->FrontChild)
				return 0;

			// declare a new block here because we want to save stack if possible
			ThredPolygon FrontPolygon, BackPolygon;

			// save our original gon
			FrontPolygon = *Polygon;

			// split the gon
			Node->SplitPolygon(&FrontPolygon, &BackPolygon);

			// filter down the front
			if(!MakeSurePolygonIn(Node->FrontChild, &FrontPolygon))
				return 0;

			// now return whatever the back gives us
			return MakeSurePolygonIn(Node->BackChild, &BackPolygon);
		}
	}
	return 0;
}


// Go through and fixup the pointers to all of the brushes for each of our polygons
void CConstructiveBsp::FixupPointers(CThredBrush* BrushList)
{
	// do the fixup
	FixupRecursive(BrushList, TreeTop);
}


//===========================================================
// This is a recursize function to fixup the pointers to the brush
//===========================================================
void CConstructiveBsp::FixupRecursive(CThredBrush* BrushList, CBspNode* Node)
{

	// get out of here if no node
	if(!Node)
		return;

	FixupRecursive(BrushList, Node->BackChild);
	FixupRecursive(BrushList, Node->FrontChild);

	do {
		// this function gets the the brush pointer from the list
		// that matches the brush id
		Node->mBrushPtr = BrushList->GetBrushById(Node->mBrushId);

        // dpw?????
//        if (!Node->mBrushPtr)
//        {
//          Node->mValidated = FALSE;
//        }

		// go to the next node
		Node = Node->CoplanarChild;
	} while(Node);
}

void CConstructiveBsp::FlushCacheInfo()
{
  KeyValue.key[0] = 0;
  KeyValue.key[1] = 0;
  gfxTextureCache.flushCache();
}

//=======================================================
// -Changed logic somewhat for the sub-brushes when filtering
// through an add brush previous to us if we are already validated.
//=======================================================
void CConstructiveBsp::RebuildOptimal(CProgressCtrl* Progress, int BuildWater, bool exporting)
{
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   char  buf[80];

   // Give some feedback...
   pFrame->UpdateGeneralStatus("Clearing out the BSP tree");
   showpcount = 1;

   // clear the bsp
   ClearBsp();
   TreeTop = NULL;

   // Let's make a random access array
   // containing all the brush
   CThredBrush* CurrentBrush;
   CThredBrush** BrushList;
   int BrushCount = 0;
   int Count;

   showpcount = 0;

   // get a Brush Count
   CurrentBrush = mBrushList->GetActiveBrush();
   while( CurrentBrush )
   {
      CurrentBrush = CurrentBrush->NextActiveBrush();
      BrushCount++;
   }

   // allocate the array
   BrushList = new CThredBrush*[BrushCount];

   // go through and allocate all the brushes to the list
   CurrentBrush = mBrushList->GetActiveBrush();
   Count = 0;
   while( CurrentBrush )
   {
      // assign the brush
      BrushList[Count++] = CurrentBrush;

      // transform
      CurrentBrush->TransformBrushPolygons();

      // init our quick bsp system
      CurrentBrush->mBrushBsp.InitializeQuickBspNodes(CurrentBrush->mNumberOfPolygons);

      // clear the bsp for the brush.
      CurrentBrush->mBrushBsp.ClearBsp();

      // go to the next brush
      CurrentBrush = CurrentBrush->NextActiveBrush();
   }

   // Go through each brush.. then go through each polygon
   // of each brush and filter it
   CBspNode* PolygonList = NULL;
   CBspNode* CurrentNode;
   int CoplanarCount;
   for(Count = 0; Count < BrushCount; Count++)
   {
      // Give some feedback...
      sprintf(buf, "Processing (+) Brushes: %d of %d", Count, BrushCount);
      pFrame->UpdateGeneralStatus(buf);

      if(Progress)
         Progress->StepIt();

      // get a current brush
      CurrentBrush = BrushList[Count];

      // if we are not active leave
      if(!(CurrentBrush->mFlags & BRUSH_ACTIVE))
         continue;

      // WE ARE ONLY DOING ADD BRUSH POLYGONS RIGHT NOW
      if(CurrentBrush->mTransactionType != BRUSH_ADD_TO_WORLD)
         continue;

      if(!BuildWater && CurrentBrush->mName[0] == '*')
         continue;

      // Don't deal with volume brushes yet
      if ((CurrentBrush->volumestate) && (!CurrentBrush->m_ShowVolPolys))
         continue;

      // go through the gons of the current brush
      for(int CurrentGon = 0; CurrentGon < CurrentBrush->mNumberOfPolygons; CurrentGon++)
      {
         // get a poly we can use
         if(PolygonList)
         {
            CurrentNode->NextNode = new CBspNode;
            CurrentNode = CurrentNode->NextNode;
         } 
         else 
         {
            PolygonList = new CBspNode;
            CurrentNode = PolygonList;
         }

         // copy over the polygon, ADDbrushes polygons start out as validated.
         CurrentNode->AssignPolygon(&CurrentBrush->mTransformedPolygons[CurrentGon]);
         CurrentNode->mValidated = TRUE;
         CurrentNode->mBrushId = CurrentBrush->mBrushId;
         CurrentNode->mPolygonNumber = CurrentGon;
         CurrentNode->mBrushPtr = CurrentBrush;
      
         // go through all of the brushes
         for(int BrushNumber = 0; BrushNumber < BrushCount; BrushNumber++)
         {
            // don't filter our polygons through ourself
            if(BrushNumber == Count)
               continue;

            if(!(BrushList[BrushNumber]->mFlags & BRUSH_ACTIVE))
               continue;

            if(!BuildWater && BrushList[BrushNumber]->mName[0] == '*')
               continue;

            // Don't deal with volume brushes yet
            if (BrushList[BrushNumber]->volumestate)
               continue;

            // don't filter our polygons through brushes
            // which have differing bounding boxes
            if(!BrushList[BrushNumber]->mBoundingBox.CheckIntersection(BrushList[Count]->mBoundingBox))
               continue;

            // for each brush go through our list of polygons
            // and filter them through appropriately
            CBspNode* CurrentPolygon = CurrentNode;
            CoplanarCount = 0;
            while(CurrentPolygon)
            {
               CoplanarCount++;
               CurrentPolygon = CurrentPolygon->CoplanarChild;
            }
            CurrentPolygon = CurrentNode;
            while(CoplanarCount--)
            {

               if(BrushList[BrushNumber]->mTransactionType == BRUSH_ADD_TO_WORLD)
               {
                  if(CurrentPolygon->mValidated)
                  {
                     if(BrushNumber < Count)
                     {
                        // Current brush comes after filter brush...
                        CurrentPolygon->perturbed = 0;
                        BrushList[BrushNumber]->CreateBrushBsp();
                        //BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepOutsideNoCoplanars(CurrentPolygon);
                        BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepOutside(CurrentPolygon);
                        BrushList[BrushNumber]->mBrushBsp.ClearBsp();
                     } 
                     else 
                     {
                        CurrentPolygon->perturbed = 0;
                        BrushList[BrushNumber]->CreateBrushBsp();
                        //BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepOutsideNoCoplanars(CurrentPolygon);
                        BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepOutsideNoCoplanars(CurrentPolygon);
                        BrushList[BrushNumber]->mBrushBsp.ClearBsp();
                     }
                  }
               } 
               else 
               {
                  // if this brush is BEFORE us
                  if(BrushNumber < Count)
                  {
                     CurrentPolygon->perturbed = 0;
                     BrushList[BrushNumber]->CreateBrushBsp();
                     BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepInside(CurrentPolygon);
                     BrushList[BrushNumber]->mBrushBsp.ClearBsp();
                  } 
                  else 
                  {
                     if(CurrentPolygon->mValidated)
                     {
                        // DPW - 3/21/97
                        //BrushList[BrushNumber]->CreateQuickBrushBsp(CurrentPolygon);
                        CurrentPolygon->perturbed = 0;
                        BrushList[BrushNumber]->CreateBrushBsp();
                        BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepOutsideNoCoplanars(CurrentPolygon);
                        BrushList[BrushNumber]->mBrushBsp.ClearBsp();
                     }
                  }
               }

               // go on to the next coplanar child
               CurrentPolygon = CurrentPolygon->CoplanarChild;
            }
         }
      }
   }

   // make sure we don't leave a quick tree lying around
   // to be cleared!! this would be BAD!
   // WE are not doing "special" bsps for subtractions brushes because
   // they don't seem to need them cause all the stuff that happens is
   // inside the brush....  we shall see.
   for(int BrushNumber = 0; BrushNumber < BrushCount; BrushNumber++)
   {
      if(BrushList[BrushNumber]->mFlags & BRUSH_ACTIVE)
      {
         //BrushList[BrushNumber]->mBrushBsp.ClearBsp();
         BrushList[BrushNumber]->mBrushBsp.TreeTop = NULL;
         BrushList[BrushNumber]->CreateBrushBsp();
      }
   }

   // DO THE SUBTRACTION BRUSHES
   for(Count = 0; Count < BrushCount; Count++)
   {
      // Give some feedback...
      sprintf(buf, "Processing (-) Brushes: %d of %d", Count, BrushCount);
      pFrame->UpdateGeneralStatus(buf);

      if(Progress)
         Progress->StepIt();

      // get a current brush
      CurrentBrush = BrushList[Count];

      // if we are not active leave
      if(!(CurrentBrush->mFlags & BRUSH_ACTIVE))
         continue;

      // WE ARE ONLY DOING SUBTRACT BRUSH POLYGONS RIGHT NOW
      if(CurrentBrush->mTransactionType != BRUSH_SUBTRACT_FROM_WORLD)
         continue;

      if(!BuildWater && CurrentBrush->mName[0] == '*')
         continue;

      // Don't deal with volume brushes yet
      if (CurrentBrush->volumestate)
         continue;

      // go through the gons of the current brush
      for(int CurrentGon = 0; CurrentGon < CurrentBrush->mNumberOfPolygons; CurrentGon++)
      {
         // get a poly we can use
         if(PolygonList)
         {
            CurrentNode->NextNode = new CBspNode;
            CurrentNode = CurrentNode->NextNode;
         } 
         else 
         {
            PolygonList = new CBspNode;
            CurrentNode = PolygonList;
         }

         ThredPolygon AddPolygon;
         AddPolygon = CurrentBrush->mTransformedPolygons[CurrentGon];

         // reverse the order of the verts
         int CurrentVert=0, MaxVert= AddPolygon.NumberOfPoints-1;
         for(; MaxVert >= 0; MaxVert--, CurrentVert++)
         {
            AddPolygon.Points[CurrentVert] = CurrentBrush->mTransformedPolygons[CurrentGon].Points[MaxVert];
         }

         // copy over the polygon, SUBbrushes polygons start out as INvalidated.
         CurrentNode->AssignPolygon(&AddPolygon);
         CurrentNode->mValidated = FALSE;
         CurrentNode->mBrushId = CurrentBrush->mBrushId;
         CurrentNode->mPolygonNumber = CurrentGon;
         CurrentNode->mBrushPtr = CurrentBrush;

         // go through all of the brushes
         for(int BrushNumber = 0; BrushNumber < BrushCount; BrushNumber++)
         {
            // don't filter our polygons through ourself
            if(BrushNumber == Count)
               continue;

            if(!(BrushList[BrushNumber]->mFlags & BRUSH_ACTIVE))
               continue;

            if(!BuildWater && BrushList[BrushNumber]->mName[0] == '*')
               continue;

            // Don't deal with volume brushes yet
            if (BrushList[BrushNumber]->volumestate)
               continue;

            // don't filter our polygons through brushes
            // which have differing bounding boxes
            if(!BrushList[BrushNumber]->mBoundingBox.CheckIntersection(BrushList[Count]->mBoundingBox))
               continue;

            // for each brush go through our list of polygons
            // and filter them through appropriately
            CBspNode* CurrentPolygon = CurrentNode;
            CoplanarCount = 0;
            while(CurrentPolygon)
            {
               CoplanarCount++;
               CurrentPolygon = CurrentPolygon->CoplanarChild;
            }
            CurrentPolygon = CurrentNode;
            while(CoplanarCount--)
            {
               if(BrushList[BrushNumber]->mTransactionType == BRUSH_ADD_TO_WORLD)
               {
                  // this is for adds BEFORE us
                  if(BrushNumber < Count)
                  {
                     if(CurrentPolygon->mValidated)
                     {
                        CurrentPolygon->perturbed = 0;
                        //BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepInsideCoplanars(CurrentPolygon);
                        BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepInsideNoCoplanars(CurrentPolygon);
                     } 
                     else 
                     {
                        CurrentPolygon->perturbed = 0;
                        BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepInsideNoCoplanars(CurrentPolygon);
                     }
                  } 
                  else 
                  {
                     // if we are already alive we have to make sure
                     // we don't have stuff INSIDE these brushes.
                     if(CurrentPolygon->mValidated)
                     {
                        CurrentPolygon->perturbed = 0;
                        BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepOutsideNoCoplanars(CurrentPolygon);
                     }
                  }
               } 
               else 
               {
                  if(CurrentPolygon->mValidated)
                  {
                     if (BrushNumber < Count)
                     {
                        // Compare with a (-) brush put in world BEFORE Current
                        CurrentPolygon->perturbed = 0;
                        BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepOutsideCoplanars(CurrentPolygon);
                        //BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepOutsideNoCoplanars(CurrentPolygon);
                     } 
                     else 
                     {
                        // Compare with a (-) brush put in world AFTER Current
                        CurrentPolygon->perturbed = 0;
                        BrushList[BrushNumber]->mBrushBsp.DoCSGFilterKeepOutsideNoCoplanars(CurrentPolygon);
                     }
                  }
               }
               // go on to the next coplanar child
               CurrentPolygon = CurrentPolygon->CoplanarChild;
            }
         }
      }
   }

   // Now go through the Volume Brushes...
   for(Count = 0; Count < BrushCount; Count++)
   {
      // Give some feedback...
      sprintf(buf, "Processing Volume Brushes: %d of %d", Count, BrushCount);
      pFrame->UpdateGeneralStatus(buf);

      // get a current brush
      CurrentBrush = BrushList[Count];

      if (CurrentBrush->volumestate)
      {
         CBspNode* polynode = PolygonList;
         while (polynode)
         {
            CBspNode* tempnode;

            tempnode = polynode;
            while (tempnode)
            {
               //if (((tempnode->mBrushPtr == CurrentBrush) && (tempnode->mBrushPtr->m_ShowVolPolys)) || (tempnode->mBrushPtr != CurrentBrush))
               //{
               if (tempnode->mValidated)
                  CurrentBrush->mBrushBsp.ProcessVolumeBrush(tempnode);
               //}

               tempnode = tempnode->CoplanarChild;
            }

            polynode = polynode->NextNode;
         }
      }
   }

   for(int BNumber = 0; BNumber < BrushCount; BNumber++)
      BrushList[BNumber]->mBrushBsp.ClearBsp();

   if (exporting)
   {
      // Try to paste together coplanar polygons into convex polygons
      Count = 0;
      CBspNode* thenode = PolygonList;
      while (thenode)
      {
         if (thenode->CoplanarChild)
         {
            // Give some feedback...
            sprintf(buf, "Merging Polygons: Polygon #: %d", Count);
            pFrame->UpdateGeneralStatus(buf);
            Count++;

            CoagulateCoplanars(thenode);
         }

         thenode = thenode->NextNode;
      }
   }

   // Give some feedback...
   pFrame->UpdateGeneralStatus("Removing Non-Drawn Polygons");

   // get rid of the polygons which AREN'T validated.
   DeleteNonDrawnPolygons(&PolygonList);

   if (exporting)
   {
      CBspNode* BestNode = PolygonList;
      CBspNode* lastnode;

      TreeTop = BestNode;
      lastnode = BestNode;

      if (BestNode)
         BestNode = BestNode->NextNode;

      Count = 0;
      while(lastnode)
      {
         // Give some feedback...
         sprintf(buf, "Building Polylist: Polygon #: %d", Count);
         pFrame->UpdateGeneralStatus(buf);
         Count++;

         lastnode->FrontChild = BestNode;
         lastnode = BestNode;
         if (BestNode)
            BestNode = BestNode->NextNode;
      }

      // get rid of the list
      delete []BrushList;

      pFrame->UpdateGeneralStatus("Fixing all texture coordinates");
      NormalizeTreePolyTextures(TreeTop);
      ProcessTreeCacheInfo(TreeTop, FALSE);
      //AssignGenericLightMaps(TreeTop, theApp.lightval);

   } 
   else 
   {
      // get the best node to partition with
      CBspNode* BestNode = GetBestNode(PolygonList);

      // get our treetop
      TreeTop = BestNode;

      // make our front/back lists have nothing in them
      CBspNode *FrontPolygonList = NULL, *BackPolygonList = NULL;

      // calculate the new sublevel for the tree
      CalculateSubLevel(TreeTop, PolygonList, &FrontPolygonList,
      &BackPolygonList, Progress);

      // recursively subdivide the tree
      DoSubTree(TreeTop, FrontPolygonList, BackPolygonList, Progress);

      // get rid of our list
      delete []BrushList;

      // Normalize all your polygons...
      NormalizeTreePolyTextures(TreeTop);
      ProcessTreeCacheInfo(TreeTop, TRUE);
      SetLighting(TreeTop);
      //AssignGenericLightMaps(TreeTop, theApp.lightval);
   }
}

ThredPoint CConstructiveBsp::ComputePolyCenter(ThredPolygon* poly)
{
  ThredPoint retval;

  retval.X = retval.Y = retval.Z = 0.0;

  for (int i = 0; i < poly->NumberOfPoints; i++)
  {
    retval.X += poly->Points[i].X;
    retval.Y += poly->Points[i].Y;
    retval.Z += poly->Points[i].Z;
  }

  retval.X = retval.X / poly->NumberOfPoints;
  retval.Y = retval.Y / poly->NumberOfPoints;
  retval.Z = retval.Z / poly->NumberOfPoints;

  return retval;
}

double CConstructiveBsp::ComputeArea(ThredPolygon* poly)
{
  Point3D triangle[3];
  int     vertex;
  double  area = 0.0;

  triangle[0].x = poly->Points[0].X;
  triangle[0].y = poly->Points[0].Y;
  triangle[0].z = poly->Points[0].Z;

  for (vertex = 2; vertex < poly->NumberOfPoints; vertex++)
  {
    triangle[1].x = poly->Points[vertex-1].X;
    triangle[1].y = poly->Points[vertex-1].Y;
    triangle[1].z = poly->Points[vertex-1].Z;
    triangle[2].x = poly->Points[vertex].X;
    triangle[2].y = poly->Points[vertex].Y;
    triangle[2].z = poly->Points[vertex].Z;

    Point3D vect1, vect2, result;

    vect1.x = triangle[1].x - triangle[0].x;
    vect1.y = triangle[1].y - triangle[0].y;
    vect1.z = triangle[1].z - triangle[0].z;

    vect2.x = triangle[2].x - triangle[0].x;
    vect2.y = triangle[2].y - triangle[0].y;
    vect2.z = triangle[2].z - triangle[0].z;

    md_cross(vect1, vect2, &result);

    area += 0.5 * sqrt(md_dot(result, result));
  }

  return (area);
}

void CConstructiveBsp::SelectAPolygon(CPoint pos, CRenderCamera* Camera, TSRenderContext* rc)
{
  // pos represents an x,y pair in screen space, so convert it to world space
  TSCamera* ts_cam = rc->getCamera();
  RectI   screen = ts_cam->getScreenViewport();
  Point3F newPos((pos.x - (screen.len_x()/2)), ((screen.len_y()/2) - pos.y), ts_cam->getNearDist());
  Point4F viewpos(pos.x, pos.y, 0.F, 1.F);
  Point4F screenpos;
  Point3F tloc;

  TMat3F screen_to_camera = ts_cam->getTCS();
  TMat3F  camera_to_world  = ts_cam->getTWC();

  screen_to_camera.inverse();
  camera_to_world.inverse();

  // Calculate the view vector in camera space
  Point3D line_o_sight;

  // Xform screen position to screen space
  ts_cam->transformProject2Inverse(viewpos, &screenpos);
  newPos.x = screenpos.x;
  newPos.y = screenpos.y;
  newPos.z = screenpos.z;

  m_mul (newPos, screen_to_camera, &tloc);
  m_mul (tloc, camera_to_world, &newPos);

  line_o_sight.x = (newPos.x - Camera->mCameraPosition.X); 
  line_o_sight.y = (newPos.y - Camera->mCameraPosition.Z);
  line_o_sight.z = (newPos.z - Camera->mCameraPosition.Y);
  line_o_sight.normalize();


  // See if the vector intersects a polygon's plane from the front-to-back bsp traversal
  found = false;
  found_distance = 0.0;
  //ThredPoint tmppnt(Camera->mCameraPosition.X, Camera->mCameraPosition.Z, Camera->mCameraPosition.Y);
  WalkSolidTreeFrontToBack (TreeTop, &Camera->mCameraPosition, line_o_sight);
}


void CConstructiveBsp::WalkSolidTreeFrontToBack(CBspNode* Node, ThredPoint* pos, Point3D line_o_sight)
{
    CBspNode*   tempnode;

	// we are at a terminating leaf.
	if(!Node)
		return;

    // check which side of the camera we are on
	if(Node->ClassifyPoint(pos) == POLYGON_IN_FRONT)
    {
		WalkSolidTreeFrontToBack(Node->FrontChild, pos, line_o_sight);

        if (!found)
        {
		  FindIntersectingNode(Node, Point3D(pos->X, pos->Z, pos->Y), line_o_sight);

          tempnode = Node->CoplanarChild;
          while ((tempnode) && (!found))
          {
            FindIntersectingNode(tempnode, Point3D(pos->X, pos->Z, pos->Y), line_o_sight);

            tempnode = tempnode->CoplanarChild;
          }
        }

		WalkSolidTreeFrontToBack(Node->BackChild, pos, line_o_sight);
	} else {
		WalkSolidTreeFrontToBack(Node->BackChild, pos, line_o_sight);

        WalkSolidTreeFrontToBack(Node->FrontChild, pos, line_o_sight);
	}

	// tree is not full
	return;
}

// Fire a ray from pos along vector line_o_sight & see who it intersects...
bool CConstructiveBsp::FindIntersectingNode(CBspNode* Node, Point3D pos, Point3D line_o_sight)
{
  Point3D temp;
  Point3D normal;
  double t;

  if(!Node->mValidated)
	return false;

  // Calculate the plane's coefficients...
  Point3D p1(Node->Polygon.Points[0].X,
             Node->Polygon.Points[0].Z,
             Node->Polygon.Points[0].Y);
  Point3D p2(Node->Polygon.Points[1].X,
             Node->Polygon.Points[1].Z,
             Node->Polygon.Points[1].Y);
  Point3D p3(Node->Polygon.Points[2].X,
             Node->Polygon.Points[2].Z,
             Node->Polygon.Points[2].Y);

  double A = p1.y * (p2.z - p3.z) + p2.y * (p3.z - p1.z) + p3.y * (p1.z - p2.z);
  double B = p1.z * (p2.x - p3.x) + p2.z * (p3.x - p1.x) + p3.z * (p1.x - p2.x);
  double C = p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y);
  double D = (-p1.x * ((p2.y * p3.z) - (p3.y * p2.z))) -
            (p2.x * ((p3.y * p1.z) - (p1.y * p3.z))) -
            (p3.x * ((p1.y * p2.z) - (p2.y * p1.z)));


  double denom = ((A * line_o_sight.x) + (B * line_o_sight.y) + (C * line_o_sight.z));

  if (denom != 0.F)
  {
    t = (-D - (A * pos.x) - (B * pos.y) - (C * pos.z)) /
            denom;

    Point3D ipoint((pos.x + (t * line_o_sight.x)),
                    (pos.y + (t * line_o_sight.y)),
                    (pos.z + (t * line_o_sight.z)));


    if (t < 0.F)
      return false;

    bool return_val;
    return_val =  PointInPoly(Node, ipoint);

    if (return_val)
    {
      found_distance = sqrt(((ipoint.x - pos.x) * (ipoint.x - pos.x)) +
                             ((ipoint.y - pos.y) * (ipoint.y - pos.y)) +
                             ((ipoint.z - pos.z) * (ipoint.z - pos.z)));
    }

    return return_val;
  } else {
    return false;
  }
}


bool CConstructiveBsp::PointInPoly(CBspNode* Node, Point3D point)
{
    int     i, next;
    double   angle, val;
    Point3D vect1, vect2;
    Point3D side1, side2;

    angle = 0.0;

    Point3D minbox(-1.0E20f, -1.0E20f, -1.0E20f);
    Point3D maxbox(+1.0E20f, +1.0E20f, +1.0E20f);

    for (i = 0; i < Node->Polygon.NumberOfPoints; i++)
    {
        next = (i+1) % Node->Polygon.NumberOfPoints;
        vect1.x = Node->Polygon.Points[i].X;
        vect1.y = Node->Polygon.Points[i].Z;
        vect1.z = Node->Polygon.Points[i].Y;
        vect2.x = Node->Polygon.Points[next].X;
        vect2.y = Node->Polygon.Points[next].Z;
        vect2.z = Node->Polygon.Points[next].Y;

        // Vectors from vertices to point in question
        side1.x = vect1.x - point.x;
        side1.y = vect1.y - point.y;
        side1.z = vect1.z - point.z;
        side2.x = vect2.x - point.x;
        side2.y = vect2.y - point.y;
        side2.z = vect2.z - point.z;

        // Turn 'em into unit vectors
        side1.normalize();
        side2.normalize();

        // Here's the angle between them vectors
        val = acos(md_dot(side1, side2));

        // Sum my angles...
        angle += val;
    }

    if ((angle > 6.1) && (angle < 6.4))
    {
        found_node = Node;
        found = true;     // DPW
        return true;
    } else {
        return false;
    }
}


bool CConstructiveBsp::APolyIsSelected()
{
  return found;
}


CBspNode* CConstructiveBsp::GetSelectedNode()
{
  return found_node;
}


//----------------------------------------------------------------------------
// Split polygon whose texture coordinates are larger than 256

void CConstructiveBsp::splitX(Poly* poly,PolyList* polyList)
{
	Vector<Point3D> points;
	Point3D iPoint;

	int v1 = poly->vertexList.size() - 1;
	for (int v2 = 0; v2 < poly->vertexList.size(); v2++) {
		Poly::Vertex* p1 = &poly->vertexList[v1];
		Poly::Vertex* p2 = &poly->vertexList[v2];

		if (p1->texture.x < 255.0f && p2->texture.x > 255.0f ||
			p2->texture.x < 255.0f && p1->texture.x > 255.0f) {
			Point3D vec = p2->point;
			vec -= p1->point;
			vec *= (255.0f - p1->texture.x) / (p2->texture.x - p1->texture.x);
			vec += p1->point;
			points.push_back(vec);
		}
		if (p1->texture.x < 255.0f)
			iPoint = p1->point;
		v1 = v2;
	}

	// Build plane and split the poly
	if (points.size() > 1) {
		Point3D vec = points[0];
		vec -= points[1];
		Point3D normal;
		md_cross(vec,poly->plane,&normal);
		TPlaneD plane(points[0],normal);
		if (plane.whichSide(iPoint) != TPlaneD::Inside)
			plane.neg();

		Poly tmp;
		Poly* npoly = new Poly;
		poly->split(plane,&tmp,npoly);

		poly->vertexList = tmp.vertexList;
		poly->textureSize.x = 256;

		npoly->plane = poly->plane;
		npoly->textureOffset = poly->textureOffset;
		npoly->material = poly->material;
		normalizeTexture(npoly);
//        npoly->handle = 0;
		polyList->push_back(npoly);
	}
}
	

//----------------------------------------------------------------------------
// Split polygon whose texture coordinates are larger than 256

void CConstructiveBsp::splitY(Poly* poly,PolyList* polyList)
{
	Vector<Point3D> points;
	Point3D iPoint;

	int v1 = poly->vertexList.size() - 1;
	for (int v2 = 0; v2 < poly->vertexList.size(); v2++) {
		Poly::Vertex* p1 = &poly->vertexList[v1];
		Poly::Vertex* p2 = &poly->vertexList[v2];

		if (p1->texture.y < 255.0f && p2->texture.y > 255.0f ||
			p2->texture.y < 255.0f && p1->texture.y > 255.0f) {
			Point3D vec = p2->point;
			vec -= p1->point;
			vec *= (255.0f - p1->texture.y) / (p2->texture.y - p1->texture.y);
			vec += p1->point;
			points.push_back(vec);
		}
		if (p1->texture.y < 255.0f)
			iPoint = p1->point;
		v1 = v2;
	}

	// Build plane and split the poly
	if (points.size() > 1) {
		Point3D vec = points[0];
		vec -= points[1];
		Point3D normal;
		md_cross(vec,poly->plane,&normal);
		TPlaneD plane(points[0],normal);
		if (plane.whichSide(iPoint) != TPlaneD::Inside)
			plane.neg();

		Poly tmp;
		Poly* npoly = new Poly;
		poly->split(plane,&tmp,npoly);

		poly->vertexList = tmp.vertexList;
		poly->textureSize.y = 256;

		npoly->plane = poly->plane;
		npoly->textureOffset = poly->textureOffset;
		npoly->material = poly->material;
		normalizeTexture(npoly);
//        npoly->handle = 0;
		polyList->push_back(npoly);
	}
}

//----------------------------------------------------------------------------
// Reset the texture coordinates after a split.
//
void CConstructiveBsp::normalizeTexture(Poly* poly)
{
#if 0
	Point2F tMin(+1.0E20f,+1.0E20f);
	Point2F tMax(-1.0E20f,-1.0E20f);

	for (int i = 0; i < poly->vertexList.size(); i++) {
		Point2F& tv = poly->vertexList[i].texture;
		tv.x += poly->textureOffset.x;
		tv.y += poly->textureOffset.y;
		tMin.setMin(tv);
		tMax.setMax(tv);
	}

	tMin.x = floor(tMin.x); tMin.y = floor(tMin.y);
	tMax.x = ceil(tMax.x); tMax.y = ceil(tMax.y);

	// Mask onto power of 2 bitmap size.
	Point2I offset;
	offset.x = int(tMin.x) & MaxTextureMask;
	offset.y = int(tMin.y) & MaxTextureMask;

	// Adjust offset to multiple of 8 to avoid
	// mipmap jumping.
	poly->textureOffset.x = offset.x & ~0x7;
	poly->textureOffset.y = offset.y & ~0x7;
	tMin.x -= double(offset.x & 0x7);
	tMin.y -= double(offset.y & 0x7);

	//
	poly->textureSize.x = int(tMax.x - tMin.x);
	poly->textureSize.y = int(tMax.y - tMin.y);
	for (int v = 0; v < poly->vertexList.size(); v++)
		poly->vertexList[v].texture -= tMin;
#endif
//#if 0
	Point2F tMin(+1.0E20f,+1.0E20f);
	Point2F tMax(-1.0E20f,-1.0E20f);

	for (int i = 0; i < poly->vertexList.size(); i++) {
		Point2F& tv = poly->vertexList[i].texture;
		tMin.setMin(tv);
		tMax.setMax(tv);
	}

	tMin.x = floor(tMin.x); tMin.y = floor(tMin.y);
	tMax.x = ceil(tMax.x); tMax.y = ceil(tMax.y);
	poly->textureSize.x = int(tMax.x - tMin.x);
	poly->textureSize.y = int(tMax.y - tMin.y);

	poly->textureOffset.x = (poly->textureOffset.x + 
		int(tMin.x)) & MaxTextureMask;
	poly->textureOffset.y = (poly->textureOffset.y + 
		int(tMin.y)) & MaxTextureMask;

	for (int v = 0; v < poly->vertexList.size(); v++)
		poly->vertexList[v].texture -= tMin;
//#endif
}


double CConstructiveBsp::maxval(double val1, double val2)
{
  if (val1 > val2)
    return val1;
  else
    return val2;
}

double CConstructiveBsp::minval(double val1, double val2)
{
  if (val1 < val2)
    return val1;
  else
    return val2;
}

double CConstructiveBsp::FindAngle(Point3D vect1, Point3D vect2)
{
  // Just to be sure
  vect1.normalize();
  vect2.normalize();

  return acos(md_dot(vect1, vect2));
}

//  ----------------------------------------------------------------------
// Returns the following:
//
//  0:  Point is not colinear
//  1:  Point is colinear & lies between the start & end
//  2:  Point is colinear & lies before the start of the line segment
//  3:  Point is colinear & lies ON the start of the line segment
//  4:  Point is colinear & lies after the end of the line segment
//  5:  Point is colinear & lies ON the end of the line segment
//  ----------------------------------------------------------------------
int CConstructiveBsp::isPointColinear(Point3D start, Point3D end, Point3D point)
{
  Point3D vect1, vect2;

  // Try a quick return
  if ((point.x == start.x) && (point.y == start.y) && (point.z == start.z))
  {
    // Point is the same as the start point of the line segment
    return 3;
  }

  // Try another quick return
  if ((point.x == end.x) && (point.y == end.y) && (point.z == end.z))
  {
    // Point is the same as the end point of the line segment
    return 5;
  }

  vect1.x = (end.x - start.x);
  vect1.y = (end.y - start.y);
  vect1.z = (end.z - start.z);
  vect1.normalize();

  vect2.x = (point.x - start.x);
  vect2.y = (point.y - start.y);
  vect2.z = (point.z - start.z);
  vect2.normalize();

  double dotprod = md_dot(vect1, vect2);

  if (fabs(dotprod) > .999)
  {
    // Where the heck does the point fall wrt the start & end points
    if (dotprod < 0.0)
    {
      // Before the start of the line segment
      return 2;
    }

    vect1.x = (start.x - end.x);
    vect1.y = (start.y - end.y);
    vect1.z = (start.z - end.z);
    vect1.normalize();

    vect2.x = (point.x - end.x);
    vect2.y = (point.y - end.y);
    vect2.z = (point.z - end.z);
    vect2.normalize();

    dotprod = md_dot(vect1, vect2);

    if (dotprod < 0.0)
    {
      // After the end of the line segment
      return 4;
    }

    return 1;

  } else {
    // point isn't colinear with the line segment
    return 0;
  }
}


void CConstructiveBsp::CoagulateCoplanars(CBspNode* node)
{
  CTransformMatrix  rotmat;
  ThredPoint        normal;
  Point3D           angle;
  CBspNode*         cnode;
  CBspNode*         tnode;
  BOOL              status;

  if (!node)
  {
    return;
  }

  normal.X = node->ConstantA;
  normal.Y = node->ConstantB;
  normal.Z = node->ConstantC;
  normal.Normalize();

  angle.x = angle.y = angle.z = 0.0;

  if (normal.X == 0)
  {
    if (normal.Z > 0)
    {
      angle.y = M_PI_VALUE;
    } else {
      angle.y = 0.0;
    }
  } else {
    if (normal.X < 0)
    {
      angle.y = ((M_PI_VALUE / 2.0) - atan(normal.Z / normal.X));
    } else {
      angle.y = -((M_PI_VALUE / 2.0) + atan(normal.Z / normal.X));
    }
  }

  angle.x = asin(normal.Y);

  rotmat.ClearMatrix();
  rotmat.CreateRotationMatrix((angle.y*(double)(M_TOTAL_DEGREES/2)/M_PI_VALUE), (angle.x*(double)(M_TOTAL_DEGREES/2)/M_PI_VALUE), (angle.z*(double)(M_TOTAL_DEGREES/2)/M_PI_VALUE));

  tnode = node;
  while (tnode)
  {
    if (tnode->mValidated)
    {
      cnode = node;
      while (cnode)
      {
        // Can't merge two of the same!!!!
        if (cnode != tnode)
        {
          // Gotta be real and visible
          if (cnode->mValidated)
          {
            // Gotta have the same volume states
            if (tnode->Polygon.volumestate == cnode->Polygon.volumestate)
            {
              // Attempt to merge these polygons together
              status = CombinePolygons(tnode, cnode, &rotmat);
            }
          }
        }

        cnode = cnode->CoplanarChild;
      }
    }

    tnode = tnode->CoplanarChild;
  }
}

// Try to combine the polygons in 2 input nodes
// If a valid polygon is created the result is placed in node1 and node2 is invalidated
BOOL CConstructiveBsp::CombinePolygons(CBspNode* node1, CBspNode* node2, CTransformMatrix* rotmat)
{
  CArray<Point3D, Point3D&> vertlist;
  double        min =  9999999.9;
  double        max = -9999999.9;
  coagscanline  scanlines;
  CBspNode*     tempnode;
  double        oldarea, newarea;
  ThredPoint    point1, point2;
  ThredPoint    temp1, temp2;
  int           i, j;

  if ((!node1) || (!node2))
  {
    return FALSE;
  }

  oldarea = 0.0;
  scanlines.clearlines();

  // Find the min and max y values for these polygons
  for (j = 0; j < 2; j++)
  {
    if (j)
    {
      tempnode = node2;
    } else {
      tempnode = node1;
    }

    oldarea += ComputeArea(&tempnode->Polygon);

    for (i = 0; i < tempnode->Polygon.NumberOfPoints; i++)
    {
      point1.X = tempnode->Polygon.Points[i].X;
      point1.Y = tempnode->Polygon.Points[i].Y;
      point1.Z = tempnode->Polygon.Points[i].Z;

      rotmat->ApplyMatrix(point1);

      if (point1.Y < min)
      {
        min = point1.Y;
      }

      if (point1.Y > max)
      {
        max = point1.Y;
      }
    }
  }

  // Rasterize the polygons into the scanlines
  for (j = 0; j < 2; j++)
  {
    if (j)
    {
      tempnode = node2;
    } else {
      tempnode = node1;
    }

    for (i = 0; i < tempnode->Polygon.NumberOfPoints; i++)
    {
      int nextvert = (i + 1) % tempnode->Polygon.NumberOfPoints;

      temp1.X = point1.X = tempnode->Polygon.Points[i].X;
      temp1.Y = point1.Y = tempnode->Polygon.Points[i].Y;
      temp1.Z = point1.Z = tempnode->Polygon.Points[i].Z;

      temp2.X = point2.X = tempnode->Polygon.Points[nextvert].X;
      temp2.Y = point2.Y = tempnode->Polygon.Points[nextvert].Y;
      temp2.Z = point2.Z = tempnode->Polygon.Points[nextvert].Z;

      rotmat->ApplyMatrix(point1);
      rotmat->ApplyMatrix(point2);

      point1.Y -= min;
      point2.Y -= min;

      if (fabs(point1.Y - point2.Y) < 1.0)
      {
        // Horizontal edge from point1 to point2
        scanlines.addpoint(point1.Y, point1.X, &temp1);
        scanlines.addpoint(point2.Y, point2.X, &temp2);
      } else {
        if (point1.Y > point2.Y)
        {
          ThredPoint  temp;

          // Swap point1 and point2
          temp.X = point1.X; temp.Y = point1.Y; temp.Z = point1.Z;
          point1.X = point2.X; point1.Y = point2.Y; point1.Z - point2.Z;
          point2.X = temp.X; point2.Y = temp.Y; point2.Z = temp.Z;

          temp.X = temp1.X; temp.Y = temp1.Y; temp.Z = temp1.Z;
          temp1.X = temp2.X; temp1.Y = temp2.Y; temp1.Z = temp2.Z;
          temp2.X = temp.X; temp2.Y = temp.Y; temp2.Z = temp.Z;
        }

        double  dx, currx;

        dx = (point2.X - point1.X) / (point2.Y - point1.Y);
        currx = point1.X;

        scanlines.addpoint(int(point1.Y), point1.X, &temp1);

        for (int sline = int(point1.Y + 1.0); sline < int(point2.Y); sline++)
        {
          if (sline < 0)
          {
            int aaa;
            aaa = sline;
          } else {
            scanlines.addpoint(sline, currx);
            currx += dx;
          }
        }

        scanlines.addpoint(int(point2.Y), point2.X, &temp2);
      }
    }
  }

  // Now find the vertices in a counter-clockwise fashion
  ThredPolygon  poly;
  int           vertcount = 0;

  for (i = 0; i < MAX_COAGLINES; i++)
  {
    if (scanlines.line[i].isvertex_s)
    {
      // Found a vertex
      vertlist.Add(scanlines.line[i].vertex_s);
    }
  }

  for (i = (MAX_COAGLINES-1); i >= 0; i--)
  {
    if (scanlines.line[i].isvertex_e)
    {
      // Found a vertex
      vertlist.Add(scanlines.line[i].vertex_e);
    }
  }

  // We might have colinear vertices now along edges, so we must remove them
  // (If we don't then the isConvex function will fail)
  vertcount = vertlist.GetSize();
  for (i = 0; i < vertcount; i++)
  {
    Point3D p1, p2, p3;
    int     v1, v2, v3;

    v1 = i;
    v2 = (i+1) % vertcount;
    v3 = (i+2) % vertcount;

    p1.x = vertlist[v1].x; p1.y = vertlist[v1].y; p1.z = vertlist[v1].z;
    p2.x = vertlist[v2].x; p2.y = vertlist[v2].y; p2.z = vertlist[v2].z;
    p3.x = vertlist[v3].x; p3.y = vertlist[v3].y; p3.z = vertlist[v3].z;

    if (isPointColinear(p1, p2, p3))
    {
      vertlist.RemoveAt(v2);
      vertcount--;
      i--;
    }
  }

  // Better check to be sure
  if (vertcount != vertlist.GetSize())
  {
    return FALSE;
  }

  // Build the new polygon
  if ((vertcount > 2) && (vertcount <= MAX_POLYGON_VERTS))
  {
    poly.NumberOfPoints = vertcount;
    poly.Colour = node1->Polygon.Colour;
    poly.material_index = node1->Polygon.material_index;
    poly.mTextureScaleShift = node1->Polygon.mTextureScaleShift;
    poly.mApplyAmbient = node1->Polygon.mApplyAmbient;

    for (i = 0; i < vertcount; i++)
    {
      poly.Points[i].X = vertlist[i].x;
      poly.Points[i].Y = vertlist[i].y;
      poly.Points[i].Z = vertlist[i].z;
    }
  } else {
    return FALSE;
  }

  // Check areas, vertex count, & convexity
  newarea = ComputeArea(&poly);
  if ((newarea != 0) && (oldarea != 0))
  {
    double  testval = fabs(newarea / oldarea);
    if ((testval < 1.0000001) && (testval > 0.999999))
    {
      if (vertcount > 2)
      {
        if (isConvex(&poly))
        {
          node1->Polygon = poly;
          node2->mValidated = FALSE;

          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

BOOL CConstructiveBsp::isEdgeShared(Point3D srcstart, Point3D srcend, Point3D deststart, Point3D destend)
{
  Point3D srcvect;
  Point3D destvect;
  double  dotprod;

  // This is the original edge in vector form
  srcvect.x = (srcend.x - srcstart.x);
  srcvect.y = (srcend.y - srcstart.y);
  srcvect.z = (srcend.z - srcstart.z);
  srcvect.normalize();

  destvect.x = (deststart.x - srcstart.x);
  destvect.y = (deststart.y - srcstart.y);
  destvect.z = (deststart.z - srcstart.z);
  destvect.normalize();

  dotprod = md_dot(srcvect, destvect);

  if (fabs(dotprod) < 0.9999)
  {
    return FALSE;
  }

  destvect.x = (destend.x - srcstart.x);
  destvect.y = (destend.y - srcstart.y);
  destvect.z = (destend.z - srcstart.z);
  destvect.normalize();

  dotprod = md_dot(srcvect, destvect);

  if (fabs(dotprod) < 0.9999)
  {
    return FALSE;
  }

  return TRUE;
}

void CConstructiveBsp::ReverseVertices(ThredPolygon* poly)
{
  ThredPolygon temp;
  int           num_polys = poly->NumberOfPoints;
  int           i;

  for (i = 0; i < poly->NumberOfPoints; i++)
  {
    temp.Points[num_polys - 1 - i].X = poly->Points[i].X;
    temp.Points[num_polys - 1 - i].Y = poly->Points[i].Y;
    temp.Points[num_polys - 1 - i].Z = poly->Points[i].Z;
  }

  for (i = 0; i < poly->NumberOfPoints; i++)
  {
    poly->Points[i].X = temp.Points[i].X;
    poly->Points[i].Y = temp.Points[i].Y;
    poly->Points[i].Z = temp.Points[i].Z;
  }
}

BOOL CConstructiveBsp::isConvex(ThredPolygon* poly)
{
  int     vert1, vert2, vert3;
  Point3D vect1, vect2;
  Point3D normal, base;
  double  dotprod;

  for (vert1 = 0; vert1 < poly->NumberOfPoints; vert1++)
  {
    vert2 = (vert1 + 1) % poly->NumberOfPoints;
    vert3 = (vert1 + 2) % poly->NumberOfPoints;

    // Make the vector from vert1 to vert2
    vect1.x = poly->Points[vert2].X - poly->Points[vert1].X;
    vect1.y = poly->Points[vert2].Y - poly->Points[vert1].Y;
    vect1.z = poly->Points[vert2].Z - poly->Points[vert1].Z;
    vect1.normalize();

    // Make the vector from vert2 to vert3
    vect2.x = poly->Points[vert3].X - poly->Points[vert2].X;
    vect2.y = poly->Points[vert3].Y - poly->Points[vert2].Y;
    vect2.z = poly->Points[vert3].Z - poly->Points[vert2].Z;
    vect2.normalize();

    // Now compute their cross product
    md_cross(vect1, vect2, &normal);

    // If not the 1st time, compute the dot product between the 1st normal & this one
    if (vert1 == 0)
    {
      base.x = normal.x;
      base.y = normal.y;
      base.z = normal.z;
    } else {
      dotprod = md_dot(base, normal);

      // If dot product is < 0 then we've found a concave poly so return false & get outta here
      if (dotprod < 0)
      {
        return FALSE;
      }
    }
  }

  return TRUE;
}

void CConstructiveBsp::ProcessVolumeBrush(CBspNode* Polygon)
{
	// do our sub-filter
	ProcessVolumeBrush(TreeTop, Polygon);
}

// A volume brush should not be hollow, otherwise this algorithm must be
// changed to handle both inward and outward facing polygons from 1 brush
// Part of the solution has be if 0'd out in the below code, but should be
// tested to be sure it works completely.
void CConstructiveBsp::ProcessVolumeBrush(CBspNode* Node, CBspNode* Polygon)
{
  if(!Node)
  {
	  return;
  }

  int result = Node->ClassifyPolygon(&Polygon->Polygon);
  // check where to go
  switch(result)
  {
    // If the polygon is in front we go down that way
    // if we can.  Otherwise we make a new node and stick
    // the polygon there.
    case POLYGON_IN_FRONT:
	  if(Node->FrontChild)
      {
		ProcessVolumeBrush(Node->FrontChild, Polygon);
	  }
//#if 0
      else {
        if (!Node->BackChild)
        {
          CThredBrush*  brush = Node->mBrushPtr;

          // Only perform this for hollow brushes....
          if ( ((brush->BrushType == CThredBrush::BOX_BRUSH) && (brush->BrushSpecifics.box.m_Solid != 0)) ||
               ((brush->BrushType == CThredBrush::TRI_BRUSH) && (brush->BrushSpecifics.triangle.m_Solid != 0)) ||
               ((brush->BrushType == CThredBrush::CYL_BRUSH) && (brush->BrushSpecifics.cylinder.m_Solid != 0)) ||
               ((brush->BrushType == CThredBrush::SPHERE_BRUSH) && (brush->BrushSpecifics.sphere.m_Solid != 0)) )
          {
            Polygon->Polygon.volumestate |= Node->mBrushPtr->volumestate;
          }
        }
      }
//#endif
	  break;

    case POLYGON_COPLANAR_SAME:
    case POLYGON_COPLANAR_REVERSE:
    case POLYGON_IN_BACK:
	  if(Node->BackChild)
      {
		ProcessVolumeBrush(Node->BackChild, Polygon);
	  } else {
        // Mark the polygon with the volume state
        Polygon->Polygon.volumestate |= Node->mBrushPtr->volumestate;
	  }
	  break;

    // If the polygon is split by the node then we
    // send it down both sides of the tree as new
    // polygons.
    case POLYGON_SPLIT:
	  {
		ThredPolygon  UsagePolygon, SavePolygon;
		CBspNode*     NewNode;

        SavePolygon = Polygon->Polygon;

		// split the gon
		Node->SplitPolygon(&Polygon->Polygon, &UsagePolygon);

        BOOL  foundmatch = FALSE;
        for (int j = 0; j < UsagePolygon.NumberOfPoints; j++)
        {
          for (int l = 0; l < UsagePolygon.NumberOfPoints; l++)
          {
            if (l != j)
            {
              if ((UsagePolygon.Points[j].X == UsagePolygon.Points[l].X) &&
                  (UsagePolygon.Points[j].Y == UsagePolygon.Points[l].Y) &&
                  (UsagePolygon.Points[j].Z == UsagePolygon.Points[l].Z))
              {
                foundmatch = TRUE;
              }
            }
          }
        }

        if (foundmatch)
        {
          Polygon->AssignPolygon(&SavePolygon);
          return;
        }

		// make a new node and filter it through the tree
		NewNode = new CBspNode;
		NewNode->AssignPolygon(&UsagePolygon);
		NewNode->mValidated = TRUE;
		NewNode->mBrushId = Polygon->mBrushId;
		NewNode->mPolygonNumber = Polygon->mPolygonNumber;
        NewNode->mBrushPtr = Polygon->mBrushPtr;

        // Preserve a polygon's volume if it was already marked
        if (Polygon->Polygon.volumestate)
        {
          NewNode->Polygon.volumestate |= Node->mBrushPtr->volumestate;
        }

		// check for something in front
		if(Node->FrontChild)
        {
		  ProcessVolumeBrush(Node->FrontChild, Polygon);
		}

		// check for something in back
		if(Node->BackChild)
        {
		  ProcessVolumeBrush(Node->BackChild, NewNode);
		} else {
          // Mark the new polygon with the volume state
          NewNode->Polygon.volumestate |= Node->mBrushPtr->volumestate;
		}

        CBspNode* tnode;

        tnode = Polygon;
        while(tnode->CoplanarChild)
        {
		  tnode = tnode->CoplanarChild;
        }

	    tnode->CoplanarChild = NewNode;
	  }
	  break;
  }
}
