//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------


#include <ptCore.h>
#include <ptDialog.h>
#include <ptBuild.h>
#include <g_cds.h>
#include <g_pal.h>
#include <g_bitmap.h>

#define PALTOOL_TITLE "PalTool v2.0"
#define STR_EQ(s1,s2) (stricmp((s1),(s2))==0)

//------------------------------------------------------------------------------
Paltool::Paltool()
{
   state   = STATE_EDIT|STATE_SHADE;
   surface = NULL;
   tabSelection = NULL;
   map = new PaletteMap;
   makeDefaultPalette();
   namePAL[0] = '\0';

   create(IDD_PALTOOL, (HWND)NULL);
   updateWindowTitle();
}


//------------------------------------------------------------------------------
Paltool::~Paltool()
{
   delete palette; palette = NULL;
   delete map;     map     = NULL;
   delete surface; surface = NULL;
}   


//------------------------------------------------------------------------------
void Paltool::makeDefaultPalette()
{
   palette = new GFXPalette();

   for (int i=10; i< 246; i++)
   {
      palette->palette[0].color[i].peRed   = i;
      palette->palette[0].color[i].peGreen = i;
      palette->palette[0].color[i].peBlue  = i;
   }
}   

//------------------------------------------------------------------------------
void Paltool::init()
{
   //--------------------------------------
   // create surface and
   // register surface controls
   createSurface();
   surface->setPalette(palette);

   control.push_back( &selectBar );
   control.push_back( &paletteGrid );
   control.push_back( &paletteShade );

   //--------------------------------------
   // register TAB Dialogs
   dlgAbout.open( GetDlgItem(getHandle(), IDC_TABCONTROL), this );
   registerTab( &dlgAbout );

   dlgShadeEdit.open( GetDlgItem(getHandle(), IDC_TABCONTROL), this );
   registerTab( &dlgShadeEdit );

   paletteBuilder.setControl( GetDlgItem(dlgShadeEdit.getHandle(), IDC_PROGRESS));
   
}


//------------------------------------------------------------------------------
bool Paltool::createSurface()
{
   HWND hWnd = GetDlgItem(getHandle(), IDC_SURFACE);
   GetWindowRect(hWnd, &surfaceRect); 
   surfaceRect.right  -= surfaceRect.left+6;
   surfaceRect.bottom -= surfaceRect.top+6;
   ScreenToClient(getHandle(), (POINT*)&surfaceRect);
   surfaceRect.top  += 3;
   surfaceRect.left += 3;
   GFXCDSSurface::create( surface,              // pointer to surface ptr
                          true,                 // is primary surface
                          surfaceRect.right,    // width
                          surfaceRect.bottom,   // height
                          getHandle(),          // HWND to surface
                          &Point2I(surfaceRect.left,
                           surfaceRect.top),    // offset in window
                          8);                   // bits per pixel
   return (surface != NULL);
}


//------------------------------------------------------------------------------
void Paltool::registerTab( GWDialog *dlg)
{
   char title[256];
   TC_ITEM tie;
   
   if (tabSelection) tabSelection->show( SW_HIDE );
   
   HWND hWnd = GetDlgItem(getHandle(), IDC_TABCONTROL);
   GetWindowText(dlg->getHandle(), title, 254);

   tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
   tie.iImage  = -1;
   tie.pszText = title; //strnew(title);
   tie.lParam  = (long)dlg;

   TabCtrl_InsertItem(hWnd, 0, &tie);
   TabCtrl_SetCurSel (hWnd, 0);

   tabDialog.push_front( dlg );
   tabSelection = dlg;
   dlg->show( SW_SHOW );
   InvalidateRect(hWnd, NULL, TRUE);
   SetFocus(hWnd);
}



//------------------------------------------------------------------------------
BOOL Paltool::onInitDialog(HWND hwndFocus, LPARAM lParam)
{
   SetMenu( getHandle(), LoadMenu( GWWindow::getHInstance(), MAKEINTRESOURCE(IDM_MENU) ) );

   return Parent::onInitDialog(hwndFocus, lParam);
}


