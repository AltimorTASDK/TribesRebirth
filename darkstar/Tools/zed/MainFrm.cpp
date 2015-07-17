// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "THRED.h"
#include "THREDDoc.h"
#include "Thredview.h"
//#include "textureview.h"
#include "childfrm.h"

// DPW - Added for material viewing
#include "matvudlg.h"

#include "MainFrm.h"
#include "gfxmetrics.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_4VIEWS, OnView4views)
	ON_UPDATE_COMMAND_UI(ID_VIEW_4VIEWS, OnUpdateView4views)
	ON_COMMAND(ID_VIEW_TEXTUREBROWSER, OnViewTexturebrowser)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTUREBROWSER, OnUpdateViewTexturebrowser)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_OBJECTVIEWER, OnViewObjectview)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTVIEWER, OnUpdateViewObjectview)
	ON_COMMAND(ID_VIEW_GROUPVIEWER, OnViewGroupview)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GROUPVIEWER, OnUpdateViewGroupview)
	ON_COMMAND(ID_VIEW_TEXTUREVIEWER, OnViewTextureview)
   ON_COMMAND( ID_VIEW_MATERIALPROPERTIES, OnViewMaterialProperties)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTUREVIEWER, OnUpdateViewTextureview)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpFinder)
	ON_CBN_SELCHANGE(ID_TOOLBAR_COMBOBOX, OnNewSelection )
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	//ID_INDICATOR_CAPS,
	//ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
    ID_INDICATOR_POS,
    ID_INDICATOR_BRUSHINFO,
	ID_GRIDINFORMATION
//	0
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    WINDOWPLACEMENT   lpwndpl;

    GetWindowPlacement(&lpwndpl);
    lpwndpl.showCmd = SW_SHOWMAXIMIZED;
    SetWindowPlacement(&lpwndpl);

	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if(!CreateStyleBar() )
	{
		TRACE0("Failed to create brush toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_wndBrushToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndBrushToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_TOP);
	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_wndBrushToolBar);
	EnableDocking(CBRS_ALIGN_ANY);

   if( !CreateObjectView() )
   {
		TRACE0("Failed to create shape view\n");
		return -1;      // fail to create
   }
   
   if( !CreateGroupView() )
   {
		TRACE0("Failed to create group view\n");
		return -1;      // fail to create
   }
   
   if( !CreateTextureView() )
   {
		TRACE0("Failed to create texture view\n");
		return -1;      // fail to create
   }
   
   if( !CreateLightView() )
   {
		TRACE0("Failed to create light view\n");
		return -1;      // fail to create
   }
   
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnView4views() 
{
   // get the child frame that is currently active
	CChildFrame * pActiveChild = ( CChildFrame * )MDIGetActive();
	if(!pActiveChild )
		return;
      
	CTHREDView  * pThredView;
   
   // get and set each pane on the splitter
   pThredView = ( CTHREDView *)pActiveChild->m_wndSplitter.GetPane( 0, 0 );
   pThredView->mViewType = ID_VIEW_SIDEVIEW;
   pThredView = ( CTHREDView *)pActiveChild->m_wndSplitter.GetPane( 0, 1 );
   pThredView->mViewType = ID_VIEW_TOPVIEW;
   pThredView = ( CTHREDView *)pActiveChild->m_wndSplitter.GetPane( 1, 0 );
   pThredView->mViewType = ID_VIEW_FRONTVIEW;
   pThredView = ( CTHREDView *)pActiveChild->m_wndSplitter.GetPane( 1, 1 );
   pThredView->mViewType = ID_VIEW_SOLIDRENDER;

	CDocument* pDoc = pActiveChild->GetActiveDocument();
	ASSERT_VALID(pDoc );

	pDoc->UpdateAllViews(NULL);
}

void CMainFrame::OnUpdateView4views(CCmdUI* pCmdUI) 
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if(!pActiveChild ) {
		pCmdUI->Enable(0);
		return;
	}
	CDocument* pDocument = pActiveChild->GetActiveDocument();
	if(pDocument )
		pCmdUI->Enable(1);
	else
		pCmdUI->Enable(0);
}



