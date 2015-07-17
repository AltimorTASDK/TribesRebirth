//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifdef _BORLANDC_
#pragma warn -inl
#pragma warn -aus
#endif

#include <ts.h>
#include <base.h>
#include <sim.h>

#include "GameBase.h"
#include "simTerrain.h"
#include "simResource.h"
#include "simInterior.h"

#include <simNetObject.h>
#include <m_random.h>

#include "darkstar.strings.h"
#include "fear.strings.h"

#include "DropPoints.h"


//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT(DropPointGroup);

//----------------------------------------------------------------------------

DropPointGroup::DropPointGroup (): SimGroup ()
{
	currentIndex = -1;
	random = false;
}

DropPointGroup::~DropPointGroup ()
{
}


//----------------------------------------------------------------------------

bool DropPointGroup::onDropPointQuery (DropPointQuery *query)
{
	SimObject *obj = NULL;

	// See if we can find the one they want, check first in
	// our group, then the named group.
	if (query->dropPointName) {
		if ((obj = findObject(query->dropPointName)) == 0) {
			if (SimObject* op = findObject(NamedDropPoints))
				if (SimGroup* gp = dynamic_cast<SimGroup*>(op))
					obj = gp->findObject(query->dropPointName);
		}
	}

	// If we can't find the one they asked for, we'll pick one.
	if (!obj && size()) {

		// Build a list to simplify selection (have to skip
		// over the named group).
		SimObjectList list;
		list.reserve(size());
		for (int i = 0; i < size(); i++) {
			obj = (*this)[i];
			const char* name = obj->getName();
			if (!name || strcmp(name,NamedDropPoints))
				list.push_back(obj);
		}

		if (list.size()) {
			if (random) {
				// Random selection (more or less)
				int seed = (int)manager->getCurrentTime();
				currentIndex = seed%(list.size());
			}
			else {
				// Sequenced selection
				if (++currentIndex >= list.size())
					currentIndex = 0;
			}
			obj = list[currentIndex];
		}
	}

	//
	if (obj) {
		SimObjectTransformQuery transQ;
		if (obj->processQuery (&transQ)) {
			query->tmat = transQ.tmat;
			return true;
		}
	}
	return false;
}


bool DropPointGroup::processQuery(SimQuery* query)
{
	switch (query->type){
		onQuery (DropPointQuery);
     
     default:
      return Parent::processQuery(query);
	}
}


//----------------------------------------------------------------------------

void DropPointGroup::inspectWrite(Inspect* s)
{
	s->write(IDITG_DROPPTS_RANDOM,random);
}

void DropPointGroup::inspectRead(Inspect* s)
{
	s->read(IDITG_DROPPTS_RANDOM,random);
}


//----------------------------------------------------------------------------

Persistent::Base::Error DropPointGroup::read(StreamIO &s, int version, int user)
{
	Parent::read(s,version,user);
	s.read(&random);
	return (s.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error DropPointGroup::write(StreamIO &s, int version, int user)
{
	Parent::write(s,version,user);
	s.write(random);
	return (s.getStatus() == STRM_OK)? Ok: WriteError;
}

