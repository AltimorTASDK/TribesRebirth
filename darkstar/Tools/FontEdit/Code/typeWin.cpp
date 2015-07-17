//			   
//  typeWin.cpp
//

#include <gw.h>
#include <commdlg.h>
#include <fn_all.h>
#include "g_fontedit.h"
#include "extras.h"
#include "wintools.h"
#include "defines.h"
#include "bmlist.h"
#include "typeWin.h"
#include "fewindow.h"

#define TEXT_ "The Quick Brown Fox..."

////////////////////////////////////////////////////////////////////////

FontTypingWindow::FontTypingWindow(FontEditMainWindow *p)
{
	parent        = p;
	fontEdit      = NULL;
	strcpy(typeBuffer, TEXT_);
}

////////////////////////////////////////////////////////////////////////

FontTypingWindow::~FontTypingWindow()
{
}

////////////////////////////////////////////////////////////////////////

bool FontTypingWindow::create(Point2I pos, Point2I size)
{
	return(createMDIChild(NULL, "Preview",
                          WS_VISIBLE | WS_CAPTION | WS_THICKFRAME,
                          pos,
                          size,
                          parent,
                          0));
}

////////////////////////////////////////////////////////////////////////

void FontTypingWindow::render()
{
	getSurface()->clear(7);
	if (!fontEdit || (fontEdit->getNumChars() < 1))
		return;

	int      endPos  = strlen(typeBuffer);
	int      justify = fontEdit->getJustification();
	char    *front, *next;
	Point2I  pos(0, 0);
	RectI    rect(0, 0, getClientSize().x-1, getClientSize().y-1);

	typeBuffer[endPos]     = '_';
	typeBuffer[endPos + 1] = '\0';
	
	front = typeBuffer;

	while (*front)
	{
		// find the end of the current line
		next = front;
		while (*next != 0x0D && *next != 0x00)
			*next++;

		// line ended in a carriabe return
		if (*next == 0x0D)
		{
			*next = 0x00;
			if (justify)
			{
				// draw only the first line, otherwise the next line
				// will clobber
				GFXDrawText_r(getSurface(), fontEdit, &rect, front);
				break;
			}
			else
				GFXDrawText_p(getSurface(), fontEdit, &pos, front);
			*next = 0x0D;
			pos.y += fontEdit->getHeight() + 5;
		}
		else
		{
			// no more text, draw this and get out
			if (justify)
				GFXDrawText_r(getSurface(), fontEdit, &rect, front);
			else
				GFXDrawText_p(getSurface(), fontEdit, &pos, front);
			break;
		}

		front = next + 1;
	}
	typeBuffer[endPos] = '\0';
}

////////////////////////////////////////////////////////////////////////

void FontTypingWindow::move(int x, int y, int w, int h)
{
	MoveWindow(hWnd, x, y, w, h, true);
}

////////////////////////////////////////////////////////////////////////

void FontTypingWindow::setFont(GFXFontEdit *f)
{
	fontEdit = f;
}

////////////////////////////////////////////////////////////////////////

void FontTypingWindow::onLButtonDown(BOOL, int x, int y, UINT)
{
	x,y;
}

////////////////////////////////////////////////////////////////////////

void FontTypingWindow::append(TCHAR ch, int, UINT)
{
	int pos = strlen(typeBuffer);

	if (ch == 0x08)
	{
		if (pos > 0)
		{
			typeBuffer[pos - 1] = '\0';
			paint();
		}
	}
	else
	{
		if (pos < TYPE_BUFFER_SIZE - 10)
		{
			typeBuffer[pos]     = char(ch);
			typeBuffer[pos + 1] = '\0';
			paint();
		}
	}
}

////////////////////////////////////////////////////////////////////////

void FontTypingWindow::clearText()
{
	strcpy(typeBuffer, "");
	paint();
}

////////////////////////////////////////////////////////////////////////

void FontTypingWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	parent->onKey(vk, fDown, cRepeat, flags);
}


