//---------------------------------------------------------------------------
//
//
//  Filename   : makepal.h
//
//
//  Description: Some prototypes for the functions in makepal.cpp. The only one
//               actually called by main is RemakeFixedColors, the other 3 functions
//               are helpers. They are called only by RemakeFixedColors.
//               
//               
//               
//               
//               
//
//               
//               
//               
//               
//               
//
//
//  Author     : Joel Franklin 
//
//
//  Date       : Friday 13th 1997
//
//
//
//---------------------------------------------------------------------------


#ifndef _MAKEPAL_H
#define _MAKEPAL_H



void RemakeFixedColors(PALETTEENTRY *, PALETTEENTRY *,
                               COUNTER, COUNTER,COUNTER,COUNTER);
ULONG ColorDifference(PALETTEENTRY *, PALETTEENTRY *);
UBYTE FindClosestMatch(PALETTEENTRY *, PALETTEENTRY *);
#ifndef MSVC
signed int CompareIntensity(const void *, const void *);
#else
signed int __cdecl CompareIntensity(const void *, const void *);
#endif

#endif // _MAKEPAL_H