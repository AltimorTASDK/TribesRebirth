#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include "types.h"
#include "g_pal.h"
#include "table.h"

void LoadPalRGBA( const char *, PALETTEENTRY *);
void LoadPalRGBA( const char *, PALETTEENTRY *);
struct RiffHdr
{
   char  hdr[4];
   int   fsize;
   char  typ[4];
   char  dta[4];
   int   dsize;
   int   version;
};

#define  MAX_RANGES  10    // only allow 10 range settings on palette
struct range
{
   int   min;
   int   max;
}palRange[MAX_RANGES];
int rangeIndex = 0;

void LoadPalRGB( const char *fname, PALETTEENTRY *pal)
{

   PALETTEENTRY tmpPal [256];
   FILE *fp;
   RiffHdr palette;

   if ((fp = fopen(fname,"rb")) == NULL)
   {
      printf("can't open file ");
      exit(0);
   }

   fread ( &palette, sizeof(palette), 1, fp );
   fread ( &tmpPal, sizeof(PALETTEENTRY), 256, fp );

   for ( int i=0; i<256; i++ )
   {
      pal[i].peBlue  = tmpPal[i].peRed;
      pal[i].peGreen = tmpPal[i].peGreen;
      pal[i].peRed   = tmpPal[i].peBlue;
      pal[i].peFlags = NULL;
   }


   fclose(fp);
//   return(0);
}

void LoadPalRGBA( const char *fname, PALETTEENTRY *pal)
{
   PALETTEENTRY tmpPal [256];
   FILE *fp;
   RiffHdr palette;

   if ((fp = fopen(fname,"rb")) == NULL)
   {
      printf("can't open file ");
      exit(0);
   }

   fread ( &palette, sizeof(palette), 1, fp );
   fread ( &tmpPal, sizeof(PALETTEENTRY), 256, fp );

   for ( int i=0; i<256; i++ )
   {
      pal[i].peBlue  = tmpPal[i].peRed;
      pal[i].peGreen = tmpPal[i].peGreen;
      pal[i].peRed   = tmpPal[i].peBlue;
      pal[i].peFlags = tmpPal[i].peFlags;
   }


   fclose(fp);
}


main(int argc, char **argp)
{
    PALETTEENTRY RGBPal[256];
    PALETTEENTRY RGBAPal[256];
    GFXPalette outPal;
    int index;
    UBYTE *lookupTable;

    if(argc < 4)
    {
      cout << "MPal Version 1.1.                                                         " << endl;
      cout << "Copyright (c) 1996,97 Dynamix, Inc.  All rights reserved.                    " << endl;
      cout << "                                                                             " << endl;
      cout << "Usage:    MPal RGB.pal RGBA.pal output.pal [r(min,max)]                      " << endl;
      cout << "                                                                             " << endl;
      cout << " RGB.pal    -  A GFXPalette                                                  " << endl;
      cout << " RGBA.pal   -  A GFXPalette                                                  " << endl;
      cout << " ouput.pal  -  The resultant merged GFXPalette                               " << endl;
      cout << " r(min,max) -  Optional, sets allowed color match range on final palette     " << endl;
      exit(1);
    }
    outPal.read(argp[1]);
    LoadPalRGBA(argp[2], RGBAPal);
    for(index = 0;index < 256;index++) {
        outPal.transColor[index].peBlue  = RGBAPal[index].peRed;
        outPal.transColor[index].peGreen = RGBAPal[index].peGreen;
        outPal.transColor[index].peRed   = RGBAPal[index].peBlue;
        outPal.transColor[index].peFlags = RGBAPal[index].peFlags;
    }

   if ( argc > 4 )
   {
      for ( int arg=4; arg<argc && strstr(argp[arg],"r("); arg++ )
      {
         if ( rangeIndex > MAX_RANGES )
         {
            printf(avar("Max RGBMatch range limit(%i) exeeded.\n",MAX_RANGES));
            return (1);
         }
         sscanf(argp[arg], "(%d,%d)", &palRange[rangeIndex].min, &palRange[rangeIndex].max);
         rangeIndex++;
      }
      if (rangeIndex)
      {
         outPal.allowedColorMatches.zero();
         for ( int i=0; i<rangeIndex; i++ )
            outPal.allowedColorMatches.set(palRange[i].min,palRange[i].max);
      }
   }

   lookupTable = new UBYTE[65536];
   makeLookup(lookupTable,outPal.transColor, outPal.color, &outPal ); 

   outPal.transMap     = new UBYTE[65536];
   outPal.transMapBase = new UBYTE[65536];
   for(index = 0;index < 65536;index++)
   {
       outPal.transMap[index] = lookupTable[index];
     outPal.transMapBase[index] = lookupTable[index];
   }

   outPal.write(argp[3]);
   return 0;
}