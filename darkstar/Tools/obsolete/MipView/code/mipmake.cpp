// 
// mipmake.cpp
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

#include <g_pal.h>
#include <g_bitmap.h>
#include <g_barray.h>

#define VERSION          1.01
#define OutStream        stdout
#define sq(x)            ((x)*(x))
#define String_Eq(x,y)   (stricmp((x),(y))==0)
   
#define Is_PPL_File(x)   (String_Eq(filenameExtension(x), "PPL"))
#define Is_PBA_File(x)   (String_Eq(filenameExtension(x), "PBA"))
#define Is_BMP_File(x)   (String_Eq(filenameExtension(x), "BMP"))
         
int         desiredDetailLevels = -1;    // "-1" indicates use max levels
GFXPalette *globalPalette       = NULL;

#define  MAX_RANGES  10    // only allow 10 range settings on palette
struct range
{
   int   min;
   int   max;
}palRange[MAX_RANGES];
int rangeIndex = 0;

///////////////////////////////////////////////////////////////////////

char *filenameExtension(char *filename)
{
   char *f = filename;

   while (*f) f++;                  // go to the end
   if (!*f) f--;                    // back up one char
   while (f != filename)
      if (*f == '.') break;        // look for '.'
      else f--;
   f++;
   return(f);
}   

///////////////////////////////////////////////////////////////////////

char *toUppercase(char *s)
{
   char *t = s;
   while (*t) *t++ = toupper(*t);
   return(s);
}

///////////////////////////////////////////////////////////////////////
         
void printUsage()
{
   cout << "MipMake Version 3.0.                                                         " << endl;
   cout << "Copyright (c) 1996,97 Dynamix, Inc.  All rights reserved.                    " << endl;
   cout << "                                                                             " << endl;
   cout << "Usage:    mipmake [options] <bitMap1> [<bitMap...n>]                         " << endl;
   cout << "                                                                             " << endl;
   cout << "Options( -o -i -l -p -r ):                                                   " << endl;
   cout << " -oFile.bmp     Where File.bmp is the name of the new output file.           " << endl;
   cout << "                if it is omitted, the original is overwritten.               " << endl;
   cout << " -in            Where n is a number to be appended to the output file        " << endl;
   cout << "                name.  If multiple files are mipmapped, this number is       " << endl;
   cout << "                incremented for each new output file.  If this value is      " << endl;
   cout << "                omitted and there are multiple files to mipmap, it is        " << endl;
   cout << "                assumed to be 0.                                             " << endl;
   cout << " -ln            Where n is the number of mipmap levels to create.  If this   " << endl;
   cout << "                value is omitted, it will generate mipmaps down to 1 pixel.  " << endl;
   cout << " -pFile.ppl     Where File.ppl is a Phoenix palette to use during the        " << endl;
   cout << "                mipmap process.  If this file is omitted, the palette        " << endl;
   cout << "                within the bitmap is used.                                   " << endl;
   cout << " -r(min,max)    Where min,max is the range of colors to use in the           " << endl;
   cout << "                mipmap process.  Ex. -r(128,207) will only use colors        " << endl;
   cout << "                128 through 207 to create the mip levels.  You may           " << endl;
   cout << "                specify more than one range up to 10 ranges.                 " << endl;
   cout << "                                                                             " << endl;
   cout << "                Multiple bitmap files and wildcards are allowed              " << endl;
   cout << "                                                                             " << endl;
   cout << "  BITMAP ARRAYS:  Bitmap array mipmaps may be built as well, simply by       " << endl;
   cout << "  including a bitmap array filename as one of the bitmap arguments.          " << endl;
   cout << "  When a bitmap array is mipmapped, n bitmap array files are written, each   " << endl;
   cout << "  containing all the bitmaps from the original array at one level of detail. " << endl;
   cout << "  The naming convention is as follows, detail level 1 is not written, since  " << endl;
   cout << "  it is the same as the original bitmap array.  Remaining detail levels are  " << endl;
   cout << "  written with the original bitmap array filename plus a single digit        " << endl;
   cout << "  specifying the detail level.  For example, if the original bitmap array    " << endl;
   cout << "  filename is \ROCKS.PBA\ then detail level 2 will be named \ROCKS2.PBA\ etc." << endl;
   exit(1);
}

