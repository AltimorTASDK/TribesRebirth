//----------------------------------------------------------------------------

//	$Workfile:   gwwinsx.h  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:02  $

// Slighlty modified to work with GW lib.

//----------------------------------------------------------------------------

/*****************************************************************************\
*                                                                             *
* windowsx.h -  Macro APIs, window message crackers, and control APIs         *
*                                                                             *
*               Version Win32 / Windows NT                                    *
*                                                                             *
*               Copyright (c) 1992-1995, Microsoft Corp.  All rights reserved.*
*                                                                             *
\*****************************************************************************/

#ifndef _INC_WINDOWSX
#define _INC_WINDOWSX

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif /* __cplusplus */

/****** KERNEL Macro APIs ****************************************************/

#define     GetInstanceModule(hInstance) (HMODULE)(hInstance)

#define     GlobalPtrHandle(lp)         \
                ((HGLOBAL)GlobalHandle(lp))

#define     GlobalLockPtr(lp)                \
                ((BOOL)GlobalLock(GlobalPtrHandle(lp)))
#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define     GlobalAllocPtr(flags, cb)        \
                (GlobalLock(GlobalAlloc((flags), (cb))))
#define     GlobalReAllocPtr(lp, cbNew, flags)       \
                (GlobalUnlockPtr(lp), GlobalLock(GlobalReAlloc(GlobalPtrHandle(lp) , (cbNew), (flags))))
#define     GlobalFreePtr(lp)                \
                (GlobalUnlockPtr(lp), (BOOL)GlobalFree(GlobalPtrHandle(lp)))

/****** GDI Macro APIs *******************************************************/

#define     DeletePen(hpen)      DeleteObject((HGDIOBJ)(HPEN)(hpen))
#define     SelectPen(hdc, hpen)    ((HPEN)SelectObject((hdc), (HGDIOBJ)(HPEN)(hpen)))
#define     GetStockPen(i)       ((HPEN)GetStockObject(i))

#define     DeleteBrush(hbr)     DeleteObject((HGDIOBJ)(HBRUSH)(hbr))
#define     SelectBrush(hdc, hbr)   ((HBRUSH)SelectObject((hdc), (HGDIOBJ)(HBRUSH)(hbr)))
#define     GetStockBrush(i)     ((HBRUSH)GetStockObject(i))

#define     DeleteRgn(hrgn)      DeleteObject((HGDIOBJ)(HRGN)(hrgn))

#define     CopyRgn(hrgnDst, hrgnSrc)               CombineRgn(hrgnDst, hrgnSrc, 0, RGN_COPY)
#define     IntersectRgn(hrgnResult, hrgnA, hrgnB)  CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_AND)
#define     SubtractRgn(hrgnResult, hrgnA, hrgnB)   CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_DIFF)
#define     UnionRgn(hrgnResult, hrgnA, hrgnB)      CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_OR)
#define     XorRgn(hrgnResult, hrgnA, hrgnB)        CombineRgn(hrgnResult, hrgnA, hrgnB, RGN_XOR)

#define     DeletePalette(hpal)     DeleteObject((HGDIOBJ)(HPALETTE)(hpal))

#define     DeleteFont(hfont)            DeleteObject((HGDIOBJ)(HFONT)(hfont))
#define     SelectFont(hdc, hfont)  ((HFONT)SelectObject((hdc), (HGDIOBJ)(HFONT)(hfont)))
#define     GetStockFont(i)      ((HFONT)GetStockObject(i))

#define     DeleteBitmap(hbm)       DeleteObject((HGDIOBJ)(HBITMAP)(hbm))
#define     SelectBitmap(hdc, hbm)  ((HBITMAP)SelectObject((hdc), (HGDIOBJ)(HBITMAP)(hbm)))

#define     InsetRect(lprc, dx, dy) InflateRect((lprc), -(dx), -(dy))

/****** USER Macro APIs ******************************************************/

#define     GetWindowInstance(hwnd) ((HMODULE)GetWindowLong(hwnd, GWL_HINSTANCE))

#define     GetWindowStyle(hwnd)    ((DWORD)GetWindowLong(hwnd, GWL_STYLE))
#define     GetWindowExStyle(hwnd)  ((DWORD)GetWindowLong(hwnd, GWL_EXSTYLE))

#define     GetWindowOwner(hwnd)    GetWindow(hwnd, GW_OWNER)

#define     GetFirstChild(hwnd)     GetTopWindow(hwnd)
#define     GetFirstSibling(hwnd)   GetWindow(hwnd, GW_HWNDFIRST)
#define     GetLastSibling(hwnd)    GetWindow(hwnd, GW_HWNDLAST)
#define     GetNextSibling(hwnd)    GetWindow(hwnd, GW_HWNDNEXT)
#define     GetPrevSibling(hwnd)    GetWindow(hwnd, GW_HWNDPREV)

#define     GetWindowID(hwnd)            GetDlgCtrlID(hwnd)

#define     SetWindowRedraw(hwnd, fRedraw)  \
                    ((void)SendMessage(hwnd, WM_SETREDRAW, (WPARAM)(BOOL)(fRedraw), 0L))

#define     SubclassWindow(hwnd, lpfn)       \
              ((WNDPROC)SetWindowLong((hwnd), GWL_WNDPROC, (LPARAM)(WNDPROC)(lpfn)))

#define     IsMinimized(hwnd)        IsIconic(hwnd)
#define     IsMaximized(hwnd)        IsZoomed(hwnd)
#define     IsRestored(hwnd)    ((GetWindowStyle(hwnd) & (WS_MINIMIZE | WS_MAXIMIZE)) == 0L)

#define     SetWindowFont(hwnd, hfont, fRedraw) FORWARD_WM_SETFONT((hwnd), (hfont), (fRedraw), SendMessage)

#define     GetWindowFont(hwnd)                 FORWARD_WM_GETFONT((hwnd), SendMessage)
#if (WINVER >= 0x030a)
#define     MapWindowRect(hwndFrom, hwndTo, lprc) \
                    MapWindowPoints((hwndFrom), (hwndTo), (POINT *)(lprc), 2)
#endif
#define     IsLButtonDown()  (GetKeyState(VK_LBUTTON) < 0)
#define     IsRButtonDown()  (GetKeyState(VK_RBUTTON) < 0)
#define     IsMButtonDown()  (GetKeyState(VK_MBUTTON) < 0)

#define     SubclassDialog(hwndDlg, lpfn) \
             ((DLGPROC)SetWindowLong(hwndDlg, DWL_DLGPROC, (LPARAM)(DLGPROC)(lpfn)))

#define     SetDlgMsgResult(hwnd, msg, result) (( \
        (msg) == WM_CTLCOLORMSGBOX      || \
        (msg) == WM_CTLCOLOREDIT        || \
        (msg) == WM_CTLCOLORLISTBOX     || \
        (msg) == WM_CTLCOLORBTN         || \
        (msg) == WM_CTLCOLORDLG         || \
        (msg) == WM_CTLCOLORSCROLLBAR   || \
        (msg) == WM_CTLCOLORSTATIC      || \
        (msg) == WM_COMPAREITEM         || \
        (msg) == WM_VKEYTOITEM          || \
        (msg) == WM_CHARTOITEM          || \
        (msg) == WM_QUERYDRAGICON       || \
        (msg) == WM_INITDIALOG             \
    ) ? (BOOL)(result) : (SetWindowLong((hwnd), DWL_MSGRESULT, (LPARAM)(LRESULT)(result)), TRUE))

#define     DefDlgProcEx(hwnd, msg, wParam, lParam, pfRecursion) \
    (*(pfRecursion) = TRUE, DefDlgProc(hwnd, msg, wParam, lParam))

#define     CheckDefDlgRecursion(pfRecursion) \
    if (*(pfRecursion)) { *(pfRecursion) = FALSE; return FALSE; }

/****** Message crackers ****************************************************/

#define HANDLE_MSG(message, fn)    \
    case (message): return HANDLE_##message((wParam), (lParam), (fn))

// void Cls_OnContextMenu(HWND  UINT compactRatio)
#define HANDLE_WM_CONTEXTMENU( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (HWND)(wParam)), 0L)
#define FORWARD_WM_CONTEXTMENU( x, y, hwnd, fn) \
    (void)(fn)( WM_CONTEXTMENU, (WPARAM)(HWND)(hwnd), MAKELPARAM((x), (y)))

/* void Cls_OnCompacting(HWND  UINT compactRatio) */
#define HANDLE_WM_COMPACTING( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam)), 0L)
#define FORWARD_WM_COMPACTING( compactRatio, fn) \
    (void)(fn)( WM_COMPACTING, (WPARAM)(UINT)(compactRatio), 0L)

/* void Cls_OnWinIniChange(HWND  LPCTSTR lpszSectionName) */
#define HANDLE_WM_WININICHANGE( wParam, lParam, fn) \
    ((fn)( (LPCTSTR)(lParam)), 0L)
#define FORWARD_WM_WININICHANGE( lpszSectionName, fn) \
    (void)(fn)( WM_WININICHANGE, 0L, (LPARAM)(LPCTSTR)(lpszSectionName))

/* void Cls_OnSysColorChange(HWND hwnd) */
#define HANDLE_WM_SYSCOLORCHANGE( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_SYSCOLORCHANGE( fn) \
    (void)(fn)( WM_SYSCOLORCHANGE, 0L, 0L)

/* BOOL Cls_OnQueryNewPalette(HWND hwnd) */
#define HANDLE_WM_QUERYNEWPALETTE( wParam, lParam, fn) \
    MAKELRESULT((BOOL)(fn)(),0L)
#define FORWARD_WM_QUERYNEWPALETTE( fn) \
    (BOOL)(DWORD)(fn)( WM_QUERYNEWPALETTE, 0L, 0L)

/* void Cls_OnPaletteIsChanging(HWND  HWND hwndPaletteChange) */
#define HANDLE_WM_PALETTEISCHANGING( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam)), 0L)
#define FORWARD_WM_PALETTEISCHANGING( hwndPaletteChange, fn) \
    (void)(fn)( WM_PALETTEISCHANGING, (WPARAM)(HWND)(hwndPaletteChange), 0L)

/* void Cls_OnPaletteChanged(HWND  HWND hwndPaletteChange) */
#define HANDLE_WM_PALETTECHANGED( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam)), 0L)
#define FORWARD_WM_PALETTECHANGED( hwndPaletteChange, fn) \
    (void)(fn)( WM_PALETTECHANGED, (WPARAM)(HWND)(hwndPaletteChange), 0L)

/* void Cls_OnFontChange(HWND hwnd) */
#define HANDLE_WM_FONTCHANGE( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_FONTCHANGE( fn) \
    (void)(fn)( WM_FONTCHANGE, 0L, 0L)

/* void Cls_OnSpoolerStatus(HWND  UINT status, int cJobInQueue) */
#define HANDLE_WM_SPOOLERSTATUS( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam), (int)(short)LOWORD(lParam)), 0L)
#define FORWARD_WM_SPOOLERSTATUS( status, cJobInQueue, fn) \
    (void)(fn)( WM_SPOOLERSTATUS, (WPARAM)(status), MAKELPARAM((cJobInQueue), 0))

/* void Cls_OnDevModeChange(HWND  LPCTSTR lpszDeviceName) */
#define HANDLE_WM_DEVMODECHANGE( wParam, lParam, fn) \
    ((fn)( (LPCTSTR)(lParam)), 0L)
#define FORWARD_WM_DEVMODECHANGE( lpszDeviceName, fn) \
    (void)(fn)( WM_DEVMODECHANGE, 0L, (LPARAM)(LPCTSTR)(lpszDeviceName))

