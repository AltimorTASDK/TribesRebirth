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

#include "MissionEditor.h"
#include "ts_camera.h"
#include "m_mul.h"
#include "simContainer.h"
#include "simEvDcl.h"
#include "simTerrain.h"
#include "simGuiTSCtrl.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "simSetIterator.h"
#include "simGame.h"
#include "netPacketStream.h"
#include "netGhostManager.h"

#define  EDITBOX_HEIGHT   20
#define  IDM_EDITBOX      IDM_MISSION_OBJ_BASE+3000
#define  IDC_TOOLBAR      IDM_MISSION_OBJ_BASE+3001

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack( push, 4 )
#endif

struct
{
   DLGTEMPLATE t;
   WORD        menu;
   WORD        classtype;
   wchar_t     caption[128];
}maskDlgTemplate;

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack( pop )
#endif

VectorPtr  <MissionEditor::MaskTypes*>   MissionEditor::maskTypes;   

bool MissionEditor::enabled = false;

//------------------------------------------------------------------------------

bool MissionEditor::create( GWWindow *parent, const char *_caption, RectI &r )
{
   //------------------------------------------------------------------------------
   // some code here to initialize Mission editor members
   hOptionsMenu    = CreateMenu();
   hObjMenu        = CreateMenu();
   hTerrainMenu    = CreateMenu();
   objMenu_IDM     = IDM_MISSION_OBJ_BASE;
   terrainMenu_IDM = IDM_MISSION_TERRAIN_BASE;
   parentCanvas    = dynamic_cast<SimGui::Canvas*>(parent);
   selectionSet.setParent( this );
   selectionSet.handCursor = SimResource::get(targetManager)->load("CursorPlus.bmp");
   selectionSet.handCursor->attribute |= BMA_TRANSPARENT;
   selectionSet.lockCursor = SimResource::get(targetManager)->load("AddState.bmp");
   selectionSet.lockCursor->attribute |= BMA_TRANSPARENT;
   selectionSet.oldCursor  = parentCanvas->getCursorBmp();
   missionFile.init( hWnd, "mis", FileInfo::TYPE_MIS, NULL );
   macroFile.init( hWnd, "mac", FileInfo::TYPE_MAC, NULL );

   //------------------------------------------------------------------------------
   // create the parent window
   caption = strnew( _caption );
   if( !Parent::createWin( parent, _caption, r, 
         WS_EX_OVERLAPPEDWINDOW | WS_EX_PALETTEWINDOW, 
         DS_3DLOOK, 
         TVS_HASLINES | TVS_HASBUTTONS | TVS_EDITLABELS | TVS_LINESATROOT ) )
         return( false );
   
   //------------------------------------------------------------------------------
   state.clear( ME_CAM_FOLLOW );
   state.clear( ME_SNAP_GRID );
   state.clear( ME_SNAP_ROT );
   state.clear( ME_ROT_X );
   state.clear( ME_ROT_Y );
   state.set( ME_ROT_Z );
   state.clear( STV_MODIFIED );
   state.set( SimTreeView::STV_UNTITLED );
   state.set( ME_SIMINPUTFOCUS );
   state.set( ME_DROP_CS );   // new objects to center of screen
   state.set( ME_DROP_TG );   // objects seek ground placement
   state.set( ME_SHOW_HANDLES );

   editBox.createWin( this, RectI(Point2I(0,getSize().y-EDITBOX_HEIGHT),getSize()), 0, 0, IDM_EDITBOX ); 

   RECT rt;
   GetClientRect( hWnd, &rt );
   MoveWindow(editBox.getHandle(),0,rt.bottom-editBox.getSize().y,(rt.right-rt.left),editBox.getSize().y, TRUE );
   tvSize.x = rt.right-rt.left;
   tvSize.y = rt.bottom-rt.top;
   MoveWindow(treeList.getHandle(), 0, 0, tvSize.x, tvSize.y-editBox.getSize().y, TRUE);

   SimObject *obj = targetManager->findObject("MissionGroup");
   if ( obj )        // if MissionGroup exists, add it to treeview
      addSet( obj, NULL );
   else
      PostMessage( hWnd, WM_COMMAND, IDM_NEW, NULL );

   show( SW_SHOW );
   
   SetFocus( parent->getHandle() );
   
   // autosave feature data - defaults to off
   autoSaveInterval = 0;
   autoSaveName = NULL;
   autoSaveTimer = 0;

   return true;
}   

//------------------------------------------------------------------------------

MissionEditor::~MissionEditor()
{
   delete [] autoSaveName;

   // remove the buttons from the toolbar
   removeButtons();
   
   delete [] caption;
   while( maskTypes.size() )
   {
      delete [] maskTypes.last()->description;
      delete maskTypes.last();
      maskTypes.pop_back();
   }
   while( objectMenus.size() )
   {
      delete [] objectMenus.last()->menuCategory;
      delete objectMenus.last();
      objectMenus.pop_back();
   }
   while( objMenuItem.size() )
   {
      delete [] objMenuItem.last()->consoleString;
      delete objMenuItem.last();
      objMenuItem.pop_back();
   }

   while( terrainMenus.size() )
   {
      delete [] terrainMenus.last()->menuCategory;
      delete terrainMenus.last();
      terrainMenus.pop_back();
   }
   while( terrainMenuItem.size() )
   {
      delete [] terrainMenuItem.last()->consoleString;
      delete terrainMenuItem.last();
      terrainMenuItem.pop_back();
   }
}   

//------------------------------------------------------------------------------
// adds a new object to the currently focused Set

SimObject* MissionEditor::addObject( SimObject *obj )
{
   HTREEITEM hParent = getSelection();

	if (!hParent) {
		SimObject* mobj = targetManager->findObject("MissionGroup");
	   if (mobj && (hParent = find(TreeView_GetRoot(treeList.getHandle()),
	   		mobj->getId())) != NULL )
	   	selectItem(hParent);
		else
			return (NULL);
	}
   if ( !isItemFolder(hParent) )
      hParent = getParent(hParent);
   if ( !hParent )
      return (NULL);

   // if the selected objected is not a group find it's parent
   SimObject *selected = getObject(hParent);
	lockManager();
   SimSet *ss = dynamic_cast<SimSet*>(selected);
   if (!ss)
      ss = selected->getGroup();

   // add the new object to the simSet and to the Editor set
   if (!ss->addObject( obj ))
   {
	   unlockManager();
      return (NULL);
   }
   obj->addToSet( SimEditSetId );

	unlockManager();

   selectItem( addSet( obj, hParent ) );

   SimEditEvent::post(obj,SimMissionEditorId,state.test(ME_SIMINPUTFOCUS) );

   // Move object based on camera drop preference
   selectionSet.clearSet();
   selectionSet.addObject( obj );
   if ( state.test(ME_DROP_AC) )       // drop at camera?
      onDropAtCamera();
   else if ( state.test(ME_DROP_CROT) )       // drop at camera with rotation?
      onDropThroughCamera();
   else if ( state.test(ME_DROP_BC) )  // drop on ground below camera
      onDropBelowCamera();
   else if ( state.test(ME_DROP_CS) )  // drop on ground at center screen
      onDropAtCenterScreen();

   if ( state.test(ME_DROP_TG) )
      onDropDown(obj);                 // put new object on ground

// try to force parent item to have a +- button to indicate children
   TV_ITEM *tvi = getItemInfo( hParent );
   tvi->mask = TVIF_CHILDREN|TVIF_HANDLE;
//   tvi->cChildren = I_CHILDRENCALLBACK;
   tvi->cChildren = 1;
   TreeView_SetItem( treeList.getHandle(), tvi );
   state.set(SimTreeView::STV_MODIFIED);
   return (obj);
}   

//------------------------------------------------------------------------------
// registers an object with MissionEditor and puts it on the MainMenu

void MissionEditor::registerObject( const char *menuCategory, const char *menuText, const char *consoleString )
{
   if ( !strcmpi(menuCategory,"separator") )
   {
      AppendMenu( hObjMenu, MF_SEPARATOR, NULL, NULL );
      DrawMenuBar( hWnd );
      return;      
   }

   // check to see if menuCategory is already registered
   int category = -1;
   for( int i=0; (category==-1) && (i<objectMenus.size()); i++ )
      if ( !strcmpi( menuCategory, objectMenus[i]->menuCategory ) )
         category = i;

   if ( category == -1 )  // if category is not registered, add it
   {
      MissionMenu *mm = new MissionMenu;
      mm->hMenu = CreateMenu();
      mm->menuCategory  = strnew( menuCategory );
      objectMenus.push_back( mm );    // add to list of menus

      AppendMenu( hObjMenu, MF_STRING|MF_POPUP, (UINT)mm->hMenu, mm->menuCategory );
      category = objectMenus.size()-1;
   }

   if ( !strcmpi(menuText,"separator") )
      AppendMenu( objectMenus[category]->hMenu, MF_SEPARATOR, NULL, NULL );
   else
   {
      AppendMenu( objectMenus[category]->hMenu, MF_STRING, objMenu_IDM, menuText );
      MissionMenuItem *mmi = new MissionMenuItem;
      mmi->consoleString = strnew( consoleString );
      objMenu_IDM++;
      objMenuItem.push_back( mmi );
   }

   DrawMenuBar( hWnd );
}   

//------------------------------------------------------------------------------
// registers SkyDefs with MissionEditor and puts it on the Terrain Menu

void MissionEditor::registerTerrain( const char *menuCategory, const char *menuText, const char *consoleString )
{
   if ( !strcmpi(menuCategory,"separator") )
   {
      AppendMenu( hObjMenu, MF_SEPARATOR, NULL, NULL );
      DrawMenuBar( hWnd );
      return;      
   }

   // check to see if menuCategory is already registered
   int category = -1;
   for( int i=0; (category==-1) && (i<terrainMenus.size()); i++ )
      if ( !strcmpi( menuCategory, terrainMenus[i]->menuCategory ) )
         category = i;

   if ( category == -1 )  // if category is not registered, add it
   {
      MissionMenu *mm = new MissionMenu;
      mm->hMenu = CreateMenu();
      mm->menuCategory  = strnew( menuCategory );
      terrainMenus.push_back( mm );    // add to list of menus

      AppendMenu( hTerrainMenu, MF_STRING|MF_POPUP, (UINT)mm->hMenu, mm->menuCategory );
      category = terrainMenus.size()-1;
   }

   if ( !strcmpi(menuText,"separator") )
      AppendMenu( terrainMenus[category]->hMenu, MF_SEPARATOR, NULL, NULL );
   else
   {
      AppendMenu( terrainMenus[category]->hMenu, MF_STRING, terrainMenu_IDM, menuText );
      MissionMenuItem *mmi = new MissionMenuItem;
      mmi->consoleString = strnew( consoleString );
      terrainMenu_IDM++;
      terrainMenuItem.push_back( mmi );
   }

   DrawMenuBar( hWnd );
}   

//------------------------------------------------------------------------------

void MissionEditor::onSize(UINT state, int cx, int cy)
{
   cx,cy; state;
   positionWindows();
}   

//------------------------------------------------------------------------------

void MissionEditor::positionWindows()
{   
   RECT r;
   GetClientRect( hWnd, &r );
   
   // check if the toolbar exists
   if( hToolbar && ::IsWindow( hToolbar ) )
   {
      SetWindowPos( hToolbar, NULL, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER );
      
      // adjust for the toolbar size
      RECT tr;
      GetWindowRect( hToolbar, &tr );
      r.top += tr.bottom - tr.top - 1;
   }
      
   if( IsWindow( editBox.getHandle() ) )
      MoveWindow(editBox.getHandle(),0,r.bottom-editBox.getSize().y,(r.right-r.left),editBox.getSize().y, TRUE );

//   tvSize.x = min( tvSize.x, (r.right-r.left) );
   tvSize.x = r.right-r.left;
   tvSize.y = r.bottom-r.top;
   if ( IsWindow(treeList.getHandle()) && IsWindow(editBox.getHandle()) )
      MoveWindow(treeList.getHandle(), 0, r.top, tvSize.x, tvSize.y-editBox.getSize().y, TRUE);
}

