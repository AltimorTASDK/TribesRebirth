//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

#include <g_pal.h>
#include <g_bitmap.h>
#include <g_surfac.h>
#include "grdblock.h"

#include "gridWindow.h"

#define STR_EQ(s1,s2) (stricmp((s1),(s2))==0)
   

//------------------------------------------------------------------------------

GridWindow::GridWindow()
{
   if (! create("StampTool",WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_CLIPCHILDREN,  
                Point2I(CW_USEDEFAULT,0), Point2I(400,430),NULL,0))
   {
      AssertFatal(0,"GridWindow:: Could not create main window");
   }

	show( GWMain::cmdShow );
	update();

   inspector = NULL;
   palette = NULL;
   rsrcManager = NULL;
   volRStream = NULL;

   phMaterialList = NULL;
   stamp = NULL;
   clipBoard = NULL;

   selectedTile.x = 0;
   selectedTile.y = 0;

   origin.x = 0;
   origin.y = 0;

   numTiles = 10;
   stamp_name[0] = '\0';
   dml_name[0] = '\0';

   helpPath = NULL;
}   

//------------------------------------------------------------------------------

bool GridWindow::init(char *execPath)
{
   char *path;

   // set path to help file
   helpPath = new char[ strlen(execPath) + strlen("STAMPTOOL.HLP") + 1];
   if (!helpPath)
   {
      displayErrDialog("Could not alloc memory");
      return false;
   }
   strcpy(helpPath, execPath);
   path = strrchr(helpPath, '\\');
   if (path == NULL)
      helpPath[0] = '\0';
   else
      path[1] = '\0';
   strcat(helpPath, "STAMPTOOL.HLP");

   // set the gridWindow menus
   if (! gridMenu.create("IDM_MENU1") )
   {
      displayErrDialog("Could not create menu");
      return false;
   }
   if (! gridMenu.setMainMenu(getHandle()) )
   {
      displayErrDialog("Could not set main menu");
      return false;
   }

   // create new TerrainStamp
   onNewTSP();

   // open the inspector dialog box
   inspector = new InspectDlg(this);
   onMove(0,0);
   inspector->updateView();
   
   // create a resource manager and
   // add an empty volume to it
   rsrcManager = new ResourceManager;
   if (! rsrcManager)
   {
      displayErrDialog("Could not create resource manager");
      return false;
   }
   volRStream = rsrcManager->add(NULL, "");

   // setup the poly renderer
   GFXSurface *surface;
   surface = getSurface();
   if (! surface)
      return false;
   
   surface->setHazeSource(GFX_HAZE_NONE);
   surface->setShadeSource(GFX_SHADE_NONE);
   surface->setAlphaSource(GFX_ALPHA_NONE);
   surface->setFillMode(GFX_FILL_TEXTURE);
   surface->setTransparency(FALSE);
   surface->setTexturePerspective(FALSE);

   return true;
}

//------------------------------------------------------------------------------

void GridWindow::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{

   if (volRStream == NULL)
      displayErrDialog("VolRStream == NULL");

   switch (id)
   {
      case IDM_OPEN_DML:
         onOpenDML();
         break; 

      case IDM_OPEN_PAL:
         onOpenPalette(); 
         break; 

      case IDM_NEW_TS: 
         onNewTSP();
         reset();
         break; 

      case IDM_OPEN_TS: 
         onOpenTSP();
         break; 

      case IDM_SAVE_TS: 
         onSaveTSP(false);
         break; 

      case IDM_SAVE_AS_TS:
         onSaveTSP(true); 
         break; 

      case IDM_CUT:
         onCut(); 
         break; 

      case IDM_COPY:
         onCopy(); 
         break; 

      case IDM_PASTE:
         onPaste(); 
         break; 

      case IDM_EXIT:
   	   destroyWindow();
   		break;

      case IDM_HELP:
         WinHelp(getHandle(), helpPath, HELP_FORCEFILE, 0);
         break;

      case IDM_ABOUT:
         MessageBox(getHandle(), "StampTool\nCopyright (c) 1997 Dynamix\n\nby Brian Lee", "About StampTool...", MB_OK);
         break;

      default:
         Parent::onCommand(id, hwndCtl, codeNotify);
         break;
   }
   inspector->updateView();
}  

