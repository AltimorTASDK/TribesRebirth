//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//               (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#include "GWTreeView.h"

#include <g_bitmap.h>
#include <g_pal.h>

#include "talgorithm.h"

//============================================================================

bool  GWTreeView::createWin( GWWindow *parent, const char *caption, RectI &r, DWORD exStyle, DWORD style, DWORD listStyle )
{
   if (parent)
      r += parent->getPosition();   

   if ( create( NULL, caption, 
        WS_OVERLAPPEDWINDOW | style,
        Point2I(r.upperL.x, r.upperL.y),
        Point2I(r.len_x(), r.len_y()),
        parent, 0, true, true) )
   {
      treeList.createWin( this, getClientSize(), exStyle, listStyle );
      state.clear();

      show (SW_SHOW);
      update();
      return ( true );
   }
   else
   {
      AssertFatal(0, "GWTreeView::createWind: could not create window");
      return ( false );        
   }
}   

//----------------------------------------------------------------------------

int GWTreeView::registerBitmap( Resource<GFXBitmap> pBM, GFXPalette *pPal )
{
   GFXPalette pal;
   if ( !pPal )
   {
      pPal = &pal;
      pal.setWindowsColors();
   }

   HDC hDC = GetDC( hWnd );
   struct   // BITMAPINFOHEADER
   {
      BITMAPINFOHEADER  bmiHeader;
      RGBQUAD           bmiColors[256];
   }bi;

   bi.bmiHeader.biSize           = sizeof( BITMAPINFOHEADER );
   bi.bmiHeader.biWidth          = pBM->width;
   bi.bmiHeader.biHeight         = -pBM->height;
   bi.bmiHeader.biPlanes         = 1;
   bi.bmiHeader.biBitCount       = 8;
   bi.bmiHeader.biCompression    = BI_RGB;
   bi.bmiHeader.biSizeImage      = pBM->imageSize;
   bi.bmiHeader.biXPelsPerMeter  = 0;
   bi.bmiHeader.biYPelsPerMeter  = 0;
   bi.bmiHeader.biClrUsed        = 256;
   bi.bmiHeader.biClrImportant   = 0;

   for ( int i = 0; i < 256; i++ )
   {
      bi.bmiColors[i].rgbRed     = pPal->palette[0].color[i].peRed;
      bi.bmiColors[i].rgbGreen   = pPal->palette[0].color[i].peGreen;
      bi.bmiColors[i].rgbBlue    = pPal->palette[0].color[i].peBlue;
      bi.bmiColors[i].rgbReserved = 0;
   }
	bitmapList.increment();
	Resource<GFXBitmap> *slot = new(&bitmapList.last()) Resource<GFXBitmap>;

   HBITMAP hBM = CreateDIBitmap( hDC, &bi.bmiHeader, CBM_INIT, pBM->pBits, (LPBITMAPINFO)&bi, DIB_RGB_COLORS );
   HIMAGELIST hIL = TreeView_GetImageList( treeList.getHandle(), TVSIL_NORMAL );
   ImageList_AddMasked( hIL, hBM, (COLORREF)0 );

   pBM->attribute |= BMA_TRANSPARENT;
   *slot = pBM;

   return (true);
}   

//----------------------------------------------------------------------------
// This function allows you to add an item to a group
// hParent - handle to group
// hInsert - item is inserted in front of this item, (or TVI_FIRST, TVI_LAST, TVI_SORT)
// tvi     - a filled out TreeViewItem structure

void GWTreeView::clear()
{
   TreeView_DeleteItem( treeList.getHandle(), TVI_ROOT );
}   

void GWTreeView::refresh( HTREEITEM hItem )
{
   if ( !hItem )
      hItem = TreeView_GetFirstVisible( treeList.getHandle() );

   while( hItem )
   {
      if ( isItemExists( getItemId(hItem) ) )
      {
         TV_ITEM *tvi = getItemInfo( hItem );
         bool selected = isItemSelected( hItem );
         tvi->mask            = TVIF_CHILDREN|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE;
         tvi->hItem           = hItem;
         tvi->stateMask       = TVIS_DROPHILITED|TVIS_SELECTED;
         tvi->state           = selected? TVIS_DROPHILITED|TVIS_SELECTED:0;
         tvi->iSelectedImage  = handleGetBitmapIndex( hItem, true );
         tvi->iImage          = handleGetBitmapIndex( hItem, selected );
         tvi->cChildren       = I_CHILDRENCALLBACK;
         int result = TreeView_SetItem( treeList.getHandle(), tvi );
         if ( result == -1 )
            return;
      }
      else
      {
         HTREEITEM hPrev = TreeView_GetPrevVisible( treeList.getHandle(), hItem );
         delItem( hItem );
         hItem = hPrev;
      }
      hItem = TreeView_GetNextVisible( treeList.getHandle(), hItem );
   }
}   

