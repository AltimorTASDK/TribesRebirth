//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _REDBOOKPLUGIN_H_
#define _REDBOOKPLUGIN_H_

class RedbookPlugin : public SimConsolePlugin
{
private:
   enum CallbackID {
      NewRedbook,
      
      Open,
      Close,
      Eject,
      Retract,
      
      GetStatus,
      GetTrackCount,
      GetTrackInfo,
      GetTrackPosition,
      
      Play,
      Stop,
      Pause,
      Resume,
      
      SetVolume,
      GetVolume,

		SetPlayMode,
   };

public:
   void init();
   void startFrame();
   void endFrame();
   const char *consoleCallback(CMDConsole*,int id, int argc, const char *argv[]);
};

#endif //_REDBOOKPLUGIN_H_
