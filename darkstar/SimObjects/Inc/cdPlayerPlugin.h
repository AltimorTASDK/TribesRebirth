//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _CDPLAYERPLUGIN_H_
#define _CDPLAYERPLUGIN_H_

#include "cdPlayerDlg.h"

class CDPlayerPlugin: public SimConsolePlugin
{
	enum CallbackID
	{
		CDPlayer,            
   };

public:
	void init();
   void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};

#endif //_CDPLAYERPLUGIN_H_
