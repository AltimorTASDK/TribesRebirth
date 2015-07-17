#include "g_bitmap.h"
#include <stdio.h>
#include <stdlib.h>

void Alphatize(GFXBitmap *src, GFXBitmap *alpha, int levs, int ncols, int bkcol)
{
   int i, j;
   float scale =  levs / 256.0;
   
   for(j = 0; j < src->getHeight(); j++)
   {
      BYTE *sBits = src->getAddress(0, j);
      BYTE *aBits = alpha->getAddress(0, j);
      for(i = 0; i < src->getWidth(); i++)
      {
         if(int(aBits[i]) * scale == 0)
            sBits[i] = bkcol;
         else
            sBits[i] += int((255-aBits[i]) * scale) * ncols;
      }
   }
}

int __cdecl main(int argc, char **argv)
{
   printf("Alphamap.exe: GFX Bitmap alpha tool.\n\n");
   if(argc == 1)
   {
      printf("Usage: >alphamap <src> <alpha> <dest> <translevels> <transrange>\n"
             "\n"
             "   <src> is the original source bitmap.\n"
             "   <alpha> is the overlay grayscale alpha channel.\n"
             "           color 0 is fully transparent,\n"
             "           color 255 is fully opaque.\n"
             "   <dest> is the filename for the combined bitmap.\n"
             "   <translevels> is the number of translucency levels in\n"
             "                 the palette.\n"
             "   <transrange> is the number of colors that are translucent.\n"
             "   <bkindex> index of the transparent color.\n");
      return 0;
   
   }
   if(argc != 7)
   {
      printf("ERROR: wrong number of arguments.\n");
      return 0;
   }
   
   GFXBitmap *srcBitmap = GFXBitmap::load(argv[1], BMF_INCLUDE_PALETTE);
   GFXBitmap *alphaBitmap = GFXBitmap::load(argv[2]);
   
   Alphatize(srcBitmap, alphaBitmap, atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
   
   srcBitmap->write(argv[3], BMF_MICROSOFT);
   return 0;
}