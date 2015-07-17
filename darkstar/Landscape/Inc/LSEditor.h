//----------------------------------------------------------------------------
//   
//  $Workfile:   LSEditor.cpp  $
//  $Version$
//  $Revision:   1.00  $
//    
//  DESCRIPTION:
//        Editor for LandScape scripts
//        
//  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
// 
//----------------------------------------------------------------------------

#ifndef _LSEDITOR_H_
#define _LSEDITOR_H_

#include "wintools.h"
#include <COMMCTRL.h>
#include <COMMDLG.h>
#include <RICHEDIT.h>

#include <gw.h>
#include <g_pal.h>
#include <g_surfac.h>

#include "LSMapper.h"
#include <g_bitmap.h>

//------------------------------------------------------------------------------
class SimInputPriorityQuery;
class LSEditor;

class EditRuleDlg : public GWDialog
{
   private:
   
      BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
      void onCommand( int id, HWND hwndCtl, UINT codeNotify );
      void setData( RuleInfo & rule );
      void getData( RuleInfo & rule );
      bool checkData( const RuleInfo & rule );
      void updateTypes();
      float atofDif( float val, const char * buff );
      
      RuleInfo * pRuleInfo;      // passed in
      RuleInfo ruleInfo;         // info used in dialog
      LSEditor * editor;         // parent
      
   public:
      void setEditor( LSEditor * ed ){ editor = ed; }
};

class LSEditor : public GWDialog, public SimObject
{
   friend EditRuleDlg;
   
private:

   typedef     GWDialog Parent;
   typedef     SimObject SimParent;

   BitSet32    flags;
   GWCanvas    *pCanvas;            // handle to editor's preview window
   HMENU       hMenu;               // the Main menu
   FileInfo    *pScriptFI;          // a Script fileInfo object
   FileInfo    *pBMFI;              // a bitmap fileInfo object
   FileInfo    *pRulesFI;           // rules fileinfo obj
   GFXPalette  *pPal;               // a palette to use
   LSMapper    *pLSMapper;          // ptr to a LandScape Mapper object
   LandScape   *pLS;                // ptr to LSMapper's LandScape object
   HINSTANCE   hDLL;                // handle to riched32.dll

   EditRuleDlg  editRuleDlg;        // rule creating/editing dialog

   Vector< RuleInfo > rules;        // the terrain rules list
   
public:
   enum
   {
      PREVIEW        = (1<<0),
      STANDALONE     = (1<<1),
      SIMINPUTFOCUS  = (1<<2),
      SCRIPTCHANGED  = (1<<3),
      RULESCHANGED   = (1<<4)
   };
   enum           // tab page's
   {
      SCRIPT         = 0,
      RULES          = 1
   };

private:
   // Landscape Editor's methods
   void saveScript();
   void loadScript();
   void loadRules();
   void saveRules();
   void saveBitmap();
   void loadBitmap();
   void render();
   void onCommand(int id, HWND hwndCtl, UINT codeNotify);
   void onSysCommand(UINT cmd, int x, int y);

   int onNotify( int id, LPNMHDR pnmhdr );
   void showTabPage( int page = -1 );
   char * getTypeDesc( int index );

//   void onDestroy();
   GFXBitmap   *get() { return (pLS->get(1)); }  // get bitmap from LS
   LandScape   *getLS() { return (pLS); }       // give user direct access

public:
   LSEditor();

   bool create( GWWindow *parent, LSMapper *lsMapper, LandScape *landscape, bool standAlone );

   // simobject methods
   bool processEvent(const SimEvent*);
   bool processQuery(SimQuery* q);
   
   bool onAdd();
   void onRemove();

   bool onSimEditEvent(const SimEditEvent *event);
   bool onSimGainFocusEvent(const SimGainFocusEvent *event);
   bool onSimLoseFocusEvent(const SimLoseFocusEvent *event);
   void onSetFocus(HWND hwndOldFocus);

   bool onSimInputPriorityQuery( SimInputPriorityQuery *query );
};

#endif   // _LSEDITOR_H_