BOOL CMainFrame::CreateStyleBar()
{
	const int nDropHeight = 200;
							  
	if (!m_wndBrushToolBar.Create(this, WS_CHILD|WS_VISIBLE|CBRS_TOP|
			CBRS_TOOLTIPS|CBRS_FLYBY) ||
		!m_wndBrushToolBar.LoadToolBar(IDR_BRUSHTOOLS) )
	{
		TRACE0("Failed to create stylebar\n");
		return FALSE;       // fail to create
	}

	// Create the combo box
	m_wndBrushToolBar.SetButtonInfo(0, ID_AXIS_X, TBBS_SEPARATOR, 130);

	// Design guide advises 12 pixel gap between combos and buttons
	m_wndBrushToolBar.SetButtonInfo(1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
	CRect rect;
	m_wndBrushToolBar.GetItemRect(0, &rect);
	rect.top = 3;
	rect.bottom = rect.top + nDropHeight;
	if (!m_wndBrushToolBar.m_comboBox.Create(
			CBS_DROPDOWNLIST|WS_VISIBLE|WS_TABSTOP|CBS_SORT,
			rect, &m_wndBrushToolBar, ID_TOOLBAR_COMBOBOX))
	{
		TRACE0("Failed to create combo-box\n");
		return FALSE;
	}

	//  Create a font for the combobox
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(logFont));

	if (!::GetSystemMetrics(SM_DBCSENABLED))
	{
		// Since design guide says toolbars are fixed height so is the font.
#ifndef _MAC
		logFont.lfHeight = -12;
#else
		logFont.lfHeight = -14;     // looks better than 12 on Mac
#endif
		logFont.lfWeight = FW_BOLD;
		logFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
		lstrcpy(logFont.lfFaceName, "MS Sans Serif");
		if (!m_wndBrushToolBar.m_font.CreateFontIndirect(&logFont))
			TRACE0("Could Not create font for combo\n");
		else
			m_wndBrushToolBar.m_comboBox.SetFont(&m_wndBrushToolBar.m_font);
	}
	else
	{
		m_wndBrushToolBar.m_font.Attach(::GetStockObject(SYSTEM_FONT));
		m_wndBrushToolBar.m_comboBox.SetFont(&m_wndBrushToolBar.m_font);
	}

	return TRUE;
}

// dis function will fill up the combo box with the
// list of groups and id's
void CMainFrame::LoadComboBox()
{

	m_wndBrushToolBar.m_comboBox.ResetContent();
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CTHREDDoc* pDoc;
	if (pActiveChild == NULL ||
	  (pDoc = (CTHREDDoc*)pActiveChild->GetActiveDocument()) == NULL)
	{
//		TRACE0("Warning: No active document for WindowNew command.\n");
//		_ASSERTE(0);
		// since we have nothing active we just have an empty
		// combo box
		return;     // command failed
	}

	// no group man....
	m_wndBrushToolBar.m_comboBox.AddString("No Group");
	m_wndBrushToolBar.m_comboBox.SetItemData(0, (DWORD)LB_ERR);
	m_wndBrushToolBar.m_comboBox.SetCurSel(0);

	// get the list of group names and add them
	// to the box
	for(int Group = 0; Group < (*pDoc->pBrushGroupArray).GetSize(); Group++ )
    {
		int Id = m_wndBrushToolBar.m_comboBox.AddString((*pDoc->pBrushGroupArray)[Group].GetName());
		m_wndBrushToolBar.m_comboBox.SetItemData(Id, Group);
		if(Group == pDoc->mCurrentGroup )
			m_wndBrushToolBar.m_comboBox.SetCurSel(Id);
	}
}

// put in a message handler from the combo box
// so that we know what the current selection is.
// have a way to "load up" the toolbar.
//GetActiveDocument
void CMainFrame::OnNewSelection()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CTHREDDoc* pDoc;
	if (pActiveChild == NULL ||
	  (pDoc = (CTHREDDoc*)pActiveChild->GetActiveDocument()) == NULL)
	{
		TRACE0("Warning: No active document for WindowNew command.\n");
		_ASSERTE(0);
		return;     // command failed
	}
	int Id = m_wndBrushToolBar.m_comboBox.GetCurSel();
	pDoc->mCurrentGroup = m_wndBrushToolBar.m_comboBox.GetItemData(Id);
	pDoc->UpdateBrushInformation();
	pDoc->UpdateAllViews(NULL);
}



