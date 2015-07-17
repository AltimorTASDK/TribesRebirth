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
#include <iostream.h>

#include <base.h>
#include <g_pal.h>

#include "pnState.h"
#include "pnOptions.h"
#include "pnFileList.h"
#include "pnPopTable.h"
#include "pnUnquantizedImage.h"
#include "pnUQImageFactory.h"
#include "pnQRenderer.h"
#include "pnPseudoTrace.h"

namespace {

const char* const sg_pProgramVersion = "1.3d";

void
usage()
{
   cout << "USAGE:\n"
        << " picaNew [options] @<inputFileName>\n"
        << " Options:\n"
        << "  -v%%d   Verbosity level [1] (0: silent, 4: maximal debug msgs)\n"
        << "\n";
}

bool
processCommandLine(const int   in_argc,
                   const char* in_argv[])
{
   bool haveValidOptionFile = false;
   
   PNGlobalState* pGlobalState = PNGlobalState::getGlobalState();
   
   for (int currArg = 1; currArg < in_argc; currArg++) {
      const char* currArgString = in_argv[currArg];

      switch (currArgString[0]) {
         case '-': {
            switch (currArgString[1]) {
               case 'v':
               case 'V': {
                  int verbosityLevel = atol(&currArgString[2]);
                  pGlobalState->setVerbosityLevel(verbosityLevel);
               }
               break;

               default:
                cerr << "Invalid option: " << currArgString << '\n';
                return false;
            }
         }
         break;

         case '@': {
            const char* responseFile = &currArgString[1];
            pGlobalState->setResponseFileName(responseFile);
            haveValidOptionFile = true;
         }
         break;

        default:
         cerr << "Invalid option: " << currArgString << "\n\n";
         return false;
      }
   }

   return haveValidOptionFile;
}

}