///////////////////////////////////////////////////////////////////////

int log2Test(int n)
{
   // if n is a power of 2 then return log2(n), otherwise return -1
   int log2n = 0;
   while (((n & 1) != 1) && (n != 0))
   {
      log2n++;
      n >>= 1;
   }
   if (n == 1)
      return(log2n);
   else
      return(-1);
}

///////////////////////////////////////////////////////////////////////
GFXBitmap *processMipmaps(GFXBitmap *bitmap, GFXPalette *palette, int detailLevels)
{
   // Generate a new bitmap where all the mipmaps will go
   GFXBitmap *nBitmap = new GFXBitmap;
   nBitmap->width        = bitmap->width;
   nBitmap->height       = bitmap->height;
   nBitmap->stride       = bitmap->stride;
   nBitmap->bitDepth     = bitmap->bitDepth;  // better be 8!
   nBitmap->pPalette     = bitmap->pPalette;
   nBitmap->attribute    = bitmap->attribute;
   nBitmap->detailLevels = detailLevels;

   // allocate enough memory for all mipmaps and setup the
   // pMipbits array
   int requiredBytes  = 0, i;
   int currentMipSize = nBitmap->width*nBitmap->height;
   for (i = 0; i < detailLevels; i++)
   {
      requiredBytes  += currentMipSize;
      currentMipSize /= 4;
   }
   nBitmap->imageSize   = requiredBytes + (requiredBytes % 4);  // round to nearest DWORD
   nBitmap->pBits       = new BYTE[requiredBytes];
   nBitmap->pMipBits[0] = nBitmap->pBits;
   currentMipSize       = nBitmap->width*nBitmap->height;

   // Setup mipmap pointers into pBits data
   for (i = 1; i < detailLevels; i++)
   {
      nBitmap->pMipBits[i] = nBitmap->pMipBits[i - 1] + currentMipSize;
      currentMipSize /= 4;
   }      
   int width = nBitmap->width, height = nBitmap->height;

   // Finally, begin generating each level of detail
   fprintf(OutStream, "1");

   // zeroth detail level is simply a copy of the original bitmap
   for (i = 0; i < width*height; i++)
      nBitmap->pBits[i] = bitmap->pBits[i];

   PALETTEENTRY palEntry, *color = palette->color;      
   int level, palIndex, x, y, u, u0, u1, v, v1, r, g, b;
   width  /= 2;
   height /= 2;

   BYTE *previousBits;
   int   previousWidth;

   // generate the maps for each level of detail
   for (level = 1; level < detailLevels; level++)
   {
      previousBits  = nBitmap->pMipBits[level - 1];
      previousWidth = 2*width;

      fprintf(OutStream, "%d", level + 1);
      i = 0;
      // visit each pixel (x,y pair) in the target mipmap
      for (y = 0; y < height; y++)
         for (x = 0; x < width; x++)
         {
            r = g = b = 0;
            u0 = 2*x;
            u1 = 2*(x + 1);
            v1 = 2*(y + 1);
            for (v = 2*y; v < v1; v++)
               for (u = u0; u < u1; u++)
               {
                  palIndex = previousBits[v*previousWidth + u];            
                   r += color[palIndex].peRed;
                   g += color[palIndex].peGreen;
                   b += color[palIndex].peBlue;
               }
            // average the intensities for this target pixel over the number
            // of samples taken from the original bitmap
            r >>= 2;   // divide ea. by four, the number of samples
            g >>= 2;
            b >>= 2;
            // find the closest valid color to the desired color for this 
            // target pixel
             nBitmap->pMipBits[level][i++] = palette->GetNearestColor(r, g, b);
         }
      width  /= 2;
      height /= 2;
   }
   return(nBitmap);
}

