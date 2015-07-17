// Splitter.cpp: implementation of the CSplitter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "thred.h"
#include "Splitter.h"
#include "threddoc.h"
#include "thredview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSplitter

IMPLEMENT_DYNCREATE(CSplitter, CSplitterWnd)

BEGIN_MESSAGE_MAP(CSplitter, CSplitterWnd)
	//{{AFX_MSG_MAP(CSplitter)
   ON_WM_KILLFOCUS()
   ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSplitter::CSplitter()
{
	Created = FALSE;
	SplitX = 0.5;
	SplitY = 0.5;
}

CSplitter::~CSplitter()
{

}

// set the positions for the spliting of the window
void CSplitter::SetSplitters( double SplitX, double SplitY )
{
   // set the private members
   SplitX = SplitX;
   SplitY = SplitY;
   
   // update them
   UpdateSplitters();
}

// update the splitters positions and move
void CSplitter::UpdateSplitters( void )
{
   CRect rect;     
   
   GetClientRect( &rect );
   
   SetColumnInfo( 0, int( rect.right * SplitX ) , 10 );
   SetRowInfo( 0, int( rect.bottom * SplitY ), 10 );
   RecalcLayout();
}


// adjust back to 50% for each split
void CSplitter::OnLButtonDblClk( UINT flags, CPoint point )
{
	SplitX = 0.5;
	SplitY = 0.5;
   
   // update them
   UpdateSplitters();
}

// get the new split values
void CSplitter::OnKillFocus( CWnd * pWnd )
{
   if( Created )
   {
      CRect clientrect;
      int current, min;
   
      GetClientRect( &clientrect );
   
      GetColumnInfo( 0, current, min );
      if( current && clientrect.right )
	      SplitX = ( double )( double( current ) / double( clientrect.right ) );
      GetRowInfo( 0, current, min );
      if( current && clientrect.bottom ) 
	      SplitY = ( double )( double( current ) / double( clientrect.bottom ) );
   }
   CSplitterWnd::OnKillFocus( pWnd );
}


