//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_RENDERCONTEXT_H_
#define _TS_RENDERCONTEXT_H_

#include <base.h>

class GFXSurface;

namespace TS
{
   //---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class Camera;
class SceneLighting;
class PointArray;

class RenderContext
{
  private:
   GFXSurface *      fpSurface;
   Camera *          fpCamera;
   SceneLighting *   fpLights;
   PointArray *      fpPointArray;
   bool              fOwnPointArray;
   bool              fLocked;

  public:
   RenderContext( );
   RenderContext(GFXSurface*    pSurface,
                 Camera*        pCamera,
                 SceneLighting* pLights );
   ~RenderContext();

   void lock(const bool in_doubleLock = false);
   void unlock();
   bool isLocked() const;

   void setPointArray( PointArray* pArray );
   void setSurface( GFXSurface *pSurface );
   void setCamera( Camera *pCamera );
   void setLights( SceneLighting *pLights );
   

   GFXSurface*    getSurface() const;
   Camera*        getCamera() const;
   SceneLighting* getLights() const;
   PointArray*    getPointArray();
   float getDetailScale();

   operator GFXSurface*()    const;
   operator Camera*()        const;
   operator SceneLighting*() const;
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

//---------------------------------------------------------------------------

inline bool
RenderContext::isLocked() const
{
   return fLocked;
}

//---------------------------------------------------------------------------

inline void
RenderContext::setSurface( GFXSurface *pSurface )
{
   AssertFatal(pSurface, "TS::RenderContext::setSurface: invalid surface" );
   fpSurface = pSurface;
}

//---------------------------------------------------------------------------

inline void
RenderContext::setCamera( Camera *pCamera )
{
   AssertFatal(pCamera, "TS::RenderContext::setCamera: invalid camera" );
   fpCamera = pCamera;
}

//---------------------------------------------------------------------------

inline void
RenderContext::setLights( SceneLighting *pLights )
{
   AssertFatal(pLights, "TS::RenderContext::setLights: invalid lights" );
   fpLights = pLights;
}

//---------------------------------------------------------------------------

inline GFXSurface*
RenderContext::getSurface() const
{
   return fpSurface;
}

//---------------------------------------------------------------------------

inline Camera*
RenderContext::getCamera() const
{
   return fpCamera;
}

//---------------------------------------------------------------------------

inline SceneLighting*
RenderContext::getLights() const
{
   return fpLights;
}

//---------------------------------------------------------------------------

inline PointArray*
RenderContext::getPointArray()
{
   return fpPointArray;
}

//---------------------------------------------------------------------------

inline
RenderContext::operator GFXSurface*() const
{
   return getSurface();
}

//---------------------------------------------------------------------------

inline
RenderContext::operator Camera*() const
{
   return getCamera();
}

//---------------------------------------------------------------------------

inline
RenderContext::operator SceneLighting*() const
{
   return getLights();
}

   //---------------------------------------------------------------------------
}; // namespace TS

//
typedef TS::RenderContext TSRenderContext;


#endif
