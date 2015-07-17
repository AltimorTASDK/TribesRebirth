//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
//-------------------------------------- External Phoenix includes...
#include <ml.h>

//-------------------------------------- Non-rendition specific GFX includes
#include "g_bitmap.h"
#include "g_pal.h"
#include "d_funcs.h"
#include "fn_table.h"
#include "fn_all.h"
#include "p_edge.h"
#include "p_funcs.h"
#include "gfxMetrics.h"
#include "r_clip.h"
#include "g_contxt.h"
#include "p_tce.h"
#include "fn_table.h"
//-------------------------------------- Rendition includes
#include "gRSfc.h"
#include "gRFn.h"
#include "renditionProxy.h"


#ifdef DEBUG
#define GRINLINE
#else
#define GRINLINE inline
#endif


//-------------------------------------- Copied from p_edge.cpp
// NOTE!
// kInitialSpanTableSize must be greater than
// (kInitialSurfaceTableSize + kInitialTSurfaceTableSize) * 16

#define kInitialEdgeTableSize       1000
#define kInitialSurfaceTableSize    1000
#define kInitialTSurfaceTableSize   100
#define kInitialSpanTableSize       30000
#define kIncrementSurfaceTableSize  1000
#define kIncrementTSurfaceTableSize 100


extern "C" DWORD nextFreeBlock;

// Variable imports from p_edge.cpp
//
extern GFXPolyStore* solidSurfs;
extern GFXPolyStore* transparentSurfs;
extern GFXPolyStore* maxSolidSurf;
extern GFXPolyStore* maxTransparentSurf;

extern DWORD maxtsurfs;
extern DWORD maxsurfs;

extern span_t* spans;
extern "C" DWORD   maxspans;

extern edge_t* edges;
extern edge_t* lastedge;
extern int     maxedges;

extern int           numPolys;
extern GFXPolyStore  surfs[MAX_POLYS];
extern GFXPolyStore* pavailsurf;
extern GFXPolyStore* curSolidSurf;
extern GFXPolyStore* curTransparentSurf;

extern int xl;
extern int xr;
extern int yt;
extern int yb;
extern int height;
extern int currentcolor;

extern void GFXSortEmitPoly(GFXSurface*);
extern void GFXSortDrawRect3d(GFXSurface*, const RectI*, float, DWORD);
extern void GFXSortDrawPoint3d(GFXSurface*, const Point2I*, float, DWORD);
extern int ScanEdges(int height);
namespace Rendition {

float  sg_constantShade;
float  sg_constantAlpha = 1.0;
float  sg_constantHaze  = 0.0;
ColorF sg_fillColor;

//-------------------------------------- Bitmap drawing temps
v_xyuv sg_bitmapVerts[4];
float  sg_lscale;
float  sg_rscale;
float  sg_tscale;
float  sg_bscale;



//-------------------------------------- We intercept these calls so we can override
//                                        the setting of g_shadeScale.  We want this
//                                        to be 255*65536 always.
void 
GR_setShadeSource(GFXSurface*    io_pSurface,
                  GFXShadeSource io_src)
{
   GFXSetShadeSource(io_pSurface, io_src);
   gfxPDC.shadeScale = 255.0f * 65536.0f;
}

void 
GR_setHazeSource(GFXSurface*   io_pSurface,
                 GFXHazeSource io_src)
{
   GFXSetHazeSource(io_pSurface, io_src);
   gfxPDC.shadeScale = 255.0f * 65536.0f;
}


//------------------------------------------------------------------------------
//-------------------------------------- Non-poly primitives
//
void 
GR_clear(GFXSurface *io_pSurface,
         DWORD       in_color)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   
   v_u32 rgb;
   
   // Zero is always black, and we want to clear the surface before there
   //  is a palette installed...
   //
   if (in_color == 0) {
      rgb = 0;
   } else {
      AssertFatal(io_pSurface->getPalette() != NULL, "Error no palette!");

      const PALETTEENTRY* pColor = &io_pSurface->getPalette()->color[in_color];
      rgb = getRedlineColor(pColor->peRed, pColor->peGreen, pColor->peBlue);
   }
   
   RenditionProxy* pProxy = pSurface->getRenditionProxy();
   vl_error vlError =
      pProxy->VL_FillBuffer(pSurface->getVCmdBuffer(),
                            pSurface->getVSurface(), 1,
                            0, 0, pSurface->getWidth(), pSurface->getHeight(),
                            rgb);
   AssertWarn(vlError == VL_SUCCESS, "Error clearing screen...");
}

