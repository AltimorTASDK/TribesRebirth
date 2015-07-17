//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SimInputPLUGIN_H_
#define _SimInputPLUGIN_H_

class SimInputManager;
class SimActionMap;
class SimActionEvent;
class SimInputEvent;

class SimInputPlugin: public SimConsolePlugin
{
   SimInputManager* getInput(bool msg=true);
   bool parseSimInputEvent(int arvc, const char *argv[], SimInputEvent *event);
   bool parseSimActionEvent(int arvc, const char *argv[], SimActionEvent *action, BitSet32 *flags);
   char *buildConsoleCommand(int argc, const char **argv);

public:
	void init();
   void startFrame();
   void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);

   static bool writeActionMap(StreamIO &s, SimActionMap *map);
};


#endif //_SimInputPLUGIN_H_