//------------------------------------------------------------------------------

void MissionEditor::onSetFocus(HWND hwndOldFocus)
{
   hwndOldFocus;
   //CMDConsole::getLocked()->evaluate("inputDeactivate mouse 0", false );
   //state.set(ME_SIMINPUTFOCUS,true);
   checkMenu( hMainMenu );
}   

//------------------------------------------------------------------------------

bool MissionEditor::onSimGainFocusEvent(const SimGainFocusEvent *event)
{
   enabled = true;
   state.set(ME_SIMINPUTFOCUS,true);
   checkMenu( hMainMenu );
   // broadcast edit event to every object
   SimEditEvent ev;
   ev.editorId = SimMissionEditorId;
   ev.state    = true;
   targetManager->processEvent(&ev);
   return ( Parent::processEvent(event) );
}   

//------------------------------------------------------------------------------

bool MissionEditor::onSimLoseFocusEvent(const SimLoseFocusEvent *event)
{
   enabled = false;
   state.set(ME_SIMINPUTFOCUS,false);
   checkMenu( hMainMenu );
   // broadcast edit event to every object
   SimEditEvent ev;
   ev.editorId = SimMissionEditorId;
   ev.state    = false;
   targetManager->processEvent(&ev);
   return ( Parent::processEvent(event) );
}   

//==============================================================================

//------------------------------------------------------------------------------
// if camera following, move camera to selected item

void MissionEditor::handleItemSelection( HTREEITEM hItem, bool dblClick )
{
   SimObject *obj = getSelectedObject();

   if ( dblClick && !dynamic_cast<SimSet*>(obj) )
      inspectObject( getObject( hItem ) );

   if (!(GetKeyState( VK_SHIFT )&0x80000000) && !(GetKeyState( VK_CONTROL )&0x80000000))
      selectionSet.clearSet();
   selectionSet.addObject( getObject(hItem) );

   if ( state.test(ME_CAM_FOLLOW) )
      onIDMCamMove();      // move camera to currently selected object
}   

//------------------------------------------------------------------------------

void MissionEditor::handleContextMenuItemInsert( HMENU hMenu )
{
   AppendMenu( hMenu, MF_STRING, MissionEditor::IDM_CAM_MOVE,     "Camera to Object" );
   AppendMenu( hMenu, MF_STRING, MissionEditor::IDM_DROP_CAM,     "Drop at Camera" );
   AppendMenu( hMenu, MF_STRING, MissionEditor::IDM_DROP_CAMROT,  "Drop with Rotate at Camera" );
   AppendMenu( hMenu, MF_STRING, MissionEditor::IDM_DROP_TERRAIN, "Drop to Terrain at Camera" );
   AppendMenu( hMenu, MF_STRING, MissionEditor::IDM_DROP_INFRONT, "Drop to Terrain at Screen Center" );
   AppendMenu( hMenu, MF_STRING, MissionEditor::IDM_DROP_DOWN,    "Drop to Terrain Below Object" );
   AppendMenu( hMenu, MF_SEPARATOR, 0, 0 );

   // add parents default menu items too
   Parent::handleContextMenuItemInsert( hMenu );

   // add our specific Sim menu items
   AppendMenu( hMenu, MF_SEPARATOR, 0, 0 );
   AppendMenu( hMenu, MF_STRING, MissionEditor::IDM_ADDTOGRP, "Add to Current Set" );
}   

//------------------------------------------------------------------------------

bool MissionEditor::handleContextMenuItemSelection( int id )
{
   switch( id )
   {
      case IDM_ADDTOGRP:
         {
            if ( selectionSet.size() )
            {
               addObject( selectionSet.front() );
               state.set(STV_MODIFIED);
            }
         }
         break;

      case IDM_DROP_CAM:   // drop object at current camera position
         onDropAtCamera();
         break;

      case IDM_DROP_CAMROT:  // drop object at current camera position and rotation
         onDropThroughCamera();
         break;

      case IDM_DROP_TERRAIN:
         onDropBelowCamera();
         break;

      case IDM_DROP_INFRONT:
         onDropAtCenterScreen();
         break;

      case IDM_DROP_DOWN:
         if ( state.test(ME_DROP_GR) ) // drop Group Relative to lowest object
            onDropDownRel();
         else
            onDropDown(&selectionSet);
         break;

      case IDM_CAM_MOVE:   // move camera to current object
         onIDMCamMove();
         break;

      default:
         // let parent handle its default menu items
         return ( Parent::handleContextMenuItemSelection( id ) );
   }
   return true;
}

//----------------------------------------------------------------------------

char* MissionEditor::handleItemNameChange( HTREEITEM hItem, char *name )
{
   if ( name == NULL )
      return NULL;
   SimObject *obj = getObject( hItem );
   if ( !obj )
      return NULL;

	lockManager();
   obj->assignName( name );
	unlockManager();

   name = (char*)avar("%4i,\"%s\", %s",obj->getId(),name,obj->getClassName());

   return name;
}   

//==============================================================================

bool MissionEditor::objectLoadPersist( const char * parentObj, const char * fileName )
{
   if( !fileName || !strlen( fileName ) )
      return( false );
      
   // allow the console to do this for us...
   Persistent::Base::Error err;
   Persistent::Base::openDictionary();
   SimObject * obj = (SimObject*)Persistent::Base::fileLoad( fileName, &err );
   Persistent::Base::closeDictionary();

   // check if loaded successfully
   if( !obj || err != Ok )
   {
      AssertWarn( 0, avar("MissionEditor::objectLoadPersist: Unable to load object:%s", fileName ) );
      return( false );
   }

   SimSet * parent = dynamic_cast<SimSet*>(targetManager->findObject( parentObj ));
   if( !parent )
      return( false );
      
   // add to this parent
   lockManager();
   parent->addObject( obj );
   unlockManager();

   return( true );
}

//------------------------------------------------------------------------------

const char * MissionEditor::stripPath( const char * file )
{
   AssertFatal( file, "Invalid filename." );
   
   char * end = const_cast< char *>( file + strlen( file ) );
   while( end != file )
   {
      if( *end == '\\' || *end == '/' || *end == ':' )
      {
         end++;
         break;
      }
      end--;
   }
   return( end );
}

//------------------------------------------------------------------------------

bool MissionEditor::onObjectLoad( FileInfo * fi, SimSet * parent )
{
   AssertFatal( parent, "Parent not found!" );
   
   if ( !fi->getName( FileInfo::OPEN_MODE, "Load Mission Object..." ) )
      return (NULL);

   // create a temporary group to add things to...
   SimGroup tmpParent;
   lockManager();
   targetManager->registerObject( &tmpParent );
   unlockManager();
   
   // call the script to save this
   CMDConsole * con = CMDConsole::getLocked();
   char idBuf[20];
   sprintf( idBuf, "%d", tmpParent.getId() );

   // have the script take care of what type of load this is
   con->executef( 3, "missionLoadObject", idBuf, stripPath( fi->fileName() ) );
   
   // check that the item was added   
   if( !tmpParent.size() )
   {
      con->printf( "missionLoadObject: Failed to load object from file (%s)", fi->fileName() );
      targetManager->unregisterObject( &tmpParent );
      return( false );
   }
   
	SimObject * obj = tmpParent.front();
   
   // remove it from this group and add into the real parent
   lockManager();
   tmpParent.removeObject( obj );
   targetManager->unregisterObject( &tmpParent );
   parent->getManager()->addObject( obj );
   parent->addObject( obj );
   unlockManager();
   
   // get the treeitem handle for the parent
   HTREEITEM hParent = findItem( parent );
   if( !hParent )
      return( false );
      
   // add to the treeview
   HTREEITEM hItem = addSet( obj, hParent );
   selectItem( hItem );
   SimEditEvent::post( parent, SimMissionEditorId, state.test( ME_SIMINPUTFOCUS ) );
   state.set(STV_MODIFIED);

   // set the state for the treeview      
   state.set(SimTreeView::STV_MODIFIED);
   state.set(SimTreeView::STV_UNTITLED);
   
   return( true );
}   

//------------------------------------------------------------------------------

bool MissionEditor::saveMission( const char * name )
{
   if( !name )
      return( false );
      
   SimObject * obj = targetManager->findObject( "MissionGroup" );
   if( !obj )
      return( false );
      
   // call the script to save this
   CMDConsole * con = CMDConsole::getLocked();
   
   // save the ted volume
   con->evaluate( "Ted::save();" );
   
   char idBuf[20];
   sprintf( idBuf, "%d", obj->getId() );
   con->executef( 3, "missionSaveObject", idBuf, name );
   
   return( true );
}

//------------------------------------------------------------------------------

bool MissionEditor::objectSavePersist( const char * objName, const char * fileName )
{
   if( !objName || !fileName || !strlen( fileName ) )
      return( false );
      
   state.clear(SimTreeView::STV_UNTITLED);
   
   SimObject * obj = targetManager->findObject( objName );
   if( !obj )
      return( false );
      
   // save it off
   Persistent::Base::openDictionary();
   bool result = obj->fileStore( fileName );
   Persistent::Base::closeDictionary();

   if ( result == Ok )
      state.clear(SimTreeView::STV_MODIFIED);
   else
      MessageBox( hWnd, avar("Error occured while trying to save %s.  File may be Read-Only.", fileName),"Error Saving...", MB_OK );
   
   return( result == Ok );
}

//------------------------------------------------------------------------------

bool MissionEditor::onObjectSave( FileInfo *fi, SimObject *obj )
{
   if ( state.test(SimTreeView::STV_UNTITLED) && !fi->getName( FileInfo::SAVE_MODE, "Save Mission Object" ) )
      return false;

   state.clear(SimTreeView::STV_UNTITLED);
   
   if ( !obj )
      return false;

   // call the script to save this
   CMDConsole * con = CMDConsole::getLocked();
   char idBuf[ 20 ];
   sprintf( idBuf, "%d", obj->getId() );
   con->executef( 3, "missionSaveObject", idBuf, fi->fileName() );
   
   // TODO - check the return value to see if should clear the modified flag
   //   state.clear(SimTreeView::STV_MODIFIED);
   return( true );
}   

// ---------------------------------------------------------
// go through and generate the complete name for an object...
const char * MissionEditor::getObjectFullName( SimObject * obj )
{
   static char nameBuf[ 1024 ];
   char scratchBuf[ 1024 ];

   // reset the namebuf...
   nameBuf[0] = '\0';
   
   // go back up to root and grab all the names
   while( obj && obj->getName() )
   {
      // tack this on... check for a root object
      if( strlen( nameBuf ) )
         sprintf( scratchBuf, "%s\\%s", obj->getName(), nameBuf );
      else
         sprintf( scratchBuf, "%s", obj->getName() );
         
      strcpy( nameBuf, scratchBuf );
      
      // go back
      obj = obj->getGroup();
   }
   
   return( nameBuf );
}
//------------------------------------------------------------------------------
// drop object at currently selected camera position

void MissionEditor::onDropAtCamera()
{
   SimObjectTransformQuery camQuery;

   // get transform of current camera
	CMDConsole *con   = CMDConsole::getLocked();
	const char *name  = con->getVariable( "MED::camera" );
   SimManager  *man  = SimGame::get()->getManager(SimGame::CLIENT);   
   SimObject *camera = man->findObject( name );

   if ( camera && camera->processQuery(&camQuery) )
   {  // get object from tree view
      camQuery.tmat.p -= selectionSet.centroidPt;
      // put it just below camera
      camQuery.tmat.p.z -= 15;      
      moveObjectRelative( &selectionSet, camQuery.tmat.p );
   }
   return;     // camera doesn't know how to give me its position
}   

