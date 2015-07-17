#ifndef _H_SIMWINCONSOLEPLUGIN
#define _H_SIMWINCONSOLEPLUGIN

#define MAX_CMDS 10

class SimWinConsolePlugin: public SimConsolePlugin, public CMDConsumer
{
   bool winConsoleEnabled;

   HANDLE stdOut;
   HANDLE stdIn;
   HANDLE stdErr;
   char inbuf[512];
   int  inpos;
   bool lineOutput;
   char curTabComplete[512];
   int  tabCompleteStart;
   char rgCmds[MAX_CMDS][512];
   int  iCmdIndex;

   void printf(const char *s, ...);

public:
   SimWinConsolePlugin();
	void init();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
   void consoleCallback(CMDConsole *, const char *consoleLine);
};

#endif
