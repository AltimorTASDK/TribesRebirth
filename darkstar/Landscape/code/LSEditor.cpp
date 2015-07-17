//############################################################################
//##   
//##  $Workfile:   LS_Dlg.cpp  $
//##  $Version$
//##  $Revision:   1.00  $
//##    
//##  DESCRIPTION:
//##        A dialog class to bring up a LandScape editor in your application
//##        
//##  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//## 
//############################################################################

#include "LSEditor.h"
#include "LS_Defs.h"
#include "simInput.h"
#include "console.h"

//############################################################################

LSEditor::LSEditor()
{
   flags.clear();
   pPal           = NULL;
   pCanvas        = NULL;
   InitCommonControls();
}

//----------------------------------------------------------------------------

bool LSEditor::create( GWWindow *parent, LSMapper *lsMapper, LandScape *landscape, bool standAlone )
{
   bool status=false;
   flags.set(STANDALONE,standAlone);
   hDLL = LoadLibrary("RICHED32.DLL");
   if ( Parent::create( "LandScapeBox", parent ) )
   {
      flags.set(SIMINPUTFOCUS);
      pLSMapper = lsMapper;
      pLS = landscape;
      pCanvas  = new GWCanvas;
      RECT r;
      GetWindowRect( GetDlgItem(hWnd,IDC_CANVAS), &r );
      POINT left;
      left.x = r.left;
      left.y = r.top;
      ScreenToClient( hWnd, &left );

      status = pCanvas->create((LPCSTR)"LandScape Map View",
         (DWORD)WS_CHILD|WS_VISIBLE,
         Point2I(left.x,left.y), Point2I(r.right-r.left,r.bottom-r.top), (GWWindow*)this,0,false,false);

      pPal = new GFXPalette();
      for ( int i=0; i<256; i++ )
         pPal->palette[0].color[i].peRed = pPal->palette[0].color[i].peGreen = pPal->palette[0].color[i].peBlue = i;
      pCanvas->getSurface()->setPalette( pPal );

      pScriptFI = new FileInfo;
      pBMFI     = new FileInfo;
      pRulesFI  = new FileInfo;

      pScriptFI->init( hWnd, ".PLS", FileInfo::TYPE_PLS, NULL ); 
      pBMFI->init( hWnd, ".BMP", FileInfo::TYPE_DIB, NULL );
      pRulesFI->init( hWnd, ".PLR", FileInfo::TYPE_PLR, NULL );

      // grab the rules from the mapper
      Vector< RuleInfo > * ruleList = pLSMapper->getRuleList();
      rules = *ruleList;
      
      // create the tabs on the tab control
      TC_ITEM  item;
      item.mask = TCIF_TEXT;
      item.pszText = " Script ";
      item.cchTextMax = strlen( " Script " ) + 1;
      SendMessage( GetDlgItem( hWnd, IDC_TABCONTROL ), TCM_INSERTITEM, 0, ( DWORD )&item );
      item.pszText = " Rules ";
      item.cchTextMax = strlen( " Rules " ) + 1;
      SendMessage( GetDlgItem( hWnd, IDC_TABCONTROL ), TCM_INSERTITEM, 1, ( DWORD )&item );
      
      // place them on top of z-order
      SetWindowPos( GetDlgItem( hWnd, IDC_SCRIPTEDIT ), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
      SetWindowPos( GetDlgItem( hWnd, IDC_RULELIST ), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
      SetWindowPos( GetDlgItem( hWnd, IDC_REMOVERULE ), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
      SetWindowPos( GetDlgItem( hWnd, IDC_EDITRULE ), HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
      
      editRuleDlg.setEditor( this );
      
      // force page 0 to be displayed
      showTabPage( SCRIPT );
      SendMessage( GetDlgItem( hWnd, IDC_TABCONTROL ), TCM_SETCURSEL, 0, 0 );
      
      hMenu = GetMenu( hWnd );
      EnableWindow( GetDlgItem( hWnd, IDC_LANDSCAPE ), false );
      if ( !flags.test(STANDALONE) ) 
         EnableWindow( GetDlgItem( hWnd, IDC_TEXTURE ), (pLSMapper->flags.test(LSMapper::RULES_LOADED)&&pLSMapper->flags.test(LSMapper::TEXTS_LOADED)) );
      else
         EnableWindow( GetDlgItem( hWnd, IDC_TEXTURE ), false );

      update();
      SetFocus( parent? parent->getHandle():getHandle() );
   }
   AssertFatal(status,"LSEditor:: Could not create main window (missing LSDlg.RC?)");
   return( status );
}

//--------------------------------------------------------------------------------

void LSEditor::render()
{
   // show bitmap
   GFXBitmap *pBM=NULL;
   pCanvas->lock();
   if ( pLS->getSize() )
      pBM = (GFXBitmap*)pLS->exec(LS_GET, 1);

   if ( pBM )
      pCanvas->getSurface()->drawBitmap2d(pBM, &Point2I(0,0), &pCanvas->getClientSize() );
   else
      pCanvas->getSurface()->clear(7);

   pCanvas->unlock();
   pCanvas->flip();
   delete pBM;  // OK even if NULL
}

//--------------------------------------------------------------------------------
// to show which page in the tab view - if many more pages are added then change how all
// this works ( fine for 2 or 3 maybe )
void LSEditor::showTabPage( int page )
{
   // check for just a refresh
   if( page == -1 )
      page = SendMessage( GetDlgItem( hWnd, IDC_TABCONTROL ), TCM_GETCURSEL, 0, 0 );
   
   if( page == -1 )
      return;
   
   // script windows
   ShowWindow( GetDlgItem( hWnd, IDC_SCRIPTEDIT ), page == SCRIPT ? SW_SHOW : SW_HIDE );
   
   // rules tab windows
   ShowWindow( GetDlgItem( hWnd, IDC_RULELIST ), page == RULES ? SW_SHOW : SW_HIDE );
   ShowWindow( GetDlgItem( hWnd, IDC_ADDRULE ), page == RULES ? SW_SHOW : SW_HIDE );
   ShowWindow( GetDlgItem( hWnd, IDC_REMOVERULE ), page == RULES ? SW_SHOW : SW_HIDE );
   ShowWindow( GetDlgItem( hWnd, IDC_EDITRULE ), page == RULES ? SW_SHOW : SW_HIDE );
   
   // stuff for each window here
   if( page == RULES )
   {
      // clear out the items in the box - and add them back in
      SendMessage( GetDlgItem( hWnd, IDC_RULELIST ), LB_RESETCONTENT, 0, 0 );
      for( int i = 0; i < rules.size(); i++ )
      {
         // make sure a valid group type
         if( rules[i].groupNum < pLSMapper->getNumBaseTypes() )
         {
            SendMessage( GetDlgItem( hWnd, IDC_RULELIST ), LB_ADDSTRING, 0, 
               ( LPARAM )avar( "Rule: %d - %s", i, getTypeDesc( rules[i].groupNum ) ) );
         }
         else
         {
            SendMessage( GetDlgItem( hWnd, IDC_RULELIST ), LB_ADDSTRING, 0, 
               ( LPARAM )avar( "Rule: %d - [undefined type: %d]", i, rules[i].groupNum ) );
         }
      }
   }
}

//--------------------------------------------------------------------------------

char * LSEditor::getTypeDesc( int index )
{
   AssertFatal( pLSMapper, "Landscape mapper invalid." );
   AssertFatal( index <= pLSMapper->getNumBaseTypes(), "Invalid terrain type index" );

   char * str = NULL;
   
   // get to the type
   for( int i = 0; i <= index; i++ )
      str = pLSMapper->getTypeDesc( !i ? true : false );
      
   return( str );
}

//--------------------------------------------------------------------------------
// process notify messages

int LSEditor::onNotify( int id, LPNMHDR pnmhdr )
{
   switch( id )
   {
      case IDC_TABCONTROL:
      
         // look at the notification code
         if( pnmhdr->code == TCN_SELCHANGE )
         {
            int id = SendMessage( GetDlgItem( hWnd, IDC_TABCONTROL ), TCM_GETCURSEL, 0, 0 );
            showTabPage( id );            
         }
         break;
         
      default:
         break;
   }
   return( 0 );
}

//--------------------------------------------------------------------------------
// Process Menu Events

void LSEditor::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   hwndCtl;
   codeNotify;

   char     buff[4096];

   SetCursor( LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)) );
   switch( id )
   {
      case IDC_EDITRULE:
         {
            int cur = SendMessage( GetDlgItem( hWnd, IDC_RULELIST ), LB_GETCURSEL, 0, 0 );
            if( cur == LB_ERR )
               break;
            editRuleDlg.createParam( "EDITRULEBOX", this, ( LPARAM )&rules[cur], true );
            showTabPage( RULES );
         }
         break;
         
      case IDC_ADDRULE:
         editRuleDlg.createParam( "EDITRULEBOX", this, NULL, true );

         // refresh the display
         showTabPage( RULES );
         
         break;
         
      case IDC_RULELIST:
         if( codeNotify == LBN_DBLCLK )
         {
            int cur = SendMessage( GetDlgItem( hWnd, IDC_RULELIST ), LB_GETCURSEL, 0, 0 );
            if( cur == LB_ERR )
               break;
            editRuleDlg.createParam( "EDITRULEBOX", this, ( LPARAM )&rules[cur], true );
            showTabPage( RULES );
         }
         break;
         
      case IDC_REMOVERULE:
         {
            int cur = SendMessage( GetDlgItem( hWnd, IDC_RULELIST ), LB_GETCURSEL, 0, 0 );
            if( cur == LB_ERR )
               break;
            
            flags.set( RULESCHANGED, true );
            
            // remove the item
            rules.erase( cur );
               
            // refresh the list
            showTabPage( RULES );
         }
         break;
         
      case IDC_LOADRULES:
         loadRules();
         showTabPage();
         break;
         
      case IDC_SAVERULES:
         saveRules();
         break;
         
      case IDC_LOAD:
         loadScript();
         break;

      case IDC_SAVE:
         saveScript();
         break;

      case IDC_PREVIEW:
      
         pLS->exec( LS_SEED, GetTickCount() );
         SendMessage( GetDlgItem( hWnd, IDC_SCRIPTEDIT ), WM_GETTEXT, (WPARAM)sizeof(buff), (LPARAM)buff );
         pLS->parseScript( buff );
         SetDlgItemText(hWnd, IDC_SEED, avar("%d", pLS->getSeed()));
         flags.set(PREVIEW);
         if ( !flags.test(STANDALONE) ) EnableWindow( GetDlgItem( hWnd, IDC_LANDSCAPE ), true );
         if ( !flags.test(STANDALONE) ) EnableWindow( GetDlgItem( hWnd, IDC_TEXTURE ), true );
         break;

      case IDC_LANDSCAPE:
         pLSMapper->applyLandScape();
         break;

      case IDC_TEXTURE:

         // set the rules
         pLSMapper->setRules( rules );
         pLSMapper->applyTextures();
         break;

      case IDM_EXIT:
         destroyWindow();
         deleteObject();
         return;

      case IDM_ABOUT:
         DialogBox( GWMain::hInstance, "AboutBox", hWnd, GenericDlg );
         break;

      case IDM_LOAD:
         loadBitmap();
         break;

      case IDM_SAVE:
         saveBitmap();
         break;

      default:
//         DialogBox( GWMain::hInstance, "NoFunctionBox", hWnd, GenericDlg );
         break;
   }
   SetCursor( LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)) );
   if ( !flags.test(STANDALONE) ) EnableWindow( GetDlgItem( hWnd, IDC_TEXTURE ), (pLSMapper->flags.test(LSMapper::RULES_LOADED)&&pLSMapper->flags.test(LSMapper::TEXTS_LOADED)) );
   render();
}   