//----------------------------------------------------------------------------

bool      GWTreeView::isItemFolder( HTREEITEM )
{
   return false;
}   
bool      GWTreeView::isItemExists( UINT )
{
   return false;
}

void      GWTreeView::handleItemExpansion( HTREEITEM, HTREEITEM )
{
}
bool      GWTreeView::handleItemCollapse( HTREEITEM, HTREEITEM )
{
   return true;
}
int       GWTreeView::handleGetBitmapIndex( HTREEITEM, bool )
{
   return 0;
}
bool      GWTreeView::handleItemRemoveFromFolder( HTREEITEM, HTREEITEM )
{
   return true;
}
char*     GWTreeView::handleItemNameChange( HTREEITEM, char* )
{
   return NULL;
}

//----------------------------------------------------------------------------


void GWTreeView::delItem( HTREEITEM hItem )
{
   TreeView_DeleteItem( treeList.getHandle(), hItem );   
}   

HTREEITEM GWTreeView::getParent( HTREEITEM hItem )
{
   return ( TreeView_GetParent( treeList.getHandle(), hItem ) );
}   

HTREEITEM GWTreeView::getSelection()
{
   return ( TreeView_GetSelection( treeList.getHandle() ) );
}   

HTREEITEM GWTreeView::getPrevSelection()
{
   return ( TreeView_GetPrevSibling( treeList.getHandle(), TreeView_GetSelection(treeList.getHandle()) ) );
}   

bool GWTreeView::isItemSelected( HTREEITEM hItem )
{
   return ( hItem == TreeView_GetSelection( treeList.getHandle() ) );
}   

bool GWTreeView::hasChildren( TV_ITEM *tvi )
{
   return ( tvi->cChildren != 0 );
}   

bool GWTreeView::selectItem( HTREEITEM hItem )
{
   return ( TreeView_SelectItem( treeList.getHandle(), hItem ) != NULL );
}   

//-----------------------------------------------------------
// this function returns a pointer to a structure that contains
// information about the item requested.  This information is
// only valid until the next time the function is called.
TV_ITEM* GWTreeView::getItemInfo( HTREEITEM hItem )
{
   return ( treeList.getItemInfo( hItem ) );
}   

//------------------------------------------------------------------------------

UINT  GWTreeView::getItemId( HTREEITEM hItem )
{
   TV_ITEM  tvi;
   tvi.hItem = hItem;
   tvi.mask = TVIF_HANDLE|TVIF_PARAM;
   tvi.state = 0;
   TreeView_GetItem( treeList.getHandle(), &tvi );
   return ( tvi.lParam );
}   

//------------------------------------------------------------------------------

HTREEITEM  GWTreeView::addItem( HTREEITEM hParent, HTREEITEM hInsert, TV_ITEM &tvi )
{
   return ( treeList.addItem( hParent, hInsert, tvi ) );
}   

//==============================================================================
// private functions to operate & interact with the treeview control

