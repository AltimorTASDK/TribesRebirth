//----------------------------------------------------------------------------

//	$Workfile:   gwdialog.h  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:02  $

//----------------------------------------------------------------------------

#ifndef _GWDIALOG_H_
#define _GWDIALOG_H_

#include <gwBase.h>

//----------------------------------------------------------------------------

class GWDialog: public GWWindow
{
	typedef GWWindow Parent;
public:
	bool create(LPCSTR name,GWWindow *parent, bool modal=false);
	bool create(int id,GWWindow *parent, bool modal=false);
	bool create(int id, HWND parent, bool modal=false);
	bool createParam(LPCSTR name,GWWindow *parent, LPARAM lParam, bool modal=false );
	bool createParam(int id,GWWindow *parent, LPARAM lParam, bool modal=false );
	bool createParam(int id, HWND parent, LPARAM lParam, bool modal=false );

   LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);

   DECLARE_MESSAGE_MAP()
};



#endif


