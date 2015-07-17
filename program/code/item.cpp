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
#include "g_bitmap.h"
#include "Item.h"
#include "fearDcl.h"
#include "DataBlockManager.h"
#include "fearGlobals.h"
#include "netGhostManager.h"
#include "playerManager.h"

#include "fear.strings.h"
#include "sfx.strings.h"
#include "stringTable.h"

//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAGS( Item, FOURCC('I','T','E','M'), ItemPersTag );

static const float ItemRotationSpeed = 3.0f;
static const float ItemUpdatePosTime = 1.0f;
static const float ItemThrowBlackout = 1.0f;


//----------------------------------------------------------------------------

// Objects we normally collide with
static int ServerCollisionMask = 
			SimPlayerObjectType | 
			VehicleObjectType |
      	SimTerrainObjectType |
      	SimInteriorObjectType |
      	StaticObjectType |
      	MoveableObjectType;

static int ClientCollisionMask = 
			SimPlayerObjectType | 
			VehicleObjectType | 
      	SimTerrainObjectType |
      	SimInteriorObjectType |
      	StaticObjectType |
      	MoveableObjectType;

// Objects for which we invoke the script collision callback
static int ScriptCollisionMask = 
			VehicleObjectType | SimPlayerObjectType | 
      	StaticObjectType | MineObjectType;


//----------------------------------------------------------------------------
// Item data defaults
//----------------------------------------------------------------------------

Item::ItemData::ItemData()
{
	imageId = -1;
	price = 0;
	//
	mass = 1.0f;
	drag = 1.0f;
	density = 1.0f;
	friction = 1.0f;
	elasticity = 0.2f;
	//
	lightType = Item::NoLight;
	lightRadius = 3;
	lightTime = 1.5;
	lightColor.set(1,1,1);
	//
	description = stringTable.insert("Item", true);
   typeString = stringTable.insert("z----"); // make it sort by default last
	hudIcon = 0;
	showInventory = true;
	showWeaponBar = false;
	hiliteOnActive = false;
}


void Item::ItemData::pack(BitStream *stream)
{
	Parent::pack(stream);

	stream->write(imageId);
	stream->write(price);

	stream->write(mass);
	stream->write(drag);
	stream->write(density);
	stream->write(friction);
	stream->write(elasticity);

   stream->writeString(hudIcon);
	stream->writeFlag(showInventory);
	stream->writeFlag(showWeaponBar);
	stream->writeFlag(hiliteOnActive);
   stream->writeString(typeString);

	stream->write(lightType);
	stream->write(lightRadius);
	stream->write(lightTime);
	stream->write(sizeof(lightColor),&lightColor);
}

void Item::ItemData::unpack(BitStream *stream)
{
	Parent::unpack(stream);

	stream->read(&imageId);
	stream->read(&price);

	stream->read(&mass);
	stream->read(&drag);
	stream->read(&density);
	stream->read(&friction);
	stream->read(&elasticity);

	char buf[256];
   stream->readString(buf);
   hudIcon = stringTable.insert(buf);
	showInventory = stream->readFlag();
	showWeaponBar = stream->readFlag();
	hiliteOnActive = stream->readFlag();
   typeString = stream->readSTString(true);

	stream->read(&lightType);
	stream->read(&lightRadius);
	stream->read(&lightTime);
	stream->read(sizeof(lightColor),&lightColor);
}

bool Item::ItemData::preload(ResourceManager *rm, bool server, char errorBuffer[256])
{
   if(hudIcon && hudIcon[0])
   {
      char iconBmpName[256];
      sprintf(iconBmpName, "lr_%s_on.bmp", hudIcon);
      hudLROnBitmap = rm->load(iconBmpName);
      if(bool(hudLROnBitmap))
         hudLROnBitmap->attribute |= BMA_TRANSPARENT;
            
      sprintf(iconBmpName, "lr_%s_off.bmp", hudIcon);
      hudLROffBitmap = rm->load(iconBmpName);
      if(bool(hudLROffBitmap))
         hudLROffBitmap->attribute |= BMA_TRANSPARENT;
      
      sprintf(iconBmpName, "I_%s_on.bmp", hudIcon);
      hudIOnBitmap = rm->load(iconBmpName);
      if(bool(hudIOnBitmap))
         hudIOnBitmap->attribute |= BMA_TRANSPARENT;
      
      sprintf(iconBmpName, "I_%s_off.bmp", hudIcon);
      hudIOffBitmap = rm->load(iconBmpName);
      if(bool(hudIOffBitmap))
         hudIOffBitmap->attribute |= BMA_TRANSPARENT;
   }
   return Parent::preload(rm, server, errorBuffer);
}

