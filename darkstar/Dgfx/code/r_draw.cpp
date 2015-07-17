//================================================================
//
// $Workfile:   r_draw.cpp  $
// $Version$
// $Revision:   1.9  $
//
// DESCRIPTION:   Rectangle drawing routines
//
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//
//================================================================

#include "d_defs.h"
#include "g_surfac.h"
#include "g_pal.h"
#include "r_clip.h"
#include "fn_all.h"
#include "g_contxt.h"

//================================================================
//
// NAME
//   GFXDrawRect2d
//
// DESCRIPTION     Draw a rectangle into a raster surface,
//
// ARGUMENTS
//       io_pSurface     Graphics surface
//       in_rect     Rectangle to draw
//       in_color    Color to draw it in
//
// RETURNS
//
// NOTES
//
//================================================================

void GFXDrawRect2d(
   GFXSurface     *io_pSurface,
   const RectI     *in_rect,
   DWORD          in_color )
{
   if (  (in_rect->upperL.y == in_rect->lowerR.y)
      || (in_rect->upperL.x == in_rect->lowerR.x) )
      GFXDrawLine2d(io_pSurface, &in_rect->upperL, &in_rect->lowerR, in_color);
   else
   {
      Point2I a;

      a(in_rect->lowerR.x, in_rect->upperL.y);
      GFXDrawLine2d(io_pSurface, &in_rect->upperL, &a, in_color);       //top
      GFXDrawLine2d(io_pSurface, &a, &in_rect->lowerR, in_color);       //right

      a(in_rect->upperL.x, in_rect->lowerR.y);
      GFXDrawLine2d(io_pSurface, &in_rect->upperL, &a, in_color);       //left
      GFXDrawLine2d(io_pSurface, &a, &in_rect->lowerR, in_color);       //bottom
   }
}

//================================================================
//
// NAME
//   GFXDrawRect2d_f
//
// DESCRIPTION     Draw a solid color rectangle
//
// ARGUMENTS
//       io_pSurface Graphics surface
//       in_rect     Rectangle to draw
//       in_color    Color to draw it in
//
// RETURNS
//
// NOTES
//       If tranlucent level is not set to 0 in surfaces palette,
//       rectangle will be drawn with translucency.  Because of
//       the GFXSurface API, this is currently the only way to
//       indicate that you want translucency without passing flags.
   //
//================================================================

extern "C" void __cdecl GFXRenderSpan();
extern "C" void __cdecl gfx_draw_translucent_span();
extern "C" void __cdecl RasterizeRect(int x, int cnt);

void GFXDrawRect2d_f(GFXSurface *io_pSurface, const RectI *in_rect, float w, DWORD in_color)
{
   w;    // unused parameter
        RectI src = *in_rect;
	if(!rectClip(&src, io_pSurface->getClipRect()))
		return;
	gfxPDC.color = in_color;
	GFXRenderSpan();

	gfxPDC.currentY = src.upperL.y;
	gfxPDC.destY = src.lowerR.y + 1;
	gfxPDC.rowAddr = io_pSurface->getAddress();
	gfxPDC.rowStride = io_pSurface->getStride();
	RasterizeRect(src.upperL.x, src.lowerR.x + 1 - src.upperL.x);
}

