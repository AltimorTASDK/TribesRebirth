// ObjectView.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "ObjectView.h"
#include "MainFrm.h"
#include "childfrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CObjectView

CObjectView::CObjectView()
{
}

CObjectView::~CObjectView()
{
}


BEGIN_MESSAGE_MAP(CObjectView, CControlBar)
	//{{AFX_MSG_MAP(CObjectView)
	ON_WM_PAINT()
   ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP                    
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectView message handlers


CSize CObjectView::CalcDynamicLayout( int nLength, DWORD dwMode )
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


void CObjectView::OnUpdateCmdUI( CFrameWnd * pFrame, BOOL fDisable )
{
}

void CObjectView::OnPaint() 
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

void CObjectView::OnWindowPosChanged( WINDOWPOS* lpwndpos )
{
   // check for floating
   if( IsFloating() )
   {
   }
}

//////////////////////////////////////////////////////////////////////////
// tree control

BEGIN_MESSAGE_MAP(CObjectViewTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CObjectViewTree)
  	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemexpanding )
  	ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnSelchanging )
  	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag )
   ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit )   
  	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit )
   ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnGetdispinfo )
   ON_WM_CAPTURECHANGED()
   ON_WM_KEYDOWN()
   ON_WM_RBUTTONDOWN()
   ON_WM_LBUTTONUP()
   ON_WM_LBUTTONDBLCLK()
   ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP                    
END_MESSAGE_MAP()

// construction
CObjectViewTree::CObjectViewTree()
{
   m_fDragging = FALSE;
   pDragList = NULL;
}

// check for change of capture
void CObjectViewTree::OnCaptureChanged( CWnd* pWnd )
{
   if( m_fDragging && ( pWnd != this ) )
      CancelDrag();
}


// key down
void CObjectViewTree::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
   const int sizeAmount = 10;
   const int minHeight = 100;
   const int minWidth = 50;
   
   if( m_fDragging && ( nChar == VK_ESCAPE ) )
      CancelDrag();
      
   // process this click
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   
   // check for ctrl ( size window by x );
   if( GetKeyState( VK_CONTROL ) & 0x8000 )
   {
      // call recalclayout twice because it saves code prob's
      switch( nChar )
      {
         case VK_LEFT:
            if( pFrame->m_wndObjectView.CurrentSize.cx > minWidth )
               pFrame->m_wndObjectView.CurrentSize.cx -= sizeAmount;
            pFrame->RecalcLayout();
            pFrame->RecalcLayout();
            pFrame->m_wndObjectView.RedrawWindow();
            return;
            break;
         case VK_RIGHT:
            pFrame->m_wndObjectView.CurrentSize.cx += sizeAmount;
            pFrame->RecalcLayout();
            pFrame->RecalcLayout();
            pFrame->m_wndObjectView.RedrawWindow();
            return;
            break;
         case VK_UP:
            if( pFrame->m_wndObjectView.CurrentSize.cy > minHeight )
               pFrame->m_wndObjectView.CurrentSize.cy -= sizeAmount;
            pFrame->RecalcLayout();
            pFrame->RecalcLayout();
            pFrame->m_wndObjectView.RedrawWindow();
            return;
            break;
         case VK_DOWN:
            pFrame->m_wndObjectView.CurrentSize.cy += sizeAmount;
            pFrame->RecalcLayout();
            pFrame->RecalcLayout();
            pFrame->m_wndObjectView.RedrawWindow();
            return;
            break;
      }
         
   }
   CTreeCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
}

// right mouse button down
void CObjectViewTree::OnRButtonDown( UINT nFlags, CPoint point )
{
   if( m_fDragging )
   {
      CancelDrag();
      return;
   }
   
   HTREEITEM   hItem;
   UINT        flags;

   SetFocus();

   // select this item
   hItem = HitTest( point, &flags );

   if( hItem )
      SelectItem( hItem );
      
   // process this click
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   pFrame->m_wndObjectView.RightClick( hItem );
   
   // bring up a popup menu
   CTreeCtrl::OnRButtonDown( nFlags, point );
}

