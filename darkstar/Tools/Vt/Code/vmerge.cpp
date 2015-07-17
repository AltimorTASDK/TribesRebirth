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

#include <resManager.h>
#include <volStrm.h>

namespace {

char const* const sg_versionString = "1.0";

void
usage()
{
   cout << "vMerge v" << sg_versionString << "\n"
        << " Usage: vmerge <volOne> <volTwo>\n"
        << "  Merges files from volTwo into volOne\n";
}

void
copyStreams(VolumeRWStream& output,
            VolumeRStream&  input)
{
   AssertFatal(output.getSize() == input.getSize(), "Size mismatch");
   
   char* pCopyBuffer = new char[input.getSize()];
   input.read(input.getSize(), pCopyBuffer);
   output.write(input.getSize(), pCopyBuffer);
   
   delete [] pCopyBuffer;
}

};


int
main(int argc, char* argv[])
{
   if (argc != 3) {
      usage();
      return EXIT_FAILURE;
   }

   VolumeRStream  inputVol;
   VolumeRWStream outputVol;

   bool test = inputVol.openVolume(argv[2]);
   AssertISV(test != false, avar("Could not open input volume: %s", argv[2]));
   
   test = outputVol.openVolume(argv[1]);
   AssertISV(test != false, avar("Could not open output volume: %s", argv[1]));

   FindMatch fileMatches("*", 1024);
   inputVol.findMatches(&fileMatches);
   AssertWarn(fileMatches.isFull() == false,
              "Exceeded max merge size (1024) files have been left out");

   for (int i = 0; i < fileMatches.numMatches(); i++) {
      const char* pFileName = fileMatches.matchList[i];
      
      // Open the stream from the input volume
      //
      bool testOpen = inputVol.open(pFileName);
      AssertFatal(testOpen == true, "Something is horribly wrong...");
      
      outputVol.open(pFileName, STRM_COMPRESS_NONE, inputVol.getSize());
      
      copyStreams(outputVol, inputVol);
      outputVol.close();
      inputVol.close();
   }

   outputVol.closeVolume();
   inputVol.closeVolume();

   return EXIT_SUCCESS;
}