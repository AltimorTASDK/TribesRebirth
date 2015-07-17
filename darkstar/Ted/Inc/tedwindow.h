#ifndef _TEDWINDOW_H
#define _TEDWINDOW_H

#include "simted.h"
#include "gwcombobox.h"
#include "gw.h"

class DisplayOptionsDlg : public GWDialog
{
   private:
      UInt8 shadowFrameColor;
      UInt8 shadowFillColor;
      bool blockOutline;
      UInt8 blockFrameColor;
      UInt8 hilightFrameColor;
      UInt8 hilightFillColor;
      UInt8 selectFrameColor;
      UInt8 selectFillColor;
      SimTed * simTed;
      
      BitSet32 shadowShow;
      BitSet32 selectShow;
      BitSet32 hilightShow;
      
   public:
      DisplayOptionsDlg();
      ~DisplayOptionsDlg();
      void fillInfo();
      void setInfo();
      void onCommand( int id, HWND hwndCtl, UINT codeNotify );
      LRESULT windowProc( UINT message, WPARAM wParam, LPARAM lParam );
};    

class AboutDlg : public GWDialog
{
   public:
      LRESULT windowProc( UINT message, WPARAM wParam, LPARAM lParam );
      void onCommand( int id, HWND hwndCtl, UINT codeNotify );
};

class TedWindow : public GWToolWin
{
   private:
      HWND hComboBox;
      HWND hMainToolBar;
      GWMenu mainMenu;
      SimTed * simTed;
      DisplayOptionsDlg displayOptions;
      AboutDlg aboutDlg;

   public:
      TedWindow( SimTed * ted, GWWindow * parent );
      ~TedWindow();

      void setupMenu();
      void onInitMenuPopup( HMENU hMenu, UINT item, BOOL fSystemMenu );
      void onUpdate();
      void onSize( UINT state, int cx, int cy );
      void onCommand( int id, HWND hwndCtl, UINT codeNotify );
   	void onShowToolBar();
   	void onHideToolBar();
      void setupToolBar();
      void updateToolBar();
      bool createMaterialView();
      void showMaterialView( bool show = true );
      bool setMaterialView();
};

#endif
