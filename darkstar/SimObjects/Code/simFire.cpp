//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <SimFire.h>
#include <SimSmoke.h>
#include <SimTagDictionary.h>
#include <SimPersistTags.h>
#include <NetPacketStream.h>
#include <NetGhostManager.h>
#include <NetEventManager.h>
#include <g_surfac.h>
#include <d_caps.h>
#include <soundFX.h>
#include <simPreLoadManager.h>
#include "simResource.h"
#include "simTimerGrp.h"
#include "console.h"
#include "simExplosion.h"
#include "coreRes.h"

//------------------------------------------------------------------------------
// a random generator for fire code
static Random fireRand;

bool SimFire::DynamicLighting = true;

#define SMOKE_ISI_SW_MULTIPLIER (2.0f)

//--------------------------------------------------------------------------- 

SimFire::SimFire( int _shapeTag, int _tshapeTag, float _fireOut )
{
   type |= SimExplosion::defaultFxObjectType;
   
   shapeTag = _shapeTag;
   translucentShapeTag = _tshapeTag;
   fireOut = _fireOut;

   pos.set(0, 0, 0);
   onObj = 0;

   producesSmoke = false;
   useLight = true;

   renderedYet = false;

   faceCam = false;
   followCam = false;

   soundID = -1;
   hSound  = SFX_INVALID_HANDLE;
}

SimFire::SimFire(SimFire & fire)
{
   type |= SimExplosion::defaultFxObjectType;

   onObj = fire.onObj;
   faceCam = fire.faceCam;
   followCam = fire.followCam;
   producesSmoke = fire.producesSmoke;
   useLight = fire.useLight;

   soundID = fire.soundID;
   shapeTag = fire.shapeTag;
   translucentShapeTag = fire.translucentShapeTag;

   fireOut = fire.fireOut;

   smokeShapeTag = fire.smokeShapeTag;
   smokeTransShapeTag = fire.smokeTransShapeTag;
   smokeToSmoke = fire.smokeToSmoke;
   smokeOut = fire.smokeOut;
   smokeOffset = fire.smokeOffset;
   smokeVel = fire.smokeVel;
   
   lightRange = fire.lightRange;
   lightColor = fire.lightColor;

   pos = fire.pos;
}

SimFire::SimFire( const SimFireRecord * record, float _fireOut )
{
   type |= SimExplosion::defaultFxObjectType;

   shapeTag = record->shapeTag;
   translucentShapeTag = record->shapeTagT;
   soundID = record->soundID;

   producesSmoke = record->smokeTag > 0;
   setSmoke(record->smokeTag,record->smokeTagT,record->ISI,record->smokeDuration);
   smokeOffset.x = record->smokeXoffset;
   smokeOffset.y = record->smokeYoffset;
   smokeOffset.z = record->smokeZoffset;
   
   lightRange = record->lightRange;
   lightColor.set(record->r, record->g, record->b);
   
   faceCam = record->faceCam==1;
   followCam = record->faceCam==2;

   renderedYet = false;
   useLight = true;

   fireOut = _fireOut;
   pos.set(0, 0, 0);
   onObj = 0;
   hSound  = SFX_INVALID_HANDLE;
}

void SimFire::setSmoke(int _shapeTag, int _tshapeTag, float _ISI, float _smokeOut)
{
   producesSmoke      = true;
   smokeShapeTag      = _shapeTag;
   smokeTransShapeTag = _tshapeTag;
   smokeToSmoke       = _ISI;
   smokeOut           = _smokeOut;
   smokeOffset.set(0, 0, 0);
   smokeVel.set(0, 0, 0);
}

void SimFire::setSmokePosVel(const Point3F & _offset, const Point3F &_vel)
{
   AssertFatal(producesSmoke,"SimFire::setSmokePosVel:  must set smoke first.");
   smokeOffset        = _offset;
   smokeVel           = _vel;
}

void SimFire::setSmokeVel(const Point3F &_vel)
{
   AssertFatal(producesSmoke,"SimFire::setSmokeVel:  must set smoke first.");
   smokeVel           = _vel;
}

void SimFire::setPosition(const Point3F & _pos)
{ 
   onObj = 0; 
   pos = _pos;
}

