//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <ml.h>
#include <console.h>
#include <simResource.h>
#include <netGhostManager.h>
#include "explosion.h"
#include "fearForceDcl.h"
#include "fearGlobals.h"
#include "projSeekMissile.h"
#include "dataBlockManager.h"
#include "smokePuff.h"

const DWORD SeekingMissile::csm_seekUpdateTimeMS = 150;
IMPLEMENT_PERSISTENT_TAG(SeekingMissile, SeekingMissilePersTag);

SeekingMissile::SeekingMissileData::SeekingMissileData()
{
   seekingTurningRadius    = 15.0f;
   nonSeekingTurningRadius = 50.0f;
   proximityDist           = 2.0f;
}

void
SeekingMissile::SeekingMissileData::pack(BitStream* bs)
{
   Parent::pack(bs);

   bs->write(projSpecialTime);
   bs->write(seekingTurningRadius);
   bs->write(nonSeekingTurningRadius);
}

void
SeekingMissile::SeekingMissileData::unpack(BitStream* bs)
{
   Parent::unpack(bs);

   bs->read(&projSpecialTime);
   bs->read(&seekingTurningRadius);
   bs->read(&nonSeekingTurningRadius);
}

bool
SeekingMissile::SeekingMissileData::preload(ResourceManager* rm,
                                            bool             server,
                                            char             errorBuffer[256])
{
   if (Parent::preload(rm, server, errorBuffer) == false)
      return false;

   if (server == false) {
      smokeShape = rm->load("rSmoke.dts");
      if (bool(smokeShape) == false) {
         sprintf(errorBuffer, "Unable to load Smoke Shape: %s", "rSmoke.dts");
         return false;
      }
   }

   return true;
}


//------------------------------------------------------------------------------
//--------------------------------------
//
SeekingMissile::SeekingMissile(const Int32 in_datFileId)
 : Projectile(in_datFileId),
   m_timeoutExplosion(false),
   m_pTarget(NULL),
   m_lockLost(false),
   m_currSeekPercentage(1.0f)
{
   m_canSeeTarget = true;
   //
   netFlags.set(Ghostable);
}

SeekingMissile::SeekingMissile()
 : Projectile(-1),
   m_timeoutExplosion(false),
   m_pTarget(NULL),
   m_lockLost(false),
   m_currSeekPercentage(1.0f)
{
   //
   netFlags.set(Ghostable);
}

SeekingMissile::~SeekingMissile()
{

}

int
SeekingMissile::getDatGroup()
{
   return DataBlockManager::SeekingMissileDataType;
}


bool
SeekingMissile::initResources(GameBase::GameBaseData* io_pDat)
{
   if(!Parent::initResources(io_pDat))
      return false;

   m_pMissileData = dynamic_cast<SeekingMissileData*>(io_pDat);
   AssertFatal(m_pMissileData != NULL, "Error");

   return true;
}

bool
SeekingMissile::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
   
   // Set up the mass of the projectile...
	setMass(m_projectileData->mass);

   if (isGhost()) {
      m_renderImage.setTranslucent(true);
      m_renderFacingCamera = true;

      m_lastPuffPos = getLinearPosition();
   }

   m_lastUpdated = wg->currentTime;
   updateSeekPercentage();

   return true;
}

void
SeekingMissile::readExplosion(BitStream* io_pStream)
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
      m_hasExploded    = true;
      m_overrideRender = true;
      stopProjectileSound();
      SmokeManager::get(manager)->removeAllInRadius(position,
                                                    m_projectileData->explosionRadius);
   } 
}

void
SeekingMissile::writeExplosion(BitStream* io_pStream)
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

