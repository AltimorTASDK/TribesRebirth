//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <simResource.h>
#include <m_random.h>
#include <g_surfac.h>
#include <console.h>
#include <netGhostManager.h>
#include "projLightning.h"
#include "dataBlockManager.h"
#include "fearDcl.h"
#include "fearGlobals.h"
#include "player.h"

IMPLEMENT_PERSISTENT_TAG(Lightning, LightningPersTag);

const DWORD Lightning::csm_targetUpdateFreq = 100;

static Random sg_randGen;

Lightning::LightningData::LightningData()
{
   boltLength        = 5.0f;
   coneAngle         = 0.0f;
   damagePerSec      = 0.0f;
   energyDrainPerSec = 0.0f;
   segmentDivisions  = 1;
   skipPercent       = 0.0;
   displaceBias      = 0.25f;
   beamWidth         = 0.2f;
   pBitmapName       = "lightningTemp.bmp";
}

void
Lightning::LightningData::pack(BitStream* bs)
{
   Parent::pack(bs);

   bs->write(segmentDivisions);
   bs->write(numSegments);
   bs->write(updateTime);
   bs->write(skipPercent);
   bs->write(displaceBias);
   bs->write(beamWidth);
   bs->writeString(pBitmapName);
}

void
Lightning::LightningData::unpack(BitStream* bs)
{
   Parent::unpack(bs);

   bs->read(&segmentDivisions);
   bs->read(&numSegments);
   bs->read(&updateTime);
   bs->read(&skipPercent);
   bs->read(&displaceBias);
   bs->read(&beamWidth);
   pBitmapName = bs->readSTString();
}

float
Lightning::LightningData::getTerminalVelocity() const
{
   return 10000000.0f;
}

//------------------------------------------------------------------------------
//--------------------------------------
//
Lightning::Lightning(const Int32 in_datFile)
 : Projectile(in_datFile),
   m_pLightningData(NULL),
   m_waitingForShutoff(false)
{
   //
   netFlags.set(Ghostable);
//   netFlags.set(ScopeAlways);

   m_pCurrentTarget = NULL;
   m_pLightningImage = new LightningRenderImage;
}

Lightning::Lightning()
 : Projectile(-1),
   m_pLightningData(NULL),
   m_waitingForShutoff(false)
{
   //
   netFlags.set(Ghostable);
//   netFlags.set(ScopeAlways);

   m_pCurrentTarget = NULL;
   m_pLightningImage = new LightningRenderImage;
}

Lightning::~Lightning()
{
   delete m_pLightningImage;
   m_pLightningImage = NULL;
}

bool
Lightning::isSustained() const
{
   return true;
}

void
Lightning::shutOffProjectile()
{
   AssertFatal(isGhost() == false, "Error, cannot shut off the client side...");

   if (m_pCurrentTarget != NULL) {
      clearNotify(m_pCurrentTarget);
      setMaskBits(TargetMask);
   }
   m_pCurrentTarget    = NULL;
   m_waitingForShutoff = true;
}

