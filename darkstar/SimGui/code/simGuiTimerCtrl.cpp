#include "simGuiTimerCtrl.h"
#include "inspectWindow.h"
#include "editor.strings.h"
#include "console.h"

namespace SimGui
{

TimerCtrl::TimerCtrl()
{
   message = 0;
   initialAlarm = 0;
   repeatAlarm = 0;
}

void TimerCtrl::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);

   insp->read(IDITG_MESSAGE_TAG, message);
   insp->read(IDITG_INITIAL_ALARM, initialAlarm);
   insp->read(IDITG_REPEAT_ALARM, repeatAlarm);
}   

void TimerCtrl::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);

   insp->write(IDITG_MESSAGE_TAG, message);
   insp->write(IDITG_INITIAL_ALARM, initialAlarm);
   insp->write(IDITG_REPEAT_ALARM, repeatAlarm);
}   

bool TimerCtrl::processEvent(const SimEvent *event)   
{ 
   switch (event->type)
   {
      case SimMessageEventType:
      {
         if( ((SimMessageEvent *)(event))->message == TimeOut)   
         {
            if(repeatAlarm != 0.0f)
               SimMessageEvent::post(this, manager->getCurrentTime() + SimTime(repeatAlarm), TimeOut);
            if(consoleCommand[0])
               CMDConsole::getLocked()->evaluate(consoleCommand, false);
            onMessage(this, message);
         }
         return (true);
      }
      default:
      	return Parent::processEvent( event );
   }
}

void TimerCtrl::onWake(void)
{
   if(initialAlarm != 0.0f)
      SimMessageEvent::post(this, manager->getCurrentTime() + SimTime(initialAlarm), TimeOut);
}

Persistent::Base::Error TimerCtrl::write( StreamIO &sio, int version, int user )
{
   sio.write(message);
   sio.write(initialAlarm);
   sio.write(repeatAlarm);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error TimerCtrl::read( StreamIO &sio, int version, int user)
{
   sio.read(&message);
   sio.read(&initialAlarm);
   sio.read(&repeatAlarm);

   return Parent::read(sio, version, user);
}



};