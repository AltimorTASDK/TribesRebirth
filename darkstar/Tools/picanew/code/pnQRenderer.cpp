//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <tString.h>
#include <g_bitmap.h>
#include <g_pal.h>
#include "pnQRenderer.h"
#include "pnUnquantizedImage.h"
#include "pnOptions.h"
#include "pnPseudoTrace.h"
#include "colorMatcher.h"

namespace {

UInt32
colorDifferenceRGB(const PALETTEENTRY& in_p1,
                   const PALETTEENTRY& in_p2)
{
   return ((int(in_p1.peRed)   - int(in_p2.peRed))   * (int(in_p1.peRed)   - int(in_p2.peRed)))   +
          ((int(in_p1.peGreen) - int(in_p2.peGreen)) * (int(in_p1.peGreen) - int(in_p2.peGreen))) +
          ((int(in_p1.peBlue)  - int(in_p2.peBlue))  * (int(in_p1.peBlue)  - int(in_p2.peBlue)));
}

bool
isSolidColor(const PALETTEENTRY* in_pPixels,
             const UInt32        in_x,
             const UInt32        in_y,
             const UInt32        in_width,
             const UInt32        in_height)
{
   const PALETTEENTRY& refPixel = in_pPixels[in_y * in_width + in_x];

   for (int yActual = int(in_y - 1); yActual <= int(in_y + 2); yActual++) {
      for (int xActual = int(in_x - 1); xActual <= int(in_x + 2); xActual++) {
         if (xActual < 0 || xActual >= int(in_width) || yActual == 0 || yActual >= int(in_height))
            continue;

         const PALETTEENTRY& cmpPixel = in_pPixels[yActual * in_width + xActual];

         if (memcmp(&cmpPixel, &refPixel, sizeof(PALETTEENTRY)) != 0)
            return false;
      }
   }

   return true;
}

void
distributeError(PALETTEENTRY* out_pPixels,
                const UInt32  in_x,
                const UInt32  in_y,
                const UInt32  in_width,
                const UInt32  in_height,
                const ColorF& in_error)
{
   // Dither error diffusion is 5/16 to down,
   //                           7/16 to right
   //                           1/16 to right down
   //                           3/16 to left  down
   
   // Right: 7/16
   if (in_x < (in_width - 1)) {
      PALETTEENTRY& rPixel = out_pPixels[in_y * in_width + (in_x + 1)];
      int newR = rPixel.peRed;
      int newG = rPixel.peGreen;
      int newB = rPixel.peBlue;
      newR += int((in_error.red   * (7.0f/16.0f)) + 0.5f);
      newG += int((in_error.green * (7.0f/16.0f)) + 0.5f);
      newB += int((in_error.blue  * (7.0f/16.0f)) + 0.5f);

      if (newR < 0) newR = 0; if (newR > 255) newR = 255;
      if (newG < 0) newG = 0; if (newG > 255) newG = 255;
      if (newB < 0) newB = 0; if (newB > 255) newB = 255;

      rPixel.peRed   = UInt8(newR);
      rPixel.peGreen = UInt8(newG);
      rPixel.peBlue  = UInt8(newB);
   }
   // down: 5/16
   if (in_y < (in_height - 1)) {
      PALETTEENTRY& rPixel = out_pPixels[(in_y + 1) * in_width + in_x];
      int newR = rPixel.peRed;
      int newG = rPixel.peGreen;
      int newB = rPixel.peBlue;
      newR += int((in_error.red   * (5.0f/16.0f)) + 0.5f);
      newG += int((in_error.green * (5.0f/16.0f)) + 0.5f);
      newB += int((in_error.blue  * (5.0f/16.0f)) + 0.5f);

      if (newR < 0) newR = 0; if (newR > 255) newR = 255;
      if (newG < 0) newG = 0; if (newG > 255) newG = 255;
      if (newB < 0) newB = 0; if (newB > 255) newB = 255;

      rPixel.peRed   = UInt8(newR);
      rPixel.peGreen = UInt8(newG);
      rPixel.peBlue  = UInt8(newB);
   }
   // right down: 1/16
   if (in_y < (in_height - 1) && in_x < (in_width - 1)) {
      PALETTEENTRY& rPixel = out_pPixels[(in_y + 1) * in_width + (in_x + 1)];
      int newR = rPixel.peRed;
      int newG = rPixel.peGreen;
      int newB = rPixel.peBlue;
      newR += int((in_error.red   * (5.0f/16.0f)) + 0.5f);
      newG += int((in_error.green * (5.0f/16.0f)) + 0.5f);
      newB += int((in_error.blue  * (5.0f/16.0f)) + 0.5f);

      if (newR < 0) newR = 0; if (newR > 255) newR = 255;
      if (newG < 0) newG = 0; if (newG > 255) newG = 255;
      if (newB < 0) newB = 0; if (newB > 255) newB = 255;

      rPixel.peRed   = UInt8(newR);
      rPixel.peGreen = UInt8(newG);
      rPixel.peBlue  = UInt8(newB);
   }
   // left down: 3/16
   if (in_y < (in_height - 1) && in_x > 0) {
      PALETTEENTRY& rPixel = out_pPixels[(in_y + 1) * in_width + (in_x - 1)];
      int newR = rPixel.peRed;
      int newG = rPixel.peGreen;
      int newB = rPixel.peBlue;
      newR += int((in_error.red   * (5.0f/16.0f)) + 0.5f);
      newG += int((in_error.green * (5.0f/16.0f)) + 0.5f);
      newB += int((in_error.blue  * (5.0f/16.0f)) + 0.5f);

      if (newR < 0) newR = 0; if (newR > 255) newR = 255;
      if (newG < 0) newG = 0; if (newG > 255) newG = 255;
      if (newB < 0) newB = 0; if (newB > 255) newB = 255;

      rPixel.peRed   = UInt8(newR);
      rPixel.peGreen = UInt8(newG);
      rPixel.peBlue  = UInt8(newB);
   }
}

int
calcShift(int x)
{
   int result = -1;
   do {
      x >>= 1;
      result++;
   }while(x);
   return (result);
}   

void
reextendFileNameAndPath(const String& in_rFileName,
                        const String& in_rNewExtension,
                        const String& in_rNewPath,
                        String&       out_rNewFileName)
{
   out_rNewFileName = in_rFileName;
   String::iterator itr;
   for (itr = out_rNewFileName.end(); itr != out_rNewFileName.begin(); --itr) {
      if (*itr == '.')
         break;
   }
   if (itr == out_rNewFileName.begin() ||
       (out_rNewFileName.end() - itr) < in_rNewExtension.length()) {
      // There was no extension, or not enough room for the new one...
      //
      char tmpBuf[512];
      strcpy(tmpBuf, out_rNewFileName.c_str());
      strcat(tmpBuf, in_rNewExtension.c_str());
      out_rNewFileName = tmpBuf;
   } else {
      char* pExt = itr;
      strcpy(pExt, in_rNewExtension.c_str());
   }

   // Ok, now the filename is re-extended, let's strip off the old path (if any), and add
   //  on the new...
   //
   for (itr = out_rNewFileName.end(); itr != (out_rNewFileName.begin() - 1); --itr) {
      if (*itr == '\\')
         break;
   }
   
   if (itr == (out_rNewFileName.begin() - 1)) {
      // No old path, simply cat on the new...
      //
      char tmpBuf[512];
      strcpy(tmpBuf, in_rNewPath.c_str());
      strcat(tmpBuf, out_rNewFileName.c_str());
      out_rNewFileName = tmpBuf;
   } else {
      char tmpBuf[512];
      char* pRoot = itr + 1;
      strcpy(tmpBuf, in_rNewPath.c_str());
      strcat(tmpBuf, pRoot);
      out_rNewFileName = tmpBuf;
   }
}

} // namespace {}


