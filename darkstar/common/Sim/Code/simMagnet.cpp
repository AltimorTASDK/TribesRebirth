//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simMagnet.h"
#include "commonEditor.strings.h"
#include <darkstar.strings.h>
#include <simMovement.h>

//-------------------------------------------------------------------------- 

SimMagnet::SimMagnet()
{
   force = 1;
}

//------------------------------------------------------------------------------

bool SimMagnet::processEvent(const SimEvent *event)
{
	switch (event->type) 
	{
      onEvent(SimTriggerEvent);
   }   
   return Parent::processEvent(event);
}  

//------------------------------------------------------------------------------

void SimMagnet::inspectRead(Inspect* insp)
{
	Parent::inspectRead(insp);
   
   insp->read(IDCOMEDIT_ITG_FORCE, force);
}   

//------------------------------------------------------------------------------

void SimMagnet::inspectWrite(Inspect* insp)
{
	Parent::inspectWrite(insp);

   insp->write(IDCOMEDIT_ITG_FORCE, force);
}


//------------------------------------------------------------------------------

bool SimMagnet::onSimTriggerEvent(const SimTriggerEvent *event)
{
//   switch (event->action)
//   {
//      case IDSIMOBJ_EKEY_ACTIVATE:                                                
//      {
// this needs to be ported to the new trigger stuff may 19, 1998
//         SimMovement *sucker = (SimMovement *)manager->findObject(event->activator);
//         AssertFatal(sucker, "SimMagnet::onSimTriggerEvent: no activator");
//      
//         // launch the sucker
//         Vector3F propulsion;
//         
//         propulsion = sucker->getTransform().p;
//         propulsion -= getTransform().p;
//         propulsion.normalize();
//         propulsion *= force;
//         
//         sucker->applyImpulse(propulsion);
//         
//         return true;
//      }
//   }
   
   return false;
}

//------------------------------------------------------------------------------
Persistent::Base::Error SimMagnet::read(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = Parent::read(sio, a, b);
   if (error != Persistent::Base::Ok) 
      return error;

   sio.read(&force);

   return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}

Persistent::Base::Error SimMagnet::write(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = Parent::write(sio, a, b);
   if (error != Persistent::Base::Ok) 
      return error;

   sio.write(force);

   return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}
