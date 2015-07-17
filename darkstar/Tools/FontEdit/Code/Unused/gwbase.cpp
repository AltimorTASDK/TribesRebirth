//----------------------------------------------------------------------------

//	$Workfile:   gwbase.cpp  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:02  $

//----------------------------------------------------------------------------

#include <stdio.h>
#include <typeinfo.h>
#include "gwbase.h"
#include "gwdialog.h"
#include "gwwinsx.h"


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

HINSTANCE GWWindow::hInstance = NULL;
GWMap::Entry GWMap::windowList[MaxWindows];
int GWMap::windowTail;

void GWMap::add(GWWindow* win)
{
	AssertFatal(windowTail < MaxWindows,
		"GWMap::add: Ran out of table space");
	windowList[windowTail].window = win;
	windowList[windowTail].handle = win->getHandle(); 
	windowTail++;
}

void GWMap::remove(GWWindow* win)
{
	for (int i = 0; i < MaxWindows; i++)
		if (windowList[i].window == win) {
			windowList[i] = windowList[windowTail--];
			break;
		}
}

GWWindow* GWMap::getWindow(HANDLE handle)
{
	for (int i = 0; i < MaxWindows; i++)
		if (windowList[i].handle == handle)
			return windowList[i].window;
	return 0;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int GWWindowClass::uniqueId;


//----------------------------------------------------------------------------

void GWWindowClass::createName()
{
	className = new char[100];
	sprintf(className,"WClass::%d:%d",GWWindow::getHInstance(),uniqueId++);
}


//----------------------------------------------------------------------------

void GWWindowClass::registerClass(UINT style,HICON icon,
	HCURSOR cursor,HBRUSH brush)
{
    WNDCLASS wndclass;
    wndclass.style         = style;
    wndclass.lpfnWndProc   = DefWindowProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = GWWindow::getHInstance();
    wndclass.hIcon         = icon;
    wndclass.hCursor       = cursor? cursor: LoadCursor(NULL,IDC_ARROW);
    wndclass.hbrBackground = brush? brush: (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = className;
    if ( ! RegisterClass( &wndclass ) )
        exit( FALSE );
}

GWWindowClass* GWWindowClass::defaultClass()
{
	static GWWindowClass* defClass;
	if (!defClass)
		defClass = new GWWindowClass(CS_HREDRAW | CS_VREDRAW);
	return defClass;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

LRESULT PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam,
                                 LPARAM lParam)
{
	GWWindow* window = GWMap::getWindow(hWnd);
	if (window) {
		if (message == WM_SETFONT || message == WM_INITDIALOG)
			// These two are deliverd through the dlgproc function.
			// We want to let the default handler deal with it's
			// own resource initialization before we get to ours.
			// Actually, we should probably make sure this is a 
			// dialog window before doing this.
			return window->GWWindow::windowProc(message,wParam,lParam);
       return window->windowProc(message,wParam,lParam);
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

//----------------------------------------------------------------------------

GWWindow::GWWindow()
{
	baseProc = NULL;
	hWnd = 0;
	wid = 0;
}

GWWindow::~GWWindow()
{
	destroyWindow();
}

bool GWWindow::attach(HWND handle)
{
	if (hWnd)
		detach();
	hWnd = handle;
	GWMap::add(this);

	// Subclass the window
	WNDPROC current = (WNDPROC)GetWindowLong(handle,GWL_WNDPROC);
	if (current == WndProc)
		// Already subclassed
		baseProc = DefWindowProc;
	else {
		SetWindowLong(handle,GWL_WNDPROC,(LONG)WndProc);
		baseProc = current;
	}
	return true;
}

void GWWindow::detach()
{
	if (hWnd != 0)
		GWMap::remove(this);
	hWnd = 0;
	baseProc = 0;
}

void GWWindow::destroyWindow()
{
	if (hWnd) {
		DestroyWindow(hWnd);
		// Detach should be trapped by WM_NCDESTROY
		// but we'll call it just in case.
		detach();
	}
}


//----------------------------------------------------------------------------

static HHOOK hookHandle;
static GWWindow* hookWindow;

LRESULT CALLBACK WndTrap(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HCBT_CREATEWND && hookWindow) {
		// Don't want any derived attach methods right now.
		hookWindow->GWWindow::attach((HWND)wParam);
		hookWindow = 0;
	}
	return CallNextHookEx(hookHandle,code,wParam,lParam);
}

void GWWindow::hookCreate(GWWindow* window)
{
	hookWindow = window;
	hookHandle = SetWindowsHookEx(WH_CBT,(HOOKPROC)WndTrap, hInstance,0);
}

void GWWindow::unhookCreate()
{
	UnhookWindowsHookEx(hookHandle);
	hookWindow = 0;
	hookHandle = 0;
}


//----------------------------------------------------------------------------

bool GWWindow::create(GWWindowClass* wclass,LPCSTR wname, DWORD style,
	const Point2I pos,const Point2I size, GWWindow* parent,int id)
{
	if (hWnd)
		detach();
	if (!wclass)
		wclass = GWWindowClass::defaultClass();
	wid = id;
	hookCreate(this);
    hWnd = CreateWindow(wclass->getName(),wname,style,
        pos.x,pos.y,
        size.x,size.y,
        parent? parent->getHandle(): NULL,
        NULL,
        hInstance,
        (LPSTR) this );
	unhookCreate();
	return hWnd != 0;
}

bool GWWindow::createMDIClient(LPCSTR wname, DWORD style,
	const Point2I pos,const Point2I size, GWWindow* parent,int id,
	CLIENTCREATESTRUCT *ccs)
{
	if (hWnd)
		detach();
	wid = id;
	hookCreate(this);
    hWnd = CreateWindow("MDICLIENT",
    	                wname,
    	                style,
                        pos.x,pos.y,
                        size.x,size.y,
                        parent ? parent->getHandle(): NULL,
                        0,
                        hInstance,
                        &ccs);
	unhookCreate();
	return hWnd != 0;
}

bool GWWindow::createMDIChild(GWWindowClass* wclass,LPCSTR wname, DWORD style,
	const Point2I pos,const Point2I size, GWWindow* parent,int id)
{
	if (hWnd)
		detach();
	if (!wclass)
		wclass = GWWindowClass::defaultClass();
	wid = id;
	hookCreate(this);

	char wclass_[1024], wname_[1024];
	strcpy(wclass_, wclass->getName());
	strcpy(wname_, wname);

    hWnd = CreateMDIWindow(wclass_,
                           wname_,
                           style,
                           pos.x,
                           pos.y,
                           size.x,
                           size.y,
                           parent?parent->getClientHandle(): NULL,
                           hInstance,
                           (LPARAM) this );
	unhookCreate();
	return hWnd != 0;
}



//----------------------------------------------------------------------------

HWND GWWindow::getParentHandle()
{
	AssertFatal(hWnd,"GWWindow::getParentHandle: Window not created");
	return GetParent(hWnd);
}

GWWindow* GWWindow::getParent()
{
	return GWMap::getWindow(getParentHandle());
}

HWND GWWindow::getAppHandle()
{
	GWWindow *p = this;
	while( p->getParent() ) 
		p = p->getParent();
	return ( p->getHandle() );
}


HINSTANCE GWWindow::getHInstance()
{
   return (hInstance);
}     

void GWWindow::setHInstance(HINSTANCE _hInstance)
{
   hInstance =  _hInstance;   
}

void GWWindow::setSize(Point2I size)
{
	SetWindowPos(getHandle(), 0, 0, 0, size.x, size.y, SWP_NOZORDER|SWP_NOMOVE);
}

void GWWindow::setClientSize(Point2I size)
{
	RECT rect;
	SetRect(&rect,0,0,size.x, size.y);
	AdjustWindowRectEx(&rect,
		GetWindowLong(hWnd, GWL_STYLE),
		GetMenu(hWnd) != NULL,
		GetWindowLong(hWnd, GWL_EXSTYLE));
	SetWindowPos(getHandle(), 0, 0, 0, rect.right, rect.bottom, 
	   SWP_NOZORDER | SWP_NOMOVE);
}

void GWWindow::setPosition(Point2I pos)
{
	SetWindowPos(getHandle(), 0, pos.x, pos.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
}

Point2I GWWindow::getSize()
{
	RECT rect;
	GetWindowRect(hWnd,&rect);
	return Point2I(rect.right - rect.left,rect.bottom - rect.top);
}

Point2I GWWindow::getClientSize()
{
	RECT rect;
	GetClientRect(hWnd,&rect);
	return Point2I(rect.right - rect.left,rect.bottom - rect.top);
}

Point2I GWWindow::getPosition()
{
	RECT rect;
	GetWindowRect(hWnd,&rect);
	return Point2I(rect.left, rect.top);
}

//----------------------------------------------------------------------------

void GWWindow::onNCDestroy()
{
	detach();
}

void GWWindow::onSetFocus(HWND hWndOldFocus)
{
/*
	// Keyboard focus for child windows
	HWND hChild = ChildWindowFromPoint(hWnd,GWMain::currentMsg.pt);
	if (hChild != hWnd)
		SetFocus(hChild);
*/
	FORWARD_WM_SETFOCUS( hWndOldFocus, defWindowProc);
}


//----------------------------------------------------------------------------

LRESULT GWWindow::defWindowProc(UINT message,WPARAM wParam,LPARAM lParam)
{

	if (baseProc)
		return CallWindowProc(baseProc,hWnd,message,wParam,lParam);
	return DefWindowProc(hWnd,message,wParam,lParam);
}


//----------------------------------------------------------------------------

LRESULT GWWindow::windowProc(UINT message,WPARAM wParam,LPARAM lParam)
{
	// Convert events to virtual methods
	switch (message) {
		HANDLE_MSG(WM_COMPACTING, onCompacting);
		HANDLE_MSG(WM_WININICHANGE, onWinIniChange);
		HANDLE_MSG(WM_SYSCOLORCHANGE, onSysColorChange);
		HANDLE_MSG(WM_QUERYNEWPALETTE, onQueryNewPalette);
		HANDLE_MSG(WM_PALETTEISCHANGING, onPaletteIsChanging);
		HANDLE_MSG(WM_PALETTECHANGED, onPaletteChanged);
		HANDLE_MSG(WM_FONTCHANGE, onFontChange);
		HANDLE_MSG(WM_SPOOLERSTATUS, onSpoolerStatus);
		HANDLE_MSG(WM_DEVMODECHANGE, onDevModeChange);
		HANDLE_MSG(WM_TIMECHANGE, onTimeChange);
		HANDLE_MSG(WM_POWER, onPower);
		HANDLE_MSG(WM_QUERYENDSESSION, onQueryEndSession);
		HANDLE_MSG(WM_ENDSESSION, onEndSession);
		HANDLE_MSG(WM_QUIT, onQuit);
		HANDLE_MSG(WM_CREATE, onCreate);
		HANDLE_MSG(WM_NCCREATE, onNCCreate);
		HANDLE_MSG(WM_DESTROY, onDestroy);
		HANDLE_MSG(WM_NCDESTROY, onNCDestroy);
		HANDLE_MSG(WM_SHOWWINDOW, onShowWindow);
		HANDLE_MSG(WM_SETREDRAW, onSetRedraw);
		HANDLE_MSG(WM_ENABLE, onEnable);
		HANDLE_MSG(WM_SETTEXT, onSetText);
		HANDLE_MSG(WM_GETTEXT, onGetText);
		HANDLE_MSG(WM_GETTEXTLENGTH, onGetTextLength);
		HANDLE_MSG(WM_WINDOWPOSCHANGING, onWindowPosChanging);
		HANDLE_MSG(WM_WINDOWPOSCHANGED, onWindowPosChanged);
		HANDLE_MSG(WM_MOVE, onMove);
		HANDLE_MSG(WM_SIZE, onSize);
		HANDLE_MSG(WM_CLOSE, onClose);
		HANDLE_MSG(WM_QUERYOPEN, onQueryOpen);
		HANDLE_MSG(WM_GETMINMAXINFO, onGetMinMaxInfo);
		HANDLE_MSG(WM_PAINT, onPaint);
		HANDLE_MSG(WM_ERASEBKGND, onEraseBkgnd);
		HANDLE_MSG(WM_ICONERASEBKGND, onIconEraseBkgnd);
		HANDLE_MSG(WM_NCPAINT, onNCPaint);
		HANDLE_MSG(WM_NCCALCSIZE, onNCCalcSize);
		HANDLE_MSG(WM_NCHITTEST, onNCHitTest);
		HANDLE_MSG(WM_QUERYDRAGICON, onQueryDragIcon);
		HANDLE_MSG(WM_DROPFILES, onDropFiles);
		HANDLE_MSG(WM_ACTIVATE, onActivate);
		HANDLE_MSG(WM_ACTIVATEAPP, onActivateApp);
		HANDLE_MSG(WM_NCACTIVATE, onNCActivate);
		HANDLE_MSG(WM_SETFOCUS, onSetFocus);
		HANDLE_MSG(WM_KILLFOCUS, onKillFocus);
		HANDLE_MSG(WM_KEYUP, onKey);
		HANDLE_MSG(WM_KEYDOWN, onKey);
		HANDLE_MSG(WM_CHAR, onChar);
		HANDLE_MSG(WM_DEADCHAR, onDeadChar);
		HANDLE_MSG(WM_SYSKEYUP, onSysKey);
		HANDLE_MSG(WM_SYSKEYDOWN, onSysKey);
		HANDLE_MSG(WM_SYSCHAR, onSysChar);
		HANDLE_MSG(WM_SYSDEADCHAR, onSysDeadChar);
		HANDLE_MSG(WM_MOUSEMOVE, onMouseMove);
		HANDLE_MSG(WM_LBUTTONDOWN, onLButtonDown);
		HANDLE_MSG(WM_LBUTTONDBLCLK, onLButtonDown);
		HANDLE_MSG(WM_LBUTTONUP, onLButtonUp);
		HANDLE_MSG(WM_RBUTTONDOWN, onRButtonDown);
		HANDLE_MSG(WM_RBUTTONDBLCLK, onRButtonDown);
		HANDLE_MSG(WM_RBUTTONUP, onRButtonUp);
		HANDLE_MSG(WM_MBUTTONDOWN, onMButtonDown);
		HANDLE_MSG(WM_MBUTTONDBLCLK, onMButtonDown);
		HANDLE_MSG(WM_MBUTTONUP, onMButtonUp);
		HANDLE_MSG(WM_NCMOUSEMOVE, onNCMouseMove);
		HANDLE_MSG(WM_NCLBUTTONDOWN, onNCLButtonDown);
		HANDLE_MSG(WM_NCLBUTTONDBLCLK, onNCLButtonDown);
		HANDLE_MSG(WM_NCLBUTTONUP, onNCLButtonUp);
		HANDLE_MSG(WM_NCRBUTTONDOWN, onNCRButtonDown);
		HANDLE_MSG(WM_NCRBUTTONDBLCLK, onNCRButtonDown);
		HANDLE_MSG(WM_NCRBUTTONUP, onNCRButtonUp);
		HANDLE_MSG(WM_NCMBUTTONDOWN, onNCMButtonDown);
		HANDLE_MSG(WM_NCMBUTTONDBLCLK, onNCMButtonDown);
		HANDLE_MSG(WM_NCMBUTTONUP, onNCMButtonUp);
		HANDLE_MSG(WM_MOUSEACTIVATE, onMouseActivate);
		HANDLE_MSG(WM_CANCELMODE, onCancelMode);
		HANDLE_MSG(WM_TIMER, onTimer);
		HANDLE_MSG(WM_INITMENU, onInitMenu);
		HANDLE_MSG(WM_INITMENUPOPUP, onInitMenuPopup);
		HANDLE_MSG(WM_MENUSELECT, onMenuSelect);
		HANDLE_MSG(WM_MENUCHAR, onMenuChar);
		HANDLE_MSG(WM_COMMAND, onCommand);
		HANDLE_MSG(WM_HSCROLL, onHScroll);
		HANDLE_MSG(WM_VSCROLL, onVScroll);
		HANDLE_MSG(WM_CUT, onCut);
		HANDLE_MSG(WM_COPY, onCopy);
		HANDLE_MSG(WM_PASTE, onPaste);
		HANDLE_MSG(WM_CLEAR, onClear);
		HANDLE_MSG(WM_UNDO, onUndo);
		HANDLE_MSG(WM_RENDERFORMAT, onRenderFormat);
		HANDLE_MSG(WM_RENDERALLFORMATS, onRenderAllFormats);
		HANDLE_MSG(WM_DESTROYCLIPBOARD, onDestroyClipboard);
		HANDLE_MSG(WM_DRAWCLIPBOARD, onDrawClipboard);
		HANDLE_MSG(WM_PAINTCLIPBOARD, onPaintClipboard);
		HANDLE_MSG(WM_SIZECLIPBOARD, onSizeClipboard);
		HANDLE_MSG(WM_VSCROLLCLIPBOARD, onVScrollClipboard);
		HANDLE_MSG(WM_HSCROLLCLIPBOARD, onHScrollClipboard);
		HANDLE_MSG(WM_ASKCBFORMATNAME, onAskCBFormatName);
		HANDLE_MSG(WM_CHANGECBCHAIN, onChangeCBChain);
		HANDLE_MSG(WM_SETCURSOR, onSetCursor);
		HANDLE_MSG(WM_SYSCOMMAND, onSysCommand);
		HANDLE_MSG(WM_MDICREATE, onMDICreate);
		HANDLE_MSG(WM_MDIDESTROY, onMDIDestroy);
		HANDLE_MSG(WM_MDIACTIVATE, onMDIActivate);
		HANDLE_MSG(WM_MDIRESTORE, onMDIRestore);
		HANDLE_MSG(WM_MDINEXT, onMDINext);
		HANDLE_MSG(WM_MDIMAXIMIZE, onMDIMaximize);
		HANDLE_MSG(WM_MDITILE, onMDITile);
		HANDLE_MSG(WM_MDICASCADE, onMDICascade);
		HANDLE_MSG(WM_MDIICONARRANGE, onMDIIconArrange);
		HANDLE_MSG(WM_MDIGETACTIVE, onMDIGetActive);
		HANDLE_MSG(WM_MDISETMENU, onMDISetMenu);
		HANDLE_MSG(WM_CHILDACTIVATE, onChildActivate);
		HANDLE_MSG(WM_INITDIALOG, onInitDialog);
		HANDLE_MSG(WM_NEXTDLGCTL, onNextDlgCtl);
		HANDLE_MSG(WM_PARENTNOTIFY, onParentNotify);
		HANDLE_MSG(WM_ENTERIDLE, onEnterIdle);
		HANDLE_MSG(WM_GETDLGCODE, onGetDlgCode);
		HANDLE_MSG(WM_CTLCOLORMSGBOX, onCtlColor);
		HANDLE_MSG(WM_CTLCOLOREDIT, onCtlColor);
		HANDLE_MSG(WM_CTLCOLORLISTBOX, onCtlColor);
		HANDLE_MSG(WM_CTLCOLORBTN, onCtlColor);
		HANDLE_MSG(WM_CTLCOLORDLG, onCtlColor);
		HANDLE_MSG(WM_CTLCOLORSCROLLBAR, onCtlColor);
		HANDLE_MSG(WM_CTLCOLORSTATIC, onCtlColor);
		HANDLE_MSG(WM_SETFONT, onSetFont);
		HANDLE_MSG(WM_GETFONT, onGetFont);
		HANDLE_MSG(WM_DRAWITEM, onDrawItem);
		HANDLE_MSG(WM_MEASUREITEM, onMeasureItem);
		HANDLE_MSG(WM_DELETEITEM, onDeleteItem);
		HANDLE_MSG(WM_COMPAREITEM, onCompareItem);
		HANDLE_MSG(WM_VKEYTOITEM, onVkeyToItem);
		HANDLE_MSG(WM_CHARTOITEM, onCharToItem);
		HANDLE_MSG(WM_QUEUESYNC, onQueueSync);
		HANDLE_MSG(WM_COMMNOTIFY, onCommNotify);
	}
	return defWindowProc(message,wParam,lParam);
}


//----------------------------------------------------------------------------
// Default virtual methods simply pass arguments to the
// default window handler
//----------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma warn -par
#endif

//----------------------------------------------------------------------------

void GWWindow::onCompacting(UINT compactRatio)
{
	FORWARD_WM_COMPACTING( compactRatio, defWindowProc);
}

void GWWindow::onWinIniChange(LPCTSTR lpszSectionName)
{
	FORWARD_WM_WININICHANGE( lpszSectionName, defWindowProc);
}

void GWWindow::onSysColorChange()
{
	FORWARD_WM_SYSCOLORCHANGE( defWindowProc);
}

BOOL GWWindow::onQueryNewPalette()
{
	return FORWARD_WM_QUERYNEWPALETTE( defWindowProc);
}

void GWWindow::onPaletteIsChanging(HWND hWndPaletteChange)
{
	FORWARD_WM_PALETTEISCHANGING( hWndPaletteChange, defWindowProc);
}

void GWWindow::onPaletteChanged(HWND hWndPaletteChange)
{
	FORWARD_WM_PALETTECHANGED( hWndPaletteChange, defWindowProc);
}

void GWWindow::onFontChange()
{
	FORWARD_WM_FONTCHANGE( defWindowProc);
}

void GWWindow::onSpoolerStatus(UINT status, int cJobInQueue)
{
	FORWARD_WM_SPOOLERSTATUS( status, cJobInQueue, defWindowProc);
}

void GWWindow::onDevModeChange(LPCTSTR lpszDeviceName)
{
	FORWARD_WM_DEVMODECHANGE( lpszDeviceName, defWindowProc);
}

void GWWindow::onTimeChange()
{
	FORWARD_WM_TIMECHANGE( defWindowProc);
}

void GWWindow::onPower(int code)
{
	FORWARD_WM_POWER( code, defWindowProc);
}

BOOL GWWindow::onQueryEndSession()
{
	return FORWARD_WM_QUERYENDSESSION( defWindowProc);
}

void GWWindow::onEndSession(BOOL fEnding)
{
	FORWARD_WM_ENDSESSION( fEnding, defWindowProc);
}

void GWWindow::onQuit(int exitCode)
{
	FORWARD_WM_QUIT( exitCode, defWindowProc);
}

BOOL GWWindow::onCreate(LPCREATESTRUCT lpCreateStruct)
{
	return (FORWARD_WM_CREATE( lpCreateStruct, defWindowProc) == -1)?
		false: true;
}

BOOL GWWindow::onNCCreate(LPCREATESTRUCT lpCreateStruct)
{
	return FORWARD_WM_NCCREATE( lpCreateStruct, defWindowProc);
}

void GWWindow::onDestroy()
{
	FORWARD_WM_DESTROY( defWindowProc);
}

void GWWindow::onShowWindow(BOOL fShow, UINT status)
{
	FORWARD_WM_SHOWWINDOW( fShow, status, defWindowProc);
}

void GWWindow::onSetRedraw(BOOL fRedraw)
{
	FORWARD_WM_SETREDRAW( fRedraw, defWindowProc);
}

void GWWindow::onEnable(BOOL fEnable)
{
	FORWARD_WM_ENABLE( fEnable, defWindowProc);
}

void GWWindow::onSetText(LPCTSTR lpszText)
{
	FORWARD_WM_SETTEXT( lpszText, defWindowProc);
}

INT GWWindow::onGetText(int cchTextMax, LPTSTR lpszText)
{
	return FORWARD_WM_GETTEXT( cchTextMax, lpszText, defWindowProc);
}

INT GWWindow::onGetTextLength()
{
	return FORWARD_WM_GETTEXTLENGTH( defWindowProc);
}

BOOL GWWindow::onWindowPosChanging(LPWINDOWPOS lpwpos)
{
	return FORWARD_WM_WINDOWPOSCHANGING( lpwpos, defWindowProc);
}

void GWWindow::onWindowPosChanged(const LPWINDOWPOS lpwpos)
{
	FORWARD_WM_WINDOWPOSCHANGED( lpwpos, defWindowProc);
}

void GWWindow::onMove(int x, int y)
{
	FORWARD_WM_MOVE( x, y, defWindowProc);
}

void GWWindow::onSize(UINT state, int cx, int cy)
{
	FORWARD_WM_SIZE( state, cx, cy, defWindowProc);
}

void GWWindow::onClose()
{
	FORWARD_WM_CLOSE( defWindowProc);
}

BOOL GWWindow::onQueryOpen()
{
	return FORWARD_WM_QUERYOPEN( defWindowProc);
}

void GWWindow::onGetMinMaxInfo(LPMINMAXINFO lpMinMaxInfo)
{
	FORWARD_WM_GETMINMAXINFO( lpMinMaxInfo, defWindowProc);
}

void GWWindow::onPaint()
{
	FORWARD_WM_PAINT( defWindowProc);
}

BOOL GWWindow::onEraseBkgnd(HDC hdc)
{
	return FORWARD_WM_ERASEBKGND( hdc, defWindowProc);
}

BOOL GWWindow::onIconEraseBkgnd(HDC hdc)
{
	return FORWARD_WM_ICONERASEBKGND( hdc, defWindowProc);
}

void GWWindow::onNCPaint(HRGN hrgn)
{
	FORWARD_WM_NCPAINT( hrgn, defWindowProc);
}

UINT GWWindow::onNCCalcSize(BOOL fCalcValidRects, NCCALCSIZE_PARAMS * lpcsp)
{
	return FORWARD_WM_NCCALCSIZE( fCalcValidRects, lpcsp, defWindowProc);
}

UINT GWWindow::onNCHitTest(int x, int y)
{
	return FORWARD_WM_NCHITTEST( x, y, defWindowProc);
}

HICON GWWindow::onQueryDragIcon()
{
	return FORWARD_WM_QUERYDRAGICON( defWindowProc);
}

void GWWindow::onDropFiles(HDROP hdrop)
{
	FORWARD_WM_DROPFILES( hdrop, defWindowProc);
}

void GWWindow::onActivate(UINT state, HWND hWndActDeact, BOOL fMinimized)
{
	FORWARD_WM_ACTIVATE( state, hWndActDeact, fMinimized, defWindowProc);
}

void GWWindow::onActivateApp(BOOL fActivate, DWORD dwThreadId)
{
	active = fActivate;
	FORWARD_WM_ACTIVATEAPP( fActivate, dwThreadId, defWindowProc);
}

BOOL GWWindow::onNCActivate(BOOL fActive, HWND hWndActDeact, BOOL fMinimized)
{
	return FORWARD_WM_NCACTIVATE( fActive, hWndActDeact, fMinimized, defWindowProc);
}

void GWWindow::onKillFocus(HWND hWndNewFocus)
{
	FORWARD_WM_KILLFOCUS( hWndNewFocus, defWindowProc);
}

void GWWindow::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if (fDown)
		FORWARD_WM_KEYDOWN( vk, cRepeat, flags, defWindowProc);
	else
		FORWARD_WM_KEYUP( vk, cRepeat, flags, defWindowProc);
}

void GWWindow::onChar(TCHAR ch, int cRepeat, UINT flags)
{
	FORWARD_WM_CHAR( ch, cRepeat, flags, defWindowProc);
}

void GWWindow::onDeadChar(TCHAR ch, int cRepeat)
{
	FORWARD_WM_DEADCHAR( ch, cRepeat, defWindowProc);
}

void GWWindow::onSysKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if (fDown)
		FORWARD_WM_SYSKEYDOWN( vk, cRepeat, flags, defWindowProc);
	else
		FORWARD_WM_SYSKEYUP( vk, cRepeat, flags, defWindowProc);
}

void GWWindow::onSysChar(TCHAR ch, int cRepeat)
{
	FORWARD_WM_SYSCHAR( ch, cRepeat, defWindowProc);
}

void GWWindow::onSysDeadChar(TCHAR ch, int cRepeat)
{
	FORWARD_WM_SYSDEADCHAR( ch, cRepeat, defWindowProc);
}

void GWWindow::onMouseMove(int x, int y, UINT keyFlags)
{
	FORWARD_WM_MOUSEMOVE( x, y, keyFlags, defWindowProc);
}

void GWWindow::onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	FORWARD_WM_LBUTTONDOWN( fDoubleClick, x, y, keyFlags, defWindowProc);
}

void GWWindow::onLButtonUp(int x, int y, UINT keyFlags)
{
	FORWARD_WM_LBUTTONUP( x, y, keyFlags, defWindowProc);
}

void GWWindow::onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	FORWARD_WM_RBUTTONDOWN( fDoubleClick, x, y, keyFlags, defWindowProc);
}

