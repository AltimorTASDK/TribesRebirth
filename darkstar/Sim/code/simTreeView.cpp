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

#include "simTreeView.h"
#include "simGame.h"
#include "simToolWin.h"
#include "inspectWindow.h"


//----------------------------------------------------------------------------

SimTreeView::SimTreeView()
{
	targetManager = 0;
}

//----------------------------------------------------------------------------

SimTreeView::~SimTreeView()
{
   while( classRegistry.size() )
   {
      delete [] classRegistry.last()->classname;
      delete classRegistry.last();
      classRegistry.pop_back();
   }
   while( script.size() )
   {
      delete [] script.last();
      script.pop_back();
   }
}   

//----------------------------------------------------------------------------

bool SimTreeView::lockManager()
{
	if (targetManager != manager)
   {   
      if ( !SimGame::get()->getManager(SimGame::SERVER) )
      {
         if ( !isDeleted() )
            deleteObject();
         return false;
      }
      else
      {
   		targetManager->lock();
      }
   }
   return true;
}

void SimTreeView::unlockManager()
{
	if (targetManager != manager)
		targetManager->unlock();
}	


//----------------------------------------------------------------------------

bool SimTreeView::createWin( GWWindow *parent, const char *caption, RectI &r, DWORD exStyle, DWORD style, DWORD listStyle )
{
   WinParent::createWin( parent, caption, r, exStyle, style, listStyle );

   hMainMenu    = CreateMenu();
   hFileMenu    = CreateMenu();
   hEditMenu    = CreateMenu();
   hScriptsMenu = CreateMenu();
   hHelpMenu    = CreateMenu();
   SetMenu( hWnd, hMainMenu );
   initMenu( hMainMenu );
   scriptMenu_IDM = IDM_SCRIPT_START;

   SetFocus( parent->getHandle() );
   return true;
}   

//----------------------------------------------------------------------------

bool SimTreeView::isItemFolder( HTREEITEM hItem )
{
   SimObject *obj = getObject( hItem );
   return ( obj && (dynamic_cast<SimSet*>(obj) != 0) );
}   

//----------------------------------------------------------------------------

bool SimTreeView::isItemExists( UINT id )
{
   return ( getObject(id) != NULL );
}   

//------------------------------------------------------------------------------

bool SimTreeView::hasChildren( TV_ITEM *tvi )
{
   SimSet *set;
	lockManager();
   SimObject *obj = targetManager->findObject( tvi->lParam );
	unlockManager();

   if ( obj && (( set = dynamic_cast<SimSet*>(obj)) != 0) )
      return ( set->size() != 0 );
   else
      return false;
}

//------------------------------------------------------------------------------

SimObject* SimTreeView::getSelectedObject()
{
   // get object from tree view
   HTREEITEM hItem = getSelection();
   if ( hItem )
     return ( getObject(hItem) );
   else
     return NULL;
}   

//----------------------------------------------------------------------------

SimObject* SimTreeView::getObject( HTREEITEM hItem )
{
   TV_ITEM *tvi;
   SimObject *obj=NULL;
   if ( hItem )
      if ( (tvi = getItemInfo( hItem )) != NULL )
         obj = targetManager->findObject(tvi->lParam);
   return obj;
}   

SimObject* SimTreeView::getObject( UINT id )
{
	lockManager();
   SimObject *obj = targetManager->findObject(id);
	unlockManager();
   return obj;
}   

// find an object in the list
HTREEITEM SimTreeView::findItem( SimObject * obj )
{
   // get the root
   HTREEITEM hRoot = TreeView_GetRoot( treeList.getHandle() );
   if( !hRoot )
      return( NULL );
      
   while( hRoot )
   {
      // do check
      SimObject * pObj = getObject( hRoot );
      if( pObj == obj )
         return( hRoot );

      hRoot = getNextItem( hRoot );
   }
   
   return( NULL );
}

//----------------------------------------------------------------------------

SimSet* SimTreeView::getSelectedParent()
{
   HTREEITEM hItem;
   SimObject *obj;
   if ( (hItem = getSelection()) != NULL )
      if ( (hItem = getParent( hItem )) != NULL )
         if ( (obj = getObject( hItem )) != NULL )
            return( dynamic_cast<SimSet*>(obj) );

   return NULL;
}   

//==============================================================================

