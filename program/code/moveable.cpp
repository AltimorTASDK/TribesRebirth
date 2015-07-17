//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <feardcl.h>
#include <soundfx.h>
#include <fearsfx.h>
#include <console.h>
#include <consoleinternal.h>
#include <simpathquery.h>
#include <sfx.strings.h>
#include <fear.strings.h>
#include <editor.strings.h>
#include <simresource.h>
#include <datablockmanager.h>
#include <moveable.h>
#include <fearGlobals.h>
#include "tsfx.h"


//----------------------------------------------------------------------------

#define UndefinedPathId   ((1 << Moveable::PathIdBits) - 1)
#define UndefinedWaypoint ((1 << Moveable::WaypointBits) - 1)


//----------------------------------------------------------------------------

Moveable::MoveableData::MoveableData() : StaticBaseData()
{
	displace = true;
	blockForward = true;
	blockBackward = true;
   sfxStart = -1;
   sfxStop = -1;
   sfxRun = -1;
	sfxBlocked = -1;
   speed = 1;
	side = NULL;
}

void Moveable::MoveableData::pack(BitStream *stream)
{
   Parent::pack(stream);
	stream->writeFlag(displace);
	stream->writeFlag(blockForward);
	stream->writeFlag(blockBackward);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxStart);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxStop);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxRun);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxBlocked);
	stream->write(speed);
}

void Moveable::MoveableData::unpack(BitStream *stream)
{
   Parent::unpack(stream);
	displace = stream->readFlag();
	blockForward = stream->readFlag();
	blockBackward = stream->readFlag();
	sfxStart = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	sfxStop = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	sfxRun = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	sfxBlocked = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	stream->read(&speed);
}


//----------------------------------------------------------------------------

Moveable::Moveable()
{
   type = MoveableObjectType;
   data = 0;
   moveState = Undefined;
	waypointStop = UndefinedWaypoint;

   spm = 0;
   pathId = UndefinedPathId;

	cachedWaypoint = -1;
	position.waypoint = 0;
	position.time = 0;
	currentPos.time = position.time;
	currentPos.waypoint = -1;
	sound = 0;
	runSound = false;
	blocked = false;

	pathCount = 0;
	pathTime = 1.0;
   lPosition.set(0.0f, 0.0f, 0.0f);
   aPosition.set(0.0f, 0.0f, 0.0f);
   lVector.set(0.0f, 0.0f, 0.0f);
   aVector.set(0.0f, 0.0f, 0.0f);
}

Moveable::~Moveable()
{
}


//----------------------------------------------------------------------------

bool Moveable::onAdd()
{
   if (!Parent::onAdd())
      return false;

	// Find the path manager
	spm = dynamic_cast<SimPathManager *>(manager->findObject(SimPathManagerId));
	AssertFatal(spm, "Pathed object could not locate path manager");

	// Switch to moveable update set
	removeFromSet(PlayerSetId);
	addToSet(MoveableSetId);

	if (moveState != Undefined) {
		buildMovementVector();
		updatePosition(true);
		setMoveState(moveState);
	}
   return true;
}

void Moveable::onRemove()
{
	Parent::onRemove();
	if (sound)
		Sfx::Manager::Stop(manager, sound);
}

int Moveable::getDatGroup()
{
   return DataBlockManager::MoveableDataType;
}

bool Moveable::initResources(GameBase::GameBaseData *gbData)
{
   if (!Parent::initResources(gbData))
		return false;
   data = static_cast<MoveableData *>(gbData);

   return true;
}


//----------------------------------------------------------------------------

void Moveable::serverProcess (DWORD ct)
{
	Parent::serverProcess(ct);

	if (pathId == UndefinedPathId) {
		if (!findPathId())
			return;
		setMoveState(moveState);
	}
	updateMoveState(0.032f);
}

void Moveable::clientProcess (DWORD curTime)
{
   float animTimeDelta = getAnimateDelta(curTime);
	Parent::clientProcess(curTime);
	lastProcessTime = curTime;

#if 0
		updateMoveState(animTimeDelta);
#else
		buildMovementVector();
		updatePosition(true);
#endif

	// Update sound position
	if (sound)
		if (Sfx::Manager *man = Sfx::Manager::find(manager)) {
	  		man->selectHandle(sound);
	 		man->setTransform(getTransform(), Point3F(0, 0, 0));
		}
}


//----------------------------------------------------------------------------