//------------------------------------------------------------------------------

void GridWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
   Point2I delta(0,0);

   if (fDown)
   {
      switch (vk)                
      {  
         case 'C':
            onCopy();
            break;

         case 'X':
            onCut();
            break;

         case 'V':
            onPaste();
            break;

         case 'O':
            origin = selectedTile;
            break;

         case 'F':
            setTileFlat(selectedTile, !isTileFlat(selectedTile));
            break;     

         case VK_NUMPAD4:
            delta.x--;            
            moveStamp(delta);
            break;
         
         case VK_NUMPAD6:
            delta.x++;    
            moveStamp(delta);
            break;

         case VK_NUMPAD8:
            delta.y--;            
            moveStamp(delta);
            break;
         
         case VK_NUMPAD2:
            delta.y++;    
            moveStamp(delta);
            break;
                 
         case VK_BACK:
         case VK_DELETE:
            deleteTile(selectedTile);
            break;

         case VK_LEFT:
            {
               TextureInfo *getTex, setTex;
               getTex = getTileTexture(selectedTile);
               if (getTex)
               {
                  setTex.textureID = getTex->textureID;
                  setTex.flags = getTex->flags;
                  setTex.tile = getTex->tile;
               }
               else
               {
                  setTex.textureID = -2;
                  setTex.flags = 0;
                  setTex.tile = selectedTile;   
               }
               setTex.textureID--;
               if (setTex.textureID < -2)
                  setTex.textureID = getDML_MaxTexId();
            
               setTileTexture(selectedTile, setTex);
            }
            break;
                      
         case VK_RIGHT:
            {
               TextureInfo *getTex, setTex;
               getTex = getTileTexture(selectedTile);
               if (getTex)
               {
                  setTex.textureID = getTex->textureID;
                  setTex.flags = getTex->flags;
                  setTex.tile = getTex->tile;
               }
               else
               {
                  setTex.textureID = -2;
                  setTex.flags = 0;
                  setTex.tile = selectedTile;   
               }
               setTex.textureID++;
               if (setTex.textureID > getDML_MaxTexId())
                  setTex.textureID = -2;
            
               setTileTexture(selectedTile, setTex);
            }
            break;
         
         case VK_UP:
            {
               TextureInfo *getTex, setTex;
               getTex = getTileTexture(selectedTile);
               if (!getTex)
                  break;

               if (getTex->textureID == -1)
                  break;

               setTex.textureID = getTex->textureID;
               setTex.flags = getTex->flags;
               setTex.tile = getTex->tile;
               
               setTex.flags++;
               if (setTex.flags > 7)
                  setTex.flags = 0;
            
               setTileTexture(selectedTile, setTex);
            }
            break;

         
         case VK_DOWN:
            {
               TextureInfo *getTex, setTex;
               getTex = getTileTexture(selectedTile);
               if (!getTex)
                  break;

               if (getTex->textureID == -1)
                  break;

               setTex.textureID = getTex->textureID;
               setTex.flags = getTex->flags;
               setTex.tile = getTex->tile;
               
               setTex.flags--;
               if (setTex.flags > 7)
                  setTex.flags = 7;
            
               setTileTexture(selectedTile, setTex);
            }
            break;

         case VK_ADD:
            numTiles-= 2;
            if (numTiles < 2)
               numTiles = 2;
            break;
         
         case VK_SUBTRACT:
            numTiles+= 2;
            break;

         default:
            Parent::onKey(vk, fDown, cRepeat, flags);
      }
   }
   selectedTile += delta;
   origin += delta;
   inspector->updateView();

   render();
}   

