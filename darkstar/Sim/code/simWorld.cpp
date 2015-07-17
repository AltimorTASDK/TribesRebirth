//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <sim.h>
#include "ts.h"
#include "simWorld.h"
#include "simCanvas.h"
#include "simTSViewport.h"
#include "simTimerGrp.h"
#include "simRenderGrp.h"
#include "simLightgrp.h"
#include "simResource.h"
#include "simPing.h"
#include "simInput.h"
#include "simContainer.h"



// bogus default GUID
GUID SimWorld::gameGuid = { 0xaaaaaaaa, 0xe610, 0x11cf, { 0xb2, 0xa9, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };


//------------------------------------------------------------------------------
SimWorld::SimWorld()
{
   renderEnabled = true;

   //resource manager object
	manager.addObject(new SimResource );

	// Hook sim engine into event stream.
	manager.addObject(new SimInputConsumerSet );
	manager.addObject(new SimActionHandler );
	
   //add canvas/rendering/camera sets
	manager.addObject(new SimCanvasSet);

   // network objects
   manager.addObject(new SimSet(false), PacketStreamSetId);
   manager.addObject(new SimSet(false), GhostManagerSetId);
   manager.addObject(new SimSet(false), SimGhostAlwaysSetId);
	// Container system
	manager.addObject(new SimDefaultOpenContainer(SimRootContainerId));

	// Default timer
	manager.addObject(new SimTimerSet(0.03f,SimTimerSetId));

	manager.addObject(new SimCameraSet);
	manager.addObject(new SimCameraMountSet);
	manager.addObject(new SimRenderSet);
	manager.addObject(new SimLightSet);

   manager.addObject(new SimSet(false), SimFrameEndNotifySetId);
   SimSet *set = new SimSet(false);
   set->assignName("NamedGuiSet");
   manager.addObject(set, NamedGuiSetId);
   manager.addObject(new SimSet(false), TaggedGuiSetId);
}


//------------------------------------------------------------------------------
SimWorld::~SimWorld()
{
   hPalette.unlock();   
}


//------------------------------------------------------------------------------
void SimWorld::advanceToTime(double time)
{
   SimObject *delegate = manager.findObject(SimCSDelegateId);
   // wake up the delegate on every time slice
   // it hast to do its thing with the session

   if(delegate)
      SimMessageEvent::post(delegate, 0);
   // send an end of advance update to all the packet stream clients...
   SimMessageEvent::post(manager.findObject(PacketStreamSetId), float(time), 0);
	manager.advanceToTime(time);      // move time forward
}


//------------------------------------------------------------------------------
void SimWorld::setPalette(int transition)
{
	if ( (bool)hPalette )
	{
	   // Inform Canvas' of palette change
      SimCanvasSet* grp = 
         static_cast<SimCanvasSet*> (manager.findObject(SimCanvasSetId));

   	SimCanvasSet::iterator ptr;
   	for (ptr = grp->begin(); ptr != grp->end(); ptr++)
   		(static_cast<SimCanvas*>(*ptr))->setPalette( hPalette, transition );
	}
}


//------------------------------------------------------------------------------
bool SimWorld::setPalette(const char *paletteName, int transition)
{
   hPalette =  SimResource::get( &manager )->load( paletteName);
   setPalette( transition );   
   return ( (bool)hPalette );
}


//------------------------------------------------------------------------------
bool SimWorld::setPalette(Resource<GFXPalette> &_hPalette, int transition)
{
   if(transition >= 0 && ((GFXPalette *) hPalette) == ((GFXPalette *) _hPalette))
      return (bool) hPalette;

   hPalette = _hPalette;
   setPalette( transition );   
   return ( (bool)hPalette );
}

//------------------------------------------------------------------------------
void SimWorld::delPalette()
{
   if ( (bool)hPalette ) 
      ((Resource<GFXPalette>&)hPalette).unlock(0);
}   

//------------------------------------------------------------------------------
void SimWorld::render()
{
   if ( !renderEnabled ) return;

   // render all simCanvas' in the simCanvasSet
   SimCanvasSet* cgrp = static_cast<SimCanvasSet*>
		(manager.findObject(SimCanvasSetId));

   cgrp->render();
   // Notify those interested in FrameEndSet
	
	SimSet* grp = static_cast<SimSet*>(manager.findObject(SimFrameEndNotifySetId)); 

   SimFrameEndNotifyEvent event;
   grp->processEvent(&event);
}