void
Lightning::determineTarget()
{
   if (m_lastTarget + csm_targetUpdateFreq > wg->currentTime)
      return;
   m_lastTarget = wg->currentTime;

   Point3F directionVector;
   getTransform().getRow(1, &directionVector);
   Point3F beginSegment = getTransform().p;
   Point3F endSegment   = beginSegment + (directionVector * m_pLightningData->boltLength);

   SimCollisionInfo  info;
   SimContainerQuery query;
   query.id       = m_pShooter != NULL ? m_pShooter->getId() : 0;
   query.type     = -1;
   query.mask     = SimPlayerObjectType |
                    SimShapeObjectType  | SimVehicleObjectType |
                    VehicleObjectType   | MineObjectType       |
                    StaticObjectType    | MoveableObjectType;
   query.detail   = SimContainerQuery::DefaultDetail;
   query.box.fMin = beginSegment;
   query.box.fMax = endSegment;

   SimContainer* pRoot = (SimContainer*)manager->findObject(SimRootContainerId);
   AssertFatal(pRoot != NULL, "No root container?");

   if (pRoot->findLOS(query, &info, SimCollisionImageQuery::High) == true) {
      ShapeBase* pObject = dynamic_cast<ShapeBase*>(info.object);
      AssertFatal(pObject != NULL, "This should never hit anything but shapebases?");

      // Make sure we can see this target...
      //
      query.mask     = SimInteriorObjectType | SimTerrainObjectType;
      query.box.fMin = beginSegment;
      query.box.fMax = pObject->getBoxCenter();
      if (pRoot->findLOS(query, &info, SimCollisionImageQuery::High) == true) {
         if (info.object != (SimObject*)pObject) {
            if (m_pCurrentTarget != NULL) {
               clearNotify(m_pCurrentTarget);
               setMaskBits(TargetMask);
            }
            m_pCurrentTarget = NULL;
            return;
         }
      }

      if (m_pCurrentTarget != pObject) {
         if (m_pCurrentTarget)
            clearNotify(m_pCurrentTarget);
         setMaskBits(TargetMask);
      }

      m_mul(info.surfaces[0].position, info.surfaces.tWorld, &m_currentTargetPosition);
      if(m_currentTargetPosition == getTransform().p)
         m_currentTargetPosition += directionVector * 0.1;

      m_pCurrentTarget = pObject;
      deleteNotify(m_pCurrentTarget);
      AssertFatal(m_pCurrentTarget != NULL, "Error, struck object not of type shapebase");
   } else {
      float dotThresh = cos(m_pLightningData->coneAngle * M_PI / 180.0f);
      float sinAngle  = sin(m_pLightningData->coneAngle * M_PI / 180.0f);

      // Ok, so the LOS didn't find a target, try a cone...
      //
      query.box.fMin = beginSegment;
      query.box.fMax = beginSegment;
      query.box.fMin.setMin(endSegment - directionVector * m_pLightningData->boltLength * sinAngle);
      query.box.fMax.setMax(endSegment + directionVector * m_pLightningData->boltLength * sinAngle);

      SimContainerList scList;
      pRoot->findIntersections(query, &scList);

      float closeDot = 2.0f;
      ShapeBase* pObject = NULL;
      for (int i = 0; i < scList.size(); i++) {
         ShapeBase* pTest = dynamic_cast<ShapeBase*>(scList[i]);
         if (pTest == NULL)
            continue;

         Point3F testVec = pTest->getBoxCenter() - beginSegment;
         float   len     = m_dot(testVec, directionVector);
         if (len > m_pLightningData->boltLength)
            continue;

         len /= testVec.len();
         if (len >= dotThresh && len < closeDot) {
            // Make sure we can see this target...
            //
            query.mask     = SimPlayerObjectType   |
                             SimShapeObjectType    | SimVehicleObjectType |
                             VehicleObjectType     | MineObjectType       |
                             StaticObjectType      | MoveableObjectType   |
                             SimInteriorObjectType | SimTerrainObjectType;
            query.box.fMin = beginSegment;
            query.box.fMax = pTest->getBoxCenter();
            if (pRoot->findLOS(query, &info, SimCollisionImageQuery::High) == true)
               if (info.object != (SimObject*)pTest)
                  continue;

            closeDot = len;
            pObject  = pTest;
         }
      }

      if (pObject != NULL) {
         if ((SimContainer*)m_pCurrentTarget != info.object) {
            if (m_pCurrentTarget)
               clearNotify(m_pCurrentTarget);
            setMaskBits(TargetMask);
         }
         m_pCurrentTarget = pObject;
         m_currentTargetPosition = pObject->getBoxCenter();
         deleteNotify(m_pCurrentTarget);

         return;
      }

      if (m_pCurrentTarget != NULL) {
         clearNotify(m_pCurrentTarget);
         setMaskBits(TargetMask);
      }
      m_pCurrentTarget = NULL;
   }
}

void
Lightning::updateImageTransform(const TMat3F& in_rTrans)
{
   setTransform(in_rTrans);
}

void
Lightning::loadResources(const bool in_isGhost)
{
   Parent::loadResources(in_isGhost);

   if (in_isGhost == true) {
      AssertFatal(m_pLightningData != NULL, "No lightning data!");
      ResourceManager* rm = SimResource::get(manager);
      AssertFatal(rm != NULL, "No Resource manager?");

      m_resBitmap = rm->load(m_pLightningData->pBitmapName, true);
      AssertFatal(bool(m_resBitmap) != false, "Error, couldn't load lightning bitmap");
   }
}

