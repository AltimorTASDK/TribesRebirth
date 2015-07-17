//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <d_caps.h>
#include <g_surfac.h>
#include <SimTagDictionary.h>
#include <SimPersistTags.h>
#include <NetPacketStream.h>
#include <NetGhostManager.h>
#include <NetEventManager.h>
#include "soundFX.h"
#include "SimExplosion.h"
#include "simResource.h"
#include "simMovement.h"
#include "simGame.h"

//------------------------------------------------------------------------------
// Sequence of events that occur when explosion created:
// 1.  constructor
// 2.  addNotify -- loads SimExplosionRecord, add to timer group
// 3.  timer update events -- ignored until detonation time is reached
//     on detonation add small bbox to container db, play sound
// 4.  after detonation we get timer updates and render queries
//     on 1st render query, we load shape (either translucent or other version,
//     depending on whether we're in hardware or software and default settings),
//     re-figure bbox size (but don't change in container db -- can't do that while
//     in render query).  Also, first time through we face the camera and
//     adjust our position to get away from surface (ground or whatever).
//     On timer events we basically do nothing unless we are attached to an object.
//     In that case, we update our position and bbox in container db.
// 5.  Explosion is removed from manager when animation plays through once (happens
//     in rener query since that is where we advance the animation).

//------------------------------------------------------------------------------

// a random number generator for explosion code
Random g_expRand;

bool SimExplosion::SoftwareTranslucency = false;
bool SimExplosion::DynamicLighting = true;
int  SimExplosion::defaultFxObjectType = SimDefaultObjectType;

#define maxShiftRadius 3.0f     // never shift pos of explosion more than 3x radius
#define maxShiftCamAxis 0.3f    // never shift pos of explosion more than 30% of
                                // distance from camera

//--------------------------------------------------------------------------- 

SimExplosion::SimExplosion( int _id, int _index)
{
   type |= defaultFxObjectType;

   info = NULL;
   explosionID = _id;
	index = _index;
	AssertFatal(explosionID==-1 || index==-1,"SimExplosion::SimExplosion:  either id or index must be -1");

   pos.set(0, 0, 0);
   onObj = 0;
	backface=false;
	shiftPos = false;
	yaxis.set(0,1,0);

   detonateTime = 0;
   detonating = false;
	renderedYet = false;
	updateBox = false; // set to true when shape first loaded because we can't update
                     // bbox size in container db while in render query

	soundOn = true;
	m_disableLight = false;
}

void SimExplosion::setPosition(const Point3F & _pos)
{ 
	onObj=0; 
	pos=_pos;
   lightPos = pos;
}

void SimExplosion::setObject(SimObject *obj,const Point3F & _offset)
{
	onObj=obj; 
	pos=_offset;
   lightPos = pos;
}

void SimExplosion::setAxis(const Point3F & _axis, bool _backface)
{ 
	if (m_dot(_axis,_axis)>.1f) // if shorter than this, we assume 0 vector
	{
		yaxis=_axis;
		yaxis.normalize();
		shiftPos = true;
		backface=_backface;
	}
}

void SimExplosion::setDetonationTime(float _time)
{ 
	detonateTime=_time;
	if (manager)
		detonateTime += manager->getCurrentTime();
}

void SimExplosion::setSound(bool s)
{
	soundOn = s;
}
   
  
//--------------------------------------------------------------------------- 
bool SimExplosion::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimTimerEvent);
   }
   return false;
}

//------------------------------------------------------------------------------
bool SimExplosion::onAdd()
{
	if (!Parent::onAdd())
		return false;

	// we may have been sent over the net (packUpdate and unpackUpdate)
	// in which case our packet may not have arrived yet -- test for that here
	if (explosionID!=-1 || index!=-1)
		if (finishAddToManager() == false)
         return false;

   if (info->spin)
      image.setAxisSpin(g_expRand.getFloat(float(M_2PI)));  // rotate randomly

  if (info->hasLight != 0 && !m_disableLight && SimExplosion::DynamicLighting) {
      m_light.setType ( TS::Light::LightPoint );
      m_dynamicLightColor0.set(info->r0, info->g0, info->b0);
      m_dynamicLightColor1.set(info->r1, info->g1, info->b1);
      m_dynamicLightColor2.set(info->r2, info->g2, info->b2);
      m_radFactor0 = info->radFactor0;
      m_radFactor1 = info->radFactor1;
      m_radFactor2 = info->radFactor2;
      m_dynamicLightRange = info->range;
      addToSet(SimLightSetId);
   }
	
	return true;
}

