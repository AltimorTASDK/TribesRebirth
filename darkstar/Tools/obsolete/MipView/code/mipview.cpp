//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#include <stdio.h>

#include <gw.h>
#include <commdlg.h>
#include <g_bitmap.h>
#include <g_surfac.h>
#include <g_pal.h>
#include "wintools.h"
#include "defines.h"

#define GFX   true
#define PALETTE "palette.ppl"    
#define BITMAP  "out.bmp"

#define MESSAGE_AREA_HEIGHT 130
#define WINDOW_HORIZ_PAD    50
#define WINDOW_VERT_PAD     50

//NOTE: if you do no load a palette DirectDraw will appear BLACK!

//----------------------------------------------------------------------------

class BrowserWindow: public GWCanvas
{
	typedef GWCanvas Parent;

  private:
	HMENU       hMenu;
	int         windowWidth, windowHeight;
	int         currentDetailLevel;
	Bool        stretch;
	GFXPalette *palette;
	GFXBitmap  *bitmap, *drawingBitmap;	
	char        bitmapFilename[512];
		
  public:
    BrowserWindow(char *cmdLine);
    ~BrowserWindow();

	void AtSay(int x, int y, char *s);
	void AtSay(int x, int y, int   v);
	void okMessage(char *s);
	void updateMessageArea();

	void render();
	void onDestroy();
	void unlock();
    void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
	void onMouseMove(int x, int y, UINT keyFlags);
	void onCommand(int id, HWND hwndCtl, UINT codeNotify);
	void BrowserWindow::incrementDetailLevel(int amount);
	void BrowserWindow::toggleStretch();
	void openBitmap();
	void openPalette();
	void loadBitmap(char *filename);
	void loadPalette(char *filename);
	void setTitle();
};

BrowserWindow::BrowserWindow(char *cmdLine)
{
	bitmap             = NULL;
	palette            = NULL;
	currentDetailLevel = 0;
	stretch            = FALSE;
	drawingBitmap      = new GFXBitmap();
	windowWidth        = 300;
	windowHeight       = 300;

    if (!create("",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		Point2I(CW_USEDEFAULT,0),
		Point2I(windowWidth, windowHeight),
		NULL,0)) 
	{
		AssertFatal(0,"BrowserWindow:: Could not create main window");
		exit(1);
	}
	hMenu = LoadMenu(GWMain::hInstance, "MainMenu");
    SetMenu( hWnd, hMenu );
	show( GWMain::cmdShow );
	update();

	char bitmapFilename[256], paletteFilename[256];
	int argCt = sscanf(cmdLine, "%s %s", paletteFilename, bitmapFilename);

	if (argCt > 0)
		loadPalette(paletteFilename);
	if (argCt > 1)
		loadBitmap(bitmapFilename);
	setTitle();
	paint();
}


BrowserWindow::~BrowserWindow()
{
}

void BrowserWindow::onDestroy()
{
	PostQuitMessage(0);
}

void BrowserWindow::okMessage(char *s)
{
	MessageBox(hWnd, s, NULL, MB_APPLMODAL);
}

void BrowserWindow::AtSay(int x, int y, char *s)
{
	TextOut(getSurface()->getDC(), x, y, s, strlen(s));
}

void BrowserWindow::AtSay(int x, int y, int v)
{
	char buffer[64];
	sprintf(buffer, "%d", v);
	AtSay(x, y, buffer);
}

void BrowserWindow::updateMessageArea()
{
	if (!bitmap)
		return;
	if (stretch && (currentDetailLevel > 0))
		AtSay(220, windowHeight - 110, "(Stretched)");
	AtSay(20, windowHeight - 130, "Detail Level");
	AtSay(20, windowHeight - 110, "Scale");
	AtSay(20, windowHeight -  90, "Width");
	AtSay(20, windowHeight -  70, "Height");

	char  buffer[32];
	sprintf(buffer, "%d of %d", currentDetailLevel + 1, bitmap->detailLevels);
	AtSay(120, windowHeight - 130, buffer);

	float scale = 100.0/pow(2, currentDetailLevel);
	sprintf(buffer, "%5.1f\%", scale);
 	AtSay(120, windowHeight - 110, buffer);
	AtSay(120, windowHeight -  90, drawingBitmap->width);
	AtSay(120, windowHeight - 70, drawingBitmap->height);
}

void BrowserWindow::unlock()
{
  	GWCanvas::unlock();
	getSurface()->lockDC();
	updateMessageArea();
	getSurface()->unlockDC();
}

void BrowserWindow::render()
{
	if (!bitmap)
		return;

	getSurface()->clear(0xff);
	drawingBitmap->width        = bitmap->width  >> currentDetailLevel;
	drawingBitmap->height       = bitmap->height >> currentDetailLevel;
	drawingBitmap->stride       = drawingBitmap->width;
	drawingBitmap->bitDepth     = bitmap->bitDepth;
	drawingBitmap->imageSize    = drawingBitmap->width*drawingBitmap->height;
	drawingBitmap->detailLevels = 1;
	drawingBitmap->attribute    = bitmap->attribute;
	drawingBitmap->pBits        = bitmap->pMipBits[currentDetailLevel];

	int posx, posy;
	if (stretch)
	{
		posx = windowWidth/2 - bitmap->width/2;
		posy = (windowHeight - MESSAGE_AREA_HEIGHT)/2 - bitmap->height/2;
		getSurface()->drawBitmap2d(drawingBitmap, 
		                       &Point2I(posx, posy),
		                       &Point2I(bitmap->width, bitmap->height));
	}
	else
	{
		posx = windowWidth/2 - drawingBitmap->width/2;
		posy = (windowHeight - MESSAGE_AREA_HEIGHT)/2 - drawingBitmap->height/2;
		getSurface()->drawBitmap2d(drawingBitmap, &Point2I(posx, posy));
	}
}

