//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

//#pragma warn -inl
//#pragma warn -aus

#include <sim.h>
#include <console.h>
#include "simPing.h"
#include "simAction.h"
#include "simGame.h"

//--------------------------------------------------------------------------- 

SimPingObject::SimPingObject()
{
	id = SimPingObjectId;
}

SimPingObject::~SimPingObject()
{
}

SimPingObject::Stats::Stats(SimObjectId id)
{
	managerId = id;
	simDelta = tickDelta = 0.0f;
	pingCount = 0;
	keepPinging = true;
}


//--------------------------------------------------------------------------- 

void SimPingObject::displayStats()
{
	for (ManagerList::iterator itr = managerList.begin();
			itr != managerList.end(); itr++) {
		Stats& stats = *itr;
		CMDConsole* console = CMDConsole::getLocked();
			console->printf("Ping: Mgr: %d  Sd %f  Td: %f",
				stats.managerId,stats.simDelta,stats.tickDelta);
	}
}	


//--------------------------------------------------------------------------- 

SimPingObject::Stats* SimPingObject::getManagerStats(SimObjectId managerId)
{
	// If an entry is not found for a manager, one is constructed.
	for (ManagerList::iterator itr = managerList.begin();
			itr != managerList.end(); itr++) {
		if ((*itr).managerId == managerId)
			return &(*itr);
	}
	managerList.increment();
	new(&managerList.last()) Stats(managerId);
	return &managerList.last();
}


//--------------------------------------------------------------------------- 

SimPingObject::Stats* SimPingObject::updateStats(const SimPingEvent* ev)
{
	Stats* stats = getManagerStats(ev->dst.address.managerId);
	stats->pingCount++;

	SimTime sDelta = manager->getCurrentTime() - ev->src.simTime;
	stats->simDelta = (stats->simDelta + sDelta) * 0.5f;

	Int32 tDelta = GetTickCount() - ev->src.tickTime;
	stats->tickDelta = (stats->tickDelta + tDelta) / 2;

	return stats;
}


//--------------------------------------------------------------------------- 

void SimPingObject::ping(SimObjectId managerId)
{
	SimPingEvent* pe = new SimPingEvent;
	//pe->flags.set(SimEvent::Guaranteed);
	pe->address.managerId = managerId;
	pe->address.objectId = SimPingObjectId;

	pe->dst.address = pe->address;
	pe->src.address.managerId = manager->getId();
	pe->src.address.objectId = SimPingObjectId;
	pe->src.simTime = manager->getCurrentTime();
	pe->src.tickTime = GetTickCount();

	manager->postCurrentEvent(pe);
}


//--------------------------------------------------------------------------- 

bool SimPingObject::processEvent(const SimEvent* event)
{
	switch (event->type) {
		case SimPingEventType: {
			const SimPingEvent* ev = static_cast<const SimPingEvent*>(event);
			if (ev->src.address.managerId == manager->getId()) {
				// One of my own.
				Stats* stats = updateStats(ev);
				if (stats->keepPinging)
					ping(stats->managerId);
			}
			else {
				// Echo pings back to source.
				SimPingEvent* pe = new SimPingEvent(*ev);
				pe->address = ev->src.address;
				pe->dst.simTime = manager->getCurrentTime();
				pe->dst.tickTime= GetTickCount();
				manager->postCurrentEvent(pe);
			}
			return true;
		}

		case SimActionEventType: {
			const SimActionEvent* ep = 
				static_cast<const SimActionEvent*>(event);
			switch(ep->action) {
				case Fire:
					displayStats();
					break;
				case StartFire: {
					Stats* stats = getManagerStats((int)ep->fValue);
					stats->keepPinging = true;
					ping(stats->managerId);
					break;
				}
				case EndFire: {
					Stats* stats = getManagerStats((int)ep->fValue);
					stats->keepPinging = false;
					break;
				}
			}
			return true;
		}
	}
	return false;
}


//--------------------------------------------------------------------------- 

bool SimPingObject::processQuery(SimQuery* /*query*/)
{
	return false;
}



//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT_TAG( SimPingEvent, SimPingEventType );

//--------------------------------------------------------------------------- 

Persistent::Base::Error 
SimPingEvent::read( StreamIO &sio,int a, int b )
{
   SimEvent::read(sio,a,b);
   sio.read(sizeof(Data),&src);
   sio.read(sizeof(Data),&dst);

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


//------------------------------------------------------------------------------

Persistent::Base::Error 
SimPingEvent::write( StreamIO &sio,int a, int b )
{
   SimEvent::write( sio, a, b );
   sio.write(sizeof(Data),&src);
   sio.write(sizeof(Data),&dst);

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}

