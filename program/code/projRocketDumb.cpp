//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <netPacketStream.h>
#include <stringTable.h>
#include <simResource.h>
#include "explosion.h"
#include "dataBlockManager.h"
#include "projRocketDumb.h"
#include "fear.strings.h"
#include "fearDcl.h"
#include "fearForceDcl.h"
#include "projAttTrail.h"
#include "smokePuff.h"
#include "fearGlobals.h"
#include "netGhostManager.h"
#include "netPacketStream.h"
#include "console.h"

IMPLEMENT_PERSISTENT_TAG(RocketDumb, RocketDumbPersTag);

//------------------------------------------------------------------------------
//-------------------------------------- Rocket dumb impl.
//
RocketDumb::RocketDumbData::RocketDumbData()
{
   trailType   = None;
   trailString = NULL;
   trailLength = 10.0;
   trailWidth  = 0.3;
}

void
RocketDumb::RocketDumbData::pack(BitStream* bs)
{
	Parent::pack(bs);

   bs->write(trailType);
   bs->writeString(trailString);
   bs->write(trailLength);
   bs->write(trailWidth);
}

void
RocketDumb::RocketDumbData::unpack(BitStream* bs)
{
	Parent::unpack(bs);

   bs->read(&trailType);
   trailString = bs->readSTString();
   bs->read(&trailLength);
   bs->read(&trailWidth);
}

bool
RocketDumb::RocketDumbData::preload(ResourceManager* rm,
                                    bool             server,
                                    char             errorBuffer[256])
{
   if (Parent::preload(rm, server, errorBuffer) == false)
      return false;

   if (server == false) {
      if (trailType == Swoosh) {
         trailBitmap = rm->load("trailAdd2.bmp");
         if(bool(trailBitmap) == false) {
            sprintf(errorBuffer, "Unable to load Trail Bitmap: %s", "trailAdd2.bmp");
            return false;
         }
      } else if (trailType == Smoke) {
         smokeShape = rm->load(trailString);
         if(bool(smokeShape) == false) {
            sprintf(errorBuffer, "Unable to load Smoke Shape: %s", trailString);
            return false;
         }
      }
   }

   return true;
}

float
RocketDumb::RocketDumbData::getTerminalVelocity() const
{
   return terminalVelocity;
}

bool
RocketDumb::RocketDumbData::getTargetVector(const Point3F& in_rShootFrom,
                                            const Point3F& in_rShootTo,
                                            const float    /*in_gravity*/,
                                            Point3F*       out_pVector,
                                            bool*          out_pInRange,
                                            const bool     in_secondTarget)
{
   if (in_secondTarget == true)
      return false;

   *out_pVector = (in_rShootTo - in_rShootFrom);
   float dist = out_pVector->len();
   out_pVector->normalize();

   if (dist < (getTerminalVelocity() * totalTime))
      *out_pInRange = true;
   else
      *out_pInRange = false;

   return true;
}

//------------------------------------------------------------------------------
//--------------------------------------
// RocketDisk implementation...
//--------------------------------------
//
RocketDumb::RocketDumb(const Int32 in_datFileId)
 : Projectile(in_datFileId),
   m_pSwooshImage(NULL),
   m_accelVector(0, 0, 0),
   m_timeoutExplosion(false)
{
   //
   netFlags.set(Ghostable);
}

RocketDumb::RocketDumb()
 : Projectile(-1),
   m_pSwooshImage(NULL),
   m_accelVector(0, 0, 0),
   m_timeoutExplosion(false)
{
   //
   netFlags.set(Ghostable);
}

int 
RocketDumb::getDatGroup()
{
   return DataBlockManager::RocketDataType;
}

RocketDumb::~RocketDumb()
{
   delete m_pSwooshImage;
   m_pSwooshImage = NULL;
}

bool
RocketDumb::initResources(GameBase::GameBaseData* io_pDat)
{
   if(!Parent::initResources(io_pDat))
      return false;

   m_pRocketData = dynamic_cast<RocketDumbData*>(io_pDat);

   return true;
}