int Item::getDatGroup()
{
   return DataBlockManager::ItemDataType;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

Item::Item()
{
	type = ItemObjectType;
   netFlags.set( Ghostable );
	itemFlags.set (Visible);
   isRotate = false;
	isCollideable = false;
	SimMovement::flags.set (UseElasticity | UseFriction | 
		UseCurrent | UseDrag );

	forceMask = -1;
	throwSource = 0;
	count = 0;
	light = 0;
	lastRest = true;
	updatePosTime = 0;

   throwerClientId      = -1;
   throwerClientLoginId = -1;
}

Item::~Item()
{
	delete light;
}


//----------------------------------------------------------------------------

bool Item::processArguments(int argc, const char **argv)
{
   if (argc < 3 || argc > 5) {
     	Console->printf("Item: <dataFile> <count> <bool pop> [bool rotate] [bool collide]");
      return false;
   }
   if(!Parent::processArguments(1, argv))
      return false;

	count = atoi(argv[1]);

	if (CMDConsole::getBool(argv[2]))
		itemFlags.set(PopItem);

	if (argc > 3 && CMDConsole::getBool(argv[3]))
		isRotate = true;

	if (argc > 4 && CMDConsole::getBool(argv[4])) {
		collisionImage.trigger = false;
		isCollideable = true;
		deleteOnDestroy = true;
	}
   return true;
}


//----------------------------------------------------------------------------

void Item::pop()
{
	// Does not pop the item right away, only marks it.
	itemFlags.set(PopItem);
}

bool Item::hideItem(bool hide)
{
	if (manager) {
		if (hide) {
			if (SimContainer* container = getContainer())
				container->removeObject (this);
		}
		else {
			if (!getContainer()) {
				if (SimContainer* root = findObject(manager,SimRootContainerId,root))
					root->addObject(this);
			}
         if (getLinearVelocity().x == 0 &&
             getLinearVelocity().y == 0 &&
             getLinearVelocity().z == 0)
            SimMovement::flags.set(AtRest);
         m_lastSunUpdated = 0;
      }
	}
	bool hidden = !isVisible();
	itemFlags.set(Visible,!hide);
	return hidden;
}


//---------------------------------------------------------------------------

void Item::onDeleteNotify(SimObject *object)
{
   if(object == throwSource)
      throwSource = NULL;
   Parent::onDeleteNotify(object);
}

void Item::throwObject(GameBase* object,float speed,bool careless)
{
	static Random random;
	Point3F pos,vel;

   if(throwSource)
      clearNotify(throwSource);
      
	throwSource = object;
   if(throwSource)
      deleteNotify(throwSource);
      
	setOwnerId(object->getId());

   throwerClientId = object->getControlClient();
   if (throwerClientId != 0) {
      PlayerManager* pPM = PlayerManager::get(manager);
      AssertFatal(pPM != NULL, "No Player manager?");
      PlayerManager::BaseRep* pCR = pPM->findBaseRep(throwerClientId);
      AssertFatal(pCR != NULL, avar("No client rep for client %d?", throwerClientId));
      throwerClientLoginId = pCR->loginId;
   } else {
      throwerClientLoginId  = -1;
   }

	if (careless) {
		// Random up vector, no point in throwing directly at
		// the ground.
		pos = object->getBoxCenter();
		vel.set(random.getFloat (-0.733, 0.733),
			random.getFloat (-0.733, 0.733),
			random.getFloat (0.5, 1.0));
	}
	else
		object->getThrowVector(&pos,&vel);

   ItemData* datptr = static_cast<ItemData *>(getDatPtr());
	vel *= speed / datptr->mass;
	vel += object->getLinearVelocity();
	setLinearVelocity(vel);

	setRot(Point3F(0.0f,0.0f,random.getFloat(0.0f,(float)M_2PI)));
	setPos(pos);
	setMaskBits(PositionMask | RotationMask | VelocityMask);
	clearRestFlag();
	dropTime = manager->getCurrentTime();
}	


//---------------------------------------------------------------------------

bool Item::initResources(GameBase::GameBaseData *dat)
{
   if(!Parent::initResources(dat))
      return false;

	collisionImage.trigger = !isCollideable;
   collisionImage.collisionLevel = ShapeCollisionImage::CollideBBox;

   ItemData* datptr = static_cast<ItemData *>(getDatPtr());

	if (isGhost()) {
		collisionMask = ClientCollisionMask;
		if (datptr->lightType != NoLight)
			addToSet(SimLightSetId);
	}
	else {
		collisionMask = ServerCollisionMask;
	}

	// Init movement coefficients.
	setMass(datptr->mass);
	setDrag(datptr->drag);
	setDensity(datptr->density);
	setFriction(datptr->friction);
	setElasticity(datptr->elasticity);
	return true;
}


//--------------------------------------------------------------------------- 

void Item::serverProcess (DWORD ct)
{
	if (itemFlags.test(Visible)) {
		Parent::serverProcess(ct);
	}
	updateBlackout();

	if (!isRotate && (getState() != Destroyed || hasHulk())) {
		bool stateChange = lastRest != isAtRest();
		lastRest = isAtRest();

		// Update the client when our state changes, or every
		// ItemUpdatePosTime as long as we're not at rest.
		if (!isAtRest()) {
			Parent::updateMovement(0.032f);
			if (updatePosTime < manager->getCurrentTime()) {
				updatePosTime = manager->getCurrentTime() + ItemUpdatePosTime;
				setMaskBits(PositionMask);
			}
		}
		else
			if (stateChange) {
				setMaskBits(PositionMask);
			}
	}
}

void Item::clientProcess (DWORD curTime)
{
	if (itemFlags.test(Visible)) {
		float dt = getAnimateDelta(curTime);
		Parent::clientProcess(curTime);

		if (isRotate) {
			Point3F rot = getRot();
			rot.z += ItemRotationSpeed * dt;
			setRot(rot);
			// Using setRot, the transform only gets updated when setPos
			// is called.  Since we are not moving, we need to force it.
			setPos(getPos());
         SimMovement::flags.set(AtRest);
		}
	}
	else
	   lastAnimateTime = curTime;

	updateBlackout();

	if (getState() != Destroyed || hasHulk())
		if (!isRotate && !isAtRest())
			Parent::updateMovement(0.001f * (curTime - lastProcessTime));
	lastProcessTime = curTime;
}

void Item::updateBlackout()
{
	// Throw blackout
	if (throwSource && dropTime + ItemThrowBlackout < manager->getCurrentTime())
	{
      clearNotify(throwSource);
   	throwSource = 0;
   }
}	


//----------------------------------------------------------------------------

bool Item::onAdd()
{
	bool atRest = isAtRest();
	if (!Parent::onAdd())
		return false;
	//
	if (isGhost() && atRest)
		SimMovement::flags.set(AtRest);
	dropTime = manager->getCurrentTime();
	if (!itemFlags.test(Visible))
		hideItem(true);
	return true;
}

void Item::onRemove()
{
	Parent::onRemove();
}	

void Item::onCollisionNotify(GameBase* object)
{
	// Ignore the throw object if it bumps into us
	if (!isGhost() && throwSource != object && object->getType() & ScriptCollisionMask)
		scriptOnCollision(object);
}

void Item::onDisplacement(SimMovement* displacer,const Point3F& delta)
{
	displacer, delta;
	if (!isGhost()) {
		clearRestFlag();
		setMaskBits(PositionMask);
	}
}

bool Item::processCollision(SimMovementInfo* info)
{
	SimCollisionInfoList& list = info->collisionList;

	// Avoid collision with the throwSource from our
	// own movement.
	if (throwSource)
		for (int i = 0; i < list.size(); i++)
	     	if (throwSource && list[i].object == throwSource) {
				// Should only be one instance of throwSource
				// in the list.
				list[i] = list.last();
	         list.decrement();
				break;
	     	}

	// Attach ourselves to elevators.
	for (int i = 0; i < list.size(); i++)
		if (list[i].object->getType() & MoveableObjectType)
			addContact(static_cast<SimMovement*>(list[i].object));

	return Parent::processCollision(info);
}	


//---------------------------------------------------------------------------

bool Item::onSimLightQuery(SimLightQuery * query)
{
	if (TSLight* lptr = getLight()) {
		query->count = 1;
		query->light[0] = lptr;
	   query->ambientIntensity.set();
		return true;
	}
	query->count = 1;
	return false;
}	

bool Item::processQuery(SimQuery* query)
{
	switch (query->type){
		onQuery(SimLightQuery);
      onQuery(SimRenderQueryImage);

     default:
      return Parent::processQuery(query);
	}
}


//---------------------------------------------------------------------------

TSLight* Item::getLight()
{
   ItemData* datptr = static_cast<ItemData *>(getDatPtr());
	if (!datptr || datptr->lightType == NoLight)
		return 0;

	float intensity;
	switch (datptr->lightType) {
		case ConstantLight:
			intensity = 1.0;
			break;
		case PulsingLight: {
			float delta = manager->getCurrentTime() - dropTime;
			intensity = 0.5 + 0.5 * sin(M_PI * delta / datptr->lightTime);
			intensity = 0.15 + intensity * 0.85;
			break;
		}
		default:
			return 0;
	}

   if (m_fading != ShapeBase::NotFading) {
      float factor;
      if (m_fading == FadingOut) {
         factor = float(wg->currentTime - m_fadeStarted) / float(csm_fadeLength);
         // Bail if we're _really faded...
         if (factor > 1.0)
            factor = 1.0;
      } else {
         factor = 1.0f - float(wg->currentTime - m_fadeStarted) / float(csm_fadeLength);
         // Bail if we're _really faded...
         if (factor < 0.0) {
            factor = 0.0;
            m_fading = NotFading;
         }
      }

      intensity *= 1.0f - factor;
   }

	if (!light) {
		light = new TSLight;
		light->setType(TSLight::LightPoint);
		light->setRange(datptr->lightRadius);
	}

	light->setIntensity(datptr->lightColor.x * intensity,
		datptr->lightColor.y * intensity,datptr->lightColor.z * intensity);
	light->setPosition(getTransform().p);
	return light;
}


//---------------------------------------------------------------------------

DWORD Item::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
	Parent::packUpdate(gm,
		mask & ~(PositionMask | RotationMask | VelocityMask),stream);

	if (stream->writeFlag(mask & InfoMask)) {
      packDatFile(stream);
      if (stream->writeFlag(sensorKey != -1))
         stream->writeInt(sensorKey, 7);

		int ghost = throwSource? gm->getGhostIndex(throwSource): -1;
		if (stream->writeFlag(ghost != -1))
	     stream->writeInt(ghost, 10);
		stream->writeFlag(isRotate);
		stream->writeFlag(isCollideable);
	}
	if (stream->writeFlag(mask & RotationMask))
		stream->writeFloat(getRot().z / M_2PI,8);
	if (stream->writeFlag(mask & PositionMask)) {
		stream->write(sizeof( Point3F ), &getPos());
	}
	if (!stream->writeFlag(isAtRest() || isRotate)) {
		if (stream->writeFlag(mask & VelocityMask)) {
			Point3F vel = getLinearVelocity();
			stream->write(sizeof( Point3F ), &vel);
		}
	}
   return 0;
}

