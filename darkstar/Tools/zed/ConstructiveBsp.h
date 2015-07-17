// ConstructiveBsp.h : header file
//

#ifndef _CONSTRUCTIVEBSP_H_
#define _CONSTRUCTIVEBSP_H_

#include "ThredPrimitives.h"
#include "RenderCamera.h"
#include "bspnode.h"
#include "thredparser.h"
#include "ts_material.h"
#include "ts_RenderContext.h"
#include "tvector.h"
#include "ts_vertex.h"

class CThredBrush;
class CTHREDDoc;

#define MAX_POLYS       3000
#define MAX_POLY_SPANS  2000
#define MAX_SLINES      200
#define MAX_COAGLINES   10000

//--------------------------------------------------------------------------
// double point stuff - here because the following are barely implemented
//--------------------------------------------------------------------------
class Point3D
{
 public:
   double x, y, z;
   Point3D( double _x, double _y, double _z ) { x = _x; y = _y; z = _z; };
   Point3D( float _x, float _y, float _z ) { x = ( double )_x; y = ( double )_y; z = ( double )_z; };
   Point3D() { x = 0.0; y = 0.0; z = 0.0; };
   Point3D& normalize();
   Point3D& set( const double _x, const double _y, const double _z );
   
	Point3D& operator +=( const double c );
	Point3D& operator +=( const Point3D& c );
	Point3D& operator -=( const double c );
	Point3D& operator -=( const Point3D& c );
	Point3D& operator *=( const double c );
	Point3D& operator *=( const Point3D& c );
	Point3D& operator /=( const double c );
	Point3D& operator /=( const Point3D& c );
	double len( void );
   Point3D& neg( void );
};

inline Point3D& Point3D::neg( void )
{
	x = -x;
	y = -y;
   z = -z;
	return *this;
}

inline Point3D& Point3D::set( const double _x, const double _y, const double _z )
{  
   x = _x;
   y = _y;
   z = _z;
   return *this;
}

inline Point3D& Point3D::operator +=( const double c )
{ 
	x += c;
	y += c;
	z += c;
   return *this;
}

inline Point3D& Point3D::operator +=( const Point3D& c )
{ 
	x += c.x; 
	y += c.y; 
	z += c.z; 
   return *this;
}

inline Point3D& Point3D::operator -=( const double c )
{ 
	x -= c;
	y -= c;
	z -= c;
   return *this;
}

inline Point3D& Point3D::operator -=( const Point3D& c )
{ 
	x -= c.x; 
	y -= c.y; 
	z -= c.z; 
   return *this;
}

inline Point3D& Point3D::operator *=( const double c )
{ 
	x *= c; 
	y *= c; 
	z *= c; 
   return *this;
}

inline Point3D& Point3D::operator *=( const Point3D& c )		
{ 
	x *= c.x; 
	y *= c.y; 
	z *= c.z; 
	return *this; 
}

inline Point3D& Point3D::operator /=( const double c )
{ 
	x /= c;
	y /= c;
	z /= c;
   return *this;
}

inline Point3D& Point3D::operator /=( const Point3D& c )		
{ 
	x /= c.x; 
	y /= c.y; 
	z /= c.z; 
	return *this; 
}

inline double Point3D::len( void )
{
   return( sqrt( ( x * x ) + ( y * y ) + ( z * z ) ) );
}

// Double precision plane
// -------------------------------------------------
struct TPlaneD : public Point3D
{
   public:
      enum Side {
         OnPlane = 0,
         Inside = 1,
         Outside = -1,
         Intersect = 2,
         Undefined = 3 };
      
      double d;
      
      TPlaneD() {}
      TPlaneD( const Point3D& p, const Point3D& n );
      TPlaneD( double _x, double _y, double _z, double _d ) 
         { x = x; y = y; z = z; d = d; };
         
      // methods
      void neg();
      void set( const Point3D& k, const Point3D& j, const Point3D& l );
      void set( const Point3D& p, const Point3D& n );
      bool set( int count, Point3D ** vl );
      Side whichSide( const Point3D& cp ) const;
      bool intersect( const Point3D& start, const Point3D& end, double * value ) const;
      void normalize();
};

inline void TPlaneD::normalize()
{
	// Only normalizes the vector component
	double s = 1.0f / sqrt( ( x * x ) + ( y * y ) + ( z * z ) );
	x *= s;
	y *= s;
	z *= s;
}

