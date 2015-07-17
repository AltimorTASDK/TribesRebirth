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
#include <netGhostManager.h>
#include <simResource.h>
#include <simEvDcl.h>
#include <stringTable.h>
#include <simTerrain.h>

#include "fearPlayerPSC.h"
#include "projLaser.h"
#include "fear.strings.h"
#include "fearDcl.h"
#include "gameBase.h"
#include "shapeBase.h"
#include "dataBlockManager.h"
#include "fearGlobals.h"
#include "player.h"
#include "PlayerManager.h"
#include "tsfx.h"

IMPLEMENT_PERSISTENT_TAG(LaserProjectile, LaserProjectilePersTag);

// All damageable objects are derived from ShapeBase.
static const int DamageMask = SimPlayerObjectType   | 
                              StaticObjectType      |
                              VehicleObjectType     |
                              MoveableObjectType    |
                              MineObjectType;

//-------------------------------------- LaserProjectile
float csg_beamRadius = 0.04f;


LaserProjectile::LaserData::LaserData()
{
   pBitmapName   = "laserPulse.bmp";
   pHitShapeName = NULL;
   detachFromShooter = false;
}

void
LaserProjectile::LaserData::pack(BitStream* bs)
{
   Parent::pack(bs);

   bs->writeString(pBitmapName);
   bs->writeString(pHitShapeName);
   bs->writeFlag(detachFromShooter);
}

void
LaserProjectile::LaserData::unpack(BitStream* bs)
{
   Parent::unpack(bs);

   pBitmapName   = bs->readSTString();
   pHitShapeName = bs->readSTString();
   detachFromShooter = bs->readFlag();
}

float
LaserProjectile::LaserData::getTerminalVelocity() const
{
   return 10000000.0f;
}


//------------------------------------------------------------------------------
//--------------------------------------
// LaserProjectile implementation...
//--------------------------------------
//
LaserProjectile::LaserProjectile(const Int32 in_datFileId)
 : Projectile(in_datFileId),
   m_startPoint(0, 0, 0),
   m_endPoint(0, 0, 0)
{
   netFlags.set(Ghostable);
   netFlags.set(ScopeAlways);

   m_hitImage.nonTranslucentShape = NULL;
   m_hitImage.translucentShape    = NULL;
}

LaserProjectile::LaserProjectile()
 : Projectile(-1),
   m_startPoint(0, 0, 0),
   m_endPoint(0, 0, 0)
{
   netFlags.set(Ghostable);
   netFlags.set(ScopeAlways);

   m_hitImage.nonTranslucentShape = NULL;
   m_hitImage.translucentShape    = NULL;
}

LaserProjectile::~LaserProjectile()
{
   delete m_hitImage.translucentShape;
   m_hitImage.translucentShape  = NULL;
   m_hitImage.translucentThread = NULL;
}

void
LaserProjectile::loadResources(const bool in_isGhost)
{
   Parent::loadResources(in_isGhost);

   if (in_isGhost == true) {
      AssertFatal(m_laserData != NULL, "No laser data!");
      ResourceManager* rm = SimResource::get(manager);
      AssertFatal(rm != NULL, "No Resource manager?");

      m_resLaserBitmap = rm->load(m_laserData->pBitmapName, true);
      AssertFatal(bool(m_resLaserBitmap) != false, "Error, couldn't load laser bitmap");

      m_resHitShape = rm->load(m_laserData->pHitShapeName, true);
      if (bool(m_resHitShape) == true) {
         m_hitImage.translucentShape     = new TS::ShapeInstance(m_resHitShape, *rm);
         m_hitImage.translucentThread = m_hitImage.translucentShape->CreateThread();
         m_hitImage.nonTranslucentShape  = NULL;
         m_hitImage.nonTranslucentThread = NULL;
      
         if (m_hitImage.translucentThread != NULL) {
            int ambient = m_hitImage.translucentThread->GetSequenceIndex("ambient");
      
            if (ambient != -1) {
               m_hitImage.translucentThread->SetSequence(ambient);
               float timeScale = m_hitImage.translucentThread->getSequence().fDuration / m_laserData->liveTime;
               m_hitImage.translucentThread->setTimeScale(timeScale);
            } else {
               m_hitImage.translucentShape->DestroyThread(m_hitImage.translucentThread);
               m_hitImage.translucentThread = NULL;
            }
         }
         m_hitImage.transform.identity();
         m_hitImage.shape  = m_hitImage.translucentShape;
         m_hitImage.thread = m_hitImage.translucentThread;
         m_hitImage.itype  = SimRenderImage::Translucent;
      }
   }
}