//------------------------------------------------------------------------------
bool Paltool::isInsideSurface(int &x, int &y)
{
   if (surface)
      if (x >= surfaceRect.left &&
          x < (surfaceRect.left+surfaceRect.right) &&
          y >= surfaceRect.top &&
          y < (surfaceRect.top+surfaceRect.bottom) )
      {
         x -= surfaceRect.left;
         y -= surfaceRect.top;  
         return (true);
      }
   return (false);
}


//------------------------------------------------------------------------------
int
Paltool::onNotify(int id, LPNMHDR pnmhdr)
{
   HWND hWnd = GetDlgItem(getHandle(), IDC_TABCONTROL);
   switch (pnmhdr->code)
   {
      case TCN_SELCHANGE:{
         int index = TabCtrl_GetCurSel(hWnd);
         tabDialog[index]->show( SW_SHOW );
         tabSelection = tabDialog[index];
         InvalidateRect(hWnd, NULL, TRUE);
         }
         return true;

      case TCN_SELCHANGING:
         tabDialog[TabCtrl_GetCurSel(hWnd)]->show( SW_HIDE );
         return false;

      case TCN_KEYDOWN: {
         TC_KEYDOWN *tck = (TC_KEYDOWN*)pnmhdr;
         int index = TabCtrl_GetCurSel(hWnd);
         tabDialog[index]->onKey(tck->wVKey, true, 0, tck->flags);
         setCursor();
         }
         return true;
   }
   return GWWindow::onNotify(id, pnmhdr);
}




//------------------------------------------------------------------------------
void Paltool::onCommand(int id, HWND hwndCtl, UINT codeNotify)
{
   switch (id) 
   {
      case IDM_OPEN:
         onOpenPalette();
         break;

      case IDM_SAVE:
      case IDM_SAVE_AS:
         onSavePalette(id == IDM_SAVE_AS);
         break;

      case IDM_OPEN_MAP:
         onOpenPaletteMap();
         break;

      case IDM_SAVE_MAP:
      case IDM_SAVE_AS_MAP:
         onSavePaletteMap(id == IDM_SAVE_AS_MAP);
         break;

      case IDYES:
      case IDNO:
      case IDOK:
      case IDCANCEL:
      case IDM_EXIT:
   	   destroyWindow();
   		break;

      default:
         Parent::onCommand(id, hwndCtl, codeNotify);
         break;
   }
}


//------------------------------------------------------------------------------
void Paltool::render()
{
   if (surface) 
   {
      surface->lock();
      surface->clear(0);

      //--------------------------------------      
      // update controls
      paletteShade.setSelect( selectBar.getSelect() );
      paletteShade.setSelectColor( paletteGrid.getAnchor() );
      paletteGrid.setOver( paletteShade.getOver() );

      if (state & STATE_EDIT)
      {
         //-------------------------------------- 
         bool enable;
         switch (state & STATE_MASK)
         {
            case STATE_HAZE:     
            case STATE_SHADE:    
               enable = paletteGrid.rangeDefined(); break;
            case STATE_HAZE_TO:  
            case STATE_RGB_MATCH:  
               enable = paletteGrid.selection.from.test(); break;
         }
         Button_Enable(GetDlgItem(dlgShadeEdit.getHandle(), IDC_APPLY), enable);
      }
      else
      {
         HWND hWnd = GetDlgItem(dlgShadeEdit.getHandle(), IDC_APPLY);
         Button_Enable(hWnd, false);

         int index = paletteGrid.getOver();
         if (index != -1)
         {
            AssertFatal(index < 256, "out of range");
            paletteGrid.zeroSelection();
            switch (state & STATE_MASK)
            {
               case STATE_HAZE:     paletteGrid.setToSelection(map->haze.inclusion[index]); break;
               case STATE_SHADE:    paletteGrid.setToSelection(map->shade.inclusion[index]); break;
               case STATE_HAZE_TO:     paletteGrid.selection.from.set(palette->GetNearestColor(map->hazeR, map->hazeG, map->hazeB )); break;
               case STATE_RGB_MATCH:   paletteGrid.selection.from = palette->allowedColorMatches; break;
            }
         }
      }            

      //--------------------------------------      
      // draw all controls
      VectorPtr<PTControl*>::iterator i = control.begin();
      for (; i != control.end(); i++)
         (*i)->render(surface);


      //--------------------------------------
      surface->unlock();
      surface->easyFlip();
   }
}


