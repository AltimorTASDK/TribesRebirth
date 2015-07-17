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
#include "g_font.h"
#include "g_bitmap.h"
#include "g_pal.h"
#include "resManager.h"

IMPLEMENT_PERSISTENT_TAG(GFXFont,         FOURCC('P','F','O','N'));

static void SetMonoBits( GFXBitmap *pBitmap, UInt32 obgc, UInt32 nfgc, UInt32 nbgc, Int32 flags );
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
#define GFXFONT_VERSION 0x00000000

static inline UInt8 max_UInt8(UInt8 v0, UInt8 v1) 
{ 
	if (v0 > v1) return(v0);
	else         return(v1);
}

//========================================================================
//                       Constructor & Destructor
//========================================================================

GFXFont::GFXFont( void )
{
	fi.scale.x = fi.scale.y = 1<<16;          // default scale factor of 1.0
	fi.foreColor          = 255;
	fi.backColor          = 0;
	fi.flags              = FONT_PROPORTIONAL;
	pPal                  = NULL;
	missing               = NULL;
	fi.numChars           = 0;
	charTable             = NULL;
	charTableLookup       = NULL;
	charTableLookupSize   = 0;
	charTableLookupOffset = 0;
}

GFXFont::~GFXFont()                    // deletes this font
{
	if (charTable)       
		delete [] charTable;
	if (charTableLookup) 
		delete [] charTableLookup;
	if (missing)
		delete missing;
	bma.free();
}


//========================================================================

void GFXFont::dump()
{
}



//========================================================================
//
// getCharInfo()
//
// Description:
//   Gets character information and bitmap for ASCII code denoted by
//   the index parameter.
//
//   This function should be used for getting character specifiec 
//   information, and/or the bitmap assiciated with a character.
//
// Arguments:
//   index    - ascii code of character requested
//   bmp      - address of a bitmap pointer (NULL ok)
//   charInfo - address of a pointer to GFXCharInfo	(NULL ok)
//
// Notes:
//   bmp and charInfo parameters may be NULL, if so, they simply will not
//   be assigned.

void GFXFont::getCharInfo(int index, const GFXBitmap **bmp, RectI *in_subRegion, GFXCharInfo **charInfo)
{
	static GFXBitmap bitmap;

	// Lookup char info and create a sub-bitmap for drawing this font
	int lookupTableIndex;
   if (fi.flags & FONT_LOWERCAPS) lookupTableIndex = toupper(char(index)) - charTableLookupOffset;
   else lookupTableIndex = index - charTableLookupOffset;
	int charTableIndex;

	if ((lookupTableIndex < 0) || 
	    (lookupTableIndex > charTableLookupSize) ||
        ((charTableIndex = charTableLookup[lookupTableIndex]) == -1))
	{
		// The requested index is not in the character set, create a
		// "missing" bitmap (pink box) and return that instead.

	 	static GFXCharInfo missingInfo;
		missingInfo.bitmapNo = 0;
		missingInfo.baseline = 0;
		missingInfo.width    = max_UInt8(UInt8(fi.fontWidth), UInt8(5));
		missingInfo.height   = max_UInt8(UInt8(fi.fontHeight), UInt8(1));
		missingInfo.x        = 0;
		missingInfo.y        = 0;

		if (bmp)
		{
			if (!missing || missing->getWidth() != missingInfo.width || missing->getHeight() != missingInfo.height)
			{
				delete missing;
				missing = GFXBitmap::create(missingInfo.width, missingInfo.height);
				memset(missing->pBits, 253, missing->getStride()*missing->getHeight()); //fill with hot pink! 
			}
			// can't call bitmap.free(), so we have to do it manually...
			if (bitmap.pBits && (bitmap.attribute & BMA_OWN_MEM))
				delete [] bitmap.pBits;
			if (bitmap.pPalette)
				delete bitmap.pPalette;
			bitmap.pPalette   = NULL;
			bitmap.width      = missing->width;
			bitmap.height     = missing->height;
			bitmap.stride     = missing->stride;
			bitmap.imageSize  = missing->imageSize;
			bitmap.attribute  = missing->attribute & ~BMA_OWN_MEM;
			bitmap.bitDepth   = missing->bitDepth;
			bitmap.pBits      = missing->pBits;
			*bmp = &bitmap;

		}
		if (charInfo)
			*charInfo = &missingInfo;
    }
	else
	{
		GFXCharInfo *cInfo = &(charTable[charTableIndex]);
		if (charInfo)
			// The user wants the character information, look it up
	 		*charInfo = cInfo;

		if (bmp)
		{
			// The client wants the bitmap, cut it out of the bitmap sheet
			*in_subRegion = RectI(cInfo->x, cInfo->y, 
			            cInfo->x + cInfo->width - 1,
						cInfo->y + cInfo->height - 1);
			GFXBitmap *bitmapSheet = *(bma.array + cInfo->bitmapNo);

			//bitmap.makeSubBitmap(bitmapSheet, &bound);
			//bitmap.attribute     |= (bitmapSheet->attribute & (BMA_TRANSPARENT | BMA_TRANSLUCENT));
			//*bmp = &bitmap;
         *bmp = bitmapSheet;
		}
	}
}

