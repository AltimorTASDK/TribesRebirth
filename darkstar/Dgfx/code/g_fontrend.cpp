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

#include <stdlib.h>
#include <filstrm.h>
#include "g_surfac.h"
#include "g_barray.h"
#include "g_bitmap.h"
#include "g_font.h"

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

Int32 GFXDrawText_p(GFXSurface    *io_pSurface,
                            GFXFont       *in_font,
                            const Point2I *in_at,
                            const void    *in_string )
{
	Point2I      pt;
	Point2I      pt2;
	UInt32       index;
	RectI        extent(9999,9999,-1,-1 );
	RectI       *lastRect;
	GFXCharInfo *charInfo;
	const GFXBitmap *bmp;
   RectI in_subRegion;

	pt.x = in_at->x;

	const void *cptr = in_string;

	if (in_font->fi.flags & FONT_UNICODE)
		index = (UInt32)(*(WORD *)cptr);
	else
		index = (UInt32)(*(BYTE *)cptr);

	lastRect = io_pSurface->getLastRect();
	while (index)
	{
		in_font->getCharInfo(index, &bmp, &in_subRegion, &charInfo);

		// adjust char's bitmap position based on font's baseline
		// and char's baseline.

		if ( (in_font->fi.scale.x != (1<<16)) || (in_font->fi.scale.y != (1<<16)) )   // if font scaled
		{
        	pt.y = in_at->y +
                ((in_font->fi.baseline +
                charInfo->baseline - in_subRegion.len_y() + 1) * in_font->fi.scale.y) / 65536;

 			pt2.x = ((in_subRegion.len_x()+1)*in_font->fi.scale.x)/65536;
			pt2.y = ((in_subRegion.len_y()+1)*in_font->fi.scale.y)/65536;

			//io_pSurface->drawBitmap2d(bmp, &pt, &pt2);
			extent.upperL.x = min(lastRect->upperL.x, extent.upperL.x);
			extent.upperL.y = min(lastRect->upperL.y, extent.upperL.y);
			extent.lowerR.x = max(lastRect->lowerR.x, extent.lowerR.x);
			extent.lowerR.y = max(lastRect->lowerR.y, extent.lowerR.y);

			if (in_font->fi.flags&FONT_FIXED)
				pt.x += ((in_font->fi.fontWidth*in_font->fi.scale.x)/65536) + in_font->fi.spacing;
			else
				pt.x += (((in_subRegion.len_x()+1)*in_font->fi.scale.x)/65536) + in_font->fi.spacing;
		}
		else                    // font is not scaled
		{
			pt.y = in_at->y +
                   in_font->fi.baseline + 
                   charInfo->baseline - in_subRegion.len_y() +1 ;

			io_pSurface->drawBitmap2d(bmp, &in_subRegion, &pt);
         
			extent.upperL.x = min(lastRect->upperL.x, extent.upperL.x);
			extent.upperL.y = min(lastRect->upperL.y, extent.upperL.y);
			extent.lowerR.x = max(lastRect->lowerR.x, extent.lowerR.x);
			extent.lowerR.y = max(lastRect->lowerR.y, extent.lowerR.y);

			if (in_font->fi.flags&FONT_FIXED)
				pt.x += in_font->fi.fontWidth + in_font->fi.spacing;
			else
				pt.x += in_subRegion.len_x()+1 + in_font->fi.spacing;
		}

		// Both Watcom and MSVC have problems with the following:
		// ((const FOO *)cptr)+=1;
		// MS doesn't understand that left side IS an Lvalue
		// WAT has apparent compiler bug that compiles 
		// ((const char*)cptr)+=1; to *((const char *)cptr)+=1;

		if (in_font->fi.flags & FONT_UNICODE)
		{
			cptr  = (const char *) (((const WORD *)cptr) + 1);
			index = (UInt32)( *(WORD *)cptr );
		}
		else
		{
			cptr = ((const char *)cptr) + 1;
			index = (UInt32)( *(BYTE *)cptr );
		}
	}

	*lastRect = extent;
	return(lastRect->lowerR.x);
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

Int32 GFXDrawText_r(GFXSurface        *io_pSurface,
                            GFXFont           *io_font,
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
	newx = GFXDrawText_p( io_pSurface, io_font, &pt, in_string );

	if ( (io_font->fi.justification & TEXT_FIT) != NULL )
		io_font->fi.scale = oldScale;      // restore original scale value

	return(newx);
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