void GWWindow::onRButtonUp(int x, int y, UINT keyFlags)
{
	FORWARD_WM_RBUTTONUP( x, y, keyFlags, defWindowProc);
}

void GWWindow::onMButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	FORWARD_WM_MBUTTONDOWN( fDoubleClick, x, y, keyFlags, defWindowProc);
}

void GWWindow::onMButtonUp(int x, int y, UINT keyFlags)
{
	FORWARD_WM_MBUTTONUP( x, y, keyFlags, defWindowProc);
}

void GWWindow::onNCMouseMove(int x, int y, UINT codeHitTest)
{
	FORWARD_WM_NCMOUSEMOVE( x, y, codeHitTest, defWindowProc);
}

void GWWindow::onNCLButtonDown(BOOL fDoubleClick, int x, int y, UINT codeHitTest)
{
	FORWARD_WM_NCLBUTTONDOWN( fDoubleClick, x, y, codeHitTest, defWindowProc);
}

void GWWindow::onNCLButtonUp(int x, int y, UINT codeHitTest)
{
	FORWARD_WM_NCLBUTTONUP( x, y, codeHitTest, defWindowProc);
}

void GWWindow::onNCRButtonDown(BOOL fDoubleClick, int x, int y, UINT codeHitTest)
{
	FORWARD_WM_NCRBUTTONDOWN( fDoubleClick, x, y, codeHitTest, defWindowProc);
}