void 
GR_drawLine2d(GFXSurface    *io_pSurface,
              const Point2I *in_st,
              const Point2I *in_en,
              DWORD          in_color)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   AssertFatal(io_pSurface->getPalette() != NULL, "Error no palette!");
   const PALETTEENTRY* pColor = &io_pSurface->getPalette()->color[in_color];

   DWORD color = FLTOC(float(pColor->peRed)   / 255.0f,
                       float(pColor->peGreen) / 255.0f,
                       float(pColor->peBlue)  / 255.0f);
   color |= 0xff000000;
   
   v_xy lVertex1 = { ITOIFIX(in_st->x),
                     ITOIFIX(in_st->y) };
   v_xy lVertex2 = { ITOIFIX(in_en->x),
                     ITOIFIX(in_en->y) };


   pSurface->SetFGColorARGB(color);
   pSurface->SetSrcFunc(V_SRCFUNC_NOTEXTURE);
   pSurface->SetBlendEnable(false);
   
   RenditionProxy* pProxy = pSurface->getRenditionProxy();
   vl_error vlError =
      pProxy->VL_Line(pSurface->getVCmdBuffer(),
                      V_FIFO_XY,
                      reinterpret_cast<v_u32*>(&lVertex1),
                      reinterpret_cast<v_u32*>(&lVertex2));
   AssertWarn(vlError == VL_SUCCESS, "Error drawing line");
}


void 
GR_drawRect2d_f(GFXSurface*  io_pSurface,
                const RectI* in_rect,
                float        /*in_w*/,
                DWORD        in_color)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   RenditionProxy* pProxy = pSurface->getRenditionProxy();
   
   RectI src = *in_rect;
   if(!rectClip(&src, io_pSurface->getClipRect()))
      return;
   const PALETTEENTRY& pColor = io_pSurface->getPalette()->color[in_color];
   
   v_u32 pixVal = ((pColor.peRed   >> 3) << 11) |
                  ((pColor.peGreen >> 2) << 5)  |
                  ((pColor.peBlue  >> 3) << 0);
   pixVal = pixVal | (pixVal << 16);

   v_foreignAddress boardAddress =
      pProxy->V_GetBufferAddress(pSurface->getVSurface()->buffer_group, 1);
   v_u32 lineBytes = 
      pProxy->V_GetBufferLinebytes(pSurface->getVSurface()->buffer_group, 1);

   // Adjust the address
   //
   boardAddress += (src.upperL.y * lineBytes) + (src.upperL.x * 2);

   vl_error vlError =
      pProxy->VL_MemFillRect(pSurface->getVCmdBuffer(),
                             boardAddress,
                             lineBytes,
                             pixVal,
                             V_PIXFMT_565,
                             src.len_x(),
                             src.len_y());
   AssertWarn(vlError == VL_SUCCESS, "Rect failed...");
}

void 
GR_drawRect2d(GFXSurface  *io_pSurface,
              const RectI *in_rect,
              DWORD        in_color)
{
   Point2I upperR, lowerL;
   upperR.x = in_rect->lowerR.x;
   upperR.y = in_rect->upperL.y;
   lowerL.x = in_rect->upperL.x;
   lowerL.y = in_rect->lowerR.y;
   
   GR_drawLine2d(io_pSurface, &in_rect->upperL, &upperR, in_color);
   GR_drawLine2d(io_pSurface, &upperR, &in_rect->lowerR, in_color);
   GR_drawLine2d(io_pSurface, &in_rect->lowerR, &lowerL, in_color);
   GR_drawLine2d(io_pSurface, &lowerL, &in_rect->upperL, in_color);
}

void 
GR_flushTextureCache(GFXSurface* io_pSurface)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   pSurface->flushTextureCache();
   
   GFXFlushTextureCache(io_pSurface);
}


//------------------------------------------------------------------------------
//-------------------------------------- Non-forwarded renderstate management
//
void 
GR_setFillColorCF(GFXSurface*   /*io_pSurface*/,
                  const ColorF* io_pColor)
{
   sg_fillColor = *io_pColor;
}

