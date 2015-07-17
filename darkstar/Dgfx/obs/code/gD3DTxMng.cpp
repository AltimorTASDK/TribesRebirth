//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gD3DTxTrans.h"
#include "gD3DTxMng.h"



//------------------------------------------------------------------------------
//-------------------------------------- Different Texture Translators derived
//--------------------------------------  from TextureMunger
//
namespace Direct3D {

//-------------------------------------- renderState members for each Munger
//
D3DTexRenderState MungerPalettized::renderState = {
   D3DBLEND_ONE,           // srcBlend
   D3DBLEND_ZERO,          // destBlend
   false                   // alpha
};

D3DTexRenderState Munger565Normal::renderState = {
   D3DBLEND_ONE,           // srcBlend
   D3DBLEND_ZERO,          // destBlend
   false                   // alpha
};

D3DTexRenderState Munger555Normal::renderState = {
   D3DBLEND_ONE,           // srcBlend
   D3DBLEND_ZERO,          // destBlend
   false                   // alpha
};

D3DTexRenderState Munger4444Alpha::renderState = {
   D3DBLEND_SRCALPHA,      // srcBlend
   D3DBLEND_INVSRCALPHA,   // destBlend
   true                    // alpha
};

D3DTexRenderState Munger1555Alpha::renderState = {
   D3DBLEND_SRCALPHA,      // srcBlend
   D3DBLEND_INVSRCALPHA,   // destBlend
   true                    // alpha
};

D3DTexRenderState Munger565LM::renderState = {
   D3DBLEND_ZERO,          // srcBlend
   D3DBLEND_SRCCOLOR,      // destBlend
   true                    // alpha
};

D3DTexRenderState Munger555LM::renderState = {
   D3DBLEND_ZERO,          // srcBlend
   D3DBLEND_SRCCOLOR,      // destBlend
   true                    // alpha
};

D3DTexRenderState Munger444LM::renderState = {
   D3DBLEND_ZERO,          // srcBlend
   D3DBLEND_SRCCOLOR,      // destBlend
   true                    // alpha
};

D3DTexRenderState Munger888LM::renderState = {
   D3DBLEND_ZERO,          // srcBlend
   D3DBLEND_SRCCOLOR,      // destBlend
   true                    // alpha
};


//-------------------------------------- Copies of the EXACT format that
//                                        canTranslate agrees to...
DDSURFACEDESC MungerPalettized::masterDDSDCopy;
DDSURFACEDESC Munger565Normal::masterDDSDCopy;
DDSURFACEDESC Munger555Normal::masterDDSDCopy;
DDSURFACEDESC Munger4444Alpha::masterDDSDCopy;
DDSURFACEDESC Munger1555Alpha::masterDDSDCopy;
DDSURFACEDESC Munger565LM::masterDDSDCopy;
DDSURFACEDESC Munger555LM::masterDDSDCopy;
DDSURFACEDESC Munger444LM::masterDDSDCopy;
DDSURFACEDESC Munger888LM::masterDDSDCopy;



int Munger565LM::getTextureType() { return GD3D_LIGHTMAP; }
int Munger555LM::getTextureType() { return GD3D_LIGHTMAP; }
int Munger444LM::getTextureType() { return GD3D_LIGHTMAP; }
int Munger888LM::getTextureType() { return GD3D_LIGHTMAP; }

int MungerPalettized::getTextureType()
{
   if (beingUsedForTransparency == false)
      return GD3D_NORMAL;
   else
      return GD3D_TRANSPARENT;
}

int Munger565Normal::getTextureType()
{
   if (beingUsedForTransparency == false)
      return GD3D_NORMAL;
   else
      return GD3D_TRANSPARENT;
}

int Munger555Normal::getTextureType()
{
   if (beingUsedForTransparency == false)
      return GD3D_NORMAL;
   else
      return GD3D_TRANSPARENT;
}

int Munger4444Alpha::getTextureType()
{
   if (beingUsedForTransparency == false)
      return GD3D_ALPHA;
   else
      return GD3D_TRANSPARENT;
}

int Munger1555Alpha::getTextureType()
{
   if (beingUsedForTransparency == false)
      return GD3D_ALPHA;
   else
      return GD3D_TRANSPARENT;
}

//------------------------------------------------------------------------------
//-------------------------------------- 8 Bit palettized functions
//

MungerPalettized::MungerPalettized(const bool in_transparentMunger)
 : beingUsedForTransparency(in_transparentMunger),
   TextureMunger(in_transparentMunger)
{

}



MungerPalettized::~MungerPalettized()
{
   // Nothing to do for this class
}


bool 
MungerPalettized::canTranslate(DDSURFACEDESC* pDDSD)
{
   AssertFatal(pDDSD != NULL, "null ptr in Direct3D::MungerPalettized::canTranslate()");

   if (((pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0) &&
       ((pDDSD->dwFlags & DDSCAPS_TEXTURE)  != 0)) {
      bool success = true;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
         success = false;
      
      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) == 0)
         success = false;

      if (pDDSD->ddpfPixelFormat.dwRGBBitCount != 8)
         success = false;

      if (success == true) {
         memcpy(&masterDDSDCopy, pDDSD, sizeof(DDSURFACEDESC));
         return true;
      }
   }
   
   return false;
}


void 
MungerPalettized::getSurfDesc(DDSURFACEDESC* out_pDDSD)
{
   AssertFatal(out_pDDSD != NULL,
               "null ptr in Direct3D::MungerPalettized::getSurfDesc()");

   // We assume that the DDSD is ours to do with what we want...
   //
   memcpy(out_pDDSD, &masterDDSDCopy, sizeof(DDSURFACEDESC));
}


