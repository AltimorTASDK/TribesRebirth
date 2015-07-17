//================================================================
//   
// $Workfile:   b_draw.cpp  $
// $Version$
// $Revision:   1.4  $
//   
// DESCRIPTION:   Bitmap Glue Code
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//   
//================================================================

#include "g_surfac.h"
#include "g_pal.h"
#include "g_bitmap.h"
#include "g_raster.h"
#include "r_clip.h"
#include "fn_all.h"

extern "C"
{

   void __cdecl gfx_draw_bm(BYTE *pDst, BYTE *pScr, UInt32 srcW, UInt32 srcH, Int32 srcNextScan, Int32 dstNextScan, GFXFlipFlag in_flip);
   void __cdecl gfx_draw_bm_s(BYTE *pDst, BYTE *pScr, UInt32 srcW, UInt32 srcH, Int32 srcNextScan, Int32 dstNextScan, GFXFlipFlag in_flip, UInt8* shadeTable);
   void __cdecl gfx_draw_transparent_bm(BYTE *pDst, BYTE *pScr, UInt32 srcW, UInt32 srcH, Int32 srcNextScan, Int32 dstNextScan, GFXFlipFlag in_flip);
   void __cdecl gfx_draw_transparent_bm_s(BYTE *pDst, BYTE *pScr, UInt32 srcW, UInt32 srcH, Int32 srcNextScan, Int32 dstNextScan, GFXFlipFlag in_flip, UInt8* shadeTable);
   void __cdecl gfx_draw_translucent_bm(BYTE *pDst, BYTE *pScr, UInt32 srcW, UInt32 srcH, Int32 srcNextScan, Int32 dstNextScan, GFXFlipFlag in_flip, BYTE *in_table );

   void __cdecl gfx_draw_stretch_bm(BYTE *pSrc, Int32 srcNextScan, UInt32 srcW, UInt32 srcH, UInt32 OsrcW, UInt32 OsrcH, 
                          BYTE* pDst, UInt32 dstNextScan, UInt32 dstW, UInt32 dstH, UInt32 OdstW, UInt32 OdstH, DWORD xAdj, DWORD yAdj, GFXFlipFlag FlipX);
   void __cdecl gfx_draw_stretch_bm_s(BYTE *pSrc, Int32 srcNextScan, UInt32 srcW, UInt32 srcH, UInt32 OsrcW, UInt32 OsrcH, 
                          BYTE* pDst, UInt32 dstNextScan, UInt32 dstW, UInt32 dstH, UInt32 OdstW, UInt32 OdstH, DWORD xAdj, DWORD yAdj, GFXFlipFlag FlipX, UInt8* shadeMap);
   void __cdecl gfx_draw_stretch_transparent_bm(BYTE *pSrc, Int32 srcNextScan, UInt32 srcW, UInt32 srcH, UInt32 OsrcW, UInt32 OsrcH, 
                           BYTE* pDst, UInt32 dstNextScan, UInt32 dstW, UInt32 dstH, UInt32 OdstW, UInt32 OdstH, DWORD xAdj, DWORD yAdj, GFXFlipFlag FlipX);
   void __cdecl gfx_draw_stretch_transparent_bm_s(BYTE *pSrc, Int32 srcNextScan, UInt32 srcW, UInt32 srcH, UInt32 OsrcW, UInt32 OsrcH, 
                           BYTE* pDst, UInt32 dstNextScan, UInt32 dstW, UInt32 dstH, UInt32 OdstW, UInt32 OdstH, DWORD xAdj, DWORD yAdj, GFXFlipFlag FlipX, UInt8* shadeMap);
   void __cdecl gfx_draw_stretch_translucent_bm(BYTE *pSrc, Int32 srcNextScan, UInt32 srcW, UInt32 srcH, UInt32 OsrcW, UInt32 OsrcH, 
                            BYTE* pDst, UInt32 dstNextScan, UInt32 dstW, UInt32 dstH, UInt32 OdstW, UInt32 OdstH, DWORD xAdj, DWORD yAdj, GFXFlipFlag FlipX, BYTE *in_table);

//   void __cdecl gfx_draw_bm_16(BYTE *pDst, BYTE *pScr, UInt32 srcW, UInt32 srcH, Int32 srcNextScan, Int32 dstNextScan, GFXFlipFlag in_flip, Int32 *in_table);
//   void __cdecl gfx_draw_transparent_bm_16(BYTE *pDst, BYTE *pScr, UInt32 srcW, UInt32 srcH, Int32 srcNextScan, Int32 dstNextScan, GFXFlipFlag in_flip, Int32 *in_table);
//   void  __cdecl gfx_raster_bm( GFXRasterList *io_raster, Int32 in_rows, Int32 in_x, Int32 in_cnt, Int32 in_u, Int32 in_v, Int32 in_flip );
//
//   void  __cdecl gfx_render_bm( GFXRasterList *in_rast, BYTE *in_surface, DWORD in_stride, BYTE *in_bits, DWORD in_bits_stride );
//   void  __cdecl gfx_render_transparent_bm( GFXRasterList *in_rast, BYTE *in_surface, DWORD in_stride, BYTE *in_bits, DWORD in_bits_stride );
//   void  __cdecl gfx_render_translucent_bm( GFXRasterList *in_rast, BYTE *in_dst, DWORD in_dst_stride, BYTE *in_src, DWORD in_src_stride, BYTE *in_table );
//
//   void  __cdecl gfx_render_stretch_bm( GFXRasterList *in_rast,BYTE *in_surface,DWORD in_stride,BYTE *in_bits,DWORD in_bits_stride,
//                                Int32 in_srcW,Int32 in_dstW, Int32 in_srcH,Int32 in_dstH, Int32 in_flipX );
//   void  __cdecl gfx_render_stretch_transparent_bm( GFXRasterList *in_rast,BYTE *in_surface,DWORD in_stride,BYTE *in_bits,DWORD in_bits_stride,
//                                Int32 in_srcW,Int32 in_dstW, Int32 in_srcH,Int32 in_dstH, Int32 in_flipX );
//   void  __cdecl gfx_render_stretch_translucent_bm( GFXRasterList *in_rast,BYTE *in_surface,DWORD in_stride,BYTE *in_bits,DWORD in_bits_stride,
//                                Int32 in_srcW,Int32 in_dstW, Int32 in_srcH,Int32 in_dstH, Int32 in_flipX, BYTE *in_table );
}   

