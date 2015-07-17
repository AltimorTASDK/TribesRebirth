//
//  bawindow.h
//

#ifndef _BAWINDOW_H_
#define _BAWINDOW_H_

class BitmapArrayWindow : public GWWindow
{
	typedef GWWindow Parent;

  private:
   Bool                 viewOneBitmap, controlKeyDown, stripPalette;
   bool                 transparent;
	char                 bitmapArrayFilename[1024];
	int			         scrollbarPosition, maxScrollbarPosition;
	int                  viewSize;
	int				      selectedIndex;
	HMENU                hMenu;
	HWND                 hScrollbar;
	GFXBitmap            *pasteBuffer;
	BitmapList           *bitmaps;
	BitmapArrayCanvas    *canvas;
		
  public:
    BitmapArrayWindow(char *cmdLine);
    ~BitmapArrayWindow();

	void newBitmapArray();
	void okMessage(char *s);
	void updateViewSize(int newSize);
	void refreshEditMenu();
	void refreshViewMenu();
	void refreshOptionsMenu();
	void positionScrollbar();
	void adjustScrollbar(UINT code, int pos);
	void leftButtonDown(int x, int y);

	void paint();

	void onDestroy();
    void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
	void onCommand(int id, HWND hwndCtl, UINT codeNotify);
	void onSize(UINT, int, int);
	void onHScroll(HWND hwndCtl, UINT code, int pos);

	void writeBitmapArray(char *filename);
	void save();
	void saveAs();
	void openBitmap();
	void openBitmapArray();
	void openPalette();
	void loadBitmap(char *filename);
	void loadBitmapArray(char *filename);
	void loadPalette(char *filename);
	void setTitle();

	void cut();
	void copy();
	void paste(GFXBitmap *insertBuffer = NULL);
	void append();
	void remove();
	void promote();
	void demote();

};

#endif