void 
MungerPalettized::translateBitmap(const GFXBitmap*  in_pTexture,
                                  const Int32       in_mipLevel,
                                  void*             out_pDDTex,
                                  const Int32       in_dstStride)
{
   // We simply need to copy the bitmap here...
   //
   AssertFatal(in_pTexture != NULL, "Error no texture passed to Munger4444Alpha::translateBitmap");
   AssertFatal(out_pDDTex  != NULL, "Error no out_ptr passed to Munger4444Alpha::translateBitmap");
   AssertFatal(in_mipLevel < in_pTexture->detailLevels,
               "Invalid detail level passed to translateBitmap");

   Int32 width    = in_pTexture->width  >> in_mipLevel;
   Int32 height   = in_pTexture->height >> in_mipLevel;
   Int32 stride   = in_pTexture->stride >> in_mipLevel;
   Int32 numBytes = width * height;

   const UInt8* pSrcBytes = static_cast<const UInt8*>(in_pTexture->pMipBits[in_mipLevel]);
   UInt8*       pDstBytes = static_cast<UInt8*>(out_pDDTex);

   if (stride == width) {
      if (in_dstStride == width) {
         memcpy(pDstBytes, pSrcBytes, numBytes);
      } else {
         for (int i = 0; i < height; i++) {
            memcpy(pDstBytes, pSrcBytes, width);
            pDstBytes += in_dstStride;
            pSrcBytes += width;
         }
      }
   } else {
      for (int i = 0; i < height; i++) {
         memcpy(pDstBytes, pSrcBytes, width);
         pDstBytes += in_dstStride;
         pSrcBytes += stride;
      }
   }
}


void 
MungerPalettized::setPalette(const GFXPalette* /*in_pPal*/)
{
   // NULL
}


bool 
MungerPalettized::attachPalette() const
{
   return true;
}


bool 
MungerPalettized::setColorKey() const
{
   return beingUsedForTransparency;
}


bool 
MungerPalettized::isSurfaceGFXFormat() const
{
   return true;
}


const D3DTexRenderState* 
MungerPalettized::getRenderState() const
{
   return &renderState;
}


//------------------------------------------------------------------------------
//-------------------------------------- 16 Bit 565 functions
//

Munger565Normal::Munger565Normal(const bool in_transparentMunger)
 : translationTable(NULL),
   beingUsedForTransparency(in_transparentMunger),
   TextureMunger(in_transparentMunger)
{
   
}



Munger565Normal::~Munger565Normal()
{
   if (translationTable != NULL)
      delete [] translationTable;

   translationTable = NULL;
}


bool 
Munger565Normal::canTranslate(DDSURFACEDESC* pDDSD)
{
   AssertFatal(pDDSD != NULL, "null ptr in Direct3D::Munger565Normal::canTranslate()");

   if (((pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0) &&
       ((pDDSD->dwFlags & DDSCAPS_TEXTURE)  != 0)) {
      bool success = true;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
         success = false;
      
      if (pDDSD->ddpfPixelFormat.dwRGBBitCount != 16)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwRBitMask != 0xf800) ||
          (pDDSD->ddpfPixelFormat.dwGBitMask != 0x7e0) ||
          (pDDSD->ddpfPixelFormat.dwBBitMask != 0x1f)) {
         success = false;
      }

      if (success == true) {
         memcpy(&masterDDSDCopy, pDDSD, sizeof(DDSURFACEDESC));
         return true;
      }
   } 
   
   // bogus ddsd
   return false;
}


void 
Munger565Normal::getSurfDesc(DDSURFACEDESC* out_pDDSD)
{
   AssertFatal(out_pDDSD != NULL,
               "null ptr in Direct3D::Munger565Normal::getSurfDesc()");

   // We assume that the DDSD is ours to do with what we want...
   //
   memcpy(out_pDDSD, &masterDDSDCopy, sizeof(DDSURFACEDESC));
}


void 
Munger565Normal::translateBitmap(const GFXBitmap*  in_pTexture,
                                 const Int32       in_mipLevel,
                                 void*             out_pDDTex,
                                 const Int32       in_dstStride)
{
   AssertFatal(translationTable != NULL, "Error: palette not initialized for Munger4444Alpha");
   AssertFatal(in_pTexture != NULL, "Error no texture passed to Munger4444Alpha::translateBitmap");
   AssertFatal(out_pDDTex  != NULL, "Error no out_ptr passed to Munger4444Alpha::translateBitmap");
   AssertFatal(in_mipLevel < in_pTexture->detailLevels,
               "Invalid detail level passed to translateBitmap");
   
   Int32 height   = in_pTexture->height >> in_mipLevel;
   Int32 width    = in_pTexture->width  >> in_mipLevel;
   Int32 stride   = in_pTexture->stride >> in_mipLevel;
   Int32 numBytes = stride * height;

   const UInt8* pSrcBytes = static_cast<const UInt8*>(in_pTexture->pMipBits[in_mipLevel]);
   UInt16*      pDstWords = static_cast<UInt16*>(out_pDDTex);

   if (in_dstStride == (stride * 2)) {
      for (int i = 0; i < numBytes; i++)
         pDstWords[i] = translationTable[pSrcBytes[i]];
   } else {
      for (int i = 0; i < height; i++) {
         for (int j = 0; j < width; j++) {
            pDstWords[j] = translationTable[pSrcBytes[j]];
         }
         // advancing a 16 bit ptr
         pDstWords += (in_dstStride/2);
         pSrcBytes += stride;
      }
   }
}


