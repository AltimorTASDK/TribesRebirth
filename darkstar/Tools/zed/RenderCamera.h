// RenderCamera.h : header file
//


#ifndef _RENDERCAMERA_H_
#define _RENDERCAMERA_H_

#include "thredprimitives.h"
#include "transformmatrix.h"
#include "Grid.h"

// DPW - Needed for TS compatibility
#include "ts_camera.h" //#include "ts_persp.h"
#include "g_surfac.h"
#include "ts_RenderContext.h"

/////////////////////////////////////////////////////////////////////////////
// CRenderCamera window
#define DEFAULT_CAMERA_X	0
#define DEFAULT_CAMERA_Y	0
#define DEFAULT_CAMERA_Z	-10

#define DEFAULT_CAMERA_YAW		0
#define DEFAULT_CAMERA_PITCH	0
#define DEFAULT_CAMERA_ROLL		0

// maximum spans we can have in a row
#define MAX_SPANS_PER_ROW	64
#define MAX_POLYS 3000


class CRenderCamera
{
public:
    void DrawRect(TSRenderContext* rc, CRect* srect, int color);
    void PositionCamera(double X, double Y, double Z);
    void SetViewType (int ViewType);
    bool Splitme (Point3F* point1, Point3F* point2, TSRenderContext* rc);
	void DrawOrthoGrid(int gridtype, int line_grid_color, int dot_grid_color, int origin_color, TSRenderContext* rc);
	void TranslateToWorld(CPoint Point, ThredPoint& TranPoint);
	CDC* GetDC() { _ASSERTE(m_pDC ); return m_pDC; }
	void DrawPoint(CPoint& Point, COLORREF Colour, TSRenderContext* rc);
   void DrawCircle( Point2I & center, int radius, COLORREF Colour, int numSegs, TSRenderContext* rc );
	void TranslateToScreen(CPoint& Point, ThredPoint TranPoint, TSRenderContext* rc);
	void DrawWireframePolygon(ThredPolygon* Polygon, COLORREF LineColour, COLORREF PointColour, TSRenderContext* rc, bool show_cornerpoints, bool show_midpoints);
	void SetViewSpecifics(CDC* pDC, int ViewType, double ZoomFactor, double GridSize);
	void MoveCameraAbsolute(double X, double Y, double Z);

	void RotateCameraWithLimits(double Yaw, double Pitch, double Roll);
	void MoveCamera(double X, double Y, double Z);
	void RotateCamera(double Yaw, double Pitch, double Roll);
	void DoPerspectiveTransform(ThredPoint& ThePoint );
	void SetupMatrix();
	ThredPoint mCameraPosition;
	ThredPoint mCameraDirection;	// pitch, yaw, roll
	CRect mClip;
	int mViewWidth;
	int mViewHeight;
	int mXOrigin;
	int mYOrigin;
	double mAngleOfVision;
	double mXScale;
	double mYScale;
	CTransformMatrix mMatrix;
	CTransformMatrix mRotateOnlyMatrix;
	double mZoomFactor;
	double mGridSize;

	// DPW - Needed for TS compatibility
	void DrawLine (Point2I* start, Point2I* end, int width, COLORREF Colour, TSRenderContext* rc);

   // for entities lights and such
   void DrawDiamond( ThredPoint & origin, float offset, COLORREF Colour, TSRenderContext* rc );
   void DrawTri( ThredPoint & origin, float offset, COLORREF Colour, TSRenderContext* rc );
   void DrawCone( ThredPoint & origin, ThredPoint & direction, float length, double angle, COLORREF Colour, TSRenderContext* rc );
   
public:
// Construction
	void SetupCamera(CRect& ClippingRect);//, RGBQUAD PaletteQuad[256]);
	void SetupCamera(int ClipMinX, int ClipMaxX, int ClipMinY, int ClipMaxY);//, RGBQUAD PaletteQuad[256]);
	CRenderCamera();
	~CRenderCamera();


// span setup stuff
private:
	// DPW - Needed for TS compatibility
//	TSRenderInterface*	pRender;
	GFXSurface*			pBuffer;
	Point2I*			poly_point_2d[MAX_POLYGON_VERTS];

	// specifics abour our current view including
	// a "good" device context.
	CDC* m_pDC;
	int mViewType;

	// a list of points we can use for generic polygon
	// type stuff
	CPoint mPointList[MAX_POLYGON_VERTS];
};

/////////////////////////////////////////////////////////////////////////////


#endif