void CObjectViewTree::OnEndlabeledit( NMHDR * pNMHDR, LRESULT * pResult )
{
   TV_DISPINFO * pInfo;
   
   // get a ptr to the info
   pInfo = ( TV_DISPINFO * )pNMHDR;
   
   // check if need to update the text for this item
   if( pInfo->item.pszText && strlen( pInfo->item.pszText ) )
   {
      int Image, SelectImage;
   
      // get the image index
      GetItemImage( pInfo->item.hItem, Image, SelectImage );
         
      CShapeDetail * pDetail;
      CShapeState * pState;
      CLightSet * pLightSet;
      
      switch( Image )
      {
         case CObjectView::STATE:
            // update the state and the titles if there is a detail
            pState = ( CShapeState * )GetItemData( pInfo->item.hItem );
            if( !pState )
               *pResult = FALSE;
            else
            {
               pState->Name = pInfo->item.pszText;
               *pResult = TRUE;
            }
            
            pDetail = pState->mDetails;
            if( pDetail )
               pDetail->UpdateTitles();
            break;
          
         case CObjectView::DETAIL:
         case CObjectView::DETAIL_OFF:
         
            // get the detail ptr and assign the name to it
            pDetail = ( CShapeDetail * )GetItemData( pInfo->item.hItem );
            if( !pDetail )
               *pResult = FALSE;
            else
            {
               pDetail->Name = pInfo->item.pszText;
               pDetail->UpdateTitles();
               
               *pResult = TRUE;   
            }
            break;
            
         case CObjectView::LIGHT_SET:
         case CObjectView::LIGHT_SET_OFF:
         
            // rename the lightset
            pLightSet = ( CLightSet * )GetItemData( pInfo->item.hItem );
            if( !pLightSet )
               *pResult = FALSE;
            else
            {
               pLightSet->Name = pInfo->item.pszText;
               *pResult = TRUE;
            }
                  
            break;
      }
   }
}

void CObjectViewTree::OnBeginlabeledit( NMHDR * pNMHDR, LRESULT * pResult )
{
   TV_DISPINFO *  pInfo;
   HTREEITEM      hItem;
   
   pInfo = ( TV_DISPINFO* )pNMHDR;
   
   SetFocus();
   
   hItem = pInfo->item.hItem;
   
   int Image, SelectImage;
   
   GetItemImage( hItem, Image, SelectImage );

   // ok to edti for non-details
   if( ( Image == CObjectView::DETAIL ) || 
      ( Image == CObjectView::DETAIL_OFF ) )
      *pResult = TRUE;
   else
      *pResult = FALSE;
}

void CObjectViewTree::OnBegindrag( NMHDR* pNMHDR, LRESULT* pResult )
{                             
   return;
// dragging is no longer used
//   HTREEITEM      hItem;
//   NM_TREEVIEW *  pTreeView;
//   
//   pTreeView = ( NM_TREEVIEW * )pNMHDR;
//   
//   hItem = pTreeView->itemNew.hItem;
//   
//   SelectItem( hItem );
//   m_hSelItem = hItem;
//      
//   // set the flag to indicate we are draging
//   m_fDragging = TRUE;
//
//   // create the drag list
//   pDragList = CreateDragImage( hItem );
//
//   RedrawWindow();
//   
//   RECT rect;
//   GetItemRect( hItem, &rect, TRUE );
//   
//   // add imagelist width + 4 pixels
//   ImageList_BeginDrag( pDragList->GetSafeHandle(), 0, 
//      pTreeView->ptDrag.x - rect.left + 24, 
//      pTreeView->ptDrag.y - rect.top );
// 
//   ImageList_DragEnter( this->m_hWnd, pTreeView->ptDrag.x, 
//      pTreeView->ptDrag.y );
//   
//   m_hTopItem = GetFirstVisibleItem();
//   m_hSelItem = GetSelectedItem();
//   m_hDropItem = NULL;
//   
//   // capture the mouse
//   SetCapture();
}

