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
#include <simTagDictionary.h>
#include <simObjectTypes.h>
#include "playerManager.h"
#include "tsfx.h"
#include "soundFx.h"
#include "explosion.h"
#include "projectile.h"
#include "simResource.h"
#include "player.h"
#include "dataBlockManager.h"
#include "fearGlobals.h"
#include "netGhostManager.h"
#include "netPacketStream.h"
#include "console.h"

// All damageable objects are derived from ShapeBase.
const int Projectile::csm_damageMask = SimPlayerObjectType    | 
                                       StaticObjectType       |
                                       VehicleObjectType  |
                                       MoveableObjectType |
                                       MineObjectType;

const Int32 Projectile::csm_collisionMask = SimPlayerObjectType    | 
                                            SimInteriorObjectType  |
                                            SimTerrainObjectType   |
                                            SimShapeObjectType     |
                                            StaticObjectType       |
                                            VehicleObjectType  |
                                            MoveableObjectType |
                                            ItemObjectType         |
                                            MineObjectType;

//------------------------------------------------------------------------------
//--------------------------------------
//
Projectile::ProjectileData::ProjectileData()
{
   bulletShapeName  = NULL;
   explosionTag     = 0;
   expRandCycle     = 0;
   collideWithOwner = false;
   ownerGraceMS     = 1000;
   collisionRadius  = 0.0f;
   mass             = 1.0f;
   damageClass      = 0;
   damageValue      = 0.0f;
   damageType       = 0;
   explosionRadius  = 0.0f;
   kickBackStrength = 0.0f;
   aimDeflection    = 0.0f;
   muzzleVelocity   = 0.0f;
   terminalVelocity = 0.0f;
   acceleration     = 0.0f;
   totalTime        = 10000.0f;
   liveTime         = 10000.0f;
   projSpecialTime  = 0.0f;
   lightRange       = 0.0f;
   lightColor       = ColorF(0, 0, 0);
   inheritedVelocityScale = 0.0f;
   soundId = -1;
}

void
Projectile::ProjectileData::pack(BitStream* bs)
{
   Parent::pack(bs);

   bs->writeString(bulletShapeName);
   bs->write(explosionTag);
   bs->write(expRandCycle);
   bs->writeFlag(collideWithOwner);
   bs->write(ownerGraceMS);
   bs->write(collisionRadius);
   bs->write(mass);
   bs->write(muzzleVelocity);
   bs->write(terminalVelocity);
   bs->write(acceleration);
   bs->write(totalTime);
   bs->write(liveTime);
   bs->write(projSpecialTime);
   bs->writeFlag(projSpecialBool);
   bs->write(lightRange);
   bs->write(lightColor.red);
   bs->write(lightColor.green);
   bs->write(lightColor.blue);
   bs->write(soundId);
   bs->write(explosionRadius);
}

void
Projectile::ProjectileData::unpack(BitStream* bs)
{
   Parent::unpack(bs);

   bulletShapeName = bs->readSTString();

   bs->read(&explosionTag);
   bs->read(&expRandCycle);
   collideWithOwner = bs->readFlag();
   bs->read(&ownerGraceMS);
   bs->read(&collisionRadius);
   bs->read(&mass);
   bs->read(&muzzleVelocity);
   bs->read(&terminalVelocity);
   bs->read(&acceleration);
   bs->read(&totalTime);
   bs->read(&liveTime);
   bs->read(&projSpecialTime);
   projSpecialBool = bs->readFlag();
   bs->read(&lightRange);
   bs->read(&lightColor.red);
   bs->read(&lightColor.green);
   bs->read(&lightColor.blue);
   bs->read(&soundId);
   bs->read(&explosionRadius);
}

bool Projectile::ProjectileData::preload(ResourceManager *rm, bool server, char errorBuffer[256])
{
   server;
   if (bulletShapeName && bulletShapeName[0] != '\0') {
      m_projRes = rm->load(bulletShapeName);
      if(!bool(m_projRes))
      {
         sprintf(errorBuffer, "Unable to load ProjectileData shape: %s", bulletShapeName);
         return false;
      }
   }
   return true;
}