void Item::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
	Parent::unpackUpdate(gm,stream);

	if (stream->readFlag()) {
		AssertFatal(manager == 0,
			"Item::unpackUpdate: Can only update shape type on initial packet");
      unpackDatFile(stream);
      if (stream->readFlag())
         sensorKey = stream->readInt(7);
		if (stream->readFlag()) {
	      if (SimNetObject* obj = gm->resolveGhost(stream->readInt(10)))
		      throwSource = dynamic_cast<GameBase*>(obj);
		}
		isRotate = stream->readFlag();
		isCollideable = stream->readFlag();
	}

	bool gotRotation = stream->readFlag();
	if (gotRotation)
		setRot(Point3F(0,0,M_2PI * stream->readFloat(8)));

	if (stream->readFlag()) {
		Point3F vec;
		stream->read(sizeof(vec), &vec);
		setPos(vec);
	}
	else
		if (gotRotation) {
			// The transform only gets updated when setPos is called.
			// Since we are not moving, we need to force it.
			setPos(getPos());
		}
	if (stream->readFlag()) {
		SimMovement::flags.set(AtRest);
		lVelocity.set(0,0,0);
	}
	else {
		if (stream->readFlag()) {
			Point3F vel;
			stream->read(sizeof(vel), &vel);
			setLinearVelocity(vel);
		}
	}
}