// mouse move
void CObjectViewTree::OnMouseMove( UINT nFlags, CPoint point )
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

         CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
         
         if( pFrame->m_wndObjectView.UseDropTarget( hTarget ) )
         {
            m_hDropItem = hTarget;
            if( hCurrentCursor != LoadCursor( NULL, IDC_ARROW ) )
               SetCursor( LoadCursor( NULL, IDC_ARROW ) );
         }         
         else
         {
            m_hDropItem = NULL;
            if( hCurrentCursor != LoadCursor( NULL, IDC_NO ) )
               SetCursor( LoadCursor( NULL, IDC_NO ) );
         }
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
void CObjectViewTree::CancelDrag( void )
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
void CObjectViewTree::OnLButtonUp( UINT nFlags, CPoint point )
{
   if( m_fDragging )
   {
      m_fDragging = FALSE;
   
      ImageList_EndDrag();
      ImageList_DragLeave( this->m_hWnd );
      TreeView_SelectDropTarget( this->m_hWnd, NULL );
  
      delete pDragList;

      // release the mouse
      ReleaseCapture();
      
      CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
      
      // check for drop potential
      if( !m_hDropItem  )
         MessageBeep( MB_ICONASTERISK );
      else if( pFrame->m_wndObjectView.UseDropTarget( m_hDropItem ) )
         pFrame->m_wndObjectView.MoveItem( m_hSelItem, m_hDropItem );
      
      RedrawWindow();
   }
}

BOOL CObjectView::MoveItem( HTREEITEM hSrc, HTREEITEM hDest )
{
   int Image, SelectImage;
   CShapeState * pState;
   CShapeDetail * pDetail;
   CLightSet * pLightSet;
   
   m_TreeWnd.GetItemImage( hDest, Image, SelectImage );

   switch( Image )
   {
      case STATE:
         pState = ( CShapeState * )m_TreeWnd.GetItemData( hDest );
         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hSrc );
         
         // remove from the linked list
         if( pDetail->pPrevious )
            pDetail->pPrevious->pNext = pDetail->pNext;
         if( pDetail->pNext )
            pDetail->pNext->pPrevious = pDetail->pPrevious;
         if( pDetail == pDetail->pState->mDetails )
            pDetail->pState->mDetails = pDetail->pNext;
                  
         // remove from the object viewer
         RemoveDetail( pDetail );
         
         // attatch to the top of the other linked list
         if( !pState->mDetails )
         {
            pState->mDetails = pDetail;
            pDetail->pNext = NULL;
            pDetail->pPrevious = NULL;
         }
         else
         {
            pDetail->pPrevious = NULL;
            pDetail->pNext = pState->mDetails->pNext;
            pState->mDetails = pDetail;
         }
         
         pDetail->pState = pState;
         
         // add to the object viewer
         AddDetail( pState, pDetail );
         
         // re-add the lightsets
         pLightSet = pDetail->mLightSets;
         while( pLightSet )
         {
            AddLightSet( pDetail, pLightSet );
            pLightSet = pLightSet->pNext;
         }
         
         break;
         
      case DETAIL:
      case DETAIL_OFF:
         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hDest );
         pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hSrc );
         
         if( pLightSet->pPrevious )
            pLightSet->pPrevious->pNext = pLightSet->pNext;
         if( pLightSet->pNext )
            pLightSet->pNext->pPrevious = pLightSet->pPrevious;
         if( pLightSet == pLightSet->pDetail->mLightSets )
            pLightSet->pDetail->mLightSets = pLightSet->pNext;
                  
         // remove from the object viewer
         RemoveLightSet( pLightSet );
         
         // attatch to the top of the other linked list
         if( !pDetail->mLightSets )
         {
            pDetail->mLightSets = pLightSet;
            pLightSet->pNext = NULL;
            pLightSet->pPrevious = NULL;
         }
         else
         {
            pLightSet->pPrevious = NULL;
            pLightSet->pNext = pDetail->mLightSets->pNext;
            pDetail->mLightSets = pLightSet;
         }
         
         pLightSet->pDetail = pDetail;
         
         // add to the object viewer
         AddLightSet( pDetail, pLightSet );
         
         break;
         
      default:
         return( FALSE );
         break;
   }

   pDetail->UpdateTitles();
   pDetail->pState->pDoc->UpdateAllViews( NULL );
   pDetail->pState->pDoc->SetModifiedFlag();
   
   return( TRUE );
}