bool
LaserProjectile::initResources(GameBase::GameBaseData* io_pDat)
{
   if(!Parent::initResources(io_pDat))
      return false;

   m_laserData = dynamic_cast<LaserData*>(io_pDat);

   setMass(m_projectileData->mass);

   return true;
}

void
LaserProjectile::buildContainerBox(const TMat3F& /*in_rMat*/,
                                       Box3F*        out_pBox)
{
   out_pBox->fMin = m_startPoint;
   out_pBox->fMax = m_endPoint;

   out_pBox->fMax.setMax(m_startPoint);
   out_pBox->fMin.setMin(m_endPoint);
}


bool
LaserProjectile::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   // We are scope always, not scoped through the containers...
   //
   SimContainer* cont = getContainer();
   AssertFatal(cont != NULL, "Have to be in the container system at this point")
   cont->removeObject(this);
   
   m_spawnTime = wg->currentTime;

   if (isGhost() == true) {
      addToSet(SimRenderSetId);
      addToSet(SimLightSetId);
      m_endLight.setType(TS::Light::LightPoint);
      m_endLastReadjusted = cg.currentTime;
      m_hitImage.transform.p = m_endPoint;

      if (m_hitTarget == true) {
         // Play the sound for the impact;
         //
         if (m_projectileData->soundId != -1)
			   TSFX::PlayAt(m_projectileData->soundId, m_hitImage.transform, Point3F(0, 0, 0));
      }
   } else {
      // find the first point of contact along this line, out to 4000 meters...
      //
      SimTerrain* pTerrain = (SimTerrain*)manager->findObject(SimTerrainId);
      AssertFatal(pTerrain, "error, no terrain");
      float dist = pTerrain->getVisibleDistance();
      if (dist >= 4000.0f)
         dist = 4000.0f;

      m_endPoint = m_startPoint + (m_directionVector * dist);

      SimCollisionInfo  collisionInfo;
      SimContainerQuery collisionQuery;

      collisionQuery.id       = m_pShooter != NULL ? m_pShooter->getId() : 0;
      collisionQuery.type     = -1;
      collisionQuery.mask     = csm_collisionMask;
      collisionQuery.detail   = SimContainerQuery::DefaultDetail;
      collisionQuery.box.fMin = m_startPoint;
      collisionQuery.box.fMax = m_endPoint;

      SimContainer* pRoot = findObject(manager, SimRootContainerId, pRoot);
      AssertFatal(pRoot != NULL, "No root container?");

      if (pRoot->findLOS(collisionQuery, &collisionInfo)) {
#ifdef DEBUG
         Console->printf("Hit object (%s) of type: %s", collisionInfo.object->getName(), collisionInfo.object->getClassName());
#endif

         // Translate collision to world coordinates...
         m_mul(collisionInfo.surfaces[0].position, collisionInfo.surfaces.tWorld, &m_endPoint);
         
         if (collisionInfo.object->getType().test(DamageMask) == true) {
            ShapeBase* pShape = static_cast<ShapeBase*>(collisionInfo.object);

            float applyDamage = m_finalDamage * m_laserData->damageValue;
            if (applyDamage != 0.0f) {
               pShape->applyDamage(LaserDamageType,
                                   applyDamage,
                                   m_endPoint,
                                   (m_endPoint - m_startPoint),
                                   Point3F(0, 0, 0),
                                   getDamageId());
            }
         }
         m_hitTarget = true;
      } else {
         // Didn't hit anything, beam fizzles at 4k meters
         //
         m_hitTarget = false;
      }

      // And we're set...
      setLinearVelocity(Point3F(0, 0, 0));
   }

   return true;
}