//------------------------------------------------------------------------------
// make the tile the mouse is over the currently selected tile
void GridWindow::onLButtonDown(BOOL, int x, int y, UINT)
{
   GFXSurface *surface = getSurface();

   // make the tile the mouse is over the currently selected tile
   selectedTile.x = (Int32)( ( (Flt32)x / (Flt32)surface->getWidth() ) * (Flt32) numTiles) - (numTiles / 2);
   selectedTile.y = (Int32)( ( (Flt32)y / (Flt32)surface->getHeight() ) * (Flt32) numTiles) - (numTiles / 2);

   inspector->updateView();
   render();
}

//------------------------------------------------------------------------------
// copy the  selected tile's attributes into the currently selected tile
void GridWindow::onRButtonDown(BOOL, int x, int y, UINT)
{
   GFXSurface *surface = getSurface();
   Point2I tile;
   TextureInfo *getTex, setTex;

   // make the tile the mouse is over the currently selected tile
   tile.x = (Int32)( ( (Flt32)x / (Flt32)surface->getWidth() ) * (Flt32) numTiles) - (numTiles / 2);
   tile.y = (Int32)( ( (Flt32)y / (Flt32)surface->getHeight() ) * (Flt32) numTiles) - (numTiles / 2);
   
   getTex = getTileTexture(selectedTile);
   if (getTex)
   {
      setTex.textureID = getTex->textureID;
      setTex.flags = getTex->flags;
      setTex.tile = tile;
   }
   else
   {
      setTex.textureID = -2;
   }

   setTileTexture(tile, setTex);
   setTileFlat(tile, isTileFlat(selectedTile));

   inspector->updateView();
   render();
}

//------------------------------------------------------------------------------

void GridWindow::onMove(int x, int y)
{
   RECT inspect_rect, main_rect;

   if (inspector) 
   {
      if (GetWindowRect(inspector->getHandle(), &inspect_rect))
      {
         if (GetWindowRect(getHandle(), &main_rect))
            MoveWindow(inspector->getHandle(),
               main_rect.right + 10,	// horizontal position
               main_rect.top,	// vertical position
               inspect_rect.right - inspect_rect.left,	// width
               inspect_rect.bottom - inspect_rect.top,	// Height
               true);
      }
   }
   Parent::onMove(x,y);
}

//------------------------------------------------------------------------------

void GridWindow::onOpenPalette()
{
	char file[MAX_PATH];
	OPENFILENAME ofn;
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = getHandle();
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Palette Files (*.ppl;*.pal;*.bmp;*.dib)\0*.ppl;*.pal;*.bmp;*.dib\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "ppl";

	if (GetOpenFileName(&ofn))
		loadPalette(file);
   render();
}

//------------------------------------------------------------------------------

void GridWindow::loadPalette(char *filename)
{
   delete palette;
   char *dot = strrchr(filename, '.');
   if (STR_EQ(dot, ".pal") || STR_EQ(dot, ".ppl"))
	   palette = GFXPalette::load(filename);
   else if (STR_EQ(dot, ".dib") || STR_EQ(dot, ".bmp"))
   {
      GFXBitmap *bmp = GFXBitmap::load(filename, BMF_INCLUDE_PALETTE);
      if (bmp) 
      {
         if (bmp->pPalette)   
         {
            palette = bmp->pPalette;
            bmp->pPalette = NULL;
            delete bmp;   
         }
         else
         {
	         displayErrDialog("Error bitmap file does not contain a palette\"%s\".", filename);
         }
      }
      else
      {
	      displayErrDialog("Error loading bitmap file \"%s\".", filename);
      }
   }

	if (palette)
	{
      getSurface()->setPalette(palette);
	}
}

//------------------------------------------------------------------------------

void GridWindow::onOpenDML()
{
	char file[MAX_PATH];

	OPENFILENAME ofn;
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = getHandle();
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "DML Files (*.dml)\0*.dml\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "dml";

	if (GetOpenFileName(&ofn))
		loadDML(file);
   render();
}