//-------------------------------------------------------------
bool MissionEditor::getCameraTransform( TMat3F & transform )
{
   SimObjectTransformQuery camQuery;
   
   // get the camera object
   CMDConsole * con     = CMDConsole::getLocked();
   const char * name    = con->getVariable( "MED::camera" );
   SimManager * man     = SimGame::get()->getManager( SimGame::CLIENT );
   SimObject * camera   = man->findObject( name );
   
   if( camera && camera->processQuery( &camQuery ) )
   {
      transform = camQuery.tmat;
      return( true );
   }   
   
   return( false );
}

//-------------------------------------------------------------

SimObject * MissionEditor::getCamera()
{
   // get the camera object
   CMDConsole * con     = CMDConsole::getLocked();
   const char * name    = con->getVariable( "MED::camera" );
   SimManager * man     = SimGame::get()->getManager( SimGame::CLIENT );
   return( man->findObject( name ) );
}

//-------------------------------------------------------------

bool MissionEditor::setCameraTransform( TMat3F & transform )
{
   // get the camera object
   CMDConsole * con     = CMDConsole::getLocked();
   const char * name    = con->getVariable( "MED::camera" );
   SimManager * man     = SimGame::get()->getManager( SimGame::CLIENT );
   SimObject * camera   = man->findObject( name );
   
   // make sure got the camera
   if( camera )
   {
      SimObjectTransformEvent event( transform );
      camera->processEvent( &event );
      return( true );
   }   
   
   return( false );
}

//-------------------------------------------------------------

void MissionEditor::onDropThroughCamera()
{
   // turn off the seek ground
   state.clear( ME_DROP_TG );
   
   SimObjectTransformQuery camQuery;
   
   // get transform of current camera
	CMDConsole *con   = CMDConsole::getLocked();
	const char *name  = con->getVariable( "MED::camera" );
   SimManager  *man  = SimGame::get()->getManager(SimGame::CLIENT);   
   SimObject *camera = man->findObject( name );

   if( camera && camera->processQuery( &camQuery ) )
   {
      EulerF rot;
      camQuery.tmat.angles( &rot );
      
      setObject( &selectionSet, camQuery.tmat.p, rot );
   }   
   return;
}

// move object relative to current location by deltaPt
void MissionEditor::setObject( SimObject *obj, Point3F& pos, EulerF& rot )
{
   SimSet   *ss;
   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         setObject( (*itr), pos, rot ); // recurse into children

   SimObjectTransformQuery objQuery;

   // get the ransform
   if( !obj || !obj->processQuery( &objQuery ) )
      return;     

   SimObjectTransformEvent event( objQuery.tmat );

   event.tmat.set( rot, pos );

   obj->processEvent(&event);
   
   state.set(SimTreeView::STV_MODIFIED);
}   


//------------------------------------------------------------------------------
// drop object on ground below currently selected camera position

void MissionEditor::onDropBelowCamera()
{
   SimObjectTransformQuery camQuery;

   // get transform of current camera
	CMDConsole *con   = CMDConsole::getLocked();
	const char *name  = con->getVariable( "MED::camera" );
   SimManager  *man  = SimGame::get()->getManager(SimGame::CLIENT);   
   SimObject *camera = man->findObject( name );

   if ( camera && camera->processQuery(&camQuery) )
   {  // do a LOS collision directly below the camera
      Point3F endPt = camQuery.tmat.p;
      endPt.z -= 10000;

      SimCollisionInfo info;
      SimContainerQuery query;
      query.id = camera->getId();   // don't collide with camera
      query.type = -1;
      query.box.fMin = camQuery.tmat.p;
      query.box.fMax = endPt;
      query.mask = selectionSet.getPlaceMask();

	   lockManager();
      SimContainer *root = NULL;
      
      root = findObject(targetManager, SimRootContainerId, root);
      if ( root->findLOS(query, &info) && (info.object->getType() & selectionSet.getPlaceMask()) )
      {
         // Translate collision to world coordinates and move object there
         Point3F  pt;
         m_mul(info.surfaces[0].position, info.surfaces.tWorld, &pt );
         pt -= selectionSet.centroidPt;
         moveObjectRelative( &selectionSet, pt );
      }   
	   unlockManager();
   }

   return;     // camera doesn't know how to give me its position
}   

//------------------------------------------------------------------------------
// drop object on ground below currently selected object

void MissionEditor::onDropDown(SimObject *obj )
{
   SimSet   *ss;
   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         onDropDown( (*itr) ); // recurse into children

   SimObjectTransformQuery objQuery;

   if ( !obj || !obj->processQuery(&objQuery) )
      return;     // doesn't know how to give me its position

   // get objects orientation, move to position
   SimObjectTransformEvent event(objQuery.tmat);

   Point3F endPt = objQuery.tmat.p;
   endPt.z -= 10000;

   SimCollisionInfo info;
   SimContainerQuery query;
   query.id = obj->getId();   // don't collide with object
   query.type = -1;
   query.box.fMin = objQuery.tmat.p;
   query.box.fMax = endPt;
   query.mask = selectionSet.getPlaceMask();

   lockManager();
   SimContainer *root = NULL;
   
   root = findObject(targetManager, SimRootContainerId, root);
   for( int i = 0; i < 2; i++ )
   {
      if ( root->findLOS(query, &info) && (info.object->getType() & selectionSet.getPlaceMask()) )
      {
         // Translate collision to world coordinates and move object there
         Point3F  pt;
         m_mul(info.surfaces[0].position, info.surfaces.tWorld, &pt );
         pt -= objQuery.tmat.p;
         moveObjectRelative( obj, pt );
         break;
      }   
      
      // attempt drop from far above...
      query.box.fMin.z += 10000;
   }
   unlockManager();

   return;     // camera doesn't know how to give me its position
}   

//------------------------------------------------------------------------------
// drop objects to ground relative to lowest object in group

void MissionEditor::onDropDownRel()
{
   Point3F  pt(0,0, selectionSet.findLowest(&selectionSet) );
   moveObjectRelative( &selectionSet, pt );
}   

//------------------------------------------------------------------------------

void MissionEditor::onDropAtCenterScreen()
{
   SimObjectTransformQuery camQuery;

   // get transform of current camera
	CMDConsole *con   = CMDConsole::getLocked();
	const char *name  = con->getVariable( "MED::camera" );
   SimManager  *man  = SimGame::get()->getManager(SimGame::CLIENT);   
   SimObject *camera = man->findObject( name );

   if ( camera && camera->processQuery(&camQuery) )
   {  // do a LOS collision directly below the camera
      Point3F endPt;
      m_mul( Point3F(0,200,0), camQuery.tmat, &endPt );

      SimCollisionInfo info;
      SimContainerQuery query;
      query.id = camera->getId();   // don't collide with camera
      query.type = -1;
      query.box.fMin = camQuery.tmat.p;
      query.box.fMax = endPt;
      query.mask = selectionSet.getPlaceMask();

	   lockManager();
      SimContainer *root = NULL;
      
      root = findObject(targetManager, SimRootContainerId, root);
      if ( root->findLOS(query, &info) && (info.object->getType() & selectionSet.getPlaceMask()) )
      {
         // Translate collision to world coordinates and move object there
         Point3F  pt;
         m_mul(info.surfaces[0].position, info.surfaces.tWorld, &pt );
         pt -= selectionSet.centroidPt;
         moveObjectRelative( &selectionSet, pt );
      }   
      else
      {
         endPt -= selectionSet.centroidPt;
         moveObjectRelative( &selectionSet, endPt );
      }
	   unlockManager();
   }

   return;     // camera doesn't know how to give me its position
}   

//------------------------------------------------------------------------------
// move camera to currently selected object(s) centroid

bool MissionEditor::onIDMCamMove()
{
   SimObjectTransformQuery camQuery;

   // get transform of current object
	CMDConsole *con = CMDConsole::getLocked();
	const char *name = con->getVariable( "MED::camera" );
   SimManager  *man = SimGame::get()->getManager(SimGame::CLIENT);   
   SimObject *camera = man->findObject( name );
   if ( !camera || !camera->processQuery(&camQuery) )
      return false;     // doesn't know how to give me its position

   float z = camQuery.tmat.p.z;
   camQuery.tmat.p = selectionSet.centroidPt; // move to centroid position
   camQuery.tmat.p.z = max(z,(selectionSet.centroidPt.z+40));  // position camera above object

   // check to see if anything between object and camera
   SimCollisionInfo collInfo;
   SimContainerQuery query;
   query.id = -1;
   query.box.fMax = camQuery.tmat.p;
   query.box.fMin = selectionSet.centroidPt;
   query.mask = selectionSet.getPlaceMask();
	query.detail = SimContainerQuery::DefaultDetail;

	lockManager();
   SimContainer *root = NULL;
   
   root = findObject(targetManager, SimRootContainerId, root);
   root->findLOS(query, &collInfo, SimCollisionImageQuery::Editor);
	unlockManager();
   if ( collInfo.object )
      m_mul(collInfo.surfaces[0].position, collInfo.surfaces.tWorld, &camQuery.tmat.p );

   moveObjectToPoint( camera, camQuery.tmat.p );
//   EulerF rot(float(-M_PI/2.0),0.0f,0.0f);   // rotate camera to looking down
//   rotateObject( camera, rot );
   return true;
}   

//------------------------------------------------------------------------------

bool MissionEditor::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
      onEvent(Sim3DMouseEvent);
      onEvent(SimEditEvent);
      onEvent(SimGainFocusEvent);
      onEvent(SimLoseFocusEvent);
	}
   return false;
}   

//------------------------------------------------------------------------------

bool MissionEditor::onAdd()
{
	if (!Parent::onAdd())
		return false;
   addToSet( SimRenderSetId );
   addToSet( SimInputConsumerSetId );
   SimEditEvent ev;
   ev.editorId = SimMissionEditorId;
   ev.state    = true;
   targetManager->processEvent(&ev);
   enabled = true;
	return true;
}   

//------------------------------------------------------------------------------

void MissionEditor::onRemove()
{
   removeFromSet( SimRenderSetId );
   removeFromSet( SimInputConsumerSetId );
	Parent::onRemove();
}   

//------------------------------------------------------------------------------

bool MissionEditor::onSimEditEvent(const SimEditEvent *event)
{
   static bool inRecurse = false;
   if ( inRecurse )
      return ( Parent::processEvent(event) );

   if (event->editorId == id)
   {
      state.set( ME_SIMINPUTFOCUS, event->state );
      checkMenu( hMainMenu );
      enabled = state.test( ME_SIMINPUTFOCUS );
      lockManager();
      // broadcast edit event to every object
      SimEditEvent ev;
      ev.editorId = SimMissionEditorId;
      ev.state    = enabled;
      inRecurse = true;
      targetManager->processEvent(&ev);
      inRecurse = false;
      unlockManager();
   }
   return ( Parent::processEvent(event) );
}   

//------------------------------------------------------------------------------

bool MissionEditor::onSim3DMouseEvent(const Sim3DMouseEvent *event)
{
   if ( !state.test(ME_SIMINPUTFOCUS) )
      return false;

   selectionSet.handleSim3DMouseEvent( event );
   return false;
}   

//------------------------------------------------------------------------------