bool
Lightning::initResources(GameBase::GameBaseData* io_pDat)
{
   if(!Parent::initResources(io_pDat))
      return false;

   m_pLightningData = dynamic_cast<LightningData*>(io_pDat);

   

   return true;
}

void
Lightning::initProjectile(const TMat3F&     in_rTrans,
                          const Point3F&    /*in_rShooterVel*/,
                          const SimObjectId in_shooterId)
{
   AssertFatal(isGhost() == false, "Cannot run on client");
   
   setTransform(in_rTrans);
   m_shooterId  = in_shooterId;
}

bool
Lightning::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);
      onQuery(SimLightQuery);

     default:
      return Parent::processQuery(query);
   }
}

bool
Lightning::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
   setLinearVelocity(Point3F(0,0,0));

   addToSet(SimLightSetId);

   if (isGhost() == false) {
      m_ghostTimer = wg->currentTime;
      m_lastTarget = wg->currentTime - csm_targetUpdateFreq;
      
      determineTarget();
   } else {
      setTransform(getTransform());
      m_pLightningImage->clearPoints();

      m_pLightningImage->pLightning = this;
      m_startInterpTime = 0;
      m_endInterpTime = 0;
   }

   return true;
}

void
Lightning::writeInitialPacket(Net::GhostManager* gm,
                              BitStream*         pStream)
{
   Parent::writeInitialPacket(gm, pStream);

   int victimGhostId = getVictimGhostId(gm);
   if (pStream->writeFlag(isAttached() && (victimGhostId != -1))) {
      pStream->writeInt(victimGhostId, 10);

      Point3F targetOffset = (m_currentTargetPosition -
                              m_pCurrentTarget->getTransform().p);
      pStream->write(sizeof(Point3F), &targetOffset);
   }
}

void
Lightning::readInitialPacket(Net::GhostManager* gm,
                             BitStream*         pStream)
{
   Parent::readInitialPacket(gm, pStream);

   bool hitting = pStream->readFlag();
   if (hitting) {
      int ghostIdNew = pStream->readInt(10);

      pStream->read(sizeof(Point3F), &m_targetOffsetPosition);

      AssertFatal(m_pCurrentTarget == NULL, "Should not have a target here...");
      SimNetObject* pTarget = gm->resolveGhost(ghostIdNew);
      AssertFatal(pTarget != NULL, "Hm, should never have trans. ghost id");
      m_pCurrentTarget = dynamic_cast<ShapeBase*>(pTarget);
      AssertFatal(m_pCurrentTarget, "Ghost isn't a shapebase!");
      deleteNotify(m_pCurrentTarget);

      m_endInterpTime      = wg->currentTime - 1;
      m_startInterpTime    = wg->currentTime - 501;
      m_prevOffsetPosition = m_targetOffsetPosition;
      m_currentTargetPosition = (m_targetOffsetPosition -
                                 m_pCurrentTarget->getTransform().p);
   }
   m_pLightningImage->clearPoints();
}

DWORD
Lightning::packUpdate(Net::GhostManager* gm,
                      DWORD              mask,
                      BitStream*         stream)
{
   DWORD parentPunt = Parent::packUpdate(gm, mask, stream);
   
   if (mask & Projectile::InitialUpdate) {
      stream->writeFlag(true);
      writeInitialPacket(gm, stream);
   } else {
      stream->writeFlag(false);

      if (mask & TargetMask) {
         stream->writeFlag(true);
         int victimGhostId = getVictimGhostId(gm);
         stream->writeFlag(isAttached() && (victimGhostId != -1));
         if (isAttached() && (victimGhostId != -1)) {
            stream->writeInt(victimGhostId, 10);

            Point3F targetOffset = (m_currentTargetPosition -
                                    m_pCurrentTarget->getTransform().p);
            stream->write(sizeof(Point3F), &targetOffset);
         }
      } else {
         stream->writeFlag(false);
      }
   }
   
   return parentPunt;
}

