//================================================================
//
// $Workfile:   g_bitmap.cpp  $
// $Version$
// $Revision:   1.7  $
//
// DESCRIPTION:
//   rev. 1.7 9/96 David Scott - added support for mipmaps
//
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================
#include <stdio.h>

#include <base.h>
#include <filstrm.h>
#include "g_bitmap.h"
#include "g_pal.h"
#include "resManager.h"

extern "C"
{
   void __cdecl gfxBitmapInvert(BYTE* pTop, BYTE* pBottom, UInt32 dwStride);
}

struct BMPHeader
{
   DWORD ver_nc;
   DWORD width;
   DWORD height;
   DWORD bitDepth;
   DWORD attribute;
};

//increment high byte if bitmap format becomes incompatible with
//the old reader.
#define BITMAP_VERSION 0x00000000

UInt32 GFXBitmap::sm_currSequenceNumber = 1;

//================================================================
// NAME
//   GFXBitmap::create
//
// DESCRIPTION
//   Create a new bitmap with the given dimensions
//
// ARGUMENTS
//   width   - width in pixels
//   height  - height in pixels
//   bpp     - bit per pixel
//
// RETURNS
//   GFXBitmap*  - if Bitmap created successfully
//   NULL     - if failed
//
// NOTES
//   Does not clear the bitmap bits
//================================================================
GFXBitmap* GFXBitmap::create(Int32 in_width, Int32 in_height, Int32 in_bpp )
{
   AssertFatal( in_width > 0 && in_height > 0 && in_bpp > 0,
      "GFXBitmap::create: invalid parameters" );

   GFXBitmap *bm = new GFXBitmap;

   if (bm == NULL)
      return NULL;

   bm->width   = in_width;
   bm->stride  = ((in_width * in_bpp >> 3)+3)&(~3);    //round up to nearest DWORD
   bm->height  = in_height;
   bm->bitDepth= in_bpp;
   bm->imageSize = bm->getHeight() * bm->getStride();

   bm->pBitsBase= new BYTE[(bm->getHeight() + 2) * bm->getStride()];
   bm->pBits = bm->pBitsBase + bm->getStride();
   bm->attribute |= BMA_OWN_MEM;

   bm->detailLevels = 1;
   bm->pMipBits[0]  = bm->pBits;

   return bm;
}

//================================================================
// NAME
//   GFXBitmap::createSubBitmap
//
// DESCRIPTION
//   Create a new bitmap with the given dimensions as a subregion
//   of this bitmap
//
// ARGUMENTS
//   in_r    - bounding rectangle in the parent bitmap's frame
//
// RETURNS
//   GFXBitmap*  - if Bitmap created successfully
//   NULL     - if failed
//
// NOTES
//   Does not clear the bitmap bits
//   Allocates no memory for the bits
//
//================================================================
GFXBitmap * GFXBitmap::createSubBitmap(const RectI *in_r)
{
   GFXBitmap *ret = new GFXBitmap;
   if(!ret)
      return NULL;

   ret->makeSubBitmap(this, in_r);
   return ret;
}


//================================================================
// NAME
//   GFXBitmap::makeSubBitmap
//
// DESCRIPTION
//   makes an existing bitmap refer to a subregion of another bitmap
//
// ARGUMENTS
//   in_bmp  - parent bitmap
//   in_r    - bounding rectangle in the parent bitmap's frame
//
// NOTES
//
//================================================================
void GFXBitmap::makeSubBitmap(GFXBitmap *in_bmp, const RectI *in_r)
{
   free();
   AssertFatal(in_bmp, "GFXBitmap::createSubBitmap: NULL argument not allowed for in_bmp.");
   AssertFatal(in_r, "GFXBitmap::createSubBitmap: NULL argument not allowed for in_r.");

   AssertFatal(in_r->upperL.x >= 0 && in_r->upperL.y >= 0 &&
               in_r->lowerR.x < in_bmp->getWidth() &&
               in_r->lowerR.y < in_bmp->getHeight(), "GFXBitmap::createSubBitmap: invalid parameters.");

   width = in_r->lowerR.x - in_r->upperL.x + 1;
   height = in_r->lowerR.y - in_r->upperL.y + 1;
   stride = in_bmp->getStride();
   bitDepth = in_bmp->bitDepth;

   imageSize = getHeight() * getStride();
   pBits = in_bmp->getAddress(&in_r->upperL);

   attribute &= ~BMA_OWN_MEM;
}

