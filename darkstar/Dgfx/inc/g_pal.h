//================================================================
//   
// $Workfile:   g_pal.h  $
// $Version$
// $Revision:   1.3  $
//   
// DESCRIPTION:
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _G_PAL_H_
#define _G_PAL_H_

#include <base.h>
#include <tBitVector.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//GFX Palette IO Flags
#define GFX_PLF_MICROSOFT (1<<31)   //save in microsoft format
#define PAL_SHIFT 8                 // (1<<8) == 256
#define MAX_MULTI_PALETTES 16

//================================================================

typedef float (*GammaFunction)(float);

class StreamIO;
class GFXBitmap;

class GFXPalette
{
public:
   Bool writeMSPal(StreamIO *in_stream, DWORD flags=0);
   Bool readMSPal(StreamIO *in_stream, DWORD flags=0);

   // ***WARNING***
   // The following entries are cast to the windows
   // LOGPALETTE structure and must not be changed.
   WORD           version;
   WORD           numberOfColors;

   struct MultiPalette
   {
      PALETTEENTRY color[256];
      BYTE *shadeMap;
      BYTE *hazeMap;
      BYTE *transMap;

      BYTE*  identityMap;     // Maps from subpalettes into the master palette
      float* indexToRMap;     // For OpenGL, maps index to color components for this
      float* indexToGMap;     //  multipalette
      float* indexToBMap;
      float* indexToAMap;

      DWORD paletteIndex; // project global palette index
      DWORD paletteType; // types defined below
   };

   enum PaletteType
   {
      NoRemapPaletteType,
      ShadeHazePaletteType,
      TranslucentPaletteType,

      ColorQuantPaletteType,
      AlphaQuantPaletteType,

      AdditiveQuantPaletteType,
      AdditivePaletteType,

      SubtractiveQuantPaletteType,
      SubtractivePaletteType,
   };

   MultiPalette palette[MAX_MULTI_PALETTES];

   Int32 numPalettes; // number of multipalettes
   Int32 shadeShift;
   Int32 hazeLevels;
   Int32 shadeLevels;

   BYTE *remapTable;
   BYTE *remapBase;  // base of the remap table - this gets aligned
                     // on a 1 << (shadeShift + 8) boundary

   Int32 hazeColor;

   BitVector allowedColorMatches;   // allowed in getClosestColor Calls...
   Int32 pal_ID;  // counter which identifies a unique palette state.
                  // must be incremented each time the palette is changed.

   GFXPalette();
   ~GFXPalette();

   static GFXPalette* create(const PALETTEENTRY *in_pe);
   static GFXPalette* create(const RGBQUAD *in_rgb);

   static GFXPalette* load(const char *in_filename, DWORD flags=0);
   static GFXPalette* load(StreamIO *in_stream, DWORD flags=0);
   Bool              read(const char *in_filename, DWORD flags=0);
   Bool              read(StreamIO *in_stream, DWORD flags=0);
   Bool              write(const char *in_filename, DWORD flags=0);
   Bool              write(StreamIO *in_stream, DWORD flags=0);
   DWORD             getSaveSize(){return(0);}

   Int32             getPal_ID();
   void              notePaletteChanged();

   void              set(const PALETTEENTRY *in_pe);
   void              set(const RGBQUAD *in_rgb);
   void              get(PALETTEENTRY *out_pe);
   void              get(RGBQUAD *out_rgb);

   inline BYTE*      getShadeMap(Fix16 in_intensity=0);
   Int32             getMaxShade();
   Int32             getMaxHaze();
   bool              isAllowed(int in_index) const;
   int               GetNearestColor( PALETTEENTRY );
   int               GetNearestColor( int r, int g, int b );
   int               mapColor( int in_color, int in_shade, int in_haze, DWORD index = DWORD(-1) );
 
   inline void       setHazeColor(Int32 in_index) { hazeColor = in_index; }
   inline Int32      getHazeColor() { return hazeColor; }
 
   BYTE*             getTransTblAddr();
   BYTE*             getTransTblAddr(Int32 in_level);  // overloaded for poly code

   void              setWindowsColors();
   inline MultiPalette *findMultiPalette(DWORD index);

   // Quantization info extensions, only present in intermediate palettes, check with
   //  hasQ..Info()
   //
  public:
   float*   m_pColorWeights;
   UInt32   m_usedRangeStart;
   UInt32   m_usedRangeEnd;

   bool hasQuantizationInfo() const { return (m_pColorWeights != NULL); }
   void initQuantizationInfo();
   void stripQuantizationInfo();
};

inline int GFXPalette::GetNearestColor( PALETTEENTRY match )
{
   return GetNearestColor(match.peRed, match.peGreen, match.peBlue);   
}   

inline Int32 GFXPalette::getPal_ID()
{
   return pal_ID;
}

inline GFXPalette::MultiPalette*
GFXPalette::findMultiPalette(DWORD index)
{
   AssertFatal(numPalettes <= 16, "Must rewrite findMultiPalette()");

   if(index == 0xFFFFFFFF)
      return palette;

   // Sigh.  An inelegant solution at best, but the only one that doesn't
   //  send BC5 into violent, spasmodic, useless warnings.  DMM
   //
   if (numPalettes >= 1  && palette[0].paletteIndex  == index) return &palette[0];
   if (numPalettes >= 2  && palette[1].paletteIndex  == index) return &palette[1];
   if (numPalettes >= 3  && palette[2].paletteIndex  == index) return &palette[2];
   if (numPalettes >= 4  && palette[3].paletteIndex  == index) return &palette[3];
   if (numPalettes >= 5  && palette[4].paletteIndex  == index) return &palette[4];
   if (numPalettes >= 6  && palette[5].paletteIndex  == index) return &palette[5];
   if (numPalettes >= 7  && palette[6].paletteIndex  == index) return &palette[6];
   if (numPalettes >= 8  && palette[7].paletteIndex  == index) return &palette[7];
   if (numPalettes >= 9  && palette[8].paletteIndex  == index) return &palette[8];
   if (numPalettes >= 10 && palette[9].paletteIndex  == index) return &palette[9];
   if (numPalettes >= 11 && palette[10].paletteIndex == index) return &palette[10];
   if (numPalettes >= 12 && palette[11].paletteIndex == index) return &palette[11];
   if (numPalettes >= 13 && palette[12].paletteIndex == index) return &palette[12];
   if (numPalettes >= 14 && palette[13].paletteIndex == index) return &palette[13];
   if (numPalettes >= 15 && palette[14].paletteIndex == index) return &palette[14];
   if (numPalettes >= 16 && palette[15].paletteIndex == index) return &palette[15];

   return palette;
}

inline int GFXPalette::mapColor( int in_color, int in_shade, int in_haze, DWORD index )
{
   MultiPalette* pMPalette = findMultiPalette(index);

   int offset;
   offset = (numberOfColors*in_shade + in_color) + 
             (numberOfColors << shadeShift) * in_haze;

   return pMPalette->shadeMap[offset];
}

inline Int32 GFXPalette::getMaxShade()
{
   return shadeLevels - 1;
}

inline Int32 GFXPalette::getMaxHaze()
{
   return hazeLevels - 1;
}

inline BYTE* GFXPalette::getShadeMap(Int32 palIndex)
{
   return palette[palIndex].shadeMap;
}

inline BYTE* GFXPalette::getTransTblAddr()
{
   return (palette[0].transMap);
}

inline GFXPalette::~GFXPalette()
{
	delete [] remapBase;

   delete [] m_pColorWeights;
   m_pColorWeights = NULL;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif


