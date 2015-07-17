//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _SIMPING_H_
#define _SIMPING_H_

#include <sim.h>


//--------------------------------------------------------------------------- 

struct SimPingEvent: public SimEvent
{
	struct Data {
		SimAddress address;
		SimTime simTime;
		Int32 tickTime;
	};
	Data src;
	Data dst;

	//
	SimPingEvent() { type = SimPingEventType; }
	SimPingEvent(const SimPingEvent &event) { *this = event; }

   DECLARE_PERSISTENT( SimPingEvent );
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};


//--------------------------------------------------------------------------- 

class SimPingObject: public SimObject
{
	struct Stats {
		SimObjectId managerId;
		float simDelta;
		float tickDelta;
		bool keepPinging;
		int pingCount;
		Stats(SimObjectId);
	};
	typedef Vector<Stats> ManagerList;
	ManagerList managerList;

	void displayStats();
	Stats* getManagerStats(SimObjectId managerId);
	Stats* updateStats(const SimPingEvent* ev);
	void ping(SimObjectId managerId);

public:
	SimPingObject();
	~SimPingObject();
	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
};


#endif