void CObjectViewTree::OnLButtonDblClk( UINT nFlags, CPoint point )
{
   UINT        flags;
   HTREEITEM   hItem;

   SetFocus();

   // select this item
   hItem = HitTest( point, &flags );

   if( hItem )
   {
      int Image, SelectImage;
      
      GetItemImage( hItem, Image, SelectImage );

      switch( Image )
      {
         case CObjectView::LIGHT_SET:
         case CObjectView::LIGHT_SET_OFF:
         
            CLightSet * pLightSet;

            // get the pointer to this state
            pLightSet = ( CLightSet * )GetItemData( hItem );
            if( pLightSet->pDetail->pState->pDoc->pActiveDetail !=
               pLightSet->pDetail )
               break;
            
            pLightSet->mActive ^= 1;
            pLightSet->SetActive( pLightSet->mActive );
            
            break;      

         case CObjectView::DETAIL:
         case CObjectView::DETAIL_OFF:
            CShapeDetail * pDetail;
            pDetail = ( CShapeDetail * )GetItemData( hItem );
            pDetail->SetActive();
            break;
            
         default:
            CTreeCtrl::OnLButtonDblClk( nFlags, point );
            break;
      }
   }
   else
      CTreeCtrl::OnLButtonDblClk( nFlags, point );
}

void CObjectViewTree::OnGetdispinfo( NMHDR * pNMHDR, LRESULT * pResult )
{
}

void CObjectViewTree::OnItemexpanding( NMHDR* pNMHDR, LRESULT* pResult )
{
}

void CObjectViewTree::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult)
{
   // check if dragging
   if( m_fDragging )
      RedrawWindow();
}


// add a state to the tree
BOOL CObjectView::AddState( CShapeState * pState )
{
   HTREEITEM hItem;   
   
   if( !pState )
      return( FALSE );
      
   // add to the tree view
   hItem = m_TreeWnd.InsertItem( pState->Name, STATE, STATE );
   
   // set the item info
   pState->hItem = hItem;
   
   // set this as the data used
   m_TreeWnd.SetItemData( hItem, ( DWORD )pState );

   m_TreeWnd.RedrawWindow();
   
   return( TRUE );
}

BOOL CObjectView::RemoveState( CShapeState * pState )
{
   if( pState->hItem )
   {
      m_TreeWnd.DeleteItem( pState->hItem );
      m_TreeWnd.RedrawWindow();
   }
   return( TRUE );
}

BOOL CObjectView::AddDetail( CShapeState * pParent, CShapeDetail * pDetail )
{
   if( !pParent || !pDetail )
      return( FALSE );
      
   HTREEITEM hParent;
   
   // get the parent item
   hParent = pParent->hItem;
   
   HTREEITEM hItem;
   
   // add to the list
   if( pDetail->pState && ( pDetail->pState->pDoc->pActiveDetail == pDetail ) )
      hItem = m_TreeWnd.InsertItem( pDetail->Name, DETAIL, DETAIL, hParent );
   else
      hItem = m_TreeWnd.InsertItem( pDetail->Name, DETAIL_OFF, DETAIL_OFF, hParent );
   
   // set the item info
   pDetail->hItem = hItem;
   m_TreeWnd.SetItemData( hItem, ( DWORD )pDetail );
   
   m_TreeWnd.RedrawWindow();
   return( TRUE );
}

BOOL CObjectView::RemoveDetail( CShapeDetail * pDetail )
{
   if( pDetail->hItem )
   {
      m_TreeWnd.DeleteItem( pDetail->hItem );
      m_TreeWnd.RedrawWindow();
   }
   return( TRUE );
}

BOOL CObjectView::AddLightSet( CShapeDetail * pParent, CLightSet * pLightSet )
{
   if( !pParent || !pLightSet )
      return( FALSE );
      
   // get the parent item
   HTREEITEM hParent = pParent->hItem;

   int Image;
   
   if( pLightSet->mActive )
      Image = LIGHT_SET;
   else
      Image = LIGHT_SET_OFF;   
   HTREEITEM hItem = m_TreeWnd.InsertItem( pLightSet->Name, Image, Image, hParent );
   m_TreeWnd.SetItemData( hItem, ( DWORD )pLightSet );

   pLightSet->hItem = hItem;
      
   m_TreeWnd.RedrawWindow();
   return( TRUE );
}

