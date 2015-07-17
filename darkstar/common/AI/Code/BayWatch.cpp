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

#include <BayWatch.h>
#include <GW.h>
#include <G_Bitmap.h>
#include <TAlgorithm.h>
#include <SimResource.h>
#include <Console.h>
#include "BWEditNode.h"
#include "BWEditNet.h"

GWWindowClass BayWatch::gwc;

HBITMAP GFXtoDIB( GFXBitmap *pBM, HWND hw );

EditNet::EditNodeList  BWCanvas::selected;

//----------------------------------------------------------------------------

BayWatch::BayWatch()
{
   hMenu       = 0;
   hStatusBar  = 0;
   hToolBar    = 0;
   flags.clear();
   pCanvas     = NULL;
   pFI         = NULL;
}

BayWatch::~BayWatch()
{
   DestroyMenu( hMenu );
}   

//----------------------------------------------------------------------------

bool BayWatch::create( GWWindow *parent, bool standAlone )
{
   bool status=false;
   flags.set(STANDALONE,standAlone);
   gwc.registerClass( 
      CS_BYTEALIGNCLIENT|CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS,
      LoadIcon(GWMain::hInstance, MAKEINTRESOURCE(1)),
      LoadCursor(NULL,IDC_ARROW),
      (HBRUSH)(COLOR_BTNFACE+1)
      );

   if ( Parent::create( 
                  &gwc,"Bayesian Network Editor", 
                  DS_MODALFRAME | DS_3DLOOK | WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
                  Point2I(50,50), Point2I(640,480),
                  parent,NULL,true,true )
      )
   {
      flags.set(SIMINPUTFOCUS);

      pFI = new FileInfo;
      pFI->init( hWnd, ".AI", FileInfo::TYPE_BAY, NULL ); 

      hMenu = LoadMenu(GWMain::hInstance,"BWMainMenu");
      SetMenu( hWnd, hMenu );

      hStatusBar = CreateStatusWindow( SBARS_SIZEGRIP|WS_CHILD|WS_VISIBLE, "Node Information", hWnd, IDC_STATUSBAR );
      initToolBar();

      pCanvas = new BWCanvas( SimResource::get(manager) );
      pCanvas->createEx( WS_EX_CLIENTEDGE,&gwc,"",WS_CHILD|WS_VISIBLE,Point2I(0,0),Point2I(1,1), this,IDC_CANVAS,NULL);

      if ( pCanvas )
      {
         RECT  r1, r2, r3;
         GetClientRect( hWnd, &r1 );
         if ( hStatusBar )
            GetClientRect( hStatusBar, &r2 );
         if ( hToolBar )
            GetClientRect( hToolBar, &r3 );
         MoveWindow( pCanvas->getCanvas(), 2,r3.bottom+2,r1.right-4,r1.bottom-r2.bottom-r3.bottom-4, true );
      }

      InvalidateRect( hWnd, NULL, TRUE );
      UpdateWindow( hWnd );
   }
   return( status );
}

//------------------------------------------------------------------------------

void BayWatch::setBayesNet( BayesNet *pBN )
{
   if ( pCanvas->nodeEditor && IsWindow(pCanvas->nodeEditor->getHandle()) )
   {
      delete pCanvas->nodeEditor;
      pCanvas->nodeEditor = NULL;
   }
   delete pCanvas->pENet;
   pCanvas->pENet = new EditNet( pCanvas->getHandle() );
   pCanvas->pENet->setBayesNet( pBN );
   InvalidateRect( hWnd, NULL, TRUE );
}

//--------------------------------------------------------------------------------

void BayWatch::onDestroy()
{
   if ( flags.test(STANDALONE) )
      PostQuitMessage(0);
   else
      Parent::onDestroy();
}

//--------------------------------------------------------------------------------