/* void Cls_OnTimeChange(HWND hwnd) */
#define HANDLE_WM_TIMECHANGE( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_TIMECHANGE( fn) \
    (void)(fn)( WM_TIMECHANGE, 0L, 0L)

/* void Cls_OnPower(HWND  int code) */
#define HANDLE_WM_POWER( wParam, lParam, fn) \
    ((fn)( (int)(wParam)), 0L)
#define FORWARD_WM_POWER( code, fn) \
    (void)(fn)( WM_POWER, (WPARAM)(int)(code), 0L)

/* BOOL Cls_OnQueryEndSession(HWND hwnd) */
#define HANDLE_WM_QUERYENDSESSION( wParam, lParam, fn) \
    MAKELRESULT((BOOL)(fn)(),0L)
#define FORWARD_WM_QUERYENDSESSION( fn) \
    (BOOL)(DWORD)(fn)( WM_QUERYENDSESSION, 0L, 0L)

/* void Cls_OnEndSession(HWND  BOOL fEnding) */
#define HANDLE_WM_ENDSESSION( wParam, lParam, fn) \
    ((fn)( (BOOL)(wParam)), 0L)
#define FORWARD_WM_ENDSESSION( fEnding, fn) \
    (void)(fn)( WM_ENDSESSION, (WPARAM)(BOOL)(fEnding), 0L)

/* void Cls_OnQuit(HWND  int exitCode) */
#define HANDLE_WM_QUIT( wParam, lParam, fn) \
    ((fn)( (int)(wParam)), 0L)
#define FORWARD_WM_QUIT( exitCode, fn) \
    (void)(fn)( WM_QUIT, (WPARAM)(exitCode), 0L)

/* This message is in Windows 3.1 only */
/* void Cls_OnSystemError(HWND  int errCode) */
#define HANDLE_WM_SYSTEMERROR( wParam, lParam, fn) 0L
#define FORWARD_WM_SYSTEMERROR( errCode, fn) 0L

/* BOOL Cls_OnCreate(HWND  LPCREATESTRUCT lpCreateStruct) */
#define HANDLE_WM_CREATE( wParam, lParam, fn) \
    ((fn)( (LPCREATESTRUCT)(lParam)) ? 0L : (LRESULT)-1L)
#define FORWARD_WM_CREATE( lpCreateStruct, fn) \
    (BOOL)(DWORD)(fn)( WM_CREATE, 0L, (LPARAM)(LPCREATESTRUCT)(lpCreateStruct))

/* BOOL Cls_OnNCCreate(HWND  LPCREATESTRUCT lpCreateStruct) */
#define HANDLE_WM_NCCREATE( wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)( (LPCREATESTRUCT)(lParam))
#define FORWARD_WM_NCCREATE( lpCreateStruct, fn) \
    (BOOL)(DWORD)(fn)( WM_NCCREATE, 0L, (LPARAM)(LPCREATESTRUCT)(lpCreateStruct))

/* void Cls_OnDestroy(HWND hwnd) */
#define HANDLE_WM_DESTROY( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_DESTROY( fn) \
    (void)(fn)( WM_DESTROY, 0L, 0L)

/* void Cls_OnNCDestroy(HWND hwnd) */
#define HANDLE_WM_NCDESTROY( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_NCDESTROY( fn) \
    (void)(fn)( WM_NCDESTROY, 0L, 0L)

/* void Cls_OnShowWindow(HWND  BOOL fShow, UINT status) */
#define HANDLE_WM_SHOWWINDOW( wParam, lParam, fn) \
    ((fn)( (BOOL)(wParam), (UINT)(lParam)), 0L)
#define FORWARD_WM_SHOWWINDOW( fShow, status, fn) \
    (void)(fn)( WM_SHOWWINDOW, (WPARAM)(BOOL)(fShow), (LPARAM)(UINT)(status))

/* void Cls_OnSetRedraw(HWND  BOOL fRedraw) */
#define HANDLE_WM_SETREDRAW( wParam, lParam, fn) \
    ((fn)( (BOOL)(wParam)), 0L)
#define FORWARD_WM_SETREDRAW( fRedraw, fn) \
    (void)(fn)( WM_SETREDRAW, (WPARAM)(BOOL)(fRedraw), 0L)

/* void Cls_OnEnable(HWND  BOOL fEnable) */
#define HANDLE_WM_ENABLE( wParam, lParam, fn) \
    ((fn)( (BOOL)(wParam)), 0L)
#define FORWARD_WM_ENABLE( fEnable, fn) \
    (void)(fn)( WM_ENABLE, (WPARAM)(BOOL)(fEnable), 0L)

/* void Cls_OnSetText(HWND  LPCTSTR lpszText) */
#define HANDLE_WM_SETTEXT( wParam, lParam, fn) \
    ((fn)( (LPCTSTR)(lParam)), 0L)
#define FORWARD_WM_SETTEXT( lpszText, fn) \
    (void)(fn)( WM_SETTEXT, 0L, (LPARAM)(LPCTSTR)(lpszText))

/* INT Cls_OnGetText(HWND  int cchTextMax, LPTSTR lpszText) */
#define HANDLE_WM_GETTEXT( wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)( (int)(wParam), (LPTSTR)(lParam))
#define FORWARD_WM_GETTEXT( cchTextMax, lpszText, fn) \
    (int)(DWORD)(fn)( WM_GETTEXT, (WPARAM)(int)(cchTextMax), (LPARAM)(LPTSTR)(lpszText))

/* INT Cls_OnGetTextLength(HWND hwnd) */
#define HANDLE_WM_GETTEXTLENGTH( wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)()
#define FORWARD_WM_GETTEXTLENGTH( fn) \
    (int)(DWORD)(fn)( WM_GETTEXTLENGTH, 0L, 0L)

/* BOOL Cls_OnWindowPosChanging(HWND  LPWINDOWPOS lpwpos) */
#define HANDLE_WM_WINDOWPOSCHANGING( wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)( (LPWINDOWPOS)(lParam))
#define FORWARD_WM_WINDOWPOSCHANGING( lpwpos, fn) \
    (BOOL)(DWORD)(fn)( WM_WINDOWPOSCHANGING, 0L, (LPARAM)(LPWINDOWPOS)(lpwpos))

/* void Cls_OnWindowPosChanged(HWND  const LPWINDOWPOS lpwpos) */
#define HANDLE_WM_WINDOWPOSCHANGED( wParam, lParam, fn) \
    ((fn)( (const LPWINDOWPOS)(lParam)), 0L)
#define FORWARD_WM_WINDOWPOSCHANGED( lpwpos, fn) \
    (void)(fn)( WM_WINDOWPOSCHANGED, 0L, (LPARAM)(const LPWINDOWPOS)(lpwpos))

/* void Cls_OnMove(HWND  int x, int y) */
#define HANDLE_WM_MOVE( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define FORWARD_WM_MOVE( x, y, fn) \
    (void)(fn)( WM_MOVE, 0L, MAKELPARAM((x), (y)))

/* void Cls_OnSize(HWND  UINT state, int cx, int cy) */
#define HANDLE_WM_SIZE( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define FORWARD_WM_SIZE( state, cx, cy, fn) \
    (void)(fn)( WM_SIZE, (WPARAM)(UINT)(state), MAKELPARAM((cx), (cy)))

/* void Cls_OnDisplayChange(UINT cBitsPerPixel, int cx, int cy) */
#define HANDLE_WM_DISPLAYCHANGE( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define FORWARD_WM_DISPLAYCHANGE( bitsPerPixel, cx, cy, fn) \
    (void)(fn)( WM_DISPLAYCHANGE, (WPARAM)(UINT)(bitsPerPixel), MAKELPARAM((cx), (cy)))

/* void Cls_OnClose(HWND hwnd) */
#define HANDLE_WM_CLOSE( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_CLOSE( fn) \
    (void)(fn)( WM_CLOSE, 0L, 0L)

/* BOOL Cls_OnQueryOpen(HWND hwnd) */
#define HANDLE_WM_QUERYOPEN( wParam, lParam, fn) \
    MAKELRESULT((BOOL)(fn)(),0L)
#define FORWARD_WM_QUERYOPEN( fn) \
    (BOOL)(DWORD)(fn)( WM_QUERYOPEN, 0L, 0L)

/* void Cls_OnGetMinMaxInfo(HWND  LPMINMAXINFO lpMinMaxInfo) */
#define HANDLE_WM_GETMINMAXINFO( wParam, lParam, fn) \
    ((fn)( (LPMINMAXINFO)(lParam)), 0L)
#define FORWARD_WM_GETMINMAXINFO( lpMinMaxInfo, fn) \
    (void)(fn)( WM_GETMINMAXINFO, 0L, (LPARAM)(LPMINMAXINFO)(lpMinMaxInfo))

/* void Cls_OnPaint(HWND hwnd) */
#define HANDLE_WM_PAINT( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_PAINT( fn) \
    (void)(fn)( WM_PAINT, 0L, 0L)

/* BOOL Cls_OnEraseBkgnd(HWND  HDC hdc) */
#define HANDLE_WM_ERASEBKGND( wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)( (HDC)(wParam))
#define FORWARD_WM_ERASEBKGND( hdc, fn) \
   (BOOL)(DWORD)(fn)( WM_ERASEBKGND, (WPARAM)(HDC)(hdc), 0L)

/* BOOL Cls_OnIconEraseBkgnd(HWND  HDC hdc) */
#define HANDLE_WM_ICONERASEBKGND( wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)( (HDC)(wParam))
#define FORWARD_WM_ICONERASEBKGND( hdc, fn) \
    (BOOL)(DWORD)(fn)( WM_ICONERASEBKGND, (WPARAM)(HDC)(hdc), 0L)

/* void Cls_OnNCPaint(HWND  HRGN hrgn) */
#define HANDLE_WM_NCPAINT( wParam, lParam, fn) \
    ((fn)( (HRGN)(wParam)), 0L)
#define FORWARD_WM_NCPAINT( hrgn, fn) \
    (void)(fn)( WM_NCPAINT, (WPARAM)(HRGN)(hrgn), 0L)

/* UINT Cls_OnNCCalcSize(HWND  BOOL fCalcValidRects, NCCALCSIZE_PARAMS * lpcsp) */
#define HANDLE_WM_NCCALCSIZE( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)( (BOOL)(0), (NCCALCSIZE_PARAMS *)(lParam))
#define FORWARD_WM_NCCALCSIZE( fCalcValidRects, lpcsp, fn) \
    (UINT)(DWORD)(fn)( WM_NCCALCSIZE, 0L, (LPARAM)(NCCALCSIZE_PARAMS *)(lpcsp))

/* UINT Cls_OnNCHitTest(HWND  int x, int y) */
#define HANDLE_WM_NCHITTEST( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam))
#define FORWARD_WM_NCHITTEST( x, y, fn) \
    (UINT)(DWORD)(fn)( WM_NCHITTEST, 0L, MAKELPARAM((x), (y)))

/* HICON Cls_OnQueryDragIcon(HWND hwnd) */
#define HANDLE_WM_QUERYDRAGICON( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)()
#define FORWARD_WM_QUERYDRAGICON( fn) \
    (HICON)(UINT)(DWORD)(fn)( WM_QUERYDRAGICON, 0L, 0L)

#ifdef _INC_SHELLAPI
/* void Cls_OnDropFiles(HWND  HDROP hdrop) */
#define HANDLE_WM_DROPFILES( wParam, lParam, fn) \
    ((fn)( (HDROP)(wParam)), 0L)
#define FORWARD_WM_DROPFILES( hdrop, fn) \
    (void)(fn)( WM_DROPFILES, (WPARAM)(HDROP)(hdrop), 0L)
