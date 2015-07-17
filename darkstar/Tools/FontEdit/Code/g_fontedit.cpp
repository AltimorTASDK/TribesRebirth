//
// g_fontedit.cpp
//

#include <filstrm.h>
#include <g_surfac.h>
#include <g_barray.h>
#include <g_oldfont.h>
#include <g_font.h>
#include <fn_all.h>
#include "defines.h"
#include "g_fontedit.h"

// LARGETST_PACKING_SHEET_SIZE is the width&height of the largest
// sheet that bitmaps can be packed onto
#define LARGEST_PACKING_SHEET_SIZE 256

// MAX_BITMAPS and MAX_CHARS specify how big the edit buffers
// should be
#define MAX_BITMAPS                512
#define MAX_CHARS                  512

//IMPLEMENT_PERSISTENT_TAG(GFXFontEdit, FOURCC('P','F','O','N'));

////////////////////////////////////////////////////////////////////////

int fitBitmapInBitmap(GFXBitmap *dst, int *linePos, GFXBitmap *src)
{
	// Attempts to fit bitmap src into bitmap dst, where the ith element of
	// linePos specifies the first available column in dst on scanline i.
	//
	// Return value is the scanline that src can be placed at in dst,
	// or -1 if placement is not possible.

	int width = dst->width, height = dst->height;
	int i = 0, j;

	while (i < height)
	{
		// is there enough room on scanline i?
		if (src->width <= width - linePos[i])
		{
			// are there enough scanlines directly below i that can also
			// fit src into this column?
			j = i;

			while (j < height)
			{
				// is there room on line j at column linePos[i]?
				if (linePos[j] <= linePos[i])
				{
					// have we now found enough room to place src?
					if (1 + (j - i) == src->height)
						return(i);
					j++;
				}
				else
					// not enough room directly below, might as well try
					// a diff't scanline
					break;
			}
		}
		i++;
	}
	// Nope, not enough room
	return(-1);
}

////////////////////////////////////////////////////////////////////////

void zap(int *table, int size)
{
	for (int i = 0; i < size; i++) table[i] = 0;
}

////////////////////////////////////////////////////////////////////////

void placeBitmap(GFXBitmap *dst, int *linePos, int placeY, GFXBitmap *src)
{
	BYTE *srcBits, *dstBits;
	int   x, y;
	int   placeX = linePos[placeY];

	// copy bits from source to dest
	for (y = 0; y < src->height; y++)
	{
		srcBits = src->pBits + src->stride*y;
		dstBits = dst->pBits + dst->stride*(y + placeY) + placeX;
		for (x = 0; x < src->width; x++)
			*dstBits++ = *srcBits++;

		// Adjust the linePos table for the current scanline to reflect
		// the inserted bitmap
		linePos[y + placeY] = placeX + src->width;
	}
}

////////////////////////////////////////////////////////////////////////