//================================================================
// NAME
//   GFXBitmap::load
//
// DESCRIPTION
//   Create a Bitmap and read it from file/resource
//
// ARGUMENTS
//   filename - resource or actual filename
//   in_flags - Bitmap IO flags
//
// RETURNS
//   GFXBitmap*  - if file exists and Bitmap allocated successfully
//   NULL     - if failed
//
//================================================================
GFXBitmap* GFXBitmap::load(const char* in_filename, DWORD in_flags )
{
   GFXBitmap *bm = new GFXBitmap;

   if (bm)
   {
      if (bm->read(in_filename, in_flags))
         return bm;
      else
         delete bm;
   }
   return NULL;
}


//================================================================
// NAME
//   GFXBitmap::load
//
// DESCRIPTION
//   Create a bitamp and read it in from the stream
//
// ARGUMENTS
//   io_steram - Read stream positioned at the start of a GFXBitmap
//
// RETURNS
//   A GFXBitmap pointer on success
//   NULL is the load failed
//
// NOTES
//
//================================================================
GFXBitmap* GFXBitmap::load(StreamIO *io_stream, DWORD in_flags)
{
   GFXBitmap *bm = new GFXBitmap;
   if (bm)
   {
      if (bm->read(io_stream, in_flags))
         return bm;
      else
         delete bm;
   }
   return (NULL);
}


//================================================================
// NAME
//   GFXBitmap::read
//
// DESCRIPTION
//   Read bitmap from file/resource
//
// ARGUMENTS
//   in_filename - actual in_filename
//   in_flags    - Bitmap IO flags BMF_*
//
// RETURNS
//   TRUE  if bitmap loaded successfully
//   FALSE on failure
//
//================================================================
Bool GFXBitmap::read(const char* in_filename, DWORD in_flags)
{
   Bool     result;
   HANDLE   fileHandle;
   fileHandle = CreateFile(in_filename,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL
                           );
   if (fileHandle == (HANDLE)-1)
   {
      AssertWarn(0, avar("GFXBitmap::read: Unable to load bitmap %s, CreateFile for reading failed.", in_filename));
      return FALSE;
   }

   FileRStream frs(fileHandle);
   result = read(&frs, in_flags);
   AssertWarn(result, avar("GFXBitmap::read: Error reading bitmap %s", in_filename));
   frs.close();
   CloseHandle(fileHandle);

   return(result);
}


//================================================================
// NAME
//   GFXBitmap::read
//
// DESCRIPTION
//   Read a GFXBitamp from the stream.
//   The contents of the object are freed first then the new object
//   is read in from the stream.
//
// ARGUMENTS
//   in_stream - Read stream positioned at a GFXBitmap
//   in_flags  - BMF_* bitamp flags
//
// RETURNS
//   TRUE is the bitmap was loaded successfully
//   FALSE if there was an error during reading
//
// NOTES
//
//================================================================
Bool GFXBitmap::read(StreamIO *in_stream, DWORD in_flags)
{
   DWORD data;
   DWORD size;
   BMPHeader header;
   Int32 numChunks = -1;
   paletteIndex = 0xFFFFFFFF;

   free();                             //release current bitamp contents

   detailLevels = 1;    // incase file doesn't include detail level

   while (numChunks--)
   {
      in_stream->read(&data);
      in_stream->read(&size);
      if ( (numChunks < 0)
           && ((data & 0x0000ffff) == FOURCC('B','M',0,0)) )
         return readMSBitmap(in_stream, in_flags);

      switch (data)
      {
         case FOURCC('P','i','D','X'):
            in_stream->read(&paletteIndex);
            break;
         case FOURCC('P','B','M','P'):
            break;

         case FOURCC('h','e','a','d'):
            AssertFatal(sizeof(header) == size, "GFXBitmap::read: Header size different than expected.");
            in_stream->read(size, (BYTE*)&header);
            numChunks= header.ver_nc & 0x00ffffff;    //mask off the version
            AssertFatal((header.ver_nc & 0xff000000) == BITMAP_VERSION, "GFXBitmap::read: incorrect file version.");

            width    = header.width;
            height   = header.height;
            bitDepth = header.bitDepth;
            stride   = ((width * bitDepth >> 3)+3)&~3;
            attribute= header.attribute;
            break;

         case FOURCC('R','I','F','F'):
            if (in_flags & BMF_INCLUDE_PALETTE)                         //load it
            {
               pPalette = new GFXPalette;
               pPalette->readMSPal(in_stream, 0);
            }
            else
               in_stream->setPosition(in_stream->getPosition()+size);   //skip it
            break;

         case FOURCC('D','E','T','L'):
			// read detail level
			in_stream->read(size, (BYTE *)&detailLevels);
			break;

         case FOURCC('d','a','t','a'):
            if (numChunks == -1)
            {
               AssertWarn(0, "GFXBitmap::read: Bad GFXBitamp format: 'data' chunk before 'head'.");
               return (FALSE);
            }
            imageSize = size;

            if (pBitsBase)
               delete [] pBitsBase;
            
            pBitsBase = new BYTE[size + stride * 2];
            pBits = pBitsBase + stride;

            attribute |= BMA_OWN_MEM;

            in_stream->read(size, (BYTE*)pBits);
            break;

         default:
            if (numChunks == -2)
            {
               AssertWarn(0, "GFXBitmap::read: not a GFXBitmap.");
               free();                    //clean up our mess
               return (FALSE);
            }
            in_stream->setPosition(in_stream->getPosition()+size);
            break;
      }
   }

	// setup the mipmap pointers into the pBits data
	pMipBits[0] = pBits;
	if (detailLevels > 1)
	{
		// since this is a mipmap, bpp==8
    	int currentMipSize = width*height;  
		for (int i = 1; i < detailLevels; i++)
		{
			pMipBits[i] = pMipBits[i - 1] + currentMipSize;
			currentMipSize /= 4;
		}
	}

   return (TRUE);
}



