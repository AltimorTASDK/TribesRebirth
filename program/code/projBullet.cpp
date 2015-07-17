//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <ts_pointArray.h>
#include <g_surfac.h>
#include <netPacketStream.h>
#include <netGhostManager.h>
#include <m_random.h>

#include "datablockmanager.h"
#include "player.h"
#include "explosion.h"
#include "projBullet.h"
#include "fear.strings.h"
#include "fearDcl.h"
#include "datablockmanager.h"
#include "fearGlobals.h"
#include "fearPlugin.h"
#include <decalmanager.h>

namespace {

Random sg_randomGenerator;

inline int
getRandomInt(const int in_max)
{
   return int(sg_randomGenerator.getFloat() * (float(in_max) + 0.99f));
}

} // namespace {}


Bullet::BulletData::BulletData()
{
   rotationPeriod = 0.0f;
   bulletHoleIndex = -1;

   tracerLength = 10;
}

void
Bullet::BulletData::pack(BitStream* bs)
{
   Parent::pack(bs);

   bs->write(rotationPeriod);
   bs->write(bulletHoleIndex);
   bs->write(tracerLength);
}

void Bullet::BulletData::unpack(BitStream* bs)
{
   Parent::unpack(bs);

   bs->read(&rotationPeriod);
   bs->read(&bulletHoleIndex);
   bs->read(&tracerLength);
}

bool
Bullet::BulletData::getTargetVector(const Point3F& in_rShootFrom,
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


//-------------------------------------- BulletNormal
IMPLEMENT_PERSISTENT_TAG(Bullet, BulletPersTag);
const Int32 Bullet::csm_normalBits = 20;

//------------------------------------------------------------------------------
//--------------------------------------
// BulletNormal implementation...
//--------------------------------------
//
Bullet::Bullet(const Int32 in_datFileId)
 : Projectile(in_datFileId),
   m_resetInterpFrom(true),
   m_isTracer(false)
{
   netFlags.set(Ghostable);
   m_sphereCollImage.transform.identity();
}

Bullet::Bullet()
 : Projectile(-1),
   m_resetInterpFrom(true),
   m_isTracer(false)
{
   //
   netFlags.set(Ghostable);
   m_sphereCollImage.transform.identity();
}

bool
Bullet::applyBaseDamage(SimCollisionInfo& io_rCollision,
                        SimObject*        io_pHit)
{
   AssertFatal(isGhost() == false, "Cannot run on client!");
   if (io_pHit->getType().test(csm_damageMask)) {
      ShapeBase* pShape = static_cast<ShapeBase*>(io_pHit);

      // Grab the first position in the surface list.
      Point3F pos;
      if (io_rCollision.surfaces.size()) {
         m_mul(io_rCollision.surfaces[0].position,
            io_rCollision.surfaces.tWorld,&pos);
      }
      else
         pos = getLinearPosition();

      float value = m_projectileData->damageValue;
      if (wg->currentTime > m_liveTime) {
         value *= 1.0f - (float(wg->currentTime - m_liveTime) /
                          float(m_totalTime - m_liveTime));
      }

      pShape->applyDamage(m_projectileData->damageType, value,
                          pos, getLinearVelocity(),
                          Point3F(0,0,0),
                          getDamageId());
      return true;
   }
   return false;
}

bool
Bullet::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
   
   if (isGhost() == true) {
      if (m_projectileData->projSpecialBool == false)
         m_overrideRender = true;

      m_renderImage.setTranslucent(true);
      m_renderFacingCamera = true;

      // Need to sim forward to account for lag...
      //
      m_spawnTime   = wg->currentTime;
      m_lastUpdated = wg->currentTime;
      m_startInterp = wg->currentTime;
      m_endInterp   = wg->currentTime + wg->interpolateTime;
      m_interpFrom  = getLinearPosition();
      predictForward(wg->currentTime,
                     wg->currentTime + wg->interpolateTime + wg->predictForwardTime,
                     m_interpTo);

      // Are we a tracer?
      if (sg_randomGenerator.getFloat() < m_pBulletData->projSpecialTime &&
          m_pBulletData->projSpecialBool == false) {
         m_isTracer       = true;
         m_overrideRender = false;
      }
   } else {
      // On adding to the server, dumb ourselves down to the level that we send to
      //  the client...
      //
      Vector3F velocity = getLinearVelocity();
      float velLen = velocity.len();
      velocity /= velLen;

      UInt32 intx = UInt32(((velocity.x + 1.0f) * 0.5) * ((1 << csm_normalBits) - 1));
      UInt32 inty = UInt32(((velocity.y + 1.0f) * 0.5) * ((1 << csm_normalBits) - 1));
      bool   zvel = velocity.z < 0.0f;

      float floatx   = (float(intx * 2) / float((1 << csm_normalBits) - 1)) - 1.0f;
      float floaty   = (float(inty * 2) / float((1 << csm_normalBits) - 1)) - 1.0f;
      float floatz;
      float  zSquared = 1 - (floatx * floatx) - (floaty * floaty);
      if (zSquared < 0) {
         floatz = 0.0f;
      } else {
         floatz = sqrt(zSquared);
         if (zvel)
            floatz = -floatz;
      }

      UInt32 writeVel = UInt32(velLen * 16);
      velLen = float(writeVel) / 16.0f;

      velocity.x = floatx;
      velocity.y = floaty;
      velocity.z = floatz;
      velocity *= velLen;
      setLinearVelocity(velocity);

      m_spawnVelocity = getLinearVelocity();
      m_spawnVelocityLen = m_spawnVelocity.len();
      m_spawnPosition = getLinearPosition();
      m_spawnTime     = sg.currentTime;
   }
   setSearchBoundaries();
   
   return true;
}

