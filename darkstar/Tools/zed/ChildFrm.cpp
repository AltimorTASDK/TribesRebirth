// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "THRED.h"
#include "THREDDoc.h"
#include "Thredview.h"

#include "ChildFrm.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_SIZE()
   ON_WM_PAINT()
   ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
//	ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
   pCurrentDetail = NULL;
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CMDIChildWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers
						
//LRESULT CChildFrame::OnSetText(WPARAM wParam, LPARAM lParam)
//{
//	const char *Text = (const char *)lParam;
//	if(Text[0] == '-' ) {
//		return Default();
//	}
//	return 1;
//}


BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpCreateStruct,
   CCreateContext* pContext ) 
{
   // create a static splitter control 
   m_wndSplitter.CreateStatic( this, 2, 2, WS_CHILD );

   SIZE size;
   CRect rect;     
   
   GetClientRect( &rect );
   size.cx = rect.right / 2;
   size.cy = rect.bottom / 2;

   m_wndSplitter.CreateView( 0, 0, RUNTIME_CLASS( CTHREDView ),size, pContext );
   m_wndSplitter.CreateView( 0, 1, RUNTIME_CLASS( CTHREDView ),size, pContext );
   m_wndSplitter.CreateView( 1, 0, RUNTIME_CLASS( CTHREDView ),size, pContext );
   m_wndSplitter.CreateView( 1, 1, RUNTIME_CLASS( CTHREDView ),size, pContext );

   // setup teh defaults
	CTHREDView  * pThredView;

   // get a view to grab the document   
   pThredView = ( CTHREDView *)m_wndSplitter.GetPane( 0, 0 );
   
   // get the doc
   CTHREDDoc * pDoc = ( CTHREDDoc * )pThredView->GetDocument();
   
   // grab the active detail and set to this child window
   pCurrentDetail = NULL; 
   //pDoc->pActiveDetail;
   
   // get and set each pane on the splitter
   pThredView = ( CTHREDView *)m_wndSplitter.GetPane( 0, 0 );
   pThredView->mViewType = ID_VIEW_SIDEVIEW;
   pThredView->pFrame = this;
   pThredView = ( CTHREDView *)m_wndSplitter.GetPane( 0, 1 );
   pThredView->mViewType = ID_VIEW_TOPVIEW;
   pThredView->pFrame = this;
   pThredView = ( CTHREDView *)m_wndSplitter.GetPane( 1, 0 );
   pThredView->mViewType = ID_VIEW_FRONTVIEW;
   pThredView->pFrame = this;
   pThredView = ( CTHREDView *)m_wndSplitter.GetPane( 1, 1 );
   pThredView->mViewType = ID_VIEW_SOLIDRENDER;
   pThredView->pFrame = this;
   
   m_wndSplitter.ShowWindow( SW_SHOWNORMAL );
   m_wndSplitter.UpdateWindow();
   
   // set active to top right
   SetActiveView( ( CView * )m_wndSplitter.GetPane( 0,0 ) );

   m_wndSplitter.Created = TRUE;

   return( TRUE );
}

void CChildFrame::OnPaint( void )
{
   // update the title's
   if( pCurrentDetail )
      pCurrentDetail->UpdateTitles();
      
   CMDIChildWnd::OnPaint();
}

void CChildFrame::OnSize( UINT nType, int cx, int cy )
{
   CMDIChildWnd::OnSize(nType, cx, cy);

   if( m_wndSplitter.Created )	
      m_wndSplitter.UpdateSplitters();	
}

void CChildFrame::RotateViews( BOOL fForward )
{
   // setup teh defaults
	CTHREDView  * pThredViewA;
	CTHREDView  * pThredViewB;
	CTHREDView  * pThredViewC;
	CTHREDView  * pThredViewD;
	UINT ViewType;

   pThredViewA = ( CTHREDView *)m_wndSplitter.GetPane( 0, 0 );
   pThredViewB = ( CTHREDView *)m_wndSplitter.GetPane( 0, 1 );
   pThredViewC = ( CTHREDView *)m_wndSplitter.GetPane( 1, 0 );
   pThredViewD = ( CTHREDView *)m_wndSplitter.GetPane( 1, 1 );
   
   // check the
   if( fForward )
   {
      // rotate around
      ViewType = pThredViewA->mViewType; 
      pThredViewA->mViewType = pThredViewC->mViewType;
      pThredViewC->mViewType = pThredViewD->mViewType;
      pThredViewD->mViewType = pThredViewB->mViewType;
      pThredViewB->mViewType = ViewType;
   }
   else
   {
      // rotate around
      ViewType = pThredViewA->mViewType; 
      pThredViewA->mViewType = pThredViewB->mViewType;
      pThredViewB->mViewType = pThredViewD->mViewType;
      pThredViewD->mViewType = pThredViewC->mViewType;
      pThredViewC->mViewType = ViewType;
   }

   // update the buttons
   pThredViewA->SetTitle();
   pThredViewB->SetTitle();
   pThredViewC->SetTitle();
   pThredViewD->SetTitle();
   
   // get the doc and update
   CTHREDDoc * pDoc = ( CTHREDDoc * )pThredViewA->GetDocument();
   pDoc->UpdateAllViews( NULL );
}
