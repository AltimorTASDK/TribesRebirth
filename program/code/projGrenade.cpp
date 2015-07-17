//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <m_random.h>
#include <netPacketStream.h>
#include <ts_shapeInst.h>
#include "explosion.h"
#include "projGrenade.h"
#include "fear.strings.h"
#include "fearDcl.h"
#include "simResource.h"
#include "smokePuff.h"
#include "dataBlockManager.h"
#include "fearGlobals.h"
#include "netGhostManager.h"
#include "netPacketStream.h"
#include "console.h"

IMPLEMENT_PERSISTENT_TAG(Grenade, GrenadePersTag);

//------------------------------------------------------------------------------
//--------------------------------------
//
Grenade::GrenadeData::GrenadeData()
{
   elasticity = 0.5f;
   pSmokeName = "smoke.dts";
}

void
Grenade::GrenadeData::pack(BitStream* bs)
{
   Parent::pack(bs);

   bs->write(elasticity);
   bs->writeString(pSmokeName);
}

void
Grenade::GrenadeData::unpack(BitStream* bs)
{
   Parent::unpack(bs);

   bs->read(&elasticity);
   pSmokeName = bs->readSTString();
}

bool
Grenade::GrenadeData::preload(ResourceManager* rm,
                              bool             server,
                              char             errorBuffer[256])
{
   if (Parent::preload(rm, server, errorBuffer) == false)
      return false;

   if (server == false) {
      smokeShape = rm->load(pSmokeName);
      if (bool(smokeShape) == false) {
         sprintf(errorBuffer, "Unable to load Smoke Shape: %s", pSmokeName);
         return false;
      }
   }

   return true;
}

bool
Grenade::GrenadeData::getTargetVector(const Point3F& in_rShootFrom,
                                      const Point3F& in_rShootTo,
                                      const float    in_gravity,
                                      Point3F*       out_pVector,
                                      bool*          out_pInRange,
                                      const bool     in_secondTarget)
{
   float g = in_gravity;
   float h    = in_rShootTo.z - in_rShootFrom.z;
   float d_sq = ((in_rShootTo.x - in_rShootFrom.x) * (in_rShootTo.x - in_rShootFrom.x)) +
                ((in_rShootTo.y - in_rShootFrom.y) * (in_rShootTo.y - in_rShootFrom.y));

   float v0_sq = getTerminalVelocity() * getTerminalVelocity();
   
   // First, determine if the target is in range.  Check this by the determinant...
   //
   float det = ((h * g + v0_sq) * (h * g + v0_sq)) -
               ((1 + (h*h)/(d_sq)) * g * g * d_sq);
   if (det < 0.0f) {
      *out_pVector = (in_rShootTo - in_rShootFrom);
      out_pVector->normalize();
      *out_pInRange = false;
      return true;
   } else if (det < 1 && in_secondTarget == true) {
      // solutions have converged
      return false;
   }


   float factor = (in_secondTarget == true) ? -1.0f : 1.0f;
   float v0x = ((h*g + v0_sq) + (factor * sqrt(det))) / (2 * (1 + ((h*h)/d_sq)));
   if (v0x < 0.0f) {
      *out_pVector = (in_rShootTo - in_rShootFrom);
      out_pVector->normalize();
      *out_pInRange = false;
      return true;
   }

   float v0y = sqrt(v0_sq - v0x);
   v0x = sqrt(v0x);

   // Need to determine if v0y is positive or neg.  Try both, and choose the
   //  closest.
   //
   float t = sqrt(d_sq) / v0x;
   float pos = (0.5 * g * t * t) + (t * v0y) - h;
   float neg = (0.5 * g * t * t) - (t * v0y) - h;

   if (abs(neg) < abs(pos))
      v0y *= -1;

   // So, now we have v0x and v0y, transform these into a directional vector...
   //
   Point3F planeVector = (in_rShootTo - in_rShootFrom);
   planeVector.z = 0;
   planeVector.normalize();

   *out_pVector = (planeVector * v0x) + (Point3F(0, 0, 1) * v0y);
   out_pVector->normalize();
   *out_pInRange = true;

   return true;
}

bool
Grenade::initResources(GameBase::GameBaseData* io_pDat)
{
   if(!Parent::initResources(io_pDat))
      return false;

   m_pGrenadeData = dynamic_cast<GrenadeData*>(io_pDat);
   AssertFatal(m_pGrenadeData != NULL, "No grenade data?");

   if (m_pGrenadeData == NULL)
      return false;
   return true;
}

