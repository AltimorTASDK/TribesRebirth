//------------------------------------------------------------------------------
// Description: Console plugin to test dynamic lights...
//    
// $Workfile: $
// $Revision: $
// $Author:   $
// $Modtime:  $
//
//------------------------------------------------------------------------------

#ifndef _TESTDLIGHTPLUGIN_H_
#define _TESTDLIGHTPLUGIN_H_

//Includes
#include "simConsoleCanvas.h"
#include "simDynamicLight.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class TestDLightPlugin : public SimConsolePlugin
{
	enum CallbackID {
      NewLinearLight,
      LinearLightFromObject
   };

public:
	void COREAPI init();
	int  COREAPI consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_TESTDLIGHTPLUGIN_H_
