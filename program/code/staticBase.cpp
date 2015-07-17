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

#include <ts.h>
#include <sim.h>
#include <simResource.h>
#include <simNetObject.h>
#include <console.h>
#include <m_random.h>

#include "staticBase.h"
#include "dataBlockManager.h"
#include "fearDcl.h"
#include "fearGlobals.h"
#include "tsfx.h"

#include "fear.strings.h"
#include "sfx.strings.h"


//----------------------------------------------------------------------------

static const float DeleteTimeout = 0.5;

static char* DamageScriptName[] =
{
	"onNone",
	"onEnabled",
	"onDisabled",
	"onDestroyed",
};

float StaticBase::prefShapeDetail = 1.0f;

static Random sg_randGen;

// Objects for which we invoke the script collision callback
static int ScriptCollisionMask = 
			SimPlayerObjectType | 
			VehicleObjectType |
			MineObjectType |
			ItemObjectType;


//----------------------------------------------------------------------------

StaticBase::StaticBaseData::StaticBaseData()
{
	sfxAmbient = -1;
	damageLevel.disabled = 0.5;
	damageLevel.destroyed = 0.75;
	maxDamage = 1.0;
   isTranslucent = false;
	outerRadius = 0;
   disableCollision = false;
}

int StaticBase::getDamageSkinLevel(int detailLevel)
{
   if(detailLevel == 1 && getDamageLevel() > data->damageLevel.disabled)
      return 2;
   else if(detailLevel == 2)
   {
      if(getDamageLevel() > data->damageLevel.destroyed)
         return 3;
      else if(getDamageLevel() > data->damageLevel.disabled)
         return 2;
      else if(getDamageLevel() > data->damageLevel.disabled * 0.5)
         return 1;
   }
   return 0;
}

void StaticBase::StaticBaseData::pack(BitStream* stream)
{
	Parent::pack(stream);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxAmbient);
   stream->writeFlag(isTranslucent);
   stream->writeFlag(disableCollision);
}	

void StaticBase::StaticBaseData::unpack(BitStream* stream)
{
	Parent::unpack(stream);
	sfxAmbient = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
   isTranslucent = stream->readFlag();
   disableCollision = stream->readFlag();
}


//----------------------------------------------------------------------------

StaticBase::StaticBase()
 : m_isTarget(false)
{
	type = StaticObjectType;
   netFlags.set( Ghostable );
	data = 0;
	
	forceMask = 0;
	collisionMask = 0;

	ambientThread = NULL;
	damageThread = NULL;
	
	ambientSound = 0;
	state = Enabled;
	active = false;
	deleteOnDestroy = false;

   SimMovement::flags.set(SimMovement::RotateBoundingBox);
}


StaticBase::~StaticBase()
{
}


//----------------------------------------------------------------------------

bool StaticBase::processArguments(int argc, const char **argv)
{
   if (argc != 1 && argc != 2) {
     	Console->printf("StaticBase: <dataFile> [deleteOnDestroy]");
      return false;
   }
   if (!Parent::processArguments(1, argv))
      return false;
	if (argc == 2)
		deleteOnDestroy = CMDConsole::getBool(argv[1]);
   return true;
}

bool StaticBase::initResources(GameBase::GameBaseData *dat)
{
	data = dynamic_cast<StaticBaseData *>(dat);
	if (!Parent::initResources(dat))
		return false;

	initCollisionInfo();

	if (getTSShape().lookupName ("ambient") != -1) {
		ambientThread = Parent::createThread (0);
		ambientThread->SetSequence ("ambient");
      float startTime = ambientThread->getSequence().fDuration * sg_randGen.getFloat();
      ambientThread->SetPosition(startTime);
	}
	else
		ambientThread = 0;

	if (getTSShape().lookupName ("visibility") != -1) {
		damageThread = Parent::createThread (0);
		damageThread->SetSequence ("visibility");
	}
	else
		damageThread = 0;

	//
	DamageState newState = state;
	state = None;
	setState(newState,false);
	return true;
}


//--------------------------------------------------------------------------- 

