//			   
//  fInfoWin.cpp
//

#include <gw.h>
#include <commdlg.h>
#include "g_fontedit.h"
#include "extras.h"
#include "wintools.h"
#include "defines.h"
#include "bmlist.h"
#include "fInfoWin.h"
#include "fewindow.h"

#define ID_CB_TRANSPARENT  1001
#define ID_CB_TRANSLUCENT  1002
#define ID_CB_MONO         1003
#define ID_CB_UNDERLINED   1004
#define ID_CB_SAVEPALETTE  1005
#define ID_RB_PROPORTIONAL 1006
#define ID_RB_FIXED        1007

#define ID_SET_FOREGROUND  5001
#define ID_SET_BACKGROUND  5002
#define ID_SET_SPACING     5003
#define ID_GET_FOREGROUND  4001
#define ID_GET_BACKGROUND  4002
#define ID_GET_SPACING     4003

////////////////////////////////////////////////////////////////////////

FontInfoWindow::FontInfoWindow(FontEditMainWindow *p)
{
	currentPalette = NULL;
	savePalette    = false;
	parent         = p;
	fontEdit       = NULL;
}

////////////////////////////////////////////////////////////////////////

FontInfoWindow::~FontInfoWindow()
{
}

////////////////////////////////////////////////////////////////////////

void FontInfoWindow::setFont(GFXFontEdit *f)
{
	fontEdit    = f; 
	savePalette = fontEdit->pPal;
	refresh();
}

////////////////////////////////////////////////////////////////////////

