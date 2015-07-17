//			   
//  viewWin.cpp
//

#include <gw.h>
#include <commdlg.h>
#include "g_fontedit.h"
#include "extras.h"
#include "wintools.h"
#include "defines.h"
#include "bmlist.h"
#include "viewWin.h"
#include "fewindow.h"

#define CHAR_MARGIN 10

////////////////////////////////////////////////////////////////////////

FontViewWindow::FontViewWindow(FontEditMainWindow *p)
{
	selectedIndex    = -1;
	parent           = p;
	vScrollbarMax    = 0;
	vScrollbarPos    = 0;
	viewVerticalSize = 0;
	fontEdit         = NULL;
}

////////////////////////////////////////////////////////////////////////

void FontViewWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	parent->onKey(vk, fDown, cRepeat, flags);
}

////////////////////////////////////////////////////////////////////////
 
FontViewWindow::~FontViewWindow()
{
}

////////////////////////////////////////////////////////////////////////

void FontViewWindow::setFont(GFXFontEdit *f)
{
	fontEdit = f;
}

////////////////////////////////////////////////////////////////////////

bool FontViewWindow::create(Point2I pos, Point2I size)
{
	return(createMDIChild( NULL,"View",
                          WS_THICKFRAME | WS_VSCROLL | WS_CAPTION | WS_VISIBLE,     
                          pos,
                          size,
                          parent,
                          0));
}

////////////////////////////////////////////////////////////////////////

void FontViewWindow::figureSizes()
{
	if (!fontEdit)
		return;

	Point2I clientSize = getClientSize();

	columns = clientSize.x/(fontEdit->getWidth() + CHAR_MARGIN);
	if (columns < 1) columns = 1;

	rows = fontEdit->getNumChars()/columns;
	if (fontEdit->getNumChars() % columns != 0)
		rows++;
	viewVerticalSize = CHAR_MARGIN + (rows + 1)*(fontEdit->heightMetric() + CHAR_MARGIN);
	vScrollbarPage   = fontEdit->heightMetric() + CHAR_MARGIN;
	refreshVScrollbar();
}

////////////////////////////////////////////////////////////////////////

void FontViewWindow::render()
{
	getSurface()->clear(7);
	
	if ((!fontEdit) || (fontEdit->getNumChars() < 1))
		return;

	figureSizes();

	int columnNo = 0, count = 0, i;
	int width  = fontEdit->getWidth(); 
	int	height = fontEdit->heightMetric();
	int firstASCII = fontEdit->getFirstASCII();
	int lastASCII  = fontEdit->getLastASCII();
		Point2I pos;

	pos.y = -vScrollbarPos + CHAR_MARGIN/2;
	pos.x = CHAR_MARGIN/2;

	for (i = firstASCII; i <= lastASCII; i++)
	{
		if (fontEdit->getCharTableIndex(i) == -1)
			// elt. i is not in the char set
			continue;

		count++;

		if (columnNo == columns)
		{
			pos.y += CHAR_MARGIN + height;
			pos.x  = CHAR_MARGIN/2;
			columnNo = 0;
		}

		if (i == selectedIndex)
		{
			// tell the main window what the selected index is in count
			// terms (i is the ASCII code, count identifies this char
			// as the ith in the set)
			parent->notifyOfSelectedIth(count - 1);

			// draw selection box
			getSurface()->drawRect2d(&RectI(pos.x-4, pos.y-4, pos.x+width+4, pos.y+height+4), 0);
			getSurface()->drawRect2d(&RectI(pos.x-3, pos.y-3, pos.x+width+3, pos.y+height+3), 0);
			getSurface()->drawRect2d(&RectI(pos.x-2, pos.y-2, pos.x+width+2, pos.y+height+2), 0);
		}

		fontEdit->drawChar(getSurface(), i, &pos);
		pos.x += CHAR_MARGIN + width;
		columnNo++;
	}
}

////////////////////////////////////////////////////////////////////////

void FontViewWindow::setSelectedIndex(int newSelectedIndex)
{
	selectedIndex = newSelectedIndex;
}

////////////////////////////////////////////////////////////////////////

void FontViewWindow::move(int x, int y, int w, int h)
{
	MoveWindow(hWnd, x, y, w, h, true);
}

////////////////////////////////////////////////////////////////////////

void FontViewWindow::onLButtonDown(BOOL, int x, int y, UINT)
{
	if ((!fontEdit) || (fontEdit->getNumChars() < 1))
		return;

	int columnNo   = 0, i;
	int width      = fontEdit->getWidth();
	int height     = fontEdit->heightMetric();
	int firstASCII = fontEdit->getFirstASCII();
	int lastASCII  = fontEdit->getLastASCII();
	Point2I pos;

	pos.y = -vScrollbarPos + CHAR_MARGIN/2;
	pos.x = CHAR_MARGIN/2;

	for (i = firstASCII; i <= lastASCII; i++)
	{
		if (fontEdit->getCharTableIndex(i) == -1)
			// elt. i is not in the char set
			continue;

		if (columnNo == columns)
		{
			pos.y += CHAR_MARGIN + height;
			pos.x  = CHAR_MARGIN/2;
			columnNo = 0;
		}
		
		if ((x >= pos.x) && (x <= pos.x + width) && (y >= pos.y) && (y <= pos.y + height))
			parent->setSelected(i);
		columnNo++;
		pos.x += CHAR_MARGIN + width;
	}
}

////////////////////////////////////////////////////////////////////////

void FontViewWindow::refreshVScrollbar()
{
	int scrollbarMax = viewVerticalSize - getClientSize().y;

	vScrollbarMax = max(0, scrollbarMax);
	vScrollbarPos = min(vScrollbarMax, vScrollbarPos);

	SCROLLINFO scrollInfo;
	memset((void *)&scrollInfo, 0, sizeof(scrollInfo));
	scrollInfo.cbSize = sizeof(scrollInfo);

	scrollInfo.fMask = scrollInfo.fMask | SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL | SIF_POS;
	scrollInfo.nMin  = 0;
	scrollInfo.nPage = vScrollbarPage; 
	if (scrollInfo.nPage < 1)
		scrollInfo.nPage = 1;
	scrollInfo.nMax  = vScrollbarMax;
	scrollInfo.nPos  = vScrollbarPos;
	SetScrollInfo(hWnd, SB_VERT, &scrollInfo, true);
}

////////////////////////////////////////////////////////////////////////

void FontViewWindow::onVScroll(HWND, UINT code, int pos)
{
	switch (code)
	{
		case SCROLLBAR_GO_TOP:
			vScrollbarPos = 0; 
			break;
		case SCROLLBAR_GO_BOTTOM: 
			vScrollbarPos = vScrollbarMax;
			break;

		case SB_LEFT:             
		case SB_LINELEFT:         
		case SB_PAGELEFT:
			vScrollbarPos -= vScrollbarPage;
			vScrollbarPos = max(0, vScrollbarPos);
			break;

		case SB_RIGHT:            
		case SB_LINERIGHT:        
		case SB_PAGERIGHT:
			vScrollbarPos += vScrollbarPage;
			vScrollbarPos  = min(vScrollbarMax, vScrollbarPos);
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION: 
			vScrollbarPos = pos;
			break;

		default:
			break;
	}
	paint();
}
