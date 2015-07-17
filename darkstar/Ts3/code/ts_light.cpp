//---------------------------------------------------------------------------
//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
//      $Version: $
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	
//---------------------------------------------------------------------------

#include "ts_light.h"
#include "ts_color.h"
#include "ts_vertex.h"

namespace TS
{


#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)

#pragma option -Jg

   template SceneLighting::iterator find(SceneLighting::iterator first, 
         SceneLighting::iterator last, const SceneLighting::value_type& x);
#endif

   //---------------------------------------------------------------------------
   
   Light::Light() : MinPointOptimizeRatio( 20.0f )
      {
      fReady = FALSE;
      setType( LightInvalid );
   	fLight.fShadowMap = 0;
      fLight.linearFalloff = true;
      fLight.fStaticLight  = false;
      }

   //---------------------------------------------------------------------------

   void Light::setFuncs()
      {
         switch (fLight.fType)
         {
            case LightDirectional:
         	   fpfPrepare                    = &Light::directionalPrepare;
               fpfCalcIntensityVertex        = &Light::directionalIntensityVertex;
               fpfCalcIntensityVertexWorld   = &Light::directionalIntensityVertexWorld;
               fpfCalcIntensityPoint         = &Light::directionalIntensityPoint;
               fpfCalcIntensityPointWorld    = &Light::directionalIntensityPointWorld;
               break;

            case LightPoint:
         	   fpfPrepare                    = &Light::pointPrepare;
               fpfCalcIntensityVertex        = &Light::pointIntensityVertex;
               fpfCalcIntensityVertexWorld   = &Light::pointIntensityVertexWorld;
               fpfCalcIntensityPoint         = &Light::pointIntensityPoint;
               fpfCalcIntensityPointWorld    = &Light::pointIntensityPointWorld;
               break;
    
            case LightDirectionalNonlinear:
         	   fpfPrepare                    = &Light::directionalPrepare;
               fpfCalcIntensityVertex        = &Light::directionalNonlinearIntensityVertex;
               fpfCalcIntensityVertexWorld   = &Light::directionalIntensityVertexWorld;
               fpfCalcIntensityPoint         = &Light::directionalIntensityPoint;
               fpfCalcIntensityPointWorld    = &Light::directionalIntensityPointWorld;
               break;
                
            case LightDirectionalWrap:
         	   fpfPrepare                    = &Light::directionalPrepare;
               fpfCalcIntensityVertex        = &Light::directionalWrapIntensityVertex;
               fpfCalcIntensityVertexWorld   = &Light::directionalIntensityVertexWorld;
               fpfCalcIntensityPoint         = &Light::directionalIntensityPoint;
               fpfCalcIntensityPointWorld    = &Light::directionalIntensityPointWorld;
               break;
                
            default:
               AssertFatal( fLight.fType == LightInvalid, 
                  "TS::Light::setFuncs: unknown light type" );
         	   fpfPrepare                    = &Light::invalidPrepare;
               fpfCalcIntensityVertex        = &Light::invalidIntensityVertex;
               fpfCalcIntensityVertexWorld   = &Light::invalidIntensityVertexWorld;
               fpfCalcIntensityPoint         = &Light::invalidIntensityPoint;
               fpfCalcIntensityPointWorld    = &Light::invalidIntensityPointWorld;
         }
      }

   //---------------------------------------------------------------------------

   #pragma argsused

   void Light::invalidPrepare( SphereF const & target,
      TMat3F const & tow )
      {
      AssertFatal( 0, "TS::Light::invalidPrepare: light type not set" );
      }

   //---------------------------------------------------------------------------

   void Light::directionalPrepare( SphereF const & target,
      TMat3F const & tow )
      {
      // calculate position and direction in object space:
      // if( !fReady  ||  1 )    // pb.. don't understand fReady...doesn't work
         {
         RMat3F two = tow;       // _just_ R mat wanted here
         two.inverse();
   	   m_mul( fLight.fWAim, two, &fLight.fDirection);
         fReady = TRUE;
         }

      if( fLight.fShadowMap )
         {
         Point3F temp;
     	   m_mul( target.center, tow, &temp );
         Point2I shadowLoc;
         shadowLoc.x = Int32((temp.x - fLight.fShadowOffset.x) * 
            fLight.fShadowScale) & fLight.fShadowMask;
         shadowLoc.y = Int32((temp.y - fLight.fShadowOffset.y) * 
            fLight.fShadowScale) & fLight.fShadowMask;
         fLight.fShadowAtten = fLight.fShadowMap
            [(shadowLoc.y << fLight.fShadowShift) + shadowLoc.x] / 255.0f;
         }
      else
         fLight.fShadowAtten = 1.0f;
      }