void Moveable::setPathId(int pid)
{
	pathId = pid;
   SimPathManager::Waypoint *pwp = 0;
   pathCount = spm->getWaypoint(pathId, position.waypoint, &pwp);
	if (!isGhost())
		onNewPath();
	buildMovementVector();
	updatePosition(true);
	if (moveState == Undefined)
		setMoveState(StopState);
	setMaskBits(PathMask);
}

bool Moveable::findPathId()
{
	// Search for a SimPath object in our group.
   SimPathQuery query;
   SimGroup &group = *getGroup();
   for (SimGroup::iterator iter = group.begin(); iter < group.end(); iter++) {
      if ((*iter)->processQuery(&query)) {
         setPathId(query.iPathID);
			return true;
      }
   }
	return false;
}


//----------------------------------------------------------------------------

void Moveable::setMoveState(State newState)
{
	if (pathId == UndefinedPathId) {
		moveState = newState;
		return;
	}
	buildMovementVector();

	// A little validation to avoid problems while editing.
	if (position.waypoint > pathCount - 1) {
		position.waypoint = pathCount - 1;
		position.time = 0;
	}
	if (position.waypoint == pathCount - 1 && position.time > 0)
		position.time = 0;

	// Make sure we are in the Stop moveState if we can't move
	if (newState == ForwardState) {
		if (position.waypoint == pathCount - 1)
			newState = StopState;
	}
	else
		if (newState == BackwardState)
			if (!position.waypoint && !position.time)
				newState = StopState;

	//
	if (!isGhost()) {
		if (newState != moveState) {
			moveState = newState;
			setMaskBits(StateMask | PositionMask);
		}
	}
	else {
		if (newState != moveState) {
			if (moveState != Undefined)
				if (newState == StopState) {
					if (getState() == Enabled && data->sfxStop != -1)
						TSFX::PlayAt(data->sfxStop, getTransform(),
							Point3F(0, 0, 0));
				}
				else
					if (data->sfxStart != -1)
						TSFX::PlayAt(data->sfxStart,getTransform(),
							Point3F(0, 0, 0));
			if (newState == StopState && sound) {
				Sfx::Manager::Stop(manager, sound);
				sound = 0;
			}
			moveState = newState;
		}
		if (moveState != StopState) {
			if (blocked) {
				if (sound && runSound) {
					Sfx::Manager::Stop(manager, sound);
					runSound = false;
					sound = 0;
				}
				if (!sound && data->sfxBlocked != -1)
					sound = TSFX::PlayAt(data->sfxBlocked,
						getTransform(), Point3F(0, 0, 0));
			}
			else {
				if (sound && !runSound) {
					Sfx::Manager::Stop(manager, sound);
					sound = 0;
				}
				if (!sound && data->sfxRun != -1) {
					sound = TSFX::PlayAt(data->sfxRun,
						getTransform(), Point3F(0, 0, 0));
					runSound = true;
				}
			}
		}
	}
}


//----------------------------------------------------------------------------