//================================================================
//   
// NAME  GFXDrawBitmap2d_f
//   
// DESCRIPTION    Non-Edge clipped bitmap without stretch  
//   
// ARGUMENTS 
//       io_pSurface    Graphics Surface
//       in_pBM         Pointer to Bitmap
//       in_at          Pointer to top left corner where bitmap will be placed
//       in_flip        flip flag
//
// RETURNS 
//       Nothing
//   
// NOTES
//       Tranlucent bitmaps must have a pointer to a palette with a
//       translucency table and a translucency level index.
//       Translucency level of 0 is treated as a solid bitmap
//   
//       If Rect clipping is on, both source and dest regions are clipped
//   
//================================================================

// This variation treates the entire bitmap as a sub-region and
// therefore copies the entire bitmap to the desired location.

void GFXDrawBitmap2d_f(
                     GFXSurface        *io_pSurface, 
                     const GFXBitmap   *in_pBM, 
                     const Point2I      *in_at, 
                     GFXFlipFlag       in_flip)
{
   RectI r;
   in_pBM->getClientRect(&r);
   GFXDrawBitmap2d_rf(io_pSurface, in_pBM, &r, in_at, in_flip);
}    

// This variation copies a rectangular sub-region from the source 
// bitmap to a location of the same size on the destination bitmap.