//========================================================================
//   
// NAME  GFXFont::getStrWidth
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

Int32 GFXFont::getStrWidth(const void *in_string) 
{
   AssertFatal(in_string,"GFXFont::getStrWidth:  null string");

	Int32  totalWidth = 0;
	UInt32 index;

	GFXCharInfo *cInfo;
   RectI rect;

	const void *cptr = in_string;

	if (fi.flags & FONT_UNICODE)
		index = (UInt32)(*(WORD *)cptr);
	else
		index = (UInt32)(*(BYTE *)cptr);

	while (index)
	{
		if (fi.flags & FONT_FIXED)
			totalWidth += fi.fontWidth + fi.spacing;
		else
		{
			getCharInfo(index, NULL, &rect, &cInfo);
			totalWidth += cInfo->width + fi.spacing;
		}

		// See note above about MS and Wat compiler problems...
		if ( fi.flags&FONT_UNICODE )
		{
			cptr  = (const char *) (((const WORD *)cptr) + 1);
			index = (UInt32)(*(WORD *)cptr);
		}
		else
		{
			cptr = ((const char *)cptr) + 1;
			index = (UInt32)(*(BYTE *)cptr);
		}
	}

	return(((totalWidth - fi.spacing)*fi.scale.x) >> 16 );
}

//========================================================================
//   
// NAME  GFXFont::getWordWidth
//   
// DESCRIPTION
//         Returns length of the first word in pixels.  A word is a 
//         token separated by space characters.   
//       
// ARGUMENTS 
//       char *in_string - pointer to character string
//       
// RETURNS 
//       Width of first word within *in_string in pixels
//       
// NOTES 
//       
//========================================================================

Int32 GFXFont::getWordWidth(const void *in_string) 
{
	Int32  totalWidth = 0;
	UInt32 index;

	GFXCharInfo *cInfo;
   RectI rect;

	const void *cptr = in_string;

   // ignore space characters at start of string
   if (fi.flags & FONT_UNICODE)
   {
      index = (UInt32)(*(WORD *)cptr);
      while (index && iswspace(index))
         cptr  = (const char *) (((const WORD *)cptr) + 1);  
   }
   else
   {
      index = (UInt32)(*(BYTE *)cptr);
      while (index && isspace(index))
         cptr = ((const char *)cptr) + 1;
   }

	while (index)
	{
		if (fi.flags & FONT_FIXED)
			totalWidth += fi.fontWidth + fi.spacing;
		else
		{
			getCharInfo(index, NULL, &rect, &cInfo);
			totalWidth += cInfo->width + fi.spacing;
		}

		// See note above about MS and Wat compiler problems...
		if ( fi.flags&FONT_UNICODE )
		{
			cptr  = (const char *) (((const WORD *)cptr) + 1);
			index = (UInt32)(*(WORD *)cptr);
         if (iswspace(index))
            break;
		}
		else
		{
			cptr = ((const char *)cptr) + 1;
			index = (UInt32)(*(BYTE *)cptr);
         if (isspace(index))
            break;
		}
	}

	return(((totalWidth - fi.spacing)*fi.scale.x) >> 16 );
}