float
Projectile::ProjectileData::getTerminalVelocity() const
{
   return muzzleVelocity;
}

bool
Projectile::ProjectileData::getTargetVector(const Point3F& in_rShootFrom,
                                            const Point3F& in_rShootTo,
                                            const float    /*in_gravity*/,
                                            Point3F*       out_pVector,
                                            bool*          out_pInRange,
                                            const bool     in_secondTarget)
{
   if (in_secondTarget == true)
      return false;

   *out_pVector = (in_rShootTo - in_rShootFrom);
   out_pVector->normalize();
   *out_pInRange = true;

   return true;
}

Projectile::Projectile(const Int32 in_datFileId)
 : m_lastUpdated(0),
   m_renderFacingCamera(false),
   m_projectileIsLive(true),
   m_projectileData(NULL),
   m_hasExploded(false),
   m_waitingForDelete(false),
   m_overrideRender(false),
   m_faceForward(true),
   m_projectileSound(0)
{
   // Set the gamebase dat file name...
   datFileName = NULL;
   datFileId   = in_datFileId;

   m_renderImage.transform.identity();

   forceMask = 0;
   setMass(1.0f);
   setDrag(1.0f);
   setDensity(1.0f);

   type = SimProjectileObjectType;
   m_shooterId = -1;
   m_pShooter  = NULL;
}

Projectile::~Projectile()
{
   delete m_renderImage.nonTranslucentShape;
   delete m_renderImage.translucentShape;
}


bool
Projectile::isProjStatic(SimObject* pObj)
{
   static const UInt32 cs_typeMask = SimInteriorObjectType  |
                                     SimTerrainObjectType   |
                                     ItemObjectType     |
                                     CorpseObjectType   |
                                     StaticObjectType       |
                                     ItemObjectType         |
                                     MineObjectType     |
                                     SimShapeObjectType;

   if (isGhost() == true)
      return true;
   else
      return (pObj->getType() & cs_typeMask) != 0;
}

//--------------------------------------
// Loads the TS::ShapeInst's, also inits
//  the render image...
//--------------------------------------
//
void
Projectile::loadResources(const bool  in_isGhost)
{
   AssertFatal(manager != NULL, "Must be added first...");
   AssertFatal(m_renderImage.shape == NULL, "Cannot call twice...");
   
   ResourceManager* rm = SimResource::get(manager);
   
   AssertFatal(m_projectileData != NULL, "Error, no data!");
   if (in_isGhost == true) {
      if (m_projectileData->bulletShapeName != NULL &&
          strcmp(m_projectileData->bulletShapeName, "") != 0) {
         m_bulletRes = rm->load(m_projectileData->bulletShapeName, true);
         AssertWarn(bool(m_bulletRes) == true,
                    "Unable to load explicitly specified resource in Projectile");
         // Set up the renderImage...
         // For right now, only support drawing of opaque renderimage...
         //
         m_renderImage.translucentShape    = new TS::ShapeInstance(m_bulletRes, *rm);
         m_renderImage.nonTranslucentShape = new TS::ShapeInstance(m_bulletRes, *rm);

         m_renderImage.translucentThread = m_renderImage.translucentShape->CreateThread();
         if (m_renderImage.translucentThread != NULL) {
            int ambient = m_renderImage.translucentThread->GetSequenceIndex("ambient");
         
            if (ambient != -1) {
               m_renderImage.translucentThread->SetSequence(ambient);
               m_renderImage.nonTranslucentThread = m_renderImage.nonTranslucentShape->CreateThread();
               m_renderImage.nonTranslucentThread->SetSequence(ambient);
            } else {
               m_renderImage.translucentShape->DestroyThread(m_renderImage.translucentThread);
               m_renderImage.translucentThread = NULL;
            }
         }
      }
   }
}

int
Projectile::getDatGroup()
{
   return DataBlockManager::ProjectileDataType;
}

