//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GWLOG_H_
#define _GWLOG_H_

//----------------------------------------------------------------------------
#include "gwBase.h"


class GWLog: public GWWindow
{
	typedef GWWindow Parent;
   HWND hLogWnd;
public:
	GWLog(LPCSTR name);
   GWLog(HWND _hWnd, LPRECT _r);
	~GWLog();

   // override from gwBase
	void destroyWindow();

   // onWindows Members
	void onSize(UINT state, int cx, int cy);
	void onSysCommand(UINT cmd, int x, int y);

   //gwLog Members
   int   getCurSel();
   void  clear();
   void  __cdecl printf(char *_format, ...);
};


#endif //_GWLOG_H_