void 
GR_setFillColorI(GFXSurface* io_pSurface,
                 Int32       in_colorIdx)
{
   AssertFatal(io_pSurface->getPalette(), "Error, no palette set...");
   
   ColorF color;
   color.red   = float(io_pSurface->getPalette()->color[in_colorIdx].peRed)   / 255.0f;
   color.green = float(io_pSurface->getPalette()->color[in_colorIdx].peGreen) / 255.0f;
   color.blue  = float(io_pSurface->getPalette()->color[in_colorIdx].peBlue)  / 255.0f;

   GR_setFillColorCF(io_pSurface, &color);
   GFXSetFillColorI(io_pSurface, in_colorIdx);
}

void 
GR_setHazeColorCF(GFXSurface*   io_pSurface,
                  const ColorF* io_pHazeColor)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   UInt8 r = UInt8(io_pHazeColor->red   * 255.0);
   UInt8 g = UInt8(io_pHazeColor->green * 255.0);
   UInt8 b = UInt8(io_pHazeColor->blue  * 255.0);

   UInt32 rgb = (r << 16) | 
                (g << 8)  |
                (b << 0);
   pSurface->SetFogColorRGB(rgb);
   GFXSetHazeColorCF(io_pSurface, io_pHazeColor);
}

void 
GR_setConstantShadeCF(GFXSurface*   /*io_pSurface*/,
                      const ColorF* cf)
{
   sg_constantShade = float(cf->red * .22 + cf->green * .67 + cf->blue * .11);
}

void 
GR_setConstantShadeF(GFXSurface* /*io_pSurface*/,
                     float       io_shade)
{
   sg_constantShade = io_shade;
}

void 
GR_setConstantHaze(GFXSurface* /*io_pSurface*/,
                   float       io_haze)
{
   sg_constantHaze = io_haze;
}

void 
GR_setConstantAlpha(GFXSurface* io_pSurface,
                    float       io_alpha)
{
   sg_constantAlpha = io_alpha;
   GFXSetConstantAlpha(io_pSurface, io_alpha);
}


//------------------------------------------------------------------------------
//-------------------------------------- Polygon primitives
//
void 
GR_attachRenderState()
{
   GFXPolyStore *polySurf;
   if(g_renderTable.isTransparent())
      polySurf = curTransparentSurf;
   else
      polySurf = curSolidSurf;

   polySurf->deviceRenderFlags = 0;
   
   // Extract state information from GFX Globals...
   //
   if (g_fillMode == GFX_FILL_CONSTANT)
      NULL;
   else
      polySurf->deviceRenderFlags |= GR_TEXTURE;
      
   if (g_shadeSource == GFX_SHADE_NONE)
      polySurf->deviceRenderFlags |= GR_SHADE_NONE;
   else if (g_shadeSource == GFX_SHADE_CONSTANT)
      polySurf->deviceRenderFlags |= GR_SHADE_CONSTANT;
   else if (g_shadeSource == GFX_SHADE_VERTEX)
      polySurf->deviceRenderFlags |= GR_SHADE_VERTEX;

   if (g_hazeSource == GFX_HAZE_NONE)
      polySurf->deviceRenderFlags |= GR_HAZE_NONE;
   else if (g_hazeSource == GFX_HAZE_CONSTANT)
      polySurf->deviceRenderFlags |= GR_HAZE_CONSTANT;
   else if (g_hazeSource == GFX_HAZE_VERTEX)
      polySurf->deviceRenderFlags |= GR_HAZE_VERTEX;

   if (g_alphaSource == GFX_ALPHA_NONE)
      polySurf->deviceRenderFlags |= GR_ALPHA_NONE;
   else if (g_alphaSource == GFX_ALPHA_CONSTANT)
      polySurf->deviceRenderFlags |= GR_ALPHA_CONSTANT;
   else if (g_alphaSource == GFX_ALPHA_VERTEX)
      polySurf->deviceRenderFlags |= GR_ALPHA_VERTEX;
   else if (g_alphaSource == GFX_ALPHA_TEXTURE)
      polySurf->deviceRenderFlags |= GR_ALPHA_TEXTURE;
      
   if (g_texturePerspective == true)
      polySurf->deviceRenderFlags |= GR_PERSPECTIVEENABLED;
   
   if (g_transparent == true)
      polySurf->deviceRenderFlags |= GR_TRANSPARENCYENABLED;
   
   // Attach color info (maybe make this conditional?)...
   //
   polySurf->deviceShade           = sg_constantShade;
   polySurf->deviceColorInfo.alpha = sg_constantAlpha;
   polySurf->deviceColorInfo.haze  = sg_constantHaze;
   polySurf->deviceColorInfo.color = sg_fillColor;
}