//------------------------------------------------------------------------------
//--------------------------------------
//
PNQuantizedRenderer::PNQuantizedRenderer(const PNOptions&  in_rOptions,
                                         GFXPalette*       in_pPalette)
 : m_pPalette(in_pPalette),
   m_paletteKey(in_rOptions.getPaletteKey()),
   m_forcePhoenix(in_rOptions.getForcePhoenix())
{
   in_rOptions.getRenderRange(m_renderFirst, m_renderLast);

   ColorF test;
   m_areDithering = in_rOptions.areDithering() && (in_rOptions.getZeroColor(test) == false);
   if (m_areDithering == true)
      m_ditherTolerance = in_rOptions.getDitherTolerance();

   if (in_rOptions.getColorSpace() != PNOptions::ColorSpaceAlpha) {
      m_pMatchObject = new RGBColorMatcher();
   } else {
      AssertFatal(m_areDithering == false, "Can't dither an alpha!");
      m_pMatchObject = new AlphaColorMatcher();
   }
   
   m_pMatchObject->setMatchPalette(m_pPalette, m_renderFirst, m_renderLast);

   ColorF zeroColor;
   if (in_rOptions.getZeroColor(zeroColor) == true) {
      m_areZeroing = true;
      m_zeroColor.peRed   = UInt8((zeroColor.red   * 255.0f) + 0.5f);
      m_zeroColor.peGreen = UInt8((zeroColor.green * 255.0f) + 0.5f);
      m_zeroColor.peBlue  = UInt8((zeroColor.blue  * 255.0f) + 0.5f);

      m_pMatchObject->setZeroColor(m_zeroColor);
   } else {
      m_areZeroing = false;
      memset(&m_zeroColor, 0, sizeof(PALETTEENTRY));
   }

   m_areEdgeMaintaining = in_rOptions.areMaintainingEdges();

   m_outputDir = in_rOptions.getOutputPath();

   m_extrudingMipLevels = in_rOptions.getExtrudingMipLevels();
   m_inAlphaMode        = in_rOptions.getColorSpace() == PNOptions::ColorSpaceAlpha;
   m_inAdditiveMode     = in_rOptions.getColorSpace() == PNOptions::ColorSpaceAdditive;
   m_inSubtractiveMode  = in_rOptions.getColorSpace() == PNOptions::ColorSpaceSubtractive;

   ColorF dummy;
   m_transparentRender  = in_rOptions.getZeroColor(dummy);

}

