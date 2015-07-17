// GroupView.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "GroupView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupView

CGroupView::CGroupView()
{
}

CGroupView::~CGroupView()
{
}


BEGIN_MESSAGE_MAP(CGroupView, CControlBar)
	//{{AFX_MSG_MAP(CGroupView)
	ON_WM_PAINT()
   ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP                    
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupView message handlers


CSize CGroupView::CalcDynamicLayout( int nLength, DWORD dwMode )
{
   CRect rect;
   CSize size;
   
   GetClientRect( rect );

   // check for floating
   if( IsFloating() )
   {  
      CWnd * pParent;
                                
      // get the parent window
      pParent = GetParent();
      
      // set the window dimensions for the tree
      size.cy -= CaptionSize;   
      m_TreeWnd.SetWindowPos( NULL, 0, 0, rect.right, rect.bottom, SWP_NOZORDER );
      
      EnableDocking( CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT );
   }
   else
      m_TreeWnd.SetWindowPos( NULL, 0, CaptionSize, rect.right, rect.bottom - 
         CaptionSize, SWP_NOZORDER );
   
   size.cx = CurrentSize.cx;
   size.cy = CurrentSize.cy;
   return( size );
}


void CGroupView::OnUpdateCmdUI( CFrameWnd * pFrame, BOOL fDisable )
{
}

void CGroupView::OnPaint() 
{
   CPaintDC dc(this);
   
   // draw the caption which is used in non-float mode to grab the thing
   if( !IsFloating() )
   {
      CRect rect;
      rect.left = 0;
      rect.top = 0;
      rect.right = CurrentSize.cx;
      rect.bottom = CaptionSize;

      CBrush brush;
      brush.CreateSolidBrush( 0x00000000 );
      dc.FillRect( &rect, &brush );  
      dc.SetTextColor( 0x00ffffff );
      
      COLORREF color;
      color = dc.GetBkColor();
      dc.SetBkColor( 0x00000000 );

      CString Title;
      GetWindowText( Title );
      dc.DrawText( Title, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
      dc.SetBkColor( color );
   }
   
   CControlBar::OnPaint();
}

void CGroupView::OnWindowPosChanged( WINDOWPOS* lpwndpos )
{
   // check for floating
   if( IsFloating() )
   {
   }
}

//////////////////////////////////////////////////////////////////////////
// tree control

BEGIN_MESSAGE_MAP(CGroupViewTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CGroupViewTree)
  	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemexpanding )
  	ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnSelchanging )
  	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag )
  	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit )
   ON_WM_CAPTURECHANGED()
   ON_WM_KEYDOWN()
   ON_WM_RBUTTONDOWN()
   ON_WM_LBUTTONUP()
   ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP                    
END_MESSAGE_MAP()

// construction
CGroupViewTree::CGroupViewTree()
{
   m_fDragging = FALSE;
   pDragList = NULL;
}

// check for change of capture
void CGroupViewTree::OnCaptureChanged( CWnd* pWnd )
{
   if( m_fDragging && ( pWnd != this ) )
      CancelDrag();
}


// key down
void CGroupViewTree::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
   if( m_fDragging && ( nChar == VK_ESCAPE ) )
      CancelDrag();
      
}

// right mouse button down
void CGroupViewTree::OnRButtonDown( UINT nFlags, CPoint point )
{
   if( m_fDragging )
   {
      CancelDrag();
      return;
   }
   
//      HTREEITEM   hItem;
//      UINT        flags;
//   
//      SetFocus();
//   
//      // select this item
//      hItem = HitTest( point, &flags );
//   
//      if( !( flags & TVHT_ONITEM ) )
//         return;
//         
//      SelectItem( hItem );
   
   // bring up a popup menu
   CTreeCtrl::OnRButtonDown( nFlags, point );
}

void CGroupViewTree::OnEndlabeledit( NMHDR * pNMHDR, LRESULT * pResult )
{
   TV_DISPINFO * pInfo;
   
   // get a ptr to the info
   pInfo = ( TV_DISPINFO * )pNMHDR;
   
   // check if need to update the text for this item
   if( pInfo->item.pszText && strlen( pInfo->item.pszText ) )
      *pResult = TRUE;   
}

