//================================================================
//
// $Workfile:   g_clear.cpp  $
// $Version$
// $Revision:   1.1  $
//
// DESCRIPTION:
//
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include "g_surfac.h"
#include "fn_all.h"
#include "g_contxt.h"


extern "C"
{
   void __cdecl _gfxClearScreen( BYTE *in_pDst, DWORD in_rows, DWORD in_width, DWORD in_dstNextScan, DWORD in_color);
}
extern "C" void __cdecl gfx_draw_fill_rgb_span();
extern "C" void __cdecl gfx_no_edge_clip();
extern "C" void __cdecl TRastX();

void GFXClearScreen16(GFXSurface *io_pSurface, DWORD in_color)
{
   io_pSurface;   // unused parameter
   in_color;      // unused parameter
/*	GFXPalette *pal = io_pSurface->getPalette();
	if(!pal)
		return;
	gfxRC.shadeMap = pal->trueColorTable;
	if(!gfxRC.shadeMap)
		return;
	gfxPRC.xLeft = 0;
	gfxPRC.xRight = io_pSurface->getWidth() << 16;
	gfxPRC.deltaXL = 0;
	gfxPRC.deltaXR = 0;
	gfxPRC.currentY = 0;
	gfxPRC.destY = io_pSurface->getHeight();
	gfxRC.shadeMap = pal->trueColorTable;
	gfxRC.color = in_color;
	gfxRC.rowAddr = io_pSurface->getAddress();
	gfxRC.stride = io_pSurface->getStride();

	gfx_no_edge_clip();
	gfx_draw_fill_rgb_span();
	TRastX();
	gfxRC.rasterLoop();
*/
}

void GFXClearScreen(GFXSurface *io_pSurface, DWORD in_color)
{
   Int32 stride;
   RectI r;
   if(io_pSurface->getBitDepth() == 16)
	{
		GFXClearScreen16(io_pSurface, in_color);
		return;
	}
   io_pSurface->getClientRect(&r);
   stride = (r.lowerR.x+1+3)&~3;   // round the width up... ((Right-Left+1)+3) & ~3
   if (r.upperL.x == 0 && (stride == io_pSurface->getStride()))
   {
      //if the buffer width (rounded to the nearest DWORD) is the same
      //as the draw-space then fill it as a block rather than scan by scan
      _gfxClearScreen(io_pSurface->getAddress(r.upperL),
                     1,
                     stride*(r.lowerR.y-r.upperL.y+1),
                     0,
                     in_color);
   }
   else  //fill scan by scan
   {
      Int32 width = r.lowerR.x-r.upperL.x+1;
      _gfxClearScreen(io_pSurface->getAddress(r.upperL),
                     r.lowerR.y-r.upperL.y+1,
                     width,
                     io_pSurface->getStride() - width,
                     in_color);
   }
}