PNQuantizedRenderer::~PNQuantizedRenderer()
{
   m_pPalette = NULL;

   delete m_pMatchObject;
   m_pMatchObject = NULL;
}

void
PNQuantizedRenderer::renderUQImageDithered(const PNUnquantizedImage* in_pRawImage)
{
   AssertFatal(in_pRawImage != NULL, "Huh?");

   const PALETTEENTRY* pPixels = in_pRawImage->getFinalPixels();

   // Setup the output bitmap...
   //
   GFXBitmap* pOutputBitmap = GFXBitmap::create(in_pRawImage->getWidth(),
                                                in_pRawImage->getHeight());
   AssertFatal(pOutputBitmap != NULL, "Unable to create bitmap...");

   // Save off the palette key...
   //
   pOutputBitmap->paletteIndex = m_paletteKey;
   if (m_inAlphaMode == true)
      pOutputBitmap->attribute |= BMA_TRANSLUCENT;
   if (m_transparentRender == true)
      pOutputBitmap->attribute |= BMA_TRANSPARENT;
   if (m_inAdditiveMode == true)
      pOutputBitmap->attribute |= BMA_ADDITIVE;
   else if (m_inSubtractiveMode == true)
      pOutputBitmap->attribute |= BMA_SUBTRACTIVE;

   // If we are zeroing, we need to mark zeroColor entries as exempt from dithering...
   //
   bool* dontRemap = new bool[in_pRawImage->getWidth() * in_pRawImage->getHeight()];
   memset(dontRemap, 0, (sizeof(bool) * in_pRawImage->getWidth() *
                                        in_pRawImage->getHeight()));
   if (m_areZeroing == true) {
      for (UInt32 i = 0; i < in_pRawImage->getWidth() * in_pRawImage->getHeight(); i++) {
         if (pPixels[i].peRed   == m_zeroColor.peRed   &&
             pPixels[i].peGreen == m_zeroColor.peGreen &&
             pPixels[i].peBlue  == m_zeroColor.peBlue) {
            dontRemap[i] = true;
         }
      }
   }

   // If we aren't dithering the edges, mark that here...
   //
   if (m_areEdgeMaintaining == true) {
      for (UInt32 i = 0; i < in_pRawImage->getWidth(); i++) {
         dontRemap[i] = true;
         dontRemap[(in_pRawImage->getWidth() * (in_pRawImage->getHeight() - 1)) + i] = true;
      }
      for (UInt32 i = 0; i < in_pRawImage->getHeight(); i++) {
         dontRemap[i * in_pRawImage->getWidth()] = true;
         dontRemap[(i * in_pRawImage->getWidth()) + (in_pRawImage->getWidth() - 1)] = true;
      }
   }

   // Make a copy of the rawImage's paletteEntries...
   //
   PALETTEENTRY* ditherPixels = new PALETTEENTRY[in_pRawImage->getWidth() *
                                                 in_pRawImage->getHeight()];
   memcpy(ditherPixels, pPixels, (sizeof(PALETTEENTRY)     *
                                  in_pRawImage->getWidth() *
                                  in_pRawImage->getHeight()));

   for (UInt32 y = 0; y < in_pRawImage->getHeight(); y++) {
      UInt8* currDestRow = pOutputBitmap->getAddress(0, y);

      for (UInt32 x = 0; x < in_pRawImage->getWidth(); x++) {
         const PALETTEENTRY& currSrcPixel = ditherPixels[y * in_pRawImage->getWidth() + x];
         const PALETTEENTRY& currRawPixel = pPixels[y * in_pRawImage->getWidth() + x];
         const PALETTEENTRY* usedPixel    = &currSrcPixel;
         UInt8& currDestPixel             = currDestRow[x];

         if (dontRemap[y * in_pRawImage->getWidth() + x] == true && m_areZeroing == true) {
            currDestPixel = 0;
            continue;
         }

         UInt8 ditheredLookup = m_pMatchObject->matchColor(currSrcPixel);

         if ((colorDifferenceRGB(currSrcPixel, m_pPalette->palette[0].color[ditheredLookup]) > m_ditherTolerance) ||
             (isSolidColor(in_pRawImage->getFinalPixels(),
                           x, y,
                           in_pRawImage->getWidth(), in_pRawImage->getHeight()) == true) ||
             dontRemap[y * in_pRawImage->getWidth() + x] == true) {
            ditheredLookup = m_pMatchObject->matchColor(currRawPixel);
            usedPixel = &currRawPixel;
         }

         currDestPixel = ditheredLookup;

         ColorF errorColor;
         errorColor.red   = (float(usedPixel->peRed) -
                             float(m_pPalette->palette[0].color[ditheredLookup].peRed));
         errorColor.green = (float(usedPixel->peGreen) -
                             float(m_pPalette->palette[0].color[ditheredLookup].peGreen));
         errorColor.blue  = (float(usedPixel->peBlue) -
                             float(m_pPalette->palette[0].color[ditheredLookup].peBlue));

         distributeError(ditherPixels,
                         x, y,
                         in_pRawImage->getWidth(), in_pRawImage->getHeight(),
                         errorColor);
      }
   }

   delete [] ditherPixels;
   delete [] dontRemap;

   if (m_extrudingMipLevels == true) {
      if (extrudeBitmapMipLevels(pOutputBitmap) == false) {
         PseudoTrace& tracer = PseudoTrace::getTracerObject();
         tracer.setErrorState();
         tracer << "PNQRenderer: problems extruding miplevels for: "
                << in_pRawImage->getImageName()
                << ".  Possible invalid size\n";
         exit(1);
      }
   }
   
   String originalName = in_pRawImage->getImageName();
   String newExtension = ".bmp";
   String newName;
   reextendFileNameAndPath(originalName, newExtension, m_outputDir, newName);

   if (pOutputBitmap->getNumDetailLevels() != 1 || m_forcePhoenix) {
      pOutputBitmap->write(newName.c_str(), 0);
   } else {
      pOutputBitmap->pPalette = m_pPalette;
      pOutputBitmap->write(newName.c_str(), BMF_INCLUDE_PALETTE | BMF_MICROSOFT);
      pOutputBitmap->pPalette = NULL;
   }

   delete pOutputBitmap;
}