void GWWindow::onNCRButtonUp(int x, int y, UINT codeHitTest)
{
	FORWARD_WM_NCRBUTTONUP( x, y, codeHitTest, defWindowProc);
}

void GWWindow::onNCMButtonDown(BOOL fDoubleClick, int x, int y, UINT codeHitTest)
{
	FORWARD_WM_NCMBUTTONDOWN( fDoubleClick, x, y, codeHitTest, defWindowProc);
}

void GWWindow::onNCMButtonUp(int x, int y, UINT codeHitTest)
{
	FORWARD_WM_NCRBUTTONUP( x, y, codeHitTest, defWindowProc);
}

int GWWindow::onMouseActivate(HWND hWndTopLevel, UINT codeHitTest, UINT msg)
{
	return FORWARD_WM_MOUSEACTIVATE( hWndTopLevel, codeHitTest, msg, defWindowProc);
}

void GWWindow::onCancelMode()
{
	FORWARD_WM_CANCELMODE( defWindowProc);
}

void GWWindow::onTimer(UINT id)
{
	FORWARD_WM_TIMER( id, defWindowProc);
}

void GWWindow::onInitMenu(HMENU hMenu)
{
	FORWARD_WM_INITMENU( hMenu, defWindowProc);
}

void GWWindow::onInitMenuPopup(HMENU hMenu, UINT item, BOOL fSystemMenu)
{
	FORWARD_WM_INITMENUPOPUP( hMenu, item, fSystemMenu, defWindowProc);
}