void StaticBase::initCollisionInfo (void)
{
	collisionMask = 
		SimPlayerObjectType | ItemObjectType | StaticObjectType |
		VehicleObjectType | MineObjectType;

	const TSShape& shape = image.shape->getShape();
	boundingBox = shape.fBounds;
	collisionImage.bbox = boundingBox;
	collisionImage.sphere.center = shape.fCenter;
	collisionImage.sphere.radius = shape.fRadius;
   collisionImage.shapeInst = image.shape;
	collisionImage.collisionDetail = 0;

	int cd = shape.findNode ("collision");
	for (int i =  0; i < shape.fDetails.size(); i++)
		if (shape.fDetails[i].fRootNodeIndex == cd)
			collisionImage.collisionDetail = i;

	if (data->outerRadius) {
		outerCollisionImage.center = image.shape->getShape().fCenter;
		outerCollisionImage.radius = data->outerRadius;
		outerCollisionImage.trigger = true;
		boundingBox.fMin.x = min(boundingBox.fMin.x,-data->outerRadius);
		boundingBox.fMin.y = min(boundingBox.fMin.y,-data->outerRadius);
		boundingBox.fMin.z = min(boundingBox.fMin.z,-data->outerRadius);
		boundingBox.fMax.x = max(boundingBox.fMax.x,data->outerRadius);
		boundingBox.fMax.y = max(boundingBox.fMax.y,data->outerRadius);
		boundingBox.fMax.z = max(boundingBox.fMax.z,data->outerRadius);
	}
	else
		outerCollisionImage.trigger = false;

   // Notify the containable parent of the new box (in world space)
   Box3F worldBox;
   m_mul(boundingBox, getTransform(), &worldBox);
   setBoundingBox(worldBox);
}

void StaticBase::onCollisionNotify(GameBase* object)
{
	if (!isGhost() && object->getType() & ScriptCollisionMask)
		scriptOnCollision(object);
}

//---------------------------------------------------------------------------

void StaticBase::updateState()
{
	// Called on the server to update the state.
	if (damageLevel >= data->maxDamage * data->damageLevel.destroyed) {
		if (damageLevel >= data->maxDamage)
			damageLevel = data->maxDamage;
		setState(Destroyed);
	}
	else
		if (damageLevel >= data->maxDamage * data->damageLevel.disabled)
			setState(Disabled);
		else
			setState(Enabled);
}

void StaticBase::setState(DamageState newState,bool script)
{
	if (!manager) {
		state = newState;
		return;
	}
	if (state != newState) {
		DamageState oldState = state;
		state = newState;
		switch(state) {
			case Enabled:
				unBlowUp();
				if (damageThread) {
					damageThread->SetPosition (0.0f);
					image.shape->animate();
				}
				startAmbientSound();
				break;
			case Disabled:
				active = false;
				stopAmbientSound();
				unBlowUp();
				if (damageThread) {
					damageThread->SetPosition (0.0f);
					image.shape->animate();
				}
				break;
			case Destroyed:
				active = false;
				damageLevel = data->maxDamage;
				stopAmbientSound();
				if (isGhost() && oldState != None)
					blowUp();
				if (!isGhost() && deleteOnDestroy)
				   SimMessageEvent::post(this, 
				   	manager->getCurrentTime() + DeleteTimeout, 1);
				if (damageThread) {
					damageThread->SetPosition (1.0f);
					image.shape->animate();
				}
				break;
		}

		setMaskBits(DamageMask);
		if (script)
			onDamageStateChange(oldState);
	}
}

const char* StaticBase::getDamageState()
{
	if (state == Enabled)
		return "Enabled";
	if (state == Disabled)
		return "Disabled";
	return "Destroyed";
}


//----------------------------------------------------------------------------

void StaticBase::setPos (const Point3F &pos)
{
	Parent::setPos(pos);
	if (ambientSound) {
		// Hack for now...
		stopAmbientSound();
		startAmbientSound();
	}
}


//----------------------------------------------------------------------------

bool StaticBase::isActive()
{
	return active;
}

bool StaticBase::setActive(bool a)
{
	if (a != active && state == Enabled) {
		active = a;
		if (active)
			onActivate();
		else
			onDeactivate();
	}
	return active;
}

void StaticBase::onActivate()
{
	if (!isGhost() && data && state == Enabled)
		if (const char* script = scriptName("onActivate"))
			Console->executef(2,script,scriptThis());
}

void StaticBase::onDeactivate()
{
	if (!isGhost() && data && state == Enabled)
		if (const char* script = scriptName("onDeactivate"))
			Console->executef(2,script,scriptThis());
}