   //---------------------------------------------------------------------------

   void Light::pointPrepare( SphereF const & target,
      TMat3F const & tow )
      {
      AssertFatal( fLight.fType == LightPoint,
         "TS::Light::pointPrepare: wrong light type" );

      // calculate position and direction in object space:
      //    the fReady check winds up prohibiting more than one call to 
      //       prepare lights within a render, so took it out.  
      // if( !fReady )
         {
         TMat3F two = tow;
         two.inverse();
     	   m_mul( fLight.fWPosition, two, &fLight.fPosition );
         fReady = TRUE;
         }

      // determine whether we should optimize direction & distance:
      // get aim vector and distance from light to surface:
      fLight.fDirection = target.center;
      fLight.fDirection -= fLight.fPosition;
      RealF distance = fLight.fDirection.lenf();
      if( distance > fLight.fRange + target.radius )
         {
         // light is out of range:
         fLight.fOptimized = LightOutOfRange;
         }
      else if( distance > target.radius * MinPointOptimizeRatio && 
	      	distance > 1.0f)
         {
         // light is far enough to optimize:
         fLight.fDirection /= distance;
			int atten = attenuationFormula ( distance );
         fLight.fDistanceAtten = (atten != .0f)? 1.0f / atten: 0.0f;
             
         // need to check each loc or vertex passed when we're near or overlap 
         // the edge of the light circle.  
         if ( distance > fLight.fRange - 2 * target.radius )
            fLight.fOptimized = LightMostlyOptimized; 
         else
            fLight.fOptimized = LightFullyOptimized;
         }
      else
         {
         // light is too close to optimize:
         fLight.fOptimized = LightNotOptimized;
         }
      }

   //---------------------------------------------------------------------------

   #pragma argsused
  
   void Light::invalidIntensityVertex( Vertex const & vertex, 
      ColorF *vi ) const
      {
      AssertFatal( 0, "TS::Light::invalidIntensityVertex: light type not set" );
      }

   //---------------------------------------------------------------------------

   void Light::directionalIntensityVertex( Vertex const & vertex, 
      ColorF *vi ) const
      {
      AssertFatal( fReady, 
         "TS::Light::directionalIntensityVertex: must call prepare first" );

      RealF lambertAtten = - m_dot( fLight.fDirection, vertex.fNormal );

      if( lambertAtten < 0.0f )
         return;

      lambertAtten *= fLight.fShadowAtten;

      vi->red   += lambertAtten * fLight.fRed;
      vi->green += lambertAtten * fLight.fGreen;
      vi->blue  += lambertAtten * fLight.fBlue;
      }

   //---------------------------------------------------------------------------

   void Light::directionalNonlinearIntensityVertex( 
      Vertex const & vertex, ColorF *vi ) const
      {
      AssertFatal( fReady, 
         "TS::Light::directionalIntensityVertex: must call prepare first" );

      RealF lambertAtten = - m_dot( fLight.fDirection, vertex.fNormal );

      if( lambertAtten < 0.0f )
         return;

      // y = (x-1)^3 - 1 (very) roughly approximates the
      // curvature of y = x^(1/3) in [0,1] at less expense
      lambertAtten -= 1.0f;
      lambertAtten = 1.0f + lambertAtten*lambertAtten*lambertAtten;
      lambertAtten *= fLight.fShadowAtten;

      vi->red   += lambertAtten * fLight.fRed;
      vi->green += lambertAtten * fLight.fGreen;
      vi->blue  += lambertAtten * fLight.fBlue;
      }

   //---------------------------------------------------------------------------

   void Light::directionalWrapIntensityVertex( Vertex const & vertex, 
      ColorF *vi ) const
      {
      AssertFatal( fReady, 
         "TS::Light::directionalWrapIntensityVertex: must call prepare first" );

      RealF lambertAtten = - m_dot( fLight.fDirection, vertex.fNormal );

      lambertAtten = (lambertAtten+1.0f) * 0.5f;

      lambertAtten *= fLight.fShadowAtten;

      vi->red   += lambertAtten * fLight.fRed;
      vi->green += lambertAtten * fLight.fGreen;
      vi->blue  += lambertAtten * fLight.fBlue;
      }

   //---------------------------------------------------------------------------

