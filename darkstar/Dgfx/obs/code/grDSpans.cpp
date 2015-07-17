//------------------------------------------------------------------------------
// Description: Spans blitters for Renditions...
//    
// $Workfile: $
// $Revision: $
// $Author:   $
// $Modtime:  $
//
//------------------------------------------------------------------------------
#include <ml.h>

//-------------------------------------- Non-rendition specific GFX includes
#include "g_bitmap.h"
#include "p_edge.h"
#include "gfxMetrics.h"
#include "g_contxt.h"
#include "p_tce.h"
//-------------------------------------- Rendition includes
#include "gRSfc.h"
#include "gRFn.h"
#include "grDSpans.h"
#include "renditionProxy.h"


namespace Rendition {

static const UInt32 sg_DSpanSetupCWords    = sizeof(GR_DSpanSetupC)    / sizeof(v_u32);
static const UInt32 sg_DSpanSetupSWords    = sizeof(GR_DSpanSetupS)    / sizeof(v_u32);
static const UInt32 sg_DSpanSetupHWords    = sizeof(GR_DSpanSetupH)    / sizeof(v_u32);
static const UInt32 sg_DSpanSetupUVWords   = sizeof(GR_DSpanSetupUV)   / sizeof(v_u32);
static const UInt32 sg_DSpanSetupUVSWords  = sizeof(GR_DSpanSetupUVS)  / sizeof(v_u32);
static const UInt32 sg_DSpanSetupUVHWords  = sizeof(GR_DSpanSetupUVH)  / sizeof(v_u32);
static const UInt32 sg_DSpanSetupUVQWords  = sizeof(GR_DSpanSetupUVQ)  / sizeof(v_u32);
static const UInt32 sg_DSpanSetupUVQSWords = sizeof(GR_DSpanSetupUVQS) / sizeof(v_u32);
static const UInt32 sg_DSpanSetupUVQHWords = sizeof(GR_DSpanSetupUVQH) / sizeof(v_u32);
static const UInt32 sg_DSpanWords          = sizeof(GR_DSpan)          / sizeof(v_u32);

GR_DSpan sg_renditionDSpans[GR_POLYMAXSPANS];

Int32
GR_CountSpans(const UInt32 in_firstSpan,
              const UInt32 in_lastSpan)
{
   AssertFatal((in_firstSpan & 0xf) == 0, "oops");

   DWORD* spanList = ((DWORD*)spanListPtr);
   UInt32 currSpan = in_firstSpan;

   UInt32 numSpans = 0;
   while (1) {
      if ((currSpan+15) >= in_lastSpan)
         break;
      
      numSpans += 15;
      currSpan = spanList[currSpan + 15];
   }
   
   numSpans += (in_lastSpan - currSpan);
   return numSpans;
}


//------------------------------------------------------------------------------
// NAME 
//    void
//    GR_CreateSpans()
//    
// DESCRIPTION 
//    Unpacks the spans from the span buffer into the GR_DSpan buffer
//    
//------------------------------------------------------------------------------
extern "C" __cdecl gr_create_spans(span_t*, const UInt32, const UInt32, GR_DSpan*);

Int32
GR_CreateSpans(const UInt32 in_firstSpan,
               const UInt32 in_lastSpan,
               GR_DSpan*    out_pSpans)
{
   gr_create_spans(spanListPtr, in_firstSpan, in_lastSpan, out_pSpans);
   return 0;
//
//   Int32 numSpans = 0;
//
//   DWORD* spanList = ((DWORD*)spanListPtr);
//   UInt32 currSpan = in_firstSpan;
//   
//   while (1) {
//      Int32 y, x;
//      DWORD packedSpan = spanList[currSpan++];
//
//      y = (packedSpan >> 22);
//      x = (packedSpan >> 11) & 0x7ff;
//      
//      out_pSpans[numSpans].xy  = (x << 16) | y;
//      out_pSpans[numSpans].cnt = packedSpan & 0x7ff;
//      numSpans++;
//      
//      if (currSpan == in_lastSpan)
//         break;
//      
//      if ((currSpan & 0xf) == 0xf)
//         currSpan = spanList[currSpan];
//   }
//
//   return numSpans;
}


//------------------------------------------------------------------------------
void 
GR_BlitSpansC(Surface*      io_pSurface,
              GFXPolyStore* io_pSf)
{
   RenditionProxy* pProxy = io_pSurface->getRenditionProxy();
   GR_DSpanSetupC spanSetup;

   UInt32 alphaMode = io_pSf->deviceRenderFlags & GR_ALPHAMASK;
   UInt32 shadeMode = io_pSf->deviceRenderFlags & GR_SHADEMASK;
   
   // Determine color and shade information...
   //
   UInt32 alphaVal  = 0xff;
   if (alphaMode == GR_ALPHA_CONSTANT) {
      alphaVal = UInt32(io_pSf->deviceColorInfo.alpha * 255.0f);
      
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else {
      io_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   }

   UInt32 finalColor = alphaVal << 24;
   if (shadeMode == GR_SHADE_CONSTANT) {
      finalColor |= (UInt32(io_pSf->deviceShade * io_pSf->deviceColorInfo.color.red   * 255.0f) << 16) |
                    (UInt32(io_pSf->deviceShade * io_pSf->deviceColorInfo.color.green * 255.0f) <<  8) |
                    (UInt32(io_pSf->deviceShade * io_pSf->deviceColorInfo.color.blue  * 255.0f) <<  0);
   } else {
      finalColor |= (UInt32(io_pSf->deviceColorInfo.color.red   * 255.0f) << 16) |
                    (UInt32(io_pSf->deviceColorInfo.color.green * 255.0f) <<  8) |
                    (UInt32(io_pSf->deviceColorInfo.color.blue  * 255.0f) <<  0);
   }
   io_pSurface->SetFGColorARGB(finalColor);
   io_pSurface->SetSrcFunc(V_SRCFUNC_NOTEXTURE);

   // Set the fog attribute
   //
   if ((io_pSf->deviceRenderFlags & GR_HAZEMASK) == GR_HAZE_CONSTANT) {
      UInt32 fog = UInt32(io_pSf->deviceColorInfo.haze * 255.0f * 65536.0f);
      io_pSurface->SetFogEnable(V_FOG_ENABLE);
      io_pSurface->SetF(fog);
   } else {
      io_pSurface->SetFogEnable(V_FOG_DISABLE);
   }

   // Create fifo buffer space for spans...
   //
   Int32 numSpans = GR_CountSpans(io_pSf->spanList,
                                  io_pSf->curblock);
   v_u32* cmdlist_ptr =
      pProxy->V_AddToCmdList(io_pSurface->getVCmdBuffer(),
                             sg_DSpanSetupCWords      +
                             sg_DSpanWords * numSpans +
                             1);         // 1 word for end-of-list
   GR_CreateSpans(io_pSf->spanList,
                  io_pSf->curblock,
                  (GR_DSpan*)(cmdlist_ptr + sg_DSpanSetupCWords));

   // Copy Span setup...
   memcpy(cmdlist_ptr, &spanSetup, sizeof(GR_DSpanSetupC));
   cmdlist_ptr += sg_DSpanSetupCWords;
   cmdlist_ptr += numSpans * sg_DSpanWords;
   
   // End of spans...
   *cmdlist_ptr = 0x80000000;
}
              

void 
GR_BlitSpansS(Surface*      io_pSurface,
              GFXPolyStore* io_pSf)
{
   RenditionProxy* pProxy = io_pSurface->getRenditionProxy();
   GR_DSpanSetupS spanSetup;
   
   float alphaVal   = 1.0f;
   UInt32 alphaMode = io_pSf->deviceRenderFlags & GR_ALPHAMASK;
   if (alphaMode == GR_ALPHA_CONSTANT) {
      alphaVal = io_pSf->deviceColorInfo.alpha;
      
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else {
      io_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   }
   io_pSurface->SetSrcFunc(V_SRCFUNC_NOTEXTURE);
   
   // For now, there is no vertex alpha
   //
   spanSetup.a0   = Int32(alphaVal * 255.0f * 65536.0f);
   spanSetup.dadx = 0;
   spanSetup.dady = 0;
   
   // Determine the r0, g0, b0 and partials for each.  We set the shadeScale in
   //  gfxPDC such that these are just straight conversions...
   //
   spanSetup.r0   = Int32(io_pSf->deviceColorInfo.color.red * io_pSf->s0);
   spanSetup.drdx = Int32(io_pSf->deviceColorInfo.color.red * io_pSf->dsdx);
   spanSetup.drdy = Int32(io_pSf->deviceColorInfo.color.red * io_pSf->dsdy);
   spanSetup.g0   = Int32(io_pSf->deviceColorInfo.color.green * io_pSf->s0);
   spanSetup.dgdx = Int32(io_pSf->deviceColorInfo.color.green * io_pSf->dsdx);
   spanSetup.dgdy = Int32(io_pSf->deviceColorInfo.color.green * io_pSf->dsdy);
   spanSetup.b0   = Int32(io_pSf->deviceColorInfo.color.blue * io_pSf->s0);
   spanSetup.dbdx = Int32(io_pSf->deviceColorInfo.color.blue * io_pSf->dsdx);
   spanSetup.dbdy = Int32(io_pSf->deviceColorInfo.color.blue * io_pSf->dsdy);
   
   // Set the fog attribute
   //
   if ((io_pSf->deviceRenderFlags & GR_HAZEMASK) == GR_HAZE_CONSTANT) {
      UInt32 fog = UInt32(io_pSf->deviceColorInfo.haze * 255.0f * 65536.0f);
      io_pSurface->SetFogEnable(V_FOG_ENABLE);
      io_pSurface->SetF(fog);
   } else {
      io_pSurface->SetFogEnable(V_FOG_DISABLE);
   }

   // Create fifo buffer space for spans...
   //
   Int32 numSpans = GR_CountSpans(io_pSf->spanList,
                                  io_pSf->curblock);
   v_u32* cmdlist_ptr =
      pProxy->V_AddToCmdList(io_pSurface->getVCmdBuffer(),
                             sg_DSpanSetupSWords      +
                             sg_DSpanWords * numSpans +
                             1);         // 1 word for end-of-list
   GR_CreateSpans(io_pSf->spanList,
                  io_pSf->curblock,
                  (GR_DSpan*)(cmdlist_ptr + sg_DSpanSetupSWords));

   // Copy Span setup...
   memcpy(cmdlist_ptr, &spanSetup, sizeof(GR_DSpanSetupS));
   cmdlist_ptr += sg_DSpanSetupSWords;
   cmdlist_ptr += numSpans * sg_DSpanWords;
   
   // End of spans...
   *cmdlist_ptr = 0x80000000;
}
              

void 
GR_BlitSpansH(Surface*      io_pSurface,
              GFXPolyStore* io_pSf)
{
   RenditionProxy* pProxy = io_pSurface->getRenditionProxy();
   GR_DSpanSetupH spanSetup;

   UInt32 alphaMode = io_pSf->deviceRenderFlags & GR_ALPHAMASK;
   UInt32 shadeMode = io_pSf->deviceRenderFlags & GR_SHADEMASK;

   // Set the color and shade information
   //
   UInt32 alphaVal  = 0xff;
   if (alphaMode == GR_ALPHA_CONSTANT) {
      alphaVal = Int32(io_pSf->deviceColorInfo.alpha * 255.0f);
      
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else {
      io_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   }

   UInt32 finalColor = alphaVal << 24;
   if (shadeMode == GR_SHADE_CONSTANT) {
      finalColor |= (Int32(io_pSf->deviceShade * io_pSf->deviceColorInfo.color.red   * 255.0f) << 16) |
                    (Int32(io_pSf->deviceShade * io_pSf->deviceColorInfo.color.green * 255.0f) <<  8) |
                    (Int32(io_pSf->deviceShade * io_pSf->deviceColorInfo.color.blue  * 255.0f) <<  0);
   } else {
      finalColor |= (Int32(io_pSf->deviceColorInfo.color.red   * 255.0f) << 16) |
                    (Int32(io_pSf->deviceColorInfo.color.green * 255.0f) <<  8) |
                    (Int32(io_pSf->deviceColorInfo.color.blue  * 255.0f) <<  0);
   }
   io_pSurface->SetFGColorARGB(finalColor);
   io_pSurface->SetSrcFunc(V_SRCFUNC_NOTEXTURE);
   io_pSurface->SetFogEnable(V_FOG_ENABLE);
   
   // Determine h0, dhdx, dhdy
   //
   spanSetup.h0   = Int32((255.0f * 65536.0f) - io_pSf->s0);
   spanSetup.dhdx = Int32(-io_pSf->dsdx);
   spanSetup.dhdy = Int32(-io_pSf->dsdy);

   // Create fifo buffer space for spans...
   //
   Int32 numSpans = GR_CountSpans(io_pSf->spanList,
                                  io_pSf->curblock);
   v_u32* cmdlist_ptr =
      pProxy->V_AddToCmdList(io_pSurface->getVCmdBuffer(),
                             sg_DSpanSetupHWords      +
                             sg_DSpanWords * numSpans +
                             1);         // 1 word for end-of-list
   GR_CreateSpans(io_pSf->spanList,
                  io_pSf->curblock,
                  (GR_DSpan*)(cmdlist_ptr + sg_DSpanSetupHWords));

   // Copy Span setup...
   memcpy(cmdlist_ptr, &spanSetup, sizeof(GR_DSpanSetupH));
   cmdlist_ptr += sg_DSpanSetupHWords;
   cmdlist_ptr += numSpans * sg_DSpanWords;
   
   // End of spans...
   *cmdlist_ptr = 0x80000000;
}
              

void 
GR_BlitSpansUV(Surface*      io_pSurface,
               GFXPolyStore* io_pSf)
{
   RenditionProxy* pProxy = io_pSurface->getRenditionProxy();
   GR_DSpanSetupUV spanSetup;
   
   // Are we drawing transparent textures?
   //
   if ((io_pSf->deviceRenderFlags & GR_TRANSPARENCYMASK) == 0) {
      io_pSurface->setFilterEnable(true);
      io_pSurface->SetChromaKey(V_CHROMAKEY_DISABLE);
   } else {
      io_pSurface->setFilterEnable(false);
      io_pSurface->SetChromaKey(V_CHROMAKEY_ENABLE);
   }

   // Set up the texture coordinates...
   //
   double stride, height;
   if (io_pSf->tce != NULL) {
      stride = io_pSf->tce->csizeX;
      height = io_pSf->tce->csizeY;
   } else {
      stride = io_pSf->textureMapStride;
      height = io_pSf->textureMapHeight;
   }

   spanSetup.u0   = Int32(io_pSf->u0   * stride);
   spanSetup.dudx = Int32(io_pSf->dudx * stride);
   spanSetup.dudy = Int32(io_pSf->dudy * stride);
   spanSetup.v0   = Int32(io_pSf->v0   * height);
   spanSetup.dvdx = Int32(io_pSf->dvdx * height);
   spanSetup.dvdy = Int32(io_pSf->dvdy * height);

   if (io_pSurface->getFilterType() == V_SRCFILTER_BILINEAR) {
      spanSetup.u0 += Int32((spanSetup.dudx + spanSetup.dudy) * -0.25);
      spanSetup.v0 += Int32((spanSetup.dvdx + spanSetup.dvdy) * -0.25);
   }

   // Set the fog attribute
   //
   if ((io_pSf->deviceRenderFlags & GR_HAZEMASK) == GR_HAZE_CONSTANT) {
      UInt32 fog = UInt32(io_pSf->deviceColorInfo.haze * 255.0f * 65536.0f);
      io_pSurface->SetFogEnable(V_FOG_ENABLE);
      io_pSurface->SetF(fog);
   } else {
      io_pSurface->SetFogEnable(V_FOG_DISABLE);
   }

   // We must determine if we need to be in REPLACE or MODULATE texture mode
   //
   UInt32 alphaMode = io_pSf->deviceRenderFlags & GR_ALPHAMASK;
   UInt32 shadeMode = io_pSf->deviceRenderFlags & GR_SHADEMASK;
   v_u32 srcFunc  = V_SRCFUNC_REPLACE;

   float alphaVal = 1.0f;
   if (alphaMode == GR_ALPHA_CONSTANT) {
      srcFunc  = V_SRCFUNC_MODULATE;
      alphaVal = io_pSf->deviceColorInfo.alpha;

      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else if (alphaMode == GR_ALPHA_TEXTURE) {
      srcFunc  = V_SRCFUNC_MODULATE;
      
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else {
      io_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   }
   UInt32 finalColor = UInt32(alphaVal * 255.0f) << 24;

   if (shadeMode == GR_SHADE_CONSTANT) {
      srcFunc = V_SRCFUNC_MODULATE;
      
      finalColor |= (UInt32(io_pSf->deviceShade * 255.0f) << 16) |
                    (UInt32(io_pSf->deviceShade * 255.0f) <<  8) |
                    (UInt32(io_pSf->deviceShade * 255.0f) <<  0);
   } else {
      finalColor |= 0x00ffffff;
   }
   io_pSurface->SetFGColorARGB(finalColor);
   io_pSurface->SetSrcFunc(srcFunc);

   // Create fifo buffer space for spans...
   //
   Int32 numSpans = GR_CountSpans(io_pSf->spanList,
                                  io_pSf->curblock);

   if (alphaMode == GR_ALPHA_CONSTANT) {
      
   } else if (alphaMode == GR_ALPHA_TEXTURE) {
      pProxy->VL_SetBlendEnable(io_pSurface->getVCmdBuffer(),V_BLEND_ENABLE);
      pProxy->VL_SetBlendDstFunc(io_pSurface->getVCmdBuffer(),V_BLENDSRCALPHAINV);
      pProxy->VL_SetBlendSrcFunc(io_pSurface->getVCmdBuffer(),V_BLENDSRCALPHA);
   }

   v_u32* cmdlist_ptr =
      pProxy->V_AddToCmdList(io_pSurface->getVCmdBuffer(),
                             sg_DSpanSetupUVWords     +
                             sg_DSpanWords * numSpans +
                             1);         // 1 word for end-of-list
   GR_CreateSpans(io_pSf->spanList,
                  io_pSf->curblock,
                  (GR_DSpan*)(cmdlist_ptr + sg_DSpanSetupUVWords));

   // Copy Span setup...
   memcpy(cmdlist_ptr, &spanSetup, sizeof(GR_DSpanSetupUV));
   cmdlist_ptr += sg_DSpanSetupUVWords;
   cmdlist_ptr += numSpans * sg_DSpanWords;
   
   // End of spans...
   *cmdlist_ptr = 0x80000000;
}


void 
GR_BlitSpansUVS(Surface*      io_pSurface,
                GFXPolyStore* io_pSf)
{
   RenditionProxy* pProxy = io_pSurface->getRenditionProxy();
   GR_DSpanSetupUVS spanSetup;
   
   // Are we drawing transparent textures?
   //
   if ((io_pSf->deviceRenderFlags & GR_TRANSPARENCYMASK) == 0) {
      io_pSurface->setFilterEnable(true);
      io_pSurface->SetChromaKey(V_CHROMAKEY_DISABLE);
   } else {
      io_pSurface->setFilterEnable(false);
      io_pSurface->SetChromaKey(V_CHROMAKEY_ENABLE);
   }

   // Set up the texture coordinates...
   //
   double stride, height;
   if (io_pSf->tce != NULL) {
      stride = io_pSf->tce->csizeX;
      height = io_pSf->tce->csizeY;
   } else {
      stride = io_pSf->textureMapStride;
      height = io_pSf->textureMapHeight;
   }

   spanSetup.u0   = Int32(io_pSf->u0   * stride);
   spanSetup.dudx = Int32(io_pSf->dudx * stride);
   spanSetup.dudy = Int32(io_pSf->dudy * stride);
   spanSetup.v0   = Int32(io_pSf->v0   * height);
   spanSetup.dvdx = Int32(io_pSf->dvdx * height);
   spanSetup.dvdy = Int32(io_pSf->dvdy * height);

   if (io_pSurface->getFilterType() == V_SRCFILTER_BILINEAR) {
      spanSetup.u0 += Int32((spanSetup.dudx + spanSetup.dudy) * -0.25);
      spanSetup.v0 += Int32((spanSetup.dvdx + spanSetup.dvdy) * -0.25);
   }

   // Set the fog attribute
   //
   if ((io_pSf->deviceRenderFlags & GR_HAZEMASK) == GR_HAZE_CONSTANT) {
      UInt32 fog = UInt32(io_pSf->deviceColorInfo.haze * 255.0f * 65536.0f);
      io_pSurface->SetFogEnable(V_FOG_ENABLE);
      io_pSurface->SetF(fog);
   } else {
      io_pSurface->SetFogEnable(V_FOG_DISABLE);
   }

   // We are automatically in modulate mode if we are gouraud shading this
   //  poly...
   //
   io_pSurface->SetSrcFunc(V_SRCFUNC_MODULATE);

   // Now determine x0 for argb...
   //
   float alphaVal   = 1.0f;
   UInt32 alphaMode = io_pSf->deviceRenderFlags & GR_ALPHAMASK;
   if (alphaMode == GR_ALPHA_CONSTANT) {
      alphaVal = io_pSf->deviceColorInfo.alpha;
      
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else if (alphaMode == GR_ALPHA_TEXTURE) {
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else {
      io_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   }
   
   // For now, there is no vertex alpha
   //
   spanSetup.a0   = Int32(alphaVal * 255.0f * 65536.0f);
   spanSetup.dadx = 0;
   spanSetup.dady = 0;
   
   // Determine the r0, g0, b0 and partials for each.  We set the shadeScale in
   //  gfxPDC such that these are just straight conversions...
   //
   spanSetup.r0   = Int32(io_pSf->s0);
   spanSetup.drdx = Int32(io_pSf->dsdx);
   spanSetup.drdy = Int32(io_pSf->dsdy);

   spanSetup.g0   = Int32(io_pSf->s0);
   spanSetup.dgdx = Int32(io_pSf->dsdx);
   spanSetup.dgdy = Int32(io_pSf->dsdy);

   spanSetup.b0   = Int32(io_pSf->s0);
   spanSetup.dbdx = Int32(io_pSf->dsdx);
   spanSetup.dbdy = Int32(io_pSf->dsdy);

   // Create fifo buffer space for spans...
   //
   Int32 numSpans = GR_CountSpans(io_pSf->spanList,
                                  io_pSf->curblock);
   v_u32* cmdlist_ptr =
      pProxy->V_AddToCmdList(io_pSurface->getVCmdBuffer(),
                             sg_DSpanSetupUVSWords     +
                             sg_DSpanWords * numSpans +
                             1);         // 1 word for end-of-list
   GR_CreateSpans(io_pSf->spanList,
                  io_pSf->curblock,
                  (GR_DSpan*)(cmdlist_ptr + sg_DSpanSetupUVSWords));

   // Copy Span setup...
   memcpy(cmdlist_ptr, &spanSetup, sizeof(GR_DSpanSetupUVS));
   cmdlist_ptr += sg_DSpanSetupUVSWords;
   cmdlist_ptr += numSpans * sg_DSpanWords;
   
   // End of spans...
   *cmdlist_ptr = 0x80000000;
}


void 
GR_BlitSpansUVH(Surface*      io_pSurface,
                GFXPolyStore* io_pSf)
{
   RenditionProxy* pProxy = io_pSurface->getRenditionProxy();
   GR_DSpanSetupUVH spanSetup;
   
   UInt32 alphaMode = io_pSf->deviceRenderFlags & GR_ALPHAMASK;
   UInt32 shadeMode = io_pSf->deviceRenderFlags & GR_SHADEMASK;

   v_u32 srcFunc = V_SRCFUNC_REPLACE;

   // Are we drawing transparent textures?
   //
   if ((io_pSf->deviceRenderFlags & GR_TRANSPARENCYMASK) == 0) {
      io_pSurface->setFilterEnable(true);
      io_pSurface->SetChromaKey(V_CHROMAKEY_DISABLE);
   } else {
      io_pSurface->setFilterEnable(false);
      io_pSurface->SetChromaKey(V_CHROMAKEY_ENABLE);
   }

   // Set up the texture coordinates...
   //
   double stride, height;
   if (io_pSf->tce != NULL) {
      stride = io_pSf->tce->csizeX;
      height = io_pSf->tce->csizeY;
   } else {
      stride = io_pSf->textureMapStride;
      height = io_pSf->textureMapHeight;
   }

   spanSetup.u0   = Int32(io_pSf->u0   * stride);
   spanSetup.dudx = Int32(io_pSf->dudx * stride);
   spanSetup.dudy = Int32(io_pSf->dudy * stride);
   spanSetup.v0   = Int32(io_pSf->v0   * height);
   spanSetup.dvdx = Int32(io_pSf->dvdx * height);
   spanSetup.dvdy = Int32(io_pSf->dvdy * height);
   
   if (io_pSurface->getFilterType() == V_SRCFILTER_BILINEAR) {
      spanSetup.u0 += Int32((spanSetup.dudx + spanSetup.dudy) * -0.25);
      spanSetup.v0 += Int32((spanSetup.dvdx + spanSetup.dvdy) * -0.25);
   }

   // Determine h0, dhdx, dhdy
   //
   io_pSurface->SetFogEnable(V_FOG_ENABLE);
   spanSetup.h0   = Int32((255.0f * 65536.0f) - io_pSf->s0);
   spanSetup.dhdx = Int32(-io_pSf->dsdx);
   spanSetup.dhdy = Int32(-io_pSf->dsdy);

   // We must determine if we need to be in REPLACE or MODULATE texture mode
   //
   UInt32 finalColor;
   float alphaVal = 1.0f;
   if (alphaMode == GR_ALPHA_CONSTANT) {
      srcFunc  = V_SRCFUNC_MODULATE;
      alphaVal = io_pSf->deviceColorInfo.alpha;

      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else if (alphaMode == GR_ALPHA_TEXTURE) {
      srcFunc  = V_SRCFUNC_MODULATE;
      
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else {
      io_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   }
   finalColor = UInt32(alphaVal * 255.0f) << 24;

   if (shadeMode == GR_SHADE_CONSTANT) {
      srcFunc = V_SRCFUNC_MODULATE;
      
      finalColor |= (UInt32(io_pSf->deviceShade * 255.0f) << 16) |
                    (UInt32(io_pSf->deviceShade * 255.0f) <<  8) |
                    (UInt32(io_pSf->deviceShade * 255.0f) <<  0);
   } else {
      finalColor |= 0x00ffffff;
   }
   io_pSurface->SetFGColorARGB(finalColor);
   io_pSurface->SetSrcFunc(srcFunc);

   // Create fifo buffer space for spans...
   //
   Int32 numSpans = GR_CountSpans(io_pSf->spanList,
                                  io_pSf->curblock);
   v_u32* cmdlist_ptr =
      pProxy->V_AddToCmdList(io_pSurface->getVCmdBuffer(),
                             sg_DSpanSetupUVHWords     +
                             sg_DSpanWords * numSpans +
                             1);         // 1 word for end-of-list
   GR_CreateSpans(io_pSf->spanList,
                  io_pSf->curblock,
                  (GR_DSpan*)(cmdlist_ptr + sg_DSpanSetupUVHWords));

   // Copy Span setup...
   memcpy(cmdlist_ptr, &spanSetup, sizeof(GR_DSpanSetupUVH));
   cmdlist_ptr += sg_DSpanSetupUVHWords;
   cmdlist_ptr += numSpans * sg_DSpanWords;
   
   // End of spans...
   *cmdlist_ptr = 0x80000000;
}



void
scaleWFactors(Surface*      io_pSurface,
              GFXPolyStore* io_pSf)
{
   Int32 surfaceStride = io_pSurface->getWidth();
   Int32 surfaceHeight = io_pSurface->getHeight();
   
   int w1, w2;
   w1 = (io_pSf->dwdx < 0) ? 0 : 2;
   w2 = (io_pSf->dwdy < 0) ? 0 : 1;
   
   double wScale = 0.0;
   switch (w1 + w2) {
     case 0: // x-, y-, use w0
      wScale = io_pSf->w0;
      break;
     case 2: // x+, y-, use w in upper right corner
      wScale = io_pSf->w0 + io_pSf->dwdx * surfaceStride;
      break;
     case 3: // x+, y+, use w in lower right corner
      wScale = io_pSf->w0 + io_pSf->dwdx * surfaceStride + io_pSf->dwdy * surfaceHeight;
      break;
     case 1: // x-, y+, use w in lower left corner
      wScale = io_pSf->w0 + io_pSf->dwdy * surfaceHeight;
      break;
     default:
      AssertFatal(0, "impossible.");
   }
   wScale = 1.0/wScale;

   io_pSf->u0   *= wScale;
   io_pSf->dudx *= wScale;
   io_pSf->dudy *= wScale;
   io_pSf->v0   *= wScale;
   io_pSf->dvdx *= wScale;
   io_pSf->dvdy *= wScale;
   io_pSf->w0   *= wScale;
   io_pSf->dwdx *= wScale;
   io_pSf->dwdy *= wScale;
}


void 
GR_BlitSpansUVQ(Surface*      io_pSurface,
                GFXPolyStore* io_pSf)
{
   RenditionProxy* pProxy = io_pSurface->getRenditionProxy();
   GR_DSpanSetupUVQ spanSetup;
   
   // Are we drawing transparent textures?
   //
   if ((io_pSf->deviceRenderFlags & GR_TRANSPARENCYMASK) == 0) {
      io_pSurface->setFilterEnable(true);
      io_pSurface->SetChromaKey(V_CHROMAKEY_DISABLE);
   } else {
      io_pSurface->setFilterEnable(false);
      io_pSurface->SetChromaKey(V_CHROMAKEY_ENABLE);
   }

   // Set up the texture coordinates...
   //
   double stride, height;
   if (io_pSf->tce != NULL) {
      stride = io_pSf->tce->csizeX;
      height = io_pSf->tce->csizeY;
   } else {
      stride = io_pSf->textureMapStride;
      height = io_pSf->textureMapHeight;
   }

   scaleWFactors(io_pSurface, io_pSf);

   spanSetup.uQ0   = Int32(io_pSf->u0   * stride);
   spanSetup.duQdx = Int32(io_pSf->dudx * stride);
   spanSetup.duQdy = Int32(io_pSf->dudy * stride);
   spanSetup.vQ0   = Int32(io_pSf->v0   * height);
   spanSetup.dvQdx = Int32(io_pSf->dvdx * height);
   spanSetup.dvQdy = Int32(io_pSf->dvdy * height);
   spanSetup.Q0    = Int32(io_pSf->w0   * (double(1 << 24) - 1));
   spanSetup.dQdx  = Int32(io_pSf->dwdx * (double(1 << 24) - 1));
   spanSetup.dQdy  = Int32(io_pSf->dwdy * (double(1 << 24) - 1));

   if (io_pSurface->getFilterType() == V_SRCFILTER_BILINEAR) {
      spanSetup.uQ0 += Int32((spanSetup.duQdx + spanSetup.duQdy) * 0.5);
      spanSetup.vQ0 += Int32((spanSetup.dvQdx + spanSetup.dvQdy) * 0.5);
      spanSetup.Q0  += Int32((spanSetup.dQdx  + spanSetup.dQdy)  * 0.5);
   }

   // Set the fog attribute
   //
   if ((io_pSf->deviceRenderFlags & GR_HAZEMASK) == GR_HAZE_CONSTANT) {
      UInt32 fog = UInt32(io_pSf->deviceColorInfo.haze * 255.0f * 65536.0f);
      io_pSurface->SetFogEnable(V_FOG_ENABLE);
      io_pSurface->SetF(fog);
   } else {
      io_pSurface->SetFogEnable(V_FOG_DISABLE);
   }

   // We must determine if we need to be in REPLACE or MODULATE texture mode
   //
   UInt32 alphaMode = io_pSf->deviceRenderFlags & GR_ALPHAMASK;
   UInt32 shadeMode = io_pSf->deviceRenderFlags & GR_SHADEMASK;
   v_u32 srcFunc  = V_SRCFUNC_REPLACE;

   float alphaVal = 1.0f;
   if (alphaMode == GR_ALPHA_CONSTANT) {
      srcFunc  = V_SRCFUNC_MODULATE;
      alphaVal = io_pSf->deviceColorInfo.alpha;

      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else if (alphaMode == GR_ALPHA_TEXTURE) {
      srcFunc  = V_SRCFUNC_MODULATE;
      
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else {
      io_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   }
   UInt32 finalColor = UInt32(alphaVal * 255.0f) << 24;

   if (shadeMode == GR_SHADE_CONSTANT) {
      srcFunc = V_SRCFUNC_MODULATE;
      
      finalColor |= (UInt32(io_pSf->deviceShade * 255.0f) << 16) |
                    (UInt32(io_pSf->deviceShade * 255.0f) <<  8) |
                    (UInt32(io_pSf->deviceShade * 255.0f) <<  0);
   } else {
      finalColor |= 0x00ffffff;
   }
   io_pSurface->SetFGColorARGB(finalColor);
   io_pSurface->SetSrcFunc(srcFunc);

   // Create fifo buffer space for spans...
   //
   Int32 numSpans = GR_CountSpans(io_pSf->spanList,
                                  io_pSf->curblock);
   v_u32* cmdlist_ptr =
      pProxy->V_AddToCmdList(io_pSurface->getVCmdBuffer(),
                             sg_DSpanSetupUVQWords    +
                             sg_DSpanWords * numSpans +
                             1);         // 1 word for end-of-list
   GR_CreateSpans(io_pSf->spanList,
                  io_pSf->curblock,
                  (GR_DSpan*)(cmdlist_ptr + sg_DSpanSetupUVQWords));

   // Copy Span setup...
   memcpy(cmdlist_ptr, &spanSetup, sizeof(GR_DSpanSetupUVQ));
   cmdlist_ptr += sg_DSpanSetupUVQWords;
   cmdlist_ptr += numSpans * sg_DSpanWords;
   
   // End of spans...
   *cmdlist_ptr = 0x80000000;
}


void 
GR_BlitSpansUVQS(Surface*      io_pSurface,
                 GFXPolyStore* io_pSf)
{
   RenditionProxy* pProxy = io_pSurface->getRenditionProxy();
   GR_DSpanSetupUVQS spanSetup;
   
   // Are we drawing transparent textures?
   //
   if ((io_pSf->deviceRenderFlags & GR_TRANSPARENCYMASK) == 0) {
      io_pSurface->setFilterEnable(true);
      io_pSurface->SetChromaKey(V_CHROMAKEY_DISABLE);
   } else {
      io_pSurface->setFilterEnable(false);
      io_pSurface->SetChromaKey(V_CHROMAKEY_ENABLE);
   }

   // Set up the texture coordinates...
   //
   double stride, height;
   if (io_pSf->tce != NULL) {
      stride = io_pSf->tce->csizeX;
      height = io_pSf->tce->csizeY;
   } else {
      stride = io_pSf->textureMapStride;
      height = io_pSf->textureMapHeight;
   }

   scaleWFactors(io_pSurface, io_pSf);

   spanSetup.uQ0   = Int32(io_pSf->u0   * stride);
   spanSetup.duQdx = Int32(io_pSf->dudx * stride);
   spanSetup.duQdy = Int32(io_pSf->dudy * stride);
   spanSetup.vQ0   = Int32(io_pSf->v0   * height);
   spanSetup.dvQdx = Int32(io_pSf->dvdx * height);
   spanSetup.dvQdy = Int32(io_pSf->dvdy * height);
   spanSetup.Q0    = Int32(io_pSf->w0   * (double(1 << 24) - 1));
   spanSetup.dQdx  = Int32(io_pSf->dwdx * (double(1 << 24) - 1));
   spanSetup.dQdy  = Int32(io_pSf->dwdy * (double(1 << 24) - 1));

   if (io_pSurface->getFilterType() == V_SRCFILTER_BILINEAR) {
      spanSetup.uQ0 += Int32((spanSetup.duQdx + spanSetup.duQdy) * 0.5);
      spanSetup.vQ0 += Int32((spanSetup.dvQdx + spanSetup.dvQdy) * 0.5);
      spanSetup.Q0  += Int32((spanSetup.dQdx  + spanSetup.dQdy)  * 0.5);
   }

   // Set the fog attribute
   //
   if ((io_pSf->deviceRenderFlags & GR_HAZEMASK) == GR_HAZE_CONSTANT) {
      UInt32 fog = UInt32(io_pSf->deviceColorInfo.haze * 255.0f * 65536.0f);
      io_pSurface->SetFogEnable(V_FOG_ENABLE);
      io_pSurface->SetF(fog);
   } else {
      io_pSurface->SetFogEnable(V_FOG_DISABLE);
   }

   // We are automatically in modulate mode if we are gouraud shading this
   //  poly...
   //
   io_pSurface->SetSrcFunc(V_SRCFUNC_MODULATE);

   // Now determine x0 for argb...
   //
   float alphaVal   = 1.0f;
   UInt32 alphaMode = io_pSf->deviceRenderFlags & GR_ALPHAMASK;
   if (alphaMode == GR_ALPHA_CONSTANT) {
      alphaVal = io_pSf->deviceColorInfo.alpha;
      
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else if (alphaMode == GR_ALPHA_TEXTURE) {
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   }  else {
      io_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   }
   
   // For now, there is no vertex alpha
   //
   spanSetup.a0   = Int32(alphaVal * 255.0f * 65536.0f);
   spanSetup.dadx = 0;
   spanSetup.dady = 0;
   
   // Determine the r0, g0, b0 and partials for each.  We set the shadeScale in
   //  gfxPDC such that these are just straight conversions...
   //
   spanSetup.r0   = Int32(io_pSf->s0);
   spanSetup.drdx = Int32(io_pSf->dsdx);
   spanSetup.drdy = Int32(io_pSf->dsdy);
   spanSetup.g0   = Int32(io_pSf->s0);
   spanSetup.dgdx = Int32(io_pSf->dsdx);
   spanSetup.dgdy = Int32(io_pSf->dsdy);
   spanSetup.b0   = Int32(io_pSf->s0);
   spanSetup.dbdx = Int32(io_pSf->dsdx);
   spanSetup.dbdy = Int32(io_pSf->dsdy);

   // Create fifo buffer space for spans...
   //
   Int32 numSpans = GR_CountSpans(io_pSf->spanList,
                                  io_pSf->curblock);
   v_u32* cmdlist_ptr =
      pProxy->V_AddToCmdList(io_pSurface->getVCmdBuffer(),
                             sg_DSpanSetupUVQSWords   +
                             sg_DSpanWords * numSpans +
                             1);         // 1 word for end-of-list
   GR_CreateSpans(io_pSf->spanList,
                  io_pSf->curblock,
                  (GR_DSpan*)(cmdlist_ptr + sg_DSpanSetupUVQSWords));

   // Copy Span setup...
   memcpy(cmdlist_ptr, &spanSetup, sizeof(GR_DSpanSetupUVQS));
   cmdlist_ptr += sg_DSpanSetupUVQSWords;
   cmdlist_ptr += numSpans * sg_DSpanWords;
   
   // End of spans...
   *cmdlist_ptr = 0x80000000;
}

void 
GR_BlitSpansUVQH(Surface*      io_pSurface,
                 GFXPolyStore* io_pSf)
{
   RenditionProxy* pProxy = io_pSurface->getRenditionProxy();
   GR_DSpanSetupUVQH spanSetup;
   
   // Are we drawing transparent textures?
   //
   if ((io_pSf->deviceRenderFlags & GR_TRANSPARENCYMASK) == 0) {
      io_pSurface->setFilterEnable(true);
      io_pSurface->SetChromaKey(V_CHROMAKEY_DISABLE);
   } else {
      io_pSurface->setFilterEnable(false);
      io_pSurface->SetChromaKey(V_CHROMAKEY_ENABLE);
   }

   // Set up the texture coordinates...
   //
   double stride, height;
   if (io_pSf->tce != NULL) {
      stride = io_pSf->tce->csizeX;
      height = io_pSf->tce->csizeY;
   } else {
      stride = io_pSf->textureMapStride;
      height = io_pSf->textureMapHeight;
   }

   scaleWFactors(io_pSurface, io_pSf);

   spanSetup.uQ0   = Int32(io_pSf->u0   * stride);
   spanSetup.duQdx = Int32(io_pSf->dudx * stride);
   spanSetup.duQdy = Int32(io_pSf->dudy * stride);
   spanSetup.vQ0   = Int32(io_pSf->v0   * height);
   spanSetup.dvQdx = Int32(io_pSf->dvdx * height);
   spanSetup.dvQdy = Int32(io_pSf->dvdy * height);
   spanSetup.Q0    = Int32(io_pSf->w0   * (double(1 << 24) - 1));
   spanSetup.dQdx  = Int32(io_pSf->dwdx * (double(1 << 24) - 1));
   spanSetup.dQdy  = Int32(io_pSf->dwdy * (double(1 << 24) - 1));

   if (io_pSurface->getFilterType() == V_SRCFILTER_BILINEAR) {
      spanSetup.uQ0 += Int32((spanSetup.duQdx + spanSetup.duQdy) * 0.5);
      spanSetup.vQ0 += Int32((spanSetup.dvQdx + spanSetup.dvQdy) * 0.5);
      spanSetup.Q0  += Int32((spanSetup.dQdx  + spanSetup.dQdy)  * 0.5);
   }

   // Determine h0, dhdx, dhdy
   //
   io_pSurface->SetFogEnable(V_FOG_ENABLE);
   spanSetup.h0   = Int32((255.0f * 65536.0f) - io_pSf->s0);
   spanSetup.dhdx = Int32(-io_pSf->dsdx);
   spanSetup.dhdy = Int32(-io_pSf->dsdy);

   // We must determine if we need to be in REPLACE or MODULATE texture mode
   //
   UInt32 alphaMode = io_pSf->deviceRenderFlags & GR_ALPHAMASK;
   UInt32 shadeMode = io_pSf->deviceRenderFlags & GR_SHADEMASK;
   v_u32 srcFunc  = V_SRCFUNC_REPLACE;

   float alphaVal = 1.0f;
   if (alphaMode == GR_ALPHA_CONSTANT) {
      srcFunc  = V_SRCFUNC_MODULATE;
      alphaVal = io_pSf->deviceColorInfo.alpha;

      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else if (alphaMode == GR_ALPHA_TEXTURE) {
      srcFunc  = V_SRCFUNC_MODULATE;
      
      io_pSurface->SetBlendEnable(V_BLEND_ENABLE);
      io_pSurface->SetBlendDstFunc(V_BLENDSRCALPHAINV);
      io_pSurface->SetBlendSrcFunc(V_BLENDSRCALPHA);
   } else {
      io_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   }
   UInt32 finalColor = UInt32(alphaVal * 255.0f) << 24;

   if (shadeMode == GR_SHADE_CONSTANT) {
      srcFunc = V_SRCFUNC_MODULATE;
      
      finalColor |= (UInt32(io_pSf->deviceShade * 255.0f) << 16) |
                    (UInt32(io_pSf->deviceShade * 255.0f) <<  8) |
                    (UInt32(io_pSf->deviceShade * 255.0f) <<  0);
   } else {
      finalColor |= 0x00ffffff;
   }
   io_pSurface->SetFGColorARGB(finalColor);
   io_pSurface->SetSrcFunc(srcFunc);

   // Create fifo buffer space for spans...
   //
   Int32 numSpans = GR_CountSpans(io_pSf->spanList,
                                  io_pSf->curblock);
   v_u32* cmdlist_ptr =
      pProxy->V_AddToCmdList(io_pSurface->getVCmdBuffer(),
                             sg_DSpanSetupUVQHWords    +
                             sg_DSpanWords * numSpans +
                             1);         // 1 word for end-of-list
   GR_CreateSpans(io_pSf->spanList,
                  io_pSf->curblock,
                  (GR_DSpan*)(cmdlist_ptr + sg_DSpanSetupUVQHWords));

   // Copy Span setup...
   memcpy(cmdlist_ptr, &spanSetup, sizeof(GR_DSpanSetupUVQH));
   cmdlist_ptr += sg_DSpanSetupUVQHWords;
   cmdlist_ptr += numSpans * sg_DSpanWords;
   
   // End of spans...
   *cmdlist_ptr = 0x80000000;
}

}; // namespace Rendition

