//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include "fearglobals.h"
#include <simResource.h>
#include "smokePuff.h"
#include "fxRenderImage.h"

UInt32 SmokeManager::sm_numPuffsAllowed = 100;

struct SmokeManager::SmokePuff
{
  public:
   bool     m_active;

   DWORD    m_endTime;

   Resource<TS::Shape> m_resShape;
   fxRenderImage       m_SmokeImage;
   float               m_axisSpin;

   Point3F  m_position;
   Point3F  m_velocity;
   Point3F  m_accel;

  public:
   SmokePuff() { m_active = false; }
   void deactivate() {
      delete m_SmokeImage.shape;
      m_resShape.unlock();
      m_active = false;
   }
   ~SmokePuff() { if(m_active) deactivate(); }
};


SmokeManager::SmokeManager()
 : m_pPuffArray(NULL),
   m_currentlyAllocatedPuffs(0),
   m_currentPuff(0),
   m_numActivePuffs(0)
{
   //
}

SmokeManager::~SmokeManager()
{
   delete [] m_pPuffArray;
   m_pPuffArray = NULL;

   m_currentlyAllocatedPuffs = 0;
   m_currentPuff             = 0;
}

void
SmokeManager::clientProcess(DWORD in_currTime)
{
   if (m_currentlyAllocatedPuffs != sm_numPuffsAllowed)
      reallocatePuffs();   

   DWORD iSlice  = in_currTime - m_lastUpdated;
   float fSlice  = float(iSlice) / 1000.0f;
   m_lastUpdated = in_currTime;

   if (m_numActivePuffs == 0)
      return;

   for (UInt32 i = 0; i < m_currentlyAllocatedPuffs; i++) {
      if (m_pPuffArray[i].m_active == true) {
         SmokePuff& rPuff = m_pPuffArray[i];
         if (in_currTime >= rPuff.m_endTime) {
            rPuff.deactivate();
            m_numActivePuffs--;
         } else {
            rPuff.m_position += rPuff.m_velocity * fSlice;

            float prevPosition = rPuff.m_SmokeImage.thread->getPosition();
            rPuff.m_SmokeImage.thread->AdvanceTime(fSlice);
            float newPosition = rPuff.m_SmokeImage.thread->getPosition();
            if (newPosition < prevPosition) {
               // Wrapped around, dammit.
               rPuff.deactivate();
               m_numActivePuffs--;
            }
         }
      }
   }
}

void
SmokeManager::reallocatePuffs()
{
   if (sm_numPuffsAllowed > SimRenderQueryImage::MaxImage)
      sm_numPuffsAllowed = SimRenderQueryImage::MaxImage;

   if (m_pPuffArray != NULL) {
      // Resetting the number of allowed puff flushes all active puffs.
      //  In the future, we may want to handle this better...
      for (UInt32 i = 0; i < m_currentlyAllocatedPuffs; i++)
         if (m_pPuffArray[i].m_active == true) {
            m_pPuffArray[i].deactivate();
         }
      delete [] m_pPuffArray;
   }

   if (sm_numPuffsAllowed != 0) m_pPuffArray = new SmokePuff[sm_numPuffsAllowed];
   else                         m_pPuffArray = NULL;

   m_currentlyAllocatedPuffs = sm_numPuffsAllowed;
   m_currentPuff             = 0;
   m_numActivePuffs          = 0;
}

bool
SmokeManager::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   addToSet(SimRenderSetId);
   addToSet(MoveableSetId);

   m_lastUpdated = wg->currentTime;

   reallocatePuffs();

   return true;
}

bool
SmokeManager::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   TS::RenderContext& rc = *query->renderContext;

   query->count = 0;
   for (UInt32 i = 0; i < m_currentlyAllocatedPuffs; i++) {
      if (m_pPuffArray[i].m_active == true) {
         SmokePuff& rPuff = m_pPuffArray[i];

         rPuff.m_SmokeImage.transform.p = rPuff.m_position;
         rPuff.m_SmokeImage.setAxisSpin(rPuff.m_axisSpin);
         rPuff.m_SmokeImage.faceCamera(rc);
         rPuff.m_SmokeImage.setSortValue(rc.getCamera());
         
         query->image[query->count++] = &rPuff.m_SmokeImage;
      }
   } 

   return true;
}

bool
SmokeManager::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);

     default:
      return Parent::processQuery(query);
   }
}

