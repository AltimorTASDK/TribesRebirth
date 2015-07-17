//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _SKYPLUGIN_H_
#define _SKYPLUGIN_H_

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class SimSkyPlanet;

class SkyPlugin: public SimConsolePlugin
{
   SimSkyPlanet *lastSkyPlanet;

public:
	void init();
	void startFrame();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};


#endif