inline void TPlaneD::set( const Point3D& p, const Point3D& n )
{
   x = n.x; y = n.y; z = n.z;
   normalize();

	// Calculate the last plane coefficient.
	d = -(p.x * x + p.y * y + p.z * z);
}

//  Double precision Vertex
// -------------------------------------------------

struct TDPolyVertex
{
   enum Flag { ClippedEdge = 0x1, };
   int flags;
   Point3D point;
   TDPolyVertex();
   void set( TDPolyVertex& start, TDPolyVertex& end, double scale )
   {
		point.x = start.point.x + (end.point.x - start.point.x) * scale;
		point.y = start.point.y + (end.point.y - start.point.y) * scale;
		point.z = start.point.z + (end.point.z - start.point.z) * scale;
   }
};

inline TDPolyVertex::TDPolyVertex()
{
   flags = 0;
}

//------------------------------------------

typedef struct
{
    Point3D startws;
    Point3D endws;
    int     startlm;
    int     endlm;
} slinetype;

class lmscanline
{
public:
  lmscanline();
  
  void clearlines();
  void addpoint(int scanline, Point3D wspoint, int lmxpoint);

  slinetype line[MAX_SLINES];
};

typedef struct
{
  double  startx;
  double  endx;

  BOOL    isvertex_s;
  BOOL    isvertex_e;
  Point3D vertex_s;
  Point3D vertex_e;
} coagsline;

class coagscanline
{
public:
  coagscanline();

  void clearlines();
  void addpoint(int scanline, double xval, ThredPoint* vertex = NULL);

  coagsline line[MAX_COAGLINES];
};

enum { TEXTURE_NONE, TEXTURE_AFFINE, TEXTURE_PERSPECTIVE };

struct PolyVertex: public TDPolyVertex {
		Point2F texture;
		void set(PolyVertex& start,PolyVertex& end,float scale);
};

// -----------------------------
// double poly stuff
//---------------------------------------------------------------------------
// The vertex class V for the tpoly template should be derived
// from struct TPolyVertex.
//
template <class V>
class TDPoly
{
public:
	typedef TPlaneD Plane;
	typedef V Vertex;
	typedef Vector<V> VertexList;
	//
	TDPoly();
	~TDPoly();
	//
	VertexList vertexList;
	//
	Plane::Side whichSide(Plane&);
	bool split(Plane& plane,TDPoly* front,TDPoly* back);
	bool clip(Plane& plane,TDPoly* front,bool onPlane = true);
	bool clip(Plane& plane,bool onPlane = true);
	bool anyClippedEdges();
};

//---------------------------------------------------------------------------

template <class V>
TDPoly<V>::TDPoly()
{
}	

template <class V>
TDPoly<V>::~TDPoly()
{
}

//---------------------------------------------------------------------------

template <class V>
bool TDPoly<V>::clip(Plane& plane,bool onPlane)
{
	TDPoly tmp;
	clip(plane,&tmp,onPlane);
	*this = tmp;
	return tmp.vertexList.size() != 0;
}


//---------------------------------------------------------------------------

template <class V>
TDPoly<V>::Plane::Side TDPoly<V>::whichSide(Plane& plane)
{
	if (!vertexList.size())
		return Plane::Undefined;

	// Find first point not on the plane
	VertexList::iterator end = vertexList.end();
	VertexList::iterator itr = vertexList.begin();
	Plane::Side side = Plane::OnPlane;
	for (; itr != end; itr++)
		if ((side = plane.whichSide((*itr).point)) != Plane::OnPlane)
			break;

	// Make sure the rest are on the same side
	for (; itr != end; itr++) {
		Plane::Side nside = plane.whichSide((*itr).point);
		if (nside != side && nside != Plane::OnPlane)
			// Crosses the plane
			return Plane::Intersect;
	}
	return side;
}

//---------------------------------------------------------------------------

template <class V>
bool TDPoly<V>::anyClippedEdges()
{
	for (VertexList::iterator itr = vertexList.begin();
			itr != vertexList.end(); itr++)
		if ((*itr).flags & TDPolyVertex::ClippedEdge)
			return true;
	return false;
}


