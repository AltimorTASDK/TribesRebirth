//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _NETPLUGIN_H_
#define _NETPLUGIN_H_

#include "simworld.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class NetPlugin: public SimConsolePlugin
{
public:
   NetPlugin() { }
	void init();
	void startFrame();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};


#endif