//------------------------------------------------------------------------------

void GridWindow::loadDML(char *filename)
{
   delete phMaterialList;

   char *name, *path;
   name = strrchr(filename,'\\');
   if (name == NULL)
   {
      name = filename;
      path = ".";
   }
   else
   {
      *name = '\0';
      name++;
      path = filename;
   }

   if (volRStream == NULL)
   {
      displayErrDialog("Error vol is null");
      return;
   }

   volRStream->open(path);

   phMaterialList = new Resource<TSMaterialList>;
   if (! phMaterialList)
   {
      displayErrDialog("Error alloc materal list.");   
   }

   (*phMaterialList) = rsrcManager->load(name, true);
   if (! (bool)(*phMaterialList))
   {
      displayErrDialog("Error loading dml file \"%s\".", name);
      return;
   }

	(*phMaterialList)->load(*rsrcManager, true);
   if ((*phMaterialList)->getMaterialsCount() == 0)
   {
      displayErrDialog("Error loading dml file \"%s\".", name);
   }
   strcpy(dml_name, name);
}

//------------------------------------------------------------------------------

void GridWindow::onNewTSP()
{
   delete stamp;
   stamp = new LSTerrainStamp;
}   

//------------------------------------------------------------------------------

void GridWindow::onOpenTSP()
{
	char file[MAX_PATH];

	OPENFILENAME ofn;
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = getHandle();
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Terrain Stamp Files (*.tsp)\0*.tsp\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "tsp";

	if (GetOpenFileName(&ofn))
		loadTSP(file);
   render();
}

//------------------------------------------------------------------------------

void GridWindow::loadTSP(char *filename)
{
   FileRStream frs;

   if (! frs.open(filename))
   {
      displayErrDialog("Could not open %s", filename);
      return;
   }

   if (stamp->read(frs,0,0))
   {
      displayErrDialog("Error in reading: %s", filename);
   }
   else
   {
      reset();
      strcpy(stamp_name, filename);
   }
   frs.close();

   setHoleTexID();
   // This tool's Y coord is in different direction as the
   // terrain system
   invertYCoord();
}

//------------------------------------------------------------------------------

void GridWindow::onSaveTSP(bool fSaveAs)
{
	char file[MAX_PATH];
	OPENFILENAME ofn;

   if (*stamp_name && !fSaveAs)
   {
      strcpy(file, stamp_name);
		saveTSP(file);
      return;
   }

	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Terrain Stamp Files (*.tsp)\0*.tsp;*.tsp\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "tsp";
	if (GetSaveFileName(&ofn))
		saveTSP(file);
}


//------------------------------------------------------------------------------

void GridWindow::saveTSP(char *filename)
{
   FileWStream fws;

   if (! fws.open(filename))
   {
      displayErrDialog("Could not save %s", filename);
      return;
   }

   // adjust the stamp to the origin before saving
   Point2I negOrigin(0,0);
   negOrigin -= origin;
   moveStamp(negOrigin);
   // invert Y, This tool's Y coord is in the wrong direction   
   invertYCoord();

   setHoleFlag();

   if (stamp->write(fws,0,0))
   {
      displayErrDialog("Error in writing: %s", filename);
   }
   else
   {
      strcpy(stamp_name, filename);
   }
   fws.close();
   setHoleTexID();

   // adjust the stamp back
   invertYCoord();
   moveStamp(origin);
}

//------------------------------------------------------------------------------
void GridWindow::setHoleFlag()
{
   Vector<LSTerrainStamp::TextureType>::iterator tex_iter;
   for (tex_iter = stamp->texList.begin(); tex_iter != stamp->texList.end(); tex_iter++)
   {
      if (tex_iter->textureID == -1)
      {
         tex_iter->flags |= GridBlock::Material::Empty;
         tex_iter->textureID = 0;
      }
   }
}   

