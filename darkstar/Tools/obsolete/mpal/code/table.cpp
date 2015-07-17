#include <windows.h>
#include "table.h"

void makeLookup(UBYTE *lookupTable, PALETTEENTRY *RGBAPal, PALETTEENTRY *RGBPal, GFXPalette *outPal)
{
    ULONG indexx;
    ULONG indexy;
    ULONG lookupIndex;
    float correction;
    float alpha;
    PALETTEENTRY colorPtr;

    memset(lookupTable, 0, sizeof(UBYTE) * 65536);
    for(indexx = 1; indexx < 255; indexx++) {
        for(indexy = 0; indexy < 255; indexy++) {
            lookupIndex = DWORD(indexx << 8) | DWORD(indexy);
            correction  = float((255.0f - float(RGBAPal[indexx].peFlags))/255.0f);
            alpha       = float(RGBAPal[indexx].peFlags)/255.0f;

            colorPtr.peRed   = RGBPal[indexy].peRed   * correction + RGBAPal[indexx].peRed   * alpha;
            colorPtr.peGreen = RGBPal[indexy].peGreen * correction + RGBAPal[indexx].peGreen * alpha;
            colorPtr.peBlue  = RGBPal[indexy].peBlue  * correction + RGBAPal[indexx].peBlue  * alpha;

            lookupTable[lookupIndex] = findClosestMatch(&colorPtr, RGBPal, outPal);
        }
   }
   
   // Zero color is ALWAYS Transparent...
   //
   for (int i = 0; i < 256; i++) {
      lookupTable[i] = i;
   }
}

UBYTE findClosestMatch(PALETTEENTRY *colorPtr, PALETTEENTRY *Pal, GFXPalette *outPal)
{
    ULONG closestIndex = 0;
    ULONG closestValue;
    ULONG index;

    closestValue=colorDifference(colorPtr,&Pal[0]);
    for (index = 0;index < 256;index++)
    {
      if (!outPal->allowedColorMatches.test(index))
         continue;

        if (colorDifference(colorPtr,&Pal[index]) < closestValue)
        {
            closestIndex=index;
            closestValue=colorDifference(colorPtr,&Pal[index]);
        }
    }
    return (UBYTE) closestIndex; 
}
    
ULONG colorDifference(PALETTEENTRY *color1, PALETTEENTRY *color2)
{
    SLONG diff,bluediff,greendiff,reddiff;
    SLONG r0=color1->peRed ;
    SLONG g0=color1->peGreen;
    SLONG b0=color1->peBlue ;
    SLONG r1=color2->peRed ;
    SLONG g1=color2->peGreen ;
    SLONG b1=color2->peBlue ;
    bluediff = (b0-b1);
    bluediff *= bluediff;
    greendiff = (g0-g1);
    greendiff *= greendiff;
    reddiff = (r0-r1);
    reddiff *= reddiff;
    diff = greendiff*6+reddiff*3+bluediff;
    return diff;
}
