//------------------------------------------------------------------------------
//
// Description:   Editor for Bayesian Networks
//    
// $Workfile$     BayWatch.cpp
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#include <wintools.h>
#include <G_Bitmap.h>

#include "BWEditNode.h"
#include "BWEditNet.h"
#include "BWDefs.h"

extern HBITMAP GFXtoDIB( GFXBitmap *pBM, HWND hw );

//------------------------------------------------------------------------------

NodeEditor::NodeEditor( ResourceManager *resManager)
{
   rm = resManager;
   en = NULL;
   hEd = NULL;
}   

//------------------------------------------------------------------------------

void NodeEditor::create( GWWindowClass *gwc, GWWindow *pParent )
{
   Parent::create( 
      gwc, "",
      DS_MODALFRAME|DS_3DLOOK|WS_OVERLAPPED|WS_VISIBLE|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX,
      Point2I(50,50),Point2I(500,300), 
      pParent, ID_NODE_EDITOR, true, true );
   SetWindowLong( hWnd, GWL_USERDATA, (LONG)en );

   hMenu = LoadMenu(GWMain::hInstance,"NodeMenu");
   SetMenu( hWnd, hMenu );

   setMaxSize(Point2I(45000,768) );

   hStatusBar = CreateStatusWindow( SBARS_SIZEGRIP|WS_CHILD|WS_VISIBLE, "Node Information", hWnd, IDC_STATUSBAR );
   initToolBar();
}   

//--------------------------------------------------------------------------------

void NodeEditor::setNode( EditNode *_en )
{
   en = _en;
   refresh();
}

//--------------------------------------------------------------------------------

void NodeEditor::refresh()
{
   SetWindowText( hNodeName, en->getBayesNode()->getName() );
   SetWindowText( hWnd, en->getBayesNode()->getName() );
   en->displayNode( this );
}   

//--------------------------------------------------------------------------------

void NodeEditor::onSize(UINT state, int cx, int cy)
{
   state, cx, cy;
   SendMessage( hStatusBar, WM_SIZE, state, MAKELONG(cx,cy) );
   SendMessage( hToolBar, WM_SIZE, state, MAKELONG(cx,cy) );
   render();
}   

//--------------------------------------------------------------------------------

int NodeEditor::onNotify(int id, LPNMHDR pnmhdr)
{
   switch( pnmhdr->code )
   {
      case TTN_NEEDTEXT:
      {
         TOOLTIPTEXT *ttt = (TOOLTIPTEXT*)pnmhdr;
         ttt->hinst = GWMain::hInstance;
         ttt->lpszText = (char*)id;
         return ( Parent::onNotify( id, pnmhdr ) );
      }
      
      case HDN_ITEMCLICK:
      {
         HD_NOTIFY *hdn = (HD_NOTIFY*)pnmhdr;
         if ( hdn->hdr.idFrom >= IDC_STATE )
         {
            if ( IsWindow(hEd) )
               SendMessage( hEd, WM_USER+1, NULL, NULL );
            int istate = hdn->hdr.idFrom - IDC_STATE;
            int item = hdn->iItem;
            if ( item == 0 )
               editStateName( istate );
            else if ( item == 1 )
               editStateValue( istate );
            else
               editStateTable( istate, item-2 );
         }
      }
   }

   return true;
}   

//--------------------------------------------------------------------------------

void NodeEditor::render()
{
   RECT  r1, r2, r3;
   GetClientRect( hWnd, &r1 );
   if ( hStatusBar )
      GetClientRect( hStatusBar, &r2 );
   if ( hToolBar )
      GetClientRect( hToolBar, &r3 );
}   

//------------------------------------------------------------------------------

