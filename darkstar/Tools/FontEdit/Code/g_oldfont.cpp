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
// DESCRIPTION:   GFX Font Class Member Functions
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//
//========================================================================

#include <filstrm.h>
#include "g_surfac.h"
#include "g_barray.h"
#include "g_oldfont.h"
#include "g_font.h"

#define max(a,b) ( ((a) > (b)) ? (a) : (b) )
#define min(a,b) ( ((a) > (b)) ? (b) : (a) )
					
static void OLDSetMonoBits( GFXBitmap *pBitmap, UInt32 obgc, UInt32 nfgc, UInt32 nbgc, Int32 flags );
#define TRANSLATE_FOREGROUND  (1<<1)
#define TRANSLATE_BACKGROUND  (1<<2)

//========================================================================
//                Definitions and Structure Declerations
//========================================================================

#define DEREF(x,y)  ((y->fi.flags&FONT_UNICODE)? (UInt32 *)x:(char *)x)
#define NEXT(x,y)   ( x += (y->fi.flags&FONT_UNICODE)? 2:1)

struct Header
{
   DWORD ver_nc;
   DWORD numBitmaps;
};

//increment high byte if bitmap array format becomes incompatible with
//the old reader.
#define GFXOldFont_VERSION 0x00000000


//========================================================================
//                       Constructor & Destructor
//========================================================================

GFXOldFont::GFXOldFont( void )
{
   fi.scale.x = fi.scale.y = 1<<16;          // default scale factor of 1.0
   fi.foreColor = 255;
   fi.backColor = 0;
   fi.flags = FONT_PROPORTIONAL;
   pPal = NULL;
   missing = NULL;
}

GFXOldFont::~GFXOldFont()                    // deletes this font
{
   bma.free();
   delete missing;
}

//========================================================================
//   
// NAME  GFXDrawText_p
//   
// DESCRIPTION
//         Draws an ASCIIZ string at given coordinates
//       
// ARGUMENTS 
//       *io_pSurface- pointer to graphics buffer
//       *in_font    - pointer to font to be used
//       *in_atRect  - x,y coordinates where character is drawn
//       *in_char    - ASCIIZ character string
//       
// RETURNS 
//       Position at end of string
//       
// NOTES 
//       This function draws the character indexed by the ASCII code
//       into the graphics buffer specified using font specified.
//       The x,y coordinates specifies the top-left corner of the
//       output field.  The character's vertical position is adjusted 
//       based on the font's baseline and the characters baseline
//       values.
//       
//========================================================================

