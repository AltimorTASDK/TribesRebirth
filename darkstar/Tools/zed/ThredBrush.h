// ThredBrush.h : header file
//


#ifndef _THREDBRUSH_H_
#define _THREDBRUSH_H_

#include "thredprimitives.h"
#include "rendercamera.h"
#include "constructivebsp.h"
#include "BrushGroup.h"
#include "ts_RenderContext.h"


class CTHREDDoc;

/////////////////////////////////////////////////////////////////////////////
// CThredBrush window
enum  { INDEX_BRUSH, INDEX_GROUP };
enum  { BRUSH_ADD_TO_WORLD, BRUSH_SUBTRACT_FROM_WORLD };
enum LastBrushAction { BRUSH_MOVE, BRUSH_ROTATE, BRUSH_SCALE, BRUSH_RESET, BRUSH_DIALOG };


// some defines for different brush flags
#define BRUSH_ACTIVE     BIT_0
#define	BRUSH_SELECTED   BIT_1
#define BRUSH_ENTITY     BIT_2

typedef struct
{
	double	m_YSize;
	int		m_Solid;
	double	m_Thickness;
	double	m_XSizeBot;
	double	m_XSizeTop;
	double	m_ZSizeBot;
	double	m_ZSizeTop;
} BoxInfo;

typedef struct
{
	int		m_Solid;
	double	m_Thickness;
	double	m_XSizeBot;
	double	m_YSize;
	double	m_ZSizeBot;
	double	m_ZSizeTop;
} TriInfo;

typedef struct
{
	double	m_BotXOffset;
	double	m_BotXSize;
	double	m_BotZOffset;
	double	m_BotZSize;
	int		m_Solid;
	double	m_Thickness;
	double	m_TopXOffset;
	double	m_TopXSize;
	double	m_TopZOffset;
	double	m_TopZSize;
	double	m_VerticalStripes;
	double	m_YSize;
    int     m_Ring;
} CylinderInfo;

typedef struct
{
	int		m_HorizontalBands;
	int		m_VerticalBands;
	double	m_XSize;
	double	m_YSize;
	double	m_ZSize;
	int		m_Solid;
	double	m_Thickness;
    int     m_HalfSphere;
} SphereInfo;

typedef struct
{
	double	m_Height;
	double	m_Length;
	double	m_NumberOfStairs;
	double	m_Width;
	BOOL	m_MakeRamp;
} StairInfo;


typedef struct
{
  int       TextureID;
  Point2I   TextureShift;

   // stored values for allowing moves/rotates of faces without altering
  double    LastTextureRotate;
  Point2I   LastTextureShift;
  
  double    TextureRotate;
  double    TextureScale;
  int       TextureFlipH;
  int       TextureFlipV;
} Texinfo;

class CThredBrush : public CObject
{
public:
   enum BrushTextureChange
   {
      TEX_ROTATE, TEX_MOVE
   };
   
    Point3F GetBrushFaceCenter(int whichface);
    void Shear(Point3F* minbound, Point3F* delta, CTHREDDoc* pDoc, ThredBox* BoundingBox, ShearType wrt);
    void RotateBrushAroundPoint(double Yaw, double Pitch, double Roll, ThredPoint point);
    void SetBrushCenter(ThredPoint center);
    void CalculateBrushCenter();
    void DoneMoveBrushCenter(double GridSize);
    void DoneResize(double GridSize);
    void Resize(Point3F* minbound, Point3F* delta, CTHREDDoc* pDoc, ThredBox* BoundingBox);
    void SetBBoxValues(Point3F min, Point3F max);
    void NormalizeTexture(int which_poly);
    void SplitBrushPolygons(CThredBrush* Brush);
    void BoxMap();
    Point2F GetCenterTextureOffsets(int which_face);
    void ShiftFaceTextureU(int which_face, double offset);
    void ShiftFaceTextureV(int which_face, double offset);
    void BeginTextureChange( BrushTextureChange type = TEX_MOVE );
    void EndTextureChange( BrushTextureChange type );
	CTransformMatrix mMatrix;
	CTransformMatrix mRenderMatrix;

    CTransformMatrix  mTotalMatrix;

	void DoneScaleBrush(int MoveType=0);
	void DoneShearBrush(int MoveType=0);

	// whether the transformed polygons are valid
	// or not
    void RotateFaceTexture(int which_face, double angle);
	void SetGroupMatrix();
	ThredPoint GetRealOrigin();
	void SetGroupPosition(ThredPoint Position);
	void ResetGroupItems();
	int mTransformedPolysValid;
	CThredBrush* GetBrushById(int BrushId );
	void RemoveBrush();
	void SetName(CString& Name);
	int CheckPoint(ThredPoint Point);

	// our name man ... our name
	CString mName;