void 
GR_drawPoint2d(GFXSurface*    io_pSurface,
               const Point2I* in_at,
               float          /*in_w*/,
               DWORD          in_color)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   AssertFatal(io_pSurface->getPalette() != NULL, "Error no palette!");
   const PALETTEENTRY* pColor = &io_pSurface->getPalette()->color[in_color];

   DWORD color = FLTOC(float(pColor->peRed)   / 255.0f,
                       float(pColor->peGreen) / 255.0f,
                       float(pColor->peBlue)  / 255.0f);
   color |= 0xff000000;
   
   v_xy lVertex = { ITOIFIX(in_at->x),
                    ITOIFIX(in_at->y) };

   pSurface->SetFGColorARGB(color);
   pSurface->SetSrcFunc(V_SRCFUNC_NOTEXTURE);
   pSurface->SetBlendEnable(false);
   
   RenditionProxy* pProxy = pSurface->getRenditionProxy();
   vl_error vlError =
      pProxy->VL_Dot(pSurface->getVCmdBuffer(),
                     V_FIFO_XY,
                     reinterpret_cast<v_u32*>(&lVertex));
   AssertWarn(vlError == VL_SUCCESS, "Error drawing point");
}

void 
GR_drawPointSort(GFXSurface*    io_pSurface,
                 const Point2I* in_at,
                 float          in_w,
                 DWORD          in_color)
{
   GR_attachRenderState();
   GFXSortDrawPoint3d(io_pSurface, in_at, in_w, in_color);
}

void 
GR_drawRectSort_f(GFXSurface  *io_pSurface,
                  const RectI *in_rect,
                  float        in_w,
                  DWORD        in_color)
{
   GR_attachRenderState();
   GFXSortDrawRect3d(io_pSurface, in_rect, in_w, in_color);
}

void 
GR_emitPoly(GFXSurface* io_pSurface)
{
   GR_attachRenderState();
   GFXSortEmitPoly(io_pSurface);
}


//-------------------------------------- Span blitting functions, located in
//                                        gRDSpans.cpp
//
extern void GR_BlitSpansC(Surface*, GFXPolyStore*);
extern void GR_BlitSpansS(Surface*, GFXPolyStore*);
extern void GR_BlitSpansH(Surface*, GFXPolyStore*);
extern void GR_BlitSpansUV(Surface*, GFXPolyStore*);
extern void GR_BlitSpansUVS(Surface*, GFXPolyStore*);
extern void GR_BlitSpansUVH(Surface*, GFXPolyStore*);
extern void GR_BlitSpansUVQ(Surface*, GFXPolyStore*);
extern void GR_BlitSpansUVQS(Surface*, GFXPolyStore*);
extern void GR_BlitSpansUVQH(Surface*, GFXPolyStore*);