void
Lightning::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
   Parent::unpackUpdate(gm, stream);
   
   if (stream->readFlag() == true) {
      // Is initial packet...
      //
      readInitialPacket(gm, stream);
   } else {
      bool newTarget = false;
      bool gottaTarget = stream->readFlag();
      if (gottaTarget == true) {
         bool hasTarget = stream->readFlag();
         if (hasTarget) {
            m_startInterpTime    = wg->currentTime;
            m_endInterpTime      = wg->currentTime + 500;
            m_prevOffsetPosition = m_targetOffsetPosition;

            int ghostIdNew = stream->readInt(10);
            stream->read(sizeof(Point3F), &m_targetOffsetPosition);

            SimNetObject* pTarget = gm->resolveGhost(ghostIdNew);
            AssertFatal(pTarget != NULL, "Hm, should never have trans. ghost id");


            ShapeBase* newTargetPtr = dynamic_cast<ShapeBase*>(pTarget);
            AssertFatal(newTargetPtr, "Ghost isn't a shapebase!");

            if (newTargetPtr != m_pCurrentTarget) {
               newTarget = true;
               if (m_pCurrentTarget)
                  clearNotify(m_pCurrentTarget);
               deleteNotify(newTargetPtr);
            } else {
               //
            }

            m_pCurrentTarget = newTargetPtr;
            m_currentTargetPosition = (m_targetOffsetPosition +
                                       m_pCurrentTarget->getTransform().p);
         } else {
            m_pCurrentTarget = NULL;
            m_endInterpTime  = wg->currentTime - 1;
         }
      }

      setTransform(getTransform());
      if (newTarget == true || gottaTarget == false)
         m_pLightningImage->clearPoints();
   }
}

void
Lightning::clientProcess(DWORD /*in_currTime*/)
{
   setTransform(getTransform());

   TMat3F soundPoint;
   soundPoint.identity();
   soundPoint.p = getCurrentTargetPosition();
   updateProjectileSound(soundPoint, Point3F(0, 0, 0));
}

void
Lightning::serverProcess(DWORD in_currTime)
{
   if (m_waitingForShutoff == true) {
      deleteObject();
      return;
   }

   determineTarget();

   if (in_currTime - m_ghostTimer > 300) {
      setMaskBits(TargetMask);
      m_ghostTimer = in_currTime;
   }

   DWORD slice = in_currTime - m_lastUpdated;
   m_lastUpdated = in_currTime;
   if (slice == 0 || m_pCurrentTarget == NULL)
      return;

   // Otherwise, apply damage in proportion to the time on the target, and
   //  the damage rate in the data file....
   Point3F mv = m_currentTargetPosition - getTransform().p;
   mv.normalize();

	if (const char* script = scriptName("damageTarget")) {
      Console->evaluatef("%s(%d, %g, %g, %g, \"%g %g %g\", \"%g %g %g\", \"%g %g %g\", %d);",
                         script, m_pCurrentTarget->getId(),
                         float(slice) * 0.001f,
                         m_pLightningData->damagePerSec,
                         m_pLightningData->energyDrainPerSec,
                         m_currentTargetPosition.x, m_currentTargetPosition.y, m_currentTargetPosition.z,
                         mv.x, mv.y, mv.z, 0.0f, 0.0f, 0.0f, 
                         getDamageId());
	} else {
      AssertFatal(false, "no damageTarget script");
   }
}

inline float
myfmax(float one, float two)
{
   if (one > two)
      return one;
   else
      return two;
}

inline float
myfmin(float one, float two)
{
   if (one > two)
      return two;
   else
      return one;
}