void
Bullet::buildContainerBox(const TMat3F& in_rMat,
                          Box3F*        out_pBox)
{
   out_pBox->fMin = in_rMat.p;
   out_pBox->fMax = in_rMat.p;
}

void
Bullet::writeInitialPacket(Net::GhostManager* gm,
                           BitStream*         pStream)
{
   Parent::writeInitialPacket(gm, pStream);

   Point3F  position = getLinearPosition();
   float time        = (position - m_spawnPosition).len() / m_spawnVelocityLen;
   DWORD timeElapsed = DWORD(time * 1000.0f);

   pStream->write(sizeof(m_spawnPosition), &m_spawnPosition);
   pStream->writeInt(timeElapsed, 15);

   // Write compressed direction...
   //
   Point3F normalVel = m_spawnVelocity;
   normalVel        /= m_spawnVelocityLen;
   pStream->writeNormalVector(&normalVel, csm_normalBits);
   
   // Velocity is rounded to nearest 16th, we assume that it will never travel faster
   //  than 1023.(15/16) m/s
   //
   UInt32 writeVel = UInt32(m_spawnVelocityLen * 16);
   pStream->writeInt(writeVel, 14);
}

void
Bullet::readInitialPacket(Net::GhostManager* io_pGM,
                          BitStream*         pStream)
{
   Parent::readInitialPacket(io_pGM, pStream);

   Point3F initialPosition;
   pStream->read(sizeof(Point3F),  &initialPosition);
   m_spawnPosition = initialPosition;
   
   DWORD elapsed = pStream->readInt(15);

   pStream->readNormalVector(&m_spawnVelocity, csm_normalBits);
   m_renderImage.faceDirection(m_spawnVelocity);

   m_spawnVelocityLen = float(pStream->readInt(14)) / 16.0f;
   m_spawnVelocity   *= m_spawnVelocityLen;
   setLinearVelocity(m_spawnVelocity);

   // Now advance the position
   initialPosition += getLinearVelocity() * (float(elapsed) * 0.001f);

   if (m_pShooter != NULL) {
      TMat3F muzzleTrans;
      if (m_pShooter->getMuzzleTransform(0, &muzzleTrans)) {
         if (dynamic_cast<Player*>(m_pShooter) != NULL) {
            TMat3F total;
            m_mul(muzzleTrans, m_pShooter->getTransform(), &total);
            muzzleTrans = total;
         }
         setPosition(muzzleTrans, true);
         return;
      }
   }

   TMat3F transform = getTransform();
   transform.p = initialPosition;
   setPosition(transform, true);
}


DWORD
Bullet::packUpdate(Net::GhostManager* gm,
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
         parentPunt |= (mask & FPExplosionMask);
   } else {
      stream->writeFlag(false);
      // Bullets never update
   }
   
   return parentPunt;
}