void GWWindow::onMenuSelect(HMENU hmenu, int item, HMENU hmenuPopup, UINT flags)
{
	FORWARD_WM_MENUSELECT( hmenu, item, hmenuPopup, flags, defWindowProc);
}

DWORD GWWindow::onMenuChar(UINT ch, UINT flags, HMENU hmenu)
{
	return FORWARD_WM_MENUCHAR( ch, flags, hmenu, defWindowProc);
}

void GWWindow::onCommand(int id, HWND hWndCtl, UINT codeNotify)
{
	FORWARD_WM_COMMAND( id, hWndCtl, codeNotify, defWindowProc);
}

void GWWindow::onHScroll(HWND hWndCtl, UINT code, int pos)
{
	FORWARD_WM_HSCROLL( hWndCtl, code, pos, defWindowProc);
}

void GWWindow::onVScroll(HWND hWndCtl, UINT code, int pos)
{
	FORWARD_WM_VSCROLL( hWndCtl, code, pos, defWindowProc);
}

void GWWindow::onCut()
{
	FORWARD_WM_CUT( defWindowProc);
}

void GWWindow::onCopy()
{
	FORWARD_WM_COPY( defWindowProc);
}

void GWWindow::onPaste()
{
	FORWARD_WM_PASTE( defWindowProc);
}

