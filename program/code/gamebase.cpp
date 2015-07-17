//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifdef _BORLANDC_
#pragma warn -inl
#pragma warn -aus
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4710)
#pragma warning(disable : 4101)
#endif

#include <typeinfo.h>

#include <ts.h>
#include <sim.h>
#include <simResource.h>
#include <console.h>

#include "GameBase.h"
#include "teamGroup.h"
#include "PlayerManager.h"

#include "commonEditor.strings.h"
#include "editor.strings.h"
#include "darkstar.strings.h"
#include "fear.strings.h"
#include "stringTable.h"
#include "dataBlockManager.h"
#include "sensorManager.h"
#include "fearglobals.h"
#include "fearplayerpsc.h"
#include "observercamera.h"
#include "projLaser.h"

#include <netghostmanager.h>

//--------------------------------------------------------------------------- 

// Timeouts used to control the rate at which the scripting
// callbacks are invoked.
static const float TimerCollisionTimeout = 1.0;

float GameBase::upFov       = 1.0;
float GameBase::upDistance  = 0.4;
float GameBase::upVelocity  = 0.4;
float GameBase::upSkips     = 0.2;
float GameBase::upOwnership = 0.2;
float GameBase::upInterest  = 0.2;


//--------------------------------------------------------------------------- 

GameBase::GameBaseData::GameBaseData()
{
	className = NULL;
   visibleToSensor = false;
	mapFilter = -1;
	description = stringTable.insert("Shape", true);
	mapIcon = NULL;
}	

bool GameBase::GameBaseData::preload(ResourceManager *rm, bool server, char errorBuffer[256])
{
   errorBuffer;
   if(server)
      return true;
   char buffer[1024];
   if(mapIcon)
   {
      sprintf(buffer, "%s_red.bmp", mapIcon);
      iconBmp[0] = rm->load(buffer);
      if(bool(iconBmp[0]))
         iconBmp[0]->attribute |= BMA_TRANSPARENT;
      sprintf(buffer, "%s_green.bmp", mapIcon);
      iconBmp[1] = rm->load(buffer);
      if(bool(iconBmp[1]))
         iconBmp[1]->attribute |= BMA_TRANSPARENT;
   }
   return true;
}


//--------------------------------------------------------------------------- 

GameBase::GameBase()
{
	teamId = -1;
	ownerId = 0;
   ctrlClientId = 0;
   ownerClientId = 0;
   lastOwnerClientId = 0;
	powerCount = 0;	// objects requiring power must have a generator...
	powerGenerator = false;
   lastAnimateTime = 0;
   lastProcessTime = 0;
   datFileId = -1;
   datFileName = NULL;
	timer = 0;
   sensorInfoBits = 0;
	data = 0;
	sensorKey = -1;
   mapName = NULL;
   fov = M_PI / 2;
}	

GameBase::~GameBase()
{
	if (timer)
		timer->free();
}

void GameBase::setDatFileName(const char *in_name)
{
   const char *oldDatFileName = datFileName;
   datFileName = stringTable.insert(in_name);
   if(data)
   {
      // data's been loaded already.. so this is a new one.
      datFileId = -1;
      if(!loadDatFile())
      {
         datFileName = oldDatFileName;
         loadDatFile();
      }
      else
         setMaskBits(BaseStateMask);
   }
}

bool GameBase::loadDatFile()
{
   DataBlockManager *dbm = (DataBlockManager *) manager->findObject(DataBlockManagerId);
	GameBaseData* res;
   if(datFileId == -1)
   {
      if(datFileName == NULL)
         return false;
      res = dbm->lookupDataBlock(datFileName, getDatGroup(), datFileId);
   }
   else
      res = dbm->lookupDataBlock(datFileId, getDatGroup());
   if(!res)
      return FALSE;
   if (initResources(res)) {
   	if (!isGhost())
			scriptOnInit();
		return true;
   }
	return false;
}

bool GameBase::processArguments(int argc, const char **argv)
{
   if(argc < 1)
      return false;
   datFileName = stringTable.insert(argv[0]);
   if (loadDatFile()) {
		if(!isGhost())
			scriptOnAdd();
		return true;
   }
	return false;
}

bool GameBase::initResources(GameBase::GameBaseData *ptr)
{
   data = ptr;

   if(!isGhost())
   {
      if(data->visibleToSensor)
      {
         setName(mapName);
         SensorManager *radMgr = dynamic_cast<SensorManager *>
                         (manager->findObject(SensorManagerId));
         sensorKey = radMgr->addTarget(this, mapName);
      }
   }
   else if(sensorKey != -1)
      addToSet(SensorVisibleSetId);
   return true;
}