bool
Projectile::initResources(GameBase::GameBaseData* io_pDat)
{
   if(!Parent::initResources(io_pDat))
      return false;

   m_projectileData = dynamic_cast<ProjectileData*>(io_pDat);

   setMass(m_projectileData->mass);

   return true;
}

bool
Projectile::applyBaseDamage(SimCollisionInfo& io_rCollision,
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

      pShape->applyDamage(m_projectileData->damageType,
                          m_projectileData->damageValue,
                          pos, getLinearVelocity(),
                          Point3F(0,0,0),
                          getDamageId());
      return true;
   }
   return false;
}

void
Projectile::applyRadiusDamage(const Point3F& io_rDetonationPoint)
{
   AssertFatal(isGhost() == false, "Cannot run on client!");

   if (m_projectileData->explosionRadius > 0.0f) {
		Explosion::applyRadiusDamage(manager,
			m_projectileData->damageType,
			io_rDetonationPoint,getLinearVelocity(),
			m_projectileData->explosionRadius,
			m_projectileData->damageValue,
			m_projectileData->kickBackStrength,
			getDamageId());
   }
}

void
Projectile::killProjectile()
{
   // Default: nothing
   //
   m_projectileIsLive = false;
}

void
Projectile::destroyProjectile()
{
   // Default: nothing
}

void
Projectile::spawnImpactEffect(SimCollisionInfo& io_rCollision)
{
   AssertFatal(isGhost() == false, "This may NOT be run on the client...");

   // First, determine if the object that was hit was a static object,
   //  if so, trust the client to explode properly...
   //
   if (isProjStatic(io_rCollision.object) == true)
      return;

   // Allow derived classes to select a bullethole based on the surface type...
   //
   m_explosionPosition = io_rCollision.surfaces[0].position;
   m_explosionAxis     = io_rCollision.surfaces[0].normal;
   m_explosionId       = getExplosionTag(io_rCollision);
   m_hasExploded = true;
   setMaskBits(FPExplosionMask);
}

void
Projectile::spawnImpactEffect(const Point3F& in_rPosition)
{
   AssertFatal(isGhost() == false, "This may NOT be run on the client...");

   m_explosionPosition = in_rPosition;
   m_explosionAxis     = Point3F(0, 0, 1);
   m_explosionId       = getExplosionTag();
   m_hasExploded = true;
   setMaskBits(FPExplosionMask);
}

void
Projectile::deflectProjectile(const float in_deflection)
{
   AssertFatal(isGhost() == false, "Cannot run on client");
   
   static Random s_random;

   if (in_deflection != 0.0f) {
      EulerF test;
      test.x = (s_random.getFloat() - 0.5) * 2 * M_PI * in_deflection;
      test.y = (s_random.getFloat() - 0.5) * 2 * M_PI * in_deflection;
      test.z = (s_random.getFloat() - 0.5) * 2 * M_PI * in_deflection;
      RMat3F testMat(test);
      
      Vector3F final;
      m_mul(getLinearVelocity(), testMat, &final);
      setLinearVelocity(final);
   }
}

void
Projectile::initProjectile(const TMat3F&     in_rTrans,
                           const Point3F&    in_rShooterVel,
                           const SimObjectId in_shooterId)
{
   AssertFatal(isGhost() == false, "Cannot run on client");
   
   setTransform(in_rTrans);
   setOwnerId(in_shooterId);

   m_shooterId  = in_shooterId;
   m_shooterVel = in_rShooterVel;
}

bool
Projectile::onSimLightQuery ( SimLightQuery * query )
{
   AssertFatal(isGhost() == true, "Should never receive this query on the server...");

   if (m_overrideRender == true) {
      query->count    = 0;
      query->light[0] = NULL;
   } else {
      query->count = 1;
      query->light[0] = &m_light;
      query->ambientIntensity.set();
      m_light.setPosition( getLinearPosition() );
   }

   return true;
}

bool
Projectile::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);
      onQuery(SimCollisionImageQuery);
      onQuery(SimLightQuery);

     default:
      return Parent::processQuery(query);
   }
}