void BayWatch::
onSize(UINT state, int cx, int cy)
{
   state, cx, cy;
   SendMessage( hStatusBar, WM_SIZE, state, MAKELONG(cx,cy) );
   SendMessage( hToolBar, WM_SIZE, state, MAKELONG(cx,cy) );

   if ( pCanvas )
   {
      RECT  r1, r2, r3;
      GetClientRect( hWnd, &r1 );
      if ( hStatusBar )
         GetClientRect( hStatusBar, &r2 );
      if ( hToolBar )
         GetClientRect( hToolBar, &r3 );
      if ( pCanvas )
         MoveWindow( pCanvas->getCanvas(), 2,r3.bottom+2,r1.right-4,r1.bottom-r2.bottom-r3.bottom-4, true );
   }
}   

//--------------------------------------------------------------------------------

int BayWatch::onNotify(int id, LPNMHDR pnmhdr)
{
   TOOLTIPTEXT *ttt = (TOOLTIPTEXT*)pnmhdr;
   if ( ttt->hdr.code == TTN_NEEDTEXT )
   {
      ttt->hinst = GWMain::hInstance;
      ttt->lpszText = (char*)id;
      return ( Parent::onNotify( id, pnmhdr ) );
   }
   return true;
}   

//--------------------------------------------------------------------------------

void BayWatch::render( HDC hDC )
{
   if ( pCanvas )
   {
      pCanvas->render( hDC );
   }
}   

//--------------------------------------------------------------------------------
// Process Menu Events

#define handleCommand( c )  case c: on##c( hwndCtl, codeNotify ); break;