void GameBase::packDatFile(BitStream *stream)
{
   stream->writeInt(datFileId, wg->dbm->getDataGroupBitSize(getDatGroup()));
}

void GameBase::unpackDatFile(BitStream *stream)
{
   datFileId = stream->readInt(wg->dbm->getDataGroupBitSize(getDatGroup()));
}

void GameBase::GameBaseData::pack(BitStream *stream)
{
   stream->writeSignedInt(mapFilter, 8);
   stream->writeString(mapIcon);
   stream->writeString(description);
}

void GameBase::GameBaseData::unpack(BitStream *stream)
{
   char buf[256];
   mapFilter = stream->readSignedInt(8);
   stream->readString(buf);
   mapIcon = stringTable.insert(buf);
   stream->readString(buf);
   description = stringTable.insert(buf, true);
}

int GameBase::getDatGroup()
{
   return -1;
}

float GameBase::getDamageFlash()
{
   return 0;
}

float GameBase::getDamageLevel()
{
	return 0;
}

float GameBase::getEnergyLevel (void)
{
	return 0;
}

void GameBase::getThrowVector(Point3F* pos,Point3F* vec)
{
	*pos = getBoxCenter();
	Point3F vv(0,8,0);
	const RMat3F& mat = getTransform();
	m_mul(vv,mat,vec);
}

void GameBase::throwObject(GameBase* object,float speed,bool careless)
{
	object,speed,careless;
}	

bool GameBase::onAdd()
{
   if(!Parent::onAdd())
      return false;

   DWORD time = wg->currentTime;

   lastProcessTime = time;
   lastAnimateTime = time;
   updateTeamId();

   if(datFileName || datFileId != -1)
   {
      bool ret = loadDatFile();
      if(ret && !isGhost())
         scriptOnAdd();
      return ret;
   }
   return true;
}

void GameBase::onRemove()
{
   PlayerManager::ClientRep *cl = wg->playerManager->findClient(ctrlClientId);
   if(cl && cl->controlObject == this)
      cl->controlObject = NULL;
   cl = wg->playerManager->findBaseRep(ownerClientId);
   if(cl && cl->ownedObject == this)
      cl->ownedObject = NULL;

	if (data && !isGhost())
		scriptOnRemove();

   if(sensorKey != -1 && !isGhost())
      ((SensorManager *) manager->findObject(SensorManagerId))->removeTarget(sensorKey);

	Parent::onRemove();
}

void GameBase::clientProcess(DWORD curTime)
{
	curTime;
}

void GameBase::serverProcess(DWORD curTime)
{
	curTime;
}

void GameBase::serverUpdateMove(PlayerMove *moves, int moveCount)
{
   moves;
   moveCount;
}

void GameBase::setPlayerControl(bool /*ctrl*/)
{
}

const char* GameBase::getScriptDefaultClass()
{
   return getClassName();
}


//----------------------------------------------------------------------------

void GameBase::updateTimers()
{
	if (timer && timer->allExpired(manager->getCurrentTime())) {
		timer->free();
		timer = 0;
	}
}


//----------------------------------------------------------------------------

bool GameBase::getMuzzleTransform (int, TMat3F *mat)
{
	*mat = getTransform ();

	return true;
}

void GameBase::getObjectMountTransform (int, TMat3F *mat)
{
	*mat = getTransform ();
}

void GameBase::getCameraTransform (float, TMat3F *mat)
{
	*mat = getTransform ();
}

bool GameBase::cameraZoomAndCrosshairsEnabled()
{
   return false;
}

//----------------------------------------------------------------------------

void GameBase::onGroupAdd()
{
	if (!isGhost()) {
		updatePowerCount();
		if (isGenerator())
			updatePowerClients(getGroup(),true);
	}
	updateTeamId();
}

void GameBase::onGroupRemove()
{
	if (!isGhost()) {
		if (isGenerator())
			updatePowerClients(getGroup(),false);
	}
}

void GameBase::onCollisionNotify(GameBase* obj)
{
}

bool GameBase::processEvent(const SimEvent* event)
{
	switch (event->type) {
		case SimMovementCollisionEventType:
			// FIX: Should check this using the object type
			GameBase* obj = (GameBase*)
				((SimMovementCollisionEvent*)event)->getObject(manager);
			if (obj)
				onCollisionNotify(obj);
			return true;
	}
	return Parent::processEvent(event);
}	

//----------------------------------------------------------------------------

