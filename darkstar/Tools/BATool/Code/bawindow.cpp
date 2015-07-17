//
//  bawindow.cpp
//
#include <stdio.h>

#include <gw.h>
#include <commdlg.h>
#include "extras.h"
#include "wintools.h"
#include "defines.h"
#include "bmlist.h"
#include "bacanvas.h"
#include "bawindow.h"
#include "g_pal.h"
#include "g_bitmap.h"
#include "g_barray.h"
#include "g_surfac.h"

////////////////////////////////////////////////////////////////////////

GFXBitmap *duplicateBitmap(GFXBitmap *src)
{
	// Anonymous bitmap copy function, used mostly for making duplicates
	// for pasting purposes (paste from the paste buffer but leave a 
	// copy *in* the paste buffer)

	GFXBitmap *dst    = new GFXBitmap;
	dst->width        = src->width;
	dst->height       = src->height;
	dst->stride       = src->stride;
	dst->bitDepth     = src->bitDepth;
	dst->imageSize    = src->imageSize;
	dst->pPalette     = NULL;
	dst->attribute    = src->attribute;
	dst->detailLevels = src->detailLevels;

	dst->pBits = new BYTE[dst->imageSize];
	int i;
	memcpy(dst->pBits, src->pBits, dst->imageSize);

	for (i = 0; i < dst->detailLevels; i++)
		dst->pMipBits[i] = src->pMipBits[i];

	return(dst);
}	

////////////////////////////////////////////////////////////////////////

