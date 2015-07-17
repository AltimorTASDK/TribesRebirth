//
//  viewWin.h
//

#ifndef _VIEWWIN_H_
#define _VIEWWIN_H_

#include <gwCanvas.h>
#include <g_surfac.h>

class FontEditMainWindow;

class FontViewWindow : public GWCanvas
{
  private:
	int                 selectedIndex;
	int                 columns, rows, viewVerticalSize;
	int                 vScrollbarMax, vScrollbarPos, vScrollbarPage;
	GFXFontEdit        *fontEdit;    
    FontEditMainWindow *parent;
		
  public:
    FontViewWindow(FontEditMainWindow *p);
    ~FontViewWindow();

	bool create(Point2I pos, Point2I size);
	void setPalette(GFXPalette *p) { getSurface()->setPalette(p, false); }
	void figureSizes();
	void setSelectedIndex(int newSelectedIndex);
	void move(int x, int y, int w, int h);
	void setFont(GFXFontEdit *f);
	
	void refreshVScrollbar();
	
	void render();
	void onLButtonDown(BOOL, int x, int y, UINT);
	void onVScroll(HWND, UINT code, int pos);
	void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);

//	void onSize(UINT, int, int);
};

#endif
