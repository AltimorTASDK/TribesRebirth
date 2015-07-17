//---------------------------------------------------------------------------
//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
//      $Version: $
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#include <g_surfac.h>

#include "ts_RenderContext.h"
#include "ts_PointArray.h"
#include "ts_light.h"

namespace TS
{

RenderContext::RenderContext()
{
   fLocked   = false;
   fpSurface = 0;
   fpCamera  = 0;
   fpLights  = 0;
   fpPointArray   = NULL;
   fOwnPointArray = false;
}

RenderContext::RenderContext(GFXSurface*    pSurface, 
                             Camera*        pCamera,
                             SceneLighting* pLights )
{
   AssertFatal( pSurface, "TS::RenderContext::RenderContext: invalid surface" );
   AssertFatal( pCamera,  "TS::RenderContext::RenderContext: invalid camera" );
   AssertFatal( pLights,  "TS::RenderContext::RenderContext: invalid lights" );

   fpSurface = pSurface;
   fpCamera  = pCamera;
   fpLights  = pLights;
   fpPointArray   = new PointArray( this );
	fOwnPointArray = true;
   fLocked        = false;
}

RenderContext::~RenderContext()
{
   AssertFatal(!isLocked(), "TS::RenderContext::~RenderContext: must unlock first" );
	if (fOwnPointArray)
      delete fpPointArray;
}

void
RenderContext::lock(const bool in_doubleLock)
{
   AssertFatal(!isLocked(), "TS::RenderContext::lock: already locked" );
   AssertFatal(fpSurface,   "TS::RenderContext::lock: invalid surface" );
   AssertFatal(fpCamera,    "TS::RenderContext::lock: invalid camera" );
   AssertFatal(fpLights,    "TS::RenderContext::lock: invalid lights" );

   GFXLockMode lockMode = GFX_NORMALLOCK;
   if (in_doubleLock == true) {
      AssertFatal(fpSurface->supportsDoubleLock() == true,
                  "Tried to double lock non-doublable surface...");
      lockMode = GFX_DOUBLELOCK;
   }

	fpPointArray->setRenderContext(this);
   fpSurface->lock(lockMode);
   fpCamera->lock();
   fpLights->lock();
   fLocked = true;
}

void
RenderContext::unlock()
{
   AssertFatal(isLocked(), "TS::RenderContext::unlock: not locked" );
   AssertFatal(fpSurface,  "TS::RenderContext::unlock: invalid surface" );
   AssertFatal(fpCamera,   "TS::RenderContext::unlock: invalid camera" );
   AssertFatal(fpLights,   "TS::RenderContext::unlock: invalid lights" );

   fpLights->unlock();
   fpCamera->unlock();
   fpSurface->unlock();
   fLocked = false;
}

void
RenderContext::setPointArray( PointArray* pArray )
{
	if (fOwnPointArray)
		delete fpPointArray;
	fpPointArray   = pArray;
	fOwnPointArray = false;
}

}; // namespace TS