void CObjectView::Enable( BOOL fState )
{
   if( fState )
   {
      m_Enabled = TRUE;
   }
   else
   {
      m_Enabled = FALSE;
   
      /// clear it
      m_TreeWnd.DeleteAllItems();
      m_TreeWnd.RedrawWindow();
   }
}

BOOL CObjectView::RemoveLightSet( CLightSet * pLightSet )
{
   if( pLightSet->hItem )
   {
      m_TreeWnd.DeleteItem( pLightSet->hItem );
      m_TreeWnd.RedrawWindow();
   }
   return( TRUE );
}

// check if we should use the drop target
BOOL CObjectView::UseDropTarget( HTREEITEM hTarget )
{
   int Image1, Image2, SelectImage;
   
   // get the image type and refuse any like ones off the bat
   m_TreeWnd.GetItemImage( hTarget, Image1, SelectImage );
   m_TreeWnd.GetItemImage( m_TreeWnd.m_hSelItem, Image2, SelectImage );

   // now just do the approvals      image1 = target, 2 = src
   if( ( Image1 == STATE ) && ( Image2 == DETAIL ) )
   {
      CShapeDetail * pDetail;
      pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( m_TreeWnd.m_hSelItem );
      
      CShapeState * pState;
      pState = ( CShapeState * )m_TreeWnd.GetItemData( hTarget );
      
      if( pState == pDetail->pState )
         return( FALSE );
               
      return( TRUE );
   }
   if( ( Image1 == DETAIL ) && ( ( Image2 == LIGHT_SET ) || ( Image2 == LIGHT_SET_OFF ) ) )
   {
      CShapeDetail * pDetail;
      pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hTarget );

      CLightSet * pLightSet;
      pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( m_TreeWnd.m_hSelItem );
      
      if( pDetail == pLightSet->pDetail )
         return( FALSE );
         
      return( TRUE );
   }
    
   // dont use this target  
   return( FALSE );
}

// rebuild the tree view
BOOL CObjectView::Rebuild( CTHREDDoc * pDoc )
{
   // remove all the items in the list
   m_TreeWnd.DeleteAllItems();

   // go through all the toplevel items
   CShapeState * pState;
   pState = pDoc->m_StateList;
   while( pState )
   {
      // add
      AddState( pState );
      
      // go through all the details
      CShapeDetail * pDetail;
      pDetail = pState->mDetails;
      while( pDetail )
      {
         // add
         AddDetail( pState, pDetail );
         
         // go through all the lightstates
         CLightSet * pLightSet;
         pLightSet = pDetail->mLightSets;
         while( pLightSet )
         {
            AddLightSet( pDetail, pLightSet );
            pLightSet = pLightSet->pNext;
         }
         pDetail = pDetail->pNext;
      }
      
      pState = pState->pNext;
   }
  
   // return success
   return( TRUE );
}