void BayWatch::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   hwndCtl;
   codeNotify;

   switch( id )
   {
      handleCommand( IDM_BW_NEW );
      handleCommand( IDM_BW_LOAD );
      handleCommand( IDM_BW_SAVE );
      handleCommand( IDM_BW_CHANCE );
      handleCommand( IDM_BW_LINK );
      handleCommand( IDM_BW_EXIT );
      handleCommand( IDM_BW_ABOUT );
      handleCommand( IDM_BW_CUT );
      handleCommand( IDM_BW_REDRAW );
      handleCommand( IDM_BW_SPAWNEDITOR );
      handleCommand( IDM_BW_PROPERTIES );
      handleCommand( IDM_BW_NOFUNCTION );
      handleCommand( IDM_BW_PRINT );
      handleCommand( IDM_BW_COPY );
      handleCommand( IDM_BW_PASTE );
      handleCommand( IDM_BW_DELETE );
      handleCommand( IDM_BW_HELP );
      handleCommand( IDM_BW_DEBUG );
      handleCommand( IDM_BW_EVALUATE );
   }
   checkMenu();
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_NEW( HWND, UINT )
{
   if ( pCanvas->nodeEditor && IsWindow(pCanvas->nodeEditor->getHandle()) )
   {
      delete pCanvas->nodeEditor;
      pCanvas->nodeEditor = NULL;
   }

   delete pCanvas->pENet;
   pCanvas->pENet = new EditNet(pCanvas->getHandle());
   pCanvas->pENet->setBayesNet( new BayesNet() );
   InvalidateRect( hWnd, NULL, TRUE );
}   

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_LOAD( HWND, UINT )
{
   if ( pCanvas->nodeEditor && IsWindow(pCanvas->nodeEditor->getHandle()) )
   {
      delete pCanvas->nodeEditor;
      pCanvas->nodeEditor = NULL;
   }

   if ( !pFI->getName(FileInfo::OPEN_MODE, "Load Belief Network...") )
      return;
   delete pCanvas->pENet;
   pCanvas->pENet = EditNet::load( pFI->fileName(), pCanvas->getHandle() );
   if ( !pCanvas->pENet )
      MessageBox( hWnd, "Error loading network","Bayes Load...", MB_OK );
   InvalidateRect( hWnd, NULL, TRUE );
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_SAVE( HWND, UINT )
{
   if ( !pFI->getName(FileInfo::SAVE_MODE, "Save Belief Network...") )
      return;
   if ( pCanvas->pENet->save(pFI->fileName()) )
      MessageBox( hWnd, "Error saving network","Bayes Save...", MB_OK );
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_CHANCE( HWND, UINT )
{
   pCanvas->pENet->newNode();
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_LINK( HWND, UINT )
{
   pCanvas->flags.toggle(BWCanvas::LINKMODE);
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_EXIT( HWND, UINT )
{
   destroyWindow();
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_ABOUT( HWND, UINT )
{
   DialogBox( GWMain::hInstance, "BWAboutBox", hWnd, GenericDlg );
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_CUT( HWND, UINT )
{
   while( pCanvas->selected.size() )
   {
      EditNode *en = pCanvas->selected.front();
      pCanvas->selected.pop_front();
      pCanvas->pENet->delNode(en);
   }
   InvalidateRect(pCanvas->getHandle(),NULL,TRUE);
   RedrawWindow(pCanvas->getHandle(),NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_REDRAW( HWND, UINT )
{
   InvalidateRect(pCanvas->getHandle(),NULL,TRUE);
   RedrawWindow(pCanvas->getHandle(),NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_SPAWNEDITOR( HWND, UINT )
{
   pCanvas->flags.toggle(BWCanvas::SPAWNEDITOR);
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_PROPERTIES( HWND, UINT )
{
   if ( pCanvas->selected.size() )
      pCanvas->edit( (EditNode *)pCanvas->selected.front() );
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_EVALUATE( HWND, UINT )
{
   pCanvas->pENet->evaluate();
   if ( pCanvas->nodeEditor )
      pCanvas->nodeEditor->refresh();
   InvalidateRect(pCanvas->getHandle(),NULL,TRUE);
   RedrawWindow(pCanvas->getHandle(),NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
}   

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_NOFUNCTION( HWND, UINT )
{
   
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_PRINT( HWND, UINT )
{
   
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_COPY( HWND, UINT )
{
   
}  

//------------------------------------------------------------------------------
 
void BayWatch::onIDM_BW_PASTE( HWND, UINT )
{
   
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_DELETE( HWND, UINT )
{
   
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_DEBUG( HWND, UINT )
{
   DebugBreak();      
}

//------------------------------------------------------------------------------

void BayWatch::onIDM_BW_HELP( HWND, UINT )
{
   DialogBox( GWMain::hInstance, "BWNoFunctionBox", hWnd, GenericDlg );
}

//------------------------------------------------------------------------------

bool BayWatch::processEvent(const SimEvent* event)
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

bool BayWatch::onAdd()
{
	if (!SimObject::onAdd())
		return false;
   addToSet( SimInputConsumerSetId );
   return ( true );
}   

//------------------------------------------------------------------------------

void BayWatch::onRemove()
{
   removeFromSet( SimInputConsumerSetId );
   delete pFI;
}   

//------------------------------------------------------------------------------

bool BayWatch::onSimEditEvent(const SimEditEvent *event)
{
   if (event->editorId == id)
      flags.set( SIMINPUTFOCUS, event->state );
   return ( true );
}   

//------------------------------------------------------------------------------

bool BayWatch::onSimGainFocusEvent(const SimGainFocusEvent *)
{
   flags.set(SIMINPUTFOCUS,true);
   return ( true );
}   

//------------------------------------------------------------------------------

bool BayWatch::onSimLoseFocusEvent(const SimLoseFocusEvent *)
{
   flags.set(SIMINPUTFOCUS,false);
   return ( true );
}   

//------------------------------------------------------------------------------

bool BayWatch::processQuery( SimQuery* query )
{
   switch ( query->type )
   {
      onQuery( SimInputPriorityQuery );
   }
   return ( false );
}

//------------------------------------------------------------------------------

bool BayWatch::onSimInputPriorityQuery( SimInputPriorityQuery *query )
{
   query->priority = SI_PRIORITY_NORMAL+1;
   return true;
}   

//------------------------------------------------------------------------------

void BayWatch::onSetFocus(HWND hwndOldFocus)
{
   hwndOldFocus;
   if ( !flags.test(STANDALONE) )
      Console->evaluate("inputDeactivate(mouse,0);", false );
   flags.set(SIMINPUTFOCUS,true);
}   

//------------------------------------------------------------------------------

void BayWatch::initToolBar()
{
   hToolBar   = CreateToolbarEx( hWnd, WS_VISIBLE|WS_CHILD|TBSTYLE_ALTDRAG|TBSTYLE_TOOLTIPS|TBSTYLE_WRAPABLE|CCS_ADJUSTABLE, IDC_TOOLBAR, 0, 0, 0, 0, 0, 32, 32, 25, 23, sizeof(TBBUTTON) );

   // add standard bitmaps to toolbar
   TBADDBITMAP tba;
   tba.hInst = HINST_COMMCTRL;
   tba.nID   = IDB_STD_SMALL_COLOR;
   SendMessage( hToolBar, TB_ADDBITMAP, NULL, (LPARAM)&tba );

   // add standard buttons to toolbar
   TBBUTTON buttons[] =
   {
      {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,NULL,0},
      {STD_FILENEW,  IDM_BW_NEW,    TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_NEW },
      {STD_FILEOPEN, IDM_BW_LOAD,   TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_LOAD },
      {STD_FILESAVE, IDM_BW_SAVE,   TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_SAVE },
      {STD_PRINT,    IDM_BW_PRINT,  TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_PRINT },
      {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,NULL,0},
      {STD_CUT,      IDM_BW_CUT,    TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_CUT },
      {STD_COPY,     IDM_BW_COPY,   TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_COPY },
      {STD_PASTE,    IDM_BW_PASTE,  TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_PASTE },
      {STD_DELETE,   IDM_BW_DELETE, TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_DELETE },
      {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,NULL,0},
      {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,NULL,0},
      {STD_DELETE,   IDM_BW_DELETE, TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_DELETE },
   };
   SendMessage( hToolBar, TB_ADDBUTTONS, 12, (LPARAM)&buttons );

   // add GFX bitmaps to toolbar
   tba.hInst = NULL;
   Resource<GFXBitmap> rBM;
   rBM = SimResource::get(manager)->load("Chance.bmp",true);
   tba.nID   = (UINT)GFXtoDIB( rBM, hToolBar );
   int newID = SendMessage( hToolBar, TB_ADDBITMAP, 1, (LPARAM)&tba );
   rBM = SimResource::get(manager)->load("Link.bmp",true);
   tba.nID   = (UINT)GFXtoDIB( rBM, hToolBar );
   SendMessage( hToolBar, TB_ADDBITMAP, 1, (LPARAM)&tba );
   rBM = SimResource::get(manager)->load("Evaluate.bmp",true);
   tba.nID   = (UINT)GFXtoDIB( rBM, hToolBar );
   SendMessage( hToolBar, TB_ADDBITMAP, 1, (LPARAM)&tba );
   rBM = SimResource::get(manager)->load("Debug.bmp",true);
   tba.nID   = (UINT)GFXtoDIB( rBM, hToolBar );
   SendMessage( hToolBar, TB_ADDBITMAP, 1, (LPARAM)&tba );

   // add GFX buttons to toolbar
   TBBUTTON gfxButtons[] = 
   {
      {newID  , IDM_BW_CHANCE,   TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_CHANCE },
      {newID+1, IDM_BW_LINK,     TBSTATE_ENABLED,TBSTYLE_CHECK, NULL, (unsigned char)IDM_BW_LINK },
      {STD_PROPERTIES, IDM_BW_PROPERTIES , TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_PROPERTIES },
      {newID+2, IDM_BW_EVALUATE, TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_EVALUATE },
      {0,0,TBSTATE_ENABLED,TBSTYLE_SEP,NULL,0},
      {newID+3, IDM_BW_DEBUG,    TBSTATE_ENABLED,TBSTYLE_BUTTON, NULL, (unsigned char)IDM_BW_DEBUG },
   };
   SendMessage( hToolBar, TB_ADDBUTTONS, 6, (LPARAM)&gfxButtons );
}   

//------------------------------------------------------------------------------

HBITMAP GFXtoDIB( GFXBitmap *pBM, HWND hw )
{
   if ( !pBM )
      return NULL;

   GFXPalette p;
   p.setWindowsColors();

   HBITMAP hBM;

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

   for ( int i=0; i<256; i++ )
   {
      bi.bmiColors[i].rgbRed     = p.palette[0].color[i].peRed;
      bi.bmiColors[i].rgbGreen   = p.palette[0].color[i].peGreen;
      bi.bmiColors[i].rgbBlue    = p.palette[0].color[i].peBlue;
      bi.bmiColors[i].rgbReserved = 0;
   }

   HDC dc = GetDC(hw);
   hBM = CreateDIBitmap( dc, &bi.bmiHeader, CBM_INIT, pBM->pBits, (LPBITMAPINFO)&bi, DIB_RGB_COLORS );
   ReleaseDC(hw, dc);

   return ( hBM );
}   

//##############################################################################
//--------------------------------------------------------------------------------

BWCanvas::BWCanvas( ResourceManager *resManager )
{
   rm = resManager;
   pENet = NULL;
   nodeEditor = NULL;
}   

//--------------------------------------------------------------------------------

void BWCanvas::onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   fDoubleClick;

   if ( selected.size() && !(keyFlags&MK_SHIFT) )
   {
      for ( iterator itr=selected.begin(); itr!=selected.end(); itr++ )
      {
         SetWindowLong( ((EditNode*)(*itr))->hWnd, GWL_STYLE, SS_CENTER|WS_CHILD|WS_VISIBLE|WS_DLGFRAME );
         ((*itr)->flags.set(EditNode::SELECTED,false));   
      }
      selected.clear();
   }

   anchorPt(x,y);    // set our anchor point
   oldPt = anchorPt;
   POINT pt;
   pt.x = x; 
   pt.y = y;
   HWND hw = ChildWindowFromPointEx( getCanvas(), pt, CWP_ALL );
   EditNode *pEN = EditNode::getEditNode( hw );
   // if en is in list, remove it, otherwise add it to selected list
   if ( pEN )
   {
      iterator itr;
      for ( itr=selected.begin(); itr!=selected.end() && (*itr)!=pEN; itr++ );
      if ( !selected.size() || (*itr)!=pEN )
      {
         pEN->flags.set(EditNode::SELECTED,true);
         if ( !flags.test(LINKMODE) || !selected.size() )
         {
            selected.push_front( pEN );
            SetWindowLong( hw, GWL_STYLE, SS_CENTER|WS_CHILD|WS_VISIBLE|WS_BORDER );
         }
         if ( fDoubleClick )
         {
            edit( pEN );
         }
      }
      else
      {
         selected.erase(itr);
         pEN->flags.set(EditNode::SELECTED,false);
         SetWindowLong( hw, GWL_STYLE, SS_CENTER|WS_CHILD|WS_VISIBLE|WS_DLGFRAME );
      }

      BringWindowToTop(hw);
      RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
      RedrawWindow(hw,NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
   }
}

//--------------------------------------------------------------------------------

void BWCanvas::onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   onLButtonDown(fDoubleClick,x,y,keyFlags);
}

//--------------------------------------------------------------------------------

void BWCanvas::onLButtonUp(int x, int y, UINT keyFlags)
{
   x;y;keyFlags;

   if ( selected.size() && flags.test(LINKMODE) )
   {
      POINT pt;
      pt.x = x; 
      pt.y = y;
      HWND hw = ChildWindowFromPointEx( getCanvas(), pt, CWP_ALL );
      EditNode *pEN = EditNode::getEditNode( hw );
      if ( pEN && (pEN != selected.front()) )   // if not connecting to self
         if ( !selected.front()->isChild(pEN) ) // if pEN not a child
            if ( !selected.front()->isParent(pEN) )   // if pEN not a parent
               selected.front()->addChild( pEN );
            else
            {
               selected.front()->delParent( pEN );
               pEN->delChild( selected.front() );
            }
         else
         {
            selected.front()->delChild( pEN );
            pEN->delParent( selected.front() );
         }

      InvalidateRect(hWnd,NULL,TRUE);
      RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
   }
//   else if ( selected.size() && !(keyFlags&MK_SHIFT) )
//   {
//      for ( EditNode::iterator itr=selected.begin(); itr!=selected.end(); itr++ )
//         ((*itr)->flags.set(EditNode::SELECTED,false));
//      selected.clear();
//   }
}

//--------------------------------------------------------------------------------

void BWCanvas::onRButtonUp(int x, int y, UINT keyFlags)
{
   onLButtonUp(x,y,keyFlags);
}

//--------------------------------------------------------------------------------

void BWCanvas::onMouseMove(int x, int y, UINT keyFlags)
{
   x; y; keyFlags;

   if ( keyFlags&(MK_LBUTTON|MK_RBUTTON) && selected.size() )
   {
         RECT r;
      Point2I pt(x,y);

      if ( !flags.test(LINKMODE) )
      {
         pt -= anchorPt;
         for ( iterator itr=selected.begin(); itr!=selected.end(); itr++ )
         {
            HWND hw = ((EditNode*)(*itr))->hWnd;
            GetWindowRect( hw, &r );
            ScreenToClient( getCanvas(), (POINT*)&r );
            ScreenToClient( getCanvas(), (POINT*)&r.right );
            if ( keyFlags&MK_RBUTTON )
            {
               SetWindowLong( hw, GWL_STYLE, SS_CENTER|WS_CHILD|WS_VISIBLE|WS_DLGFRAME );
               MoveWindow( hw, r.left, r.top, r.right-r.left+pt.x, r.bottom-r.top+pt.y, true );
               SetWindowLong( hw, GWL_STYLE, SS_CENTER|WS_CHILD|WS_VISIBLE|WS_BORDER );
               InvalidateRect(hWnd,NULL,TRUE);
               RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
               RedrawWindow(hw,NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
            }
            else
            {
               MoveWindow( hw, r.left+pt.x, r.top+pt.y, r.right-r.left, r.bottom-r.top, true );
               InvalidateRect(hWnd,NULL,TRUE);
               RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
               RedrawWindow(hw,NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
            }
         }
         anchorPt += pt;
      }
      else
      {
         HDC hDC = GetDC( getCanvas() );
         SetROP2(hDC,R2_NOT);
         MoveToEx( hDC, anchorPt.x,anchorPt.y, NULL );
         LineTo( hDC, oldPt.x, oldPt.y );
         MoveToEx( hDC, anchorPt.x,anchorPt.y, NULL );
         LineTo( hDC,x,y );
         oldPt = pt;
         ReleaseDC(getCanvas(),hDC);
      }
   }
}

//------------------------------------------------------------------------------

void BWCanvas::onPaint()
{
   PAINTSTRUCT ps;
   BeginPaint( hWnd, &ps );
   render( ps.hdc );
   EndPaint( hWnd, &ps );
}   

//--------------------------------------------------------------------------------

void BWCanvas::render( HDC hDC )
{
   SetROP2(hDC,R2_BLACK);
   if ( pENet )
      pENet->render( hDC );
}   

//------------------------------------------------------------------------------

HWND BWCanvas::getCanvas()
{
   return hWnd;
}   

//------------------------------------------------------------------------------

void BWCanvas::edit( EditNode* pBN )
{

   if ( !nodeEditor || !IsWindow(nodeEditor->getHandle()) || flags.test(SPAWNEDITOR) )
   {
      nodeEditor = new NodeEditor(rm);
      nodeEditor->create( &BayWatch::gwc, this );
      nodeEditor->setNode( pBN );
   }
   else
      nodeEditor->setNode( pBN );
}   

//------------------------------------------------------------------------------

void BayWatch::checkMenu()
{
   CheckMenuItem( hMenu, IDM_BW_SPAWNEDITOR, pCanvas->flags.test(BWCanvas::SPAWNEDITOR)? MF_CHECKED:MF_UNCHECKED );
}   