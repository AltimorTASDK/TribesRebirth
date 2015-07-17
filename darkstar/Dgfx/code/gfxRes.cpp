//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <gfxRes.h>
#include "g_bitmap.h"
#include "g_barray.h"
#include "g_pal.h"
#include "g_font.h"


//NOTE: this variable is referencced in FileIO/ResManager.cpp 
//to force the linker to link this module
int gfxResTypes = 0;		

// MS formats supported by GFX
static ResourceTypeGFXBitmap 			_resdib(".dib");
static ResourceTypeGFXBitmap 			_resbmp(".bmp");
static ResourceTypeGFXBitmap 			_respng(".png");
static ResourceTypeGFXPalette			_respal(".pal");

// GFX formats
static ResourceTypeGFXBitmap 			_respbm(".pbm");
static ResourceTypeGFXBitmapArray	_resbma(".pba");
static ResourceTypeGFXPalette			_resppl(".ppl");
static ResourceTypeGFXPalette			_resdpl(".dpl");
static ResourceTypeGFXFont				_resfnt(".pft");


//-------------------------------------- GFXBitmap
void* 
ResourceTypeGFXBitmap::construct(StreamIO *stream, int)
{
   // Force load the palette associated with this BMP (if any)
   return (void*)GFXBitmap::load(stream, BMF_INCLUDE_PALETTE);
}

void 
ResourceTypeGFXBitmap::destruct(void *p)
{
   delete (GFXBitmap*)p;
}


//-------------------------------------- GFXBitmapArray
void* 
ResourceTypeGFXBitmapArray::construct(StreamIO *stream, int)
{
   return (void*)GFXBitmapArray::load(stream,0);
}

void 
ResourceTypeGFXBitmapArray::destruct(void *p)
{
   delete (GFXBitmapArray*)p;
}


//-------------------------------------- GFXPalette
void* 
ResourceTypeGFXPalette::construct(StreamIO *stream, int)
{
   return (void*)GFXPalette::load(stream);
}

void 
ResourceTypeGFXPalette::destruct(void *p)
{
   delete (GFXPalette*)p;
}


//-------------------------------------- GFXFont
void* 
ResourceTypeGFXFont::construct(StreamIO *stream, int)
{
   return (void*)GFXFont::load(stream);
}

void 
ResourceTypeGFXFont::destruct(void *p)
{
   delete (GFXFont*)p;
}
