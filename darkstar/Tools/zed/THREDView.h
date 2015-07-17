// THREDView.h : interface of the CTHREDView class
//
/////////////////////////////////////////////////////////////////////////////

#include "thredprimitives.h"
#include "RenderCamera.h"

// DPW - Added for TS compatibility
//#include "gfxwin.h"
#include "ts_camera.h"
#include "g_surfac.h"
#include "ts_RenderContext.h"
#include "ts_pointArray.h"
#include "childfrm.h"


#define ANGLE_CONVERSION  (((double)M_PI_VALUE)/(double)(M_TOTAL_DEGREES/2))


class CTHREDView : public CView
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CTHREDView)

// Attributes
public:
	
    void DoBrushRotate(CPoint point, int DeltaX, int DeltaY);
    void SetPalette(GFXPalette* Pal);
	void SetTool(int Tool);
	CTHREDView();
	void SetTitle();
	void ViewZoomin();
	void ViewZoomout();
	int GetTool(void);
	CTHREDDoc* GetDocument();
	UINT mViewType;

    CRect selectRect;
    BOOL  showSelectRect;
	CButton	ViewButton;
	CChildFrame * pFrame;

protected:
    void DrawSolid(ThredPolygon* poly, int color, TSRenderContext* rc, int vertex_dir);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTHREDView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTHREDView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  void  ResizeBuffer(int cx, int cy);


// Generated message map functions
protected:
	//{{AFX_MSG(CTHREDView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnToolsBrushResetbrush();
	afx_msg void OnToolsBrushShowbrush();
	afx_msg void OnUpdateToolsBrushShowbrush(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnToolsEntity();
	afx_msg void OnUpdateToolsEntity(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBrushGroupsMakenewgroup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBrushGroupsAddtogroup(CCmdUI* pCmdUI);
	afx_msg void OnBrushGroupsSelect();
	afx_msg void OnUpdateBrushGroupsSelect(CCmdUI* pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnToolsBrushMoveselectedbrushes();
	afx_msg void OnUpdateToolsBrushMoveselectedbrushes(CCmdUI* pCmdUI);
	afx_msg void OnTextureEditFace();
	afx_msg void OnUpdateTextureEditFace(CCmdUI* pCmdUI);
	afx_msg void OnEditModeButton();
	afx_msg void OnUpdateEditModeButton(CCmdUI* pCmdUI);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewCentercameraatorigin();
	afx_msg void OnToolsCentercam();
	afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	//}}AFX_MSG
	afx_msg void OnViewButton();
	afx_msg void OnViewType(UINT nID);
	afx_msg void OnViewTypeCmdUi(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
private:
	double mZoomFactor;
	int mShowBrush;
	//int mViewAxis;
    int m_LeftOp;
    int m_RightOp;
	CPoint mStartPoint;
    CPoint mLastCursorPos;
	CPoint mCurrentPoint;
	CRenderCamera mCamera;

	double	m_XAxisRotation;	// In radians
	double	m_YAxisRotation;	// In radians
	double	m_ZAxisRotation;	// In radians

    // For New 3space
    TSRenderContext*      m_renderContext;
    TSPointArray          DefaultPointArray;
	TSPerspectiveCamera*  m_pTSCamera;
	GFXSurface*           pBuffer;  // The view allocs/reallocs this on size updates & gives to r.c.
    double                 nearPlane;
	double                 farPlane;
	TSSceneLighting 	  m_GSceneLights;
    bool                  ShiftIsDown;
    bool                  CtrlIsDown;
    bool                  AltIsDown;
    bool                  TextureEditing;

};

#ifndef _DEBUG  // debug version in THREDView.cpp
inline CTHREDDoc* CTHREDView::GetDocument()
   { return (CTHREDDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
