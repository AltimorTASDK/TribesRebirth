//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ESFPLUGIN_H_
#define _ESFPLUGIN_H_


class ESFPlugin : public SimConsolePlugin
{
public:
   void init();
   const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};


#endif //_ESFPLUGIN_H_
