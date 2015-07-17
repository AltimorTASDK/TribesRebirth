//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <g_surfac.h>
#include "player.h"
#include "fearDcl.h"
#include "fearGlobals.h"
#include "projRepair.h"
#include "dataBlockManager.h"

IMPLEMENT_PERSISTENT_TAG(RepairEffect, RepairEffectPersTag);

static Random sg_randGen;

RepairEffect::RepairEffectData::RepairEffectData()
{
   numSegments = 2;
}

RepairEffect::RepairEffect(int in_datFileId)
 : Parent(in_datFileId),
   m_targetEvaluated(false)
{
   delete m_pLightningImage;
   m_pLightningImage = new RepairRenderImage;

}

RepairEffect::RepairEffect()
 : Parent(-1),
   m_targetEvaluated(false)
{
   delete m_pLightningImage;
   m_pLightningImage = new RepairRenderImage;

}

int
RepairEffect::getDatGroup()
{
   return DataBlockManager::RepairEffectDataType;
}

void
RepairEffect::serverProcess(DWORD in_currTime)
{
   if (m_waitingForShutoff == true) {
      deleteObject();
      return;
   }

   determineTarget();
   m_lastUpdated = in_currTime;

   if (in_currTime - m_ghostTimer > 300) {
      setMaskBits(TargetMask);
      m_ghostTimer = in_currTime;
   }
}

void
RepairEffect::determineTarget()
{
   ShapeBase* pTarget = m_pCurrentTarget;

   //--------------------------------------
   if (m_lastTarget + csm_targetUpdateFreq > wg->currentTime)
      return;
   m_lastTarget = wg->currentTime;

   if (m_targetEvaluated == true && pTarget == NULL) {
      // self healing, do not switch...
	   if (const char* script = scriptName("checkDone")) {
	   	Console->evaluatef("%s(%d, %d);", script, scriptThis(), m_pShooter->getId());
	   } else {
         AssertFatal(false, "no checkdone script");
      }
      return;
   }


   Player* pShooter = dynamic_cast<Player*>(getShooter());
   AssertFatal(pShooter != NULL, "Cannot determine target");
   UInt32 mask = SimPlayerObjectType   |
                 SimShapeObjectType    | SimVehicleObjectType |
                 VehicleObjectType     | MineObjectType       |
                 StaticObjectType      | MoveableObjectType   |
                 SimInteriorObjectType | SimTerrainObjectType;

   Point3F pos, norm;
   SimObject* pHit = pShooter->findLOSObject(pShooter->getEyeTransform(),
                                             m_pLightningData->boltLength,
                                             mask,
                                             &pos, &norm);
   if (pHit && (pHit->getType() & (SimInteriorObjectType | SimTerrainObjectType)))
      pHit = NULL;

   if (pHit != NULL) {
      if ((SimObject*)m_pCurrentTarget != pHit) {
         if (m_pCurrentTarget)
            clearNotify(m_pCurrentTarget);
      }

      m_currentTargetPosition = pos;
      if(m_currentTargetPosition == getTransform().p)
      {
         Point3F directionVector;
         pShooter->getTransform().getRow(1, &directionVector);
         m_currentTargetPosition += directionVector * 0.1;
      }

      m_pCurrentTarget = dynamic_cast<ShapeBase*>(pHit);
      AssertFatal(m_pCurrentTarget != NULL, "Error, struck object not of type shapebase");
      deleteNotify(m_pCurrentTarget);
   } else {
      if (m_pCurrentTarget != NULL) {
         clearNotify(m_pCurrentTarget);
      }
         
      m_pCurrentTarget = NULL;
   }

   //--------------------------------------
   if (isGhost() == false) {
      AssertFatal(m_pShooter != NULL, "This should probably not happen, ever.  Cannot recover from this");
      if (m_targetEvaluated == true) {
         if (m_pCurrentTarget != pTarget && pTarget != NULL) {
            pShooter->setImageTriggerUp(0);
         } else {
	         if (const char* script = scriptName("checkDone")) {
	   	   	Console->evaluatef("%s(%d, %d);", script, scriptThis(), m_pShooter->getId());
	         } else {
               AssertFatal(false, "no checkdone script");
            }
         }
      } else {
         Int32 id;
         if (m_pCurrentTarget != NULL)
            id = m_pCurrentTarget->getId();
         else
            id = m_pShooter->getId();

	      if (const char* script = scriptName("onAcquire")) {
	   		Console->evaluatef("%s(%d, %d, %d);", script, scriptThis(), m_pShooter->getId(), id);
	      } else {
            AssertFatal(false, "no acquire script");
         }

         m_targetEvaluated = true;
      }
   }
}

void
RepairEffect::resetEndPoint()
{
   AssertFatal(isGhost(), "only on client");

   ShapeBase* pShooter = getShooter();
   if (pShooter == NULL)
      return;

   UInt32 mask = SimPlayerObjectType |
                 SimShapeObjectType  | SimVehicleObjectType |
                 VehicleObjectType   | MineObjectType       |
                 StaticObjectType    | MoveableObjectType;

   Point3F pos, norm;
   SimObject* pHit = pShooter->findLOSObject(pShooter->getEyeTransform(),
                                             100.0f,
                                             mask,
                                             &pos, &norm);
   if ((ShapeBase*)pHit == m_pCurrentTarget) {
      m_currentTargetPosition = pos;
   }
}

