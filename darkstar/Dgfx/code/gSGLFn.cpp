//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <windows.h>

#include "d_funcs.h"
#include "fn_all.h"
#include "g_pal.h"
#include "g_bitmap.h"

#include "gSGLSfc.h"

namespace PowerSGL {


// Fill modes, shade, haze and alpha sources are listed in inc\d_defs.h
//
void 
SetFillMode(GFXSurface* /*io_pSurface*/,
            GFXFillMode /*in_fm*/)
{
   // Possible fill modes:
   //  GFX_FILL_CONSTANT: Color fill
   //  GFX_FILL_TEXTURE:  Texture fill
   //  GFX_FILL_TWOPASS:  Texture with lightmap
   //
}


void 
SetShadeSource(GFXSurface*    /*io_pSurface*/,
               GFXShadeSource /*in_ss*/)
{
   // GFX_SHADE_NONE:     Draw at full brightness
   //          _CONSTANT: Draw at level set by SetConstantShadeCF
   //          _VERTEX:   Gouraud shaded by values sent to AddVertexVTC
   //
}


void 
SetHazeSource(GFXSurface*   /*io_pSurface*/,
              GFXHazeSource /*in_hs*/)
{
   // GFX_HAZE_NONE:     Draw with no haze
   //         _CONSTANT: Draw at level set by SetConstantHaze
   //         _VERTEX:   Gouraud shaded by values sent to AddVertexVTC
}


void 
SetAlphaSource(GFXSurface*    /*io_pSurface*/,
               GFXAlphaSource /*in_as*/)
{
   // GFX_ALPHA_NONE:     Draw fully opaque
   //          _CONSTANT: Draw at level set by SetConstantAlpha
   //          _VERTEX:   Unused currently
   //          _TEXTURE:  Draw translucent texture
   //          _FILL:     Draw a solid color index from the Palette transColor
   //                      (g_pal.h : 112) table.  (Index is send through
   //                      setFillColorI (alpha value is in peFlags)
}


void 
SetFillColorCF(GFXSurface*   /*io_pSurface*/,
               const ColorF* /*in_pColor*/)
{

}

void 
SetHazeColorCF(GFXSurface*   /*io_pSurface*/,
               const ColorF* /*in_pColor*/)
{

}

void 
SetConstantShadeCF(GFXSurface*   /*io_pSurface*/,
                   const ColorF* /*in_pShadeColor*/)
{

}


void 
SetConstantHaze(GFXSurface* /*io_pSurface*/,
                float       /*in_haze*/)
{

}


void 
SetConstantAlpha(GFXSurface* /*io_pSurface*/,
                 float       /*in_alpha*/)
{
   
}


void 
SetTexturePerspective(GFXSurface* /*io_pSurface*/,
                      Bool        /*perspTex*/)
{
   // Maybe not necessary for accelerated surface?  All vertices have w
   //  infomation passed to AddVertexVTC
}


void 
AddVertexVTC(GFXSurface*          /*io_pSurface*/,
             const Point3F*       /*in_pVert*/,
             const Point2F*       /*in_pTex*/,
             const GFXColorInfoF* /*in_pColor*/,
             DWORD                /*in_softwareEdgeKey*/)
{
   // Function must handle the case that in_pTex or in_pColor are NULL.  This
   //  should never happen unless the parameters are unnecessary to the current
   //  poly type.
   // Note: the Z parameter of the in_pVert struct is actually w
   // in_softwareEdgeKey can be ignored
}


void 
EmitPoly(GFXSurface* /*io_pSurface*/)
{
   // Send the current set of vertices to the card...
   //
}


void 
DrawPoint(GFXSurface*    io_pSurface,
          const Point2I* /*in_pt*/,
          float          /*in_w*/,
          DWORD          /*in_color*/)
{
   AssertFatal(io_pSurface->getPalette(), "No palette attached");
}


void 
DrawLine2d(GFXSurface*    io_pSurface,
           const Point2I* /*in_st*/,
           const Point2I* /*in_en*/,
           DWORD          /*in_color*/)
{
   AssertFatal(io_pSurface->getPalette(), "No palette attached");
}


void 
DrawRect2d(GFXSurface*  io_pSurface,
           const RectI* /*in_pRect*/,
           DWORD        /*in_color*/)
{
   // Draws rect _lines_
   //
   AssertFatal(io_pSurface->getPalette(), "No palette attached");
}


void 
DrawRect_f(GFXSurface*  io_pSurface,
           const RectI* /*in_pRect*/,
           float        /*in_w*/,
           DWORD        /*in_color*/)
{
   // Draws filled rect, against the depth buffer...
   //
   AssertFatal(io_pSurface->getPalette(), "No palette attached");
}


void 
DrawBitmap2d_f(GFXSurface*      /*io_pSurface*/,
               const GFXBitmap* /*in_pBM*/,
               const Point2I*   /*in_at*/,
               GFXFlipFlag      /*in_flip*/)
{
   // Note that clipping must be performed for bitmaps recommended form for
   //  hardware poly based bitmap draws is as Glide (glidesfc.cpp: ~1773)
   //
}


void 
DrawBitmap2d_rf(GFXSurface*      /*io_pSurface*/,
                const GFXBitmap* /*in_pBM*/,
                const RectI*     /*in_subRegion*/,
                const Point2I*   /*in_at*/,
                GFXFlipFlag      /*in_flip*/)
{

}


void 
DrawBitmap2d_sf(GFXSurface*      /*io_pSurface*/,
                const GFXBitmap* /*in_pBM*/,
                const Point2I*   /*in_at*/,
                const Point2I*   /*in_stretch*/,
                GFXFlipFlag      /*in_flip*/)
{

}


void 
SetTextureMap(GFXSurface*      /*io_pSurface*/,
              const GFXBitmap* /*pTextureMap*/)
{
   
}


void 
RegisterTexture(GFXSurface*      /*io_pSurface*/,
                GFXTextureHandle /*in_handle*/,
                int              /*in_sizeX*/,
                int              /*in_sizeY*/,
                int              /*in_offsetX*/,
                int              /*in_offsetY*/,
                int              /*in_lightScale*/,
                GFXLightMap*     /*io_pLightMap*/,
                const GFXBitmap* /*in_pTexture*/,
                int              /*in_mipLevel*/)
{

}


void 
RegisterTextureCB(GFXSurface*      /*io_pSurface*/,
                  GFXTextureHandle /*in_handle*/,
                  GFXCacheCallback /*in_cb*/,
                  int              /*in_csizeX*/,
                  int              /*in_csizeY*/,
                  int              /*in_lightScale*/,
                  GFXLightMap*     /*io_pLightMap*/)
{

}


Bool 
SetTextureHandle(GFXSurface*      /*io_pSurface*/,
                 GFXTextureHandle /*in_handle*/)
{
   return false;
}


void 
HandleSetLightMap(GFXSurface*  /*io_pSurface*/,
                  int          /*in_lightScale*/,
                  GFXLightMap* /*io_pLightMap*/)
{
   // Set the lightmap of the current texture handle
}


void 
HandleSetTextureMap(GFXSurface*      /*io_pSurface*/,
                    const GFXBitmap* /*in_pTexture*/)
{
   // Set the texturemap of the current texture handle
}


void 
HandleSetTextureSize(GFXSurface* /*io_pSurface*/,
                     int         /*in_newSize*/)
{
   // Set the texturesize of the current texture handle
   // Only used for callback textures
}


void 
FlushTextureCache(GFXSurface* /*io_pSurface*/)
{
   // Remove all textures from the card...
   //
}

void 
FlushTexture(GFXSurface*      /*in_pSurface*/,
             const GFXBitmap* /*in_pTexture*/,
             const bool       /*in_reload*/)
{
   // Remove specific texture from the cache.
   //
}


GFXLightMap* 
HandleGetLightMap(GFXSurface* /*io_pSurface*/)
{
   return NULL;
}


void 
SetTransparency(GFXSurface* /*io_pSurface*/,
                Bool        /*in_transFlag*/)
{
   // Enable chromakey.  ChromaColor is always (0, 0, 0)
}


void 
SetZTest(GFXSurface* /*io_pSurface*/,
         int         /*in_enable*/)
{
   // GFX_NO_ZTEST:        Disable Z testing and writing
   //    _ZTEST_AND_WRITE: Normal test and write
   //    _ZTEST:           Test, don't write
   //    _ZWRITE:          Write, don't test
   //    _ZALWAYSBEHIND:   Equivalent to ZWRITE
}


void 
Draw3dBegin(GFXSurface* /*io_pSurface*/)
{
   // Prepare for drawing 3d
}


void 
Draw3dEnd(GFXSurface* /*io_pSurface*/)
{
   // End 3d drawing
}


void 
ClearScreen(GFXSurface* io_pSurface,
            DWORD       /*in_color*/)
{
   // Mostly obsolete, usually a background fill poly is drawn instead...
   //
   AssertFatal(io_pSurface->getPalette(), "No palette attached");
}


void 
ClearZBuffer(GFXSurface* /*io_pSurface*/)
{
   // Mostly obsolete, usually a background fill poly is drawn instead...
}

//------------------------------------------------------------------------------
// All functions below this point are probably unnecessary for hardware...
//
//--------------------------------------
void 
HandleSetMipLevel(GFXSurface*  /*io_pSurface*/,
                  int          /*in_mipLevel*/)
{
   // Not necessary for hardware in which mips are downloaded and selected
   //  automatically
}

void 
AddVertexV(GFXSurface*    io_pSurface,
           const Point3F* in_pVert,
           DWORD          in_sofwareEdgeKey)
{
   // Obsolete: pass to full function...
   AddVertexVTC(io_pSurface, in_pVert, NULL, NULL, in_sofwareEdgeKey);
}

void 
AddVertexVT(GFXSurface*    io_pSurface,
            const Point3F* in_pVert,
            const Point2F* in_pTex,
            DWORD          in_sofwareEdgeKey)
{
   // Obsolete: pass to full function...
   AddVertexVTC(io_pSurface, in_pVert, in_pTex, NULL, in_sofwareEdgeKey);
}

void 
AddVertexVC(GFXSurface*          io_pSurface,
            const Point3F*       in_pVert,
            const GFXColorInfoF* in_pColor,
            DWORD                in_sofwareEdgeKey)
{
   // Obsolete: pass to full function...
   AddVertexVTC(io_pSurface, in_pVert, NULL, in_pColor, in_sofwareEdgeKey);
}

void 
SetTextureWrap(GFXSurface* /*io_pSurface*/,
               Bool        /*in_wrapEnable*/)
{
   // Probably not necessary in hardware.  No texture coordinates that would
   //  cause edge clamping are passed.
}

//------------------------------------------------------------------------------
// void SetFillColorI(GFXSurface* io_pSurface, Int32 in_index)
// void SetHazeColorI(GFXSurface* io_pSurface, Int32 in_index)
// void SetConstantShadeF(GFXSurface* io_pSurface, float in_shade);
//
//  These functions are actually mostly useless in hardware, default is to just
// convert them to their absolute color equivalent, and pass them to the
// corresponding CF function.
//
//------------------------------------------------------------------------------
//
void 
SetFillColorI(GFXSurface* io_pSurface,
              Int32       in_index,
              DWORD       paletteIndex)
{
   // Recommended form for hardware...
   //
   AssertFatal(io_pSurface->getPalette() != NULL, "No palette attached to surface");
   GFXPalette::MultiPalette *mp = io_pSurface->getPalette()->findMultiPalette(paletteIndex);

   PALETTEENTRY* pColors = mp->color;
   ColorF fillColor;
   fillColor.set(float(pColors[in_index].peRed)   / 255.0f,
                 float(pColors[in_index].peGreen) / 255.0f,
                 float(pColors[in_index].peBlue)  / 255.0f);
   SetFillColorCF(io_pSurface, &fillColor);
}

void 
SetHazeColorI(GFXSurface* io_pSurface,
              Int32       in_index)
{
   // Recommended form for hardware...
   //
   AssertFatal(io_pSurface->getPalette() != NULL, "No palette attached to surface");

   PALETTEENTRY* pColors = io_pSurface->getPalette()->palette[0].color;
   ColorF hazeColor;
   hazeColor.set(float(pColors[in_index].peRed)   / 255.0f,
                 float(pColors[in_index].peGreen) / 255.0f,
                 float(pColors[in_index].peBlue)  / 255.0f);
   SetHazeColorCF(io_pSurface, &hazeColor);
}

void 
SetConstantShadeF(GFXSurface* io_pSurface,
                  float       in_shade)
{
   // Recommended form for hardware...
   //
   ColorF shadeColor;
   shadeColor.set(in_shade, in_shade, in_shade);
   SetConstantShadeCF(io_pSurface, &shadeColor);
}

}; // namespace PowerSGl