void
Lightning::buildContainerBox(const TMat3F& /*in_rMat*/,
                             Box3F*        out_pBox)
{
   out_pBox->fMin = Point3F(0, 0, 0);
   out_pBox->fMax = Point3F(0, 0, 0);

   TMat3F mat;
   TMat3F final;
   ShapeBase* pShooter = getShooter();
   if (pShooter && pShooter->getMuzzleTransform(0, &mat)) {
      if (dynamic_cast<Player*>(pShooter) != NULL)
         m_mul(mat, pShooter->getTransform(), &final);
      else
         final = mat;
      //
   } else {
      return;
   }
      
   if (manager && canRender()) {
      Point3F one = final.p;
      Point3F two;
      if (getChargeTarget()) {
         two = getChargeTarget()->getBoxCenter();
      } else {
         two = final.p;
      }

      out_pBox->fMin.x = myfmin(one.x, two.x);
      out_pBox->fMin.y = myfmin(one.y, two.y);
      out_pBox->fMin.z = myfmin(one.z, two.z);
      out_pBox->fMax.x = myfmax(one.x, two.x);
      out_pBox->fMax.y = myfmax(one.y, two.y);
      out_pBox->fMax.z = myfmax(one.z, two.z);
   }
}

Point3F
Lightning::getCurrentTargetPosition()
{
   ShapeBase* pTarget = getChargeTarget();
   if (pTarget != NULL) {
      if (wg->currentTime < m_endInterpTime) {
         float factor = float(wg->currentTime - m_startInterpTime) / float(m_endInterpTime - m_startInterpTime);
         Point3F interp = m_targetOffsetPosition * factor + m_prevOffsetPosition * (1.0f - factor);
         
         m_currentTargetPosition = pTarget->getTransform().p + interp;
      } else {
         m_currentTargetPosition = pTarget->getTransform().p + m_targetOffsetPosition;
      }
   }

   return m_currentTargetPosition;
}

bool
Lightning::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if (canRender() == true) {
      query->count    = 1;
      query->image[0] = m_pLightningImage;

      m_pLightningImage->itype     = SimRenderImage::Translucent;
      m_pLightningImage->sortValue = 0.998f;

      TMat3F final;
      TMat3F muzzle;

      ShapeBase* pShooter = getShooter();
      if ((pShooter->getType() & SimPlayerObjectType) != 0)
         getShooter()->getAimedMuzzleTransform(0, &final);
      else
         getShooter()->getMuzzleTransform(0, &final);

      m_pLightningImage->m_attached   = isAttached();
      m_pLightningImage->m_startPoint = final.p;
      if (isAttached()) {
         m_pLightningImage->m_endPoint = getCurrentTargetPosition();

         Point3F finalY;
         final.getRow(1, &finalY);
         Point3F newY = m_pLightningImage->m_endPoint - m_pLightningImage->m_startPoint;
         newY.normalize();
         Point3F rotVec;
         m_cross(finalY, newY, &rotVec);
         rotVec.normalize();

         float dotProd = m_dot(newY, finalY);
         if (dotProd < -1.0f)     dotProd = -1.0f;
         else if (dotProd > 1.0f) dotProd = 1.0f;
         float angle = acos(dotProd);
         
         AngAxisF angleAxis(rotVec, angle);
         RMat3F rotationMat(angleAxis);

         TMat3F reallyFinal;
         m_mul(rotationMat, final, &reallyFinal);
                      
         m_pLightningImage->m_baseTransform = reallyFinal;

         m_pLightningImage->updateSegments();
      } else {
         m_pLightningImage->clearPoints();
      }
   } else {
      query->count    = 0;
      query->image[0] = NULL;
   }

   return true;
}

bool
Lightning::onSimLightQuery(SimLightQuery* query)
{
   if (isAttached() && canRender() == true) {
      query->count    = 1;
      query->light[0] = &m_light;
      query->ambientIntensity.set();
      m_light.setPosition(getCurrentTargetPosition());
   } else {
      query->count    = 0;
   }

   return true;
}

int
Lightning::getDatGroup()
{
   return DataBlockManager::LightningDataType;
}

void Lightning::readExplosion(BitStream*)  { }
void Lightning::writeExplosion(BitStream*) { }

int
Lightning::getVictimGhostId(Net::GhostManager* gm)
{
   if (m_pCurrentTarget) {
      return gm->getGhostIndex(m_pCurrentTarget);
   } else
      return -1;
}

ShapeBase*
Lightning::getShooter()
{
   return m_pShooter;
}

ShapeBase*
Lightning::getChargeTarget()
{
   return m_pCurrentTarget;
}

bool
Lightning::canRender()
{
   if (getShooter() && (getChargeTarget() || isAttached() == false))
      return true;
   else
      return false;
}

