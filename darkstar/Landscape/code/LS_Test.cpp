//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   LSTest.cpp  $
//北  $Version$
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北        Program to exercise the LandScape library
//北        
//北  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#include "LSEditor.h"
#include "simterrain.h"

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

/*int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,	LPSTR lpszCmdLine, int nCmdShow )
{
   hPrevInstance;
   lpszCmdLine;
   nCmdShow;

	MSG   msg;
	// Altimor: Unused
   //bool  notify;

	GWWindow::setHInstance(hInstance);
   LandScape *pLS = new LandScape(256);
   LSEditor *pLSE = new LSEditor();
   pLSE->create( NULL, NULL, pLS, true );

	while(1)
   {
      GetMessage(&msg, NULL, 0, 0);
      if( msg.message == WM_QUIT ) return (0);

      // Dialog boxes need to have their messages translated differently.
      GWWindow* win = GWMap::getWindow(msg.hwnd);
      if (win && dynamic_cast<GWDialog*>(win))
         if (IsDialogMessage(msg.hwnd,&msg))
            continue;

      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}*/

