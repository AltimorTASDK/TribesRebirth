// LightView.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "LightView.h"
#include "MainFrm.h"
#include "childfrm.h"
#include "light.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLightView

CLightView::CLightView()
{
}

//--------------------------------------------------------------

CLightView::~CLightView()
{
}


BEGIN_MESSAGE_MAP(CLightView, CControlBar)
	//{{AFX_MSG_MAP(CLightView)
	ON_WM_PAINT()
   ON_WM_WINDOWPOSCHANGED()
   ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP                    
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightView message handlers

CSize CLightView::CalcDynamicLayout( int nLength, DWORD dwMode )
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

//--------------------------------------------------------------

void CLightView::OnUpdateCmdUI( CFrameWnd * pFrame, BOOL fDisable )
{
}

//--------------------------------------------------------------

void CLightView::OnPaint() 
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

//--------------------------------------------------------------

void CLightView::OnWindowPosChanged( WINDOWPOS* lpwndpos )
{
   // check for floating
   if( IsFloating() )
   {
   }
}

//////////////////////////////////////////////////////////////////////////
// tree control

BEGIN_MESSAGE_MAP(CLightViewTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CLightViewTree)
  	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemexpanding )
  	ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnSelchanging )
  	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged )
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

//--------------------------------------------------------------

CLightViewTree::CLightViewTree()
{
   m_fDragging = FALSE;
   pDragList = NULL;
}

//--------------------------------------------------------------
// check for change of capture
void CLightViewTree::OnCaptureChanged( CWnd* pWnd )
{
   if( m_fDragging && ( pWnd != this ) )
      CancelDrag();
}


//--------------------------------------------------------------
// key down
void CLightViewTree::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
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
            if( pFrame->m_wndLightView.CurrentSize.cx > minWidth )
               pFrame->m_wndLightView.CurrentSize.cx -= sizeAmount;
            pFrame->RecalcLayout();
            pFrame->RecalcLayout();
            pFrame->m_wndLightView.RedrawWindow();
            return;
            break;
         case VK_RIGHT:
            pFrame->m_wndLightView.CurrentSize.cx += sizeAmount;
            pFrame->RecalcLayout();
            pFrame->RecalcLayout();
            pFrame->m_wndLightView.RedrawWindow();
            return;
            break;
         case VK_UP:
            if( pFrame->m_wndLightView.CurrentSize.cy > minHeight )
               pFrame->m_wndLightView.CurrentSize.cy -= sizeAmount;
            pFrame->RecalcLayout();
            pFrame->RecalcLayout();
            pFrame->m_wndLightView.RedrawWindow();
            return;
            break;
         case VK_DOWN:
            pFrame->m_wndLightView.CurrentSize.cy += sizeAmount;
            pFrame->RecalcLayout();
            pFrame->RecalcLayout();
            pFrame->m_wndLightView.RedrawWindow();
            return;
            break;
            
      }
   }
   else
   {
      if( nChar == VK_DELETE )
      {
         HTREEITEM hItem = GetSelectedItem();
         if( hItem )
         {
            // get the pntr to the item
            CItemBase * pItem = ( CItemBase * )GetItemData( hItem );
            if( pItem )
               pItem->remove();
         }
      }
   }
   
   CTreeCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
}

//--------------------------------------------------------------
// right mouse button down
void CLightViewTree::OnRButtonDown( UINT nFlags, CPoint point )
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
   pFrame->m_wndLightView.RightClick( hItem );
   
   // bring up a popup menu
   CTreeCtrl::OnRButtonDown( nFlags, point );
}

//--------------------------------------------------------------

void CLightViewTree::OnEndlabeledit( NMHDR * pNMHDR, LRESULT * pResult )
{
   // get a ptr to the info
   TV_DISPINFO * pInfo = ( TV_DISPINFO * )pNMHDR;
   
   // check if need to update the text for this item
   if( pInfo->item.pszText && strlen( pInfo->item.pszText ) )
   {
      CItemBase * base = ( CItemBase * )GetItemData( pInfo->item.hItem );
      *pResult = base->onNameChanged( pInfo->item.pszText );
   }
}

//--------------------------------------------------------------

void CLightViewTree::OnBeginlabeledit( NMHDR * pNMHDR, LRESULT * pResult )
{
   TV_DISPINFO * pInfo = ( TV_DISPINFO* )pNMHDR;
   
   SetFocus();
   
   CItemBase * base = ( CItemBase * )GetItemData( pInfo->item.hItem );
   *pResult = !( base->getFlags() & CItemBase::CAN_NAME );
}