void
Bullet::unpackUpdate(Net::GhostManager* gm,
                               BitStream*         stream)
{
   Parent::unpackUpdate(gm, stream);
   
   if (stream->readFlag() == true) {
      // Is initial packet...
      //
      readInitialPacket(gm, stream);
   }
}

void
Bullet::clientProcess(DWORD in_currTime)
{
   if (m_hasExploded == true || in_currTime > m_totalTime)
      return;
   if (m_renderImage.thread != NULL) {
      float deltaT = float(in_currTime - m_lastUpdated) / 1000.0f;
      m_renderImage.thread->AdvanceTime(deltaT);
   }
   if (m_pBulletData->rotationPeriod != 0.0f) {
      DWORD rotation = (in_currTime - m_spawnTime) % DWORD(m_pBulletData->rotationPeriod * 1000.0f);
      float fRotation = float(rotation) / (m_pBulletData->rotationPeriod * 1000.0f);

      m_renderImage.setAxisSpin(fRotation * M_2PI);
   }

   DWORD slice = in_currTime - m_lastUpdated;
   if (slice == 0)
      return;

   Point3F movePosition;
   if (in_currTime <= m_endInterp) {
      if (m_resetInterpFrom) {
         if (m_pShooter != NULL) {
            TMat3F muzzleTrans;
            if (m_pShooter->getMuzzleTransform(0, &muzzleTrans)) {
               if (dynamic_cast<Player*>(m_pShooter) != NULL) {
                  TMat3F total;
                  m_mul(muzzleTrans, m_pShooter->getTransform(), &total);
                  muzzleTrans = total;
               }
               m_interpFrom = muzzleTrans.p;
            }
         }
         m_resetInterpFrom = false;
      }

      float factor = float(in_currTime - m_startInterp) / float(m_endInterp - m_startInterp);
      TMat3F finalTransform = getTransform();
      Point3F finalPos;
      finalPos.interpolate(m_interpFrom, m_interpTo, factor);
      movePosition = finalPos;
   } else {
      if (m_resetInterpFrom) {
         // Somehow we entirely missed the interpolation cycle...
         if (m_pShooter != NULL) {
            TMat3F muzzleTrans;
            if (m_pShooter->getMuzzleTransform(0, &muzzleTrans)) {
               if (dynamic_cast<Player*>(m_pShooter) != NULL) {
                  TMat3F total;
                  m_mul(muzzleTrans, m_pShooter->getTransform(), &total);
                  muzzleTrans = total;
               }
               TMat3F reallyFinal = getTransform();
               reallyFinal.p = muzzleTrans.p;
               setPosition(reallyFinal, true);
            }
         }
         m_resetInterpFrom = false;
      }

      SimCollisionInfo dummyColl;
      SimObject*       dummyPObject;

      Point3F finalPos = getLinearPosition();
      finalPos += getLinearVelocity() * (float(slice) * 0.001f);
      movePosition = finalPos;
   }

   SimCollisionInfo  collisionInfo;
   SimContainerQuery collisionQuery;
   collisionQuery.type     = -1;
   collisionQuery.mask     = csm_collisionMask & ~(SimInteriorObjectType | SimTerrainObjectType);
   collisionQuery.detail   = SimContainerQuery::DefaultDetail;
   collisionQuery.box.fMin = getTransform().p;
   collisionQuery.box.fMax = movePosition;

   if (m_projectileData->collideWithOwner == false || m_ownerGraceMS > 0)
      collisionQuery.id = getOwnerId();
   else
      collisionQuery.id = -1;
   decayGraceTime(in_currTime - m_lastUpdated);

   SimContainer* root = findObject(manager, SimRootContainerId, root);
   AssertFatal(root != NULL, "No root object...");
   
   if (root->findLOS(collisionQuery, &collisionInfo, SimCollisionImageQuery::High)) {
      if (isProjStatic(collisionInfo.object) == true) {
         // Bang!  We hit something!
         collisionInfo.transform(collisionInfo.surfaces.tWorld);

         Explosion* pExplosion = new Explosion(getExplosionTag(collisionInfo));
         pExplosion->setPosition(collisionInfo.surfaces[0].position);
         pExplosion->setAxis(collisionInfo.surfaces[0].normal);
         manager->addObject(pExplosion);

         int bhIndex = getBulletHoleIndex(collisionInfo);
         if (bhIndex != -1) {
            DecalManager* dmgr = (DecalManager*)manager->findObject(DecalManagerId);

            if (dmgr) {
               dmgr->addBulletHole(collisionInfo.surfaces[0].position,
                                   collisionInfo.surfaces[0].normal, 
                                   DecalManager::DECALTYPE(bhIndex));
            }
         }

         m_hasExploded    = true;
         m_overrideRender = true;
         stopProjectileSound();
      }
   } else {
      if (in_currTime > m_finalTime) {
         Explosion* pExplosion = new Explosion(getExplosionTag(storedInfo));
         pExplosion->setPosition(m_finalPosition);
         pExplosion->setAxis(m_finalNormal);
         manager->addObject(pExplosion);
         
         int bhIndex = getBulletHoleIndex(storedInfo);
         if (bhIndex != -1) {
            DecalManager *dmgr =  (DecalManager*)manager->findObject(DecalManagerId);

            if (dmgr)
               dmgr->addBulletHole(m_finalPosition, m_finalNormal,
                                   DecalManager::DECALTYPE(bhIndex));
         }

         m_hasExploded    = true;
         m_overrideRender = true;
         stopProjectileSound();
         return;
      }

      // force position update, no collision
      //
      TMat3F finalTransform = getTransform();
      finalTransform.p = movePosition;

      setPosition(finalTransform, true);
      m_lastUpdated = in_currTime;
   }

   if (m_overrideRender == false)
      updateProjectileSound(getTransform(), getLinearVelocity());
}