bool SimExplosion::finishAddToManager()
{
 	AssertFatal(explosionID!=-1 || index!=-1,
		"SimExplosion::onSimAddNotifyEvent:  either id or index must be set.");

	if (explosionID!=-1)
		info = SimExplosionTable::lookup(explosionID,manager);
	else
	{
		info = SimExplosionTable::get(index,manager);
		explosionID = info->id;
	}

   if (info == NULL) {
      AssertWarn(0, "SimExplosion::onSimAddNotifyEvent: invalid id");
      return false;
   }

	if (!info->faceCam) // if faceCam, set rmat during first render
		image.faceDirection(yaxis);

   if (onObj)
      deleteNotify(onObj);

	// add to timer Set -- unless we are already in a container
	// if we are, then we are in an explosion cloud, and will
	// get timer updates via the cloud (so no need to add ourself
	// to the timer set).
	if (!getContainer())
		addToSet(SimTimerSetId);

	detonateTime += manager->getCurrentTime();

	// don't know actual explosion duration till detonation,
	// so set 5 sec duration for now so object eventually
	// gets erased even if it is never rendered
	expireTime = detonateTime + 5.0f;

	getPosition(image.transform.p);

   return true;
}

bool SimExplosion::onSimTimerEvent(const SimTimerEvent *)
{
	float curTime = manager->getCurrentTime();

	if (curTime>expireTime)
	{
		deleteObject();
		return true;
	}

	if (!detonating && curTime>detonateTime)
	{
		detonate();
		detonating = true;
	}

	if (!detonating)
		return true;

	// update position and move (in containers) -- only if attached to object
	if (onObj || updateBox)
	{
		Point3F & p = image.transform.p;
		getPosition(p);
		updateBBox(p);
		updateBox=false; // just updated it
	}

	return true;
}
 
//------------------------------------------------------------------------------
void SimExplosion::onRemove()
{
	// remove from whatever container we're in
	if (getContainer())
		getContainer()->removeObject(this);

   if (image.translucentShape)
      delete image.translucentShape;

   if (image.nonTranslucentShape)
      delete image.nonTranslucentShape;

   Parent::onRemove();
}

//------------------------------------------------------------------------------
void SimExplosion::onDeleteNotify(SimObject* /*object*/)
{
      onObj = NULL;
}