    // DPW - Needed for recalling original brush parameters...
    enum TypesOBrush {UNKNOWN_BRUSH=0, BOX_BRUSH, TRI_BRUSH, CYL_BRUSH, SPHERE_BRUSH, STAIR_BRUSH} BrushType;
    union
    {
      BoxInfo       box;
      TriInfo       triangle;
      CylinderInfo  cylinder;
      SphereInfo    sphere;
      StairInfo     staircase;
      //BoxInfo       volume;
    } BrushSpecifics;


   	CThredBrush*  NextSelectedBrush;
    CThredBrush*  PrevSelectedBrush;

    // DPW - One texture id per face...
    Texinfo* mTextureID;
    
	// number of the group we belong to.
	int mBrushGroup;
	COLORREF mBrushColour;

	CTHREDDoc* pDoc;
   	ThredPoint mBrushPosition[2];
	ThredPoint mRenderBrushPosition[2];
    ThredPoint mBrushCenter;
    ThredPoint mBrushRealCenter;

	// the brush flags
	int mFlags;

	// associations with entities
	int mEntityId;

	// the id of this brush.
	int mBrushId;

    int m_ShowVolPolys;

    // The type of brush translated into a 32 bit integer (0 = non-volume brush)
    UInt32  volumestate;

	ThredPoint GetAveragePoint();
	int IsEntity() { return (mFlags & BRUSH_ENTITY); }
	void DoneRotateBrush(double RotateSnap, int MoveType =INDEX_BRUSH);
	void DoneMoveBrush(double GridSize, CTHREDDoc* pDoc, int MoveType=INDEX_BRUSH);
	void CreateQuickBrushBsp(CBspNode* Polygon);
	void RenderBrushStandard(CRenderCamera& Camera, int color, int color_select, TSRenderContext* rc);

	// define a copy constructor
    CThredBrush &operator=(CThredBrush &Brush );  // Right side is the argument.
	void Serialize(CThredParser& Parser, int maxTextureID = 0 );
	void AllocateBrushPolygons(int NumberOfPolygons);
	void DeleteBrushPolygons();
	void AllocateBrushPolyTextures(int NumberOfPolygons);
	void DeleteBrushPolyTextures();
	void CreateBrushBsp();
	void TransformBrushPolygons();
	void BrushAttributesDialog(CTHREDDoc* doc, int MoveType=INDEX_BRUSH);
	void ResetBrush(ThredPoint newpos, int MoveType=INDEX_BRUSH);
	void ScaleBrush(double X, double Y, double Z, int MoveType=INDEX_BRUSH);
    void MoveBrushCenter(double X, double Y, double Z, CTHREDDoc* pDoc, int MoveType=INDEX_BRUSH);
	void MoveBrush(double X, double Y, double Z, CTHREDDoc* pDoc, int MoveType=INDEX_BRUSH);
	void RotateBrush(double Yaw, double Pitch, double Roll, int MoveType=INDEX_BRUSH);
	void ShearBrush(double X, double Y, double Z, int MoveType=INDEX_BRUSH);
	void RenderBrush(CRenderCamera& Camera, COLORREF LineColour, COLORREF PointColour, COLORREF SelectColour, COLORREF BBoxColour, TSRenderContext* rc, bool showbb);
	void ShowOrigin(CRenderCamera& Camera, COLORREF Colour, TSRenderContext* rc);
	CThredBrush* NextEntityBrush(int EntityNumber);
	CThredBrush* NextActiveBrush();
	CThredBrush* GetActiveBrush();
	CThredBrush* GetNextBrush() { return NextBrush; }
	void SetNextBrush(CThredBrush* Next) { NextBrush = Next; }

	CThredBrush();
	~CThredBrush();
	CConstructiveBsp   mBrushBsp;
	int mNumberOfPolygons, mNumberOfTransformedPolygons;
	ThredPolygon* mTransformedPolygons;
	ThredPolygon* mPolygons;

    ThredPolygon  BBox[6];

	int	mTransactionType;
	ThredBox mBoundingBox;

	CThredBrush* PrevBrush;
	CThredBrush* NextBrush;
private:
	// make these private so we have to use the functions.

	void SetupBrushMatrix();
	// a list of points we can use for generic polygon
	// type stuff
	CPoint mPointList[MAX_POLYGON_VERTS];


	// All of the BRUSHES positional information
	ThredPoint mBrushRotation[2];
	ThredPoint mRenderBrushRotation[2];
	ThredPoint mBrushScale[2];
	ThredPoint mBrushShear[2];
	CTransformMatrix mBrushMatrix;

	// all of the GROUPS positional information
	CTransformMatrix mGroupMatrix;

	// here is our standard "pre-done" matrix
	CTransformMatrix mPredoneMatrix;

};

#endif

