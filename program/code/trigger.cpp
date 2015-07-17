//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <esfobjecttypes.h>
#include <darkstar.strings.h>
#include <editor.strings.h>
#include <commoneditor.strings.h>
#include <Console.h>
#include <stringTable.h>
#include <DataBlockManager.h>

#include "Trigger.h"
#include "feardcl.h"

//------------------------------------------------------------------------------

#define TIMEOUT   (1.0f/4.0f)
#define MAX_RATE  (1.0f/30.0f)
#define RATE      (1.0f/4.0f)

const int CollisionMask = 
			SimPlayerObjectType | 
			SimProjectileObjectType |
			VehicleObjectType |
      	MoveableObjectType |
			MineObjectType;


//------------------------------------------------------------------------------
// triggers script functions
//    onEnter_trigger
//    onContact_trigger
//    onLeave_trigger

Trigger::TriggerData::TriggerData()
{
   rate = RATE;
}

//------------------------------------------------------------------------------

Trigger::Trigger()
{
   type = TriggerObjectType | SimMissionObjectType;

   active = true;
   isSphere = true;
   netFlags.clear(SimNetObject::Ghostable);

   collisionMask = CollisionMask;

   boundingBox.fMin.set(-1, -1, -1);
   boundingBox.fMax.set( 1,  1,  1);
   sphereCollisionImage.center.set();

	rotation.set(0,0,0);
   boundingBoxRotate = true;
   boxCollisionImage.trigger = true;
   sphereCollisionImage.trigger = true;
}


//------------------------------------------------------------------------------

Trigger::~Trigger()
{
}

//------------------------------------------------------------------------------
void Trigger::syncBoxes()
{
   setMaskBits(SizedMask);

   image.box = boxCollisionImage.box = boundingBox;
   float radius = min(boundingBox.len_x(), boundingBox.len_y());
   sphereCollisionImage.radius = min(radius, boundingBox.len_z());
}

//------------------------------------------------------------------------------
bool Trigger::callScript()
{
	return !netFlags.test(SimNetObject::Ghostable);
}

//------------------------------------------------------------------------------
void Trigger::onEnter(GameBase* obj)
{
	if (callScript())
	 	if (const char* script = scriptName("onEnter")) {
			char buff[32];
			strcpy(buff,scriptThis());
			Console->executef(3,script,buff,obj->scriptThis());
	 	}
}

void Trigger::onContact(GameBase* obj)
{
	if (callScript())
	 	if (const char* script = scriptName("onContact")) {
			char buff[32];
			strcpy(buff,scriptThis());
			Console->executef(3,script,buff,obj->scriptThis());
	 	}
}

void Trigger::onLeave(GameBase* obj)
{
	if (callScript())
	 	if (const char* script = scriptName("onLeave")) {
			char buff[32];
			strcpy(buff,scriptThis());
			Console->executef(3,script,buff,obj->scriptThis());
	 	}
}

void Trigger::onActivate()
{
	if (callScript())
	 	if (const char* script = scriptName("onActivate"))
			Console->executef(2,script,scriptThis());
}

void Trigger::onDeactivate()
{
	if (callScript())
	 	if (const char* script = scriptName("onDeactivate"))
			Console->executef(2,script,scriptThis());
}


//------------------------------------------------------------------------------
bool Trigger::isActive()
{
	return active;
}

bool Trigger::setActive(bool tf)
{
   if (active != tf) {
      active = tf;
      if (!active && triggerList.size()) {
      	// if we are being disabled, for each object 
         // call the onLeave_trigger script and remove the notify
         while (triggerList.size()) {
            clearNotify(triggerList.last().object);
				onLeave(triggerList.last().object);
            triggerList.pop_back();
         }
         removeFromSet(SimTimerSetId);
      }
      if (active)
			onActivate();
      else
			onDeactivate();
   }
   return true;
}


//------------------------------------------------------------------------------
bool Trigger::onAdd()
{
   if (!Parent::onAdd())
      return false;

	TMat3F mat;
	mat.set(EulerF(rotation.x,rotation.y,rotation.z),getLinearPosition());
   setPosition(mat,true);

   // Add ourselves to the root container
   SimContainer *root = findObject(manager, SimRootContainerId, root);
   AssertFatal(root, "Trigger::onAdd: no root container");
   root->addObject(this);

   if (manager->isServer())
      netFlags.clear(SimNetObject::Ghostable);
   syncBoxes();

	if (!isGhost())
		onActivate();
   return true;
}