BOOL CObjectView::RightClick( HTREEITEM hItem )
{
   int      Flags;
   int      Image, SelectImage;
   int      value;
   CShapeDetail * pDetail;
   CShapeState * pState;
   CLightSet * pLightSet;
   CMenu    menu;
   CPoint   point;

   // try and get a document ptr, if cant return false
	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
   CChildFrame * pChild = ( CChildFrame * )pFrame->MDIGetActive();
   if( !pChild )
      return( FALSE );
   CTHREDDoc * pDoc = pChild->pCurrentDetail->pState->pDoc;
   if( !pDoc )
      return( FALSE );
      
   GetCursorPos( &point );

   // check that it hit an item
   if( !hItem )
   {
      // create a popup menu         
      menu.CreatePopupMenu();
      menu.AppendMenu( MF_STRING, 1, "Add State" );
      
      Flags = MF_STRING;
      // check for paste ability
      if( !pDoc->m_ObjectEdit.pState )
         Flags |= MF_GRAYED;
      menu.AppendMenu( Flags, 2, "Paste State" );
      menu.AppendMenu( MF_STRING, 3, "Properties..." );
      
      value = menu.TrackPopupMenu( TPM_RETURNCMD | TPM_CENTERALIGN |
         TPM_RIGHTBUTTON, point.x, point.y, this, NULL );
         
      switch( value )
      {
         case 1:
            pDoc->AddState( "" );
            
            break;
            
         case 2:
         
            // add the state
            pDoc->PasteState();
            
            break;

         case 3:
            
            // do the properties
            pDoc->Properties();
            
            break;            
            
         default:
            return( FALSE );
      }
      
      return( TRUE );
   }
      
   // determine the type of item hit
   m_TreeWnd.GetItemImage( hItem, Image, SelectImage );
   
   switch( Image )
   {
      case STATE:
         // get the pointer to this state
         pState = ( CShapeState * )m_TreeWnd.GetItemData( hItem );

         // create a popup menu         
         menu.CreatePopupMenu();
         menu.AppendMenu( MF_STRING, 1, "Add Detail" );
         menu.AppendMenu( MF_STRING, 2, "Remove" );
         menu.AppendMenu( MF_STRING, 3, "Copy" );
         menu.AppendMenu( MF_SEPARATOR );
         menu.AppendMenu( MF_STRING, 4, "Import detail..." );
         menu.AppendMenu( MF_SEPARATOR );
         
         
         Flags = MF_STRING;
         if( !pDoc->m_ObjectEdit.pDetail )
            Flags |= MF_GRAYED;
         menu.AppendMenu( Flags, 5, "Paste Detail" );
         menu.AppendMenu( MF_GRAYED | MF_STRING, 6, "Settings..." );
         value = menu.TrackPopupMenu( TPM_RETURNCMD | TPM_CENTERALIGN |
            TPM_RIGHTBUTTON, point.x, point.y, this, NULL );
            
         switch( value )
         {
            case 1:
               pDetail = pState->AddDetail( "" );
               
               break;
               
            case 2:
                  
               if( ( pDoc->NumDetails() <= 1 ) && ( pState->mDetails ) )
               {
                  AfxMessageBox("Cannot remove state, zed requires at least \n"
                                 "one detail in the document.", MB_OK, 0);
                  return( FALSE );
               }           
               
               if( AfxMessageBox( "This action cannot be undone.  Continue?",
                  MB_YESNO, 0 ) == IDYES )
               {
                  pState->pDoc->RemoveState( pState );
               }
               
               break;
               
            case 3:
            
               pState->pDoc->CopyState( pState );
               break;
               
            case 4:
               
               pState->ImportDetail();
               
               break;
               
            case 5:
            
               // paste the detail
               pDetail = pState->PasteDetail();
               
               break;
               
            case 6:
               break;
               
            default:
               return( FALSE );
         }
         
         break;
         
      case DETAIL:
      case DETAIL_OFF:
      
         // get the pointer to this detail
         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hItem );

         // create a popup menu         
         menu.CreatePopupMenu();
         menu.AppendMenu( MF_STRING, 1, "Set active" );
//         menu.AppendMenu( MF_STRING, 2, "Add LightSet" );
//
//         Flags = MF_STRING;
//         if( !pDoc->m_ObjectEdit.pLightSet )
//            Flags |= MF_GRAYED;
//         menu.AppendMenu( Flags, 3, "Paste LightSet" );
         menu.AppendMenu( MF_STRING, 4, "Remove" );
         menu.AppendMenu( MF_STRING, 5, "Copy" );
         menu.AppendMenu( MF_STRING, 6, "Settings..." );
         value = menu.TrackPopupMenu( TPM_RETURNCMD | TPM_CENTERALIGN |
            TPM_RIGHTBUTTON, point.x, point.y, this, NULL );
            
         switch( value )
         {
            case 1:
               pDetail->SetActive();
               break;
               
            case 2:
               pDetail->AddLightSet( "" );
               break;
            
            case 3:
               pDetail->PasteLightSet();
               break;
               
            case 4:

               if( pDoc->NumDetails() <= 1 )
               {            
                     AfxMessageBox("Cannot remove detail, zed requires at least \n"
                     "one detail in the document.", MB_OK, 0);
                     return( FALSE );
               }

               // prompt user ( this is pretty final )
               if( AfxMessageBox( "This action cannot be undone.  Continue?",
                  MB_YESNO, 0 ) == IDYES )
               {
                  // remove the detail
                  pDetail->pState->RemoveDetail( pDetail );
               }
               
               break;
               
            case 5:
               
               // copy the the internal clibboard
               pDetail->pState->CopyDetail( pDetail );
               break;
               
            case 6:
               pDetail->DoSettings();
               
               break;
               
            default:
               return( FALSE );
         }
         
         break;
         
      case LIGHT_SET:
      case LIGHT_SET_OFF:
      
         // get the pointer to this state
         pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hItem );
   
         // create a popup menu         
         menu.CreatePopupMenu();
         Flags = MF_STRING;
         if( pDoc->pActiveDetail != pLightSet->pDetail )
            Flags |= MF_GRAYED;
         menu.AppendMenu( Flags, 1, "Toggle active" );
         menu.AppendMenu( MF_STRING, 2, "Remove" );
         menu.AppendMenu( MF_STRING, 3, "Copy" );
         menu.AppendMenu( MF_STRING | MF_GRAYED, 4, "Settings..." );
         value = menu.TrackPopupMenu( TPM_RETURNCMD | TPM_CENTERALIGN |
            TPM_RIGHTBUTTON, point.x, point.y, this, NULL );
            
         switch( value )
         {
            case 1:
               if( pLightSet->mActive )
                  pLightSet->SetActive( FALSE );
               else
                  pLightSet->SetActive( TRUE );
               break;
               
            case 2:
            
               // prompt user ( this is pretty final )
               if( AfxMessageBox( "This action cannot be undone.  Continue?",
                  MB_YESNO, 0 ) == IDYES )
               {
                  pLightSet->pDetail->RemoveLightSet( pLightSet );
               }
               break;
            
            case 3:
               // copy the the internal clibboard
               pLightSet->pDetail->CopyLightSet( pLightSet );
               break;
               
            case 4:
               break;
            
            default:
               return( FALSE );
         }
         
         break;
      
      default:
         return( FALSE );
   }
   
   return( TRUE );
}   