bool
LaserProjectile::readjustEnd()
{
   m_endLastReadjusted = cg.currentTime;

   if (m_laserData->detachFromShooter == false) {
      // Client need to find the owner object, if it exists, then update it's m_startPosition
      //  based on the position of the weapon.  for now, simply leave it as is...
      //
      Player*    pShooter     = NULL;
      if (m_pShooter != NULL) {
         pShooter = dynamic_cast<Player*>(m_pShooter);
      }

      if (pShooter != NULL) {
         TMat3F final;
         if (pShooter->getAimedMuzzleTransform(0, &final) == true) {
            // Ok, if we are not clientRep predicting the laser, just set the direction,
            //  and get out.  Otherwise, we need to recast the laser.
            UInt32 shooterId = m_pShooter != NULL? m_pShooter->getId() : 0;
            m_startPoint = final.p;
            if (cg.psc->getControlObject()->getId() != shooterId) {
               m_directionVector = (m_endPoint - m_startPoint).normalize();
            } else {
               SimTerrain* pTerrain = (SimTerrain*)manager->findObject(SimTerrainId);
               AssertFatal(pTerrain, "error, no terrain");
               float dist = pTerrain->getVisibleDistance();
               if (dist >= 4000.0f)
                  dist = 4000.0f;

               final.getRow(1, &m_directionVector);
               m_directionVector.normalize();
               m_endPoint = m_startPoint + (m_directionVector * dist);

               SimCollisionInfo  collisionInfo;
               SimContainerQuery collisionQuery;

               collisionQuery.id       = shooterId;
               collisionQuery.type     = -1;
               collisionQuery.mask     = csm_collisionMask;
               collisionQuery.detail   = SimContainerQuery::DefaultDetail;
               collisionQuery.box.fMin = m_startPoint;
               collisionQuery.box.fMax = m_endPoint;

               SimContainer* pRoot = findObject(manager, SimRootContainerId, pRoot);
               AssertFatal(pRoot != NULL, "No root container?");

               if (pRoot->findLOS(collisionQuery, &collisionInfo)) {
                  // Translate collision to world coordinates...
                  m_mul(collisionInfo.surfaces[0].position, collisionInfo.surfaces.tWorld, &m_endPoint);
                  m_hitTarget = true;
               } else {
                  m_hitTarget = false;
               }
            }
         } else {
            return false;
         }
      }
   }

   return true;
}

bool
LaserProjectile::onSimLightQuery(SimLightQuery* query)
{
   UInt32 currLength      = wg->currentTime - m_spawnTime;
   float  intensityFactor = 1.0f - ((float(currLength) / 1000.0f) / m_projectileData->liveTime);

   if (intensityFactor <= 0.0f) {
      query->count    = 0;
      query->light[0] = NULL;
      return true;
   }

   if (m_endLastReadjusted != cg.currentTime) {
      if (readjustEnd() == false) {
         query->count    = 0;
         query->light[0] = NULL;

         return true;
      }
   }

   m_endLight.setPosition(m_endPoint);
   m_endLight.setIntensity(m_projectileData->lightColor * intensityFactor);
   m_endLight.setRange(m_projectileData->lightRange);

   query->count    = 1;
   query->light[0] = &m_endLight;

   return true;
}

bool
LaserProjectile::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   // We render through the render group rather than the container system...
   //
   if (query->containerRenderQuery == true || m_overrideRender == true) {
      query->count    = 0;
      query->image[0] = NULL;
      return true;
   }

   if (wg->currentTime > m_spawnTime + UInt32(m_projectileData->liveTime * 1000.0f)) {
      query->count    = 0;
      query->image[0] = NULL;
      return true;
   }

   if (readjustEnd() == false) {
      query->count    = 0;
      query->image[0] = NULL;
      return true;
   }

   query->count    = 1;
   query->image[0] = &m_beamRenderImage;

   m_beamRenderImage.m_startPoint = m_startPoint;
   m_beamRenderImage.m_endPoint   = m_endPoint;

   // Face the camera!
   TSRenderContext& rc = *query->renderContext;
   Point3F vec1 = rc.getCamera()->getTCW().p;
   vec1 -= m_beamRenderImage.m_startPoint;
   m_cross(vec1, m_directionVector, &m_beamRenderImage.m_perpVec);
   m_beamRenderImage.m_perpVec.normalize();

   float invFactor = rc.getCamera()->transformProjectRadius(m_endPoint, csg_beamRadius);
   if (invFactor < 1.0f)
      m_beamRenderImage.m_endFactor = 1.0f / invFactor;
   else
      m_beamRenderImage.m_endFactor = 1.0f;
   
   invFactor = rc.getCamera()->transformProjectRadius(m_startPoint, csg_beamRadius);
   if (invFactor < 1.0f)
      m_beamRenderImage.m_startFactor = 1.0f / invFactor;
   else
      m_beamRenderImage.m_startFactor = 1.0f;

   m_beamRenderImage.m_time       = (1.0f - m_startIntensity) + 
                                    m_startIntensity * (float(wg->currentTime - m_spawnTime) /
                                                        (m_projectileData->liveTime * 1000.0f));
   m_beamRenderImage.m_pBitmap    = m_resLaserBitmap;

   // Set the hit effect...
   //
   if (bool(m_resHitShape) == true && m_hitTarget) {
      m_hitImage.faceCamera(rc);
      m_hitImage.setSortValue(rc.getCamera()); // does nothing if not translucent
      query->image[query->count++] = &m_hitImage;
   }

   return true;
}

