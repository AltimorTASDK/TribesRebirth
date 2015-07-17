//========================================================================
//   
// $Workfile:$
// $Version$
// $Revision:$
//   
// DESCRIPTION:   Circle Glue Code
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//
//========================================================================

#include "g_surfac.h"
#include "g_pal.h"
#include "r_clip.h"

// Extern .asm functions...
//
extern "C"
{
void __cdecl GFXasmCircle(GFXRaster *, const RectI *);
void __cdecl GFXasmCircleFill(GFXRaster *, const RectI *);

void __cdecl gfx_draw_fill_raster(BYTE          *in_surface,
                                  Int32          in_width,
                                  GFXRasterList *in_rast);
void __cdecl gfx_draw_translucent_raster(BYTE          *in_surface,
                                         Int32          in_width,                 
                                         GFXRasterList *in_rast);
};




//================================================================
//   
// NAME  GFXDrawCircle2d
//   
// ARGUMENTS 
//       io_pSurface    Graphics Surface
//       in_rect        Bounding rectangle of circle
//       in_color       Color to draw it in (when given)
//
// RETURNS 
//       Nothing
//   
//================================================================

void GFXDrawCircle2d(
   GFXSurface        *io_pSurface,
   const RectI       *in_rect,
   DWORD             in_color )
{
	GFXRasterList *rastList;
	RectI *lastRect;
	RectI *clipRect;

	rastList = io_pSurface->getRasterList();
	lastRect = io_pSurface->getLastRect();
	clipRect = io_pSurface->getClipRect();

   // fill raster list with circle information
   GFXasmCircle( rastList->list, in_rect );

   rastList->top_y     = in_rect->upperL.y;
   rastList->height    = in_rect->lowerR.y - in_rect->upperL.y + 1;
   rastList->topRaster = rastList->list;
   rastList->unusedRasters = rastList->maxRasters - (rastList->height + 1);
   rastList->color = in_color;
   rastList->hazeShift = 0;
   
   // RECTANGLE CLIP
   if ( io_pSurface->getFlag( GFX_DMF_RCLIP ) )
   {
      if (rectClip( rastList, clipRect, lastRect) )
         return;
      lastRect->upperL.x = max( clipRect->upperL.x, in_rect->upperL.x );
      lastRect->lowerR.x = min( clipRect->lowerR.x, in_rect->lowerR.x );
   }
   else
      *lastRect = *in_rect;

   // render raster list
   gfx_draw_fill_raster( io_pSurface->getAddress(),
                         io_pSurface->getStride(),
                         rastList );
}


//================================================================
//   
// NAME  GFXDrawCircle2d_f
//   
// ARGUMENTS 
//       io_pSurface        Graphics Buffer
//       in_rect        Bounding rectangle of circle
//       in_color       Color to draw it in (when given)
//
// RETURNS 
//       Nothing
//   
//================================================================

void GFXDrawCircle2d_f(
   GFXSurface         *io_pSurface,
   const RectI        *in_rect,
   DWORD             in_color )
{
	GFXRasterList *rastList;
	RectI *lastRect;
	RectI *clipRect;
   Bool drawTrans = FALSE;

	rastList = io_pSurface->getRasterList();
	lastRect = io_pSurface->getLastRect();
	clipRect = io_pSurface->getClipRect();

   // fill raster list with circle information
   GFXasmCircleFill( rastList->list, in_rect );

   rastList->top_y     = in_rect->upperL.y;
   rastList->height    = in_rect->lowerR.y - in_rect->upperL.y;
   rastList->topRaster = rastList->list;
   rastList->unusedRasters = rastList->maxRasters - (rastList->height + 1);
   rastList->color = in_color;
   rastList->hazeShift = 0;

   // RECTANGLE CLIP
   if ( io_pSurface->getFlag( GFX_DMF_RCLIP ) )
   {
      if (rectClip( rastList, clipRect, lastRect ) )
         return;
      lastRect->upperL.x = max( clipRect->upperL.x, in_rect->upperL.x );
      lastRect->lowerR.x = min( clipRect->lowerR.x, in_rect->lowerR.x );
   }
   else
      *lastRect = *in_rect;

   // render raster list
   if(drawTrans)
   {
      gfx_draw_translucent_raster( io_pSurface->getAddress(),
                         io_pSurface->getStride(),
                         rastList);
   }
   else
   {
      gfx_draw_fill_raster( io_pSurface->getAddress(),
                         io_pSurface->getStride(),
                         rastList);
   }
}


