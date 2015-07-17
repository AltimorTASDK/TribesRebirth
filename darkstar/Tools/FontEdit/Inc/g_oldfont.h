// OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD
// OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD
// OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD
// OLD - 
// OLD -   This is OLD GFXFont code, it's purpose is to provide i/o
// OLD -   of old fonts (well, input only actually) to the font edit
// OLD -   application
// OLD - 
// OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD
// OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD
// OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD - OLD

//========================================================================
//   
// $Workfile:$
// $Version$
// $Revision:$
//   
// DESCRIPTION:   GFX Font Class Header File
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//
//========================================================================

#ifndef _G_OLDFONT_H_
#define _G_OLDFONT_H_

#include <stdlib.h>
#include <filstrm.h>
#include "g_barray.h"
#include "g_bitmap.h"
#include "g_pal.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//========================================================================

typedef struct   tagGFXOldCharInfo           // structure holding information for each char
{
   Int32      baseline;                   // this chars offset from baseline from top
   Int32      x,y;                        // optional hot-point for this character
}GFXOldCharInfo;

typedef  struct   tagGFXOldFontInfo          // structure holding base font information
{
   UInt32   flags;                        // Holds font flag information
   UInt32   justification;                // Holds the font justification
   Int32    numChars;                     // Number of chars. in font.
   Int32    fontHeight;                   // For fixed height fonts.
   Int32    fontWidth;                    // For fixed width fonts.
   Int32    foreColor;                    // Foreground color for mono fonts
   Int32    backColor;                    // Background color for mono fonts
   Int32    baseline;                     // Distance of baseline from the top.
   Point2I  scale;                        // fix 16.16 scaling factor
   Int32    spacing;                      // spacing between characters
}GFXOldFontInfo;

#if 0
// THESE ARE DECLARED IN THE REAL (new) g_font.h
enum FontJustification                    // various font drawing attributes
{
   TEXT_LEFT         = 1<<0,
   TEXT_RIGHT        = 1<<1,
   TEXT_CENTER_H     = 1<<2,              // center text horizontally
   TEXT_CENTER_V     = 1<<3,              // center text vertically
   TEXT_TOP          = 1<<4,
   TEXT_BOTTOM       = 1<<5,
   TEXT_FIT          = 1<<6,              // force text to fit in rectangle
};

enum FontFlags                            // font type descriptors
{
   FONT_PROPORTIONAL = 1<<0,
   FONT_FIXED        = 1<<1,
   FONT_MONO         = 1<<2,
   FONT_BITMAP       = 1<<3,
   FONT_TRUETYPE     = 1<<4,
   FONT_RASTERIZED   = 1<<5,
   FONT_TRANSPARENT  = 1<<6,
   FONT_TRANSLUCENT  = 1<<7,
   FONT_UNDERLINED   = 1<<8,              // draw line on baseline before font
   FONT_UNICODE      = 1<<9               // chars are WORD's instead of BYTE's
};
#endif

//========================================================================
//                              Font Class
//========================================================================

class GFXOldFont
{
public:
   GFXBitmapArray bma;                    // DIB containing font & bits
   GFXOldFontInfo    fi;                     // font information structure
   GFXPalette     *pPal;                  // optional palette
   GFXBitmap      *missing;               // DEBUG ONLY missing character bitmap

   GFXOldFont();                             // constructor
   ~GFXOldFont();                            // deletes this font
static GFXOldFont *  load( const char * );     // loads a font from a file
static GFXOldFont *  load( StreamIO * );       // loads a font from a stream

   Bool      read( const char *);          // read a font from a file
   Bool      read( StreamIO * );           // read a font from a stream
   
   Bool      write( const char * );        // write font data to file
   Bool      write( StreamIO * );          // write font data to stream

   void      getExtents( GFXOldFontInfo *io_fi ); // fills FontInfo strucure with font info
   void      flattenRemap();
   Int32     getJustification();           // returns font justification flag
   Int32     getWidth();                   // returns fixed font width
   Int32     getWidth( Int32 );            // returns with of char in pixels
   Int32     getStrWidth( const void * );  // returns length of string in pixels
   Int32     getHeight();                  // returns fixed font height
   Int32     getHeight(Int32);             // returns height of char in pixels
   Int32     getStrHeight(const void * );  // returns height of string in pixels
   Int32     getForeColor();               // returns foreground color (mono)
   Int32     getBackColor();               // returns background color
   Int32     getSpacing();                 // returns spacing between characters
   Point2I   getScale();                   // returns font scale
   DWORD     getSaveSize();                // returns size of this PFNT chunk
   void      getCharInfo(int index, GFXBitmap **bmp, GFXOldCharInfo **charInfo);

   void      setJustification(Int32);      // set font justification flag(s) to TRUE/FALSE
   void      setForeColor( Int32 );        // set foreground color (mono)
   void      setBackColor( Int32 );        // set background color
   void      setScale( Point2I * );        // set font scale
   void      setSpacing( Int32 );          // set spacing between characters
   
   void      remapColors( BYTE remapTable[256] );   // remap the entire font to a new set of
                                                   // indices in the palette
   void      remapColors( GFXPalette * );  // remaps font to current palette
   void      shadeColors( GFXPalette * );  // remaps font to fonts palette based on shade palette passed
};

//========================================================================
//                           Inline Functions
//========================================================================

inline void     GFXOldFont::flattenRemap() { bma.flattenRemap(); }
inline Int32    GFXOldFont::getJustification( void ) { return (fi.justification); }
inline Int32    GFXOldFont::getWidth() { return ((fi.fontWidth*fi.scale.x)>>16); }
inline Int32    GFXOldFont::getHeight( void ) { return ((fi.fontHeight*fi.scale.y)>>16); }
inline Int32    GFXOldFont::getForeColor( void ) { return (fi.foreColor); }
inline Int32    GFXOldFont::getBackColor( void ) { return (fi.backColor); }
inline Point2I  GFXOldFont::getScale( void ) { return (fi.scale); }
inline Int32    GFXOldFont::getSpacing( void ) { return (fi.spacing); }

inline void     GFXOldFont::setJustification( Int32 in_flag ) { fi.justification = in_flag; }
inline void     GFXOldFont::setScale( Point2I *in_scale ) { fi.scale = *in_scale; }
inline void     GFXOldFont::setSpacing( Int32 in_spacing ) { fi.spacing = in_spacing; }

inline Int32    GFXOldFont::getWidth( Int32 in_char ) 
{ 
   GFXBitmap *bmp;
   getCharInfo( in_char, &bmp, NULL);
   return ( (bmp->getWidth()*fi.scale.x) >> 16 );
}
inline Int32    GFXOldFont::getHeight( Int32 in_char ) 
{ 
   GFXBitmap *bmp;
   getCharInfo( in_char, &bmp, NULL);
   return ( (bmp->getHeight()*fi.scale.y) >> 16 ); 
}




inline void     GFXOldFont::getExtents( GFXOldFontInfo *io_fi )
{
   io_fi->flags         = fi.flags;
   io_fi->justification = fi.justification;                
   io_fi->numChars      = fi.numChars;                     
   io_fi->fontHeight    = fi.fontHeight;                   
   io_fi->fontWidth     = fi.fontWidth;                    
   io_fi->foreColor     = fi.foreColor;                    
   io_fi->backColor     = fi.backColor;                    
   io_fi->baseline      = fi.baseline;                     
   io_fi->scale         = fi.scale;
   io_fi->spacing       = fi.spacing;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // _G_OLDFONT_H_