void
RepairEffect::shutOffProjectile()
{
   AssertFatal(isGhost() == false, "Error, cannot shut off the client side...");

   if (m_pCurrentTarget != NULL) {
      clearNotify(m_pCurrentTarget);
   }
   m_pCurrentTarget    = NULL;
   m_waitingForShutoff = true;

   if (m_targetEvaluated == true) {
      AssertFatal(m_pShooter != NULL, "Cannot use repair beam wo/ shooter on server...");
      if (const char* script = scriptName("onRelease")) {
         Console->evaluatef("%s(%d, %d);", script, scriptThis(), m_pShooter->getId());
      } else {
         AssertFatal(false, "no release script");
      }
   }
}

void
RepairEffect::RepairRenderImage::updateSegments()
{
   for (int i = 0; i < 4; i++) {
      if (m_lines[i].endTime < wg->currentTime ||
          m_lines[i].m_points == NULL)
         createSinSegments(i);
      if (m_lines[i].m_points)
         m_lines[i].m_points[0] = m_startPoint;
   }
}

RepairEffect::RepairRenderImage::~RepairRenderImage()
{
   //
}

void
RepairEffect::RepairRenderImage::createSinSegments(const int in_line)
{
   AssertFatal((pLightning->m_pLightningData->segmentDivisions > 0 &&
                pLightning->m_pLightningData->segmentDivisions <= 6),
                "<= 1 || > 64 segments not allowed");
   pLightning->resetEndPoint();

   int numPoints = (1 << pLightning->m_pLightningData->segmentDivisions) + 1;

   Point3F* createArray;
   bool     createPhase;
   if (m_lines[in_line].m_points == NULL) {
      // First time through?
      m_lines[in_line].m_points = new Point3F[numPoints];
      createArray = m_lines[in_line].m_points;
      createPhase = false;
      m_lines[in_line].createPhase = true;
   } else {
      if (m_lines[in_line].m_pointsAnim == NULL)
         m_lines[in_line].m_pointsAnim = new Point3F[numPoints];
      else
         memcpy(m_lines[in_line].m_points, m_lines[in_line].m_pointsAnim, sizeof(Point3F) * numPoints);
      createArray = m_lines[in_line].m_pointsAnim;
      createPhase = m_lines[in_line].createPhase;
      m_lines[in_line].createPhase = !m_lines[in_line].createPhase;
   }

   createArray[0]             = m_startPoint;
   createArray[numPoints - 1] = Point3F(0, 1, 0);

   int displaceIndex   = in_line <= 1 ? 0 : 2;
   float phaseConstant = float(in_line + 2) * M_PI;
   float phaseShift    = createPhase ? M_PI : 0.0f;
   for (int i = 1; i < numPoints - 1; i++) {
      float factor = i / float(numPoints - 1);
      createArray[i].y             = factor;
      if (displaceIndex == 0) {
         createArray[i].z = 0.0f;
         createArray[i].x = sin(factor * phaseConstant + phaseShift) * pLightning->m_pLightningData->displaceBias;
      } else {
         createArray[i].x = 0.0f;
         createArray[i].z = sin(factor * phaseConstant + phaseShift) * pLightning->m_pLightningData->displaceBias;
      }
   }

   // Check to make sure we have all the points we need.
   if (m_lines[in_line].m_pointsAnim == NULL)
      createSinSegments(in_line);

   m_lines[in_line].createTime = wg->currentTime;
   m_lines[in_line].endTime    = wg->currentTime + pLightning->m_pLightningData->updateTime +
                                 (sg_randGen.getFloat() *
                                  pLightning->m_pLightningData->updateTime *
                                  pLightning->m_pLightningData->skipPercent);
}

void
RepairEffect::RepairRenderImage::render(TS::RenderContext& io_rRC)
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

      for (int i = 0; i < 2; i++) {
         pArray->reset();
         
         int numPoints = (1 << pLightning->m_pLightningData->segmentDivisions) + 1;
         int j;
         Point3F tempArray[2];
         float length = (m_endPoint - m_startPoint).len();

         Point3F normal = m_endPoint - m_startPoint;
         normal.normalize();
         Point3F cross;
         m_cross(camVect, normal, &cross);
         cross.normalize();


         for (j = 0; j < numPoints; j++) {
            Point3F linePoint = getPoint(j, i, wg->currentTime, m_baseTransform, length);
            
            tempArray[0] = linePoint + (cross * (pLightning->m_pLightningData->beamWidth * 0.5f));
            tempArray[1] = linePoint - (cross * (pLightning->m_pLightningData->beamWidth * 0.5f));
            pArray->addPoints(2, tempArray);
         }

         TS::VertexIndexPair polyPairs[3];
         for (j = 0; j < numPoints - 1; j++) {
            // We are drawing from j * 2 to (j + 1) * 2...
            //
            int baseIndex = j * 2;

            // Make this into a table... DMM
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
   }
}

bool
RepairEffect::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if (canRender() == true) {
      resetEndPoint();

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
         m_pLightningImage->m_endPoint = m_currentTargetPosition;

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
RepairEffect::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);

     default:
      return Parent::processQuery(query);
   }
}