bool
LaserProjectile::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);
      onQuery(SimLightQuery);

     default:
      return Parent::processQuery(query);
   }
}


void
LaserProjectile::initProjectile(const TMat3F&     in_rTrans,
                                const Point3F&    /*in_rShooterVel*/,
                                const SimObjectId in_shooterId)
{
   AssertFatal(isGhost() == false, "Cannot run on client");
   
   m_finalDamage = 0.0f;

   setTransform(in_rTrans);

   m_shooterId  = in_shooterId;
   m_startPoint = in_rTrans.p;

   // Set the velocity
   in_rTrans.getRow(1, &m_directionVector);
}

void
LaserProjectile::setEnergy(const float in_energy,
                           const float in_pOfM)
{
   m_finalDamage = in_energy * in_pOfM;
   if (m_finalDamage > 55.0f)
      m_startIntensity = 1.0f;
   else
      m_startIntensity = m_finalDamage / 55.0f;
}

void
LaserProjectile::clientProcess(DWORD in_currTime)
{
   Box3F newBox;
   TMat3F dummy;
   buildContainerBox(dummy, &newBox);
   setBoundingBox(newBox);

   if (m_hitImage.thread != NULL) {
      float deltaT = float(in_currTime - m_lastUpdated) / 1000.0f;
      m_hitImage.thread->AdvanceTime(deltaT);
   }

   m_lastUpdated = in_currTime;
}

void
LaserProjectile::serverProcess(DWORD in_currTime)
{
   if (in_currTime > m_spawnTime + UInt32(m_projectileData->liveTime * 1000.0f))
      deleteObject();
}

void
LaserProjectile::writeInitialPacket(Net::GhostManager* io_pGM,
                                    BitStream*         pStream)
{
   Parent::writeInitialPacket(io_pGM, pStream);

   // We only need this in the case that we don't have an object to find it
   //  from on the other side.
   int ownerGhostId;
   if (m_pShooter != NULL) {
      ownerGhostId = io_pGM->getGhostIndex(m_pShooter);
   } else {
      ownerGhostId = -1;
   }

   if (ownerGhostId == -1) {
      pStream->write(sizeof(Point3F), &m_startPoint);
   }
   pStream->writeFlag(m_hitTarget);

   pStream->write(sizeof(Point3F), &m_endPoint);

   pStream->writeFloat(m_startIntensity, 5);
}

void
LaserProjectile::readInitialPacket(Net::GhostManager* io_pGM,
                                   BitStream*         pStream)
{
   Parent::readInitialPacket(io_pGM, pStream);

   if (m_pShooter == NULL) {
      pStream->read(sizeof(Point3F), &m_startPoint);
   } else {
      // We need to catch the transform from the shooter now, in case he is deleted
      //  shortly.
      TMat3F final;
      if (m_pShooter->getMuzzleTransform(0, &final) == true) {
         if (dynamic_cast<Player*>(m_pShooter) != NULL) {
            TMat3F temp;
            m_mul(final, m_pShooter->getTransform(), &temp);
            final = temp;
         }
            
         m_startPoint = final.p;
      } else {
         AssertFatal(false, "Error, should never be unable to get muzzle transform in this case");
         m_startPoint = Point3F(0, 0, 0);
      }
   }
   m_hitTarget = pStream->readFlag();

   pStream->read(sizeof(Point3F), &m_endPoint);

   m_startIntensity = pStream->readFloat(5);
   m_directionVector = (m_endPoint - m_startPoint).normalize();
}

DWORD
LaserProjectile::packUpdate(Net::GhostManager* gm,
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
   } else {
      stream->writeFlag(false);
      // Bullets never update
   }
   
   return parentPunt;
}