bool
Projectile::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
   
   loadResources(isGhost());

   // so we get server/clientProcess events...
   addToSet(PlayerSetId);

   if (isGhost() == false) {
      bool addSuccess = addToSet("MissionCleanup");
      AssertFatal(addSuccess == true, "Could not add to cleanup group");

      // Get the real pointer to the shooter...
      //
      SimObject* pBase = manager->findObject(m_shooterId);
      if (pBase != NULL) {
         m_pShooter = dynamic_cast<ShapeBase*>(pBase);
         AssertFatal(m_pShooter != NULL, "error, shooter must always be a shapebase");
         deleteNotify(m_pShooter);

         m_shooterClientId = m_pShooter->getControlClient();
         if(!m_shooterClientId)
            m_shooterClientId = m_pShooter->getLastOwnerClient();
         if (m_shooterClientId != 0) {
            PlayerManager* pPM = PlayerManager::get(manager);
            AssertFatal(pPM != NULL, "No Player manager?");
            PlayerManager::BaseRep* pCR = pPM->findBaseRep(m_shooterClientId);
            if (pCR != NULL) {
               m_shooterClientLoginId = pCR->loginId;
            } else {
               m_shooterClientLoginId = -1;
            }
         } else {
            m_shooterClientLoginId = -1;
         }
      } else {
         m_pShooter = NULL;
      }
      m_shooterId = -1;

      // Set the velocity
      Point3F velocityVector;
      getTransform().getRow(1,&velocityVector);
   
      velocityVector *= m_projectileData->muzzleVelocity;
      velocityVector += m_shooterVel * m_projectileData->inheritedVelocityScale;
      setLinearVelocity(velocityVector);

      deflectProjectile(m_projectileData->aimDeflection);

      getTransform().getRow(1, &m_instTerminalVelocity);
      m_instTerminalVelocity *= m_projectileData->terminalVelocity;
   }

   // Projectiles are always added to the container system for ghost scoping...
   //
   AssertFatal(getContainer() == NULL, "Cannot be in a container at this point...");
   SimContainer* pRoot = findObject(manager, SimRootContainerId, pRoot);
   AssertFatal(pRoot != NULL, "No root container...");
   pRoot->addObject(this);

   // It's not necessary to load shape resources if we are the server object...
   //
   if (isGhost() == true) {
      if (m_projectileData->lightRange != 0.0f) {
         m_light.setType(TS::Light::LightPoint);
         m_light.setIntensity(m_projectileData->lightColor);
         m_light.setRange(m_projectileData->lightRange);

         addToSet(SimLightSetId);
      }
   }

   // SimMovement settings...
   //
   collisionMask = csm_collisionMask;
   if (m_projectileData->collideWithOwner == false ||
       (m_projectileData->collideWithOwner == true &&
        m_projectileData->ownerGraceMS > 0)) {
      excludedId = getOwnerId();
   } else {
      excludedId = 0;
   }
   
   setTimeSlice(1.0f/30.0f);
   
   m_lastUpdated = wg->currentTime;
   
   m_ownerGraceMS = m_projectileData->ownerGraceMS + wg->currentTime;
   m_totalTime    = UInt32(m_projectileData->totalTime * 1000.0f) + wg->currentTime;
   m_liveTime     = UInt32(m_projectileData->liveTime  * 1000.0f) + wg->currentTime;

   return true;
}

void
Projectile::onRemove()
{
   stopProjectileSound();

   Parent::onRemove();
}

bool
Projectile::onSimCollisionImageQuery(SimCollisionImageQuery* query)
{
   query->count    = 0; 
   query->image[0] = NULL;
   
   return true;
}


