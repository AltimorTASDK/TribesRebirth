//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <sim.h>
#include <fn_table.h>

#include "gwMain.h"
#include "simConsolePlugin.h"
#include "simResource.h"
#include "simTSViewport.h"
#include "genericActions.h"



//----------------------------------------------------------------------------

SimConsolePlugin::SimConsolePlugin()
{
	console = 0;
	manager = 0;
}

SimConsolePlugin::~SimConsolePlugin()
{
}


//----------------------------------------------------------------------------

void SimConsolePlugin::init(CMDConsole* con,SimManager* mgr)
{
	console = con;
	manager = mgr;
	// Call derived init method
	init();
}

void SimConsolePlugin::init() {}
void SimConsolePlugin::startFrame() {}
void SimConsolePlugin::endFrame() {}
const char *SimConsolePlugin::consoleCallback(CMDConsole*,int,int,const char *[])
{
	return NULL;
}


void SimConsolePlugin::setManager(SimManager* mgr)
{
   manager = mgr;
}