//------------------------------------------------------------------------------
void GridWindow::setHoleTexID()
{
   Vector<LSTerrainStamp::TextureType>::iterator tex_iter;
   for (tex_iter = stamp->texList.begin(); tex_iter != stamp->texList.end(); tex_iter++)
   {
      if (tex_iter->flags & GridBlock::Material::Empty)
      {
         tex_iter->flags &= ~GridBlock::Material::Empty;
         tex_iter->textureID = -1;
      }
   }   
}   

//------------------------------------------------------------------------------

void GridWindow::onDestroy()
{
   inspector->destroyWindow();
   delete inspector;
   PostQuitMessage(0);  
   exit(0);
}

//------------------------------------------------------------------------------

void GridWindow::render()
{
   if (volRStream == NULL)
      return;  // window not yet fully initialized...

   GFXSurface* surface = getSurface();
   if (! surface)
      return;

   surface->lock();
   surface->clear(0);

   // draw textures if a DML was loaded
   drawTextures();

   // draw grid lines
   drawGrid();
   drawFlatTiles();
   drawOrigin();

   surface->unlock();
   surface->easyFlip();
}   

//------------------------------------------------------------------------------

void GridWindow::drawTextures()
{
   Vector<LSTerrainStamp::TextureType>::iterator tex_iter;
   GFXSurface *surface = getSurface();
   Int32 warned = 0;
   Int32 maxTexID;
   
   maxTexID = getDML_MaxTexId();

   Flt32 dx = (Flt32)surface->getWidth() / (Flt32)numTiles;
   Flt32 dy = (Flt32)surface->getHeight() / (Flt32)numTiles;

   for (tex_iter = stamp->texList.begin(); tex_iter != stamp->texList.end(); tex_iter++)
   {
      if (tileVisible(tex_iter->tile))
      {
         if (tex_iter->textureID > maxTexID)
         {
            if (! warned) 
            {
               displayErrDialog("The current DML has less textures then the previous DML.  Readjusting....");
               warned = 1;   
            }
            tex_iter->textureID = maxTexID;
         }

         Flt32 x = (Flt32)(tex_iter->tile.x + (numTiles / 2)) * dx;
         Flt32 y = (Flt32)(tex_iter->tile.y + (numTiles / 2)) * dy;

         if (tex_iter->textureID >= 0)
         {
            // draw the texture
            const GFXBitmap *bmp = ((*phMaterialList)->getMaterial(tex_iter->textureID)).getTextureMap();
            static Flt32 _u[8][6] = { {0,1,1,0,1,0}, 
                                      {0,0,1,0,1,1},
                                      {1,0,0,1,0,1},
                                      {1,1,0,1,0,0},
                                      {0,1,1,0,1,0},
                                      {0,0,1,0,1,1},
                                      {1,0,0,1,0,1},
                                      {1,1,0,1,0,0} };
            static Flt32 _v[8][6] = { {0,0,1,0,1,1}, 
                                      {1,0,0,1,0,1},
                                      {0,0,1,0,1,1},
                                      {1,0,0,1,0,1},
                                      {1,1,0,1,0,0},
                                      {0,1,1,0,1,0},
                                      {1,1,0,1,0,0},
                                      {0,1,1,0,1,0} };
            BYTE flag = tex_iter->flags;
            if (flag > 7)
            {
               displayErrDialog("Invalid flags were set!!");
               exit(1);   
            }
            else
            {
               surface->setTextureMap(bmp);
               surface->addVertex(&Point3F(  x    ,   y    , 1), &Point2F(_u[flag][0],_v[flag][0]), (DWORD)0);
               surface->addVertex(&Point3F(x + dx ,   y    , 1), &Point2F(_u[flag][1],_v[flag][1]), (DWORD)0);
               surface->addVertex(&Point3F(x + dx , y + dy , 1), &Point2F(_u[flag][2],_v[flag][2]), (DWORD)0);
               surface->emitPoly();
               surface->addVertex(&Point3F(  x    ,   y    , 1), &Point2F(_u[flag][3],_v[flag][3]), (DWORD)0);
               surface->addVertex(&Point3F(x + dx , y + dy , 1), &Point2F(_u[flag][4],_v[flag][4]), (DWORD)0);
               surface->addVertex(&Point3F(     x , y + dy , 1), &Point2F(_u[flag][5],_v[flag][5]), (DWORD)0); 
               surface->emitPoly();
            }
         }
         else 
         {
            if (tex_iter->textureID == -1)
            {
               // draw an X marking a hole
               Point2I snapStart((Int32)x, (Int32)y);
               Point2I snapEnd((Int32)(x + dx), (Int32)(y + dy));

               surface->drawLine2d(&snapStart, &snapEnd, 249);
            
               snapStart.x = (Int32) (x + dx);
               snapEnd.x   = (Int32) x;

               surface->drawLine2d(&snapStart, &snapEnd, 249);
            }
         }
      }
   }
}   

