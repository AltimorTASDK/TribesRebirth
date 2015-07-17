//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <simTrigger.h>
#include <esfobjecttypes.h>
#include <darkstar.strings.h>
#include <editor.strings.h>
#include <commoneditor.strings.h>
#include <missioneditor.h>
#include <simterrain.h>
#include <Console.h>
#include <stringTable.h>

#define TIMEOUT   (1.0f/4.0f)
#define MAX_RATE  (1.0f/30.0f)
#define RATE      (1.0f/4.0f)


//------------------------------------------------------------------------------
// triggers script functions
//    onEnter_trigger
//    onContact_trigger
//    onLeave_trigger


//------------------------------------------------------------------------------
SimTrigger::SimTrigger()
{
   type        = SimTriggerObjectType | SimMissionObjectType;
   resolve     = true;
   onEnterFn   = NULL;
   onContactFn = NULL;
   onLeaveFn   = NULL;
   onAddFn     = NULL;
   onEnabledFn = NULL;
   onDisabledFn= NULL;

   isEnabled   = true;
   isSphere    = true;
   rate        = RATE;
   className   = NULL;
   netFlags.clear(SimNetObject::Ghostable);

   collisionMask.set( SimPlayerObjectType | SimProjectileObjectType | SimVehicleObjectType );

   boundingBox.fMin.set(-1, -1, -1);
   boundingBox.fMax.set( 1,  1,  1);
   sphereCollisionImage.center.set();

   boundingBoxRotate         = true;
   boxCollisionImage.trigger = true;
   sphereCollisionImage.trigger = true;
}

//------------------------------------------------------------------------------
SimTrigger::~SimTrigger()
{
   delete [] onEnterFn;
   delete [] onContactFn;
   delete [] onLeaveFn;
   delete [] onAddFn;
   delete [] onEnabledFn;
   delete [] onDisabledFn;
}

//------------------------------------------------------------------------------
void SimTrigger::syncBoxes()
{
   setMaskBits(NETMASK_SIZED);

   boxCollisionImage.box = boundingBox;
   image.box             = boundingBox;

   float radius = min(boundingBox.len_x(), boundingBox.len_y());
   sphereCollisionImage.radius = min(radius, boundingBox.len_z());
}


//------------------------------------------------------------------------------
void SimTrigger::fnLookup(const char *fn, const char *name, char **result)
{
   *result = NULL;
   char function[256];
   if (name)
   {
      strcpy(function,name);
      strcat(function,"::");
      strcat(function,fn);
      if (Console->isFunction(function))
         *result = strnew(function);
   }
   if (!*result && className)
   {
      strcpy(function,className);
      strcat(function,"::");
      strcat(function,fn);
      if (Console->isFunction(function))
         *result = strnew(function);
   }
   if (!*result)
   {
      if (Console->isFunction(fn))
         *result = strnew(fn);
   }
}   

//------------------------------------------------------------------------------
void SimTrigger::fnResolve()
{
   if (resolve)
   {
      delete [] onEnterFn;
      delete [] onContactFn;
      delete [] onLeaveFn;
      delete [] onAddFn;
      delete [] onEnabledFn;
      delete [] onDisabledFn;

      const char *name = getName();
      //-------------------------------------- resolve onEnter_
      fnLookup("trigger::onEnter", name, &onEnterFn);
      fnLookup("trigger::onContact", name, &onContactFn);
      fnLookup("trigger::onLeave", name, &onLeaveFn);
      fnLookup("trigger::onAdd", name, &onAddFn);
      fnLookup("trigger::onEnabled", name, &onEnabledFn);
      fnLookup("trigger::onDisabled", name, &onDisabledFn);
      resolve = false;
   }
}   

//------------------------------------------------------------------------------
const char* SimTrigger::callScript(int argc, char **fn, ...)
{
   if (!manager->isServer()) 
      return (NULL);

   const char *argv[16];
   va_list args;
   va_start(args, fn);

   // if we're ghosted the mission editor is on, don't call any scripts!
   if (netFlags.test(SimNetObject::Ghostable))
      return (NULL);

   fnResolve();
   if (!*fn) 
      return (NULL);

   argv[0] = *fn;
   for(int i = 1; i < argc; i++)
      argv[i] = va_arg(args, const char *);
   va_end(args);
   return Console->execute(argc, argv);
}   


