#ifndef _H_SIMTELNETPLUGIN
#define _H_SIMTELNETPLUGIN

#include "winsock.h"

class SimTelnetPlugin: public SimConsolePlugin, public CMDConsumer
{
   enum State
   {
      EnteringPassword,
      Connected,
   };

   struct TelnetClient
   {
      SOCKET fd;
      char curLine[CMDConsole::MaxLineLength];
      int curPos;
      int state;
      int passcount;
   };
   int maxConnections;
   SOCKET acceptSocket;
   int acceptPort;
   Vector<TelnetClient> connectionList;
public:
   SimTelnetPlugin();
   ~SimTelnetPlugin();
	void init();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
   void consoleCallback(CMDConsole *, const char *consoleLine);
};

#endif

