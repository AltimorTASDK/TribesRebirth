//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

#include <m_random.h>
#include <simTagDictionary.h>
#include <simLightGrp.h>
#include <stringTable.h>
#include <simResource.h>
#include <tsFx.h>

#include "explosion.h"
#include "dataBlockManager.h"
#include "player.h"
#include "fearGlobals.h"
#include "fearDcl.h"


IMPLEMENT_PERSISTENT_TAG(Explosion, ExplosionPersTag);


//----------------------------------------------------------------------------

// All damageable objects must be derived from ShapeBase.
static int DamageMask =
         SimPlayerObjectType | 
         StaticObjectType |
         VehicleObjectType |
         MoveableObjectType |
			MineObjectType;

// a random number generator for explosion code
Random g_expRandom;

//bool Explosion::SoftwareTranslucency = false;
bool Explosion::sm_useDynamicLighting = true;


Explosion::ExplosionData::ExplosionData()
{
   pShapeName = NULL;
   soundId    = -1;

   faceCamera = false;
   randomSpin = false;

   hasLight      = false;
   shiftPosition = false;

   timeScale     = 1.0f;
}

void
Explosion::ExplosionData::pack(BitStream* bs)
{
   Parent::pack(bs);

   bs->writeString(pShapeName);
   bs->write(soundId);
   bs->write(timeScale);

   bs->writeFlag(faceCamera);
   bs->writeFlag(randomSpin);
   bs->writeFlag(hasLight);

   if (hasLight) {
      bs->write(lightRange);
      bs->write(timeZero);
      bs->write(timeOne);
      bs->write(sizeof(ColorF) * 3, (void*)colors);
      bs->write(sizeof(float)  * 3, (void*)radFactors);
   }

   bs->writeFlag(shiftPosition);
}

void
Explosion::ExplosionData::unpack(BitStream* bs)
{
   Parent::unpack(bs);

   char buf[256];
   bs->readString(buf);
   pShapeName = stringTable.insert(buf, true);

   bs->read(&soundId);
   bs->read(&timeScale);

   faceCamera = bs->readFlag();
   randomSpin = bs->readFlag();
   hasLight   = bs->readFlag();

   if (hasLight) {
      bs->read(&lightRange);
      bs->read(&timeZero);
      bs->read(&timeOne);
      bs->read(sizeof(ColorF) * 3, (void*)colors);
      bs->read(sizeof(float)  * 3, (void*)radFactors);
   }

   shiftPosition = bs->readFlag();
}

bool Explosion::ExplosionData::preload(ResourceManager *rm, bool server, char errorBuffer[256])
{
   server;
   if (!server && pShapeName[0] != '\0') {
      m_expRes = rm->load(pShapeName);
      if(!bool(m_expRes))
      {
         sprintf(errorBuffer, "Unable to load ExplosionData shape: %s", pShapeName);
         return false;
      }
   }
   return true;
}

//--------------------------------------------------------------------------- 
Explosion::Explosion(const Int32 in_datFileId)
 : m_explosionData(NULL),
   m_axis(0, 0, 0),
   m_position(0, 0, 0)
{
   datFileName = NULL;
   datFileId   = in_datFileId;

   m_renderImage.transform.identity();
}

void
Explosion::setPosition(const Point3F& in_rPosition)
{ 
   m_position = in_rPosition;
   m_renderImage.transform.p = in_rPosition;

   TMat3F newPosition = getTransform();
   newPosition.p = m_position;
   SimMovement::setPosition(newPosition, true);
}

void
Explosion::setAxis(const Point3F& in_rAxis)
{ 
   // if shorter than this, we assume 0 vector
   if (m_dot(in_rAxis, in_rAxis) > 0.1f) {
      m_axis = in_rAxis;
      m_axis.normalize();
   }
}
  
bool
Explosion::initResources(GameBase::GameBaseData* io_pDat)
{
   if(!Parent::initResources(io_pDat))
      return false;

   m_explosionData = dynamic_cast<ExplosionData*>(io_pDat);

   return true;
}

int
Explosion::getDatGroup()
{
   return DataBlockManager::ExplosionDataType;
}

