//================================================================
//	
// Initial Author: Mark Frohnmayer 
//	
// Description: Surface class for subsurfaces 
//	
// $Workfile:   g_sub.cpp  $
// $Revision:   1.3  $
// $Author  $
// $Modtime $
//
//================================================================

#include "g_sub.h"
#include "g_pal.h"


GFXSubSurface::GFXSubSurface()
{
	surfaceType = SURFACE_SUBSURFACE;
   pParent = NULL;
}


Bool GFXSubSurface::create(GFXSurface *&pSurface, GFXSurface *parent, const RectI *in_subRegion)
{
   GFXSubSurface *lpSurface = new GFXSubSurface;
   if(!lpSurface)
	{
	   AssertWarn(0, "GFXSubSurface::create: Failed to create a surface.");
	   pSurface = NULL;
   	return FALSE;
   }
   lpSurface->surfaceBitDepth = parent->getBitDepth();
   lpSurface->surfaceWidth = in_subRegion->lowerR.x - in_subRegion->upperL.x;
   lpSurface->surfaceHeight = in_subRegion->lowerR.y - in_subRegion->upperL.y;
   lpSurface->x0 = in_subRegion->upperL.x;
   lpSurface->y0 = in_subRegion->upperL.y;
      
   lpSurface->isPrimary = FALSE;
	lpSurface->setClipRect( &RectI(0, 0, lpSurface->surfaceWidth - 1, lpSurface->surfaceHeight - 1) );
   lpSurface->pParent = parent;
   pSurface = lpSurface;
	lpSurface->functionTable = parent->getFunctionTable();
   return TRUE;
}

Bool GFXSubSurface::getRGBFormat(RGBFormat *rgb)
{
   if(pParent)
      return getRGBFormat(rgb);
   else
      return FALSE;
}

void GFXSubSurface::setParent(GFXSurface *parent)
{
   AssertFatal(!isLocked(), "GFXSubSurface::setParent: cannot set the parent of a locked subSurface.");
   pParent = parent;
   if(pParent)
	{
      functionTable = pParent->getFunctionTable();
      surfaceBitDepth = pParent->getBitDepth();
	}
}

void GFXSubSurface::setRegion(const RectI *in_subRegion)
{
   AssertFatal(!isLocked(), "GFXSubSurface::setRegion: cannot set the region of a locked subSurface.");
   x0 = in_subRegion->upperL.x;
   y0 = in_subRegion->upperL.y;
   surfaceWidth = in_subRegion->lowerR.x - x0;
   surfaceHeight = in_subRegion->lowerR.y - y0;
}

void GFXSubSurface::_lock(const GFXLockMode in_lockMode)
{
   AssertFatal(in_lockMode == GFX_NORMALLOCK,
               "Double locking not supported on Sub Surface");

   AssertFatal(pParent, "GFXSubSurface::lock: cannot lock a subsurface with no parent.");
	pParent->lock();
	pSurfaceBits = pParent->getAddress(x0, y0);
	surfaceStride= pParent->getStride();
}


void GFXSubSurface::_unlock()
{
	pSurfaceBits = NULL;
	surfaceStride= 0;
   pParent->unlock();
}

void GFXSubSurface::update(const RectI *)
{
   AssertFatal(TRUE, "GFXSubSurface::update: update is not a valid operation on a subsurface.");
}

void GFXSubSurface::flip()
{
   AssertFatal(TRUE, "GFXSubSurface::flip: flip is not a valid operation on a subsurface.");
}

//void GFXSubSurface::_setPalette(GFXPalette *in_pPal, Int32 in_start, Int32 in_count, Bool in_rsvPal)
Bool GFXSubSurface::_setPalette(GFXPalette *in_pPal, Int32 in_start, Int32 in_count, bool )
{
   AssertFatal(in_start >= 0 && in_start < 256, 
      "GFXSubSurface::setPalette: start out of range.");
   AssertFatal(in_count > 0 && in_count <= 256, 
      "GFXSubSurface::setPalette: count out of range.");
   AssertFatal( in_start+in_count <= 256, 
      "GFXSubSurface::setPalette: start+count out of range.");

   AssertFatal(in_pPal, "GFXSubSurface::setPalette: NULL argument for in_pPal is not allowed.");
	pPalette = in_pPal;
   extern void clearMultiPalette();
   clearMultiPalette();

	return TRUE;
}

void GFXSubSurface::drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at)
{
   AssertFatal(pParent, "GFXSubSurface::drawSurface: cannot drawSurface with a NULL parent.");
   Point2I at(in_at->x + x0, in_at->y + y0);   
   pParent->drawSurface(lpSourceSurface, in_subRegion, &at);   
}