//---------------------------------------------------------------------------
// Split the poly along the given plane.
// Returns true if the poly was actually split, if it wasn't the poly
// will copied into either in the front or back side.
//
template <class V>
bool TDPoly<V>::split(Plane& plane,TDPoly* front,TDPoly* back)
{
	// Make sure the given poly is not degenerate.
	if (front)
		front->vertexList.clear();
	if (back)
	 	back->vertexList.clear();
	if (vertexList.size() < 3)
		return false;

	// Assume both back & front are degenerate polygons.
	bool back_degen = true,front_degen = true;

	// Perform the splitting.  The polygon is closed by starting with
	// the last point.
	int start = vertexList.size() - 1;
	Plane::Side psign = plane.whichSide(vertexList[start].point);

	V vertex;
	double time;
	for (int end = 0; end < vertexList.size(); end++)
	{
		// This assumes that whichSide() continues to return
		// -1 for outside, 0 on the plane and 1 for inside.
		Plane::Side csign = plane.whichSide(vertexList[end].point);

		switch(psign * 3 + csign) {
			case -4:		// P-, C-
			case -1:		// P., C-
				if (back)
					back->vertexList.push_back(vertexList[end]);
				back_degen = false;
				break;
			case 4:		// P+, C+
			case 1:		// P., C+
				if (front)
					front->vertexList.push_back(vertexList[end]);
				front_degen = false;
				break;
			case -3:		// P-, C.
			case 0:		// P., C.
			case 3:		// P+, C.
				if (back)
					back->vertexList.push_back(vertexList[end]);
				if (front)
					front->vertexList.push_back(vertexList[end]);
				break;
			case -2:		// P-, C+
				if (plane.intersect(vertexList[start].point,
						vertexList[end].point,&time)) {
					vertex.set(vertexList[start],vertexList[end],time);
					if (back)
						back->vertexList.push_back(vertex);
					if (front) {
						front->vertexList.push_back(vertex);
						front->vertexList.push_back(vertexList[end]);
					}
					front_degen = false;
				}
				break;

			case 2:		// P+, C-
				if (plane.intersect(vertexList[start].point,
						vertexList[end].point,&time)) {
					vertex.set(vertexList[start],vertexList[end],time);
					if (front)
						front->vertexList.push_back(vertex);
					if (back) {
						back->vertexList.push_back(vertex);
						back->vertexList.push_back(vertexList[end]);
					}
					back_degen = false;
				}
				break;
		}

		psign = csign;
		start = end;
	}

	// Check for degeneracy.
	// This check should be a little more complete.
	if (front && front->vertexList.size() < 3)
		front_degen = true;
	if (back && back->vertexList.size() < 3)
		back_degen = true;

	if (front_degen && back_degen) {
		// If both front and back are degenerate then I assume that
		// it is on the plane and default to the front half space.
		if (back)
			back->vertexList.clear();
		if (front) {
			front->vertexList.setSize(vertexList.size());
			for (int i = 0; i < vertexList.size(); i++)
				front->vertexList[i] = vertexList[i];
		}
	}
	else {
		if (front && front_degen)
			front->vertexList.clear();
		if (back && back_degen)
			back->vertexList.clear();
	}


	if (front && back)
		return front->vertexList.size() && back->vertexList.size();
	else
		if (front)
			return front->vertexList.size() != 0;
		else
			if (back)
				return back->vertexList.size() != 0;
	return false;
}


