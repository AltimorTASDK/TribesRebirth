//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _SIMWAYPOINT_H_
#define _SIMWAYPOINT_H_

#include <sim.h>
#include "simMovement.h"
#include "simInterior.h"


//--------------------------------------------------------------------------- 

struct SimWaypointQuery: public SimQuery
{
	SimWaypointQuery() { type = SimWaypointQueryType; }

	// Index requested:
	int index;

	// Return values:
	int count;					// Total # of waypoints
	Point3F position;			// Waypoint[index] position
	EulerF  rotation;			// Waypoint[index] rotation
	Point3F vector;			// Vector from index to index+1
};


#endif