void
PNQuantizedRenderer::renderUQImage(const PNUnquantizedImage* in_pRawImage)
{
   if (m_areDithering == true) {
      renderUQImageDithered(in_pRawImage);
      return;
   }

   AssertFatal(in_pRawImage != NULL, "Huh?");

   const PALETTEENTRY* pPixels = in_pRawImage->getFinalPixels();

   // Setup the output bitmap...
   //
   GFXBitmap* pOutputBitmap = GFXBitmap::create(in_pRawImage->getWidth(),
                                                in_pRawImage->getHeight());
   AssertFatal(pOutputBitmap != NULL, "Unable to create bitmap...");

   // Save off the palette key...
   //
   pOutputBitmap->paletteIndex = m_paletteKey;

   if (m_inAlphaMode == true)
      pOutputBitmap->attribute |= BMA_TRANSLUCENT;
   if (m_transparentRender == true)
      pOutputBitmap->attribute |= BMA_TRANSPARENT;
   if (m_inAdditiveMode == true)
      pOutputBitmap->attribute |= BMA_ADDITIVE;
   if (m_inSubtractiveMode == true)
      pOutputBitmap->attribute |= BMA_SUBTRACTIVE;

   UInt32 currentIndex  = 0;
   for (UInt32 y = 0; y < in_pRawImage->getHeight(); y++) {
      UInt8* currentRow = pOutputBitmap->getAddress(0, y);
      for (UInt32 x = 0; x < in_pRawImage->getWidth(); x++) {
         const PALETTEENTRY& rEntry = pPixels[currentIndex++];

         *(currentRow++) = m_pMatchObject->matchColor(rEntry);
      }
   }

   if (m_extrudingMipLevels == true) {
      if (extrudeBitmapMipLevels(pOutputBitmap) == false) {
         PseudoTrace& tracer = PseudoTrace::getTracerObject();
         tracer.setErrorState();
         tracer << "PNQRenderer: problems extruding miplevels for: "
                << in_pRawImage->getImageName()
                << ".  Possible invalid size\n";
         exit(1);
      }
   }
   
   String originalName = in_pRawImage->getImageName();
   String newExtension = ".bmp";
   String newName;
   reextendFileNameAndPath(originalName, newExtension, m_outputDir, newName);

   if (pOutputBitmap->getNumDetailLevels() != 1 || m_forcePhoenix) {
      pOutputBitmap->write(newName.c_str(), 0);
   } else {
      pOutputBitmap->pPalette = m_pPalette;
      pOutputBitmap->write(newName.c_str(), BMF_INCLUDE_PALETTE | BMF_MICROSOFT);
      pOutputBitmap->pPalette = NULL;
   }

   delete pOutputBitmap;
}