//---------------------------------------------------------------------------
// Clip the poly along the given plane.
// Only keeps the inside portion.  Returns true if anything was
// put in the output poly.  Pass onPlane as true to consider
// polys on the plane to be considered inside.
//
template <class V>
bool TDPoly<V>::clip(Plane& plane,TDPoly* front,bool onPlane)
{
	// Make sure the given poly is not degenerate.
	front->vertexList.clear();
	if (vertexList.size() < 3)
		return false;

	// Assume both back & front are degenerate polygons.
	bool back_degen = true,front_degen = true;

	// Perform the splitting.  The polygon is closed by starting with
	// the last point.
	int start = vertexList.size() - 1;
	Plane::Side psign = plane.whichSide(vertexList[start].point);

	V vertex;
	double time;
	for (int end = 0; end < vertexList.size(); end++)
	{
		// This assumes that whichSide() continues to return
		// -1 for outside, 0 on the plane and 1 for inside.
		Plane::Side csign = plane.whichSide(vertexList[end].point);

		switch(psign * 3 + csign) {
			case -4:		// P-, C-
			case -1:		// P., C-
				back_degen = false;
				break;
			case 4:		// P+, C+
			case 1:		// P., C+
				front->vertexList.push_back(vertexList[end]);
				front_degen = false;
				break;
			case 0:		// P., C.
				front->vertexList.push_back(vertexList[end]);
				front->vertexList.last().flags |= TDPolyVertex::ClippedEdge;
				break;
			case -3:		// P-, C.
			case 3:		// P+, C.
				front->vertexList.push_back(vertexList[end]);
				break;
			case -2:		// P-, C+
				if (plane.intersect(vertexList[start].point,
						vertexList[end].point,&time)) {
					vertex.set(vertexList[start],vertexList[end],time);
					front->vertexList.push_back(vertex);
					front->vertexList.last().flags |= TDPolyVertex::ClippedEdge;
					front->vertexList.push_back(vertexList[end]);
					front_degen = false;
				}
				break;

			case 2:		// P+, C-
				if (plane.intersect(vertexList[start].point,
						vertexList[end].point,&time)) {
					vertex.set(vertexList[start],vertexList[end],time);
					front->vertexList.push_back(vertex);
					back_degen = false;
				}
				break;
		}

		psign = csign;
		start = end;
	}

	// Check for degeneracy.
	// This check should be a little more complete.
	if (front->vertexList.size() < 3)
		front_degen = true;

	if (front_degen)
		if (back_degen && onPlane) {
			// If both front and back are degenerate then I assume that
			// it is on the plane.
			front->vertexList.setSize(vertexList.size());
			for (int i = 0; i < vertexList.size(); i++)
				front->vertexList[i] = vertexList[i];
		}
		else
			front->vertexList.clear();

	return front->vertexList.size();
}
// ----------------------------------------
// end double poly stuff

class Poly: public TDPoly<PolyVertex>
{
	public:
		typedef TDPoly<PolyVertex>::VertexList VertexList;
		Point2I textureSize;
		Point2I textureOffset;
		//BoxMapping boxMapping;
		TPlaneD plane;
		int material;
        GFXTextureHandle handle;
		//
		void rotate();
		bool isPointPtr(Point3D* ptr);
};
typedef Vector<Poly*> PolyList;


class ThredEdge
{
public:
  ThredEdge() { valid = 0; };

  Point3D vert[2];
  int     valid;
};

// the actual holder for the tree.
class CConstructiveBsp : public CObject
{
// Construction
public:
    void    ProcessVolumeBrush(CBspNode* Polygon);
    void    ProcessVolumeBrush(CBspNode* Node, CBspNode* Polygon);
    static  void FlushCacheInfo();
    BOOL    CombinePolygons(CBspNode* node1, CBspNode* node2, CTransformMatrix* rotmat);
    double  FindAngle(Point3D vect1, Point3D vect2);
    int     isPointColinear(Point3D start, Point3D end, Point3D point);
    GFXLightMap* CopyLightMap(GFXLightMap* src);
    BOOL isEdgeShared(Point3D srcstart, Point3D srcend, Point3D deststart, Point3D destend);
    ThredPoint ComputePolyCenter(ThredPolygon* poly);
    void    CalculateLightMaps(CBspNode* Node, ThredPoint* lightpos, double lightdist1, double lightdist2, double maxintensity);
    void    AssignLightMap(CBspNode* Node, int inval);
    void    AssignGenericLightMaps(CBspNode* Node, int inval);
    BOOL    isConvex(ThredPolygon* poly);
	void    DoCSGFilterKeepInsideCoplanars(CBspNode* Polygon);
    void    DoCSGFilterKeepInsideCoplanars(CBspNode* Node, CBspNode* Polygon, int& NumberFront, int& NumberBack);
	void    DoCSGFilterKeepInsideNoCoplanars(CBspNode* Polygon);
    void    DoCSGFilterKeepInsideNoCoplanars(CBspNode* Node, CBspNode* Polygon, int& NumberFront, int& NumberBack);
    bool    isAWithinB(ThredPolygon* polyA, ThredPolygon* polyB);
    double  ComputeArea(ThredPolygon* poly);
    void    DeleteCoplanarPolygons(CBspNode** PolygonList);
    void	DoCSGFilterKeepOutsideNoCoplanars(CBspNode* Polygon);
    void	DoCSGFilterKeepOutsideCoplanars(CBspNode* Polygon);
    void	DoCSGFilterKeepOutsideNoCoplanars(CBspNode* Node, 
	        CBspNode* Polygon, int& NumberFront, int& NumberBack);
    void	DoCSGFilterKeepOutsideCoplanars(CBspNode* Node, 
	        CBspNode* Polygon, int& NumberFront, int& NumberBack);

