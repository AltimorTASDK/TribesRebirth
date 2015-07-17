//---------------------------------------------------------------------------
//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
//      $Version: $
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_LIGHT_H_
#define _TS_LIGHT_H_

#include <ml.h>

#include <tvector.h>
#include <talgorithm.h>

namespace TS
{
#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

   class Vertex;

   //---------------------------------------------------------------------------
   // class Light
   //    a single light source

   class Light 
      {
      //------------------------------------------------------------------------
      // Minimum ratio of distance to radius for omni lights to be optimized:
      const RealF          MinPointOptimizeRatio;
      enum  PrepareOptimizations {
            LightOutOfRange,              LightNotOptimized,
            LightFullyOptimized,          LightMostlyOptimized,
            };

   public:
      enum LightType
         {
         LightInvalid      = 0,
         LightDirectional  = 1,
         LightPoint        = 2,
         LightParallelPoint= 3,
         LightCone         = 4,
         LightDirectionalNonlinear  = 5,
         LightDirectionalWrap       = 6, 
         };

      struct LightInfo
         {
         //==============================================================
         // put stuff used most often all together here in first 64 bytes
         // to improve cache performance:

         Int32    fType;         // type of light (e.g. LightPoint)

         RealF    fRed;  
         RealF    fGreen;
         RealF    fBlue;

         Point3F  fDirection;    // Direction of incident light (L)
         RealF    fShadowAtten;

         //==============================================================
         // stuff here is used for optimized point lights - next 8 bytes
         Int32    fOptimized;
         RealF    fDistanceAtten;

         //==============================================================
         // stuff here for unoptimized point lights - next 24 bytes

         Point3F  fPosition;     // Position of point or spot
         RealF    fAttenuation0; // constant attenuation
         RealF    fAttenuation1; // linear attenuation
         RealF    fAttenuation2; // quadratic attenuation

         //==============================================================

         RealF    fRange;        // Cut off range
         RealF    fFalloff;      // Fall off ???
         bool     linearFalloff; 
         RealF    fTheta;        // Inner angle of spot lights
         RealF    fPhi;          // Outer angle of spot lights

         Point3F  fAim;          // Aim of cone light in obj space (K)

         Point3F  fWPosition;    // Position of light in world space
         Point3F  fWAim;         // Direction of light in world space

         Point3F  fLightToSurface;  // For cone light
         RealF    fCosTheta;     // cosine of theta
         RealF    fInvCosThetaMinusCosPhi;   // 1 / (cos(theta) - cos(phi))

         // for shadowing effects with directional lights only
         UInt8 *  fShadowMap;
         Int32    fShadowMask;
         Int32    fShadowShift;
         RealF    fShadowScale;
         Point2F  fShadowOffset;

         bool     fStaticLight;
         };

   private:

      friend class SceneLighting;

      //----------------------------------------------------
      // instance data:
   protected:
      typedef void (Light::*prepareFunction)
         (SphereF const &, TMat3F const &);

      typedef void (Light::*calcIntensityVertexFunction)
         (Vertex const &, ColorF *) const;

      typedef void (Light::*calcIntensityPointFunction)
         (Point3F const &, ColorF *) const;

      // fReady is used to keep track of whether any light property or the 
      // object transform has changed since the last time the light was
      // prepared.
      Bool                 fReady;

      prepareFunction                  fpfPrepare;
      calcIntensityVertexFunction      fpfCalcIntensityVertex;
      calcIntensityVertexFunction      fpfCalcIntensityVertexWorld;
      calcIntensityPointFunction       fpfCalcIntensityPoint;
      calcIntensityPointFunction       fpfCalcIntensityPointWorld;
   public:
      LightInfo            fLight;
      //
      //----------------------------------------------------

      //----------------------------------------------------
      // instance methods:
   protected:
      void         invalidate();

      void         resetFuncs();
      void         setFuncs();
         
      void         prepare( SphereF const & target, TMat3F const & tow );