GFXBitmap *newClearedBitmap(int w, int h)
{
	GFXBitmap *bitmap = GFXBitmap::create(w, h);
	if (bitmap)
		memset((void *)bitmap->pBits, 252, bitmap->imageSize);

	return(bitmap);
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::packBitmaps()
{
	int attribute = bma.numBitmaps > 0 ? bma.array[0]->attribute : 0;
	int i, j, thisHeight, prevHeight;

	// The first phase is to sort the chars on height so that the 
	// space allocation on the bitmaps is somewhat efficient
	int *sortedTable = new int[fi.numChars];

	// Simple insertion sort, slow, but tables shouldn't be longer
	// than about 256 elts	
	sortedTable[0] = 0;
	for (i = 1; i < fi.numChars; i++)
	{
		thisHeight = charTable[i].height;
		j = i;
		while (j > 0)
		{
			prevHeight = charTable[sortedTable[j - 1]].height;

			if (thisHeight < prevHeight)
			{
				sortedTable[j] = sortedTable[j - 1];
				j--;
			}
			else
				break;
		}
		sortedTable[j] = i;
	}

	// The second phase is to use the sort table to one by one
	// insert the char bitmaps into new bitmap sheets

	int         currentSheet, charIndex, ascii, placeY, firstOnThisSheet;
	int         linePos[LARGEST_PACKING_SHEET_SIZE];
	GFXBitmap **newArray = new GFXBitmap*[fi.numChars];
	GFXBitmap  *sheet, *charBitmap;
		
	// create an initial bitmap sheet
	currentSheet     = 0;
	sheet            = newClearedBitmap(LARGEST_PACKING_SHEET_SIZE,
	                                    LARGEST_PACKING_SHEET_SIZE);
	firstOnThisSheet = 0;
	zap(linePos, LARGEST_PACKING_SHEET_SIZE);

	i = 0;

	// Scan the ascii table
	while (i < fi.numChars)
	{
		charIndex = sortedTable[i];

		// grab the bitmap for this ascii into a temp sub-bitmap 
		charBitmap = getBitmapAbsolute(charIndex);

		// see if the bitmap can be placed on the sheet
		placeY = fitBitmapInBitmap(sheet, linePos, charBitmap);

		if (placeY == -1)
		{
			// no room on this sheet, move the sheet off to the
			// new bitmap array, create a new sheet, retry
			newArray[currentSheet] = sheet;
			sheet                  = newClearedBitmap(LARGEST_PACKING_SHEET_SIZE,
			                                          LARGEST_PACKING_SHEET_SIZE);
			firstOnThisSheet       = i;
			zap(linePos, 256);
			currentSheet++;
		}
		else
		{
			// adjust the char's info so that it can find its bitmap
			// on the new sheet
			charTable[charIndex].x        = linePos[placeY];
			charTable[charIndex].y        = placeY;
			charTable[charIndex].bitmapNo = currentSheet;

			// place the bitmap for the char on the sheet and
			// adjust the linePos table
			placeBitmap(sheet, linePos, placeY, charBitmap);

			// go on to the next char
			i++;
		}
	
		// get rid of the temporary sub-bitmap
		delete charBitmap;
	}

	// add the last sheet to the new array
	newArray[currentSheet] = sheet;

	// get rid of the old bitmap array and attach the new one
	bma.free();
	
	// attatch the new bitmap array
	bma.numBitmaps = currentSheet + 1;
	bma.array      = newArray;

	// fine, the last of the chars were packed onto a 256x256
	// sheet, lets see if we can pack the same chars onto a smaller
	// sheet (keep gettting smaller until we no longer can pack)
	
	Bool everythingFit;
	int  pass = 0;
	int  sWidth  = LARGEST_PACKING_SHEET_SIZE;
	int  sHeight = LARGEST_PACKING_SHEET_SIZE;
	GFXCharInfo *cache = new GFXCharInfo[fi.numChars - firstOnThisSheet];
		
	while (1)
	{
		// make a copy of the fontChar info from the chars on the last
		// sheet, so we can revert if we need to
		for (i = firstOnThisSheet; i < fi.numChars; i++)
			cache[i - firstOnThisSheet] = charTable[sortedTable[i]];
		
		// reduce the size of this bitmap from the last
		if ((pass++) % 2 == 0) sHeight /= 2;
		else                   sWidth  /= 2;

		zap(linePos, LARGEST_PACKING_SHEET_SIZE);

		sheet = newClearedBitmap(sWidth, sHeight);
		everythingFit = true;
		i = firstOnThisSheet;

		while (i < fi.numChars)
		{
			charIndex = sortedTable[i];

			// grab the bitmap for this ascii into a temp sub-bitmap 
			charBitmap = getBitmapAbsolute(charIndex);

			// see if the bitmap can be placed on the sheet
			placeY = fitBitmapInBitmap(sheet, linePos, charBitmap);

			if (placeY == -1)
			{
				everythingFit = false;
				break;
			}
			else
			{
				// adjust the char's info so that it can find its bitmap
				// on the new sheet
				charTable[charIndex].x        = linePos[placeY];
				charTable[charIndex].y        = placeY;
				charTable[charIndex].bitmapNo = currentSheet;

				// place the bitmap for the char on the sheet and
				// adjust the linePos table
				placeBitmap(sheet, linePos, placeY, charBitmap);

				// go on to the next char
				i++;
			}
		}

		if (everythingFit)
		{
			// delete the previous one that was too big
			delete newArray[currentSheet];
			// stick in the new one
			newArray[currentSheet] = sheet;
			// ...and try an even smaller size
		}
		else
		{
			// it didn't all fit, we should have stayed with the
			// previous size, so move the saved fontChar info back
			delete sheet;
			for (i = firstOnThisSheet; i < fi.numChars; i++)
				charTable[sortedTable[i]] = cache[i - firstOnThisSheet];
			break;
		}
	}
	delete [] cache;
	delete [] sortedTable;
	
	for (i = 0; i < bma.numBitmaps; i++)
		bma.array[i]->attribute = attribute;
}

////////////////////////////////////////////////////////////////////////

int GFXFontEdit::getCharTableIndex(int ascii)
{
	int lookupIndex = ascii - charTableLookupOffset;

	if (lookupIndex < 0 || lookupIndex >= charTableLookupSize)
		return(-1);
	return(charTableLookup[lookupIndex]);
}

////////////////////////////////////////////////////////////////////////
	
void GFXFontEdit::deleteChar(int ascii)
{
	// Delete the char by setting its lookup table value to zero and
	// decrementing the char count.
	// Proper cleanup of dangling bitmaps and FonCharInfo structures
	// will occur during packing

	charTableLookup[ascii - charTableLookupOffset] = -1;
	fi.numChars--;
	computeUsefulInformation();

	return;
}
												  
////////////////////////////////////////////////////////////////////////

void GFXFontEdit::pack()
{
	/*   pack data for this font, this means:
	 *
	 *   1. Truncate the charLookupTable as much as possible and
	 *      recompute charLookupTableOffset
	 *						 
	 *	 2. Truncate the charTable, removing dead wood (such as chars
	 *      deleted during editing)
	 *
	 *   3. Repack what chars remain onto new sheets
	 */ 

	if (staticSized)	// already packed
		return;

	int i;
	staticSized = true;

	// Truncate the char lookup table from MAX_CHARS elts to the
	// min. required to lookup firstASCII...lastASCII
	charTableLookupSize   = (lastASCII - firstASCII) + 1;
	charTableLookupOffset = firstASCII;
	Int16 *newCharTableLookup = new Int16[charTableLookupSize];
	for (i = 0; i < charTableLookupSize; i++)
		newCharTableLookup[i] = charTableLookup[i + firstASCII];
	delete [] charTableLookup;
	charTableLookup = newCharTableLookup;

	// Truncate the char table from MAX_CHARS to the minimum 
	// needed for fi.numChars chars
	GFXCharInfo *newCharTable = new GFXCharInfo[fi.numChars];
	int nextAvailable = 0;
	for (i = 0; i < charTableLookupSize; i++)
	{
		if (charTableLookup[i] != -1)
		{
			newCharTable[nextAvailable] = charTable[charTableLookup[i]];
			charTableLookup[i]          = nextAvailable;
			nextAvailable++;
		}	
	}
	delete [] charTable;
	charTable = newCharTable;

	packBitmaps();
	computeUsefulInformation();
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::unpack(GFXFont *font)
{
	// In order to allow easy editing of a font, we generate extra space
	// in the arrays for copying/pasting, etc.

	// if "font" is null then we are unpacking the current object
	// (this), so we need to make a copy of ourselves:
	if (font == NULL)
	{
		font = new GFXFont;
		font->bma.numBitmaps        = bma.numBitmaps;
		font->bma.array             = bma.array;
		font->fi                    = fi;
		font->charTable             = charTable;
		font->charTableLookup       = charTableLookup;
		font->charTableLookupOffset = charTableLookupOffset;
		font->charTableLookupSize   = charTableLookupSize;
		font->pPal                  = pPal;
	}

	// if "font" is non-null then we are unpacking a font that was
	// just read from a file.
	
	// defaultInit() will attempt to delete these arrays before it creates
	// the new ones, we want to keep our copies of them, and they'll be
	// deleted at the bottom
	bma.array       = NULL;  
	charTable       = NULL;
	charTableLookup = NULL;

	// now clobber everyting, create the unpacked space
	defaultInit();

	// now copy packed to unpacked
	fi = font->fi;
	int i;
	bma.numBitmaps = font->bma.numBitmaps;
	for (i = 0; i < bma.numBitmaps; i++)
	{
		bma.array[i]       = font->bma.array[i];
		font->bma.array[i] = NULL;  // so it doesn't get deleted immediately
	}
	font->bma.numBitmaps = 0;

	int index;
	int count = font->charTableLookupSize + font->charTableLookupOffset;

	for (i = 0; i < count; i++)
	{
		index = i - font->charTableLookupOffset;
		if (index >= 0)
			charTableLookup[i] = font->charTableLookup[index];
	}

	for (i = 0; i < fi.numChars; i++)
		charTable[i] = font->charTable[i];

	computeUsefulInformation();

	if (font->pPal)
	{
		pPal = font->pPal;
		font->pPal = NULL;       // so it doesn't get deleted immediately
	}
	fi.scale.x = 1<<16;
	fi.scale.y = 1<<16;
	
	delete font;
}

////////////////////////////////////////////////////////////////////////

void copyBits(GFXBitmap *dstBitmap, GFXBitmap *srcBitmap)
{
	int x, y;

	BYTE *src, *dst;

	for (y = 0; y < dstBitmap->height; y++)
	{
		src = srcBitmap->pBits + y*srcBitmap->stride;
		dst = dstBitmap->pBits + y*dstBitmap->stride;
		for (x = 0; x < dstBitmap->width; x++)
			*dst++ = *src++;
	}
}

////////////////////////////////////////////////////////////////////////

GFXBitmap *GFXFontEdit::getBitmap(int ascii)
{
	// plucks a bitmap for this ascii from a bitmap sheet

	int charTableIndex = getCharTableIndex(ascii);
	if (charTableIndex == -1)
	{
		// The requested index is not in the character set
		return(NULL);
	}
	
	return(getBitmapAbsolute(charTableIndex));
}

////////////////////////////////////////////////////////////////////////

GFXBitmap *GFXFontEdit::getBitmapAbsolute(int charTableIndex)
{
	// pluck a bitmap (based on index in charTable, not ascii code)

	GFXCharInfo *cInfo;
	GFXBitmap       *bitmap0, *bitmap1, *bitmapSheet;

	cInfo = &(charTable[charTableIndex]);

	RectI bound(cInfo->x, 
            cInfo->y, 
            cInfo->x + cInfo->width  - 1,
			cInfo->y + cInfo->height - 1);

	// get a pointer to the sheet containing the char we're grabbing
	bitmapSheet = bma.array[cInfo->bitmapNo];

	// get a sub bitmap, note this uses shared memory
	bitmap0 = bitmapSheet->createSubBitmap(&bound);

	// from the sub bitmap create a new stand-alone bitmap
    bitmap1 = GFXBitmap::create(cInfo->width, cInfo->height);
	bitmap1->attribute = bitmapSheet->attribute;
	copyBits(bitmap1, bitmap0);

	// delete the temporary sub bitmap (won't delete pBits data since
	// it shares with original sheet	
	delete bitmap0;
	return(bitmap1);
}

////////////////////////////////////////////////////////////////////////

GFXCharInfo *GFXFontEdit::getFontCharInfoOnly(int ascii)
{
	// returns only the char info, without allocating a sub-bitmap
	// (like the base class getFontCharInfo() does)

	int charTableIndex = getCharTableIndex(ascii);
	if (charTableIndex == -1)
	{
		// The requested index is not in the character set
		return(NULL);
	}
	return(&(charTable[charTableIndex]));
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::defaultInit()
{
	// called when a new font is asked for, create space for new chars
	// to be added 
	staticSized      = false;
	bma.free();
	bma.array        = new GFXBitmap*[MAX_BITMAPS];
	bma.userData     = NULL;
	bma.userDataSize = 0;
	bma.numBitmaps   = 0;
	fi.numChars      = 0;
	fi.fontWidth     = 0;
	fi.fontHeight    = 0;
	fi.flags        |= FONT_TRANSPARENT | FONT_MONO;
	fi.spacing       = 1;
	fi.foreColor     = 255;
	fi.backColor     = 0;
	fi.baseline      = 0;
	firstASCII       = LARGE_INT;
	lastASCII        = -1;
	maxCharBaseline  = 0;
	nextAvailableCharTableIndex = 0;

	if (charTable)
		delete charTable;
	if (charTableLookup)
		delete charTableLookup;
	charTable             = new GFXCharInfo[MAX_CHARS];
	charTableLookup       = new Int16[MAX_CHARS];
	charTableLookupOffset = 0;
	charTableLookupSize   = MAX_CHARS;

	memset((void *)charTable,       0, MAX_CHARS*sizeof(GFXCharInfo));
	memset((void *)bma.array,       0, MAX_BITMAPS*sizeof(GFXBitmap *));

	for (int i = 0; i < MAX_BITMAPS; i++)
		charTableLookup[i] = -1;
}

////////////////////////////////////////////////////////////////////////

bool GFXFontEdit::place(GFXBitmap *bitmap, GFXCharInfo *charInfo)
{
	// stick the new bitmap at the end of the bitmap array.  when
	// the font is saved, it'll get packed in with the rest of
	// the bitmaps

	if (staticSized)
		return(false);

	if (bma.numBitmaps == MAX_BITMAPS)
		return(false);

	int thisIndex = bma.numBitmaps;

	// make a COPY of the bitmap, the original gets deleted by whoever
	// asked for the insertion

	GFXBitmap *newBitmap = new GFXBitmap;
	newBitmap->width     = bitmap->width;
	newBitmap->height    = bitmap->height;
	newBitmap->stride    = bitmap->stride;
	newBitmap->bitDepth  = bitmap->bitDepth;
	newBitmap->imageSize = bitmap->imageSize;
	newBitmap->attribute = bitmap->attribute;
	newBitmap->pBits = new BYTE[bitmap->imageSize];
	memcpy(newBitmap->pBits, bitmap->pBits, bitmap->imageSize);

	bma.array[thisIndex] = newBitmap;
	bma.numBitmaps++;

	charInfo->bitmapNo = thisIndex;
	charInfo->x        = 0;
	charInfo->y        = 0;
	charInfo->width    = bitmap->width;
	charInfo->height   = bitmap->height;

	return(true);
}

////////////////////////////////////////////////////////////////////////

int GFXFontEdit::getFirstASCII()
{
	// return ASCII code of the first char in the char set
	return(fi.numChars == 0 ? -1 : firstASCII);
}

////////////////////////////////////////////////////////////////////////

int GFXFontEdit::getLastASCII()
{
	// return ASCII code of the last char in the char set
	return(fi.numChars == 0 ? -1 : lastASCII);
}

////////////////////////////////////////////////////////////////////////

Bool GFXFontEdit::insertBitmap(int asciiValue, GFXBitmap *bitmap, int baseline)
{
	// ALL INSERTIONS into a font come through here
	// insert the bitmap into this font, and give it the ASCII (lookup)
	// value specified by "asciiValue"

	if (staticSized)
		return(false);

	if (bitmap->width > 255 || bitmap->height > 255)
		// character bitmaps can't have dimensions above 255, because
		// the widht&height members of GFXCharInfo are only 8 bits ea.
		return(false);

	if (bitmap->width  > LARGEST_PACKING_SHEET_SIZE ||
	    bitmap->height > LARGEST_PACKING_SHEET_SIZE)
		// character bitmaps must be small enough to fit on the largest
		// of the packed sheets
		return(false);

	int thisIndex = nextAvailableCharTableIndex + 1;
	if (thisIndex == MAX_CHARS)
		// there is no more room in the buffers for another character,
		// the constant MAX_CHARS needs to be increased.
		return(false);
	nextAvailableCharTableIndex++;

	if (charTableLookup[asciiValue] != -1)
		// this ascii value is already being used, so delete the old char
		deleteChar(asciiValue);
	
	charTableLookup[asciiValue]   = thisIndex;
	charTable[thisIndex].baseline = baseline;
	
	if (!place(bitmap, &charTable[thisIndex]))
	{
		// there were no more bitmap buffers available for pasting,
		// the constant MAX_BITMAPS needs to be increased
		charTableLookup[asciiValue] = -1;
		return(false);
	}

	fi.fontWidth    = max(fi.fontWidth, bitmap->width);
	fi.fontHeight   = max(fi.fontHeight, bitmap->height);
	fi.baseline     = max(fi.baseline, bitmap->height - baseline);
	maxCharBaseline = max(maxCharBaseline, baseline);

	firstASCII = min(firstASCII, asciiValue);
	lastASCII  = max(lastASCII, asciiValue);
	fi.numChars++;

	return(true);
}

////////////////////////////////////////////////////////////////////////

Bool GFXFontEdit::appendBitmap(GFXBitmap *bitmap, int baseline)
{
	return(insertBitmap(lastASCII + 1, bitmap, baseline));
}

////////////////////////////////////////////////////////////////////////

GFXFontEdit::GFXFontEdit() : GFXFont()
{
	staticSized = true;
	fi.numChars = 0;
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::drawChar(GFXSurface *s, int ascii, Point2I *pos)
{
	char buffer[2];
	buffer[0] = char(ascii);
	buffer[1] = '\0';

	GFXDrawText_p(s, this, pos, buffer);
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::setFlag(int flag, bool value)
{
	if (value)
		fi.flags |= flag;
	else
		fi.flags &= (~flag);

	if (flag == FONT_TRANSPARENT)
	{
		for (int i = 0; i < bma.numBitmaps; i++)
			if (value)
				bma.array[i]->attribute |= BMA_TRANSPARENT;
			else
				bma.array[i]->attribute &= ~BMA_TRANSPARENT;
	}
	if (flag == FONT_TRANSLUCENT)
	{
		for (int i = 0; i < bma.numBitmaps; i++)
			if (value)
				bma.array[i]->attribute |= BMA_TRANSLUCENT;
			else
				bma.array[i]->attribute &= ~BMA_TRANSLUCENT;
	}
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::computeUsefulInformation()
{
	// Recompute info such as the first and last ASCII's in the char
	// set, the max char baseline, and the next available slot in
	// the character lookup table

	int i, charTableIndex;
	int maxCharTableIndex = -1;

	fi.fontWidth  = 0;
	fi.fontHeight = 0;

	firstASCII      = LARGE_INT;
	lastASCII       = -1;
	maxCharBaseline = 0;

	for (i = 0; i < charTableLookupSize; i++)
	{
		charTableIndex = charTableLookup[i];
		if (charTableIndex != -1)
		{
 			fi.fontWidth      = max(int(fi.fontWidth), int(charTable[charTableIndex].width));
			fi.fontHeight     = max(int(fi.fontHeight), int(charTable[charTableIndex].height));
			maxCharTableIndex = max(maxCharTableIndex, charTableIndex);
			maxCharBaseline   = max(maxCharBaseline, int(charTable[charTableIndex].baseline));
			lastASCII         = i + charTableLookupOffset;
			if (firstASCII == LARGE_INT)
				firstASCII = i + charTableLookupOffset;
		}
	}
	nextAvailableCharTableIndex = maxCharTableIndex + 1;	

	if (!(fi.flags & FONT_PROPORTIONAL))
		if (!(fi.flags & FONT_FIXED))
			// font is set to neither fixed nor proportional, so set it to one
			setFlag(FONT_PROPORTIONAL, true);
}
		
////////////////////////////////////////////////////////////////////////

GFXFontEdit::GFXFontEdit(GFXOldFont *oldFont) : GFXFont()
{
	// Create a font from an old font
	defaultInit();			 

	memcpy(&fi, &oldFont->fi, sizeof(oldFont->fi));
	fi.numChars = 0;

	GFXBitmap      *bitmap;
	GFXOldCharInfo *oldCharInfo;

	pPal = oldFont->pPal;
	oldFont->pPal = NULL;

	for (int i = 0; i < MAX_CHARS; i++)
	{
		oldFont->getCharInfo(i, &bitmap, &oldCharInfo);
		if (bitmap != oldFont->missing)
			insertBitmap(i, bitmap, oldCharInfo->baseline);
	}
	computeUsefulInformation();
}

////////////////////////////////////////////////////////////////////////

GFXFontEdit::GFXFontEdit(GFXFont *font) : GFXFont()
{
	// Create a fontEdit object from a font object.
	// "font" has just been read from disk, we want to make it into
	// a GFXFontEdit object, with lots of room (in the char tables)
	// for adding new characters

	unpack(font);
}

////////////////////////////////////////////////////////////////////////

Bool GFXFontEdit::save(const char *in_name)
{
	int justification = fi.justification;
	fi.justification = 0;
	pack();
   // although we are inherting from GFXFont, we
   // must cast <*this> to <GFXFont *> so that
   // we will get get the persistance of GFXFont
   // rather then GFXFontEdit
   GFXFont *gfxFont = new GFXFont(*this);
	Bool result = gfxFont->write(in_name);
	unpack(NULL);
	fi.justification = justification;

	return(result);
}

////////////////////////////////////////////////////////////////////////

GFXFontEdit *GFXFontEdit::loadOldFont(const char *filename)
{
	GFXOldFont *oldFont = GFXOldFont::load(filename);

	if (!oldFont)
		return(NULL);

	GFXFontEdit *newFont = new GFXFontEdit(oldFont);
	delete oldFont;
	return(newFont);
}

////////////////////////////////////////////////////////////////////////

GFXFontEdit *GFXFontEdit::load(const char *in_name)
{
	GFXFont *font = GFXFont::load(in_name);

	if (!font)
		return(NULL);
				
	GFXFontEdit *fontEdit = new GFXFontEdit(font);
	return(fontEdit);
}

////////////////////////////////////////////////////////////////////////

int GFXFontEdit::asciiOfIthChar(int i)
{
	// return the ascii value of the ith character in the font
	int n, count = 0;

	for (n = 0; n < charTableLookupSize; n++)
		if (charTableLookup[n] != -1)
		{
			count++;                 // chars counted so far
			if (count == i)
				break;               // found the target no
		}

	return(n + charTableLookupOffset);
}

////////////////////////////////////////////////////////////////////////

int GFXFontEdit::asciiOfNextChar(int asciiOfCurrent, int amount)
{
	// return the ascii value of the next char "amount" chars away
	// in the array

	int n = (asciiOfCurrent - charTableLookupOffset) + 1, count = 0;

	while (n < charTableLookupSize)
	{
		if ((n > 0) && (charTableLookup[n] != -1))
		{
			count++;
			if (count == amount)
				break;
		}
		n++;
	}
	n += charTableLookupOffset;
	return(n < charTableLookupSize ? n : -1);
}

////////////////////////////////////////////////////////////////////////

int GFXFontEdit::asciiOfPrevChar(int asciiOfCurrent, int amount)
{
	// return the ascii value of the next char "amount" chars away
	// in the array

	int n = (asciiOfCurrent - charTableLookupOffset) - 1, count = 0;

	while (n >= 0)
	{
		if ((n < charTableLookupSize) && (charTableLookup[n] != -1))
		{
			count--;
			if (count == amount)
				break;
		}
		n--;
	}
	n += charTableLookupOffset;
	return(n >= 0 ? n : -1);
}

////////////////////////////////////////////////////////////////////////

Bool GFXFontEdit::setBaseline(int ascii, int newBaseline)
{
	// change the baseline of an individual char, return
	// true on success.

	int index = getCharTableIndex(ascii);
	
	if (index != -1)
	{
		charTable[index].baseline = newBaseline;
		return(true);
	}
	return(false);
}

////////////////////////////////////////////////////////////////////////

bool GFXFontEdit::changeASCII(int oldASCII, int newASCII)
{
	// change the ASCII identification of a char, as long as there is
	// not a conflict and old and new values are valid

	int oldLookupIndex = oldASCII - charTableLookupOffset;
	int newLookupIndex = newASCII - charTableLookupOffset;

	if ((oldLookupIndex >= 0)                   && 
	    (oldLookupIndex < charTableLookupSize)  &&
		(newLookupIndex >= 0)                   &&
		(newLookupIndex < charTableLookupSize)  &&
		(charTableLookup[newLookupIndex] == -1))
	{
		charTableLookup[newLookupIndex] = charTableLookup[oldLookupIndex];
		charTableLookup[oldLookupIndex] = -1;
		// instead of trying to maintain first&last ASCII here, just
		// recompute them...
		computeUsefulInformation();
		return(true);
	}
	return(false);
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::setBackground(int colorIndex)
{
	if (fi.flags && FONT_MONO == 0)
		return;

	BYTE  oldBackground = fi.backColor;
	BYTE  newBackground = colorIndex;
	BYTE *data;
	int  i, p, imageSize;;
	
	for (i = 0; i < bma.numBitmaps; i++)
	{
		imageSize = bma.array[i]->imageSize;
		data      = bma.array[i]->pBits;
		for (p = 0; p < imageSize; p++)
			if (data[p] == oldBackground)
				data[p] = newBackground;
	}
	fi.backColor = newBackground;
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::setForeground(int colorIndex)
{
	if (fi.flags && FONT_MONO == 0)
		return;

	BYTE  newForeground = colorIndex;
	BYTE *data;
	int  i, p, imageSize;;
	
	for (i = 0; i < bma.numBitmaps; i++)
	{
		imageSize = bma.array[i]->imageSize;
		data      = bma.array[i]->pBits;
		for (p = 0; p < imageSize; p++)
			if (data[p] != fi.backColor)
				data[p] = newForeground;
	}
	fi.foreColor = newForeground;
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::setPalette(GFXPalette *newPalette)
{
	// The main window takes care of deleting palettes
	pPal = newPalette;
}
	
////////////////////////////////////////////////////////////////////////

int GFXFontEdit::findNextAvailableASCII(int ascii, int direction)
{
	// search the lookup table for the nearest available ascii slot

	int currentPos      = ascii - charTableLookupOffset;
	int positionsProbed = 0;

	while (positionsProbed < charTableLookupSize)
	{
		if (charTableLookup[currentPos] == -1)
			return(currentPos);

		currentPos += direction;

		// wrap-around if we've gone past either end
		if (currentPos < 0)
			currentPos = charTableLookupSize - 1;
		else if (currentPos > charTableLookupSize)
			currentPos = 0;
					
		positionsProbed++;
	}
	// search exhausted, no empty spaces found
	return(-1);
}

////////////////////////////////////////////////////////////////////////

Bool GFXFontEdit::setASCII32Width(int newWidth)
{
	// Set the width of the space char
	int lookupASCII32 = getCharTableIndex(32);

	if (newWidth < 1 || newWidth > 255 || lookupASCII32 == -1)
		return(false);

	deleteChar(32);
	addASCII32(newWidth);
	return(true);
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::addASCII32(int width)
{
	if (getCharTableIndex(32) != -1)
		return;

	if (width == -1)
		width = max(3, int(fi.spacing)*2);

	GFXBitmap *newBitmap = GFXBitmap::create(width, 1);
	memset((void *)newBitmap->pBits, BYTE(fi.backColor), newBitmap->imageSize);
	if (bma.numBitmaps > 0)
		newBitmap->attribute = bma.array[0]->attribute;

	insertBitmap(32, newBitmap, 0);
	delete newBitmap;	
}	

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::setSpacing(int newSpacing)
{
	if (newSpacing >= 0)
		fi.spacing = newSpacing;
}

////////////////////////////////////////////////////////////////////////

void GFXFontEdit::toggleJustificationFlag(int flag)
{
	if (fi.justification & flag)
		fi.justification &= (~flag);
	else
		fi.justification |= flag;
}