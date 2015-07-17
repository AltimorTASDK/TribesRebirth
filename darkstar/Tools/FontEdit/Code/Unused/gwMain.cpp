//--------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//--------------------------------------------------------------------------------


#include "gwMain.h"
#include "gwBase.h"
#include "gwDialog.h"

#define PEEK_AT_MESSAGES false


GWMain* GWMain::pMain;
HINSTANCE GWMain::hInstance;
HINSTANCE GWMain::hPrevInstance;
int GWMain::cmdShow;
LPSTR GWMain::cmdLine;
int GWMain::exitCode;
MSG GWMain::currentMsg;


//----------------------------------------------------------------------------

GWMain::GWMain()
{
	AssertFatal(pMain == 0,
		"GWMain::GWMain: Application main instantiated twice");
	pMain = this;
}

GWMain::~GWMain()
{
	pMain = 0;
}


//----------------------------------------------------------------------------

int GWMain::messageLoop()
{
	while (processMessage())
		;
	return exitCode;
}

bool GWMain::processMessage()
{
	while(1)
	{
#if PEEK_AT_MESSAGES
	    if (PeekMessage(&currentMsg, NULL, 0, 0, PM_NOREMOVE)) {
#endif
			// Get pending message
	    	if (!GetMessage(&currentMsg, NULL, 0, 0)) {
				exitCode = currentMsg.wParam;
				return false;
			}
			// Dialog boxes need to have their messages
			// translated differently.
			GWWindow* win = GWMap::getWindow(currentMsg.hwnd);
			if (win && dynamic_cast<GWDialog*>(win))
				if (IsDialogMessage(currentMsg.hwnd,&currentMsg))
					return true;
			// Normal translate/dispatch
	        TranslateMessage(&currentMsg);
	        DispatchMessage(&currentMsg);
			return true;

#if PEEK_AT_MESSAGES
	    }
		else
			onIdle();
#endif
	}
}

void GWMain::onIdle()
{
}


//----------------------------------------------------------------------------

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow )
{
	GWWindow::setHInstance(hInstance);
	GWMain::hInstance = hInstance;
	GWMain::hPrevInstance = hPrevInstance;
	GWMain::cmdShow = nCmdShow;
	GWMain::cmdLine = lpszCmdLine;

	if (!GWMain::hPrevInstance)
		GWMain::pMain->initApplication();
	GWMain::pMain->initInstance();
	return GWMain::pMain->messageLoop();
}


//----------------------------------------------------------------------------

bool GWMain::initApplication()
{
	return true;
}

bool GWMain::initInstance()
{
	return true;
}