int
main(int argc, const char* argv[])
{
   PNGlobalState::initGlobalState();

   if (processCommandLine(argc, argv) == false) {
      cout << "PicaNew v" << sg_pProgramVersion << '\n'
           << " Based on Pica by Joel Franklin which was\n"
           << " Based on DynaColor by Rhett Anderson\n"
           << " Glorious Thanks and Praise to the PNG and ZLib Folks...\n"
           << " @ Dynamix 1998, Original author Dave Moore\n\n";
      usage();
      return EXIT_FAILURE;
   }

   PNGlobalState* pGlobalState = PNGlobalState::getGlobalState();

   PNOptions  instOptions;           // Options for this run
   PNFileList procFileList;          // List of files to process

   if (instOptions.attachResponseFile(pGlobalState->getResponseFileName()) == false) {
      cerr << "Unable to open response file: "
           << pGlobalState->getResponseFileName() << '\n'
           << " - ABORTING - \n";
      return EXIT_FAILURE;
   }

   if (instOptions.validateOptions() == false) {
      cerr << "Invalid options detected, exiting...\n\n";
      exit(1);
   }

   instOptions.processResponseFile(procFileList);

   // Whether we are choosing colors or rendering, we need the unquantized images,
   //  so load them into memory...
   //
   PNUQImageFactory& rUQFactory = PNUQImageFactory::getInstance();
   for (int i = 0; i < procFileList.getNumFiles(); i++) {
      PNFileEntry& rFileEntry = procFileList.getFile(i);
      AssertFatal(rFileEntry.pUQImage  == NULL, "Huh?");
      AssertFatal(rFileEntry.pFileName != NULL, "Huh?");

      rFileEntry.pUQImage = rUQFactory.createUQImage(rFileEntry.pFileName);

      if (rFileEntry.pUQImage == NULL) {
         cerr << "main(): Unable to load file: " << rFileEntry.pFileName << '\n';
         exit(1);
      }
   }

   // We will need an authoritative final palette either way as well...
   //
   GFXPalette* pAuthPalette = new GFXPalette;
   memset(pAuthPalette->palette[0].color, 0, sizeof(PALETTEENTRY) * 256);
   if (instOptions.hasBasePalette() == true) {
      const char* pPaletteName = instOptions.getBasePalName();
      AssertFatal(pPaletteName != NULL, "No base palette name?  Here?");

      if (strstr(pPaletteName, ".tap") == NULL &&
          strstr(pPaletteName, ".TAP") == NULL) {
         Bool palLoadSuccess = pAuthPalette->read(pPaletteName);
         if (palLoadSuccess == false) {
            cerr << "Unable to load base palette: " << pPaletteName << ". Exiting...\n";
            exit(1);
         }
      } else {
         FILE *fp;
         if((fp = fopen(pPaletteName, "rb")) == NULL) {
             cerr << "ERROR - Cannot read Palette from : " << pPaletteName << "\n";
             exit(0);
         }

         for (int i = 0; i < 256; i++) {
            int red, green, blue, alpha;
            fscanf(fp, "%d %d %d %d", &red,
                                      &green,
                                      &blue,
                                      &alpha);
            pAuthPalette->palette[0].color[i].peRed   = UInt8(red);
            pAuthPalette->palette[0].color[i].peGreen = UInt8(green);
            pAuthPalette->palette[0].color[i].peBlue  = UInt8(blue);
            pAuthPalette->palette[0].color[i].peFlags = UInt8(alpha);
         }
         fclose(fp);
      }
   }
   // Always, always, always, the first color must be all 0
   //
   memset(&(pAuthPalette->palette[0].color[0]), 0, sizeof(PALETTEENTRY));


   // If we are choosing colors, set up the popularity table, and go to it...
   //
   if (instOptions.areChoosingColors() == true && procFileList.getNumFiles() != 0) {
      PNPopularityTable popularityTable(instOptions);

      // Files must register their colors with the popularity table..
      //
      PseudoTrace& tracer = PseudoTrace::getTracerObject();
      tracer.pushVerbosityLevel(1);
      tracer << "Scanning:  ";
      
      for (int i = 0; i < procFileList.getNumFiles(); i++) {
         PNFileEntry& rFileEntry = procFileList.getFile(i);
         AssertFatal(rFileEntry.pUQImage != NULL, "Huh?");

         rFileEntry.pUQImage->registerColors(popularityTable,
                                             rFileEntry.fileWeight);
         tracer << '.';
         tracer.flush();
      }
      tracer << " done\n";
      tracer.popVerbosityLevel();

      // Set up any fixed colors that may be required...
      //
      popularityTable.prepFixedColors(instOptions, pAuthPalette);

      // And quantize away!
      //
      popularityTable.quantizeColors(instOptions, pAuthPalette);
   }

   // If we are rendering the bitmaps, set up a color matching ditherer, and
   //  render away!
   //
   if (instOptions.areRenderingBitmaps() == true && procFileList.getNumFiles() != 0) {
      PNQuantizedRenderer pnRenderer(instOptions, pAuthPalette);

      PseudoTrace& tracer = PseudoTrace::getTracerObject();
      tracer.pushVerbosityLevel(1);
      tracer << "Rendering: ";
      
      for (int i = 0; i < procFileList.getNumFiles(); i++) {
         PNFileEntry& rFileEntry = procFileList.getFile(i);
         AssertFatal(rFileEntry.pUQImage != NULL, "Huh?");

         pnRenderer.renderUQImage(rFileEntry.pUQImage);

         if (instOptions.getExtrudingMipLevels() == false)
            tracer << '.';
         else
            tracer << '*';
         tracer.flush();
      }
      tracer << " done\n";
      tracer.popVerbosityLevel();

      // Store off the computed weights into the Palette...
      //
      const UInt32* pChosenFreqs = pnRenderer.getChosenFreqs();
      if (pAuthPalette->hasQuantizationInfo() == false)
         pAuthPalette->initQuantizationInfo();

      for (int i = 0; i < 256; i++)
         pAuthPalette->m_pColorWeights[i] += float(pChosenFreqs[i]);

      UInt32 renderStart, renderEnd;
      instOptions.getRenderRange(renderStart, renderEnd);
      pAuthPalette->m_usedRangeStart = renderStart;
      pAuthPalette->m_usedRangeEnd   = renderEnd;
   }

   // Write out the palette, and we're finished!
   //
   if (instOptions.getStripQuantizationInfo() == true)
      pAuthPalette->stripQuantizationInfo();

   if (instOptions.areChoosingColors() == true) {
      // First copy in the unique paletteKey, which we stuff into index 0
      //
      pAuthPalette->palette[0].paletteIndex = instOptions.getPaletteKey();

      // Now, technically, we're lying a bit about which type of palette this is,
      //  but the mpMerger tool will fix this up for us...
      //
      if (instOptions.getColorSpace() == PNOptions::ColorSpaceAlpha) {
         pAuthPalette->palette[0].paletteType = GFXPalette::AlphaQuantPaletteType;
      } else if (instOptions.getColorSpace() == PNOptions::ColorSpaceAdditive) {
         pAuthPalette->palette[0].paletteType = GFXPalette::AdditiveQuantPaletteType;
      } else if (instOptions.getColorSpace() == PNOptions::ColorSpaceSubtractive) {
         pAuthPalette->palette[0].paletteType = GFXPalette::SubtractiveQuantPaletteType;
      } else {
         pAuthPalette->palette[0].paletteType = GFXPalette::ColorQuantPaletteType;
      }

      if (instOptions.isStandAlonePalette() == true) {
         pAuthPalette->palette[0].paletteType = GFXPalette::NoRemapPaletteType;
      }

      for (int i = 1; i < 256; i++) {
         PALETTEENTRY& rPEntry = pAuthPalette->palette[0].color[i];
         if (rPEntry.peRed   == 0 &&
             rPEntry.peGreen == 0 &&
             rPEntry.peBlue  == 0)
            rPEntry.peBlue = 1;
      }

      // Write out our auth palette to the specified output name...
      //
      const char* pPaletteName = instOptions.getOutputPalName();
      pAuthPalette->write(pPaletteName);
   } else if (instOptions.areRenderingBitmaps() == true) {
      // Just write the palette back out to the base palette...
      //
      AssertFatal(instOptions.hasBasePalette() == true,
                  "If we aren't choosing colors, we must have a base palette...");
      pAuthPalette->write(instOptions.getBasePalName());
   } else {
      // Write out our auth palette to the specified output name...
      //
      pAuthPalette->palette[0].paletteIndex = instOptions.getPaletteKey();
      const char* pPaletteName = instOptions.getOutputPalName();
      if (pPaletteName != NULL)
         pAuthPalette->write(pPaletteName);
   }

   // Cleanup
   delete pAuthPalette;
   instOptions.unattachResponseFile();
   PNGlobalState::shutdownGlobalState();
   return EXIT_SUCCESS;
}