void
Explosion::loadResources()
{
   AssertFatal(manager != NULL, "hrmph");

   AssertFatal(m_explosionData != NULL, "Error, no exp data");

   // Read shape, initialize thread
   ResourceManager* rm = SimResource::get(manager);
   m_hShape = rm->load(m_explosionData->pShapeName, true);
   AssertFatal((bool)m_hShape, avar("Unable to load explosion shape: %s", m_explosionData->pShapeName));
   
   m_renderImage.shape = new TS::ShapeInstance(m_hShape, *rm);
   AssertFatal(m_renderImage.shape != NULL,
               "Could not create explosion shape instance");

   m_pVisThread = m_renderImage.shape->CreateThread();
   if (m_pVisThread->GetSequenceIndex("visibility") != -1) {
      m_pVisThread->SetSequence("visibility", 0.0f, false);
      m_pVisThread->SetPosition((2.0f / 3.0f) + g_expRandom.getFloat() * (m_pVisThread->getSequence().fDuration / 3.0f));
      m_pVisThread->setTimeScale(0.0f);
   } else {
      m_renderImage.shape->DestroyThread(m_pVisThread);
      m_pVisThread = NULL;
   }

   m_renderImage.thread = m_renderImage.shape->CreateThread();
   m_renderImage.thread->setTimeScale(1.0f / m_explosionData->timeScale);

   m_renderImage.translucentShape  = m_renderImage.shape;
   m_renderImage.translucentThread = m_renderImage.thread;
   m_renderImage.setTranslucent(true);

   // set-up shape space bounding box -- real size
   m_bbox.fMin  = m_renderImage.shape->getShape().fCenter;
   m_bbox.fMin += m_renderImage.shape->fRootDeltaTransform.p; // could be trouble if root has rot.
   m_bbox.fMax  = m_bbox.fMin;
   m_bbox.fMin -= m_renderImage.shape->getShape().fRadius;
   m_bbox.fMax += m_renderImage.shape->getShape().fRadius;
   setPosition(Point3F(m_position));
}

//------------------------------------------------------------------------------
bool
Explosion::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   loadResources();

   if (m_explosionData->randomSpin == true)
      m_renderImage.setAxisSpin(g_expRandom.getFloat(float(M_2PI)));
   if (m_explosionData->faceCamera == false)
      m_renderImage.faceDirection(m_axis);

   m_renderImage.transform.p = m_position;

   m_spawnTime   = wg->currentTime;
   m_lastUpdated = m_spawnTime;

   m_expireTime = m_spawnTime +
                  UInt32(m_explosionData->timeScale *
                         m_renderImage.thread->getSequence().fDuration *
                         1000.0f);
   m_timeZeroMS = UInt32(m_explosionData->timeScale * m_renderImage.thread->getSequence().fDuration * 1000.0f * m_explosionData->timeZero);
   m_timeOneMS  = UInt32(m_explosionData->timeScale * m_renderImage.thread->getSequence().fDuration * 1000.0f * m_explosionData->timeOne);

   if (m_explosionData->hasLight == true && sm_useDynamicLighting == true) {
      m_light.setType(TS::Light::LightPoint);
      addToSet(SimLightSetId);
   }

   AssertFatal(getContainer() == NULL, "Cannot be in a container at this point...");
   SimContainer* pRoot = findObject(manager, SimRootContainerId, pRoot);
   AssertFatal(pRoot != NULL, "No root container...");
   pRoot->addObject(this);

   addToSet(PlayerSetId);

   // And play the explosion sound...
   //
   TSFX::PlayAt(m_explosionData->soundId, TMat3F(EulerF(0, 0, 0), m_position), Point3F(0, 0, 0));
   return true;
}

void
Explosion::onRemove()
{
   // remove from whatever container we're in
   if (getContainer())
      getContainer()->removeObject(this);

   if (m_renderImage.shape)
      delete m_renderImage.shape;

   Parent::onRemove();
}

bool
Explosion::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if (wg->currentTime > m_expireTime) {
      query->count    = 0;
      query->image[0] = NULL;
      return true;
   }

   if (m_renderImage.thread != NULL) {
      float deltaT = float(wg->currentTime - m_lastUpdated) / 1000.0f;
      m_renderImage.thread->AdvanceTime(deltaT);
      m_lastUpdated = wg->currentTime;
   }

   TSRenderContext& rc = *(query->renderContext);

   static float s_maxShiftRadius  = 3.0f;
   static float s_maxShiftCamAxis = 0.3f;
   if (m_explosionData->shiftPosition == true) {
      m_renderImage.findCameraAxis(rc);
      float dot = m_dot(m_renderImage.getCameraAxis(), m_axis);
      if (dot < 0) {
      	if (dot > -1.0f / s_maxShiftRadius)
      		dot = -s_maxShiftRadius;
      	else
      		dot = 1.0f / dot;

      	// don't let shape move more than
      	// some % of distance from camera 
      	float shiftDist = m_renderImage.shape->getShape().fRadius * dot;
      	if (-shiftDist > m_renderImage.getCameraDistance() * s_maxShiftCamAxis) 
      		shiftDist = -m_renderImage.getCameraDistance() * s_maxShiftCamAxis;
                                                       
      	Point3F getAway = m_renderImage.getCameraAxis();
      	getAway    *= shiftDist;
      	m_renderImage.transform.p = m_position + getAway;
      }
   }

   if (m_explosionData->faceCamera == true)
      m_renderImage.faceCamera(rc, true); // face camera -- don't need to get axis again

   // set image
   m_renderImage.setSortValue(rc.getCamera());
   query->count    = 1;
   query->image[0] = &m_renderImage;

   return true;
}