Int32 GFXOLDDrawText_p(
   GFXSurface        *io_pSurface,
   GFXOldFont           *in_font,
   const Point2I      *in_at,
   const void        *in_string )
{
   Point2I   pt;
   Point2I   pt2;
   UInt32   index;
   RectI     extent( 9999,9999,-1,-1 );
	RectI     *lastRect;
   GFXBitmap *bmp;
   GFXOldCharInfo *charInfo;

   pt.x = in_at->x;

   const void *cptr=in_string;

   if ( in_font->fi.flags&FONT_UNICODE )
      index = (UInt32)(*(WORD *)cptr);
   else
      index = (UInt32)(*(BYTE *)cptr);

	lastRect = io_pSurface->getLastRect();
   while (index)
   {
      (const_cast<GFXOldFont*>(in_font))->getCharInfo(index, &bmp, &charInfo);
      // adjust char's bitmap position based on font's baseline
      // and char's baseline.

      if ( (in_font->fi.scale.x != (1<<16)) || (in_font->fi.scale.y != (1<<16)) )   // if font scaled
      {
         pt.y = in_at->y +
                ((in_font->fi.baseline +
                charInfo->baseline - bmp->getHeight()) * in_font->fi.scale.y) / 65536;

         pt2.x = (bmp->getWidth()*in_font->fi.scale.x)/65536;
         pt2.y = (bmp->getHeight()*in_font->fi.scale.y)/65536;

         io_pSurface->drawBitmap2d( bmp, &pt, &pt2 );
         extent.upperL.x = min( lastRect->upperL.x, extent.upperL.x );
         extent.upperL.y = min( lastRect->upperL.y, extent.upperL.y );
         extent.lowerR.x = max( lastRect->lowerR.x, extent.lowerR.x );
         extent.lowerR.y = max( lastRect->lowerR.y, extent.lowerR.y );

         if (in_font->fi.flags&FONT_FIXED)
            pt.x += ((in_font->fi.fontWidth*in_font->fi.scale.x)/65536) + in_font->fi.spacing;
         else
            pt.x += ((bmp->getWidth()*in_font->fi.scale.x)/65536) + in_font->fi.spacing;
      }
      else                    // font is not scaled
      {
         pt.y = in_at->y +
                in_font->fi.baseline + 
                charInfo->baseline - bmp->getHeight();

         io_pSurface->drawBitmap2d( bmp, &pt );
         
         extent.upperL.x = min( lastRect->upperL.x, extent.upperL.x );
         extent.upperL.y = min( lastRect->upperL.y, extent.upperL.y );
         extent.lowerR.x = max( lastRect->lowerR.x, extent.lowerR.x );
         extent.lowerR.y = max( lastRect->lowerR.y, extent.lowerR.y );

         if (in_font->fi.flags&FONT_FIXED)
            pt.x += in_font->fi.fontWidth + in_font->fi.spacing;
         else
            pt.x += bmp->getWidth() + in_font->fi.spacing;
      }

		// Both Watcom and MSVC have problems with the following:
      // ((const FOO *)cptr)+=1;
      // MS doesn't understand that left side IS an Lvalue
      // WAT has apparent compiler bug that compiles 
      // ((const char*)cptr)+=1; to *((const char *)cptr)+=1;

      if ( in_font->fi.flags&FONT_UNICODE )
      {
         cptr = (const char *) (((const WORD *)cptr) + 1);
         index = (UInt32)( *(WORD *)cptr );
      }
      else
      {
         cptr = ((const char *)cptr) + 1;
         index = (UInt32)( *(BYTE *)cptr );
      }
   }

   *lastRect = extent;

   return ( lastRect->lowerR.x );
}

//========================================================================
//   
// NAME  GFXDrawText_r
//   
// DESCRIPTION
//         Draws a text string justified to the rectangle
//       
// ARGUMENTS 
//       *io_pSurface - pointer to graphics buffer
//       *in_font - pointer to font to be used
//       *in_at   - x,y coordinates where character is drawn
//       *in_string - ASCIIZ character string
//       
// RETURNS 
//       position at end of string
//       
// NOTES 
//       This function draws the character indexed by the ASCII code
//       into the graphics buffer specified using font specified.
//       The x,y coordinates specifies the top-left corner of the
//       output field.  The string is justified inside the rectangle
//       based on its justification flags.
//       
//========================================================================

