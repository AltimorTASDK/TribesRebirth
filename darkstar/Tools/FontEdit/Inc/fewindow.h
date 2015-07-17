//
//  fewindow.h
//

#ifndef _FEWINDOW_H_
#define _FEWINDOW_H_


class GFXFontEdit;
class FontViewWindow;
class FontEditWindow;
class FontTypingWindow;
class CharInfoWindow;
class FontInfoWindow;

class FontEditMainWindow : public GWWindow
{
	typedef GWWindow Parent;

  private:
    Bool              controlKeyDown, stripPalette, autoTile;
	char              fontFilename[1024];
	int			      selectedASCII;
	HMENU             hMenu;
	int               pasteBufferBaseline;
	GFXBitmap        *pasteBuffer;
	GFXFontEdit      *fontEdit;
	GFXPalette       *currentPalette;

	GWWindow         *mdiClientWindow;
	FontViewWindow   *viewWindow;
	FontEditWindow   *editWindow;
	FontTypingWindow *typingWindow;
	CharInfoWindow   *charInfoWindow;
	FontInfoWindow   *fontInfoWindow;

   enum Constants {
      BMP_SHEET_WIDTH = 640,
      BMP_SHEET_SPACING = 10,
      BMP_SHEET_BORDER = 20,
   };
		
  public:
    FontEditMainWindow(char *cmdLine);
    ~FontEditMainWindow();

	void adjustSelected(int delta);
	void setSelected(int newIndex);
	void setSelectedIth(int i);
	void adjustSelectedIth(int i);
	void notifyOfSelectedIth(int i);

	void setPalette(GFXPalette *p);
	void setSubWindowSizes();
	void newFont();
	void newWindowsFont();
	void resetFont(GFXFontEdit *newFont);
	void okMessage(char *s);
	void refreshEditMenu();
	void refreshWindowMenu();
	void refreshPreviewMenu();

	void refresh();      // asks all windows to repaint
	void paint();        // asks only graphic windows to repaint

	void onDestroy();
   void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
   void onChar(TCHAR ch, int cRepeat, UINT flags);
	void onCommand(int id, HWND hwndCtl, UINT codeNotify);
	void onSize(UINT, int, int);

	void writeFont(char *filename);
	void save();
	void saveAs();
	void openBitmap();
	void openFont();
	void openPalette();
	void loadBitmap(char *filename);
	void loadFont(char *filename);
	Bool isOldFontFile(char *filename);
	void loadPalette(char *filename);
	void exportBitmapArray();
	void exportBitmapSheet();
   void importBitmapSheet();
   int  getBitmapSheetHeight();
   Bool writeBitmapSheet(char *filename);
   Bool readBitmapSheet(char *filename);
	void setTitle();

	void cut();
	void copy();
	void paste();
	void append();
	void remove();
	void promote();
	void demote();

};

#endif
