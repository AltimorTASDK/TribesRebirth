#ifndef _H_SIMGUITIMERCTRL
#define _H_SIMGUITIMERCTRL

#include "simGuiCtrl.h"
#include "inspect.h"

namespace SimGui
{

class TimerCtrl : public Control
{
private:
   typedef Control Parent;

   Int32 message;
   float initialAlarm;
   float repeatAlarm;
   enum { TimeOut = 1000 };
public:   
   TimerCtrl();
   
   bool processEvent(const SimEvent *event);

   void inspectRead(Inspect *);
   void inspectWrite(Inspect *);
   
   void onWake();

   DECLARE_PERSISTENT(TimerCtrl);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

};

#endif