Int32 GFXOLDDrawText_r(
   GFXSurface        *io_pSurface,
   GFXOldFont           *io_font,
   const RectI        *in_at,
   const void        *in_string )
{
   Point2I   pt;
   Point2I   oldScale;                  // holds old scale for TEXT_FIT
   Int32    newx;                      // holds new right x position after draw

   // if TEXT_FIT, save off original scale and modify scale values
   // so that text fits inside of in_at rectangle.

   if ( (io_font->fi.justification & TEXT_FIT) != NULL )
   {
      oldScale = io_font->fi.scale;
      io_font->fi.scale.x = ( (in_at->lowerR.x - in_at->upperL.x + 1) << 16) /
                                io_font->getStrWidth(in_string);

      io_font->fi.scale.y = ( (in_at->lowerR.y - in_at->upperL.y + 1) << 16) /
                                io_font->fi.fontHeight;
   }

   pt.x = in_at->upperL.x;             // default for x position
   pt.y = in_at->upperL.y;             // default for y position

   // adjust strings x start based on horizontal justification
   switch ( io_font->fi.justification&(TEXT_LEFT|TEXT_RIGHT|TEXT_CENTER_H) )
   {
      case TEXT_RIGHT:
         if ( io_font->fi.scale.x != (1<<16) )
            pt.x = in_at->lowerR.x -
                   (((io_font->getStrWidth(in_string) - 1) *
                   io_font->fi.scale.x)/65536);
         else
            pt.x = in_at->lowerR.x - io_font->getStrWidth(in_string) + 1;
         break;

      case TEXT_CENTER_H:
         if ( io_font->fi.scale.x != (1<<16) )
            pt.x += ((in_at->lowerR.x - pt.x) -
                    (((io_font->getStrWidth(in_string) - 1) *
                    io_font->fi.scale.x)/65536))/2;
         else
            pt.x += ((in_at->lowerR.x - pt.x) - io_font->getStrWidth(in_string))/2;
         break;

      case TEXT_LEFT:
      default:
         break;
   }

   // adjust strings y based on vertical justification
   switch ( io_font->fi.justification&(TEXT_TOP|TEXT_BOTTOM|TEXT_CENTER_V) )
   {
      case TEXT_BOTTOM:
         if ( io_font->fi.scale.y != (1<<16) )
            pt.y = in_at->lowerR.y -
                   (((io_font->fi.baseline - 1) *
                   io_font->fi.scale.y)/65536);
         else
            pt.y = in_at->lowerR.y - io_font->fi.baseline + 1;
         break;

      case TEXT_CENTER_V:
         if ( io_font->fi.scale.y != (1<<16) )
            pt.y += ((in_at->lowerR.y - pt.y) -
                    (((io_font->fi.baseline - 1) *
                    io_font->fi.scale.y)/65536))/2;
         else
            pt.y += ((in_at->lowerR.y - pt.y) - io_font->fi.baseline)/2;
         break;

      case TEXT_TOP:
      default:
         break;
   }

   // now draw string at the adjusted coordinates
   newx = GFXOLDDrawText_p( io_pSurface, io_font, &pt, in_string );

   if ( (io_font->fi.justification & TEXT_FIT) != NULL )
      io_font->fi.scale = oldScale;      // restore original scale value

   return (newx);
}

//========================================================================
//   
// NAME  GFXOldFont::getStrWidth
//   
// DESCRIPTION
//         Returns length of string in pixels
//       
// ARGUMENTS 
//       char *in_str   - pointer to character string
//       
// RETURNS 
//       Width of character string in pixels
//       
// NOTES 
//       
//========================================================================

Int32 GFXOldFont::getStrWidth( const void *in_string ) 
{
   Int32    totalWidth = 0;
   UInt32   index;

   const void *cptr=in_string;

   if ( fi.flags&FONT_UNICODE )
      index = (UInt32)(*(WORD *)cptr);
   else
      index = (UInt32)(*(BYTE *)cptr);

   while (index)
   {
      if (fi.flags&FONT_FIXED)
         totalWidth += fi.fontWidth + fi.spacing;
      else
      {
         GFXBitmap *bmp;
         getCharInfo(index, &bmp, NULL);
         totalWidth += bmp->getWidth() + fi.spacing;
      }

		// See note above about MS and Wat compiler problems...
      //
      if ( fi.flags&FONT_UNICODE )
      {
         cptr = (const char *) (((const WORD *)cptr) + 1);
         index = (UInt32)(*(WORD *)cptr);
      }
      else
      {
         cptr = ((const char *)cptr) + 1;
         index = (UInt32)(*(BYTE *)cptr);
      }
   }

   return ( ((totalWidth-fi.spacing) * fi.scale.x) >> 16 );
}

//========================================================================
//   
// NAME  GFXOldFont::getStrHeight
//   
// DESCRIPTION
//         Returns height of string in pixels
//       
// ARGUMENTS 
//       char *in_char  - pointer to string of characters
//       
// RETURNS 
//       Height of tallest character in string
//       
// NOTES 
//       *** WARNING ***  You are more likely to use the fontHeight
//       parameter in the GFXOldFontInfo structure in the font.  This
//       contains the \maximum height of the font and will ensure that
//       words are aligned in a sentence.  This function only returns
//       the maximum height of a character within a single string.
//       Use with caution.
//       
//========================================================================

Int32      GFXOldFont::getStrHeight( const void *in_string)
{
   Int32    maxHeight = 0;
   Int32    height;
   UInt32   index;

   const void *cptr=in_string;

   if ( fi.flags&FONT_UNICODE )
      index = (UInt32)(*(WORD *)cptr);
   else
      index = (UInt32)(*(BYTE *)cptr);

   while (index)
   {
      GFXBitmap *bmp;
      getCharInfo( index, &bmp, NULL);
      height = bmp->getHeight();
      if ( maxHeight <  height )
         maxHeight = height;

      if ( fi.flags&FONT_UNICODE )
      {
         cptr = (const char *) (((const WORD *)cptr) + 1);
         index = (UInt32)(*(WORD *)cptr);
      }
      else
      {
         cptr = ((const char *)cptr) + 1;
         index = (UInt32)(*(BYTE *)cptr);
      }
   }

   return ( (maxHeight * fi.scale.y) >> 16 );
}