void GWWindow::onClear()
{
	FORWARD_WM_CLEAR( defWindowProc);
}

void GWWindow::onUndo()
{
	FORWARD_WM_UNDO( defWindowProc);
}

HANDLE GWWindow::onRenderFormat(UINT fmt)
{
	return FORWARD_WM_RENDERFORMAT( fmt, defWindowProc);
}

void GWWindow::onRenderAllFormats()
{
	FORWARD_WM_RENDERALLFORMATS( defWindowProc);
}

void GWWindow::onDestroyClipboard()
{
	FORWARD_WM_DESTROYCLIPBOARD( defWindowProc);
}

void GWWindow::onDrawClipboard()
{
	FORWARD_WM_DRAWCLIPBOARD( defWindowProc);
}

void GWWindow::onPaintClipboard(HWND hWndCBViewer, const LPPAINTSTRUCT lpPaintStruct)
{
	FORWARD_WM_PAINTCLIPBOARD( hWndCBViewer, lpPaintStruct, defWindowProc);
}

void GWWindow::onSizeClipboard(HWND hWndCBViewer, const LPRECT lprc)
{
	FORWARD_WM_SIZECLIPBOARD( hWndCBViewer, lprc, defWindowProc);
}

void GWWindow::onVScrollClipboard(HWND hWndCBViewer, UINT code, int pos)
{
	FORWARD_WM_VSCROLLCLIPBOARD( hWndCBViewer, code, pos, defWindowProc);
}

void GWWindow::onHScrollClipboard(HWND hWndCBViewer, UINT code, int pos)
{
	FORWARD_WM_HSCROLLCLIPBOARD( hWndCBViewer, code, pos, defWindowProc);
}

void GWWindow::onAskCBFormatName(int cchMax, LPTSTR rgchName)
{
	FORWARD_WM_ASKCBFORMATNAME( cchMax, rgchName, defWindowProc);
}

void GWWindow::onChangeCBChain(HWND hWndRemove, HWND hWndNext)
{
	FORWARD_WM_CHANGECBCHAIN( hWndRemove, hWndNext, defWindowProc);
}

BOOL GWWindow::onSetCursor(HWND hWndCursor, UINT codeHitTest, UINT msg)
{
	return FORWARD_WM_SETCURSOR( hWndCursor, codeHitTest, msg, defWindowProc);
}

void GWWindow::onSysCommand(UINT cmd, int x, int y)
{
	FORWARD_WM_SYSCOMMAND( cmd, x, y, defWindowProc);
}

HWND GWWindow::onMDICreate(const LPMDICREATESTRUCT lpmcs)
{
	return FORWARD_WM_MDICREATE( lpmcs, defWindowProc);
}

void GWWindow::onMDIDestroy(HWND hWndDestroy)
{
	FORWARD_WM_MDIDESTROY( hWndDestroy, defWindowProc);
}