int GWTreeView::onNotify( int id, LPNMHDR pnmhdr )
{
   id;
   switch( pnmhdr->code)
   {
      case TVN_BEGINDRAG:
      case TVN_BEGINRDRAG:
         {
            HIMAGELIST hIml;
            NM_TREEVIEW *lItem=(NM_TREEVIEW *)pnmhdr;
            hIml = TreeView_CreateDragImage(treeList.getHandle(), lItem->itemNew.hItem);
            ImageList_BeginDrag(hIml, 0, 0, 0);
            ImageList_DragEnter(treeList.getHandle(), lItem->ptDrag.x, lItem->ptDrag.y);
            TreeView_SelectDropTarget(treeList.getHandle(), (HTREEITEM)NULL);
            ShowCursor(FALSE);
            SetCapture(hWnd);
            state.set(GWTV_DRAGGING);
            hDragItem = lItem->itemNew.hItem;
         }
         break;

      case TVN_ENDLABELEDIT:  // true if edited, false otherwise
         {
            TV_ITEM *tvi = &(TV_ITEM)(((TV_DISPINFO*)pnmhdr)->item);
            char *newName = handleItemNameChange( tvi->hItem, tvi->pszText );
            if ( newName )
            {
               tvi->pszText = strnew( newName );
               tvi->cchTextMax = strlen( newName );
               return true;
            }   
         }
         return false;

      case TVN_BEGINLABELEDIT:
         return false;

      case TVN_ITEMEXPANDING:
         {
            NM_TREEVIEW *lItem=(NM_TREEVIEW *)pnmhdr;
            switch( lItem->action )
            {
               case TVE_EXPAND:
                  handleItemExpansion( getParent(lItem->itemNew.hItem), lItem->itemNew.hItem );
                  break;
               case TVE_COLLAPSE:
                  handleItemCollapse( getParent(lItem->itemNew.hItem), lItem->itemNew.hItem );
                  break;
            }
            return false;
         }

      case TVN_GETDISPINFO:
         {
            TV_DISPINFO *lItem=(TV_DISPINFO *)pnmhdr;
            if ( lItem->item.mask & TVIF_CHILDREN )
               lItem->item.cChildren = hasChildren(&lItem->item)? 1:0;
            if ( lItem->item.mask & TVIF_IMAGE )
            {
               lItem->item.iImage = handleGetBitmapIndex( lItem->item.hItem, false );
               lItem->item.iSelectedImage = handleGetBitmapIndex( lItem->item.hItem, true );
            }
         }
   }
   return ( Parent::onNotify( id, pnmhdr ) );
}   

//------------------------------------------------------------------------------

void GWTreeView::onMouseMove(int x, int y, UINT keyFlags)
{
   keyFlags;
   if (state.test(GWTV_DRAGGING))
   {
      TV_HITTESTINFO tvHit;       // Hit test information
      HTREEITEM      hTarget;
      tvHit.pt.x = x;
      tvHit.pt.y = y;
      ImageList_DragMove(x,y);
      ImageList_DragShowNolock( false );
      if ( (hTarget = TreeView_HitTest(treeList.getHandle(), &tvHit)) != NULL )
         TreeView_SelectDropTarget(treeList.getHandle(), hTarget);
      ImageList_DragShowNolock( true );
   }
}   

//------------------------------------------------------------------------------

void GWTreeView::onLButtonDown(BOOL dblClick, int x, int y, UINT keyFlags)
{
   keyFlags;
   TV_HITTESTINFO tvHit;       // Hit test information
   HTREEITEM      hTarget;
   tvHit.pt.x = x;
   tvHit.pt.y = y;
//   if ( ((hTarget = TreeView_HitTest(treeList.getHandle(), &tvHit)) != NULL)
//        && !isItemFolder( hTarget ) )
   if ( (hTarget = TreeView_HitTest(treeList.getHandle(), &tvHit)) != NULL )
      handleItemSelection( hTarget, (dblClick == TRUE) );
}   

//------------------------------------------------------------------------------

void GWTreeView::onLButtonUp(int x, int y, UINT keyFlags)
{
   Parent::onLButtonUp(x,y,keyFlags);
   if (state.test(GWTV_DRAGGING))
   {
      dropItem( x,y );
      releaseDrag();
   }
}   

//------------------------------------------------------------------------------

void GWTreeView::onRButtonUp(int x, int y, UINT keyFlags)
{
   Parent::onRButtonUp(x,y,keyFlags);
   HTREEITEM hTarget;
   TV_HITTESTINFO tvHit;       // Hit test information
   tvHit.pt.x = x;
   tvHit.pt.y = y;
   if ( (hTarget = TreeView_HitTest(treeList.getHandle(), &tvHit)) == NULL )
   {
      releaseDrag();
      return;              
   }

   HMENU hMenu = CreatePopupMenu();
   bool showMenu = handleDragMenuItemInsert( hMenu, hDragItem, hTarget );
   ShowCursor(TRUE);
   POINT pt;
   pt.x = x;
   pt.y = y;
   ClientToScreen( hWnd, &pt );
   if ( showMenu )
   {
      int result = TrackPopupMenuEx( hMenu, TPM_RETURNCMD, pt.x, pt.y, hWnd, NULL );
      ShowCursor(FALSE);
      if ( handleDragMenuItemSelection( result ) )
         onLButtonUp( x, y, keyFlags );
      else
         releaseDrag();
   }
   else
      onLButtonUp( x, y, keyFlags );
}   

//------------------------------------------------------------------------------

void GWTreeView::contextMenu()
{
   POINT pt;
   GetCursorPos( &pt );
   onContextMenu( pt.x, pt.y, hWnd );
}   