bool
SimExplosion::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if ((float)manager->getCurrentTime() > expireTime)
   	return false;

   SimRenderQueryImage* qp = query;
   TSRenderContext & rc = *qp->renderContext;
   GFXSurface *srf = rc.getSurface();

   // shall we draw translucent?
   bool translucent = (srf->getCaps()&GFX_DEVCAP_SUPPORTS_CONST_ALPHA || 
                       SoftwareTranslucency ||
   	                 info->shapeTag==-1)
   	                && info->translucentShapeTag!=-1;

   // if no explosion to display, then don't display it                      
   if( translucent && info->translucentShapeTag == -1 )
      return false;
      
   if( !translucent && info->shapeTag == -1)
      return false;                      


   if (!renderedYet || translucent!=image.drawTranslucent)
   {
   	// need to switch modes
   	if (translucent && !image.translucentShape)
   		// need to load translucent shape
   		getShape(info->translucentShapeTag,
   		         &image.translucentShape,
   		         &image.translucentThread);
   	else if (!translucent && !image.nonTranslucentShape)
   		// need to load non-translucent shape
   		getShape(info->shapeTag,
   		         &image.nonTranslucentShape,
   		         &image.nonTranslucentThread);

   	image.setTranslucent(translucent);

   	if (!renderedYet)
   	{
   		// got right radius (above)
   		// update bounding box in container db next timer update
   		updateBox = true;
   		// set start of animation based on detonation time
   		prevAnimTime = detonateTime;
   		// got right duration (above)
   		// set expireTime based on this
   		expireTime = detonateTime +
   		             info->timeScale * image.thread->getSequence().fDuration;

         // Set up the light switch points...
         //
         if (info->hasLight != 0 && !m_disableLight && SimExplosion::DynamicLighting) {
            m_lightTime0 = detonateTime + info->t0 * (expireTime - detonateTime);
            m_lightTime1 = detonateTime + info->t1 * (expireTime - detonateTime);
         }
         
   		if ((float)manager->getCurrentTime() > expireTime)
   			return false; // oops, we should be gone already...
   	}
   }

   if (!renderedYet)
   {
   	renderedYet=true;

   	// find line from camera to object
   	image.findCameraAxis(rc);

   	// shift object along camera axis to be away from surface
   	if (shiftPos && !info->preventShiftPos)
   	{
   		float dot = m_dot(image.getCameraAxis(),yaxis);
   		if (dot < 0 ) // don't shift if exp. on other side of surface
   		{
   			if (dot > -1/maxShiftRadius)
   				dot = -maxShiftRadius;
   			else
   				dot = 1/dot;
   			// don't let shape move more than
   			// some % of distance from camera 
   			float shiftDist = image.shape->getShape().fRadius * dot;
   			if (-shiftDist > image.getCameraDistance() * maxShiftCamAxis) 
   				shiftDist = -image.getCameraDistance() * maxShiftCamAxis;
   		                                                 
   			Point3F getAway = image.getCameraAxis();
   			getAway *= shiftDist;
   			pos += getAway;
   			getPosition(image.transform.p);
   		}
   	}

   	if (backface && image.testBackfacing(yaxis))
   	{
   		manager->deleteObject( this );
   		return false;
   	}

   	if (info->faceCam)
   	{
   		image.faceCamera(rc,false); // face camera -- don't need to get axis again
   	}      
   }

   // if playing a recording, keep facing camera every render
   if (SimGame::isPlayback && renderedYet && info->faceCam)
      image.faceCamera(rc,true);

   // advance the animation (based on last render, not last update)
   float dTime = (float) manager->getCurrentTime() - prevAnimTime;
   prevAnimTime = manager->getCurrentTime();
   image.thread->AdvanceTime(dTime);

   // get haze value from container
	SimContainer * cnt = getContainer();
   AssertFatal(cnt, "SimExplosion::onSimRenderQueryImage: somehow no container");
	if (cnt)
		image.hazeValue = cnt->getHazeValue(rc, image.transform.p);

   // set image
   image.setSortValue(qp->renderContext->getCamera());
   qp->count = 1;
   qp->image[0] = &image;

   return true;
}

//--------------------------------------------------------------------------- 
bool SimExplosion::processQuery(SimQuery *query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);
      onQuery(SimLightQuery);

     default:
      return Parent::processQuery(query);
   }
}

//--------------------------------------------------------------------------- 

void SimExplosion::getShape(int shapeTag,
                                    TSShapeInstance **ppShape,
                                    TSShapeInstance::Thread **ppThread)
{
   // Read shape, initialize thread
   ResourceManager *rm = SimResource::get(manager);
	const char* shapeName = SimTagDictionary::getString(manager,shapeTag);
   hShape = rm->load(shapeName);
   AssertFatal((bool)hShape,"Unable to load explosion shape");
   
   *ppShape = new TS::ShapeInstance(hShape, *rm);
   AssertFatal((bool)*ppShape,"Could not create explosion shape instance");

   *ppThread = (*ppShape)->CreateThread();
//   (*ppThread)->setTimeScale((*ppThread)->getSequence().fDuration/info->duration);
   (*ppThread)->setTimeScale(info->timeScale);
   (*ppThread)->SetPosition(0);
	(*ppShape)->animateRoot();

	// set-up shape space bounding box -- real size
	bbox.fMin = (*ppShape)->getShape().fCenter;
	bbox.fMin += (*ppShape)->fRootDeltaTransform.p; // could be trouble if root has rot.
	bbox.fMax = bbox.fMin;
	bbox.fMin -= (*ppShape)->getShape().fRadius;
	bbox.fMax += (*ppShape)->getShape().fRadius;
}