bool
RocketDumb::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
   
   // Set up the mass of the projectile...
   //
	setMass(m_projectileData->mass);

   // We need to setup the accel vector...
   //
   m_accelVector = getLinearVelocity();
   m_accelVector.normalize();
   m_accelVector *= m_projectileData->acceleration;
   addForce(TribesRocketForce, m_accelVector);
   
   if (isGhost() == true) {
      setTerminalVelocity();
      m_renderImage.setTranslucent(true);
      
      if (m_pRocketData->trailType == RocketDumbData::Swoosh) {
         m_pSwooshImage = new SimpleSwooshTrail(m_pRocketData->trailLength,
                                                m_pRocketData->trailWidth);
         m_pSwooshImage->setResources(m_pRocketData->trailBitmap);
      }

      m_lastPuffPos = getLinearPosition();

      // Need to sim forward to account for lag...
      //
      m_lastUpdated = wg->currentTime;
      m_startInterp = wg->currentTime;
      m_endInterp   = wg->currentTime + wg->interpolateTime;
      m_interpFrom  = getLinearPosition();
      predictForward(wg->currentTime,
                     wg->currentTime + wg->predictForwardTime,
                     m_interpTo);
   } else {
      m_lastGhosted = wg->currentTime;
   }

   return true;
}

void
RocketDumb::buildContainerBox(const TMat3F& in_rMat,
                                 Box3F*        out_pBox)
{
   // Build a bounding box that is rooted at the current position,
   //  and extends out to allow half a second of motion by the bullet...
   //
   out_pBox->fMin = in_rMat.p;
   out_pBox->fMax = in_rMat.p;
   if (m_projectileData != NULL) {
      out_pBox->fMin -= Point3F(m_projectileData->collisionRadius, m_projectileData->collisionRadius, m_projectileData->collisionRadius);
      out_pBox->fMax += Point3F(m_projectileData->collisionRadius, m_projectileData->collisionRadius, m_projectileData->collisionRadius);
   }
}

void
RocketDumb::writeInitialPacket(Net::GhostManager* gm,
                                   BitStream*         pStream)
{
   Parent::writeInitialPacket(gm, pStream);

   // Three things are sent with the initial packet:
   //  1. Position (initial)
   //  2. Velocity
   //  3. Shooter's GhostId (not yet)
   //
   Point3F  position = getLinearPosition();
   Vector3F velocity = getLinearVelocity();
   
   pStream->write(sizeof(position), &position);
   pStream->write(sizeof(velocity), &velocity);
}

void
RocketDumb::readInitialPacket(Net::GhostManager* gm,
                                  BitStream*         pStream)
{
   Parent::readInitialPacket(gm, pStream);

   Point3F  position;
   Vector3F velocity;
   
   pStream->read(sizeof(position), &position);
   pStream->read(sizeof(velocity), &velocity);

   setLinearVelocity(velocity);

   TMat3F transform = getTransform();
   transform.p = position;
   setPosition(transform, true);

   velocity.normalize();
   m_renderImage.faceDirection(velocity);
}


DWORD
RocketDumb::packUpdate(Net::GhostManager* gm,
                             DWORD              mask,
                             BitStream*         stream)
{
   // We care only about one bit, position, since the velocity and rotation never
   //  change.  Send over velocity only on the initialPacket.  Note that the
   //  Parent::packUpdate() call skips entirely over the Projectile, which
   //  set responsibility for all ghosting on the derived class...
   //
   DWORD parentPunt = Parent::packUpdate(gm, mask, stream);
   
   if (mask & Projectile::InitialUpdate) {
      stream->writeFlag(true);
      writeInitialPacket(gm, stream);

      if (m_hasExploded == true)
         parentPunt |= FPExplosionMask;
      return parentPunt;
   } else {
      stream->writeFlag(false);
   
      if (mask & Projectile::FPPositionMask) {
         stream->writeFlag(true);
         
         Point3F position = getLinearPosition();
         stream->write(sizeof(position), &position);
      } else {
         stream->writeFlag(false);
      }
   
      stream->writeFlag(m_projectileIsLive);

      return parentPunt;
   }
}