//------------------------------------------------------------------------------

void GridWindow::drawGrid()
{
   GFXSurface *surface = getSurface();
   Flt32 dx, dy;
   Point2F start, end;
   Int32 width, height;
   Point2I snapStart, snapEnd; // snap to Ints
   int i;

   width = surface->getWidth();
   height = surface->getHeight();

   dx = (Flt32)width / (Flt32)numTiles;
   dy = (Flt32)height / (Flt32)numTiles;
   
   // draw the horizontal lines of the grid
   start.x = 0;
   start.y = dy;
   end.x   = (Flt32)(width - 1);
   end.y   = dy;
   snapStart.x = (Int32) start.x;
   snapEnd.x = (Int32) end.x;

   for (i = 1; i < numTiles; i++)
   {
      snapStart.y = (Int32) start.y;
      snapEnd.y = (Int32) end.y;

      surface->drawLine2d(&snapStart, &snapEnd, 249);
      start.y += dy;
      end.y += dy;
   }

   // draw the vertical lines of the grid
   start.x = dx;
   start.y = 0;
   end.x   = dx;
   end.y   = (Flt32)(height - 1);
   snapStart.y = (Int32) start.y;
   snapEnd.y = (Int32) end.y;

   for (i = 1; i < numTiles; i++)
   {
      snapStart.x = (Int32) start.x;
      snapEnd.x = (Int32) end.x;

      surface->drawLine2d(&snapStart, &snapEnd, 249);
      start.x += dx;
      end.x += dx;
   }

   // draw the tile currently selected
   if (tileVisible(selectedTile))
   {
      static Flt32 x_start_offset[] = {0.0f, 0.0f, 0.0f, 1.0f };
      static Flt32 y_start_offset[] = {0.0f, 0.0f, 1.0f, 0.0f };
      static Flt32 x_end_offset[] = {1.0f, 0.0f, 1.0f, 1.0f };
      static Flt32 y_end_offset[] = {0.0f, 1.0f, 1.0f, 1.0f };

      start.x = (Flt32)(selectedTile.x + (numTiles / 2)) * dx;
      start.y = (Flt32)(selectedTile.y + (numTiles / 2)) * dy;
      end.x = start.x + dx;
      end.y = start.y + dy;

      for (i = 0; i < 4 ; i++)
      {
         snapStart.x = (Int32) start.x + dx * x_start_offset[i];
         snapStart.y = (Int32) start.y + dy * y_start_offset[i];
         snapEnd.x = (Int32) start.x + dx * x_end_offset[i];
         snapEnd.y = (Int32) start.y + dy * y_end_offset[i];

         surface->drawLine2d(&snapStart, &snapEnd, 255);
      }
   }
} 

//------------------------------------------------------------------------------