    void  ValidateNodeTextureIndex(CBspNode* Node, int TMax);
    void  InvalidateNodeCacheKeys (CBspNode* Node);
    void  ReverseVertices(ThredPolygon* poly);
    void  CoagulateCoplanars(CBspNode* node);
    void  SetLighting(CBspNode* Node);
    void  ProcessCacheInfoHere(CBspNode* Node, bool do_poly_splitting);
    void  ProcessTreeCacheInfo(CBspNode* Node, bool do_poly_splitting);

    static GFXTextureHandle GetNextCacheKey(CBspNode* node);
    bool PointInPoly(CBspNode* Node, Point3D point);
    int NormalizeTreePolyTextures(CBspNode* Node);
    CBspNode* GetSelectedNode();
    bool APolyIsSelected();
//    bool PointInPoly (Point3F point, ThredPolygon polygon, TSPerspectiveCamera* ts_cam);
    bool FindIntersectingNode(CBspNode* Node, Point3D pos, Point3D line_o_sight);
    void WalkSolidTreeFrontToBack(CBspNode* Node, ThredPoint* pos, Point3D line_o_sight);
    void SelectAPolygon(CPoint pos, CRenderCamera* Camera, TSRenderContext* rc);
	void FixupRecursive(CThredBrush* BrushList, CBspNode* Node);
	void FixupPointers(CThredBrush* BrushList );
	int MakeSurePolygonIn(CBspNode* Node, ThredPolygon* Polygon);
	void ConvexCalculateSubLevel(CBspNode *Node, CBspNode *MainPolygonList,
	CBspNode **FrontPolygonList, CBspNode **BackPolygonList, CProgressCtrl* Progess=NULL);
	void BuildPolygonListFromTree(CBspNode* List, CBspNode* Node );
	void ConvexFilterDown(CBspNode* Node, CBspNode* Poly );
	void FilterDownSubtree(CBspNode* Node);
	int IsAllSplits(CBspNode *Node);
	void ConvexDoSubTree(CBspNode *Node, CBspNode *FrontPolygonList,CBspNode *BackPolygonList);
	CBspNode* ConvexGetBestNode(CBspNode *PolyList);
	void CreateConvexBsp(CThredBrush* Brush);

	int AddQuickPolygonToTree(ThredPolygon* Polygon, CBspNode* CheckPoly, CThredBrush* parentbrush);
	int InitializeQuickBspNodes(int NumberOfNodes);
	void DoCSGFilterKeepInside(CBspNode* Polygon);
	void DoCSGFilterKeepOutside(CBspNode* Polygon);
	void CountSplits(CBspNode* Splitter, CBspNode* PolyList, int& FrontCount, int& BackCount, int& Splits);
	CBspNode* CreateBrushPolygonList();
	void ProcessBrush(CThredBrush* Brush, int BrushId, bool do_csg_and_tree);
	void RebuildQuick(CProgressCtrl* Progress, int BuildWater, bool do_csg_and_tree);
	void RebuildOptimal(CProgressCtrl* Progress, int BuildWater, bool exporting);
	void SetBrushList(CThredBrush* List) { mBrushList = List; }
	void BspManager();
	void RenderOrthoView(CRenderCamera* Camera);
	void RenderSolidWorld(CRenderCamera* Camera, TSRenderContext* rc, int TextureType, CTHREDDoc* doc);
	void Export(CString& Filename, CTHREDDoc* pDoc);
	void Serialize(CThredParser& Parser);
	int SubtractPolygonFromTree(CBspNode* Polygon);
	void ClearBsp();
	void ResetNewNodes();
	int AddPolygonToTree(CBspNode* Polygon);
	CConstructiveBsp();
	~CConstructiveBsp();
	void	FilterBrushBsp(CConstructiveBsp& BrushBsp);
    void    FilterCSGOnly(CThredBrush* Brush);
	CBspNode	*TreeTop;
	CBspNode	*CSGTop;
	CThredBrush* mBrushList;
    GFXSurface*   localpBuffer;

    // For point array's drawPoly() method...
    TS::VertexIndexPair         pairs[1000];
    Point2F                     texture_indices[1000];


// DPW - Added for TS compatibility
protected:
    double maxval(double val1, double val2);
    double minval(double val1, double val2);
    void splitX(Poly* poly,PolyList* polyList);
    void splitY(Poly* poly,PolyList* polyList);
    void normalizeTexture(Poly* poly);
    void FindTextureCenter(CBspNode* curr_node, CThredBrush* brush, int which_poly);

