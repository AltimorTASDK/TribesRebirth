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

#ifndef _G_FONT_H_
#define _G_FONT_H_

#include <types.h>
#include <m_point.h>
#include <m_rect.h>
#include <persist.h>
#include "g_barray.h"

#ifdef __BORLANDC__
#pragma option -a4		  
#endif
#ifdef _MSC_VER
#pragma pack(push,4)    
#endif

//========================================================================

typedef  struct   tagGFXFontInfo          // structure holding base font information
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
} GFXFontInfo;

typedef struct tagGFXCharInfo
{
	UInt8 bitmapNo;			 // index to sheet in bma containing this char
	UInt8 x, y;              // offset on sheet of this char
	UInt8 width, height;
	Int8  baseline;
	UInt8 padding[2];        // alignment to 8 bytes;
} GFXCharInfo;

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
   FONT_UNICODE      = 1<<9,              // chars are WORD's instead of BYTE's
   FONT_LOWERCAPS    = 1<<10              // the font tool will output an all caps font
};

//========================================================================
//                              Font Class
//========================================================================

class GFXPalette;

class GFXFont : public Persistent::VersionedBase
{
public:
   GFXBitmapArray bma;                    // DIB containing font & bits
   GFXFontInfo    fi;                     // font information structure
   GFXPalette     *pPal;                  // optional palette
   GFXBitmap      *missing;               // DEBUG ONLY missing character bitmap

   //
   // About the char lookup tables:
   //
   // The charTable array contains character information for each character 
   // in the font.  The index of a particular character's info in charTable 
   // is must be found via the charTableLookup array.
   //
   // The charTableLookup array contains the index into the charTable array
   // for each character in the font.
   // An index into charTable can be found in charTableLookup as follows:
   //
   //  index_into_charTable = charTableLookup[ASCII_CODE - charTableLooupOffset];
   //
   // if (ASCII_CODE - charTable LookupOffset) < 0  OR   >= charTableLookupSize
   // then the character corresponding to ASCII_CODE is NOT in the font.
   // Also, if charTableLookup[ASCII_CODE - charTableLookupOffset] == -1 then
   // the character is not in the font (use getCharInfo() to gracefully
   // find the correct character information even if the requested character
   // isn't in the font).
   //
   GFXCharInfo *charTable;
   Int16       *charTableLookup;
   Int16        charTableLookupOffset;
   Int16        charTableLookupSize;

   GFXFont();                             // constructor
   ~GFXFont();                            // deletes this font

   static GFXFont * load( const char * );     // loads a font from a file
   static GFXFont * load( StreamIO * );       // loads a font from a stream

	// generic read(char *) and read(StreamIO *) are no longer supported, Base::load() should
	// be used instead
    // Bool read( const char *);          // read a font from a file
    // Bool read(StreamIO *);             // read a font from a stream

   Bool write(const char *);        // write font data to file
   Bool write(StreamIO *);          // write font data to stream
 
   // Persistent functions
   DECLARE_PERSISTENT(GFXFont);
   Persistent::Base::Error read(StreamIO &, int version, int user);
   Persistent::Base::Error write(StreamIO &, int version, int user); 
   void dump();
   int  version();

  public:
   void     getExtents( GFXFontInfo *io_fi ); // fills FontInfo strucure with font info
   void     flattenRemap();
   Int32    getJustification();           // returns font justification flag
   Int32    getWidth();                   // returns fixed font width
   Int32    getWidth( Int32 );            // returns with of char in pixels
   Int32    getWordWidth( const void * ); // returns length of a word in pixels
   Int32    getHeight();                  // returns fixed font height
   Int32    getHeight(Int32);             // returns height of char in pixels
   Int32    getForeColor();               // returns foreground color (mono)
   Int32    getBackColor();               // returns background color
   Int32    getSpacing();                 // returns spacing between characters
   Point2I  getScale();                   // returns font scale
   DWORD    getSaveSize();                // returns size of this PFNT chunk

   virtual Int32 getStrHeight(const void * );  // returns height of string in pixels
   virtual Int32 getStrWidth( const void * );  // returns length of string in pixels

   virtual void getCharInfo(int index, const GFXBitmap **bmp, RectI *in_subRegion, GFXCharInfo **charInfo);

   void     setJustification(Int32);      // set font justification flag(s) to TRUE/FALSE
   void     setScale( Point2I * );        // set font scale
   void     setSpacing( Int32 );          // set spacing between characters
   
   virtual void setBackColor( Int32 );        // set background color
   virtual void setForeColor( Int32 );        // set foreground color (mono)
   virtual void remapColors( BYTE remapTable[256] );   // remap the entire font to a new set of
                                                               // indices in the palette
   virtual void remapColors( GFXPalette * );  // remaps font to current palette

   virtual void shadeColors( GFXPalette * );  // remaps font to fonts palette based on shade palette passed

   void     wrapStr(char *str, Int32 width); // modifies *str so that it will word wrap within the
                                                     // specified width.  Note: *str is modified by making substitutions
                                                     // among spaces among carriage returns.   Tabs are not supported.
};

//========================================================================
//                           Inline Functions
//========================================================================

inline void    GFXFont::flattenRemap()           { bma.flattenRemap(); }
inline Int32   GFXFont::getJustification( void ) { return (fi.justification); }
inline Int32   GFXFont::getWidth()               { return ((fi.fontWidth*fi.scale.x)>>16); }
inline Int32   GFXFont::getHeight( void )        { return ((fi.fontHeight*fi.scale.y)>>16); }
inline Int32   GFXFont::getForeColor( void )     { return (fi.foreColor); }
inline Int32   GFXFont::getBackColor( void )     { return (fi.backColor); }
inline Point2I GFXFont::getScale( void )         { return (fi.scale); }
inline Int32   GFXFont::getSpacing( void )       { return (fi.spacing); }

inline void    GFXFont::setJustification( Int32 in_flag ) { fi.justification = in_flag; }
inline void    GFXFont::setScale( Point2I *in_scale )     { fi.scale = *in_scale; }
inline void    GFXFont::setSpacing( Int32 in_spacing )    { fi.spacing = in_spacing; }

inline Int32   GFXFont::getWidth( Int32 in_char ) 
{ 
	GFXCharInfo *cInfo;
   RectI temp;
	getCharInfo(in_char, NULL, &temp, &cInfo);
	return((cInfo->width*fi.scale.x) >> 16);
}

inline Int32   GFXFont::getHeight( Int32 in_char ) 
{ 
	GFXCharInfo *cInfo;
   RectI temp;
	getCharInfo(in_char, NULL, &temp, &cInfo);
	return((cInfo->height*fi.scale.y) >> 16);
}

inline void    GFXFont::getExtents( GFXFontInfo *io_fi )
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

#endif // _G_FONT_H_