      void         directionalPrepare( SphereF const & target, 
                                 TMat3F const & tow );
      void         pointPrepare( SphereF const & target, 
                                 TMat3F const & tow );
      void         invalidPrepare( SphereF const & targetLoc, 
                                 TMat3F const & tow );


      void         directionalIntensityPoint( Point3F const & loc, ColorF * vi ) const;
      void         directionalIntensityVertex( Vertex const & vertex, ColorF * vi ) const;
      void         directionalNonlinearIntensityVertex( Vertex const & vertex, ColorF * vi ) const;
      void         directionalWrapIntensityVertex( Vertex const & vertex, ColorF * vi ) const;
      void         directionalIntensityPointWorld( Point3F const & loc, ColorF * vi ) const;
      void         directionalIntensityVertexWorld( Vertex const & vertex, ColorF * vi ) const;

      void         pointIntensityPoint( Point3F const & loc, ColorF * vi ) const;
      void         pointIntensityVertex( Vertex const & vertex, ColorF * vi ) const;
      void         pointIntensityPointWorld( Point3F const & loc, ColorF * vi ) const;
      void         pointIntensityVertexWorld( Vertex const & vertex, ColorF * vi ) const;

      void         invalidIntensityPoint( Point3F const & loc, ColorF * vi ) const;
      void         invalidIntensityVertex( Vertex const & vertex, ColorF * vi ) const;
      void         invalidIntensityPointWorld( Point3F const & loc, ColorF * vi ) const;
      void         invalidIntensityVertexWorld( Vertex const & vertex, ColorF * vi ) const;
   public:
      void         calcIntensity( Point3F const & loc, ColorF * vi ) const;
      void         calcIntensity( Vertex const & vertex, ColorF * vi ) const;
      void         calcIntensityWorld( Point3F const & loc, ColorF * vi ) const;
      void         calcIntensityWorld( Vertex const & vertex, ColorF * vi ) const;

   	          Light();

      bool         isStaticLight() const { return fLight.fStaticLight; }
      void         resetShadow();
      void         setShadow( Point2F const  & offset, RealF scale, 
                              Int32 size, UInt8 * map );
   	void         setShadowOffset(Point2F const & offset);

      void         setType( LightType type );
      void         setIntensity( RealF red, RealF green, RealF blue );
      void         setIntensity( const ColorF& color );
      void         setAim( Point3F const & aim );
      void         setPosition( Point3F const & pos );
      void         setDistanceCoefficients( RealF a0, RealF a1, RealF a2 );
      void         setRange( RealF range );
		
      float       getRange (void) {return fLight.fRange;}
      float       attenuationFormula ( float d )  const   {
                        return ( fLight.fAttenuation0 + 
                        (fLight.fAttenuation1 + fLight.fAttenuation2 * d) * d  
                        );
                     }
      

      //
      //----------------------------------------------------
      };

   //---------------------------------------------------------------------------
   // class SceneLighting
   //    a collection of light sources

   class SceneLighting : public VectorPtr<Light*>
      {
      //----------------------------------------------------
      // instance data:
   private:
      ColorF            fAmbientIntensity; // intensity of ambient light

      // it would be bad to add/remove lights in the middle of things:
      Bool              fLocked;
      // allow us to ignore the lights if we want -- set with lightsOn() and lightsOff()
		Bool              fLightsOn;
      //----------------------------------------------------

      //----------------------------------------------------
      // instance methods:
   public:

      SceneLighting();
      ~SceneLighting();

      // Lock/Unlock state
      void         lock();
      void         unlock();
      Bool         isLocked() const;

		void         lightsOn() { fLightsOn = true; }
		void         lightsOff() { fLightsOn = false; }

      void         setObjectTransform();

      void         prepare( SphereF const & target, 
                              TMat3F const & tow );
      
      void         setAmbientIntensity( ColorF const & intensity );
   	ColorF const &  getAmbientIntensity() const;

      void         installLight( Light * pLight );
      void         removeLight( Light * pLight );