//========================================================================
//   
// NAME  GFXFont::getStrHeight
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
//       parameter in the GFXFontInfo structure in the font.  This
//       contains the maximum height of the font and will ensure that
//       words are aligned in a sentence.  This function only returns
//       the maximum height of a character within a single string.
//       Use with caution.
//       
//========================================================================

Int32 GFXFont::getStrHeight( const void *in_string)
{
	Int32  maxHeight = 0;
	Int32  height;
	UInt32 index;
	GFXCharInfo *cInfo;
   RectI rect;

	const void *cptr = in_string;

	if (fi.flags & FONT_UNICODE)
		index = (UInt32)(*(WORD *)cptr);
	else
		index = (UInt32)(*(BYTE *)cptr);

	while (index)
	{
		getCharInfo(index, NULL, &rect, &cInfo);
		height = cInfo->height;
		if (maxHeight <  height)
			maxHeight = height;

		if (fi.flags & FONT_UNICODE)
		{
			cptr  = (const char *) (((const WORD *)cptr) + 1);
			index = (UInt32)(*(WORD *)cptr);
		}
		else
		{
			cptr  = ((const char *)cptr) + 1;
			index = (UInt32)(*(BYTE *)cptr);
		}
	}

   return((maxHeight * fi.scale.y) >> 16 );
}

//========================================================================
//   
// NAME  GFXFont::remapColors
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

void  GFXFont::remapColors(BYTE remapTable[256])
{
   int i;
   fi.backColor = 0;
   for(i = 0; i < bma.numBitmaps; i++)
      RemapBitmap(bma.array[i], remapTable);
}

void   GFXFont::remapColors( GFXPalette *in_pal )
{
   if ( in_pal )
      fi.backColor = 0;
}

//========================================================================
//   
// NAME  GFXFont::shadeColors
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

void     GFXFont::shadeColors( GFXPalette *in_pal)
{
   if ( in_pal )
      fi.backColor = 0;
}

//========================================================================
//   
// NAME  GFXFont::setForeColor
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

void GFXFont::setForeColor( Int32 in_color )
{
   if (fi.flags & FONT_MONO )
   {
      for ( int i=0; i<bma.numBitmaps; i++ )
         SetMonoBits( bma.array[i], NULL, in_color, NULL, TRANSLATE_FOREGROUND );
      fi.foreColor = in_color;
   }
}

//========================================================================
//   
// NAME  GFXFont::setBackColor
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

void GFXFont::setBackColor( Int32 in_color )
{
   if (fi.flags & FONT_MONO )
   {
      for ( int i=0; i<bma.numBitmaps; i++ )
         SetMonoBits( bma.array[i], fi.backColor, NULL, in_color, TRANSLATE_BACKGROUND );
      fi.backColor = in_color;
   }
}

//========================================================================

Persistent::Base::Error GFXFont::read(StreamIO &stream, int, int)
{
	// Read the general font info
	if (!stream.read((DWORD)sizeof(GFXFontInfo), (BYTE *)&fi))
		return(ReadError);

	// Read the char lookup table characteristics
	if (!stream.read((DWORD)sizeof(Int16), (BYTE *)&charTableLookupSize))   
		return(ReadError);
	if (!stream.read((DWORD)sizeof(Int16), (BYTE *)&charTableLookupOffset)) 
		return(ReadError);

	// Allocate and then read the char lookup table
	charTableLookup = new Int16[charTableLookupSize];
	if (!charTableLookup)
		return(ReadError);
	if (!stream.read((DWORD)charTableLookupSize*sizeof(Int16),
	                 (BYTE *)charTableLookup))
		return(ReadError);

	// Allocate and then read the char info table
	charTable = new GFXCharInfo[fi.numChars];
	if (!charTable)
	{
		delete [] charTableLookup;
		charTableLookup = NULL;
		return(ReadError);
	}

	if (!stream.read(fi.numChars*sizeof(GFXCharInfo),
	                 (BYTE *)charTable))
	{
		delete [] charTableLookup;
		charTableLookup = NULL;
		delete [] charTable;
		charTable = NULL;
		return(ReadError);
	}

	// Read the bitmap array
	bma.read(&stream, NULL);

	// Find out if there is a palette attached
	DWORD paletteIsAttached;
	if (!stream.read((DWORD *)&paletteIsAttached))
	{
		delete [] charTableLookup;
		charTableLookup = NULL;
		delete [] charTable;
		charTable = NULL;
		return(ReadError);
	}

	// If there is a palette, read it, otherwise our work here is done
	if (paletteIsAttached)
		pPal = GFXPalette::load(&stream);
	else
		pPal = NULL;

	return(Ok);
}

