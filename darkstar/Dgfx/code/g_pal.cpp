//================================================================
//
// $Workfile:   g_pal.cpp  $
// $Version$
// $Revision:   1.3  $
//
// DESCRIPTION:
//
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#include <types.h>
#include <string.h>
#include <filstrm.h>
#include "resManager.h"

#include "g_pal.h"


BYTE windowsColors[16][3] =
{
{ 0,   0,   0   },
{ 128, 0,   0   },
{ 0,   128, 0   },
{ 128, 128, 0   },
{ 0,   0,   128 },
{ 128, 0,   128 },
{ 0,   128, 128 },
{ 192, 192, 192 },
{ 128, 128, 128 },
{ 255, 0,   0   },
{ 0,   255, 0   },
{ 255, 255, 0   },
{ 0,   0,   255 },
{ 255, 0,   255 },
{ 0,   255, 255 },
{ 255, 255, 255 },
};

static Int32 next_pal_ID = 0;

//increment high byte if palette format become incompatible with
//old reader.
#define PALETTE_VERSION 0x00000000

void GFXPalette::notePaletteChanged()
{
   pal_ID++;
   next_pal_ID++;
}

//================================================================
// NAME
//   GFXPalette::GetNearestColor
//
// DESCRIPTION
//   Find color in palette that most closely matches the RGB values passed
//
// ARGUMENTS
//   PALETTEENTRY containing peRed, peGreen, and peBlue
//
// RETURNS
//   Index into palette for color found
//
// NOTES
//   Default Does not match to Windows system colors 0-10 or 245-255
//
//================================================================
bool GFXPalette::isAllowed(int in_index) const
{
   // We happen to know that this function does not cause any state changes in the bit
   //  vector, so we'll fool the compiler a bit here...
   //
   GFXPalette* ncThis = const_cast<GFXPalette*>(this);

   return (ncThis->allowedColorMatches.test(in_index));
}


int GFXPalette::GetNearestColor( int r, int g, int b)
{
   int index;
   int returnIndex=255;
   int lowError = 256*256*256;
   int err;
   int   dx,dy,dz;

   for( index=0; index<256; index++ )             // Exclude system palette
   {
      // Only check allowed colors if table is not null...
      if (!allowedColorMatches.test(index))
         continue;
         
      dx = r - palette[0].color[index].peRed;
      dy = g - palette[0].color[index].peGreen;
      dz = b - palette[0].color[index].peBlue;
      err = dx*dx + dy*dy + dz*dz;
      if( err < lowError )
      {
         lowError = err;
         returnIndex = index;
      }
   }

   // Always allow match to black
   //
   dx = r - palette[0].color[0].peRed;
   dy = g - palette[0].color[0].peGreen;
   dz = b - palette[0].color[0].peBlue;
   err = dx*dx + dy*dy + dz*dz;
   if( err < lowError )
   {
      lowError = err;
      returnIndex = 0;
   }

   return returnIndex;
}

//================================================================
// NAME
//   GFXPalette::GFXPalette
//
// DESCRIPTION
//   Palette constructor
//
// ARGUMENTS
//   none
//
// RETURNS
//   initialize number of coloer entries to ZERO
//
//================================================================
GFXPalette::GFXPalette()
{
   hazeColor = 0;
   version        = 0x300;
   numberOfColors = 256;
   memset(palette[0].color, 0, sizeof(PALETTEENTRY)*numberOfColors);
	numPalettes    = 1;
   palette[0].shadeMap = NULL;
   palette[0].hazeMap = NULL;
   palette[0].transMap = NULL;
   palette[0].paletteIndex = 0;
   palette[0].paletteType = NoRemapPaletteType;
   hazeLevels = 0;
   shadeLevels = 0;
   shadeShift = 0;
   remapTable = NULL;
   remapBase = NULL;
   allowedColorMatches.setSize(256);
   allowedColorMatches.zero();
   allowedColorMatches.set(10,245);
   pal_ID = next_pal_ID++;

   m_pColorWeights   = NULL;
   m_usedRangeStart = 0;
   m_usedRangeEnd   = 0;
}


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void GFXPalette::set(const PALETTEENTRY *pe)
{
   memcpy(palette[0].color, pe, sizeof(PALETTEENTRY)*numberOfColors);
   notePaletteChanged();
}


