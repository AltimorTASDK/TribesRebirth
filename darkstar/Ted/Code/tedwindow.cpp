#include "simAction.h"
#include "simConsolePlugin.h"
#include "tedwindow.h"
#include "simted.h"
#include "tedplugin.h"
#include "tedresource.h"

#define VERSION_HIGH 1
#define VERSION_LOW  30

#define RESULTVAR "$dlgResult"
#define CANCEL_STR "[cancel]"
#define OK_STR "[ok]"
#define YES_STR "[yes]"
#define NO_STR "[no]"

TedWindow::TedWindow( SimTed * ted, GWWindow * parent )
{
   // crate the window
   if( !createEx( WS_EX_TOOLWINDOW, 0, "TED", WS_OVERLAPPED | WS_CLIPCHILDREN,
      Point2I( CW_USEDEFAULT, 0 ), Point2I( 502, 200 ), parent, 0, 0, true, false ) )
   {
      AssertFatal( 0, "TED:: Could not create main window" );
      exit( 1 );
   }
   
   // get the ptr to the SimTEd object
   simTed = ted;

   // create the main window and attach to this window      
   mainMenu.create( "IDM_TEDMENU" );
   mainMenu.setMainMenu( getHandle() );

   // create the main toolbar
   onShowToolBar();
   
   // setup the info
   setupMenu();
   setupToolBar();

   // set the size of the window
   // height = window height - client height + status height + toolbar height
   // width = toolbar width
   RECT rect;
   GetWindowRect( getHandle(), &rect );
   int height = ( rect.bottom - rect.top );
   GetClientRect( getHandle(), &rect );
   height -= ( rect.bottom - rect.top );
   GetWindowRect( hMainToolBar, &rect );
   height += ( rect.bottom - rect.top );
   int width = ( rect.right - rect.left );
   GetWindowRect( getStatusBarHandle(), &rect );
   height += ( rect.bottom - rect.top );
   SetWindowPos( getHandle(), NULL, 0, 0, width, height, SWP_NOMOVE );
   
   // show the window
   show( GWMain::cmdShow );
   update();
}

void TedWindow::onShowToolBar()
{
   TBBUTTON tbButtons[] = {
      { 0, TM_FILENEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 1, TM_FILEOPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 2, TM_FILESAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 3, TM_EDITUNDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 4, TM_EDITREDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },

      // space for combo
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      
      { 5, TM_EDITCLEARSELECTION, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 6, IDT_SELECT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 7, IDT_ADJUSTHEIGHT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 8, IDT_SETHEIGHT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 9, IDT_PIN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 10, IDT_ROTATE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 11, IDT_SMOOTH, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 12, TM_EDITFLOATSELECTION, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 13, TM_MISCRELIGHT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 14, IDT_SETTERRAINTYPE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 },
      { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 },
      { 15, TM_MISCLANDSCAPER, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0 }
   };

   int numButtons = sizeof( tbButtons ) / sizeof( TBBUTTON );
   
   // create the toolbar ( doesnt use gwtool version because of
   // messed up sized toolbar ) - besides, the toolbar class doesn't
   // do much anyhow
   hMainToolBar = CreateToolbarEx( getHandle(),
      WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS |
      CCS_ADJUSTABLE, IDC_TOOLBAR, 16, getHInstance(), IDB_TOOLBAR,
      tbButtons, numButtons, 16, 16, 16, 16, sizeof( TBBUTTON ) );
   if( !hMainToolBar )
   {
      AssertFatal( 0, "TED:: Could not create toolbar window" );
      exit( 1 );
   }
   
   // create the combobox
   hComboBox = CreateWindowEx( 0, "COMBOBOX", "", WS_CHILD |
      WS_BORDER | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST,
      126, 0, 72, 250, hMainToolBar, ( HMENU )IDC_COMBOBOX, NULL,
      NULL );
   if( !hComboBox )
   {
      AssertFatal( 0, "TED:: Could not create combobox window" );
      exit( 1 );
   }
   
   // Get the handle to the ToolTip window.
   HWND hWndTT = ( HWND )SendMessage( hMainToolBar, TB_GETTOOLTIPS, 0, 0 );
   if (hWndTT)
   {
      TOOLINFO info;
      
      // Fill out the TOOLINFO structure.
      info.cbSize = sizeof( info );
      info.uFlags = TTF_IDISHWND | TTF_CENTERTIP | TTF_SUBCLASS;
      info.lpszText = ( LPSTR )TM_COMBOBOX;
      info.hwnd = hMainToolBar;
      info.uId = ( int )hComboBox;
      info.hinst = getHInstance();

      // Set up the ToolTip for the combo box.
      SendMessage( hWndTT, TTM_ADDTOOL, 0, ( LPARAM )( TOOLINFO * )&info );
   }   
   
   // create the status bar
   int rightEdges[3] = {60, 250, -1,};
   int numEdges = sizeof(rightEdges)/sizeof(rightEdges[0]);
   
   createStatusBar( 0, numEdges, rightEdges );
}