void SimFire::setObject(SimObject *obj,const Point3F & _offset)
{
   onObj = obj; 
   pos = _offset;
}

void SimFire::setFireDuration(float _time)
{ 
   fireOut = _time;
}

void SimFire::setSound(int _id)
{
   soundID = _id;
}

//--------------------------------------------------------------------------- 

bool SimFire::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimTimerEvent);
        case SimMessageEventType :
            manager->deleteObject(this);
            break;
   }
   return false;
}

//------------------------------------------------------------------------------
bool SimFire::onAdd()
{
   if (!Parent::onAdd())
      return false;
    finishAddToManager();
   return true;
}

void SimFire::finishAddToManager()
{
   if (onObj)
      deleteNotify(onObj);

   // don't know shape size yet, fake it for now
   bbox.fMin.set(-1,-1,-1);
   bbox.fMax.set(1,1,1);
   if (!faceCam && !followCam)
      image.transform.identity();
   Point3F & p = image.transform.p;
   image.transform.flags |= TMat3F::Matrix_HasTranslation; // will keep this flag setting
   getPosition(p);
   updateBBox(p);
   updateBox = false;
    fireHeight = 0;

   // set fire start and end time
   prevAnimTime = manager->getCurrentTime();
   fireOut += prevAnimTime;

   // set next smoke time
   if (producesSmoke)
      nextSmokeTime = smokeToSmoke*.5f + prevAnimTime;

   // add to timer set
   addToSet(SimTimerSetId);

   // add to container db so we get render query
   SimContainer *root = NULL;
   
   root = findObject(manager, SimRootContainerId,root);
   root->addObject(this);

   // start sound if there is one
   if (soundID != -1)
      hSound = Sfx::Manager::PlayAt( manager, soundID, TMat3F(EulerF(0, 0, 0), p), Point3F(0, 0, 0));
      
   // prepare the light
   if (lightRange > 0.0f && SimFire::DynamicLighting)
   {
      glow.setType(TS::Light::LightPoint);
      glow.setIntensity(lightColor);
      glow.setRange(lightRange);
      glow.setPosition(p);
      
      addToSet(SimLightSetId);
   }
}

//------------------------------------------------------------------------------
bool SimFire::onSimLightQuery ( SimLightQuery * query )
{
   if (!useLight)
      return false;
      
   query->ambientIntensity.set();
   
   query->count = 1;
   query->light[0] = &glow;

   return true;
}

//------------------------------------------------------------------------------
bool SimFire::onSimTimerEvent(const SimTimerEvent *)
{
   // get current time
   float now = manager->getCurrentTime();

    // check if the time is up
    if (now>fireOut)
    {
        manager->deleteObject(this);
      return true;
    }

   if (onObj || updateBox)
   {
      // update position of fire
      Point3F & p = image.transform.p;
      getPosition(p);
      updateBBox(p);
      updateBox = false;
   }

   // produces a smoke puff?
   if (now>nextSmokeTime && renderedYet)
   {
      SimSmoke * smoke = new SimSmoke(smokeShapeTag,smokeTransShapeTag,smokeOut);
      Point3F p = image.transform.p;
      p += smokeOffset;
      p.z += fireHeight;
      smoke->setPosition(p);
      smoke->setVelocity(smokeVel);
      smoke->setFaceCam();
      manager->addObject(smoke);
      if (inHardware)
         nextSmokeTime = now + smokeToSmoke;
      else
         nextSmokeTime = now + SMOKE_ISI_SW_MULTIPLIER * smokeToSmoke;
   }
   
   return true;
}

//------------------------------------------------------------------------------
void SimFire::onRemove()
{
   // remove from whatever container we're in
   if (getContainer())
      getContainer()->removeObject(this);

   Sfx::Manager::Stop( manager, hSound );

   if (image.translucentShape)
      delete image.translucentShape;
   if (image.nonTranslucentShape)
      delete image.nonTranslucentShape;
      
   if(lightRange > 0.0f && SimFire::DynamicLighting)
      removeFromSet(SimLightSetId);

   Parent::onRemove();
}