void Trigger::onRemove()
{
	Parent::onRemove();
	if (!isGhost() && getDatPtr() && active)
		setActive(false);
}


//------------------------------------------------------------------------------
void Trigger::onDeleteNotify(SimObject *object)
{
   TriggerList::iterator i = triggerList.begin();
   for (; i != triggerList.end(); i++)
      if (i->object == object) {
         triggerList.erase(i);
         if (triggerList.size() == 0)
            removeFromSet(SimTimerSetId);
         return;
      }
}   


//------------------------------------------------------------------------------
bool Trigger::onSimObjectTransformQuery(SimObjectTransformQuery *query)
{
   query->tmat = getTransform();
   return (true);
}


//------------------------------------------------------------------------------
bool Trigger::onSimCollisionImageQuery(SimCollisionImageQuery *query)
{
   query->count = 1;
   if (isSphere) {
      query->image[0] = &sphereCollisionImage;
      sphereCollisionImage.transform = getTransform();
   }
   else {
      query->image[0] = &boxCollisionImage;
      boxCollisionImage.transform = getTransform();
   }
   return (true);
}

//------------------------------------------------------------------------------
bool Trigger::onSimRenderQueryImage(SimRenderQueryImage *query)
{
   query->count    = 1;
   query->image[0] = &image;
   image.transform = getTransform();
   return (true);
}

//------------------------------------------------------------------------------
bool Trigger::onSimImageTransformQuery(SimImageTransformQuery *query)
{
   query->transform = getTransform();
   return (true);
}

//------------------------------------------------------------------------------
bool Trigger::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{
	EulerF rot;
	event->tmat.angles(&rot);
	rotation.set(rot.x,rot.y,rot.z);
   setPosition(event->tmat,true);
   setMaskBits(PositionMask);
   return (true);
}

//------------------------------------------------------------------------------
bool Trigger::onSimMovementCollisionEvent(const SimMovementCollisionEvent *event)
{
	if (isGhost() || !active)
		return false;
   SimObject *sobj = event->getObject(manager);
   if (!sobj)
      return false;
   if (!(sobj->getType() & collisionMask))
      return (false);
	GameBase* object = dynamic_cast<GameBase*>(sobj);
	if (!object)
		return false;

   // if the object is already in contact with us just adjust its
   // expire time
   TriggerList::iterator i = triggerList.begin();
   for (; i != triggerList.end(); i++) {
      if (i->object == object) {
         i->expire = manager->getCurrentTime() + TIMEOUT;
         return true;
      }
   }

   // a new object!  Add it to the contact list and call 
   // the onEnter_trigger script
   triggerList.increment();
   triggerList.last().object = object;
   triggerList.last().expire = manager->getCurrentTime() + TIMEOUT;
   triggerList.last().contact= manager->getCurrentTime() + 
   	static_cast<TriggerData*>(getDatPtr())->rate;
	deleteNotify(object);
	onEnter(object);

   if (triggerList.size() == 1)
      addToSet(SimTimerSetId);
   return true;
}

//------------------------------------------------------------------------------
bool Trigger::onSimTimerEvent(const SimTimerEvent *event)
{
   event;
   SimTime currentTime = manager->getCurrentTime();
   TriggerList::iterator i = triggerList.begin();
   for (; i != triggerList.end(); ) {
      // has this object expired (ie it's no longer in contact) ?
      if (i->expire <= currentTime) {
      	// clear the notify and call the onLeave_trigger script
         clearNotify(i->object);
			onLeave(i->object);
         triggerList.erase(i);
      }
      else {
      	// call the onContact_trigger script
         if (i->contact <= currentTime) {
				onContact(i->object);
            i->contact = currentTime+
			   	static_cast<TriggerData*>(getDatPtr())->rate;
         }
         i++;
      }
   }
   return true;
}   


