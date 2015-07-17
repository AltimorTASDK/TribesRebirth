//
//  editWin.cpp
//

#include <gw.h>
#include <commdlg.h>
#include "g_fontedit.h"
#include "extras.h"
#include "wintools.h"
#include "defines.h"
#include "bmlist.h"
#include "editWin.h"
#include "fewindow.h"

void drawShadowRect(GFXSurface *s, RectI *r)
{
	s->drawRect2d(&RectI(r->left()-1, r->top()-1, r->right()+1, r->bottom()+1), 248);
	s->drawRect2d(&RectI(r->left()-1, r->top()-1, r->right(), r->bottom()), 255);
	s->drawRect2d_f(r, 7);
}

////////////////////////////////////////////////////////////////////////

static bool pointInRect(int x, int y, RectI r)
{
	if (x >= r.left() && x <= r.right() && y >= r.top() && y <= r.bottom())
		return(true);
	else
		return(false);
}

////////////////////////////////////////////////////////////////////////

FontEditWindow::FontEditWindow(FontEditMainWindow *p)
{
	grabbed           = Grab_None;
	parent            = p;
	fontEdit          = NULL;
	selectedASCII     = -1;
	lastFontCount     = LARGE_INT;
	lastSelectedASCII = LARGE_INT;
	magPosition       = 2;
}

////////////////////////////////////////////////////////////////////////

FontEditWindow::~FontEditWindow()
{
}

////////////////////////////////////////////////////////////////////////