void
Lightning::onDeleteNotify(SimObject* io_pDelete)
{
   if (io_pDelete == (SimObject*)m_pCurrentTarget) {
      m_pCurrentTarget = NULL;
   }

   Parent::onDeleteNotify(io_pDelete);
}

//--------------------------------------

void
Lightning::LightningRenderImage::render(TS::RenderContext& io_rRC)
{
   if (m_attached == true) {
      AssertFatal(isCreated(), "no points?");
      TS::PointArray* pArray = io_rRC.getPointArray();
      GFXSurface* pSurface   = io_rRC.getSurface();

      pSurface->setFillMode(GFX_FILL_TEXTURE);
      pSurface->setShadeSource(GFX_SHADE_NONE);
      pSurface->setHazeSource(GFX_HAZE_NONE);
      if ((pLightning->m_resBitmap->attribute & BMA_ADDITIVE) != 0)
         pSurface->setAlphaSource(GFX_ALPHA_ADD);
      else if ((pLightning->m_resBitmap->attribute & BMA_SUBTRACTIVE) != 0)
         pSurface->setAlphaSource(GFX_ALPHA_SUB);
      else
         pSurface->setAlphaSource(GFX_ALPHA_TEXTURE);
      pSurface->setZTest(GFX_ZTEST);
      pSurface->setTexturePerspective(false);
      pSurface->setTransparency(false);
      pSurface->setTextureMap(pLightning->m_resBitmap);
      pSurface->setFillColor(255);

      Point2F texPoints[4];
      texPoints[0].set(0, 0);
      texPoints[1].set(1, 0);
      texPoints[2].set(1, 1);
      texPoints[3].set(0, 1);
      pArray->useTextures(texPoints);
      pArray->useTextures(true);
      pArray->useIntensities(false);

      Point3F camVect;
      io_rRC.getCamera()->getTCW().getRow(1, &camVect);

      for (int i = 0; i < pLightning->m_pLightningData->numSegments; i++) {
         pArray->reset();
         
         int numPoints = (1 << pLightning->m_pLightningData->segmentDivisions) + 1;
         int j;

         Point3F normal = (m_endPoint - m_startPoint);
         normal.normalize();

         Point3F cross;
         m_cross(normal, camVect, &cross);
         cross.normalize();
         float length = (m_endPoint - m_startPoint).len();

         Point3F tempArray[2];
         for (j = 0; j < numPoints; j++) {
            Point3F linePoint = m_lines[i].getPoint(j, wg->currentTime, m_baseTransform, length);
            tempArray[0] = linePoint + (cross * (pLightning->m_pLightningData->beamWidth * 0.5f));
            tempArray[1] = linePoint - (cross * (pLightning->m_pLightningData->beamWidth * 0.5f));
            pArray->addPoints(2, tempArray);
         }

         TS::VertexIndexPair polyPairs[3];
         for (j = 0; j < (1 << pLightning->m_pLightningData->segmentDivisions); j++) {
            // We are drawing from j * 2 to (j + 1) * 2...
            //
            int baseIndex = j * 2;

            polyPairs[0].fVertexIndex  = baseIndex + 0;
            polyPairs[0].fTextureIndex = 3;
            polyPairs[1].fVertexIndex  = baseIndex + 3;
            polyPairs[1].fTextureIndex = 1;
            polyPairs[2].fVertexIndex  = baseIndex + 1;
            polyPairs[2].fTextureIndex = 2;
            pArray->drawPoly(3, polyPairs, 0);

            polyPairs[0].fVertexIndex  = baseIndex + 0;
            polyPairs[0].fTextureIndex = 3;
            polyPairs[1].fVertexIndex  = baseIndex + 2;
            polyPairs[1].fTextureIndex = 0;
            polyPairs[2].fVertexIndex  = baseIndex + 3;
            polyPairs[2].fTextureIndex = 1;
            pArray->drawPoly(3, polyPairs, 0);

            polyPairs[0].fVertexIndex  = baseIndex + 0;
            polyPairs[0].fTextureIndex = 3;
            polyPairs[1].fVertexIndex  = baseIndex + 1;
            polyPairs[1].fTextureIndex = 2;
            polyPairs[2].fVertexIndex  = baseIndex + 3;
            polyPairs[2].fTextureIndex = 1;
            pArray->drawPoly(3, polyPairs, 0);

            polyPairs[0].fVertexIndex  = baseIndex + 0;
            polyPairs[0].fTextureIndex = 3;
            polyPairs[1].fVertexIndex  = baseIndex + 3;
            polyPairs[1].fTextureIndex = 1;
            polyPairs[2].fVertexIndex  = baseIndex + 2;
            polyPairs[2].fTextureIndex = 0;
            pArray->drawPoly(3, polyPairs, 0);
         }
      }
      pSurface->setZTest(GFX_ZTEST_AND_WRITE);
   } else {
   }
}