//------------------------------------------------------------------------------
void GFXPalette::set(const RGBQUAD *in_rgb)
{
   RGBQUAD  rgbq;

   PALETTEENTRY *pe = palette[0].color;
   for(Int32 i=0; i<256; i++, pe++, in_rgb++)
   {
      // copy from the in_rgb_quad color table to the new palette
      // palette_entry table

      rgbq = *in_rgb;

      pe->peRed   = rgbq.rgbRed;
      pe->peGreen = rgbq.rgbGreen;
      pe->peBlue  = rgbq.rgbBlue;
      pe->peFlags = PC_NOCOLLAPSE;  // don't eliminate duplicates
   }
   notePaletteChanged();
}


//------------------------------------------------------------------------------
void GFXPalette::get(PALETTEENTRY *out_pe)
{
   memcpy(out_pe, palette[0].color, sizeof(PALETTEENTRY)*numberOfColors);
}


//------------------------------------------------------------------------------
void GFXPalette::get(RGBQUAD *out_rgb)
{
   PALETTEENTRY *pe = palette[0].color;
   for(Int32 i=0; i<256; i++, pe++, out_rgb++)
   {
      // copy from the out_rgb_quad color table to the new palette
      // palette_entry table

      out_rgb->rgbRed    = pe->peRed;
      out_rgb->rgbGreen  = pe->peGreen;
      out_rgb->rgbBlue   = pe->peBlue;
      out_rgb->rgbReserved = 0;
   }
}


//------------------------------------------------------------------------------
GFXPalette* GFXPalette::create(const PALETTEENTRY *in_pe)
{
   GFXPalette *pal = new GFXPalette;
   if (pal)
   {
      pal->numberOfColors = 256;
      pal->set(in_pe);
      return pal;

   }
   return NULL;
}


//------------------------------------------------------------------------------
GFXPalette* GFXPalette::create(const RGBQUAD *in_rgb)
{
   GFXPalette *pal = new GFXPalette;
   if (pal)
   {
      pal->numberOfColors = 256;
      pal->set(in_rgb);
      return pal;
   }
   return NULL;
}

//================================================================
// NAME
//   GFXPalette::load
//
// DESCRIPTION
//   Create and load a palette.
//
// ARGUMENTS
//   in_filename - filename of palette to load
//   in_flags    - GFXPalette laod flags
//
// RETURNS
//   Pointer to palette if successful
//   NULL if out of memory or palette file does not exist
//
//================================================================
GFXPalette* GFXPalette::load(const char *in_filename, DWORD) //DWORD flags)
{
   GFXPalette *pal = new GFXPalette;
   if (pal)
      if (pal->read(in_filename))
         return pal;
      else
         delete pal;
   return (NULL);
}


//================================================================
// NAME
//   GFXPalette::load
//
// DESCRIPTION
//   Create and Load a palette from a stream.
//
// ARGUMENTS
//   in_stream - a Read capable stream
//   in_flags    - GFXPalette laod flags
//
// RETURNS
//   pointer to a newly allocated GFXPalette
//
// NOTES
//   Assumes the stream is positioned at palette data
//
//================================================================
GFXPalette* GFXPalette::load(StreamIO *in_stream, DWORD in_flags)
{
   GFXPalette *pal = new GFXPalette;
   if (pal)
   {
      if (pal->read(in_stream, in_flags))
         return pal;
      else
         delete pal;
   }
   return (NULL);
}