void
SeekingMissile::adjustVelocity(DWORD    in_numMS,
                               Point3F* out_pNewVel)
{
   // See if we need to turn.  We only turn if the rocket is sufficiently
   //  off course...
   //
   Point3F currVel      = getLinearVelocity();
   Point3F dirToTarget  = getTargetDirection();

   Point3F currVelNorm = currVel;
   currVelNorm.normalize();
   dirToTarget.normalize();

   float dot = m_dot(currVelNorm, dirToTarget);
   if (dot < 0.0f || m_canSeeTarget == false) {
      *out_pNewVel = getLinearVelocity();
      if (m_pTarget == NULL) {
         return;
      } else {
         m_lockLost = true;
         killProjectile();
         return;
      }
   }

   *out_pNewVel = getLinearVelocity();

   float angle = (m_angularVel * (float(in_numMS) / 1000.0f));
   float cosAngle = m_cos(angle);

   if (cosAngle > dot) {
//
//   if ((1.0f - dot) > 0.0025f) {
      // Need to update velocity.  Basically, we turn about the normal
      //  described by the cross product of the vectors above, at
      //  the speed given by the angular velocity variable.

      Point3F turningNormal;
      m_cross(dirToTarget, currVelNorm, &turningNormal);
      turningNormal.normalize();
      AngAxisF axisAngle(turningNormal, angle);
      RMat3F rotMat(axisAngle);
      Point3F newVelNorm;
      m_mul(currVelNorm, rotMat, &newVelNorm);
      AssertFatal(fabs(currVelNorm.len() - 1.0f) < 0.01f, "error");

      newVelNorm *= m_projectileData->muzzleVelocity;
      

      *out_pNewVel = newVelNorm;
   } else {
      *out_pNewVel = dirToTarget * currVel.len();
   }
}

void
SeekingMissile::clientProcess(DWORD in_currTime)
{
   if (m_hasExploded == true)
      return;
   if (m_renderImage.thread != NULL) {
      float deltaT = float(in_currTime - m_lastUpdated) * 0.001f;
      m_renderImage.thread->AdvanceTime(deltaT);
   }

   DWORD slice = in_currTime - m_lastUpdated;
   if (slice == 0)
      return;
   m_lastUpdated = in_currTime;

   setTimeSlice(float(slice) * 0.001f);
   TMat3F tmat;
   updateMovement(&tmat);

   SimCollisionInfo  info;
   SimContainerQuery collisionQuery;
   collisionQuery.id     = -1;
   collisionQuery.type   = -1;
   collisionQuery.mask   = csm_collisionMask;
   collisionQuery.detail = SimContainerQuery::DefaultDetail;
   collisionQuery.box.fMin   = getLinearPosition();
   collisionQuery.box.fMax   = tmat.p;

   SimContainer* pRoot = (SimContainer*)manager->findObject(SimRootContainerId);
   AssertFatal(pRoot != NULL, "No root object...");

   if (pRoot->findLOS(collisionQuery, &info, SimCollisionImageQuery::High)) {
      // Bang!  We hit something!
      info.transform(info.surfaces.tWorld);

      Explosion* pExplosion = new Explosion(getExplosionTag(info));
      pExplosion->setPosition(info.surfaces[0].position);
      pExplosion->setAxis(info.surfaces[0].normal);
      manager->addObject(pExplosion);
      SmokeManager::get(manager)->removeAllInRadius(info.surfaces[0].position,
                                                    m_projectileData->explosionRadius);

      m_hasExploded    = true;
      m_overrideRender = true;
      stopProjectileSound();
      return;
   }

   updateTrail();
   updateProjectileSound(getTransform(), getLinearVelocity());
}

