//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ITRLIGHT_H_
#define _ITRLIGHT_H_

//Includes
#include <persist.h>
#include <tString.h>
#include <m_point.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace ZedLight {

class PersLight : public Persistent::Base
{
   friend class PersLightList;

  public:
   ~PersLight();

   class PersState;
   
   // Instance Data...
   Vector<PersState*> state;

   int    id;
   String name;
   float  animationDuration;
   DWORD  flags;

   class PersState : public Persistent::Base
   {
     public:
      enum LightType {
         PointLight = 0,
         SpotLight  = 1,
         
         __forceDWORD1 = 0x7fffffff
      };
      enum Falloff {
         Distance = 0,
         Linear   = 1,

         __forceDWORD2 = 0x7fffffff
      };

      // State data...
      float  stateDuration;   // how long does this light state last
      ColorF color;

      struct Emitter {
         LightType lightType;    // spot/point

         Point3F   pos;
         Point3F   spotVector;
         float     spotTheta;      // inner angle for spotlights
         float     spotPhi;        // outer angle for spotlights

         Falloff   falloff;
         float     d1, d2, d3;       // Distance coefficients
      };

      Vector<Emitter> m_emitterList;

     public:
      // Persistent IO
      DECLARE_PERSISTENT(PersState);
      Error read(StreamIO &,  int version, int );
      Error write(StreamIO &, int version, int );
   };
   
   // Persistent IO
   DECLARE_PERSISTENT(PersLight);
   Error read(StreamIO &,  int version, int );
   Error write(StreamIO &, int version, int );
};

class PersLightList : public Persistent::Base, public Vector<PersLight*>
{
   typedef Vector<PersLight*> Parent;
   
  public:
   ~PersLightList();
  
   void clear();
  
   // Persistent IO
   DECLARE_PERSISTENT(PersLightList);
   Error read(StreamIO &,  int version, int );
   Error write(StreamIO &, int version, int );
};



}; // namespace ZedLight

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_ITRLIGHT_H_