inline UInt16 
Munger565Normal::mungeEntry(UInt16 r, UInt16 g, UInt16 b)
{
   return (((r >> 3) << 11) |
           ((g >> 2) << 5)  |
           ((b >> 3) << 0));
}


void 
Munger565Normal::setPalette(const GFXPalette *in_pPal)
{
   AssertFatal(in_pPal != NULL, "Error, bogus palette passed to Munger4444Alpha::setPalette");
   
   if (translationTable == NULL)
      translationTable = new UInt16[256];

   for (int i = 0; i < 256; i++) {
      const PALETTEENTRY* pEntry = &(in_pPal->color[i]);

      UInt16 r = pEntry->peRed;
      UInt16 g = pEntry->peGreen;
      UInt16 b = pEntry->peBlue;

      translationTable[i] = mungeEntry(r, g, b);
   }
}


bool 
Munger565Normal::attachPalette() const
{
   return false;
}


bool 
Munger565Normal::setColorKey() const
{
   return beingUsedForTransparency;
}


const D3DTexRenderState* 
Munger565Normal::getRenderState() const
{
   return &renderState;
}


//------------------------------------------------------------------------------
//-------------------------------------- 16 Bit 555 functions
//

Munger555Normal::Munger555Normal(const bool in_transparentMunger)
 : translationTable(NULL),
   beingUsedForTransparency(in_transparentMunger),
   TextureMunger(in_transparentMunger)
{
   
}



Munger555Normal::~Munger555Normal()
{
   if (translationTable != NULL)
      delete [] translationTable;

   translationTable = NULL;
}


bool 
Munger555Normal::canTranslate(DDSURFACEDESC* pDDSD)
{
   AssertFatal(pDDSD != NULL, "null ptr in Direct3D::Munger555Normal::canTranslate()");

   if (((pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0) &&
       ((pDDSD->dwFlags & DDSCAPS_TEXTURE)  != 0)) {
      bool success = true;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
         success = false;
      
      if (pDDSD->ddpfPixelFormat.dwRGBBitCount != 16)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwRBitMask != 0x7c00) ||
          (pDDSD->ddpfPixelFormat.dwGBitMask != 0x3e0) ||
          (pDDSD->ddpfPixelFormat.dwBBitMask != 0x1f)) {
         success = false;
      }

      if (success == true) {
         memcpy(&masterDDSDCopy, pDDSD, sizeof(DDSURFACEDESC));
         return true;
      }
   } 
   
   // bogus ddsd
   return false;
}


void 
Munger555Normal::getSurfDesc(DDSURFACEDESC* out_pDDSD)
{
   AssertFatal(out_pDDSD != NULL,
               "null ptr in Direct3D::Munger555Normal::getSurfDesc()");

   // We assume that the DDSD is ours to do with what we want...
   //
   memcpy(out_pDDSD, &masterDDSDCopy, sizeof(DDSURFACEDESC));
}


void 
Munger555Normal::translateBitmap(const GFXBitmap*  in_pTexture,
                                 const Int32       in_mipLevel,
                                 void*             out_pDDTex,
                                 const Int32       in_dstStride)
{
   AssertFatal(translationTable != NULL, "Error: palette not initialized for Munger4444Alpha");
   AssertFatal(in_pTexture != NULL, "Error no texture passed to Munger4444Alpha::translateBitmap");
   AssertFatal(out_pDDTex  != NULL, "Error no out_ptr passed to Munger4444Alpha::translateBitmap");
   AssertFatal(in_mipLevel < in_pTexture->detailLevels,
               "Invalid detail level passed to translateBitmap");
   AssertWarn(in_pTexture->width == in_pTexture->stride,
              "Warning: stride != width for texture?!?");
   
   Int32 height   = in_pTexture->height >> in_mipLevel;
   Int32 width    = in_pTexture->width  >> in_mipLevel;
   Int32 stride   = in_pTexture->stride >> in_mipLevel;
   Int32 numBytes = width * height;

   const UInt8* pSrcBytes = static_cast<const UInt8*>(in_pTexture->pMipBits[in_mipLevel]);
   UInt16*      pDstBytes = static_cast<UInt16*>(out_pDDTex);

   if (in_dstStride == (stride * 2)) {
      for (int i = 0; i < numBytes; i++)
         pDstBytes[i] = translationTable[pSrcBytes[i]];
   } else {
      for (int i = 0; i < height; i++) {
         for (int j = 0; j < width; j++) {
            pDstBytes[j] = translationTable[pSrcBytes[j]];
         }
         // advancing a 16 bit ptr
         pDstBytes += (in_dstStride/2);
         pSrcBytes += stride;
      }
   }
}


inline UInt16 
Munger555Normal::mungeEntry(UInt16 r, UInt16 g, UInt16 b)
{
   return (((r >> 3) << 10) |
           ((g >> 3) << 5)  |
           ((b >> 3) << 0));
}


void 
Munger555Normal::setPalette(const GFXPalette *in_pPal)
{
   AssertFatal(in_pPal != NULL, "Error, bogus palette passed to Munger4444Alpha::setPalette");
   
   if (translationTable == NULL)
      translationTable = new UInt16[256];

   for (int i = 0; i < 256; i++) {
      const PALETTEENTRY* pEntry = &(in_pPal->color[i]);

      UInt16 r = pEntry->peRed;
      UInt16 g = pEntry->peGreen;
      UInt16 b = pEntry->peBlue;

      translationTable[i] = mungeEntry(r, g, b);
   }
}


bool 
Munger555Normal::attachPalette() const
{
   return false;
}