///////////////////////////////////////////////////////////////////////

GFXBitmap *generateMipmaps(GFXBitmap *bitmap)
{
   GFXPalette *palette;
   fprintf(OutStream, "%dx%d, ", bitmap->width, bitmap->height);

   // if no global palette was specified, then the bitmap must have
   // one...
   if (globalPalette)
      palette = globalPalette;
   else if (bitmap->pPalette)      
      palette = bitmap->pPalette;
   else
   {
      fprintf(OutStream, "no palette, skipped\n");
      return(NULL);
   }

   // set valid palette ranges
   if ( rangeIndex )   // if specified, clear all settings first
         palette->allowedColorMatches.zero();
   for ( int i=0; i<rangeIndex; i++ )
      palette->allowedColorMatches.set(palRange[i].min,palRange[i].max);

   // verify various attributes of the bitmap
   if (bitmap->bitDepth != 8)
   {
      fprintf(OutStream, "not 8 bit, skipped\n");
      return(NULL);
   }
   // verify that the bitmap is of a power of two, and at the same
   // time, find out how many levels of detail are required to get
   // to one pixel
   int maxDetailLevels = 1;
   int log2Width       = log2Test(bitmap->width);
   int log2Height      = log2Test(bitmap->height);
   if ((log2Width == -1) || (log2Height == -1))
   {
      fprintf(OutStream, "size is not a power of 2, skipped\n");
      return(NULL);
   }
   maxDetailLevels += min(log2Width, log2Height);

   // desiredDetailLevels -  no. specified by user, -1 for max
   // maxDetailLevels     -  dtl lvls to take width or height to 1 pixel
   // detailLevels        -  final dtl lvls accounting for bounds
   int detailLevels;
   if (desiredDetailLevels == -1)
      detailLevels = maxDetailLevels;  // carry detail lvls to 1 pixel
   else
      detailLevels = min(desiredDetailLevels, maxDetailLevels);
   detailLevels = max(1, min(detailLevels, BMA_MAX_MIPMAPS));

   if (detailLevels == 1)
   {
      fprintf(OutStream, "no work for one detail level, skipping\n");
      return(NULL);
   }
   fprintf(OutStream, "%d levels..", detailLevels);

   GFXBitmap *newBitmap = processMipmaps(bitmap, palette, detailLevels);
   newBitmap->pPalette = bitmap->pPalette;
   bitmap->pPalette    = NULL;       // so it doesn't get deleted twice

   return(newBitmap);
}

///////////////////////////////////////////////////////////////////////

int processBMPFile(char *bitmapFilename, char *outputFilename)
{
   if (!outputFilename) outputFilename = bitmapFilename;

   GFXBitmap *bitmap = GFXBitmap::load(bitmapFilename, BMF_INCLUDE_PALETTE);
   if (!bitmap)
   {
      fprintf(OutStream, "unable to load, skipped\n");
      return(0);
   }
   GFXBitmap *newBitmap = generateMipmaps(bitmap);
   if (!newBitmap)
   {
      printf("MipMake:  Unable to generate mipmap for %s", bitmapFilename);
      AssertMessage( 0, avar("MipMake:  Unable to generate mipmap for %s",bitmapFilename) );
      exit(1);
   }
   fprintf(OutStream, "-->%12s..", outputFilename);
   int returnValue;
   if (newBitmap->write(outputFilename, 1234))
   {
      returnValue = 1;
      fprintf(OutStream, "OK\n");
   }
   else
   {
      returnValue = 0;
      fprintf(OutStream, "FAILED\n");
   }
   delete newBitmap;
   delete bitmap;

   return(returnValue);
}                                    

///////////////////////////////////////////////////////////////////////

void figurePBAFilename(char *out, char *in, int n)
{
   // decide on a reasonable filename for the nth level of detail of 
   // the pba file named by in

   char prefix[32];
   int  i = 0;
   while (in[i] != '.' && in[i] != '\0')
   {
      prefix[i] = in[i];
      i++;
   }
   prefix[i] = '\0';
   
   sprintf(out, "%s.%d.PBA", prefix, n);
}