bool CObjectView::IsActive()
{
   if( GetFocus() == ( CWnd * )&m_TreeWnd )
      if( m_TreeWnd.GetSelectedItem() )
         return( TRUE );
   return( FALSE );
}

void CObjectView::CopyCurrent()
{
   HTREEITEM hItem;
   CShapeDetail * pDetail;
   CShapeState * pState;
   CLightSet * pLightSet;
   int      Image, SelectImage;
   
   hItem = m_TreeWnd.GetSelectedItem();

   if( !hItem )
      return;
      
   // determine the type of item
   m_TreeWnd.GetItemImage( hItem, Image, SelectImage );
   
   switch( Image )
   {
      case STATE:
         pState = ( CShapeState * )m_TreeWnd.GetItemData( hItem );
         pState->pDoc->CopyState( pState );
         break;
         
      case DETAIL:
      case DETAIL_OFF:
         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hItem );
         pDetail->pState->CopyDetail( pDetail );
         break;
         
      case LIGHT_SET:
      case LIGHT_SET_OFF:
         pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hItem );
         pLightSet->pDetail->CopyLightSet( pLightSet );
         break;
    }
}

void CObjectView::CutCurrent()
{
   HTREEITEM hItem;
   CShapeDetail * pDetail;
   CShapeState * pState;
   CLightSet * pLightSet;
   int      Image, SelectImage;
   
   hItem = m_TreeWnd.GetSelectedItem();

   if( !hItem )
      return;
      
   // determine the type of item
   m_TreeWnd.GetItemImage( hItem, Image, SelectImage );
   
   switch( Image )
   {
      case STATE:
         pState = ( CShapeState * )m_TreeWnd.GetItemData( hItem );
         pState->pDoc->CopyState( pState );
         
         if( pState->pDoc->NumDetails() <= 1 )
         {            
            AfxMessageBox("Cannot remove state, zed requires at least \n"
               "one detail in the document.", MB_OK, 0);
            return;
         }
         
         pState->pDoc->RemoveState( pState );
         break;
         
      case DETAIL:
      case DETAIL_OFF:
         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hItem );
         pDetail->pState->CopyDetail( pDetail );
         
         if( pDetail->pState->pDoc->NumDetails() <= 1 )
         {            
            AfxMessageBox("Cannot remove detail, zed requires at least \n"
               "one detail in the document.", MB_OK, 0);
            return;
         }
         pDetail->pState->RemoveDetail( pDetail );
         break;
         
      case LIGHT_SET:
      case LIGHT_SET_OFF:
         pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hItem );
         pLightSet->pDetail->CopyLightSet( pLightSet );
         pLightSet->pDetail->RemoveLightSet( pLightSet );
         break;
    }
}
void CObjectView::DeleteCurrent()
{
   HTREEITEM hItem;
   CShapeDetail * pDetail;
   CShapeState * pState;
   CLightSet * pLightSet;
   int      Image, SelectImage;
   
   hItem = m_TreeWnd.GetSelectedItem();

   if( !hItem )
      return;
      
   // prompt user ( this is pretty final )
   if( AfxMessageBox( "This action cannot be undone.  Continue?",
      MB_YESNO, 0 ) != IDYES )
      return;
      
   // determine the type of item
   m_TreeWnd.GetItemImage( hItem, Image, SelectImage );
   
   switch( Image )
   {
      case STATE:
         pState = ( CShapeState * )m_TreeWnd.GetItemData( hItem );

         if( pState->pDoc->NumDetails() <= 1 )
         {            
            AfxMessageBox("Cannot remove state, zed requires at least \n"
               "one detail in the document.", MB_OK, 0);
            return;
         }
         
         pState->pDoc->RemoveState( pState );
         break;
         
      case DETAIL:
      case DETAIL_OFF:
         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hItem );
         if( pDetail->pState->pDoc->NumDetails() <= 1 )
         {            
            AfxMessageBox("Cannot remove detail, zed requires at least \n"
               "one detail in the document.", MB_OK, 0);
            return;
         }
         pDetail->pState->RemoveDetail( pDetail );
         break;
         
      case LIGHT_SET:
      case LIGHT_SET_OFF:
         pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hItem );
         pLightSet->pDetail->RemoveLightSet( pLightSet );
         break;
    }
}