void Moveable::updateMoveState(float dt)
{
	if (pathId == UndefinedPathId || !dt)
		return;
	switch (moveState) {
		case StopState:
			blocked = false;
			break;
		case ForwardState: {
			Position oldPos = position;
			buildMovementVector();
			while (dt > 0 && moveState == ForwardState) {
				position.time += dt;
				if (position.time > pathTime) {
					dt = position.time - pathTime;
					position.time = 0;
					position.waypoint++;
					buildMovementVector();
				}
				else
					dt = 0;
				if (!updatePosition(isGhost())) {
					position = oldPos;
					break;
				}
				setMaskBits(PositionMask);
				if (position.waypoint == pathCount - 1) {
					if (!isGhost()) {
						onWaypoint();
						onLast();
					}
					if (moveState == ForwardState)
						setMoveState(StopState);
				}
				else
					if (!position.time) {
						if (position.waypoint == waypointStop)
							setMoveState(StopState);
						if (!isGhost())
							onWaypoint();
					}
			}
			break;
		}
		case BackwardState: {
			Position oldPos = position;
			buildMovementVector();
			while (dt > 0 && moveState == BackwardState) {
				position.time -= dt;
				if (position.time <= 0) {
					dt = -position.time;
					if (position.waypoint == 0)
						position.time = 0;
					else {
						position.waypoint--;
						buildMovementVector();
						position.time = pathTime;
					}
				}
				else
					dt = 0;
				if (!updatePosition(isGhost())) {
					position = oldPos;
					break;
				}
				setMaskBits(PositionMask);
				if (!position.time) {
					if (!isGhost()) {
						onWaypoint();
						onFirst();
					}
					if (moveState == BackwardState)
						setMoveState(StopState);
				}
				else
					if (position.time == pathTime) {
						if (position.waypoint + 1 == waypointStop)
							setMoveState(StopState);
						if (!isGhost())
							onWaypoint();
					}
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------

void Moveable::onDeactivate()
{
	setMoveState(StopState);
	Parent::onDeactivate();
}

void Moveable::onDamageStateChange(DamageState prevState)
{
	if (getState() != Enabled)
		setMoveState(StopState);
	Parent::onDamageStateChange(prevState);
}

//----------------------------------------------------------------------------

void Moveable::buildMovementVector()
{
	if (cachedWaypoint != position.waypoint) {
		cachedWaypoint = position.waypoint;

	   // Get waypoint from path manager
	   SimPathManager::Waypoint *pwp = 0;
	   pathCount = spm->getWaypoint(pathId, position.waypoint, &pwp);
	   if (!pwp || !pathCount || position.waypoint >= pathCount) {
			cachedWaypoint = -1;
			position.waypoint = 0;
			position.time = 0;
			return;
	   }

		lPosition = pwp->position;
		aPosition.x = pwp->rotation.x;
		aPosition.y = pwp->rotation.y;
		aPosition.z = pwp->rotation.z;
		lVector = pwp->lVector;
		aVector = pwp->aVector;

		// Get travel time for this segment.
		pathTime = lVector.len() / data->speed;
		if (!pathTime)
			pathTime = 1.0;
		if (position.time > pathTime)
			position.time = pathTime;
	}
}



//----------------------------------------------------------------------------

bool Moveable::updatePosition(bool force)
{
	if (position.waypoint == currentPos.waypoint &&
			position.time == currentPos.time)
		return true;

	// Store old stuff for unwinding
	TMat3F mat = getTransform();
	Point3F oldRot = getRot();
	Point3F oldPos = getPos();

	// New pos/rot
	Point3F rot,pos;
	if (pathTime) {
		float dt = position.time / pathTime;
	   pos.x = lPosition.x + (lVector.x * dt);
	   pos.y = lPosition.y + (lVector.y * dt);
	   pos.z = lPosition.z + (lVector.z * dt);
	   rot.x = aPosition.x + (aVector.x * dt);
	   rot.y = aPosition.y + (aVector.y * dt);
	   rot.z = aPosition.z + (aVector.z * dt);
	}
	else {
		rot = aPosition;
		pos = lPosition;
	}

	// Update velocity
	float vlen = lVector.len();
	if (!vlen || moveState == StopState)
		lVelocity.set(0,0,0);
	else {
		lVelocity = lVector;
		lVelocity *= data->speed / vlen;
		if (moveState == BackwardState)
			lVelocity.neg();
	}

	if (isGhost()) {
		setRot(rot);
		setPos(pos);
	}
	else {
		if (data->displace) {
			// Move everything out of the way
			setRot(rot);
			setPos(pos);
			bool clear = displaceObjects(mat);
		   if ((moveState != ForwardState || data->blockForward) &&
					(moveState != BackwardState || data->blockBackward) &&
		   		!clear && !force) {
				if (!blocked)
					setMaskBits(StateMask);
				blocked = true;
				setRot(oldRot);
				setPos(oldPos);
				return false;
			}
			if (blocked)
				setMaskBits(StateMask);
			blocked = false;
		}
		else {
			if (!force) {
				// Don't move if there is something in the way
				TMat3F mat;
				MovementInfo info;
				mat.set(EulerF(rot.x,rot.y,rot.z),pos);
				info.image = getSelfImage();
				if ((moveState == ForwardState && !data->blockForward) ||
						(moveState == BackwardState && !data->blockBackward) ||
						testPosition(mat,&info)) {
					setRot(rot);
					setPos(pos);
					if (blocked)
						setMaskBits(StateMask);
					blocked = false;
				}
				else {
					if (!blocked)
						setMaskBits(StateMask);
					blocked = true;
					return false;
				}
			}
			else {
				// No displacement and force pos, not much to do.
				setRot(rot);
				setPos(pos);
			}
		}
	}
	currentPos = position;
   return true;
}


//----------------------------------------------------------------------------
bool Moveable::setWaypoint(float waypoint)
{
//	if (waypoint < 0 || waypoint > pathCount - 1)
//		return false;
	position.waypoint = waypoint;
	position.time = 0; // pathTime - position.waypoint;
	setMaskBits(PositionMask);
//	updatePosition(true);
	return true;
}

bool Moveable::moveToWaypoint(int waypoint)
{
	if (waypoint < 0 || waypoint > pathCount - 1)
		return false;
	waypointStop = waypoint;
	if (waypoint > position.waypoint)
		setMoveState(ForwardState);
	else
		if (waypoint < position.waypoint || position.time > 0)
			setMoveState(BackwardState);
		else
			// Already there!
			setMoveState(StopState);
	return true;
}

void Moveable::moveForward()
{
	if (isActive() && getState() == Enabled) {
		waypointStop = UndefinedWaypoint;
		setMoveState(ForwardState);
	}
}

void Moveable::moveBackward()
{
	if (isActive() && getState() == Enabled) {
		waypointStop = UndefinedWaypoint;
		setMoveState(BackwardState);
	}
}

void Moveable::stop()
{
	if (isActive() && getState() == Enabled) {
		waypointStop = UndefinedWaypoint;
		setMoveState(StopState);
	}
}

const char* Moveable::getMoveStateName()
{
	switch(moveState) {
		case Undefined:
			return "Undefined";
		case ForwardState:
			return "MoveForward";
		case BackwardState:
			return "MoveBackward";
		case StopState:
			return "Stop";
	}
	return "";
}	

float Moveable::getWaypointPosition()
{
	return position.waypoint + position.time / pathTime;
}


//----------------------------------------------------------------------------

void Moveable::onFirst()
{
   if (const char *script = scriptName("onFirst"))
      Console->executef(2, script, scriptThis());
}

void Moveable::onLast()
{
   if (const char *script = scriptName("onLast"))
      Console->executef(2, script, scriptThis());
}

void Moveable::onWaypoint()
{
   if (const char *script = scriptName("onWaypoint")) {
		char buff[16];
		sprintf(buff,"%d",position.waypoint);
      Console->executef(4, script, scriptThis(), buff);
   }
}

bool Moveable::processBlocker(SimObject* obj)
{
	if (GameBase* go = dynamic_cast<GameBase*>(obj)) {
		if (go->getType().test(CorpseObjectType | ItemObjectType)) {
			// Corpses & items don't block
			go->clearRestFlag();
			return false;
		}
	   if (const char *script = scriptName("onBlocker")) {
			char buff[200];
			strcpy(buff,scriptThis());
	      Console->executef(3, script, buff, go->scriptThis());
	   }
		return true;
	}
	return false;
}

void Moveable::onNewPath()
{
   if (const char *script = scriptName("onNewPath"))
      Console->executef(2, script, scriptThis());
}


//----------------------------------------------------------------------------

DWORD Moveable::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
	int pmask = mask & ~(PositionMask | RotationMask | VelocityMask);
   Parent::packUpdate(gm,pmask,stream);

	if (mask & InitialUpdate) {
	   packDatFile(stream);
		// Don't send the rest if there's no path defined
		if (pathId == UndefinedPathId)
			mask &= ~(PathMask | PositionMask | StateMask);
	}

	if (stream->writeFlag(mask & PathMask)) {
      stream->writeInt(pathId,8);
   }
	if (stream->writeFlag(mask & PositionMask)) {
      stream->writeInt(position.waypoint,WaypointBits);
      stream->write(position.time);
	}
   if (stream->writeFlag(mask & StateMask)) {
		stream->writeFlag(blocked);
		stream->writeInt(waypointStop,WaypointBits);
		stream->writeInt(moveState,2);
   }
   return 0;
}

void Moveable::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
   Parent::unpackUpdate(gm,stream);

	if (!manager)
	   unpackDatFile(stream);

	if (stream->readFlag()) {
		pathId = stream->readInt(PathIdBits);
   }
	if (stream->readFlag()) {
      position.waypoint = stream->readInt(WaypointBits);
      stream->read(&position.time);
   }
   if (stream->readFlag()) {
		blocked = stream->readFlag();
      waypointStop = stream->readInt(WaypointBits);
		State newState = (State)stream->readInt(2);
		if (manager)
			setMoveState(newState);
		else
			moveState = newState;
	}
}

//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG( Moveable, FearMoveablePersTag);

Persistent::Base::Error Moveable::read(StreamIO &sio, int iVer, int iUsr)
{
   Parent::read(sio, iVer, iUsr);

   return ((sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

Persistent::Base::Error Moveable::write(StreamIO &sio, int iVer, int iUsr)
{
   Parent::write(sio, iVer, iUsr);

   return ((sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