CFrameWnd* CMainFrame::CreateNewGameViewFrame(CRuntimeClass* pViewClass,  CDocTemplate* pTemplate, CDocument* pDoc, CFrameWnd* pOther)
{
	// make sure we have a doc
	if (pDoc != NULL) {
		ASSERT_VALID(pDoc);
	}

	// create a frame wired to the specified document
	CCreateContext context;
	context.m_pCurrentFrame = pOther;
	context.m_pCurrentDoc = pDoc;
	context.m_pNewViewClass = pViewClass;
	context.m_pNewDocTemplate = pTemplate;

	// make the object
	CFrameWnd* pFrame = (CFrameWnd*)(TEMPLATE_CHILD_FRAME_CLASS->CreateObject());
	if (pFrame == NULL)
	{
		TRACE1("Warning: Dynamic create of frame %hs failed.\n",
			TEMPLATE_CHILD_FRAME_CLASS->m_lpszClassName);
		return NULL;
	}
	ASSERT_KINDOF(CFrameWnd, pFrame);

	// create new from resource
	if (!pFrame->LoadFrame(TEMPLATE_RESOURCE_ID,
			WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,   // default frame styles
			NULL, &context))
	{
		TRACE0("Warning: CDocTemplate couldn't create a frame.\n");
		// frame will be deleted in PostNcDestroy cleanup
		return NULL;
	}

	// it worked !
	return pFrame;
}