void
Bullet::predictForward(const DWORD in_currTime,
                       const DWORD in_predictTime,
                       Point3F&    out_rPosition)
{
   // Bullets have no acceleration, so this is quite easy...
   //
   Point3F startPos = getLinearPosition();
   float timeFloat  = float(in_predictTime - in_currTime) * 0.001f;
   out_rPosition = startPos + (m_spawnVelocity * timeFloat);
}

Int32
Bullet::getBulletHoleIndex(SimCollisionInfo& io_rCollision)
{
   if (io_rCollision.object == NULL)
      return -1;

   UInt32 type = io_rCollision.object->getType();
   if ((type & SimInteriorObjectType) != 0)
      if (m_pBulletData->bulletHoleIndex != -1)
         return DecalManager::TYPE_HOLE_BASIC;

   return -1;
}

int
Bullet::getDatGroup()
{
   return DataBlockManager::BulletDataType;
}

void
Bullet::spawnImpactEffect(SimCollisionInfo& io_rCollision)
{
   AssertFatal(isGhost() == false, "This may NOT be run on the client...");

   // Allow derived classes to select a bullethole based on the surface type...
   //
   m_explosionId       = getExplosionTag(io_rCollision);
   m_explosionPosition = io_rCollision.surfaces[0].position;
   m_explosionAxis     = io_rCollision.surfaces[0].normal;
   m_hasExploded       = true;

   // First, determine if the object that was hit was a static object,
   //  if so, trust the client to explode properly...
   //
   if (isProjStatic(io_rCollision.object) == true)
      return;
   setMaskBits(FPExplosionMask);
}

void
Bullet::readExplosion(BitStream* io_pStream)
{
   // Derive the position
   Int32 ms = io_pStream->readInt(15);
   m_explosionPosition = m_spawnPosition + (m_spawnVelocity * (float(ms) / 1000.0f));

   io_pStream->readNormalVector(&m_explosionAxis, 6);
   m_explosionId = io_pStream->readInt(wg->dbm->getDataGroupBitSize(DataBlockManager::ExplosionDataType));

   if (m_hasExploded == false) {
      Explosion* pExplosion = new Explosion(m_explosionId);
      pExplosion->setAxis(m_explosionAxis);
      pExplosion->setPosition(m_explosionPosition);
      manager->addObject(pExplosion);
      m_hasExploded    = true;
      m_overrideRender = true;
      stopProjectileSound();
   }
}

void
Bullet::writeExplosion(BitStream* io_pStream)
{
   float len1     = (m_explosionPosition - m_spawnPosition).len();
   float len2     = m_spawnVelocityLen;
   UInt32 expTime = UInt32((len1 / len2) * 1000.0f);

   io_pStream->writeInt(expTime, 15);
   io_pStream->writeNormalVector(&m_explosionAxis, 6);
   io_pStream->writeInt(m_explosionId,
                        wg->dbm->getDataGroupBitSize(DataBlockManager::ExplosionDataType));
}

