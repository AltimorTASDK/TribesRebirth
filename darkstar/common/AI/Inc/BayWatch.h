//------------------------------------------------------------------------------
//
// Description:   Base class for Bayesian Network Node object used in AI
//    
// $Workfile$     Bayes.h
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#ifndef _BAYWATCH_H_
#define _BAYWATCH_H_

#include <wintools.h>
#include <sim.h>
#include <simInput.h>
#include <gw.h>
#include <BayesNode.h>
#include <BayesNet.h>
#include <BayesChance.h>

#include "BWDefs.h"
#include "BWEditNode.h"
#include "BWEditNet.h"


class BayWatch;
//------------------------------------------------------------------------------

class BWCanvas : public GWWindow
{
   typedef           GWWindow Parent;
   Point2I           anchorPt;
   Point2I           oldPt;
   ResourceManager   *rm;

public:
   EditNet           *pENet;
   NodeEditor        *nodeEditor;
   BitSet32          flags;

   static   EditNet::EditNodeList  selected;       // objects in net that are selected or hilightted
   typedef  EditNet::EditNodeList::iterator iterator;

   enum BWCSTATES
   {
      LINKMODE       = (1<<0),
      SPAWNEDITOR    = (1<<1),
   };

public:
   BWCanvas( ResourceManager *resManager );
   void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
   void onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
   void onLButtonUp(int x, int y, UINT keyFlags);
   void onRButtonUp(int x, int y, UINT flags);
   void onMouseMove(int x, int y, UINT keyFlags);
   void onPaint();

   void render( HDC hDC );
   HWND getCanvas();
   void edit( EditNode* pBN );
};

//------------------------------------------------------------------------------

class BayWatch : public GWWindow, public SimObject
{
private:

   typedef     GWWindow Parent;
   typedef     SimObject SimParent;

   BitSet32    flags;
   FileInfo    *pFI;                // a Script fileInfo object
   BWCanvas    *pCanvas;            // handle to our bayesian canvas

   HMENU       hMenu;               // the Main menu
   HWND        hStatusBar;          // handle to status bar
   HWND        hToolBar;            // handle to our toolbar

   void onCommand(int id, HWND hwndCtl, UINT codeNotify);
   void onDestroy();
   void onSize(UINT state, int cx, int cy);
   int  onNotify(int id, LPNMHDR pnmhdr);

   void render( HDC hDC );
   void initToolBar();
   void checkMenu();

public:
   enum BWSTATES
   {
      STANDALONE     = (1<<0),
      SIMINPUTFOCUS  = (1<<1),
   };
   static GWWindowClass gwc;

public:
   BayWatch();
   ~BayWatch();
   bool create( GWWindow *parent, bool standAlone );

   // simobject methods
   bool processEvent(const SimEvent*);
   bool onAdd();
   void onRemove();
   bool onSimEditEvent(const SimEditEvent *event);
   bool onSimGainFocusEvent(const SimGainFocusEvent *event);
   bool onSimLoseFocusEvent(const SimLoseFocusEvent *event);
   void onSetFocus(HWND hwndOldFocus);
   void setBayesNet( BayesNet *pBN );

   // command methods
   void onIDM_BW_NEW( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_LOAD( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_SAVE( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_CHANCE( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_LINK( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_EXIT( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_ABOUT( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_CUT( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_REDRAW( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_SPAWNEDITOR( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_PROPERTIES( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_NOFUNCTION( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_PRINT( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_COPY( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_PASTE( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_DELETE( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_HELP( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_DEBUG( HWND hwndCtl, UINT codeNotify );
   void onIDM_BW_EVALUATE( HWND hwndCtl, UINT codeNotify );

   bool processQuery(SimQuery* q);
   bool onSimInputPriorityQuery( SimInputPriorityQuery *query );
};

#endif // _BAYWATCH_H_