void CGroupViewTree::OnBegindrag( NMHDR* pNMHDR, LRESULT* pResult )
{
   HTREEITEM      hItem;
   NM_TREEVIEW *  pTreeView;
   
   pTreeView = ( NM_TREEVIEW * )pNMHDR;
   
   hItem = pTreeView->itemNew.hItem;
   
   // set the flag to indicate we are draging
   m_fDragging = TRUE;

   // create the drag list
   pDragList = CreateDragImage( hItem );

   RedrawWindow();
   
   RECT rect;
   GetItemRect( hItem, &rect, TRUE );
   
   // add imagelist width + 4 pixels
   ImageList_BeginDrag( pDragList->GetSafeHandle(), 0, 
      pTreeView->ptDrag.x - rect.left + 24, 
      pTreeView->ptDrag.y - rect.top );
 
   ImageList_DragEnter( this->m_hWnd, pTreeView->ptDrag.x, 
      pTreeView->ptDrag.y );
   
   m_hTopItem = GetFirstVisibleItem();
   m_hSelItem = GetSelectedItem();
   m_hDropItem = NULL;
   
   // capture the mouse
   SetCapture();
}

// mouse move
void CGroupViewTree::OnMouseMove( UINT nFlags, CPoint point )
{
   HTREEITEM    hTarget;
   UINT         flags;
   HCURSOR      hCurrentCursor;
    
   if( m_fDragging )
   {  
      // perform a hit test  
      hTarget = HitTest( point, &flags );
      
      hCurrentCursor = GetCursor();
      
      // check if over an item
      if( !( flags & TVHT_ONITEM ) )
      {
         if( hCurrentCursor != LoadCursor( NULL, IDC_NO ) )
            SetCursor( LoadCursor( NULL, IDC_NO ) );

         if( m_hDropItem )
         {
            TreeView_SelectDropTarget( this->m_hWnd, NULL );
            ImageList_DragLeave( this->m_hWnd );
            RedrawWindow();
            ImageList_DragEnter( this->m_hWnd, point.x, point.y );
            m_hDropItem = NULL;
         }
      }
      else
      {
         if( hTarget != m_hDropItem )
         {
            TreeView_SelectDropTarget( this->m_hWnd, hTarget );
            ImageList_DragLeave( this->m_hWnd );
            RedrawWindow();
            ImageList_DragEnter( this->m_hWnd, point.x, point.y );
         }
         
         m_hDropItem = hTarget;

         if( hCurrentCursor != LoadCursor( NULL, IDC_ARROW ) )
            SetCursor( LoadCursor( NULL, IDC_ARROW ) );
      }         

      // check for a change in top position
      if( ( m_hTopItem != GetFirstVisibleItem() ) || 
         ( m_hSelItem != GetSelectedItem() ) )
      {
         // set this
         m_hTopItem = GetFirstVisibleItem();
         m_hSelItem = GetSelectedItem();
         ImageList_DragLeave( this->m_hWnd );
         RedrawWindow();
         ImageList_DragEnter( this->m_hWnd, point.x, point.y );
      }
   
      // Drag the item to the current position of the mouse cursor. 
      ImageList_DragMove( point.x, point.y );
   }
}

// cancel a drag operation
void CGroupViewTree::CancelDrag( void )
{
   if( !m_fDragging )
      return;
      
   m_fDragging = FALSE;

   // release the mouse
   ReleaseCapture();
   
   ImageList_EndDrag();
   ImageList_DragLeave( this->m_hWnd );
   TreeView_SelectDropTarget( this->m_hWnd, NULL );

   delete pDragList;
   
   RedrawWindow();
}

// left mouse button up
void CGroupViewTree::OnLButtonUp( UINT nFlags, CPoint point )
{
   if( m_fDragging )
   {
      m_fDragging = FALSE;
   
      if( !m_hDropItem  )
         MessageBeep( MB_ICONASTERISK );
         
      ImageList_EndDrag();
      ImageList_DragLeave( this->m_hWnd );
      TreeView_SelectDropTarget( this->m_hWnd, NULL );
  
      delete pDragList;

      // release the mouse
      ReleaseCapture();
      
      RedrawWindow();
   }
}

void CGroupViewTree::OnItemexpanding( NMHDR* pNMHDR, LRESULT* pResult )
{
}

void CGroupViewTree::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult)
{
   // check if dragging
   if( m_fDragging )
      RedrawWindow();
}
