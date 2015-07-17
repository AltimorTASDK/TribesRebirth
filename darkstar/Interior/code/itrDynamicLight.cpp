//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <ts_shapeInst.h>
#include "itrDynamicLight.h"
#include "itrInstance.h"

ITRDynamicLight::ITRDynamicLight()
 : m_affectedSurfaces(32)
{
   // Does nothing but reset the blocksize for the surface vector.  All action
   //  happens in the first update() call...
}


UInt8 ITRDynamicLight::getIntensity(float in_distSquared) const
{
   if (in_distSquared >= m_radiusSq)
      return 0;

   AssertFatal(in_distSquared >= 0, avar("foo: %f", in_distSquared));
   float ret = sqrt(in_distSquared) * m_distanceBias;

   return (255 - UInt8(ret));
}


void ITRDynamicLight::update(ITRInstance&   io_rInstance,
                        const Point3F& in_rNewPosition,
                        const float    in_radius,
                        const UInt32   in_packedColor)
{
   m_position    = in_rNewPosition;
   m_radius      = in_radius;
   m_packedColor = in_packedColor;
   
   // Calculate new distance bias...
   //
   m_radiusSq     = in_radius * in_radius;
   m_distanceBias = 255.0f / in_radius;
   
   // Send update message to the itrinstance to find affected surfaces...
   //
   io_rInstance.findAffectedSurfaces(this);
}


void ITRDynamicLight::clear(ITRInstance& io_rInstance)
{
   // Send update message to the itrinstance to clear affected surfaces...
   //
   io_rInstance.clearAffectedSurfaces(this);
}