#endif  /* _INC_SHELLAPI */

/* void Cls_OnActivate(HWND  UINT state, HWND hwndActDeact, BOOL fMinimized) */
#define HANDLE_WM_ACTIVATE( wParam, lParam, fn) \
    ((fn)( (UINT)LOWORD(wParam), (HWND)(lParam), (BOOL)HIWORD(wParam)), 0L)
#define FORWARD_WM_ACTIVATE( state, hwndActDeact, fMinimized, fn) \
    (void)(fn)( WM_ACTIVATE, MAKEWPARAM((state), (fMinimized)), (LPARAM)(HWND)(hwndActDeact))

/* void Cls_OnActivateApp(HWND  BOOL fActivate, DWORD dwThreadId) */
#define HANDLE_WM_ACTIVATEAPP( wParam, lParam, fn) \
    ((fn)( (BOOL)(wParam), (DWORD)(lParam)), 0L)
#define FORWARD_WM_ACTIVATEAPP( fActivate, dwThreadId, fn) \
    (void)(fn)( WM_ACTIVATEAPP, (WPARAM)(BOOL)(fActivate), (LPARAM)(dwThreadId))

/* BOOL Cls_OnNCActivate(HWND  BOOL fActive, HWND hwndActDeact, BOOL fMinimized) */
#define HANDLE_WM_NCACTIVATE( wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)( (BOOL)(wParam), 0L, 0L)
#define FORWARD_WM_NCACTIVATE( fActive, hwndActDeact, fMinimized, fn) \
    (BOOL)(DWORD)(fn)( WM_NCACTIVATE, (WPARAM)(BOOL)(fActive), 0L)

/* void Cls_OnSetFocus(HWND  HWND hwndOldFocus) */
#define HANDLE_WM_SETFOCUS( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam)), 0L)
#define FORWARD_WM_SETFOCUS( hwndOldFocus, fn) \
    (void)(fn)( WM_SETFOCUS, (WPARAM)(HWND)(hwndOldFocus), 0L)

/* void Cls_OnKillFocus(HWND  HWND hwndNewFocus) */
#define HANDLE_WM_KILLFOCUS( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam)), 0L)
#define FORWARD_WM_KILLFOCUS( hwndNewFocus, fn) \
    (void)(fn)( WM_KILLFOCUS, (WPARAM)(HWND)(hwndNewFocus), 0L)

/* void Cls_OnKey(HWND  UINT vk, BOOL fDown, int cRepeat, UINT flags) */
#define HANDLE_WM_KEYDOWN( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam), TRUE, (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_KEYDOWN( vk, cRepeat, flags, fn) \
    (void)(fn)( WM_KEYDOWN, (WPARAM)(UINT)(vk), MAKELPARAM((cRepeat), (flags)))

/* void Cls_OnKey(HWND  UINT vk, BOOL fDown, int cRepeat, UINT flags) */
#define HANDLE_WM_KEYUP( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam), FALSE, (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_KEYUP( vk, cRepeat, flags, fn) \
    (void)(fn)( WM_KEYUP, (WPARAM)(UINT)(vk), MAKELPARAM((cRepeat), (flags)))