///////////////////////////////////////////////////////////////////////

int processPBAFile(char *bitmapArrayFilename)
{
   GFXBitmapArray *bitmaps = GFXBitmapArray::load(bitmapArrayFilename, 0);
   if (!bitmaps)
   {   
      fprintf(OutStream, "unable to load, skipped\n");
      return(0);
   }

   int i, processedCt = 0, maxDetailLevels = 0;
   GFXBitmap *newBitmap, *tmp;

   fprintf(OutStream, "Array of %d bitmaps:\n", bitmaps->numBitmaps);

   // generate mipmaps for each bitmap in the array
   for (i = 0; i < bitmaps->numBitmaps; i++)
   {
      fprintf(OutStream, " Element %3d:   ", i, bitmaps->array[i]->detailLevels);
      newBitmap = generateMipmaps(bitmaps->array[i]);
      if (newBitmap)
      {
         processedCt++;
         maxDetailLevels = max(maxDetailLevels, int(newBitmap->detailLevels));
         // swap out the un-mipmapped bitmap
         tmp = bitmaps->array[i];
         bitmaps->array[i] = newBitmap;
         delete tmp;
         fprintf(OutStream, " done\n");
      }
   }

   // Because the sizes of the individual bitmaps may vary, the
   // numbers of levels of detail generated on each may vary.

   // For each level of detail, generate a bitmap array and extract
   // that level of detail from each original bitmap, then write
   // out the bitmap array.
   
   int   level;
   char  nBitmapFilename[16000];
   GFXBitmapArray *nBitmaps;
   nBitmaps             = new GFXBitmapArray;
   nBitmaps->numBitmaps = bitmaps->numBitmaps;
   nBitmaps->array      = new GFXBitmap*[nBitmaps->numBitmaps];
   for (i = 0; i < nBitmaps->numBitmaps; i++)
      nBitmaps->array[i] = NULL;

   // For each level of detail, generate a temporary bitmap array.  
   // Skip the zeroth level, since it is identicial to the input pba
   for (level = 1; level < maxDetailLevels; level++)
   {
      int levelToUse;
      GFXBitmap *bitmap, *nBitmap;

      // for each bitmap in the array, extract out the current detail level
      for (i = 0; i < nBitmaps->numBitmaps; i++)
      {
      	bitmap  = bitmaps->array[i];   // mipmapped version
         if (level < bitmap->detailLevels)      
            levelToUse = level;
         else
            // the bitmap doesn't have this many detail levels, so we'll have
            // to improvise by using the nearest detail level
            levelToUse = bitmap->detailLevels - 1;

         nBitmap = GFXBitmap::create(bitmap->width  >> levelToUse,
                                     bitmap->height >> levelToUse,
                              bitmap->bitDepth);
         nBitmap->attribute = bitmap->attribute;
         memcpy(nBitmap->pBits, bitmap->pMipBits[levelToUse], nBitmap->imageSize);

         // now stash this bitmap in the new array
         nBitmaps->array[i] = nBitmap;
      }

      // the bitmap array for this level is now generated, so write it out
      figurePBAFilename(nBitmapFilename, bitmapArrayFilename, level);      
      fprintf(OutStream, "%12s:  Writing level %d to %12s...", 
              bitmapArrayFilename, level, nBitmapFilename);

      if (nBitmaps->write(nBitmapFilename, 0))
         fprintf(OutStream, "OK\n");
      else
         fprintf(OutStream, "FAILED\n");

      // now get rid of all the temporary bitmaps we just generated for this level
      for (i = 0; i < bitmaps->numBitmaps; i++)
      {
         delete nBitmaps->array[i];
         nBitmaps->array[i] = NULL;
      }
   }

   delete nBitmaps;
   delete bitmaps;

   return(processedCt);
}

///////////////////////////////////////////////////////////////////////

