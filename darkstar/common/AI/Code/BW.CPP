//------------------------------------------------------------------------------
//
// Description:   Editor for Bayesian Networks
//    
// $Workfile$     BayWatch.cpp
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#include <BayWatch.h>
#include <gw.h>
#include <simVolume.h>
#include <simWorld.h>

//----------------------------------------------------------------------------

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,	LPSTR lpszCmdLine, int nCmdShow )
{
   hPrevInstance;
   lpszCmdLine;
   nCmdShow;

	MSG   msg;

	GWWindow::setHInstance(hInstance);
	
   SimWorld *client = new SimWorld;
   SimManager *manager = client->getManager();

	SimVolume *vol = new SimVolume();
   manager->addObject(vol);
   vol->open( "Editor.vol" );

   BayWatch *pBW = new BayWatch();
   manager->addObject(pBW);
   pBW->create( NULL, true );
   pBW->setBayesNet( new BayesNet() );
	
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
}