void
Lightning::LightningRenderImage::updateSegments()
{
   for (int i = 0; i < pLightning->m_pLightningData->numSegments; i++) {
      if (m_lines[i].endTime < wg->currentTime ||
          m_lines[i].m_points == NULL)
         createSegments(i);
      if (m_lines[i].m_points)
         m_lines[i].m_points[0] = m_startPoint;
   }
}

void
Lightning::resetEndPoint()
{
//   AssertFatal(isGhost(), "only on client");
//
//   ShapeBase* pShooter = getShooter();
//   if (pShooter == NULL)
//      return;
//
//   TMat3F mat, final;
//   pShooter->getAimedMuzzleTransform(0, &final);
//
//   Point3F direction;
//   final.getRow(1, &direction);
//
//   Point3F beginSegment = final.p;
//   Point3F endSegment   = final.p + (direction * 100.0f);   // No need to confine the beam
//                                                            //  on the client...
//   SimCollisionInfo  info;
//   SimContainerQuery query;
//   query.id       = m_pShooter != NULL ? m_pShooter->getId() : 0;
//   query.type     = -1;
//   query.mask     = SimPlayerObjectType |
//                    SimShapeObjectType  | SimVehicleObjectType |
//                    VehicleObjectType   | MineObjectType       |
//                    StaticObjectType    | MoveableObjectType;
//   query.detail   = SimContainerQuery::DefaultDetail;
//   query.box.fMin = beginSegment;
//   query.box.fMax = endSegment;
//
//   SimContainer* pRoot = (SimContainer*)manager->findObject(SimRootContainerId);
//   AssertFatal(pRoot != NULL, "No root container?");
//
//   if (pRoot->findLOS(query, &info, SimCollisionImageQuery::High) == true) {
//      // If it ain't the same, use the old end point...
//      if ((SimContainer*)m_pCurrentTarget != info.object)
//         return;
//
//      m_mul(info.surfaces[0].position, info.surfaces.tWorld, &m_currentTargetPosition);
//      m_pLightningImage->m_endPoint = m_currentTargetPosition;
//   }
}

