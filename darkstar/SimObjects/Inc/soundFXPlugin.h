//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SFXPLUGIN_H_
#define _SFXPLUGIN_H_


// Must include after sim stuff


class SoundFXPlugin : public SimConsolePlugin
{
public:
	void init();
	void startFrame();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};


#endif //_SFXPLUGIN_H_