   void Light::pointIntensityVertex( Vertex const & vertex, ColorF *vi )
      const 
      {
      AssertFatal( fReady, 
         "TS::Light::pointIntensityVertex: must call prepare first" );
      AssertFatal( fLight.fType == LightPoint,
         "TS::Light::pointIntensityVertex: wrong light type" );

      // optimization levels: 
      //    0 - out of range
      //    1 - none
      //    2 - distance and direction fixed
      
      if( fLight.fOptimized == LightOutOfRange )
         return;

      // The "mostly" case is for when we have optimized the light, but we're close
      // to the edge of the circle and it may be that points must be individually
      // checked.  
      Point3F     direction;
      RealF       distance;
      if( fLight.fOptimized == LightNotOptimized 
               || fLight.fOptimized == LightMostlyOptimized )
      {
         direction = vertex.fPoint;
         direction -= fLight.fPosition;
         if ( (distance = direction.lenf()) > fLight.fRange )
            return;
      }

      // add in distance attenuation:
      RealF lambertAtten;
      if( fLight.fOptimized == LightNotOptimized  )  {
         lambertAtten = - m_dot( direction, vertex.fNormal );
         if ( lambertAtten < 0.0f )
            return;
         
         if ( distance > 0.001f )  {
            lambertAtten /= distance;     // normalize the above dot product. 
            float formula = attenuationFormula ( distance );
            if ( formula > 1.0 )
               lambertAtten /= formula;
         }
         else
            lambertAtten = 1.0f;
      }
      else  // Light<>Optimized.  distance attenuation precalculated:
      {
         // When optimized, then this vector serves as reasonable approximate
         // to dot all normals against.  Its alrady normalized.  
         lambertAtten = - m_dot( fLight.fDirection, vertex.fNormal );
         if( lambertAtten < 0.0f )
            return;
         lambertAtten *= fLight.fDistanceAtten;
      }

      vi->red   += lambertAtten * fLight.fRed;
      vi->green += lambertAtten * fLight.fGreen;
      vi->blue  += lambertAtten * fLight.fBlue;
      }

   //---------------------------------------------------------------------------

   #pragma argsused  // loc is not used

   void Light::invalidIntensityPoint( Point3F const & loc, ColorF *vi ) 
      const
      {
      AssertFatal( 0, "TS::Light::invalidIntensityPoint: light type not set" );
      }

   //---------------------------------------------------------------------------

   #pragma argsused  // loc is not used

   void Light::directionalIntensityPoint( Point3F const &loc, 
      ColorF *vi ) const
      {
      AssertFatal( fReady, 
         "TS::Light::directionalIntensityPoint: must call prepare first" );
      vi->red   += fLight.fRed;
      vi->green += fLight.fGreen;
      vi->blue  += fLight.fBlue;
      }

   //---------------------------------------------------------------------------

   void Light::pointIntensityPoint( Point3F const & loc, ColorF *vi ) 
      const
      {
      AssertFatal( fLight.fType == LightPoint,
         "TS::Light::pointIntensityPoint: wrong light type" );
      AssertFatal( fReady, 
         "TS::Light::pointIntensityPoint: must call prepare first" );

      // optimization levels: 
      //    0 - out of range
      //    1 - none
      //    2 - distance and direction fixed
      
      if( fLight.fOptimized == LightOutOfRange )
         return;
      RealF atten = 1.0f;
      if( fLight.fOptimized == LightNotOptimized )  {
         Point3F direction = loc;
         direction -= fLight.fPosition;
         RealF distance = direction.lenf();
         // direction /= distance;
         float formula = attenuationFormula ( distance );
         if ( formula > 1.0f )
            atten = 1.0f / formula;
      }
      else  // distance attenuation precalculated:
         atten = fLight.fDistanceAtten;

      vi->red   += atten * fLight.fRed;
      vi->green += atten * fLight.fGreen;
      vi->blue  += atten * fLight.fBlue;
      }

   //---------------------------------------------------------------------------

   #pragma argsused
  
   void Light::invalidIntensityVertexWorld( Vertex const & vertex, 
      ColorF *vi ) const
      {
      AssertFatal( 0, 
         "TS::Light::invalidIntensityVertexWorld: light type not set" );
      }

   //---------------------------------------------------------------------------