void
RocketDumb::unpackUpdate(Net::GhostManager* gm,
                               BitStream*         stream)
{
   Parent::unpackUpdate(gm, stream);
   
   if (stream->readFlag() == true) {
      // Is initial packet...
      //
      readInitialPacket(gm, stream);
   } else {
      // Is normal packet
      //
      if (stream->readFlag() == true) {
         // Position
         //
         Point3F position;
         stream->read(sizeof(position), &position);
         
         TMat3F transform = getTransform();
         transform.p = position;
         setPosition(transform, true);
      }

      m_projectileIsLive = stream->readFlag();
      if (m_projectileIsLive == false) {
         forceMask |= SimMovementGravityForce;
         if (isForceActive(TribesRocketForce) == true)
            removeForce(TribesRocketForce);
      } else {
         forceMask &= ~SimMovementGravityForce;
         if (isForceActive(TribesRocketForce) == false)
            addForce(TribesRocketForce, m_accelVector);
      }

      // Need to sim forward to account for lag...
      //
      m_lastUpdated = wg->currentTime;
      m_startInterp = wg->currentTime;
      m_endInterp   = wg->currentTime + wg->interpolateTime;
      m_interpFrom  = getLinearPosition();
      predictForward(wg->currentTime,
                     wg->currentTime + wg->predictForwardTime,
                     m_interpTo);
   }
}

void
RocketDumb::setTerminalVelocity()
{
   AssertFatal(isGhost() == true, "Only the client needs to perform this calculation");
   
   m_instTerminalVelocity = getLinearVelocity();
   m_instTerminalVelocity *= (m_projectileData->terminalVelocity / m_projectileData->muzzleVelocity);
}

void
RocketDumb::predictForward(const DWORD in_currTime,
                           const DWORD in_predictTime,
                           Point3F&    out_rPosition)
{
   float updateSecs = float(in_predictTime - in_currTime) * 0.001f;
   float accelSecs  = 0.0f;
   float afterSecs  = 0.0f;

   // Have to be a bit careful here, since the rocket can have it's acceleration
   //  turned off in mid-update...
   //
   if (m_projectileIsLive == true) {
      Point3F curVel = getLinearVelocity();
      if (fabs(curVel.x) >= fabs(m_instTerminalVelocity.x) &&
          fabs(curVel.y) >= fabs(m_instTerminalVelocity.y) &&
          fabs(curVel.z) >= fabs(m_instTerminalVelocity.z)) {
         accelSecs = 0.0f;
         afterSecs = updateSecs;
      } else {
         
         accelSecs = ((m_instTerminalVelocity - curVel.x).len() / m_accelVector.len());
         if (accelSecs > updateSecs) {
            accelSecs = updateSecs;
            afterSecs = 0.0f;
         } else {
            afterSecs = updateSecs - accelSecs;
         }
      }
   }

   Point3F finalPos = getLinearPosition();
   Point3F finalVel = getLinearVelocity();

   if (accelSecs > 0.0f) {
      addForce(TribesRocketForce, m_accelVector);

      finalPos += m_accelVector * (0.5 * accelSecs * accelSecs);
      finalVel += m_accelVector * accelSecs;
   }
   
   if (afterSecs > 0.0f) {
      removeForce(TribesRocketForce);
      setLinearVelocity(m_instTerminalVelocity);
      finalVel = m_instTerminalVelocity;

      finalPos += m_instTerminalVelocity * afterSecs;
   }

   out_rPosition = finalPos;
   setLinearVelocity(finalVel);
}