//------------------------------------------------------------------------------
//--------------------------------------
// Grenade implementation...
//--------------------------------------
//
Grenade::Grenade(const Int32 in_datFileId)
 : Projectile(in_datFileId),
   m_lastSurfaceType(0),
   m_primedExplosion(false),
   m_pGrenadeData(NULL)
{
   m_faceForward = false;

   flags |= (SimMovement::UseElasticity |
             SimMovement::UseCurrent    |
             SimMovement::UseDrag);

   //
   netFlags.set(Ghostable);
}

Grenade::Grenade()
 : Projectile(-1),
   m_lastSurfaceType(0),
   m_primedExplosion(false),
   m_pGrenadeData(NULL)
{
   m_faceForward = false;

   flags |= (SimMovement::UseElasticity |
             SimMovement::UseCurrent    |
             SimMovement::UseDrag);

   //
   netFlags.set(Ghostable);
}

bool
Grenade::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
   
   m_collisionImage.radius = m_projectileData->collisionRadius;

   setFriction(1.0);
   setElasticity(m_pGrenadeData->elasticity);
   
   forceMask |= SimMovementGravityForce;
   m_primerTime = UInt32(m_projectileData->liveTime * 1000.0f) + wg->currentTime;

   if (isGhost() == true) {
      flags.set(UseAngulerVelocity);
      setAngulerVelocity(Point3F(6, 2, 4));

      m_renderImage.setTranslucent(true);
      m_lastPuff = manager->getCurrentTime() + 0.1;

      // Sim forward to account for lag...
      //
      m_puffInterval = UInt32(m_projectileData->projSpecialTime * 1000.0f) + 100; // in ms...

      m_lastUpdated = wg->currentTime - wg->interpolateTime;
      clientProcess(wg->currentTime);
   } else {
      m_lastGhosted = wg->currentTime;
   }

   return true;
}

void
Grenade::buildContainerBox(const TMat3F& in_rMat,
                               Box3F*        out_pBox)
{
   out_pBox->fMin = in_rMat.p;
   out_pBox->fMax = in_rMat.p;
   if (m_projectileData != NULL) {
      out_pBox->fMin -= Point3F(m_projectileData->collisionRadius, m_projectileData->collisionRadius, m_projectileData->collisionRadius);
      out_pBox->fMax += Point3F(m_projectileData->collisionRadius, m_projectileData->collisionRadius, m_projectileData->collisionRadius);
   }
}

void
Grenade::writeInitialPacket(Net::GhostManager* gm,
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
Grenade::readInitialPacket(Net::GhostManager* gm,
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
Grenade::packUpdate(Net::GhostManager* gm,
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
   } else {
      stream->writeFlag(false);
   
      if (mask & Projectile::FPPositionMask) {
         stream->writeFlag(true);
         
         Point3F position = getLinearPosition();
         stream->write(sizeof(position), &position);
      } else {
         stream->writeFlag(false);
      }

      if (mask & Projectile::FPVelocityMask) {
         stream->writeFlag(true);
         
         Point3F velocity = getLinearVelocity();
         stream->write(sizeof(velocity), &velocity);
      } else {
         stream->writeFlag(false);
         
         Point3F velocity = getLinearVelocity();
         velocity.normalize();
         stream->writeNormalVector(&velocity, 9);
      }

      if ((mask & FGCollisionSoundMask) != 0) {
         stream->writeFlag(true);

         stream->writeInt(m_lastSurfaceType, 4);   // Max enum in TS::Material is
                                                   //  containable in 4 bits
      } else {
         stream->writeFlag(false);
      }
   }
   
   return parentPunt;
}

void
Grenade::unpackUpdate(Net::GhostManager* gm,
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
      
      if (stream->readFlag() == true) {
         // Velocity (proper)
         //
         Point3F vel;
         stream->read(sizeof(vel), &vel);
         setLinearVelocity(vel);
      } else {
         // Velocity (compressed)
         //
         Vector3F velocity;
         stream->readNormalVector(&velocity, 9);

         // Velocity is a normalized vector that points in the proper direction.  Assume
         //  that our speed is correct, but we need to update the direction...
         //
         velocity *= getLinearVelocity().len();
         setLinearVelocity(velocity);
      }
      
      if (stream->readFlag() == true) {
         // Sound to play...
         //
         int soundToPlay = stream->readInt(4);
         playBounceSound(soundToPlay);
      }

      // Sim forward to account for lag...
      //
      m_lastUpdated = wg->currentTime - wg->interpolateTime;
      clientProcess(wg->currentTime);
   }
}

