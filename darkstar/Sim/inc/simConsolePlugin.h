//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _SIMCONSOLEPLUGIN_H_
#define _SIMCONSOLEPLUGIN_H_

//----------------------------------------------------------------------------

#include "console.h"

class SimManager;

class SimConsolePlugin: public CMDCallback
{
protected:
	// This would normally be the client manager.
	SimManager* manager;
	CMDConsole* console;

public:
	SimConsolePlugin();
	virtual ~SimConsolePlugin();
	void init(CMDConsole* con,SimManager* mgr);
   virtual void setManager(SimManager* mgr);

	// Methods added by new plugins
	virtual const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
	virtual void init();
	virtual void startFrame();
	virtual void endFrame();
};

#endif