//================================================================
// NAME
//   GFXPalette::read
//
// DESCRIPTION
//   Read a Palette from the file into this palette structure.
//
// ARGUMENTS
//   in_filename - filename of palette to load
//   in_flags    - GFXPalette laod flags
//
// RETURNS
//   TRUE  on success
//   FALSE on failure
//
// NOTES
//
//================================================================
Bool GFXPalette::read(const char *in_filename, DWORD in_flags)
{
   Bool   result;
   HANDLE fileHandle;
   fileHandle = CreateFile(in_filename,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL
                           );
   if (fileHandle == INVALID_HANDLE_VALUE)
   {
      AssertWarn(0, avar("GFXPalette::read: Unable to open file %s, CreateFile for reading failed.", in_filename));
      return (FALSE);
   }

   FileRStream frs(fileHandle);
   result = read(&frs, in_flags);
   AssertWarn(result, avar("GFXPalette::read: error reading palette %s", in_filename));
   frs.close();
   CloseHandle(fileHandle);

   notePaletteChanged();

   return(result);
}


//================================================================
// NAME
//   GFXPalette::read
//
// DESCRIPTION
//   Read a file into this palette
//
// ARGUMENTS
//   in_stream - a Read capable stream
//   in_flags  - not used
//
// RETURNS
//   TRUE  if read was successful
//   FALSE if read failed
//
// FILE FORMAT
//
//   GFXPalette
//
//       PPAL: sz
//          head: sz
//             Version: (byte)
//             NumChunks: (3 bytes)
//             shadeShift: (1 byte)
//   				hazeShift:  (1 byte)
//
//          info: sz    (optional)
//             strings
//          data: sz
//             Array of 256 PALETTEENTRY structs
//   			pspl: sz
//             Array of shade tables
//   			ptpl: sz
//   				Translucency remap table
//
//   Microsoft Palette
//
//       RIFF: sz
//          PAL : DATA
//             number of colors: (dword)
//             version number: (dword)
//             Array of n RGBQUAD structs
//
//================================================================
Bool GFXPalette::read(StreamIO *in_stream, DWORD in_flags)
{
   DWORD type;
   in_stream->read(&type);
   if(type == FOURCC('R','I','F','F'))
   {
      // clear out the size field and then call MS pal loader
      in_stream->read(&type);
      return readMSPal(in_stream, in_flags);
   }

   AssertFatal(type == FOURCC('P','L','9','8'), "Error loading old version palette");

   delete [] remapBase;
   remapBase = NULL;
   remapTable = NULL;

   in_stream->read(&numPalettes);
   in_stream->read(&shadeShift);
   in_stream->read(&hazeLevels);
   in_stream->read(&hazeColor);
   in_stream->read(32, allowedColorMatches.begin());

   shadeLevels = 1 << shadeShift;

   DWORD remapTableSize = 0;

   int i;
   for(i = 0; i < numPalettes; i++)
   {
      in_stream->read(1024, palette[i].color);
      in_stream->read(&palette[i].paletteIndex);
      in_stream->read(&palette[i].paletteType);
   
      palette[i].shadeMap = NULL;
      palette[i].hazeMap = NULL;
      palette[i].transMap = NULL;

      if(palette[i].paletteType == ShadeHazePaletteType) {
         remapTableSize += 256 * shadeLevels * (hazeLevels + 1);
         remapTableSize += sizeof(BYTE) * 256;
         remapTableSize += 4 * (sizeof(float) * 256);
      } else if(palette[i].paletteType == TranslucentPaletteType ||
                palette[i].paletteType == AdditivePaletteType    ||
                palette[i].paletteType == SubtractivePaletteType) {
         remapTableSize += 65536;
         remapTableSize += sizeof(BYTE) * 256;
         remapTableSize += 4 * (sizeof(float) * 256);
      } else if (palette[i].paletteType == NoRemapPaletteType) {
         // We dynamically generate these at the end of the remap table, they are NOT
         //  written to disk...
         //
         remapTableSize += sizeof(BYTE) * 256;
         remapTableSize += 4 * (sizeof(float) * 256);
      }
   }

   // now build the remap table

   if(remapTableSize)
   {
      DWORD padOffset = shadeLevels * 256;
      remapBase = new BYTE[ remapTableSize + padOffset ];

      BYTE *remap = (BYTE *) (( (DWORD) remapBase + padOffset) & ~(padOffset - 1));
      remapTable = remap;

      in_stream->read(remapTableSize, remapTable);

      for(i = 0; i < numPalettes; i++)
      {
         if(palette[i].paletteType == ShadeHazePaletteType)
         {
            palette[i].shadeMap = remap;
            palette[i].hazeMap = remap + 256 * shadeLevels * hazeLevels;
            remap += 256 * shadeLevels * (hazeLevels + 1);
         }
         else if (palette[i].paletteType == TranslucentPaletteType  ||
                  palette[i].paletteType == AdditivePaletteType     ||
                  palette[i].paletteType == SubtractivePaletteType)
         {
            palette[i].transMap = remap;
            remap += 65536;
         }
      }
      for(i = 0; i < numPalettes; i++)
      {
         if(palette[i].paletteType == ShadeHazePaletteType   ||
            palette[i].paletteType == TranslucentPaletteType ||
            palette[i].paletteType == AdditivePaletteType    ||
            palette[i].paletteType == SubtractivePaletteType)
         {
            // All remap palettes have these maps...
            //
            palette[i].identityMap = remap;
            remap += 256 * sizeof(BYTE);

            palette[i].indexToRMap = (float*)remap;
            remap += 256 * sizeof(float);
            palette[i].indexToGMap = (float*)remap;
            remap += 256 * sizeof(float);
            palette[i].indexToBMap = (float*)remap;
            remap += 256 * sizeof(float);
            palette[i].indexToAMap = (float*)remap;
            remap += 256 * sizeof(float);
         }
      }
      for(i = 0; i < numPalettes; i++)
      {
         if(palette[i].paletteType == NoRemapPaletteType) {
            // All remap palettes have these maps...
            //
            palette[i].identityMap = remap;
            remap += 256 * sizeof(BYTE);

            palette[i].indexToRMap = (float*)remap;
            remap += 256 * sizeof(float);
            palette[i].indexToGMap = (float*)remap;
            remap += 256 * sizeof(float);
            palette[i].indexToBMap = (float*)remap;
            remap += 256 * sizeof(float);
            palette[i].indexToAMap = (float*)remap;
            remap += 256 * sizeof(float);

            for (int j = 0; j < 256; j++) {
               palette[i].identityMap[j] = j;
               palette[i].indexToRMap[j] = float(palette[i].color[j].peRed)   / 255.0f;
               palette[i].indexToGMap[j] = float(palette[i].color[j].peGreen) / 255.0f;
               palette[i].indexToBMap[j] = float(palette[i].color[j].peBlue)  / 255.0f;
               palette[i].indexToAMap[j] = 1.0f;
            }
         }
      }
   }


   // Extensions to the palette structure are stored in the order that their flags
   //  are declared in the header...
   //
   bool hasQInfo;
   in_stream->read(&hasQInfo);
   if (hasQInfo == true) {
      m_pColorWeights = new float[256];

      // Quantization info is
      //  0: 256 float   weights
      //  1:   2 UInt32  selection/used range
      //
      in_stream->read(256 * sizeof(float), (void*)m_pColorWeights);
      in_stream->read(&m_usedRangeStart);
      in_stream->read(&m_usedRangeEnd);
   }


   DWORD unused;
   in_stream->read(&unused);

   notePaletteChanged();
   return (TRUE);

}