//------------------------------------------------------------------------------
void Paltool::onPaint()
{
   render();
   Parent::onPaint();
}


//------------------------------------------------------------------------------
void Paltool::setState(DWORD s)
{
   state = s;   
   paletteGrid.zeroSelection();
}  

 
//------------------------------------------------------------------------------
void Paltool::setCursor()
{
   if (state == STATE_RENDERING)
      SetCursor(LoadCursor(NULL,MAKEINTRESOURCE(IDC_WAIT))); 
   else if (state & STATE_EDIT)
      SetCursor(LoadCursor(NULL,MAKEINTRESOURCE(IDC_ARROW)));
   else
      SetCursor(LoadCursor(NULL,MAKEINTRESOURCE(IDC_CROSS)));
}   

//------------------------------------------------------------------------------
BOOL Paltool::onSetCursor(HWND hwndCursor, UINT codeHitTest, UINT msg)
{
   if (hwndCursor == getHandle())      
   {
      setCursor();
      return (true); // signal I've handled it
   }
   else
      return (Parent::onSetCursor(hwndCursor, codeHitTest, msg));
}   


//------------------------------------------------------------------------------
void Paltool::onApply()
{
   if (state == (STATE_EDIT|STATE_HAZE))
   {
      for (int i=0; i<256; i++)
         if (paletteGrid.selection.from.test(i))
            map->haze.inclusion[i] = paletteGrid.selection.from;
   }
   else if (state == (STATE_EDIT|STATE_SHADE))
   {
      for (int i=0; i<256; i++)
         if (paletteGrid.selection.from.test(i))
            map->shade.inclusion[i] = paletteGrid.selection.from;
   }
   else if (state == (STATE_EDIT|STATE_HAZE_TO))
   {
      for (int i=0; i<256; i++)
         if (paletteGrid.selection.from.test(i))
         {
            map->hazeR = palette->palette[0].color[i].peRed;
            map->hazeG = palette->palette[0].color[i].peGreen;
            map->hazeB = palette->palette[0].color[i].peBlue;
            break;
         }
   }
   else if (state == (STATE_EDIT|STATE_RGB_MATCH))
   {
      palette->allowedColorMatches = paletteGrid.selection.from;
   }
   paletteGrid.zeroSelection();
}   


//------------------------------------------------------------------------------
void Paltool::onGenerate()
{
   DWORD oldState = state;
   state = STATE_RENDERING;
   setCursor();

   getPaletteParams();
   paletteBuilder.build(palette, map);

   state = oldState;
   setCursor();
   if (surface)
      surface->setPalette( palette );
}   


//------------------------------------------------------------------------------
void Paltool::getPaletteParams()
{
   char buff[100];
   GetDlgItemText(dlgShadeEdit.getHandle(), IDC_GAMMA, buff, 99);  sscanf(buff, "%f", &map->gamma);         
   GetDlgItemText(dlgShadeEdit.getHandle(), IDC_HAZE_MAX, buff, 99);  sscanf(buff, "%f", &map->haze.max);         
   GetDlgItemText(dlgShadeEdit.getHandle(), IDC_SHADE_MAX, buff, 99);  sscanf(buff, "%f", &map->shade.max);         
   GetDlgItemText(dlgShadeEdit.getHandle(), IDC_HAZE_LEVELS, buff, 99); sscanf(buff, "%d", &map->haze.levels);
   GetDlgItemText(dlgShadeEdit.getHandle(), IDC_SHADE_LEVELS, buff, 99); sscanf(buff, "%d", &map->shade.levels);
   map->useLUV = (Button_GetCheck(GetDlgItem(dlgShadeEdit.getHandle(), IDC_USE_LUV)) == BST_CHECKED);

   map->haze.max  /= 100.0f;
   map->shade.max /= 100.0f;
}   