void SimTreeView::handleItemExpansion( HTREEITEM /*hParent*/, HTREEITEM hItem )
{
   SimSet *set;
	lockManager();
   TV_ITEM *tvi = treeList.getItemInfo( hItem );
   SimObject *obj = getObject( hItem );
   if ( (set = dynamic_cast<SimSet*>(obj)) != 0 )
   {
      TreeView_Expand( treeList.getHandle(), hItem, TVE_COLLAPSE|TVE_COLLAPSERESET );
      tvi->mask = TVIF_IMAGE;
      tvi->iImage = handleGetBitmapIndex( hItem, true );
      TreeView_SetItem( treeList.getHandle(), tvi );
      for ( SimObjectList::iterator iter=set->begin(); iter!=set->end(); iter++ )
         addSet( *iter, hItem );
   }
	unlockManager();
}   

//----------------------------------------------------------------------------

bool SimTreeView::handleItemCollapse( HTREEITEM hParent, HTREEITEM hItem )
{
   hParent;
	lockManager();
   TV_ITEM *tvi = treeList.getItemInfo( hItem );
   SimObject *obj = getObject( hItem );
   if ( dynamic_cast<SimSet*>(obj) )
   {
      tvi->mask = TVIF_IMAGE;
      tvi->iImage = handleGetBitmapIndex( hItem, false );
      TreeView_SetItem( treeList.getHandle(), tvi );
   }   
   TreeView_Expand( treeList.getHandle(), hItem, TVE_COLLAPSE|TVE_COLLAPSERESET );
	unlockManager();
   return true;
}   

//------------------------------------------------------------------------------
// 0 = node, unselected
// 1 = node, selected
// 2 = SimGroup, unselected
// 3 = SimGroup, selected
// 4 = SimSet, unselected
// 5 = SimSet, selected
// 6 = Locked, unselected
// 7 = Locked, selected

int SimTreeView::matchClassToIndex( const char *className, bool selected )
{
   ClassRegistry::iterator itr;
   for ( itr=classRegistry.begin(); itr!=classRegistry.end(); itr++ )
      if ( !strnicmp((*itr)->classname, className, strlen((*itr)->classname)) )
         break;
   int index;
   if ( itr!=classRegistry.end() )
      index = selected? (*itr)->selectedBmpIndex : (*itr)->normalBmpIndex;
   else
      index = -1;
   return index;
}   

int SimTreeView::handleGetBitmapIndex( SimObject *obj, bool selected )
{
   int index = matchClassToIndex( obj->getClassName(),selected );
   if ( index == -1 )
   {
      if ( dynamic_cast<SimGroup*>(obj) )
         index = matchClassToIndex( "SimGroup",selected );
      if ( index == -1 )
      {
         if ( dynamic_cast<SimSet*>(obj) )
            index = matchClassToIndex( "SimSet",selected );
         if ( index == -1 )
         {
            if( obj->isLocked() )
               index = matchClassToIndex( "Locked", selected );
               
            if( index == -1 )
               index = selected;
         }
      }
   }
   return index;
}   

int SimTreeView::handleGetBitmapIndex( HTREEITEM hItem, bool selected )
{
   static entry=false;  // treeList has a reentry problem when getting info
   if ( entry )
      return 0;

   entry = true;
	lockManager();
   SimObject *obj = getObject( hItem );
   int index = handleGetBitmapIndex( obj, selected );
	unlockManager();
   entry = false;
   return index;
}   

//------------------------------------------------------------------------------

bool SimTreeView::handleItemDroppedOnFolder( HTREEITEM hTarget, HTREEITEM hDropItem )
{
   bool removeFromSource=false;
	lockManager();


   SimSet *pTarget = (SimSet*)getObject( hTarget );
   SimObject *obj = getObject(hDropItem);
   SimObject *pParent = getObject( getParent(hDropItem) );

   if ( pTarget != obj )
   {
      state.set(STV_MODIFIED);
      if ( pTarget->find(pTarget->begin(), pTarget->end(), obj) != pTarget->end() )
      {
         if ( pTarget->reOrder( obj ) )
            refresh( hTarget );
      }
      else
      {
         if ( !dynamic_cast<SimGroup*>(pTarget) )
         {
            if ( dynamic_cast<SimGroup*>(pParent) )
               removeFromSource = false;    // Group->Set, copy
            else if ( (GetKeyState( VK_SHIFT ) & 0x80000000) == 0 )
               removeFromSource = true;     // Set->Set, but pressing shift key, copy
            else
               removeFromSource = true;     // Set->Set, copying set by menu
         }
         else if ( !dynamic_cast<SimGroup*>(pParent) )
         {
         	unlockManager();
            return false;                    // set->Group is not allowed
         }
         else
         {
            state.clear(GWTV_COPYING);       // Group->Group, moving
            removeFromSource =  true;
         }
      	pTarget->addObject(obj);
         WinParent::handleItemDroppedOnFolder( hTarget, hDropItem );
      }
   }
	unlockManager();
   return removeFromSource;  // don't modify list after return
}

