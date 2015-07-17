// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "thredprimitives.h"
#include "objectview.h"
#include "groupview.h"
#include "textureview.h"
#include "lightview.h"
#include "materialproperties.h"

// Instead of using a class here we could have just put these controls
// directly into CMainFrame.  As it is they are sending messages to the
// main frame just like they were part of it instead of part of a control
// bar.
class CStyleBar : public CToolBar
{
public:
	CComboBox   m_comboBox;
	CFont       m_font;
};


class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
    void UpdateGeneralStatus(char* text);
    void UpdateRenderInfo();
    void UpdateBrushInfoPOS(ThredPoint position);
    void UpdateBrushInfoSIZE(ThredPoint size);
    void UpdateBrushInfoROTATION(double rotation);
	void UpdateGridSize(int GridSize, int SnapOn);
	CFrameWnd* CreateNewGameViewFrame(CRuntimeClass* pViewClass,  CDocTemplate* pTemplate, CDocument* pDoc, CFrameWnd* pOther);
	void MakeNewView(CRuntimeClass* pViewRuntimeClass);
	void LoadComboBox();
	BOOL CreateStyleBar();
   	BOOL CreateTextureView();
	BOOL CreateObjectView();
	BOOL CreateGroupView();
	BOOL CreateLightView();
	CMainFrame();


// Attributes
public:
	void Show4Views();

	CStatusBar  	m_wndStatusBar;
	CToolBar    	m_wndToolBar;
	CStyleBar    	m_wndBrushToolBar;
	CObjectView 	m_wndObjectView;
	CGroupView 	    m_wndGroupView;
	CTextureView    m_wndTextureView;
	CLightView 	    m_wndLightView;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	afx_msg void OnNewSelection();

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	//CStatusBar  m_wndStatusBar;
	//CToolBar    m_wndToolBar;
	//CStyleBar    m_wndBrushToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
   afx_msg void OnViewMaterialProperties();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnView4views();
	afx_msg void OnUpdateView4views(CCmdUI* pCmdUI);
	afx_msg void OnViewTexturebrowser();
	afx_msg void OnUpdateViewTexturebrowser(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewObjectview();
	afx_msg void OnUpdateViewObjectview(CCmdUI* pCmdUI);
	afx_msg void OnViewGroupview();
	afx_msg void OnUpdateViewGroupview(CCmdUI* pCmdUI);
	afx_msg void OnViewTextureview();
	afx_msg void OnUpdateViewTextureview( CCmdUI* pCmdUI );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
