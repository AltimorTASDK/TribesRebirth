//========================================================================
//   
// $Workfile:$
// $Version$
// $Revision:$
//   
// DESCRIPTION:   2D Line
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
void __cdecl GFXasmLine2D(GFXRaster *, Point2I *, Point2I *);

void __cdecl gfx_draw_fill_raster(BYTE          *in_surface,
                                  Int32          in_width,
                                  GFXRasterList *in_rast);
void __cdecl gfx_draw_translucent_raster(BYTE          *in_surface,
                                         Int32          in_width,                 
                                         GFXRasterList *in_rast);
};


//================================================================
//   
// NAME  GFXDrawLine2d
//   
// ARGUMENTS 
//       io_pSurface    Graphics Surface
//       in_st          Line starting point
//       in_en          Line ending point
//       in_color       Color to draw it in (when given)
//
// RETURNS 
//       Nothing
//   
// NOTES 
//       X1,Y1 are contained in in_st
//       X2,Y2 are contained in in_en
//       These points may represent any coordinates.  The
//       GFXasmLine2D() routine handles lines in all directions.
//       
//================================================================

void GFXDrawLine2d(
   GFXSurface        *io_pSurface,
   const Point2I      *in_st,
   const Point2I      *in_en,
   DWORD             in_color )
{
	if(!(io_pSurface->getRasterList()->list))
		GFXSurface::createRasterList(3000);

   Point2I   		pt1,pt2;
	RectI				*lastRect;
	GFXRasterList  *rastList;
   Bool           drawTrans = FALSE;
   
   pt1 = *in_st;
   pt2 = *in_en;

   // RECTANGLE CLIP
   if (io_pSurface->getFlag( GFX_DMF_RCLIP ))
   {
      if (!rectClip( &pt1, &pt2, io_pSurface->getClipRect() )) // clip this line
         return;                                         // exit if line is gone
   }

   // store dimensions of this primitive
	lastRect = io_pSurface->getLastRect();
   lastRect->upperL.x = min(pt1.x,pt2.x);
   lastRect->upperL.y = min(pt1.y,pt2.y);
   lastRect->lowerR.x = max(pt1.x,pt2.x);
   lastRect->lowerR.y = max(pt1.y,pt2.y);


   // fill raster list with line information
	rastList = io_pSurface->getRasterList();
   GFXasmLine2D( rastList->list, &pt1, &pt2 );

   rastList->top_y     = min(pt1.y, pt2.y);
   rastList->height    = m_abs( pt1.y - pt2.y + 1L );
   rastList->topRaster = rastList->list;
   rastList->unusedRasters = rastList->maxRasters - (rastList->height + 1);
   rastList->color = in_color;
   rastList->hazeShift = 0;
   
   // render raster list
   if(drawTrans)
      gfx_draw_translucent_raster( io_pSurface->getAddress(),
                         io_pSurface->getStride(),
                         rastList );
   else   
      gfx_draw_fill_raster( io_pSurface->getAddress(),
                         io_pSurface->getStride(),
                         rastList );
}

