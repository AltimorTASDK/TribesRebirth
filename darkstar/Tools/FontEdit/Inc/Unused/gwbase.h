//----------------------------------------------------------------------------

//	$Workfile:   gwbase.h  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:00  $

//----------------------------------------------------------------------------

#ifndef _GWBASE_H_
#define _GWBASE_H_

#ifndef STRICT
	#define  STRICT
#endif
#include <windows.h>
#include <shellapi.h>
#pragma hdrstop

#include <base.h>
#include <types.h>
#include <m_point.h>


extern HINSTANCE hInstance;
		    
//----------------------------------------------------------------------------

class GWWindow;

class GWMap
{
	struct Entry
	{
		HWND handle;
		GWWindow* window;
	};
	enum Constants { MaxWindows = 100 };
	static int windowTail;
	static Entry windowList[MaxWindows];

public:
	static void add(GWWindow*);
	static void remove(GWWindow*);
	static GWWindow* getWindow(HANDLE);
};


//----------------------------------------------------------------------------

class GWWindowClass
{
	static int uniqueId;
	char* className;

	void createName();

public:
	GWWindowClass();
	GWWindowClass(int style);
    void registerClass(UINT style,HICON = 0,HCURSOR = 0,HBRUSH = 0);
	const char* getName();

	//
	static GWWindowClass* defaultClass();
};

inline GWWindowClass::GWWindowClass()
{
	createName();
}

inline GWWindowClass::GWWindowClass(int style)
{
	createName();
	registerClass(style);
}

inline const char* GWWindowClass::getName()
{
	return className;
}


//----------------------------------------------------------------------------

class GWWindow
{
	WNDPROC baseProc;

protected:
	static HINSTANCE hInstance;
	HWND hWnd;
	HWND hWndClient;          // For MDI apps
	int wid;
	bool active;

	void hookCreate(GWWindow*);
	void unhookCreate();
public:
	GWWindow();
	HWND getHandle();
	HWND getParentHandle();
	HWND getClientHandle() { return hWndClient; }
	HWND getAppHandle();
	GWWindow* getParent();

   static HINSTANCE getHInstance();   
   static void setHInstance(HINSTANCE hInstance);   

	BOOL show(int nCmdShow);
	bool create(GWWindowClass* wclass,LPCSTR wname, DWORD style,
		const Point2I pos,const Point2I size, GWWindow* parent,int id);

	bool createMDIChild(GWWindowClass* wclass,LPCSTR wname, DWORD style,
		const Point2I pos,const Point2I size, GWWindow* parent,int id);

	bool GWWindow::createMDIClient(LPCSTR wname, DWORD style,
	    const Point2I pos,const Point2I size, GWWindow* parent,int id,
   	    CLIENTCREATESTRUCT *ccs);

	bool isActive() { return active; }

	virtual ~GWWindow();
	virtual bool attach(HWND);
	virtual void detach();
	virtual void destroyWindow();
	virtual void update();

	virtual void setSize(Point2I size);
	virtual void setClientSize(Point2I size);
	virtual void setPosition(Point2I pos);
	Point2I getSize();
	Point2I getClientSize();
	Point2I getPosition();

	//
	virtual LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT defWindowProc(UINT message,WPARAM wParam,LPARAM lParam);