void GridWindow::drawFlatTiles()
{
   GFXSurface *surface = getSurface();
   Flt32 dx, dy;
   Point2F start;
   Point2I snapStart, snapEnd; // snap to Ints

   dx = (Flt32)surface->getWidth() / (Flt32)numTiles;
   dy = (Flt32)surface->getHeight() / (Flt32)numTiles;
   
   Vector<Point2I>::iterator flat_iter;
 
   for (flat_iter = stamp->flatList.begin(); flat_iter != stamp->flatList.end(); flat_iter++)
   {
      if (tileVisible(*flat_iter))
      {
         // draw an F on the tile
         start.x = (Flt32)((*flat_iter).x + (numTiles / 2)) * dx + dx * 0.2;
         start.y = (Flt32)((*flat_iter).y + (numTiles / 2)) * dy + dy * 0.2;

         snapStart.x = (Int32) start.x;
         snapStart.y = (Int32) start.y;
         snapEnd.x   = (Int32) (start.x + dx * 0.6);
         snapEnd.y   = (Int32) start.y;

         surface->drawLine2d(&snapStart, &snapEnd, 252);
         
         snapStart.x = (Int32) start.x;
         snapStart.y = (Int32) start.y;
         snapEnd.x   = (Int32) start.x;
         snapEnd.y   = (Int32) (start.y + dy * 0.6);

         surface->drawLine2d(&snapStart, &snapEnd, 252);

         snapStart.x = (Int32) start.x;
         snapStart.y = (Int32) (start.y + dy * 0.3);
         snapEnd.x   = (Int32) (start.x + dx * 0.6);
         snapEnd.y   = (Int32) (start.y + dy * 0.3);

         surface->drawLine2d(&snapStart, &snapEnd, 252);

      }  
   }
} 

//------------------------------------------------------------------------------

void GridWindow::drawOrigin()
{
   GFXSurface *surface = getSurface();
   Flt32 dx, dy, x, y;

   dx = (Flt32)surface->getWidth() / (Flt32)numTiles;
   dy = (Flt32)surface->getHeight() / (Flt32)numTiles;
 
   if (tileVisible(origin))
   {
      RectI rect;

      // draw an F on the tile
      x = (Flt32)(origin.x + (numTiles / 2)) * dx + dx * 0.2;
      y = (Flt32)(origin.y + (numTiles / 2)) * dy + dy * 0.2;

      rect.upperL.x = (Int32) x;
      rect.upperL.y = (Int32) y;
      rect.lowerR.x = (Int32) (x + dx * 0.6);
      rect.lowerR.y = (Int32) (y + dy * 0.6);
      surface->drawCircle2d(&rect,251);  
   }
} 

//------------------------------------------------------------------------------

bool GridWindow::tileVisible(Point2I &tile)
{
   Int32 half_num_tiles = numTiles / 2;
   if ( ((tile.x + half_num_tiles) < 0) || ((tile.x + half_num_tiles) >= numTiles) )
      return false;
   if ( ((tile.y + half_num_tiles) < 0) || ((tile.y + half_num_tiles) >= numTiles) )
      return false;

   return true;
}   

//------------------------------------------------------------------------------

void GridWindow::reset()
{
   selectedTile.x = 0;
   selectedTile.y = 0;

   origin.x = 0;
   origin.y = 0;

   numTiles = 10;
   stamp_name[0] = '\0';
}   

//------------------------------------------------------------------------------

TextureInfo *GridWindow::getTileTexture(Point2I &tile)
{
   Vector<LSTerrainStamp::TextureType>::iterator tex_iter;
   
   for (tex_iter = stamp->texList.begin(); tex_iter != stamp->texList.end(); tex_iter++)
   {
      if (tex_iter->tile == tile)
      {
         return &(*tex_iter);
      }
   }
   return NULL;
}

//------------------------------------------------------------------------------

void GridWindow::setTileTexture(Point2I &tile, TextureInfo &tile_info)
{
   Vector<LSTerrainStamp::TextureType>::iterator tex_iter;
   
   for (tex_iter = stamp->texList.begin(); tex_iter != stamp->texList.end(); tex_iter++)
   {
      if (tex_iter->tile == tile)
      {
         if (tile_info.textureID == -2)
         {
            stamp->texList.erase(tex_iter);
         }
         else
         {
            tex_iter->textureID = tile_info.textureID;
            tex_iter->flags = tile_info.flags;
         }
         return;
      }
   }
   if (tile_info.textureID != -2)
      stamp->texList.push_back(tile_info);
}