//================================================================
// NAME
//   GFXBitmap::readMSBitmap
//
// DESCRIPTION
//   Read a Microsoft DIB from the stream.
//
// ARGUMENTS
//   in_stream - Read stream
//   in_flags  - BMF_* bitamp flags
//
// RETURNS
//   TRUE  if the bitmap was loaded successfully
//   FALSE if there was an error during reading
//
// NOTES
//
//================================================================
Bool GFXBitmap::readMSBitmap(StreamIO *in_stream, DWORD in_flags)
{
   BITMAPFILEHEADER   bf;
   BITMAPINFOHEADER   bi;

   // position at FILEHEADER
   in_stream->setPosition(in_stream->getPosition() - 8);

   in_stream->read(sizeof(bf), &bf);
   in_stream->read(sizeof(bi), &bi);
   bitDepth = bi.biBitCount;

   width  = bi.biWidth;
   stride = ((width * bitDepth >> 3)+3)&(~3);            //round up to nearest DWORD
   height = m_abs(bi.biHeight);

   if (bf.bfReserved1 == MS_BITMAP_PI_MAGIC && bf.bfReserved2 != 0xffff) {
      paletteIndex = DWORD(bf.bfReserved2);
   } else {
      paletteIndex = DWORD(-1);
   }

   //AssertWarn(((bi.biClrUsed==256)||(bi.biClrUsed==0)),
   //   "GFXBitmap::readMSBitmap:  Palette does not contain 256 colors.");

   if ( (bitDepth != 8) && (bitDepth != 24) )
      return false;

   if ( (in_flags & BMF_INCLUDE_PALETTE) && (bitDepth != 24) )
   {
      pPalette = new GFXPalette;
      if (!pPalette)
      {
         AssertWarn(0,"GFXBitmap::read: Unable to load bitmap, NEW failed.");
         return (FALSE);
      }

      pPalette->numberOfColors = (WORD) (bi.biClrUsed? bi.biClrUsed:256);
      in_stream->read(pPalette->numberOfColors * sizeof(RGBQUAD), (BYTE*)(pPalette->palette[0].color));
      pPalette->set((RGBQUAD*)pPalette->palette[0].color); //convert from RGBQUAD to PALETTEENTRY
   }
   else if ( bitDepth != 24 )
      in_stream->setPosition( (bi.biClrUsed? bi.biClrUsed:256) * sizeof(RGBQUAD) + in_stream->getPosition());

   imageSize = height * stride;
   pBitsBase = new BYTE[imageSize + stride * 2];
   pBits = pBitsBase + stride;
   attribute |= BMA_OWN_MEM;

   in_stream->read(imageSize, (BYTE*)pBits);

   if (bi.biHeight >= 1)         //DOH! a bottom-up DIB those Microsoft guys
   {                             //are really weird! so make it top-down... ahhhh.
      gfxBitmapInvert( pBits, pBits + ((height-1) * stride), stride);
   }

	detailLevels = 1;
	pMipBits[0]  = pBits;

   return TRUE;
}