//------------------------------------------------------------------------------
void Paltool::onMouseMove(int x, int y, UINT keyFlags)
{
   Parent::onMouseMove( x, y, keyFlags);
   if (isInsideSurface(x,y))
   {
      VectorPtr<PTControl*>::iterator i = control.begin();
      for (; i != control.end(); i++)
         (*i)->onMouseMove(x, y, keyFlags);

      // make sure top tab has focus
      HWND hWnd = GetDlgItem(getHandle(), IDC_TABCONTROL);
      SetFocus(hWnd);

      // update RGB display
      int index = paletteGrid.getOver();
      if (index == -1 ) index = paletteShade.getOver();
      if (index != -1 && palette)
         SetDlgItemText(dlgShadeEdit.getHandle(), IDC_RGB,
            avar("%3d (%3d,%3d,%3d)",  index,
            palette->palette[0].color[index].peRed,
            palette->palette[0].color[index].peGreen,
            palette->palette[0].color[index].peBlue ));
      else
         SetDlgItemText(dlgShadeEdit.getHandle(), IDC_RGB, "??? (---,---,---)"); 
   }

   render();
}


//------------------------------------------------------------------------------
void Paltool::onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   Parent::onLButtonDown(fDoubleClick, x, y, keyFlags);
   if (isInsideSurface(x,y))
   {
      VectorPtr<PTControl*>::iterator i = control.begin();
      for (; i != control.end(); i++)
         (*i)->onLButtonDown(fDoubleClick, x, y, keyFlags);
   }
   render();
}


//------------------------------------------------------------------------------
void Paltool::onRButtonUp(int x, int y, UINT keyFlags)
{
   Parent::onRButtonUp( x, y, keyFlags);
   if (isInsideSurface(x,y))
   {
      VectorPtr<PTControl*>::iterator i = control.begin();
      for (; i != control.end(); i++)
         (*i)->onRButtonUp(x, y, keyFlags);
   }
   render();
}


//------------------------------------------------------------------------------
void Paltool::onLButtonUp(int x, int y, UINT keyFlags)
{
   Parent::onLButtonUp( x, y, keyFlags);
   if (isInsideSurface(x,y))
   {
      VectorPtr<PTControl*>::iterator i = control.begin();
      for (; i != control.end(); i++)
         (*i)->onLButtonUp(x, y, keyFlags);
   }
   render();
}


//------------------------------------------------------------------------------
void Paltool::onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   Parent::onRButtonDown(fDoubleClick, x, y, keyFlags);
   if (isInsideSurface(x,y))
   {
      VectorPtr<PTControl*>::iterator i = control.begin();
      for (; i != control.end(); i++)
         (*i)->onRButtonDown(fDoubleClick, x, y, keyFlags);
   }
   render();
}

void Paltool::updateWindowTitle()
{
   SetWindowText(getHandle(), avar("%s     %s", PALTOOL_TITLE, namePAL));
}   


//------------------------------------------------------------------------------
void Paltool::setExtension(char *filename, char *ext)
{
   char *dot = strrchr(filename, '.');
   if (!dot) strcat(filename, ext);
   else strcpy(dot, ext);
}   


//------------------------------------------------------------------------------
void Paltool::onOpenPalette()
{
	char file[1024];
	OPENFILENAME ofn;
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
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
}


//------------------------------------------------------------------------------
void Paltool::loadPalette(char *filename)
{
   char buffer[1024];
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
	         sprintf(buffer, "Error bitmap file does not contain a palette\"%s\".", filename);
	         MessageBox(getHandle(), buffer, NULL, MB_APPLMODAL);
         }
      }
      else
      {
	      sprintf(buffer, "Error loading bitmap file \"%s\".", filename);
	      MessageBox(getHandle(), buffer, NULL, MB_APPLMODAL);
      }
   }

	if (palette)
	{
      setExtension(filename, ".ppl");
      strcpy(namePAL, filename);

      loadPaletteMap( filename );

		if (surface) surface->setPalette(palette);
      InvalidateRect(getHandle(), NULL, TRUE);
	}
	else 
	{
      namePAL[0]= '\0';
      makeDefaultPalette();
		if (surface) surface->setPalette(palette);
		sprintf(buffer, "Error loading the palette \"%s\".", filename);
	   MessageBox(getHandle(), buffer, NULL, MB_APPLMODAL);
	}
   updateWindowTitle();
}