bool
Projectile::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   AssertFatal(isGhost() == true, "Servers shouldn't be rendering!");

   TSRenderContext & rc = *query->renderContext;

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
//   SimContainer * cnt = getContainer();
//   if (cnt)
//      m_renderImage.hazeValue = cnt->getHazeValue(*query->renderContext, m_renderImage.transform.p);
   m_renderImage.hazeValue = 0;

   // Set up basic transform

   // Lets do any special fxRenderImage processing here...
   //
   if (m_renderFacingCamera == true) {
      m_renderImage.transform.p = getTransform().p;
      m_renderImage.faceCamera(rc);
   } else if (m_faceForward == true) {
      m_renderImage.transform.p = getTransform().p;
      Point3F test = getLinearVelocity();
      test.normalize();
      m_renderImage.faceDirection(test);
   } else {
      m_renderImage.transform = getTransform();
   }
    
   // Return render image here...
   //
   m_renderImage.setSortValue(rc.getCamera()); // does nothing if not translucent
   query->image[query->count++] = &m_renderImage;

   return true;
}

void
Projectile::buildContainerBox(const TMat3F& in_rMat,
                                 Box3F*        out_pBox)
{
   // Assumes that the projectile is 0 radius.
   //
   Parent::buildContainerBox(in_rMat, out_pBox);
}

//------------------------------------------------------------------------------
// NAME 
//    bool
//    Projectile::moveProjectile(CollisionSurface& /*out_rSurface*/,
//                                   SimObject*&       /*out_pObject*/)
//    
// DESCRIPTION 
//    Moves the projectile with the SimMovement interface...
//    
// NOTES 
//    Makes the following assumptions:
//     1. Rotations do not cause collisions
//     2. Projectile has radius 0
//     3. Projectile explodes on impact (no bounce calculated)
//     4. Clients don't need to know what they bounced against
//    If any of these are false, the derived class should not call this function
//     in its clientPredict or serverUpdate
//------------------------------------------------------------------------------
void
Projectile::decayGraceTime(const UInt32 in_decayStep)
{
   // Set up to include or exclude the shooter from the test.  Decay the grace period.
   if (m_ownerGraceMS != 0) {
      m_ownerGraceMS -= in_decayStep;
      if (m_ownerGraceMS <= 0)
         excludedId = 0;
   }
}

bool
Projectile::moveProjectile(const UInt32      in_currTime,
                           SimCollisionInfo& out_rCollision,
                           SimObject*&       out_pObject,
                           const bool        in_doCollisions,
                           const bool        in_overrideTimeSlice)
{
   // Steps to take:
   //  1. Set time slice based on last updated time
   //  2. Collide line (start = position, end = position + vel * timeSlice
   //  3. -If collision, return true, with out_variables set.
   //       Note: only if server.  Client object simply returns true
   //  4. -Else updateMovement() and setPosition, return false.

   // 1.
   //
   Int32 intTimeSlice;
   float floatTimeSlice;
   if (in_overrideTimeSlice == true) {
      intTimeSlice = Int32(in_currTime - m_lastUpdated);
      if (intTimeSlice <= 0)
         return false;

      floatTimeSlice = float(intTimeSlice) * 0.001f;
      setTimeSlice(floatTimeSlice);
   } else {
      floatTimeSlice = getTimeSlice();
      intTimeSlice   = Int32(floatTimeSlice * 1000.0f);
   }

   if (m_projectileData->collisionRadius == 0.0f) {
      SimContainerQuery collisionQuery;
      collisionQuery.id     = -1;
      collisionQuery.type   = -1;
      collisionQuery.mask   = csm_collisionMask;
      collisionQuery.detail = SimContainerQuery::DefaultDetail;

      SimContainer* root = findObject(manager, SimRootContainerId, root);
      AssertFatal(root != NULL, "No root object...");
   
      while ((m_lastUpdated + intTimeSlice) <= in_currTime) {
         TMat3F finalPosition;
         // Move the projectile
         updateMovement(&finalPosition);
         decayGraceTime(intTimeSlice);

         if (in_doCollisions == true) {
            if (m_projectileData->collideWithOwner == false || m_ownerGraceMS > 0)
               collisionQuery.id = getOwnerId();
            collisionQuery.box.fMin = getTransform().p;
            collisionQuery.box.fMax = finalPosition.p;

            if (root->findLOS(collisionQuery, &out_rCollision, SimCollisionImageQuery::High)) {
               // Bang!  We hit something!
               out_pObject  = out_rCollision.object;
               return true;
            }
         }
         
         // force position update, we've already collided, or aren't colliding
         //
         setPosition(finalPosition, true);
         m_lastUpdated += intTimeSlice;
      }
   } else {
      // Move a sphere.  Note that these do not return an object, and must do
      //  radius damage...
      //
      while ((m_lastUpdated + intTimeSlice) <= in_currTime) {
         TMat3F finalPosition;
         // Move the projectile
         updateMovement(&finalPosition);
         decayGraceTime(intTimeSlice);

         if (in_doCollisions == true) {
            m_testCollision = false;
            stepPosition(finalPosition, 0.1f, 0.3f);
            if (m_testCollision == true) {
               // Bang!  We hit something!
               out_pObject  = NULL;
               return true;
            }
         } else {
            setPosition(finalPosition, true);
         }
         
         // force position update, we've already collided, or aren't colliding
         //
         m_lastUpdated += intTimeSlice;
      }
   }

   return false;
}