//------------------------------------------------------------------------------
void SimExplosion::detonate()
{
	// don't know shape size yet, fake it for now
	bbox.fMin.set(-1,-1,-1);
	bbox.fMax.set(1,1,1);
	image.transform.identity();
	image.transform.flags |= TMat3F::Matrix_HasTranslation; // keep this flag set
	Point3F & p = image.transform.p;
	getPosition(p);
	updateBBox(p);

	// add to container db so we get render query
	// may already be in a container (ExplosionCloud container, in
	// which case we leave it alone).
	if (!getContainer())
	{
		SimContainer *root = NULL;
      
      root = findObject(manager, SimRootContainerId,root);
		root->addObject(this);
	}

   if (soundOn && info->soundID != EXP_nosound)
   {
      Point3F p;
      getPosition(p);
      Sfx::Manager::PlayAt( manager, info->soundID, TMat3F(EulerF(0, 0, 0), p), Point3F(0, 0, 0));
   }
}

void SimExplosion::updateBBox(const Point3F & p)
{
	Box3F box;
	box = bbox;
	box.fMin += p;
	box.fMax += p;
	setBoundingBox(box);
}

void SimExplosion::getPosition(Point3F &position)
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

void SimExplosion::getLightPosition(Point3F &position)
{
   position = lightPos;
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

SimExplosion::SimExplosion(const SimExplosion &explosion)
{
   info = NULL;
   explosionID = explosion.explosionID;
	index = explosion.index;
   pos = explosion.pos;
   onObj = 0;
	backface = explosion.backface;
	shiftPos = explosion.shiftPos;
	yaxis = explosion.yaxis;

   detonateTime = explosion.detonateTime;
	if (explosion.manager)
		detonateTime -= explosion.manager->getCurrentTime();

   detonating = false;
	renderedYet = false;
	updateBox = false; // set to true when shape first loaded because we can't update
	                   // bbox size in container db while in render query

	soundOn = explosion.soundOn;
	m_disableLight = explosion.m_disableLight;
}

void SimExplosion::createOnClients(SimExplosion* explosion, SimManager* mgr, float scopeDist)
{
   SimSet * packetStreams = static_cast<SimSet*>(mgr->findObject(PacketStreamSetId));
   if (packetStreams)
   {
      for (SimSet::iterator itr = packetStreams->begin(); itr != packetStreams->end(); itr++)
      {
         Net::PacketStream * pStream = dynamic_cast<Net::PacketStream *>(*itr);
         if (!pStream)
            break;
         Net::GhostManager * gm = pStream->getGhostManager();
         if (gm->getCurrentMode() == Net::GhostManager::GhostNormalMode)
         {
            #ifdef debug
            SimMovement * scope = dynamic_cast<SimMovement*>(gm->getScopeObject());
            AssertFatal(scope,
               "SimDebrisCloud::createOnClients:  scope object not SimMovement...better change this then.");
            #else
            SimMovement * scope = static_cast<SimMovement*>(gm->getScopeObject());
            #endif
            Point3F offset = scope->getTransform().p - explosion->pos;
            if (m_dot(offset,offset) > scopeDist * scopeDist)
               break;
            Net::RemoteCreateEvent * addEvent = new Net::RemoteCreateEvent;
            SimExplosion *explosionCopy = new SimExplosion(*explosion);
            addEvent->newRemoteObject = explosionCopy;
            // if attached to an object, see if it is in scope...
            if (explosion->onObj)
            {
               int gidx = pStream->getGhostManager()->
                           getGhostIndex(static_cast<SimNetObject*>(explosion->onObj));
               if (gidx==-1)
                  continue; // object must be out of scope
            }
            pStream->getEventManager()->postRemoteEvent(addEvent);
         }            
      }
   }
   if (!explosion->manager)
      // this was a temporary object
      delete explosion;
}

//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
// Standard Persistent IO stuff
//
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 


IMPLEMENT_PERSISTENT_TAG( SimExplosion, SimExplosionPersTag );

void SimExplosion::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
	explosionID = stream->readInt( 20 );
	index=-1;

   if (stream->readFlag())
   {
   	UInt8 dtime;
	   stream->read( sizeof(dtime), (void *) &dtime);
	   detonateTime = ((float) dtime) / 10.0f;
   }
   else
      detonateTime = 0.0f;

	stream->read( sizeof(pos), (void *) &pos );

   stream->readNormalVector(&yaxis, 4);

	backface     = stream->readFlag();
	bool remoteObject = stream->readFlag();
	soundOn      = stream->readFlag();
	shiftPos     = stream->readFlag();

	if (remoteObject)
	{
		int remoteID = stream->readInt(10);
		onObj = gm->resolveGhost(remoteID);
	}

	// initialize a few variables
	detonating=renderedYet=false;
   lightPos = pos;   // ensure lights are in a sane place on client

	if (manager)
		finishAddToManager();
}

