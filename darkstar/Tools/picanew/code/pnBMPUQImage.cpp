//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <g_bitmap.h>

#include "pnPseudoTrace.h"
#include "pnBMPUQImage.h"
#include "pnPopTable.h"


PNBMPUQImage::PNBMPUQImage()
 : m_pFinalPixels(NULL)
{
   
}

PNBMPUQImage::~PNBMPUQImage()
{
   delete [] m_pFinalPixels;
   m_pFinalPixels = NULL;
}

const PALETTEENTRY*
PNBMPUQImage::getFinalPixels() const
{
   return m_pFinalPixels;
}

void
PNBMPUQImage::registerColors(PNPopularityTable& in_rPopTable,
                             const UInt32       in_weight)
{
   AssertFatal(m_pFinalPixels != NULL, "No data to register...");

   in_rPopTable.registerRGBColors(m_pFinalPixels,
                                  getWidth() * getHeight(),
                                  in_weight);
}

bool
PNBMPUQImage::loadFile(const char* in_pFileName)
{
   AssertFatal(in_pFileName != NULL, "No bitmap fileName pointer supplied.");

   PseudoTrace& tracer = PseudoTrace::getTracerObject();

   GFXBitmap* pBitmap = GFXBitmap::load(in_pFileName);

   if (pBitmap == NULL) {
      tracer.pushVerbosityLevel(1);
      tracer << "BMP Loader unable to load file: " << in_pFileName << "\n";
      tracer.popVerbosityLevel();
      return false;
   }
   tracer.pushVerbosityLevel(4);
   tracer << "BMP Loader successfully loaded file: " << in_pFileName << "\n";
   tracer.popVerbosityLevel();

   if (pBitmap->getBitDepth() != 24) {
      tracer.pushVerbosityLevel(1);
      tracer << "BMP Loader (" << in_pFileName << ") file is not 24 bit.\n"
             << "File is " << pBitmap->getBitDepth() << "bit, unable to quantize\n";
      tracer.popVerbosityLevel();
      return false;
   }

   setImageName(in_pFileName);
   setImageDimensions(pBitmap->getWidth(), pBitmap->getHeight());

   m_pFinalPixels = new PALETTEENTRY[getWidth() * getHeight()];

   UInt32 currentIndex = 0;
   for (UInt32 y = 0; y < getHeight(); y++) {
      BYTE* currentRow = pBitmap->getAddress(0, y);
      for (UInt32 x = 0; x < getWidth(); x++) {
         m_pFinalPixels[currentIndex].peBlue  = *(currentRow++);
         m_pFinalPixels[currentIndex].peGreen = *(currentRow++);
         m_pFinalPixels[currentIndex].peRed   = *(currentRow++);
         currentIndex++;
      }
   }
   AssertFatal(currentIndex == getWidth() * getHeight(), "Something is screwy...");

   delete pBitmap;
   return true;
}