//--------------------------------------------------------------------------------

void LSEditor::onSysCommand(UINT cmd, int x, int y)
{
   switch( cmd )
   {
      case SC_CLOSE:
      {
         destroyWindow();
         deleteObject();
      }
   }
   
   Parent::onSysCommand( cmd, x, y );
}
         

//--------------------------------------------------------------------------------
// loads the rules and the group info ( LSMapper version ignores group info )
void LSEditor::loadRules()
{
   if( pRulesFI->getName( FileInfo::OPEN_MODE, "Load LandScape Rules" ) )
   {
      // load them 
      if( !pLSMapper->loadRules( pRulesFI->fileName(), rules ) )
      {
         MessageBox( hWnd, avar( "Failed to load rule file: %s", pRulesFI->fileName() ), "Error", MB_OK );
         return;                                              
      }
      
      flags.set( RULESCHANGED, false );
   }
}

//--------------------------------------------------------------------------------

void LSEditor::saveRules()
{
   if( pRulesFI->getName( FileInfo::SAVE_MODE, "Save LandScape Rules" ) )
   {
      // save them
      if( !pLSMapper->saveRules( pRulesFI->fileName(), rules ) )
      {
         MessageBox( hWnd, avar( "Failed to save rule file: %s", pRulesFI->fileName()), "Error", MB_OK );
         return;
      }
      
      flags.set( RULESCHANGED, false );
   }
}