void
SmokeManager::addPuff(const char*    in_pPuffName,
                      const float    in_puffTime,
                      const Point3F& in_rPuffPosition,
                      const Point3F& in_rPuffVelocity)
{
   if (m_pPuffArray == NULL)
      return;


   SmokePuff& rSmokePuff = m_pPuffArray[m_currentPuff];
   m_currentPuff++;
   if (m_currentPuff >= m_currentlyAllocatedPuffs)
      m_currentPuff = 0;

   if (rSmokePuff.m_active == true) {
      rSmokePuff.deactivate();
      m_numActivePuffs--;
   }

   rSmokePuff.m_active = true;
   
   // Ok, the set up the shape resource and the render image...
   //
   rSmokePuff.m_SmokeImage.transform.identity();
   rSmokePuff.m_SmokeImage.itype = SimRenderImage::Translucent;
   ResourceManager* rm = SimResource::get(manager);
   rSmokePuff.m_resShape = rm->load(in_pPuffName, true);
   AssertFatal(bool(rSmokePuff.m_resShape) == true, avar("No shape: %s!", in_pPuffName));
   rSmokePuff.m_SmokeImage.shape = new TS::ShapeInstance(rSmokePuff.m_resShape, *rm);
   AssertFatal(rSmokePuff.m_SmokeImage.shape != NULL, "Unable to create shapeInst");
   rSmokePuff.m_SmokeImage.thread = rSmokePuff.m_SmokeImage.shape->CreateThread();
   AssertFatal(rSmokePuff.m_SmokeImage.thread != NULL, "Unable to create animThread");
   float timeScale = rSmokePuff.m_SmokeImage.thread->getSequence().fDuration / in_puffTime;
   rSmokePuff.m_SmokeImage.thread->setTimeScale(timeScale);
   
   // Fudgy, but it really sucks when the animation wraps due to precision loss.
   //
   rSmokePuff.m_endTime  = wg->currentTime + DWORD(in_puffTime * 1000.0f);

   static Random rand;
   rSmokePuff.m_axisSpin = rand.getFloat() * M_2PI;

   rSmokePuff.m_position = in_rPuffPosition;
   rSmokePuff.m_velocity = in_rPuffVelocity;
   m_numActivePuffs++;
}

void
SmokeManager::addPuff(Resource<TS::Shape>& rSmokeShape,
                      const float          in_puffTime,
                      const Point3F&       in_rPuffPosition,
                      const Point3F&       in_rPuffVelocity)
{
   if (m_pPuffArray == NULL)
      return;
   AssertFatal(bool(rSmokeShape) == true, "Error, bogus resource passed");

   SmokePuff& rSmokePuff = m_pPuffArray[m_currentPuff];
   m_currentPuff++;
   if (m_currentPuff >= m_currentlyAllocatedPuffs)
      m_currentPuff = 0;

   if (rSmokePuff.m_active == true) {
      rSmokePuff.deactivate();
      m_numActivePuffs--;
   }

   rSmokePuff.m_active = true;
   
   // Ok, the set up the shape resource and the render image...
   //
   rSmokePuff.m_SmokeImage.transform.identity();
   rSmokePuff.m_SmokeImage.itype = SimRenderImage::Translucent;

   rSmokePuff.m_resShape = rSmokeShape;

   ResourceManager* rm = SimResource::get(manager);
   rSmokePuff.m_SmokeImage.shape = new TS::ShapeInstance(rSmokePuff.m_resShape, *rm);
   AssertFatal(rSmokePuff.m_SmokeImage.shape != NULL, "Unable to create shapeInst");
   rSmokePuff.m_SmokeImage.thread = rSmokePuff.m_SmokeImage.shape->CreateThread();
   AssertFatal(rSmokePuff.m_SmokeImage.thread != NULL, "Unable to create animThread");
   float timeScale = rSmokePuff.m_SmokeImage.thread->getSequence().fDuration / in_puffTime;
   rSmokePuff.m_SmokeImage.thread->setTimeScale(timeScale);
   
   // Fudgy, but it really sucks when the animation wraps due to precision loss.
   //
   rSmokePuff.m_endTime  = wg->currentTime + DWORD(in_puffTime * 1000.0f);

   static Random rand;
   rSmokePuff.m_axisSpin = rand.getFloat() * M_2PI;

   rSmokePuff.m_position = in_rPuffPosition;
   rSmokePuff.m_velocity = in_rPuffVelocity;
   m_numActivePuffs++;
}

void
SmokeManager::removeAllInRadius(Point3F position,
                                float   in_radius)
{
   float distSq = in_radius * in_radius;

   if (m_numActivePuffs != 0) {
      for (UInt32 i = 0; i < sm_numPuffsAllowed; i++) {
         if (m_pPuffArray[i].m_active == true) {
            if ((position - m_pPuffArray[i].m_position).lenSq() <= distSq) {
               m_pPuffArray[i].deactivate();
               m_numActivePuffs--;
            }
         }
      }
   }
}