bool 
Munger555Normal::setColorKey() const
{
   return beingUsedForTransparency;
}


const D3DTexRenderState* 
Munger555Normal::getRenderState() const
{
   return &renderState;
}


//------------------------------------------------------------------------------
//-------------------------------------- 16 Bit 4444 functions
//

Munger4444Alpha::Munger4444Alpha(const bool in_driverAlphaInverted,
                                 const bool in_transparentMunger)
 : translationTable(NULL),
   driverAlphaInverted(in_driverAlphaInverted),
   beingUsedForTransparency(in_transparentMunger),
   TextureMunger(in_transparentMunger)
{
   if (driverAlphaInverted) {
      renderState.srcBlend  = D3DBLEND_INVSRCALPHA;
      renderState.destBlend = D3DBLEND_SRCALPHA;
   }
}



Munger4444Alpha::~Munger4444Alpha()
{
   if (translationTable != NULL)
      delete [] translationTable;

   translationTable = NULL;
}


bool 
Munger4444Alpha::canTranslate(DDSURFACEDESC* pDDSD)
{
   AssertFatal(pDDSD != NULL, "null ptr in Direct3D::Munger565Normal::canTranslate()");

   if (((pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0) &&
       ((pDDSD->dwFlags & DDSCAPS_TEXTURE)  != 0)) {
      bool success = true;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) == 0)
         success = false;
      
      if (pDDSD->ddpfPixelFormat.dwRGBBitCount != 16)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwRBitMask != 0xf00) ||
          (pDDSD->ddpfPixelFormat.dwGBitMask != 0xf0)  ||
          (pDDSD->ddpfPixelFormat.dwBBitMask != 0xf)   ||
          (pDDSD->ddpfPixelFormat.dwRGBAlphaBitMask != 0xf000)) {
         success = false;
      }

      if (success == true) {
         memcpy(&masterDDSDCopy, pDDSD, sizeof(DDSURFACEDESC));
         return true;
      }
   } 
   
   return false;
}


void 
Munger4444Alpha::getSurfDesc(DDSURFACEDESC* out_pDDSD)
{
   AssertFatal(out_pDDSD != NULL,
               "null ptr in Direct3D::Munger565Normal::getSurfDesc()");

   // We assume that the DDSD is ours to do with what we want...
   //
   memcpy(out_pDDSD, &masterDDSDCopy, sizeof(DDSURFACEDESC));
}


void 
Munger4444Alpha::translateBitmap(const GFXBitmap*  in_pTexture,
                                 const Int32       in_mipLevel,
                                 void*             out_pDDTex,
                                 const Int32       in_dstStride)
{
   AssertFatal(translationTable != NULL, "Error: palette not initialized for Munger4444Alpha");
   AssertFatal(in_pTexture != NULL, "Error no texture passed to Munger4444Alpha::translateBitmap");
   AssertFatal(out_pDDTex  != NULL, "Error no out_ptr passed to Munger4444Alpha::translateBitmap");
   AssertFatal(in_mipLevel < in_pTexture->detailLevels,
               "Invalid detail level passed to translateBitmap");
   AssertWarn(in_pTexture->width == in_pTexture->stride,
              "Warning: stride != width for texture?!?");
      
   Int32 width    = in_pTexture->width  >> in_mipLevel;
   Int32 height   = in_pTexture->height >> in_mipLevel;
   Int32 stride   = in_pTexture->stride >> in_mipLevel;
   Int32 numBytes = width * height;

   const UInt8* pSrcBytes = static_cast<const UInt8*>(in_pTexture->pMipBits[in_mipLevel]);
   UInt16*      pDstBytes = static_cast<UInt16*>(out_pDDTex);

   if (in_dstStride == (stride * 2)) {
      for (int i = 0; i < numBytes; i++)
         pDstBytes[i] = translationTable[pSrcBytes[i]];
   } else {
      for (int i = 0; i < height; i++) {
         for (int j = 0; j < width; j++) {
            pDstBytes[j] = translationTable[pSrcBytes[j]];
         }
         // advancing a 16 bit ptr
         pDstBytes += (in_dstStride/2);
         pSrcBytes += stride;
      }
   }
}



inline UInt16 
Munger4444Alpha::mungeEntry(UInt16 r, UInt16 g, UInt16 b, UInt16 a)
{
   if (driverAlphaInverted == false) {
      UInt16 final;
      final  = (a << 8) & 0xf000;
      final |= (r << 4) & 0x0f00;
      final |= g & 0x00f0;
      final |= b >> 4;

      return final;
   } else {
      UInt16 final;
      final  = (15 - (a >> 4)) << 12;
      final |= (r << 4) & 0x0f00;
      final |= (g & 0x00f0);
      final |= (b >> 4);

      return final;
   }
}


void 
Munger4444Alpha::setPalette(const GFXPalette *in_pPal)
{
   AssertFatal(in_pPal != NULL, "Error, bogus palette passed to Munger4444Alpha::setPalette");
   
   if (translationTable == NULL)
      translationTable = new UInt16[256];

   for (int i = 0; i < 256; i++) {
      UInt16 r, g, b, a;
      const PALETTEENTRY* pEntry;

      if (beingUsedForTransparency == false)
         pEntry = &(in_pPal->transColor[i]);
      else
         pEntry = &(in_pPal->color[i]);

      r = pEntry->peRed;
      g = pEntry->peGreen;
      b = pEntry->peBlue;

      if (beingUsedForTransparency == false) {
         a = pEntry->peFlags;
      } else {
         a = 255;
      }

      translationTable[i] = mungeEntry(r, g, b, a);
   }

   if (beingUsedForTransparency == true) {
      // if we're using this blitter for translucency, then the first
      //  color entry is completely transparent...
      translationTable[0] = mungeEntry(0, 0, 0, 0);
   }
}