BitmapArrayWindow::BitmapArrayWindow(char *cmdLine)
{
	canvas                = NULL;
	pasteBuffer           = NULL;
	bitmaps               = new BitmapList;
	viewOneBitmap         = false;
	stripPalette          = true;
	transparent           = false;
	controlKeyDown        = false;
	viewSize              = 100;
	scrollbarPosition     = 0;
	maxScrollbarPosition  = 0;
	selectedIndex         = -1;
	strcpy(bitmapArrayFilename, DEFAULT_FILENAME);

    if (!create(NULL, 
				"",
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
				Point2I(CW_USEDEFAULT,0),
				Point2I(500, 350),
				NULL,0)) 
	{
		AssertFatal(0,"BitmapArrayWindow:: Could not create main window");
		exit(1);
	}
	hMenu = LoadMenu(GWMain::hInstance, "MainMenu");
    SetMenu(hWnd, hMenu);

	canvas = new BitmapArrayCanvas;	  
    if (!canvas->create("",
		                WS_CHILD | WS_VISIBLE,
		                Point2I(CW_USEDEFAULT,0),
		                Point2I(200, 200),
		                this, ID_CANVAS)) 
	{
		AssertFatal(0,"BitmapArrayWindow:: Could not create canvas");
		exit(1);
	}
	canvas->parentWindow = this;

	hScrollbar = CreateWindow("SCROLLBAR",	NULL,
						      WS_CHILD | WS_VISIBLE | SBS_HORZ,
							  0, 200,
							  300, GetSystemMetrics(SM_CYVSCROLL),
							  hWnd, 
							  HMENU(ID_SCROLLBAR), 
							  getHInstance(),
							  NULL);

	char bitmapFilename[256], paletteFilename[256];
	int argCt = sscanf(cmdLine, "%s %s", paletteFilename, bitmapFilename);
	if (argCt > 0)
		loadPalette(paletteFilename);
	if (argCt > 1)
		loadBitmapArray(bitmapFilename);

	setTitle();
	positionScrollbar();
	updateViewSize(viewSize);
	show(GWMain::cmdShow);
	refreshEditMenu();
	refreshViewMenu();
	refreshOptionsMenu();
	update();
	paint();
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::newBitmapArray()
{
	// called when the "New" menu item is selected
	
	strcpy(bitmapArrayFilename, DEFAULT_FILENAME);
	bitmaps->Zap();
	selectedIndex = -1;
	refreshEditMenu();
	setTitle();
	updateViewSize(viewSize);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::cut()
{
	// called when the "Cut" menu item is selected
	
	if (selectedIndex == -1)
		return;
	
	if (pasteBuffer)
		delete pasteBuffer;

	bitmaps->SetCurrent(selectedIndex);
	pasteBuffer = duplicateBitmap(bitmaps->GetCurrent());
	bitmaps->DeleteCurrent();
	if (selectedIndex == bitmaps->count)
		// they cut the last one
		selectedIndex--;
	if (bitmaps->count == 0)
		selectedIndex = -1;
	refreshEditMenu();
	updateViewSize(viewSize);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::copy()
{
	// called when the "Copy" menu item is selected
	
	if (selectedIndex == -1)
		return;

	if (pasteBuffer)
		delete pasteBuffer;

	pasteBuffer = duplicateBitmap(bitmaps->GetItem(selectedIndex));
	refreshEditMenu();
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::paste(GFXBitmap *insertBuffer)
{
	// If insertBuffer==NULL then call is from "Paste Before" menu item,
	// otherwise call is from loadBitmap()

	// Paste either from the paste buffer or from the supplied argument,
	// this allows loadBitmap() to insert multiple items and not affect
	// the paste buffer

	if (!insertBuffer)
	{
		if (!pasteBuffer)
			return;
		else
			insertBuffer = duplicateBitmap(pasteBuffer);
	}

	if (selectedIndex == -1)
	{
		// paste at the very front
		bitmaps->Rewind();
		bitmaps->Insert(insertBuffer);
	}
	else
	{
		// paste in front of selected item
		bitmaps->SetCurrent(selectedIndex);
		bitmaps->Insert(insertBuffer);
	}
	updateViewSize(viewSize);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::append()
{
	// called when "Paste After" menu item is selected
	
	if (!pasteBuffer)
		return;

	GFXBitmap *tmp = duplicateBitmap(pasteBuffer);
	if (selectedIndex == -1)
	{
		// paste at the very end
		bitmaps->Append(tmp);
		selectedIndex = bitmaps->count - 1;
	}
	else
	{
		// paste just after the selected item
		bitmaps->SetCurrent(selectedIndex);
		bitmaps->Skip();
		bitmaps->Insert(tmp);
		selectedIndex++;
	}
	updateViewSize(viewSize);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::remove()
{
	// Called when "Delete" menu item is selected.
	// Remove the selected item from the list and delete the bitmap
	// from memory
	
	if (selectedIndex == -1)
		return;

	bitmaps->SetCurrent(selectedIndex);
	GFXBitmap *tmp = bitmaps->GetCurrent();
	bitmaps->DeleteCurrent();
	delete tmp;
	if (selectedIndex == bitmaps->count)
		// they deleted the last one
		selectedIndex--;
	if (bitmaps->count == 0)
		selectedIndex = -1;
	refreshEditMenu();
	updateViewSize(viewSize);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::promote()
{
	// called when "Promote" menu item is selected
	
	if (selectedIndex <= 0)
		return;
	bitmaps->Promote(selectedIndex);
	selectedIndex--;
	refreshEditMenu();
	paint();
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::demote()
{
	// called when "Demote" menu item is selected
	
	if (selectedIndex == -1 || selectedIndex == bitmaps->count - 1)
		return;
	bitmaps->Demote(selectedIndex);
	selectedIndex++;
	refreshEditMenu();
	paint();
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::leftButtonDown(int x, int y)
{
	// Called from BitmapArrayCanvas::onLButtonDown, since it's the
	// canvas that captures the mouse click
	
	Point2I clientSize = canvas->getClientSize();

	if (x < clientSize.x && y < clientSize.y)
	{
		selectedIndex = canvas->findItem(x, y);
		refreshEditMenu();
		paint();
	}
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::updateViewSize(int newSize)
{
	// Recalculate position of bitmaps on the canvas, and max
	// scrollbar ranges, called whenever properties of the group
	// of bitmaps changes, e.g. a bitmap is pasted or cut, or the
	// window size changes.
	
	bitmaps->ReEvaluateDimensions();
	int bitmapCt = bitmaps->count;
	viewSize = newSize;

	if (!canvas)
		return;

	float oldPosition;
	if (maxScrollbarPosition == 0)
		oldPosition = 0.0;
	else
		oldPosition = float(scrollbarPosition)/float(maxScrollbarPosition);

	if (viewOneBitmap)
	{
		if (bitmaps)
			maxScrollbarPosition = bitmapCt - 1;
	}
	else
	{
		int     totalWidth;
		float   scale      = viewSize/100.0;
		Point2I canvasSize = canvas->getClientSize();

		totalWidth = bitmapCt*bitmaps->width*scale + BITMAP_MARGIN*(bitmapCt + 1);
		maxScrollbarPosition = totalWidth - canvasSize.x;
	}

	if (maxScrollbarPosition > 0)	
	{
		scrollbarPosition = round(oldPosition*float(maxScrollbarPosition));
		SetScrollRange(hScrollbar, SB_CTL, 0, maxScrollbarPosition, true);	
	}
	else
	{
		// all bitmaps fit in the window, restrict movement of the scrollbar
		SetScrollRange(hScrollbar, SB_CTL, 0, 0, true);	
		// set scroll value to max/2, this will center the bitmaps in the window
		scrollbarPosition = maxScrollbarPosition/2;
	}

	SetScrollPos(hScrollbar, SB_CTL, scrollbarPosition, true);
	refreshViewMenu();
	paint();
}							 

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::refreshOptionsMenu()
{
	// Set check/uncheck stuff on the Options menu
	
   CheckMenuItem(hMenu, IDM_OPTIONS_STRIP, stripPalette? MF_CHECKED:MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_OPTIONS_TRANSPARENT, transparent? MF_CHECKED:MF_UNCHECKED);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::refreshViewMenu()
{
	// Set check/uncheck stuff on the View menu
	
	if (viewOneBitmap)   CheckMenuItem(hMenu, IDM_VIEW_FIXED, MF_CHECKED);	
	else                 CheckMenuItem(hMenu, IDM_VIEW_FIXED, MF_UNCHECKED);
	if (viewSize == 25)  CheckMenuItem(hMenu, IDM_VIEW_25,    MF_CHECKED);	
	else                 CheckMenuItem(hMenu, IDM_VIEW_25,    MF_UNCHECKED);
	if (viewSize == 50)  CheckMenuItem(hMenu, IDM_VIEW_50,    MF_CHECKED);	
	else                 CheckMenuItem(hMenu, IDM_VIEW_50,    MF_UNCHECKED);
	if (viewSize == 100) CheckMenuItem(hMenu, IDM_VIEW_100,   MF_CHECKED);	
	else                 CheckMenuItem(hMenu, IDM_VIEW_100,   MF_UNCHECKED);
	if (viewSize == 200) CheckMenuItem(hMenu, IDM_VIEW_200,   MF_CHECKED);	
	else                 CheckMenuItem(hMenu, IDM_VIEW_200,   MF_UNCHECKED);
	if (viewSize == 400) CheckMenuItem(hMenu, IDM_VIEW_400,   MF_CHECKED);	
	else                 CheckMenuItem(hMenu, IDM_VIEW_400,   MF_UNCHECKED);
	if (viewSize == 400) CheckMenuItem(hMenu, IDM_VIEW_400,   MF_CHECKED);	
	else                 CheckMenuItem(hMenu, IDM_VIEW_400,   MF_UNCHECKED);
	if (viewSize == 800) CheckMenuItem(hMenu, IDM_VIEW_800,   MF_CHECKED);	
	else                 CheckMenuItem(hMenu, IDM_VIEW_800,   MF_UNCHECKED);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::refreshEditMenu()
{
	// Set check/uncheck stuff on the Edit menu
	
	if (selectedIndex >= 0)
	{
		EnableMenuItem(hMenu, IDM_EDIT_CUT,     MF_ENABLED);	
		EnableMenuItem(hMenu, IDM_EDIT_COPY,    MF_ENABLED);	
		EnableMenuItem(hMenu, IDM_EDIT_DELETE,  MF_ENABLED);	
	}
	else
	{
		EnableMenuItem(hMenu, IDM_EDIT_CUT,     MF_GRAYED);	
		EnableMenuItem(hMenu, IDM_EDIT_COPY,    MF_GRAYED);	
		EnableMenuItem(hMenu, IDM_EDIT_DELETE,  MF_GRAYED);	
	}

	if (selectedIndex > 0)
		EnableMenuItem(hMenu, IDM_EDIT_PROMOTE, MF_ENABLED);	
	else
		EnableMenuItem(hMenu, IDM_EDIT_PROMOTE, MF_GRAYED);	

	if (selectedIndex < bitmaps->count - 1)
		EnableMenuItem(hMenu, IDM_EDIT_DEMOTE,  MF_ENABLED);	
	else
		EnableMenuItem(hMenu, IDM_EDIT_DEMOTE,  MF_GRAYED);	

	if (pasteBuffer)
	{
		EnableMenuItem(hMenu, IDM_EDIT_PASTE,   MF_ENABLED);	
		EnableMenuItem(hMenu, IDM_EDIT_APPEND,  MF_ENABLED);	
	}
	else
	{	
		EnableMenuItem(hMenu, IDM_EDIT_PASTE,   MF_GRAYED);	
		EnableMenuItem(hMenu, IDM_EDIT_APPEND,  MF_GRAYED);	
	}
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::paint()
{
	if (canvas)
	{
		canvas->bitmaps       = bitmaps;
		canvas->offset        = scrollbarPosition;
		canvas->viewOneBitmap = viewOneBitmap;
		canvas->viewSize      = viewSize;
		canvas->selectedIndex = selectedIndex;
		canvas->paint();
	}
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::onSize(UINT, int, int)
{
	if (canvas)
	{
		Point2I clientSize = getClientSize();
		canvas->setPosition(Point2I(0,0));
		canvas->setClientSize(Point2I(clientSize.x, 
		   						      clientSize.y - SCROLLBAR_HEIGHT));
	}
	positionScrollbar();
	updateViewSize(viewSize);
	paint();
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::positionScrollbar()
{
	// Called if the window size changes, this function assures the
	// scrollbar of being at the bottom of the window, and being the
	// same width as the window
	Point2I clientSize = getClientSize();
	SetWindowPos(hScrollbar, 0,
				 0, clientSize.y - SCROLLBAR_HEIGHT,
				 clientSize.x, SCROLLBAR_HEIGHT,
				 SWP_SHOWWINDOW);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::adjustScrollbar(UINT code, int pos)
{
	// Scrollbar callback, makes appropriate adjustments to the
	// scrollbar based on movements (mouse or kbd).

	if (maxScrollbarPosition < 0)
		// all bitmaps fit in the window, don't move anything
		return;

	int pageAmount;
	if (bitmaps->count == 0)
		pageAmount = 0;
	else
		pageAmount = maxScrollbarPosition/bitmaps->count;

	switch (code)
	{
		case SCROLLBAR_GO_TOP:    scrollbarPosition = 0;   break;
		case SCROLLBAR_GO_BOTTOM: 
			scrollbarPosition = maxScrollbarPosition;   
			break;
		case SB_LEFT:             scrollbarPosition--;     break;
		case SB_LINELEFT:         scrollbarPosition--;     break;
		case SB_RIGHT:            scrollbarPosition++;     break;
		case SB_LINERIGHT:        scrollbarPosition++;     break;
		case SB_PAGERIGHT:
			if (viewOneBitmap)
				scrollbarPosition++;
			else
				scrollbarPosition += pageAmount;
			break;
		case SB_PAGELEFT:
			if (viewOneBitmap)
				scrollbarPosition--;
			else
				scrollbarPosition -= pageAmount;
			break;
			case SB_THUMBPOSITION: scrollbarPosition = pos; break;
		case SB_THUMBTRACK:    scrollbarPosition = pos; break;
	}

	if (scrollbarPosition < 0) scrollbarPosition = 0;
	else if(scrollbarPosition > maxScrollbarPosition)
		scrollbarPosition = maxScrollbarPosition;

	SetScrollPos(hScrollbar, SB_CTL, scrollbarPosition, true);
	paint();
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::onHScroll(HWND hwndCtl, UINT code, int pos)
{
	if (hwndCtl == hScrollbar)
		adjustScrollbar(code, pos);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{

	if (vk == VK_CONTROL)
		controlKeyDown = fDown;

	if (!fDown)
	{
		switch (vk)                
		{                          
		case VK_PRIOR:
			adjustScrollbar(SB_PAGELEFT, 0);
			return;

		case VK_NEXT:
			adjustScrollbar(SB_PAGERIGHT, 0);
			return;
		
		case VK_UP:
		case VK_RIGHT:
			if (selectedIndex != -1 && selectedIndex < bitmaps->count - 1)
				selectedIndex++;
			paint();
			break;

		case VK_DOWN:
		case VK_LEFT:
			if (selectedIndex > 0)
				selectedIndex--;
			paint();
			break;

		case VK_DELETE:
			remove();
			break;

		case VK_INSERT:
			paste();
			break;

		case VK_HOME:
			adjustScrollbar(SCROLLBAR_GO_TOP, 0);
			break;

		case VK_END:
			adjustScrollbar(SCROLLBAR_GO_BOTTOM, 0);
			break;

		case 0xBB:
		case VK_ADD:
			demote();
			break;

		case 0xBD:
		case VK_SUBTRACT:
			promote();
			break;

		case 'x':
		case 'X':
			if (controlKeyDown)	cut();
			break;

		case 'c':
		case 'C':
			if (controlKeyDown)	copy();
			break;

		case 'v':
		case 'V':
			if (controlKeyDown)	paste();
			break;

		case 'a':
		case 'A':
			if (controlKeyDown) append();
      }
   }
   Parent::onKey(vk, fDown, cRepeat, flags);
}   

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::loadBitmap(char *filename)
{
	// Called from openBitmap(), loads one bitmap from the specified
	// file and inserts it into the current array.

	GFXBitmap *newBitmap;
	newBitmap = GFXBitmap::load(filename, stripPalette?0:BMF_INCLUDE_PALETTE);
	if (newBitmap)
	{
      newBitmap->attribute |= transparent? BMA_TRANSPARENT:0;
		// strip off the palette so it doesn't get written in the array
		if (newBitmap->pPalette)
		{
			delete newBitmap->pPalette;
			newBitmap->pPalette = NULL;
		}
		paste(newBitmap);
	}
	else 
	{
		char buffer[1024];
		sprintf(buffer, "Error loading the array \"%s\".", filename);
		okMessage(buffer);
	}
	paint();
}	

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::openBitmap()
{
	// Called from the "Insert Bitmap..." menu item, prompts for
	// bitmap files (open file dialog) and calls loadBitmap() to
	// open them.

	char         file[2048];
	OPENFILENAME ofn;

	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Bitmap Files\0*.bmp\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "bmp";
	ofn.Flags           = OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if (GetOpenFileName(&ofn))
	{
		// parse multi-select filename
		char path[1024];
		int i, j, pathEndPoint;
		
		// extract the path
		i = 0;
		while (file[i] != '\0')
		{
			path[i] = file[i];
			i++;
		}

		if (file[i+1] == '\0')
		{
			// if there is only one string then only one files was
			// selected, ie no multi-select
			loadBitmap(file);
			return;
		}

		pathEndPoint = i;
		path[pathEndPoint++] = '\\';

		// extract and open the bitmaps
		while (file[i] != '\0' || file[i+1] != '\0')
		{
			i++;                         // skip null delimiter
			j = pathEndPoint;
			while (file[i] != '\0')
				path[j++] = file[i++];
			path[j] = '\0';
			loadBitmap(path);
		}
	}
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::loadBitmapArray(char *filename)
{
	// Called by openBitmapArray(), performs actual reading of the
	// a bitmap array from a file.

	GFXBitmapArray *newBitmaps;
	newBitmaps = GFXBitmapArray::load(filename, 
	                                  stripPalette?0:BMF_INCLUDE_PALETTE);
	if (newBitmaps)
	{
      for ( int i=0; i<newBitmaps->getCount(); i++ )
         newBitmaps->getBitmap(i)->attribute |= transparent? BMA_TRANSPARENT:0;

		bitmaps->SetBitmapArray(newBitmaps);
		delete newBitmaps;
		strcpy(bitmapArrayFilename, filename);
		if (selectedIndex >= bitmaps->count)
			selectedIndex = bitmaps->count - 1;
		updateViewSize(viewSize);
		setTitle();
	}
	else 
	{
		char buffer[1024];
		sprintf(buffer, "Error loading the bitmap array \"%s\".", filename);
		okMessage(buffer);
	}
	paint();
}	

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::writeBitmapArray(char *filename)
{
	// Called by save() and/or saveAs(), performs actual writing of
	// the bitmap array
	
	GFXBitmapArray bitmapArray;
	bitmapArray.array      = new GFXBitmap*[bitmaps->count];
	bitmapArray.numBitmaps = bitmaps->count;
	
	bitmaps->Rewind();
	for (int i = 0; i < bitmaps->count; i++)
	{
		bitmapArray.array[i] = bitmaps->GetCurrent();
      bitmapArray.array[i]->attribute |= transparent? BMA_TRANSPARENT:0;
		bitmaps->Skip();
	}
	if (bitmapArray.write(filename, stripPalette?0:BMF_INCLUDE_PALETTE))
	{
		strcpy(bitmapArrayFilename, filename);
		setTitle();
	}
	else
	{
		char buffer[2048];
		sprintf(buffer, "Error writing \"%s\" (perhaps it is read-only?)");
		okMessage(buffer);
	}
	bitmapArray.numBitmaps = 0;    // so destrutor won't delete bitmaps
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::save()
{
	// Called when "Save" menu item is selected, asks for a filename
	// of one hasn't yet been specified.

	if (bitmaps->count > 0)
	{
		if (strcmp(bitmapArrayFilename, DEFAULT_FILENAME) == 0)
			saveAs();
		else
			writeBitmapArray(bitmapArrayFilename);
	}
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::saveAs()
{
	// Called when "Save As..." menu item is selected, presents Save
	// file dialog, canns writeBitmapArray() to perfrom actual writing
	if (bitmaps->count < 1)
		return;

	char         file[1024];
	OPENFILENAME ofn;

	strcpy(file, bitmapArrayFilename);
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Bitmap Array Files\0*.pba\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "pba";
	if (GetSaveFileName(&ofn))
		writeBitmapArray(file);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::openBitmapArray()
{
	// Called when "Open..." menu item is selected, presents Open File
	// dialog, calls loadBitmapArray() for actual file read

	char         file[1024];
	OPENFILENAME ofn;

	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Bitmap Array Files\0*.pba\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "pba";
	if (GetOpenFileName(&ofn))
		loadBitmapArray(file);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::loadPalette(char *filename)
{
	// Called by openPalette() or the constructor (if cmd line arg was
	// supplied), loads and sets a palette
	GFXPalette *newPalette = GFXPalette::load(filename);

	if (newPalette)
	{
		canvas->setPalette(newPalette);
		paint();
	}
	else 
	{
		char buffer[1024];
		sprintf(buffer, "Error loading the palette \"%s\".", filename);
		okMessage(buffer);
	}
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::openPalette()
{
	// Called when "Open Palette..." menu item is selected, presents
	// Open File dialog and calls loadPalette() to do actual open
	char file[1024];

	OPENFILENAME ofn;
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Palette Files\0*.ppl;\0*.pal\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "ppl";

	if (GetOpenFileName(&ofn))
		loadPalette(file);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::onCommand( int id, HWND hwndCtl, UINT codeNotify )
{
	hwndCtl;
	codeNotify;

	switch(id)
	{
		case IDM_FILE_NEW:        newBitmapArray();  break;
		case IDM_FILE_OPEN:       openBitmapArray(); break;
		case IDM_FILE_SAVE:       save();            break;
		case IDM_FILE_SAVEAS:     saveAs();          break;
		case IDM_FILE_EXIT:       destroyWindow();   break;
		case IDM_FILE_OPEN_PAL:   openPalette();     break;
		case IDM_FILE_INSERT_BMP: openBitmap();      break;

		case IDM_EDIT_CUT:      cut();               break;
		case IDM_EDIT_COPY:     copy();              break;
		case IDM_EDIT_PASTE:    paste();             break;
		case IDM_EDIT_DELETE:   remove();            break;
		case IDM_EDIT_APPEND:   append();            break;
		case IDM_EDIT_PROMOTE:  promote();           break;
		case IDM_EDIT_DEMOTE:   demote();            break;

		case IDM_VIEW_FIXED:
			viewOneBitmap = !viewOneBitmap;
			updateViewSize(viewSize);  
			break;

		case IDM_VIEW_25:     updateViewSize(25);  break;
		case IDM_VIEW_50:     updateViewSize(50);  break;
		case IDM_VIEW_100:    updateViewSize(100); break;
		case IDM_VIEW_200:    updateViewSize(200); break;
		case IDM_VIEW_400:    updateViewSize(400); break;
		case IDM_VIEW_800:    updateViewSize(800); break;
	
		case IDM_OPTIONS_STRIP:
			stripPalette = !stripPalette;
			refreshOptionsMenu();
			break;

      case IDM_OPTIONS_TRANSPARENT:
         transparent = !transparent;
			refreshOptionsMenu();
			break;

		case IDM_ABOUT:
			DialogBox(GWMain::hInstance, "AboutBox", hWnd, GenericDlg);
			break;
   }
}   

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::setTitle()
{
	SetWindowText(hWnd, bitmapArrayFilename);
}
////////////////////////////////////////////////////////////////////////

BitmapArrayWindow::~BitmapArrayWindow()
{
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::onDestroy()
{
	PostQuitMessage(0);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayWindow::okMessage(char *s)
{
	// Generic message box with single "OK" button

	MessageBox(hWnd, s, NULL, MB_APPLMODAL);
}

//----------------------------------------------------------------------------

class CMain : public GWMain
{
	BitmapArrayWindow* window;

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
	window = new BitmapArrayWindow(cmdLine);
	return true;
}