//--------------------------------------------------------------

void CLightViewTree::OnBegindrag( NMHDR* pNMHDR, LRESULT* pResult )
{                             
   HTREEITEM      hItem;
   NM_TREEVIEW *  pTreeView;
   
   pTreeView = ( NM_TREEVIEW * )pNMHDR;
   
   hItem = pTreeView->itemNew.hItem;
   
   SelectItem( hItem );
   m_hSelItem = hItem;
      
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

//--------------------------------------------------------------

// mouse move
void CLightViewTree::OnMouseMove( UINT nFlags, CPoint point )
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
         
         if( pFrame->m_wndLightView.UseDropTarget( hTarget ) )
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

//--------------------------------------------------------------
// cancel a drag operation
void CLightViewTree::CancelDrag( void )
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

//--------------------------------------------------------------
// left mouse button up
void CLightViewTree::OnLButtonUp( UINT nFlags, CPoint point )
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
      else if( pFrame->m_wndLightView.UseDropTarget( m_hDropItem ) )
      {
         CItemBase * src = ( CItemBase * )GetItemData( m_hSelItem );
         CItemBase * dest = ( CItemBase * )GetItemData( m_hDropItem );
         
         AssertFatal( src && dest, "Failed to get item data." );
         dest->onDrop( src );
      }
      
      RedrawWindow();
   }
}

//--------------------------------------------------------------

void CLightViewTree::OnLButtonDblClk( UINT nFlags, CPoint point )
{
   UINT        flags;
   HTREEITEM   hItem;

   SetFocus();

   // select this item
   hItem = HitTest( point, &flags );

   if( hItem )
   {
      CItemBase * base = ( CItemBase * )GetItemData( hItem );
      base->inspect( true );      
   }
   else
      CTreeCtrl::OnLButtonDblClk( nFlags, point );
}

//--------------------------------------------------------------

void CLightViewTree::OnGetdispinfo( NMHDR * pNMHDR, LRESULT * pResult )
{
}

//--------------------------------------------------------------

void CLightViewTree::OnItemexpanding( NMHDR* pNMHDR, LRESULT* pResult )
{
}

//--------------------------------------------------------------

void CLightViewTree::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult)
{
   // check if dragging
   if( m_fDragging )
      RedrawWindow();
}

//--------------------------------------------------------------

void CLightViewTree::OnSelchanged( NMHDR * pNMHDR, LRESULT * pResult )
{
   NM_TREEVIEW *  pInfo = ( NM_TREEVIEW * )pNMHDR;
   CItemBase * base = ( CItemBase * )GetItemData( pInfo->itemNew.hItem );
   base->inspect( true, false );
   base->onSelect(); 
   SetFocus();   
}

////------------------------------------------------------------
//BOOL CLightView::AddLight( CLight * pLight )
//{
//   HTREEITEM hItem;   
//   
//   if( !pLight )
//      return( FALSE );
//      
//   // add to the tree view
//   hItem = m_TreeWnd.InsertItem( pLight->getName(), LIGHT, LIGHT );
//   
//   // set the item info
//   pLight->hItem = hItem;
//   
//   // set this as the data used
//   m_TreeWnd.SetItemData( hItem, ( DWORD )pLight );
//
//   m_TreeWnd.RedrawWindow();
//   
//   return( TRUE );
//}
//
////------------------------------------------------------------
//BOOL CLightView::RemoveLight( CLight * pLight )
//{
//   if( pLight->hItem )
//   {
//      m_TreeWnd.DeleteItem( pLight->hItem );
//      m_TreeWnd.RedrawWindow();
//   }
//   return( true );
//}
//
////------------------------------------------------------------
//BOOL CLightView::AddLightState( CLight * pParent, CLightState * pLightState )
//{
//   if( !pParent || !pLightState )
//      return( false );
//      
//   HTREEITEM hParent = pParent->hItem;
//   
//   pLightState->hItem = m_TreeWnd.InsertItem( "hello", STATE, STATE, hParent );
//   
//   m_TreeWnd.SetItemData( pLightState->hItem, ( DWORD )pLightState );
//   m_TreeWnd.RedrawWindow();
//   
//   return( TRUE );
//}
//
////------------------------------------------------------------
//BOOL CLightView::RemoveLightState( CLightState * pLightState )
//{
//   if( pLightState->hItem )
//   {
//      m_TreeWnd.DeleteItem( pLightState->hItem );
//      m_TreeWnd.RedrawWindow();
//   }
//   
//   return( TRUE );
//}
//
////------------------------------------------------------------
//BOOL CLightView::AddLightEmitter( CLightState * pParent, CLightEmitter * pLightEmitter )
//{
//   if( !pParent || !pLightEmitter )
//      return( false );
//      
//   // get the parent item
//   HTREEITEM hParent = pParent->hItem;
//
//   HTREEITEM hItem = m_TreeWnd.InsertItem( "emitter", EMITTER, EMITTER, hParent );
//   m_TreeWnd.SetItemData( hItem, ( DWORD )pLightEmitter );
//
//   pLightEmitter->hItem = hItem;
//   m_TreeWnd.RedrawWindow();
//
//   return( true );
//}
//
////------------------------------------------------------------
//BOOL CLightView::RemoveLightEmitter( CLightEmitter * pLightEmitter )
//{
//   if( pLightEmitter->hItem )
//   {
//      m_TreeWnd.DeleteItem( pLightEmitter->hItem );
//      m_TreeWnd.RedrawWindow();
//   }
//   return( true );
//}