void NodeEditor::initToolBar()
{
   hToolBar   = CreateToolbarEx( hWnd, WS_VISIBLE|WS_CHILD|TBSTYLE_ALTDRAG|TBSTYLE_TOOLTIPS|TBSTYLE_WRAPABLE|CCS_ADJUSTABLE, IDC_TOOLBAR, 0, 0, 0, 0, 0, 18, 18, 16, 16, sizeof(TBBUTTON) );

   // add GFX bitmaps to toolbar
   TBADDBITMAP tba;
   tba.hInst = NULL;
   Resource<GFXBitmap> rBM;
   rBM = rm->load("AddState.bmp",true);
   tba.nID   = (UINT)GFXtoDIB( rBM, hToolBar );
   int newID = SendMessage( hToolBar, TB_ADDBITMAP, 1, (LPARAM)&tba );
   rBM = rm->load("DelState.bmp",true);
   tba.nID   = (UINT)GFXtoDIB( rBM, hToolBar );
   SendMessage( hToolBar, TB_ADDBITMAP, 1, (LPARAM)&tba );
   rBM = rm->load("Reset.bmp",true);
   tba.nID   = (UINT)GFXtoDIB( rBM, hToolBar );
   SendMessage( hToolBar, TB_ADDBITMAP, 1, (LPARAM)&tba );
   rBM = rm->load("Recalculate.bmp",true);
   tba.nID   = (UINT)GFXtoDIB( rBM, hToolBar );
   SendMessage( hToolBar, TB_ADDBITMAP, 1, (LPARAM)&tba );

   // add GFX buttons to toolbar
   TBBUTTON gfxButtons[] = 
   {
      {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,NULL,0},
      {newID  , IDM_BW_ADDSTATE,    TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_ADDSTATE },
      {newID+1, IDM_BW_DELSTATE,    TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_DELSTATE },
      {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,NULL,0},
      {newID+2, IDM_BW_RESET,       TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_RESET },
      {newID+3, IDM_BW_RECALCULATE, TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_RECALCULATE },
      {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,NULL,0},
   };
   SendMessage( hToolBar, TB_ADDBUTTONS, 7, (LPARAM)&gfxButtons );

   int btncount = SendMessage( hToolBar, TB_BUTTONCOUNT, NULL, NULL );
   int x_off = 0;
   RECT r;
   for ( int i=0; i<btncount; i++ )
   {
      SendMessage( hToolBar, TB_GETITEMRECT, i, (LPARAM)&r );
      x_off += (r.right-r.left);
   }

   hNodeName = CreateWindow( "EDIT", "", 
         ES_AUTOVSCROLL|ES_MULTILINE|DS_3DLOOK|WS_CHILD|WS_VISIBLE|WS_BORDER,
         x_off,
         0,
         200,
         r.bottom-r.top,
         hToolBar,
         (HMENU)IDC_NODENAME,
         GWMain::hInstance,
         NULL );

   HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   SendMessage( hNodeName, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0) );
}   

//------------------------------------------------------------------------------

void NodeEditor::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   hwndCtl, codeNotify;
   switch( id )
   {
      case IDM_BW_CLOSE:
      case IDM_BW_EXIT:
         destroyWindow();
         return;
      
      case IDM_BW_ADDSTATE:
         en->getBayesNode()->addState( (char*)avar("State %i",en->getBayesNode()->numStates()) );
         en->addStateHeader( this, en->getBayesNode()->numStates()-1 );
         break;

      case IDC_NODENAME:
         if ( codeNotify == EN_VSCROLL )
         {
            char buff[128];
            GetWindowText( hwndCtl, buff, 128 );
            *strchr(buff,'\r')=0;
            en->getBayesNode()->setName( buff );
            SetWindowText( hWnd, buff );
            SendMessage( hwndCtl, WM_VSCROLL, MAKELONG(0,SB_TOP), NULL );
            InvalidateRect( GetParent(hWnd), NULL, TRUE );
            UpdateWindow( GetParent(hWnd) );
         }
         break;

      case IDM_BW_DELSTATE:
      case IDM_BW_RESET:
      case IDM_BW_RECALCULATE:
         DialogBox( GWMain::hInstance, "NoFunctionBox", hWnd, GenericDlg );
         break;

      default:
         break;
   }
}   

//------------------------------------------------------------------------------

struct StateNameEdit
{
   EditNode *en;
   NodeEditor     *pNE;
   int            index;
   enum  StateNameEditType
   {
      ED_STATE_NAME,
      ED_STATE_VALUE,
      ED_STATE_TABLE,
   }editType;
   int            item;
}SNE;

WNDPROC DefEdProc;

LRESULT PASCAL EdProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   if ( ((message==WM_KEYUP) && (wParam==0xd)) || (message==WM_USER_CLOSE)  )
   {
      switch( SNE.editType )
      {
         case StateNameEdit::ED_STATE_NAME:
            {
               char buff[128];
               GetWindowText( hWnd, buff, 128 );
               DestroyWindow( hWnd );
               *strchr(buff,'\r')=0;
               SNE.en->getBayesNode()->setStateName( SNE.index, buff );
               HWND hh = SNE.pNE->stateHeaderList[SNE.index];
               HD_ITEM hdi;
               hdi.mask       = HDI_TEXT|HDI_FORMAT|HDI_WIDTH;
               hdi.fmt        = HDF_CENTER|HDF_STRING;
               hdi.cxy        = STATE_COL_WIDTH;
               hdi.pszText    = buff;
               hdi.cchTextMax = strlen( hdi.pszText );
               Header_DeleteItem( hh, 0 );
               Header_InsertItem( hh, 0, &hdi );
            }
            break;
         
         case StateNameEdit::ED_STATE_VALUE:
            {
               char buff[128];
               GetWindowText( hWnd, buff, 128 );
               DestroyWindow( hWnd );
               *strchr(buff,'\r')=0;
               SNE.en->getBayesNode()->setState( SNE.index, atof(buff) );
               HWND hh = SNE.pNE->stateHeaderList[SNE.index];
               HD_ITEM hdi;
               hdi.mask       = HDI_TEXT|HDI_FORMAT|HDI_WIDTH;
               hdi.fmt        = HDF_RIGHT|HDF_STRING;
               hdi.cxy        = TABLE_COL_WIDTH;
               hdi.pszText    = (char*)avar( "%3.4f",atof(buff));
               hdi.cchTextMax = strlen( hdi.pszText );
               Header_DeleteItem( hh, 1 );
               Header_InsertItem( hh, 1, &hdi );
            }
            break;

         case StateNameEdit::ED_STATE_TABLE:
            {
               char buff[128];
               GetWindowText( hWnd, buff, 128 );
               DestroyWindow( hWnd );
               *strchr(buff,'\r')=0;
               ((ChanceNode*)SNE.en->getBayesNode())->setCPVal( atof(buff), SNE.index, SNE.item );
               HWND hh = SNE.pNE->stateHeaderList[SNE.index];
               HD_ITEM hdi;
               hdi.mask       = HDI_TEXT|HDI_FORMAT|HDI_WIDTH;
               hdi.fmt        = HDF_RIGHT|HDF_STRING;
               hdi.cxy        = TABLE_COL_WIDTH;
               hdi.pszText    = (char*)avar( "%3.4f",((ChanceNode*)SNE.en->getBayesNode())->getCPVal(SNE.index, SNE.item) );
               hdi.cchTextMax = strlen( hdi.pszText );
               Header_DeleteItem( hh, SNE.item+2 );
               Header_InsertItem( hh, SNE.item+2, &hdi );
            }
            break;
      }
   }
   return CallWindowProc( DefEdProc, hWnd, message, wParam, lParam );
}   