   void Light::directionalIntensityVertexWorld( Vertex const & vertex, 
      ColorF *vi ) const
      {
      AssertFatal( fLight.fType == LightDirectional,
         "TS::Light::directionalIntensityVertexWorld: wrong light type" );

      RealF lambertAtten = - m_dot( fLight.fWAim, vertex.fNormal );
      if( lambertAtten < 0.0f )
         return;

      // add in shadow attenuation, if any:
      if( fLight.fShadowMap )
         {
         Int32 x, y;
         m_RealF_to_Int32((vertex.fPoint.x - fLight.fShadowOffset.x) * fLight.fShadowScale, &x);
         m_RealF_to_Int32((vertex.fPoint.y - fLight.fShadowOffset.y) * fLight.fShadowScale, &y);
         lambertAtten *= fLight.fShadowMap[
            ((y & fLight.fShadowMask) << fLight.fShadowShift) + 
            (x & fLight.fShadowMask)] / 255.0f;
         }

      vi->red   += lambertAtten * fLight.fRed;
      vi->green += lambertAtten * fLight.fGreen;
      vi->blue  += lambertAtten * fLight.fBlue;
      }

   //---------------------------------------------------------------------------

   void Light::pointIntensityVertexWorld( Vertex const & vertex, 
      ColorF *vi ) const
      {
      AssertFatal( fLight.fType == LightPoint,
         "TS::Light::pointIntensityVertexWorld: wrong light type" );

      // note: will not be optimized since prepare won't be called

      Point3F direction = vertex.fPoint;
      direction -= fLight.fWPosition;
      RealF distance = direction.lenf();
      direction /= distance;

      RealF lambertAtten = - m_dot( direction, vertex.fNormal );
      if( lambertAtten < 0.0f )
         return;

      // add in distance attenuation:
      lambertAtten *= attenuationFormula ( distance );

      vi->red   += lambertAtten * fLight.fRed;
      vi->green += lambertAtten * fLight.fGreen;
      vi->blue  += lambertAtten * fLight.fBlue;
      }

   //---------------------------------------------------------------------------

   #pragma argsused

   void Light::invalidIntensityPointWorld( Point3F const & loc, 
      ColorF *vi ) const
      {
      AssertFatal( 0, 
         "TS::Light::invalidIntensityPointWorld: light type not set" );
      }

   //---------------------------------------------------------------------------

   void Light::directionalIntensityPointWorld( Point3F const & loc, 
      ColorF *vi ) const
      {
      RealF atten;
      if( fLight.fShadowMap )
         {
         Point2I shadowLoc;
         shadowLoc.x = Int32((loc.x - fLight.fShadowOffset.x) * fLight.fShadowScale) & fLight.fShadowMask;
         shadowLoc.y = Int32((loc.y - fLight.fShadowOffset.y) * fLight.fShadowScale) & fLight.fShadowMask;
         atten = fLight.fShadowMap[(shadowLoc.y << fLight.fShadowShift) + shadowLoc.x] / 256.0f;
         }
      else
         atten = 1.0f;

      vi->red   += atten * fLight.fRed;
      vi->green += atten * fLight.fGreen;
      vi->blue  += atten * fLight.fBlue;
      }

   //---------------------------------------------------------------------------

   void Light::pointIntensityPointWorld( Point3F const & loc, 
      ColorF *vi ) const
      {
      AssertFatal( fLight.fType == LightPoint,
         "TS::Light::pointIntensityPointWorld: wrong light type" );

      // note: will not be optimized since prepare won't be called
      
      Point3F direction = loc;
      direction -= fLight.fWPosition;
      RealF distance = direction.lenf();
      direction /= distance;

		int atten = attenuationFormula ( distance );
      RealF distanceAtten = (atten != .0f)? 1.0f / atten: 0.0f;

      vi->red   += distanceAtten * fLight.fRed;
      vi->green += distanceAtten * fLight.fGreen;
      vi->blue  += distanceAtten * fLight.fBlue;
      }

   //---------------------------------------------------------------------------
   // SceneLighting methods
   //---------------------------------------------------------------------------

   SceneLighting::SceneLighting()
      {
      fAmbientIntensity.red = 0.0f;
      fAmbientIntensity.green = 0.0f;
      fAmbientIntensity.blue = 0.0f;

		fLightsOn = true;
      fLocked = FALSE;
      }

   //---------------------------------------------------------------------------

   SceneLighting::~SceneLighting()
      {
      AssertFatal( !isLocked(),
         "TS::SceneLighting::~SceneLighting: must call unlock() first" );
      }

   //---------------------------------------------------------------------------
   // lock so lights may be used for rendering 
   //    (can't install/remove while locked)

   void SceneLighting::lock()
      {
      AssertFatal( !isLocked(),
         "TS::SceneLighting::Lock: must call unlock() first" );

      for( iterator light = begin(); light != end(); light++ )
         (*light)->invalidate();

      fLocked = TRUE;
      }

