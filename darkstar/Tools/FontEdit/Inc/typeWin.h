//
//  typeWin.h
//

#ifndef _TYPEWIN_H_
#define _TYPEWIN_H_

#include <gwCanvas.h>
#include <g_surfac.h>

#define TYPE_BUFFER_SIZE 4096

class FontEditMainWindow;

class FontTypingWindow : public GWCanvas
{
  private:
	char                typeBuffer[TYPE_BUFFER_SIZE];
	GFXFontEdit        *fontEdit;
    FontEditMainWindow *parent;
		
  public:
    FontTypingWindow(FontEditMainWindow *p);
    ~FontTypingWindow();

	bool create(Point2I pos, Point2I size);
	void setPalette(GFXPalette *p) { getSurface()->setPalette(p, false); }
	void move(int x, int y, int w, int h);
	void setFont(GFXFontEdit *f);
	void render();
	void onLButtonDown(BOOL, int x, int y, UINT);
	void clearText();
	void append(TCHAR ch, int cRepeat, UINT flags);
	void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
};

#endif
