//============================================================================
//   
//  $Workfile:   TV.h  $
//  $Version$
//  $Revision:   1.00  $
//    
//  DESCRIPTION:
//        TreeView class decleration
//        
//        This base class allows you to display abstract objects that
//        are represented by some sort of tree hierarchy.  A good example
//        of this is the SimEngine's SimGroup and SimObject hierarchy.
//        Please look at SimTreeView.cpp for an example of how to use
//        this base class to represent a hierarchy structure.
//        
//  (c) Copyright 1997, Dynamix Inc.   All rights reserved.
// 
//============================================================================

#ifndef  _TV_H_
#define  _TV_H_

#include <types.h>
#include <bitset.h>
#include <gw.h>
#include <commctrl.h>   // includes the common control header
#include <m_rect.h>

//----------------------------------------------------------------------------
// this is just a class needed by the TreeView base class.
// you should never have to deal with this class.
class GWTreeList : public GWWindow
{
   typedef GWWindow Parent;
public:
   HIMAGELIST     hIml;    // handle to image list

public:
   bool createWin( GWWindow *parent, Point2I& size, DWORD exStyle, DWORD listStyle );
   ~GWTreeList();

   //-----------------------------------------------------------
   HTREEITEM   addItem( HTREEITEM hParent, HTREEITEM hInsert, TV_ITEM &tvi );
   TV_ITEM*    getItemInfo( HTREEITEM hItem );

   void        onLButtonDown(BOOL dblClick, int x, int y, UINT keyFlags);
};

inline GWTreeList::~GWTreeList()
{
}   

//----------------------------------------------------------------------------
// GWTreeView is a base class from which you can derive a class that will
// display a tree hierarchy.  Interactions with this treeview get translated
// into simple functions which you can override to operate on your hierarchy.

#define  IMAGE_WIDTH    16
#define  IMAGE_HEIGHT   16

class GFXBitmap;

class GWTreeView : public GWWindow
{
private:
   void        onMouseMove(int x, int y, UINT keyFlags);
   void        onLButtonDown(BOOL dblClick, int x, int y, UINT keyFlags);
   void        onLButtonUp(int x, int y, UINT keyFlags);
   void        onRButtonUp(int x, int y, UINT keyFlags);
   void        dropItem(int x, int y);
   void        releaseDrag();

protected:
   typedef GWWindow  Parent;
   GWTreeList        treeList;
   HTREEITEM         hDragItem;
   BitSet32          state;

   int               onNotify(int id, LPNMHDR pnmhdr);
   void              onContextMenu(int x, int y, HWND hWnd );
   virtual void      onSize(UINT state, int cx, int cy);
   virtual bool      hilightItem( HTREEITEM hItem );
   virtual bool      makeVisible( HTREEITEM hItem );
   virtual HTREEITEM find( HTREEITEM hItemStart, int id );

public:
   virtual bool      createWin( GWWindow *parent, const char *caption, RectI &r, DWORD exStyle=WS_EX_PALETTEWINDOW, DWORD style=NULL, DWORD listStyle=TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT );
   virtual int       registerBitmap( Resource<GFXBitmap> pBM, GFXPalette *pPal );
   virtual void      clear();
   virtual void      refresh( HTREEITEM hItem=NULL );
   virtual void      delItem( HTREEITEM hItem );
   virtual HTREEITEM getParent( HTREEITEM hItem );
   virtual HTREEITEM getSelection();
   virtual HTREEITEM getPrevSelection();
   virtual bool      isItemFolder( HTREEITEM hItem ); // =0;
   virtual bool      isItemExists( UINT id ); // =0;
   virtual bool      isItemSelected( HTREEITEM hItem );
   virtual bool      hasChildren( TV_ITEM *tvi );
   virtual TV_ITEM*  getItemInfo( HTREEITEM hItem );
   virtual bool      selectItem( HTREEITEM hItem );
   virtual HTREEITEM getNextItem( HTREEITEM hItem );
   void              contextMenu();
   UINT              getItemId( HTREEITEM hItem );

   // This is an important function.  in the TV_ITEM structure there is a 
   // field called lParam.  Place a pointer to an object (SimObject*) you 
   // are representing with the treeView item.  When you get called with 
   // the handleItem... methods below, retrieve your object from the lParam
   // in the TV_ITEM structure and use your objects methods to perform the
   // appropriate tasks.
   virtual HTREEITEM addItem( HTREEITEM hParent, HTREEITEM hInsert, TV_ITEM &tvi );

   //----------------------------------------------------------------------------
   // Below are the functions you will likely override in your derived
   // class.  In these functions you put the code to operate on the
   // objects you are representing in the treeView.  The TreeView base
   // class handles the visual representation with treeView items for you.

   virtual void      handleItemSelection( HTREEITEM hItem, bool dblClick );

   // expansion is when you click on the + button in front of a group
   // you must provide information on children about to be exposed
   virtual void      handleItemExpansion( HTREEITEM hParent, HTREEITEM hItem );//=0;
   virtual bool      handleItemCollapse( HTREEITEM hParent, HTREEITEM hItem );//=0;

   // get icon to display in items open/closed states
   virtual int       handleGetBitmapIndex( HTREEITEM hItem, bool selected=false );//=0;

   // called when you drag/drop an item
   virtual bool      handleItemDroppedOnFolder( HTREEITEM hFolder, HTREEITEM hDropItem );
   virtual bool      handleItemDroppedOnItem( HTREEITEM hItem, HTREEITEM hDropItem );

   // drag menu appears when you drop an item with right mouse button
   virtual bool      handleDragMenuItemInsert( HMENU hMenu, HTREEITEM hItem, HTREEITEM hTarget );
   virtual bool      handleDragMenuItemSelection( int id );

   // context menu appears when you right click on an item
   virtual void      handleContextMenuItemInsert( HMENU hMenu );
   virtual bool      handleContextMenuItemSelection( int id );

   // if you drag with 'copying==false' item is removed from its group
   virtual bool      handleItemRemoveFromFolder( HTREEITEM hFolder, HTREEITEM hItem );//=0;

   // change name of item
   virtual char*     handleItemNameChange( HTREEITEM hItem, char* name );//=0;

   enum
   {
      GWTV_DRAGGING  = (1<<0),
      GWTV_COPYING   = (1<<1),
      GWTV_LASTSTATE = (1<<1),  // shift count must match previous
   };

   enum
   {
      IDM_CANCEL,
      IDM_MOVE,
      IDM_COPY,
      IDM_TV_USER,        // This must always be last enum
   };
   Vector<Resource<GFXBitmap> > bitmapList;
};

// for addItem(), the hInsert parameter may also be one of the following:
// TVI_FIRST
// TVI_LAST
// TVI_SORT

// allowed extended TreeView listStyles   (* = default)
// TVS_HASBUTTONS *
// TVS_HASLINES *
// TVS_LINESATROOT *
// TVS_EDITLABELS
// TVS_DISABLEDRAGDROP
// TVS_SHOWSELALWAYS

#endif   // _TV_H_
