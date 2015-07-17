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
#include "simResource.h"
#include "simTagDictionary.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int SimResource::mgrCount;

//------------------------------------------------------------------------------

SimResource::SimResource()
{
	id = SimResourceObjectId;
	if (!ResourceManager::sm_pManager)
		ResourceManager::sm_pManager = new ResourceManager;
	mgrCount++;
   myMgr = ResourceManager::sm_pManager;
}

SimResource::~SimResource()
{
	// I guess we assume here that all of the member objects
	// have been deleted already.
	if (!--mgrCount) {
		delete ResourceManager::sm_pManager;
		ResourceManager::sm_pManager = 0;
	}
}

ResourceObject *SimResource::loadByName(SimManager *manager, const char *name, bool block)
{
   block;
   if(!manager)
      return NULL;

   SimResource *p = (SimResource *) manager->findObject(SimResourceObjectId);
   if(!p)
      return NULL;
   return p->get()->load(name);
}

ResourceObject *SimResource::loadByTag(SimManager *manager, Int32 tag, bool block)
{
   block;
   if(!manager)
      return NULL;
   SimResource *p = (SimResource *) manager->findObject(SimResourceObjectId);
   if(!p)
      return NULL;
   if(!tag)
      return NULL;
   const char *name = SimTagDictionary::getString(manager, tag);
   if(!name)
      return NULL;
   return p->get()->load(name);
}