SimCollisionImage*
Bullet::getSelfImage()
{
   AssertFatal(m_projectileData != NULL, "error, called before initialized");
   AssertFatal(m_projectileData->collisionRadius != 0.0f,
               "Error, should never be called for point projectiles!");

   m_sphereCollImage.transform = getTransform();
   m_sphereCollImage.center = Point3F(0, 0, 0);
   m_sphereCollImage.radius = m_projectileData->collisionRadius;

   return &m_sphereCollImage;
}

bool
Bullet::initResources(GameBase::GameBaseData* io_pDat)
{
   if(!Parent::initResources(io_pDat))
      return false;

   m_pBulletData = dynamic_cast<BulletData*>(io_pDat);
   AssertFatal(m_pBulletData != NULL, "No grenade data?");

   if (m_pBulletData == NULL)
      return false;
   return true;
}

bool
Bullet::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);
      onQuery(SimLightQuery);

     default:
      return Parent::processQuery(query);
   }
}

bool
Bullet::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if (m_overrideRender == true || wg->currentTime >= m_totalTime) {
      return true;
   }

   AssertFatal(isGhost() == true, "Servers shouldn't be rendering!");

   TSRenderContext & rc = *query->renderContext;
   if (m_isTracer == false) {
      // If the client has predicted that the server will delete us, and
      //  we haven't yet received an update, skip all this work...
      //
      if (m_renderImage.shape == NULL ||
          m_overrideRender == true) {
         query->count = 0;

         return true;
      }

      // Set haze value
      //
      m_renderImage.hazeValue = 0;

      // Set up basic transform
      if (m_resetInterpFrom == true) {
         if (m_pShooter != NULL) {
            TMat3F muzzleTrans;
            if (m_pShooter->getMuzzleTransform(0, &muzzleTrans)) {
               if (dynamic_cast<Player*>(m_pShooter) != NULL) {
                  TMat3F total;
                  m_mul(muzzleTrans, m_pShooter->getTransform(), &total);
                  muzzleTrans = total;
               }

               m_renderImage.transform.p = muzzleTrans.p;
            }
         }
      } else {
         m_renderImage.transform.p = getTransform().p;
      }

      // Lets do any special fxRenderImage processing here...
      //
      if (m_renderFacingCamera == true) {
         m_renderImage.faceCamera(rc);
      }

      if (wg->currentTime >= m_liveTime) {
         float value = 1.0f - (float(wg->currentTime - m_liveTime) /
                               float(m_totalTime - m_liveTime));
         if (value < 0.0f)
            value = 0.0f;
         else if (value > 1.0f)
            value = 1.0f;

         m_renderImage.shape->setAlphaAlways(true, value);
      } else {
         m_renderImage.shape->setAlphaAlways(false);
      }

      // Return render image here...
      //
      m_renderImage.setSortValue(rc.getCamera()); // does nothing if not translucent
      query->image[query->count++] = &m_renderImage;
   } else {
      if (m_overrideRender == true) {
         query->count = 0;
         return true;
      }
      query->image[query->count++] = &m_tracerImage;

      float timeforLen = m_pBulletData->tracerLength / m_spawnVelocityLen;
      DWORD time = DWORD(timeforLen * 1000.0f);
      if (wg->currentTime < (m_spawnTime + (time / 2))) {
         query->count = 0;
         return true;
      }
      
      float factor = 1.0;
      DWORD startTime = wg->currentTime - time;
      if (startTime < (m_spawnTime + (time / 2))) {
         factor    = float(startTime - m_spawnTime + (time / 2)) / float(m_spawnTime + (time / 2));
         startTime = (m_spawnTime + (time / 2));
      }
      DWORD dif  = wg->currentTime - startTime;
      float difFloat = float(dif) / 1000.0f;

      if (dif == 0) {
         query->count = 0;
         return true;
      }

      m_tracerImage.m_endPoint   = getLinearPosition();
      m_tracerImage.m_startPoint = m_tracerImage.m_endPoint - getLinearVelocity() * difFloat;
      m_tracerImage.m_factor     = factor;

      const TMat3F & twc = rc.getCamera()->getTWC();
      // don't need x & y, so skip the extra work...
      float cd = twc.p.y + getTransform().p.x * twc.m[0][1] +
                           getTransform().p.y * twc.m[1][1] +
                           getTransform().p.z * twc.m[2][1];
      float nd = rc.getCamera()->getNearDist();
      m_tracerImage.sortValue = (cd<=nd) ? 0.99f : nd / cd;
      m_tracerImage.itype     = SimRenderImage::Translucent;
   }

   return true;
}


