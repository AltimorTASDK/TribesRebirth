//
//  cInfoWin.h
//

#ifndef _CINFOWIN_H_
#define _CINFOWIN_H_

class FontEditMainWindow;

class CharInfoWindow : public GWWindow
{
  private:
	bool                savePalette;
	int                 selectedASCII;
	GFXFontEdit        *fontEdit;
    FontEditMainWindow *parent;

	HWND hWndASCII;
	HWND hWndASCIISet;
	HWND hWndBaseline;
	HWND hWndBaselineSet;
	HWND hWndWidth;
	HWND hWndWidthSet;
	HWND hWndHeight;

	HWND createGet(int id, int x, int y, int w, int h);
		
  public:
    CharInfoWindow(FontEditMainWindow *p);
    ~CharInfoWindow();

	HWND createCheckBox(int id, int x, int y, char *text);
	void refresh();

	bool create(Point2I pos, Point2I size);
	void move(int x, int y, int w, int h);
	void setSelectedASCII(int newSelected);
	void setFont(GFXFontEdit *f);
	void onLButtonDown(BOOL, int x, int y, UINT);
	void onCommand(int id, HWND hWndCtl, UINT codeNotify);
	void setASCII32Width();
	void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
};

#endif