//------------------------------------------------------------------------------
bool SimTrigger::enable(bool tf)
{
   if (isEnabled != tf)
   {
      isEnabled = tf;
      if (!isEnabled && contactList.size())         
      {  // if we are being disabled, for each object 
         // call the onLeave_trigger script and remove the notify
         while (contactList.size())
         {
            clearNotify(contactList.last().object);
            callScript(3, &onLeaveFn, stringId, contactList.last().stringId);
            contactList.pop_back();
         }   
         removeFromSet(SimTimerSetId);
      }
      if (isEnabled)
         callScript(2, &onEnabledFn, stringId);
      else
         callScript(2, &onDisabledFn, stringId);
   }
   return (true);
}

//------------------------------------------------------------------------------
bool SimTrigger::onAdd()
{
   if (!Parent::onAdd())
      return (false);

   // Add ourselves to the root container
   SimContainer *root = findObject(manager, SimRootContainerId, root);
   AssertFatal(root, "SimTrigger::onAdd: no root container");
   root->addObject(this);
   
   if (manager->isServer())
   {
      itoa(getId(), stringId, 10);     // we'll be using this a lot! 
      netFlags.clear(SimNetObject::Ghostable);
   }

   syncBoxes();

   callScript(2, &onAddFn, stringId);
   if (isEnabled)
      callScript(2, &onEnabledFn, stringId);
   else
      callScript(2, &onDisabledFn, stringId);

   return (true);
}

//------------------------------------------------------------------------------
void SimTrigger::onGroupAdd()
{
   Parent::onGroupAdd();   
}   


//------------------------------------------------------------------------------
void SimTrigger::onDeleteNotify(SimObject *object)
{
   ContactList::iterator i = contactList.begin();
   for (; i != contactList.end(); i++)
      if (i->object == object)
      {
         contactList.erase(i);
         if (contactList.size() == 0)
            removeFromSet(SimTimerSetId);
         return;
      }
}   

//------------------------------------------------------------------------------
bool SimTrigger::onSimObjectTransformQuery(SimObjectTransformQuery *query)
{
   query->tmat = getTransform();
   return (true);
}

//------------------------------------------------------------------------------
bool SimTrigger::onSimCollisionImageQuery(SimCollisionImageQuery *query)
{
   query->count    = 1;
   if (isSphere)
   {
      query->image[0] = &sphereCollisionImage;
      sphereCollisionImage.transform = getTransform();
   }
   else
   {
      query->image[0] = &boxCollisionImage;
      boxCollisionImage.transform = getTransform();
   }
   return (true);
}

//------------------------------------------------------------------------------
bool SimTrigger::onSimRenderQueryImage(SimRenderQueryImage *query)
{
   query->count    = 1;
   query->image[0] = &image;
   image.transform = getTransform();
   return (true);
}

//------------------------------------------------------------------------------
bool SimTrigger::onSimImageTransformQuery(SimImageTransformQuery *query)
{
   query->transform = getTransform();
   return (true);
}

//------------------------------------------------------------------------------
bool SimTrigger::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{
   setPosition(event->tmat, NULL, true);
   setMaskBits(NETMASK_MOVED);
   return (true);
}

//------------------------------------------------------------------------------
bool SimTrigger::onSimMovementCollisionEvent(const SimMovementCollisionEvent *event)
{
   SimObject *object = event->getObject(manager);
   if (!object)
      return (false);

   if ( !(object->getType() & collisionMask) )
      return (false);

   // if the object is already in contact with us just adjust its
   // expire time
   ContactList::iterator i = contactList.begin();
   for (; i != contactList.end(); i++)
   {
      if (i->object == object)
      {
         i->expire = manager->getCurrentTime() + TIMEOUT;
         return (true);
      }
   }

   // a new object!  Add it to the contact list and call 
   // the onEnter_trigger script
   contactList.increment();
   contactList.last().object = object;
   contactList.last().expire = manager->getCurrentTime() + TIMEOUT;
   contactList.last().contact= manager->getCurrentTime() + rate;
   deleteNotify(object);
   itoa(object->getId(), contactList.last().stringId, 10);
   callScript(3, &onEnterFn, stringId, contactList.last().stringId);

   if (contactList.size() == 1)
      addToSet(SimTimerSetId);
   return (true);
}