void
LaserProjectile::unpackUpdate(Net::GhostManager* gm,
                              BitStream*         stream)
{
   Parent::unpackUpdate(gm, stream);
   
   if (stream->readFlag() == true) {
      // Is initial packet...
      //
      readInitialPacket(gm, stream);
   } else {
      AssertFatal(0, "only one update allowed!");
   }
}


void
LaserProjectile::BeamRenderImage::render(TS::RenderContext& io_rc)
{
   TS::PointArray* pArray = io_rc.getPointArray();
   GFXSurface*     srf    = io_rc.getSurface();
   pArray->reset();

   Point3F beamPts[4];
   
   // Draw our bbox
   beamPts[0] = m_startPoint - (m_perpVec * csg_beamRadius * m_startFactor);
   beamPts[1] = m_startPoint + (m_perpVec * csg_beamRadius * m_startFactor);
   beamPts[2] = m_endPoint   + (m_perpVec * csg_beamRadius * m_endFactor);
   beamPts[3] = m_endPoint   - (m_perpVec * csg_beamRadius * m_endFactor);

   AssertFatal(m_pBitmap != NULL, "No texture?");

   Point2F texVerts[2];
   if (srf->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA) {
      srf->setFillMode(GFX_FILL_TEXTURE);
      srf->setShadeSource(GFX_SHADE_NONE);
      srf->setHazeSource(GFX_HAZE_NONE);

      texVerts[0].y = 0;
      texVerts[1].y = 1;
      if ((m_pBitmap->attribute & BMA_ADDITIVE) != 0)
         srf->setAlphaSource(GFX_ALPHA_ADD);
      else if ((m_pBitmap->attribute & BMA_SUBTRACTIVE) != 0)
         srf->setAlphaSource(GFX_ALPHA_SUB);
      else
         srf->setAlphaSource(GFX_ALPHA_TEXTURE);
      srf->setTextureMap(m_pBitmap);
      srf->setTexturePerspective(false);
      srf->setTransparency(false);
      pArray->useTextures(true);
      pArray->useTextures(texVerts);
   } else {
      srf->setFillMode(GFX_FILL_CONSTANT);
      srf->setShadeSource(GFX_SHADE_NONE);
      srf->setHazeSource(GFX_HAZE_NONE);

      UInt32 yVal = m_pBitmap->getHeight() / 2;
      UInt32 xVal = m_pBitmap->getWidth() * m_time;

      UInt8* pColor = m_pBitmap->getAddress(xVal, yVal);

      srf->setAlphaSource(GFX_ALPHA_FILL);
      pArray->useTextures(false);
      srf->setFillColor(*pColor, 1975);
   }

   texVerts[0].x = m_time;
   texVerts[1].x = m_time;

   pArray->useIntensities(false);
   int index = pArray->addPoints(4, beamPts);

   TS::VertexIndexPair polyPairs[8];
   polyPairs[0].fVertexIndex  = index + 0;
   polyPairs[0].fTextureIndex = 0;
   polyPairs[1].fVertexIndex  = index + 1;
   polyPairs[1].fTextureIndex = 1;
   polyPairs[2].fVertexIndex  = index + 2;
   polyPairs[2].fTextureIndex = 1;
   polyPairs[3].fVertexIndex  = index + 3;
   polyPairs[3].fTextureIndex = 0;

   polyPairs[4].fVertexIndex  = index + 3;
   polyPairs[4].fTextureIndex = 0;
   polyPairs[5].fVertexIndex  = index + 2;
   polyPairs[5].fTextureIndex = 1;
   polyPairs[6].fVertexIndex  = index + 1;
   polyPairs[6].fTextureIndex = 1;
   polyPairs[7].fVertexIndex  = index + 0;
   polyPairs[7].fTextureIndex = 0;

   pArray->drawPoly(4, polyPairs + 0, 0);
   pArray->drawPoly(4, polyPairs + 4, 0);
}

int 
LaserProjectile::getDatGroup()
{
   return DataBlockManager::LaserDataType;
}

void
LaserProjectile::readExplosion(BitStream*)
{
   AssertFatal(0, "Should never explode a laser!");
}

void
LaserProjectile::writeExplosion(BitStream*)
{
   AssertFatal(0, "Should never explode a laser!");
}