DWORD SimExplosion::packUpdate(Net::GhostManager *gm, DWORD, BitStream *stream)
{
   stream->writeInt( explosionID,20); // limits range a bit but saves 12 bits

	UInt8 dtime;
	if (manager)
		dtime = (detonateTime-manager->getCurrentTime()) * 10;
	else
		dtime = detonateTime * 10;
   stream->writeFlag(dtime);
   if (dtime)
   	stream->write( sizeof(dtime), (void *) &dtime);

	stream->write( sizeof(pos), (void *) &pos );

   stream->writeNormalVector(&yaxis, 4);

	stream->writeFlag(backface);
	stream->writeFlag(onObj);
	stream->writeFlag(soundOn);
	stream->writeFlag(shiftPos);

	if (onObj)
	{
		int remoteID = gm->getGhostIndex(static_cast<SimNetObject*>(onObj));
		stream->writeInt(remoteID,10);
	}
   return 0;
}


bool
SimExplosion::onSimLightQuery ( SimLightQuery * query )
{
   query->ambientIntensity.set();
   
   if (!renderedYet ||
       info->hasLight == 0 ||
       m_disableLight ||
       manager->getCurrentTime() > expireTime ||
       !SimExplosion::DynamicLighting)
   {
      query->count = 0;
      query->light[0] = NULL;
   } else {
	   query->count = 1;
      query->light[0] = &m_light;
      
      if (onObj != NULL) {
         AssertWarn(0, "Not yet done...");
      }

      Point3F tempPos;
      getLightPosition(tempPos);
      m_light.setPosition( tempPos );
      
      // Determine the multiplying factor for intensity and range...
      //
      ColorF color;
      float factor;
      float currTime = manager->getCurrentTime();

      if (currTime < m_lightTime0) {
         float factor2 = (m_lightTime0 - currTime) / (m_lightTime0 - detonateTime);

         factor = m_radFactor1 + factor2 * (m_radFactor0 - m_radFactor1);
         color.interpolate(m_dynamicLightColor1, m_dynamicLightColor0, factor2);

      } else if (currTime <= m_lightTime1 &&
                 m_lightTime1 != m_lightTime0) {

         float factor2 = (m_lightTime1 - currTime) / (m_lightTime1 - m_lightTime0);

         factor = m_radFactor1 + factor2 * (m_radFactor2 - m_radFactor1);
         color.interpolate(m_dynamicLightColor2, m_dynamicLightColor1, factor2);
      } else {
         AssertFatal(m_lightTime1 != expireTime, "Should be in the case before this.");

         factor = (expireTime - currTime) / (expireTime - m_lightTime1);
         color = m_dynamicLightColor2 * factor;
         factor *= m_radFactor2;
      }

      m_light.setIntensity(color);
      m_light.setRange(m_dynamicLightRange * factor);
   }

	return true;
}