//--------------------------------------------------------------------------------

void  LSEditor::saveScript()
{
   if ( pScriptFI->getName( FileInfo::SAVE_MODE, "Save LandScape Script" ) )
   {
      char  buff[4096];
      FileWStream fws( pScriptFI->fileName() );
      *(int*)buff = SendMessage( GetDlgItem( hWnd, IDC_SCRIPTEDIT ), WM_GETTEXT, (WPARAM)sizeof(buff), (LPARAM)buff+4 ) + 1;   // +1 for the 0 at end
      if ( !*(int*)buff )  return;
      if ( !fws.write( *(int*)buff+4, (void*)buff ) )
         MessageBox( hWnd, avar("Error occured attempting to write:%s",pScriptFI->fileName()), "Syntax Error", MB_OK );
   }
}

//--------------------------------------------------------------------------------

void  LSEditor::loadScript()
{
   if ( pScriptFI->getName( FileInfo::OPEN_MODE, "Load LandScape Script" ) )
   {
      char  buff[4096];
      Int32 numBytes;
      FileRStream frs( pScriptFI->fileName() );
      if ( frs.read( &numBytes ) )
         if ( numBytes && frs.read( numBytes, (void*)buff ) )
         {
            SendMessage( GetDlgItem( hWnd, IDC_SCRIPTEDIT ), WM_SETTEXT, NULL, (LPARAM)buff );
            if ( !flags.test(STANDALONE) ) EnableWindow( GetDlgItem( hWnd, IDC_LANDSCAPE ), true );
            if ( !flags.test(STANDALONE) ) EnableWindow( GetDlgItem( hWnd, IDC_TEXTURE ), true );
         }
         else
            MessageBox( hWnd, avar("Error occured attempting to read:%s",pScriptFI->fileName()), "Syntax Error", MB_OK );
   }
}