void CObjectView::PasteCurrent()
{
   HTREEITEM hItem;
   CShapeDetail * pDetail;
   CShapeState * pState;
   CLightSet * pLightSet;
   int      Image, SelectImage;
   
   hItem = m_TreeWnd.GetSelectedItem();

   // get the document ptr
	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
   CChildFrame * pChild = ( CChildFrame * )pFrame->MDIGetActive();
   if( !pChild ) return;
   CTHREDDoc * pDoc = pChild->pCurrentDetail->pState->pDoc;
   if( !pDoc ) return;
   
   if( !hItem )
   {
      pDoc->PasteState();
      return;
   }
      
   // determine the type of item
   m_TreeWnd.GetItemImage( hItem, Image, SelectImage );
   
   switch( Image )
   {
      case STATE:
         // check if there is a state there
         if( pDoc->m_ObjectEdit.pState )
            pDoc->PasteState();
         else
         {
            pState = ( CShapeState * )m_TreeWnd.GetItemData( hItem );
            pState->PasteDetail();
         }
         break;
         
      case DETAIL:
      case DETAIL_OFF:
         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hItem );
   
         // check if there is a detail in the clip
         if( pDoc->m_ObjectEdit.pDetail )
            pDetail->pState->PasteDetail();
         else
            pDetail->PasteLightSet();
         break;
         
      case LIGHT_SET:
      case LIGHT_SET_OFF:
         // check if there is a lightset in the clip
         if( pDoc->m_ObjectEdit.pLightSet )
         {
            pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hItem );
            pLightSet->pDetail->PasteLightSet();
         }
         break;
    }
}