void
Lightning::LightningRenderImage::createSegments(const int in_line)
{
   AssertFatal((pLightning->m_pLightningData->segmentDivisions > 0 &&
                pLightning->m_pLightningData->segmentDivisions <= 6),
                "<= 1 || > 64 segments not allowed");

   pLightning->resetEndPoint();

   int numPoints = (1 << pLightning->m_pLightningData->segmentDivisions) + 1;

   Point3F* createArray;
   if (m_lines[in_line].m_points == NULL) {
      // First time through?
      m_lines[in_line].m_points = new Point3F[numPoints];
      createArray = m_lines[in_line].m_points;
   } else {
      if (m_lines[in_line].m_pointsAnim == NULL)
         m_lines[in_line].m_pointsAnim = new Point3F[numPoints];
      else
         memcpy(m_lines[in_line].m_points, m_lines[in_line].m_pointsAnim, sizeof(Point3F) * numPoints);
      createArray = m_lines[in_line].m_pointsAnim;
      
   }

   createArray[0]             = m_startPoint;
   createArray[numPoints - 1] = m_endPoint;

   // Recurse away...
   subdivideLine(createArray, 0, numPoints - 1);

   float length = (m_endPoint - m_startPoint).len();
   Point3F xNorm, yNorm, zNorm;
   m_baseTransform.getRow(0, &xNorm);
   m_baseTransform.getRow(1, &yNorm);
   m_baseTransform.getRow(2, &zNorm);
   int i;
   for (i = 1; i < numPoints; i++) {
      float newX, newY, newZ;
      newX = m_dot(createArray[i] - m_startPoint, xNorm);
      newY = m_dot(createArray[i] - m_startPoint, yNorm) / length;
      newZ = m_dot(createArray[i] - m_startPoint, zNorm);
      createArray[i].x = newX;
      createArray[i].y = newY;
      createArray[i].z = newZ;
   }

   // Check to make sure we have all the points we need.
   if (m_lines[in_line].m_pointsAnim == NULL)
      createSegments(in_line);

   m_lines[in_line].createTime = wg->currentTime;
   m_lines[in_line].endTime    = wg->currentTime + pLightning->m_pLightningData->updateTime +
                                 (sg_randGen.getFloat() *
                                  pLightning->m_pLightningData->updateTime *
                                  pLightning->m_pLightningData->skipPercent);

   Point3F* points     = m_lines[in_line].m_points;
   Point3F* pointsAnim = m_lines[in_line].m_pointsAnim;
   Point3F normal = m_endPoint - m_startPoint;
   normal.normalize();
   for (i = 1; i < numPoints - 1; i++) {
      float rand = 0.70 + .20 * sg_randGen.getFloat();
      points[i].x = rand * pointsAnim[i].x;
      points[i].z = rand * pointsAnim[i].z;
   }
}

void
Lightning::LightningRenderImage::subdivideLine(Point3F*  in_pPoints,
                                               const int in_left,
                                               const int in_right)
{
   if (in_left == (in_right - 1))
      return;

   int half = (in_left + in_right) / 2;
   
   // First, we need to create a point at a randomly displace location in
   //  the line between the two points.  We want this to be biased towards the center,
   //  but not always exactly on...
   //
   float midPoint = 0.5;
   in_pPoints[half] = in_pPoints[in_left] + (in_pPoints[in_right] -
                                             in_pPoints[in_left]) * midPoint;

   // Ok, now m_points[half] is at some random point along the line.  We need to generate
   //  a (psuedo) random coordinate system, and start displacing along the perpendiculars.
   //
   float length;
   Point3F xNormal = (in_pPoints[in_right] - in_pPoints[in_left]);
   length = xNormal.len();
   xNormal /= length;

   float minLength = pLightning->m_pLightningData->boltLength;
   minLength /= float(1 << pLightning->m_pLightningData->segmentDivisions);
   minLength *= 1.5f;
   if (length > minLength) {
      Point3F junk(1.0f, 0.0f, 0.0f);
      if (fabs(xNormal.x) >0.8f)
         junk = Point3F(0.0f, 1.0f, 0.0f);

      Point3F yNormal;
      m_cross(junk, xNormal, &yNormal);
      yNormal.normalize();

      // Ok, now xNormal and yNormal form an (incomplete) set of orthonormal basis vectors
      //  What we need to do is randomly displace along y and z.  We want to displace
      //  in proportion to the length of the distance between the seed points.
      //
      float displacementLen = length * pLightning->m_pLightningData->displaceBias *
                              sg_randGen.getFloat();

      AngAxisF axisAng(xNormal, sg_randGen.getFloat() * 2.0f * M_PI);
      RMat3F rotMat(axisAng);
      Point3F resultVec;
      m_mul(yNormal, rotMat, &resultVec);
      resultVec.normalize();

      Point3F resultDisp(resultVec.x, resultVec.y, resultVec.z);
      resultDisp  *= displacementLen;
      in_pPoints[half] += resultDisp;
   }

   // recurse...
   subdivideLine(in_pPoints, in_left, half);
   subdivideLine(in_pPoints, half, in_right);
}

void
Lightning::LightningRenderImage::clearPoints()
{
   for (int i = 0; i < 8; i++) {
      delete [] m_lines[i].m_points;
      m_lines[i].m_points = NULL;
      delete [] m_lines[i].m_pointsAnim;
      m_lines[i].m_pointsAnim = NULL;
      m_lines[i].createTime = 0;
   }
}

Lightning::LightningRenderImage::~LightningRenderImage()
{

}

