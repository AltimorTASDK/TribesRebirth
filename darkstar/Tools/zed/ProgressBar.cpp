// ProgressBar.cpp : implementation file
	
#include "stdafx.h"
#include "ProgressBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CProgressBar, CProgressCtrl)

BEGIN_MESSAGE_MAP(CProgressBar, CProgressCtrl)
	//{{AFX_MSG_MAP(CProgressBar)
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CProgressBar::CProgressBar()
{
   m_bSuccess = FALSE;
   m_Rect.SetRect(0,0,0,0);
}

CProgressBar::CProgressBar( int MaxValue, BOOL bSmooth, int Pane )
{
   Create( MaxValue, bSmooth, Pane );
}

CProgressBar::~CProgressBar()
{
    Clear();
}

// get the status bar
CStatusBar* CProgressBar::GetStatusBar()
{
   CFrameWnd *pFrame = (CFrameWnd*)AfxGetMainWnd();
   if (!pFrame || !pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd))) 
      return NULL;

   CStatusBar* pBar = (CStatusBar*) pFrame->GetMessageBar();
   if (!pBar || !pBar->IsKindOf(RUNTIME_CLASS(CStatusBar))) 
      return NULL;

   return pBar;
}

// Create the CProgressCtrl as a child of the status bar positioned
// over the 'pane" pane.
// Sets the range to be 0 to MaxValue, with a step of 1.
BOOL CProgressBar::Create( int MaxValue, BOOL bSmooth, int Pane )
{
   m_bSuccess = FALSE;

   // set the allowable range
   m_range = MaxValue;
   
   CStatusBar *pStatusBar = GetStatusBar();
   if( !pStatusBar )
      return FALSE;

	DWORD dwStyle = WS_CHILD|WS_VISIBLE;
#ifdef PBS_SMOOTH	
   if( bSmooth )
      dwStyle |= PBS_SMOOTH;
#endif

   // Create the progress bar
   VERIFY( m_bSuccess = CProgressCtrl::Create( dwStyle, CRect( 0, 0, 0, 0), 
      pStatusBar, 1) );
   if( !m_bSuccess ) 
      return FALSE;

   // Set range and step
   SetRange( 0, MaxValue );
   SetStep( 1 );

   m_pane = Pane;
   
   // Resize the control to its desired width
   Resize();

   return TRUE;
}

void CProgressBar::Clear()
{
   // Hide the window. This is necessary so that a cleared
   // window is not redrawn if "Resize" is called
   ModifyStyle( WS_VISIBLE, 0 );

   CStatusBar *pStatusBar = GetStatusBar();
   
   if( pStatusBar ) 
      pStatusBar->RedrawWindow();
}

void CProgressBar::SetRange( int nLower, int nUpper, int nStep )
{
   if( !m_bSuccess ) 
      return;
   CProgressCtrl::SetRange( nLower, nUpper );
   CProgressCtrl::SetStep( nStep );
}

int CProgressBar::SetPos( int nPos )
{
   ModifyStyle( 0, WS_VISIBLE );
   return( ( m_bSuccess )? CProgressCtrl::SetPos( nPos ) : 0 );
}

int  CProgressBar::OffsetPos( int nPos )
{ 
   ModifyStyle( 0, WS_VISIBLE );
   return( ( m_bSuccess ) ? CProgressCtrl::OffsetPos( nPos ) : 0 );
}

int  CProgressBar::SetStep( int nStep )
{ 
   ModifyStyle( 0, WS_VISIBLE );
   return( ( m_bSuccess ) ? CProgressCtrl::SetStep(nStep) : 0 );	
}

int  CProgressBar::StepIt()			 
{ 
    ModifyStyle( 0, WS_VISIBLE );
    return( ( m_bSuccess ) ? CProgressCtrl::StepIt() : 0 );
}

void CProgressBar::Resize() 
{	
    CStatusBar *pStatusBar = GetStatusBar();
    if( !pStatusBar ) 
      return;

    // Now calculate the rectangle in which we will draw the progress bar
    CRect rc;
    pStatusBar->GetItemRect( m_pane , rc );
    
    // Leave a litle vertical margin (10%) between the top and bottom of the bar
    int Height = rc.bottom - rc.top;
    rc.bottom -= Height/10;
    rc.top    += Height/10;

    // Resize the window
    if (::IsWindow(m_hWnd) && (rc != m_Rect))
        MoveWindow(&rc);

	m_Rect = rc;
}

BOOL CProgressBar::OnEraseBkgnd(CDC* pDC) 
{
    Resize();
    return CProgressCtrl::OnEraseBkgnd(pDC);
}