/* void Cls_OnChar(HWND  TCHAR ch, int cRepeat, UINT flags) */
#define HANDLE_WM_CHAR( wParam, lParam, fn) \
    ((fn)( (TCHAR)(wParam), (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_CHAR( ch, cRepeat, flags, fn) \
    (void)(fn)( WM_CHAR, (WPARAM)(TCHAR)(ch), MAKELPARAM((cRepeat), (flags)))

/* void Cls_OnDeadChar(HWND  TCHAR ch, int cRepeat) */
#define HANDLE_WM_DEADCHAR( wParam, lParam, fn) \
    ((fn)( (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)
#define FORWARD_WM_DEADCHAR( ch, cRepeat, fn) \
    (void)(fn)( WM_DEADCHAR, (WPARAM)(TCHAR)(ch), MAKELPARAM((cRepeat),0))

/* void Cls_OnSysKey(HWND  UINT vk, BOOL fDown, int cRepeat, UINT flags) */
#define HANDLE_WM_SYSKEYDOWN( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam), TRUE, (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_SYSKEYDOWN( vk, cRepeat, flags, fn) \
    (void)(fn)( WM_SYSKEYDOWN, (WPARAM)(UINT)(vk), MAKELPARAM((cRepeat), (flags)))

/* void Cls_OnSysKey(HWND  UINT vk, BOOL fDown, int cRepeat, UINT flags) */
#define HANDLE_WM_SYSKEYUP( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam), FALSE, (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_SYSKEYUP( vk, cRepeat, flags, fn) \
    (void)(fn)( WM_SYSKEYUP, (WPARAM)(UINT)(vk), MAKELPARAM((cRepeat), (flags)))

/* void Cls_OnSysChar(HWND  TCHAR ch, int cRepeat) */
#define HANDLE_WM_SYSCHAR( wParam, lParam, fn) \
    ((fn)( (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)
#define FORWARD_WM_SYSCHAR( ch, cRepeat, fn) \
    (void)(fn)( WM_SYSCHAR, (WPARAM)(TCHAR)(ch), MAKELPARAM((cRepeat), 0))

/* void Cls_OnSysDeadChar(HWND  TCHAR ch, int cRepeat) */
#define HANDLE_WM_SYSDEADCHAR( wParam, lParam, fn) \
    ((fn)( (TCHAR)(wParam), (int)(short)LOWORD(lParam)), 0L)
#define FORWARD_WM_SYSDEADCHAR( ch, cRepeat, fn) \
    (void)(fn)( WM_SYSDEADCHAR, (WPARAM)(TCHAR)(ch), MAKELPARAM((cRepeat), 0))

/* void Cls_OnMouseMove(HWND  int x, int y, UINT keyFlags) */
#define HANDLE_WM_MOUSEMOVE( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_MOUSEMOVE( x, y, keyFlags, fn) \
    (void)(fn)( WM_MOUSEMOVE, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

/* void Cls_OnLButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define HANDLE_WM_LBUTTONDOWN( wParam, lParam, fn) \
    ((fn)( FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_LBUTTONDOWN( fDoubleClick, x, y, keyFlags, fn) \
    (void)(fn)( (fDoubleClick) ? WM_LBUTTONDBLCLK : WM_LBUTTONDOWN, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

/* void Cls_OnLButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define HANDLE_WM_LBUTTONDBLCLK( wParam, lParam, fn) \
    ((fn)( TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

/* void Cls_OnLButtonUp(HWND  int x, int y, UINT keyFlags) */
#define HANDLE_WM_LBUTTONUP( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_LBUTTONUP( x, y, keyFlags, fn) \
    (void)(fn)( WM_LBUTTONUP, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

/* void Cls_OnRButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define HANDLE_WM_RBUTTONDOWN( wParam, lParam, fn) \
    ((fn)( FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_RBUTTONDOWN( fDoubleClick, x, y, keyFlags, fn) \
    (void)(fn)( (fDoubleClick) ? WM_RBUTTONDBLCLK : WM_RBUTTONDOWN, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

/* void Cls_OnRButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define HANDLE_WM_RBUTTONDBLCLK( wParam, lParam, fn) \
    ((fn)( TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

/* void Cls_OnRButtonUp(HWND  int x, int y, UINT flags) */
#define HANDLE_WM_RBUTTONUP( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_RBUTTONUP( x, y, keyFlags, fn) \
    (void)(fn)( WM_RBUTTONUP, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

/* void Cls_OnMButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define HANDLE_WM_MBUTTONDOWN( wParam, lParam, fn) \
    ((fn)( FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_MBUTTONDOWN( fDoubleClick, x, y, keyFlags, fn) \
    (void)(fn)( (fDoubleClick) ? WM_MBUTTONDBLCLK : WM_MBUTTONDOWN, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

/* void Cls_OnMButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT keyFlags) */
#define HANDLE_WM_MBUTTONDBLCLK( wParam, lParam, fn) \
    ((fn)( TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

/* void Cls_OnMButtonUp(HWND  int x, int y, UINT flags) */
#define HANDLE_WM_MBUTTONUP( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_MBUTTONUP( x, y, keyFlags, fn) \
    (void)(fn)( WM_MBUTTONUP, (WPARAM)(UINT)(keyFlags), MAKELPARAM((x), (y)))

/* void Cls_OnNCMouseMove(HWND  int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCMOUSEMOVE( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCMOUSEMOVE( x, y, codeHitTest, fn) \
    (void)(fn)( WM_NCMOUSEMOVE, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)))

/* void Cls_OnNCLButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCLBUTTONDOWN( wParam, lParam, fn) \
    ((fn)( FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCLBUTTONDOWN( fDoubleClick, x, y, codeHitTest, fn) \
    (void)(fn)( (fDoubleClick) ? WM_NCLBUTTONDBLCLK : WM_NCLBUTTONDOWN, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)))

/* void Cls_OnNCLButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCLBUTTONDBLCLK( wParam, lParam, fn) \
    ((fn)( TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

/* void Cls_OnNCLButtonUp(HWND  int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCLBUTTONUP( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCLBUTTONUP( x, y, codeHitTest, fn) \
    (void)(fn)( WM_NCLBUTTONUP, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)))

/* void Cls_OnNCRButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCRBUTTONDOWN( wParam, lParam, fn) \
    ((fn)( FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCRBUTTONDOWN( fDoubleClick, x, y, codeHitTest, fn) \
    (void)(fn)( (fDoubleClick) ? WM_NCRBUTTONDBLCLK : WM_NCRBUTTONDOWN, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)) )

/* void Cls_OnNCRButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCRBUTTONDBLCLK( wParam, lParam, fn) \
    ((fn)( TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

/* void Cls_OnNCRButtonUp(HWND  int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCRBUTTONUP( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCRBUTTONUP( x, y, codeHitTest, fn) \
    (void)(fn)( WM_NCRBUTTONUP, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)) )

/* void Cls_OnNCMButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCMBUTTONDOWN( wParam, lParam, fn) \
    ((fn)( FALSE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCMBUTTONDOWN( fDoubleClick, x, y, codeHitTest, fn) \
    (void)(fn)( (fDoubleClick) ? WM_NCMBUTTONDBLCLK : WM_NCMBUTTONDOWN, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)) )

/* void Cls_OnNCMButtonDown(HWND  BOOL fDoubleClick, int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCMBUTTONDBLCLK( wParam, lParam, fn) \
    ((fn)( TRUE, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)

/* void Cls_OnNCMButtonUp(HWND  int x, int y, UINT codeHitTest) */
#define HANDLE_WM_NCMBUTTONUP( wParam, lParam, fn) \
    ((fn)( (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_NCMBUTTONUP( x, y, codeHitTest, fn) \
    (void)(fn)( WM_NCMBUTTONUP, (WPARAM)(UINT)(codeHitTest), MAKELPARAM((x), (y)) )

/* int Cls_OnMouseActivate(HWND  HWND hwndTopLevel, UINT codeHitTest, UINT msg) */
#define HANDLE_WM_MOUSEACTIVATE( wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)( (HWND)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam))
#define FORWARD_WM_MOUSEACTIVATE( hwndTopLevel, codeHitTest, msg, fn) \
    (int)(DWORD)(fn)( WM_MOUSEACTIVATE, (WPARAM)(HWND)(hwndTopLevel), MAKELPARAM((codeHitTest), (msg)))

/* void Cls_OnCancelMode(HWND hwnd) */
#define HANDLE_WM_CANCELMODE( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_CANCELMODE( fn) \
    (void)(fn)( WM_CANCELMODE, 0L, 0L)

/* void Cls_OnTimer(HWND  UINT id) */
#define HANDLE_WM_TIMER( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam)), 0L)
#define FORWARD_WM_TIMER( id, fn) \
    (void)(fn)( WM_TIMER, (WPARAM)(UINT)(id), 0L)

/* void Cls_OnInitMenu(HWND  HMENU hMenu) */
#define HANDLE_WM_INITMENU( wParam, lParam, fn) \
    ((fn)( (HMENU)(wParam)), 0L)
#define FORWARD_WM_INITMENU( hMenu, fn) \
    (void)(fn)( WM_INITMENU, (WPARAM)(HMENU)(hMenu), 0L)

/* void Cls_OnInitMenuPopup(HWND  HMENU hMenu, UINT item, BOOL fSystemMenu) */
#define HANDLE_WM_INITMENUPOPUP( wParam, lParam, fn) \
    ((fn)( (HMENU)(wParam), (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam)), 0L)
#define FORWARD_WM_INITMENUPOPUP( hMenu, item, fSystemMenu, fn) \
    (void)(fn)( WM_INITMENUPOPUP, (WPARAM)(HMENU)(hMenu), MAKELPARAM((item),(fSystemMenu)))

/* void Cls_OnMenuSelect(HWND  HMENU hmenu, int item, HMENU hmenuPopup, UINT flags) */
// added && lParam to get rid of CodeGaurd warning
#define HANDLE_WM_MENUSELECT( wParam, lParam, fn)                  \
    ((fn)( (HMENU)(lParam),  \
    (int)(LOWORD(wParam)),          \
    (HIWORD(wParam) & MF_POPUP && lParam) ? GetSubMenu((HMENU)lParam, LOWORD(wParam)) : 0L, \
    (UINT)(((short)HIWORD(wParam) == -1) ? 0xFFFFFFFF : HIWORD(wParam))), 0L)
#define FORWARD_WM_MENUSELECT( hmenu, item, hmenuPopup, flags, fn) \
    (void)(fn)( WM_MENUSELECT, MAKEWPARAM((item), (flags)), (LPARAM)(HMENU)((hmenu) ? (hmenu) : (hmenuPopup)))

#define HANDLE_WM_ENTERMENULOOP( wParam, lParam, fn)  \
   ((fn)( (bool)(wParam)), 0L)
#define FORWARD_WM_ENTERMENULOOP( trackPopupMenu, fn)    \
   (void)(fn)( WM_ENTERMENULOOP, (WPARAM)(trackPopupMenu), 0L)

/* DWORD Cls_OnMenuChar(HWND  UINT ch, UINT flags, HMENU hmenu) */
#define HANDLE_WM_MENUCHAR( wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)( (UINT)(LOWORD(wParam)), (UINT)HIWORD(wParam), (HMENU)(lParam))
#define FORWARD_WM_MENUCHAR( ch, flags, hmenu, fn) \
    (DWORD)(fn)( WM_MENUCHAR, MAKEWPARAM(flags, (WORD)(ch)), (LPARAM)(HMENU)(hmenu))

/* void Cls_OnCommand(HWND  int id, HWND hwndCtl, UINT codeNotify) */
#define HANDLE_WM_COMMAND( wParam, lParam, fn) \
    ((fn)( (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L)
#define FORWARD_WM_COMMAND( id, hwndCtl, codeNotify, fn) \
    (void)(fn)( WM_COMMAND, MAKEWPARAM((UINT)(id),(UINT)(codeNotify)), (LPARAM)(HWND)(hwndCtl))

/* void Cls_OnHScroll(HWND  HWND hwndCtl, UINT code, int pos) */
#define HANDLE_WM_HSCROLL( wParam, lParam, fn) \
    ((fn)( (HWND)(lParam), (UINT)(LOWORD(wParam)), (int)(short)HIWORD(wParam)), 0L)
#define FORWARD_WM_HSCROLL( hwndCtl, code, pos, fn) \
    (void)(fn)( WM_HSCROLL, MAKEWPARAM((UINT)(int)(code),(UINT)(int)(pos)), (LPARAM)(UINT)(hwndCtl))

/* void Cls_OnVScroll(HWND  HWND hwndCtl, UINT code, int pos) */
#define HANDLE_WM_VSCROLL( wParam, lParam, fn) \
    ((fn)( (HWND)(lParam), (UINT)(LOWORD(wParam)),  (int)(short)HIWORD(wParam)), 0L)
#define FORWARD_WM_VSCROLL( hwndCtl, code, pos, fn) \
    (void)(fn)( WM_VSCROLL, MAKEWPARAM((UINT)(int)(code),(UINT)(int)(pos)), (LPARAM)(HWND)(hwndCtl))

/* void onMM_MCINotify(WPARAM wFlags, LPARAM lDev) */
#define HANDLE_MM_MCINOTIFY( wParam, lParam, fn) \
    ((fn)( wParam, lParam ), 0L)
#define FORWARD_MM_MCINOTIFY( wParam, lParam, fn) \
    (void)(fn)( MM_MCINOTIFY, wParam, lParam)

/* void onMM_MIXMControlChange(WPARAM hMixer, LPARAM dwControlId) */
#define HANDLE_MM_MIXM_CONTROL_CHANGE( wParam, lParam, fn) \
    ((fn)( wParam, lParam ), 0L)
#define FORWARD_MM_MIXM_CONTROL_CHANGE( wParam, lParam, fn) \
    (void)(fn)( MM_MIXM_CONTROL_CHANGE, wParam, lParam)

/* void onMM_MIXMControlChange(WPARAM hMixer, LPARAM dwControlId) */
#define HANDLE_MM_MIXM_LINE_CHANGE( wParam, lParam, fn) \
    ((fn)( wParam, lParam ), 0L)
#define FORWARD_MM_MIXM_LINE_CHANGE( wParam, lParam, fn) \
    (void)(fn)( MM_MIXM_LINE_CHANGE, wParam, lParam)

/* void Cls_OnCut(HWND hwnd) */
#define HANDLE_WM_CUT( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_CUT( fn) \
    (void)(fn)( WM_CUT, 0L, 0L)

/* void Cls_OnCopy(HWND hwnd) */
#define HANDLE_WM_COPY( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_COPY( fn) \
    (void)(fn)( WM_COPY, 0L, 0L)

/* void Cls_OnPaste(HWND hwnd) */
#define HANDLE_WM_PASTE( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_PASTE( fn) \
    (void)(fn)( WM_PASTE, 0L, 0L)

/* void Cls_OnClear(HWND hwnd) */
#define HANDLE_WM_CLEAR( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_CLEAR( fn) \
    (void)(fn)( WM_CLEAR, 0L, 0L)

/* void Cls_OnUndo(HWND hwnd) */
#define HANDLE_WM_UNDO( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_UNDO( fn) \
    (void)(fn)( WM_UNDO, 0L, 0L)

/* HANDLE Cls_OnRenderFormat(HWND  UINT fmt) */
#define HANDLE_WM_RENDERFORMAT( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HANDLE)(fn)( (UINT)(wParam))
#define FORWARD_WM_RENDERFORMAT( fmt, fn) \
    (HANDLE)(UINT)(DWORD)(fn)( WM_RENDERFORMAT, (WPARAM)(UINT)(fmt), 0L)

/* void Cls_OnRenderAllFormats(HWND hwnd) */
#define HANDLE_WM_RENDERALLFORMATS( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_RENDERALLFORMATS( fn) \
    (void)(fn)( WM_RENDERALLFORMATS, 0L, 0L)

/* void Cls_OnDestroyClipboard(HWND hwnd) */
#define HANDLE_WM_DESTROYCLIPBOARD( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_DESTROYCLIPBOARD( fn) \
    (void)(fn)( WM_DESTROYCLIPBOARD, 0L, 0L)

/* void Cls_OnDrawClipboard(HWND hwnd) */
#define HANDLE_WM_DRAWCLIPBOARD( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_DRAWCLIPBOARD( fn) \
    (void)(fn)( WM_DRAWCLIPBOARD, 0L, 0L)

/* void Cls_OnPaintClipboard(HWND  HWND hwndCBViewer, const LPPAINTSTRUCT lpPaintStruct) */
#define HANDLE_WM_PAINTCLIPBOARD( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam), (const LPPAINTSTRUCT)GlobalLock((HGLOBAL)(lParam))), GlobalUnlock((HGLOBAL)(lParam)), 0L)
#define FORWARD_WM_PAINTCLIPBOARD( hwndCBViewer, lpPaintStruct, fn) \
    (void)(fn)( WM_PAINTCLIPBOARD, (WPARAM)(HWND)(hwndCBViewer), (LPARAM)(LPPAINTSTRUCT)(lpPaintStruct))

/* void Cls_OnSizeClipboard(HWND  HWND hwndCBViewer, const LPRECT lprc) */
#define HANDLE_WM_SIZECLIPBOARD( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam), (const LPRECT)GlobalLock((HGLOBAL)(lParam))), GlobalUnlock((HGLOBAL)(lParam)), 0L)
#define FORWARD_WM_SIZECLIPBOARD( hwndCBViewer, lprc, fn) \
    (void)(fn)( WM_SIZECLIPBOARD, (WPARAM)(HWND)(hwndCBViewer), (LPARAM)(LPRECT)(lprc))

/* void Cls_OnVScrollClipboard(HWND  HWND hwndCBViewer, UINT code, int pos) */
#define HANDLE_WM_VSCROLLCLIPBOARD( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam), (UINT)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define FORWARD_WM_VSCROLLCLIPBOARD( hwndCBViewer, code, pos, fn) \
    (void)(fn)( WM_VSCROLLCLIPBOARD, (WPARAM)(HWND)(hwndCBViewer), MAKELPARAM((code), (pos)))

/* void Cls_OnHScrollClipboard(HWND  HWND hwndCBViewer, UINT code, int pos) */
#define HANDLE_WM_HSCROLLCLIPBOARD( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam), (UINT)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define FORWARD_WM_HSCROLLCLIPBOARD( hwndCBViewer, code, pos, fn) \
    (void)(fn)( WM_HSCROLLCLIPBOARD, (WPARAM)(HWND)(hwndCBViewer), MAKELPARAM((code), (pos)))

/* void Cls_OnAskCBFormatName(HWND  int cchMax, LPTSTR rgchName) */
#define HANDLE_WM_ASKCBFORMATNAME( wParam, lParam, fn) \
    ((fn)( (int)(wParam), (LPTSTR)(lParam)), 0L)
#define FORWARD_WM_ASKCBFORMATNAME( cchMax, rgchName, fn) \
    (void)(fn)( WM_ASKCBFORMATNAME, (WPARAM)(int)(cchMax), (LPARAM)(rgchName))

/* void Cls_OnChangeCBChain(HWND  HWND hwndRemove, HWND hwndNext) */
#define HANDLE_WM_CHANGECBCHAIN( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam), (HWND)(lParam)), 0L)
#define FORWARD_WM_CHANGECBCHAIN( hwndRemove, hwndNext, fn) \
    (void)(fn)( WM_CHANGECBCHAIN, (WPARAM)(HWND)(hwndRemove), (LPARAM)(HWND)(hwndNext))

/* BOOL Cls_OnSetCursor(HWND  HWND hwndCursor, UINT codeHitTest, UINT msg) */
#define HANDLE_WM_SETCURSOR( wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)(fn)( (HWND)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam))
#define FORWARD_WM_SETCURSOR( hwndCursor, codeHitTest, msg, fn) \
    (BOOL)(DWORD)(fn)( WM_SETCURSOR, (WPARAM)(HWND)(hwndCursor), MAKELPARAM((codeHitTest), (msg)))

/* void Cls_OnSysCommand(HWND  UINT cmd, int x, int y) */
#define HANDLE_WM_SYSCOMMAND( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define FORWARD_WM_SYSCOMMAND( cmd, x, y, fn) \
    (void)(fn)( WM_SYSCOMMAND, (WPARAM)(UINT)(cmd), MAKELPARAM((x), (y)))

/* HWND Cls_MDICreate(HWND  const LPMDICREATESTRUCT lpmcs) */
#define HANDLE_WM_MDICREATE( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)( (LPMDICREATESTRUCT)(lParam))
#define FORWARD_WM_MDICREATE( lpmcs, fn) \
    (HWND)(UINT)(DWORD)(fn)( WM_MDICREATE, 0L, (LPARAM)(LPMDICREATESTRUCT)(lpmcs))

/* void Cls_MDIDestroy(HWND  HWND hwndDestroy) */
#define HANDLE_WM_MDIDESTROY( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam)), 0L)
#define FORWARD_WM_MDIDESTROY( hwndDestroy, fn) \
    (void)(fn)( WM_MDIDESTROY, (WPARAM)(hwndDestroy), 0L)

/* NOTE: Usable only by MDI client windows */
/* void Cls_MDIActivate(HWND  BOOL fActive, HWND hwndActivate, HWND hwndDeactivate) */
#define HANDLE_WM_MDIACTIVATE( wParam, lParam, fn) \
    ((fn)( (BOOL)(lParam == (LPARAM)hWnd), (HWND)(lParam), (HWND)(wParam)), 0L)
#define FORWARD_WM_MDIACTIVATE( fActive, hwndActivate, hwndDeactivate, fn) \
    (void)(fn)( WM_MDIACTIVATE, (WPARAM)(hwndDeactivate), (LPARAM)(hwndActivate))

/* void Cls_MDIRestore(HWND  HWND hwndRestore) */
#define HANDLE_WM_MDIRESTORE( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam)), 0L)
#define FORWARD_WM_MDIRESTORE( hwndRestore, fn) \
    (void)(fn)( WM_MDIRESTORE, (WPARAM)(hwndRestore), 0L)

/* HWND Cls_MDINext(HWND  HWND hwndCur, BOOL fPrev) */
#define HANDLE_WM_MDINEXT( wParam, lParam, fn) \
    (LRESULT)(HWND)(fn)( (HWND)(wParam), (BOOL)lParam)
#define FORWARD_WM_MDINEXT( hwndCur, fPrev, fn) \
    (HWND)(UINT)(DWORD)(fn)( WM_MDINEXT, (WPARAM)(hwndCur), (LPARAM)(fPrev))

/* void Cls_MDIMaximize(HWND  HWND hwndMaximize) */
#define HANDLE_WM_MDIMAXIMIZE( wParam, lParam, fn) \
    ((fn)( (HWND)(wParam)), 0L)
#define FORWARD_WM_MDIMAXIMIZE( hwndMaximize, fn) \
    (void)(fn)( WM_MDIMAXIMIZE, (WPARAM)(hwndMaximize), 0L)

/* BOOL Cls_MDITile(HWND  UINT cmd) */
#define HANDLE_WM_MDITILE( wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)( (UINT)(wParam))
#define FORWARD_WM_MDITILE( cmd, fn) \
    (BOOL)(DWORD)(fn)( WM_MDITILE, (WPARAM)(cmd), 0L)

/* BOOL Cls_MDICascade(HWND  UINT cmd) */
#define HANDLE_WM_MDICASCADE( wParam, lParam, fn) \
    (LRESULT)(DWORD)(fn)( (UINT)(wParam))
#define FORWARD_WM_MDICASCADE( cmd, fn) \
    (BOOL)(DWORD)(fn)( WM_MDICASCADE, (WPARAM)(cmd), 0L)

/* void Cls_MDIIconArrange(HWND hwnd) */
#define HANDLE_WM_MDIICONARRANGE( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_MDIICONARRANGE( fn) \
    (void)(fn)( WM_MDIICONARRANGE, 0L, 0L)

/* HWND Cls_MDIGetActive(HWND hwnd) */
#define HANDLE_WM_MDIGETACTIVE( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)()
#define FORWARD_WM_MDIGETACTIVE( fn) \
    (HWND)(UINT)(DWORD)(fn)( WM_MDIGETACTIVE, 0L, 0L)

/* HMENU Cls_MDISetMenu(HWND  BOOL fRefresh, HMENU hmenuFrame, HMENU hmenuWindow) */
#define HANDLE_WM_MDISETMENU( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)( (BOOL)(wParam), (HMENU)(wParam), (HMENU)(lParam))
#define FORWARD_WM_MDISETMENU( fRefresh, hmenuFrame, hmenuWindow, fn) \
    (HMENU)(UINT)(DWORD)(fn)( WM_MDISETMENU, (WPARAM)((fRefresh) ? (hmenuFrame) : 0), (LPARAM)(hmenuWindow))

/* void Cls_OnChildActivate(HWND hwnd) */
#define HANDLE_WM_CHILDACTIVATE( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_CHILDACTIVATE( fn) \
    (void)(fn)( WM_CHILDACTIVATE, 0L, 0L)

/* BOOL Cls_OnInitDialog(HWND  HWND hwndFocus, LPARAM lParam) */
#define HANDLE_WM_INITDIALOG( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(BOOL)(fn)( (HWND)(wParam), lParam)
#define FORWARD_WM_INITDIALOG( hwndFocus, lParam, fn) \
    (BOOL)(DWORD)(fn)( WM_INITDIALOG, (WPARAM)(HWND)(hwndFocus), (lParam))

/* HWND Cls_OnNextDlgCtl(HWND  HWND hwndSetFocus, BOOL fNext) */
#define HANDLE_WM_NEXTDLGCTL( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HWND)(fn)( (HWND)(wParam), (BOOL)(lParam))
#define FORWARD_WM_NEXTDLGCTL( hwndSetFocus, fNext, fn) \
    (HWND)(UINT)(DWORD)(fn)( WM_NEXTDLGCTL, (WPARAM)(HWND)(hwndSetFocus), (LPARAM)(fNext))

/* void Cls_OnParentNotify(HWND  UINT msg, HWND hwndChild, int idChild) */
#define HANDLE_WM_PARENTNOTIFY( wParam, lParam, fn) \
    ((fn)( (UINT)LOWORD(wParam), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L)
#define FORWARD_WM_PARENTNOTIFY( msg, hwndChild, idChild, fn) \
    (void)(fn)( WM_PARENTNOTIFY, MAKEWPARAM(msg, idChild), (LPARAM)(hwndChild))

/* void Cls_OnEnterIdle(HWND  UINT source, HWND hwndSource) */
#define HANDLE_WM_ENTERIDLE( wParam, lParam, fn) \
    ((fn)( (UINT)(wParam), (HWND)(lParam)), 0L)
#define FORWARD_WM_ENTERIDLE( source, hwndSource, fn) \
    (void)(fn)( WM_ENTERIDLE, (WPARAM)(UINT)(source), (LPARAM)(HWND)(hwndSource))

/* UINT Cls_OnGetDlgCode(HWND  LPMSG lpmsg) */
#define HANDLE_WM_GETDLGCODE( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)( (LPMSG)(lParam))
#define FORWARD_WM_GETDLGCODE( lpmsg, fn) \
    (UINT)(DWORD)(fn)( WM_GETDLGCODE, (lpmsg ? lpmsg->wParam : 0), (LPARAM)(LPMSG)(lpmsg))

/* HBRUSH Cls_OnCtlColor(HWND  HDC hdc, HWND hwndChild, int type) */
#define HANDLE_WM_CTLCOLORMSGBOX( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)( (HDC)(wParam), (HWND)(lParam), CTLCOLOR_MSGBOX)
#define FORWARD_WM_CTLCOLORMSGBOX( hdc, hwndChild, fn) \
    (HBRUSH)(UINT)(DWORD)(fn)( WM_CTLCOLORMSGBOX, (WPARAM)(HDC)(hdc), (LPARAM)(HWND)(hwndChild))

#define HANDLE_WM_CTLCOLOREDIT( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)( (HDC)(wParam), (HWND)(lParam), CTLCOLOR_EDIT)
#define FORWARD_WM_CTLCOLOREDIT( hdc, hwndChild, fn) \
    (HBRUSH)(UINT)(DWORD)(fn)( WM_CTLCOLOREDIT, (WPARAM)(HDC)(hdc), (LPARAM)(HWND)(hwndChild))

#define HANDLE_WM_CTLCOLORLISTBOX( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)( (HDC)(wParam), (HWND)(lParam), CTLCOLOR_LISTBOX)
#define FORWARD_WM_CTLCOLORLISTBOX( hdc, hwndChild, fn) \
    (HBRUSH)(UINT)(DWORD)(fn)( WM_CTLCOLORLISTBOX, (WPARAM)(HDC)(hdc), (LPARAM)(HWND)(hwndChild))

#define HANDLE_WM_CTLCOLORBTN( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)( (HDC)(wParam), (HWND)(lParam), CTLCOLOR_BTN)
#define FORWARD_WM_CTLCOLORBTN( hdc, hwndChild, fn) \
    (HBRUSH)(UINT)(DWORD)(fn)( WM_CTLCOLORBTN, (WPARAM)(HDC)(hdc), (LPARAM)(HWND)(hwndChild))

#define HANDLE_WM_CTLCOLORDLG( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)( (HDC)(wParam), (HWND)(lParam), CTLCOLOR_DLG)
#define FORWARD_WM_CTLCOLORDLG( hdc, hwndChild, fn) \
    (HBRUSH)(UINT)(DWORD)(fn)( WM_CTLCOLORDLG, (WPARAM)(HDC)(hdc), (LPARAM)(HWND)(hwndChild))

#define HANDLE_WM_CTLCOLORSCROLLBAR( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)( (HDC)(wParam), (HWND)(lParam), CTLCOLOR_SCROLLBAR)
#define FORWARD_WM_CTLCOLORSCROLLBAR( hdc, hwndChild, fn) \
    (HBRUSH)(UINT)(DWORD)(fn)( WM_CTLCOLORSCROLLBAR, (WPARAM)(HDC)(hdc), (LPARAM)(HWND)(hwndChild))

#define HANDLE_WM_CTLCOLORSTATIC( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)( (HDC)(wParam), (HWND)(lParam), CTLCOLOR_STATIC)
#define FORWARD_WM_CTLCOLORSTATIC( hdc, hwndChild, fn) \
    (HBRUSH)(UINT)(DWORD)(fn)( WM_CTLCOLORSTATIC, (WPARAM)(HDC)(hdc), (LPARAM)(HWND)(hwndChild))

/* void Cls_OnSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw) */
#define HANDLE_WM_SETFONT( wParam, lParam, fn) \
    ((fn)( (HFONT)(wParam), (BOOL)(lParam)), 0L)
#define FORWARD_WM_SETFONT( hfont, fRedraw, fn) \
    (void)(fn)( WM_SETFONT, (WPARAM)(HFONT)(hfont), (LPARAM)(BOOL)(fRedraw))

/* HFONT Cls_OnGetFont(HWND hwnd) */
#define HANDLE_WM_GETFONT( wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HFONT)(fn)()
#define FORWARD_WM_GETFONT( fn) \
    (HFONT)(UINT)(DWORD)(fn)( WM_GETFONT, 0L, 0L)

/* void Cls_OnDrawItem(HWND  const DRAWITEMSTRUCT * lpDrawItem) */
#define HANDLE_WM_DRAWITEM( wParam, lParam, fn) \
    ((fn)( (const DRAWITEMSTRUCT *)(lParam)), 0L)
#define FORWARD_WM_DRAWITEM( lpDrawItem, fn) \
    (void)(fn)( WM_DRAWITEM, (WPARAM)(((const DRAWITEMSTRUCT *)lpDrawItem)->CtlID), (LPARAM)(const DRAWITEMSTRUCT *)(lpDrawItem))

/* void Cls_OnMeasureItem(HWND  MEASUREITEMSTRUCT * lpMeasureItem) */
#define HANDLE_WM_MEASUREITEM( wParam, lParam, fn) \
    ((fn)( (MEASUREITEMSTRUCT *)(lParam)), 0L)
#define FORWARD_WM_MEASUREITEM( lpMeasureItem, fn) \
    (void)(fn)( WM_MEASUREITEM, (WPARAM)(((MEASUREITEMSTRUCT *)lpMeasureItem)->CtlID), (LPARAM)(MEASUREITEMSTRUCT *)(lpMeasureItem))

/* void Cls_OnDeleteItem(HWND  const DELETEITEMSTRUCT * lpDeleteItem) */
#define HANDLE_WM_DELETEITEM( wParam, lParam, fn) \
    ((fn)( (const DELETEITEMSTRUCT *)(lParam)), 0L)
#define FORWARD_WM_DELETEITEM( lpDeleteItem, fn) \
    (void)(fn)( WM_DELETEITEM, (WPARAM)(((const DELETEITEMSTRUCT *)(lpDeleteItem))->CtlID), (LPARAM)(const DELETEITEMSTRUCT *)(lpDeleteItem))

/* int Cls_OnCompareItem(HWND  const COMPAREITEMSTRUCT * lpCompareItem) */
#define HANDLE_WM_COMPAREITEM( wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)( (const COMPAREITEMSTRUCT *)(lParam))
#define FORWARD_WM_COMPAREITEM( lpCompareItem, fn) \
    (int)(DWORD)(fn)( WM_COMPAREITEM, (WPARAM)(((const COMPAREITEMSTRUCT *)(lpCompareItem))->CtlID), (LPARAM)(const COMPAREITEMSTRUCT *)(lpCompareItem))

/* int Cls_OnVkeyToItem(HWND  UINT vk, HWND hwndListbox, int iCaret) */
#define HANDLE_WM_VKEYTOITEM( wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)( (UINT)LOWORD(wParam), (HWND)(lParam), (int)(short)HIWORD(wParam))
#define FORWARD_WM_VKEYTOITEM( vk, hwndListBox, iCaret, fn) \
    (int)(DWORD)(fn)( WM_VKEYTOITEM, MAKEWPARAM((vk), (iCaret)), (LPARAM)(hwndListBox))

/* int Cls_OnCharToItem(HWND  UINT ch, HWND hwndListbox, int iCaret) */
#define HANDLE_WM_CHARTOITEM( wParam, lParam, fn) \
    (LRESULT)(DWORD)(int)(fn)( (UINT)LOWORD(wParam), (HWND)(lParam), (int)(short)HIWORD(wParam))
#define FORWARD_WM_CHARTOITEM( ch, hwndListBox, iCaret, fn) \
    (int)(DWORD)(fn)( WM_CHARTOITEM, MAKEWPARAM((UINT)(iCaret), (UINT)(ch)), (LPARAM)(hwndListBox))

/* void Cls_OnQueueSync(HWND hwnd) */
#define HANDLE_WM_QUEUESYNC( wParam, lParam, fn) \
    ((fn)(),0L)
#define FORWARD_WM_QUEUESYNC( fn) \
    (void)(fn)( WM_QUEUESYNC, 0L, 0L)

#if (WINVER >= 0x030a)
/* void Cls_OnCommNotify(HWND  int cid, UINT flags) */
#define HANDLE_WM_COMMNOTIFY( wParam, lParam, fn) \
    ((fn)( (int)(wParam), (UINT)LOWORD(lParam)), 0L)
#define FORWARD_WM_COMMNOTIFY( cid, flags, fn) \
    (void)(fn)( WM_COMMNOTIFY, (WPARAM)(cid), MAKELPARAM((flags), 0))
#endif

/* void Cls_OnDeviceChange( ... ) */
#define HANDLE_WM_DEVICECHANGE(wParam, lParam, fn) \
    ((fn)(wParam, lParam),0L)
#define FORWARD_WM_DEVICECHANGE(fn, wParam, lParam) \
    (void)(fn)(WM_DEVICECHANGE, wParam, lParam)

/****** Static control message APIs ******************************************/

#define Static_Enable(hwndCtl, fEnable)         EnableWindow((hwndCtl), (fEnable))

#define Static_GetText(hwndCtl, lpch, cchMax)   GetWindowText((hwndCtl), (lpch), (cchMax))
#define Static_GetTextLength(hwndCtl)           GetWindowTextLength(hwndCtl)
#define Static_SetText(hwndCtl, lpsz)           SetWindowText((hwndCtl), (lpsz))

#define Static_SetIcon(hwndCtl, hIcon)          ((HICON)(UINT)(DWORD)SendMessage((hwndCtl), STM_SETICON, (WPARAM)(HICON)(hIcon), 0L))
#define Static_GetIcon(hwndCtl, hIcon)          ((HICON)(UINT)(DWORD)SendMessage((hwndCtl), STM_GETICON, 0L, 0L))

/****** Button control message APIs ******************************************/

#define Button_Enable(hwndCtl, fEnable)         EnableWindow((hwndCtl), (fEnable))

#define Button_GetText(hwndCtl, lpch, cchMax)   GetWindowText((hwndCtl), (lpch), (cchMax))
#define Button_GetTextLength(hwndCtl)           GetWindowTextLength(hwndCtl)
#define Button_SetText(hwndCtl, lpsz)           SetWindowText((hwndCtl), (lpsz))

#define Button_GetCheck(hwndCtl)            ((int)(DWORD)SendMessage((hwndCtl), BM_GETCHECK, 0L, 0L))
#define Button_SetCheck(hwndCtl, check)     ((void)SendMessage((hwndCtl), BM_SETCHECK, (WPARAM)(int)(check), 0L))

#define Button_GetState(hwndCtl)            ((int)(DWORD)SendMessage((hwndCtl), BM_GETSTATE, 0L, 0L))
#define Button_SetState(hwndCtl, state)     ((UINT)(DWORD)SendMessage((hwndCtl), BM_SETSTATE, (WPARAM)(int)(state), 0L))

#define Button_SetStyle(hwndCtl, style, fRedraw) ((void)SendMessage((hwndCtl), BM_SETSTYLE, (WPARAM)LOWORD(style), MAKELPARAM(((fRedraw) ? TRUE : FALSE), 0)))

/****** Edit control message APIs ********************************************/

#define Edit_Enable(hwndCtl, fEnable)           EnableWindow((hwndCtl), (fEnable))

#define Edit_GetText(hwndCtl, lpch, cchMax)     GetWindowText((hwndCtl), (lpch), (cchMax))
#define Edit_GetTextLength(hwndCtl)             GetWindowTextLength(hwndCtl)
#define Edit_SetText(hwndCtl, lpsz)             SetWindowText((hwndCtl), (lpsz))

#define Edit_LimitText(hwndCtl, cchMax)         ((void)SendMessage((hwndCtl), EM_LIMITTEXT, (WPARAM)(cchMax), 0L))

#define Edit_GetLineCount(hwndCtl)              ((int)(DWORD)SendMessage((hwndCtl), EM_GETLINECOUNT, 0L, 0L))
#define Edit_GetLine(hwndCtl, line, lpch, cchMax) ((*((int *)(lpch)) = (cchMax)), ((int)(DWORD)SendMessage((hwndCtl), EM_GETLINE, (WPARAM)(int)(line), (LPARAM)(LPTSTR)(lpch))))

#define Edit_GetRect(hwndCtl, lprc)             ((void)SendMessage((hwndCtl), EM_GETRECT, 0L, (LPARAM)(RECT *)(lprc)))
#define Edit_SetRect(hwndCtl, lprc)             ((void)SendMessage((hwndCtl), EM_SETRECT, 0L, (LPARAM)(const RECT *)(lprc)))
#define Edit_SetRectNoPaint(hwndCtl, lprc)      ((void)SendMessage((hwndCtl), EM_SETRECTNP, 0L, (LPARAM)(const RECT *)(lprc)))

#define Edit_GetSel(hwndCtl)                    ((DWORD)SendMessage((hwndCtl), EM_GETSEL, 0L, 0L))
#define Edit_SetSel(hwndCtl, ichStart, ichEnd)  ((void)SendMessage((hwndCtl), EM_SETSEL, (ichStart), (ichEnd)))
#define Edit_ReplaceSel(hwndCtl, lpszReplace)   ((void)SendMessage((hwndCtl), EM_REPLACESEL, 0L, (LPARAM)(LPCTSTR)(lpszReplace)))

#define Edit_GetModify(hwndCtl)                 ((BOOL)(DWORD)SendMessage((hwndCtl), EM_GETMODIFY, 0L, 0L))
#define Edit_SetModify(hwndCtl, fModified)      ((void)SendMessage((hwndCtl), EM_SETMODIFY, (WPARAM)(UINT)(fModified), 0L))

#define Edit_ScrollCaret(hwndCtl)               ((BOOL)(DWORD)SendMessage((hwndCtl), EM_SCROLLCARET, 0, 0L))

#define Edit_LineFromChar(hwndCtl, ich)         ((int)(DWORD)SendMessage((hwndCtl), EM_LINEFROMCHAR, (WPARAM)(int)(ich), 0L))
#define Edit_LineIndex(hwndCtl, line)           ((int)(DWORD)SendMessage((hwndCtl), EM_LINEINDEX, (WPARAM)(int)(line), 0L))
#define Edit_LineLength(hwndCtl, line)          ((int)(DWORD)SendMessage((hwndCtl), EM_LINELENGTH, (WPARAM)(int)(line), 0L))

#define Edit_Scroll(hwndCtl, dv, dh)            ((void)SendMessage((hwndCtl), EM_LINESCROLL, (WPARAM)(dh), (LPARAM)(dv)))

#define Edit_CanUndo(hwndCtl)                   ((BOOL)(DWORD)SendMessage((hwndCtl), EM_CANUNDO, 0L, 0L))
#define Edit_Undo(hwndCtl)                      ((BOOL)(DWORD)SendMessage((hwndCtl), EM_UNDO, 0L, 0L))
#define Edit_EmptyUndoBuffer(hwndCtl)           ((void)SendMessage((hwndCtl), EM_EMPTYUNDOBUFFER, 0L, 0L))

#define Edit_SetPasswordChar(hwndCtl, ch)       ((void)SendMessage((hwndCtl), EM_SETPASSWORDCHAR, (WPARAM)(UINT)(ch), 0L))

#define Edit_SetTabStops(hwndCtl, cTabs, lpTabs) ((void)SendMessage((hwndCtl), EM_SETTABSTOPS, (WPARAM)(int)(cTabs), (LPARAM)(const int *)(lpTabs)))

#define Edit_FmtLines(hwndCtl, fAddEOL)         ((BOOL)(DWORD)SendMessage((hwndCtl), EM_FMTLINES, (WPARAM)(BOOL)(fAddEOL), 0L))

#define Edit_GetHandle(hwndCtl)                 ((HLOCAL)(UINT)(DWORD)SendMessage((hwndCtl), EM_GETHANDLE, 0L, 0L))
#define Edit_SetHandle(hwndCtl, h)              ((void)SendMessage((hwndCtl), EM_SETHANDLE, (WPARAM)(UINT)(HLOCAL)(h), 0L))

#if (WINVER >= 0x030a)
#define Edit_GetFirstVisibleLine(hwndCtl)       ((int)(DWORD)SendMessage((hwndCtl), EM_GETFIRSTVISIBLELINE, 0L, 0L))

#define Edit_SetReadOnly(hwndCtl, fReadOnly)    ((BOOL)(DWORD)SendMessage((hwndCtl), EM_SETREADONLY, (WPARAM)(BOOL)(fReadOnly), 0L))

#define Edit_GetPasswordChar(hwndCtl)           ((TCHAR)(DWORD)SendMessage((hwndCtl), EM_GETPASSWORDCHAR, 0L, 0L))

#define Edit_SetWordBreakProc(hwndCtl, lpfnWordBreak) ((void)SendMessage((hwndCtl), EM_SETWORDBREAKPROC, 0L, (LPARAM)(EDITWORDBREAKPROC)(lpfnWordBreak)))
#define Edit_GetWordBreakProc(hwndCtl)          ((EDITWORDBREAKPROC)SendMessage((hwndCtl), EM_GETWORDBREAKPROC, 0L, 0L))
#endif /* WINVER >= 0x030a */

/****** ScrollBar control message APIs ***************************************/

/* NOTE: flags parameter is a collection of ESB_* values, NOT a boolean! */
#define ScrollBar_Enable(hwndCtl, flags)            EnableScrollBar((hwndCtl), SB_CTL, (flags))

#define ScrollBar_Show(hwndCtl, fShow)              ShowWindow((hwndCtl), (fShow) ? SW_SHOWNORMAL : SW_HIDE)

#define ScrollBar_SetPos(hwndCtl, pos, fRedraw)     SetScrollPos((hwndCtl), SB_CTL, (pos), (fRedraw))
#define ScrollBar_GetPos(hwndCtl)                   GetScrollPos((hwndCtl), SB_CTL)

#define ScrollBar_SetRange(hwndCtl, posMin, posMax, fRedraw)    SetScrollRange((hwndCtl), SB_CTL, (posMin), (posMax), (fRedraw))
#define ScrollBar_GetRange(hwndCtl, lpposMin, lpposMax)         GetScrollRange((hwndCtl), SB_CTL, (lpposMin), (lpposMax))

/****** ListBox control message APIs *****************************************/

#define ListBox_Enable(hwndCtl, fEnable)            EnableWindow((hwndCtl), (fEnable))

#define ListBox_GetCount(hwndCtl)                   ((int)(DWORD)SendMessage((hwndCtl), LB_GETCOUNT, 0L, 0L))
#define ListBox_ResetContent(hwndCtl)               ((BOOL)(DWORD)SendMessage((hwndCtl), LB_RESETCONTENT, 0L, 0L))

#define ListBox_AddString(hwndCtl, lpsz)            ((int)(DWORD)SendMessage((hwndCtl), LB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)(lpsz)))
#define ListBox_InsertString(hwndCtl, index, lpsz)  ((int)(DWORD)SendMessage((hwndCtl), LB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(LPCTSTR)(lpsz)))

#define ListBox_AddItemData(hwndCtl, data)          ((int)(DWORD)SendMessage((hwndCtl), LB_ADDSTRING, 0L, (LPARAM)(data)))
#define ListBox_InsertItemData(hwndCtl, index, data) ((int)(DWORD)SendMessage((hwndCtl), LB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(data)))

#define ListBox_DeleteString(hwndCtl, index)        ((int)(DWORD)SendMessage((hwndCtl), LB_DELETESTRING, (WPARAM)(int)(index), 0L))

#define ListBox_GetTextLen(hwndCtl, index)          ((int)(DWORD)SendMessage((hwndCtl), LB_GETTEXTLEN, (WPARAM)(int)(index), 0L))
#define ListBox_GetText(hwndCtl, index, lpszBuffer)  ((int)(DWORD)SendMessage((hwndCtl), LB_GETTEXT, (WPARAM)(int)(index), (LPARAM)(LPCTSTR)(lpszBuffer)))

#define ListBox_GetItemData(hwndCtl, index)         ((LRESULT)(DWORD)SendMessage((hwndCtl), LB_GETITEMDATA, (WPARAM)(int)(index), 0L))
#define ListBox_SetItemData(hwndCtl, index, data)   ((int)(DWORD)SendMessage((hwndCtl), LB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data)))

#if (WINVER >= 0x030a)
#define ListBox_FindString(hwndCtl, indexStart, lpszFind) ((int)(DWORD)SendMessage((hwndCtl), LB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind)))
#define ListBox_FindItemData(hwndCtl, indexStart, data) ((int)(DWORD)SendMessage((hwndCtl), LB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ListBox_SetSel(hwndCtl, fSelect, index)     ((int)(DWORD)SendMessage((hwndCtl), LB_SETSEL, (WPARAM)(BOOL)(fSelect), (LPARAM)(index)))
#define ListBox_SelItemRange(hwndCtl, fSelect, first, last)    ((int)(DWORD)SendMessage((hwndCtl), LB_SELITEMRANGE, (WPARAM)(BOOL)(fSelect), MAKELPARAM((first), (last))))

#define ListBox_GetCurSel(hwndCtl)                  ((int)(DWORD)SendMessage((hwndCtl), LB_GETCURSEL, 0L, 0L))
#define ListBox_SetCurSel(hwndCtl, index)           ((int)(DWORD)SendMessage((hwndCtl), LB_SETCURSEL, (WPARAM)(int)(index), 0L))

#define ListBox_SelectString(hwndCtl, indexStart, lpszFind) ((int)(DWORD)SendMessage((hwndCtl), LB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind)))
#define ListBox_SelectItemData(hwndCtl, indexStart, data)   ((int)(DWORD)SendMessage((hwndCtl), LB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ListBox_GetSel(hwndCtl, index)              ((int)(DWORD)SendMessage((hwndCtl), LB_GETSEL, (WPARAM)(int)(index), 0L))
#define ListBox_GetSelCount(hwndCtl)                ((int)(DWORD)SendMessage((hwndCtl), LB_GETSELCOUNT, 0L, 0L))
#define ListBox_GetTopIndex(hwndCtl)                ((int)(DWORD)SendMessage((hwndCtl), LB_GETTOPINDEX, 0L, 0L))
#define ListBox_GetSelItems(hwndCtl, cItems, lpItems) ((int)(DWORD)SendMessage((hwndCtl), LB_GETSELITEMS, (WPARAM)(int)(cItems), (LPARAM)(int *)(lpItems)))

#define ListBox_SetTopIndex(hwndCtl, indexTop)      ((int)(DWORD)SendMessage((hwndCtl), LB_SETTOPINDEX, (WPARAM)(int)(indexTop), 0L))

#define ListBox_SetColumnWidth(hwndCtl, cxColumn)   ((void)SendMessage((hwndCtl), LB_SETCOLUMNWIDTH, (WPARAM)(int)(cxColumn), 0L))
#define ListBox_GetHorizontalExtent(hwndCtl)        ((int)(DWORD)SendMessage((hwndCtl), LB_GETHORIZONTALEXTENT, 0L, 0L))
#define ListBox_SetHorizontalExtent(hwndCtl, cxExtent)     ((void)SendMessage((hwndCtl), LB_SETHORIZONTALEXTENT, (WPARAM)(int)(cxExtent), 0L))

#define ListBox_SetTabStops(hwndCtl, cTabs, lpTabs) ((BOOL)(DWORD)SendMessage((hwndCtl), LB_SETTABSTOPS, (WPARAM)(int)(cTabs), (LPARAM)(int *)(lpTabs)))

#define ListBox_GetItemRect(hwndCtl, index, lprc)   ((int)(DWORD)SendMessage((hwndCtl), LB_GETITEMRECT, (WPARAM)(int)(index), (LPARAM)(RECT *)(lprc)))

#define ListBox_SetCaretIndex(hwndCtl, index)       ((int)(DWORD)SendMessage((hwndCtl), LB_SETCARETINDEX, (WPARAM)(int)(index), 0L))
#define ListBox_GetCaretIndex(hwndCtl)              ((int)(DWORD)SendMessage((hwndCtl), LB_GETCARETINDEX, 0L, 0L))

#define ListBox_FindStringExact(hwndCtl, indexStart, lpszFind) ((int)(DWORD)SendMessage((hwndCtl), LB_FINDSTRINGEXACT, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind)))

#define ListBox_SetItemHeight(hwndCtl, index, cy)   ((int)(DWORD)SendMessage((hwndCtl), LB_SETITEMHEIGHT, (WPARAM)(int)(index), MAKELPARAM((cy), 0)))
#define ListBox_GetItemHeight(hwndCtl, index)       ((int)(DWORD)SendMessage((hwndCtl), LB_GETITEMHEIGHT, (WPARAM)(int)(index), 0L))
#endif  /* WINVER >= 0x030a */

#define ListBox_Dir(hwndCtl, attrs, lpszFileSpec)   ((int)(DWORD)SendMessage((hwndCtl), LB_DIR, (WPARAM)(UINT)(attrs), (LPARAM)(LPCTSTR)(lpszFileSpec)))

/****** ComboBox control message APIs ****************************************/

#define ComboBox_Enable(hwndCtl, fEnable)       EnableWindow((hwndCtl), (fEnable))

#define ComboBox_GetText(hwndCtl, lpch, cchMax) GetWindowText((hwndCtl), (lpch), (cchMax))
#define ComboBox_GetTextLength(hwndCtl)         GetWindowTextLength(hwndCtl)
#define ComboBox_SetText(hwndCtl, lpsz)         SetWindowText((hwndCtl), (lpsz))

#define ComboBox_LimitText(hwndCtl, cchLimit)   ((int)(DWORD)SendMessage((hwndCtl), CB_LIMITTEXT, (WPARAM)(int)(cchLimit), 0L))

#define ComboBox_GetEditSel(hwndCtl)            ((DWORD)SendMessage((hwndCtl), CB_GETEDITSEL, 0L, 0L))
#define ComboBox_SetEditSel(hwndCtl, ichStart, ichEnd) ((int)(DWORD)SendMessage((hwndCtl), CB_SETEDITSEL, 0L, MAKELPARAM((ichStart), (ichEnd))))

#define ComboBox_GetCount(hwndCtl)              ((int)(DWORD)SendMessage((hwndCtl), CB_GETCOUNT, 0L, 0L))
#define ComboBox_ResetContent(hwndCtl)          ((int)(DWORD)SendMessage((hwndCtl), CB_RESETCONTENT, 0L, 0L))

#define ComboBox_AddString(hwndCtl, lpsz)       ((int)(DWORD)SendMessage((hwndCtl), CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)(lpsz)))
#define ComboBox_InsertString(hwndCtl, index, lpsz) ((int)(DWORD)SendMessage((hwndCtl), CB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(LPCTSTR)(lpsz)))

#define ComboBox_AddItemData(hwndCtl, data)     ((int)(DWORD)SendMessage((hwndCtl), CB_ADDSTRING, 0L, (LPARAM)(data)))
#define ComboBox_InsertItemData(hwndCtl, index, data) ((int)(DWORD)SendMessage((hwndCtl), CB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(data)))

#define ComboBox_DeleteString(hwndCtl, index)   ((int)(DWORD)SendMessage((hwndCtl), CB_DELETESTRING, (WPARAM)(int)(index), 0L))

#define ComboBox_GetLBTextLen(hwndCtl, index)           ((int)(DWORD)SendMessage((hwndCtl), CB_GETLBTEXTLEN, (WPARAM)(int)(index), 0L))
#define ComboBox_GetLBText(hwndCtl, index, lpszBuffer)  ((int)(DWORD)SendMessage((hwndCtl), CB_GETLBTEXT, (WPARAM)(int)(index), (LPARAM)(LPCTSTR)(lpszBuffer)))

#define ComboBox_GetItemData(hwndCtl, index)        ((LRESULT)(DWORD)SendMessage((hwndCtl), CB_GETITEMDATA, (WPARAM)(int)(index), 0L))
#define ComboBox_SetItemData(hwndCtl, index, data)  ((int)(DWORD)SendMessage((hwndCtl), CB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data)))

#define ComboBox_FindString(hwndCtl, indexStart, lpszFind)  ((int)(DWORD)SendMessage((hwndCtl), CB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind)))
#define ComboBox_FindItemData(hwndCtl, indexStart, data)    ((int)(DWORD)SendMessage((hwndCtl), CB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ComboBox_GetCurSel(hwndCtl)                 ((int)(DWORD)SendMessage((hwndCtl), CB_GETCURSEL, 0L, 0L))
#define ComboBox_SetCurSel(hwndCtl, index)          ((int)(DWORD)SendMessage((hwndCtl), CB_SETCURSEL, (WPARAM)(int)(index), 0L))

#define ComboBox_SelectString(hwndCtl, indexStart, lpszSelect)  ((int)(DWORD)SendMessage((hwndCtl), CB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszSelect)))
#define ComboBox_SelectItemData(hwndCtl, indexStart, data)      ((int)(DWORD)SendMessage((hwndCtl), CB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ComboBox_Dir(hwndCtl, attrs, lpszFileSpec)  ((int)(DWORD)SendMessage((hwndCtl), CB_DIR, (WPARAM)(UINT)(attrs), (LPARAM)(LPCTSTR)(lpszFileSpec)))

#define ComboBox_ShowDropdown(hwndCtl, fShow)       ((BOOL)(DWORD)SendMessage((hwndCtl), CB_SHOWDROPDOWN, (WPARAM)(BOOL)(fShow), 0L))

#if (WINVER >= 0x030a)
#define ComboBox_FindStringExact(hwndCtl, indexStart, lpszFind)  ((int)(DWORD)SendMessage((hwndCtl), CB_FINDSTRINGEXACT, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind)))

#define ComboBox_GetDroppedState(hwndCtl)           ((BOOL)(DWORD)SendMessage((hwndCtl), CB_GETDROPPEDSTATE, 0L, 0L))
#define ComboBox_GetDroppedControlRect(hwndCtl, lprc) ((void)SendMessage((hwndCtl), CB_GETDROPPEDCONTROLRECT, 0L, (LPARAM)(RECT *)(lprc)))

#define ComboBox_GetItemHeight(hwndCtl)             ((int)(DWORD)SendMessage((hwndCtl), CB_GETITEMHEIGHT, 0L, 0L))
#define ComboBox_SetItemHeight(hwndCtl, index, cyItem) ((int)(DWORD)SendMessage((hwndCtl), CB_SETITEMHEIGHT, (WPARAM)(int)(index), (LPARAM)(int)cyItem))

#define ComboBox_GetExtendedUI(hwndCtl)             ((UINT)(DWORD)SendMessage((hwndCtl), CB_GETEXTENDEDUI, 0L, 0L))
#define ComboBox_SetExtendedUI(hwndCtl, flags)      ((int)(DWORD)SendMessage((hwndCtl), CB_SETEXTENDEDUI, (WPARAM)(UINT)(flags), 0L))
#endif  /* WINVER >= 0x030a */

/****** Alternate porting layer macros ****************************************/

/* USER MESSAGES: */

#define GET_WPARAM(wp, lp)                      (wp)
#define GET_LPARAM(wp, lp)                      (lp)

#define GET_WM_ACTIVATE_STATE(wp, lp)           LOWORD(wp)
#define GET_WM_ACTIVATE_FMINIMIZED(wp, lp)      (BOOL)HIWORD(wp)
#define GET_WM_ACTIVATE_HWND(wp, lp)            (HWND)(lp)
#define GET_WM_ACTIVATE_MPS(s, fmin, hwnd)   \
        (WPARAM)MAKELONG((s), (fmin)), (LONG)(hwnd)

#define GET_WM_CHARTOITEM_CHAR(wp, lp)          (TCHAR)LOWORD(wp)
#define GET_WM_CHARTOITEM_POS(wp, lp)           HIWORD(wp)
#define GET_WM_CHARTOITEM_HWND(wp, lp)          (HWND)(lp)
#define GET_WM_CHARTOITEM_MPS(ch, pos, hwnd) \
        (WPARAM)MAKELONG((pos), (ch)), (LONG)(hwnd)

#define GET_WM_COMMAND_ID(wp, lp)               LOWORD(wp)
#define GET_WM_COMMAND_HWND(wp, lp)             (HWND)(lp)
#define GET_WM_COMMAND_CMD(wp, lp)              HIWORD(wp)
#define GET_WM_COMMAND_MPS(id, hwnd, cmd)    \
        (WPARAM)MAKELONG(id, cmd), (LONG)(hwnd)

#define WM_CTLCOLOR                             0x0019

#define GET_WM_CTLCOLOR_HDC(wp, lp, msg)        (HDC)(wp)
#define GET_WM_CTLCOLOR_HWND(wp, lp, msg)       (HWND)(lp)
#define GET_WM_CTLCOLOR_TYPE(wp, lp, msg)       (WORD)(msg - WM_CTLCOLORMSGBOX)
#define GET_WM_CTLCOLOR_MSG(type)               (WORD)(WM_CTLCOLORMSGBOX+(type))
#define GET_WM_CTLCOLOR_MPS(hdc, hwnd, type) \
        (WPARAM)(hdc), (LONG)(hwnd)


#define GET_WM_MENUSELECT_CMD(wp, lp)               LOWORD(wp)
#define GET_WM_MENUSELECT_FLAGS(wp, lp)             (UINT)(int)(short)HIWORD(wp)
#define GET_WM_MENUSELECT_HMENU(wp, lp)             (HMENU)(lp)
#define GET_WM_MENUSELECT_MPS(cmd, f, hmenu)  \
        (WPARAM)MAKELONG(cmd, f), (LONG)(hmenu)

/* Note: the following are for interpreting MDIclient to MDI child messages. */
#define GET_WM_MDIACTIVATE_FACTIVATE(hwnd, wp, lp)  (lp == (LONG)hwnd)
#define GET_WM_MDIACTIVATE_HWNDDEACT(wp, lp)        (HWND)(wp)
#define GET_WM_MDIACTIVATE_HWNDACTIVATE(wp, lp)     (HWND)(lp)
/* Note: the following is for sending to the MDI client window. */
#define GET_WM_MDIACTIVATE_MPS(f, hwndD, hwndA)\
        (WPARAM)(hwndA), 0

#define GET_WM_MDISETMENU_MPS(hmenuF, hmenuW) (WPARAM)hmenuF, (LONG)hmenuW

#define GET_WM_MENUCHAR_CHAR(wp, lp)                (TCHAR)LOWORD(wp)
#define GET_WM_MENUCHAR_HMENU(wp, lp)               (HMENU)(lp)
#define GET_WM_MENUCHAR_FMENU(wp, lp)               (BOOL)HIWORD(wp)
#define GET_WM_MENUCHAR_MPS(ch, hmenu, f)    \
        (WPARAM)MAKELONG(ch, f), (LONG)(hmenu)

#define GET_WM_PARENTNOTIFY_MSG(wp, lp)             LOWORD(wp)
#define GET_WM_PARENTNOTIFY_ID(wp, lp)              HIWORD(wp)
#define GET_WM_PARENTNOTIFY_HWNDCHILD(wp, lp)       (HWND)(lp)
#define GET_WM_PARENTNOTIFY_X(wp, lp)               (int)(short)LOWORD(lp)
#define GET_WM_PARENTNOTIFY_Y(wp, lp)               (int)(short)HIWORD(lp)
#define GET_WM_PARENTNOTIFY_MPS(msg, id, hwnd) \
        (WPARAM)MAKELONG(id, msg), (LONG)(hwnd)
#define GET_WM_PARENTNOTIFY2_MPS(msg, x, y) \
        (WPARAM)MAKELONG(0, msg), MAKELONG(x, y)

#define GET_WM_VKEYTOITEM_CODE(wp, lp)              (int)(short)LOWORD(wp)
#define GET_WM_VKEYTOITEM_ITEM(wp, lp)              HIWORD(wp)
#define GET_WM_VKEYTOITEM_HWND(wp, lp)              (HWND)(lp)
#define GET_WM_VKEYTOITEM_MPS(code, item, hwnd) \
        (WPARAM)MAKELONG(item, code), (LONG)(hwnd)

#define GET_EM_SETSEL_START(wp, lp)                 (INT)(wp)
#define GET_EM_SETSEL_END(wp, lp)                   (lp)
#define GET_EM_SETSEL_MPS(iStart, iEnd) \
        (WPARAM)(iStart), (LONG)(iEnd)

#define GET_EM_LINESCROLL_MPS(vert, horz)     \
        (WPARAM)horz, (LONG)vert

#define GET_WM_CHANGECBCHAIN_HWNDNEXT(wp, lp)       (HWND)(lp)

#define GET_WM_HSCROLL_CODE(wp, lp)                 LOWORD(wp)
#define GET_WM_HSCROLL_POS(wp, lp)                  HIWORD(wp)
#define GET_WM_HSCROLL_HWND(wp, lp)                 (HWND)(lp)
#define GET_WM_HSCROLL_MPS(code, pos, hwnd)    \
        (WPARAM)MAKELONG(code, pos), (LONG)(hwnd)

#define GET_WM_VSCROLL_CODE(wp, lp)                 LOWORD(wp)
#define GET_WM_VSCROLL_POS(wp, lp)                  HIWORD(wp)
#define GET_WM_VSCROLL_HWND(wp, lp)                 (HWND)(lp)
#define GET_WM_VSCROLL_MPS(code, pos, hwnd)    \
        (WPARAM)MAKELONG(code, pos), (LONG)(hwnd)

/****** C runtime porting macros ****************************************/

#if !defined(__BORLANDC__)

#define _ncalloc    calloc
#define _nexpand    _expand
#define _ffree      free
#define _fmalloc    malloc
#define _fmemccpy   _memccpy
#define _fmemchr    memchr
#define _fmemcmp    memcmp
#define _fmemcpy    memcpy
#define _fmemicmp   _memicmp
#define _fmemmove   memmove
#define _fmemset    memset
#define _fmsize     _msize
#define _frealloc   realloc
#define _fstrcat    strcat
#define _fstrchr    strchr
#define _fstrcmp    strcmp
#define _fstrcpy    strcpy
#define _fstrcspn   strcspn
#define _fstrdup    _strdup
#define _fstricmp   _stricmp
#define _fstrlen    strlen
#define _fstrlwr    _strlwr
#define _fstrncat   strncat
#define _fstrncmp   strncmp
#define _fstrncpy   strncpy
#define _fstrnicmp  _strnicmp
#define _fstrnset   _strnset
#define _fstrpbrk   strpbrk
#define _fstrrchr   strrchr
#define _fstrrev    _strrev
#define _fstrset    _strset
#define _fstrspn    strspn
#define _fstrstr    strstr
#define _fstrtok    strtok
#define _fstrupr    _strupr
#define _nfree      free
#define _nmalloc    malloc
#define _nmsize     _msize
#define _nrealloc   realloc
#define _nstrdup    _strdup
#define hmemcpy     MoveMemory

#endif /* !__BORLANDC__ */

#define DECLARE_HANDLE32    DECLARE_HANDLE

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif       /* __cplusplus */

#endif  /* !_INC_WINDOWSX */



