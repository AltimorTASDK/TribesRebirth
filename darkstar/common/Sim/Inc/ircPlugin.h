//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _IRCPLUGIN_H_
#define _IRCPLUGIN_H_


#include <simConsolePlugin.h>

class IRCPlugin : public SimConsolePlugin
{
  
public:
   void init();
   const char* consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};



#endif //_IRCPLUGIN_H_
