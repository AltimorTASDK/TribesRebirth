//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GRIDWINDOW_H_
#define _GRIDWINDOW_H_

#include <gw.h>
#include <resManager.h>
#include <ts_material.h>
#include <string.h>

#include "LSTerrainStamp.h"
#include "inspectDlg.h"

class InspectDlg;

class GridWindow : public GWCanvas
{
private:
   typedef GWCanvas Parent;
   
   GWMenu          gridMenu;
   InspectDlg      *inspector;

   GFXPalette      *palette;
   ResourceManager *rsrcManager;
   VolumeListEntry *volRStream;

private:
   char *helpPath;

   Resource<TSMaterialList> *phMaterialList;
   LSTerrainStamp *stamp;
   LSTerrainStamp *clipBoard; // for cut, paste, copy

   Point2I selectedTile;   
   Int32 numTiles;      // num of tile in lenght and width of display

   Point2I origin;
   char stamp_name[MAX_PATH];
   char dml_name[MAX_PATH];

public:
   GridWindow();
   bool init(char *execPath);

   // windows functions
	void onCommand(int id, HWND hwndCtl, UINT codeNotify);
   void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
   void onLButtonDown(BOOL, int x, int y, UINT);
   void onRButtonDown(BOOL b, int x, int y, UINT k);
   void onMove(int x, int y);
   void onOpenPalette();
   void loadPalette(char *filename);
   void onOpenDML();
   void loadDML(char *filename);
   void onNewTSP();
   void onOpenTSP();
   void loadTSP(char *filename);
   void onSaveTSP(bool fSaveAs);
   void saveTSP(char *filename);
	void onDestroy();

   // clipBoard functions
   void onCut();
   void onCopy();
   void onPaste();
   
   // functions for displaying the stamp
   void render();
   void drawTextures();
   void drawGrid();
   void drawFlatTiles();
   void drawOrigin();
   bool tileVisible(Point2I &tile);

   // functions for updating guts of terrain stamp 
   void reset();

   Point2I *getSelectedTile();
   char *getDML_Name();
   int  getDML_MaxTexId();
   char *getStampName();
   void deleteTile(Point2I &tile);

   int  getStampClampValue();
   void setStampClampValue(int clamp);

   bool isTileFlat(Point2I &tile);
   void setTileFlat(Point2I &tile, bool flat);

   TextureInfo *getTileTexture(Point2I &tile);
   void setTileTexture(Point2I &tile, TextureInfo &tex_info);
   void invertYCoord();    // oops, I drew the stamp with Y positive toward the bottom
                           // of the window, Darkstar, has Y positive up the window
   void setHoleFlag();
   void setHoleTexID();

   void moveStamp(Point2I &delta);

   void displayErrDialog(char *fmt, ...);
};

//--------------------------------------

inline Point2I *GridWindow::getSelectedTile()
{
   return &selectedTile;
}

//--------------------------------------

inline char *GridWindow::getDML_Name()
{
   if (dml_name[0] =='\0')
      return "None Specified";
   
   return dml_name;
}   

//--------------------------------------

inline int GridWindow::getDML_MaxTexId()
{
   if (phMaterialList == NULL)
      return -1;

   return ((*phMaterialList)->getMaterialsCount() - 1);
}   

//--------------------------------------

inline char *GridWindow::getStampName()
{
   char *name;

   if (stamp_name[0] =='\0')
      return "Untitled";

   name = strrchr(stamp_name, '\\');
   
   if (name)
      name++;
   else
      name = stamp_name;

   return name;
} 

//--------------------------------------

inline void GridWindow::deleteTile(Point2I &tile)
{
   TextureInfo tex;
   
   tex.textureID = -2;
   setTileTexture(tile, tex);
   setTileFlat(tile, false);   
}

//--------------------------------------

inline int GridWindow::getStampClampValue()
{
   return stamp->clamp_max_detail;
} 

inline void GridWindow::setStampClampValue(int clamp)
{
   stamp->clamp_max_detail = clamp;
}

#endif //_GRIDWINDOW_H_