bool 
Munger4444Alpha::attachPalette() const
{
   return false;
}


bool 
Munger4444Alpha::setColorKey() const
{
   return false;
}


const D3DTexRenderState* 
Munger4444Alpha::getRenderState() const
{
   return &renderState;
}




//------------------------------------------------------------------------------
//-------------------------------------- 16 Bit 1555 functions
//

Munger1555Alpha::Munger1555Alpha(const bool in_driverAlphaInverted,
                                 const bool in_transparentMunger)
 : translationTable(NULL),
   driverAlphaInverted(in_driverAlphaInverted),
   beingUsedForTransparency(in_transparentMunger),
   TextureMunger(in_transparentMunger)
{
   if (driverAlphaInverted) {
      renderState.srcBlend  = D3DBLEND_INVSRCALPHA;
      renderState.destBlend = D3DBLEND_SRCALPHA;
   }
}



Munger1555Alpha::~Munger1555Alpha()
{
   if (translationTable != NULL)
      delete [] translationTable;

   translationTable = NULL;
}


bool 
Munger1555Alpha::canTranslate(DDSURFACEDESC* pDDSD)
{
   AssertFatal(pDDSD != NULL, "null ptr in Direct3D::Munger565Normal::canTranslate()");

   if (((pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0) &&
       ((pDDSD->dwFlags & DDSCAPS_TEXTURE)  != 0)) {
      bool success = true;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) == 0)
         success = false;
      
      if (pDDSD->ddpfPixelFormat.dwRGBBitCount != 16)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwRBitMask != 0x7c00) ||
          (pDDSD->ddpfPixelFormat.dwGBitMask != 0x3e0)  ||
          (pDDSD->ddpfPixelFormat.dwBBitMask != 0x1f)   ||
          (pDDSD->ddpfPixelFormat.dwRGBAlphaBitMask != 0x8000)) {
         success = false;
      }

      if (success == true) {
         memcpy(&masterDDSDCopy, pDDSD, sizeof(DDSURFACEDESC));
         return true;
      }
   } 
   
   return false;
}


void 
Munger1555Alpha::getSurfDesc(DDSURFACEDESC* out_pDDSD)
{
   AssertFatal(out_pDDSD != NULL,
               "null ptr in Direct3D::Munger1555Normal::getSurfDesc()");

   // We assume that the DDSD is ours to do with what we want...
   //
   memcpy(out_pDDSD, &masterDDSDCopy, sizeof(DDSURFACEDESC));
}


void 
Munger1555Alpha::translateBitmap(const GFXBitmap*  in_pTexture,
                                 const Int32       in_mipLevel,
                                 void*             out_pDDTex,
                                 const Int32       in_dstStride)
{
   AssertFatal(translationTable != NULL, "Error: palette not initialized for Munger4444Alpha");
   AssertFatal(in_pTexture != NULL, "Error no texture passed to Munger4444Alpha::translateBitmap");
   AssertFatal(out_pDDTex  != NULL, "Error no out_ptr passed to Munger4444Alpha::translateBitmap");
   AssertFatal(in_mipLevel < in_pTexture->detailLevels,
               "Invalid detail level passed to translateBitmap");
   AssertWarn(in_pTexture->width == in_pTexture->stride,
              "Warning: stride != width for texture?!?");
      
   Int32 width    = in_pTexture->width  >> in_mipLevel;
   Int32 height   = in_pTexture->height >> in_mipLevel;
   Int32 stride   = in_pTexture->stride >> in_mipLevel;
   Int32 numBytes = width * height;

   const UInt8* pSrcBytes = static_cast<const UInt8*>(in_pTexture->pMipBits[in_mipLevel]);
   UInt16*      pDstBytes = static_cast<UInt16*>(out_pDDTex);

   if (in_dstStride == (stride * 2)) {
      for (int i = 0; i < numBytes; i++)
         pDstBytes[i] = translationTable[pSrcBytes[i]];
   } else {
      for (int i = 0; i < height; i++) {
         for (int j = 0; j < width; j++) {
            pDstBytes[j] = translationTable[pSrcBytes[j]];
         }
         // advancing a 16 bit ptr
         pDstBytes += (in_dstStride/2);
         pSrcBytes += stride;
      }
   }
}



inline UInt16 
Munger1555Alpha::mungeEntry(UInt16 r, UInt16 g, UInt16 b, UInt16 a)
{
   if (driverAlphaInverted == false) {
      UInt16 final;
      final  = (a << 8) & 0x8000;
      final |= (r << 7) & 0x7c00;
      final |= (g << 2) & 0x03e0;
      final |= (b >> 3);

      return final;
   } else {
      UInt16 final;
      final  = ((255 - a) << 8) & 0x8000;
      final |= (r << 7) & 0x7c00;
      final |= (g << 2) & 0x03e0;
      final |= (b >> 3);

      return final;
   }
}


void 
Munger1555Alpha::setPalette(const GFXPalette *in_pPal)
{
   AssertFatal(in_pPal != NULL, "Error, bogus palette passed to Munger4444Alpha::setPalette");
   
   if (translationTable == NULL)
      translationTable = new UInt16[256];

   for (int i = 0; i < 256; i++) {
      UInt16 r, g, b, a;
      const PALETTEENTRY* pEntry;

      if (beingUsedForTransparency == false)
         pEntry = &(in_pPal->transColor[i]);
      else
         pEntry = &(in_pPal->color[i]);

      r = pEntry->peRed;
      g = pEntry->peGreen;
      b = pEntry->peBlue;

      if (beingUsedForTransparency == false) {
         a = pEntry->peFlags;
      } else {
         a = 255;
      }

      translationTable[i] = mungeEntry(r, g, b, a);
   }

   if (beingUsedForTransparency == true) {
      // if we're using this blitter for translucency, then the first
      //  color entry is completely transparent...
      translationTable[0] = mungeEntry(0, 0, 0, 0);
   }
}