int makeMipmaps(char *bitmapFilename, char *outputFilename)
{
   fprintf(OutStream, "%12s:  ", bitmapFilename);
   if (Is_BMP_File(bitmapFilename))
      return(processBMPFile(bitmapFilename, outputFilename));
   else if (Is_PBA_File(bitmapFilename))
      return(processPBAFile(bitmapFilename));

   fprintf(OutStream, "unfamiliar with file extension\n");
   return(0);
}

///////////////////////////////////////////////////////////////////////
     
int main(int argc, char *argv[])
{
   HANDLE            h;
   WIN32_FIND_DATA   findData;
   char              bmpOutFile[256];
   char              *bmpFile[4096];  // process up to 4096 bitmaps
   char              *outputFilename = NULL;
   char              path[256];
   int               outputFileNum  = 0;
   bool              incOutput = false;
   int               currentArg=0;                // current argument being parsed
   int               i;
   int               bitmapCt = 0;

   fprintf(OutStream, avar("\nMipmake Version %.2f\n",VERSION));
   if (argc < 2)
      printUsage();

   // change string arguments to all uppercase
   for ( i=0; i<argc; i++ )
      toUppercase( argv[i] );

   while ( ++currentArg < argc )
   {
      char *token = argv[currentArg];
      char *arg = token+2;
      switch( FOURCC(0,0,token[0],token[1]) )
      {
         case FOURCC(0,0,'-','O'):  // outfile
            outputFilename = arg;
            break;

         case FOURCC(0,0,'-','I'):  // index for temp file
            incOutput = true;
            outputFileNum = atoi( arg );
            break;

         case FOURCC(0,0,'-','L'):  // max # of levels to create
            desiredDetailLevels = atoi( arg );
            if (desiredDetailLevels > BMA_MAX_MIPMAPS)
               printUsage();
            break;

         case FOURCC(0,0,'-','P'):  // Phoenix palette (optional)
            if (Is_PPL_File(arg))
            {
               // load the palette
               printf("%s:  ", arg);
               globalPalette = GFXPalette::load(arg);
               if (!globalPalette)
               {
                  AssertFatal(0, "Error loading palette, only understand .ppl format");
                  exit(1);
               }      
               printf("loaded\n");
            }
            break;

         case FOURCC(0,0,'-','R'):  // allowed palette range(s) (optional)
            palRange[rangeIndex].min = atoi( strtok(arg,"(,") );
            palRange[rangeIndex].max = atoi( strtok(NULL,",") );
            printf("Palette Range %d:  %d - %d\n", rangeIndex, 
               palRange[rangeIndex].min, palRange[rangeIndex].max);
            rangeIndex++;
            break;

         default:    // otherwise a .bmp or .bma
            {
               strcpy(path,token);
               char *p;
               if( (p = strrchr(path,'\\')) != NULL )
               {
                  p++;
                  *p=0;
               }
               else
                  path[0]=0;

               // allow for wildcards by using find/next file
               h = FindFirstFile(token, &findData);
               if (h != INVALID_HANDLE_VALUE)
                  while (1)
                  {
                     if ( path[0] )
                        bmpFile[bitmapCt++] = strnew(avar("%s\\%s",path,findData.cFileName));
                     else
                        bmpFile[bitmapCt++] = strnew(findData.cFileName);
                     if (!FindNextFile(h, &findData))
                        break;
                  }
               FindClose(h);
            }
            break;
      }
   }   

   for ( int i=0; i<bitmapCt; i++ )
   {
      if ( !outputFilename )
         makeMipmaps( bmpFile[i], bmpFile[i] ); // overwrite existing file
      else
      {
         if ( incOutput ) 
            sprintf( bmpOutFile, "%s%i", outputFilename, outputFileNum++ );
         else
            strcpy( bmpOutFile, outputFilename );
         makeMipmaps( bmpFile[i], bmpOutFile ); // output to temp file
      }
   }

   printf("\n%d bitmaps processed\n", bitmapCt);
   return(0);
}                        