void GameBase::setTeam(int _team)
{
	// Not much else to do at this point.
	int old = teamId;
	teamId = _team;
	if(!isGhost() && teamId != old)
   {
	   setMaskBits(BaseStateMask);
	   if (const char* script = scriptName("onSetTeam"))
	   	Console->evaluatef("%s(%s,%d);",script,scriptThis(),old);
      if (ownerClientId)
         sg.playerManager->setTeam(ownerClientId, teamId);
   }
}

SimObject *GameBase::getTeamObject (const char *objectName)
{
	SimGroup *teamGroup = getTeamGroup();
	return (teamGroup ? teamGroup->findObject (objectName) : NULL);
}

SimGroup *GameBase::getTeamGroup (void)
{
	AssertFatal(manager, "Getting team group without manager");
	
	PlayerManager *playerManager = 
		(PlayerManager*)(manager->findObject(PlayerManagerId));

	if (playerManager)
		return playerManager->getTeamGroup (getTeam());
	return NULL;
}

void GameBase::updateTeamId()
{
	if (!isGhost()) {
		// Let's find out what team we belong too.
		const char* msg = stringTable.lookup("MissionGroup");
		SimGroup* parent = getGroup();
		while (parent) {
			if (parent->getName() == msg) {
				// No team.
			   setTeam(-1);
				return;
			}
			if (TeamGroup* grp = dynamic_cast<TeamGroup*>(parent)) {
				setTeam(grp->getTeam());
				return;
			}
			parent = parent->getGroup();
		}
	}
}


//----------------------------------------------------------------------------

void GameBase::updatePowerCount()
{
	if (powerGenerator) {
		powerCount = 1;
		onPowerStateChange(this);
	}
	else
		powerCount = 0;
	// The generators may exist in this or in any parent group.
	SimGroup *group = getGroup();
	while (group) {
		for (int i = 0; i < group->size(); i++) {
			GameBase* obj = dynamic_cast<GameBase*>((*group)[i]);
			if (obj && obj->isGenerator()) {
				powerCount++;
				onPowerStateChange(obj);
			}
		}
		group = group->getGroup();
	}
}

void GameBase::updatePowerClients(SimGroup* group,bool power)
{
	if (group)
		for (int i = 0; i < group->size(); i++) {
			GameBase *obj = dynamic_cast<GameBase *>((*group)[i]);
			if (obj) {
				if (power) {
					++obj->powerCount;
					obj->onPowerStateChange(this);
				}
				else {
					if (obj->powerCount) {
						--obj->powerCount;
						obj->onPowerStateChange(this);
					}
				}
			}
			else {
				SimGroup *subGroup = dynamic_cast<SimGroup *>((*group)[i]);
				if (subGroup)
					updatePowerClients(subGroup, power);
			}
		}
}

void GameBase::generatePower(bool power)
{
	if (!isGhost() && powerGenerator != power) {
		if (manager) {
			powerGenerator = power;
			updatePowerClients(getGroup(),power);
		}
	}
}	

void GameBase::forcePowerState(bool on)
{
	// Normal power state management is not maintained by
	// this class on the on the client. This method is used
	// by derived classes that tansfer power states from
	// the server
	if (isGhost())
		powerCount = on? 1: 0;
}

void GameBase::onPowerStateChange(GameBase* generator)
{
	generator;
}	

//----------------------------------------------------------------------------
// Virtuals here for bas classes
bool GameBase::isActive()
{
	return true;
}

bool GameBase::setActive(bool a)
{
	return a;
}


//----------------------------------------------------------------------------

const char* GameBase::scriptThis()
{
	static char name[25];
	sprintf(name,"%d",getId());
	return name;
}

const char* GameBase::scriptName(const char* callback)
{
	static char name[256];

	// Based off object name
	if (const char* objName = getName()) {
		sprintf(name,"%s::%s",objName,callback);
		if (Console->isFunction(name))
			return name;
	}

	// Based off the dat file name provided by derived classes
	const char *dat = data ? data->dbmDatFileName : getDatFileName();
	if (dat) {
		sprintf(name,"%s::%s",dat,callback);
		if (Console->isFunction(name))
			return name;
	}

	// Classname specified in the dat file
	if (data && data->className && data->className[0]) {
		sprintf(name,"%s::%s",data->className,callback);
		if (Console->isFunction(name))
			return name;
	}

	// Based off the default (C++ class name)
	sprintf(name,"%s::%s",getScriptDefaultClass(),callback);
	if (Console->isFunction(name))
		return name;

	// Oh well, try the global
	if (Console->isFunction(callback)) {
		strcpy(name,callback);
		return name;
	}

	return 0;
}

//----------------------------------------------------------------------------