//------------------------------------------------------------------------------
bool SimTrigger::onSimTimerEvent(const SimTimerEvent *event)
{
   event;
   SimTime currentTime = manager->getCurrentTime();
   ContactList::iterator i = contactList.begin();
   for (; i != contactList.end(); )
   {
      // has this object expired (ie it's no longer in contact) ?
      if (i->expire <= currentTime)
      {  // clear the notify and call the onLeave_trigger script
         clearNotify(i->object);
         callScript(3, &onLeaveFn, stringId, i->stringId);
         contactList.erase(i);
      }
      else
      {  // call the onContact_trigger script
         if (i->contact <= currentTime)
         {
            callScript(3, &onContactFn, stringId, i->stringId);
            i->contact = currentTime+rate;
         }
         i++;
      }
   }
   return (true);
}   


//------------------------------------------------------------------------------
bool SimTrigger::onSimEditEvent(const SimEditEvent *event)
{
   if (event->state)
   {
      boxCollisionImage.trigger = false;
      sphereCollisionImage.trigger = false;
      netFlags.set(SimNetObject::Ghostable);
   }
   else
   {
      boxCollisionImage.trigger = true;
      sphereCollisionImage.trigger = true;
      netFlags.clear(SimNetObject::Ghostable);
   }
   resolve = true;
   return (true);
}

//------------------------------------------------------------------------------
bool SimTrigger::processQuery(SimQuery *query)
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
bool SimTrigger::processEvent(const SimEvent *event)
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
void SimTrigger::setFieldValue(StringTableEntry slotName, const char *value)
{
   if (stricmp(slotName, "isEnabled") == 0 )   
      enable( (*value == 'T' || *value == 't' || *value == '1') );
   Parent::setFieldValue(slotName, value);
}   


//------------------------------------------------------------------------------
void SimTrigger::inspectWrite(Inspect *inspect)
{
   EulerF  rot;
   TMat3F  mat = getTransform();

   Parent::inspectWrite(inspect);

   mat.angles(&rot);

   inspect->write(IDCOMEDIT_ITG_CLASSNAME, (char *) className);
   inspect->write(IDCOMEDIT_ITG_INITIAL, isEnabled);

   bool player     = collisionMask & SimPlayerObjectType;
   bool projectile = collisionMask & SimProjectileObjectType;
   bool vehicle    = collisionMask & SimVehicleObjectType;
   inspect->write(IDCOMEDIT_ITG_PLAYER, player);
   inspect->write(IDCOMEDIT_ITG_PROJECTILE, projectile);
   inspect->write(IDCOMEDIT_ITG_VEHICLE, vehicle);

   inspect->write(IDCOMEDIT_ITG_SPHERE,  isSphere);
   inspect->write(IDCOMEDIT_ITG_CONTACT_RATE, rate);
   
   inspect->write(IDITG_POSITION, mat.p);
   inspect->write(IDITG_ROTATION, Point3F(rot.x, rot.y, rot.z));

   inspect->write(IDCOMEDIT_ITG_DIMENSIONS, 
      Point3F(boundingBox.len_x(), boundingBox.len_y(), boundingBox.len_z()));
}

