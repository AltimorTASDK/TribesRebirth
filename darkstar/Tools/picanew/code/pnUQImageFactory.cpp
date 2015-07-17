//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <string.h>

#include "pnPseudoTrace.h"
#include "pnUQImageFactory.h"
#include "pnBMPUQImage.h"
#include "pnPNGUQImage.h"

PNUQImageFactory PNUQImageFactory::sm_theUQImageFactory;

PNUQImageFactory::PNUQImageFactory()
{

}

PNUQImageFactory::~PNUQImageFactory()
{

}

PNUQImageFactory&
PNUQImageFactory::getInstance()
{
   return sm_theUQImageFactory;
}

PNUnquantizedImage*
PNUQImageFactory::createUQImage(const char* in_pFileName)
{
   PseudoTrace& tracer = PseudoTrace::getTracerObject();
   PNUnquantizedImage* pImage = NULL;

   if (strstr(in_pFileName, ".tga") != NULL ||
       strstr(in_pFileName, ".TGA") != NULL ||
       strstr(in_pFileName, ".Tga") != NULL) {
      AssertFatal(0, "Unsupported as of yet...");

   } else if (strstr(in_pFileName, ".bmp") != NULL ||
              strstr(in_pFileName, ".BMP") != NULL ||
              strstr(in_pFileName, ".Bmp") != NULL) {
      pImage = new PNBMPUQImage;
      if (pImage->loadFile(in_pFileName) == true) {
         tracer.pushVerbosityLevel(3);
         tracer << "PNUQImageFactory: successfully created image for: "
                << in_pFileName << '\n';
         tracer.popVerbosityLevel();
      } else {
         tracer.pushVerbosityLevel(3);
         tracer << "PNUQImageFactory: unsuccessful in creating image for: "
                << in_pFileName << '\n';
         tracer.popVerbosityLevel();
         
         delete pImage;
         pImage = NULL;
      }
   } else if (strstr(in_pFileName, ".png") != NULL ||
              strstr(in_pFileName, ".PNG") != NULL ||
              strstr(in_pFileName, ".Png") != NULL) {
      pImage = new PNPNGUQImage;
      if (pImage->loadFile(in_pFileName) == true) {
         tracer.pushVerbosityLevel(3);
         tracer << "PNUQImageFactory: successfully created image for: "
                << in_pFileName << '\n';
         tracer.popVerbosityLevel();
      } else {
         tracer.pushVerbosityLevel(3);
         tracer << "PNUQImageFactory: unsuccessful in creating image for: "
                << in_pFileName << '\n';
         tracer.popVerbosityLevel();
         
         delete pImage;
         pImage = NULL;
      }
   }

   return pImage;
}