void NodeEditor::editStateName( int index )
{
   RECT  r;
   if ( stateHeaderList.size() )
      GetClientRect( stateHeaderList[0], &r );

   hEd = CreateWindow( "EDIT", en->getBayesNode()->getStateName(index), 
         ES_AUTOVSCROLL|ES_MULTILINE|DS_3DLOOK|WS_CHILD|WS_VISIBLE|WS_BORDER,
         0,0,
         STATE_COL_WIDTH,
         r.bottom,
         stateHeaderList[ index ],
         (HMENU)IDC_STATE_ED,
         GWMain::hInstance,
         NULL );

   SetFocus( hEd );
   SNE.en        = en;
   SNE.pNE        = this;
   SNE.index      = index;
   SNE.editType   = StateNameEdit::ED_STATE_NAME;
   DefEdProc = (WNDPROC)GetWindowLong( hEd, GWL_WNDPROC );
   SetWindowLong( hEd, GWL_WNDPROC, (LONG)EdProc );
   HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   SendMessage( hEd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0) );
   SendMessage( hEd, EM_SETSEL, 0, -1 );
}

//------------------------------------------------------------------------------

void NodeEditor::editStateValue( int index )
{
   RECT  r;
   if ( stateHeaderList.size() )
      GetClientRect( stateHeaderList[0], &r );

   HWND hEd = CreateWindow( "EDIT", avar("%3.4f",en->getBayesNode()->getState(index)), 
         ES_AUTOVSCROLL|ES_MULTILINE|DS_3DLOOK|WS_CHILD|WS_VISIBLE|WS_BORDER,
         STATE_COL_WIDTH,0,
         TABLE_COL_WIDTH,
         r.bottom,
         stateHeaderList[ index ],
         (HMENU)IDC_STATE_ED,
         GWMain::hInstance,
         NULL );

   SetFocus( hEd );
   SNE.en        = en;
   SNE.pNE        = this;
   SNE.index      = index;
   SNE.editType   = StateNameEdit::ED_STATE_VALUE;
   DefEdProc = (WNDPROC)GetWindowLong( hEd, GWL_WNDPROC );
   SetWindowLong( hEd, GWL_WNDPROC, (LONG)EdProc );
   HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   SendMessage( hEd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0) );
   SendMessage( hEd, EM_SETSEL, 0, -1 );
}

//------------------------------------------------------------------------------

void NodeEditor::editStateTable( int iState, int index )
{
   RECT  r;
   if ( stateHeaderList.size() )
      GetClientRect( stateHeaderList[0], &r );

   HWND hEd = CreateWindow( "EDIT", avar("%3.4f",((ChanceNode*)en->getBayesNode())->getCPVal(iState,index)),
         ES_AUTOVSCROLL|ES_MULTILINE|DS_3DLOOK|WS_CHILD|WS_VISIBLE|WS_BORDER,
         STATE_COL_WIDTH+TABLE_COL_WIDTH + index*TABLE_COL_WIDTH, 0,
         TABLE_COL_WIDTH,
         r.bottom,
         stateHeaderList[ iState ],
         (HMENU)IDC_STATE_ED,
         GWMain::hInstance,
         NULL );

   SetFocus( hEd );
   SNE.en        = en;
   SNE.pNE        = this;
   SNE.index      = iState;
   SNE.item       = index;
   SNE.editType   = StateNameEdit::ED_STATE_TABLE;
   DefEdProc = (WNDPROC)GetWindowLong( hEd, GWL_WNDPROC );
   SetWindowLong( hEd, GWL_WNDPROC, (LONG)EdProc );
   HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   SendMessage( hEd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0) );
   SendMessage( hEd, EM_SETSEL, 0, -1 );
}
