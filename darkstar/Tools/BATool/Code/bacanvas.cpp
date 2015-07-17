//
// bacanvas.cpp
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
#include "g_bitmap.h"
#include "g_barray.h"
#include "g_surfac.h"

////////////////////////////////////////////////////////////////////////

BitmapArrayCanvas::BitmapArrayCanvas() : GWCanvas()
{
	parentWindow = NULL;
	palette      = NULL;
	bitmaps      = NULL;
}

////////////////////////////////////////////////////////////////////////

BitmapArrayCanvas::~BitmapArrayCanvas()
{
	if (palette) delete palette;
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayCanvas::setPalette(GFXPalette *p)
{
	if (palette) delete palette;
	palette = p;
	getSurface()->setPalette(palette, false);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayCanvas::render()
{
	// Draw the bitmaps on the surface

	getSurface()->clear(7);
	if (!bitmaps || (bitmaps->count == 0))
		return;

	int        baseline;
	float      scale = viewSize/100.0;
	Point2I    pos, dim, size, clientSize = getClientSize();
	GFXBitmap *bitmap;

	size.x   = bitmaps->width *scale;
	size.y   = bitmaps->height*scale;
	baseline = clientSize.y/2 - size.y/2 + size.y;

	if (viewOneBitmap)	
	{
		// draw one bitmap in the center of the window
		bitmaps->SetCurrent(offset);
		bitmap = bitmaps->GetCurrent();
		dim.x  = bitmap->width *scale;
		dim.y  = bitmap->height*scale;
		pos.x  = clientSize.x/2 - dim.x/2; //size.x/2;
		pos.y  = baseline - dim.y;
		getSurface()->drawBitmap2d(bitmap, &pos, &dim);
		if (offset == selectedIndex)
		{
			// draw selection box
			Point2I ul(clientSize.x/2 - size.x/2, clientSize.y/2 - size.y/2);
			getSurface()->drawRect2d(&RectI(ul.x-3, ul.y-3, ul.x + size.x+2, ul.y + size.y+2), 0);
			getSurface()->drawRect2d(&RectI(ul.x-4, ul.y-4, ul.x + size.x+3, ul.y + size.y+3), 0);
			getSurface()->drawRect2d(&RectI(ul.x-5, ul.y-5, ul.x + size.x+4, ul.y + size.y+4), 0);
		}
	}
	else
	{
		// draw multiple bitmaps accross the window
		int nextXpos = (-offset) + BITMAP_MARGIN;
		bitmaps->Rewind();
		for (int i = 0; i < bitmaps->count; i++)
		{
			bitmap = bitmaps->GetCurrent();
			dim.x = bitmap->width *scale;
			dim.y = bitmap->height*scale;
			pos.x = nextXpos + (size.x/2 - dim.x/2);
			pos.y = baseline - dim.y;
			if ((pos.x < clientSize.x) && (pos.x + size.x > 0))
			{
				getSurface()->drawBitmap2d(bitmap, &pos, &dim);
				if (i == selectedIndex)
				{
					// draw selection box
					Point2I ul(nextXpos, baseline - size.y);
					getSurface()->drawRect2d(&RectI(ul.x-3, ul.y-3, ul.x + size.x+2, ul.y + size.y+2), 0);
					getSurface()->drawRect2d(&RectI(ul.x-4, ul.y-4, ul.x + size.x+3, ul.y + size.y+3), 0);
					getSurface()->drawRect2d(&RectI(ul.x-5, ul.y-5, ul.x + size.x+4, ul.y + size.y+4), 0);
				}
			}
			bitmaps->Skip(); 
			nextXpos += (size.x + BITMAP_MARGIN);
		}
	}
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayCanvas::drawIndices()
{
	// Draw the bitmap indices on the surface, unfortunately this has
	// to be done separately from drawing the actual bitmaps due to
	// TextOut() needing to happen *after* the surface has exposed the
	// back buffer.

	if (!bitmaps || (bitmaps->count == 0))
		return;

	char buffer[32];

	int        baseline, textline;
	float      scale = viewSize/100.0;
	Point2I    size, clientSize = getClientSize();
	GFXBitmap *bitmap;

	size.x   = bitmaps->width *scale;
	size.y   = bitmaps->height*scale;
	baseline = clientSize.y/2 - size.y/2 + size.y;
	textline = min(baseline + 14, clientSize.y - 30);

	if (viewOneBitmap)	
	{
		// draw one index for the bitmap in the center of the window
		sprintf(buffer, "%d", offset);
		TextOut(getSurface()->getDC(), clientSize.x/2, textline, buffer, strlen(buffer));
	}
	else
	{
		// draw multiple indices accross the window
		int nextXpos = (-offset) + BITMAP_MARGIN, posX;
		for (int i = 0; i < bitmaps->count; i++)
		{
			posX = nextXpos + size.x/2;
			if (posX < clientSize.x && posX > 0)
			{
				sprintf(buffer, "%d", i);
				TextOut(getSurface()->getDC(), posX, textline, buffer, strlen(buffer));
			}
			nextXpos += (size.x + BITMAP_MARGIN);
		}
	}	
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayCanvas::unlock()
{
  	GWCanvas::unlock();
	getSurface()->lockDC();
	drawIndices();
	getSurface()->unlockDC();
}	

////////////////////////////////////////////////////////////////////////

int BitmapArrayCanvas::findItem(int x, int y)
{
	// Search for a bitmap that lies at (x,y) on the currently rendered
	// surface, used for selecting by mouseclick.

	if (!bitmaps || (bitmaps->count == 0))
		return(-1);

	char buffer[32];

	int        baseline, y0;
	float      scale = viewSize/100.0;
	Point2I    size, clientSize = getClientSize();
	GFXBitmap *bitmap;

	size.x   = bitmaps->width *scale;
	size.y   = bitmaps->height*scale;
	baseline = clientSize.y/2 - size.y/2 + size.y;
	y0       = baseline - size.y;
	
	if (viewOneBitmap)	
	{
		// search the one bitmap on the screen
		if (inBoundingBox(x, y,
		                 clientSize.x/2 - size.x/2, y0,
		                 clientSize.x/2 + size.x/2, baseline))
			return(offset);
	}
	else
	{
		// search the multiple bitmaps accross the screen
		int nextXpos = (-offset) + BITMAP_MARGIN, posX;
		for (int i = 0; i < bitmaps->count; i++)
		{
			if (inBoundingBox(x, y,
			                  nextXpos, y0,
							  nextXpos + size.x, baseline))
				return(i);
			nextXpos += (size.x + BITMAP_MARGIN);
		}
	}	
	return(-1);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayCanvas::onLButtonDown(BOOL, int x, int y, UINT)
{
	if (parentWindow)
		parentWindow->leftButtonDown(x, y);
}

////////////////////////////////////////////////////////////////////////

void BitmapArrayCanvas::setClientSize(Point2I size)
{
	// if you minimize the window, size.x can be negative, and a
	// error of type "GFXCDSSurface::create Invalid width or height"
	// will occur.

	if (size.x > 0 && size.y > 0)
		GWCanvas::setClientSize(size);
}