void GFXDrawBitmap2d_rf(
                     GFXSurface        *io_pSurface, 
                     const GFXBitmap   *in_pBM, 
                     const RectI       *in_subRegion, 
                     const Point2I     *in_at, 
                     GFXFlipFlag       in_flip)
{
   BYTE        *pSrc;
   RectI        src;
   RectI        srcClip;
   RectI        srcOrg;
   Int32       srcW, srcH;
   Int32       srcNextScan;
	RectI        *lastRect;

   BYTE        *pDst;
   Int32       dstNextScan;
	GFXPalette  *pPalette;

	lastRect = io_pSurface->getLastRect();
   *lastRect = *in_subRegion;                 // get source region
   *lastRect -= &(in_subRegion->upperL);      // normalize to 0,0
   *lastRect += in_at;                        // add offset for dest region

   src = *in_subRegion;                               // get source region

   in_pBM->getClientRect( &srcClip );
   in_pBM->getClientRect( &srcOrg );

   // clip source and dest sub-regions
   if (io_pSurface->getFlag(GFX_DMF_RCLIP))
      if ( !rectClip(&src, &srcClip, lastRect, io_pSurface->getClipRect()) )
         return;

   srcW = src.lowerR.x - src.upperL.x + 1;
   srcH = src.lowerR.y - src.upperL.y + 1;

   // calculate scan offset to next source row based on flip-flags
   switch (in_flip)
   {
      case GFX_FLIP_NONE:
         srcNextScan  = in_pBM->getStride()-srcW;
         break;
      case GFX_FLIP_X:
         src.upperL.x =  (srcOrg.lowerR.x - srcOrg.upperL.x) - src.upperL.x;
         srcNextScan  = in_pBM->getStride()+srcW;
         break;
      case GFX_FLIP_Y:
         src.upperL.y = (srcOrg.lowerR.y - srcOrg.upperL.y) - src.upperL.y;
         srcNextScan  = -srcW-in_pBM->getStride();
         break;
      case GFX_FLIP_X|GFX_FLIP_Y:
         src.upperL.x =  (srcOrg.lowerR.x - srcOrg.upperL.x) - src.upperL.x;
         src.upperL.y = (srcOrg.lowerR.y - srcOrg.upperL.y) - src.upperL.y;
         srcNextScan  = srcW-in_pBM->getStride();
         break;
   }

   pSrc = in_pBM->getAddress(&src.upperL);

   // calculate scan offset to next dest row
   dstNextScan = io_pSurface->getStride();
   pDst = io_pSurface->getAddress(lastRect->upperL);

//   dstNextScan -= srcW;
   dstNextScan -= srcW << (io_pSurface->getBitDepth() >> 4);

   // call the appropriate assembly language draw routine
	pPalette = io_pSurface->getPalette();
   GFXPalette::MultiPalette *mpal;
   if(pPalette)
      mpal = pPalette->findMultiPalette(in_pBM->paletteIndex);
   else
      mpal = NULL;

   switch (in_pBM->attribute & BMA_DRAW_FLAGS)
   {
      case BMA_TRANSLUCENT|BMA_TRANSPARENT:
      case BMA_TRANSLUCENT:
			AssertFatal(io_pSurface->getFlag(GFX_DMF_WRITEONLY) == 0, "GFXDrawBitmap2d_rf: Cannot draw translucent bitmaps to a writeonly surface.");
         gfx_draw_translucent_bm(pDst, pSrc, srcW, srcH, srcNextScan, dstNextScan, in_flip, mpal->transMap );
			break;

      case BMA_TRANSPARENT:
         if (in_pBM->paletteIndex == DWORD(-1) || !mpal)
            gfx_draw_transparent_bm(pDst, pSrc, srcW, srcH, srcNextScan, dstNextScan, in_flip);
         else
			{
	         AssertFatal(mpal->identityMap != NULL, "This should never happen, null idMap");
            gfx_draw_transparent_bm_s(pDst, pSrc, srcW, srcH, srcNextScan, dstNextScan, in_flip, mpal->identityMap);
         }
			break;

      case BMA_NORMAL:
      default:
         if (in_pBM->paletteIndex == DWORD(-1) || !mpal)
            gfx_draw_bm(pDst, pSrc, srcW, srcH, srcNextScan, dstNextScan, in_flip);
         else
			{
         	AssertFatal(mpal->identityMap != NULL, "This should never happen, null idMap");
            gfx_draw_bm_s(pDst, pSrc, srcW, srcH, srcNextScan, dstNextScan, in_flip, mpal->identityMap);
         }
			break;
   }
}   