void
Grenade::clientProcess(DWORD in_currTime)
{
   if (m_hasExploded == true)
      return;

   Int32 intTimeSlice = Int32(in_currTime - m_lastUpdated);
   if (intTimeSlice <= 0)
      return;

   // unlike bullets, we need to make sure that we're updating the trail
   //  properly, so we have an upper bound on the timeslice...
   float floatTimeSlice = float(intTimeSlice) * 0.001f;
   if (floatTimeSlice > m_projectileData->projSpecialTime) {
      floatTimeSlice = m_projectileData->projSpecialTime;
      intTimeSlice   = UInt32(m_projectileData->projSpecialTime * 1000.0f);
   }
   setTimeSlice(floatTimeSlice);

   while (m_lastUpdated + intTimeSlice <= in_currTime) {
      TMat3F tmat;
      updateMovement(&tmat);

      DWORD time = intTimeSlice;
      bool hitSomething = false;
      while (losStep(tmat, time) == true &&
             timeSlice != 0) {
         // We assume that the number of bounces is small, and that
         //  the timeslice is small enough to use only one gravity
         //  calculation per update.
         //
         tmat = getTransform();
         tmat.p += getLinearVelocity() * (time * 0.001f);
         if (didContact() == true)
            hitSomething = true;
      }
      decayGraceTime(intTimeSlice);

      updateTrail(in_currTime);
      m_lastUpdated += intTimeSlice;

      if (hitSomething == true && m_lastUpdated > m_primerTime) {
         readExplosion(NULL);
         return;
      }

      if (m_lastUpdated != in_currTime &&
          UInt32(m_lastUpdated + intTimeSlice) > in_currTime) {
         intTimeSlice = in_currTime - m_lastUpdated;
         floatTimeSlice = float(intTimeSlice) * 0.001f;
         setTimeSlice(floatTimeSlice);
      }
   }

   updateProjectileSound(getTransform(), getLinearVelocity());
}

void
Grenade::serverProcess(DWORD in_currTime)
{
   if (m_waitingForDelete == true) {
      if (in_currTime >= m_deleteTime)
         deleteObject();
      return;
   }

   UInt32 timeSlice = UInt32(getTimeSlice() * 1000.0f);

   // Update the position if half a second has passed...
   if (in_currTime - m_lastGhosted > 500) {
      m_lastGhosted = wg->currentTime;
      setMaskBits(FPPositionMask);
   }

   if (in_currTime > m_totalTime) {
      destroyProjectile();
      return;
   }

   // prime the explosion?
   if (m_primedExplosion == false && in_currTime > m_primerTime)
      m_primedExplosion = true;

   while (m_lastUpdated + timeSlice < in_currTime) {
      TMat3F tmat;
      updateMovement(&tmat);

      DWORD time = timeSlice;
      bool hitSomething = false;
      while (losStep(tmat, time) == true && time != 0) {
         // We assume that the number of bounces is small, and that
         //  the timeslice is small enough to use only one gravity
         //  calculation per update.
         //
         tmat    = getTransform();
         tmat.p += getLinearVelocity() * (time * 0.001f);
         if (didContact() == true)
            hitSomething = true;
      }
      decayGraceTime(timeSlice);

      if (hitSomething == true) {
         m_lastGhosted = wg->currentTime;
         setMaskBits(FPPositionMask | FPVelocityMask);

         if (m_primedExplosion == true) {
            destroyProjectile();
            break;
         }
      }
      m_lastUpdated += timeSlice;
   }
}

void
Grenade::destroyProjectile()
{
   // Apply radius damage to anyone in the vicinity
   applyRadiusDamage(getTransform().p);
   
   m_hasExploded = true;
   m_lastGhosted = wg->currentTime;
   setMaskBits(FPExplosionMask);

   m_waitingForDelete = true;
   m_deleteTime       = wg->currentTime + 500;
}

void
Grenade::updateTrail(DWORD in_currTime)
{
   AssertFatal(isGhost() == true, "Server objects don't have trails");
   AssertFatal(manager != NULL, "Not yet added...");

   if (in_currTime > (m_lastPuff + m_puffInterval)) {
      SmokeManager* pSM = SmokeManager::get(manager);
      pSM->addPuff(m_pGrenadeData->smokeShape,
                   1.0, getLinearPosition(), Point3F(0, 0, 10) * 0.1);
      m_lastPuff = in_currTime;
      
      // Calculate the time to the next puff...
      //
      float currSpeed = getLinearVelocity().len();
      float factor = (m_projectileData->muzzleVelocity - currSpeed) / m_projectileData->muzzleVelocity;
      if (factor < 0.0f) factor = 0.0f;
      m_puffInterval = UInt32((m_projectileData->projSpecialTime +
                               m_projectileData->projSpecialTime * 2.0f * factor) * 1000.0f);
   }
}

