//
// babuild.cpp
//

#include <stdio.h>
#include <stdlib.h>

#include <core.h>
#include "g_bitmap.h"
#include "g_barray.h"

#define VERSION      "1.01"
#define OutStream    stdout
#define MAX_BITMAPS  2048

///////////////////////////////////////////////////////////////////////

void
printUsage()
{
   fprintf(OutStream, 
    "babuild:  invalid parameters\n"
    "\tUsage1:    babuild FIL0.BMP ... FILEn.BMP OUTFILE.PBA\n"
    "\tUsage2:    babuild @FILE.RSP OUTFILE.PBA\n\n"
    "1: The last argument specifies the filename of the bitmap array file\n"
    " that will be constructed from the bitmaps specified by the prior\n"
    " arguments.  E.g.\n\n"
    "    babuild x.bmp y.bmp z.bmp w.pba\n"
    " Concatenates bitmaps from the files x.bmp, y.bmp and z.bmp into a\n"
    " bitmap array, which is written to w.pba.\n\n"
    " Wildcards are accepted for BMP filenames.\n\n"
    "2: The argument preceded by @ specifies a response file, from which\n"
    " bitmap names will be loaded.  It is assumed that each bitmap name is\n"
    " separated by whitespace, though not necessarily CRs.  Wildcards are\n"
    " accepted. E.g.\n\n"
    " file.rsp contains:\n"
    "  file*.bmp other.bmp\n"
    "  new.bmp\n\n");
   exit(1);                      
}


void
processAndLoad(const char* in_pFilename,
               GFXBitmap** out_ppBmArray,
               int&        io_bmCount)
{
   if (strchr(in_pFilename,'*') || strchr(in_pFilename,'?')) {
      HANDLE h;
      WIN32_FIND_DATA findData;
      h = FindFirstFile(in_pFilename, &findData);

      if (h != INVALID_HANDLE_VALUE)
         while (io_bmCount < MAX_BITMAPS) {
            GFXBitmap* tmp;
            fprintf(OutStream, "%12s:  Reading...", findData.cFileName);
            tmp = GFXBitmap::load(findData.cFileName);
            if (tmp) {
               fprintf(OutStream, "%dx%d\n", tmp->width, tmp->height);
               out_ppBmArray[io_bmCount++] = tmp;          
            }
            else
               fprintf(OutStream, "failed\n");
            if (!FindNextFile(h, &findData)) break;
         }
      FindClose(h);
   } else {
      fprintf(OutStream, "%12s:  Reading...", in_pFilename);
      GFXBitmap* tmp = GFXBitmap::load(in_pFilename);
      if (tmp) {
         out_ppBmArray[io_bmCount++] = tmp;          
         fprintf(OutStream, "%dx%d\n", tmp->width, tmp->height);
      } else {
         fprintf(OutStream, "failed\n");
         exit (1);
      }
   }
}


void
loadFromResponseFile(const char*  in_pFilename,
                     GFXBitmap**  out_ppBmArray,
                     int&         out_rNumBmLoaded)
{
   const char* pTokenizers = " \t\r\n";

   // Filename still has '@' prepended...
   //
   FileRStream frs(in_pFilename+1);
   AssertFatal(frs.getStatus() == STRM_OK,
               "Error, could not open response file.");

   // Load the whole kit 'n' kaboodle...
   //
   char* respFileBuffer;
   UInt32 fileSize = frs.getSize();
   respFileBuffer = new char[fileSize];
   frs.read(fileSize, respFileBuffer);
   
   const char* pEnd = (respFileBuffer + fileSize);
   const char* currToken = strtok(respFileBuffer, pTokenizers);
   while (currToken != NULL && currToken < pEnd) {
      processAndLoad(currToken, out_ppBmArray, out_rNumBmLoaded);
      currToken = strtok(NULL, pTokenizers);
   }
   
   delete [] respFileBuffer;
}

int
main(int argc, char **argv)
{
   AssertSetFlags(1);
   fprintf(OutStream, "\nBabuld Version " VERSION "\n");

   if (argc < 3)
      // need at least one bmp and one pba name
      printUsage();

   int currentArg = 1, bitmapCt = 0;
   GFXBitmap **bitmaps = new GFXBitmap*[MAX_BITMAPS];
   GFXBitmap* tmp;

   HANDLE h;
   WIN32_FIND_DATA findData;

   if (argv[1][0] == '@') {
      loadFromResponseFile(argv[1], bitmaps, bitmapCt);
   } else {
      while (currentArg < argc - 1)
      {
         processAndLoad(argv[currentArg], bitmaps, bitmapCt);
         currentArg++;
      }
   }

   if (bitmapCt == MAX_BITMAPS)
      fprintf(OutStream, 
                "\nWarning: bitmap buffer full at %d bitmaps, consider increasing\n"
                "MAX_BITMAPS in babuild.cpp\n\n");
   
   if (bitmapCt > 0)
   {
      GFXBitmapArray bitmapArray;
      bitmapArray.numBitmaps = bitmapCt;
      bitmapArray.array      = bitmaps;

      fprintf(OutStream, "Writing %d bitmaps to %s...",
              bitmapCt, argv[argc - 1]);
      if (bitmapArray.write(argv[argc - 1], 0))
         fprintf(OutStream, "OK\n\n");
      else
         fprintf(OutStream, "FAILED!\n\n");
      bitmapArray.array = NULL;  // so destructor doesn't try to delete the static array
   }
   else
      printUsage();


   delete [] bitmaps;
   return(0);
}
