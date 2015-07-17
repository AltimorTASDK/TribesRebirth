//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include "gwLog.h"


GWLog::GWLog(LPCSTR name)
{
   if (Parent::create(0, name,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		Point2I(CW_USEDEFAULT,0),
		Point2I(380,150),
		NULL,0)) 
   {
      RECT rc;
	   show( SW_SHOW );
	   update();
      GetClientRect(getHandle(), &rc);

      hLogWnd = CreateWindowEx( 0L, "LISTBOX", "",
         LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | WS_VISIBLE | 
         WS_CHILD | WS_BORDER | WS_VSCROLL,
         rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
         getHandle(), NULL, getHInstance(), NULL );

      MENUITEMINFO mi;
      mi.cbSize   = sizeof(MENUITEMINFO);  
      mi.fMask    = MIIM_TYPE; 
      mi.fType    = MFT_SEPARATOR; 
      mi.fState   = MFS_DEFAULT; 
      mi.wID      = 0; 
      mi.hSubMenu = NULL; 
      mi.hbmpChecked    = NULL; 
      mi.hbmpUnchecked  = NULL; 
      mi.dwItemData     = NULL; 
      mi.dwTypeData     = NULL; 
      mi.cch            = 0; 

      HMENU hSysMenu = GetSystemMenu(hWnd, false);
      InsertMenuItem( hSysMenu, 0, true, &mi); 

      mi.fMask    = MIIM_TYPE | MIIM_ID; 
      mi.fType    = MFT_STRING; 
      mi.wID      = WM_USER; 
      mi.dwTypeData     = "Clear Log"; 
      mi.cch            = strlen(mi.dwTypeData); 
      InsertMenuItem( hSysMenu, 0, true, &mi); 
	}
}

GWLog::GWLog(HWND _hWnd, LPRECT _r)
{
   hLogWnd = CreateWindowEx( 0L, "LISTBOX", "",
      LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | WS_VISIBLE | 
      WS_CHILD | WS_BORDER | WS_VSCROLL,
      _r->left, _r->top, _r->right - _r->left, _r->bottom - _r->top,
      _hWnd, NULL, getHInstance(), NULL );
}

GWLog::~GWLog()
{
}


void GWLog::destroyWindow()
{
   DestroyWindow(hLogWnd);
	Parent::destroyWindow();
}

void GWLog::onSize(UINT state, int cx, int cy)
{
   RECT rc;
   Parent::onSize(state, cx, cy);   
   GetClientRect(getHandle(), &rc);
   MoveWindow( hLogWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, true);
}


void GWLog::onSysCommand(UINT cmd, int x, int y)
{
   if ( cmd == WM_USER ) clear(); 
   else Parent::onSysCommand(cmd, x, y);
}

int GWLog::getCurSel()
{
   return (SendMessage( hLogWnd, LB_GETCURSEL, NULL, NULL ));
}

void GWLog::clear()
{
   SendMessage( hLogWnd, LB_RESETCONTENT, 0, 0 );
}


void __cdecl GWLog::printf(char *_format, ...)
{
   char buff[512];
   va_list va;
   va_start(va, _format);
   vsprintf( buff, _format, (LPSTR)va );
   va_end(va);
   SendMessage( hLogWnd, LB_SETCURSEL, 
      SendMessage( hLogWnd, LB_ADDSTRING, NULL, (LPARAM)buff ),
      NULL);
}