void 
GR_dispatchSpans(Surface*      io_pSurface,
                 GFXPolyStore* io_pSf)
{
   if ((io_pSf->deviceRenderFlags & GR_TEXTURE) != 0) {
      // Drawing a textured poly
      //
      UInt32 shadeFlags = io_pSf->deviceRenderFlags & GR_SHADEMASK;
      UInt32 hazeFlags  = io_pSf->deviceRenderFlags & GR_HAZEMASK;
      bool perspective  = (io_pSf->deviceRenderFlags & GR_PERSPECTIVEMASK) == 0 ? false : true;

      if ((shadeFlags == GR_SHADE_NONE || shadeFlags == GR_SHADE_CONSTANT) &&
          (hazeFlags  == GR_HAZE_NONE  || hazeFlags  == GR_HAZE_CONSTANT)) {
         if (perspective == true)
            GR_BlitSpansUVQ(io_pSurface, io_pSf);
         else
            GR_BlitSpansUV(io_pSurface, io_pSf);
      } else if (shadeFlags == GR_SHADE_VERTEX) {
         AssertFatal(hazeFlags != GR_HAZE_VERTEX,
                     "Shade/Haze interp not allowed");
         if (perspective == true)
             GR_BlitSpansUVQS(io_pSurface, io_pSf);
         else
            GR_BlitSpansUVS(io_pSurface, io_pSf);
      } else if (hazeFlags == GR_HAZE_VERTEX) {
         AssertFatal(shadeFlags != GR_SHADE_VERTEX,
                     "Shade/Haze interp not allowed");
         if (perspective == true) {
            GR_BlitSpansUVQH(io_pSurface, io_pSf);
         }
         else
            GR_BlitSpansUVH(io_pSurface, io_pSf);
      }
   } else {
      // Drawing a solid fill poly, what type?
      //
      UInt32 shadeFlags = io_pSf->deviceRenderFlags & GR_SHADEMASK;
      UInt32 hazeFlags  = io_pSf->deviceRenderFlags & GR_HAZEMASK;
      
      if ((shadeFlags == GR_SHADE_NONE || shadeFlags == GR_SHADE_CONSTANT) &&
          (hazeFlags  == GR_HAZE_NONE  || hazeFlags  == GR_HAZE_CONSTANT)) {
         GR_BlitSpansC(io_pSurface, io_pSf);
      } else if (shadeFlags == GR_SHADE_VERTEX) {
         AssertFatal(hazeFlags != GR_HAZE_VERTEX,
                     "Shade/Haze interp not allowed");
         GR_BlitSpansS(io_pSurface, io_pSf);
      } else if (hazeFlags == GR_HAZE_VERTEX) {
         AssertFatal(shadeFlags != GR_SHADE_VERTEX,
                     "Shade/Haze interp not allowed");
         GR_BlitSpansH(io_pSurface, io_pSf);
      }
   }
}

void 
GR_DrawSpans(GFXSurface* io_pSurface)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   GFXPolyStore *sf = solidSurfs;
   while(sf < curSolidSurf) {
      if(sf->spanList == sf->curblock) {
         sf++;
         continue;
      }
      if(sf->tce) {
         if ((sf->tce->flags & GFXTextureCacheEntry::needsRecache) != 0 ||
             pSurface->setTexture(sf->tce) == false) {
            gfxTextureCache.prepareForDraw(sf->tce, io_pSurface->getPalette());
            pSurface->downloadTexture(sf->tce);
         }
      } else {
         if (pSurface->setTexture(sf->textureMap) == false) {
            pSurface->downloadTexture(sf->textureMap,
                                      sf->textureMapStride,
                                      sf->textureMapHeight);
         }
      }
      pSurface->primeForDraw();
      GFXMetrics.incRenderedPolys();
      GR_dispatchSpans(pSurface, sf);
      sf++;
   }

   sf = transparentSurfs;
   while(sf < curTransparentSurf) {
      if(sf->spanList == sf->curblock) {
         sf++;
         continue;
      }

      bool textureAlpha =
         (sf->deviceRenderFlags & GR_ALPHA_TEXTURE) ? true : false;
      bool textureTransparent =
         (sf->deviceRenderFlags & GR_TRANSPARENCYENABLED) ? true : false;
      if(sf->tce) {
         if (pSurface->setTexture(sf->tce) == false) {
            gfxTextureCache.prepareForDraw(sf->tce, io_pSurface->getPalette());
            pSurface->downloadTexture(sf->tce,
                                      textureAlpha,
                                      textureTransparent);
         }
      } else {
         if (pSurface->setTexture(sf->textureMap) == false) {
            pSurface->downloadTexture(sf->textureMap,
                                      sf->textureMapStride,
                                      sf->textureMapHeight,
                                      textureAlpha,
                                      textureTransparent);
         }
      }
      pSurface->primeForDraw();
      GFXMetrics.incRenderedPolys();
      GR_dispatchSpans(pSurface, sf);
      sf++;
   }
}

void 
GR_draw3DBegin(GFXSurface* io_pSurface)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   if (pSurface->m_draw3DBegun++ != 0)
      return;
      
   GFXDraw3DBegin(io_pSurface);
   io_pSurface->lock();
}

void 
GR_draw3DEnd(GFXSurface* io_pSurface)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   if (--pSurface->m_draw3DBegun != 0)
      return;
      
   spanListPtr = spans;

   GFXMetrics.numSpansUsed = gfx_scan_edges(height);
   if(nextFreeBlock >= maxspans) {
      maxspans += kInitialSpanTableSize;
      spans = (span_t*)realloc(spans, sizeof(span_t) * (maxspans + 32));
   }

   GR_DrawSpans(io_pSurface);
   io_pSurface->unlock();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-------------------------------------- BITMAPS (Stolen from glidesfc.cpp)