void GameBase::scriptOnAdd()
{
	if (const char* script = scriptName("onAdd"))
		Console->executef(2,script,scriptThis());
}

void GameBase::scriptOnInit()
{
	if (const char* script = scriptName("onInit"))
		Console->executef(2,script,scriptThis());
}

void GameBase::scriptOnRemove()
{
	if (const char* script = scriptName("onRemove"))
		Console->executef(2,script,scriptThis());
}

void GameBase::scriptOnCollision(GameBase* obj)
{
	if (!timer)
		timer = Timer::newTimer();
	if (timer->isCollisionExpired(obj,manager->getCurrentTime())) {
		if (const char* script = scriptName("onCollision")) {
			char buff[256];
			strcpy(buff,scriptThis());
			Console->executef(3,script,buff,obj->scriptThis());
		}
	}
}


//----------------------------------------------------------------------------

float GameBase::getUpdatePriority(CameraInfo *camInfo, DWORD updateMask,
	 int updateSkips)
{
	updateMask;
	// If it's the scope object, must be high priority
	if (camInfo->camera == this) {
		// Most priorities are between 0 and 1, so this
		// should be something larger.
		return 10.0f;
	} if (dynamic_cast<LaserProjectile*>(this) != NULL &&
	      (updateMask & 0x8000000) != 0) {
      // Laser creation is really important, less so if
      //  it's just a position update.
      return 9.9f;
   }

	// Calculate a priority used to decide if this object
	// will be updated on the client.  All the weights
	// are calculated 0 -> 1  Then weighted together at the
	// end to produce a priority.
	Point3F pos = getBoxCenter();
	pos -= camInfo->pos;
	float dist = pos.len();
	if (dist == 0.0f) dist = 0.001f;
	pos *= 1.0f / dist;

	// Weight based on linear distance, the basic stuff.
	float wDistance = (dist < camInfo->visibleDistance)?
		1.0f - (dist / camInfo->visibleDistance): 0.0f;

	// Weight by field of view, objects directly in front
	// will be weighted 1, objects behind will be 0
	float dot = m_dot(pos,camInfo->orientation);
   bool inFov = dot > camInfo->cosFov;
   float wFov = inFov? 1.0f: 0;

	// Weight by linear velocity parallel to the viewing plane
	// (if it's the field of view, 0 if it's not).
	float wVelocity = 0.0f;
	if (inFov) {
		Point3F vec;
		m_cross(camInfo->orientation,getLinearVelocity(),&vec);
		wVelocity = (vec.lenf() * camInfo->fov) /
			(camInfo->fov * camInfo->visibleDistance);
		if (wVelocity > 1.0f)
			wVelocity = 1.0f;
	}

	// Weight by interest.
	float wInterest;
	if (getType().test(SimPlayerObjectType))
		wInterest = 0.75f;
	else
		if (getType().test(SimProjectileObjectType)) {
			// Projectiles are more interesting if they
			// are heading for us.
			wInterest = 0.30f;
			float dot = -m_dot(pos,getLinearVelocity());
			if (dot > 0.0f)
				wInterest += 0.20 * dot;
         if (dynamic_cast<LaserProjectile*>(this) != NULL) {
            // Laser creations are very interesting
         }
		}
		else
			if (getType().test(ItemObjectType))
				wInterest = 0.25f;
			else
				// Everything else is less interesting.
				wInterest = 0.0f;

	// Weight by updateSkips
	float wSkips = updateSkips * 0.5;

	// Weight by ownership, anything belonging to the camera
	// should be of great interest.
	float wOwnership;
	if (getOwnerId() == camInfo->camera->getId())
		wOwnership = 1.0f;
	else
		wOwnership = 0.0f;

	//
	// Calculate final priority, should total to about 1.0f
	//
   
	float priority =
		wFov       *  upFov  +
		wDistance  *  upDistance  +
		wVelocity  *  upVelocity   +
		wSkips     *  upSkips      +
		wOwnership *  upOwnership  +
		wInterest  *  upInterest   ;
   return priority;
}


//----------------------------------------------------------------------------
// Packet PSC data used for mounted objects

void GameBase::writePacketData(BitStream *)
{
}

void GameBase::readPacketData(BitStream *)
{
}

//----------------------------------------------------------------------------

int GameBase::getOwnerClient()
{
   return ownerClientId;
}

int GameBase::getLastOwnerClient()
{
   return lastOwnerClientId;
}

int GameBase::getControlClient()
{
   return ctrlClientId;
}