void GFXPalette::setWindowsColors()
{
   int i;
   for(i = 0; i < 8; i++)
   {
      palette[0].color[i].peRed   = windowsColors[i][0];
      palette[0].color[i].peGreen = windowsColors[i][1];
      palette[0].color[i].peBlue  = windowsColors[i][2];
   }
   for(i = 248; i < 256; i++)
   {
      palette[0].color[i].peRed   = windowsColors[i - 240][0];
      palette[0].color[i].peGreen = windowsColors[i - 240][1];
      palette[0].color[i].peBlue  = windowsColors[i - 240][2];
   }
}

//================================================================
//
// NAME
//   GFXPalette::readMSPal
//
// DESCRIPTION
//   Read a Microsoft RIFF Palette from the stream.
//
// ARGUMENTS
//   in_stream - a Read enabled IO stream
//   in_flags  - not used
//
// RETURNS
//   TRUE  on success
//   FALSE on failure
//
// FILE FORMAT
//
//   Microsoft Palette
//
//       RIFF: sz
//          PAL : DATA
//             number of colors: (dword)
//             version number: (dword)
//             Array of n RGBQUAD structs
//
//================================================================

Bool GFXPalette::readMSPal(StreamIO *in_stream, DWORD)
{
   DWORD data;
   DWORD size;

   in_stream->read(&data);             //get block ID
   in_stream->read(&size);             //get block size

   if (data == FOURCC('R','I','F','F'))   //skip the RIFF and get the 'PAL '
   {
      in_stream->read(&data);          //get block ID 'PAL '
      in_stream->read(&size);          //get block ID 'DATA'
   }

   if (data == FOURCC('P','A','L',' '))
   {
      delete[] remapBase;
      remapBase = NULL;

      remapBase = new UInt8[(256 * sizeof(BYTE)) + (4 * sizeof(float) * 256) + 256];
      BYTE* remap = (BYTE*)(UInt32(remapBase + 256) & ~256);

      in_stream->read(&data);          //get the number of colors in the palette
      //convert low word from Motarola format to Intel then DIV by 4
      numberOfColors = (WORD)(((data<<8)|(data>>8)) & 0xffff);
      numberOfColors = (numberOfColors>>2)-1;

      in_stream->read(&data);          //skip the version number
      version        = 0x300;

      in_stream->read(numberOfColors*sizeof(PALETTEENTRY), (BYTE*)palette[0].color);
      
      shadeShift = hazeLevels = shadeLevels = 0;
      numPalettes = 1;
      palette[0].shadeMap = NULL;
      palette[0].hazeMap = NULL;
      palette[0].transMap = NULL;
      palette[0].paletteType = NoRemapPaletteType;
      palette[0].paletteIndex = -1;

      palette[0].identityMap = remap;
      remap += 256;
      palette[0].indexToRMap = (float*)remap;
      remap += 256 * sizeof(float);
      palette[0].indexToGMap = (float*)remap;
      remap += 256 * sizeof(float);
      palette[0].indexToBMap = (float*)remap;
      remap += 256 * sizeof(float);
      palette[0].indexToAMap = (float*)remap;

      for (int j = 0; j < 256; j++)
      {
         palette[0].identityMap[j] = j;
         palette[0].indexToRMap[j] = float(palette[0].color[j].peRed)   / 255.0f;
         palette[0].indexToGMap[j] = float(palette[0].color[j].peGreen) / 255.0f;
         palette[0].indexToBMap[j] = float(palette[0].color[j].peBlue)  / 255.0f;
         palette[0].indexToAMap[j] = float(palette[0].color[j].peFlags) / 255.0f;
      }

//      setWindowsColors();

      notePaletteChanged();
      return (TRUE);
   }
   
   
   AssertWarn(0, "GFXPalette::readMSPal: This is not a Microsoft palette.");
   return (FALSE);
}

