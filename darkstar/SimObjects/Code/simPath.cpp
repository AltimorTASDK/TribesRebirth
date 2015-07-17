//---------------------------------------------------------------------------

// Group with sub-objects that respond to the SimObjectTransformQuery
// This group does not store it's sub-objects.  When the object is
// written out, it "compresses" the sub-object locations into a
// waypoint vector.
//
// The waypoint vector is "uncompressed" back into it's original
// objects the first time a SimEditEvent is recieved.
//
// The sub-objects must be amenable to being constructed and
// added to the manager with no other initialization than a
// SetObjectTransform event

// This group responds to SimWaypoint queries and returns data
// in either it's "compressed" or "uncompressed" state.

//---------------------------------------------------------------------------

#include <editor.strings.h>
#include <simpath.h>
#include <simpathquery.h>

//---------------------------------------------------------------------------

SimPath::SimPath()
{
   spm      = NULL;
   iPathID  = -1;
   fEditing = false;
   isCompressed = false;
   isLooping = false;
}

SimPath::~SimPath()
{
}

bool SimPath::onAdd()
{
   WaypointList::iterator  iter;

   Parent::onAdd();
   
   if(!isCompressed)
      compressWaypoints();

   // Get the path manager
   spm = dynamic_cast<SimPathManager *>
                     (manager->findObject(SimPathManagerId));

   AssertFatal(spm, "simPath::onAdd: cannot locate SimPathManager");

   // Create our path in the path manager
   iPathID = spm->addPath(iPathID, isLooping);

   AssertFatal(iPathID != -1, "simPath::onAdd: could not add path to manager");

   if (waypoint.size() > 0)
   {
      // Send all of this path's waypoints to the path manager
      for (iter = waypoint.begin(); iter < waypoint.end(); iter ++)
         spm->addWaypoint(iPathID, iter->position, iter->rotation);
   }

   // allow for an empty path
   return (true);
}

void SimPath::onRemove()
{
   // Delete the path and update the clients
   spm->processEvent(&SimPathEvent(NULL, iPathID));
	Parent::onRemove();
}


//----------------------------------------------------------------------------