//--------------------------------------------------------------------------------

void  LSEditor::saveBitmap()
{
   if ( pBMFI->getName( FileInfo::SAVE_MODE, "Save Terrain Bitmap" ) )
      pLS->exec( LS_SAVE, pBMFI->fileName() );
}

//--------------------------------------------------------------------------------

void  LSEditor::loadBitmap()
{
   if ( pBMFI->getName( FileInfo::OPEN_MODE, "Load Terrain Bitmap" ) )
      pLS->exec( LS_LOAD, pBMFI->fileName() );
}

//------------------------------------------------------------------------------

bool LSEditor::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
      onEvent(SimEditEvent);
      onEvent(SimGainFocusEvent);
      onEvent(SimLoseFocusEvent);
	}
   return false;
}   

//------------------------------------------------------------------------------

bool LSEditor::onAdd()
{
   if (SimParent::onAdd() == false)
      return false;

   addToSet( SimInputConsumerSetId );
   return true;
}   

//------------------------------------------------------------------------------

void LSEditor::onRemove()
{
   removeFromSet( SimInputConsumerSetId );
   delete pCanvas;
   delete pPal;
   delete pScriptFI;
   delete pBMFI;
   delete pRulesFI;
   
   SimParent::onRemove();
}   

//------------------------------------------------------------------------------