void
SeekingMissile::serverProcess(DWORD in_currTime)
{
   if (m_waitingForDelete == true) {
      if (in_currTime >= m_deleteTime) {
         deleteObject();
      }
      return;
   }

   if (m_lastSeekUpdate + csm_seekUpdateTimeMS < in_currTime) {
      updateSeekPercentage();
   }

   DWORD slice = in_currTime - m_lastUpdated;
   if (slice == 0)
      return;

   if (m_lockLost == false) {
      Point3F newVel;
      adjustVelocity(slice, &newVel);
      setLinearVelocity(newVel);
   } else {
      //
   }

   // Default is simply to move forward until we reach the current time...
   //
   bool collided;
   if ((getTargetPosition() - getLinearPosition()).len() > m_pMissileData->proximityDist) {
      SimObject* pObject = NULL;
      SimCollisionInfo collision;
      collided = moveProjectile(in_currTime, collision, pObject, true, true);
   } else {
      // we're pretty close, nuke'em.
      collided = true;
   }

   // Out of gas, self destruct...
   if (collided == false && in_currTime > m_liveTime)
      collided = true;

   if (collided == true) {
      // We collided with an object, the surface info is contained in surface
      //
      AssertFatal(m_projectileData->damageClass == 1, "Must be radius damage for seekMissile");

      // Apply radius damage to anyone in the vicinity
      applyRadiusDamage(getLinearPosition());

      // And explode!
      spawnImpactEffect(getLinearPosition());
      
      // We're done.
      //
      m_waitingForDelete = true;
      m_deleteTime       = in_currTime + 500;
   }

   setMaskBits(FPPositionMask | FPVelocityMask);
}

void
SeekingMissile::killProjectile()
{
   // Ok, what we do here is simply add in gravity, and kill the rocket motor...
   //
   forceMask |= SimMovementGravityForce;
}

void
SeekingMissile::destroyProjectile()
{
   AssertFatal(isGhost() == false, "only on server!");
   // At this point, the projectile has expired wo/ hitting anything, so we want
   //  it to self-destruct.  It's going to be deleted by the base class, so we just need
   //  to create the explosion effect...
   //
   m_hasExploded       = true;
   m_timeoutExplosion  = true;
}

void
SeekingMissile::readInitialPacket(Net::GhostManager* gm, BitStream* pStream)
{
   Parent::readInitialPacket(gm, pStream);

   Point3F pos;
   Point3F vel;
   
   pStream->read(sizeof(pos), &pos);
   pStream->read(sizeof(vel), &vel);

   TMat3F transform = getTransform();
   transform.p = pos;
   setPosition(transform, true);

   // Hack, but we want to be a little slow...
   setLinearVelocity(vel * 0.5);
}

void
SeekingMissile::writeInitialPacket(Net::GhostManager* gm, BitStream* pStream)
{
   Parent::writeInitialPacket(gm, pStream);

   // Three things are sent with the initial packet:
   //  1. Position (initial)
   //  2. Vel (initial)
   //  3. Target ghost id, if it exists, position if not...
   //
   Point3F pos = getLinearPosition();
   Point3F vel = getLinearVelocity();
   
   pStream->write(sizeof(pos), &pos);
   pStream->write(sizeof(vel), &vel);
}

DWORD
SeekingMissile::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
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
   
      return parentPunt;
   }
}

void
SeekingMissile::unpackUpdate(Net::GhostManager* gm, BitStream* stream)
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
         
         Point3F oldPosition = getLinearPosition();

         TMat3F transform = getTransform();
         transform.p = position;
         setPosition(transform, true);

         // Now we need to calculate the velocity that will take us from
         //  our current position to the servers in the time between updates
         //
         Point3F difference = getLinearPosition() - oldPosition;
         difference *= float(csm_seekUpdateTimeMS) * 0.001f;
         setLinearVelocity(difference);
      }
   }
}

Point3F
SeekingMissile::getTargetDirection()
{
   if (isGhost())
      return getTargetPosition() - getLinearPosition();

   Point3F dirToTarget  = getTargetPosition() - getLinearPosition();
   float   distToTarget = dirToTarget.len();
   float   timeToTarget = distToTarget / m_projectileData->muzzleVelocity;
   dirToTarget += getTargetVelocity() * timeToTarget;

   // cast a line of sight to the above predicted position.  if we cannot see it,
   //  target the player.  If we can't see the player, go straight...
   //
   SimContainerQuery collisionQuery;
   SimCollisionInfo  info;
   collisionQuery.id     = getId();
   collisionQuery.type   = -1;
   collisionQuery.mask   = csm_collisionMask & ~SimPlayerObjectType;
   collisionQuery.detail = SimContainerQuery::DefaultDetail;

   collisionQuery.box.fMin = getLinearPosition();
   collisionQuery.box.fMax = getLinearPosition() + dirToTarget;

   SimContainer* pRoot = (SimContainer*)manager->findObject(SimRootContainerId);
   if (pRoot->findLOS(collisionQuery, &info, SimCollisionImageQuery::High) == true &&
       info.object != m_pTarget) {
      // Ok, try the player...
      //
      dirToTarget = getTargetPosition() - getLinearPosition();
      collisionQuery.box.fMax = getLinearPosition() + dirToTarget;
      if (pRoot->findLOS(collisionQuery, &info, SimCollisionImageQuery::High) == true &&
          info.object != m_pTarget) {
         m_canSeeTarget = false;
         return (getLinearVelocity() * 1000.0f);
      } else {
         return dirToTarget;
      }
   } else {
      return dirToTarget;
   }
}