//------------------------------------------------------------------------------
bool Trigger::onSimEditEvent(const SimEditEvent *event)
{
   if (event->state) {
      boxCollisionImage.trigger = false;
      sphereCollisionImage.trigger = false;
      netFlags.set(SimNetObject::Ghostable);
   }
   else {
      boxCollisionImage.trigger = true;
      sphereCollisionImage.trigger = true;
      netFlags.clear(SimNetObject::Ghostable);
   }
   return true;
}

//------------------------------------------------------------------------------
bool Trigger::processQuery(SimQuery *query)
{
   switch (query->type)
   {
      onQuery(SimObjectTransformQuery);
      onQuery(SimCollisionImageQuery);
      onQuery(SimRenderQueryImage);
      onQuery(SimImageTransformQuery);
   }
   return (Parent::processQuery(query));
}

//------------------------------------------------------------------------------
bool Trigger::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimEditEvent);
      onEvent(SimObjectTransformEvent);
      onEvent(SimTimerEvent);
      onEvent(SimMovementCollisionEvent);
   }
   return (Parent::processEvent(event));
}

//------------------------------------------------------------------------------
void Trigger::inspectWrite(Inspect *inspect)
{
   Parent::inspectWrite(inspect);

   inspect->write(IDCOMEDIT_ITG_SPHERE,  isSphere);
   inspect->write(IDITG_POSITION, getTransform().p);
   inspect->write(IDITG_ROTATION, rotation);
   inspect->write(IDCOMEDIT_ITG_DIMENSIONS, 
      Point3F(boundingBox.len_x(), boundingBox.len_y(), boundingBox.len_z()));
}

//------------------------------------------------------------------------------
void Trigger::inspectRead(Inspect *inspect)
{
   Parent::inspectRead(inspect);

   Point3F pos,dim;
   inspect->read(IDCOMEDIT_ITG_SPHERE,  isSphere);
   inspect->read(IDITG_POSITION, pos);
   inspect->read(IDITG_ROTATION, rotation);
   inspect->read(IDCOMEDIT_ITG_DIMENSIONS, dim);

	TMat3F mat;
	mat.set(EulerF(rotation.x,rotation.y,rotation.z),pos);
   setPosition(mat,true);
   setMaskBits(PositionMask);
 
   dim.x /= 2;
   dim.y /= 2;
   dim.z /= 2;

   boundingBox.fMin.set(-dim.x, -dim.y, -dim.z);
   boundingBox.fMax.set( dim.x,  dim.y,  dim.z);

   syncBoxes();
}

//------------------------------------------------------------------------------
int Trigger::getDatGroup()
{
	return DataBlockManager::TriggerDataType;
}

//------------------------------------------------------------------------------
void Trigger::initPersistFields()
{
	Parent::initPersistFields();
   addField("position", TypePoint3F, Offset(lPosition, Trigger));
   addField("rotation", TypePoint3F, Offset(rotation, Trigger));
   addField("boundingBox", TypeBox3F, Offset(boundingBox, Trigger));
   addField("isSphere", TypeBool, Offset(isSphere, Trigger));
}

//------------------------------------------------------------------------------
DWORD Trigger::packUpdate(Net::GhostManager *, DWORD dwMask, BitStream *stream)
{
   if (stream->writeFlag(dwMask & PositionMask))
      stream->write(sizeof(TMat3F), &getTransform());

   if (stream->writeFlag(dwMask & SizedMask))
      stream->write(sizeof(Box3F), (void *)&boundingBox);
   return (0);
}

void Trigger::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
   TMat3F mat;

   if (stream->readFlag()) {
      stream->read(sizeof(TMat3F), &mat);
      processEvent(&SimObjectTransformEvent(mat));
   }

   if (stream->readFlag()) {
      stream->read(sizeof(Box3F), (void *)&boundingBox);
      syncBoxes();
   }
}

//------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAGS( Trigger, FOURCC('T','R','G','R'), TriggerPersTag );

Persistent::Base::Error Trigger::write(StreamIO &sio, int iVersion, int iUser)
{
   Parent::write(sio, iVersion, iUser);
   return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}

Persistent::Base::Error Trigger::read(StreamIO &sio, int iVersion, int iUser)
{
   Parent::read(sio, iVersion, iUser);
   return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}

