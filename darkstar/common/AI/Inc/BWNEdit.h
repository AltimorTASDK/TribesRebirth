//------------------------------------------------------------------------------
//
// Description:   Node Editor for BayWatch network editor
//    
// $Workfile$     BWNEdit.h
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#ifndef _BWNEdit_H_
#define _BWNEdit_H_

#include "BWEditNode.h"
#include "BWEditNet.h"
#include "BWDefs.h"

class EditNode;

class NodeEditor : public GWWindow
{
   ResourceManager *rm;
   typedef     GWWindow Parent;
   EditNode    *en;
   void onCommand(int id, HWND hwndCtl, UINT codeNotify);

   HMENU       hMenu;               // the Main menu
   HWND        hStatusBar;          // handle to status bar
   HWND        hNodeName;           // handle to node name window
   HWND        hEd;                 // handle to data text edit window

   void onSize(UINT state, int cx, int cy);
   int  onNotify(int id, LPNMHDR pnmhdr);
   void render();

public:
   Vector<HWND>   parentHeaderList;
   Vector<HWND>   descList;
   Vector<HWND>   stateHeaderList;
   HWND           hToolBar;         // handle to our toolbar

public:
   NodeEditor( ResourceManager *resManager );
   void create( GWWindowClass *gwc, GWWindow *pParent );
   void initToolBar();
   void initHeader();
   void setNode( EditNode *pEN );
   void refresh();

   void editStateName( int index );
   void editStateValue( int index );
   void editStateTable( int iState, int index );
};

#endif   // _BWNEdit_H_