//------------------------------------------------------------------------------
void SimFire::onDeleteNotify(SimObject* object)
{
   if (object == onObj)
   {
      fireOut = -1;  // will delete ourself next timer update
                     // problematic to delete ourself in more than 1 place in the code...
      onObj = 0;
   }
}

//--------------------------------------------------------------------------- 
bool SimFire::processQuery(SimQuery *query)
{
   switch (query->type)
   {
      onQuery(SimLightQuery);
   
      case SimRenderQueryImageType: 
      {
         if ((float)manager->getCurrentTime() > fireOut)
            return false;

         if (shapeTag==-1 && translucentShapeTag==-1)
               return false;

           SimRenderQueryImage *qp = static_cast<SimRenderQueryImage *>(query);
         TSRenderContext & rc = *qp->renderContext;
         GFXSurface *srf = rc.getSurface();

         // shall we draw translucent?
         bool inHardware = srf->getCaps()&GFX_DEVCAP_SUPPORTS_CONST_ALPHA;
         bool translucent = (inHardware || 
                             SimExplosion::SoftwareTranslucency ||
                             shapeTag==-1)
                            && translucentShapeTag!=-1;

         if (!renderedYet || translucent!=image.drawTranslucent)
         {
            // need to switch modes
            if (translucent && !image.translucentShape)
               // need to load translucent shape
               getShape(translucentShapeTag,
                        &image.translucentShape,
                        &image.translucentThread);
            else if (!translucent && !image.nonTranslucentShape)
               // need to load non-translucent shape
               getShape(shapeTag,
                        &image.nonTranslucentShape,
                        &image.nonTranslucentThread);

            image.setTranslucent(translucent);

            if (!renderedYet)
               // got right radius (above)
               // update bounding box in container db next timer update
               updateBox = true;
         }

         // face camera
         if ( (!renderedYet && faceCam) || followCam)
            image.faceCamera(rc);

         // advance the animation
         float now = manager->getCurrentTime();
         image.thread->AdvanceTime(now-prevAnimTime);
         prevAnimTime = now;

         // get haze value from container
			SimContainer * cnt = getContainer();
			if (cnt)
				image.hazeValue = cnt->getHazeValue(rc, image.transform.p);

         image.setSortValue(rc.getCamera());
           qp->count = 1;
           qp->image[0] = &image;

         renderedYet = true;

         return true;
      }
   }
   return false;
}

//--------------------------------------------------------------------------- 

void SimFire::getShape(int shapeTag,
                               TSShapeInstance **ppShape,
                               TSShapeInstance::Thread **ppThread)
{
   // Read shape, initialize thread
   ResourceManager *rm = SimResource::get(manager);
   const char* shapeName = SimTagDictionary::getString(manager,shapeTag);
   hShape = rm->load(shapeName);
   AssertFatal((bool)hShape,"Unable to load fire shape");
   
   (*ppShape) = new TS::ShapeInstance(hShape, *rm);
   AssertFatal((bool)(*ppShape),"Could not create fire shape instance");

   (*ppThread) = (*ppShape)->CreateThread(); // pos 0, seq 0 by default
   AssertFatal(*ppThread,"Could not create fire animation thread");
   (*ppShape)->animateRoot(); // never have to do this again
   (*ppThread)->SetPosition(fireRand.getFloat(0.0f,1.0f)); // start at random position

   // set-up shape space bounding box -- real size
   bbox.fMin = (*ppShape)->getShape().fCenter;
   bbox.fMin += (*ppShape)->fRootDeltaTransform.p; // could be trouble if root has rot.
   bbox.fMax = bbox.fMin;
   bbox.fMin -= (*ppShape)->getShape().fRadius;
   bbox.fMax += (*ppShape)->getShape().fRadius;

   // get fire height
   Box3F bounds;
   TS::ObjectInstance * obj = (*ppShape)->getNode(0)->fObjectList[0];
   obj->getBox(*ppShape,bounds);
   fireHeight = bounds.fMax.z - bounds.fMin.z;
}

//------------------------------------------------------------------------------

void SimFire::updateBBox(const Point3F & p)
{
   Box3F box;
   box = bbox;
   box.fMin += p;
   box.fMax += p;
   setBoundingBox(box);
}