//================================================================
//
// NAME
//   GFXPalette::write
//
// DESCRIPTION
//   Write palette out to in_filename
//
// ARGUMENTS
//   in_filename - filename to save palette as
//   in_flags    - palette flags
//
// RETURNS
//   TRUE  on successful write
//   FALSE on failure
//
// NOTES
//
//================================================================

Bool GFXPalette::write(const char *in_filename, DWORD in_flags)
{
   FileWStream fws;
   if (fws.open(in_filename) == false) {
      AssertWarn(0, avar("GFXFont::write: Unable to write font %s, CreateFile for writing failed.", in_filename));
      return false;
   }

   Bool result = write(&fws, in_flags);
   fws.close();

   AssertWarn(result, avar("GFXPalette::write: error writing palette %s", in_filename));
   return(result);
}

//================================================================
//
// NAME
//   GFXPalette::write
//
// DESCRIPTION
//   Write palette out to in_filename
//
// ARGUMENTS
//   in_stream  - WRITE enabled stream to write palette to
//   in_flags   - palette flags
//
// RETURNS
//   TRUE  on successful write
//   FALSE on failure
//
// NOTES
//
//================================================================

Bool GFXPalette::write(StreamIO *in_stream, DWORD in_flags)
{
   if (in_flags & GFX_PLF_MICROSOFT)
      return writeMSPal(in_stream, in_flags);
   in_stream->write((DWORD)FOURCC('P','L','9','8'));
   in_stream->write(numPalettes);
   in_stream->write(shadeShift);
   in_stream->write(hazeLevels);
   in_stream->write(hazeColor);
   in_stream->write(32, allowedColorMatches.begin());

   int i;
   DWORD remapTableSize = 0;

   for(i = 0; i < numPalettes; i++)
   {
      in_stream->write(1024, palette[i].color);
      in_stream->write(palette[i].paletteIndex);
      in_stream->write(palette[i].paletteType);

      if(palette[i].paletteType == ShadeHazePaletteType) {
         remapTableSize += 256 * shadeLevels * (hazeLevels + 1);

         remapTableSize += 256 * sizeof(BYTE);
         remapTableSize += 4 * (256 * sizeof(float));
      } else if(palette[i].paletteType == TranslucentPaletteType ||
                palette[i].paletteType == AdditivePaletteType    ||
                palette[i].paletteType == SubtractivePaletteType) {
         remapTableSize += 65536;
         
         remapTableSize += 256 * sizeof(BYTE);
         remapTableSize += 4 * (256 * sizeof(float));
      }
   }

   if(remapTableSize)
      in_stream->write(remapTableSize, remapTable);

   if (hasQuantizationInfo() == true) {
      in_stream->write(bool(true));

      in_stream->write(256 * sizeof(float), (void*)m_pColorWeights);
      in_stream->write(m_usedRangeStart);
      in_stream->write(m_usedRangeEnd);
   } else {
      in_stream->write(bool(false));
   }

   in_stream->write(DWORD(0)); // expansion variable

   return (in_stream->getStatus() == STRM_OK);
}