      void         calcIntensity( Vertex const & vertex, 
                              ColorF * intensity ) const;
      void         calcIntensity( Point3F const & loc, Int32 normalCode, 
                              ColorF * intensity ) const;
      void         calcIntensity( Point3F const & loc, 
                              ColorF * intensity ) const;
      void         calcIntensityWorld( Vertex const & vertex, 
                              ColorF * intensity ) const;
      void         calcIntensityWorld( Point3F const & loc, 
                              ColorF * intensity ) const;

      //
      //----------------------------------------------------
      };

   //---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

   //---------------------------------------------------------------------------
   // Light methods
   //---------------------------------------------------------------------------

   inline void Light::invalidate()
      {
      fReady = FALSE;
      }

   inline void Light::setType( LightType type )
      {
      fLight.fType = type;
      setFuncs();
      invalidate();
      }

   //---------------------------------------------------------------------------

   inline void Light::setIntensity( RealF red, RealF green, RealF blue )
      {
      fLight.fRed = red;
      fLight.fGreen = green;
      fLight.fBlue = blue;
      invalidate();
      }

   inline void Light::setIntensity( const ColorF& color )
      {
      setIntensity(color.red, color.green, color.blue);
      }

   inline void Light::setAim( Point3F const & aim )
      {
      fLight.fWAim = aim;
      invalidate();
      }

   inline void Light::setPosition( Point3F const & pos )
      {
      fLight.fWPosition = pos;
      invalidate();
      }

   inline void Light::setDistanceCoefficients( RealF a0, RealF a1, 
      RealF a2 )
      {
      fLight.fAttenuation0 = a0;
      fLight.fAttenuation1 = a1;
      fLight.fAttenuation2 = a2;
      invalidate();
      }

   inline void Light::setRange( RealF range )
      {
      fLight.fRange = range;
      invalidate();
      }

   //---------------------------------------------------------------------------

   inline void Light::resetShadow()
      {
      fLight.fShadowMap = 0;
      invalidate();
      }

   inline void Light::setShadow( Point2F const & offset, RealF scale, 
      Int32 size, UInt8 * map )
      {
      // width and height must be the same
      // size is log2(width or height)

      AssertFatal( fLight.fType == LightDirectional,
         "TS::Light::setShadow: only directional lights can have shadow maps" );

      fLight.fShadowShift = size;
      fLight.fShadowMask = (1<<size) - 1;
      fLight.fShadowMap = map;
      fLight.fShadowScale = scale;
      fLight.fShadowOffset = offset;
      invalidate();
      }

   inline void Light::setShadowOffset(Point2F const & offset)
      {
   	fLight.fShadowOffset = offset;
      invalidate();
      }

   //---------------------------------------------------------------------------

   inline void Light::prepare( SphereF const & target,
      TMat3F const & tow )
      {
      (this->*fpfPrepare)( target, tow );
      }

   inline void Light::calcIntensity( Vertex const & vertex, ColorF * vi ) const
      {
      (this->*fpfCalcIntensityVertex)( vertex, vi );
      }

   inline void Light::calcIntensityWorld( Vertex const & vertex, 
      ColorF * vi ) const
      {
      (this->*fpfCalcIntensityVertexWorld)( vertex, vi );
      }

   inline void Light::calcIntensity( Point3F const & loc, ColorF * vi ) const
      {
      (this->*fpfCalcIntensityPoint)( loc, vi );
      }

   inline void Light::calcIntensityWorld( Point3F const & loc, 
      ColorF * vi ) const
      {
      (this->*fpfCalcIntensityPointWorld)( loc, vi );
      }

   //---------------------------------------------------------------------------
   // SceneLighting methods
   //---------------------------------------------------------------------------

   inline Bool SceneLighting::isLocked() const
      {
      return fLocked;
      }

   //---------------------------------------------------------------------------
   
   inline void SceneLighting::setAmbientIntensity( 
      ColorF const & intensity )
      { 
      fAmbientIntensity = intensity; 
      }

   inline ColorF const & SceneLighting::getAmbientIntensity() const
      {
      return fAmbientIntensity;
      }

   //---------------------------------------------------------------------------
}; // namespace TS


//
typedef TS::Light TSLight;
typedef TS::SceneLighting TSSceneLighting;


#endif
