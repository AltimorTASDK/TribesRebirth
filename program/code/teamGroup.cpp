//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifdef _BORLANDC_
#pragma warn -inl
#pragma warn -aus
#endif

#include <base.h>
#include <sim.h>
#include <simResource.h>

#include "gameBase.h"
#include "teamGroup.h"
#include "PlayerManager.h"

#include "darkstar.strings.h"
#include "fear.strings.h"
#include "stringTable.h"

//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT(TeamGroup);


//----------------------------------------------------------------------------

TeamGroup::TeamGroup (): SimGroup ()
{
	teamId = -1;
}

//----------------------------------------------------------------------------

bool TeamGroup::onAdd (void)
{
	// Allocate id and assign name before calling the
	// parent onAdd because it will add all it's members
	// to the manager and the teamId needs to be set
	// before this happens.

   char nameBuf[10] = "teamX";
   
	PlayerManager *playerManager = 
		(PlayerManager*)(manager->findObject(PlayerManagerId));

   teamId = playerManager->getNumTeams();
   nameBuf[4] = '0' + teamId;
   SimObject::assignName(nameBuf);

	playerManager->teamAdded (teamId);

   return Parent::onAdd();
}