//------------------------------------------------------------------------------
void SimTrigger::inspectRead(Inspect *inspect)
{
   Point3F pos, rot, dim;

   Parent::inspectRead(inspect);

   bool tf;
   Inspect_Str name;
   inspect->read(IDCOMEDIT_ITG_CLASSNAME, name);
   className = stringTable.insert(name);
   inspect->read(IDCOMEDIT_ITG_INITIAL, tf);
   
   bool flag;
   inspect->read(IDCOMEDIT_ITG_PLAYER, flag);
   collisionMask.set(SimPlayerObjectType, flag);

   inspect->read(IDCOMEDIT_ITG_PROJECTILE, flag);
   collisionMask.set(SimProjectileObjectType, flag);

   inspect->read(IDCOMEDIT_ITG_VEHICLE, flag);
   collisionMask.set(SimVehicleObjectType, flag);

   inspect->read(IDCOMEDIT_ITG_SPHERE,  isSphere);
   inspect->read(IDCOMEDIT_ITG_CONTACT_RATE, rate);
   if (rate < MAX_RATE)
      rate = MAX_RATE;

   inspect->read(IDITG_POSITION, pos);
   inspect->read(IDITG_ROTATION, rot);
   inspect->read(IDCOMEDIT_ITG_DIMENSIONS, dim);

   SimObjectTransformQuery query;
   processQuery(&query);
   SimObjectTransformEvent event(query.tmat);

   event.tmat.set(EulerF(rot.x, rot.y, rot.z), pos);
   processEvent(&event);
 
   dim.x /= 2;
   dim.y /= 2;
   dim.z /= 2;

   boundingBox.fMin.set(-dim.x, -dim.y, -dim.z);
   boundingBox.fMax.set( dim.x,  dim.y,  dim.z);

   enable(tf);
   syncBoxes();
}

//------------------------------------------------------------------------------

void SimTrigger::initPersistFields()
{
   addField("className", TypeString, Offset(className, SimTrigger));
   addField("isEnabled", TypeBool, Offset(isEnabled, SimTrigger));
   addField("isSphere", TypeBool, Offset(isSphere, SimTrigger));
   addField("rate", TypeFloat, Offset(rate, SimTrigger));
   addField("collisionMask", TypeInt, Offset(collisionMask, SimTrigger));
   addField("transform", TypeTMat3F, Offset(transform, SimTrigger));
   addField("boundingBox", TypeBox3F, Offset(boundingBox, SimTrigger));
}

//------------------------------------------------------------------------------
Persistent::Base::Error SimTrigger::write(StreamIO &sio, int iVersion, int iUser)
{
   Parent::write(sio, iVersion, iUser);

   sio.writeString(className);
   sio.write(isEnabled);
   sio.write(isSphere);
   sio.write(rate);
   sio.write(collisionMask);

   sio.write(sizeof(TMat3F), (void *)&getTransform());
   sio.write(sizeof(Box3F),  (void *)&boundingBox);

   return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}

//------------------------------------------------------------------------------
Persistent::Base::Error SimTrigger::read(StreamIO &sio, int iVersion, int iUser)
{
   TMat3F mat;

   Parent::read(sio, iVersion, iUser);

   className = sio.readSTString();
   sio.read(&isEnabled);
   sio.read(&isSphere);
   sio.read(&rate);
   sio.read(sizeof(BitSet32), &collisionMask);

   sio.read(sizeof(TMat3F), (void *)&mat);
   sio.read(sizeof(Box3F),  (void *)&boundingBox);
   resolve = true;

   processEvent(&SimObjectTransformEvent(mat));

   return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}

//------------------------------------------------------------------------------
DWORD SimTrigger::packUpdate(Net::GhostManager *, DWORD dwMask, BitStream *stream)
{
   if (stream->writeFlag(dwMask & NETMASK_MOVED))
      stream->write(sizeof(TMat3F), &getTransform());

   if (stream->writeFlag(dwMask & NETMASK_SIZED))
      stream->write(sizeof(Box3F), (void *)&boundingBox);
   return (0);
}

//------------------------------------------------------------------------------
void SimTrigger::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
   TMat3F mat;

   if (stream->readFlag())
   {
      stream->read(sizeof(TMat3F), &mat);
      processEvent(&SimObjectTransformEvent(mat));
   }

   if (stream->readFlag())
   {
      stream->read(sizeof(Box3F), (void *)&boundingBox);
      syncBoxes();
   }
}