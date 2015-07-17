//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _INTERIORPLUGIN_H_
#define _INTERIORPLUGIN_H_

// Must include after sim stuff

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class InteriorPlugin: public SimConsolePlugin
{
	enum CallbackID {
      SetInteriorShapeState,
      SetInteriorLightState,
      
      AnimateLight,
      StopLightAnim,
      ResetLight,
      
      ToggleBoundingBox
	};

public:
	void init();
	void startFrame();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};


#endif
