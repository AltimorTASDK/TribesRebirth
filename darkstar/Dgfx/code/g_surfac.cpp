//================================================================
//	
// Initial Author: Rick Overman 
//	
// Description 
//	
// $Workfile:   g_surfac.cpp  $
// $Revision:   1.2  $
// $Author  $
// $Modtime $
//
//================================================================

#include "d_defs.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "g_pal.h"

//static GFXSurface members
GFXRasterList GFXSurface::rasterList = {0,}; 	//rasterization buffer

GFXRasterList::~GFXRasterList()
{
   if (list != NULL)
      delete [] list;
   list = NULL;
}

GFXSurface::GFXSurface()
{
	clipRect(0, 0, 1, 1);
	lastRect(0, 0, 1, 1);
	flags         = GFX_DMF_RCLIP;
	pPalette      = NULL;
	pSurfaceBits  = NULL;
	surfaceStride = 0;	
	surfaceWidth  = 0;
	surfaceHeight = 0;
	next     	  = this; 	//used for page flipping
	pages			  = 1;
   popCount      = 0;
   lockCount     = 0;
   dcLockCount   = 0;
   gamma         = 1.0f;
   hDC           = NULL;
   m_draw3DBegun = false;
}

void GFXSurface::setSurfaceAlphaBlend(ColorF *color, float alpha)
{
   color;
   alpha;
   // do nothing by default
}

GFXSurface::~GFXSurface()
{

}

DWORD GFXSurface::getCaps()
{
   return 0;
}

void 
GFXSurface::update(const RectI* /*rect*/)
{
   flip();
}

void 
GFXSurface::easyFlip(void)
{
   AssertFatal(0, "GFXSurface::easyFlip: not a valid operation on this type of surface.");
}

GFXSurface* GFXSurface::findPrimary()
{
	GFXSurface *surface = this;
	for ( int i=0; i<pages; i++)
	{
		if ( surface->isPrimary ) 
			return ( surface );
		surface = surface->next;
	}
	return ( NULL );
}


Bool GFXSurface::createRasterList(Int32 lEntries)
{
   AssertFatal( lEntries > 0, 
      "GFXSurface::createRasterList: Invalid number of entries.");
   rasterList.list = new GFXRaster[lEntries];
	AssertFatal(rasterList.list, 
      "GFXSurface::createRasterList: list allocation failed.");
   if (rasterList.list != NULL)
   {
      rasterList.top_y           = 0;
      rasterList.topRaster       = NULL;
      rasterList.height          = 0;
      rasterList.freeRaster      = rasterList.list;
      rasterList.maxRasters      = lEntries;
      rasterList.unusedRasters   = lEntries;
#ifdef DEBUG
      //initially mark all rasters unused during use this will get cleared
      //on exit check and report the number of unused rasters
      for (Int32 ndx=0; ndx<rasterList.maxRasters; ndx++)
         rasterList.list[ndx].f = RASTER_UNUSED;
#endif
   	return TRUE;
   }
   return FALSE;
}


void GFXSurface::disposeRasterList()
{
	if ( rasterList.list )
	{
#ifdef DEBUG
   	Int32 unused = 0;
   	for ( Int32 ndx=0; ndx<rasterList.maxRasters; ndx++ )
   	   if ( rasterList.list[ndx].f & RASTER_UNUSED ) 
   	      unused++;
   	AssertWarn(unused != 0, avar("ALL %d rasters were used!  Possible memory overflow.", rasterList.maxRasters));
   	AssertMessage(unused != 0, "Increase the size of the raster table");

   	AssertMessage(0, avar("%d of %d Rasters Used", rasterList.maxRasters-unused, rasterList.maxRasters));
#endif          
		delete [] rasterList.list;
      rasterList.list = NULL;
	}
}

void GFXSurface::drawSurfaceToBitmap(GFXBitmap *bmp)
{
   if(bmp->getWidth() != surfaceWidth || bmp->getHeight() != surfaceHeight || bmp->bitDepth != 24 || !pPalette)
      return;
   lock();
   int i, j;
   for(j = 0; j < surfaceHeight; j++)
   {
      BYTE *dptr = bmp->getAddress(0,j);
      BYTE *sptr = getAddress(0,j);
      for(i = 0; i < surfaceWidth; i++)
      {
         PALETTEENTRY *pe = pPalette->palette[0].color + *sptr++;
         *dptr++ = pe->peBlue;
         *dptr++ = pe->peGreen;
         *dptr++ = pe->peRed;
      }
   }
   unlock();
}


void 
GFXSurface::getBitmap(GFXBitmap *bmp)
{
   AssertFatal(isLocked(), "GFXSurface::getBitmap: surface must be locked.");
   AssertFatal(bmp, "GFXSurface::getBitmap: bitmap is NULL");
   
   bmp->width    = getWidth();
   bmp->height   = getHeight();
   bmp->stride   = getStride();
   bmp->pBits    = getAddress();
   bmp->bitDepth = getBitDepth();
}

bool
GFXSurface::supportsDoubleLock() const
{
   return false;
}
