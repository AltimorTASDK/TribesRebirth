//--------------------------------------------------------------------------- 

//  Done:
//		Contact mask
//		Forward on contact
//		Contact start Delay 
//		Timeout on for AutoBackward
//		Time scale for motion
//		Sound { start, moving, stop }
//		Rotation
//		Respond to triggers
//		Persistance

//	 Missing:
//		Stop at each waypoint flag
//		Trigger other objects on state change
//		Displacement of objects (elevator)
//		Damage for failed displacement
//		Don't load shape resource on server

//	 Notes:
//		Currently does no collision on movment


//--------------------------------------------------------------------------- 

#ifdef __BORLANDC__
#pragma warn -inl
#pragma warn -aus
#endif

#include "simMovingShape.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include "simWaypoint.h"
#include "simTimerGrp.h"
#include "simGame.h"
#include "simAction.h"

//----------------------------------------------------------------------------

static char ShapeInspectString[] = "IDMITR_*";


//--------------------------------------------------------------------------- 

SimMovingShape::SimMovingShape()
{
	state = Start;
	objectMask = -1;

	record.timeScale = 1.0f;
	record.forwardDelay = 0.0f;
	record.backwardDelay = 0.0f;
	record.startSoundId = 0;
	record.stopSoundId = 0;
	record.runningSoundId = 0;
	record.collisionDamage = 0;

	lVector.set(0.0f,0.0f,0.0f);
	setTimeSlice(1.0f/30.0f);
}

SimMovingShape::~SimMovingShape()
{
}


//--------------------------------------------------------------------------- 

bool SimMovingShape::setMoveState(SimMovingShape::State newState)
{
	switch (newState) {
		case Start:
		case End:
			// Cant set these states externally
			return false;
		case Stop:
			removeFromSet(SimTimerSetId);
			state = newState;
			break;
		case Forward: {
			if (state != End) {
				if (state != Forward && state != Backward)
					addToSet(SimTimerSetId);
				if (state == Start) {
					if (!buildMovementVector(0)) {
						state = Start;
						return false;
					}
					//
					wayPoint = 0;
					time = 0.0f;
				}
				state = newState;
			}
			break;
		}
		case Backward: {
			if (state != Start) {
				if (state != Forward && state != Backward)
					addToSet(SimTimerSetId);
				state = newState;
			}
			break;
		}
	}
	return true;
}


//----------------------------------------------------------------------------

void SimMovingShape::updateMoveState(SimTime dt)
{
	float delta = dt * record.timeScale;
	float startTime = time;
	switch (state) {
		case Forward: {
			if ((time += delta) > 1.0f) {
				time -= 1.0f;
				++wayPoint;
				if (!buildMovementVector(wayPoint))  {
					removeFromSet(SimTimerSetId);
					state = End;
					lPosition += lVector;
					aPosition += aVector;
					lVector.set(0.0f,0.0f,0.0f);
					aVector.set(0.0f,0.0f,0.0f);
					time = 0.0f;
					if (record.flags.test(Record::AutoBackward))
						SimMessageEvent::post(this,
							float(manager->getCurrentTime()) + record.backwardDelay,
							Backward);
				}
			}
			if (!updateMovement())
				time = startTime;
			break;
		}
		case Backward: {
			if ((time -= delta) < 0.0f) {
				time += 1.0f;
				--wayPoint;
				if (!buildMovementVector(wayPoint)) {
					removeFromSet(SimTimerSetId);
					time = 0.0f;
					state = Start;
					if (record.flags.test(Record::AutoForward))
						SimMessageEvent::post(this,
							float(manager->getCurrentTime()) + record.forwardDelay,
							Forward);
				}
			}
			if (!updateMovement())
				time = startTime;
			break;
		}
	}
}


//----------------------------------------------------------------------------

bool SimMovingShape::updateMovement()
{
	Point3F pos;
	pos.x = lPosition.x + lVector.x * time;
	pos.y = lPosition.y + lVector.y * time;
	pos.z = lPosition.z + lVector.z * time;

	EulerF angle;
	angle.x = aPosition.x + aVector.x * time;
	angle.y = aPosition.y + aVector.y * time;
	angle.z = aPosition.z + aVector.z * time;

	// Set base class
	TMat3F mat;
	mat.set(angle,pos);
	TMat3F omat = getTransform();

	set(angle, lPosition);
	if (!displaceObjects(omat)) {
      set(omat, true);
		return false;
	}

   setMaskBits(PosRotMask);
	return true;
}