//--------------------------------------
//
void
bitmapDraw(GFXSurface*      io_pSfc,
           const GFXBitmap* in_pBM)
{
   Surface* pSurface = static_cast<Surface*>(io_pSfc);

   // Set up the texture...
   //
   if (pSurface->setTexture(in_pBM->pBits) == false) {
      bool alpha  = (in_pBM->attribute & BMA_TRANSLUCENT) ? true : false;
      bool transp = (in_pBM->attribute & BMA_TRANSPARENT) ? true : false;

      pSurface->downloadTexture(in_pBM->pBits,
                                in_pBM->getStride(),
                                in_pBM->getHeight(),
                                alpha, transp);
   }
   
   // Prime the texture... (Note, since Glide doesn't handle translucency,
   //  for consistency, we won't either.  this may have to change...)
   //
   if ((in_pBM->attribute & BMA_TRANSPARENT) != 0) {
      pSurface->primeForBitmap(true, false);
   } else {
      pSurface->primeForBitmap(false, false);
   }

   RenditionProxy* pProxy = pSurface->getRenditionProxy();
   vl_error vlError = pProxy->VL_Trifan(pSurface->getVCmdBuffer(),
                                        V_FIFO_XYUV, 4,
                                        (v_u32*)sg_bitmapVerts);
   AssertFatal(vlError == VL_SUCCESS, "Error drawing bitmap fan...");
}

bool
setupBitmapPoints(GFXSurface *sfc,
                  int x0, int y0,
                  int x1, int y1)
{
   sg_lscale = sg_tscale = 0.0f;
   sg_rscale = sg_bscale = 1.0f;

   if(sfc->getFlags() & GFX_DMF_RCLIP) {
      RectI *cr = sfc->getClipRect();
      float le = float(cr->upperL.x);
      float te = float(cr->upperL.y);
      float re = float(cr->lowerR.x + 1);
      float be = float(cr->lowerR.y + 1);

      if(x0 >= re || (x0 + x1) <= le || y0 >= be || (y0 + y1) <= te)
         return false;

      if(x0 < le)
         sg_lscale = (le - x0) / x1;
      if(y0 < te)
         sg_tscale = (te - y0) / y1;
      if(x0 + x1 > re)
         sg_rscale = (re - x0) / x1;
      if(y0 + y1 > be)
         sg_bscale = (be - y0) / y1;
   }

   sg_bitmapVerts[0].x = FLTOIFIX(float(x0 + sg_lscale * x1));
   sg_bitmapVerts[0].y = FLTOIFIX(float(y0 + sg_tscale * y1));
   sg_bitmapVerts[1].x = FLTOIFIX(float(x0 + sg_rscale * x1));
   sg_bitmapVerts[1].y = FLTOIFIX(float(y0 + sg_tscale * y1));
   sg_bitmapVerts[2].x = FLTOIFIX(float(x0 + sg_rscale * x1));
   sg_bitmapVerts[2].y = FLTOIFIX(float(y0 + sg_bscale * y1));
   sg_bitmapVerts[3].x = FLTOIFIX(float(x0 + sg_lscale * x1));
   sg_bitmapVerts[3].y = FLTOIFIX(float(y0 + sg_bscale * y1));

   return true;
}

void
setupBitmapTexCoords(float x0, float y0,
                     float x1, float y1)
{
   sg_bitmapVerts[0].u = FLTOIFIX(float(x0 + (x1 - x0) * sg_lscale));
   sg_bitmapVerts[0].v = FLTOIFIX(float(y0 + (y1 - y0) * sg_tscale));
   sg_bitmapVerts[1].u = FLTOIFIX(float(x0 + (x1 - x0) * sg_rscale));
   sg_bitmapVerts[1].v = FLTOIFIX(float(y0 + (y1 - y0) * sg_tscale));
   sg_bitmapVerts[2].u = FLTOIFIX(float(x0 + (x1 - x0) * sg_rscale));
   sg_bitmapVerts[2].v = FLTOIFIX(float(y0 + (y1 - y0) * sg_bscale));
   sg_bitmapVerts[3].u = FLTOIFIX(float(x0 + (x1 - x0) * sg_lscale));
   sg_bitmapVerts[3].v = FLTOIFIX(float(y0 + (y1 - y0) * sg_bscale));
}