bool MissionEditor::getLOSIntercept( const Sim3DMouseEvent *event, SimCollisionInfo *info, DWORD allowedTypes )
{
   Point3F  endPt = event->direction;
   endPt *= 10000;          // scale line segment
   endPt += event->startPt; // add to starting point
   
   SimContainerQuery query;
	if (targetManager == manager)
      // don't collide with this ID (the camera)
	   query.id = (dynamic_cast<SimGui::TSControl*>(event->sender))->getObject()->getId();
	else
		// Don't collide with camera ghost?
		// Should think about this one
		query.id = -1;
   query.type = -1;
   query.box.fMax = endPt;
   query.box.fMin = event->startPt;
   query.mask = allowedTypes;
	query.detail = SimContainerQuery::DefaultDetail;

	if ( !lockManager() )
      return false;

	// Run the LOS on the client, then find out which 
	// object it is on the server.
   info->object = NULL;
   SimContainer *root = NULL;
   
   root = findObject(manager, SimRootContainerId, root);
   if ( root->findLOS(query, info, SimCollisionImageQuery::Editor) &&
	   	(info->object->getType() & allowedTypes) ) {
		if (SimNetObject* no = dynamic_cast<SimNetObject*>(info->object)) {
			DWORD index = no->getNetIndex();
			if (index != DWORD(-1))
				if (SimObject* obj = targetManager->findObject(2049))
					if (Net::PacketStream* psc = dynamic_cast<Net::PacketStream*>(obj))
						info->object = psc->getGhostManager()->resolveGhostParent(index);
		}
  	}

	unlockManager();
   return info->object != NULL;
}

//------------------------------------------------------------------------------
// move object to desired location
void  MissionEditor::moveObjectToPoint( SimObject *obj, Point3F& pt )
{
   SimSet   *ss;
   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         moveObjectToPoint( (*itr), pt ); // recurse into children

   SimObjectTransformQuery objQuery;

   if ( !obj || !obj->processQuery(&objQuery) )
      return;     // doesn't know how to give me its position

   // get objects orientation, move to position
   SimObjectTransformEvent event(objQuery.tmat);

   // values constrained to a particular axis
   if (state.test(ME_CONST_X))
      pt.x = event.tmat.p.x;
   if (state.test(ME_CONST_Y))
      pt.y = event.tmat.p.y;
   if (state.test(ME_CONST_Z))
      pt.z = event.tmat.p.z;

   event.tmat.p = pt;
	event.tmat.flags |= TMat3F::Matrix_HasTranslation;

   obj->processEvent(&event);
   state.set(SimTreeView::STV_MODIFIED);
}   

//------------------------------------------------------------------------------
// move object relative to current location by deltaPt
void MissionEditor::moveObjectRelative( SimObject *obj, Point3F& deltaPt )
{
   SimSet   *ss;
   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         moveObjectRelative( (*itr), deltaPt ); // recurse into children

   SimObjectTransformQuery objQuery;

   if ( !obj || !obj->processQuery(&objQuery) )
      return;     // doesn't know how to give me its position

   // get objects orientation, move to position
   SimObjectTransformEvent event(objQuery.tmat);

   // values constrained to a particular axis
   if (state.test(ME_CONST_X))
      deltaPt.x = 0;
   if (state.test(ME_CONST_Y))
      deltaPt.y = 0;
   if (state.test(ME_CONST_Z))
      deltaPt.z = 0;

   event.tmat.p += deltaPt;
	event.tmat.flags |= TMat3F::Matrix_HasTranslation;

   obj->processEvent(&event);
   state.set(SimTreeView::STV_MODIFIED);
}   

//------------------------------------------------------------------------------
// rotate object relative to a point by the angle provided

void  MissionEditor::rotateObject( SimObject *obj, Point3F& anchor, AngleF angle )
{
   SimSet   *ss;
   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         rotateObject( (*itr), anchor, angle ); // recurse into children

   SimObjectTransformQuery objQuery;
   SimObjectTransformEvent event(objQuery.tmat);

   if ( !obj || !obj->processQuery(&objQuery) )
      return;

   TMat3F tmat(true);
   tmat.preRotateZ(angle);

   objQuery.tmat.p -= anchor;
   m_mul( objQuery.tmat, tmat, &event.tmat );
   event.tmat.p += anchor;

	event.tmat.flags |= TMat3F::Matrix_HasTranslation|TMat3F::Matrix_HasRotation;

   obj->processEvent(&event);
   state.set(SimTreeView::STV_MODIFIED);
}   

//------------------------------------------------------------------------------
// rotate object relative to angle provided

void  MissionEditor::rotateObject( SimObject *obj, AngleF angle )
{
   SimObjectTransformQuery objQuery;
   SimObjectTransformEvent event(objQuery.tmat);

   if ( !obj || !obj->processQuery(&objQuery) )
      return;

   // get objects positon, rotate to orientation
   event.tmat.identity();
   event.tmat.preRotateZ( angle );
   event.tmat.p = objQuery.tmat.p;
	event.tmat.flags |= TMat3F::Matrix_HasTranslation;

   obj->processEvent(&event);
   state.set(SimTreeView::STV_MODIFIED);
}

//------------------------------------------------------------------------------
// adjust the rotation of an object
void MissionEditor::rotateObject( SimObject *obj, Point3F& anchor, EulerF& rot )
{
   SimSet   *ss;
   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         rotateObject( (*itr), anchor, rot ); // recurse into children

   SimObjectTransformQuery objQuery;
   SimObjectTransformEvent event(objQuery.tmat);

   if ( !obj || !obj->processQuery(&objQuery) )
      return;

   TMat3F tmat(true);
   tmat.preRotateX(rot.x);
   tmat.preRotateY(rot.y);
   tmat.preRotateZ(rot.z);

   objQuery.tmat.p -= anchor;
   m_mul( objQuery.tmat, tmat, &event.tmat );
   event.tmat.p += anchor;

	event.tmat.flags |= TMat3F::Matrix_HasTranslation|TMat3F::Matrix_HasRotation;

   obj->processEvent(&event);
   state.set(SimTreeView::STV_MODIFIED);
}

//------------------------------------------------------------------------------
// set the rotation of an object
void  MissionEditor::rotateObject( SimObject *obj, EulerF& angle )
{
   // recurse into children
   SimSet   *ss;
   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         rotateObject( (*itr), angle ); 
         
   SimObjectTransformQuery objQuery;

   if( !obj || !obj->processQuery(&objQuery) )
      return;

   // get objects positon, rotate to orientation
   SimObjectTransformEvent event(objQuery.tmat);
   event.tmat.set( angle, objQuery.tmat.p );

   obj->processEvent(&event);
   state.set(SimTreeView::STV_MODIFIED);
}

//------------------------------------------------------------------------------
// rotate object with snapping - rot is passed in already clipped to snap angle
void  MissionEditor::rotateObject( SimObject *obj, EulerF& rot, EulerF& snap )
{
   // recurse into children
   SimSet   *ss;
   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         rotateObject( (*itr), rot, snap );
         
   SimObjectTransformQuery objQuery;

   if( !obj || !obj->processQuery(&objQuery) )
      return;

   // get objects positon, rotate to orientation
   SimObjectTransformEvent event( objQuery.tmat );

   EulerF angles;
   objQuery.tmat.angles( &angles );

   // snap them
   ( angles.x < 0.f ) ? ( angles.x -= snap.x / 4 ) : ( angles.x += snap.x / 4 );
   ( angles.y < 0.f ) ? ( angles.y -= snap.y / 4 ) : ( angles.y += snap.y / 4 );
   ( angles.z < 0.f ) ? ( angles.z -= snap.z / 4 ) : ( angles.z += snap.z / 4 );
   angles.x = angles.x - fmod( angles.x, snap.x );
   angles.y = angles.y - fmod( angles.y, snap.y );
   angles.z = angles.z - fmod( angles.z, snap.z );
      
   angles += rot;
      
   // set the values
   event.tmat.set( angles, objQuery.tmat.p );
   
   obj->processEvent(&event);
   state.set(SimTreeView::STV_MODIFIED);
}

//------------------------------------------------------------------------------

void MissionEditor::RenderImage::render( TSRenderContext &rc )
{
	// XXX This is not a good idea (looping through target
	// manager on the other thread).
   for (SimSetIterator i(targetManager); *i; ++i)
   {
      SimObjectTransformQuery objQuery;
      GFXSurface *sfc = rc.getSurface();
      TSPointArray *pa = rc.getPointArray();
      if( (*i)->processQuery(&objQuery) )
      {
         // it has a transform...
         int index = me->handleGetBitmapIndex( (*i), false );
         
         GFXBitmap *bmp = me->bitmapList[index];

         TS::TransformedVertex &v = 
            pa->getTransformedVertex(pa->addPoint(objQuery.tmat.p));
         if(v.fStatus & TS::TransformedVertex::Projected)
         {
            Point2I ul(v.fPoint.x, v.fPoint.y);
            Point2I lr;
            if (bmp)
            {
               lr =  Point2I(bmp->getWidth() >> 1, bmp->getHeight() >> 1);
               ul -= Point2I(bmp->getWidth() >> 2, bmp->getHeight() >> 2);
               lr += ul;
               sfc->drawBitmap2d(bmp, &ul);
            }
            else
            {
               lr  = Point2I(6,6);
               ul -= Point2I(3,3);
               lr += ul;
               sfc->drawRect2d_f(&RectI(ul, lr), 255);
            }
         }
      }
   }   
}

bool MissionEditor::onSimRenderQueryImage(SimRenderQueryImage *query)
{
   if(!state.test(ME_SIMINPUTFOCUS) || !state.test(ME_SHOW_HANDLES))
      return false;
	query->count = 1;
	query->image[0] = &renderImage;
   renderImage.itype = SimRenderImage::Overlay;
   renderImage.targetManager = targetManager;
   renderImage.me = this;
	return true;
}

bool MissionEditor::processQuery( SimQuery* query )
{
   switch ( query->type )
   {
      onQuery( SimRenderQueryImage );
      onQuery( SimInputPriorityQuery );
   }
   return ( Parent::processQuery( query ) );
}

//------------------------------------------------------------------------------

bool MissionEditor::onSimInputPriorityQuery( SimInputPriorityQuery *query )
{
   query->priority = SI_PRIORITY_NORMAL+1;
   return true;
}   

//------------------------------------------------------------------------------