//================================================================
// NAME
//   GFXBitmap::write
//
// DESCRIPTION
//   Write a bitmap to disk.
//
// ARGUMENTS
//   in_filename - path\filename to save bitmap.
//   in_flags    - BMF_* bitmap flags
//      BMF_INCLUDE_PALETTE - if set writes the palette with the bitmap
//
// RETURNS
//   TRUE if written to disk successfully
//   FASLE if error occured.  Use GetLastError for more detail.
//
// NOTES
//
//================================================================
Bool GFXBitmap::write(const char* in_filename, DWORD in_flags)
{
   FileWStream fws;
   if (fws.open(in_filename) == false) {
      AssertWarn(0, avar("GFXBitmap::write: failed to open file %s", in_filename));
      return false;
   }

   Bool result = write(&fws, in_flags);
   fws.close();

   return (result);
}


//================================================================
// NAME
//   GFXBitmap::write
//
// DESCRIPTION
//   Write a GFXBitmap format bitmap to disk.
//
// ARGUMENTS
//   in_stream   - Write stream to write bitmap to.
//   in_flags    - BMF_* bitmap flags
//      BMF_MICROSOFT -to write out as a Microsoft DIB
//
// RETURNS
//   TRUE if written to successfully
//   FASLE if error occured.  Use GetLastError for more detail.
//
// NOTES
//
//================================================================
Bool GFXBitmap::write(StreamIO *in_stream, DWORD in_flags)
{
   BMPHeader header;

   if (in_flags & BMF_MICROSOFT) return writeMSBitmap(in_stream, in_flags);

   in_stream->write((DWORD)FOURCC('P','B','M','P'));     //PBMP:sz
   in_stream->write((DWORD)getSaveSize());


   in_stream->write((DWORD)FOURCC('h','e','a','d'));     //head:sz
   in_stream->write((DWORD)sizeof(BMPHeader));

   //write out the header
   //header.ver_nc  = BITMAP_VERSION + 1 + ((pPalette) ? 1 : 0);
   // one extra chunk for detailLevel 
//   header.ver_nc  = BITMAP_VERSION + 2 + ((pPalette) ? 1 : 0);

   header.ver_nc =  BITMAP_VERSION; // goes in some high order bits
                                    // numChunks are masked off on read
   header.ver_nc += 3;              // one for data, one for detail levels
   if (pPalette && (in_flags & BMF_INCLUDE_PALETTE))
      header.ver_nc += 1;           // another chunk for palette
         
   header.width   = width;
   header.height  = height;
   header.bitDepth= bitDepth;
   header.attribute= attribute;
   in_stream->write(sizeof(BMPHeader), (BYTE*)&header);

   //write the palette out if it exists
   if (pPalette && (in_flags & BMF_INCLUDE_PALETTE))
       pPalette->writeMSPal(in_stream);

   //write out the bitmap bits
   in_stream->write((DWORD)FOURCC('d','a','t','a'));     //data:sz
   in_stream->write((DWORD)imageSize);
   in_stream->write(imageSize, pBits);

   // write out the number of detail levels
   in_stream->write((DWORD)FOURCC('D','E','T','L'));
   in_stream->write((DWORD)sizeof(Int32));
   in_stream->write(sizeof(Int32), (BYTE *)&detailLevels);

   in_stream->write((DWORD)FOURCC('P','i','D','X'));
   in_stream->write((DWORD)sizeof(DWORD));
   in_stream->write(paletteIndex);

   return (in_stream->getStatus() == STRM_OK);
}


