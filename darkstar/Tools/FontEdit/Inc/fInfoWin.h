//
//  fInfoWin.h
//

#ifndef _FINFOWIN_H_
#define _FINFOWIN_H_

class FontEditMainWindow;

class FontInfoWindow : public GWWindow
{
  private:
	GFXPalette         *currentPalette;
	GFXFontEdit        *fontEdit;
    FontEditMainWindow *parent;

	HWND hWndTransparent;
	HWND hWndTranslucent;
	HWND hWndMono;
	HWND hWndUnderlined;
	HWND hWndSavePalette;
	HWND hWndProportional;
	HWND hWndFixed;
	
	HWND hWndFontCt;
	HWND hWndFontHeight;
	HWND hWndFontWidth;
	HWND hWndFontBaseline;
	HWND hWndFontForeground;
	HWND hWndFontBackground;
	HWND hWndFontSpacing;
	HWND hWndPaletteIncl;

	HWND hWndFontForegroundSet;
	HWND hWndFontBackgroundSet;
	HWND hWndFontSpacingSet;
		
  public:
	bool savePalette;
    FontInfoWindow(FontEditMainWindow *p);
    ~FontInfoWindow();

	void setPalette(GFXPalette *p) { currentPalette = p; }
	HWND createCheckBox(int id, int x, int y, char *text);
	HWND createGet(int id, int x, int y, int w, int h);
	void refresh();

	bool create(Point2I pos, Point2I size);
	void move(int x, int y, int w, int h);
	void setFont(GFXFontEdit *f);

	void onLButtonDown(BOOL, int x, int y, UINT);
	void onCommand(int id, HWND hWndCtl, UINT codeNotify);
	void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
};

#endif


















								  