void CMainFrame::MakeNewView(CRuntimeClass* pViewRuntimeClass)
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
	  (pDocument = pActiveChild->GetActiveDocument()) == NULL)
	{
		TRACE0("Warning: No active document for WindowNew command.\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return;     // command failed
	}

	// otherwise we have a new frame !
	CDocTemplate* pTemplate = pDocument->GetDocTemplate();
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame = CreateNewGameViewFrame(pViewRuntimeClass, pTemplate, pDocument, pActiveChild);
	if (pFrame == NULL)
	{
		TRACE0("Warning: failed to create new frame.\n");
		return;     // command failed
	}

	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

	

void CMainFrame::OnViewTexturebrowser() 
{
	MaterialView	material_dialog;
	CTHREDDoc*		pDoc;

	pDoc = (CTHREDDoc*)MDIGetActive()->GetActiveDocument();

	if (pDoc)
	{
		// DPW - Added new dialog to view material list
		material_dialog.material_count = pDoc->GetMaterialList()->getMaterialsCount();
		material_dialog.material_list = pDoc->GetMaterialList();
		material_dialog.palette = pDoc->GetPalette();

		material_dialog.DoModal();
	}

	// DPW - MakeNewView(RUNTIME_CLASS(CTextureView ) );
}

void CMainFrame::OnViewMaterialProperties()
{
   CMaterialProperties materialDialog;
   CTHREDDoc* pDoc;

	pDoc = (CTHREDDoc*)MDIGetActive()->GetActiveDocument();

	if( pDoc )
	{
		materialDialog.m_materialCount = pDoc->GetMaterialList()->getMaterialsCount();
		materialDialog.m_materialList = pDoc->GetMaterialList();
		materialDialog.m_palette = pDoc->GetPalette();

		materialDialog.DoModal();
	}
}

void CMainFrame::OnUpdateViewTexturebrowser(CCmdUI* pCmdUI) 
{
	CFrameWnd* pFrame= GetActiveFrame();

	if(pFrame == this || pFrame == NULL )
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

// update the size of the grid in the pane
void CMainFrame::UpdateGridSize(int GridSize, int SnapOn)
{
	CString Text;
	Text.Format("Gridsize: %d %s", GridSize, SnapOn ? "Snap On" : "Snap Off");
	m_wndStatusBar.SetPaneText(3, Text);
}


void CMainFrame::UpdateBrushInfoPOS(ThredPoint position)
{
	CString Text;

    Text.Format("Pos: (%5.1f,%5.1f,%5.1f)", position.X, position.Y, position.Z);
	m_wndStatusBar.SetPaneText(1, Text);
}


void CMainFrame::UpdateBrushInfoSIZE(ThredPoint size)
{
	CString Text;

    Text.Format("Size: (%5.1f,%5.1f,%5.1f)", size.X, size.Y, size.Z);
	m_wndStatusBar.SetPaneText(2, Text);
}


void CMainFrame::UpdateBrushInfoROTATION(double rotation)
{
	CString Text;

    Text.Format("Rotation: %5.1f", rotation);
	m_wndStatusBar.SetPaneText(2, Text);
}


// update the size of the grid in the pane
void CMainFrame::UpdateRenderInfo()
{
	CString Text;

    Text.Format("Polys Emitted: %d  Rendered: %d  Spans: %d  Edges: %d", GFXMetrics.emittedPolys, GFXMetrics.renderedPolys, GFXMetrics.numSpansUsed, GFXMetrics.numEdgesUsed);
	m_wndStatusBar.SetPaneText(0, Text);
    m_wndStatusBar.SendMessage(WM_PAINT, NULL, 0); 
}


void CMainFrame::UpdateGeneralStatus(char* text)
{
	CString Text;

    Text.Format("%s", text);
    m_wndStatusBar.SetPaneText(0, Text);
    m_wndStatusBar.SendMessage(WM_PAINT, NULL, 0); 
}

void CMainFrame::Show4Views()
{
  OnView4views();
}


void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIFrameWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
//	::SendMessage(m_hWndMDIClient, WM_MDITILE, MDITILE_HORIZONTAL, 0);	
}

BOOL CMainFrame::CreateObjectView( void )
{
   CRect rect;
   rect.right = 160;
   rect.left = 0;
   rect.bottom = 600;
   rect.top = 0;
      
   m_wndObjectView.CreateEx( WS_EX_CLIENTEDGE, AfxRegisterWndClass( CS_DBLCLKS, 
      LoadCursor( NULL, IDC_ARROW ) ), "Shape Viewer", WS_CHILD | 
      WS_VISIBLE | WS_CLIPSIBLINGS | CBRS_ALIGN_LEFT, rect, this, 0x1234 );

   m_wndObjectView.GetClientRect( &rect );
   m_wndObjectView.CurrentSize.cx = rect.right;
   m_wndObjectView.CurrentSize.cy = rect.bottom;

   m_wndObjectView.SetBarStyle( CBRS_ALIGN_LEFT );
   EnableDocking( CBRS_ALIGN_LEFT );
   m_wndObjectView.EnableDocking( CBRS_ALIGN_LEFT );
   DockControlBar( &m_wndObjectView );   

   m_wndObjectView.GetClientRect( &rect );
   
   CWindowDC   wndDC( &m_wndObjectView );
   CSize TextSize;
   CString Title;
   
   m_wndObjectView.GetWindowText( Title );
   TextSize = wndDC.GetTextExtent( Title );
   
   rect.top += TextSize.cy + 6;
   
   // save this value
   m_wndObjectView.CaptionSize = rect.top;
   
   m_wndObjectView.m_TreeWnd.Create( WS_VISIBLE | TVS_HASLINES | 
      TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_EDITLABELS, 
      rect, &m_wndObjectView, 0x1245 );
      
   // add an item
	m_wndObjectView.m_ImageList.Create( 20, 20, ILC_COLOR16 | ILC_MASK, 4, 0 );

	// load the bitmaps and add them to the image lists
   CBitmap Bitmap;
   Bitmap.LoadBitmap( IDB_STATE );
	m_wndObjectView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();
   
   Bitmap.LoadBitmap( IDB_DETAIL );
	m_wndObjectView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();

   Bitmap.LoadBitmap( IDB_DETAIL2 );
	m_wndObjectView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();
   
   Bitmap.LoadBitmap( IDB_LIGHT );
	m_wndObjectView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();
   
   Bitmap.LoadBitmap( IDB_LIGHT2 );
	m_wndObjectView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();
   
	m_wndObjectView.m_TreeWnd.SetImageList( &m_wndObjectView.m_ImageList, 
      TVSIL_NORMAL );
   
   // show the bar as default
	ShowControlBar( &m_wndObjectView, TRUE, TRUE );
   
   return( TRUE );
}

BOOL CMainFrame::CreateGroupView( void )
{
   CRect rect;
   rect.right = 200;
   rect.left = 0;
   rect.bottom = 300;
   rect.top = 0;
      
   m_wndGroupView.CreateEx( WS_EX_CLIENTEDGE, AfxRegisterWndClass( CS_DBLCLKS, 
      LoadCursor( NULL, IDC_ARROW ) ), "Group Viewer", WS_CHILD | 
      WS_VISIBLE | WS_CLIPSIBLINGS | CBRS_ALIGN_LEFT, rect, this, 0x1234 );

   m_wndGroupView.GetClientRect( &rect );
   m_wndGroupView.CurrentSize.cx = rect.right;
   m_wndGroupView.CurrentSize.cy = rect.bottom;

   m_wndGroupView.SetBarStyle( CBRS_ALIGN_LEFT  );
   EnableDocking( CBRS_ALIGN_LEFT );
   m_wndGroupView.EnableDocking( CBRS_ALIGN_LEFT );
   DockControlBar( &m_wndGroupView );   

   m_wndGroupView.GetClientRect( &rect );
   
   CWindowDC   wndDC( &m_wndGroupView );
   CSize TextSize;
   CString Title;
   
   m_wndGroupView.GetWindowText( Title );
   TextSize = wndDC.GetTextExtent( Title );
   
   rect.top += TextSize.cy + 6;
   
   // save this value
   m_wndGroupView.CaptionSize = rect.top;
   
   m_wndGroupView.m_TreeWnd.Create( WS_VISIBLE | TVS_HASLINES | 
      TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_EDITLABELS, 
      rect, &m_wndGroupView, 0x1245 );
      
   // add an item
	m_wndGroupView.m_ImageList.Create( 20, 20, ILC_COLOR16 | ILC_MASK, 3, 0 );

	// load the bitmaps and add them to the image lists
   CBitmap Bitmap;
   Bitmap.LoadBitmap( IDB_STATE );
	m_wndGroupView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();
   
   Bitmap.LoadBitmap( IDB_DETAIL );
	m_wndGroupView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();
   
   Bitmap.LoadBitmap( IDB_LIGHT );
	m_wndGroupView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();
   
	m_wndGroupView.m_TreeWnd.SetImageList( &m_wndGroupView.m_ImageList, 
      TVSIL_NORMAL );
   

   HTREEITEM hItem;   
   hItem = m_wndGroupView.m_TreeWnd.InsertItem( "Group", 0, 0 );
   m_wndGroupView.m_TreeWnd.InsertItem( "Entity", 1, 1, hItem );
   m_wndGroupView.m_TreeWnd.InsertItem( "Light", 2, 2, hItem );
   
   // dont show the group bar as default
	ShowControlBar( &m_wndGroupView, FALSE, FALSE);
   
   return( TRUE );
}

BOOL CMainFrame::CreateLightView( void )
{
   CRect rect;
   rect.right = 200;
   rect.left = 0;
   rect.bottom = 600;
   rect.top = 0;
      
   m_wndLightView.CreateEx( WS_EX_CLIENTEDGE, AfxRegisterWndClass( CS_DBLCLKS, 
      LoadCursor( NULL, IDC_ARROW ) ), "Light Viewer", WS_CHILD | 
      WS_VISIBLE | WS_CLIPSIBLINGS | CBRS_ALIGN_LEFT, rect, this, 0x1234 );

   m_wndLightView.GetClientRect( &rect );
   m_wndLightView.CurrentSize.cx = rect.right;
   m_wndLightView.CurrentSize.cy = rect.bottom;

   m_wndLightView.SetBarStyle( CBRS_ALIGN_LEFT  );
   EnableDocking( CBRS_ALIGN_LEFT );
   m_wndLightView.EnableDocking( CBRS_ALIGN_LEFT );
   DockControlBar( &m_wndLightView );   

   m_wndLightView.GetClientRect( &rect );
   
   CWindowDC   wndDC( &m_wndLightView );
   CSize TextSize;
   CString Title;
   
   m_wndLightView.GetWindowText( Title );
   TextSize = wndDC.GetTextExtent( Title );
   
   rect.top += TextSize.cy + 6;
   
   // save this value
   m_wndLightView.CaptionSize = rect.top;
   
   m_wndLightView.m_TreeWnd.Create( WS_VISIBLE | TVS_HASLINES | 
      TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_EDITLABELS, 
      rect, &m_wndLightView, 0x1245 );
      
   // add an item
	m_wndLightView.m_ImageList.Create( 20, 20, ILC_COLOR16 | ILC_MASK, 2, 0 );

	// load the bitmaps and add them to the image lists
   CBitmap Bitmap;
   Bitmap.LoadBitmap( IDB_LIGHT );
	m_wndLightView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();
   
   Bitmap.LoadBitmap( IDB_STATE );
	m_wndLightView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();

   Bitmap.LoadBitmap( IDB_DETAIL );
	m_wndLightView.m_ImageList.Add( &Bitmap, 0x00ffffff );
   Bitmap.DeleteObject();
   
	m_wndLightView.m_TreeWnd.SetImageList( &m_wndLightView.m_ImageList, 
      TVSIL_NORMAL );
   
   // dont show the group bar as default
	ShowControlBar( &m_wndLightView, false, false );
   
   return( TRUE );
}


BOOL CMainFrame::CreateTextureView()
{
   CRect rect;
   rect.right = 160;
   rect.left = 0;
   rect.bottom = 600;
   rect.top = 0;
      
   m_wndTextureView.CreateEx( WS_EX_CLIENTEDGE, AfxRegisterWndClass( CS_DBLCLKS, 
      LoadCursor( NULL, IDC_ARROW ) ), "Texture Viewer", WS_CHILD | 
      WS_VISIBLE | WS_CLIPSIBLINGS | CBRS_ALIGN_LEFT, rect, this, 0x2345 );

   m_wndTextureView.GetClientRect( &rect );
   m_wndTextureView.CurrentSize.cx = rect.right;
   m_wndTextureView.CurrentSize.cy = rect.bottom;

   m_wndTextureView.SetBarStyle( CBRS_ALIGN_LEFT );
   EnableDocking( CBRS_ALIGN_LEFT );
   m_wndTextureView.EnableDocking( CBRS_ALIGN_LEFT );
   DockControlBar( &m_wndTextureView );   

   m_wndTextureView.GetClientRect( &rect );
   
   CWindowDC   wndDC( &m_wndTextureView );
   CSize TextSize;
   CString Title;
   
   m_wndTextureView.GetWindowText( Title );
   TextSize = wndDC.GetTextExtent( Title );
   
   rect.top += TextSize.cy + 6;
   
   // save this value
   m_wndTextureView.CaptionSize = rect.top;

   m_wndTextureView.m_ListWnd.Create( WS_CHILD | WS_VSCROLL | WS_VISIBLE |
      LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWVARIABLE | LBS_DISABLENOSCROLL | 
      LBS_NOTIFY, rect, &m_wndTextureView, 0x1230 );
   
	ShowControlBar( &m_wndTextureView, FALSE, FALSE );

   return( TRUE );
}


void CMainFrame::OnViewObjectview() 
{
	BOOL bVisible = ((m_wndObjectView.GetStyle() & WS_VISIBLE) != 0);

	ShowControlBar(&m_wndObjectView, !bVisible, FALSE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewObjectview(CCmdUI* pCmdUI) 
{
	BOOL bVisible = ((m_wndObjectView.GetStyle() & WS_VISIBLE) != 0);
	pCmdUI->SetCheck(bVisible);   
}

void CMainFrame::OnViewGroupview() 
{
	BOOL bVisible = ((m_wndGroupView.GetStyle() & WS_VISIBLE) != 0);

	ShowControlBar(&m_wndGroupView, !bVisible, FALSE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewGroupview(CCmdUI* pCmdUI) 
{
	BOOL bVisible = ((m_wndGroupView.GetStyle() & WS_VISIBLE) != 0);
	pCmdUI->SetCheck(bVisible);   
}

void CMainFrame::OnViewTextureview() 
{
	BOOL bVisible = ((m_wndTextureView.GetStyle() & WS_VISIBLE) != 0);

	ShowControlBar(&m_wndTextureView, !bVisible, FALSE);
	RecalcLayout();
}

void CMainFrame::OnUpdateViewTextureview(CCmdUI* pCmdUI) 
{
	BOOL bVisible = ((m_wndTextureView.GetStyle() & WS_VISIBLE) != 0);
	pCmdUI->SetCheck(bVisible);   
}