//------------------------------------------------------------------------------

bool GridWindow::isTileFlat(Point2I &tile)
{
   Vector<Point2I>::iterator flat_iter;
 
   for (flat_iter = stamp->flatList.begin(); flat_iter != stamp->flatList.end(); flat_iter++)
   {
      if (*flat_iter == tile)
         return true;
   }   

   return false;
}

//------------------------------------------------------------------------------

void GridWindow::setTileFlat(Point2I &tile, bool flat)
{  
   Vector<Point2I>::iterator flat_iter;
 
   for (flat_iter = stamp->flatList.begin(); flat_iter != stamp->flatList.end(); flat_iter++)
   {
      if (*flat_iter == tile)
      {
         if (! flat)
            stamp->flatList.erase(flat_iter);
         return;
      }
   }   
   if (flat)
      stamp->flatList.push_back(tile);
}

//------------------------------------------------------------------------------

void GridWindow::invertYCoord()
{
   Vector<Point2I>::iterator flat_iter;
   for (flat_iter = stamp->flatList.begin(); flat_iter != stamp->flatList.end(); flat_iter++)
   {
      flat_iter->y *= -1;
   }   

   Vector<LSTerrainStamp::TextureType>::iterator tex_iter;
   for (tex_iter = stamp->texList.begin(); tex_iter != stamp->texList.end(); tex_iter++)
   {
      tex_iter->tile.y *= -1;
   }
}   

//------------------------------------------------------------------------------

void GridWindow::moveStamp(Point2I &delta)
{
   Vector<Point2I>::iterator flat_iter;
   for (flat_iter = stamp->flatList.begin(); flat_iter != stamp->flatList.end(); flat_iter++)
   {
      *flat_iter += delta;
   }   

   Vector<LSTerrainStamp::TextureType>::iterator tex_iter;
   for (tex_iter = stamp->texList.begin(); tex_iter != stamp->texList.end(); tex_iter++)
   {
      tex_iter->tile += delta;
   }
}   

//------------------------------------------------------------------------------

void GridWindow::displayErrDialog(char *fmt, ...)
{
   char buffer[1024];

   va_list args;

   va_start(args, fmt);
   vsprintf(buffer, fmt, args);
   va_end(ap);
	MessageBox(getHandle(), buffer, NULL, MB_APPLMODAL);
}   


//------------------------------------------------------------------------------

void GridWindow::onCopy()
{
   TextureInfo *tex;

   if (! clipBoard)
      clipBoard = new LSTerrainStamp;

   clipBoard->flatList.clear();
   clipBoard->texList.clear();

   if (isTileFlat(selectedTile))
      clipBoard->flatList.push_back(selectedTile);
   tex = getTileTexture(selectedTile);
   if (tex)
      clipBoard->texList.push_back(*tex);
} 
  
//------------------------------------------------------------------------------

void GridWindow::onCut()
{
   onCopy();
   deleteTile(selectedTile);
} 

//------------------------------------------------------------------------------

void GridWindow::onPaste()
{
   if (! clipBoard)
      return;

   deleteTile(selectedTile);
   // copy the contents of the tile on
   // the selected tile - later, allow for pasting of multiple
   // tiles.
   
   Vector<Point2I>::iterator flat_iter;
 
   for (flat_iter = clipBoard->flatList.begin(); flat_iter != clipBoard->flatList.end(); flat_iter++)
   {
      setTileFlat(selectedTile, true); 
   }   

   Vector<LSTerrainStamp::TextureType>::iterator tex_iter;
   
   for (tex_iter = clipBoard->texList.begin(); tex_iter != clipBoard->texList.end(); tex_iter++)
   {
      tex_iter->tile = selectedTile;
      setTileTexture(selectedTile, *tex_iter);
   }
} 