//--------------------------------------------------------------

void CLightView::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
   switch( nChar )
   {
      case VK_DELETE:
         OutputDebugString( "GOT IT!" );
         break;
   }
}

void CLightView::Enable( BOOL fState )
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


//--------------------------------------------------------------
// check if we should use the drop target

BOOL CLightView::UseDropTarget( HTREEITEM hTarget )
{
   CItemBase * target = ( CItemBase * )m_TreeWnd.GetItemData( hTarget );
   CItemBase * source = ( CItemBase * )m_TreeWnd.GetItemData( m_TreeWnd.m_hSelItem );

   // check if the item likes it
   return( target->onDragOver( source ) );
}

BOOL CLightViewTree::Rebuild( CItemBase * root )
{
   AssertFatal( CLight::currentArray, "No light array." );
   
   // check if need to rebuild the entire thing... 
   if( !root )
   {
      // remove all the items in the list
      DeleteAllItems();

      // go through them all
      for( int i = 0; i < CLight::currentArray->GetSize(); i++ )
      {
         CLight * pLight = &(*CLight::currentArray)[i];
         pLight->add( NULL, CItemBase::LIGHT, CItemBase::LIGHT );
         
         for( int j = 0; j < pLight->states.GetSize(); j++ )
         {
            CLightState * pLightState = &pLight->states[j];
            pLightState->add( pLight, CItemBase::LIGHT_STATE, CItemBase::LIGHT_STATE );
            
            for( int k = 0; k < pLightState->emitters.GetSize(); k++ )
            {
               CLightEmitter * pLightEmitter = &pLightState->emitters[k];
               pLightEmitter->add( pLightState, CItemBase::LIGHT_EMITTER, CItemBase::LIGHT_EMITTER );
            }
         }
      }
      verifyInspect();
      // redraw the window
      if( CItemBase::pDoc )
         CItemBase::pDoc->UpdateAllViews(NULL);
      return( true );   
   }

   // check if building a light
   CLight * pLight = dynamic_cast<CLight*>(root);
   if( pLight )
   {
      // remove just the children...
      while( GetChildItem( pLight->hTreeItem ) )
         DeleteItem( GetChildItem( pLight->hTreeItem ) );

      for( int j = 0; j < pLight->states.GetSize(); j++ )
      {
         CLightState * pLightState = &pLight->states[j];
         pLightState->add( pLight, CItemBase::LIGHT_STATE, CItemBase::LIGHT_STATE );
         
         for( int k = 0; k < pLightState->emitters.GetSize(); k++ )
         {
            CLightEmitter * pLightEmitter = &pLightState->emitters[k];
            pLightEmitter->add( pLightState, CItemBase::LIGHT_EMITTER, CItemBase::LIGHT_EMITTER );
         }
      }
      verifyInspect();
      // redraw the window
      if( CItemBase::pDoc )
         CItemBase::pDoc->UpdateAllViews(NULL);
      return( true );
   }
      
   // check if just building a lightstate...
   CLightState * pLightState = dynamic_cast<CLightState*>(root);
   if( pLightState )
   {
      // remove just the children...
      while( GetChildItem( pLightState->hTreeItem ) )
         DeleteItem( GetChildItem( pLightState->hTreeItem ) );

      for( int k = 0; k < pLightState->emitters.GetSize(); k++ )
      {
         CLightEmitter * pLightEmitter = &pLightState->emitters[k];
         pLightEmitter->add( pLightState, CItemBase::LIGHT_EMITTER, CItemBase::LIGHT_EMITTER );
      }
      verifyInspect();
      // redraw the window
      if( CItemBase::pDoc )
         CItemBase::pDoc->UpdateAllViews(NULL);
      return( true );
   }
   
   // redraw the window
   if( CItemBase::pDoc )
      CItemBase::pDoc->UpdateAllViews(NULL);
   return( false );
}