bool FontEditWindow::create(Point2I pos, Point2I size)
{
	if (createMDIChild(NULL, "Edit", 
                       WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | WS_THICKFRAME | WS_CAPTION, 
                       pos, 
                       size,
                       parent,
		               0))
	{
		refreshVScrollbar();
		return(true);
	}
	else
		return(false);
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::setFont(GFXFontEdit *f)
{
	fontEdit = f;
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::render()
{
	getSurface()->clear(7);
	if ((selectedASCII == -1) || (!fontEdit) || (fontEdit->getNumChars() < 1))
		return;

	const GFXBitmap *bitmap;
   RectI inSubRegion;

	fontEdit->getCharInfo(selectedASCII, &bitmap, &inSubRegion, &currentCharInfo);

	Point2I clientSize = getClientSize();
	Point2I scale, pos;

	int iScale       = 1 << (magPosition - 2);
	int scaledWidth  = bitmap->width*iScale;
	int scaledHeight = fontEdit->getHeight()*iScale;
	scale.x = 1 << (14 + magPosition);
	scale.y = scale.x;

	fontEdit->setScale(&scale);
	pos.x = clientSize.x/2 - scaledWidth/2;
	pos.y = clientSize.y/2 - scaledHeight/2;

	int fontBaseline = pos.y + iScale*fontEdit->getBaseline();
	int charBaseline = fontBaseline + iScale*currentCharInfo->baseline;

	fontEdit->drawChar(getSurface(), selectedASCII, &pos);
	scale.x = scale.y = 1<<16;
	fontEdit->setScale(&scale);

	if (magPosition < 2)
		return;

	// draw scaffolding
	getSurface()->drawLine2d(&Point2I(0,fontBaseline),
	                     &Point2I(clientSize.x,fontBaseline), 0);
	getSurface()->drawLine2d(&Point2I(0,charBaseline),
	                     &Point2I(clientSize.x,charBaseline), 0);
	fontBaselineRect.upperL.x = clientSize.x - 30;
	fontBaselineRect.upperL.y = fontBaseline - 4;
	fontBaselineRect.lowerR.x = clientSize.x - 21;
	fontBaselineRect.lowerR.y = fontBaseline + 4;
	charBaselineRect.upperL.x = pos.x + scaledWidth + 10;
	charBaselineRect.upperL.y = charBaseline - 4;
	charBaselineRect.lowerR.x = pos.x + scaledWidth + 19;
	charBaselineRect.lowerR.y = charBaseline + 4;
	drawShadowRect(getSurface(), &fontBaselineRect);
	drawShadowRect(getSurface(), &charBaselineRect);
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::onMouseMove(int, int y, UINT)
{
	if (grabbed == Grab_None)
		return;

	if (y < 15)
		y = 15;
	else if (y > getClientSize().y - 15)
		y = getClientSize().y - 15;

	int deltaY = (mouseDown.y - y) >> (magPosition - 2);

	switch (grabbed)
	{
		case Grab_Font_Base:
		{
			int newBaseline = baselineMouseDownY - deltaY;
			if (newBaseline != fontEdit->getBaseline())
			{
				fontEdit->setBaseline(newBaseline);
				parent->refresh();
			}
			break;
		}
		case Grab_Char_Base:
		{
			int newBaseline = baselineMouseDownY - deltaY;
			if (newBaseline != currentCharInfo->baseline)
			{
				currentCharInfo->baseline = newBaseline;
				parent->refresh();
			}
			break;
		}
		case Grab_Hot_X:
			break;
		case Grab_Hot_Y:
			break;
		case Grab_None:
//			FORWARD_WM_MOUSEMOVE( x, y, keyFlags, defWindowProc);
			break;
	}	
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::onLButtonDown(BOOL, int x, int y, UINT)
{
	if (magPosition < 2)
		// dont' grab anything if we're less than 100%
		return;

	mouseDown.x = x;
	mouseDown.y = y;

	if (pointInRect(x, y, fontBaselineRect))
	{
		baselineMouseDownY = fontEdit->getBaseline();
		grabbed = Grab_Font_Base;
	}
	else if (pointInRect(x, y, charBaselineRect))
	{
		baselineMouseDownY = currentCharInfo->baseline;
		grabbed = Grab_Char_Base;
	}
	else		
;//		FORWARD_WM_LBUTTONDOWN( fDoubleClick, x, y, keyFlags, defWindowProc);
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::onLButtonUp(int, int, UINT)
{
	if (grabbed != Grab_None)
		grabbed = Grab_None;
	else
;//		FORWARD_WM_LBUTTONUP( x, y, keyFlags, defWindowProc);
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::onHScroll(HWND, UINT code, int pos)
{
	switch (code)
	{
		case SCROLLBAR_GO_TOP: 
			parent->setSelectedIth(1);
			break;
		case SCROLLBAR_GO_BOTTOM: 
			parent->setSelectedIth(LARGE_INT);
			break;

		case SB_LEFT:             
		case SB_LINELEFT:         
		case SB_PAGELEFT:
			parent->adjustSelectedIth(-1);
			break;

		case SB_RIGHT:            
		case SB_LINERIGHT:        
		case SB_PAGERIGHT:
			parent->adjustSelectedIth(1);
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION: 
			parent->setSelectedIth(pos + 1);
			break;

		default:
			break;
	}
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::onVScroll(HWND, UINT code, int pos)
{
	switch (code)
	{
		case SCROLLBAR_GO_TOP: 
			magPosition = 0;
			break;
		case SCROLLBAR_GO_BOTTOM: 
			magPosition = 6;
			break;

		case SB_LEFT:             
		case SB_LINELEFT:         
		case SB_PAGELEFT:
			magPosition = max(0, magPosition - 1);
			break;

		case SB_RIGHT:            
		case SB_LINERIGHT:        
		case SB_PAGERIGHT:
			magPosition = min(6, magPosition + 1);
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION: 
			magPosition = pos;
			break;

		default:
			break;
	}
	refreshVScrollbar();
	paint();
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::setSelectedASCII(int newselectedASCII)
{
	selectedASCII = newselectedASCII;
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::notifyOfSelectedIth(int i)
{
	selectedIth = i;
	refreshHScrollbar();
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::refreshHScrollbar()
{
	int numChars;
	if (fontEdit)
		numChars = fontEdit->getNumChars();
	else
		numChars = 0;

	SCROLLINFO scrollInfo;

	memset((void *)&scrollInfo, 0, sizeof(scrollInfo));
	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask  = SIF_POS;

	if (lastFontCount != numChars)
	{
		lastFontCount     = numChars;
		scrollInfo.fMask |= (SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL);
		scrollInfo.nMin   = 0;
		scrollInfo.nPage  = 1;

		if (numChars == 0)
			scrollInfo.nMax = 0;
		else
			scrollInfo.nMax = numChars - 1;
	}
	if (selectedIth == -1)
		scrollInfo.nPos = 0;
	else
		scrollInfo.nPos = selectedIth;
	SetScrollInfo(hWnd, SB_HORZ, &scrollInfo, true);
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::refreshVScrollbar()
{
	SCROLLINFO scrollInfo;
	memset((void *)&scrollInfo, 0, sizeof(scrollInfo));
	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	scrollInfo.nMin  = 0;
	scrollInfo.nMax  = 6;
	scrollInfo.nPage = 1;
	scrollInfo.nPos  = magPosition;
	SetScrollInfo(hWnd, SB_VERT, &scrollInfo, true);
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::move(int x, int y, int w, int h)
{
	MoveWindow(hWnd, x, y, w, h, true);
}

////////////////////////////////////////////////////////////////////////

void FontEditWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	parent->onKey(vk, fDown, cRepeat, flags);
}