//----------------------------------------------------------------------------

bool SimMovingShape::buildMovementVector(int index)
{
	// Find the first object in our group that responds
	// to the waypoint query.
	SimWaypointQuery query;
	query.index = index;

	SimGroup& grp = *getGroup();
	for (SimGroup::iterator itr = grp.begin(); itr != grp.end(); itr++)
		if ((*itr)->processQuery(&query)) {
			if (query.index < query.count - 1) {
				lPosition = query.position;
				lVector = query.vector;
				aPosition.x = query.rotation.x;
				aPosition.y = query.rotation.y;
				aPosition.z = query.rotation.z;

				query.index++;
				(*itr)->processQuery(&query);
				aVector.x = query.rotation.x - aPosition.x;
				aVector.y = query.rotation.y - aPosition.y;
				aVector.z = query.rotation.z - aPosition.z;
				return true;
			}
		}

	return false;
}


//--------------------------------------------------------------------------- 

bool SimMovingShape::processEvent(const SimEvent* event)
{
	switch (event->type) {
		onEvent(SimMovementCollisionEvent);
		onEvent(SimMessageEvent);
		onEvent(SimTimerEvent);
      onEvent(SimTriggerEvent);
		//
		case SimActionEventType: {
			const SimActionEvent* ep = 
				static_cast<const SimActionEvent*>(event);
			switch(ep->action) {
				case MoveForward:
					setMoveState(Forward);
					break;
				case MoveBackward:
					setMoveState(Backward);
					break;
			}
		}
	}
	return Parent::processEvent(event);
}


//--------------------------------------------------------------------------- 

bool SimMovingShape::onAdd()
{
	if (!Parent::onAdd())
		return false;

	switch (state)	{
		case Start:
			if (record.flags.test(Record::AutoForward))
				SimMessageEvent::post(this,
					float(manager->getCurrentTime()) + record.forwardDelay,
					Forward);
			break;
		case End:
			if (record.flags.test(Record::AutoBackward))
				SimMessageEvent::post(this,
					float(manager->getCurrentTime()) + record.backwardDelay,
					Backward);
			break;
		case Forward:
		case Backward:
			addToSet(SimTimerSetId);
			break;
	}

	return true;
}


//----------------------------------------------------------------------------

bool SimMovingShape::onSimMovementCollisionEvent(const SimMovementCollisionEvent* event)
{
	if (record.flags.test(Record::ForwardOnCollision) &&
			state != Forward && state != End) {
		SimObject* obj = event->getObject(manager);
		if (obj && obj->getType() & objectMask) {
			// Only use the forward delay if we are not moving
			// (or there is none).
			if (state == Backward || !record.forwardDelay)
				setMoveState(Forward);
			else
				SimMessageEvent::post(this,
					float(manager->getCurrentTime()) + record.forwardDelay,
					Forward);
		}
	}
	return true;
}	


//----------------------------------------------------------------------------

bool SimMovingShape::onSimMessageEvent(const SimMessageEvent* event)
{
	setMoveState(State(event->message));
	return Parent::onSimMessageEvent(event);
}


//----------------------------------------------------------------------------

bool SimMovingShape::onSimTimerEvent(const SimTimerEvent* event)
{
	if (event->timerId == SimTimerSetId) {
		updateMoveState(event->timerInterval);
		return true;
	}

	// We happen to know the base class process timer events:
	return true;
}


//----------------------------------------------------------------------------

bool SimMovingShape::onSimTriggerEvent(const SimTriggerEvent* event)
{
	switch(event->action) {
      case SimTriggerEvent::Activate:
         setMoveState(Forward);
         break;
      case SimTriggerEvent::Deactivate:
         setMoveState(Backward);
         break;
      default:
         return false;
   }
	return true;
}


//--------------------------------------------------------------------------- 

