//
//  fewindow.cpp
//

#include <gw.h>
#include <commdlg.h>
#include <g_bitmap.h>
#include <g_surfac.h>
#include <g_pal.h>
#include "g_fontedit.h"
#include "extras.h"
#include "wintools.h"
#include "defines.h"
#include "bmlist.h"
#include "viewWin.h"
#include "editWin.h"
#include "typeWin.h"
#include "cInfoWin.h"
#include "fInfoWin.h"
#include "fewindow.h"

const Point2I fontInfoWindowSize(342,200);
const Point2I charInfoWindowSize(220,200);
char BUFFER_FULL_MESSAGE[] = "Edit buffers are full.  Try Saving and then "
                             "re-opening your font.";

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

FontEditMainWindow::FontEditMainWindow(char *cmdLine)
{
	autoTile             = true;
	hWndClient           = NULL;
	pasteBuffer          = NULL;
	pasteBufferBaseline  = 0;
	fontEdit             = NULL;
	currentPalette       = NULL;
	stripPalette         = true;
	controlKeyDown       = false;
	selectedASCII        = -1;
	mdiClientWindow      = new GWWindow;
	viewWindow           = new FontViewWindow(this);
	editWindow           = new FontEditWindow(this);
	typingWindow         = new FontTypingWindow(this);
	charInfoWindow       = new CharInfoWindow(this);
	fontInfoWindow       = new FontInfoWindow(this);

	strcpy(fontFilename, DEFAULT_FILENAME);

    if (!create(NULL, 
				"",
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
				Point2I(CW_USEDEFAULT,0),
				Point2I(1000, 600),
				NULL,0)) 
	{
		AssertFatal(0,"FontEditMainWindow:: Could not create main window");
		exit(1);
	}
	hMenu = LoadMenu(GWMain::hInstance, "MainMenu");
    SetMenu(hWnd, hMenu);

	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu = GetSubMenu(GetMenu(hWnd), 3);
	ccs.idFirstChild = 1000;

#if 1
	mdiClientWindow->createMDIClient(NULL,
                                     MDIS_ALLCHILDSTYLES | WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN,
									 Point2I(0, 0),
									 getClientSize(),
									 this, 
									 0,
									 &ccs);

	hWndClient = mdiClientWindow->getHandle();

#else

    hWndClient = CreateWindow("MDICLIENT",
                              NULL,
                              MDIS_ALLCHILDSTYLES | WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN,
                              0,
                              0,
                              getClientSize().x,
                              getClientSize().y,
                              hWnd,
                              0,
                              hInstance,
                              &ccs);
#endif
	if (!hWndClient)
	{
		AssertFatal(0,"FontEditMainWindow:: Could not create client window");
		exit(1);
	}	

	Point2I clientSize = getClientSize();
	Point2I wSizes(clientSize.x/2, 2*clientSize.y/3);	
	Point2I zSizes(clientSize.x/3, clientSize.y/3);

	if (!viewWindow->create(Point2I(0,0), wSizes))
	{
		AssertFatal(0,"FontEditMainWindow:: Could not create View window");
		exit(1);
	}		
	if (!editWindow->create(Point2I(wSizes.x+1,0), wSizes))
	{
		AssertFatal(0,"FontEditMainWindow:: Could not create Edit window");
		exit(1);
	}		
	if (!typingWindow->create(Point2I(0, wSizes.y+1), zSizes)) 
	{
		AssertFatal(0,"FontEditMainWindow:: Could not create Typing window");
		exit(1);
	}		
	if (!fontInfoWindow->create(Point2I(wSizes.x+1, wSizes.y+1), zSizes)) 
	{
		AssertFatal(0,"FontEditMainWindow:: Could not create font info window");
		exit(1);
	}		
	if (!charInfoWindow->create(Point2I(2*wSizes.x+1, wSizes.y+1), zSizes)) 
	{
		AssertFatal(0,"FontEditMainWindow:: Could not create char info window");
		exit(1);
	}		

	char paletteFilename[256];
	int argCt = sscanf(cmdLine, "%s", paletteFilename);
	if (argCt > 0)
		loadPalette(paletteFilename);

	setSubWindowSizes();
	setTitle();
	show(GWMain::cmdShow);
	refreshEditMenu();
	refreshWindowMenu();
	refreshPreviewMenu();
	update();
	newFont();
	// Altimor: Temporary because the menus don't work
	openFont();
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::setSubWindowSizes()
{

	Point2I clientSize = getClientSize();
	int bottomSizeY = charInfoWindowSize.y;
	int bottomPosY;

	bottomPosY = clientSize.y - bottomSizeY;

	Point2I viewWindowPos(0, 0);
	Point2I viewWindowSize(clientSize.x/2, clientSize.y - bottomSizeY);
	Point2I editWindowPos(viewWindowPos.x + viewWindowSize.x, 0);
	Point2I editWindowSize(clientSize.x/2, clientSize.y - bottomSizeY);
	Point2I typeWindowPos(0, viewWindowPos.y + viewWindowSize.y);
	Point2I typeWindowSize(clientSize.x - (charInfoWindowSize.x + fontInfoWindowSize.x),
	                       bottomSizeY);
	Point2I fontInfoWindowPos(typeWindowPos.x + typeWindowSize.x,
	                          bottomPosY);
	Point2I charInfoWindowPos(clientSize.x - charInfoWindowSize.x,
	                          bottomPosY);

	if (viewWindow)
		viewWindow->move(viewWindowPos.x,  viewWindowPos.y,
		                 viewWindowSize.x, viewWindowSize.y);
	if (editWindow)
		editWindow->move(editWindowPos.x,  editWindowPos.y,
		                 editWindowSize.x, editWindowSize.y);
	if (typingWindow)
		typingWindow->move(typeWindowPos.x, typeWindowPos.y,
		                   typeWindowSize.x, typeWindowSize.y);
	if (fontInfoWindow)
		fontInfoWindow->move(fontInfoWindowPos.x,  fontInfoWindowPos.y,
		                     fontInfoWindowSize.x, fontInfoWindowSize.y);
	if (charInfoWindow)
		charInfoWindow->move(charInfoWindowPos.x,  charInfoWindowPos.y,
		                     charInfoWindowSize.x, charInfoWindowSize.y);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::resetFont(GFXFontEdit *newFont)
{
	// inform all windows of the new font
	fontEdit = newFont;
	typingWindow  ->setFont(newFont);
	editWindow    ->setFont(newFont);
	viewWindow    ->setFont(newFont);
	fontInfoWindow->setFont(newFont);
	charInfoWindow->setFont(newFont);
}
	
////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::newFont()
{
	// called when the "New" menu item is selected

	// probably should ask users if they want to save current font
	if (fontEdit)
	{
		// don't let it delete the palette, we need to save it incase
		// it gets attached to the new font
		fontEdit->setPalette(NULL);
		delete fontEdit;
	}
	fontEdit = new GFXFontEdit;
	fontEdit->defaultInit();
	resetFont(fontEdit);
	setSelected(-1);               // calls paint()
}

////////////////////////////////////////////////////////////////////////

void CopyBits(GFXBitmap *pDstBM, 
              char      *pSrcOrgBits, 
              DWORD      srcStride, 
              RectI     *r, 
              char       backColor, 
              char       foreColor)
{
   char  *pSrcBits;     // pointer to current source bits
   char  *pDstBits;     // pointer to destination bits
   int   dstY=0;        // starting destination row

   for ( int y=r->upperL.y; y<=r->lowerR.y; y++, dstY++ )
   {
      pSrcBits = (pSrcOrgBits + y*srcStride + r->upperL.x);
      pDstBits = (char*)(pDstBM->pBits + dstY*pDstBM->getStride());

      for (int i=0; i<(r->lowerR.x - r->upperL.x + 1); i++ )
         if ( *pSrcBits != backColor )
		 {
            *pDstBits++ = foreColor;
            *pSrcBits++;
		}
         else
         {
            *pDstBits++ = backColor;
            pSrcBits++;
         }

   }
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::newWindowsFont()
{
	CHOOSEFONT chooseFont;
	LOGFONT    logFont;

	memset(&chooseFont, 0, sizeof(CHOOSEFONT));
	chooseFont.lStructSize = sizeof(CHOOSEFONT);
	chooseFont.hwndOwner   = hWnd;
	chooseFont.lpLogFont   = &logFont;
	chooseFont.Flags       = CF_SCREENFONTS | CF_EFFECTS;
	chooseFont.rgbColors   = RGB(0, 0, 0);
	chooseFont.nFontType   = SCREEN_FONTTYPE;

	if (!ChooseFont(&chooseFont))
		return;

	newFont();

	// use the edit window GWCanvas to paint the bitmaps to
	char        buffer[4] = "   ";
	SIZE        size;
	GFXSurface *pSurface = editWindow->getSurface();
	GFXBitmap  *bitmap, *clippedBitmap;
	HFONT       hFont    = CreateFontIndirect(chooseFont.lpLogFont);
	TEXTMETRIC  textMetric;
	COLORREF backgroundColorRef = RGB(  0,   0,   0);
	COLORREF foregroundColorRef = RGB(255, 255, 255);
	
	// Altimor: these methods don't exist anymore
	//pSurface->lockDC();
	SelectObject(pSurface->getDC(), hFont);
	SetBkColor(pSurface->getDC(), backgroundColorRef);
	SetTextColor(pSurface->getDC(), foregroundColorRef);
	GetTextMetrics(pSurface->getDC(), &textMetric);
	//pSurface->unlockDC();

	Point2I center = editWindow->getClientSize();
	RectI   clip;
	center.x /= 2;
	center.y /= 2;

	for (int i = 32; i < 256; i++)
	{
		buffer[1] = (char)i;

		pSurface->lock();
		pSurface->clear(0);
		//pSurface->lockDC();

		TextOut(pSurface->getDC(), center.x, center.y, buffer, 3);

		// GetTextExtentPoint32() seemed to give me slightly less than
		// the actual width on some large bold italic point sizes, so I
		// added a space after the char, since

		GetTextExtentPoint32(pSurface->getDC(), buffer, 3, &size);
		bitmap = GFXBitmap::create(size.cx, size.cy);
		bitmap->attribute |= BMA_TRANSPARENT;
		RectI r(0, 0, size.cx - 1, size.cy - 1);
		CopyBits(bitmap, (char*)pSurface->getAddress(center.x, center.y), pSurface->getStride(), &r, 0, 255);		
		pSurface->unlock();
		//pSurface->unlockDC();
		pSurface->easyFlip();

		// now clip the raw bitmap so we don't waste any space
		clip.upperL.x = 0xfffffff;
		clip.upperL.y = 0xfffffff;
		clip.lowerR.x = -1;
		clip.lowerR.y = -1;
		
		int row, col, found = 0, newBaseline;		
		
		for (row = 0; row < size.cy; row++)
			for (col = 0; col < size.cx; col++)
			{
				if (*bitmap->getAddress(col, row) == 255)
				{
					found = 1;
					if (clip.upperL.x > col) clip.upperL.x = col;
					if (clip.lowerR.x < col) clip.lowerR.x = col;
					if (clip.upperL.y > row) clip.upperL.y = row;
					if (clip.lowerR.y < row) clip.lowerR.y = row;
				}
			}

		if (!found)
		{
			// bitmap is blank, probably a space, leave the width alone
			// but truncate height to one line
			clip.upperL.x = 0; 
			clip.upperL.y = size.cy - textMetric.tmDescent - 1;
			clip.lowerR.x = size.cx/3;  // since size.cx was for three spaces
			clip.lowerR.y = clip.upperL.y + 1;
		}

		RectI cutRect(0, 0, clip.lowerR.x - clip.upperL.x, clip.lowerR.y - clip.upperL.y);
		clippedBitmap = GFXBitmap::create(1 + cutRect.lowerR.x, 1 + cutRect.lowerR.y);
		CopyBits(clippedBitmap, 
				(char*)bitmap->getAddress(clip.upperL.x, clip.upperL.y),
				bitmap->getStride(), &cutRect, 0, 255);
		clippedBitmap->attribute = bitmap->attribute;
		newBaseline = textMetric.tmDescent - ((bitmap->height - 1) - clip.lowerR.y);
		delete bitmap;

		fontEdit->insertBitmap(i, clippedBitmap, newBaseline);
	}		

	setSelected('Q');
	refreshPreviewMenu();
	refresh();
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::cut()
{
	// called when the "Cut" menu item is selected
	if (selectedASCII == -1)
		return;
	
	int nextSelectedASCII = fontEdit->asciiOfNextChar(selectedASCII, 1);

	if (pasteBuffer)
		delete pasteBuffer;
	pasteBuffer         = fontEdit->getBitmap(selectedASCII);
	pasteBufferBaseline = fontEdit->getFontCharInfoOnly(selectedASCII)->baseline;

	fontEdit->deleteChar(selectedASCII);
	setSelected(nextSelectedASCII);	
	fontInfoWindow->refresh();
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::copy()
{
	// called when the "Copy" menu item is selected
	if (selectedASCII == -1)
		return;

	if (pasteBuffer)
		delete pasteBuffer;
	pasteBuffer         = fontEdit->getBitmap(selectedASCII);
	pasteBufferBaseline = fontEdit->getFontCharInfoOnly(selectedASCII)->baseline;
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::paste()
{
	// called when "Paste Before" menu item is selected
	if (!pasteBuffer)
		return;

	int newASCII = fontEdit->findNextAvailableASCII(selectedASCII, -1);
	if (fontEdit->insertBitmap(newASCII, pasteBuffer, pasteBufferBaseline))
	{
		setSelected(newASCII);
		fontInfoWindow->refresh();
	}
	else
		okMessage(BUFFER_FULL_MESSAGE);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::append()
{
	// called when "Paste After" menu item is selected
	if (!pasteBuffer)
		return;

	int newASCII = fontEdit->findNextAvailableASCII(selectedASCII, 1);
	if (fontEdit->insertBitmap(newASCII, pasteBuffer, pasteBufferBaseline))
	{
		setSelected(newASCII);
		fontInfoWindow->refresh();
	}
	else
		okMessage(BUFFER_FULL_MESSAGE);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::remove()
{
	// Called when "Delete" menu item is selected.
	if (selectedASCII == -1)
		return;

	int nextSelectedASCII = fontEdit->asciiOfNextChar(selectedASCII, 1);
	fontEdit->deleteChar(selectedASCII);
	setSelected(nextSelectedASCII);	
	fontInfoWindow->refresh();
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::promote()
{
	// called when "Promote" menu item is selected
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::demote()
{
	// called when "Demote" menu item is selected
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::refreshWindowMenu()
{
	if (autoTile)
	{
		CheckMenuItem(hMenu, IDM_WINDOW_AUTOTILE, MF_CHECKED);
		setSubWindowSizes();
	}
	else
		CheckMenuItem(hMenu, IDM_WINDOW_AUTOTILE, MF_UNCHECKED);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::refreshPreviewMenu()
{
	int j = 0;

	if (fontEdit) 
		j = fontEdit->getJustification();

	CheckMenuItem(hMenu, IDM_PREVIEW_TEXT_LEFT,     j & TEXT_LEFT     ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_PREVIEW_TEXT_RIGHT,    j & TEXT_RIGHT    ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_PREVIEW_TEXT_CENTER_H, j & TEXT_CENTER_H ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_PREVIEW_TEXT_CENTER_V, j & TEXT_CENTER_V ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_PREVIEW_TEXT_TOP,      j & TEXT_TOP      ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_PREVIEW_TEXT_BOTTOM,   j & TEXT_BOTTOM   ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_PREVIEW_TEXT_FIT,      j & TEXT_FIT      ? MF_CHECKED : MF_UNCHECKED);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::refreshEditMenu()
{
	// Set check/uncheck stuff on the Edit menu
	
	if (selectedASCII >= 0)
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

void FontEditMainWindow::notifyOfSelectedIth(int i)
{
	// FontViewWindow is letting us know what char no. is selected,
	// we pass this info to the edit window so it can update it's
	// horiz scroll bar
	editWindow->notifyOfSelectedIth(i);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::setSelectedIth(int i)
{
	// select the ith char in the font, as opposed to an absolute
	// ASCII value.  This is used by a scrollbar that goes from 0 to n

	setSelected(fontEdit->asciiOfIthChar(i));
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::adjustSelectedIth(int delta)
{
	// jump delta chars forward or backward in the char set
	if (delta > 0)
		setSelected(fontEdit->asciiOfNextChar(selectedASCII, delta));
	else
		setSelected(fontEdit->asciiOfPrevChar(selectedASCII, delta));
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::setSelected(int newIndex)
{
	if (!fontEdit || fontEdit->getNumChars() == 0)
		selectedASCII = -1;
	else
	{
		selectedASCII = newIndex;
		if (selectedASCII < fontEdit->getFirstASCII())
			selectedASCII = fontEdit->getFirstASCII();
		if (selectedASCII > fontEdit->getLastASCII())
			selectedASCII = fontEdit->getLastASCII();
	}
	viewWindow->setSelectedIndex(selectedASCII);
	editWindow->setSelectedASCII(selectedASCII);
	charInfoWindow->setSelectedASCII(selectedASCII);
	refreshEditMenu();
	paint();
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::adjustSelected(int delta)
{
	setSelected(selectedASCII + delta);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::refresh()
{
	fontInfoWindow->refresh();
	charInfoWindow->refresh();
	paint();	
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::paint()
{
   // make sure that all sub windows are
   // created before sending them paint 
   // commands
   if (typingWindow->getHandle() == NULL)
      return; 

	viewWindow->paint();
	editWindow->paint();
	typingWindow->paint();
	SetFocus(hWnd);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::onSize(UINT, int cx, int cy)
{
	if (cx < 1 || cy < 1)
		return;

	if (hWndClient)
	{	
		Point2I clientSize = getClientSize();
		SetWindowPos(hWndClient,
		             HWND_BOTTOM,
					 0, 0, clientSize.x, clientSize.y,
					 0);
	}
	if (autoTile)
		setSubWindowSizes();
	paint();
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::onChar(TCHAR ch, int cRepeat, UINT flags)
{
	// pass typed chars to the preview window, filter out all control
	// chars except backspace and enter
	if (ch > 31 || ch == 0x08 || ch == 0x0D)
	{
		for (int i = 0; i < cRepeat; i++)
			typingWindow->append(ch, cRepeat, flags);
	}
	else
	   Parent::onChar(ch, cRepeat, flags);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if (vk == VK_CONTROL)
		controlKeyDown = fDown;

	if (cRepeat != (int)flags)
		cRepeat = flags = 100;

	if (!fDown)
	{
		switch (vk)                
		{                          
		case VK_PRIOR:
			adjustSelectedIth(-1);
			return;

		case VK_NEXT:
			adjustSelectedIth(1);
			return;
		
		case VK_UP:
		case VK_RIGHT:
			adjustSelectedIth(1);
			return;

		case VK_DOWN:
		case VK_LEFT:
			adjustSelectedIth(-1);
			return;

		case VK_HOME:
			setSelected(-1);
			return;
						 
		case VK_END:
			setSelected(LARGE_INT);
			break;

		case VK_DELETE:
			remove();
			break;

		case VK_INSERT:						   
			paste();
			break;
		case 0xBB:
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
			break;
		default:
   			break;
      }
   }
   Parent::onKey(vk, fDown, cRepeat, flags);
}   

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::loadBitmap(char *filename)
{
	// Called from openBitmap(), loads one bitmap from the specified
	// file and inserts it into the current array.
			    
	GFXBitmap *newBitmap;
	newBitmap = GFXBitmap::load(filename, 0);
	if (newBitmap)
	{
		if (!fontEdit->appendBitmap(newBitmap))
		{
			char buffer[1024];
			sprintf(buffer, "The bitmap \"%s\" could not be inserted.  Perhaps it is too big to be a character.",
			        filename);
			okMessage(buffer);
		}
		delete newBitmap;
		paint();
		//paste(newBitmap);
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

void FontEditMainWindow::openBitmap()
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

void FontEditMainWindow::setPalette(GFXPalette *p)
{
	if (currentPalette)
		delete currentPalette;

	currentPalette = p;
	
	viewWindow->setPalette(p);
	editWindow->setPalette(p);
	typingWindow->setPalette(p);
	fontInfoWindow->setPalette(p);
}			

////////////////////////////////////////////////////////////////////////

Bool FontEditMainWindow::isOldFontFile(char *filename)
{
	// open the specified file, look at the tag, see if it's a new
	// or old font

	HANDLE fileHandle = CreateFile(filename,
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		char buffer[1024];
		sprintf(buffer, "Unable to open the file \"%s\".", filename);
		okMessage(buffer);
		return(false);
	}
	FileRStream frs(fileHandle);

	DWORD data;
	frs.read(&data);
	frs.close();
	CloseHandle(fileHandle);

	if (data == FOURCC('P', 'F', 'N', 'T'))
		return(true);
	else
		return(false);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::loadFont(char *filename)
{
	// Called by openFont(), performs actual reading of the
	// a bitmap array from a file.

	GFXFontEdit *newFont;;
	if (isOldFontFile(filename))
		newFont = GFXFontEdit::loadOldFont(filename);
	else
		newFont = GFXFontEdit::load(filename);

	if (newFont)
	{
		if (fontEdit)
			delete fontEdit;
		resetFont(newFont);   // assignes fontEdit = newFont
		strcpy(fontFilename, filename);
		if (fontEdit->pPal)
			setPalette(fontEdit->pPal);
						
		setTitle();
		refreshPreviewMenu();
		setSelected(0);
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

void FontEditMainWindow::writeFont(char *filename)
{
	// Called by save() and/or saveAs(), performs actual writing of
	// the bitmap array

	fontEdit->setPalette(fontInfoWindow->savePalette ? currentPalette : NULL);
	
	if (fontEdit->save(filename))
	{
		strcpy(fontFilename, filename);
		setTitle();
	}
	else
	{
		char buffer[2048];
		sprintf(buffer, "Error writing \"%s\" (perhaps it is read-only?)", filename);
		okMessage(buffer);
	}
	refresh();
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::save()
{
	// Called when "Save" menu item is selected, asks for a filename
	// of one hasn't yet been specified.

	if (fontEdit->getNumChars() > 0)
	{
		if (strcmp(fontFilename, DEFAULT_FILENAME) == 0)
			saveAs();
		else
			writeFont(fontFilename);
	}
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::exportBitmapArray()
{
	// Called when "Save As..." menu item is selected, presents Save
	// file dialog, calls writeFontEditMain() to perfrom actual writing
	if (fontEdit->getNumChars() < 1)
		return;

	char         file[1024];
	OPENFILENAME ofn;

	file[0] = '\0';
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
		if (!fontEdit->bma.write(file, 0))
		{	
			char buffer[1024];
			sprintf(buffer, "Error writing \"%s\".", file);
			okMessage(buffer);
		}
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::exportBitmapSheet()
{
   // Called when "Export Bitmap Sheet..." menu item is selected, presents Save
	// file dialog, canns writeBitmapSheet() to perfrom actual writing
	if (fontEdit->getNumChars() < 1)
		return;

	char         file[1024];
	OPENFILENAME ofn;

	file[0] = '\0';
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Bitmap Files\0*.bmp\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "bmp";
   if (GetSaveFileName(&ofn))
   {
      if (!writeBitmapSheet(file))
		{	
			char buffer[1024];
			sprintf(buffer, "Error writing \"%s\".", file);
			okMessage(buffer);
		}
   }
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::importBitmapSheet()
{
	// Called when "Import Bitmap Sheet..." menu item is selected, presents Open File
	// dialog, calls readBitmapSheet() for actual file read

   if (fontEdit->getNumChars() < 1)
   {
   	okMessage("A font must be loaded first.");
      return;
   }

	char         file[1024];
	OPENFILENAME ofn;

	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Font Files\0*.bmp\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "bmp";
	if (GetOpenFileName(&ofn))
      readBitmapSheet(file);
}

////////////////////////////////////////////////////////////////////////

int FontEditMainWindow::getBitmapSheetHeight()
{
   Int32 lineLen, lineHeight, idx, height;
   GFXBitmapArray &bma = fontEdit->bma;

   height = BMP_SHEET_BORDER;
   idx = 0;
   while (idx < bma.numBitmaps)
   {
      lineLen = BMP_SHEET_BORDER;
      lineHeight = 0;
      while (idx < bma.numBitmaps)
      {
         if ( (lineLen + bma[idx]->width + BMP_SHEET_BORDER) < BMP_SHEET_WIDTH )
         {
            lineLen += bma[idx]->width + BMP_SHEET_SPACING;
            if (lineHeight < bma[idx]->height)
               lineHeight = bma[idx]->height;
            idx++;
         }      
         else
            break;
      }
      height += lineHeight + BMP_SHEET_SPACING;
   }
   height += BMP_SHEET_BORDER;
   
   return height;      
}  
 
////////////////////////////////////////////////////////////////////////

Bool FontEditMainWindow::writeBitmapSheet(char *filename)
{
   Int32 lineLen, lineHeight, idx, height;
   GFXBitmapArray &bma = fontEdit->bma;
   
   // first get height of the bmpSheet.
   // width is preset by BMP_SHEET_WIDTH
   height = getBitmapSheetHeight();

   // create a bitmap big enough to store the entire bma
   GFXBitmap *bmpSheet = GFXBitmap::create(BMP_SHEET_WIDTH, height);
   memset(bmpSheet->pBits, fontEdit->fi.backColor, bmpSheet->getHeight() * bmpSheet->getStride() );

   // copy the bma into the new bitmap sheet
   height = BMP_SHEET_BORDER;
   idx = 0;
   while (idx < bma.numBitmaps)
   {
      lineLen = BMP_SHEET_BORDER;
      lineHeight = 0;
      while (idx < bma.numBitmaps)
      {
         if ( (lineLen + bma[idx]->width + BMP_SHEET_BORDER) < BMP_SHEET_WIDTH )
         {
            // copy the individual bma bitmaps into bmpSheet
            BYTE *src = bma[idx]->pBits;
            BYTE *dest = bmpSheet->pBits + (height * bmpSheet->stride + lineLen);
            for (int j = 0; j < bma[idx]->height; j++)
            {
               memcpy(dest, src, bma[idx]->width);
               src += bma[idx]->stride;
               dest += bmpSheet->stride;
            }

            lineLen += bma[idx]->width + BMP_SHEET_SPACING;
            if (lineHeight < bma[idx]->height)
               lineHeight = bma[idx]->height;
            idx++;
         }      
         else
            break;
      }
      height += lineHeight + BMP_SHEET_SPACING;
   }

   // set the palette
   // save the bitmap
   bmpSheet->pPalette = currentPalette;
   Bool result = bmpSheet->write(filename);

   bmpSheet->pPalette = NULL;
   delete bmpSheet;
   return result;
}   

////////////////////////////////////////////////////////////////////////

Bool FontEditMainWindow::readBitmapSheet(char *filename)
{
   Int32 lineLen, lineHeight, idx, height;
   GFXBitmapArray &bma = fontEdit->bma;
   
   // first get height of the bmpSheet.
   // width is preset by BMP_SHEET_WIDTH
   height = getBitmapSheetHeight();   

   // load the bitmap sheet and make sure its the right size
   GFXBitmap *bmpSheet = new GFXBitmap;
   if ( !bmpSheet->read(filename, BMF_INCLUDE_PALETTE))
   {
      okMessage("Could not read the bitmap");
      return false;  
   }
   if ( (bmpSheet->width != BMP_SHEET_WIDTH) || (bmpSheet->height != height) )
   {
      char buffer[1024];
		sprintf(buffer, " \"%s\" is not the right size for the current font.\nPerhaps the bitmap does match the loaded font", filename);
		okMessage(buffer);
      return false;
   }

   // copy the bitmap sheet into the bma
   height = BMP_SHEET_BORDER;
   idx = 0;
   while (idx < bma.numBitmaps)
   {
      lineLen = BMP_SHEET_BORDER;
      lineHeight = 0;
      while (idx < bma.numBitmaps)
      {
         if ( (lineLen + bma[idx]->width + BMP_SHEET_BORDER) < BMP_SHEET_WIDTH )
         {
            // copy the individual bma bitmaps into bmpSheet
            BYTE *dest = bma[idx]->pBits;
            BYTE *src = bmpSheet->pBits + (height * bmpSheet->stride + lineLen);
            for (int j = 0; j < bma[idx]->height; j++)
            {
               memcpy(dest, src, bma[idx]->width);
               dest += bma[idx]->stride;
               src += bmpSheet->stride;
            }

            lineLen += bma[idx]->width + BMP_SHEET_SPACING;
            if (lineHeight < bma[idx]->height)
               lineHeight = bma[idx]->height;
            idx++;
         }      
         else
            break;
      }
      height += lineHeight + BMP_SHEET_SPACING;
   }

   // set the palette
   // save the bitmap
   setPalette(bmpSheet->pPalette);
   bmpSheet->pPalette = NULL;
   delete bmpSheet;
   return true;
}  

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::saveAs()
{
	// Called when "Save As..." menu item is selected, presents Save
	// file dialog, canns writeFontEditMain() to perfrom actual writing
	if (fontEdit->getNumChars() < 1)
		return;

	char         file[1024];
	OPENFILENAME ofn;

	strcpy(file, fontFilename);
	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Font Files\0*.pft\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "pft";
	if (GetSaveFileName(&ofn))
		writeFont(file);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::openFont()
{
	// Called when "Open..." menu item is selected, presents Open File
	// dialog, calls loadFontEditMain() for actual file read

	char         file[1024];
	OPENFILENAME ofn;

	memset((void *) &ofn, 0, sizeof(ofn));
	ofn.hwndOwner       = hWnd;
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = "Font Files\0*.pft\0\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFile       = file;
	ofn.nMaxFile        = sizeof(file);
	file[0]             = 0;
	ofn.lpstrInitialDir = ".";
	ofn.lpstrDefExt     = "pft";
	if (GetOpenFileName(&ofn))
		loadFont(file);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::loadPalette(char *filename)
{
	// Called by openPalette() or the constructor (if cmd line arg was
	// supplied), loads and sets a palette
	GFXPalette *newPalette = GFXPalette::load(filename);

	if (newPalette)
	{
		setPalette(newPalette);
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

void FontEditMainWindow::openPalette()
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

void FontEditMainWindow::onCommand(int id, HWND, UINT)
{
	switch(id)
	{
		case IDM_FILE_NEW:         newFont();  break;
		case IDM_FILE_NEW_WINDOWS: newWindowsFont();  break;
		case IDM_FILE_OPEN:        openFont(); break;

		case IDM_WINDOW_AUTOTILE:
			autoTile = !autoTile;
			refreshWindowMenu();
			break;

		case IDM_FILE_SAVE:       save();            break;
		case IDM_FILE_SAVEAS:     saveAs();          break;
		case IDM_FILE_EXIT:       destroyWindow();   break;
		case IDM_FILE_OPEN_PAL:   openPalette();     break;
		case IDM_FILE_INSERT_BMP: openBitmap();      break;
		case IDM_FILE_EXPORT_PBA: exportBitmapArray();      break;
      case IDM_FILE_EXPORT_BMP_SHEET: exportBitmapSheet();break;
      case IDM_FILE_IMPORT_BMP_SHEET: importBitmapSheet();break;

		case IDM_EDIT_CUT:      cut();               break;
		case IDM_EDIT_COPY:     copy();              break;
		case IDM_EDIT_PASTE:    paste();             break;
		case IDM_EDIT_DELETE:   remove();            break;
		case IDM_EDIT_APPEND:   append();            break;
		case IDM_EDIT_ADDSPACE:
		{
			if (fontEdit->getCharTableIndex(32) == -1)
				fontEdit->addASCII32();
			else
				okMessage("The space character (ASCII 32) is already in this character set.");
			setSelected(32);
			refresh();
			break;
		}

		case IDM_PREVIEW_CLEAR:
			typingWindow->clearText();
			break;

		case IDM_PREVIEW_TEXT_LEFT: 
			fontEdit->toggleJustificationFlag(TEXT_LEFT);
			refreshPreviewMenu(); paint();
			break;

		case IDM_PREVIEW_TEXT_RIGHT: 
			fontEdit->toggleJustificationFlag(TEXT_RIGHT);
			refreshPreviewMenu(); paint();
			break;


		case IDM_PREVIEW_TEXT_CENTER_H: 
			fontEdit->toggleJustificationFlag(TEXT_CENTER_H);
			refreshPreviewMenu(); paint();
			break;


		case IDM_PREVIEW_TEXT_CENTER_V: 
			fontEdit->toggleJustificationFlag(TEXT_CENTER_V);
			refreshPreviewMenu(); paint();
			break;


		case IDM_PREVIEW_TEXT_TOP: 
			fontEdit->toggleJustificationFlag(TEXT_TOP);
			refreshPreviewMenu(); paint();
			break;


		case IDM_PREVIEW_TEXT_BOTTOM: 
			fontEdit->toggleJustificationFlag(TEXT_BOTTOM);
			refreshPreviewMenu(); paint();
			break;


		case IDM_PREVIEW_TEXT_FIT: 
			fontEdit->toggleJustificationFlag(TEXT_FIT);
			refreshPreviewMenu(); paint();
			break;

		case IDM_ABOUT:
			DialogBox(GWMain::hInstance, "AboutBox", hWnd, GenericDlg);
			break;
   }
}   

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::setTitle()
{
	SetWindowText(hWnd, fontFilename);
}

////////////////////////////////////////////////////////////////////////

FontEditMainWindow::~FontEditMainWindow()
{
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::onDestroy()
{
	PostQuitMessage(0);
}

////////////////////////////////////////////////////////////////////////

void FontEditMainWindow::okMessage(char *s)
{
	// Generic message box with single "OK" button

	MessageBox(hWnd, s, NULL, MB_APPLMODAL);
}

//----------------------------------------------------------------------------

class CMain : public GWMain
{
	FontEditMainWindow* window;

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
	window = new FontEditMainWindow(cmdLine);
	return true;
}