bool
PNQuantizedRenderer::extrudeBitmapMipLevels(GFXBitmap* io_pBitmap)
{
   UInt8* pOriginalBits = io_pBitmap->pBitsBase;
   UInt8* pOriginalBitsReal = io_pBitmap->pBits;
   UInt32 width         = io_pBitmap->width;
   UInt32 height        = io_pBitmap->height;

   // Make sure this is a power of 2...
   if (((width  & (width  - 1)) != 0) ||
       ((height & (height - 1)) != 0) ||
       width < 4 || height < 4) {
      return false;
   }

   // Recalculate the size we'll need, and set up the pMipBits...
   //
   UInt32 calcWidth  = width;
   UInt32 calcHeight = height;
   UInt32 numDetailLevels = 0;
   UInt32 sizeRequired    = 0;
   do {
      io_pBitmap->pMipBits[numDetailLevels] = (UInt8*)sizeRequired;
      numDetailLevels++;

      sizeRequired += calcWidth * calcHeight;
      
      if (calcWidth == 1 && calcHeight == 1)
         break;

      if (calcWidth != 1)
         calcWidth >>= 1;
      if (calcHeight != 1)
         calcHeight >>= 1;
   } while (true);

   if (numDetailLevels > 9)
      return false;

   // Ok, reallocate the pBits for the bitmap, and set up the miplevel pointers,
   //  round the array up to the nearest dword...
   //
   sizeRequired = (sizeRequired + 3) & ~3;
   io_pBitmap->pBits        = new UInt8[sizeRequired];
   io_pBitmap->pBitsBase    = io_pBitmap->pBits;
   io_pBitmap->imageSize    = sizeRequired;
   io_pBitmap->detailLevels = numDetailLevels;

   for (UInt32 i = 0; i < numDetailLevels; i++) {
      io_pBitmap->pMipBits[i] = UInt32(io_pBitmap->pMipBits[i]) + io_pBitmap->pBits;
   }

   // Copy the original bits into the first miplevel...
   //
   memcpy(io_pBitmap->pMipBits[0], pOriginalBitsReal, (width * height));

   for (UInt32 i = 1; i < numDetailLevels; i++) {
      UInt8* prevBits   = io_pBitmap->pMipBits[i - 1];
      UInt32 prevWidth  = width  >> (i - 1);
      UInt32 prevHeight = height >> (i - 1);
      if (prevWidth == 0)
         prevWidth = 1;
      if (prevHeight == 0)
         prevHeight = 1;

      UInt8* currBits   = io_pBitmap->pMipBits[i];
      UInt32 currWidth  = width  >> i;
      UInt32 currHeight = height >> i;
      if (currWidth == 0)
         currWidth = 1;
      if (currHeight == 0)
         currHeight = 1;

      for (UInt32 yBase = 0; yBase < currHeight; yBase++) {
         for (UInt32 xBase = 0; xBase < currWidth; xBase++) {
            UInt32 r, g, b, a;
            r = 0; g = 0; b = 0; a = 0;

            UInt32 computedColors = 0;
            for (int y = 0; y < 2; y++) {
               for (int x = 0; x < 2; x++) {
                  UInt32 xActual, yActual;
                  yActual = 2 * yBase + y;
                  xActual = 2 * xBase + x;

                  // This ONLY happens when the mip levels have collapsed to a single
                  //  pixel in one dimension...
                  //
                  if (yActual >= prevHeight ||
                      xActual >= prevWidth)
                     continue;

                  UInt8 lookUpVal = prevBits[yActual * prevWidth + xActual];
                  if (lookUpVal == 0 && m_areZeroing)
                     continue;
                  
                  const PALETTEENTRY& palEntry = m_pPalette->palette[0].color[lookUpVal];
                  computedColors++;
                  r += palEntry.peRed;
                  g += palEntry.peGreen;
                  b += palEntry.peBlue;
                  a += palEntry.peFlags;
               }
            }
            AssertFatal(computedColors > 1 || m_areZeroing, "Shouldn't ever happen...");

            if (computedColors > 1) {
               PALETTEENTRY dummyPEntry;
               dummyPEntry.peRed   = float(r) / float(computedColors);
               dummyPEntry.peGreen = float(g) / float(computedColors);
               dummyPEntry.peBlue  = float(b) / float(computedColors);
               dummyPEntry.peFlags = float(a) / float(computedColors);
            
               currBits[yBase * currWidth + xBase] = m_pMatchObject->matchColor(dummyPEntry);
            } else {
               currBits[yBase * currWidth + xBase] = 0;
            }
         }
      }
   }

   // Ok, we're set!  Delete the original bits attached to the object, and get outta here...
   // 
   delete [] pOriginalBits;

   return true;
}