void SimMovingShape::inspectRead(Inspect *sd)
{
	Parent::inspectRead(sd);
	sd->readDivider();

	bool f1,f2,f3;
	sd->read(IDITG_MITR_COLLISIONFORWARD, f1);
	sd->read(IDITG_MITR_AUTOFORWARD, f2);
	sd->read(IDITG_MITR_AUTOBACKWARD, f3);
	record.flags.set(Record::ForwardOnCollision,f1);
	record.flags.set(Record::AutoForward,f2);
	record.flags.set(Record::AutoBackward,f3);

   sd->read(IDITG_MITR_TIMESCALE, record.timeScale);
   sd->read(IDITG_MITR_FORWARDDELAY, record.forwardDelay);
   sd->read(IDITG_MITR_BACKWARDDELAY, record.backwardDelay);

	// See if we need to kickstart
	switch (state)	{
		case Start:
			if (record.flags.test(Record::AutoForward))
				SimMessageEvent::post(this,
					float(manager->getCurrentTime()) + record.forwardDelay,
					Forward);
			break;
		case End:
			if (record.flags.test(Record::AutoBackward))
				SimMessageEvent::post(this,
					float(manager->getCurrentTime()) + record.backwardDelay,
					Backward);
			break;
	}
}   

void SimMovingShape::inspectWrite(Inspect *sd)
{
	Parent::inspectWrite(sd);
	sd->writeDivider();

	sd->write(IDITG_MITR_COLLISIONFORWARD,
		bool(record.flags.test(Record::ForwardOnCollision)));
	sd->write(IDITG_MITR_AUTOFORWARD,
		bool(record.flags.test(Record::AutoForward)));
	sd->write(IDITG_MITR_AUTOBACKWARD,
		bool(record.flags.test(Record::AutoBackward)));

   sd->write(IDITG_MITR_TIMESCALE, record.timeScale);
   sd->write(IDITG_MITR_FORWARDDELAY, record.forwardDelay);
   sd->write(IDITG_MITR_BACKWARDDELAY, record.backwardDelay);
}


//----------------------------------------------------------------------------

int SimMovingShape::version()
{
	return 2;
}

Persistent::Base::Error SimMovingShape::read(StreamIO &s, int ver, int b)
{
#if 1
	if (CMDConsole::getLocked()->getBoolVariable("SimShapeOldFormat")) {
		return Parent::read(s,ver,b);
	}
#endif

	int version;
	s.read(&version);
	AssertFatal(version == 1 || version == SimMovingShape::version(),
		"SimMovingShape:: incorrect file version");
	Parent::read(s,ver,b);

	// Attributes
	s.read(&objectMask);

	s.read(sizeof(record.flags),&record.flags);
	s.read(&record.timeScale);
	if (version == 1) {
		float tmp;
		s.read(&tmp);
	}
	s.read(&record.forwardDelay);
	s.read(&record.backwardDelay);
	s.read(&record.startSoundId);
	s.read(&record.stopSoundId);
	s.read(&record.runningSoundId);
	s.read(&record.collisionMask);
	s.read(&record.collisionDamage);

	// Current state
	UInt8 st; s.read(&st); state = State(st);
	s.read(&time);
	s.read(&wayPoint);
	if (state == Forward || state == Backward) {
		s.read(sizeof(lPosition),&lPosition);
		s.read(sizeof(lVector),&lVector);
		s.read(sizeof(aPosition),&aPosition);
		s.read(sizeof(aVector),&aVector);
	}
	return (s.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error SimMovingShape::write(StreamIO &s, int ver, int b)
{
	s.write(SimMovingShape::version());
	Parent::write(s,ver,b);

	// Attributes
	s.write(objectMask);
	s.write(sizeof(record.flags),&record.flags);
	s.write(record.timeScale);
	s.write(record.forwardDelay);
	s.write(record.backwardDelay);
	s.write(record.startSoundId);
	s.write(record.stopSoundId);
	s.write(record.runningSoundId);
	s.write(record.collisionMask);
	s.write(record.collisionDamage);

	// Current state
	s.write(UInt8(state));
	s.write(time);
	s.write(wayPoint);
	if (state == Forward || state == Backward) {
		s.write(sizeof(lPosition),&lPosition);
		s.write(sizeof(lVector),&lVector);
		s.write(sizeof(aPosition),&aPosition);
		s.write(sizeof(aVector),&aVector);
	}
	return (s.getStatus() == STRM_OK)? Ok: WriteError;
}

