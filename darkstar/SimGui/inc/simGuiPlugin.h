//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUIPLUGIN_H_
#define _SIMGUIPLUGIN_H_


//------------------------------------------------------------------------------
namespace SimGui
{

class Plugin : public SimConsolePlugin
{
public:
	void init();
	void startFrame();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};

};
#endif //_SINGUIPLUGIN_H_
