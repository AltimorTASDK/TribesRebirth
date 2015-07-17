//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ITRBASICLIGHTING_H_
#define _ITRBASICLIGHTING_H_

//Includes
#include <persist.h>
#include <tString.h>
#include <zedPersLight.h>
#include "itrgeometry.h"
#include "itrlighting.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//----------------------------------------------------------------------------

class GFXBitmap;

namespace ITRBasicLighting
{
   struct LightMap;
   struct StateLightMap;

   struct CallbackInfo
   {
      bool ( * collide )( void * obj, Point3F & start, Point3F & end, ColorF & col );
      void * obj;
   };
   
   struct MapPoint {
      int average;   // Point to be averaged.
      Point3F pos;
      int mapX;
      int mapY;
      bool visible;
   };
   typedef Vector<MapPoint> MapPointList;

   struct Color {
      float red,green,blue;
      Color& operator+=(const Color&);
      Color& operator*=(const float&);
      void   set(float r, float g, float b) { red = r; green = g; blue = b; }
      DWORD  buildPackedColor() const;
   };

   struct Light {
      struct State;
      
      // Instance Data...
      Vector<State*> state;
      int            id;
      String         name;
      float          animationDuration;
      DWORD          flags;

      struct State {
         struct Surface {
            int surface;
            int lightIndex;
            int mapIndex;
         };
         enum LightType {
            PointLight = 0,
            SpotLight  = 1,
            DirectionalLight = 2
         };
         enum Falloff {
            Distance = 0,
            Linear   = 1,
         };

         // State data...
         float stateDuration;    // how long does this light state last
         Color color;

         struct Emitter {
            LightType lightType;    // spot/point

            Point3F   pos;
            Point3F   spotVector;
            float     spotTheta;      // inner angle for spotlights
            float     spotPhi;        // outer angle for spotlights

            Falloff   falloff;
            float     d1, d2, d3;       // Distance coefficients

            // Not from perslight...
            float  radius;

            double getDistCoef(const Point3F& in_pos);
            bool   isLit(ITRGeometry& geometry,ITRGeometry::Surface& surface);
            bool   operator==(const Emitter&) const;
         };
         Vector<Emitter> m_emitterList;

         Int32 uniqueKey;       // if uniqueKey is -1, then surface is unique,.
                                //  or the first of a series of similar states.
                                //  Otherwise, uKey is index of state that this
                                //  one matches.  Matching states share
                                //  surfaceData
         Vector<Surface> surfaceData;

         bool   light(ITRGeometry&  geometry,
                      MapPointList& pointList,
                      Point3F&      normal,
                      LightMap*     map,
                      CallbackInfo* callback );
         bool   lightState(ITRGeometry&   geometry,
                           MapPointList&  pointList,
                           Point3F&       normal,
                           StateLightMap* map);
         void   appendSurface(int surface,int lightIndex, int mapIndex);
      };
      
      Vector<int> uniqueStates;  // indices of states that are unique.

      const Light& operator=(const ZedLight::PersLight&);
      void scale(const float in_worldScale);
   };
   typedef Vector<Light*> LightList;

   // Only the setColor and emission mask material properties are supported
   //  under basic lighting...
   //
   struct MaterialProp : public Persistent::Base {
      bool  modified;
      enum EmissionType {
         flatColor,
         flatEmission,
      } emissionType;

      Color color;
      float intensity;

      bool  modifiedPhong;
      float phongExp;
      
      GFXBitmap *pBitmap;
      GFXBitmap *pEmissionMask;
      
	   Error read( StreamIO &, int version, int );
	   Error write( StreamIO &, int version, int );
   };
   typedef Vector<MaterialProp> MaterialPropList;

   void writeMaterialPropList(StreamIO&         io_rStream, 
                              MaterialPropList& io_rMList);
   void readMaterialPropList(StreamIO&         io_rStream, 
                             MaterialPropList& io_rMList);

   //
   extern float filterScale;
   extern int   lightScale;
   extern bool  g_testPattern;
   extern bool  g_applyAmbientOutside;
   extern bool  g_useNormal;
   extern bool  g_useLightMaps;
   extern bool  g_missionLighting;
   extern float distancePrecision;
   extern float normalPrecision;
   extern int   g_quantumNumber;
   extern ColorF m_ambientIntensity;

   //
   void light( ITRGeometry & geometry, LightList & lights,
      ITRLighting * lighting, ITRMissionLighting * missionLighting, 
      CallbackInfo * callback );
   void light(ITRGeometry&         geometry,
              LightList&           lights,
              MaterialPropList&    matProps,
              ITRLighting*         lighting );
   void fillAmbient( LightMap * map );

//-------------------------------------- INLINE COLOR OPS
   inline DWORD
   Color::buildPackedColor() const
   {
      float mono = red   * 0.299 +
                   green * 0.587 +
                   blue  * 0.114;
      DWORD r, g, b, m;
      r = DWORD((red   * 255.0f) + 0.5f);
      g = DWORD((green * 255.0f) + 0.5f);
      b = DWORD((blue  * 255.0f) + 0.5f);
      m = DWORD((mono  * 255.0f) + 0.5f);
      return (m << 24) |
             (r << 16) |
             (g << 8) |
             (b << 0);
   }

   inline Color
   operator*(const Color& in_color, const float& in_mul)
   {
      Color ret = in_color;
      ret.red   *= in_mul;
      ret.green *= in_mul;
      ret.blue  *= in_mul;
      return ret;
   }
   inline Color
   operator-(const Color& in_color1, const Color& in_color2)
   {
      Color ret = in_color1;
      ret.red   -= in_color2.red;
      ret.green -= in_color2.green;
      ret.blue  -= in_color2.blue;
      return ret;
   }
}; // namespace ITRBasicLighting


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // _ITRBASICLIGHTING_H_

