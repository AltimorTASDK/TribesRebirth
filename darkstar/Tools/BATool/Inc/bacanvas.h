//
// bacanvas.h
//

#ifndef _BACANVAS_H_
#define _BACANVAS_H_

class BitmapArrayWindow;

class BitmapArrayCanvas : public GWCanvas
{
  public:
	Bool               viewOneBitmap;
	int                viewSize;
	int                offset;
	int                selectedIndex;
	GFXPalette        *palette;
	BitmapList        *bitmaps;
	BitmapArrayWindow *parentWindow;

	BitmapArrayCanvas();
	~BitmapArrayCanvas();
	void setPalette(GFXPalette *p);
	void render();
	void unlock();
	void drawIndices();
	void onLButtonDown(BOOL, int x, int y, UINT);  
	int  findItem(int x, int y);
	void setClientSize(Point2I size);
};

#endif