//--------------------------------------------------------------

void CLightViewTree::verifyInspect()
{
   if( !CItemBase::propSheet )
      return;
      
   // check if the inspect window is up
   if( !CItemBase::propSheet->m_hWnd || !::IsWindow( CItemBase::propSheet->m_hWnd ) )
      return;
   
   // get the selected item and check if the inspect window is
   // the same item, if not then inspect the current item
   HTREEITEM hItem = GetSelectedItem();
   if( hItem )
   {
      CItemBase * base = ( CItemBase * )GetItemData( hItem );
      base->inspect( true, false );
   }
   else
      blank.inspect( true, false );
}

//--------------------------------------------------------------
// rebuild the tree view
BOOL CLightView::Rebuild( CTHREDDoc * pDoc, CLightArray & lightArray )
{
   // set the current array.. total kludge! but a light has no parent
   CLight::currentArray = &lightArray;
   
   // remove all the items in the list
   m_TreeWnd.DeleteAllItems();

   // set the ptr to the tree ctrl
   CItemBase::tree = &m_TreeWnd;
   CItemBase::pDoc = pDoc;
   
   // go through them all
   for( int i = 0; i < lightArray.GetSize(); i++ )
   {
      CLight * pLight = &lightArray[i];
      pLight->add( NULL, CItemBase::LIGHT, CItemBase::LIGHT );
      
      for( int j = 0; j < pLight->states.GetSize(); j++ )
      {
         CLightState * pLightState = &pLight->states[j];
         pLightState->add( pLight, CItemBase::LIGHT_STATE, CItemBase::LIGHT_STATE );
         
         for( int k = 0; k < pLightState->emitters.GetSize(); k++ )
         {
            CLightEmitter * pLightEmitter = &pLightState->emitters[k];
            pLightEmitter->add( pLightState, CItemBase::LIGHT_EMITTER, CItemBase::LIGHT_EMITTER );
         }
      }
   }
   
   // return success
   return( TRUE );
}

void CLightView::pasteCurrent()
{
   HTREEITEM hItem = m_TreeWnd.GetSelectedItem();
   if( !hItem )
      return;
   
   // get the pntr to the item
   CItemBase * pItem = ( CItemBase * )m_TreeWnd.GetItemData( hItem );
   if( !pItem )
      return;
      
   pItem->paste();
}

//--------------------------------------------------------------

void CLightView::copyCurrent()
{
   HTREEITEM hItem = m_TreeWnd.GetSelectedItem();
   if( !hItem )
      return;
   
   // get the pntr to the item
   CItemBase * pItem = ( CItemBase * )m_TreeWnd.GetItemData( hItem );
   if( !pItem )
      return;
      
   pItem->copy();
}

//--------------------------------------------------------------

void CLightView::cutCurrent()
{
   HTREEITEM hItem = m_TreeWnd.GetSelectedItem();
   if( !hItem )
      return;
   
   // get the pntr to the item
   CItemBase * pItem = ( CItemBase * )m_TreeWnd.GetItemData( hItem );
   if( !pItem )
      return;
      
   pItem->copy();
   pItem->remove();
}

//--------------------------------------------------------------

void CLightView::removeCurrent()
{
   HTREEITEM hItem = m_TreeWnd.GetSelectedItem();
   if( !hItem )
      return;
   
   // get the pntr to the item
   CItemBase * pItem = ( CItemBase * )m_TreeWnd.GetItemData( hItem );
   if( !pItem )
      return;
      
   pItem->remove();
}

//--------------------------------------------------------------

