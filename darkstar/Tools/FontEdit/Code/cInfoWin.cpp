//			   
//  cInfoWin.cpp
//

#include <gw.h>
#include <commdlg.h>
#include "g_fontedit.h"
#include "extras.h"
#include "wintools.h"
#include "defines.h"
#include "bmlist.h"
#include "cInfoWin.h"
#include "fewindow.h"

#define ITEM_COLUMN 100
#define ITEM_WIDTH  90

#define ID_GET_ASCII    4001
#define ID_GET_BASELINE 4002
#define ID_GET_WIDTH    4003
#define ID_SET_ASCII    5001
#define ID_SET_WIDTH    5003
#define ID_SET_BASELINE 5002

////////////////////////////////////////////////////////////////////////

CharInfoWindow::CharInfoWindow(FontEditMainWindow *p)
{
	selectedASCII = -1;
	savePalette   = true;
	parent        = p;
	fontEdit      = NULL;
}

////////////////////////////////////////////////////////////////////////

CharInfoWindow::~CharInfoWindow()
{
}

////////////////////////////////////////////////////////////////////////

void CharInfoWindow::setFont(GFXFontEdit *f)
{
	fontEdit = f;
}

////////////////////////////////////////////////////////////////////////

void CharInfoWindow::setSelectedASCII(int newSelected)
{
	selectedASCII = newSelected;
	refresh();
}

////////////////////////////////////////////////////////////////////////

HWND CharInfoWindow::createGet(int id, int x, int y, int w, int h)
{
	CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE| SS_BLACKFRAME,
                 x, y, w, h, hWnd, (HMENU)0, hInstance, NULL);
		
	return(CreateWindow("EDIT", "", WS_TABSTOP | WS_CHILD | WS_VISIBLE,
	                    x+2, y+4, w-4, h-6, hWnd, (HMENU)id, hInstance, NULL));
}		

////////////////////////////////////////////////////////////////////////

bool CharInfoWindow::create(Point2I pos, Point2I size)
{
	if (!createMDIChild(NULL,
                        "Character Info",
                        WS_CAPTION | WS_VISIBLE,
                        pos,
                        size,
                        parent,
                        0))
		return(false);
	
	CreateWindow("STATIC", "ASCII:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 5, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	CreateWindow("STATIC", "Baseline:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 30, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	CreateWindow("STATIC", "Width:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 80, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	CreateWindow("STATIC", "Height:",
	             WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				 10, 105, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);

	hWndASCII    = createGet(ID_GET_ASCII, ITEM_COLUMN, 5,  70, 20);
	hWndASCIISet = CreateWindow("Button", "Set",
	                            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				        		ITEM_COLUMN + 75, 5, 30, 20,
						  		hWnd, (HMENU)ID_SET_ASCII, hInstance, NULL);

	hWndBaseline = createGet(ID_GET_BASELINE, ITEM_COLUMN, 30, 70, 20);
	hWndASCIISet = CreateWindow("Button", "Set",
	                            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				        		ITEM_COLUMN + 75, 30, 30, 20,
						  		hWnd, (HMENU)ID_SET_BASELINE, hInstance, NULL);

#if 0
	hWndWidth    =	CreateWindow("STATIC", "",
                                 WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				                 ITEM_COLUMN, 80, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
#endif
	hWndWidth    = createGet(ID_GET_WIDTH, ITEM_COLUMN, 80, 70, 20);
	hWndWidthSet = CreateWindow("Button", "Set",
	                            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				        		ITEM_COLUMN + 75, 80, 30, 20,
						  		hWnd, (HMENU)ID_SET_WIDTH, hInstance, NULL);


	hWndHeight   = CreateWindow("STATIC", "",
	                            WS_CHILD | SS_SIMPLE | WS_VISIBLE,
				                ITEM_COLUMN, 105, ITEM_WIDTH, 20, hWnd, (HMENU)0, hInstance, NULL);
	return(true);
}

////////////////////////////////////////////////////////////////////////

void CharInfoWindow::setASCII32Width()
{
	if (selectedASCII == 32)
	{
		char buffer[1024];
		int  intVal;
		GetWindowText(hWndWidth, buffer, 1024);
	 	if (sscanf(buffer, "%d", &intVal) == 1)
		{
			if (!fontEdit->setASCII32Width(intVal))
				parent->okMessage("That is not a valid width for the space character.");
		}
		else
			parent->okMessage("That is not a valid width for the space character.");
	}
	parent->refresh();
}

////////////////////////////////////////////////////////////////////////

void CharInfoWindow::onCommand(int id, HWND hWndCtl, UINT codeNotify)
{
	char buffer[1024];
	int  intVal;

	switch (id)
	{
		case ID_SET_WIDTH:
			setASCII32Width();
			break;

		case ID_SET_ASCII:
		{
			GetWindowText(hWndASCII, buffer, 1024);
			if (sscanf(buffer, "%d", &intVal) == 1)
			{
				if (fontEdit->changeASCII(selectedASCII, intVal))
					parent->setSelected(intVal);
				else
				{
					parent->okMessage("That ASCII code is being used already.");
					refresh();
				}
			}
			else
			{
				parent->okMessage("That is not a valid ASCII code.");
				refresh();
			}
			break;
		}

		case ID_SET_BASELINE:
			GetWindowText(hWndBaseline, buffer, 1024);
			if (sscanf(buffer, "%d", &intVal) == 1)
			{
				if ((intVal > -128) && (intVal < 128))
				{
					fontEdit->setBaseline(selectedASCII, intVal);
					parent->paint();
				}
				else
					parent->okMessage("Valid baseline values are -127 to 127.");
			}
			else
				parent->okMessage("That is not a valid baseline value.");
			refresh();
			break;

		default:
			GWWindow::onCommand(id, hWndCtl, codeNotify);
			break;
	}
}

////////////////////////////////////////////////////////////////////////

void CharInfoWindow::move(int x, int y, int w, int h)
{
	MoveWindow(hWnd, x, y, w, h, true);
}

////////////////////////////////////////////////////////////////////////

void CharInfoWindow::onLButtonDown(BOOL, int x, int y, UINT)
{
	x,y;
}

////////////////////////////////////////////////////////////////////////

HWND CharInfoWindow::createCheckBox(int id, int x, int y, char *text)
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

void CharInfoWindow::refresh()
{
	if ((selectedASCII == -1) || (!fontEdit) || (fontEdit->getNumChars() < 1))	
		return;

	if (selectedASCII == 32)
	{
		EnableWindow(hWndWidthSet, true);
		EnableWindow(hWndWidth, true);
	}
	else
	{
		EnableWindow(hWndWidthSet, false);
		EnableWindow(hWndWidth, false);
	}

	char buffer[1024];
	GFXCharInfo *charInfo = fontEdit->getFontCharInfoOnly(selectedASCII);

	sprintf(buffer, "%d", selectedASCII);
	SetWindowText(hWndASCII, buffer);

	sprintf(buffer, "%d", charInfo->baseline);
	SetWindowText(hWndBaseline, buffer);

	sprintf(buffer, "%d  ", charInfo->width);
	SetWindowText(hWndWidth, buffer);

	sprintf(buffer, "%d  ", charInfo->height);
	SetWindowText(hWndHeight, buffer);
}

////////////////////////////////////////////////////////////////////////

void CharInfoWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	parent->onKey(vk, fDown, cRepeat, flags);
}