void 
GR_drawBitmap2d_f(GFXSurface*      io_pSurface,
                  const GFXBitmap* in_pBM,
                  const Point2I*   in_at,
                  GFXFlipFlag      in_flip)
{
   if(!setupBitmapPoints(io_pSurface,
                         in_at->x, in_at->y,
                         in_pBM->getWidth(), in_pBM->getHeight())) {
      return;
   }

   UInt32 stride = in_pBM->getStride();
   float hScale  = float(in_pBM->getWidth())/float(stride);

   float halfPixel = (1.0f/float(in_pBM->getWidth())) * hScale / 2.0f;

   if (in_flip != GFX_FLIP_NONE) {
      float coords[4];
      coords[0] = halfPixel;
      coords[1] = halfPixel;
      coords[2] = hScale - halfPixel;
      coords[3] = 1.0f - halfPixel;
      
      float temp;
      if ((in_flip & GFX_FLIP_X) != 0) {
         temp = coords[2];
         coords[2] = coords[0];
         coords[0] = temp;
      }
      if ((in_flip & GFX_FLIP_Y) != 0) {
         temp = coords[1];
         coords[1] = coords[3];
         coords[3] = temp;
      }
      setupBitmapTexCoords(coords[0], coords[1], coords[2], coords[3]);
   } else {
      setupBitmapTexCoords(halfPixel, halfPixel, hScale - halfPixel, 1.0f - halfPixel);
   }

   bitmapDraw(io_pSurface, in_pBM);
}

void 
GR_drawBitmap2d_rf(GFXSurface*      io_pSurface,
                   const GFXBitmap* in_pBM,
                   const RectI*     in_subRegion,
                   const Point2I*   in_at,
                   GFXFlipFlag      in_flip)
{
   if(!setupBitmapPoints(io_pSurface,
                         in_at->x, in_at->y,
                         in_subRegion->len_x() + 1,
                         in_subRegion->len_y() + 1)) {
      return;
   }
   
   UInt32 stride = in_pBM->getStride();
   float hScale  = 1.0f/float(stride - 1);
   float vScale  = 1.0f/float(in_pBM->getHeight() - 1);
   
   if (in_flip != GFX_FLIP_NONE) {
      float coords[4];
      coords[0] = float(in_subRegion->upperL.x) * hScale;
      coords[1] = float(in_subRegion->upperL.y) * vScale;
      coords[2] = float(in_subRegion->lowerR.x + 1) * hScale;
      coords[3] = float(in_subRegion->lowerR.y + 1) * vScale;
      
      float temp;
      if ((in_flip & GFX_FLIP_X) != 0) {
         temp = coords[2];
         coords[2] = coords[0];
         coords[0] = temp;
      }
      if ((in_flip & GFX_FLIP_Y) != 0) {
         temp = coords[1];
         coords[1] = coords[3];
         coords[3] = temp;
      }
      setupBitmapTexCoords(coords[0], coords[1], coords[2], coords[3]);
   } else {
      setupBitmapTexCoords(float(in_subRegion->upperL.x) * hScale,
                           float(in_subRegion->upperL.y) * vScale,
                           float(in_subRegion->lowerR.x + 1) * hScale,
                           float(in_subRegion->lowerR.y + 1) * vScale);
   }

   bitmapDraw(io_pSurface, in_pBM);
}

void 
GR_drawBitmap2d_sf(GFXSurface*      io_pSurface,
                   const GFXBitmap* in_pBM,
                   const Point2I*   in_at,
                   const Point2I*   in_stretch,
                   GFXFlipFlag      in_flip)
{
   if(!setupBitmapPoints(io_pSurface,
                         in_at->x,      in_at->y,
                         in_stretch->x, in_stretch->y)) {
      return;
   }

   UInt32 stride = in_pBM->getStride();
   float hScale  = float(in_pBM->getWidth())/float(stride);

   if (in_flip != GFX_FLIP_NONE) {
      float coords[4];
      coords[0] = 0;
      coords[1] = 0;
      coords[2] = hScale;
      coords[3] = 1.0f;
      
      float temp;
      if ((in_flip & GFX_FLIP_X) != 0) {
         temp = coords[2];
         coords[2] = coords[0];
         coords[0] = temp;
      }
      if ((in_flip & GFX_FLIP_Y) != 0) {
         temp = coords[1];
         coords[1] = coords[3];
         coords[3] = temp;
      }
      setupBitmapTexCoords(coords[0], coords[1], coords[2], coords[3]);
   } else {
      setupBitmapTexCoords(0, 0, hScale, 1.0f);
   }

   bitmapDraw(io_pSurface, in_pBM);
}