bool LSEditor::onSimEditEvent(const SimEditEvent *event)
{
   if (event->editorId == id)
      flags.set( SIMINPUTFOCUS, event->state );
   return ( true );
}   

//------------------------------------------------------------------------------

bool LSEditor::onSimGainFocusEvent(const SimGainFocusEvent *)
{
   flags.set(SIMINPUTFOCUS,true);
   return ( true );
}   

//------------------------------------------------------------------------------

bool LSEditor::onSimLoseFocusEvent(const SimLoseFocusEvent *)
{
   flags.set(SIMINPUTFOCUS,false);
   return ( true );
}   

//------------------------------------------------------------------------------

bool LSEditor::processQuery( SimQuery* query )
{
   switch ( query->type )
   {
      onQuery( SimInputPriorityQuery );
   }
   return ( false );
}

//------------------------------------------------------------------------------

bool LSEditor::onSimInputPriorityQuery( SimInputPriorityQuery *query )
{
   query->priority = SI_PRIORITY_NORMAL+1;
   return true;
}   

//------------------------------------------------------------------------------

void LSEditor::onSetFocus(HWND hwndOldFocus)
{
   hwndOldFocus;
   if ( !flags.test(STANDALONE) )
      Console->evaluate("inputDeactivate(mouse,0);", false );
   flags.set(SIMINPUTFOCUS,true);
}   

//------------------------------------------------------------------------------
// DIALOG HANDLERS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// edit rules dialog handler routines
//------------------------------------------------------------------------------

BOOL EditRuleDlg::onInitDialog( HWND hwndFocus, LPARAM lParam )
{
   GWDialog::onInitDialog( hwndFocus, lParam );
   
   // create a new struct
   pRuleInfo = ( RuleInfo * )lParam;
      
   // check if should add to the list or not...
   if( pRuleInfo == NULL )
   {
      // init the thing
      ruleInfo.groupNum = 0;
      ruleInfo.Altitude.min = 0.0f;
      ruleInfo.Altitude.max = 0.0f;
      ruleInfo.Altitude.mean = 0.0f;
      ruleInfo.Altitude.sdev = 0.0f;
      ruleInfo.AltWeight = 0.0f;
      ruleInfo.adjHeights = 0;
      ruleInfo.Slope.min = 0.0f;
      ruleInfo.Slope.max = 0.0f;
      ruleInfo.Slope.mean = 0.0f;
      ruleInfo.Slope.sdev = 0.0f;
      ruleInfo.SlopeWeight = 0.0f;
      ruleInfo.adjSlopes = 0;
   }
   else
      // just copy it
      ruleInfo = *pRuleInfo;
   
   // update the types combo
   updateTypes();
   
   // set this info
   setData( ruleInfo );
   
   return( true );
}

//------------------------------------------------------------------------------