TedWindow::~TedWindow()
{
}

void TedWindow::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{
   hwndCtl; codeNotify;
   
   // get the console

   // set the cursor to the waiting glass ( or whatnot )
   SetCursor( LoadCursor( NULL, IDC_WAIT ) );
   
   // look at the id, some will transfer to console commands ( those functions
   // that are implemented mainly in tedplugin.cpp or use the built-in 
   // dialog box's of the console ) while others will use the SimTed 
   // object to do the work - some of the functions are being removed 
   // from tedplugin and put into simted
	switch( id )
	{
      case IDC_COMBOBOX:
         if( codeNotify == CBN_SELENDOK )
         {  
            // get the index
            int detail = SendMessage( hComboBox, CB_GETCURSEL, 0, 0 );
            
            simTed->setBrushDetail( detail );
         }
         break;
         
      case TM_FILEOPEN:
         Console->evaluate( "Ted::open();", false );
         break;
         
      case TM_FILENEW:
         Console->evaluate( "Ted::new();", false );
         break;
         
      case TM_FILECLOSE:
         Console->evaluate( "Ted::close();", false );
         break;
         
      case TM_FILESAVE:
         Console->evaluate( "Ted::save( $TED::currFile );", false );
   
         // check the save off the mission
         if( Console->getBoolVariable( "$TED::success", false ) )
            if( Console->getBoolVariable( "$TED::terrainNameChange", false ) )
               Console->evaluate( "saveMission();", false );
         break;

      case TM_FILESAVEAS:
         Console->evaluate( "Ted::save();", false );
         if( Console->getBoolVariable( "$TED::success", false ) )
            if( Console->getBoolVariable( "$TED::terrainNameChange", false ) )
               Console->evaluate( "saveMission();", false );
         break;
      
      case TM_FILELOADPALETTE:
         Console->evaluate( "Ted::loadPalette();", false );
         break;
         
      case TM_FILELOADMATERIALLIST:
         Console->evaluate( "Ted::loadMatList();", false );
         break;
         
      case TM_FILEEXIT:
         Console->evaluate( "Ted::quitTed();", false );
         break;

      case TM_EDITUNDO:
         simTed->undo();
         break;
         
      case TM_EDITREDO:
         simTed->redo();
         break;
         
      case TM_EDITCOPY:

         simTed->processAction( "copy" );
         break;
         
      case TM_EDITPASTEALL:
      case TM_EDITPASTEMATERIALS:
      case TM_EDITPASTEHEIGHTS:
      {
         // save the current paste value
         //UInt8 oldVal = simTed->getPasteValue();
         UInt8 val = SimTed::Material | SimTed::Height;
         
         // check for material or height pasting
         if( id == TM_EDITPASTEMATERIALS )
            val = SimTed::Material;
         else if( id == TM_EDITPASTEHEIGHTS )
            val = SimTed::Height;
            
         // set the pasting mode, do the paste, reset the paste mode
         simTed->setFloatPasteVal( val );
         simTed->setSelFloating( true );
         
         //simTed->setPasteValue( val );
         //simTed->processAction( "paste" );
         //imTed->setPasteValue( oldVal );
         break;
      }
      
      case TM_EDITSAVESELECTION:
         Console->evaluate( "Ted::saveSelection();", false );
         break;
         
      case TM_EDITLOADSELECTION:
         Console->evaluate( "Ted::loadSelection();", false );
         break;
                        
      case TM_EDITCLEARSELECTION:
         simTed->clearSelect();
         break;
         
      case TM_EDITFLOATSELECTION:
         simTed->floatCurrent();
         break;
         
      case TM_EDITNAMESELECTION:
         Console->evaluate( "Ted::addNamedSelection();", false );
         break;
         
      case TM_EDITGETSELECTION:
         Console->evaluate( "Ted::selectNamedSelection();", false );
         break;
         
      case TM_EDITDELETESELECTION:
         Console->evaluate( "Ted::removeNamedSelection();", false );
         break;

      case TM_SETTINGSMISC:
         break;
         
      case TM_SETTINGSDISPLAY:
         displayOptions.createParam( "IDD_TEDDISPLAYOPTIONS", this, ( DWORD )simTed );
         break;

      case TM_BRUSHSNAP:
         simTed->setSnap( simTed->getSnap() ? false : true );
         break;
         
      case TM_BRUSHFEATHER:
         simTed->setFeather( simTed->getFeather() ? false : true );
         break;
         
      case TM_BRUSHVALUEADJUSTHEIGHT:
         Console->evaluate( "Ted::setAdjustVal();", false );
         break;
         
      case TM_BRUSHVALUESETHEIGHT:
         Console->evaluate( "Ted::setHeightVal();", false );
         break;
         
      case TM_BRUSHVALUEMATERIALINDEX:
         Console->evaluate( "Ted::selectMaterial();", false );
         break;
         
      case TM_BRUSHVALUETERRAINTYPE:
         Console->evaluate( "Ted::setTerrainType();", false );
         break;
         
      case TM_BRUSHVALUEFLAGMASK:
         Console->evaluate( "Ted::setFlagVal();", false );
         break;
         
      case TM_BRUSHVALUEPINDETAIL:
         Console->evaluate( "Ted::setPinDetailVal();", false );
         break;
         
      case TM_BRUSHVALUEPINDETAILMAX:
         Console->evaluate( "Ted::setPinDetailMax();", false );
         break;
         
      case TM_BRUSHVALUEPASTEMASK:
         Console->evaluate( "Ted::setPasteVal();", false );
         break;
         
      case TM_BRUSHVALUESCALEFACTOR:
         Console->evaluate( "Ted::setScaleVal();", false );
         break;

      case TM_BRUSHVALUESMOOTH:
         Console->evaluate( "Ted::setSmoothVal();", false );
         break;
         
      case TM_MISCLANDSCAPER:
         Console->evaluate( "LS::Editor( $TED::mainWindow );", false );
         Console->evaluate( "flushTextureCache();", false );
         break;
         
      case TM_MISCLISTCOMMANDS:
         Console->evaluate( "Ted::listCommands();", false );
         break;
      
      case TM_MISCRELIGHT:
         simTed->relight(false);
         break;

      case TM_MISCCLEARPINMAP:
         Console->evaluate( "Ted::clearPinMaps();", false );
         break;
         
      case TM_MISCABOUT:
      
         aboutDlg.create( "IDD_ABOUTTED", this );
         
         break;

      // ----------------------------------
      //        toolbar functions
      // ----------------------------------
      case IDT_SELECT:
         // set the mouse functions
         simTed->setLButtonAction( "select" );
         simTed->setRButtonAction( "deselect" );
         break;

      case IDT_ADJUSTHEIGHT:
         
         // set the mouse functions
         simTed->setLButtonAction( "raiseHeight" );
         simTed->setRButtonAction( "lowerHeight" );
         break;

      case IDT_SETHEIGHT:
      
         // set the mouse functions
         simTed->setLButtonAction( "setHeight" );
         simTed->setRButtonAction( "undo" );
         break;
         
      case IDT_PIN:
      
         // set the mouse functions
         simTed->setLButtonAction( "pin" );
         simTed->setRButtonAction( "clearPin" );
         break;
         
      case IDT_ROTATE:
      
         // set the mouse functions
         simTed->setLButtonAction( "rotateLeft" );
         simTed->setRButtonAction( "rotateRight" );
         break;
         
      case IDT_SMOOTH:
      
         // set the mouse functions
         simTed->setLButtonAction( "smooth" );
         simTed->setRButtonAction( "undo" );
         break;

      case IDT_SETTERRAINTYPE:
      
         const char *result;
         result = Console->evaluate( "Ted::setTerrainType();" );
         if( !result || !stricmp( result, "True") )
            simTed->processAction( "setTerrainType" );
      
         break;
                  
      default:
      
         if( id == -1 )
            break;
               
         // check for detail level
         if( id & TM_DETAILLEVELBASE && ( id < ( TM_DETAILLEVELBASE << 1 ) ) )
         {
            // set the detail
            simTed->setBrushDetail( id - TM_DETAILLEVELBASE );
         }
               
         // check the selection's
         if( id & TM_SELECTIONBASE && ( id < ( TM_SELECTIONBASE << 1 ) ) )
         {
            SimTed::TedAction action;
            
            // get the info
            if( simTed->getActionInfo( id - TM_SELECTIONBASE, action ) )
            {
               char * command = new char [ strlen( "Ted::processAction(" ) +
                  strlen( action.name.c_str() ) + 10];
                              
               // set the command
               sprintf( command, "Ted::processAction(\"%s\");", action.name.c_str() );
                    
               // call this routine
               Console->evaluate( command, false );
               
               delete [] command;
            }
         }
         
         if( id & TM_LBUTTONBASE && ( id < ( TM_LBUTTONBASE << 1 ) ) )
         {
            SimTed::TedAction action;
            
            // get the info
            if( simTed->getActionInfo( id - TM_LBUTTONBASE, action ) )
               simTed->setLButtonAction( action.name.c_str() );
         }
         
         if( id & TM_RBUTTONBASE && ( id < ( TM_RBUTTONBASE << 1 ) ) )
         {
            SimTed::TedAction action;
            
            // get the info
            if( simTed->getActionInfo( id - TM_RBUTTONBASE, action ) )
               simTed->setRButtonAction( action.name.c_str() );
         }
         
         break;                   
   }
   
   // set the cursor back
   SetCursor( LoadCursor( NULL, IDC_ARROW ) );

   // update the toolbar   
   updateToolBar();
   
   // update the window ( gray out buttons, set checks.... )
   update();
}