void GWWindow::onMDIActivate(BOOL fActive, HWND hWndActivate, HWND hWndDeactivate)
{
	FORWARD_WM_MDIACTIVATE( fActive, hWndActivate, hWndDeactivate, defWindowProc);
}

void GWWindow::onMDIRestore(HWND hWndRestore)
{
	FORWARD_WM_MDIRESTORE( hWndRestore, defWindowProc);
}

HWND GWWindow::onMDINext(HWND hWndCur, BOOL fPrev)
{
	return FORWARD_WM_MDINEXT( hWndCur, fPrev, defWindowProc);
}

void GWWindow::onMDIMaximize(HWND hWndMaximize)
{
	FORWARD_WM_MDIMAXIMIZE( hWndMaximize, defWindowProc);
}

BOOL GWWindow::onMDITile(UINT cmd)
{
	return FORWARD_WM_MDITILE( cmd, defWindowProc);
}

BOOL GWWindow::onMDICascade(UINT cmd)
{
	return FORWARD_WM_MDICASCADE( cmd, defWindowProc);
}

void GWWindow::onMDIIconArrange()
{
	FORWARD_WM_MDIICONARRANGE( defWindowProc);
}

HWND GWWindow::onMDIGetActive()
{
	return FORWARD_WM_MDIGETACTIVE( defWindowProc);
}