//========================================================================
//   
// NAME  GFXFont::load
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

GFXFont* GFXFont::load(const char *in_name)
{
	HANDLE fileHandle = CreateFile(in_name,
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
	if ( fileHandle == INVALID_HANDLE_VALUE )
	{
		AssertWarn(0, avar("GFXFont::load: Unable to load font %s, CreateFile for reading failed.", in_name));
		return(FALSE);
	}
	FileRStream frs(fileHandle);

	GFXFont *font = load(&frs);
	frs.close();
	CloseHandle(fileHandle);
	return(font);
}

//========================================================================
//   
// NAME  GFXFont::load
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

GFXFont* GFXFont::load(StreamIO *in_stream)
{
	Persistent::Base    *base;
	GFXFont *font;
	Error    error;
	base = Persistent::Base::load(*in_stream, &error);

	font = dynamic_cast<GFXFont *>(base);
	if (!font && base)
		// persist loaded something...but it isn't a font
		delete base;

	return(font);
}

//========================================================================
//   
// NAME  GFXFont::read
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

// generic read(char *) is no longer supported, Base::load() should
// be used instead.  

#if 0
Bool GFXFont::read( const char *in_name )
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
      AssertWarn(0, avar("GFXFont::load: Unable to load font %s, CreateFile for reading failed.", in_name));
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
      AssertWarn(0, avar("GFXFont::load: Unable to load font %s, StreamIO for reading failed.", in_name));
      return FALSE;
   }
}
#endif

//========================================================================
//   
// NAME  GFXFont::read
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
#if 0
Bool GFXFont::read( StreamIO *in_stream )
{
	return(read(*in_stream, version(), 0) == Ok ? true : false);
}
#endif

//========================================================================

Persistent::Base::Error GFXFont::write(StreamIO &stream, int, int)
{
	// Write the general font info structure
	stream.write((DWORD)sizeof(GFXFontInfo), (BYTE *)&fi);

	// Write the char lookup table characteristics
	stream.write((DWORD)sizeof(Int16), (BYTE *)&charTableLookupSize);
	stream.write((DWORD)sizeof(Int16), (BYTE *)&charTableLookupOffset);

	// Write the char lookup table data
	stream.write((DWORD)charTableLookupSize*sizeof(Int16), 
	                 (BYTE *)charTableLookup);

	// Write the char info table data
	stream.write((DWORD)fi.numChars*sizeof(GFXCharInfo),
	              (BYTE *)charTable);

	// Write the bitmap array
	bma.write(&stream, NULL); 

	// Write (conditionally) the palette
	if (pPal)
	{
		stream.write((DWORD)true);      // palette is attached
		pPal->write(&stream);  
	}
	else
		stream.write((DWORD)false);     // palette is NOT attached
   
	return(Ok);
}

int 
GFXFont::version()
{
   return (2);
}


//========================================================================
//   
// NAME  GFXFont::write
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

Bool GFXFont::write( const char *in_name )
{
   FileWStream fws;
   if (fws.open(in_name) == false) {
      AssertWarn(0, avar("GFXFont::write: Unable to write font %s, CreateFile for writing failed.", in_name));
      return false;
   }

   Bool result = write(&fws);
   fws.close();

   if ( result )
      return TRUE;
   else
   {
      AssertWarn(0, avar("GFXFont::write: Unable to write font %s, StreamIO for writing failed.", in_name));
      return FALSE;
   }
}

//========================================================================
//   
// NAME  GFXFont::write
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