//========================================================================
//   
// NAME  GFXOldFont::remapColors
//   
// DESCRIPTION
//         Remap font's bits to current palette
//       
// ARGUMENTS 
//       *in_pal - Pointer to palette to remap to
//       or remapTable[256] - array of new indices
//       
// RETURNS 
//       Nothing
//       
// NOTES 
//       This function maps the font's bits to the current palette.  It 
//       does this by finding the color in the current palette that most
//       closely matches the font's bit color.
//       
//========================================================================

static void RemapBitmap(GFXBitmap *bmp, BYTE remapTable[256])
{
   BYTE  *bPtr;      // bit pointer

   bPtr = bmp->pBits;  // get pointer to bitmaps bits

   for ( int i=0; i<bmp->imageSize; i++)
   {
      *bPtr = remapTable[*bPtr];
      bPtr++;
   }
}

void  GFXOldFont::remapColors (BYTE remapTable[256])
{
   int i;
   fi.backColor = 0;
   for(i = 0; i < bma.numBitmaps; i++)
      RemapBitmap(bma.array[i], remapTable);
}

void   GFXOldFont::remapColors( GFXPalette *in_pal )
{
   if ( in_pal )
      fi.backColor = 0;
}

//========================================================================
//   
// NAME  GFXOldFont::shadeColors
//   
// DESCRIPTION
//         Performs a 1-to-1 remap of the fonts bits to the specified palette
//       
// ARGUMENTS 
//       *in_pal  - pointer to the target shade palette
//       
// RETURNS 
//       Nothing
//       
// NOTES 
//       This function performs a remap of the fonts bits to the color
//       index specified in the shade palette.  
//       
//========================================================================

void     GFXOldFont::shadeColors( GFXPalette *in_pal)
{
   if ( in_pal )
      fi.backColor = 0;
}

//========================================================================
//   
// NAME  GFXOldFont::setForeColor
//   
// DESCRIPTION
//         For mono fonts, remaps all non-background colors to new color
//       
// ARGUMENTS 
//       *in_color  - new foreground color
//       
// RETURNS 
//       Nothing
//       
// NOTES 
//       Any color that is not a background color is mapped to the
//       new color.  non-mono fonts are not affected.
//       
//========================================================================

void     GFXOldFont::setForeColor( Int32 in_color )
{
   if (fi.flags & FONT_MONO )
   {
      for ( int i=0; i<bma.numBitmaps; i++ )
         OLDSetMonoBits( bma.array[i], NULL, in_color, NULL, TRANSLATE_FOREGROUND );
      fi.foreColor = in_color;
   }
}

//========================================================================
//   
// NAME  GFXOldFont::setBackColor
//   
// DESCRIPTION
//       remaps background color to new color
//       
// ARGUMENTS 
//       *in_color  - new background color
//       
// RETURNS 
//       Nothing
//       
// NOTES 
//       All fonts are affected
//       
//========================================================================

void     GFXOldFont::setBackColor( Int32 in_color )
{
   if (fi.flags & FONT_MONO )
   {
      for ( int i=0; i<bma.numBitmaps; i++ )
         OLDSetMonoBits( bma.array[i], fi.backColor, NULL, in_color, TRANSLATE_BACKGROUND );
      fi.backColor = in_color;
   }
}


void GFXOldFont::getCharInfo(int index, GFXBitmap **bmp, GFXOldCharInfo **charInfo)
{
   *bmp = bma[index];
   if (*bmp)
   {
      if (charInfo)
         *charInfo = (GFXOldCharInfo*)bma.getUserData(index);
   }
   else
   {
      static GFXOldCharInfo missingInfo;
      if (!missing || missing->getWidth() != fi.fontWidth || missing->getHeight() != fi.fontHeight)
      {
         delete missing;
         missing = GFXBitmap::create(max(Int32(fi.fontWidth>>1),Int32(5)), fi.fontHeight);
         memset(missing->pBits, 253, missing->getStride()*missing->getHeight()); //fill with hot pink! 
         missingInfo.baseline = 0;
         missingInfo.x = missingInfo.y = 0;
      }
      *bmp = missing;
      if (charInfo) *charInfo = &missingInfo;
   }
}