void EditRuleDlg::updateTypes()
{
   SendMessage( GetDlgItem( hWnd, IDC_TYPECOMBO ), CB_RESETCONTENT, 0, 0 );
   
   // go through and add all of the types to the combobox
   for( int i = 0; i < editor->pLSMapper->getNumBaseTypes(); i++ )
      SendMessage( GetDlgItem( hWnd, IDC_TYPECOMBO ), CB_ADDSTRING, 0, 
         ( LPARAM )editor->getTypeDesc( i ) );
}

//------------------------------------------------------------------------------
// set all the fields data values in the dialog box

void EditRuleDlg::setData( RuleInfo & rule )
{
   // set the terrain type field
   SendMessage( GetDlgItem( hWnd, IDC_TYPENUM ), WM_SETTEXT, 0, ( LPARAM )avar( "%d", rule.groupNum ) );
   
   // set all the edit box info's
   SendMessage( GetDlgItem( hWnd, IDC_ALTMIN ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.Altitude.min ) );
   SendMessage( GetDlgItem( hWnd, IDC_ALTMAX ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.Altitude.max ) );
   SendMessage( GetDlgItem( hWnd, IDC_ALTMEAN ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.Altitude.mean ) );
   SendMessage( GetDlgItem( hWnd, IDC_ALTSDEV ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.Altitude.sdev ) );
   SendMessage( GetDlgItem( hWnd, IDC_ALTWEIGHT ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.AltWeight ) );
   SendMessage( GetDlgItem( hWnd, IDC_ALTADJ ), BM_SETCHECK, rule.adjHeights ? BST_CHECKED : BST_UNCHECKED, 0 );
   
   SendMessage( GetDlgItem( hWnd, IDC_SLOPEMIN ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.Slope.min ) );
   SendMessage( GetDlgItem( hWnd, IDC_SLOPEMAX ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.Slope.max ) );
   SendMessage( GetDlgItem( hWnd, IDC_SLOPEMEAN ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.Slope.mean ) );
   SendMessage( GetDlgItem( hWnd, IDC_SLOPESDEV ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.Slope.sdev ) );
   SendMessage( GetDlgItem( hWnd, IDC_SLOPEWEIGHT ), WM_SETTEXT, 0, ( LPARAM )avar( "%f", rule.SlopeWeight ) );
   SendMessage( GetDlgItem( hWnd, IDC_SLOPEADJ ), BM_SETCHECK, rule.adjSlopes ? BST_CHECKED : BST_UNCHECKED, 0 );
}

//------------------------------------------------------------------------------
// returns a different float only if the ascii representation is different

float EditRuleDlg::atofDif( float val, const char * buff )
{
   char floatBuf[64];
   sprintf( floatBuf, "%f", val );
   if( !stricmp( floatBuf, buff ) )
      return( val );
      
   return( atof( buff ) );
}

//------------------------------------------------------------------------------