//================================================================
//   
// NAME  GFXDrawBitmap2d_sf
//   
// DESCRIPTION    Non-Edge clipped bitmap with stretch  
//   
// ARGUMENTS 
//       io_pSurface        Graphics Buffer
//       in_pBM         Pointer to Bitmap
//       in_at          Pointer to Desination point on surface
//       in_stretch     Pointer to W&H stretch values
//       in_flip        flip flag
//
// RETURNS 
//       Nothing
//   
// NOTES
//       Tranlucent bitmaps must have a pointer to a palette with a
//       translucency table and a translucency level index.
//       Translucency level of 0 is treated as a solid bitmap
//   
//================================================================

void GFXDrawBitmap2d_sf(
                     GFXSurface *io_pSurface,
                     const GFXBitmap *in_pBM,
                     const Point2I *in_at,
                     const Point2I *in_stretch,
                     GFXFlipFlag in_flip)
{
   //PRE clipping width and height
   Int32 OsrcW = in_pBM->getWidth();
   Int32 OsrcH = m_abs(in_pBM->getHeight());
   Int32 OdstW = in_stretch->x;
   Int32 OdstH = in_stretch->y;
   Int32 xAdj = 0;
   Int32 yAdj = 0;
   BYTE *pSrc;
   BYTE *pDst;
   RectI src;
   RectI srcOrg;
	RectI *lastRect;
   Int32 delta;
	GFXPalette  *pPalette;
   Bool FlipX=(in_flip & GFX_FLIP_X != 0);


	lastRect = io_pSurface->getLastRect();
   lastRect->upperL = *in_at;
   lastRect->lowerR = *in_at;

   lastRect->lowerR += *in_stretch;
   lastRect->lowerR -= 1;

   in_pBM->getClientRect(&src);
   in_pBM->getClientRect(&srcOrg);

   // clip source and dest sub-regions
   if (io_pSurface->getFlag( GFX_DMF_RCLIP ))
      if (!rectClipScaled(&src, lastRect, io_pSurface->getClipRect(), &xAdj, &yAdj, in_flip))
         return;

   // calculate scan offset to next source row based on flip-flags
   if (in_flip & GFX_FLIP_Y)
   {
      delta = src.lowerR.y - srcOrg.lowerR.y + src.upperL.y - srcOrg.upperL.y;
      src.lowerR.y -= delta;
      src.upperL.y -= delta;
   }
   if (in_flip & GFX_FLIP_X)
   {
      delta = src.lowerR.x - srcOrg.lowerR.x + src.upperL.x - srcOrg.upperL.x;
      src.lowerR.x -= delta;
      src.upperL.x -= delta;
   }

   //POST clipping width and height
   Int32 srcW = src.lowerR.x - src.upperL.x + 1;
   Int32 srcH = src.lowerR.y - src.upperL.y + 1;
   Int32 dstW = lastRect->lowerR.x - lastRect->upperL.x + 1;
   Int32 dstH = lastRect->lowerR.y - lastRect->upperL.y + 1;

   // calculate scan offset to next dest row
   Int32 dstNextScan  = io_pSurface->getStride();
   if (!FlipX)
   {
      dstNextScan -= dstW;
      pDst = io_pSurface->getAddress(lastRect->upperL);
   }
   else
   {
      dstNextScan += dstW;
      pDst = io_pSurface->getAddress(lastRect->lowerR.x, lastRect->upperL.y);
   }

   // calculate scan offset to next source row based on flip-flags
   Int32 srcNextScan  = ((in_flip & GFX_FLIP_Y)? -1:1) * in_pBM->getStride();      // - srcW;
   if (in_flip & GFX_FLIP_Y)  
      pSrc = in_pBM->getAddress(src.upperL.x,src.lowerR.y);
   else
      pSrc = in_pBM->getAddress(&src.upperL);

   // call the appropriate assembly language draw routine
	pPalette = io_pSurface->getPalette();
   GFXPalette::MultiPalette *mpal;
   if(pPalette)
      mpal = pPalette->findMultiPalette(in_pBM->paletteIndex);
   else
      mpal = NULL;
   
   switch (in_pBM->attribute & BMA_DRAW_FLAGS)
   {
      case BMA_TRANSLUCENT|BMA_TRANSPARENT:
      case BMA_TRANSLUCENT:
			AssertFatal(io_pSurface->getFlag(GFX_DMF_WRITEONLY) == 0, "GFXDrawBitmap2d_sf: Cannot draw translucent bitmaps to a writeonly surface.");
         gfx_draw_stretch_translucent_bm(pSrc, srcNextScan, srcW, srcH, OsrcW, OsrcH, pDst, dstNextScan, dstW, dstH, OdstW, OdstH, xAdj, yAdj, FlipX, mpal->transMap );
         break;

//      case BMA_FUZZY|BMA_TRANSPARENT:
//      case BMA_FUZZY|BMA_TRANSPARENT|BMA_TRANSLUCENT:
//         gfx_draw_fuzzy_translucent_bm(pSrc, srcNextScan, srcW, srcH, OsrcW, OsrcH, pDst, dstNextScan, dstW, dstH, OdstW, OdstH, xAdj, yAdj, FlipX);
//         break;
//      case BMA_FUZZY:
//      case BMA_FUZZY|BMA_TRANSLUCENT:
//         gfx_draw_fuzzy_transparent_bm(pSrc, srcNextScan, srcW, srcH, OsrcW, OsrcH, pDst, dstNextScan, dstW, dstH, OdstW, OdstH, xAdj, yAdj, FlipX);
//         break;

      case BMA_TRANSPARENT:
         if (in_pBM->paletteIndex == DWORD(-1) || !mpal)
            gfx_draw_stretch_transparent_bm(pSrc, srcNextScan, srcW, srcH, OsrcW, OsrcH, pDst, dstNextScan, dstW, dstH, OdstW, OdstH, xAdj, yAdj, FlipX);
         else
			{
         	AssertFatal(mpal->identityMap != NULL, "Should never have null id map...");
            gfx_draw_stretch_transparent_bm_s(pSrc, srcNextScan, srcW, srcH, OsrcW, OsrcH, pDst, dstNextScan, dstW, dstH, OdstW, OdstH, xAdj, yAdj, FlipX, mpal->identityMap);
         }
			break;

      case BMA_NORMAL:
      default:
         if (in_pBM->paletteIndex == DWORD(-1) || !mpal)
            gfx_draw_stretch_bm(pSrc, srcNextScan, srcW, srcH, OsrcW, OsrcH, pDst, dstNextScan, dstW, dstH, OdstW, OdstH, xAdj, yAdj, FlipX);
         else
			{
         	AssertFatal(mpal->identityMap != NULL, "Should never have null id map...");
            gfx_draw_stretch_bm_s(pSrc, srcNextScan, srcW, srcH, OsrcW, OsrcH, pDst, dstNextScan, dstW, dstH, OdstW, OdstH, xAdj, yAdj, FlipX, mpal->identityMap);
         }
			break;
   }
}

//================================================================
//   
// NAME  GFXDrawBitmap2d_af
//   
// DESCRIPTION    Non-Edge clipped rotated bitmap
//   
// ARGUMENTS 
//       io_pSurface        Graphics Buffer
//       in_pBM         Pointer to Bitmap
//       in_at          Pointer to Desination point on surface
//       in_stretch     Pointer to W&H stretch values
//       in_angle       angle to rotate 0-65536
//
// RETURNS 
//       Nothing
//   
// NOTES
//       Tranlucent bitmaps must have a pointer to a palette with a
//       translucency table and a translucency level index.
//       Translucency level of 0 is treated as a solid bitmap
//   
//================================================================

//void GFXDrawBitmap2d_af(
//                     GFXSurface *io_pSurface,
//                     const GFXBitmap *in_pBM,
//                     const Point2I *in_at,
//                     const Point2I *in_stretch,
//                     Int32 in_angle)
void GFXDrawBitmap2d_af(
                     GFXSurface *,
                     const GFXBitmap *,
                     const Point2I *,
                     const Point2I *,
                     Int32)
{

}