// update the gui
void TedWindow::onInitMenuPopup( HMENU hMenu, UINT item, BOOL fSystemMenu )
{
   item;
   if( fSystemMenu )
      return;

   // get the terrain pntr
   SimTerrain * terrain = simTed->getTerrain();
   
   int menuCount = GetMenuItemCount( hMenu );
   
   HMENU hMainMenu = mainMenu.getMenuHandle();
   
   // go through all the buttons
   for( int i = 0; i < menuCount; i++ )
   {
      bool enable = false;
      bool check = false;
   
      // get the id
      int id = GetMenuItemID( hMenu, i );
      
      // look at the item
      switch( id )
      {
         // always enabled buttons
         case TM_FILEOPEN:
         case TM_FILENEW:
         case TM_FILEEXIT:
         case TM_MISCABOUT:
            enable = true;
            break;
            
         case TM_FILECLOSE:
         case TM_FILESAVE:
         case TM_FILESAVEAS:
         case TM_EDITLOADSELECTION:
            enable = ( terrain ) ? true : false;
            break;

         case TM_EDITUNDO:
            if( simTed->getUndoCount() && terrain )
               enable = true;
            break;         
               
         case TM_EDITREDO:
            if( simTed->getRedoCount() && terrain )
               enable = true;
            break;         
         
         case TM_EDITPASTEALL:
         case TM_EDITPASTEMATERIALS:
         case TM_EDITPASTEHEIGHTS:
            if( simTed->pasteBufferValid() && terrain )
               enable = true;
            break;

         case TM_BRUSHSNAP:
            if( terrain )
               enable = true;
            if( simTed->getSnap() )
               check = true;
            break;
            
         case TM_BRUSHFEATHER:
            if( terrain )
               enable = true;
            if( simTed->getFeather() )
               check = true;
            break;

         case TM_EDITCLEARSELECTION:
         case TM_EDITFLOATSELECTION:
         case TM_EDITNAMESELECTION:
         case TM_EDITCOPY:
         case TM_EDITSAVESELECTION:
            if( terrain && simTed->currentSelValid() )
               enable = true;
            break;
            
         case TM_EDITGETSELECTION:
         case TM_EDITDELETESELECTION:
            if( terrain && simTed->getNumNamedSelections() )
               enable = true;
            break;
            
         case TM_BRUSHVALUESMOOTH:
         case TM_BRUSHVALUEADJUSTHEIGHT:
         case TM_BRUSHVALUESETHEIGHT:
         case TM_BRUSHVALUEMATERIALINDEX:
         case TM_BRUSHVALUEFLAGMASK:
         case TM_BRUSHVALUEPASTEMASK:
         case TM_BRUSHVALUESCALEFACTOR:
         case TM_BRUSHVALUETERRAINTYPE:
         case TM_BRUSHVALUEPINDETAIL:
         case TM_BRUSHVALUEPINDETAILMAX:
         case TM_MISCLANDSCAPER:
         case TM_MISCLISTCOMMANDS:
         case TM_MISCCLEARPINMAP:
         case TM_MISCRELIGHT:
         case TM_FILELOADPALETTE:
         case TM_FILELOADMATERIALLIST:
         case TM_SETTINGSMISC:
         case TM_SETTINGSDISPLAY:
            if( terrain )    
               enable = true;
            break;
            
         default:
         
            if( id == -1 )
               break;
               
            // check the details
            if( id & TM_DETAILLEVELBASE && ( id < ( TM_DETAILLEVELBASE << 1 ) ) )
            {
               if( terrain )
                  enable = true;
               if( simTed->getBrushDetail() == ( id - TM_DETAILLEVELBASE ) )
                  check = true;
            }
                  
            // check the selection's
            if( id & TM_SELECTIONBASE && ( id < ( TM_SELECTIONBASE << 1 ) ) )
            {
               if( terrain && simTed->currentSelValid() )
                  enable = true;
            }
            
            // check the buttons
            if( id & TM_LBUTTONBASE && ( id < ( TM_LBUTTONBASE << 1 ) ) ) 
            {
               if( simTed->getLButtonAction() == ( id - TM_LBUTTONBASE ) )
                  check = true;
               if( terrain )
                  enable = true;
            }
            
            if( id & TM_RBUTTONBASE && ( id < ( TM_RBUTTONBASE << 1 ) ) )
            {
               if( simTed->getRButtonAction() == ( id - TM_RBUTTONBASE ) )
                  check = true;
               if( terrain )
                  enable = true;
            }
            break;
      }
   
      // set the item state
      EnableMenuItem( hMainMenu, id, enable ? MF_ENABLED : MF_GRAYED );
      CheckMenuItem( hMainMenu, id, check ? MF_CHECKED : MF_UNCHECKED );
   }
}