void EditRuleDlg::getData( RuleInfo & rule )
{
   // grab all the info
   char buffer[200];
   
   // get the group number
   SendMessage( GetDlgItem( hWnd, IDC_TYPENUM ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.groupNum = atoi( buffer );
   
   // altitude info
   SendMessage( GetDlgItem( hWnd, IDC_ALTMIN ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.Altitude.min = atofDif( ruleInfo.Altitude.min, buffer );
   SendMessage( GetDlgItem( hWnd, IDC_ALTMAX ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.Altitude.max = atofDif( ruleInfo.Altitude.max, buffer );
   SendMessage( GetDlgItem( hWnd, IDC_ALTMEAN ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.Altitude.mean = atofDif( ruleInfo.Altitude.mean, buffer );
   SendMessage( GetDlgItem( hWnd, IDC_ALTSDEV ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.Altitude.sdev = atofDif( ruleInfo.Altitude.sdev, buffer );
   SendMessage( GetDlgItem( hWnd, IDC_ALTWEIGHT ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.AltWeight = atofDif( ruleInfo.AltWeight, buffer );
   rule.adjHeights = ( SendMessage( GetDlgItem( hWnd, IDC_ALTADJ ), BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ? true : false;
   
   // slope info
   SendMessage( GetDlgItem( hWnd, IDC_SLOPEMIN ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.Slope.min = atofDif( ruleInfo.Slope.min, buffer );
   SendMessage( GetDlgItem( hWnd, IDC_SLOPEMAX ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.Slope.max = atofDif( ruleInfo.Slope.max, buffer );
   SendMessage( GetDlgItem( hWnd, IDC_SLOPEMEAN ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.Slope.mean = atofDif( ruleInfo.Slope.mean, buffer );
   SendMessage( GetDlgItem( hWnd, IDC_SLOPESDEV ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.Slope.sdev = atofDif( ruleInfo.Slope.sdev, buffer );
   SendMessage( GetDlgItem( hWnd, IDC_SLOPEWEIGHT ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
   rule.SlopeWeight = atofDif( ruleInfo.SlopeWeight, buffer );
   rule.adjSlopes = ( SendMessage( GetDlgItem( hWnd, IDC_SLOPEADJ ), BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ? true : false;
}

//------------------------------------------------------------------------------
// go through the fields and check that their values are ok

bool EditRuleDlg::checkData( const RuleInfo & rule )
{
   // check the group numbers
   if( rule.groupNum < 0 )
   {
      MessageBox( hWnd, avar( "Invalid terrain type [ %d ] selected", rule.groupNum ), "Error", MB_OK );
      return( false );
   }
   
   // check the adjust check's
   if( rule.adjSlopes > 1 || rule.adjHeights > 1 )
   {
      MessageBox( hWnd, avar( "%s field is invalid", ( rule.adjSlopes > 1 ) ? "adjSlopes:" : "adjAlt" ), "Error", MB_OK );
      return( false );
   }
   
   return( true );
}

//------------------------------------------------------------------------------
 

void EditRuleDlg::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   hwndCtl;

   switch( codeNotify )
   {
      case CBN_SELENDOK:
      {
         // set the text of the position here
         int pos = SendMessage( GetDlgItem( hWnd, IDC_TYPECOMBO ), CB_GETCURSEL, 0, 0 );

         if( pos != CB_ERR )
            SendMessage( GetDlgItem( hWnd, IDC_TYPENUM ), WM_SETTEXT, 0, ( LPARAM )avar( "%d", pos ) );
            
         break;
      }
         
      case EN_CHANGE:
      {
         // grab the text from the edit box ( only numbers... )
         char buffer[200];
         SendMessage( GetDlgItem( hWnd, IDC_TYPENUM ), WM_GETTEXT, ( WPARAM )sizeof( buffer ), ( LPARAM )buffer );
         
         // get the number from the field
         int pos = atoi( buffer );

         // check if the value is within our ranges            
         int count = SendMessage( GetDlgItem( hWnd, IDC_TYPECOMBO ), CB_GETCOUNT, 0, 0 );
         
         if( ( pos >= 0 ) && ( pos < count ) )
            SendMessage( GetDlgItem( hWnd, IDC_TYPECOMBO ), CB_SETCURSEL, pos, 0 );
         else
            SendMessage( GetDlgItem( hWnd, IDC_TYPECOMBO ), CB_SETCURSEL, -1, 0 );
            
         break;
      }  
   }   
   
   switch( id )
   {
      case IDOK:
      
         // get the info and check that it is ok
         getData( ruleInfo );
         if( checkData( ruleInfo ) )
         {
            // check if add the rule to the end of the rule list
            if( pRuleInfo )
               *pRuleInfo = ruleInfo;
            else
               editor->rules.push_back( ruleInfo );
            editor->flags.set( LSEditor::RULESCHANGED, true );
            EndDialog( hWnd, 0 );
         }
         break;
         
      default:
         break;
   }
}