void GameBase::setOwnerClient(int clientId)
{
   if(clientId != 0)
      lastOwnerClientId = clientId;
   if(clientId == ownerClientId)
      return;
   PlayerManager::ClientRep *cl = sg.playerManager->findBaseRep(ownerClientId);
   if(cl && cl->ownedObject == this)
      cl->ownedObject = NULL;
   cl = sg.playerManager->findBaseRep(clientId);
   if(cl && cl->ownedObject && cl->ownedObject != this)
      cl->ownedObject->setOwnerClient(0);
   if(cl)
      cl->ownedObject = this;
   ownerClientId = clientId;
   setMaskBits(BaseStateMask);
}

void GameBase::setControlClient(int clientId)
{
   if(clientId == ctrlClientId)
      return;
   PlayerManager::ClientRep *cl = sg.playerManager->findClient(ctrlClientId);
   if(cl && cl->controlObject == this)
   {
      cl->controlObject = NULL;
      cl->playerPSC->setControlObject(NULL);
   }
   cl = sg.playerManager->findClient(clientId);
   if(cl && cl->controlObject && cl->controlObject != this)
      cl->controlObject->setControlClient(0);
   if(cl)
   {
      cl->controlObject = this;
      cl->playerPSC->setControlObject(this);
   }
   ctrlClientId = clientId;
   setMaskBits(BaseStateMask);
}

//----------------------------------------------------------------------------

DWORD GameBase::packUpdate(Net::GhostManager *, DWORD mask, BitStream *stream)
{
   if (stream->writeFlag (mask & BaseStateMask)) {
		stream->writeInt(teamId,TeamIdBits);
      if(stream->writeFlag(ctrlClientId))
         stream->writeInt(ctrlClientId - 2048, 7);
      if(stream->writeFlag(ownerClientId))
         stream->writeInt(ownerClientId - 2048, 7);
	}
	return 0;
}

void GameBase::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
	if (stream->readFlag()) {
      int newTeam = stream->readInt(TeamIdBits);
      int newCtrlClient = 0;
      int newOwnerClient = 0;
      if(stream->readFlag())
         newCtrlClient = stream->readInt(7) + 2048;
      if(stream->readFlag())
         newOwnerClient = stream->readInt(7) + 2048;

      PlayerManager::ClientRep *cl;
      if(newCtrlClient != ctrlClientId)
      {
         cl = cg.playerManager->findClient(ctrlClientId);
         if(cl && cl->controlObject == this)
            cl->controlObject = NULL;
         cl = cg.playerManager->findClient(newCtrlClient);
         if(cl)
            cl->controlObject = this;
         ctrlClientId = newCtrlClient;
      }
      if(newOwnerClient != ownerClientId)
      {
         cl = cg.playerManager->findBaseRep(ownerClientId);
         if(cl && cl->ownedObject == this)
            cl->ownedObject = NULL;
         cl = cg.playerManager->findBaseRep(newOwnerClient);
         if(cl)
            cl->ownedObject = this;
         ownerClientId = newOwnerClient;
      }
      if(newOwnerClient != 0)
         lastOwnerClientId = newOwnerClient;
      setTeam(newTeam);
	}
}	


void GameBase::setName(const char *name)
{
   if(name)
   {
      char buf[256];
      int d = 0;

      while(*name)
      {
         if(*name == '<' && *(name+1) == 'v')
         {
            char *str = strchr((char *) name, '>');
            if(str)
            {
               *str = 0;
               const char *var = Console->getVariable(name + 2);
               if(strlen(var) + d > 254)
                  break;
               strcpy(buf + d, var);
               d += strlen(var);
               *str = '>';
               name = str + 1;
            }
            else
               buf[d++] = *name++;
         }
         else
            buf[d++] = *name++;

         if(d >= 255)
            break;
      }
      buf[d] = 0;
   
      mapName = stringTable.insert(buf, true);
   }
   else
      mapName = stringTable.insert("");
   if(!isGhost() && sensorKey != -1)
   {
      SensorManager *radMgr = dynamic_cast<SensorManager *>
                      (manager->findObject(SensorManagerId));
      radMgr->changeTargetName(sensorKey, mapName);
   }
}

//----------------------------------------------------------------------------

void GameBase::inspectRead(Inspect *inspector)
{
   SimObject::inspectRead(inspector);
	long team_num;
	inspector->read(IDITG_TEAM_NUMBER, team_num);
   Inspect_Str buf;
   inspector->read(IDITG_NAME, buf);
   setName(buf);
}

void GameBase::inspectWrite(Inspect *inspector)
{
   SimObject::inspectWrite(inspector);
	inspector->write(IDITG_TEAM_NUMBER, (long)getTeam () );
   inspector->write(IDITG_NAME, mapName ? (char *) mapName : "");
}