//----------------------------------------------------------------------------

void Item::initPersistFields()
{
   Parent::initPersistFields();
   addField("rotates", TypeBool, Offset(isRotate, Item));
   addField("collideable", TypeBool, Offset(isCollideable, Item));
   addField("count", TypeInt, Offset(count, Item));
}

//----------------------------------------------------------------------------

Persistent::Base::Error Item::read(StreamIO &sio, int, int)
{
	int version;
	sio.read(&version);

	Parent::read (sio, 0, 0);

	sio.read(&isRotate);
	sio.read(&count);

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error Item::write(StreamIO &sio, int, int)
{
	int version = 0;
	sio.write(version);

	Parent::write (sio, 0, 0);

	sio.write(isRotate);
	sio.write(count);

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}

bool
Item::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if (isAtRest() == false && m_lastSunUpdated + 250 < wg->currentTime) {
      float sunAttenuation;
      ColorF positionalColor;
      bool overrideSun = getPositionalColor(positionalColor, sunAttenuation);
      updateSunOverride(overrideSun, sunAttenuation, positionalColor);
      m_lastSunUpdated = wg->currentTime;
   }

   return Parent::processQuery(query);
}

Int32
Item::getDamageId()
{
   // Check to make sure that the client that owns the number is
   //  the correct client...
   if (throwerClientId == 0)
      return 0;

   PlayerManager* pPM = PlayerManager::get(manager);
   AssertFatal(pPM, "No playermanager");
   PlayerManager::BaseRep* pCR = pPM->findBaseRep(throwerClientId);

   if (pCR == NULL || pCR->loginId != throwerClientLoginId)
      return 0;
   
   return throwerClientId;
}
