//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SHELLOPENPLUGIN_H_
#define _SHELLOPENPLUGIN_H_

//Includes
#include "simConsolePlugin.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class ShellOpenPlugin : public SimConsolePlugin
{
	enum CallbackID
	{
      ShellOpen,
      ShellOpenAndGoWin,
      HTMLOpen,
      HTMLOpenAndGoWin,
      Notepad
   };

public:
	void init();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SHELLOPENPLUGIN_H_