void MissionEditor::initMenu( HMENU hMainMenu )
{
   // Make "New Mission" first item on the File menu
   AppendMenu( hFileMenu, MF_STRING, MissionEditor::IDM_NEW,        "New Mission" );
   AppendMenu( hFileMenu, MF_STRING, MissionEditor::IDM_LOAD,       "Load..." );
   AppendMenu( hFileMenu, MF_STRING, MissionEditor::IDM_SAVE,       "Save" );
   AppendMenu( hFileMenu, MF_STRING, MissionEditor::IDM_SAVE_AS,    "Save As..." );
   AppendMenu( hFileMenu, MF_SEPARATOR, 0, 0 );
   AppendMenu( hFileMenu, MF_STRING, MissionEditor::IDM_LOAD_MACRO, "Load Macro..." );
   AppendMenu( hFileMenu, MF_STRING, MissionEditor::IDM_SAVE_MACRO, "Save Macro..." );
   AppendMenu( hFileMenu, MF_SEPARATOR, 0, 0 );

   AppendMenu( hEditMenu, MF_STRING, MissionEditor::IDM_CAM_MOVE,          "Camera to Object" );
   AppendMenu( hEditMenu, MF_STRING, MissionEditor::IDM_DROP_CAM,          "Drop at Camera" );
   AppendMenu( hEditMenu, MF_STRING, MissionEditor::IDM_DROP_CAMROT,       "Drop with rotate at Camera" );
   AppendMenu( hEditMenu, MF_STRING, MissionEditor::IDM_DROP_TERRAIN,      "Drop to Terrrain at Camera" );
   AppendMenu( hEditMenu, MF_STRING, MissionEditor::IDM_DROP_INFRONT,      "Drop to Terrain at Screen Center" );
   AppendMenu( hEditMenu, MF_STRING, MissionEditor::IDM_DROP_DOWN,         "Drop to Terrain Below Object" );
   AppendMenu( hEditMenu, MF_SEPARATOR, 0, 0 );

   Parent::initMenu( hMainMenu );

   // add these to the bottom of the edit menu
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_EDITOR,         "Show Edit Objects" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_SHOW_HANDLES,   "Show Grab Handles" );
   AppendMenu( hOptionsMenu, MF_SEPARATOR, 0, 0 );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_SNAP_GRID,      "Snap to Grid" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_SNAP_ROT,       "Snap Rotations" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_GRANULARITY,    "Set Grid Snap Granuarity..." );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_ROTSETTINGS,    "Set Rotation Settings..." );
   AppendMenu( hOptionsMenu, MF_SEPARATOR, 0, 0 );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CONST_X,        "Constrain X-axis" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CONST_Y,        "Constrain Y-axis" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CONST_Z,        "Constrain Z-axis" );
   AppendMenu( hOptionsMenu, MF_SEPARATOR, 0, 0 );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_ROT_X,          "Rotate X-axis" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_ROT_Y,          "Rotate Y-axis" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_ROT_Z,          "Rotate Z-axis" );
   AppendMenu( hOptionsMenu, MF_SEPARATOR, 0, 0 );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CAM_FOLLOW,     "Camera Following" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CAM_DROP_A,     "Drop At Camera" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CAM_DROP_ROT,   "Drop with Rotate At Camera" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CAM_DROP_B,     "Drop Below Camera x Terrain" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CAM_DROP_C,     "Drop to Terrain at Screen Center" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CAM_DROP_T,     "Objects Snap To Terrain" );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_CAM_DROP_R,     "Drop Groups Relative to Lowest" );
   AppendMenu( hOptionsMenu, MF_SEPARATOR, 0, 0 );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_GRAB_MASK,      "Set Mask for Grab..." );
   AppendMenu( hOptionsMenu, MF_STRING, MissionEditor::IDM_PLACE_MASK,     "Set Mask for Placement..." );

   InsertMenu( hMainMenu, 2, MF_BYPOSITION|MF_POPUP, (UINT)hOptionsMenu,   "Options" );
   InsertMenu( hMainMenu, 3, MF_BYPOSITION|MF_POPUP, (UINT)hObjMenu,       "Objects" );
   InsertMenu( hMainMenu, 4, MF_BYPOSITION|MF_POPUP, (UINT)hTerrainMenu,   "Terrain" );

   DrawMenuBar( hWnd );
}   

//------------------------------------------------------------------------------

int MissionEditor::onNotify( int id, LPNMHDR pnmhdr )
{
   id;

   // get the help text from the button
   LPTOOLTIPTEXT lpToolTipText = ( LPTOOLTIPTEXT )pnmhdr;
   if( lpToolTipText->hdr.code ==  TTN_NEEDTEXT )
   {
      PSTR help = NULL;
      
      for( int i = 0; i < toolbarButtons.size(); i++ )
      {
         if( lpToolTipText->hdr.idFrom == toolbarButtons[i].id )
         {
            help = toolbarButtons[i].help;
            break;
         }
      }
      
      lpToolTipText->lpszText = help;
      return( true );
   }
   return( Parent::onNotify( id, pnmhdr ) );
}

//------------------------------------------------------------------------------
// handle the sc_close notification for saving of the mission..
void MissionEditor::onSysCommand(UINT cmd, int x, int y)
{
   switch( cmd )
   {
      case SC_CLOSE:
      {
			lockManager();
         
         // check if the mission has been modified
         SimObject * obj = targetManager->findObject( "MissionGroup" );
         if( obj && state.test( SimTreeView::STV_MODIFIED ) )
         {
            // prompt for saving of mission
            if( IDYES == MessageBox( hWnd, "File modified.  Do you wish to save?", "Save File?", MB_YESNO ) )
            {
               // save it
               if ( !onObjectSave( &missionFile, obj ) )
                  MessageBox( hWnd, "Error occured while trying to save the Mission.  File may be Read-Only.", 
                     "Error Saving...", MB_OK );
            } 
         }
         
         unlockManager();
      }
   }
   
   Parent::onSysCommand( cmd, x, y );
}

//------------------------------------------------------------------------------