bool
Projectile::processCollision(MovementInfo* info)
{
   bool pReturn = Parent::processCollision(info);
   if (pReturn == true)
      m_testCollision = true;

   return pReturn;
}

void
Projectile::clientProcess(DWORD in_currTime)
{
   // Default is to predict nothing, but animate the thread...
   //
   if (m_renderImage.thread != NULL) {
      float deltaT = float(in_currTime - m_lastUpdated) / 1000.0f;
      m_renderImage.thread->AdvanceTime(deltaT);
   }

   m_lastUpdated = in_currTime;
}

void
Projectile::serverProcess(DWORD in_currTime)
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

   // Default is simply to move forward until we reach the current time...
   //
   SimCollisionInfo collision;
   SimObject* pObject = NULL;
   bool collided = moveProjectile(in_currTime, collision, pObject, true, false);

   if (collided == true) {
      // We collided with an object, the surface info is contained in surface
      //
      AssertFatal(m_projectileData->collisionRadius != 0.0f || pObject != NULL,
                  "NULL collision object with a line collision object");
      
      if (m_projectileData->damageClass == 0) {
         // Apply damage to the collided object
         applyBaseDamage(collision, pObject);

         collision.surfaces[0].transform(collision.surfaces.tWorld);

         // And explode!
         spawnImpactEffect(collision);
      } else if (m_projectileData->damageClass == 1) {
         // Apply radius damage to anyone in the vicinity
         applyRadiusDamage(getLinearPosition());

         // And explode!
         spawnImpactEffect(getLinearPosition());
      } else {
         AssertFatal(false, "misunderstood damageclass");
      }
      
      // We're done.
      //
      m_waitingForDelete = true;
      m_deleteTime       = in_currTime + 500;
   }
}

void
Projectile::readInitialPacket(Net::GhostManager* io_pGM,
                              BitStream*         io_pStream)
{
   unpackDatFile(io_pStream);

   if (io_pStream->readFlag() == true) {
      UInt32 tempId = io_pStream->readInt(10);
      SimNetObject* pShooter = io_pGM->resolveGhost(tempId);
      if (pShooter != NULL) {
         m_pShooter = dynamic_cast<ShapeBase*>(pShooter);
         if (m_pShooter) {
            deleteNotify(m_pShooter);
            setOwnerId(m_pShooter->getId());
         }
         return;
      }
   }
   setOwnerId(-1);
}

void
Projectile::writeInitialPacket(Net::GhostManager* io_pGM,
                               BitStream*         io_pStream)
{
   packDatFile(io_pStream);

   int ownerGhostId;
   if (m_pShooter != NULL) {
      ownerGhostId = io_pGM->getGhostIndex(m_pShooter);
   } else {
      ownerGhostId = -1;
   }

   if (ownerGhostId != -1) {
      io_pStream->writeFlag(true);
      io_pStream->writeInt(ownerGhostId, 10);
   } else {
      io_pStream->writeFlag(false);
   }
}

Int32
Projectile::getBulletHoleIndex(SimCollisionInfo& /*io_rCollision*/)
{
   return -1;
}