bool SimPath::getCompressedWaypoint(SimWaypointQuery* wp)
{
	// Return position at index and vector index->index+1
	// Read it out of the waypoint vector
	wp->vector.set(0.0f,0.0f,0.0f);
	wp->count = waypoint.size();
	if (isLooping)
		wp->index %= wp->count;
	if (wp->index >= 0 && wp->index < waypoint.size()) {
		Waypoint &wp1 = waypoint[wp->index];
		wp->position = wp1.position;
		wp->rotation = wp1.rotation;

		// Build vector to next waypoint
		Waypoint *wp2;
		if (wp->index < waypoint.size() - 1)
			wp2 = &waypoint[wp->index+1];
		else
			wp2 = &waypoint[0];
		wp->vector.x = wp2->position.x - wp->position.x;
		wp->vector.y = wp2->position.y - wp->position.y;
		wp->vector.z = wp2->position.z - wp->position.z;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------

bool SimPath::getUncompressedWaypoint(SimWaypointQuery* wp)
{
	// Return position at index and vector index->index+1
	// Actually have to query our sub-objects.
	wp->vector.set(0.0f,0.0f,0.0f);
	wp->count = size();
	if (isLooping)
		wp->index %= wp->count;
	if (wp->index >= 0 && wp->index < size()) {
		SimObjectTransformQuery query;
		SimObject *obj = (*this)[wp->index];
		if (obj->processQuery(&query)) {
			wp->position = query.tmat.p;
			query.tmat.angles(&wp->rotation);

			// Build vector to next waypoint
			if (wp->index < size() - 1)
				obj = (*this)[wp->index+1];
			else
				obj = (*this)[0];
			if (obj->processQuery(&query)) {
				wp->vector.x = query.tmat.p.x - wp->position.x;
				wp->vector.y = query.tmat.p.y - wp->position.y;
				wp->vector.z = query.tmat.p.z - wp->position.z;
				return true;
			}
		}
	}
	return false;
}


//----------------------------------------------------------------------------

void SimPath::compressWaypoints()
{
	// Convert sub-objects into elements in the waypoint vector.
	// The compres flag is not set since this function is
	// called only on IO were we don't want to permanently
	// compress the waypoints (would have to delete the sub-objects)
	Waypoint way;
	waypoint.clear();
	waypoint.reserve(size());
	SimObjectTransformQuery query;
	for (iterator itr = begin(); itr != end(); itr++)
		if ((*itr)->processQuery(&query)) {
			way.persistTag = (*itr)->getPersistTag();
			way.position = query.tmat.p;
			query.tmat.angles(&way.rotation);
			waypoint.push_back(way);
		}
}


//----------------------------------------------------------------------------

void SimPath::uncompressWaypoints()
{
	// Convert elements in the waypoint vector into sub-objects
	// Builds the objects based on the persist tag of the
	// objects existing during the compress.
	SimObject* obj;
	SimObjectTransformEvent event;
	event.time = manager->getCurrentTime();
	for (int i = 0; i < waypoint.size(); i++) {
		Waypoint& wp = waypoint[i];
		if ((obj = static_cast<SimObject*>
				(Persistent::create(wp.persistTag))) != 0) {
			manager->registerObject(obj);
			addObject(obj);

			// Move it into position
			event.tmat.set(wp.rotation,wp.position);
			obj->processEvent(&event);
		}
	}
	isCompressed = false;
}

//---------------------------------------------------------------------------

bool SimPath::processArguments(int argc, const char **argv)
{
   return Parent::processArguments(argc,argv);
}

bool SimPath::onSimMessageEvent(const SimMessageEvent *)
{
   WaypointList::iterator  iter;
   SimPathManager::Path   *path;

   // re-compress the waypoints
   compressWaypoints();

   // Construct a path object
   path = new SimPathManager::Path(iPathID, isLooping);

   // Add all of its waypoints
   for (iter = waypoint.begin(); iter < waypoint.end(); iter ++)
   {
      path->waypoints.push_back(new 
         SimPathManager::Waypoint(iter->position, iter->rotation));
   }

   // Construct the path event object
   spm->processEvent(&SimPathEvent(path, iPathID));

   // Send another message event
   SimMessageEvent::post(this,
      (float)manager->getCurrentTime() + 5.0f, 0);

   return (true);
}

bool SimPath::onSimEditEvent(const SimEditEvent * event)
{
   // Mission editor is getting activated
   if (isCompressed)
      uncompressWaypoints();

   // send an edit event to each of the markers
	for(iterator itr = begin(); itr != end(); itr++)
      SimEditEvent::post(*itr, event->editorId, event->state);
   
   // Make sure we only do this once
   if (fEditing == false)
   {
      fEditing = true;

      // Start sending update messages to ourself
      SimMessageEvent::post(this,
         (float)manager->getCurrentTime() + 5.0f, 0);
   }

   return (true);
}

bool SimPath::processEvent(const SimEvent* event)
{
	switch(event->type)
   {
      onEvent(SimEditEvent);
      onEvent(SimMessageEvent);
	}
	return Parent::processEvent(event);
}

bool SimPath::processQuery(SimQuery* query)
{
	switch(query->type)
   {
      case SimPathQueryType :
      {
         SimPathQuery *q = static_cast<SimPathQuery *>(query);

         q->iPathID       = iPathID;
         q->iNumWaypoints = waypoint.size();

         return (true);
      }

		case SimWaypointQueryType:
      {
			SimWaypointQuery* q = static_cast<SimWaypointQuery*>(query);

			if (isCompressed)
				return getCompressedWaypoint(q);

			return getUncompressedWaypoint(q);
      }
	}

	return Parent::processQuery(query);
}


//----------------------------------------------------------------------------

enum Flags {
	Loop =         BIT(1),
	Compressed =   BIT(2),
};


void SimPath::initPersistFields()
{
   addField("isLooping", TypeBool, Offset(isLooping, SimPath));
   addField("isCompressed", TypeBool, Offset(isCompressed, SimPath));
}

Persistent::Base::Error SimPath::write(StreamIO &sio, int , int )
{
	// Override default base class IO, don't want to write or
	// read our sub-objects

	// Make sure the vector is filled in.
	if (!isCompressed)
		compressWaypoints();
   Int32 flags = 0;
   if(isCompressed)
      flags |= Compressed;
   if(isLooping)
      flags |= Loop;
	sio.write(flags);
	sio.write(Int16(waypoint.size()));
	sio.write(waypoint.size() * sizeof(WaypointList::value_type),
		waypoint.address());

   sio.write(iPathID);

	return (sio.getStatus() == STRM_OK) ? Ok: WriteError;
}

Persistent::Base::Error SimPath::read(StreamIO &sio, int , int )
{
	// Override default base class IO, don't want to write or
	// read our sub-objects

	Int32 flags;
	sio.read(&flags);
   isCompressed = true;
   if(flags & Loop)
      isLooping = true;
	Int16 size; sio.read(&size);
	waypoint.setSize(size);
	sio.read(size * sizeof(WaypointList::value_type),waypoint.address());

 	return (sio.getStatus() == STRM_OK) ? Ok: ReadError;
}

