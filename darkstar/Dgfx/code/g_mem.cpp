//================================================================
//	
// Initial Author: Mark Frohnmayer 
//	
// Description: Surface class for memory surfaces 
//	
// $Workfile:   g_mem.cpp  $
// $Revision:   1.5  $
// $Author  $
// $Modtime $
//
//================================================================

#include "g_mem.h"
#include "g_pal.h"
#include "g_bitmap.h"
#include "fn_table.h"

GFXSurface * GFXMemSurface::create(GFXBitmap *in_bits)
{
   GFXMemSurface *ret = new GFXMemSurface;
   if(ret) 
   {
      ret->pBitmap = in_bits;
      ret->surfaceWidth = in_bits->getWidth();
      ret->surfaceHeight = in_bits->getHeight();
      ret->surfaceBitDepth = in_bits->getBitDepth();
      ret->isPrimary = FALSE;
      ret->setClipRect(&RectI(0,0,ret->surfaceWidth - 1, ret->surfaceHeight - 1));
      ret->bmpIsMine = FALSE;
      return ret;
   }
	AssertWarn(0, "GFXMemSurface::create: Failed to create a surface.");
   return NULL;
}

GFXMemSurface::GFXMemSurface()
{
	surfaceType = SURFACE_MEMORY;
   pBitmap = NULL;
   bmpIsMine = FALSE;
	functionTable = &rclip_table;
}


GFXMemSurface::~GFXMemSurface()
{
   if (bmpIsMine)
   {
      if(pBitmap)
         delete pBitmap;
   }
}

// commented out this create - can't create a bmp from within
// dgfx.dll -markf

GFXSurface * GFXMemSurface::create(Int32 lWidth, Int32 lHeight, Int32 bpp)
{
   GFXBitmap *bmp;
   bmp = GFXBitmap::createInline(lWidth, lHeight, bpp);
   if(!bmp)
   {
      AssertWarn(0, "GFXMemSurface::create: Failed to allocate bitmap.");
      return NULL;
   }
   
   GFXMemSurface *ret = (GFXMemSurface *) GFXMemSurface::create(bmp);
   
   if(!ret)
   {
      AssertWarn(0, "GFXMemSurface::create: Failed to allocate surface.");
      delete bmp;
      return NULL;
   }
   ret->bmpIsMine = TRUE;
   return ret;
}

void GFXMemSurface::_lock(const GFXLockMode /*in_lockMode*/)
{
	pSurfaceBits = pBitmap->getAddress(0,0);
	surfaceStride= pBitmap->getStride();
}


void GFXMemSurface::_unlock()
{
	pSurfaceBits = NULL;
	surfaceStride= 0;
}

void GFXMemSurface::update(const RectI *)
{
   AssertFatal(TRUE, "GFXMemSurface::update: update is not a valid operation on a memory surface.");
}

void GFXMemSurface::flip()
{
   AssertFatal(TRUE, "GFXMemSurface::flip: flip is not a valid operation on a memory surface.");
}


Bool GFXMemSurface::_setPalette(GFXPalette *in_pPal, Int32 in_start, Int32 in_count, Bool /*in_rsvPal*/)
{
   AssertFatal(in_start >= 0 && in_start < 256, 
      "GFXMemSurface::setPalette: start out of range.");
   AssertFatal(in_count > 0 && in_count <= 256, 
      "GFXMemSurface::setPalette: count out of range.");
   AssertFatal( in_start+in_count <= 256, 
      "GFXMemSurface::setPalette: start+count out of range.");

   AssertFatal(in_pPal, "GFXMemSurface::setPalette: NULL argument for in_pPal is not allowed.");
	pPalette = in_pPal;
   extern void clearMultiPalette();
   clearMultiPalette();
	return TRUE;
}

Bool GFXMemSurface::getRGBFormat(RGBFormat *)
{
   return FALSE;
}

GFXBitmap * GFXMemSurface::getSurfaceBitmap()
{
   return pBitmap;
}

void GFXMemSurface::setBitmap(GFXBitmap *bmp)
{
   if(bmpIsMine)
   {
      if(pBitmap)
         delete pBitmap;
   }
   pBitmap = bmp;
   bmpIsMine = FALSE;
}

void GFXMemSurface::drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at)
{
	GFXBitmap bmp;
	AssertFatal(!lpSourceSurface->isLocked(), "GFXCDSSurface::drawSurface: Source surface is locked.");
   lpSourceSurface->lock();
	lpSourceSurface->getBitmap(&bmp);
	Int32 flags = getFlags();
   clearFlag(GFX_DMF_ECLIP);
	drawBitmap2d( &bmp, in_subRegion, in_at );
	setFlags(flags);
   lpSourceSurface->unlock();
}