bool FontInfoWindow::create(Point2I pos, Point2I size)
{
	if (!createMDIChild(NULL,
                        "Font Info",
                        WS_VISIBLE | WS_CAPTION,
                        pos,
                        size,
                        parent,
                        0))
		return(false);

#define CBRB_COLUMN 220
#define ITEM_COLUMN 100
#define ITEM_WIDTH  90

	CreateWindow("STATIC", "Count:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 5, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	CreateWindow("STATIC", "Widht:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 25, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	CreateWindow("STATIC", "Height:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 45, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	CreateWindow("STATIC", "Baseline:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 65, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	CreateWindow("STATIC", "Palette:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 85, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);


	CreateWindow("STATIC", "Foreground:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 105, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	CreateWindow("STATIC", "Background:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 126, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	CreateWindow("STATIC", "Spacing:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 147, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);


	hWndFontCt         = CreateWindow("STATIC", "", WS_CHILD | SS_SIMPLE | WS_VISIBLE,
	                                  ITEM_COLUMN, 5, 70, 20, hWnd, (HMENU)0, hInstance, NULL);
	hWndFontWidth      = CreateWindow("STATIC", "", WS_CHILD | SS_SIMPLE | WS_VISIBLE,
   	                                  ITEM_COLUMN, 25, 70, 20, hWnd, (HMENU)0, hInstance, NULL);
 	hWndFontHeight     = CreateWindow("STATIC", "", WS_CHILD | SS_SIMPLE | WS_VISIBLE,
	                                  ITEM_COLUMN, 45, 70, 20, hWnd, (HMENU)0, hInstance, NULL);
  	hWndFontBaseline   = CreateWindow("STATIC", "", WS_CHILD | SS_SIMPLE | WS_VISIBLE,
     	                              ITEM_COLUMN, 65, 70, 20, hWnd, (HMENU)0, hInstance, NULL);

	hWndFontForeground    = createGet(ID_GET_FOREGROUND, ITEM_COLUMN, 105, 70, 20);

	hWndFontForegroundSet = CreateWindow("Button", "Set",
	                            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				        		ITEM_COLUMN + 75, 105, 30, 20,
						  		hWnd, (HMENU)ID_SET_FOREGROUND, hInstance, NULL);

	hWndFontBackground    = createGet(ID_GET_BACKGROUND, ITEM_COLUMN, 126, 70, 20);
	hWndFontBackgroundSet = CreateWindow("Button", "Set",
	                            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				        		ITEM_COLUMN + 75, 126, 30, 20,
						  		hWnd, (HMENU)ID_SET_BACKGROUND, hInstance, NULL);

	hWndFontSpacing    = createGet(ID_GET_SPACING, ITEM_COLUMN, 147, 70, 20);
	hWndFontSpacingSet = CreateWindow("Button", "Set",
	                            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				        		ITEM_COLUMN + 75, 147, 30, 20,
						  		hWnd, (HMENU)ID_SET_SPACING, hInstance, NULL);

	hWndPaletteIncl    = CreateWindow("STATIC", "", WS_CHILD | SS_SIMPLE | WS_VISIBLE,
	                                  ITEM_COLUMN, 85, 70, 20, hWnd, (HMENU)0, hInstance, NULL);

	hWndTransparent  = createCheckBox(ID_CB_TRANSPARENT, CBRB_COLUMN,  5, "Transparent");
	hWndTranslucent  = createCheckBox(ID_CB_TRANSLUCENT, CBRB_COLUMN, 25, "Translucent");
	hWndMono         = createCheckBox(ID_CB_MONO,        CBRB_COLUMN, 45, "Mono");
	hWndUnderlined   = createCheckBox(ID_CB_UNDERLINED,  CBRB_COLUMN, 65, "Underlined");
	hWndSavePalette  = createCheckBox(ID_CB_SAVEPALETTE, CBRB_COLUMN, 85, "Save Palette");

	hWndProportional = CreateWindow("BUTTON", "Proportional",
	                   WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP | BS_TEXT,
		  	  		   CBRB_COLUMN, 115, 120, 20, hWnd, (HMENU)ID_RB_PROPORTIONAL, hInstance, NULL);

	hWndFixed        = CreateWindow("BUTTON", "Fixed",
	                   WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_TEXT,
		  	  		   CBRB_COLUMN, 135, 120, 20, hWnd, (HMENU)ID_RB_FIXED, hInstance, NULL);


	return(true);
}

////////////////////////////////////////////////////////////////////////

HWND FontInfoWindow::createGet(int id, int x, int y, int w, int h)
{
   	return(CreateWindow("EDIT", "", WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER,
	                    x, y, w, h, hWnd, (HMENU)id, hInstance, NULL));

}		

////////////////////////////////////////////////////////////////////////

static char color_error_msg[]   = "Valid colors are 0 to 255";
static char spacing_error_msg[] = "Valid spacing is 0 to 255";

void FontInfoWindow::onCommand(int id, HWND hWndCtl, UINT codeNotify)
{
	bool    checked;
	LRESULT state = SendMessage(hWndCtl, BM_GETCHECK, 0, 0);

	GFXFontInfo fontInfo;
	if (fontEdit)	
		fontEdit->getExtents(&fontInfo);
	else
		memset((void *)&fontInfo, 0, sizeof(fontInfo));

	switch (state)
	{
		case BST_CHECKED:   
			checked = true;  
			break;
		case BST_UNCHECKED:	
			checked = false; 
			break;
		default:
			GWWindow::onCommand(id, hWndCtl, codeNotify);
			return;
	}

	switch (id)
	{
		case ID_SET_SPACING:		
		{
			char buffer[1024];
			int  intVal;
			GetWindowText(hWndFontSpacing, buffer, 1024);
			if (sscanf(buffer, "%d", &intVal) == 1)
			{
				if (intVal >= 0 && intVal <= 255)
					fontEdit->setSpacing(intVal);
				else
					parent->okMessage(spacing_error_msg);
			}
			else
				parent->okMessage(spacing_error_msg);
			parent->refresh();
			break;
		}
		case ID_SET_FOREGROUND:		
		{
			if (fontInfo.flags & FONT_MONO)
			{
				char buffer[1024];
				int  intVal;
				GetWindowText(hWndFontForeground, buffer, 1024);
				if (sscanf(buffer, "%d", &intVal) == 1)
				{
					if (intVal >= 0 && intVal <= 255)
					{
						fontEdit->setForeground(intVal);
						parent->paint();
					}
					else
						parent->okMessage(color_error_msg);
				}
				else
					parent->okMessage(color_error_msg);
				refresh();
			}
			else
				parent->okMessage("Foreground can only be changed on a mono font.");
			break;
		}
	
		case ID_SET_BACKGROUND:		
		{
			if (fontInfo.flags & FONT_MONO)
			{
				char buffer[1024];
				int  intVal;
				GetWindowText(hWndFontBackground, buffer, 1024);
				if (sscanf(buffer, "%d", &intVal) == 1)
				{
					if (intVal >= 0 && intVal <= 255)
					{
						fontEdit->setBackground(intVal);
						parent->paint();
					}
					else
						parent->okMessage(color_error_msg);
				}
				else
					parent->okMessage(color_error_msg);
			}
			else
				parent->okMessage("Background can only be changed on a mono font.");
			break;
		}
				
		case ID_CB_TRANSPARENT: 
			fontEdit->setFlag(FONT_TRANSPARENT, checked);
			break;
		case ID_CB_TRANSLUCENT:
			if (fontEdit->pPal)
				fontEdit->setFlag(FONT_TRANSLUCENT, checked);
			else
			{
				fontEdit->setFlag(FONT_TRANSLUCENT, false);
				parent->okMessage("No palette is available for translucency.");
			}
			break;
		case ID_CB_MONO:
			fontEdit->setFlag(FONT_MONO, checked);
			break;
		case ID_CB_UNDERLINED:
			fontEdit->setFlag(FONT_UNDERLINED, checked);
			break;
		case ID_CB_SAVEPALETTE:
			if (!currentPalette)
			{
				savePalette = false;
				parent->okMessage("There is no palette loaded.");
			}
			else
				savePalette = checked;
			break;
		case ID_RB_PROPORTIONAL:
			fontEdit->setFlag(FONT_PROPORTIONAL, checked);
			fontEdit->setFlag(FONT_FIXED, !checked);
			break;
		case ID_RB_FIXED:
			fontEdit->setFlag(FONT_FIXED, checked);
			fontEdit->setFlag(FONT_PROPORTIONAL, !checked);
			break;
		default:
			GWWindow::onCommand(id, hWndCtl, codeNotify);
			return;
	}
	parent->refresh();
}

////////////////////////////////////////////////////////////////////////

void FontInfoWindow::move(int x, int y, int w, int h)
{
	MoveWindow(hWnd, x, y, w, h, true);
}

////////////////////////////////////////////////////////////////////////

void FontInfoWindow::onLButtonDown(BOOL, int x, int y, UINT)
{
		x,y;
}

////////////////////////////////////////////////////////////////////////

HWND FontInfoWindow::createCheckBox(int id, int x, int y, char *text)
{
   return(CreateWindow("BUTTON", text, 
	                   WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_TEXT,
		  	  		   x, y,
					   100, 20,
					   hWnd,
					   (HMENU)id,
					   hInstance,
					   NULL));
}

////////////////////////////////////////////////////////////////////////

void FontInfoWindow::refresh()
{
	char         buffer[1024];
	GFXFontInfo fontInfo;
	
	if (fontEdit)	
		fontEdit->getExtents(&fontInfo);
	else
		memset((void *)&fontInfo, 0, sizeof(fontInfo));

	SendMessage(hWndTransparent, 
	            BM_SETCHECK, 
	            (fontInfo.flags & FONT_TRANSPARENT) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(hWndTranslucent, 
	            BM_SETCHECK, 
	            (fontInfo.flags & FONT_TRANSLUCENT) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(hWndMono, 
	            BM_SETCHECK, 
	            (fontInfo.flags & FONT_MONO) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(hWndMono, 
	            BM_SETCHECK, 
	            (fontInfo.flags & FONT_MONO) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(hWndUnderlined, 
	            BM_SETCHECK, 
	            (fontInfo.flags & FONT_UNDERLINED) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(hWndSavePalette, BM_SETCHECK, 
	            savePalette ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(hWndProportional,
	            BM_SETCHECK,
				(fontInfo.flags & FONT_PROPORTIONAL) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(hWndFixed,
	            BM_SETCHECK,
				(fontInfo.flags & FONT_FIXED) ? BST_CHECKED : BST_UNCHECKED, 0);

	if (!fontEdit)
		return;

	if (fontEdit->pPal)
		SetWindowText(hWndPaletteIncl, "Yes");
	else
		SetWindowText(hWndPaletteIncl, "No    ");

	sprintf(buffer, "%d  ", fontEdit->getNumChars());
	SetWindowText(hWndFontCt, buffer);

	sprintf(buffer, "%d  ", fontEdit->getWidth());
	SetWindowText(hWndFontWidth, buffer);

	sprintf(buffer, "%d  ", fontEdit->getHeight());
	SetWindowText(hWndFontHeight, buffer);

	sprintf(buffer, "%d   ", fontEdit->getBaseline());
	SetWindowText(hWndFontBaseline, buffer);

	if (fontInfo.flags & FONT_MONO)
	{
		sprintf(buffer, "%d  ", fontInfo.foreColor);
		SetWindowText(hWndFontForeground, buffer);

		sprintf(buffer, "%d  ", fontInfo.backColor);
		SetWindowText(hWndFontBackground, buffer);
		EnableWindow(hWndFontForegroundSet, true);
		EnableWindow(hWndFontForeground,    true);
		EnableWindow(hWndFontBackgroundSet, true);
		EnableWindow(hWndFontBackground,    true);
		}
	else
	{
		EnableWindow(hWndFontForegroundSet, false);
		EnableWindow(hWndFontForeground,    false);
		EnableWindow(hWndFontBackgroundSet, false);
		EnableWindow(hWndFontBackground,    false);
		SetWindowText(hWndFontForeground, "n/a  ");
		SetWindowText(hWndFontBackground, "n/a  ");
	}

	if (fontInfo.flags & FONT_FIXED)
	{
		EnableWindow(hWndFontSpacing, false);
		EnableWindow(hWndFontSpacingSet, false);
		SetWindowText(hWndFontSpacing, "n/a  ");
	}
	else
	{
		EnableWindow(hWndFontSpacing, true);
		EnableWindow(hWndFontSpacingSet, true);
		sprintf(buffer, "%d  ", fontEdit->getSpacing());
		SetWindowText(hWndFontSpacing, buffer);
	}
}

////////////////////////////////////////////////////////////////////////

void FontInfoWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	parent->onKey(vk, fDown, cRepeat, flags);
}

