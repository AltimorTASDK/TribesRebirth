//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simSoundSource.h"
#include "commonEditor.strings.h"
#include <simSoundSequence.h>
#include <darkstar.strings.h>

//-------------------------------------------------------------------------- 

SimSoundSource::SimSoundSource()
{
   channel = -1;
   soundId = 0;
}

//------------------------------------------------------------------------------

bool SimSoundSource::processEvent(const SimEvent *event)
{
	switch (event->type) 
	{
      onEvent(SimTriggerEvent);
   }   
   return Parent::processEvent(event);
}  

//------------------------------------------------------------------------------

void SimSoundSource::inspectRead(Inspect* insp)
{
	Parent::inspectRead(insp);
   
   insp->read(IDCOMEDIT_ITG_SOUND, (Int32 &)soundId);
   insp->read(IDCOMEDIT_ITG_CHANNEL, channel);
   
   if(channel < -1)
      channel = -1;
   if(channel >= SimSoundSequence::maxChannel)
      channel = SimSoundSequence::maxChannel - 1;
}   

//------------------------------------------------------------------------------

void SimSoundSource::inspectWrite(Inspect* insp)
{
	Parent::inspectWrite(insp);

   insp->write(IDCOMEDIT_ITG_SOUND, (Int32)soundId);
   insp->write(IDCOMEDIT_ITG_CHANNEL, channel);
}


//------------------------------------------------------------------------------

bool SimSoundSource::onSimTriggerEvent(const SimTriggerEvent *event)
{
   switch (event->action)
   {
      case SimTriggerEvent::Activate:                                                
      {
         SimSoundSequence::BroadcastQueueAt(manager, channel, soundId, getTransform());
         return true;
      }
   }
   
   return false;
}

//------------------------------------------------------------------------------
Persistent::Base::Error SimSoundSource::read(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = Parent::read(sio, a, b);
   if (error != Persistent::Base::Ok) 
      return error;

   sio.read((Int32 *)&soundId);
   sio.read(&channel);

   return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}

Persistent::Base::Error SimSoundSource::write(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = Parent::write(sio, a, b);
   if (error != Persistent::Base::Ok) 
      return error;

   sio.write((Int32)soundId);     
   sio.write(channel);

   return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}
