//----------------------------------------------------------------------------

//	$Workfile:   gwdialog.h  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:02  $

//----------------------------------------------------------------------------

#ifndef _GWDIALOG_H_
#define _GWDIALOG_H_


//----------------------------------------------------------------------------

class GWDialog: public GWWindow
{
	typedef GWWindow Parent;
public:
	bool create(LPCSTR name,GWWindow *parent);
	bool create(int id,GWWindow *parent);
    LRESULT windowProc(UINT message, WPARAM wParam, LPARAM lParam);
};



#endif