bool StaticBase::getCommandStatus(CommandStatus* status)
{
	// Damage level is set to 0, 1 or 2
	status->damageLevel = state - 1;
	status->active = active;
	return true;
}


//---------------------------------------------------------------------------

void StaticBase::startAmbientSound()
{
	if (!ambientSound && isGhost() && data && data->sfxAmbient != -1) {
      ambientSound = TSFX::PlayAt(data->sfxAmbient, getTransform(), Point3F(0, 0, 0));
	}
}

void StaticBase::stopAmbientSound()
{
	if (ambientSound) {
		Sfx::Manager::Stop(manager, ambientSound);
		ambientSound = 0;
	}
}


//--------------------------------------------------------------------------- 

void StaticBase::serverProcess (DWORD)
{
	updateTimers();
	updateScriptThreads(0.03);
	updateEnergy(0.03);
	updateDamage(0.03);
	updateState();
}

void StaticBase::clientProcess (DWORD curTime)
{
   float animTimeDelta = getAnimateDelta(curTime);
   lastAnimateTime = curTime;

	if (ambientThread && state == Enabled)
		ambientThread->AdvanceTime(animTimeDelta);

	updateShieldThread(animTimeDelta);
	updateScriptThreads(animTimeDelta);
}


//----------------------------------------------------------------------------

bool StaticBase::onAdd()
{
   if (!Parent::onAdd())
      return false;

	addToSet(PlayerSetId);

   if (isTarget() == true)
      addToSet(TargetableSetId);

	if (isGhost())
	   lastAnimateTime = cg.currentTime;
	return true;
}

void StaticBase::onRemove ()
{
	stopAmbientSound();
	Parent::onRemove();
}


//----------------------------------------------------------------------------

void StaticBase::scriptOnAdd()
{
	Parent::scriptOnAdd();
	onDamageStateChange(None);
}


//----------------------------------------------------------------------------

void StaticBase::setDamageLevel (float level)
{
	Parent::setDamageLevel(level);
	updateState();
}

void StaticBase::onDamageStateChange(DamageState oldState)
{
	oldState;
	if (!isGhost())
		if (const char* script = scriptName(DamageScriptName[state]))
			Console->executef(2,script,scriptThis());
}

void StaticBase::onPowerStateChange(GameBase* generator)
{
	if (!isGhost() && data && state == Enabled)
		if (const char* script = scriptName("onPower"))
			Console->evaluatef("%s(%s,%s,%d);",script,scriptThis(),
				isPowered()? "True": "False", generator->getId());
}


//----------------------------------------------------------------------------

bool StaticBase::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{
	Parent::processEvent(event);

	if (!isGhost())
		setMaskBits(PositionMask | RotationMask);
	return true;
}

bool StaticBase::processEvent(const SimEvent* event)
{
	switch (event->type) {
		case SimMessageEventType: {
			deleteObject();
			return true;
		}
		onEvent (SimObjectTransformEvent);
		default:
			return Parent::processEvent (event);
	}
}


//---------------------------------------------------------------------------

void StaticBase::setDetailScale( void )
{
   image.shape->setDetailScale( prefShapeDetail );
}


bool StaticBase::onSimCollisionImageQuery (SimCollisionImageQuery *query)
{

	if (data->disableCollision == false && state != Destroyed || damageThread) {
		query->count = 1;
		query->image[0] = &collisionImage;
		collisionImage.transform = getTransform();
		if (outerCollisionImage.trigger) {
			query->image[query->count++] = &outerCollisionImage;
			outerCollisionImage.transform = getTransform();
		}
		return true;
	}
	return false;
}

bool StaticBase::processQuery(SimQuery* query)
{
	switch (query->type){
		onQuery (SimCollisionImageQuery);
		onQuery(SimRenderQueryImage);
		
		default:
		 return Parent::processQuery (query);
	}
}


//---------------------------------------------------------------------------

DWORD StaticBase::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
	Parent::packUpdate(gm,mask,stream);

	if (stream->writeFlag(mask & (PositionMask | RotationMask))) {
		stream->write(sizeof( Point3F ), &getPos());
		stream->write(sizeof( Point3F ), &getRot());
	}

	if (stream->writeFlag(mask & DamageMask)) {
		if (!stream->writeFlag(state == Enabled))
			stream->writeFlag(state == Disabled);
		float percentage = damageLevel / data->maxDamage;
		stream->writeFloat(percentage,DamageLevelBits);
	}

	if (stream->writeFlag(mask & InfoMask)) {
      stream->writeFlag(m_isTarget);
   }

   return 0;
}

