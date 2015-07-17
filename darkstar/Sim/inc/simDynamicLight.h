//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMDYNAMICLIGHT_H_
#define _SIMDYNAMICLIGHT_H_

//Includes
#include "simCollideable.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class SimDynamicLight : public SimObject
{
   typedef SimObject Parent;
   // instance data.  Derived classes must manipulate this info through
   //  the access functions.  Each of these has a default setting, in
   //  simDynamicLight.cpp
   //
  private:
   SimContainerList m_affectedObjects;
   
   Box3F    m_boundingBox;    // bounding box in world space

   Point3F  m_currPosition;   // Current position in object space
   TMat3F   m_transform;      // transform to world space
   
   float    m_radius;
   DWORD    m_packedColor;
   float    m_distSqBias;

   void updateBoundingBox();

  protected:
   float m_movementTolerance;    // Derived lights may override these to select
   float m_radiusTolerance;      //  how far a light must move before it updates
                                 //  the surfaces affected by it.  Default value
                                 //  is 0.0f

   virtual void setAffectedObjects();
   virtual void clearAffectedObjects();

  public:
   SimDynamicLight();
   SimDynamicLight(const Point3F& in_rPosition,
                              const TMat3F&  in_rTransform,
                              const float    in_radius      = 1.0f,
                              const DWORD    in_packedColor = 0xffffffff);
   ~SimDynamicLight();

   virtual UInt8 getIntensity(const float in_distSq) const;
   virtual void  updateLight();

   Point3F getPosition() const    { return m_currPosition; }
   TMat3F  getTransform() const   { return m_transform; }
   float   getRadius() const      { return m_radius; }
   DWORD   getPackedColor() const { return m_packedColor; }

   void setPosition(const Point3F& in_rPosition);
   void setTransform(const TMat3F& in_rTransform);
   void setRadius(const float in_radius);
   void setPackedColor(const DWORD in_packedColor);

   bool onAdd();
   void onRemove();
};

class SimDynamicLightUpdateEvent: public SimEvent
{
  public:
   SimDynamicLight* m_pDLight;
   Point3F          worldPosition;

	SimDynamicLightUpdateEvent() { type = SimDynamicLightUpdateEventType; }
};

class SimDynamicLightDecRefCntEvent: public SimEvent
{
  public:
   SimDynamicLight* m_pDLight;

	SimDynamicLightDecRefCntEvent() { type = SimDynamicLightDecRefCntEventType; }
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SIMDYNAMICLIGHT_H_