bool
Bullet::onSimLightQuery ( SimLightQuery * query )
{
   AssertFatal(isGhost() == true, "Should never receive this query on the server...");

   if (m_overrideRender == true) {
      query->count    = 0;
      query->light[0] = NULL;
   } else {
      if (wg->currentTime >= m_liveTime) {
         float value = 1.0f - (float(wg->currentTime - m_liveTime) /
                               float(m_totalTime - m_liveTime));
         if (value < 0.0f)
            value = 0.0f;
         else if (value > 1.0f)
            value = 1.0f;

         m_light.setIntensity((m_projectileData->lightColor * value));
      } else {
         m_light.setIntensity(m_projectileData->lightColor);
      }

      query->count = 1;
      query->light[0] = &m_light;
      query->ambientIntensity.set();
      m_light.setPosition( getLinearPosition() );
   }

   return true;
}

void
Bullet::TracerRenderImage::render(TSRenderContext& io_rRC)
{
   Point3F bulletVec = m_endPoint - m_startPoint;
   Point3F camVect;
   io_rRC.getCamera()->getTCW().getRow(1, &camVect);

   Point3F normalVec;
   m_cross(bulletVec, camVect, &normalVec);
   normalVec.normalize();

   float dist   = (io_rRC.getCamera()->getCC() - m_endPoint).len();
   float radius = io_rRC.getCamera()->projectRadius(dist, 1.0);
   float actualWidth = (1.0 / radius) * 0.5;

   Point3F renderPts[3];
   renderPts[0] = m_startPoint;
   renderPts[1] = m_endPoint - (normalVec * (actualWidth * 0.5 * m_factor));
   renderPts[2] = m_endPoint + (normalVec * (actualWidth * 0.5 * m_factor));

   TS::PointArray* pArray = io_rRC.getPointArray();
   GFXSurface* pSurface   = io_rRC.getSurface();

   pSurface->setFillMode(GFX_FILL_CONSTANT);
   pSurface->setShadeSource(GFX_SHADE_NONE);
   pSurface->setHazeSource(GFX_HAZE_NONE);
   pSurface->setAlphaSource(GFX_ALPHA_FILL);
   pSurface->setFillColor(250, 1976);
   pSurface->setZTest(GFX_ZTEST);
   pSurface->setTexturePerspective(false);
   pSurface->setTransparency(false);

   pArray->useTextures(false);
   pArray->useIntensities(false);
   
   int offset = pArray->addPoints(3, renderPts);
   
   TS::VertexIndexPair polyPairs[3];
   polyPairs[0].fVertexIndex = offset + 0;
   polyPairs[1].fVertexIndex = offset + 1;
   polyPairs[2].fVertexIndex = offset + 2;
   pArray->drawPoly(3, polyPairs, 0);

   polyPairs[0].fVertexIndex = offset + 0;
   polyPairs[1].fVertexIndex = offset + 2;
   polyPairs[2].fVertexIndex = offset + 1;
   pArray->drawPoly(3, polyPairs, 0);
   pSurface->setAlphaSource(GFX_ALPHA_NONE);
   pSurface->setZTest(GFX_ZTEST_AND_WRITE);
}

