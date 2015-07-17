//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------


#include <sim.h>
#include <simTimerGrp.h>


//--------------------------------------------------------------------------- 

SimTimerSet::SimTimerSet(SimTime interval, SimObjectId _id)
{
	id = _id;
   timerInterval = interval;
}

//--------------------------------------------------------------------------- 

bool SimTimerSet::onAdd()
{
   SimTimeEvent::post(this,(float)manager->getCurrentTime() + timerInterval);
   return Parent::onAdd();
}

bool SimTimerSet::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
		case SimTimeEventType:
		{
			SimTimerEvent ev;
			ev.time = manager->getCurrentTime();
         ev.timerId = id;
			ev.timerInterval = timerInterval;
			// use index rather than iterator in case object added to timer set during update
			for (int i=0;i<objectList.size();i++)
				// We want immediate delivery...
				objectList[i]->processEvent(&ev);
			SimTimeEvent::post(this,(float)manager->getCurrentTime() + timerInterval);
			return true;
		}
	}
	return Parent::processEvent(event);
}