bool 
Munger1555Alpha::attachPalette() const
{
   return false;
}


bool 
Munger1555Alpha::setColorKey() const
{
   // If the driver could do color key, we wouldn't being used for it in
   //  the first place...
   //
   return false;
}


const D3DTexRenderState* 
Munger1555Alpha::getRenderState() const
{
   return &renderState;
}


//------------------------------------------------------------------------------
//-------------------------------------- 16 Bit 565 Lightmap functions
//

Munger565LM::Munger565LM(const bool  in_lightMapInverted,
                         const DWORD in_lightMapMode)
 : lightMapInverted(in_lightMapInverted)
{
   renderState.destBlend = (D3DBLEND)in_lightMapMode;
}



Munger565LM::~Munger565LM()
{
   // NULL
}


bool 
Munger565LM::canTranslate(DDSURFACEDESC* pDDSD)
{
   // Note: this is the same as Munger565Normal's equivalent function

   AssertFatal(pDDSD != NULL, "null ptr in Direct3D::Munger565Normal::canTranslate()");

   if (((pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0) &&
       ((pDDSD->dwFlags & DDSCAPS_TEXTURE)  != 0)) {
      bool success = true;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
         success = false;
      
      if (pDDSD->ddpfPixelFormat.dwRGBBitCount != 16)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwRBitMask != 0xf800) ||
          (pDDSD->ddpfPixelFormat.dwGBitMask != 0x7e0) ||
          (pDDSD->ddpfPixelFormat.dwBBitMask != 0x1f)) {
         success = false;
      }

      if (success == true) {
         memcpy(&masterDDSDCopy, pDDSD, sizeof(DDSURFACEDESC));
         return true;
      }
   } 
   
   return false;
}


void 
Munger565LM::getSurfDesc(DDSURFACEDESC* out_pDDSD)
{
   // Note: this is the same as Munger565Normal's equivalent function

   AssertFatal(out_pDDSD != NULL,
               "null ptr in Direct3D::Munger565Normal::getSurfDesc()");

   // We assume that the DDSD is ours to do with what we want...
   //
   memcpy(out_pDDSD, &masterDDSDCopy, sizeof(DDSURFACEDESC));
}


inline UInt16 
Munger565LM::mungeEntry(UInt16 in_lmValue)
{
   // Note: In 565 LMaps, we have to make sure that full bright is
   //  actually full bright, since the shifts cause the lm to interpret
   //  0xffff as less than full intensity...
   //
   if (lightMapInverted == false) {
      UInt16 final;
      UInt16 r = (in_lmValue >> 8) & 0xf;
      UInt16 g = (in_lmValue >> 4) & 0xf;
      UInt16 b = (in_lmValue >> 0) & 0xf;

      final = (r << 12) |
              (g << 7)  |
              (b << 1);

      if (in_lmValue == 0xffff)
         final = 0xffff;

      return final;
   } else {
      UInt16 final;
      final  = (15 - ((in_lmValue >> 8)  & 0xf)) << 12;   // r invert
      final |= (15 - ((in_lmValue >> 4)  & 0xf)) << 7;    // g invert
      final |= (15 - ((in_lmValue >> 0)  & 0xf)) << 1;    // b invert

      if (in_lmValue == 0xffff)
         final = 0x0;

      return final;
   }
}


void 
Munger565LM::translateLightmap(const GFXLightMap* in_pLightmap,
                               void*              out_pDDLightmap,
                               const Int32        in_dstStride)
{
   AssertFatal(in_pLightmap != NULL, "No lightmap passed to translateLightmap");
   AssertFatal(out_pDDLightmap != NULL, "No out_ptr passed to translateLightmap");

   Int32         numEntries = in_pLightmap->size.x * in_pLightmap->size.y;
   const UInt16* pEntries   = &in_pLightmap->data;
   UInt16*       pOutput    = static_cast<UInt16*>(out_pDDLightmap);

   if (in_dstStride == (in_pLightmap->size.x * 2)) {
      for (int i = 0; i < numEntries; i++) {
         pOutput[i] = mungeEntry(pEntries[i]);
      }
   } else {
      for (int i = 0; i < in_pLightmap->size.y; i++) {
         for (int j = 0; j < in_pLightmap->size.x; j++) {
            pOutput[j] = mungeEntry(pEntries[j]);
         }
         pEntries += in_pLightmap->size.x;
         pOutput  += in_dstStride / 2;
      }
   }
}


const D3DTexRenderState* 
Munger565LM::getRenderState() const
{
   return &renderState;
}




//------------------------------------------------------------------------------
//-------------------------------------- 16 Bit 565 Lightmap functions
//

Munger555LM::Munger555LM(const bool  in_lightMapInverted,
                         const DWORD in_lightMapMode)
 : lightMapInverted(in_lightMapInverted)
{
   renderState.destBlend = (D3DBLEND)in_lightMapMode;
}



Munger555LM::~Munger555LM()
{
   // NULL
}


