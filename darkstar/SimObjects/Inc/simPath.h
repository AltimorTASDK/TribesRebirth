//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

#ifndef _SIMPATH_H_
#define _SIMPATH_H_

#include <sim.h>
#include "simMovement.h"
#include "simInterior.h"
#include "simWaypoint.h"
#include <simpathmanager.h>


//--------------------------------------------------------------------------- 

class SimPath: public SimGroup
{
	typedef SimGroup Parent;
public:
private:
	struct Waypoint {
		Point3F position;
		EulerF  rotation;
		Int32   persistTag;
	};
	typedef Vector<Waypoint> WaypointList;

	//
   bool            fEditing;
   Int32           iPathID;
	WaypointList    waypoint;
   SimPathManager *spm;
   bool           isLooping;
   bool           isCompressed;

	//
protected:
	void compressWaypoints();
	void uncompressWaypoints();
	bool getCompressedWaypoint(SimWaypointQuery* query);
	bool getUncompressedWaypoint(SimWaypointQuery* query);

public:
	SimPath();
	~SimPath();

	bool onAdd();
	void onRemove();
   bool onSimEditEvent(const SimEditEvent *);
   bool onSimMessageEvent(const SimMessageEvent *);
   bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
   bool processArguments(int argc, const char **argv);

   inline Int32 getPathID(){ return iPathID; }

   DECLARE_PERSISTENT(SimPath);
	static void initPersistFields();
	Persistent::Base::Error write(StreamIO &sio, int, int);
	Persistent::Base::Error read(StreamIO &sio, int, int);
};


#endif