void
RocketDumb::clientProcess(DWORD in_currTime)
{
   if (m_hasExploded == true)
      return;
   if (in_currTime == m_lastUpdated)
      return;
   if (m_renderImage.thread != NULL) {
      float deltaT = float(in_currTime - m_lastUpdated) / 1000.0f;
      m_renderImage.thread->AdvanceTime(deltaT);
   }


   if (in_currTime <= m_endInterp) {
      Int32 intTimeSlice   = Int32(in_currTime - m_lastUpdated);

      while (m_lastUpdated + intTimeSlice <= in_currTime) {
         // Do interpolation...
         float factor          = float(m_lastUpdated - m_startInterp) /
                                 float(m_endInterp - m_startInterp);
         TMat3F finalTransform = getTransform();
         Point3F finalPos;
         finalPos.interpolate(m_interpFrom, m_interpTo, factor);

         SimCollisionInfo  collisionInfo;
         SimContainerQuery collisionQuery;
         collisionQuery.type   = -1;
         collisionQuery.mask   = csm_collisionMask;
         collisionQuery.detail = SimContainerQuery::DefaultDetail;
         collisionQuery.box.fMin = getTransform().p;
         collisionQuery.box.fMax = finalPos;

         if (m_projectileData->collideWithOwner == false || m_ownerGraceMS > 0)
            collisionQuery.id = getOwnerId();
         else
            collisionQuery.id = -1;
         decayGraceTime(intTimeSlice);

         SimContainer* root = findObject(manager, SimRootContainerId, root);
         AssertFatal(root != NULL, "No root object...");
      
         if (root->findLOS(collisionQuery, &collisionInfo, SimCollisionImageQuery::High)) {
            // Bang!  We hit something!
            collisionInfo.transform(collisionInfo.surfaces.tWorld);

            Explosion* pExplosion = new Explosion(getExplosionTag(collisionInfo));
            pExplosion->setPosition(collisionInfo.surfaces[0].position);
            pExplosion->setAxis(collisionInfo.surfaces[0].normal);
            manager->addObject(pExplosion);
            SmokeManager::get(manager)->removeAllInRadius(collisionInfo.surfaces[0].position,
                                                          m_projectileData->explosionRadius);

            m_hasExploded    = true;
            m_overrideRender = true;
            stopProjectileSound();
            return;
         } else {
            // force position update, no collision
            //
            finalTransform.p = finalPos;
            setPosition(finalTransform, true);
            m_lastUpdated += intTimeSlice;
         }

         updateTrail();

         if (m_lastUpdated != in_currTime && UInt32(m_lastUpdated + intTimeSlice) > in_currTime) {
            intTimeSlice   = in_currTime - m_lastUpdated;
         }
      }
   } else {
      Int32 intTimeSlice   = Int32(in_currTime - m_lastUpdated);
      float floatTimeSlice = float(intTimeSlice) * 0.001f;
      if (intTimeSlice <= 0)
         return;

      setTimeSlice(floatTimeSlice);

      while (m_lastUpdated + intTimeSlice <= in_currTime) {
         // Check to see if we need to turn off the acceleration.
         // We only manage the motor if the projectile is live...
         //
         if (m_projectileIsLive == true) {
            Vector3F curVel = getLinearVelocity();
            if (fabs(curVel.x) >= fabs(m_instTerminalVelocity.x) &&
                fabs(curVel.y) >= fabs(m_instTerminalVelocity.y) &&
                fabs(curVel.z) >= fabs(m_instTerminalVelocity.z)) {
               removeForce(TribesRocketForce);
               setLinearVelocity(m_instTerminalVelocity);
            } else {
               // Turn it back on, the server overrode us...
               addForce(TribesRocketForce, m_accelVector);
            }
         }
         
         SimCollisionInfo collision;
         SimObject*       pObject = NULL;
         bool collided = moveProjectile(m_lastUpdated + intTimeSlice,
                                        collision,
                                        pObject, true, false);
         if (collided == true) {
            if (isProjStatic(pObject) == true) {
               collision.transform(collision.surfaces.tWorld);

               Explosion* pExplosion = new Explosion(getExplosionTag(collision));
               pExplosion->setPosition(collision.surfaces[0].position);
               pExplosion->setAxis(collision.surfaces[0].normal);
               manager->addObject(pExplosion);
               SmokeManager::get(manager)->removeAllInRadius(collision.surfaces[0].position,
                                                             m_projectileData->explosionRadius);

               m_hasExploded    = true;
               m_overrideRender = true;
               stopProjectileSound();
               return;
            }

            m_lastUpdated += intTimeSlice;
         }

         updateTrail();

         if (m_lastUpdated != in_currTime &&
             UInt32(m_lastUpdated + intTimeSlice) > in_currTime) {
            intTimeSlice = in_currTime - m_lastUpdated;
            floatTimeSlice = float(intTimeSlice) * 0.001f;
            setTimeSlice(floatTimeSlice);
         }
      }
   }
   updateProjectileSound(getTransform(), getLinearVelocity());
}

void
RocketDumb::serverProcess(DWORD in_currTime)
{
   // Apply the acceleration...
   //
   if (isForceActive(TribesRocketForce) == true) {
      Vector3F curVel = getLinearVelocity();
      // This comparison is sufficient, since the vectors are simply scalar multiples of
      //  each other, and is faster than len() calls...
      //
      if (fabs(curVel.x) >= fabs(m_instTerminalVelocity.x) &&
          fabs(curVel.y) >= fabs(m_instTerminalVelocity.y) &&
          fabs(curVel.z) >= fabs(m_instTerminalVelocity.z)) {
         removeForce(TribesRocketForce);
         setLinearVelocity(m_instTerminalVelocity);
      }
   }

//   // Update the position if 3/4 of a second has passed...
//   if (in_currTime - m_lastGhosted > 750) {
//      m_lastGhosted = wg->currentTime;
//      setMaskBits(FPPositionMask);
//   }

   Parent::serverProcess(in_currTime);
}