bool 
Munger555LM::canTranslate(DDSURFACEDESC* pDDSD)
{
   // Note: this is the same as Munger565Normal's equivalent function

   AssertFatal(pDDSD != NULL, "null ptr in Direct3D::Munger565Normal::canTranslate()");

   if (((pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0) &&
       ((pDDSD->dwFlags & DDSCAPS_TEXTURE)  != 0)) {
      bool success = true;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
         success = false;
      
      // Must make sure we aren't looking at a 1555 format...
      //
      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) != 0)
         success = false;

      if (pDDSD->ddpfPixelFormat.dwRGBBitCount != 16)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwRBitMask != 0x7c00) ||
          (pDDSD->ddpfPixelFormat.dwGBitMask != 0x3e0) ||
          (pDDSD->ddpfPixelFormat.dwBBitMask != 0x1f)) {
         success = false;
      }

      if (success == true) {
         memcpy(&masterDDSDCopy, pDDSD, sizeof(DDSURFACEDESC));
         return true;
      }
   } 
   
   return false;
}


void 
Munger555LM::getSurfDesc(DDSURFACEDESC* out_pDDSD)
{
   // Note: this is the same as Munger565Normal's equivalent function

   AssertFatal(out_pDDSD != NULL,
               "null ptr in Direct3D::Munger565Normal::getSurfDesc()");

   // We assume that the DDSD is ours to do with what we want...
   //
   memcpy(out_pDDSD, &masterDDSDCopy, sizeof(DDSURFACEDESC));
}


inline UInt16 
Munger555LM::mungeEntry(UInt16 in_lmValue)
{
   // Note: In 565 LMaps, we have to make sure that full bright is
   //  actually full bright, since the shifts cause the lm to interpret
   //  0xffff as less than full intensity...
   //
   if (lightMapInverted == false) {
      UInt16 final;
      UInt16 r = (in_lmValue >> 8) & 0xf;
      UInt16 g = (in_lmValue >> 4) & 0xf;
      UInt16 b = (in_lmValue >> 0) & 0xf;

      final = (r << 11) |
              (g << 6)  |
              (b << 1);

      if (in_lmValue == 0xffff)
         final = 0x7fff;

      return final;
   } else {
      UInt16 final;
      final  = (15 - ((in_lmValue >> 8)  & 0xf)) << 11;   // r invert
      final |= (15 - ((in_lmValue >> 4)  & 0xf)) << 6;    // g invert
      final |= (15 - ((in_lmValue >> 0)  & 0xf)) << 1;    // b invert

      if (in_lmValue == 0xffff)
         final = 0x0;

      return final;
   }
}


void 
Munger555LM::translateLightmap(const GFXLightMap* in_pLightmap,
                               void*              out_pDDLightmap,
                               const Int32        in_dstStride)
{
   AssertFatal(in_pLightmap != NULL, "No lightmap passed to translateLightmap");
   AssertFatal(out_pDDLightmap != NULL, "No out_ptr passed to translateLightmap");

   Int32         numEntries = in_pLightmap->size.x * in_pLightmap->size.y;
   const UInt16* pEntries   = &in_pLightmap->data;
   UInt16*       pOutput    = static_cast<UInt16*>(out_pDDLightmap);

   if (in_dstStride == (in_pLightmap->size.x * 2)) {
      for (int i = 0; i < numEntries; i++) {
         pOutput[i] = mungeEntry(pEntries[i]);
      }
   } else {
      for (int i = 0; i < in_pLightmap->size.y; i++) {
         for (int j = 0; j < in_pLightmap->size.x; j++) {
            pOutput[j] = mungeEntry(pEntries[j]);
         }
         pEntries += in_pLightmap->size.x;
         pOutput  += in_dstStride / 2;
      }
   }
}


const D3DTexRenderState* 
Munger555LM::getRenderState() const
{
   return &renderState;
}



//------------------------------------------------------------------------------
//-------------------------------------- 16 Bit 444 Lightmap functions
//

Munger444LM::Munger444LM(const bool  in_lightMapInverted,
                         const DWORD in_lightMapMode)
 : lightMapInverted(in_lightMapInverted)
{
   renderState.destBlend = (D3DBLEND)in_lightMapMode;
}



Munger444LM::~Munger444LM()
{
   // NULL
}


bool 
Munger444LM::canTranslate(DDSURFACEDESC* pDDSD)
{
   // Adapted from Munger4444Alpha
   //
   AssertFatal(pDDSD != NULL, "null ptr in Direct3D::Munger565Normal::canTranslate()");

   if (((pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0) &&
       ((pDDSD->dwFlags & DDSCAPS_TEXTURE)  != 0)) {
      bool success = true;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) == 0)
         success = false;
      
      if (pDDSD->ddpfPixelFormat.dwRGBBitCount != 16)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwRBitMask != 0xf00) ||
          (pDDSD->ddpfPixelFormat.dwGBitMask != 0xf0)  ||
          (pDDSD->ddpfPixelFormat.dwBBitMask != 0xf)   ||
          (pDDSD->ddpfPixelFormat.dwRGBAlphaBitMask != 0xf000)) {
         success = false;
      }

      if (success == true) {
         memcpy(&masterDDSDCopy, pDDSD, sizeof(DDSURFACEDESC));
         return true;
      }
   }
   
   return false;
}


void 
Munger444LM::getSurfDesc(DDSURFACEDESC* out_pDDSD)
{
   AssertFatal(out_pDDSD != NULL,
               "null ptr in Direct3D::Munger565Normal::getSurfDesc()");

   // We assume that the DDSD is ours to do with what we want...
   //
   memcpy(out_pDDSD, &masterDDSDCopy, sizeof(DDSURFACEDESC));
}