FunctionTable powersgl_table =
{
   PowerSGL::ClearScreen,    

   PowerSGL::DrawPoint,
   PowerSGL::DrawLine2d,
   PowerSGL::DrawRect2d,
   PowerSGL::DrawRect_f,

   PowerSGL::DrawBitmap2d_f,
   PowerSGL::DrawBitmap2d_rf,
   PowerSGL::DrawBitmap2d_sf,
   NULL,

   GFXDrawText_p,    // Unnecessary to modify
   GFXDrawText_r,    // Unnecessary to modify

   PowerSGL::Draw3dBegin,
   PowerSGL::Draw3dEnd,
   PowerSGL::AddVertexV,
   PowerSGL::AddVertexVT,
   PowerSGL::AddVertexVC,
   PowerSGL::AddVertexVTC,
   PowerSGL::EmitPoly,
   PowerSGL::SetShadeSource,
   PowerSGL::SetHazeSource,
   PowerSGL::SetAlphaSource,
   PowerSGL::SetFillColorCF,
   PowerSGL::SetFillColorI,
   PowerSGL::SetHazeColorCF,
   PowerSGL::SetHazeColorI,
   PowerSGL::SetConstantShadeCF,
   PowerSGL::SetConstantShadeF,
   PowerSGL::SetConstantHaze,
   PowerSGL::SetConstantAlpha,
   PowerSGL::SetTransparency,
   PowerSGL::SetTextureMap,
   PowerSGL::SetFillMode,
   PowerSGL::SetTexturePerspective,

   PowerSGL::RegisterTexture,
   PowerSGL::RegisterTextureCB,
   PowerSGL::SetTextureHandle,
   GFXAllocateLightMap,          // Unnecessary to modify
   PowerSGL::HandleGetLightMap,
   PowerSGL::HandleSetLightMap,
   PowerSGL::HandleSetTextureMap,
   PowerSGL::HandleSetMipLevel,
   PowerSGL::HandleSetTextureSize,
   PowerSGL::FlushTextureCache,
   PowerSGL::SetZTest,
   PowerSGL::ClearZBuffer,
   PowerSGL::SetTextureWrap,

   PowerSGL::FlushTexture
};
