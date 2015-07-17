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

#include "mpmState.h"
#include "mpmMerger.h"
#include "mpmOptions.h"
#include "mpmFileList.h"
#include "mpmPopTable.h"

namespace {

const char* const sg_pProgramVersion = "1.3c";

void
usage()
{
   cout << "USAGE:\n"
        << " mpMerger [options] @<inputFileName>\n"
        << " Options:\n"
        << "  -v%%d   Verbosity level [1] (0: silent, 4: maximal debug msgs)\n"
        << "\n";
}

bool
processCommandLine(const int   in_argc,
                   const char* in_argv[])
{
   bool haveValidOptionFile = false;
   
   MPMGlobalState* pGlobalState = MPMGlobalState::getGlobalState();
   
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

} // namespace {}


int
main(int         in_argc,
     const char* in_argv[])
{
   MPMGlobalState::initGlobalState();

   if (processCommandLine(in_argc, in_argv) == false) {
      cout << "MPMerger v" << sg_pProgramVersion << '\n'
           << " MultiPalette Merger\n"
           << " @ Dynamix 1998, Original author Dave Moore\n\n";
      usage();
      return EXIT_FAILURE;
   }

   MPMGlobalState* pGlobalState = MPMGlobalState::getGlobalState();

   // Set up the options for this run...
   //
   MPMOptions  instOptions;           // Options for this run
   MPMFileList procFileList;          // List of files to process
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
   AssertFatal(procFileList.getNumFiles() <= MAX_MULTI_PALETTES, "Too many palettes, exiting...");

   // Whether we are choosing colors or rendering, we need the unquantized palettes,
   //  so load them into memory...
   //
   for (int i = 0; i < procFileList.getNumFiles(); i++) {
      MPMFileEntry& rFileEntry = procFileList.getFile(i);
      AssertFatal(rFileEntry.pRawPalette == NULL, "Huh?");
      AssertFatal(rFileEntry.pFileName   != NULL, "Huh?");

      rFileEntry.pRawPalette = GFXPalette::load(rFileEntry.pFileName);

      if (rFileEntry.pRawPalette == NULL) {
         cerr << "main(): Unable to load file: " << rFileEntry.pFileName << '\n';
         exit(1);
      }
   }

   // We will need an authoritative final palette either way as well...
   //
   GFXPalette* pFinalMasterPalette = new GFXPalette;
   memset(pFinalMasterPalette->palette[0].color, 0, sizeof(PALETTEENTRY) * 256);
   if (instOptions.hasBasePalette() == true) {
      const char* pPaletteName = instOptions.getBasePalName();
      AssertFatal(pPaletteName != NULL, "No base palette name?  Here?");

      Bool palLoadSuccess = pFinalMasterPalette->read(pPaletteName);
      if (palLoadSuccess == false) {
         cerr << "Unable to load base palette: " << pPaletteName << ". Exiting...\n";
         exit(1);
      }
   }

   // We are always choosing colors for the master palette, so create a Popularity Table,
   //  and register our colors...
   //
   MPMPopularityTable popTable(instOptions);
   popTable.prepFixedColors(instOptions, pFinalMasterPalette);
   bool haveSomeRealColors = false;
   for (int i = 0; i < procFileList.getNumFiles(); i++) {
      MPMFileEntry& rFileEntry = procFileList.getFile(i);
      AssertFatal(rFileEntry.pRawPalette != NULL, "Huh?");

      if (rFileEntry.pRawPalette->palette[0].paletteType == GFXPalette::ColorQuantPaletteType) {
         printf(".");
         popTable.registerRGBColors(rFileEntry.pRawPalette, rFileEntry.fileWeight);
         haveSomeRealColors = true;
      } else
         printf("*");
   }

   // If we have NO files, we're just copying from the base palette to the new palette...
   if (procFileList.getNumFiles() == 0) {
      
   } else {
      AssertFatal(haveSomeRealColors == true, "Must have a least _one_ non-alpha palette...");
      
      // Quantize out the master palette..
      popTable.quantizeColors(instOptions, pFinalMasterPalette);
   }

   // Insert the mandatory entries into the master palette.  These are:
   //  palette[0].type  = HazeShadePaletteType
   //  palette[0].index = DWORD(-1)
   //           color[0]: (0, 0, 0)
   //         color[255]: (1, 1, 1)
   //         color[254]: instOptions.hazeColor
   //          hazeColor: 254
   //       correct bitvector entries.  renderStart-Last + 0, 254, 254
   //
   PALETTEENTRY hazeColor;
   PALETTEENTRY black, white;
   black.peRed = black.peGreen = black.peBlue = black.peFlags = 0;
   white.peRed = white.peGreen = white.peBlue = 255;
   white.peFlags = 0;
   hazeColor.peRed   = UInt8((instOptions.getHazeColor().red   * 255.0f) + 0.5f);
   hazeColor.peGreen = UInt8((instOptions.getHazeColor().green * 255.0f) + 0.5f);
   hazeColor.peBlue  = UInt8((instOptions.getHazeColor().blue  * 255.0f) + 0.5f);
   hazeColor.peFlags = 0;
   pFinalMasterPalette->palette[0].color[0]     = black;
   pFinalMasterPalette->palette[0].color[255]   = white;
   pFinalMasterPalette->palette[0].color[254]   = hazeColor;
   pFinalMasterPalette->palette[0].paletteIndex = DWORD(-1);
   pFinalMasterPalette->palette[0].paletteType  = GFXPalette::ShadeHazePaletteType;
   pFinalMasterPalette->hazeColor = 254;

   UInt32 renderFirst, renderLast;
   instOptions.getMasterRenderRange(renderFirst, renderLast);
   pFinalMasterPalette->allowedColorMatches.zero();
   for (UInt32 i = renderFirst; i <= renderLast; i++)
      pFinalMasterPalette->allowedColorMatches.set(i);
   pFinalMasterPalette->allowedColorMatches.set(0);
//   pFinalMasterPalette->allowedColorMatches.set(255);
   pFinalMasterPalette->allowedColorMatches.set(254);

   // Insert the shadow palette...
   //
   pFinalMasterPalette->palette[1].paletteIndex = -2;
   pFinalMasterPalette->palette[1].paletteType  = GFXPalette::TranslucentPaletteType;
   for (int i = 0; i < 256; i++) {
      pFinalMasterPalette->palette[1].color[i].peRed   = 0;
      pFinalMasterPalette->palette[1].color[i].peGreen = 0;
      pFinalMasterPalette->palette[1].color[i].peBlue  = 0;
      pFinalMasterPalette->palette[1].color[i].peFlags = i;
   } 
   pFinalMasterPalette->numPalettes++;

   // Ok, now we need to map the multipalettes into the master palette...
   //
   MPMMerger mpmMerger(instOptions);
   for (int i = 0; i < procFileList.getNumFiles(); i++) {
      MPMFileEntry& rFileEntry = procFileList.getFile(i);
      mpmMerger.insertPalette(pFinalMasterPalette, rFileEntry.pRawPalette);
   }

   // And create the remap tables if necessary...
   //
   mpmMerger.createRemapTables(pFinalMasterPalette);

   // And we are DONE!  Write out the master palette to the specified name...
   //
   // First make sure that in no palettes any color other than 0 is fully black
   //
   for (int i = 0; i < pFinalMasterPalette->numPalettes; i++) {
      GFXPalette::MultiPalette& rMPalette = pFinalMasterPalette->palette[i];

      for (int j = 1; j < 256; j++) {
         PALETTEENTRY& rPEntry = rMPalette.color[j];
         if (rPEntry.peRed   == 0 &&
             rPEntry.peGreen == 0 &&
             rPEntry.peBlue  == 0)
            rPEntry.peBlue = 1;
      }
   }

   AssertFatal(instOptions.getOutputPalName() != NULL, "Error, no output name!");
   pFinalMasterPalette->write(instOptions.getOutputPalName());

   //-------------------------------------- Clean-up
   delete pFinalMasterPalette;
   MPMGlobalState::shutdownGlobalState();
   return EXIT_SUCCESS;
}