void SimFire::getPosition(Point3F &position)
{
   position = pos;
   if (onObj)
   {
      SimObjectTransformQuery query;
      if ( onObj->processQuery( &query ) )
      {
         // if matrix has rotation, better multiply through...
         // assume no scale even if flag set...
         if ( query.tmat.flags & TMat3F::Matrix_HasRotation )
            m_mul(pos,query.tmat,&position);
         else
            position += query.tmat.p;
      }
   }
}

void SimFire::createOnClients(SimFire * fire, SimManager * mgr)
{
   SimSet * packetStreams = static_cast<SimSet*>(mgr->findObject(PacketStreamSetId));
   if (packetStreams)
   {
      for (SimSet::iterator itr = packetStreams->begin(); itr != packetStreams->end(); itr++)
      {
         Net::PacketStream * pStream = dynamic_cast<Net::PacketStream *>(*itr);
         if (pStream && pStream->getGhostManager()->getCurrentMode() ==
                        Net::GhostManager::GhostNormalMode)
         {
            Net::RemoteCreateEvent * addEvent = new Net::RemoteCreateEvent;
            SimFire *fireCopy = new SimFire(*fire);
            addEvent->newRemoteObject = fireCopy;
            // if attached to an object, see if it is in scope...
            if (fire->onObj)
            {
               int gidx = pStream->getGhostManager()->getGhostIndex(static_cast<SimNetObject*>(fire->onObj));
               if (gidx==-1)
                  continue; // object must be out of scope
            }
            pStream->getEventManager()->postRemoteEvent(addEvent);
         }
      }
   }
   if (!fire->manager)
      // this was a temporary object
      delete fire;
}

//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
// Standard Persistent IO stuff
//
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 


IMPLEMENT_PERSISTENT_TAG( SimFire, SimFirePersTag );

void SimFire::unpackUpdate(Net::GhostManager * gm, BitStream *stream)
{
   bool remoteObject  = stream->readFlag();
   faceCam       = stream->readFlag();
   followCam     = stream->readFlag();
   producesSmoke = stream->readFlag();

   stream->read( sizeof(soundID), (void *) &soundID);
   stream->read( sizeof(shapeTag), (void *) &shapeTag);
   stream->read( sizeof(translucentShapeTag), (void *) &translucentShapeTag);

   UInt16 ftime;
   stream->read( sizeof(ftime), (void *) &ftime);
   fireOut = ftime;

   if (producesSmoke)
   {
      stream->read( sizeof(smokeShapeTag), (void*) &smokeShapeTag);
      stream->read( sizeof(smokeTransShapeTag), (void*) &smokeTransShapeTag);
      stream->read( sizeof(smokeToSmoke), (void*) &smokeToSmoke);
      stream->read( sizeof(smokeOut), (void*) &smokeOut);
      stream->read( sizeof(smokeOffset), (void*) &smokeOffset);
      stream->read( sizeof(smokeVel), (void*) &smokeVel);
   }

   stream->read( sizeof(pos), (void *) &pos );

   if (remoteObject)
   {
      int remoteID;
      stream->read( sizeof(remoteID), (void *) &remoteID);
      onObj = gm->resolveGhost(remoteID);
   }
}

DWORD SimFire::packUpdate(Net::GhostManager * gm, DWORD, BitStream *stream)
{
   stream->writeFlag(onObj);
   stream->writeFlag(faceCam);
   stream->writeFlag(followCam);
   stream->writeFlag(producesSmoke);

   stream->write( sizeof(soundID), (void *) &soundID);
   stream->write( sizeof(shapeTag), (void *) &shapeTag);
   stream->write( sizeof(translucentShapeTag), (void *) &translucentShapeTag);

   UInt16 ftime = fireOut;
   stream->write( sizeof(ftime), (void *) &ftime);

   if (producesSmoke)
   {
      stream->write( sizeof(smokeShapeTag), (void*) &smokeShapeTag);
      stream->write( sizeof(smokeTransShapeTag), (void*) &smokeTransShapeTag);
      stream->write( sizeof(smokeToSmoke), (void*) &smokeToSmoke);
      stream->write( sizeof(smokeOut), (void*) &smokeOut);
      stream->write( sizeof(smokeOffset), (void*) &smokeOffset);
      stream->write( sizeof(smokeVel), (void*) &smokeVel);
   }

   stream->write( sizeof(pos), (void *) &pos );

   if (onObj)
   {
      int remoteID = gm->getGhostIndex(static_cast<SimNetObject*>(onObj));
      stream->write( sizeof(remoteID), (void *) &remoteID);
   }
   return 0;
}