//------------------------------------------------------------------------------

bool SimTreeView::handleItemDroppedOnItem( HTREEITEM hTarget, HTREEITEM hDropItem )
{
   bool removeFromSource=false;
	lockManager();

   HTREEITEM hParent = getParent( hTarget );
   SimSet *pTarget = (SimSet*)getObject( hParent );
   SimObject *obj = getObject(hDragItem);
   SimObject *pParent = getObject( getParent(hDropItem) );

   if ( pTarget != obj )
   {
      state.set(STV_MODIFIED);
      if ( pTarget->find(pTarget->begin(), pTarget->end(), obj) != pTarget->end() )
      {
         if ( pTarget->reOrder( obj, getObject(hTarget) ) )
            refresh( hParent );
      }
      else
      {
         if ( !dynamic_cast<SimGroup*>(pTarget) )
         {
            if ( dynamic_cast<SimGroup*>(pParent) )
               removeFromSource = false;    // Group->Set, copy
            else if ( (GetKeyState( VK_SHIFT ) & 0x80000000) == 0 )
               removeFromSource = true;     // Set->Set, but pressing shift key, copy
            else
               removeFromSource = true;     // Set->Set, copying set by menu
         }
         else if ( !dynamic_cast<SimGroup*>(pParent) )
         {
         	unlockManager();
            return false;                    // set->Group is not allowed
         }
         else
         {
            state.clear(GWTV_COPYING);       // Group->Group, moving
            removeFromSource =  true;
         }
      	pTarget->addObject(obj);
         pTarget->reOrder( obj, getObject(hTarget) );
         WinParent::handleItemDroppedOnItem( hTarget, hDropItem );
      }
   }
	unlockManager();
   return removeFromSource;  // don't modify list after return
}

//------------------------------------------------------------------------------

bool SimTreeView::handleItemRemoveFromFolder( HTREEITEM hFolder, HTREEITEM hItem )
{
	lockManager();
   SimSet *parent = (SimSet*)getObject( hFolder );
   SimObject *obj = getObject(hItem);
   parent->removeObject( obj );
   state.set(STV_MODIFIED);
	unlockManager();
   return ( true );  // true indicates we want item removed from TreeList
}   

//------------------------------------------------------------------------------
//  based on the objects target, bring up appropriate drag menu
// Group -> Group == force a move ( no menu ) default behavior
// Group -> Set   == force a copy ( no menu )
// Set   -> Set   == copy or move, bring up default menu
// Set   -> Group == Not allowed! ( no menu )

bool SimTreeView::handleDragMenuItemInsert( HMENU hMenu, HTREEITEM hItem, HTREEITEM hTarget )
{
   if ( !hTarget )
      return false;

   HTREEITEM hParent = getParent(hItem);
   if ( !isItemFolder(hTarget) )
      hTarget = getParent(hTarget);
   if ( hParent == hTarget )  // parent-target are same, just reordering objects
      return false;           // no menu for drop on self group

   SimObject *pParent = getObject( hParent );
   SimObject *pTarget = getObject( hTarget );

   if ( GetKeyState( VK_SHIFT )&0x80000000 )
      return false;              // they want copy, no menu

   if ( !dynamic_cast<SimGroup*>(pTarget) )
      if ( !dynamic_cast<SimGroup*>(pParent) )
      {                                // Set->Set, copy/move choice
         WinParent::handleDragMenuItemInsert( hMenu,hItem,hTarget );
         return true;                  // show user menu
      }

   return false;     // no menu needed
}   

//------------------------------------------------------------------------------