void GWTreeView::onContextMenu( int x, int y, HWND hwnd )
{
   if (state.test(GWTV_DRAGGING))
      return;

   HMENU hMenu = CreatePopupMenu();
   handleContextMenuItemInsert( hMenu );
   ShowCursor(TRUE);
   POINT pt;
   pt.x = x;
   pt.y = y;
//   ClientToScreen( hwnd, &pt );
   int result = TrackPopupMenuEx( hMenu, TPM_RETURNCMD, pt.x, pt.y, hwnd, NULL );
   ShowCursor(FALSE);
   handleContextMenuItemSelection( result );
}   

//------------------------------------------------------------------------------

void GWTreeView::onSize(UINT state, int cx, int cy)
{
   state;
   MoveWindow(treeList.getHandle(), 0, 0, cx, cy, TRUE);
}   

//------------------------------------------------------------------------------

bool GWTreeView::hilightItem( HTREEITEM hItem )
{
   return ( TreeView_SelectDropTarget(treeList.getHandle(), hItem) != NULL );
}

//------------------------------------------------------------------------------

bool GWTreeView::makeVisible( HTREEITEM hItem )
{
   return ( TreeView_EnsureVisible(treeList.getHandle(), hItem) == TRUE );
}

//------------------------------------------------------------------------------

HTREEITEM GWTreeView::find( HTREEITEM hItemStart, int id )
{
   HTREEITEM hItem=NULL;

   if ( !hItemStart )
      return NULL;

   if ( getItemInfo( hItemStart )->lParam == id )
      return hItemStart;

   if ( isItemFolder(hItemStart) )
      hItem = find( TreeView_GetChild(treeList.getHandle(),hItemStart),id );

   if ( hItem )
      return ( hItem );
   else
      return ( find( TreeView_GetNextSibling(treeList.getHandle(),hItemStart),id ));
}

//------------------------------------------------------------------------------

void GWTreeView::dropItem( int x, int y )
{
   bool success;
   HTREEITEM hTarget;

   TV_HITTESTINFO tvHit;       // Hit test information
   tvHit.pt.x = x;
   tvHit.pt.y = y;
   if ( (hTarget = TreeView_HitTest(treeList.getHandle(), &tvHit)) == NULL )
      return;

   // if target is a group, add item to group
   // if target is an item, insert in front of item
   if ( isItemFolder( hTarget ) )
      success = handleItemDroppedOnFolder( hTarget, hDragItem );
   else
      success = handleItemDroppedOnItem ( hTarget, hDragItem );
      
   // if item represents a unique object delete it from its old group   
   if ( success && !state.test(GWTV_COPYING) )
      if ( handleItemRemoveFromFolder( getParent( hDragItem ), hDragItem ) )
         TreeView_DeleteItem(treeList.getHandle(), hDragItem);
}

//------------------------------------------------------------------------------

void GWTreeView::releaseDrag()
{
   ImageList_EndDrag();
   TreeView_SelectDropTarget(treeList.getHandle(), (HTREEITEM)NULL);
   ImageList_DragLeave(treeList.getHandle());
   ReleaseCapture();
   ShowCursor(TRUE);
   state.clear(GWTV_DRAGGING);
}   

//==============================================================================
//============================================================================
// code to handle the treeList control, you should never have to deal
// with the treeview at this level.

bool GWTreeList::createWin( GWWindow *parent, Point2I& size, DWORD exStyle, DWORD listStyle )
{

   GWWindowClass *wClass = new GWWindowClass;
   wClass->setName(WC_TREEVIEW);

   if ( Parent::createEx( exStyle, wClass, (LPCSTR)NULL,
            WS_VISIBLE | WS_CHILD | listStyle,
            Point2I(0,0),
            size,
            parent, 0) )
   {
      hIml = ImageList_Create( IMAGE_WIDTH, IMAGE_HEIGHT,ILC_MASK|ILC_COLOR8,0,1 );
      TreeView_SetImageList( hWnd, hIml, TVSIL_NORMAL );
      show (SW_SHOW);
      update();
      delete wClass;
      return ( true );
   }
   else
   {
      AssertFatal(0, "GWTreeList::createWin: could not create window");
      delete wClass;
      return ( false );        
   }
}   

//------------------------------------------------------------------------------

HTREEITEM GWTreeList::addItem( HTREEITEM hParent, HTREEITEM hInsert, TV_ITEM &tvi )
{
   TV_INSERTSTRUCT tvIns;
   tvIns.item = tvi;
   tvIns.hInsertAfter = hInsert;
   tvIns.hParent = hParent;

   return ( TreeView_InsertItem( hWnd, &tvIns ) );
}   