void MissionEditor::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   switch( id )
   {
      case IDM_NEW:
         {
				lockManager();
            SimObject *obj = targetManager->findObject( "MissionGroup" );
            if ( obj && state.test(SimTreeView::STV_MODIFIED) )
               if ( IDNO == MessageBox( hWnd, "Existing mission not saved.  Continue?", "Create New Mission", MB_YESNO ) )
                  break;
            if ( obj )
            {
               SimGroup *set = dynamic_cast<SimGroup*>(obj);
               targetManager->deleteObject( set );
            }
            clear(); // remove existing tree
            obj = new SimGroup;
      		targetManager->addObject(obj);
            targetManager->assignName( obj, "MissionGroup" );
            targetManager->assignId( obj, SimMissionGroupId );
            HTREEITEM hItem = addSet( obj );  // add to treeview and select
            selectItem( hItem );
            state.set(SimTreeView::STV_MODIFIED);
            state.set(SimTreeView::STV_UNTITLED);
            CMDConsole::getLocked()->evaluate("focusServer();", false );
            CMDConsole::getLocked()->evaluate("MissionInit();", false );
            CMDConsole::getLocked()->evaluate("focusClient();", false );
				unlockManager();
         }
         break;

      case IDM_LOAD:
         {
				lockManager();
            SimObject *oldobj = targetManager->findObject( "MissionGroup" );
            if ( oldobj && state.test(SimTreeView::STV_MODIFIED) && 
                 IDNO == MessageBox( hWnd, "Do you want to clobber existing base tree?","Remove Old, Load New...", MB_YESNO ) )
               break;
            selectionSet.clearSet();
            onObjectLoad( &missionFile, NULL );
//            SimObject *obj = onObjectLoad( &missionFile );
//            if ( obj )
//            {
//               if ( oldobj )
//                  oldobj->deleteObject();
//               clear(); // remove existing tree
//               obj = dynamic_cast<SimGroup*>(obj);
//               if ( !obj )
//               {
//                  MessageBox( hWnd, "You can only load SimGroup here.  Use Load Macro...","Error Loading Mission...", MB_OK );
//                  delete obj;
//                  break;
//               }
//               targetManager->addObject( obj );
//               targetManager->assignName( obj, "MissionGroup" );
//               targetManager->assignId( obj, SimMissionGroupId );
//               HTREEITEM hItem = addSet( obj );  // add to treeview and select
//               selectItem( hItem );
//               SimEditEvent::post(obj,SimMissionEditorId,state.test(ME_SIMINPUTFOCUS) );
//               selectItem( hItem );
//               state.clear(SimTreeView::STV_MODIFIED);
//               state.clear(SimTreeView::STV_UNTITLED);
//            }
				unlockManager();
         }
         break;

      case IDM_SAVE:
         {
				lockManager();
            SimObject *obj = targetManager->findObject( "MissionGroup" );
            if ( obj )
               if ( !onObjectSave( &missionFile, obj ) )
                  MessageBox( hWnd, "Error occured while trying to save the Mission.  File may be Read-Only.","Error Saving...", MB_OK );
				unlockManager();
         }
         break;

      case IDM_SAVE_AS:
         {
				lockManager();
            SimObject *obj = targetManager->findObject( "MissionGroup" );
            if ( obj )
            {
               state.set(SimTreeView::STV_MODIFIED); // fake it into thinking it needs to save
               state.set(SimTreeView::STV_UNTITLED); // fake it into thinking it needs a name
               if ( !onObjectSave( &missionFile, obj ) )
                  MessageBox( hWnd, "Error occured while trying to save the Mission.  File may be Read-Only.","Error Saving...", MB_OK );
            }
				unlockManager();
         }
         break;

      case IDM_LOAD_MACRO:
         {
            SimObject *pSet = getSelectedObject();
            if ( !pSet )
            {
               MessageBox( hWnd, "Must have something selected to use Load Macro.  Use \"Load\", or select something","Error Loading Macro...", MB_OK );
               break;
            }
            selectionSet.clearSet();
            HTREEITEM hParent = getSelection();
            if( !isItemFolder( hParent ) )
               hParent = getParent( hParent );

            SimSet *set = ( SimSet* )getObject( hParent );
            onObjectLoad( &macroFile, set );
            
//            SimObject *obj = onObjectLoad( &macroFile );
//            if ( obj )
//            {
//               HTREEITEM hParent = getSelection();              // get object as an hTree item
//               if ( !isItemFolder( hParent ) )
//                  hParent = getParent( hParent );     // get items parent on the tree
//
//               SimSet *set = (SimSet*)getObject( hParent );   // get object associated with this hitem
//					lockManager();
//               set->getManager()->addObject( obj );
//               set->addObject( obj );
//					unlockManager();
//               HTREEITEM hItem = addSet( obj, hParent );  // add to treeview and select
//               selectItem( hItem );
//               SimEditEvent::post(set,SimMissionEditorId,state.test(ME_SIMINPUTFOCUS) );
//               state.set(STV_MODIFIED);
//            }
         }
         break;

      case IDM_SAVE_MACRO:
         {
            SimObject *obj;
            if ( (obj = getSelectedObject()) != NULL )
               if ( IDOK == MessageBox( hWnd, avar("Save %4i,\"%s\", %s?",obj->getId(),obj->getName(),obj->getClassName()), "Saving Macro", MB_OKCANCEL ) )
               {
                  state.set(SimTreeView::STV_MODIFIED);  // fake it into thinking it needs to save
                  state.set(SimTreeView::STV_UNTITLED);  // fake it into thinking it needs to save
                  if ( !onObjectSave( &macroFile, obj ) )
                     MessageBox( hWnd, "Error occured while trying to save the Macro.  File may be Read-Only.","Error Saving...", MB_OK );
               }
         }
         break;

      case IDM_EDITBOX:
         switch( codeNotify )
         {
            case EN_MAXTEXT:   // <Return> pressed
               {
                  char buff[256];
                  GetWindowText( editBox.getHandle(), buff, 256 );
						lockManager();
                  CMDConsole::getLocked()->evaluate("focusServer();", false );
                  CMDConsole::getLocked()->evaluate( buff, false );
                  CMDConsole::getLocked()->evaluate("focusClient();", false );
						unlockManager();
                  SetWindowText( editBox.getHandle(), "" );
               }
               break;
         }
         break;

      case IDM_EDITOR:
         {
            state.toggle(ME_SIMINPUTFOCUS);
            enabled = state.test( ME_SIMINPUTFOCUS );
				lockManager();
            // broadcast edit event to every object
            SimEditEvent ev;
            ev.editorId = SimMissionEditorId;
            ev.state    = enabled;
            targetManager->processEvent(&ev);
 				unlockManager();
         }
         break;

      case IDM_CAM_FOLLOW:
         state.toggle(ME_CAM_FOLLOW);
         break;

      case IDM_CAM_DROP_A:
      case IDM_CAM_DROP_B:
      case IDM_CAM_DROP_C:
      case IDM_CAM_DROP_ROT:
         state.set(ME_DROP_AC, id==IDM_CAM_DROP_A);
         state.set(ME_DROP_BC, id==IDM_CAM_DROP_B);
         state.set(ME_DROP_CS, id==IDM_CAM_DROP_C);
         state.set(ME_DROP_CROT, id==IDM_CAM_DROP_ROT);
         break;

      case IDM_CAM_DROP_T:
         state.toggle(ME_DROP_TG);
         break;

      case IDM_CAM_DROP_R:
         state.toggle(ME_DROP_GR);

      case IDM_SNAP_GRID:
         state.toggle(ME_SNAP_GRID);
         break;

      case IDM_SNAP_ROT:
         state.toggle(ME_SNAP_ROT);
         break;

      
      case IDM_GRANULARITY:
         {
         maskDlgTemplate.t.style   = DS_3DLOOK|WS_POPUP|WS_BORDER|
	         	WS_SYSMENU|WS_CAPTION|DS_MODALFRAME|WS_VISIBLE;
         maskDlgTemplate.t.cdit    = 0;
         maskDlgTemplate.t.x       = 10;
         maskDlgTemplate.t.y       = 10;
         maskDlgTemplate.t.cx      = 300;
         maskDlgTemplate.t.cy      = 150;
         maskDlgTemplate.menu      = 0;
         maskDlgTemplate.classtype = 0;
         wcscpy( maskDlgTemplate.caption, L"Granularity Settings" );
         Point3F grain = selectionSet.getGranularity();
         if ( IDOK == DialogBoxIndirectParam( GWMain::hInstance, 
         	(DLGTEMPLATE*)&maskDlgTemplate, hWnd, 
         		setGrainDlgProc, (LPARAM)&grain ) )
            selectionSet.setGranularity( grain );
         }
         SetFocus( parentCanvas->getHandle() );
         break;

      case IDM_ROTSETTINGS:
         {
            maskDlgTemplate.t.style   = DS_3DLOOK|WS_POPUP|WS_BORDER|
	            	WS_SYSMENU|WS_CAPTION|DS_MODALFRAME|WS_VISIBLE;
            maskDlgTemplate.t.cdit    = 0;
            maskDlgTemplate.t.x       = 10;
            maskDlgTemplate.t.y       = 10;
            maskDlgTemplate.t.cx      = 300;
            maskDlgTemplate.t.cy      = 150;
            maskDlgTemplate.menu      = 0;
            maskDlgTemplate.classtype = 0;
            wcscpy( maskDlgTemplate.caption, L"Rotation Settings" );
            DialogBoxIndirectParam( GWMain::hInstance, 
               ( DLGTEMPLATE* )&maskDlgTemplate, hWnd, 
               setRotationDlgProc, ( LPARAM )&selectionSet );
         }
         
         SetFocus( parentCanvas->getHandle() );
         
         break;
         
      case IDM_CONST_X:
         state.toggle(ME_CONST_X);
         break;

      case IDM_CONST_Y:
         state.toggle(ME_CONST_Y);
         break;

      case IDM_CONST_Z:
         state.toggle(ME_CONST_Z);
         break;

      case IDM_ROT_X:
         state.toggle(ME_ROT_X);
         break;

      case IDM_ROT_Y:
         state.toggle(ME_ROT_Y);
         break;
         
      case IDM_ROT_Z:
         state.toggle(ME_ROT_Z);
         break;
         
      case IDM_CUT:
         Persistent::Base::openDictionary();
         ((SimSet)selectionSet).fileStore( "clip.tmp" );
         Persistent::Base::closeDictionary();
         selectionSet.deleteSet();
         break;

      case IDM_COPY:
         Persistent::Base::openDictionary();
         ((SimSet)selectionSet).fileStore( "clip.tmp" );
         Persistent::Base::closeDictionary();
         break;

      case IDM_DELETE:
         selectionSet.deleteSet();
         break;

//      case IDM_REMOVE:
//         break;

      case IDM_DUPLICATE:
         Persistent::Base::openDictionary();
         ((SimSet)selectionSet).fileStore( "clip.tmp" );
         Persistent::Base::closeDictionary();
         onCommand( IDM_PASTE, hwndCtl, codeNotify );
         break;

      case IDM_PASTE:
         {
            selectionSet.clearSet();
            Persistent::Base::Error err;
            Persistent::Base::openDictionary();
            SimObject *obj = (SimObject*)Persistent::Base::fileLoad( "clip.tmp", &err );
            Persistent::Base::closeDictionary();
            if ( err != Ok )
               return;

            SimSet *ss = dynamic_cast<SimSet*>(obj);
            if ( !ss )
               return;

            HTREEITEM hParent = getSelection();
            if ( !isItemFolder(hParent) )
               hParent = getParent( hParent );
            SimSet *prnt = (SimSet*)getObject( hParent );

            for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
            {
               prnt->getManager()->addObject( (*itr) );
   				prnt->addObject((*itr));
               addSet( (*itr), hParent );
               selectionSet.addObject( (*itr) );
               SimEditEvent::post((*itr),SimMissionEditorId,state.test(ME_SIMINPUTFOCUS) );
            }
            state.set(STV_MODIFIED);
         }
         break;

      case IDM_GRAB_MASK:
         {
         maskDlgTemplate.t.style   = DS_3DLOOK|WS_POPUP|WS_BORDER|
	         	WS_SYSMENU|WS_CAPTION|DS_MODALFRAME|WS_VISIBLE;
         maskDlgTemplate.t.cdit    = 0;
         maskDlgTemplate.t.x       = 10;
         maskDlgTemplate.t.y       = 10;
         maskDlgTemplate.t.cx      = 300;
         maskDlgTemplate.t.cy      = 150;
         maskDlgTemplate.menu      = 0;
         maskDlgTemplate.classtype = 0;
         wcscpy(maskDlgTemplate.caption, L"Grab Mask Settings");
         DWORD mask = selectionSet.getGrabMask();
         if ( IDOK == DialogBoxIndirectParam( GWMain::hInstance, 
         	(DLGTEMPLATE*)&maskDlgTemplate, hWnd, 
         		setMaskDlgProc, (LPARAM)&mask ) )
            selectionSet.setGrabMask( mask );
         }
         SetFocus( parentCanvas->getHandle() );
         break;

      case IDM_PLACE_MASK:
         {
         maskDlgTemplate.t.style   = DS_3DLOOK|WS_POPUP|WS_BORDER|
         		WS_SYSMENU|WS_CAPTION|DS_MODALFRAME|WS_VISIBLE;
         maskDlgTemplate.t.cdit    = 0;
         maskDlgTemplate.t.x       = 10;
         maskDlgTemplate.t.y       = 10;
         maskDlgTemplate.t.cx      = 300;
         maskDlgTemplate.t.cy      = 150;
         maskDlgTemplate.menu      = 0;
         maskDlgTemplate.classtype = 0;
         
         wcscpy(maskDlgTemplate.caption, L"Place Mask Settings");
         DWORD mask = selectionSet.getPlaceMask();
         if ( IDOK == DialogBoxIndirectParam( GWMain::hInstance, 
         	(DLGTEMPLATE*)&maskDlgTemplate, hWnd, 
         		setMaskDlgProc, (LPARAM)&mask ) )
            selectionSet.setPlaceMask( mask );
         }
         SetFocus( parentCanvas->getHandle() );
         break;

      case IDM_SHOW_HANDLES:
         state.toggle(ME_SHOW_HANDLES);
         break;

      case IDM_DROP_CAM:   // drop object at current camera position
         onDropAtCamera();
         break;

      case IDM_DROP_CAMROT:   // drop object at current camera position and rotation
         onDropThroughCamera();
         break;

      case IDM_DROP_TERRAIN:
         onDropBelowCamera();
         break;

      case IDM_DROP_INFRONT:
         onDropAtCenterScreen();
         break;

      case IDM_DROP_DOWN:
         if ( state.test(ME_DROP_GR) ) // drop Group Relative to lowest object
            onDropDownRel();
         else
            onDropDown(&selectionSet);
         break;

      case IDM_CAM_MOVE:   // move camera to current object
         onIDMCamMove();
         break;
         
      default:    // probably is a registered menu console command
         if ( id >= IDM_MISSION_OBJ_BASE && id < IDM_MISSION_OBJ_BASE+1000 )
         {
            int   index = id-IDM_MISSION_OBJ_BASE;
				lockManager();
            CMDConsole::getLocked()->evaluate("focusServer();", false );
            CMDConsole::getLocked()->evaluate( objMenuItem[index]->consoleString, false );
            CMDConsole::getLocked()->evaluate("focusClient();", false );
				unlockManager();
         }   
         else if ( id >= IDM_MISSION_TERRAIN_BASE && id < IDM_MISSION_TERRAIN_BASE+1000 )
         {
            int   index = id-IDM_MISSION_TERRAIN_BASE;
				lockManager();
            CMDConsole::getLocked()->evaluate("focusServer();", false );
            CMDConsole::getLocked()->evaluate( terrainMenuItem[index]->consoleString, false );
            CMDConsole::getLocked()->evaluate("focusClient();", false );
				unlockManager();
         }   
         else if( id >= IDM_MISSION_BUTTON_BASE && id < IDM_MISSION_BUTTON_BASE+1000 )
         {
            // got a button id - now try and find the button
            for( int i = 0; i < toolbarButtons.size(); i++ )
               if( toolbarButtons[i].id == ( unsigned int )id )
               {
                  // check for a toggle button then determine the state of this button
                  if( toolbarButtons[i].type == ToolbarButton::Toggle )
                  {
                     // check the state and call the command responsible for it
                     if( ::SendMessage( hToolbar, TB_ISBUTTONCHECKED, id, 0 ) )
                        CMDConsole::getLocked()->evaluate( toolbarButtons[i].commandA, false );
                     else
                        CMDConsole::getLocked()->evaluate( toolbarButtons[i].commandB, false );
                  }
                  else
                     CMDConsole::getLocked()->evaluate( toolbarButtons[i].commandA, false );
               }
                  
            break;
         }
         
         checkMenu( hMainMenu );
         Parent::onCommand( id, hwndCtl, codeNotify );
         break;
   }
   checkMenu( hMainMenu );
}   

//------------------------------------------------------------------------------

void MissionEditor::checkMenu( HMENU hMenu )
{
   if ( !IsMenu(hMenu) )
      return;

   UINT  flag;
   
	CMDConsole *con = CMDConsole::getLocked();
	const char *name = con->getVariable( "MED::camera" );
   SimManager  *man = SimGame::get()->getManager(SimGame::CLIENT);   
   SimObject *camera = man->findObject( name );
   flag = (camera != NULL)? MF_ENABLED:MF_GRAYED;

   EnableMenuItem( hMenu, IDM_DROP_CAM,      flag );
   EnableMenuItem( hMenu, IDM_DROP_CAMROT,   flag );
   EnableMenuItem( hMenu, IDM_DROP_TERRAIN,  flag );
   EnableMenuItem( hMenu, IDM_DROP_INFRONT,  flag );
   EnableMenuItem( hMenu, IDM_CAM_MOVE,      flag );

   //-------------------------------------------
   if ( !IsMenu(hOptionsMenu) )
      return;

   CheckMenuItem( hOptionsMenu, IDM_CAM_FOLLOW, state.test(ME_CAM_FOLLOW)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_SNAP_GRID,  state.test(ME_SNAP_GRID)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_SNAP_ROT,    state.test(ME_SNAP_ROT)? MF_CHECKED:MF_UNCHECKED );

   CheckMenuItem( hOptionsMenu, IDM_CONST_X, state.test(ME_CONST_X)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_CONST_Y, state.test(ME_CONST_Y)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_CONST_Z, state.test(ME_CONST_Z)? MF_CHECKED:MF_UNCHECKED );

   CheckMenuItem( hOptionsMenu, IDM_ROT_X, state.test(ME_ROT_X)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_ROT_Y, state.test(ME_ROT_Y)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_ROT_Z, state.test(ME_ROT_Z)? MF_CHECKED:MF_UNCHECKED );

   CheckMenuItem( hOptionsMenu, IDM_CAM_DROP_A, state.test(ME_DROP_AC)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_CAM_DROP_ROT, state.test(ME_DROP_CROT)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_CAM_DROP_B, state.test(ME_DROP_BC)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_CAM_DROP_C, state.test(ME_DROP_CS)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_CAM_DROP_T, state.test(ME_DROP_TG)? MF_CHECKED:MF_UNCHECKED );
   CheckMenuItem( hOptionsMenu, IDM_CAM_DROP_R, state.test(ME_DROP_GR)? MF_CHECKED:MF_UNCHECKED );

   CheckMenuItem( hOptionsMenu, IDM_EDITOR, state.test(ME_SIMINPUTFOCUS)? MF_CHECKED:MF_UNCHECKED );

   CheckMenuItem( hOptionsMenu, IDM_SHOW_HANDLES, state.test(ME_SHOW_HANDLES)? MF_CHECKED:MF_UNCHECKED );
}   