void
Bullet::setSearchBoundaries()
{
   SimContainerQuery query;
   query.id       = (m_pShooter != NULL) ? m_pShooter->getId() : -1;
   query.type     = -1;
   query.mask     = SimInteriorObjectType | SimTerrainObjectType;
   query.detail   = SimContainerQuery::DefaultDetail;
   query.box.fMin = getLinearPosition();
   query.box.fMax = getLinearPosition() + (getLinearVelocity() *
                                           m_projectileData->totalTime);

   SimContainer* pRoot = (SimContainer*)manager->findObject(SimRootContainerId);
   AssertFatal(pRoot, "No Root container?");

   if (pRoot->findLOS(query, &storedInfo, SimCollisionImageQuery::High)) {
      m_mul(storedInfo.surfaces[0].position, storedInfo.surfaces.tWorld, &m_finalPosition);
      m_mul(storedInfo.surfaces[0].normal,   (RMat3F)storedInfo.surfaces.tWorld, &m_finalNormal);
      m_finalPosition += m_finalNormal * 0.10;

      m_finalObject = storedInfo.object;

      // Determine the time at which this happens...
      //
      Point3F travel = m_finalPosition - getLinearPosition();
      float time     = travel.len() / getLinearVelocity().len();

      m_finalTime = wg->currentTime + DWORD(time * 1000.0f);
   } else {
      // Object doesn't collide with anything, wait 10 secs for object delete
      m_finalTime     = wg->currentTime + DWORD(m_projectileData->totalTime * 1000.0f) + 10000;
      m_finalPosition = Point3F(0, 0, 0);
      m_finalNormal   = Point3F(0, 0, 1);
      m_finalObject   = NULL;
   }
}

void
Bullet::serverProcess(DWORD in_currTime)
{
   if (m_waitingForDelete == true) {
      if (in_currTime >= m_deleteTime) {
         deleteObject();
      }
      return;
   }

   if (m_projectileIsLive == true &&
       in_currTime > m_liveTime) {
      killProjectile();
   } else if (in_currTime > m_totalTime) {
      destroyProjectile();
      m_waitingForDelete = true;
      m_deleteTime       = in_currTime + 500;
      return;
   }

   // Bullets simply contstruct a limited line of sight query, and run off that
   //
   DWORD slice = in_currTime - m_lastUpdated;
   if (slice == 0)
      return;
   m_lastUpdated = in_currTime;

   if (in_currTime > m_finalTime) {
      slice -= in_currTime - m_finalTime;
   }

   SimCollisionInfo  info;
   SimContainerQuery query;
   query.id       = -1;
   query.type     = -1;
   query.mask     = csm_collisionMask & ~(SimInteriorObjectType | SimTerrainObjectType);
   query.detail   = SimContainerQuery::DefaultDetail;
   query.box.fMin = getLinearPosition();
   query.box.fMax = getLinearPosition() + (getLinearVelocity() * (float(slice) * 0.001f));

   if (m_projectileData->collideWithOwner == false || m_ownerGraceMS > 0)
      query.id = getOwnerId();
   decayGraceTime(slice);

   SimContainer* pRoot = (SimContainer*)manager->findObject(SimRootContainerId);
   AssertFatal(pRoot, "No Root container?");

   if (pRoot->findLOS(query, &info, SimCollisionImageQuery::High)) {
      // Bang!  Hit something before we ran out of time!
      //
      TMat3F finalTransform = getTransform();
      m_mul(info.surfaces[0].position, info.surfaces.tWorld, &finalTransform.p);
      setTransform(finalTransform);

      if (m_projectileData->damageClass == 0) {
         applyBaseDamage(info, info.object);
         info.surfaces[0].transform(info.surfaces.tWorld);
         spawnImpactEffect(info);
      } else if (m_projectileData->damageClass == 1) {
         applyRadiusDamage(getLinearPosition());
         spawnImpactEffect(getLinearPosition());
      } else {
         AssertFatal(false, "misunderstood damageclass");
      }
      m_waitingForDelete = true;
      m_deleteTime       = in_currTime + 500;
   } else {
      if (in_currTime >= m_finalTime) {
         // Erk.  Missed.  And we're at the end of our "rope".  Pretend we
         //  collided against that object instead...
         //
         if (m_finalObject != NULL) {
            if (m_projectileData->damageClass == 0) {
               applyBaseDamage(storedInfo, m_finalObject);
               storedInfo.surfaces[0].transform(storedInfo.surfaces.tWorld);
               spawnImpactEffect(storedInfo);
            } else if (m_projectileData->damageClass == 1) {
               applyRadiusDamage(m_finalPosition);
               spawnImpactEffect(m_finalPosition);
            } else {
               AssertFatal(false, "misunderstood damageclass");
            }
         }
         m_waitingForDelete = true;
         m_deleteTime       = in_currTime + 500;
         return;
      }
      TMat3F finalTransform = getTransform();
      finalTransform.p = query.box.fMax;
      setPosition(finalTransform, true);
   }
}