Int32
Projectile::getExplosionTag(SimCollisionInfo& /*io_rCollision*/)
{
   // By default, ignore surface information...
   //
   return Projectile::getExplosionTag();
}

Int32
Projectile::getExplosionTag()
{
   AssertFatal(m_projectileData != NULL, "No attributes?");
   static Random s_randomGenerator;

   if (m_projectileData->expRandCycle <= 1) {
      return m_projectileData->explosionTag;
   } else {
      float floatRand = s_randomGenerator.getFloat() * m_projectileData->expRandCycle;
      int intRand(floatRand);

      return m_projectileData->explosionTag + intRand;
   }
}

float
Projectile::getInheritedVelScale() const
{
   AssertFatal(m_projectileData != NULL, "Error, must have attribs loaded...");
   return m_projectileData->inheritedVelocityScale;
}

DWORD
Projectile::packUpdate(Net::GhostManager* gm,
                       DWORD              mask,
                       BitStream*         stream)
{
   DWORD parentPunt = Parent::packUpdate(gm, mask, stream);

   if ((mask & InitialUpdate) == 0) {
      stream->writeFlag(false);
      if (m_hasExploded == true) {
         stream->writeFlag(true);
         writeExplosion(stream);
      } else {
         stream->writeFlag(false);
      }
   } else { 
      stream->writeFlag(true);
      if (m_hasExploded == true) {
         parentPunt |= (mask & FPExplosionMask);
      }
   }

   return parentPunt;
}

void
Projectile::unpackUpdate(Net::GhostManager* gm,
                         BitStream*         stream)
{
   Parent::unpackUpdate(gm, stream);

   if (stream->readFlag() == false) {
      if (stream->readFlag() == true) {
         readExplosion(stream);
      }
   }
}

void
Projectile::setEnergy(const float, const float)
{
   // Ignore by default
}

bool
Projectile::isSustained() const
{
   return false;
}

void
Projectile::shutOffProjectile()
{
   AssertFatal(0, "should not be here");
}

void
Projectile::updateImageTransform(const TMat3F&)
{
   AssertFatal(0, "should not be here");
}

bool
Projectile::isTargetable() const
{
   return false;
}

void
Projectile::setTarget(ShapeBase* /*io_pTarget*/)
{
   AssertFatal(0, "should not be here");
}

float
Projectile::getTargetRange() const
{
   AssertFatal(0, "should not be here");
   return 0.0f;
}

void
Projectile::updateProjectileSound(const TMat3F&  in_rTrans,
                                  const Point3F& in_rVel)
{
   if (m_projectileData->soundId != -1) {
      if (m_projectileSound == 0) {
         m_projectileSound = TSFX::PlayAt(m_projectileData->soundId, in_rTrans, in_rVel);
      } else {
         // Update the handle...
         Sfx::Manager* pMan = Sfx::Manager::find(manager);
         if (pMan->selectHandle(m_projectileSound))
            pMan->setTransform(in_rTrans, in_rVel);
      }
   }
}

void
Projectile::stopProjectileSound()
{
   if (m_projectileData->soundId != -1) {
      if (m_projectileSound != 0) {
         Sfx::Manager::Stop(manager, m_projectileSound);
         m_projectileSound = 0;
      }
   }
}

void
Projectile::onDeleteNotify(SimObject* io_pDelete)
{
   if (io_pDelete == (SimObject*)m_pShooter)
      m_pShooter = NULL;

   Parent::onDeleteNotify(io_pDelete);
}

Int32
Projectile::getDamageId()
{
   // Check to make sure that the client that owns the number is
   //  the correct client...
   if (m_shooterClientId == 0)
      return 0;

   PlayerManager* pPM = PlayerManager::get(manager);
   AssertFatal(pPM, "No playermanager");
   PlayerManager::ClientRep* pCR = pPM->findClient(m_shooterClientId);

   if (pCR == NULL || pCR->loginId != m_shooterClientLoginId)
      return 0;
   
   return m_shooterClientId;
}