Point3F
SeekingMissile::getTargetPosition()
{
   if (m_pTarget != NULL) {
      return m_pTarget->getBoxCenter();
   } else {
      if (isGhost() == false)
         return (getLinearPosition() + (getLinearVelocity() * 1000.0f));
      else
         return m_targetPosition;
   }
}

Point3F
SeekingMissile::getTargetVelocity()
{
   if (m_pTarget != NULL) {
      return m_pTarget->getLinearVelocity();
   } else {
      return Point3F(0, 0, 0);
   }
}

void
SeekingMissile::buildContainerBox(const TMat3F& in_rMat,
                                  Box3F*        out_pBox)
{
   // Assumes that the projectile is 0 radius.
   //
   out_pBox->fMin = in_rMat.p;
   out_pBox->fMax = in_rMat.p;
}

bool
SeekingMissile::isTargetable() const
{
   return true;
}

void
SeekingMissile::setTarget(ShapeBase* io_pTarget)
{
   m_pTarget = io_pTarget;
   deleteNotify(m_pTarget);
}

void
SeekingMissile::onDeleteNotify(SimObject* io_pObject)
{
   if (io_pObject == (SimObject*)m_pTarget)
      m_pTarget = NULL;
   Parent::onDeleteNotify(io_pObject);
}

float
SeekingMissile::getTargetRange() const
{
   return 1000.0f;
}

void
SeekingMissile::updateSeekPercentage()
{
   const char* script = scriptName("updateTargetPercentage");
   if (script != NULL && getTargetScriptId() != -1) {
      const char* pRet = Console->evaluatef("%s(%d);", script, getTargetScriptId());
      m_currSeekPercentage = atof(pRet);
   } else {
      m_currSeekPercentage = 1.0f;
   }

   float radius = m_pMissileData->nonSeekingTurningRadius +
                  ((m_pMissileData->seekingTurningRadius -
                    m_pMissileData->nonSeekingTurningRadius) *
                   m_currSeekPercentage);
   m_angularVel     = (m_pMissileData->muzzleVelocity / (2.0f * M_PI * radius));
   m_lastSeekUpdate = m_lastUpdated;
}

int
SeekingMissile::getTargetScriptId()
{
   if (m_pTarget != NULL)
      return m_pTarget->getId();
   else
      return -1;
}

void
SeekingMissile::updateTrail()
{
   AssertFatal(isGhost() == true, "Server objects don't have trails");
   AssertFatal(manager != NULL, "Not yet added...");

   if (m_hasExploded == true || m_overrideRender)
      return;

   float len = (getLinearPosition() - m_lastPuffPos).len();
   int numPuffs = int(len / m_pMissileData->projSpecialTime);
   if (numPuffs == 0)
      return;

   Point3F puffNorm = (getLinearPosition() - m_lastPuffPos);
   puffNorm /= len;
   puffNorm *= m_pMissileData->projSpecialTime;

   for (int i = 0; i < numPuffs; i++) {
      m_lastPuffPos += puffNorm;
      
      SmokeManager* pSM = SmokeManager::get(manager);
      AssertFatal(pSM != NULL, "No smoke puff manager?");

      pSM->addPuff(m_pMissileData->smokeShape, 0.4, m_lastPuffPos, Point3F(0, 0, 0));
   }
}
