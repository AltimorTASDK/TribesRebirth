//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PTDIALOG_H_
#define _PTDIALOG_H_

#include <core.h>
#include <gw.h>
#include <palMap.h>
#include <ptDefines.h>
#include <ptBuild.h>

#include <ptControl.h>
#include <ptAbout.h>
#include <ptShadeEdit.h>

class Paltool: public GWDialog
{
protected:
	typedef GWDialog Parent;

   //--------------------------------------
   // tab Dialogs
   VectorPtr<GWDialog*> tabDialog;
   GWDialog   *tabSelection;
   About       dlgAbout;
   ShadeEdit   dlgShadeEdit;

   //--------------------------------------
   // GFX stuff
   GFXSurface *surface;
   GFXPalette *palette;
   RECT surfaceRect;
   char namePAL[MAX_PATH];

   PaletteBuilder paletteBuilder;
   PaletteMap *map;
   DWORD state;

   void updateWindowTitle();
   void makeDefaultPalette();
   bool createSurface();
   bool isInsideSurface(int &x, int &y);
   void onOpenPalette();
   void onSavePalette(bool fSaveAs=false);
   void onOpenPaletteMap();
   void onSavePaletteMap(bool fSaveAs=false);
   void loadPalette(char *filename);
   void savePalette(char *filename);
   void loadPaletteMap(char *filename);
   void savePaletteMap(char *filename);

   void setCursor();
   void getPaletteParams();
   void setExtension(char *name, char *ext);

public:   
   //--------------------------------------
   // surface controls
   VectorPtr<PTControl*> control;
   SelectBar      selectBar;
   PaletteGrid    paletteGrid;
   PaletteShade   paletteShade;

   //--------------------------------------
   Paltool();
   ~Paltool();

   BOOL onInitDialog(HWND hwndFocus, LPARAM lParam);
   void init();
   void registerTab( GWDialog *dlg );
   DWORD getState() { return state; };
   void  setState(DWORD s);


   //--------------------------------------
   void onCommand(int id, HWND hwndCtl, UINT codeNotify);
   int  onNotify(int id, LPNMHDR pnmhdr);
   void render();
   void onPaint();
   void onDestroy();
   void onApply();
   void onGenerate();

	BOOL onSetCursor(HWND hwndCursor, UINT codeHitTest, UINT msg);
	void onMouseMove(int x, int y, UINT keyFlags);
	void onLButtonUp(int x, int y, UINT keyFlags);
	void onRButtonUp(int x, int y, UINT keyFlags);
	void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);

};   


inline void Paltool::onDestroy()
{
	PostQuitMessage(0);
}



#endif //_PTDIALOG_H_