//------------------------------------------------------------------------------

BOOL CALLBACK MissionEditor::setMaskDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   BOOL  msgProcessed;
   RECT  r;
   static   UINT *mask;

   msgProcessed = FALSE;
   switch (msg) 
   {
      case WM_INITDIALOG:         
         {
            mask = (UINT*)lParam; // pointer to mask variable
            GetWindowRect(hWnd, &r);

            r.bottom = ((80 + 20*maskTypes.size() + GetSystemMetrics(SM_CYSIZEFRAME)+
                         GetSystemMetrics(SM_CYFIXEDFRAME))*LOWORD(GetDialogBaseUnits()))/8;
            r.right = 320;
            r.left = (GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left)) / 2;
            r.top  = (GetSystemMetrics(SM_CYSCREEN) - (r.bottom - r.top)) / 2;

            for ( int i=0; i<maskTypes.size(); i++ )
            {
               CreateWindow( "BUTTON", maskTypes[i]->description, 
                             BS_AUTOCHECKBOX|WS_VISIBLE|WS_CHILD,
                             10,10+20*i,300,20,
                             hWnd, (HMENU)(1000+i), GWMain::hInstance, NULL );
               CheckDlgButton( hWnd, (1000+i), (*mask&(maskTypes[i]->mask))? MF_CHECKED:MF_UNCHECKED );
            }
            CreateWindow( "BUTTON", "Ok", 
                          BS_DEFPUSHBUTTON|WS_VISIBLE|WS_CHILD,
                          r.right/2-30,20+20*maskTypes.size(),60,30,
                          hWnd, (HMENU)IDOK, GWMain::hInstance, NULL );

            SetWindowPos(hWnd, NULL, r.left,r.top,r.right,r.bottom, SWP_NOZORDER);
            msgProcessed = TRUE;
         }
         break;

      case WM_COMMAND:            
         switch(LOWORD(wParam))
         {
            case IDCANCEL:
               EndDialog(hWnd, wParam);         
               break;

            case IDOK:
               {
                  int val = 0;
                  for ( int i=0; i<maskTypes.size(); i++ )
                     val |= IsDlgButtonChecked(hWnd,1000+i)? maskTypes[i]->mask:0;

                  *mask = val;
                  EndDialog(hWnd, wParam);
                  msgProcessed = TRUE;
               }
               break;  
         }
         break;
   }
   return (msgProcessed);               
}   

//------------------------------------------------------------------------------

BOOL CALLBACK MissionEditor::setGrainDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   BOOL  msgProcessed;
   RECT  r;
   static   Point3F *grain;

   msgProcessed = FALSE;
   switch (msg) 
   {
      case WM_INITDIALOG:         
         {
            // fill dlg with buttons and resize dialog box
            grain = (Point3F*)lParam;
            GetWindowRect(hWnd, &r);
            r.bottom = ((80 + 20*4 + GetSystemMetrics(SM_CYSIZEFRAME)+
                         GetSystemMetrics(SM_CYFIXEDFRAME))*LOWORD(GetDialogBaseUnits()))/8;
            r.right = 320;
            r.left = (GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left)) / 2;
            r.top  = (GetSystemMetrics(SM_CYSCREEN) - (r.bottom - r.top)) / 2;
            CreateWindow( "STATIC", "X grain: ", WS_VISIBLE|WS_CHILD, 10,10+22*0,300,20,hWnd, (HMENU)(-1), GWMain::hInstance, NULL );
            CreateWindow( "EDIT", avar("%5.3f",grain->x), WS_VISIBLE|WS_CHILD, 100,10+22*0,200,20,hWnd, (HMENU)(1000), GWMain::hInstance, NULL );
            CreateWindow( "STATIC", "Y grain: ", WS_VISIBLE|WS_CHILD, 10,10+22*1,300,20,hWnd, (HMENU)(-1), GWMain::hInstance, NULL );
            CreateWindow( "EDIT", avar("%5.3f",grain->y), WS_VISIBLE|WS_CHILD, 100,10+22*1,200,20,hWnd, (HMENU)(1001), GWMain::hInstance, NULL );
            CreateWindow( "STATIC", "Z grain: ", WS_VISIBLE|WS_CHILD, 10,10+22*2,300,20,hWnd, (HMENU)(-1), GWMain::hInstance, NULL );
            CreateWindow( "EDIT", avar("%5.3f",grain->z), WS_VISIBLE|WS_CHILD, 100,10+22*2,200,20,hWnd, (HMENU)(1002), GWMain::hInstance, NULL );
            CreateWindow( "BUTTON", "Ok", BS_DEFPUSHBUTTON|WS_VISIBLE|WS_CHILD,10,20+22*3,60,30,hWnd, (HMENU)IDOK, GWMain::hInstance, NULL );
            CreateWindow( "BUTTON", "Use Terrain Grid", BS_DEFPUSHBUTTON|WS_VISIBLE|WS_CHILD,100,20+22*3,150,30,hWnd, (HMENU)IDYES, GWMain::hInstance, NULL );
            SetWindowPos(hWnd, NULL, r.left,r.top,r.right,r.bottom, SWP_NOZORDER);
            msgProcessed = TRUE;
         }
         break;

      case WM_COMMAND:            
         switch(LOWORD(wParam))
         {
            case IDCANCEL:
               EndDialog(hWnd, wParam);         
               break;

            case IDYES: // use default terrain grid values
               {
                  SimManager *targetManager = SimGame::get()->getManager(SimGame::SERVER);
                  if ( targetManager )
                  {
                     SimObject *obj = targetManager->findObject( SimTerrainId );
                     if ( obj )
                     {
                        SimTerrain *st = dynamic_cast<SimTerrain*>(obj);
                        if ( st )
                        {
                           int scale = st->getGridFile()->getScale();
                           grain->x = grain->y = 1<<scale;
                           SetDlgItemText(hWnd,1000,avar("%5.3f",grain->x));
                           SetDlgItemText(hWnd,1001,avar("%5.3f",grain->y));
                        }
                     }
                  }
               }
               break;

            case IDOK:
               {
                  char  buff [40];
                  GetDlgItemText(hWnd,1000,buff,40); grain->x = atof( buff );
                  GetDlgItemText(hWnd,1001,buff,40); grain->y = atof( buff );
                  GetDlgItemText(hWnd,1002,buff,40); grain->z = atof( buff );
                  EndDialog(hWnd, wParam);
                  msgProcessed = TRUE;
               }
               break;  
         }
         break;
   }
   return (msgProcessed);               
}   

//------------------------------------------------------------------------------

float MissionEditor::atofDif( float val, const char * buff )
{
   char floatBuf[64];
   sprintf( floatBuf, "%f", val );
   if( !stricmp( floatBuf, buff ) )
      return( val );
      
   return( atof( buff ) );
}

BOOL CALLBACK MissionEditor::setRotationDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   BOOL  msgProcessed;
   RECT  r;
   static   SelectionSet * selectionSet;
   static   Point3F     degreeSnap;
   static   float       granularity;

   msgProcessed = FALSE;
   switch (msg) 
   {
      case WM_INITDIALOG:         
         {
            // fill dlg with buttons and resize dialog box
            selectionSet = ( SelectionSet * )lParam;

            // get the base values
            degreeSnap = selectionSet->getRotDegreeSnap();
            granularity = selectionSet->getRotGranularity();
            
            GetWindowRect(hWnd, &r);
            r.bottom = ( ( 80 + 20 * 4 + GetSystemMetrics( SM_CYSIZEFRAME ) +
               GetSystemMetrics( SM_CYFIXEDFRAME ) ) * LOWORD( GetDialogBaseUnits() ) ) / 8;
            r.right = 320;
            r.left = ( GetSystemMetrics( SM_CXSCREEN ) - ( r.right - r.left ) ) / 2;
            r.top  = ( GetSystemMetrics( SM_CYSCREEN ) - ( r.bottom - r.top ) ) / 2;
            CreateWindow( "STATIC", "X Degree Snap: ", WS_VISIBLE|WS_CHILD, 10,10+22*0,300,20,hWnd, (HMENU)(-1), GWMain::hInstance, NULL );
            CreateWindow( "EDIT", avar("%f",degreeSnap.x), WS_VISIBLE|WS_CHILD, 130,10+22*0,200,20,hWnd, (HMENU)(1000), GWMain::hInstance, NULL );
            CreateWindow( "STATIC", "Y Degree Snap: ", WS_VISIBLE|WS_CHILD, 10,10+22*1,300,20,hWnd, (HMENU)(-1), GWMain::hInstance, NULL );
            CreateWindow( "EDIT", avar("%f",degreeSnap.y), WS_VISIBLE|WS_CHILD, 130,10+22*1,200,20,hWnd, (HMENU)(1001), GWMain::hInstance, NULL );
            CreateWindow( "STATIC", "Z Degree Snap: ", WS_VISIBLE|WS_CHILD, 10,10+22*2,300,20,hWnd, (HMENU)(-1), GWMain::hInstance, NULL );
            CreateWindow( "EDIT", avar("%f",degreeSnap.z), WS_VISIBLE|WS_CHILD, 130,10+22*2,200,20,hWnd, (HMENU)(1002), GWMain::hInstance, NULL );
            CreateWindow( "STATIC", "Granularity: ", WS_VISIBLE|WS_CHILD, 10,10+22*3,300,20,hWnd, (HMENU)(-1), GWMain::hInstance, NULL );
            CreateWindow( "EDIT", avar("%f",granularity), WS_VISIBLE|WS_CHILD, 130,10+22*3,200,20,hWnd, (HMENU)(1003), GWMain::hInstance, NULL );

            CreateWindow( "BUTTON", "Ok", BS_DEFPUSHBUTTON|WS_VISIBLE|WS_CHILD,10,20+22*4,60,30,hWnd, (HMENU)IDOK, GWMain::hInstance, NULL );
            SetWindowPos(hWnd, NULL, r.left,r.top,r.right,r.bottom, SWP_NOZORDER);
            msgProcessed = TRUE;
         }
         break;

      case WM_COMMAND:            
         switch(LOWORD(wParam))
         {
            case IDCANCEL:
               EndDialog(hWnd, wParam);         
               break;

            case IDOK:
               {
                  char  buff [40];
                  GetDlgItemText(hWnd,1000,buff,40); 
                  degreeSnap.x = atofDif( degreeSnap.x, buff );
                  GetDlgItemText(hWnd,1001,buff,40); 
                  degreeSnap.y = atofDif( degreeSnap.y, buff );
                  GetDlgItemText(hWnd,1002,buff,40); 
                  degreeSnap.z = atofDif( degreeSnap.z, buff );
                  GetDlgItemText(hWnd,1003,buff,40); 
                  granularity = atofDif( granularity, buff );
                  
                  // set the data
                  selectionSet->setRotDegreeSnap( degreeSnap );
                  selectionSet->setRotGranularity( granularity );
                  
                  EndDialog(hWnd, wParam);
                  msgProcessed = TRUE;
               }
               break;  
         }
         break;
   }
   return (msgProcessed);               
}   

//------------------------------------------------------------------------------