bool SimTreeView::handleDragMenuItemSelection( int id )
{
   // VC doesn't like empty switches...
   //
//   switch( id )
//   {
//      //Currently SimTreeView does not handle anything from the drag menu
//
//     default:
//      break;
//   } 

   // let parent handle its default menu items
   return ( WinParent::handleDragMenuItemSelection( id ) );
}   

//------------------------------------------------------------------------------

void SimTreeView::handleContextMenuItemInsert( HMENU hMenu )
{
   // add parents default menu items too
   WinParent::handleContextMenuItemInsert( hMenu );

   // add our specific Sim menu items
   AppendMenu( hMenu, MF_STRING, SimTreeView::IDM_CUT,       "Cut" );
   AppendMenu( hMenu, MF_STRING, SimTreeView::IDM_COPY,      "Copy" );
   AppendMenu( hMenu, MF_STRING, SimTreeView::IDM_PASTE,     "Paste" );
   AppendMenu( hMenu, MF_STRING, SimTreeView::IDM_DELETE,    "Delete" );
   AppendMenu( hMenu, MF_STRING, SimTreeView::IDM_REMOVE,    "Remove from Set" );
   AppendMenu( hMenu, MF_STRING, SimTreeView::IDM_DUPLICATE, "Duplicate" );
   AppendMenu( hMenu, MF_SEPARATOR, 0, 0 );
   AppendMenu( hMenu, MF_STRING, SimTreeView::IDM_EDIT,      "Edit..." );
   AppendMenu( hMenu, MF_STRING, SimTreeView::IDM_LOCK,      "Lock" );
   AppendMenu( hMenu, MF_STRING, SimTreeView::IDM_UNLOCK,    "UnLock" );
}   

//------------------------------------------------------------------------------

bool SimTreeView::handleContextMenuItemSelection( int id )
{
   // for SimTreeView class, the context menu members are already part
   // of the main menu, so just pass them onto the main menu handler.
   // if your derived context menu has unique items not also handled
   // by the main menu, just switch on their id's here and handle them.
   onCommand( id, NULL, NULL );
   return ( true );
}   

//----------------------------------------------------------------------------

HTREEITEM SimTreeView::addSet( SimObject *obj, HTREEITEM hParent )
{
   TV_ITEM           tvi;
   const char*       name;

   if (obj->getName())
      name = avar("%4i,\"%s\", %s",obj->getId(),obj->getName(),obj->getClassName());
   else
      name = avar("%4i,\"\", %s",obj->getId(),obj->getClassName());

   tvi.mask       = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_CHILDREN;
   tvi.pszText    = (char*)name;
   tvi.cchTextMax = strlen( name );
	// The manager's id is not a normal valid object id, 
	// use id 0 which is reserved for the manager.
   tvi.lParam     = (obj == targetManager)? 0: (LPARAM)obj->getId();
   tvi.cChildren  = dynamic_cast<SimSet*>(obj)? I_CHILDRENCALLBACK:0;
   tvi.iImage     = handleGetBitmapIndex( obj, false );
   tvi.iSelectedImage = handleGetBitmapIndex( obj, true );

   return ( addItem( hParent, TVI_LAST, tvi ) );  // add this group
}

//----------------------------------------------------------------------------

char* SimTreeView::handleItemNameChange( HTREEITEM hItem, char *name )
{
	lockManager();
   SimObject *obj = getObject( hItem );
   if ( !obj )
      return NULL;

   obj->assignName( name );
	unlockManager();

   return ( (char*)avar("%4i,\"%s\", %s",obj->getId(),obj->getName(),obj->getClassName()) );
}   

//------------------------------------------------------------------------------

bool SimTreeView::onAdd()
{
	if (!Parent::onAdd())
		return false;
   if (!targetManager->findObject(SimToolSetId))
   {
	   SimToolSet* set = new SimToolSet;
      targetManager->addObject((SimObject*)set);
      targetManager->assignId(set, SimToolSetId);
      targetManager->assignName(set, "SimToolSet");
   }
   addToSet(SimToolSetId);
	return true;
}


//------------------------------------------------------------------------------
// view/edit objects information in an edit window