//------------------------------------------------------------------------------
void Paltool::onSavePalette(bool fSaveAs)
{
	char file[1024];
	OPENFILENAME ofn;

   if (*namePAL && !fSaveAs)
   {
      strcpy(file, namePAL);
		savePalette(file);
      return;
   }

	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Palette Files (*.ppl;*.pal;*.bmp;*.dib)\0*.ppl;*.pal;*.bmp;*.dib\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "ppl";
	if (GetSaveFileName(&ofn))
		savePalette(file);
}


//------------------------------------------------------------------------------
void Paltool::savePalette(char *filename)
{
	char buffer[1024];
   setExtension(filename, ".ppl");

   if (palette->write( filename ))
   {
      strcpy(namePAL, filename);
      savePaletteMap( filename );
	}
   else
   {
	   sprintf(buffer, "Error saving the palette \"%s\".", filename);
	   MessageBox(getHandle(), buffer, NULL, MB_APPLMODAL);
   }
   updateWindowTitle();
}


//------------------------------------------------------------------------------
void Paltool::onOpenPaletteMap()
{
	char file[1024];
	OPENFILENAME ofn;
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "PaletteMap Files (*.pmp)\0*.pmp\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "pmp";
	if (GetOpenFileName(&ofn))
		loadPaletteMap(file);
}


//------------------------------------------------------------------------------
void Paltool::onSavePaletteMap(bool fSaveAs)
{
	char file[1024];
	OPENFILENAME ofn;

   if (*namePAL && !fSaveAs)
   {
      strcpy(file, namePAL);
		savePaletteMap(file);
      return;
   }

	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "PaletteMap Files (*.pmp)\0*.pmp\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "pmp";
	if (GetSaveFileName(&ofn))
		savePaletteMap(file);
}




//------------------------------------------------------------------------------
void Paltool::loadPaletteMap(char *filename)
{
	char buffer[1024];
   setExtension(filename, ".pmp");

   delete map;
   Persistent::Base::Error err;
   map = (PaletteMap*)Persistent::Base::fileLoad(filename, &err);
   if (!map)
   {
      map = new PaletteMap();
		sprintf(buffer, "Unable to load paletteMap \"%s\".", filename);
	   MessageBox(getHandle(), buffer, NULL, MB_APPLMODAL);
	}

   SetDlgItemText(dlgShadeEdit.getHandle(), IDC_GAMMA, avar("%f", map->gamma));         
   SetDlgItemText(dlgShadeEdit.getHandle(), IDC_HAZE_MAX, avar("%f", map->haze.max*100.0));         
   SetDlgItemText(dlgShadeEdit.getHandle(), IDC_SHADE_MAX, avar("%f", map->shade.max*100.0));         
   SetDlgItemText(dlgShadeEdit.getHandle(), IDC_HAZE_LEVELS, avar("%d", map->haze.levels));
   SetDlgItemText(dlgShadeEdit.getHandle(), IDC_SHADE_LEVELS, avar("%d", map->shade.levels));
   Button_SetCheck(GetDlgItem(dlgShadeEdit.getHandle(), IDC_USE_LUV), map->useLUV ? BST_CHECKED : BST_UNCHECKED);
}   


//------------------------------------------------------------------------------
void Paltool::savePaletteMap(char *filename)
{
	char buffer[1024];
   setExtension(filename, ".pmp");

   if (map->fileStore(filename) != Persistent::Base::Ok)
   {
		sprintf(buffer, "Error saving the paletteMap \"%s\".", filename);
	   MessageBox(getHandle(), buffer, NULL, MB_APPLMODAL);
	}
}   


