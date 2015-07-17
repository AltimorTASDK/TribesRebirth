//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PREFERENCEPLUGIN_H_
#define _PREFERENCEPLUGIN_H_


class PreferencePlugin : public SimConsolePlugin
{
	enum CallbackID {
      NewPrefObject,
      Notify,
      Close,
      Save,
      Restore,
      SetInt,
      SetFloat,
      SetString,
   };

public:
	void init();
	void startFrame();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};


#endif //_PREFERENCEPLUGIN_H_
