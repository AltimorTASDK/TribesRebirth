//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMRESOURCE_H_
#define _SIMRESOURCE_H_

#include "tVector.h"
#include "simBase.h"
#include "simEv.h"
#include "streams.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class DLLAPI SimResource: public SimSet
{
	// It's assumed there is only one SimResource object per manager
	// and that it's one of the first objects constructed and one of
	// the last destroyed.
private:
	// Only one manager for all SimResource objects (Static manager* in ResourceManager)
	static int mgrCount;

   ResourceManager *myMgr;

	// List of volumes registered as part of a SimResource set
	// Only want to register each volume once.
public:
	SimResource();
	~SimResource();

	ResourceManager* get();
	static ResourceManager* get(SimManager *manager);
	static SimResource* find(SimManager *manager);
   static ResourceObject *loadByName(SimManager *manager, const char *name, bool block);
   static ResourceObject *loadByTag(SimManager *manager, Int32 tag, bool block);
};


//------------------------------------------------------------------------------

inline ResourceManager* SimResource::get()
{
	return myMgr;
}

inline ResourceManager* SimResource::get(SimManager *manager)
{
   SimResource *p = static_cast<SimResource*>
   	(manager->findObject(SimResourceObjectId));
	return p? p->get(): 0;
}

inline SimResource* SimResource::find(SimManager *manager)
{
   return static_cast<SimResource*>
   	(manager->findObject(SimResourceObjectId));
}


#endif
