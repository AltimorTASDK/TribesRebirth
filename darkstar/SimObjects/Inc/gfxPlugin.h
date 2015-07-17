//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _GFXPLUGIN_H_
#define _GFXPLUGIN_H_

// Must include after sim stuff

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class GFXPlugin: public SimConsolePlugin
{
   bool m_fullyInitialized;

public:
	void init();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};


#endif