//--------------------------------------------------------------------------- 
bool
Explosion::processQuery(SimQuery *query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);
      onQuery(SimLightQuery);

     default:
      return Parent::processQuery(query);
   }
}

void
Explosion::clientProcess(DWORD in_currTime)
{
   if (in_currTime > m_expireTime)
      deleteObject();
}

//--------------------------------------------------------------------------- 

void
Explosion::buildContainerBox(const TMat3F& in_rMat,
                             Box3F*        out_pBox)
{
//   *out_pBox = m_bbox;
   out_pBox->fMin = in_rMat.p - Point3F(0.1, 0.1, 0.1);
   out_pBox->fMax = in_rMat.p + Point3F(0.1, 0.1, 0.1);
}

bool
Explosion::onSimLightQuery(SimLightQuery* query )
{
   query->ambientIntensity.set();
   
   if (m_explosionData->hasLight == false || sm_useDynamicLighting == false) {
      query->count = 0;
      query->light[0] = NULL;
   } else {
      Int32 currTime = Int32(wg->currentTime - m_spawnTime);
      if (currTime < 0 || wg->currentTime >= m_expireTime) {
         query->count    = 0;
         query->light[0] = NULL;
         return true;
      }

      query->count    = 1;
      query->light[0] = &m_light;
      
      m_light.setPosition(m_position);
      
      // Determine the multiplying factor for intensity and range...
      //
      ColorF color;
      float factor;

      if (currTime < m_timeZeroMS) {
         float factor2 = float(m_timeZeroMS - currTime) /
                         float(m_timeZeroMS);

         factor = m_explosionData->radFactors[1] +
                  (factor2 * (m_explosionData->radFactors[0] -
                              m_explosionData->radFactors[1]));

         color.interpolate(m_explosionData->colors[1],
                           m_explosionData->colors[0], factor2);
      } else if (currTime <= m_timeOneMS &&
                 m_timeOneMS != m_timeZeroMS) {

         float factor2 = float(m_timeOneMS - currTime) /
                         float(m_timeOneMS - m_timeZeroMS);

         factor = m_explosionData->radFactors[1] +
                  (factor2 * (m_explosionData->radFactors[2] -
                              m_explosionData->radFactors[1]));

         color.interpolate(m_explosionData->colors[2],
                           m_explosionData->colors[1], factor2);
      } else {
         AssertFatal(m_timeOneMS != Int32(m_expireTime - m_spawnTime),
                     "Should be in the case before this.");

         factor = float((m_expireTime - m_spawnTime) - currTime) /
                  float((m_expireTime - m_spawnTime) - m_timeOneMS);
         color = m_explosionData->colors[2] * factor;
         factor *= m_explosionData->radFactors[2];
      }

      m_light.setIntensity(color);
      m_light.setRange(m_explosionData->lightRange * factor);
   }

   return true;
}


//----------------------------------------------------------------------------

void Explosion::applyRadiusDamage(SimManager* manager,
	int type,const Point3F& pos,const Point3F& vec,
	float radius,float damageValue,
	float force, int srcObjectId)
{
   SimContainer* root = findObject(manager, SimRootContainerId,root);
   SimContainerQuery query;
	query.id     = -1;
	query.type   = -1;
	query.mask   = DamageMask;
	query.detail = SimContainerQuery::DefaultDetail;
	query.box.fMin = pos - Point3F(radius, radius, radius);
	query.box.fMax = pos + Point3F(radius, radius, radius);

   SimContainerList scList;
   root->findIntersections(query, &scList);
   
   for (SimContainerList::iterator itr = scList.begin();
         itr != scList.end(); itr++) {
		ShapeBase* pShape = static_cast<ShapeBase*>(*itr);

//      if (pShape->getId() == exemptId)
//         continue;
//
		// Get the midpoint of the containable
		// And the distance to the collision...
		Point3F mVec = pShape->getBoxCenter();
		mVec -= pos;
		float dist = mVec.len();

      float min = pShape->getBoundingBox().len_x();
      if (pShape->getBoundingBox().len_y() < min)
      	min = pShape->getBoundingBox().len_y();
      if (pShape->getBoundingBox().len_z() < min)
      	min = pShape->getBoundingBox().len_z();

      dist -= min;

		if (dist <= 1.0f)
		   dist = 1;
		if (dist >= radius)
		   continue;

		float od = 1.0 / dist;
		Point3F vVec;
		if (vec.x + vec.y + vec.z == 0) {
			// Use vector to center if no velocity
			// was passed.
			vVec.x = mVec.x * od;
			vVec.y = mVec.y * od;
			vVec.z = mVec.z * od;
		}
		else
			vVec = vec;

		float scale = (radius - dist) / radius;
		float damage = damageValue * scale;
		mVec *= force * scale * od;

      float cover = pShape->coverage(pos);
      damage *= cover;
      mVec   *= cover;
      
      if (cover != 0.0f)
		   pShape->applyDamage(type,damage,pos,vVec,mVec,srcObjectId);
   }
}