void StaticBase::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
	Parent::unpackUpdate(gm,stream);

	if (stream->readFlag()) {
		Point3F pos,rot;
		stream->read(sizeof(pos), &pos);
		stream->read(sizeof(rot), &rot);
		setRot(rot);
		setPos(pos);
	}

	if (stream->readFlag()) {
		DamageState newState;
		if (stream->readFlag())
			newState = Enabled;
		else
			if (stream->readFlag())
				newState = Disabled;
			else
				newState = Destroyed;
		damageLevel = stream->readFloat(DamageLevelBits);
      reSkin();
		setState(newState);
	}
	if (stream->readFlag()) {
      setIsTarget(stream->readFlag());
   }
}


//----------------------------------------------------------------------------

void StaticBase::initPersistFields()
{
   Parent::initPersistFields();
   addField("deleteOnDestroy", TypeBool, Offset(deleteOnDestroy, StaticBase));
}

Persistent::Base::Error StaticBase::read(StreamIO &sio, int, int)
{
	int version;
	sio.read(&version);

	Parent::read (sio, 0, 0);

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error StaticBase::write(StreamIO &sio, int, int)
{
	int version = 0;
	sio.write(version);

	Parent::write (sio, 0, 0);

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}

bool
StaticBase::setIsTarget(const bool in_isTarget)
{
   if (manager != NULL) {
      if (in_isTarget == false) {
         if (m_isTarget == true)
            removeFromSet(TargetableSetId);
      } else {
         if (m_isTarget == false)
            addToSet(TargetableSetId);
      }
   }
   setMaskBits(InfoMask);
   return (m_isTarget = in_isTarget);
}

bool
StaticBase::getTarget(Point3F* out_pTarget,
                      int*     out_pTeam)
{
   AssertFatal(isTarget() == true, "Shouldn't get this call if not a target");

   if (getState() == Enabled) {
      *out_pTarget = getBoxCenter();
      *out_pTeam   = getTeam();

      return true;
   } else {
      return false;
   }
}


//--------------------------------------------------------------------------- 
//
//--------------------------------------------------------------------------- 

//----------------------------------------------------------------------------

StaticShape::StaticShape()
{
}

StaticShape::~StaticShape()
{
}


//----------------------------------------------------------------------------

int StaticShape::getDatGroup()
{
	return DataBlockManager::StaticShapeDataType;
}

DWORD StaticShape::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
	Parent::packUpdate(gm,mask,stream);

	if (stream->writeFlag(mask & InfoMask)) {
		packDatFile(stream);
      if(stream->writeFlag(sensorKey != -1))
         stream->writeInt(sensorKey, 7);
	}
	return 0;
}

void StaticShape::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
	Parent::unpackUpdate(gm,stream);

	if (stream->readFlag()) {
		unpackDatFile(stream);
      if(stream->readFlag())
         sensorKey = stream->readInt(7);
	}
}	



//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAGS( StaticShape, FOURCC('S','S','H','P'), StaticShapePersTag );

Persistent::Base::Error StaticShape::read(StreamIO & sio, int ver, int user)
{
	int version;
	sio.read(&version);

	Parent::read(sio,ver,user);

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error StaticShape::write(StreamIO &sio, int ver, int user)
{
	int version = 0;
	sio.write(version);

	Parent::write(sio,ver,user);

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}

bool
StaticBase::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   setDetailScale ();
   if (Parent::processQuery(query) == true) {
      if (data->isTranslucent) {
         image.itype = SimRenderImage::Translucent;
         TS::Camera* pCamera = query->renderContext->getCamera();
   
         const TMat3F& rTWC = pCamera->getTWC();
         float cd = rTWC.p.y + image.transform.p.x * rTWC.m[0][1] +
                               image.transform.p.y * rTWC.m[1][1] +
                               image.transform.p.z * rTWC.m[2][1];
         float nd = pCamera->getNearDist();
         image.sortValue = (cd <= nd) ? 0.99f : nd / cd;
      }

      return true;
   }
   return false;
}


bool StaticShape::processQuery(SimQuery* query)
{
	switch (query->type){
//      onQuery(SimRenderQueryImage);

	  default:
	   return Parent::processQuery (query);
	}
}
