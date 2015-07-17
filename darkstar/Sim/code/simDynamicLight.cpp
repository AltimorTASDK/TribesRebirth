//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simDynamicLight.h"

namespace {

inline float
getDistanceBias(const float in_radius)
{
   return (255.0f / (in_radius * in_radius));
}

const Point3F sg_defPosition(0.0f, 0.0f, 0.0f);

const float sg_defMovementTolerance = 0.0f;
const float sg_defRadiusTolerance   = 0.0f;
const float sg_defRadius            = 1.0f;
const DWORD sg_defPackedColor       = 0xffffffff;


// Sqrt lookup table, and dummy class to initialize it...
//
UInt8 sg_dynamicLightIntenTable[256];

class SQRTINIT { public: SQRTINIT(); } __sg_initObject;
SQRTINIT::SQRTINIT()
{
   for (int i = 0; i < 256; i++) {
      float sqrt = m_sqrt(float(i)/255.0);
      sqrt *= 255.0f;
      sg_dynamicLightIntenTable[i] = UInt8(255) - UInt8(sqrt);
   }
}

}



SimDynamicLight::SimDynamicLight()
{
   m_currPosition = sg_defPosition;
   m_transform.identity();

   m_radius      = sg_defRadius;
   m_distSqBias  = getDistanceBias(m_radius);
   m_packedColor = sg_defPackedColor;

   m_movementTolerance = sg_defMovementTolerance;
   m_radiusTolerance   = sg_defRadiusTolerance;
}


SimDynamicLight::SimDynamicLight(const Point3F& in_rPosition,
                                 const TMat3F&  in_rTransform,
                                 const float    in_radius,
                                 const DWORD    in_packedColor)
 : m_currPosition(in_rPosition),
   m_transform(in_rTransform),
   m_radius(in_radius),
   m_packedColor(in_packedColor)
{
   m_distSqBias = getDistanceBias(m_radius);
   m_transform.identity();

   m_movementTolerance = sg_defMovementTolerance;
   m_radiusTolerance   = sg_defRadiusTolerance;
}


SimDynamicLight::~SimDynamicLight()
{

}

UInt8 
SimDynamicLight::getIntensity(const float in_distSq) const
{
   float biasedDistSq = in_distSq * m_distSqBias;

   if (biasedDistSq > 255.0f) {
      return 0;
   }
   
   return sg_dynamicLightIntenTable[UInt8(biasedDistSq)];
}

void 
SimDynamicLight::updateLight()
{
   setAffectedObjects();
}


void 
SimDynamicLight::updateBoundingBox()
{
   Point3F worldPoint;
   m_mul(m_currPosition, m_transform, &worldPoint);
   
   Box3F newBBox(worldPoint, worldPoint);

   Point3F radPoint(m_radius, m_radius, m_radius);
   newBBox.fMin -= radPoint;
   newBBox.fMax += radPoint;

   m_boundingBox = newBBox;
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    SimDynamicLight::setAffectedObjects()
//    
// DESCRIPTION 
//    Determines the objects that are currently affected by the dynamic light,
//     and sends them tracking events.
//    
// NOTES 
//    Some trickiness occurs here:
//     When the updateDynamicLight event is sent to the containable, if it
//    is processed, the container MUST increment a reference count on the
//    registered light it tracks.  So the first update set to a C'able
//    creates the registered entry, and increments it's reference count
//    to one.  After sending all of these messages, we will send a
//    decDynamicLightRefCount event to all objects that were affected by
//    the previous state of this light.  If the light no longer affects
//    that object, it will not have had its reference count increased to two
//    by an updateLight event, and it will deregister the light.  If the
//    light was previously affected and is still, it's final refCount should
//    be one.  Likewise if an objectis newly affected, it's refCount should
//    be one.
//   clearAffectedObjects is called when the light is removed from the world,
//    it sends decrement events to all objects it affects which should remove
//    it completely from all object registries, and clears its internal
//    registry.
//------------------------------------------------------------------------------
void 
SimDynamicLight::setAffectedObjects()
{
   updateBoundingBox();    // Make sure bounding box is in the correct place
   
   // Query the Container database to determine which Containable objects
   //  are affected by this light...
   //
   SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);

   SimContainerQuery scQuery;
   scQuery.id   = id;
   scQuery.type = -1;
   scQuery.mask = -1;
	scQuery.detail = SimContainerQuery::DefaultDetail;
   scQuery.box  = m_boundingBox;
   
   // it'd be nice to be able to mask in only container objects in this query
	SimContainerList scList;
	root->findIntersections(scQuery, &scList);

   Point3F worldPoint;
   m_mul(m_currPosition, m_transform, &worldPoint);

   SimDynamicLightUpdateEvent sdluEvent;
   sdluEvent.m_pDLight = this;

   // Pump register and decRefCount events directly into the
   //  containers message function.
   //
   int i;
   for (i = 0; i < scList.size(); i++) {
      // DMMNOTE: Need to transform into conatiners space...
      //
      if (!scList[i]->isOpen())
         continue;
      sdluEvent.worldPosition = worldPoint;
      scList[i]->processEvent(&sdluEvent);
   }
   
   // Send decrement ref count events...
   //
   SimDynamicLightDecRefCntEvent sdldrcEvent;
   sdldrcEvent.m_pDLight = this;

   for (i = 0; i < m_affectedObjects.size(); i++)
      m_affectedObjects[i]->processEvent(&sdldrcEvent);

   // Set our currently affected list...
   //
   m_affectedObjects.clear();
   for (i = 0; i < scList.size(); i++)
      if (scList[i]->isOpen())
         m_affectedObjects.push_back(scList[i]);
}

void 
SimDynamicLight::clearAffectedObjects()
{
   // Send decrement ref count events...
   //
   SimDynamicLightDecRefCntEvent sdldrcEvent;
   sdldrcEvent.m_pDLight = this;

   for (int i = 0; i < m_affectedObjects.size(); i++)
      m_affectedObjects[i]->processEvent(&sdldrcEvent);

   // Clear our currently affected list...
   //
   m_affectedObjects.clear();
}


void 
SimDynamicLight::setPosition(const Point3F& in_rPosition)
{
   if (m_dist(in_rPosition, m_currPosition) <= m_movementTolerance)
      return;

   m_currPosition = in_rPosition;
   setAffectedObjects();
}

void 
SimDynamicLight::setTransform(const TMat3F& in_rTransform)
{
   m_transform = in_rTransform;
   
   setAffectedObjects();
}

void 
SimDynamicLight::setRadius(const float in_radius)
{
   if (m_abs(in_radius - m_radius) <= m_radiusTolerance)
      return;
   
   m_radius     = in_radius;
   m_distSqBias = getDistanceBias(in_radius);
   setAffectedObjects();
}


void 
SimDynamicLight::setPackedColor(const DWORD in_packedColor)
{
   if (in_packedColor == m_packedColor)
      return;
   
   m_packedColor = in_packedColor;
   setAffectedObjects();
}

bool SimDynamicLight::onAdd()
{
   if(!Parent::onAdd())
      return false;
   setAffectedObjects();
   return true;
}

void SimDynamicLight::onRemove()
{
   clearAffectedObjects();
   Parent::onRemove();
}