// setup the toolbar with info
void TedWindow::setupToolBar()
{
   // reset the combo
   SendMessage( hComboBox, CB_RESETCONTENT, 0, 0 );
   
   // add them  
   for( int i = 0; i < simTed->getMaxBrushDetail(); i++ )
   {
      char name[ 32 ];
      sprintf( name, "Level %d", i );
      SendMessage( hComboBox, CB_INSERTSTRING, ( WPARAM )-1, 
         ( LPARAM )name );
   }
   
   // update the toolbar
   updateToolBar();
}

void TedWindow::updateToolBar()
{
   SimTerrain * terrain = simTed->getTerrain();
   int sel = simTed->currentSelValid();
   int undo = simTed->getUndoCount();
   int redo = simTed->getRedoCount();

   // look at buttons ( the missing ones are always enabled )
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, TM_FILESAVE, MAKELONG( terrain ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, TM_EDITUNDO, MAKELONG( ( terrain && undo ) ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, TM_EDITREDO, MAKELONG( ( terrain && redo ) ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, TM_EDITCLEARSELECTION, MAKELONG( ( terrain && sel ) ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, IDT_SELECT, MAKELONG( terrain ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, IDT_ADJUSTHEIGHT, MAKELONG( terrain ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, IDT_SETHEIGHT, MAKELONG( terrain ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, IDT_PIN, MAKELONG( terrain ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, IDT_ROTATE, MAKELONG( terrain ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, IDT_SMOOTH, MAKELONG( terrain ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, TM_EDITFLOATSELECTION, MAKELONG( ( terrain && sel ) ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, TM_MISCRELIGHT, MAKELONG( terrain ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, IDT_SETTERRAINTYPE, MAKELONG( ( terrain && sel ) ? true : false, 0 ) );
   SendMessage( hMainToolBar, TB_ENABLEBUTTON, TM_MISCLANDSCAPER, MAKELONG( terrain ? true : false, 0 ) );

   // adjust the specific controls
   if( terrain )
   {
      // do the detail combo
      SendMessage( hComboBox, CB_SETCURSEL, simTed->getBrushDetail(), 0 );

      int left = simTed->getLButtonAction();
      int right = simTed->getRButtonAction();      
      bool check;
      
      // do the modes
      if( ( left == simTed->getActionIndex( "select" ) ) &&
         ( right == simTed->getActionIndex( "deselect" ) ) )
         check = true;
      else
         check = false;
      SendMessage( hMainToolBar, TB_CHECKBUTTON, IDT_SELECT, MAKELONG( check, 0 ) );
         
      if( ( left == simTed->getActionIndex( "raiseHeight" ) ) &&
         ( right == simTed->getActionIndex( "lowerHeight" ) ) )
         check = true;
      else
         check = false;
      SendMessage( hMainToolBar, TB_CHECKBUTTON, IDT_ADJUSTHEIGHT, MAKELONG( check, 0 ) );

      if( ( left == simTed->getActionIndex( "setHeight" ) ) &&
         ( right == simTed->getActionIndex( "undo" ) ) )
         check = true;
      else
         check = false;
      SendMessage( hMainToolBar, TB_CHECKBUTTON, IDT_SETHEIGHT, MAKELONG( check, 0 ) );

      if( ( left == simTed->getActionIndex( "pin" ) ) && 
         ( right == simTed->getActionIndex( "clearPin" ) ) )
         check = true;
      else
         check = false;
      SendMessage( hMainToolBar, TB_CHECKBUTTON, IDT_PIN, MAKELONG( check, 0 ) );
                  
      if( ( left == simTed->getActionIndex( "rotateLeft" ) ) &&
         ( right == simTed->getActionIndex( "rotateRight" ) ) )
         check = true;
      else
         check = false;
      SendMessage( hMainToolBar, TB_CHECKBUTTON, IDT_ROTATE, MAKELONG( check, 0 ) );
      
      if( ( left == simTed->getActionIndex( "smooth" ) ) && 
         ( right == simTed->getActionIndex( "undo" ) ) )
         check = true;
      else
         check = false;
      SendMessage( hMainToolBar, TB_CHECKBUTTON, IDT_SMOOTH, MAKELONG( check, 0 ) );
   }
}


// setup the menu with all the popup's supplied with proper info
void TedWindow::setupMenu()
{
   HMENU hMainMenu = mainMenu.getMenuHandle();
   
   // fill in the detail info
   HMENU hBrush = GetSubMenu( hMainMenu, 5 );
   if( !hBrush )
      return;
      
   HMENU hDetails = GetSubMenu( hBrush, 0 );
   if( !hDetails )
      return;

   // clear them all first
   while( DeleteMenu( hDetails, 0, MF_BYPOSITION ) );
   
   // add them  
   for( int i = 0; i < simTed->getMaxBrushDetail(); i++ )
   {
      char name[ 32 ];
      sprintf( name, "Level %d", i );
      AppendMenu( hDetails, MF_STRING | MF_ENABLED,
         TM_DETAILLEVELBASE + i, name );
   }
   
   HMENU hSelection = GetSubMenu( hMainMenu, 3 );
   if( !hSelection )
      return;
      
   // get the mouse handles
   HMENU hLButton = GetSubMenu( hBrush, 5 );
   HMENU hRButton = GetSubMenu( hBrush, 6 );
   
   if( !hLButton || !hRButton )
      return;
      
   // clear them all first
   while( DeleteMenu( hSelection, 0, MF_BYPOSITION ) );
   while( DeleteMenu( hLButton, 0, MF_BYPOSITION ) );
   while( DeleteMenu( hRButton, 0, MF_BYPOSITION ) );
   
   for( int j = 0; j < simTed->getNumActions(); j++ )
   {
      SimTed::TedAction action;
      
      // get the info
      if( simTed->getActionInfo( j, action ) )
      {
         if( action.mouseAction )
         {
            AppendMenu( hSelection, MF_STRING | MF_ENABLED,
               TM_SELECTIONBASE + j, action.name.c_str() );
            AppendMenu( hLButton, MF_STRING | MF_ENABLED,
               TM_LBUTTONBASE + j, action.name.c_str() );
            AppendMenu( hRButton, MF_STRING | MF_ENABLED,
               TM_RBUTTONBASE + j, action.name.c_str() );
         }
      }
   }
   
}

void TedWindow::onSize( UINT state, int cx, int cy )
{
   state;
   
   // get the status bar handle and resize it also
   HWND hStatus = getStatusBarHandle();
   
   MoveWindow( hStatus, 0, cx - 10, cy, cx, true );

   // do the toolbar
   MoveWindow( hMainToolBar, 0, 0, cy, cx, true );
}

void TedWindow::onUpdate()
{
   updateToolBar();
}

// material view stuff
bool TedWindow::createMaterialView()
{
   return( true );
}

void TedWindow::showMaterialView( bool show )
{
   show;
}

bool TedWindow::setMaterialView()
{
   return( true );
}

// --------------------------------------
//           dialog handlers
// --------------------------------------

LRESULT AboutDlg::windowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
   switch( message )
   {
      case WM_INITDIALOG:

         HWND hStatic = GetDlgItem( getHandle(), IDC_ABOUTTEXT );
      
         char text[ 120 ];
         sprintf( text, "Version: %d.%d\nDate: %s\nTime: %s\n",
            VERSION_HIGH, VERSION_LOW, __DATE__, __TIME__ );
         SetWindowText( hStatic, text );
         
         break;
   }
   return( GWDialog::windowProc( message, wParam, lParam ) );
}

void AboutDlg::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   hwndCtl; codeNotify;
   
   switch( id )
   {
      case IDOK:
      
         EndDialog( getHandle(), 0 );
         
         break;
   }
}


DisplayOptionsDlg::DisplayOptionsDlg() :
   simTed( NULL ),
   shadowShow( 0 ),
   shadowFrameColor( 0 ),
   shadowFillColor( 0 ),
   blockOutline( 0 ),
   blockFrameColor( 0 ),
   hilightShow( 0 ),
   hilightFrameColor( 0 ),
   hilightFillColor( 0 ),
   selectShow( 0 ),
   selectFrameColor( 0 ),
   selectFillColor( 0 )
{
}

DisplayOptionsDlg::~DisplayOptionsDlg()
{
}

void DisplayOptionsDlg::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   hwndCtl; codeNotify;
   
   switch( id )
   {
      case IDOK:
      
         // set the info and quit out
         setInfo();
         EndDialog( getHandle(), 0 );
         
         break;
   }
}

LRESULT DisplayOptionsDlg::windowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
   switch( message )
   {
      case WM_INITDIALOG:
      
         // get the simted object
         simTed = ( SimTed * )lParam;
         
         // get all the info
         shadowShow = simTed->getShadowShow();
         shadowFrameColor = simTed->getShadowFrameColor();
         shadowFillColor = simTed->getShadowFillColor();
         
         selectShow = simTed->getSelectShow();
         selectFrameColor = simTed->getSelectFrameColor();
         selectFillColor = simTed->getSelectFillColor();
         
         hilightShow = simTed->getHilightShow();
         hilightFrameColor = simTed->getHilightFrameColor();
         hilightFillColor = simTed->getHilightFillColor();
         
         blockOutline = simTed->getBlockOutline();
         blockFrameColor = simTed->getBlockFrameColor();
         
         // fill the dialog with the info
         fillInfo();
         
         break;
   }
   
   return( GWDialog::windowProc( message, wParam, lParam ) );
}

void DisplayOptionsDlg::fillInfo()
{
   // fill the check box's
   SendDlgItemMessage( getHandle(), IDC_SELFRAMECHECKBOX, BM_SETCHECK,
      ( selectShow.test( SimTed::Frame ) ) ? BST_CHECKED : BST_UNCHECKED, 0 );
   SendDlgItemMessage( getHandle(), IDC_SELFILLCHECKBOX, BM_SETCHECK,
      ( selectShow.test( SimTed::Fill ) ) ? BST_CHECKED : BST_UNCHECKED, 0 );
   SendDlgItemMessage( getHandle(), IDC_SELOUTLINECHECKBOX, BM_SETCHECK,
      ( selectShow.test( SimTed::Outline ) ) ? BST_CHECKED : BST_UNCHECKED, 0 );

   SendDlgItemMessage( getHandle(), IDC_HILIGHTFRAMECHECKBOX, BM_SETCHECK,
      ( hilightShow.test( SimTed::Frame ) ) ? BST_CHECKED : BST_UNCHECKED, 0 );
   SendDlgItemMessage( getHandle(), IDC_HILIGHTFILLCHECKBOX, BM_SETCHECK,
      ( hilightShow.test( SimTed::Fill ) ) ? BST_CHECKED : BST_UNCHECKED, 0 );
   SendDlgItemMessage( getHandle(), IDC_HILIGHTOUTLINECHECKBOX, BM_SETCHECK,
      ( hilightShow.test( SimTed::Outline ) ) ? BST_CHECKED : BST_UNCHECKED, 0 );

   SendDlgItemMessage( getHandle(), IDC_SHADOWFRAMECHECKBOX, BM_SETCHECK,
      ( shadowShow.test( SimTed::Frame ) ) ? BST_CHECKED : BST_UNCHECKED, 0 );
   SendDlgItemMessage( getHandle(), IDC_SHADOWFILLCHECKBOX, BM_SETCHECK,
      ( shadowShow.test( SimTed::Fill ) ) ? BST_CHECKED : BST_UNCHECKED, 0 );
   SendDlgItemMessage( getHandle(), IDC_SHADOWOUTLINECHECKBOX, BM_SETCHECK,
      ( shadowShow.test( SimTed::Outline ) ) ? BST_CHECKED : BST_UNCHECKED, 0 );

   SendDlgItemMessage( getHandle(), IDC_BLOCKOUTLINECHECKBOX, BM_SETCHECK,
      ( blockOutline ) ? BST_CHECKED : BST_UNCHECKED, 0 );
}

void DisplayOptionsDlg::setInfo()
{
   // go through and collect all the info
   selectShow.set( SimTed::Frame, ( SendDlgItemMessage( getHandle(), IDC_SELFRAMECHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) );
   selectShow.set( SimTed::Fill, ( SendDlgItemMessage( getHandle(), IDC_SELFILLCHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) );
   selectShow.set( SimTed::Outline, ( SendDlgItemMessage( getHandle(), IDC_SELOUTLINECHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) );

   hilightShow.set( SimTed::Frame, ( SendDlgItemMessage( getHandle(), IDC_HILIGHTFRAMECHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) );
   hilightShow.set( SimTed::Fill, ( SendDlgItemMessage( getHandle(), IDC_HILIGHTFILLCHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) );
   hilightShow.set( SimTed::Outline, ( SendDlgItemMessage( getHandle(), IDC_HILIGHTOUTLINECHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) );

   shadowShow.set( SimTed::Frame, ( SendDlgItemMessage( getHandle(), IDC_SHADOWFRAMECHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) );
   shadowShow.set( SimTed::Fill, ( SendDlgItemMessage( getHandle(), IDC_SHADOWFILLCHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) );
   shadowShow.set( SimTed::Outline, ( SendDlgItemMessage( getHandle(), IDC_SHADOWOUTLINECHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) );

   blockOutline = ( SendDlgItemMessage( getHandle(), IDC_BLOCKOUTLINECHECKBOX, BM_GETCHECK, 0, 0 ) == BST_CHECKED );
      
   // set them
   simTed->setShadowShow( shadowShow );
   simTed->setHilightShow( hilightShow );
   simTed->setSelectShow( selectShow );
   
   simTed->setBlockOutline( blockOutline );
}