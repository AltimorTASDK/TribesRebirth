//----------------------------------------------------------------------------

//	$Workfile:   gwdialog.cpp  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:02  $

//----------------------------------------------------------------------------

#include "gwbase.h"
#include "gwdialog.h"

//----------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam,
                                 LPARAM lParam)
{
	// Under 3.1 ?  These messages are passed directly to this
	// function, and not sent through SendMessage.
	if (message != WM_SETFONT && message != WM_INITDIALOG)
		// Pass to normal handler
		return FALSE;

	// Deliver the messages by hand.
	GWWindow* window = GWMap::getWindow(hDlg);
	if (window)
		window->windowProc(message,wParam,lParam);
	return FALSE;
}


//----------------------------------------------------------------------------

bool GWDialog::create(LPCSTR name,GWWindow* parent)
{
	hookCreate(this);
	hWnd = CreateDialog(hInstance,name,
		parent->getHandle(),DlgProc);
	unhookCreate();
	return hWnd != NULL;
}

bool GWDialog::create(int id,GWWindow* parent)
{
	hookCreate(this);
	hWnd = CreateDialog(hInstance,
		MAKEINTRESOURCE(id),parent? parent->getHandle(): 0,DlgProc);
	unhookCreate();
	return hWnd != NULL;
}

LRESULT GWDialog::windowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
		case WM_SETFONT:
		case WM_INITDIALOG:
			// Should not pass these on the default handler
			// since they come from dlgproc.
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					destroyWindow();
					break;
				default:
					return Parent::windowProc(message, wParam, lParam);
			}
			break;
		default:
			return Parent::windowProc(message, wParam, lParam);
	}
	return 0;
}