HMENU GWWindow::onMDISetMenu(BOOL fRefresh, HMENU hmenuFrame, HMENU hmenuWindow)
{
	return FORWARD_WM_MDISETMENU( fRefresh, hmenuFrame, hmenuWindow, defWindowProc);
}

void GWWindow::onChildActivate()
{
	FORWARD_WM_CHILDACTIVATE( defWindowProc);
}

BOOL GWWindow::onInitDialog(HWND hWndFocus, LPARAM lParam)
{
	return FORWARD_WM_INITDIALOG( hWndFocus, lParam, defWindowProc);
}

HWND GWWindow::onNextDlgCtl(HWND hWndSetFocus, BOOL fNext)
{
	return FORWARD_WM_NEXTDLGCTL( hWndSetFocus, fNext, defWindowProc);
}

void GWWindow::onParentNotify(UINT msg, HWND hWndChild, int idChild)
{
	FORWARD_WM_PARENTNOTIFY( msg, hWndChild, idChild, defWindowProc);
}

void GWWindow::onEnterIdle(UINT source, HWND hWndSource)
{
	FORWARD_WM_ENTERIDLE( source, hWndSource, defWindowProc);
}

UINT GWWindow::onGetDlgCode(LPMSG lpmsg)
{
	return FORWARD_WM_GETDLGCODE( lpmsg, defWindowProc);
}

HBRUSH GWWindow::onCtlColor(HDC hdc, HWND hWndChild, int type)
{
	switch (type) {
    	case CTLCOLOR_MSGBOX:
			return FORWARD_WM_CTLCOLORMSGBOX( hdc, hWndChild, defWindowProc);
		case CTLCOLOR_EDIT:
			return FORWARD_WM_CTLCOLOREDIT( hdc, hWndChild, defWindowProc);
		case CTLCOLOR_LISTBOX:
			return FORWARD_WM_CTLCOLORLISTBOX( hdc, hWndChild, defWindowProc);
		case CTLCOLOR_BTN:
			return FORWARD_WM_CTLCOLORBTN( hdc, hWndChild, defWindowProc);
		case CTLCOLOR_DLG:
			return FORWARD_WM_CTLCOLORDLG( hdc, hWndChild, defWindowProc);
		case CTLCOLOR_SCROLLBAR:
			return FORWARD_WM_CTLCOLORSCROLLBAR( hdc, hWndChild, defWindowProc);
		case CTLCOLOR_STATIC:
			return FORWARD_WM_CTLCOLORSTATIC( hdc, hWndChild, defWindowProc);
	}
	return 0;
}

void GWWindow::onSetFont(HFONT hfont, BOOL fRedraw)
{
	FORWARD_WM_SETFONT( hfont, fRedraw, defWindowProc);
}

HFONT GWWindow::onGetFont()
{
	return FORWARD_WM_GETFONT( defWindowProc);
}

void GWWindow::onDrawItem(const DRAWITEMSTRUCT * lpDrawItem)
{
	FORWARD_WM_DRAWITEM( lpDrawItem, defWindowProc);
}

void GWWindow::onMeasureItem(MEASUREITEMSTRUCT * lpMeasureItem)
{
	FORWARD_WM_MEASUREITEM( lpMeasureItem, defWindowProc);
}

void GWWindow::onDeleteItem(const DELETEITEMSTRUCT * lpDeleteItem)
{
	FORWARD_WM_DELETEITEM( lpDeleteItem, defWindowProc);
}

int GWWindow::onCompareItem(const COMPAREITEMSTRUCT * lpCompareItem)
{
	return FORWARD_WM_COMPAREITEM( lpCompareItem, defWindowProc);
}

int GWWindow::onVkeyToItem(UINT vk, HWND hWndListBox, int iCaret)
{
	return FORWARD_WM_VKEYTOITEM( vk, hWndListBox, iCaret, defWindowProc);
}

int GWWindow::onCharToItem(UINT ch, HWND hWndListBox, int iCaret)
{
	return FORWARD_WM_CHARTOITEM( ch, hWndListBox, iCaret, defWindowProc);
}

void GWWindow::onQueueSync()
{
	FORWARD_WM_QUEUESYNC( defWindowProc);
}

void GWWindow::onCommNotify(int cid, UINT flags)
{
	FORWARD_WM_COMMNOTIFY( cid, flags, defWindowProc);
}