//------------------------------------------------------------------------------

TV_ITEM* GWTreeList::getItemInfo( HTREEITEM hItem )
{
   static TV_ITEM tvi;
   static char text[256];
   tvi.hItem     = hItem;
   tvi.mask      = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM | TVIF_STATE | TVIF_HANDLE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
   tvi.pszText   = text;
   tvi.cchTextMax = 256;
   TreeView_GetItem( hWnd, &tvi );
   return ( &tvi ); 
}   

//------------------------------------------------------------------------------

void GWTreeList::onLButtonDown(BOOL dblClick, int x, int y, UINT keyFlags)
{
   Parent::onLButtonDown( dblClick, x, y, keyFlags );
   getParent()->onLButtonDown( dblClick, x, y, keyFlags );
}   

//==============================================================================
//============================================================================
// These are the virtual functions you override in your derived class
// to operate on the objects you are representing in the TreeView.
// you should be able to do most of what you need by overriding 
// these controls.

//------------------------------------------------------------------------------

void GWTreeView::handleItemSelection( HTREEITEM hItem, bool dblClick )
{
   hItem;
   dblClick;
}   

//------------------------------------------------------------------------------

bool GWTreeView::handleItemDroppedOnFolder( HTREEITEM hFolder, HTREEITEM hDropItem )
{
   TV_ITEM *tvi = treeList.getItemInfo( hFolder );
   if ( tvi->state & TVIS_EXPANDED )
      return ( addItem(hFolder, TVI_LAST, *getItemInfo(hDropItem))  != NULL );
   else
      return true;
}

//------------------------------------------------------------------------------

bool GWTreeView::handleItemDroppedOnItem( HTREEITEM hItem, HTREEITEM hDropItem )
{
   HTREEITEM hParent = getParent( hItem );

   hItem = TreeView_GetPrevSibling( treeList.getHandle(), TreeView_GetSelection(treeList.getHandle()) );
   return ( addItem(hParent, hItem? hItem:TVI_LAST, *getItemInfo(hDropItem)) != NULL);
}

//------------------------------------------------------------------------------

bool GWTreeView::handleDragMenuItemInsert( HMENU hMenu, HTREEITEM hDragItem, HTREEITEM hTargetItem )
{
   hDragItem;
   if ( hTargetItem )
   {
      AppendMenu( hMenu, MF_STRING, IDM_CANCEL, "Cancel" );
      AppendMenu( hMenu, MF_STRING, IDM_MOVE, "Move" );
      AppendMenu( hMenu, MF_STRING, IDM_COPY, "Copy" );
   }
   return ( hTargetItem != NULL ); // true if there is one, false otherwise.
}   

//------------------------------------------------------------------------------
// return true if you want it to handle the dropped object
// return false if you want to cancel the drag/drop

bool GWTreeView::handleDragMenuItemSelection( int id )
{
   switch( id )
   {
      case IDM_CANCEL:
         return ( false );    // just release drag, don't drop item

      case IDM_MOVE:
         state.clear(GWTV_COPYING);     // remove item from parent, drop into new parent
         break;

      case IDM_COPY:
         state.set(GWTV_COPYING);     // remove item from parent, drop into new parent
         break;
   }
   return ( true );           // proceed with the drop.
}   

//------------------------------------------------------------------------------

void GWTreeView::handleContextMenuItemInsert( HMENU hMenu )
{
   AppendMenu( hMenu, MF_STRING, IDM_CANCEL, "Cancel" );
}   

//------------------------------------------------------------------------------
// return true if you want it to handle the dropped object
// return false if you want to cancel the drag/drop

bool GWTreeView::handleContextMenuItemSelection( int id )
{
   switch( id )
   {
      case IDM_CANCEL:
         return ( false );    // just release drag, don't drop item
   }
   return ( true );           // proceed with the drop.
}   

// get the next item in the tree - traversal of tree
HTREEITEM GWTreeView::getNextItem( HTREEITEM hItem )
{
   if( hItem == NULL )
      return( NULL );
      
   HTREEITEM hti = NULL;
   
   // get the child if there is one
   hti = TreeView_GetChild( treeList.getHandle(), hItem );
   if( hti )
      return( hti );
      
   // doh.. get sibling or go back up
   while( ( hti = TreeView_GetNextSibling( treeList.getHandle(), hItem ) ) == NULL )
   {
      if( ( hItem = TreeView_GetParent( treeList.getHandle(), hItem ) ) == NULL )
         return( NULL );
   }
   
   return( hti );
}