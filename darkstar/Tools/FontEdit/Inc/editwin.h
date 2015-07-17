//
//  editWin.h
//

#ifndef _EDITWIN_H_
#define _EDITWIN_H_

#include <gwCanvas.h>
#include <g_surfac.h>
#include <g_bitmap.h>
#include <g_pal.h>

class FontEditMainWindow;

class FontEditWindow : public GWCanvas
{
  private:
	enum 
	{ 
		Grab_None, 
	    Grab_Font_Base, 
	    Grab_Char_Base, 
	    Grab_Hot_X, 
	    Grab_Hot_Y 
	};

	int                 grabbed;
	int                 selectedASCII, lastSelectedASCII, selectedIth;
	int                 lastFontCount;
	int                 magPosition, baselineMouseDownY;
	Point2I             mouseDown;
	RectI               fontBaselineRect, charBaselineRect;
	GFXCharInfo    *currentCharInfo;
	GFXFontEdit        *fontEdit;
    FontEditMainWindow *parent;
		
  public:
    FontEditWindow(FontEditMainWindow *p);
    ~FontEditWindow();
	bool create(Point2I pos, Point2I size);

	void setSelectedASCII(int newSelectedASCII);
	void notifyOfSelectedIth(int i);

	void setPalette(GFXPalette *p) { getSurface()->setPalette(p, false); }
	void refreshHScrollbar();
	void refreshVScrollbar();
	void move(int x, int y, int w, int h);
	void setFont(GFXFontEdit *f);
	void onHScroll(HWND, UINT code, int pos);
	void onVScroll(HWND, UINT code, int pos);
	void onLButtonUp(int x, int y, UINT keyFlags);
	void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void onMouseMove(int x, int y, UINT keyFlags);
	void render();

	HDC  getDC() { return(GetDC(hWnd)); }
	void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
};

#endif
