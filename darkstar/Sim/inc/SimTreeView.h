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

#ifndef _SIMTREEVIEW_H_
#define _SIMTREEVIEW_H_

#include <sim.h>
#include <GWTreeView.h>
#include "WinTools.h"

//----------------------------------------------------------------------------

class SimTreeView : public GWTreeView, public SimObject
{
   typedef  SimObject Parent;
   typedef  GWTreeView WinParent;
   VectorPtr   <char*>  script;

public:
	SimTreeView();
   SimTreeView::~SimTreeView();

	bool lockManager();
	void unlockManager();

   virtual bool      createWin( GWWindow *parent, const char *caption, RectI &r, DWORD exStyle=WS_EX_PALETTEWINDOW, DWORD style=NULL, DWORD listStyle=TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT );
   virtual bool      isItemFolder( HTREEITEM hItem );
   virtual bool      isItemExists( UINT id );
   virtual bool      hasChildren( TV_ITEM *tvi );
   SimObject *       getObject( HTREEITEM hItem );
   SimObject *       getObject( UINT id );
   HTREEITEM         findItem( SimObject * obj );
   SimObject *       getSelectedObject();
   SimSet  *         getSelectedParent();
   bool              hilightObjectItem( SimObject *obj );
   int               matchClassToIndex( const char *className, bool selected );

   // expose simObjects associated with simSet to be expanded
   virtual void      handleItemExpansion( HTREEITEM hParent, HTREEITEM hItem );
   virtual bool      handleItemCollapse( HTREEITEM hParent, HTREEITEM hItem );
   virtual int       handleGetBitmapIndex( HTREEITEM hItem, bool selected );
   virtual int       handleGetBitmapIndex( SimObject *obj, bool selected );

   // adds a simObject to a simSet
   virtual bool      handleItemDroppedOnFolder( HTREEITEM hFolder, HTREEITEM hDropItem );
   virtual bool      handleItemDroppedOnItem( HTREEITEM hItem, HTREEITEM hDropItem );

   // removes a simObject from a simSet
   virtual bool      handleItemRemoveFromFolder( HTREEITEM hFolder, HTREEITEM hItem );

   // menu that appears when you drop a simObject with the right mouse button
   virtual bool      handleDragMenuItemInsert( HMENU hMenu, HTREEITEM hItem, HTREEITEM hTarget );
   virtual bool      handleDragMenuItemSelection( int id );

   // menu that appears when you right click on a simObject
   virtual void      handleContextMenuItemInsert( HMENU hMenu );
   virtual bool      handleContextMenuItemSelection( int id );

   // change name of object
   virtual char*     handleItemNameChange( HTREEITEM hItem, char *name );

   //------------------------------------------
   virtual HTREEITEM addSet( SimObject *obj, HTREEITEM hParent=TVI_ROOT );

   //------------------------------------------
	bool onAdd();

   //------------------------------------------
   enum
   {
      STV_MODIFIED  = (GWTV_LASTSTATE<<1),
      STV_UNTITLED  = (GWTV_LASTSTATE<<2),
      STV_LASTSTATE = (GWTV_LASTSTATE<<2), // shift count must match previous
   };

   void setModified(){ state.set( STV_MODIFIED ); }
   
   //------------------------------------------
	SimManager* targetManager;

   //------------------------------------------
   // menu stuff
   struct   ClassReg
   {
      int   normalBmpIndex;
      int   selectedBmpIndex;
      char  *classname; 
   };
   typedef  VectorPtr<ClassReg*> ClassRegistry;
   ClassRegistry  classRegistry;

   int            scriptMenu_IDM;

   HMENU          hMainMenu;
   HMENU          hFileMenu;
   HMENU          hEditMenu;
   HMENU          hHelpMenu;
   HMENU          hScriptsMenu;
   virtual void   initMenu( HMENU hMenu );
   virtual void   checkMenu( HMENU hMenu );
   virtual void   onCommand( int id, HWND hwndCtl, UINT codeNotify );
   virtual void   onDestroy();

   virtual bool   inspectObject( SimObject *obj );
   virtual bool   lockObject( SimObject * obj, bool lock );
   void           registerClass( const char *classname, int normalBmpIndex, int selectedBmpIndex );
   void           registerScript( const char *description, const char *consoleString );

   enum
   {
      IDM_EXIT = GWTreeView::IDM_TV_USER + 100,
      IDM_STV_FILE_USER,   // This must always be last enum
   };

   enum
   {
      IDM_CUT = GWTreeView::IDM_TV_USER + 200,
      IDM_COPY,
      IDM_PASTE,
      IDM_DELETE,
      IDM_REMOVE,
      IDM_DUPLICATE,
      IDM_EDIT,
      IDM_LOCK,
      IDM_UNLOCK,
      IDM_STV_EDIT_USER,  // This must always be last enum
   };

   enum  
   {
      IDM_SCRIPT_START = GWTreeView::IDM_TV_USER + 300,
      IDM_STV_USER = 400  
   };
};

// allowed extended TreeView listStyles   (* = default)
// TVS_HASBUTTONS *
// TVS_HASLINES *
// TVS_LINESATROOT *
// TVS_EDITLABELS
// TVS_DISABLEDRAGDROP
// TVS_SHOWSELALWAYS

#endif   // _SIMTREEVIEW_H_