//================================================================
// NAME
//   GFXBitmap::write
//
// DESCRIPTION
//   Write a Microsoft DIB format bitmap to disk.
//
// ARGUMENTS
//   in_stream   - Write stream to write bitmap to.
//   in_flags    - BMF_* bitmap flags
//      BMF_INCLUDE_PALETTE - if set writes the palette with the bitmap
//
// RETURNS
//   TRUE if written to successfully
//   FASLE if error occured.  Use GetLastError for more detail.
//
// NOTES
//
//================================================================
Bool GFXBitmap::writeMSBitmap(StreamIO *in_stream, DWORD /*in_flags*/)
{
   RGBQUAD           rgb[256];
   BITMAPINFOHEADER  bi;
   BITMAPFILEHEADER  bf;

   if ( pPalette )
      bi.biClrUsed = pPalette->numberOfColors;
   else if ( bitDepth != 24 )
      bi.biClrUsed = 256;
   else
      bi.biClrUsed = 0;

   imageSize   = getHeight() * getStride();
   bf.bfType   = (WORD)FOURCC('B','M',0,0);     //Type of file 'BM'
   bf.bfOffBits= sizeof(BITMAPINFOHEADER)
               + sizeof(BITMAPFILEHEADER)
               + (sizeof(RGBQUAD)*bi.biClrUsed);
//   bf.bfSize = ((getHeight()*getStride()) + bf.bfOffBits) >> 2;
   bf.bfSize            = (bf.bfOffBits + imageSize);
   bi.biSize            = sizeof(BITMAPINFOHEADER) ;
   bi.biWidth           = getWidth();
   bi.biHeight          = -getHeight();         //our data is top-down
   bi.biPlanes          = 1;
   bi.biBitCount        = (WORD)bitDepth;
   bi.biCompression     = BI_RGB ;
   bi.biSizeImage       = imageSize;
   bi.biXPelsPerMeter   = 0 ;
   bi.biYPelsPerMeter   = 0 ;
   bi.biClrUsed         = 0 ;
   bi.biClrImportant    = 0 ;

   // We want to store off our palette index in the reserved fields, so...
   bf.bfReserved1 = MS_BITMAP_PI_MAGIC;
   if (paletteIndex == DWORD(-1)) {
      bf.bfReserved1 = WORD(0);
      bf.bfReserved2 = WORD(0);
   } else {
      AssertWarn(paletteIndex < 65534, "Unrepresentable palette index in MS Bitmap");
      bf.bfReserved2 = WORD(paletteIndex);
   }

   //Most applications don't adhere to the spec and assume Bottom-Up so
   //unless requested save all DIB's Bottom-Up
   if (bi.biHeight < 0)
   {
      bi.biHeight = -bi.biHeight;
      gfxBitmapInvert(pBits, pBits+(getHeight()-1)*getStride(), getStride());
   }

   in_stream->write(sizeof(bf), &bf);
   in_stream->write(sizeof(bi), &bi);
   for (Int32 ndx=0; ndx<256; ndx++)
   {
      if(pPalette)
      {
         rgb[ndx].rgbRed      = pPalette->palette[0].color[ndx].peRed;
         rgb[ndx].rgbGreen    = pPalette->palette[0].color[ndx].peGreen;
         rgb[ndx].rgbBlue     = pPalette->palette[0].color[ndx].peBlue;
      }
      else
      {
         rgb[ndx].rgbRed = (unsigned char ) ndx;
         rgb[ndx].rgbGreen = (unsigned char ) ndx;
         rgb[ndx].rgbBlue = (unsigned char ) ndx;
      }
      rgb[ndx].rgbReserved = 0;
   }
   //write the palette
   if ( bitDepth != 24 )
      in_stream->write(sizeof(RGBQUAD)*256, (BYTE*)&rgb);

   //write the bitmap bits
   // in_stream->write(imageSize, pBits);
   // imageSize may includes mipmaps, in which case we only want to write
   // the the first height*stride bytes
   in_stream->write(height*stride, pBits);

   if (bi.biHeight != getHeight())  //did we invert it?
      gfxBitmapInvert(pBits, pBits+(getHeight()-1)*getStride(), getStride());

   AssertWarn(in_stream->getStatus() == STRM_OK, "GFXBitmap::write: IO error writing bitamp");
   return(in_stream->getStatus() == STRM_OK);
}


//================================================================
// NAME
//   GFXBitmap::getSaveSize
//
// DESCRIPTION
//   get the size of a PBMP chunk on disk
//
// ARGUMENTS
//   none
//
// RETURNS
//   size of PBMP chunk
//
// NOTES
//   Assumes if pPalette that palette will be written to disk
//
//================================================================
DWORD GFXBitmap::getSaveSize()
{
   //magic number 8 is size of chunk_ID and chunk_size
   return
   (
      8+                                           // head:sz
      (4*4)+                                       //    header info
      ((pPalette) ? 256 * sizeof(PALETTEENTRY)+8+4+4+8 : 0) + // RIFF:sz palette chunk size (if any)
      8+                                           // data:sz
      imageSize                                     //   pBits
   );
}
