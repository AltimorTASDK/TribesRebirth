#ifndef _TABLE_H
#define _TABLE_H

#include <g_pal.h>
typedef unsigned char UBYTE;
typedef unsigned long ULONG;
typedef signed short SSHORT;
typedef signed long SLONG;

void makeLookup(UBYTE *lookupTable, PALETTEENTRY *RGBAPal, PALETTEENTRY *RGBPal, GFXPalette *outPal);
UBYTE findClosestMatch(PALETTEENTRY *colorPtr, PALETTEENTRY *Pal, GFXPalette *outPal);
ULONG colorDifference(PALETTEENTRY *, PALETTEENTRY *);

#endif
