//================================================================
//   
// $Workfile:   g_bitmap.h  $
// $Version$
// $Revision:   1.6  $
//   
// DESCRIPTION:
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _G_BITMAP_H_
#define _G_BITMAP_H_

#include "m_point.h"
#include "m_rect.h"
#include "g_pal.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


//Bitmap Attributes
#define BMA_NORMAL      (0<<0)     // no special effects
#define BMA_TRANSPARENT (1<<0)     // transparent color
#define BMA_FUZZY       (1<<1)     // dither when stretched
#define BMA_TRANSLUCENT (1<<2)     // drawn semi-translucent
#define BMA_OWN_MEM     (1<<3)     // owns memory
#define BMA_ADDITIVE    (1<<4)     // drawn semi-translucent
#define BMA_SUBTRACTIVE (1<<5)     // drawn semi-translucent
#define BMA_ALPHA88     (1<<6)     // alpha texture 8 8
#define BMA_DRAW_FLAGS (BMA_TRANSLUCENT|BMA_FUZZY|BMA_TRANSPARENT|BMA_NORMAL|BMA_ADDITIVE|BMA_SUBTRACTIVE)
#define BMA_MAX_MIPMAPS  9

//Bitmap read/write flags
#define BMF_INCLUDE_PALETTE   (UInt32)(1<<0)  // include palette when reading
#define BMF_MICROSOFT         (UInt32)(1<<31) // save as Microsoft bitmap (DIB)

#define MS_BITMAP_PI_MAGIC (0xf5f7)


//================================================================
//                       Bitmap Class Decleration
//================================================================

class GFXPalette;
class StreamIO;

class GFXBitmap
{
  private:
   static UInt32 currentDriverSeqCount;
  public:
   struct GlideCacheInfo {
      Int32 wrapCount;
      UInt32 texAddress;
      UInt8 aspectRatio;
      UInt8 textureFormat;
      UInt8 smallLod;
      UInt8 largeLod;
      UInt32 driverSeqCount;
   };

   struct CacheInfo {
      UInt32 bitmapSequenceNum;
      UInt32 cacheDefined0;
      UInt32 cacheDefined1;
      UInt32 cacheDefined2;

      CacheInfo(const UInt32 in_seq) : bitmapSequenceNum(in_seq),
                                       cacheDefined0(0xffffffff),
                                       cacheDefined1(0xffffffff),
                                       cacheDefined2(0xffffffff) { }
      CacheInfo() : bitmapSequenceNum(0xffffffff),
                    cacheDefined0(0xffffffff),
                    cacheDefined1(0xffffffff),
                    cacheDefined2(0xffffffff) { }
      void clear() {
         cacheDefined0 = 0xffffffff;
         cacheDefined1 = 0xffffffff;
         cacheDefined2 = 0xffffffff;
      }
   };


protected:
   Bool   readMSBitmap(StreamIO *io_stream, DWORD in_flags=0);
   Bool   writeMSBitmap(StreamIO *io_stream, DWORD in_flags=0);
   void free();            //depending on type free/delete the bitmap

   static UInt32 sm_currSequenceNumber;
   mutable CacheInfo m_cacheInfo;

public:
   Int32       width;      // actual width in pixels
   Int32       height;     // height in pixels
   Int32       stride;     // (width+3)&~3
   Int32       bitDepth;   // # of bits per pixel
   Int32       imageSize;  // number of bytes pointed to by pBits
   
   BYTE        *pBitsBase;
   BYTE        *pBits;
   GFXPalette  *pPalette;
   DWORD       attribute;
   DWORD       paletteIndex; // matches the palette index in a palette's multipal

   BYTE       *pMipBits[BMA_MAX_MIPMAPS];	
   Int32       detailLevels;
   
   GFXBitmap();
   ~GFXBitmap();

   static GFXBitmap* create(Int32 in_width, Int32 in_height, Int32 in_bpp=8);
   static GFXBitmap* createInline(Int32 in_width, Int32 in_height, Int32 in_bpp=8);
   GFXBitmap* createSubBitmap(const RectI *in_bounds);
   void   makeSubBitmap(GFXBitmap *in_bmp, const RectI *in_r);
   
   static GFXBitmap* load(const char* in_filename, DWORD in_flags=0);
   static GFXBitmap* load(StreamIO *io_stream, DWORD in_flags=0);
   Bool   read(const char* in_filename, DWORD in_flags=0);
   Bool   read(StreamIO *io_stream, DWORD in_flags=0);
   Bool   write(const char* in_filename, DWORD in_flags=BMF_MICROSOFT);
   Bool   write(StreamIO *io_stream, DWORD in_flags=BMF_MICROSOFT);
   DWORD  getSaveSize();

