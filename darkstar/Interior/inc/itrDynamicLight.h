//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ITRDYNAMICLIGHT_H_
#define _ITRDYNAMICLIGHT_H_

//Includes
#include <base.h>
#include <m_point.h>
#include <tVector.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class ITRInstance;

class ITRDynamicLight {
   Point3F m_position;
   float   m_radius;
   UInt32  m_packedColor;

   float   m_radiusSq;        // Both...
   float   m_distanceBias;    // Calculated in update(), maps 0..radius to
                              //  0..255 for use with intensity lookup table
  public:
   ITRDynamicLight();
   
   void update(ITRInstance&   io_rInstance,
                       const Point3F& in_rNewPosition,
                       const float    in_radius,
                       const UInt32   in_packedColor);
   void clear(ITRInstance& io_rInstance);

   Point3F getPosition() const    { return m_position; }
   float   getRadius() const      { return m_radius; }
   UInt32  getPackedColor() const { return m_packedColor; }

   UInt8 getIntensity(float in_distSquared) const;

   // Placed here for ITRInstance to find...
   //
   typedef Vector<int> SurfaceVector;
   SurfaceVector m_affectedSurfaces;
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_ITRDYNAMICLIGHT_H_