void BrowserWindow::onMouseMove(int, int, UINT)
{
}

void BrowserWindow::toggleStretch()
{
	stretch = !stretch;
	if (stretch)
	    CheckMenuItem(hMenu, IDM_STRETCH, MF_CHECKED);
	else
	    CheckMenuItem(hMenu, IDM_STRETCH, MF_UNCHECKED);
	paint();
}

void BrowserWindow::incrementDetailLevel(int amount)
{
	currentDetailLevel += amount;
	if (currentDetailLevel < 0)
		currentDetailLevel = 0;
	else if (currentDetailLevel == bitmap->detailLevels)
		currentDetailLevel = bitmap->detailLevels - 1;
	paint();
}
		
void BrowserWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if (!fDown)
	{
		switch (vk)                //adjust Direct Draw screen resolution
		{                          //ignored if in DIB section
		case VK_UP:
			stretch = !stretch;
			paint();
			return;

		case VK_LEFT:
			incrementDetailLevel(-1);
			return;

		case VK_RIGHT:
			incrementDetailLevel(1);
			return;
      }
   }
   Parent::onKey(vk, fDown, cRepeat, flags);
}   

void BrowserWindow::loadBitmap(char *filename)
{
	GFXBitmap *newBitmap = GFXBitmap::load(filename, BMF_INCLUDE_PALETTE);
	if (newBitmap)
	{
		strcpy(bitmapFilename, filename);
		currentDetailLevel = 0;
		if (bitmap)
			delete bitmap;
		bitmap = newBitmap;
		if (bitmap->pPalette)
		{
			if (palette)
				delete palette;
			palette = bitmap->pPalette;
			bitmap->pPalette = NULL;       // so it doesn't go away w/bitmap
			getSurface()->setPalette(palette);
		}
		windowWidth  = max(300, int(bitmap->width + WINDOW_HORIZ_PAD));
		windowHeight = max(300, int(bitmap->height + MESSAGE_AREA_HEIGHT + WINDOW_VERT_PAD));
		setClientSize(Point2I(windowWidth, windowHeight));
		setTitle();
	}
	else 
	{
		char buffer[1024];
		sprintf(buffer, "Error loading the bitmap \"%s\".", filename);
		okMessage(buffer);
	}
	paint();
}	

void BrowserWindow::openBitmap()
{
	char file[1024];
   
	OPENFILENAME ofn;
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner = hWnd;
   
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "Bitmap Files\0*.bmp;*.dib\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = file;
	ofn.nMaxFile = sizeof(file);
	file[0] = 0;

	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt = "bmp";

	if (GetOpenFileName(&ofn))
		loadBitmap(file);
}

void BrowserWindow::loadPalette(char *filename)
{
	GFXPalette *newPalette = GFXPalette::load(filename);

	if (newPalette)
	{
		if (palette)
			delete palette;
		palette = newPalette;
		getSurface()->setPalette(palette, false);
		paint();
	}
	else 
	{
		char buffer[1024];
		sprintf(buffer, "Error loading the palette \"%s\".", filename);
		okMessage(buffer);
	}
}

void BrowserWindow::openPalette()
{
	char file[1024];
   
	OPENFILENAME ofn;
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner = hWnd;
   
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = "Palette Files\0*.ppl;\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = file;
	ofn.nMaxFile = sizeof(file);
	file[0] = 0;

	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt = "ppl";

	if (GetOpenFileName(&ofn))
		loadPalette(file);
}

void BrowserWindow::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
   hwndCtl;
   codeNotify;

   switch( id )
   {
   	case IDM_EXIT:
    	destroyWindow();
    	break;

    case IDM_ABOUT:
    	DialogBox( GWMain::hInstance, "AboutBox", hWnd, GenericDlg );
    	break;

	case IDM_STRETCH:
		toggleStretch();
		break;

	case IDM_NEXT_LEVEL:
		incrementDetailLevel(1);
		break;

	case IDM_PREV_LEVEL:
		incrementDetailLevel(-1);
		break;

	case IDM_OPEN_BMP:		  
		openBitmap();
		break;

	case IDM_OPEN_PPL:		  
		openPalette();
		break;

    case IDM_HELP:
    case IDM_NEW:
    case IDM_SAVE:
    case IDM_SAVE_AS:
    default:
         DialogBox( GWMain::hInstance, "NoFunctionBox", hWnd, GenericDlg );
         break;
   }
}   

void BrowserWindow::setTitle()
{
	char buffer[1024];
	if (bitmap)
	{
		sprintf(buffer, "Mipview - %s", bitmapFilename);
		SetWindowText(hWnd, buffer);
	}
	else
		SetWindowText(hWnd, "Mipview - No bitmap loaded");
}

//----------------------------------------------------------------------------

class CMain: public GWMain
{
	BrowserWindow* window;

public:
	CMain();
	~CMain();
	virtual bool initInstance();
} Main;

CMain::CMain()
{
	window = 0;
}

CMain::~CMain()
{
	delete window;
}

bool CMain::initInstance()
{
	window = new BrowserWindow(cmdLine);
	return true;
}