int MissionEditor::handleGetBitmapIndex( SimObject *obj, bool selected )
{
   if ( !selected )
      selected = selectionSet.isObjectSelected( obj->getId() );
   return ( SimTreeView::handleGetBitmapIndex( obj, selected ) );
}   

//------------------------------------------------------------------------------

int MissionEditor::ToolbarButton::maxButtons    = 32;
int MissionEditor::ToolbarButton::width         = 24;
int MissionEditor::ToolbarButton::height        = 22;
int MissionEditor::ToolbarButton::currentID     = 0;

//------------------------------------------------------------------------------

bool MissionEditor::addButton( const char * name, const char * bmp, const char * help, 
   const char * commandA, const char * commandB )
{
	CMDConsole * con   = CMDConsole::getLocked();

   // make sure a unique button
   if( getButton( name ) )
   {
      con->printf( "addMissionButton: button '%s' already exists", name );
      return( false );
   }

   Resource <GFXBitmap> pBM;
   
   // check if adding a seperator
   if( bmp )
   {
      // load in the bmp
      ResourceManager * rm = SimResource::get( manager );
      pBM = rm->load( bmp);
      if ( !( bool )pBM )
      {
         con->printf( "addMissionButton: failed to load bitmap '%s'", bmp );
         return( false );
      }
   }
   
   // check for valid window
   if( !( getHandle() && ::IsWindow( getHandle() ) ) )
      return( false );

   // check if too many buttons
   if( toolbarButtons.size() >= ToolbarButton::maxButtons )
      return( false );

   // check for an id
   if( ToolbarButton::currentID >= 1000 )
   {
      con->printf( "addMissionButton: too many registered buttons" );
      return( false );
   }   
   
   // create a bitmap for use with this bitmap
   GFXPalette p;
   p.setWindowsColors();

   HBITMAP hBM;
   TBADDBITMAP tbAddBM;
   
   // check for seperator
   if( bmp )
   {
      struct   // BITMAPINFOHEADER
      {
         BITMAPINFOHEADER  bmiHeader;
         RGBQUAD           bmiColors[256];
      }bi;

      // fill the bitmap header info
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

      // set the color's
      for ( int i=0; i<256; i++ )
      {
         bi.bmiColors[i].rgbRed     = p.palette[0].color[i].peRed;
         bi.bmiColors[i].rgbGreen   = p.palette[0].color[i].peGreen;
         bi.bmiColors[i].rgbBlue    = p.palette[0].color[i].peBlue;
         bi.bmiColors[i].rgbReserved = 0;
      }

      // create a dib 
      HWND hw = getHandle();
      HDC dc = ::GetDC( hw );
      hBM = ::CreateDIBitmap( dc, &bi.bmiHeader, CBM_INIT, pBM->pBits, (LPBITMAPINFO)&bi, DIB_RGB_COLORS );
      ::ReleaseDC( hw, dc );

      // fill in the bitmap info
      tbAddBM.hInst = NULL;
      tbAddBM.nID   = ( UINT )hBM;
   }

   // fill in the button info
   TBBUTTON tbButton;
   tbButton.idCommand   = ToolbarButton::currentID + IDM_MISSION_BUTTON_BASE;
   tbButton.fsState     = TBSTATE_ENABLED;
   tbButton.dwData      = 0L;
   tbButton.iString     = 0;

   // get the type of button
   if( !bmp )
      tbButton.fsStyle = TBSTYLE_SEP;
   else
      tbButton.fsStyle = commandB ? TBSTYLE_CHECK : TBSTYLE_BUTTON;
   
   // add the button     
   if( toolbarButtons.size() == 0 )
   {
      // check if a sep button, cannot be added as first button ( needs an image )
      if( !bmp )
      {
         con->printf( "addButton: cannot add seperator as first item" );
         return( false );
      }
      
      // check if already exists - shouldn't
      if( getToolbarHandle() && ::IsWindow( getToolbarHandle() ) )
         return( false );
      
      // create a toolbar
      if( getHandle() && ::IsWindow( getHandle() ) ) 
      {
         show( SW_SHOW );
         update();
         RECT rc;
         GetClientRect( getHandle(), &rc );

         HWND toolres;
      
         //create the toolbar
         tbButton.iBitmap = 0;
         toolres = CreateToolbarEx( hWnd, WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS,
            IDC_TOOLBAR, 1, NULL, ( UINT )hBM, &tbButton, 1, ToolbarButton::width, ToolbarButton::height, 
            ToolbarButton::width, ToolbarButton::height, sizeof( TBBUTTON ) );

         if ( !toolres )
         {
            con->printf( "addButton: could not create tool bar" );
            return(false);
         }

         hToolbar = toolres;
         positionWindows();
      }
   }
   else
   {
      // add a button
      ( bool )::SendMessage( getToolbarHandle(), TB_BUTTONSTRUCTSIZE, sizeof( TBBUTTON ), ( long ) 0 ); 
      if( ( bool )pBM )  
         tbButton.iBitmap = SendMessage( getToolbarHandle(), TB_ADDBITMAP, ( WORD )1, ( long )&tbAddBM );
      else
         tbButton.iBitmap = 0;
         
      // now add the button
      if( !( bool )::SendMessage( getToolbarHandle(), TB_ADDBUTTONS, ( WORD )1, ( long )&tbButton ) )
      {
         con->printf( "addButton: failed to add button '%s'", name );
         return( false );
      }
   }

   // insert the button into the list - only has to have name
   ToolbarButton btn;
   btn.id = ToolbarButton::currentID + IDM_MISSION_BUTTON_BASE;
   btn.name = new char[ strlen( name ) + 1 ]; strcpy( btn.name, name );
   btn.help = NULL;
   btn.commandA = NULL;
   btn.commandB = NULL;
   
   // check if the params are there
   if( help )
   {
      btn.help = new char[ strlen( help ) + 1 ]; 
      strcpy( btn.help, help );
   }
   if( commandA )
   {
      btn.commandA = new char[ strlen( commandA ) + 1 ]; 
      strcpy( btn.commandA, commandA );
   }
   
   if( commandB )
   {
      btn.commandB = new char[ strlen( commandB ) + 1 ];
      strcpy( btn.commandB, commandB );
   }

   // set the button type - poochy way to do this but that's the console for ya...
   if( !bmp )
      btn.type = ToolbarButton::Gap;
   else if( !commandB )
      btn.type = ToolbarButton::Normal;
   else
      btn.type = ToolbarButton::Toggle;
   
   toolbarButtons.push_back( btn );

   // in the number of buttons
   ToolbarButton::currentID++;
   
   return( true );
}

//------------------------------------------------------------------------------

bool MissionEditor::removeButtons()
{
   while( toolbarButtons.size() )
   {
      if( !removeButton( toolbarButtons[0].name ) )
         return( false );
   }
   return( true );
}

//------------------------------------------------------------------------------

bool MissionEditor::removeButton( const char * name )
{
   // find the button - cannot use getbutton because this is looking for the index only
   for( int i = 0; i < toolbarButtons.size(); i++ )
   {
      if( !stricmp( name, toolbarButtons[i].name ) )
      {
         if( hToolbar && ::IsWindow( hToolbar ) )
         {
            if( !SendMessage( hToolbar, TB_DELETEBUTTON, i, 0 ) )
            {
               CMDConsole::getLocked()->printf( "removeMissionButton: failed to remove button '%s'", name );
               return( false );
            }
            
            // check if the toolbar should go away
            if( toolbarButtons.size() == 1 )
            {
               DestroyWindow( hToolbar );
               hToolbar = NULL;   
               positionWindows();
            }
         }
            
         // clear up some mem and remove from the button list
         delete [] toolbarButtons[i].name;
         delete [] toolbarButtons[i].help;
         delete [] toolbarButtons[i].commandA;
         delete [] toolbarButtons[i].commandB;
         toolbarButtons.erase(i);
      
         return( true );
      }
   }
  
   CMDConsole::getLocked()->printf( "removeMissionButton: button '%s' not in toolbar", name );
   return( false );
}

//--------------------------------------------------------------------------------------

bool MissionEditor::setButtonChecked( const char * name, bool check )
{
   ToolbarButton * button = getButton( name );
   
   if( !button )
   {
      CMDConsole::getLocked()->printf( "setButtonChecked: button '%s' not in toolbar", name );
      return( false );
   }
   
   if( hToolbar && ::IsWindow( hToolbar ) )
   {
      if( button->type == ToolbarButton::Toggle )
         ::SendMessage( hToolbar, TB_CHECKBUTTON, button->id, MAKELONG( check, 0 ) );
      else
         check = true;
         
      // process the command
      CMDConsole::getLocked()->evaluate( check ? button->commandA : button->commandB, false );

      return( true );               
   }
   else
      return( false );
}

//--------------------------------------------------------------------------------------
// check if the button is checked ( has been pressed for the toggle buttons )

bool MissionEditor::isButtonChecked( const char * name )
{
   ToolbarButton * button = getButton( name );
   if( !button )
   {
      CMDConsole::getLocked()->printf( "isButtonChecked: button '%s' not in toolbar", name );
      return( false );
   }   
   return( ::SendMessage( hToolbar, TB_ISBUTTONCHECKED, button->id, 0 ) );
}

//--------------------------------------------------------------------------------------
// gray a button

bool MissionEditor::setButtonEnabled( const char * name, bool enable )
{
   ToolbarButton * button = getButton( name );
   if( !button )
   {
      CMDConsole::getLocked()->printf( "setButtonEnabled: button '%s' not in toolbar", name );
      return( false );
   }   
   ::SendMessage( hToolbar, TB_ENABLEBUTTON, button->id, MAKELONG( enable, 0 ) );
   return( true );
}

//--------------------------------------------------------------------------------------
// check if a button has been grayed out

bool MissionEditor::isButtonEnabled( const char * name )
{
   ToolbarButton * button = getButton( name );
   return( ::SendMessage( hToolbar, TB_ISBUTTONENABLED, button->id, 0 ) );
}

//--------------------------------------------------------------------------------------
// get a named button 

MissionEditor::ToolbarButton * MissionEditor::getButton( const char * name )
{
   if( !name )
      return( NULL );
      
   for( int i = 0; i < toolbarButtons.size(); i++ )
      if( !stricmp( name, toolbarButtons[i].name ) )
         return( &toolbarButtons[i] );
         
   return( NULL );
}

//--------------------------------------------------------------------------------------
// AUTOSAVE features
//--------------------------------------------------------------------------------------

bool MissionEditor::setAutoSaveInterval( int min )
{
   if( min < 0 )
      return( false );
      
   // set the number of minutes between save's
   autoSaveInterval = min;

   // post the event
   postAutoSaveEvent();
   
   return( true );
}

//--------------------------------------------------------------------------------------

bool MissionEditor::setAutoSaveName( const char * name )
{  
   if( !name || !strlen( name ) )
      return( false );
   
   // remove the current name
   if( autoSaveName )
      delete [] autoSaveName;
 
   // create and copy the name     
   autoSaveName = new char[ strlen( name ) + 1 ];
   strcpy( autoSaveName, name );
   
   return( true );
}

//------------------------------------------------------------------------------

void MissionEditor::postAutoSaveEvent()
{
   if( autoSaveTimer )
   {
      ::KillTimer( hWnd, AutoSaveTimerID );
      autoSaveTimer = 0;
   }
      
   // make sure not turning off...
   if( autoSaveInterval )
      autoSaveTimer = ::SetTimer( hWnd, AutoSaveTimerID, 
         ( 1000 * 60 * autoSaveInterval ), NULL );
}

//------------------------------------------------------------------------------

void MissionEditor::onTimer( UINT id )
{
   if( id == AutoSaveTimerID )
   {
      // save out the mission
      saveMission( autoSaveName );

      // post another event
      postAutoSaveEvent();      
   }
}