	// Windows events
	virtual void onCompacting(UINT compactRatio);
	virtual void onWinIniChange(LPCTSTR lpszSectionName);
	virtual void onSysColorChange();
	virtual BOOL onQueryNewPalette();
	virtual void onPaletteIsChanging(HWND hwndPaletteChange);
	virtual void onPaletteChanged(HWND hwndPaletteChange);
	virtual void onFontChange();
	virtual void onSpoolerStatus(UINT status, int cJobInQueue);
	virtual void onDevModeChange(LPCTSTR lpszDeviceName);
	virtual void onTimeChange();
	virtual void onPower(int code);
	virtual BOOL onQueryEndSession();
	virtual void onEndSession(BOOL fEnding);
	virtual void onQuit(int exitCode);
	virtual BOOL onCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL onNCCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void onDestroy();
	virtual void onNCDestroy();
	virtual void onShowWindow(BOOL fShow, UINT status);
	virtual void onSetRedraw(BOOL fRedraw);
	virtual void onEnable(BOOL fEnable);
	virtual void onSetText(LPCTSTR lpszText);
	virtual INT onGetText(int cchTextMax, LPTSTR lpszText);
	virtual INT onGetTextLength();
	virtual BOOL onWindowPosChanging(LPWINDOWPOS lpwpos);
	virtual void onWindowPosChanged(const LPWINDOWPOS lpwpos);
	virtual void onMove(int x, int y);
	virtual void onSize(UINT state, int cx, int cy);
	virtual void onClose();
	virtual BOOL onQueryOpen();
	virtual void onGetMinMaxInfo(LPMINMAXINFO lpMinMaxInfo);
	virtual void onPaint();
	virtual BOOL onEraseBkgnd(HDC hdc);
	virtual BOOL onIconEraseBkgnd(HDC hdc);
	virtual void onNCPaint(HRGN hrgn);
	virtual UINT onNCCalcSize(BOOL fCalcValidRects, NCCALCSIZE_PARAMS * lpcsp);
	virtual UINT onNCHitTest(int x, int y);
	virtual HICON onQueryDragIcon();
	virtual void onDropFiles(HDROP hdrop);
	virtual void onActivate(UINT state, HWND hwndActDeact, BOOL fMinimized);
	virtual void onActivateApp(BOOL fActivate, DWORD dwThreadId);
	virtual BOOL onNCActivate(BOOL fActive, HWND hwndActDeact, BOOL fMinimized);
	virtual void onSetFocus(HWND hwndOldFocus);
	virtual void onKillFocus(HWND hwndNewFocus);
	virtual void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
	virtual void onChar(TCHAR ch, int cRepeat, UINT flags);
	virtual void onDeadChar(TCHAR ch, int cRepeat);
	virtual void onSysKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
	virtual void onSysChar(TCHAR ch, int cRepeat);
	virtual void onSysDeadChar(TCHAR ch, int cRepeat);
	virtual void onMouseMove(int x, int y, UINT keyFlags);
	virtual void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	virtual void onLButtonUp(int x, int y, UINT keyFlags);
	virtual void onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	virtual void onRButtonUp(int x, int y, UINT flags);
	virtual void onMButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	virtual void onMButtonUp(int x, int y, UINT flags);
	virtual void onNCMouseMove(int x, int y, UINT codeHitTest);
	virtual void onNCLButtonDown(BOOL fDoubleClick, int x, int y, UINT codeHitTest);
	virtual void onNCLButtonUp(int x, int y, UINT codeHitTest);
	virtual void onNCRButtonDown(BOOL fDoubleClick, int x, int y, UINT codeHitTest);
	virtual void onNCRButtonUp(int x, int y, UINT codeHitTest);
	virtual void onNCMButtonDown(BOOL fDoubleClick, int x, int y, UINT codeHitTest);
	virtual void onNCMButtonUp(int x, int y, UINT codeHitTest);
	virtual int onMouseActivate(HWND hwndTopLevel, UINT codeHitTest, UINT msg);
	virtual void onCancelMode();
	virtual void onTimer(UINT id);
	virtual void onInitMenu(HMENU hMenu);
	virtual void onInitMenuPopup(HMENU hMenu, UINT item, BOOL fSystemMenu);
	virtual void onMenuSelect(HMENU hmenu, int item, HMENU hmenuPopup, UINT flags);
	virtual DWORD onMenuChar(UINT ch, UINT flags, HMENU hmenu);
	virtual void onCommand(int id, HWND hwndCtl, UINT codeNotify);
	virtual void onHScroll(HWND hwndCtl, UINT code, int pos);
	virtual void onVScroll(HWND hwndCtl, UINT code, int pos);
	virtual void onCut();
	virtual void onCopy();
	virtual void onPaste();
	virtual void onClear();
	virtual void onUndo();
	virtual HANDLE onRenderFormat(UINT fmt);
	virtual void onRenderAllFormats();
	virtual void onDestroyClipboard();
	virtual void onDrawClipboard();
	virtual void onPaintClipboard(HWND hwndCBViewer, const LPPAINTSTRUCT lpPaintStruct);
	virtual void onSizeClipboard(HWND hwndCBViewer, const LPRECT lprc);
	virtual void onVScrollClipboard(HWND hwndCBViewer, UINT code, int pos);
	virtual void onHScrollClipboard(HWND hwndCBViewer, UINT code, int pos);
	virtual void onAskCBFormatName(int cchMax, LPTSTR rgchName);
	virtual void onChangeCBChain(HWND hwndRemove, HWND hwndNext);
	virtual BOOL onSetCursor(HWND hwndCursor, UINT codeHitTest, UINT msg);
	virtual void onSysCommand(UINT cmd, int x, int y);
	virtual HWND onMDICreate(const LPMDICREATESTRUCT lpmcs);
	virtual void onMDIDestroy(HWND hwndDestroy);
	virtual void onMDIActivate(BOOL fActive, HWND hwndActivate, HWND hwndDeactivate);
	virtual void onMDIRestore(HWND hwndRestore);
	virtual HWND onMDINext(HWND hwndCur, BOOL fPrev);
	virtual void onMDIMaximize(HWND hwndMaximize);
	virtual BOOL onMDITile(UINT cmd);
	virtual BOOL onMDICascade(UINT cmd);
	virtual void onMDIIconArrange();
	virtual HWND onMDIGetActive();
	virtual HMENU onMDISetMenu(BOOL fRefresh, HMENU hmenuFrame, HMENU hmenuWindow);
	virtual void onChildActivate();
	virtual BOOL onInitDialog(HWND hwndFocus, LPARAM lParam);
	virtual HWND onNextDlgCtl(HWND hwndSetFocus, BOOL fNext);
	virtual void onParentNotify(UINT msg, HWND hwndChild, int idChild);
	virtual void onEnterIdle(UINT source, HWND hwndSource);
	virtual UINT onGetDlgCode(LPMSG lpmsg);
	virtual HBRUSH onCtlColor(HDC hdc, HWND hwndChild, int type);
	virtual void onSetFont(HFONT hfont, BOOL fRedraw);
	virtual HFONT onGetFont();
	virtual void onDrawItem(const DRAWITEMSTRUCT * lpDrawItem);
	virtual void onMeasureItem(MEASUREITEMSTRUCT * lpMeasureItem);
	virtual void onDeleteItem(const DELETEITEMSTRUCT * lpDeleteItem);
	virtual int onCompareItem(const COMPAREITEMSTRUCT * lpCompareItem);
	virtual int onVkeyToItem(UINT vk, HWND hwndListbox, int iCaret);
	virtual int onCharToItem(UINT ch, HWND hwndListbox, int iCaret);
	virtual void onQueueSync();
	virtual void onCommNotify(int cid, UINT flags);
};

inline HWND GWWindow::getHandle( void )
{
	return hWnd;
}

inline BOOL GWWindow::show(int nCmdShow)
{
	return ShowWindow(hWnd,nCmdShow);
}

inline void GWWindow::update()
{
	UpdateWindow(hWnd);
}

#endif