void
Grenade::playBounceSound(const int   in_surfaceType)
{
   int soundId;

   switch (in_surfaceType) {
     case TS::Material::MetalType:
      soundId = IDSFX_GRENADE_METAL;
      break;

     case TS::Material::ConcreteType:
     case TS::Material::MarbleType:
     case TS::Material::StoneType:
      soundId = IDSFX_GRENADE_STONE;
      break;

     case TS::Material::GlassType:
      soundId = IDSFX_GRENADE_DEFAULT;
      break;

     default:
      soundId = IDSFX_GRENADE_DEFAULT;
      break;
   }

   if(soundId)
      Sfx::Manager::PlayAt(manager, soundId, getTransform(), Point3F(0, 0, 0));
}

int
Grenade::getDatGroup()
{
   return DataBlockManager::GrenadeDataType;
}

void
Grenade::readExplosion(BitStream*)
{
   // nothing to read, just blow up.

   if (m_hasExploded == true)
      return;
   m_hasExploded    = true;
   m_overrideRender = true;
   stopProjectileSound();

   Explosion* pExplosion = new Explosion(getExplosionTag());
   pExplosion->setPosition(getLinearPosition());
   manager->addObject(pExplosion);

   SmokeManager::get(manager)->removeAllInRadius(getLinearPosition(),
                                                 m_pGrenadeData->explosionRadius);
}

void
Grenade::writeExplosion(BitStream*)
{
   // nothing.
}

bool
Grenade::losStep(TMat3F& io_rPosition, 
                 DWORD&  io_rTime)
{
   SimContainer* pRoot = (SimContainer*)manager->findObject(SimRootContainerId);
   AssertFatal(pRoot, "No root container?");

   SimCollisionInfo  info;
   SimContainerQuery query;
   query.id       = -1;
   query.type     = -1;
   query.mask     = csm_collisionMask;
   query.detail   = SimContainerQuery::DefaultDetail;
   query.box.fMin = getTransform().p;
   query.box.fMax = io_rPosition.p;
   if (m_projectileData->collideWithOwner == false || m_ownerGraceMS > 0)
      query.id = getOwnerId();

   if (pRoot->findLOS(query, &info, SimCollisionImageQuery::High)) {
      // Bang!  We hit something!  Ok, process the surface, to update the movement
      //  parameters, then, calculate the time it took to hit this surface, and step
      //  away.
      //
      info.surfaces[0].transform(info.surfaces.tWorld);
      
      float velLen = getLinearVelocity().len();
      if (processSurface(info.surfaces[0].normal,
                         info.surfaces[0], 0) == true) {
         // OK, we really collided.  Set the new transform, and figure
         //  out the time it took to get here.
         //
         io_rPosition.p = info.surfaces[0].position;
         Point3F difVec = info.surfaces[0].position - getLinearPosition();
         setTransform(io_rPosition);

         float difTime = difVec.len() / velLen;
         UInt32 difMS  = UInt32(difTime * 1000.0f);
         io_rTime -= difMS;
         if (io_rTime & 0x8000000)  // lame, but its a UInt32.  Arithmetic precision errors
            io_rTime = 0;           //  in the division above can cause this to go wrong.

         return true;
      } else {
         // WTF?  We didn't collide?
         io_rTime = 0;
         setPosition(io_rPosition, true);
         return false;
      }


   } else {
      io_rTime = 0;
      setPosition(io_rPosition, true);
      return false;
   }
}

bool
Grenade::processSurface(Vector3F                normal,
                        const CollisionSurface& surface,
                        BitSet32                /*flags*/)
{
   const float SurfaceShift = 0.01f;

   bool  returnVal = false;
   float dot       = m_dot(normal, lVelocity);
   if (dot < 0.0f) {
      contact = true;

      normal *= -dot + SurfaceShift;
      lVelocity += normal * 2.0f;

      if (surface.material == NULL)
         lVelocity *= coefficient.elasticity;
      else
         lVelocity *= coefficient.elasticity * ((TSMaterial*)surface.material)->getElasticity();
      returnVal = true;

      float len = getLinearVelocity().len();
      if (len > m_projectileData->muzzleVelocity * 0.15) {
         m_lastGhosted = wg->currentTime;
         setMaskBits(FGCollisionSoundMask);

         if (surface.material != 0) {
            TS::Material* pMaterial = (TS::Material*)surface.material;
            m_lastSurfaceType       = pMaterial->fParams.fType;
         } else {
            m_lastSurfaceType = TS::Material::DefaultType;
         }
      }
   }

   return returnVal;
}