void 
GR_drawBitmap2d_af(GFXSurface*      /*io_pSurface*/,
                   const GFXBitmap* /*in_pBM*/,
                   const Point2I*   /*in_at*/,
                   const Point2I*   /*in_stretch*/,
                   Int32            /*rotate*/)
{
   // DMMTOIMP
   AssertFatal(0, "NI");
}


void 
GR_setZTest(GFXSurface* io_pSurface,
            int         enable)
{
   if (enable == GFX_ZTEST  ||
       enable == GFX_ZWRITE ||
       enable == GFX_ZTEST_AND_WRITE ||
       enable == GFX_ZALWAYSBEHIND) {
      rendition_table.fnDrawPoint  = GR_drawPointSort;
      rendition_table.fnDrawRect_f = GR_drawRectSort_f;
   } else if(enable == GFX_NO_ZTEST) {
      rendition_table.fnDrawPoint  = GR_drawPoint2d;
      rendition_table.fnDrawRect_f = GR_drawRect2d_f;
   }

   GFXSetZTest(io_pSurface, enable);
}


void 
GR_flushTexture(GFXSurface*      io_pSurface,
                const GFXBitmap* in_pBitmap,
                const bool       in_reload)
{
   io_pSurface->flushTexture(in_pBitmap, in_reload);
}

//------------------------------------------------------------------------------
#define UNIMP_FUNCTIONS 1
//--------------------------------------
void 
GR_drawCircle2d(GFXSurface*  /*io_pSurface*/,
                const RectI* /*in_rect*/,
                DWORD        /*in_color*/)
{
   // DMMTOIMP
   AssertFatal(0, "NI");
}

void 
GR_drawCircle2d_f(GFXSurface*  /*io_pSurface*/,
                  const RectI* /*in_rect*/,
                  DWORD        /*in_color*/)
{
   // DMMTOIMP
   AssertFatal(0, "NI");
}

}; // namespace Rendition

using namespace Rendition;


//------------------------------------------------------------------------------
#define FN_TABLE 1
//-------------------------------------- RENDITION FUNCTION TABLE
//
FunctionTable rendition_table = {
   GR_clear,

   GR_drawPoint2d,
   GR_drawLine2d,

   GR_drawCircle2d,
   GR_drawCircle2d_f,
   
   GR_drawRect2d,
   GR_drawRect2d_f,

   GR_drawBitmap2d_f,
   GR_drawBitmap2d_rf,
   GR_drawBitmap2d_sf,
   GR_drawBitmap2d_af,

   GFXDrawText_p,
   GFXDrawText_r,

   GR_draw3DBegin,
   GR_draw3DEnd,

   // New function interface: NOTE: For rendition, most of these we will simply
   //  implement in terms of the GFX interface, or intercept and forward the
   //  message...
   //
   GFXAddVertexV,
   GFXAddVertexVT,
   GFXAddVertexVC,
   GFXAddVertexVTC,
   GR_emitPoly,
   
   GR_setShadeSource,
   GR_setHazeSource,
   GFXSetAlphaSource,
   GR_setFillColorCF,
   GR_setFillColorI,
   GR_setHazeColorCF,
   GFXSetHazeColorI,
   GR_setConstantShadeCF,
   GR_setConstantShadeF,
   GR_setConstantHaze,
   GR_setConstantAlpha,
   GFXSetTransparency,
   GFXSetTextureMap,
   GFXSetFillMode,
   GFXSetColorModel,
   GFXSetTexturePerspective,
   
   GFXRegisterTexture,
   GFXRegisterTextureCB,
   GFXSetTextureHandle,
   GFXAllocateLightMap,
   GFXHandleGetLightMap,
   GFXHandleSetLightMap,

   GFXHandleSetTextureMap,
   GFXHandleSetMipLevel,
   GFXHandleSetTextureSize,
   GR_flushTextureCache,
   
   GR_setZTest,
   GFXClearZBuffer,
   GFXSetTextureWrap,
   
   GR_flushTexture
};