//------------------------------------------------------------------------------
// SimFireTable
//------------------------------------------------------------------------------

SimFireTable::SimFireTable()
{
   id = SimDefaultFireTableId;
}

SimFireTable::~SimFireTable()
{
}

bool SimFireTable::processEvent(const SimEvent *)
{
   return ( false );
}

bool SimFireTable::processQuery(SimQuery *)
{
   return ( false );
}

SimFireTable* SimFireTable::find(SimManager* io_pManager)
{
   AssertFatal(io_pManager != NULL, "No manager to find table!");

   return (SimFireTable*)io_pManager->findObject(SimDefaultFireTableId);
}

const SimFireRecord* SimFireTable::lookup(Int32 id, SimManager* io_pManager)
{
   SimFireTable * et = SimFireTable::find(io_pManager);
   AssertFatal(et,"SimFireTable::lookup: no Fire table loaded");
   return et->lookup(id);
}

const SimFireRecord* SimFireTable::lookup(Int32 id)
{
   for (int i=0; i<table.size(); i++)
      if (table[i].id==id)
         return &table[i];
   return 0;
}

void SimFireTable::add(const SimFireRecord & item, SimManager* io_pManager)
{
   SimFireTable * et = SimFireTable::find(io_pManager);
   if (!et)
      et = new SimFireTable();
   et->add(item);
}

void SimFireTable::add(const SimFireRecord & item)
{
   table.increment();
   table.last() = item;
}

bool SimFireTable::load(int tableTagID,SimManager * mgr)
{
   const char* tableName = SimTagDictionary::getString(mgr,tableTagID);
   return load(tableName,mgr);
}

bool SimFireTable::load(const char * fileName,SimManager *mgr)
{
   SimFireTable * ft = SimFireTable::find(mgr);
   if (!ft)
   {
      ft = new SimFireTable();
      mgr->addObject(ft);
   }
      
   return ft->load(fileName);
}

bool SimFireTable::load(const char * fileName)
{
   Resource<RawData> rawData = SimResource::get(manager)->load(fileName);

   if (!(bool)rawData)
      return false;

   SimFireRecord *data = (SimFireRecord *) (*rawData).data;
   int sz = (*rawData).size / sizeof(SimFireRecord);
   SimFireRecord *end  = data + sz;

   for (;data!=end; data++)
   {
      table.increment();
      table.last() = *data;
   }

   return true;
}

void SimFireTable::onPreLoad(SimPreLoadManager *splm)
{
   for (int i =0; i < table.size(); i++)
   {
      SimFireRecord &entry = table[i];
      if (entry.shapeTag)
      {
         const char *name = SimTagDictionary::getString(entry.shapeTag);
         if (name) splm->preLoadTSShape(name);
      }
      if (entry.shapeTagT)
      {
         const char *name = SimTagDictionary::getString(entry.shapeTagT);
         if (name) splm->preLoadTSShape(name);
      }
      if (entry.smokeTag)
      {
         const char *name = SimTagDictionary::getString(entry.smokeTag);
         if (name) splm->preLoadTSShape(name);
      }
      if (entry.smokeTagT)
      {
         const char *name = SimTagDictionary::getString(entry.smokeTagT);
         if (name) splm->preLoadTSShape(name);
      }
   }
}   

bool SimFireTable::processArguments(int  io_argc,
                                         const char** in_argv)
{
   if(io_argc != 1) {
      Console->printf("SimFireTable: <FireTableName>.dat");
      return false;
   }

   if(load(in_argv[0]) == false) {
      Console->printf("SimFireTable: unable to load table %s", in_argv[0]);
      return false;
   }

   return true;
}

IMPLEMENT_PERSISTENT(SimFireTable);