BOOL CLightView::RightClick( HTREEITEM hItem )
{
   CItemBase * base = NULL;
   
   if( hItem )
      base = ( CItemBase * )m_TreeWnd.GetItemData( hItem );

   CPoint pnt;
   GetCursorPos( &pnt );
   
   CMenu menu;
   menu.CreatePopupMenu();
   
   if( base )
   {
      // add all the commands
      menu.AppendMenu( MF_STRING, CItemBase::REMOVE, "Remove" );
      menu.AppendMenu( MF_STRING, CItemBase::CUT, "Cut" );
      menu.AppendMenu( MF_STRING, CItemBase::COPY, "Copy" );
   
      base->onContextMenu( menu );
   
      // add some more
      menu.AppendMenu( MF_SEPARATOR );
      menu.AppendMenu( MF_STRING, CItemBase::INSPECT, "Properties..." );
   }
   else // clicked somewhere other than on an item
   {
      menu.AppendMenu( MF_STRING, CItemBase::ADD_LIGHT, "Add Light" );
      
      UINT menuFlags = MF_STRING;
      
      // check for a light to past - only case needed to be handled special like
      if( !CItemBase::copyObj || !dynamic_cast<CLight *>(CItemBase::copyObj) )
         menuFlags |= MF_GRAYED;
      menu.AppendMenu( menuFlags, CItemBase::PASTE_LIGHT, "Paste Light" );
   }

   // get the id selectedd
   int val = menu.TrackPopupMenu( TPM_RETURNCMD | TPM_CENTERALIGN |
      TPM_RIGHTBUTTON, pnt.x, pnt.y, this, NULL );
      
   switch( val )
   {
      // handle the generics
      case CItemBase::REMOVE:
         base->remove();
         break;
         
      case CItemBase::CUT:
         base->copy();
         base->remove();
         break;
         
      case CItemBase::COPY:
         base->copy();
         break;
      
      case CItemBase::INSPECT:
         base->inspect(true,true);
         break;
         
      case CItemBase::ADD_LIGHT:
      {
         // add a light with a default state, and emitter...
			CLight Light( true );
			( *CLight::currentArray ).Add( Light );
         m_TreeWnd.Rebuild( NULL );
         break;
      }
      
      case CItemBase::PASTE_LIGHT:
      {
         CLight light;
         
         // get the copy obj - handled special for lights
         CLight * pLight = dynamic_cast< CLight *>( CItemBase::copyObj );
         if( !pLight )
            break;
            
         // add it
         light = *pLight;
			CLight::currentArray->Add( light );
         m_TreeWnd.Rebuild( NULL );
            
         break;
      }
      default:
         // forward to the obj
         if( base )
            base->onContextMenuSelect( val );
         break;
   }
   
   return( true );
}   

//--------------------------------------------------------------

bool CLightView::IsActive()
{
   if( GetFocus() == ( CWnd * )&m_TreeWnd )
      if( m_TreeWnd.GetSelectedItem() )
         return( TRUE );
   return( FALSE );
}

//--------------------------------------------------------------

bool CLightView::IsOpen()
{
   return( ( GetStyle() & WS_VISIBLE ) ? true : false );
}