Bool GFXFont::write( StreamIO *in_stream )
{
	return(store(*in_stream) == Ok ? true : false);
}

//========================================================================
//   
// NAME  GFXFont::getSaveSize
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

DWORD GFXFont::getSaveSize( void )
{
	DWORD size = 0;

	// general font info structure
	size += sizeof(GFXFontInfo);

	// char lookup table characteristics
	size += sizeof(Int16);      // charTableLookupSize
	size += sizeof(Int16);      // charTableLookupOffset

	// char lookup table data
	size += charTableLookupSize*sizeof(Int16);

	// char info table data
	size += fi.numChars*sizeof(GFXCharInfo);

	// bitmap array
	size += bma.getSaveSize();

	// Write (conditionally) the palette
	size += sizeof(DWORD);       // T/F flag of whether pal is attached

	if (pPal)
		size += pPal->getSaveSize();

	return(size);
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

void SetMonoBits( GFXBitmap *pBitmap, UInt32 obgc, UInt32 nfgc, UInt32 nbgc, Int32 flags )
{
   BYTE  *bPtr;      // bit pointer

   bPtr = pBitmap->pBits;  // get pointer to bitmaps bits

   for ( int i=0; i<pBitmap->imageSize; i++)
   {
      if ( (flags&TRANSLATE_BACKGROUND) && (UInt32(*bPtr) == obgc) ) // if color is old background color, set to new color
         *bPtr = (BYTE) nbgc;
      else if ( (flags&TRANSLATE_FOREGROUND) && (UInt32(*bPtr) != obgc) )
         *bPtr = (BYTE) nfgc;     // else set it to new foreground color
      bPtr++;
   }
}

//========================================================================
//   
// NAME  GFXFont::wrapStr
//
// DESCRIPTION
//       modifies *str so that it will word wrap within the
//       specified width.  Carriage returns are removed
//       and replaced with spaces or spaces are removed and
//       replaced by carriage returns.    
//       
// ARGUMENTS 
//       str   - String to be modified
//       width - wrapping width in pixels
// RETURNS 
//       
// NOTES 
//       Tabs are not supported.
//       
//========================================================================
void GFXFont::wrapStr(char *str, Int32 width)
{
   Int32 spaceLen; // length of a space character
   Int32 lineLen;  // length of current line
   Int32 wordLen;  // length of current word
   char *lastSpace;
      
   spaceLen = getWidth(' ') + getSpacing();

   lineLen = 0;
   lastSpace = NULL;
   while (*str)
   {
      // convert spaces
      if (isspace(*str))
      {
         lineLen += spaceLen;
         *str = ' ';
         lastSpace = str;
         str++;
      }
      else 
      {
         wordLen = getWordWidth((void *)str);
         if ((lineLen + wordLen) > width)
         {
            if (lastSpace)
            {
               *lastSpace = '\n';
               lastSpace = NULL;
               lineLen = 0;
            }  
         }
         lineLen += wordLen;
         while (*str && (! isspace(*str)) )
            str++;
      }           
   }
}


//========================================================================

//========================================================================
//
// GFXObsolete class is used only for asserting that a client attempted
// to load an obsolete font
//
//========================================================================

class GFXObsoleteFont : public Persistent::VersionedBase
{
  public:
	DECLARE_PERSISTENT(GFXObsoleteFont);
	Persistent::Base::Error read(StreamIO &, int version, int user);
	Persistent::Base::Error write(StreamIO &, int version, int user);
   void dump();
	int  version();
};
 
IMPLEMENT_PERSISTENT_TAG(GFXObsoleteFont, FOURCC('P','F','N','T'));
Persistent::Base::Error GFXObsoleteFont::read(StreamIO &, int, int)
{
	AssertFatal(0, avar("GFXObsoleteFont::read:  Attempt to load an obsolete font."));
	return(WriteError);
}

int 
GFXObsoleteFont::version()
{
   return(0);
}

Persistent::Base::Error 
GFXObsoleteFont::write(StreamIO &, int /*version*/, int /*user*/)
{
   return(WriteError);
}

void 
GFXObsoleteFont::dump()
{
   //
}

