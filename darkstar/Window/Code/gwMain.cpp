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

GWMain* GWMain::pMain;
HINSTANCE GWMain::hInstance;
HINSTANCE GWMain::hPrevInstance;
int GWMain::cmdShow;
LPSTR GWMain::cmdLine;
Vector<const char*> GWMain::processedCmdLine;
int GWMain::exitCode;
MSG GWMain::currentMsg;
DWORD GWMain::processId;
HACCEL GWMain::hAccel;
bool GWMain::foreground;

//----------------------------------------------------------------------------

GWMain::GWMain()
{
	AssertFatal(pMain == 0,
		"GWMain::GWMain: Application main instantiated twice");
   hAccel = 0;
	pMain = this;
	checkFocus = true;
   nonfocusTimeout = 100;
   foreground = false;
	processId = GetCurrentProcessId();
}

GWMain::~GWMain()
{
	pMain = 0;
}


//----------------------------------------------------------------------------

int GWMain::messageLoop()
{
	while (processMessage()) 
	{
		if (checkFocus) 
		{
			HWND window = GetForegroundWindow();
			if (window) 
			{
            DWORD wndProcessId;
            GetWindowThreadProcessId(window, &wndProcessId);
            foreground = (wndProcessId == processId);
            if (!foreground)
			   	// We're in the background, go to sleep until something 
			   	// interesting happens...
               MsgWaitForMultipleObjects(0, NULL, false, nonfocusTimeout, QS_ALLINPUT);
			}
		}
		onIdle();
	}
   onExit();
	return exitCode;
}

bool GWMain::processMessage()
{
	while (PeekMessage(&currentMsg, NULL, 0, 0, PM_REMOVE)) {
		// Check for exit.
		if (currentMsg.message == WM_QUIT) {
			exitCode = currentMsg.wParam;
       	return false;
		}
		// Dialog boxes need to have their messages
		// translated differently.
		GWWindow* win = GWMap::getWindow(currentMsg.hwnd);
		if (win && dynamic_cast<GWDialog*>(win))
			if (IsDialogMessage(currentMsg.hwnd,&currentMsg))
				return true;

      if (hAccel && TranslateAccelerator(currentMsg.hwnd,hAccel,&currentMsg))
         return true;

		// Normal translate/dispatch
		TranslateMessage(&currentMsg);
		DispatchMessage(&currentMsg);
   }
	return true;
}


//----------------------------------------------------------------------------
void GWMain::onIdle()
{
}


//----------------------------------------------------------------------------
void GWMain::onExit()
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

   // Process the command line...
   //
   GWMain::cmdLine = lpszCmdLine;
   GWMain::processedCmdLine.clear();
   const char* tokCmdLine;

   char* searchLine = new char[strlen(lpszCmdLine) + 1];
   strcpy(searchLine, lpszCmdLine);
   while ((tokCmdLine = strtok(searchLine, " \t")) != NULL) {
      // search the same line...
      searchLine = NULL;

      Int32 strSize = strlen(tokCmdLine) + 1;
      char* cmd = new char[strSize];
      strcpy(cmd, tokCmdLine);

      GWMain::processedCmdLine.push_back(cmd);
   }
   delete [] searchLine;

	if (!GWMain::hPrevInstance)
		GWMain::pMain->initApplication();
	GWMain::pMain->initInstance();
	return GWMain::pMain->messageLoop();
}

int main(int argc, const char **argv)
{
   char commandLine[512];
   int i;

   GWMain::hInstance = GetModuleHandle(argv[0]);
   GWWindow::setHInstance(GWMain::hInstance);
   GWMain::hPrevInstance = 0;
   GWMain::cmdShow = SW_SHOW;

   commandLine[0] = 0;
   for(i = 1; i < argc; i++)
   {
      strcat(commandLine, argv[i]);
      strcat(commandLine, " ");
      GWMain::processedCmdLine.push_back(argv[i]);
   }
   GWMain::cmdLine = commandLine;
   if(!GWMain::hPrevInstance)
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