//========================================================================
//   
// NAME  GFXOldFont::load
//
// DESCRIPTION    
//       loads a font from a file
//       
// ARGUMENTS 
//       char *in_name - Filename of font to be loaded.
//       
// RETURNS 
//       If successful, a pointer to the font object.  Otherwise, NULL
//       
// NOTES 
//       This is a static function so that you can attempt to load the 
//       font before you actually instantiate a class object.
//       
//========================================================================

GFXOldFont* GFXOldFont::load( const char *in_name )
{
   GFXOldFont  *fPtr = new GFXOldFont;

   if (fPtr)
   {
      if (fPtr->read(in_name))
         return fPtr;
      else
         delete fPtr;
   }
   return NULL;
}

//========================================================================
//   
// NAME  GFXOldFont::load
//
// DESCRIPTION    
//       loads a font from a stream
//       
// ARGUMENTS 
//       char *in_stream   - StreamIO stream to load
//       
// RETURNS 
//       If successful, a pointer to the font object.  Otherwise, NULL
//       
// NOTES 
//       This is a static function so that you can attempt to load the 
//       font before you actually instantiate a class object.
//       
//========================================================================

GFXOldFont* GFXOldFont::load( StreamIO *in_stream )
{
   GFXOldFont  *fPtr = new GFXOldFont;

   if (fPtr)
   {
      if (fPtr->read(in_stream))
         return fPtr;
      else
         delete fPtr;
   }
   return NULL;
}

//========================================================================
//   
// NAME  GFXOldFont::read
//
// DESCRIPTION    
//       reads a font from a file
//       
// ARGUMENTS 
//       char *in_name - Filename of font to be loaded.
//       
// RETURNS 
//       TRUE if successful, otherwise FALSE
//       
// NOTES 
//       
//========================================================================

Bool GFXOldFont::read( const char *in_name )
{
   Bool           success;
   HANDLE         fileHandle;    // handle of opened file

   fileHandle = CreateFile(in_name,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL
                           );
   if ( fileHandle == INVALID_HANDLE_VALUE )
   {
      AssertWarn(0, avar("GFXOldFont::load: Unable to load font %s, CreateFile for reading failed.", in_name));
      return FALSE;
   }

   FileRStream frs(fileHandle);
   success = read( &frs );
   frs.close();
   CloseHandle( fileHandle );

   if ( success )
      return TRUE;
   else
   {
      AssertWarn(0, avar("GFXOldFont::load: Unable to load font %s, StreamIO for reading failed.", in_name));
      return FALSE;
   }
}

//========================================================================
//   
// NAME  GFXOldFont::read
//
// DESCRIPTION    
//       reads a font from a file stream
//       
// ARGUMENTS 
//       in_stream - file stream to open RIFF font file
//       
// RETURNS 
//       TRUE if successful, otherwise FALSE
//       
// NOTES 
//       
//========================================================================

Bool GFXOldFont::read( StreamIO *in_stream )
{
   Int32       numChunks = -1;
   DWORD       ver_nc;                 // combination version & number of chunks
   DWORD       data;
   DWORD       size;


   while (numChunks--)
   {
      in_stream->read(&data);
      in_stream->read(&size);
      switch (data)
      {

         case FOURCC('P','F','N','T'):
            break;

         case FOURCC('h','e','a','d'):
            in_stream->read( &ver_nc );
            AssertFatal( (ver_nc&0xff000000) <= GFXOldFont_VERSION, "GFXOldFont::read:  Cannot read newer version of font format");
            numChunks = ver_nc&0xffffff;  // get number of remaining chunks
            in_stream->read( sizeof(GFXOldFontInfo), (BYTE *)&fi );
            break;

         case FOURCC('P','P','A','L'):
            pPal = GFXPalette::load( in_stream );
            break;

         case FOURCC('P','B','M','A'):
            bma.read( in_stream, NULL );
            break;

         default:
            if (numChunks != -1)
               in_stream->setPosition(in_stream->getPosition()+size);
            else
               return (FALSE);
            break;
      }
   }

   return (TRUE);
}