//================================================================
// NAME
//   GFXPalette::writeMSPal
//
// DESCRIPTION
//   Write a Microsoft Format palette out to a stream
//
// ARGUMENTS
//   in_stream  - WRITE enabled stream to write palette to
//   in_flags   - palette flags
//
// RETURNS
//   TRUE  on successful write
//   FALSE on failure
//
// NOTES
//
//================================================================
Bool GFXPalette::writeMSPal(StreamIO *in_stream, DWORD) //DWORD in_flags)
{
   in_stream->write((DWORD)FOURCC('R','I','F','F'));   //write RIFF:sz
   in_stream->write((DWORD)(256*sizeof(PALETTEENTRY))+8+4+4);

   in_stream->write((DWORD)FOURCC('P','A','L',' '));   //write PAL :data
   in_stream->write((DWORD)FOURCC('d','a','t','a'));

   //write the size of color entries (256) + 4 in Motarola format
   in_stream->write((DWORD)0x0404);
   in_stream->write((WORD)0x300);          //write the version number

   in_stream->write((WORD)numberOfColors);

   in_stream->write(numberOfColors*sizeof(PALETTEENTRY), (BYTE*)palette[0].color);
   return (in_stream->getStatus() == STRM_OK);
}

void
GFXPalette::initQuantizationInfo()
{
   stripQuantizationInfo();

   m_pColorWeights = new float[256];
   memset(m_pColorWeights, 0, sizeof(float) * 256);
}

void
GFXPalette::stripQuantizationInfo()
{
   delete [] m_pColorWeights;
   m_pColorWeights = NULL;
}