//void CLightView::CopyCurrent()
//{
//   HTREEITEM hItem;
//   CShapeDetail * pDetail;
//   CShapeState * pState;
//   CLightSet * pLightSet;
//   int      Image, SelectImage;
//   
//   hItem = m_TreeWnd.GetSelectedItem();
//
//   if( !hItem )
//      return;
//      
//   // determine the type of item
//   m_TreeWnd.GetItemImage( hItem, Image, SelectImage );
//   
//   switch( Image )
//   {
//      case STATE:
//         pState = ( CShapeState * )m_TreeWnd.GetItemData( hItem );
//         pState->pDoc->CopyState( pState );
//         break;
//         
//      case DETAIL:
//      case DETAIL_OFF:
//         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hItem );
//         pDetail->pState->CopyDetail( pDetail );
//         break;
//         
//      case LIGHT_SET:
//      case LIGHT_SET_OFF:
//         pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hItem );
//         pLightSet->pDetail->CopyLightSet( pLightSet );
//         break;
//    }
//}
//
//void CLightView::CutCurrent()
//{
//   HTREEITEM hItem;
//   CShapeDetail * pDetail;
//   CShapeState * pState;
//   CLightSet * pLightSet;
//   int      Image, SelectImage;
//   
//   hItem = m_TreeWnd.GetSelectedItem();
//
//   if( !hItem )
//      return;
//      
//   // determine the type of item
//   m_TreeWnd.GetItemImage( hItem, Image, SelectImage );
//   
//   switch( Image )
//   {
//      case STATE:
//         pState = ( CShapeState * )m_TreeWnd.GetItemData( hItem );
//         pState->pDoc->CopyState( pState );
//         
//         if( pState->pDoc->NumDetails() <= 1 )
//         {            
//            AfxMessageBox("Cannot remove state, zed requires at least \n"
//               "one detail in the document.", MB_OK, 0);
//            return;
//         }
//         
//         pState->pDoc->RemoveState( pState );
//         break;
//         
//      case DETAIL:
//      case DETAIL_OFF:
//         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hItem );
//         pDetail->pState->CopyDetail( pDetail );
//         
//         if( pDetail->pState->pDoc->NumDetails() <= 1 )
//         {            
//            AfxMessageBox("Cannot remove detail, zed requires at least \n"
//               "one detail in the document.", MB_OK, 0);
//            return;
//         }
//         pDetail->pState->RemoveDetail( pDetail );
//         break;
//         
//      case LIGHT_SET:
//      case LIGHT_SET_OFF:
//         pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hItem );
//         pLightSet->pDetail->CopyLightSet( pLightSet );
//         pLightSet->pDetail->RemoveLightSet( pLightSet );
//         break;
//    }
//}
//void CLightView::DeleteCurrent()
//{
//   HTREEITEM hItem;
//   CShapeDetail * pDetail;
//   CShapeState * pState;
//   CLightSet * pLightSet;
//   int      Image, SelectImage;
//   
//   hItem = m_TreeWnd.GetSelectedItem();
//
//   if( !hItem )
//      return;
//      
//   // prompt user ( this is pretty final )
//   if( AfxMessageBox( "This action cannot be undone.  Continue?",
//      MB_YESNO, 0 ) != IDYES )
//      return;
//      
//   // determine the type of item
//   m_TreeWnd.GetItemImage( hItem, Image, SelectImage );
//   
//   switch( Image )
//   {
//      case STATE:
//         pState = ( CShapeState * )m_TreeWnd.GetItemData( hItem );
//
//         if( pState->pDoc->NumDetails() <= 1 )
//         {            
//            AfxMessageBox("Cannot remove state, zed requires at least \n"
//               "one detail in the document.", MB_OK, 0);
//            return;
//         }
//         
//         pState->pDoc->RemoveState( pState );
//         break;
//         
//      case DETAIL:
//      case DETAIL_OFF:
//         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hItem );
//         if( pDetail->pState->pDoc->NumDetails() <= 1 )
//         {            
//            AfxMessageBox("Cannot remove detail, zed requires at least \n"
//               "one detail in the document.", MB_OK, 0);
//            return;
//         }
//         pDetail->pState->RemoveDetail( pDetail );
//         break;
//         
//      case LIGHT_SET:
//      case LIGHT_SET_OFF:
//         pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hItem );
//         pLightSet->pDetail->RemoveLightSet( pLightSet );
//         break;
//    }
//}

//void CLightView::PasteCurrent()
//{
//   HTREEITEM hItem;
//   CShapeDetail * pDetail;
//   CShapeState * pState;
//   CLightSet * pLightSet;
//   int      Image, SelectImage;
//   
//   hItem = m_TreeWnd.GetSelectedItem();
//
//   // get the document ptr
//	CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
//   CChildFrame * pChild = ( CChildFrame * )pFrame->MDIGetActive();
//   if( !pChild ) return;
//   CTHREDDoc * pDoc = pChild->pCurrentDetail->pState->pDoc;
//   if( !pDoc ) return;
//   
//   if( !hItem )
//   {
//      pDoc->PasteState();
//      return;
//   }
//      
//   // determine the type of item
//   m_TreeWnd.GetItemImage( hItem, Image, SelectImage );
//   
//   switch( Image )
//   {
//      case STATE:
//         // check if there is a state there
//         if( pDoc->m_LightEdit.pState )
//            pDoc->PasteState();
//         else
//         {
//            pState = ( CShapeState * )m_TreeWnd.GetItemData( hItem );
//            pState->PasteDetail();
//         }
//         break;
//         
//      case DETAIL:
//      case DETAIL_OFF:
//         pDetail = ( CShapeDetail * )m_TreeWnd.GetItemData( hItem );
//   
//         // check if there is a detail in the clip
//         if( pDoc->m_LightEdit.pDetail )
//            pDetail->pState->PasteDetail();
//         else
//            pDetail->PasteLightSet();
//         break;
//         
//      case LIGHT_SET:
//      case LIGHT_SET_OFF:
//         // check if there is a lightset in the clip
//         if( pDoc->m_LightEdit.pLightSet )
//         {
//            pLightSet = ( CLightSet * )m_TreeWnd.GetItemData( hItem );
//            pLightSet->pDetail->PasteLightSet();
//         }
//         break;
//    }
//}