//---------------------------------------------------------------------------

void GameBase::initPersistFields()
{
   addField("dataBlock", TypeString, Offset(datFileName, GameBase));
   addField("name", TypeCaseString, Offset(mapName, GameBase));
}

//---------------------------------------------------------------------------

Persistent::Base::Error GameBase::read(StreamIO &sio, int, int)
{
	int version;
	sio.read (&version);

	Parent::read (sio, 0, 0);

	sio.read (&teamId);
	UInt8 count;
   char buff[256];

   sio.readString(buff);
   sio.readString(buff);
   if(buff[0])
      datFileName = stringTable.insert(buff);

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


Persistent::Base::Error GameBase::write(StreamIO &sio, int, int)
{
	int version = 0;
	sio.write (version);

	Parent::write (sio, 0, 0);

	sio.write (teamId);
   const char *scriptData = "";
	sio.writeString (scriptData);
   sio.writeString (datFileName);

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}


//---------------------------------------------------------------------------
// Timer and it's freeList
//---------------------------------------------------------------------------

GameBase::Timer::FreeList GameBase::Timer::freeList;


//----------------------------------------------------------------------------

GameBase::Timer::FreeList::FreeList()
{
	nextFree = 0;
}

GameBase::Timer::FreeList::~FreeList()
{
	while (nextFree) {
		GameBase::Timer* obj = nextFree;
		nextFree = obj->nextFree;
		delete obj;
	}
}

void GameBase::Timer::FreeList::push(GameBase::Timer* obj)
{
	obj->nextFree = nextFree;
	nextFree = obj;
}

GameBase::Timer* GameBase::Timer::FreeList::pop()
{
	if (GameBase::Timer *obj = nextFree) {
		nextFree = obj->nextFree;
		return obj;
	}
	return 0;
}


//---------------------------------------------------------------------------

GameBase::Timer::Timer()
{
	entryList.reserve(ReservedSize);
	nextFree = 0;
	maxTime = 0;
}

GameBase::Timer::~Timer()
{
}


//----------------------------------------------------------------------------

GameBase::Timer* GameBase::Timer::newTimer()
{
	if (Timer* obj = freeList.pop()) {
		obj->entryList.clear();
		return obj;
	}
	return new Timer();
}

void GameBase::Timer::free()
{
	freeList.push(this);
}


//----------------------------------------------------------------------------

GameBase::Timer::Entry* GameBase::Timer::find(SimObject* obj,SimTime time)
{
	for (int i = 0; i < entryList.size(); i++ ) {
		Entry* entry = &entryList[i];
		if (entry->object == obj)
			return entry;
		if (time > entry->collisionTime) {
			entry->object = obj;
			entry->collisionTime = 0;
			return entry;
		}
	}
	entryList.increment();
	Entry& entry = entryList.last();
	entry.object = obj;
	entry.collisionTime = 0;
	return &entry;
}

bool GameBase::Timer::isCollisionExpired(SimObject* obj,SimTime time)
{
	Entry* entry = find(obj,time);
	if (time > entry->collisionTime) {
		if ((entry->collisionTime = time + TimerCollisionTimeout) > maxTime)
			maxTime = entry->collisionTime;
		return true;
	}
	return false;
}

bool GameBase::Timer::allExpired(SimTime time)
{
	return time > maxTime;
}

bool GameBase::getCommandStatus(CommandStatus* status)
{
	status->active = false;
	status->damageLevel = 0;
	return false;
}

void GameBase::setSensorPinged(BYTE fPinged)
{
   sensorInfoBits = (sensorInfoBits & ((1 << SensorPingedBase) - 1)) |
               (DWORD(fPinged) << SensorPingedBase);
}

BYTE GameBase::getSensorPinged()
{
   return BYTE(sensorInfoBits >> SensorPingedBase);
}

void GameBase::setAlwaysVisibleToTeam(int iTeamID, bool fVisible)
{
   if (fVisible)
      sensorInfoBits |= (1 << (iTeamID + AlwaysVisBase));
   else
      sensorInfoBits &= ~(1 << (iTeamID + AlwaysVisBase));
}

bool GameBase::getAlwaysVisibleToTeam(int iTeamID)
{
   return bool(sensorInfoBits & (1 << (iTeamID + AlwaysVisBase)));
}

void GameBase::setNeverVisibleToTeam(int iTeamID, bool fVisible)
{
   if (fVisible)
      sensorInfoBits |= (1 << (iTeamID + NeverVisBase));
   else
      sensorInfoBits &= ~(1 << (iTeamID + NeverVisBase));
}

bool GameBase::getNeverVisibleToTeam(int iTeamID)
{
   return bool(sensorInfoBits & (1 << (iTeamID + NeverVisBase)));
}

void GameBase::setVisibleToTeam(int iTeamID, bool fVisible)
{
   if (fVisible)
      sensorInfoBits |= (1 << (iTeamID + TeamVisBase)) &
      	~(sensorInfoBits >> (NeverVisBase - TeamVisBase));
   else
      sensorInfoBits &= ~(1 << (iTeamID + TeamVisBase));
}

bool GameBase::getVisibleToTeam(int iTeamID)
{
   return bool(sensorInfoBits & 
   	((1 << (iTeamID + AlwaysVisBase)) |
		(1 << (iTeamID + TeamVisBase))));
}

bool GameBase::getImageEnhanceable()
{
   // gotta use psc info...
   return (false);
}

bool GameBase::isCloaked()
{
   return (false);
}

bool GameBase::isJammingSensor()
{
   return (false);
}

float GameBase::getHaze(const Point3F &cameraPos)
{
   if(!getContainer())
      return 1.0;
   return (getContainer()->getHazeValue(cameraPos, getLinearPosition()));
}

//------------------------------------------------------------------------------

#define CameraRadius	0.025;
float GameBase::validateEyePoint (TMat3F *trans, float camDist)
{
	SimContainerQuery cq;
	cq.id = getId();
	cq.type = -1;
	cq.mask = SimTerrainObjectType | SimInteriorObjectType | VehicleObjectType | MoveableObjectType | StaticObjectType;
	
   // start los at eye point
	cq.box.fMin = trans->p;

   // end it at ffdv * -camDist + pos
   Point3F dir;
   trans->getRow(1, &dir);

   cq.box.fMax = dir;
   cq.box.fMax *= -camDist * 2.5; // test a longer los in case
   cq.box.fMax += trans->p;
   
   SimCollisionInfo info;
	SimContainer* root = findObject(manager,SimRootContainerId,(SimContainer*)0);
   
   // run los's through all backfacing surfaces:

   if(!root->findLOS(cq,&info))
   {
      dir *= camDist;
      trans->p -= dir;
      return camDist;
   }

	// Should be only one surface.
	CollisionSurfaceList& surfaceList = info.surfaces;
   Point3F normal;
   Point3F endpos;

	m_mul(surfaceList[0].position,surfaceList.tWorld,&endpos);
   m_mul(surfaceList[0].normal,(RMat3F &) surfaceList.tWorld, &normal);

   float dot = m_dot(dir, normal);
   if(dot >= 0.01)
   {
      float colDist = m_dot(trans->p - endpos, dir) - (1 / dot) * CameraRadius;
      if(colDist > camDist)
         colDist = camDist;
      if(colDist < 0)
         colDist = 0;

      dir *= colDist;
      trans->p -= dir;
		camDist = colDist;
   }
   else
   {
      // for now just ignore collision
      // FIXME: this is the case we should re-run
      // by subbing in the collision point for cq.box.fMin

      dir *= camDist;
      trans->p -= dir;
   }

	return camDist;
}

//------------------------------------------------------------------------------
void GameBase::buildScopeAndCameraInfo(Net::GhostManager *cr, CameraInfo *camInfo)
{
   TMat3F transform = getEyeTransform();
   if (getContainer() != NULL)
      camInfo->visibleDistance = getContainer()->getScopeVisibleDist();
   else
		camInfo->visibleDistance = 1500.0f;

   transform.getRow(1, &camInfo->orientation);
   camInfo->camera = this;
   camInfo->pos = transform.p;
	// Sin of 1/2 total FOV.
	camInfo->fov = fov / 2;
	camInfo->sinFov = m_sin(fov * 0.5f);
	camInfo->cosFov = m_cos(fov * 0.5f);

	if( wg->editMode ) {
		// Base class scopes everything
		SimNetObject::buildScopeAndCameraInfo(cr,camInfo);
		return;
	}

	// Always scope myself and my observer cam
   PlayerManager::ClientRep *cl = sg.playerManager->findClient(ctrlClientId);
	cr->objectInScope(this);
   if(cl && cl->observerCamera)
      cr->objectInScope(cl->observerCamera);

	// Scope everything else
	SimContainer* root = findObject(manager,SimRootContainerId,root);
	if (root) {
		SimContainerScopeContext scopeContext;
		scopeContext.lock(cr,camInfo);
		root->scope(scopeContext);
		scopeContext.unlock();
	}
}

//---------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

void FearRenderImage::renderBoundingBox(TSRenderContext& rc)
{
   TS::PointArray* pArray = rc.getPointArray();
   Point3F bboxPts[8];
   
   // Draw container bbox
   bboxPts[0].set(m_containerBox.fMin.x, m_containerBox.fMin.y, m_containerBox.fMin.z);
   bboxPts[1].set(m_containerBox.fMin.x, m_containerBox.fMax.y, m_containerBox.fMin.z);
   bboxPts[2].set(m_containerBox.fMin.x, m_containerBox.fMax.y, m_containerBox.fMax.z);
   bboxPts[3].set(m_containerBox.fMin.x, m_containerBox.fMin.y, m_containerBox.fMax.z);
   bboxPts[4].set(m_containerBox.fMax.x, m_containerBox.fMin.y, m_containerBox.fMin.z);
   bboxPts[5].set(m_containerBox.fMax.x, m_containerBox.fMax.y, m_containerBox.fMin.z);
   bboxPts[6].set(m_containerBox.fMax.x, m_containerBox.fMax.y, m_containerBox.fMax.z);
   bboxPts[7].set(m_containerBox.fMax.x, m_containerBox.fMin.y, m_containerBox.fMax.z);
   
   pArray->reset();
   int start = pArray->addPoints(8, bboxPts);
   
   const int color = 253;
   pArray->drawLine(start + 0, start + 1, color);
   pArray->drawLine(start + 1, start + 2, color);
   pArray->drawLine(start + 2, start + 3, color);
   pArray->drawLine(start + 3, start + 0, color);
   pArray->drawLine(start + 4, start + 5, color);
   pArray->drawLine(start + 5, start + 6, color);
   pArray->drawLine(start + 6, start + 7, color);
   pArray->drawLine(start + 7, start + 4, color);
   pArray->drawLine(start + 0, start + 4, color);
   pArray->drawLine(start + 1, start + 5, color);
   pArray->drawLine(start + 2, start + 6, color);
   pArray->drawLine(start + 3, start + 7, color);
}

//------------------------------------------------------------------------------

void FearRenderImage::render(TSRenderContext& rc)
{
   ColorF oldColor;
   ColorF oldAmbientColor;
   int    sunIndex = -1;

   TS::SceneLighting* pLighting = rc.getLights();
   if (m_attenuateSun == true) {
      // Note that this assumes that the sunlight is the only directional light...
      //
      for (int i = 0; i < pLighting->size(); i++) {
         TS::Light* rLight = (*pLighting)[i];

         if ((rLight->fLight.fType == TS::Light::LightDirectional ||
              rLight->fLight.fType == TS::Light::LightDirectionalWrap) &&
             (rLight->fLight.fRed   != 0.0 ||
              rLight->fLight.fGreen != 0.0 ||
              rLight->fLight.fBlue  != 0.0)) {
            oldColor.set(rLight->fLight.fRed,
                         rLight->fLight.fGreen,
                         rLight->fLight.fBlue);

            ColorF newColor;
            newColor.interpolate(oldColor, m_attenuationColor, m_attenuationFactor);

            rLight->fLight.fRed   = newColor.red;
            rLight->fLight.fGreen = newColor.green;
            rLight->fLight.fBlue  = newColor.blue;

            sunIndex = i;
            break;
         }
      }

      // HACK for ambient light
      //
      float attenIntensity = 0.3 * m_attenuationColor.red   +
                             0.5 * m_attenuationColor.green +
                             0.2 * m_attenuationColor.blue;
      attenIntensity *= m_attenuationFactor;
      oldAmbientColor = pLighting->getAmbientIntensity();
      float newAtten = attenIntensity * 2.0f;
      if (newAtten > 1.0f)
         newAtten = 1.0f;
      ColorF newAmb = oldAmbientColor * newAtten;
      pLighting->setAmbientIntensity(newAmb);
   }

   TribesShadowRenderImage::render(rc);

   if (m_attenuateSun == true) {
      if (sunIndex != -1) {
         TS::Light* rLight = (*pLighting)[sunIndex];

         rLight->fLight.fRed   = oldColor.red;
         rLight->fLight.fGreen = oldColor.green;
         rLight->fLight.fBlue  = oldColor.blue;
      }
      pLighting->setAmbientIntensity(oldAmbientColor);
   }

//   if (SimShape::getDrawShapeBBox() == true)
//      renderBoundingBox(rc);
}

void
FearRenderImage::updateSunOverride(const bool    in_attenuate,
                                   const float   in_factor,
                                   const ColorF& in_color)
{
   m_attenuateSun      = in_attenuate;
   m_attenuationFactor = in_factor;
   m_attenuationColor  = in_color;
}