//void
//RocketDumb::killProjectile()
//{
//   // Ok, what we do here is simply add in gravity, and kill the rocket motor...
//   //
//   forceMask |= SimMovementGravityForce;
//
//   if (isForceActive(TribesRocketForce) == true)
//      removeForce(TribesRocketForce);
//
//   m_projectileIsLive = false;
//   m_lastGhosted = wg->currentTime;
//   setMaskBits(PRStatusMask);
//}

void
RocketDumb::destroyProjectile()
{
   AssertFatal(isGhost() == false, "only on server!");
   // At this point, the projectile has expired wo/ hitting anything, so we want
   //  it to self-destruct.  It's going to be deleted by the base class, so we just need
   //  to create the explosion effect...
   //

   setMaskBits(FPExplosionMask);
   m_hasExploded       = true;
   m_timeoutExplosion  = true;
}

bool
RocketDumb::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);

     default:
      return Parent::processQuery(query);
   }
}

bool
RocketDumb::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   Parent::processQuery(query);

   if (m_overrideRender == false) {
      if (m_pSwooshImage != NULL) {
         m_pSwooshImage->updateTransform(m_renderImage.transform, manager->getCurrentTime());
         m_pSwooshImage->setSortValue(query->renderContext->getCamera());
         query->image[query->count++] = m_pSwooshImage;
      }
   }
   
   return true;
}


void
RocketDumb::updateTrail()
{
   AssertFatal(isGhost() == true, "Server objects don't have trails");
   AssertFatal(manager != NULL, "Not yet added...");

   if (m_hasExploded == true || m_overrideRender)
      return;

   if (m_pRocketData->trailType == RocketDumbData::Smoke) {
      float len = (getLinearPosition() - m_lastPuffPos).len();
      int numPuffs = int(len / m_pRocketData->projSpecialTime);
      if (numPuffs == 0)
         return;

      Point3F puffNorm = (getLinearPosition() - m_lastPuffPos);
      puffNorm /= len;
      puffNorm *= m_pRocketData->projSpecialTime;

      for (int i = 0; i < numPuffs; i++) {
         m_lastPuffPos += puffNorm;
         
         SmokeManager* pSM = SmokeManager::get(manager);
         pSM->addPuff(m_pRocketData->smokeShape, 0.4, m_lastPuffPos, getLinearVelocity() * 0.25f);
      }
   }
}

void
RocketDumb::readExplosion(BitStream* io_pStream)
{
   Point3F position;
   Point3F axis;
   Int32   id;
   if (io_pStream->readFlag() == true) {
      position = getLinearPosition();
      axis     = Point3F(0, 1, 0);
      id       = getExplosionTag();
   } else {
      io_pStream->read(sizeof(Point3F), &position);
      io_pStream->readNormalVector(&axis, 5);
      id = io_pStream->readInt(wg->dbm->getDataGroupBitSize(DataBlockManager::ExplosionDataType));
   }

   if (m_hasExploded == false) {
      m_hasExploded = true;

      Explosion* pExplosion = new Explosion(id);
      pExplosion->setPosition(position);
      pExplosion->setAxis(axis);
      manager->addObject(pExplosion);
      m_hasExploded = true;
      m_overrideRender = true;
      SmokeManager::get(manager)->removeAllInRadius(position,
                                                    m_projectileData->explosionRadius);
   } 
}

void
RocketDumb::writeExplosion(BitStream* io_pStream)
{
   if (m_timeoutExplosion == true) {
      io_pStream->writeFlag(true);
   } else {
      io_pStream->writeFlag(false);

      io_pStream->write(sizeof(Point3F), &m_explosionPosition);
      io_pStream->writeNormalVector(&m_explosionAxis, 5);
      io_pStream->writeInt(m_explosionId,
                           wg->dbm->getDataGroupBitSize(DataBlockManager::ExplosionDataType));
   }
}