bool SimTreeView::inspectObject( SimObject *obj )
{
   if ( !obj )
      return false;

   // make sure object inspection window is up
	lockManager();
   InspectWindow *oi = dynamic_cast<InspectWindow *>(targetManager->findObject("objectInspector"));
   if ( !oi )
   {
      POINT curs;
      GetCursorPos( &curs );
      Point2I pos(curs.x,curs.y);
      Point2I size(300,300);

      oi = new InspectWindow( (GWWindow*)this, "Object Info", pos, size, (SimTagDictionary *)NULL );
      targetManager->addObject(oi,"objectInspector");
   }

   oi->setObjToInspect( obj, obj ? obj->getClassName() : NULL );
	unlockManager();

   return true;
}   

// lock/unlock object(s)
bool SimTreeView::lockObject( SimObject * obj, bool lock )
{
   if( !obj )
      return( false );
      
   SimSet   *ss;
   
   // recurse into children
   if( ( ss = dynamic_cast<SimSet*>( obj ) ) != NULL )
      for( SimSet::iterator itr = ss->begin(); itr != ss->end(); itr++ )
         lockObject( (*itr), lock );
   else
      obj->setLocked( lock );
   
   return( true );   
}

//------------------------------------------------------------------------------

void SimTreeView::initMenu( HMENU hMainMenu )
{
   AppendMenu( hFileMenu, MF_STRING, SimTreeView::IDM_EXIT,       "Exit" );

   AppendMenu( hEditMenu, MF_STRING, SimTreeView::IDM_CUT,        "Cut" );
   AppendMenu( hEditMenu, MF_STRING, SimTreeView::IDM_COPY,       "Copy" );
   AppendMenu( hEditMenu, MF_STRING, SimTreeView::IDM_PASTE,      "Paste" );
   AppendMenu( hEditMenu, MF_STRING, SimTreeView::IDM_DELETE,     "Delete" );
   AppendMenu( hEditMenu, MF_STRING, SimTreeView::IDM_REMOVE,     "Remove from Set" );
   AppendMenu( hEditMenu, MF_STRING, SimTreeView::IDM_DUPLICATE,  "Duplicate" );
   AppendMenu( hEditMenu, MF_SEPARATOR, 0, 0 );
   AppendMenu( hEditMenu, MF_STRING, SimTreeView::IDM_EDIT,       "Edit..." );
   AppendMenu( hEditMenu, MF_STRING, SimTreeView::IDM_LOCK,       "Lock" );
   AppendMenu( hEditMenu, MF_STRING, SimTreeView::IDM_UNLOCK,     "UnLock" );
   

   AppendMenu( hMainMenu, MF_POPUP, (UINT)hFileMenu,     "File" );
   AppendMenu( hMainMenu, MF_POPUP, (UINT)hEditMenu,     "Edit" );
   AppendMenu( hMainMenu, MF_POPUP, (UINT)hScriptsMenu,  "Scripts" );

   DrawMenuBar( hWnd );
}   

//------------------------------------------------------------------------------

