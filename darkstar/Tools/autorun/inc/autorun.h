//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _AUTOSTART_H_
#define _AUTOSTART_H_

#include <core.h>
#include <gw.h>
#include <tagDictionary.h>
#include <console.h>
#include <persist.h>
#include <registry.h>


//------------------------------------------------------------------------------
void warning(const char *msg, ...);

//------------------------------------------------------------------------------
class ConOut : public CMDConsumer
{
   virtual void consoleCallback(CMDConsole *, const char *consoleLine);
};

void ConOut::consoleCallback(CMDConsole *, const char *consoleLine)
{
   consoleLine;
#ifdef DEBUG
   warning("%s", consoleLine);
#endif
}



//----------------------------------------------------------------------------
//------------------------------------------------------------------------------
class ASConsole: public CMDConsole
{
private:
   typedef CMDConsole Parent;
   enum {
      NewTitleWindow = 10000,         
      NewButton,
      NewBitmap,
      ShowWindow,
      CenterWindow,         
      SetPosition,
      SetIcon,
      Explore,
      Open,
      IsRunning,
      Quit,
      RegistryOpen,
      RegistryClose,
      RegistryRead,
      RegistryWrite,
   };

   Registry reg;
   ResourceManager rm;
   ConOut co;

   virtual const char *consoleCallback(CMDConsole*,int id,int argc, const char *argv[]);
   const char *windowHandle(GWWindow *win);

public:
   ASConsole();
   ~ASConsole();
   void init();
};

#endif //_AUTOSTART_H_