    struct
    {
      double start;
      double end;
    } polyspan[MAX_POLY_SPANS];

	Point2I*			poly_point_2d[MAX_POLYGON_VERTS];
//	Point4I*			poly_point_3d[MAX_POLYGON_VERTS];
	Point2F*			tex_point_2d[MAX_POLYGON_VERTS];
//	Point4I*			render_poly_point_3d[MAX_POLYGON_VERTS];
	Point2F*			render_tex_point_2d[MAX_POLYGON_VERTS];
	TSMaterialList*		poly_mat_list;
   GFXLightMap *        genericLightmap;
      
private:
    void DeleteBrushIDNodes(CBspNode* Node, int BrushId);
	CBspNode* GetNewQuickNode();
	void QuickFilterPoly(CBspNode* Node, ThredPolygon* Polygon, CThredBrush* parentbrush);
	int  CountChildren(CBspNode* Node );
	void DeleteChildren(CBspNode* Node, int NumberToDelete, int OriginalCount);
	void DoCSGFilterKeepOutside(CBspNode* Node, CBspNode* Polygon, int& NumberFront, int& NumberBack);
	void DoCSGFilterKeepInside(CBspNode* Node, CBspNode* Polygon, int& NumberFront, int& NumberBack);
	void CalculateSubLevel(CBspNode *Node, CBspNode *MainPolygonList, CBspNode **FrontPolygonList, CBspNode **BackPolygonList, CProgressCtrl* Progress=NULL);
	void DoSubTree(CBspNode *Node, CBspNode *FrontPolygonList, CBspNode *BackPolygonList, CProgressCtrl* Progress);
	void DeleteNonDrawnPolygons(CBspNode** PolygonList );
	CBspNode* GetBestNode(CBspNode *PolyList);
	void CollectBspStats(CBspNode* Node);
	//void DrawSolidNode(CBspNode* Node, CRenderCamera* Camera, TSRenderContext* rc, int TextureType);
    void DrawSolidNode(CBspNode* Node, CRenderCamera* Camera, TSRenderContext* rc, int TextureType, BOOL showvolumes);
	void RenderSolidTreeBackToFront(CBspNode* Node, CRenderCamera* Camera, TSRenderContext* rc, int TextureType, BOOL showvolumes);
	//int RenderSolidTreeBackToFront(CBspNode* Node, CRenderCamera* Camera, TSRenderContext* rc, int TextureType);
	void	FilterBrushBsp(CBspNode* Node, CConstructiveBsp& BrushBsp);
	void FilterNodeThroughBsp(CBspNode* OriginalNode, CBspNode* Node, ThredPolygon* Polygon, int& NumberFront, int& NumberBack, int Mode);
	void DeleteTree(CBspNode* Node);
	void DrawNode(CBspNode* Node,  CRenderCamera* Camera);
	void RenderTreeBackToFront(CBspNode* Node, CRenderCamera* Camera);
	int IntersectLinePlane(ThredPoint* Point1, ThredPoint* Point2, CBspNode* Node, ThredPoint* NewPoint);
	void ResetNewNodes(CBspNode* Node);
	int ClassifyPoint(CBspNode* Node, ThredPoint* Point);
	ThredPolygon* SplitPolygon(CBspNode* Node, ThredPolygon* SplitPolygon, ThredPolygon* NewPolygon);
	int ClassifyPolygon(CBspNode* Node, ThredPolygon* Polygon);
	void FilterPolygonThroughTree(CBspNode* Node, CBspNode* Polygon, FilterType Mode);
	void FilterPolygonThroughTreeSubtract(CBspNode* Node, CBspNode* Polygon, FilterType Mode);

	int SerializeTree(CThredParser& Parser, CBspNode* Node);
	//int PolygonCount, VertCount;
	CPoint mPointList[MAX_POLYGON_VERTS];

    CBspNode* found_node;
    double    found_distance;

	// Some stats for the BSP
	int mNodes, mCoplanarNodes, mOriginalPolys, mSplits, mTotalPolys;

	// this is a list of CBspNodes that are used when
	// we are creating quick trees for filtering
	CBspNode* mQuickBspNodes;
	int       mNumberOfQuickNodes;
	int       mCurrentQuickNode;

    bool      found;
};


/////////////////////////////////////////////////////////////////////////////
#endif
