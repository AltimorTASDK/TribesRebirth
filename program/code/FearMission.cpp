//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#pragma warn -inl
#pragma warn -aus

#include <ts.h>
#include <base.h>
#include <sim.h>

#include "simConsole.h"
#include "FearMission.h"
#include "Player.h"
#include "FearCam.h"
#include "Fear.Strings.h"

//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT(FearMission);


//--------------------------------------------------------------------------- 

FearMission::FearMission(const char *name)
{
   strcpy(missionName, name);
   netFlags.set( UseForScoping);
   netFlags.set( NoGhost);
}

FearMission::FearMission ()
{
   missionName[0] = 0;
}

FearMission::~FearMission()
{
}


//--------------------------------------------------------------------------- 

void FearMission::loadServerResources ()
{
	SimSet *group = new SimSet (true);
	manager->addObject (group);
	manager->assignName (group, "MissionCleanup");

   char nameBuf[256];
	sprintf (nameBuf, "%s.server.cs", missionName);
	FileRStream stream;
	stream.open (nameBuf);
	if (stream.getStatus() != STRM_OK)
      strcpy(nameBuf, "default.server.cs");

	CMDConsole::getLocked()->evaluate( nameBuf );

   sprintf(nameBuf, "%s.mis", missionName);
   Persistent::Base::Error err;
	Persistent::Base* base = Persistent::Base::fileLoad( nameBuf, &err );
	if (err == Persistent::Base::Ok && base)
	{
		SimSet* obj = dynamic_cast<SimSet*>(base);
		if (obj)
		{
			manager->addObject(obj);
			manager->assignName(obj, "MissionGroup");
         obj->setOwnObjects(true);
		}
		else
		{
			delete base;
		}
	}
}

void FearMission::loadClientResources ()
{
}

//--------------------------------------------------------------------------- 

void FearMission::onRemove()
{
	SimObject *group = manager->findObject("MissionCleanup");

   if(group)
      group->deleteObject();
	
	group = manager->findObject("MissionGroup");

   if(group)
      group->deleteObject();
   
   Parent::onRemove();
}

bool FearMission::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

	if (!isGhost())
		loadServerResources ();
	else
		loadClientResources ();

	if (!id)
		manager->assignId (this);
		
	return true;
}


//--------------------------------------------------------------------------- 

//---------------------------------------------------------------------------

Persistent::Base::Error FearMission::read(StreamIO &sio, int, int)
{
	SimNetObject::read (sio, 0, 0);
	
	int size;
	
	sio.read (&size);
	if (size)
		{
			sio.read (size, missionName);
         missionName[size] = 0;
		}
	else
		missionName[0] = 0;

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


Persistent::Base::Error FearMission::write(StreamIO &sio, int, int)
{
	SimNetObject::write (sio, 0, 0);
	
	int size;
	
   size = strlen(missionName);
	sio.write (size);
	if (size)
		sio.write (size, missionName);

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}