void SimTreeView::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   hwndCtl, codeNotify;
   SimObject   *obj;
   SimSet    *prnt;
	lockManager();

   switch( id )
   {
      case IDM_EXIT:
         destroyWindow();
         break;

      case IDM_CUT:
         if ( (obj = getSelectedObject()) !=  NULL )
         {
            // persist selected object
            obj->fileStore( "temp\\clip.tmp" );

            // remove it from parent
            obj->deleteObject();
            delItem( getSelection() );
            state.set(STV_MODIFIED);
         }
         break;

      case IDM_COPY:
         if ( (obj = getSelectedObject()) !=  NULL )
            obj->fileStore( "temp\\clip.tmp" );
         break;

      case IDM_PASTE:
         {
            // unpersist object to get a duplicate
            Persistent::Base::Error err;
            obj = (SimObject*)Persistent::Base::fileLoad( "temp\\clip.tmp", &err );
            if ( err != Ok )
               return;

            // add to simTree
            HTREEITEM hParent = getSelection();
            if ( !isItemFolder(hParent) )
               hParent = getParent( hParent );

            prnt = (SimSet*)getObject( hParent );
            prnt->getManager()->addObject( obj );
				prnt->addObject(obj);
            HTREEITEM hItem = addSet( obj, hParent );
            selectItem( hItem );
            state.set(STV_MODIFIED);
         }
         break;

      case IDM_DELETE:
         obj = getSelectedObject();
         if ( obj )
         {
            obj->deleteObject();
            delItem( getSelection() );
            state.set(STV_MODIFIED);
         }
         break;

      case IDM_REMOVE:
         obj = getSelectedObject();
         if ( obj )
         {
            prnt = getSelectedParent();
				prnt->removeObject(obj);
            delItem( getSelection() );
            state.set(STV_MODIFIED);
         }
         break;

      case IDM_DUPLICATE:
         {
            obj = getSelectedObject();
            
            // persist object to get a duplicate
            if ( obj->fileStore( "temp\\clip.tmp" ) != Ok )
            {
					unlockManager();
               return;
            }

            Persistent::Base::Error err;
            obj = (SimObject*)Persistent::Base::fileLoad( "temp\\clip.tmp", &err );
            if ( err != Ok )
            {
					unlockManager();
               return;
            }
            // perhaps delete clip.tmp to clean up directory

            HTREEITEM hParent = getSelection();
            if ( !isItemFolder(hParent) )
               hParent = getParent( hParent );

            prnt = (SimSet*)getObject( hParent );
            prnt->getManager()->addObject( obj );
            prnt->addObject( obj );
            HTREEITEM hItem = addSet( obj, getParent(getSelection()) );
            selectItem( hItem );
            state.set(STV_MODIFIED);
         }
         break;

      case IDM_EDIT:
         inspectObject( getSelectedObject() );
         state.set(STV_MODIFIED);
         break;

      case IDM_LOCK:
         lockObject( getSelectedObject(), true );
         state.set( STV_MODIFIED );
         break;
         
      case IDM_UNLOCK:
         lockObject( getSelectedObject(), false );
         state.set( STV_MODIFIED );
         break;
         
      default:
         if ( (id>=IDM_SCRIPT_START) && (id<=scriptMenu_IDM) )
         {
            int scriptIndex = id-IDM_SCRIPT_START;
            CMDConsole::getLocked()->evaluate( script[scriptIndex], false );
         }
         // let parent handle its default menu items
         WinParent::handleContextMenuItemSelection( id );
   }
   checkMenu( hMainMenu );
	unlockManager();
}

//------------------------------------------------------------------------------

void SimTreeView::onDestroy()
{
   manager->deleteObject( this );
}   

//------------------------------------------------------------------------------

void SimTreeView::checkMenu( HMENU hMenu )
{
   UINT  flag;
   
   if ( getSelection() )
      flag = MF_ENABLED;
   else
      flag = MF_GRAYED;

   EnableMenuItem( hMenu, IDM_CUT, flag );
   EnableMenuItem( hMenu, IDM_COPY, flag );
   EnableMenuItem( hMenu, IDM_DELETE, flag );
   EnableMenuItem( hMenu, IDM_REMOVE, flag );
   EnableMenuItem( hMenu, IDM_DUPLICATE, flag );
   EnableMenuItem( hMenu, IDM_EDIT, flag );

   // seperate check to see if clip.tmp exists
   EnableMenuItem( hMenu, IDM_PASTE, flag );
}   

//------------------------------------------------------------------------------

bool SimTreeView::hilightObjectItem( SimObject *obj )
{
   HTREEITEM hItem;
   if ( (hItem = find(TreeView_GetRoot(treeList.getHandle()), obj->getId())) != NULL )
   {
      TreeView_Select( treeList.getHandle(), hItem, TVGN_CARET );
      return true;
   }
   else
      return false;
}   

//------------------------------------------------------------------------------

void SimTreeView::registerClass( const char *classname, int normalBmpIndex, int selectedBmpIndex )
{
   ClassReg *cr = new ClassReg;
   cr->normalBmpIndex = normalBmpIndex;
   cr->selectedBmpIndex = selectedBmpIndex;
   cr->classname = strnew( classname );
   classRegistry.push_front( cr );
}

//------------------------------------------------------------------------------

void SimTreeView::registerScript( const char *description, const char *consoleString )
{
   if ( !strcmpi(description,"separator") )
      AppendMenu( hScriptsMenu, MF_SEPARATOR, NULL, NULL );
   else
   {
      AppendMenu( hScriptsMenu, MF_STRING, scriptMenu_IDM, description );
      script.push_back( strnew( consoleString ) );
      scriptMenu_IDM++;
   }
   DrawMenuBar( hWnd );
}   