inline UInt16 
Munger444LM::mungeEntry(UInt16 in_lmValue)
{
   if (lightMapInverted == false)
      return in_lmValue;
   else {
      UInt16 final;
      final  = (15 - ((in_lmValue >> 12) & 0xf)) << 12;  // alpha invert
      final |= (15 - ((in_lmValue >> 8)  & 0xf)) << 8;   // r invert
      final |= (15 - ((in_lmValue >> 4)  & 0xf)) << 4;   // g invert
      final |= (15 - ((in_lmValue >> 0)  & 0xf)) << 0;   // b invert

      return final;
   }
}


void 
Munger444LM::translateLightmap(const GFXLightMap* in_pLightmap,
                               void*              out_pDDLightmap,
                               const Int32        in_dstStride)
{
   AssertFatal(in_pLightmap != NULL, "No lightmap passed to translateLightmap");
   AssertFatal(out_pDDLightmap != NULL, "No out_ptr passed to translateLightmap");

   Int32         numEntries = in_pLightmap->size.x * in_pLightmap->size.y;
   const UInt16* pEntries   = &in_pLightmap->data;
   UInt16*       pOutput    = static_cast<UInt16*>(out_pDDLightmap);

   if (in_dstStride == (in_pLightmap->size.x * 2)) {
      for (int i = 0; i < numEntries; i++) {
         pOutput[i] = mungeEntry(pEntries[i]);
      }
   } else {
      for (int i = 0; i < in_pLightmap->size.y; i++) {
         for (int j = 0; j < in_pLightmap->size.x; j++) {
            pOutput[j] = mungeEntry(pEntries[j]);
         }
         pEntries += in_pLightmap->size.x;
         pOutput  += in_dstStride / 2;
      }
   }
}


const D3DTexRenderState* 
Munger444LM::getRenderState() const
{
   return &renderState;
}


//------------------------------------------------------------------------------
//-------------------------------------- 32 Bit 888 Lightmap functions
//

Munger888LM::Munger888LM(const bool  in_lightMapInverted,
                         const DWORD in_lightMapMode)
 : lightMapInverted(in_lightMapInverted)
{
   renderState.destBlend = (D3DBLEND)in_lightMapMode;
}



Munger888LM::~Munger888LM()
{
   // NULL
}


bool 
Munger888LM::canTranslate(DDSURFACEDESC* pDDSD)
{
   AssertFatal(pDDSD != NULL, "null ptr in Direct3D::Munger565Normal::canTranslate()");

   if (((pDDSD->dwFlags & DDSD_PIXELFORMAT) != 0) &&
       ((pDDSD->dwFlags & DDSCAPS_TEXTURE)  != 0)) {
      bool success = true;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_RGB) == 0)
         success = false;

      if (pDDSD->ddpfPixelFormat.dwRGBBitCount != 32)
         success = false;

      if ((pDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) == 0)
         success = false;
      
      if ((pDDSD->ddpfPixelFormat.dwRBitMask != 0xff0000) ||
          (pDDSD->ddpfPixelFormat.dwGBitMask != 0xff00)   ||
          (pDDSD->ddpfPixelFormat.dwBBitMask != 0xff)     ||
          (pDDSD->ddpfPixelFormat.dwRGBAlphaBitMask != 0xff000000)) {
         success = false;
      }

      if (success == true) {
         memcpy(&masterDDSDCopy, pDDSD, sizeof(DDSURFACEDESC));
         return true;
      }
   }
   
   return false;
}


void 
Munger888LM::getSurfDesc(DDSURFACEDESC* out_pDDSD)
{
   AssertFatal(out_pDDSD != NULL,
               "null ptr in Direct3D::Munger565Normal::getSurfDesc()");

   // We assume that the DDSD is ours to do with what we want...
   //
   memcpy(out_pDDSD, &masterDDSDCopy, sizeof(DDSURFACEDESC));
}


inline UInt32 
Munger888LM::mungeEntry(UInt16 in_lmValue)
{
   if (in_lmValue == 0xffff)
      if (lightMapInverted == false)
         return 0xffffffff;
      else
         return 0x00000000;

   UInt32 r, g, b, a;
   UInt32 final;

   r = (in_lmValue >> 8) & 0xf;
   g = (in_lmValue >> 4) & 0xf;
   b = (in_lmValue >> 0) & 0xf;
   a = (in_lmValue >> 12) & 0xf;

   if (lightMapInverted == false) {
      final = (a << 28) |
              (r << 20) |
              (g << 12) |
              (b <<  4);
   } else {
      final = ((15 - a) << 28) |
              ((15 - r) << 20) |
              ((15 - g) << 12) |
              ((15 - b) <<  4);
   }
   return final;
}


void 
Munger888LM::translateLightmap(const GFXLightMap* in_pLightmap,
                               void*              out_pDDLightmap,
                               const Int32        in_dstStride)
{
   AssertFatal(in_pLightmap != NULL, "No lightmap passed to translateLightmap");
   AssertFatal(out_pDDLightmap != NULL, "No out_ptr passed to translateLightmap");

   const UInt16* pEntries   = &in_pLightmap->data;
   UInt32*       pOutput    = static_cast<UInt32*>(out_pDDLightmap);

   for (int i = 0; i < in_pLightmap->size.y; i++) {
      for (int j = 0; j < in_pLightmap->size.x; j++) {
         pOutput[j] = mungeEntry(pEntries[j]);
      }
      pEntries += in_pLightmap->size.x;
      pOutput  += in_dstStride / 4;
   }
}


const D3DTexRenderState* 
Munger888LM::getRenderState() const
{
   return &renderState;
}


}; // namespace Direct3D