   //---------------------------------------------------------------------------
   // unlock so lights may be installed and removed

   void SceneLighting::unlock()
   {
      AssertFatal( isLocked(),
         "TS::SceneLighting::Unlock: must call lock() first" );
      
      fLocked = FALSE;
   }

   //---------------------------------------------------------------------------
   // Install a light

   void SceneLighting::installLight( Light * pLight )
      {
      AssertFatal( pLight != 0,
         "TS::SceneLighting::installLight: attempt to install null pointer" );

      AssertFatal( !isLocked(),
         "TS::SceneLighting::installLight: must call unlock() first" );

      // see if the light is already activated:
   #ifndef NDEBUG
      iterator light = find( begin(), end(), pLight );
      AssertFatal( light == end(),
         "TS::LightSceneLighting::installLight: light already installed");
   #endif

      push_back( pLight );
      }

   //---------------------------------------------------------------------------
   // Remove a previously installed light

   void SceneLighting::removeLight( Light * pLight )
      {
      AssertFatal( pLight != 0,
         "TS::SceneLighting::removeLight: attempt to remove null pointer" );

      AssertFatal( !isLocked(),
         "TS::SceneLighting::removeLight: must call unlock() first" );

      iterator light = find( begin(), end(), pLight );
      AssertFatal( light != end(), 
         "TS::LightSceneLighting::removeLight: light must be installed first");

      // remove it:
      *light = last();
      decrement();
      }

   //---------------------------------------------------------------------------
   // Prepare
   //
   // transform lights into object space, and perform any optimizations
   // based on target size and location

   void SceneLighting::prepare( SphereF const & target, 
      TMat3F const & tow )
      {
      AssertFatal( isLocked(),
         "TS::SceneLighting::prepare: must call lock() first" );

		if (fLightsOn)
	      for( iterator light = begin(); light != end(); light++ )
   	      (*light)->prepare( target, tow );
      }

   //---------------------------------------------------------------------------
   // CalcIntensity
   //
   // calculate lighting intensity for a surface, returning a TSReal
   // value ranging from 0 (not lit) to 1 (maximum illumination).
   // It is possible for the lights at a surface to sum to a value
   // greater than 1, but this function will limit its return value to 1.
   // 

   //---------------------------------------------------------------------------

   void SceneLighting::calcIntensity( Vertex const & vertex, 
      ColorF * pIntensity ) const
      {
      AssertFatal( isLocked(),
         "TS::SceneLighting::calcIntensity: must call lock() first");

      ColorF vi = fAmbientIntensity;

		if (fLightsOn)
	      for( const_iterator light = begin(); light != end(); light++ )
   	      (*light)->calcIntensity( vertex, &vi );

      Color::saturate ( vi, pIntensity );
      }

   //---------------------------------------------------------------------------

   void SceneLighting::calcIntensityWorld( Vertex const & vertex, 
      ColorF * pIntensity ) const
      {
      AssertFatal( isLocked(),
         "TS::SceneLighting::calcIntensityWorld: must call lock() first");

      ColorF vi = fAmbientIntensity;

		if (fLightsOn)
	      for( const_iterator light = begin(); light != end(); light++ )
   	      (*light)->calcIntensityWorld( vertex, &vi );

      Color::saturate ( vi, pIntensity );
      }

   //---------------------------------------------------------------------------

   void SceneLighting::calcIntensity( Point3F const & loc, 
      ColorF * pIntensity ) const
      {
      AssertFatal( isLocked(),
         "TS::SceneLighting::calcIntensity: must call lock() first");

      ColorF vi = fAmbientIntensity;

		if (fLightsOn)
	      for( const_iterator light = begin(); light != end(); light++ )
   	      (*light)->calcIntensity( loc, &vi );

      Color::saturate ( vi, pIntensity );
      }

   //---------------------------------------------------------------------------

   void SceneLighting::calcIntensityWorld( Point3F const & loc, 
      ColorF * pIntensity ) const
      {
      AssertFatal( isLocked(),
         "TS::SceneLighting::calcIntensityWorld: must call lock() first");

      ColorF vi = fAmbientIntensity;

		if (fLightsOn)
	      for( const_iterator light = begin(); light != end(); light++ )
   	      (*light)->calcIntensityWorld( loc, &vi );

      Color::saturate ( vi, pIntensity );
      }


//---------------------------------------------------------------------------
};  // namespace TS
