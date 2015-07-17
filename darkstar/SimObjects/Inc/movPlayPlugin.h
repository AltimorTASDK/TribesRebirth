//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _MOVPLAYPLUGIN_H_
#define _MOVPLAYPLUGIN_H_

#include <SimConsolePlugin.h>

class MovPlayPlugin : public SimConsolePlugin
{
	enum CallbackID {
      NewMovPlay,
      OpenMovie,
      CloseMovie,
      PlayMovie,
      PlayMovieToComp,
      StopMovie,
      PauseMovie,
   };

public:
	void init();
	void startFrame();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};

#endif //_MOVPLAYPLUGIN_H_