//========================================================================
//   
// NAME  GFXOldFont::write
//
// DESCRIPTION    
//       writes a font to a file
//       
// ARGUMENTS 
//       char *in_name - Filename of font to be saved.
//       
// RETURNS 
//       TRUE if successful
//       
// NOTES 
//       
//========================================================================

Bool GFXOldFont::write( const char *in_name )
{
   HANDLE         fileHandle;    // handle of opened file
   Bool           success;

   fileHandle = CreateFile(in_name,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL
                           );
   if ( fileHandle == INVALID_HANDLE_VALUE )
   {
      AssertWarn(0, avar("GFXOldFont::write: Unable to write font %s, CreateFile for writing failed.", in_name));
      return FALSE;
   }

   FileWStream frs(fileHandle);
   success = write( &frs );
   frs.close();
   CloseHandle( fileHandle );

   if ( success )
      return TRUE;
   else
   {
      AssertWarn(0, avar("GFXOldFont::write: Unable to write font %s, StreamIO for writing failed.", in_name));
      return FALSE;
   }
}

//========================================================================
//   
// NAME  GFXOldFont::write
//
// DESCRIPTION    
//       writes a font to a file stream
//       
// ARGUMENTS 
//       in_stream - file stream to open RIFF font file
//       
// RETURNS 
//       TRUE if successful, otherwise FALSE
//       
// NOTES 
//       
//========================================================================

Bool GFXOldFont::write( StreamIO *in_stream )
{
   in_stream->write(FOURCC('P','F','N','T'));      // PFNT:sz
   in_stream->write( (DWORD)getSaveSize());

   in_stream->write(FOURCC('h','e','a','d'));      // head:sz
   in_stream->write((DWORD)(sizeof(DWORD) + sizeof(GFXOldFontInfo)));
   in_stream->write((DWORD)(GFXOldFont_VERSION + pPal? 2:1));     // 2 chunks if palette
   in_stream->write((DWORD)sizeof(GFXOldFontInfo), (BYTE *)&fi);

   if (pPal)
      pPal->write( in_stream );                    // PPAL:sz

   bma.write( in_stream, NULL );                   // PBMA:sz
   
   return TRUE;
}

//========================================================================
//   
// NAME  GFXOldFont::getSaveSize
//
// DESCRIPTION    
//       Gets a block image size for a font
//       
// ARGUMENTS 
//       
// RETURNS 
//       size of PFNT block, otherwise NULL
//       
// NOTES 
//       
//========================================================================

DWORD GFXOldFont::getSaveSize( void )
{
   //magic number 12 is size of chunk_ID + chunk_size + (version&numChunks)

   DWORD size;

   size = 12 + sizeof( GFXOldFontInfo );
   if (pPal)
      size += pPal->getSaveSize();

   size += bma.getSaveSize();

   return (size);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//±
//± NAME
//±   SetMonoBits( pBitmap, oldBackColor, newForeColor, newBackColor, flags)
//±   
//± DESCRIPTION
//±   This function translates foreground and background colors in a mono
//±   font to their new values.  Any color that is not a background color
//±   is considered to be a foreground color and is converted.
//±   
//±   flags are either TRANSLATE_FOREGROUND or TRANSLATE_BACKGROUND
//±   
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

void OLDSetMonoBits( GFXBitmap *pBitmap, UInt32 obgc, UInt32 nfgc, UInt32 nbgc, Int32 flags )
{
   BYTE  *bPtr;      // bit pointer

   bPtr = pBitmap->pBits;  // get pointer to bitmaps bits

   for ( int i=0; i<pBitmap->imageSize; i++)
   {
      if ( (flags&TRANSLATE_BACKGROUND) && ((UINT)*bPtr == obgc) ) // if color is old background color, set to new color
         *bPtr = (BYTE) nbgc;
      else if ( (flags&TRANSLATE_FOREGROUND) && ((UINT)*bPtr != obgc) )
         *bPtr = (BYTE) nfgc;     // else set it to new foreground color
      bPtr++;
   }
}