   Int32  getHeight() const;     
   Int32  getWidth() const;
   Int32  getStride() const;
   Int32  getBitDepth() const;
   inline void getSize( Point2I * size ) const;

   inline Int32       getNumDetailLevels() const { return detailLevels; }
   inline const BYTE* getDetailLevel(const int in_index) const {
      AssertFatal(in_index >= 0 && in_index < detailLevels, "Out of bounds detail level");
      return pMipBits[in_index];
   }

   void   getClientRect(RectI *out_rect) const;
   BYTE*  getAddress(const Point2I *in_pt) const;
   BYTE*  getAddress(Int32 in_x, Int32 in_y) const;
   inline void resolvePaletteIndex(GFXPalette *pal);
   UInt32 getSequenceNumber() const { return m_cacheInfo.bitmapSequenceNum; }
   void clearSequenceNumber() { m_cacheInfo.bitmapSequenceNum = 0; }
   CacheInfo& getCacheInfo() const { return m_cacheInfo; }
   GlideCacheInfo &getGlideCacheInfo() { return * ((GlideCacheInfo *) &m_cacheInfo); }
};


//================================================================
//                    Bitmap Class Inline Functions
//================================================================

inline Int32 GFXBitmap::getHeight() const
{
   return height;
}


inline Int32 GFXBitmap::getWidth() const
{
   return width;
}

inline Int32 GFXBitmap::getBitDepth() const
{
   return bitDepth;
}

inline void GFXBitmap::getSize( Point2I * size ) const
{
   size->x = this->getWidth();
   size->y = this->getHeight();
}

inline Int32 GFXBitmap::getStride() const
{
   return stride;
}


inline void GFXBitmap::getClientRect(RectI *out_rect) const
{
   //what is the size of the bitmap used by the buffer
   out_rect->operator()(0,0, width-1, height-1);
}


inline BYTE* GFXBitmap::getAddress(const Point2I *in_pt) const
{
   return getAddress(in_pt->x, in_pt->y);
}


inline BYTE* GFXBitmap::getAddress(Int32 in_x, Int32 in_y) const
{
   return (pBits + (in_y * stride) + in_x * (bitDepth >> 3));
}

inline GFXBitmap::GFXBitmap()
 : m_cacheInfo(sm_currSequenceNumber++)
{
   pBits   = NULL;
   pBitsBase = NULL;
   pPalette= NULL;
   width   = 0;
   stride  = 0;
   height  = 0;
   attribute = BMA_NORMAL;
   paletteIndex = 0xFFFFFFFF;

   pMipBits[0] = NULL;
   pMipBits[1] = NULL;
   pMipBits[2] = NULL;
   pMipBits[3] = NULL;
   pMipBits[4] = NULL;
   pMipBits[5] = NULL;
   pMipBits[6] = NULL;
   pMipBits[7] = NULL;
   pMipBits[8] = NULL;
	detailLevels = 0;
}

inline GFXBitmap*
GFXBitmap::createInline(Int32 in_width, Int32 in_height, Int32 in_bpp )
{
   AssertFatal(in_width > 0 && in_height > 0 && in_bpp > 0,
               "GFXBitmap::create: invalid parameters" );

   GFXBitmap *bm = new GFXBitmap;
   if (bm == NULL)
      return NULL;

   bm->width   = in_width;
   bm->stride  = ((in_width * in_bpp >> 3)+3)&(~3);    //round up to nearest DWORD
   bm->height  = in_height;
   bm->bitDepth= in_bpp;
   bm->imageSize = bm->getHeight() * bm->getStride();

   bm->pBitsBase = new BYTE[(bm->getHeight()+2) * bm->getStride()];
   bm->pBits = bm->pBitsBase + bm->getStride();
   bm->attribute |= BMA_OWN_MEM;

   bm->detailLevels = 1;
   bm->pMipBits[0]  = bm->pBits;

   return bm;
}

inline GFXBitmap::~GFXBitmap()
{
   free();
}

inline void GFXBitmap::free()
{
   if (pPalette) delete pPalette;
   if (pBitsBase && (attribute & BMA_OWN_MEM)) delete [] pBitsBase;

   pBits    = NULL;
   pBitsBase= NULL;
   pPalette = NULL;
   attribute= BMA_NORMAL;
}


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif

