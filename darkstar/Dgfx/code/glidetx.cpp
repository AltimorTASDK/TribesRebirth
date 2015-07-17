#include <stdlib.h>
#include "fn_all.h"
#include "g_pal.h"
#include "g_bitmap.h"
#include "p_txcach.h"

#define __MSC__
#define DYNAHEADER
#include "glide.h"
#include "glidetx.h"
#include "gfxmetrics.h"
#include "g_surfac.h"

int NearestPow(int);
namespace Glide 
{

// global mip level cap... if this is set lower than 9, 
// the highest details of textures don't get downloaded and/or
// textures are subsampled on download.

extern TextureCache *tCache;
int mipCap = 9;
bool allowSubSampling = true;
static DWORD curTexSeqCount = 1;
//----------------------------------------------------------------------------

int CountBits(int in_val)
{
	int ret = 0;
	while(in_val)
	{
		ret++;
		in_val >>= 1;		
	}
	return ret;
}

TMU::TMU()
{
   wrapCount = 2;
   curTextureWrapCount = 0;
   curTextureAddress = 0;
   curPaletteIndex = 0xFFFFFFFF;
}

//----------------------------------------------------------------------------

// setCurrentTexture makes the currentTexture equal
// to the texture associated with key.
// if it's not currently in the cache, it returns false.

bool TMU::setCurrentTexture(GFXBitmap::GlideCacheInfo &ci, DWORD paletteIndex, GFXPalette *palette)
{
   if(ci.driverSeqCount != curTexSeqCount)
      return false;

   if(ci.wrapCount == wrapCount || (ci.wrapCount == wrapCount - 1 &&
      ci.texAddress > curTexAddress))
   {
      if(ci.wrapCount == curTextureWrapCount && ci.texAddress == curTextureAddress)
         return true;
      activate(ci, paletteIndex, palette);
      return true;
   }
   return false;
}

void TMU::invalidateAllTextures()
{
   wrapCount += 2;
}

void TMU::flushTexture(GFXBitmap* texture, GFXPalette* io_pPal, const bool in_reload)
{
   if (texture == NULL)
      return;

   GFXBitmap::GlideCacheInfo &ci = texture->getGlideCacheInfo();
   // if the texture's not in the cache, no big deal...
   if(ci.driverSeqCount != curTexSeqCount)
      return;
   if(!( ci.wrapCount == wrapCount || (ci.wrapCount == wrapCount - 1 && ci.texAddress > curTexAddress ) ) )
      return;

   // it's in the cache:
   if(!in_reload) // if we just want to flush it, mark its wrap count as 0
   {
      ci.wrapCount = 0;
      return;
   }
   int hPow = NearestPow(texture->getHeight());
   int wPow = NearestPow(texture->getWidth());

   if (texture->getWidth() == wPow  &&
       texture->getHeight() == hPow &&
       (texture->attribute & BMA_TRANSLUCENT) == 0) {
      downloadData(ci, texture->getAddress(0, 0));
   } else {
      if ((texture->attribute & BMA_TRANSLUCENT) == 0) {
         for(int j = 0; j < texture->getHeight(); j++) {
            BYTE *src = texture->getAddress(0, j);
            BYTE *dst = tCache->downloadBuffer + j * wPow;
            BYTE *dstLast = dst + texture->getWidth();
            while(dst < dstLast)
            {
               *dst++ = *src++;
            }
         }
         downloadData(ci, tCache->downloadBuffer);
      } else {
         if(transRemapIndex != texture->paletteIndex) {
            transRemapIndex = texture->paletteIndex;
            int i;
            GFXPalette::MultiPalette *mp = io_pPal->findMultiPalette(transRemapIndex);
            for(i = 0; i < 256; i++)
               transRemapTable[i] = (DWORD(mp->color[i].peFlags) << 8) | i;
         }
         int i, j, k;
         DWORD *destPtr = (DWORD *) tCache->downloadBuffer;
         BYTE *srcPtr = texture->getAddress(0,0);
         for(k = 0; k < texture->detailLevels; k++) {
            int stride = texture->getStride() >> (k + 1);
            int height = texture->getHeight() >> k;
            for(j = 0; j < height; j++)
            {
               for(i = 0; i < stride; i++)
               {
                  *destPtr++ = transRemapTable[*srcPtr] | 
                              (transRemapTable[*(srcPtr+1)] << 16);
                  srcPtr += 2;
               }
            }
         }
         downloadData(ci, tCache->downloadBuffer);
      }
   }
}

void TMU::advanceCurrentTexturePtr(GFXBitmap::GlideCacheInfo &ci)
{
   DWORD size = grTexCalcMemRequired(ci.smallLod,
                  ci.largeLod, ci.aspectRatio,
                  ci.textureFormat);
   // 8-byte align size...
   size = (size + 7) & ~7;
#ifdef DEBUG
   GFXMetrics.textureBytesDownloaded += size;
   GFXMetrics.incNumTexturesDownloaded();
#endif
   if(size + curTexAddress >= maxTexAddress)
   {
      wrapCount += 1;
      curTexAddress = minTexAddress;
#ifdef DEBUG
      if (tmuId == 0) {
         GFXMetrics.numTMU0Wraps++;
      } else if (tmuId == 1) {
         GFXMetrics.numTMU1Wraps++;
      }
#endif
   }

   UInt32 low  = curTexAddress;
   UInt32 high = curTexAddress + size;
   low  &= ~((2 << 20) - 1);
   high &= ~((2 << 20) - 1);
   if (low != high) {
      curTexAddress = high;

      // double check.  On banshee, this isn't always obvious...
      if(size + curTexAddress >= maxTexAddress) {
         wrapCount    += 1;
         curTexAddress = minTexAddress;
#ifdef DEBUG
         if (tmuId == 0) {
            GFXMetrics.numTMU0Wraps++;
         } else if (tmuId == 1) {
            GFXMetrics.numTMU1Wraps++;
         }
#endif
      }
   }
   ci.wrapCount = wrapCount;
   ci.texAddress = curTexAddress;
   curTexAddress += size;
}

int subsamp;
int startdet;

void TMU::calcLodAndAspectRatio(GFXBitmap::GlideCacheInfo &ci, int width, int height, int mipMapLevels)
{
	AssertFatal(width > 0 && width <= 256, "Invalid width in bitmap");
   AssertFatal(height > 0 && height <= 256, "Invalid height in bitmap");

	// figure out LOD and ASPECT ratio.
	int widthCount = CountBits(width);
	int heightCount = CountBits(height);

   int maxC = 0;
   subsamp = 0;
   startdet = 0;
   if(mipCap != 9)
   {
      maxC = max(widthCount - mipCap, heightCount - mipCap);
      if(maxC > 0)
      {
         maxC = min(maxC, min(widthCount -1, heightCount - 1));
         if(maxC > 0)
         {
            if(!allowSubSampling)
            {
               if(maxC > mipMapLevels - 1)
                  maxC = mipMapLevels - 1;
            }
            widthCount -= maxC;
            heightCount -= maxC;
            mipMapLevels -= maxC;
            startdet += maxC;
            if(mipMapLevels <= 0)
            {
               subsamp = 1 - mipMapLevels;
               startdet -= subsamp;
               mipMapLevels = 1;
            }
         }
      }
   }
	// ok, GR_LOD_256 = 0, GR_LOD_1 = 8
	// formula = 9 - # of highest bit in maxSize

	int bitCount = max(widthCount, heightCount);
   
   // don't download any mipmaps past a 1 wide/high (bitmaps include these for OGL)
   int detailLevels = min(widthCount, min(heightCount, mipMapLevels));

	ci.largeLod = 9 - bitCount;
	ci.smallLod = ci.largeLod + detailLevels - 1;

   AssertFatal(ci.largeLod <= ci.smallLod && ci.smallLod <= GR_LOD_1, "Invalid LOD");

	// figure out the aspect ratio...

	ci.aspectRatio = GR_ASPECT_1x1;
	// for each bit that width is greater than heigth, subtract one
	// for each bit that height is greater than width, add one.

	ci.aspectRatio += (heightCount - widthCount);
}

void TMU::downloadData(GFXBitmap::GlideCacheInfo &ci, const void *srcPtr)
{
   GrTexInfo ti;
   ti.smallLod = ci.smallLod;
   ti.largeLod = ci.largeLod;
   ti.aspectRatio = ci.aspectRatio;
   ti.format = ci.textureFormat;
   ti.data = (void *) srcPtr;
   grTexDownloadMipMap(tmuId, ci.texAddress, GR_MIPMAPLEVELMASK_BOTH, &ti);
}

void TMU::downloadTranslucentBitmap88(GFXBitmap *bmp, GFXPalette *pal, GFXBitmap::GlideCacheInfo &ci)
{
   ci.driverSeqCount = curTexSeqCount;
   
   calcLodAndAspectRatio(ci, bmp->getWidth(), bmp->getHeight(), bmp->detailLevels);
	AssertFatal(ci.aspectRatio <= GR_ASPECT_1x8,
			"Invalid aspect ratio for input bitmap.");
   ci.textureFormat = GR_TEXFMT_AP_88;
   advanceCurrentTexturePtr(ci); // allocate memory on the card...

   int i, j, k;
   UInt8* destPtr = (UInt8*) tCache->downloadBuffer;

   for(k = 0; k < bmp->detailLevels; k++)
   {
      const UInt8* srcPtr = (const UInt8*)bmp->getDetailLevel(k);
      int stride = bmp->getStride() >> k;
      int height = bmp->getHeight() >> k;
      for(j = 0; j < height; j++)
      {
         memcpy(destPtr, srcPtr, stride);
         destPtr += stride;
         srcPtr  += stride;
      }
   }

   downloadData(ci, tCache->downloadBuffer);
   activate(ci, bmp->paletteIndex, pal);
}

void TMU::downloadTranslucentBitmap(GFXBitmap *bmp, GFXPalette *pal, GFXBitmap::GlideCacheInfo &ci)
{
   if (bmp->attribute & BMA_ALPHA88) {
      downloadTranslucentBitmap88(bmp, pal, ci);
      return;
   }

   ci.driverSeqCount = curTexSeqCount;
   
   if(transRemapIndex != bmp->paletteIndex)
   {
      transRemapIndex = bmp->paletteIndex;
      int i;
      GFXPalette::MultiPalette *mp = pal->findMultiPalette(transRemapIndex);
      for(i = 0; i < 256; i++)
         transRemapTable[i] = (DWORD(mp->color[i].peFlags) << 8) | i;
   }

   calcLodAndAspectRatio(ci, bmp->getWidth(), bmp->getHeight(), bmp->detailLevels);
	AssertFatal(ci.aspectRatio <= GR_ASPECT_1x8,
			"Invalid aspect ratio for input bitmap.");
   ci.textureFormat = GR_TEXFMT_AP_88;
   advanceCurrentTexturePtr(ci); // allocate memory on the card...

   int i, j, k;
   DWORD *destPtr = (DWORD *) tCache->downloadBuffer;
   if(subsamp)
   {
      const BYTE *srcPtr = bmp->getDetailLevel(bmp->detailLevels - 1);
      int step = 1 << subsamp;
      int shift = bmp->detailLevels - 1 + subsamp;
      int height = bmp->getHeight() >> shift;
      int stride = bmp->getStride() >> (shift + 1);
      int ystep = ((step << 1) * stride) * (step -1);
      for(j = 0; j < height; j++)
      {
         for(i = 0; i < stride;i++)
         {
            *destPtr++ = transRemapTable[*srcPtr] | 
                         (transRemapTable[*(srcPtr + step)] << 16);
            srcPtr += step << 1;
         }
         srcPtr += ystep;
      }
   }
   else
   {
      const BYTE *srcPtr = bmp->getDetailLevel(startdet);
      for(k = 0; k < bmp->detailLevels; k++)
      {
         int stride = bmp->getStride() >> (k + 1);
         int height = bmp->getHeight() >> k;
         for(j = 0; j < height; j++)
         {
            if (stride != 0) {
               for(i = 0; i < stride; i++)
               {
                  *destPtr++ = transRemapTable[*srcPtr] | 
                              (transRemapTable[*(srcPtr+1)] << 16);
                  srcPtr += 2;
               }
            } else {
               *destPtr++ = transRemapTable[*srcPtr];
            }
         }
      }
   }
   downloadData(ci, tCache->downloadBuffer);
   activate(ci, bmp->paletteIndex, pal);
}

void TMU::downloadBitmap(GFXBitmap *bmp, GFXPalette *pal, GFXBitmap::GlideCacheInfo &ci)
{
   ci.driverSeqCount = curTexSeqCount;
   calcLodAndAspectRatio(ci, bmp->getWidth(), bmp->getHeight(), bmp->detailLevels);
	AssertFatal(ci.aspectRatio <= GR_ASPECT_1x8,
			"Invalid aspect ratio for input bitmap.");
   if(bmp->bitDepth == 8)
      ci.textureFormat = GR_TEXFMT_P_8;
   else if(bmp->bitDepth == 16)
      ci.textureFormat = GR_TEXFMT_RGB_565;
      
   advanceCurrentTexturePtr(ci); // allocate memory on the card...
   if(!subsamp || bmp->bitDepth != 8)
      downloadData(ci, bmp->getDetailLevel(startdet));
   else
   {
      const BYTE *srcPtr = bmp->getDetailLevel(startdet);
      BYTE *destPtr = (BYTE *) tCache->downloadBuffer + 65536;

      int shift = bmp->detailLevels - 1 + subsamp;
      int height = bmp->getHeight() >> shift;
      int stride = bmp->getStride() >> shift;
      int xstep = 1 << subsamp;
      int ystep = (xstep * stride) * (xstep - 1);
      for(int i = 0; i < height; i++)
      {
         for(int j = 0; j < stride; j++)
         {
            *destPtr++ = *srcPtr;
            srcPtr += xstep;
         }
         srcPtr += ystep;
      }
      downloadData(ci, tCache->downloadBuffer + 65536);
   }
   activate(ci, bmp->paletteIndex, pal);
}

void TMU::downloadLightMap(GFXBitmap::GlideCacheInfo &ci, GFXLightMap *map, int lmapWidth, int lmapHeight)
{
   ci.driverSeqCount = curTexSeqCount;
   allowSubSampling = false;
   calcLodAndAspectRatio(ci, lmapWidth, lmapHeight, 1);
   allowSubSampling = true;
	AssertFatal(ci.aspectRatio <= GR_ASPECT_1x8,
			"Invalid aspect ratio for input light map.");
   ci.textureFormat = GR_TEXFMT_ARGB_4444;
   advanceCurrentTexturePtr(ci);

	int i, j;
	UInt16 *srcPtr = &(map->data);
   UInt16 *dbuff = (UInt16 *) gfxTextureCache.allocateChunk(lmapWidth * lmapHeight * 2);

	UInt16 *dstPtr = dbuff;
	//(UInt16 *) tCache->downloadBuffer;
	for(j = 0; j < map->size.y; j++)
	{
		for(i = 0; i < map->size.x; i++)
			*dstPtr++ = *srcPtr++;
		dstPtr += lmapWidth - map->size.x;
	}
   downloadData(ci, dbuff);
   activate(ci, 0xFFFFFFFF, NULL);
}

void TMU::activate(GFXBitmap::GlideCacheInfo &ci, DWORD paletteIndex, GFXPalette *palette)
{
   // check if we need to download a different palette
   if (palette) {
      if (paletteIndex == 0) {
         // Remapped bitmap
         //
         if (curPaletteIndex != DWORD(palette)) {
            GFXPalette::MultiPalette *mp = &palette->palette[0];
            GuTexTable t;

            int i;
            for(i = 0; i < 256; i++)
            {
               PALETTEENTRY color = mp->color[i];
               t.palette.data[i] = (UInt32(color.peRed)   << 16) | 
                                   (UInt32(color.peGreen) << 8 ) | 
                                   (UInt32(color.peBlue) );
            }
            tCache->tmu[0].curPaletteIndex = 0xFFFFFFFF;
            tCache->tmu[1].curPaletteIndex = 0xFFFFFFFF;
      
            grTexDownloadTable(tmuId, GR_TEXTABLE_PALETTE, (void *) &t);
            GFXMetrics.numPaletteDLs++;
            curPaletteIndex = DWORD(palette);
         }
      } else {
         // Normal bitmap
         //
         if (paletteIndex != curPaletteIndex) {
            GFXPalette::MultiPalette *mp = palette->findMultiPalette(paletteIndex);
            GuTexTable t;

            int i;
            for(i = 0; i < 256; i++)
            {
               PALETTEENTRY color = mp->color[i];
               t.palette.data[i] = (UInt32(color.peRed)   << 16) | 
                                   (UInt32(color.peGreen) << 8 ) | 
                                   (UInt32(color.peBlue) );
            }
            tCache->tmu[0].curPaletteIndex = 0xFFFFFFFF;
            tCache->tmu[1].curPaletteIndex = 0xFFFFFFFF;
      
            grTexDownloadTable(tmuId, GR_TEXTABLE_PALETTE, (void *) &t);
            curPaletteIndex = paletteIndex;
            GFXMetrics.numPaletteDLs++;
         }
      }
   } else {
      // No need, it's a truecolor.
   }

//   if (palette && paletteIndex != curPaletteIndex)
//   {
//      GFXPalette::MultiPalette *mp = palette->findMultiPalette(paletteIndex);
//      GuTexTable t;
//
//      int i;
//      for(i = 0; i < 256; i++)
//      {
//         PALETTEENTRY color = mp->color[i];
//         t.palette.data[i] = (UInt32(color.peRed)   << 16) | 
//                             (UInt32(color.peGreen) << 8 ) | 
//                             (UInt32(color.peBlue) );
//      }
//      tCache->tmu[0].curPaletteIndex = 0xFFFFFFFF;
//      tCache->tmu[1].curPaletteIndex = 0xFFFFFFFF;
//      
//      grTexDownloadTable(tmuId, GR_TEXTABLE_PALETTE, (void *) &t);
//      curPaletteIndex = paletteIndex;
//      GFXMetrics.numPaletteDLs++;
//   }

   GrTexInfo ti;
   ti.smallLod = ci.smallLod;
   ti.largeLod = ci.largeLod;
   ti.aspectRatio = ci.aspectRatio;
   ti.format = ci.textureFormat;

   curTextureAddress = ci.texAddress;
   curTextureWrapCount = ci.wrapCount;

   grTexSource(tmuId, ci.texAddress, GR_MIPMAPLEVELMASK_BOTH, &ti);
}

TextureCache::TextureCache(int tmuc, int in_tableSize)
{
   curTexSeqCount++;
   frameU = 1;
   curTMU = 0;
   tmuCount = tmuc;
   if(tmuCount > MAX_TMUS)
      tmuCount = MAX_TMUS;
	tableSize = in_tableSize;

	int i, j;
	downloadBuffer = new BYTE[174762]; // Maximum 256x256x16 image mip-mapped to 1x1

   for(i = 0; i < tmuCount; i++)
   {
      tmu[i].tmuId = i;
      tmu[i].wrapCount = 0;

      tmu[i].minTexAddress = grTexMinAddress(i);
      tmu[i].maxTexAddress = grTexMaxAddress(i);

      tmu[i].minTexAddress = (tmu[i].minTexAddress + 7) & ~7;
      tmu[i].maxTexAddress = tmu[i].maxTexAddress & ~7;
      tmu[i].curTexAddress = tmu[i].minTexAddress;
   }
}

void TextureCache::flush()
{
   for(int j = 0; j < tmuCount; j++)
      tmu[j].invalidateAllTextures();
}

TextureCache::~TextureCache()
{
	delete [] downloadBuffer;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

inline int HandleCache::HashHandle(GFXTextureHandle tex)
{
	int k = tex.key[0] ^ tex.key[1];
	return (k >> 16) ^ (k & 0xFFFF);
}

inline Bool HandleCache::HandleEqual(GFXTextureHandle t1, GFXTextureHandle t2)
{
	return (t1.key[0] == t2.key[0]) && (t1.key[1] == t2.key[1]);
}


void HandleCache::touch(HandleCacheEntry *entry)
{
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;

	entry->next = freeListTail;
	entry->prev = freeListTail->prev;

	freeListTail->prev = entry;
	entry->prev->next = entry;
}

HandleCacheEntry* HandleCache::getFreeEntry()
{
	HandleCacheEntry *ret = freeListHead->next;
	if(ret->lMap)
	{
		gfxLightMapCache.release(ret->lMap);
		ret->lMap = 0;
	}

	int index = HashHandle(ret->handle) & (entryTableSize - 1);
	HandleCacheEntry **walk = &entryTable[index];
	while(*walk)
	{
		if(*walk == ret)
		{
			*walk = ret->hashLink;
			break;
		}
		walk = &((*walk)->hashLink);
	}
	touch(ret);
	currentEntry = ret;

   ret->flags = 0;
   ret->lMapInfo.driverSeqCount = 0xFFFFFFFF;
   ret->tMapInfo.driverSeqCount = 0xFFFFFFFF;
   ret->bmp   = NULL;

	return ret;
}

void HandleCache::HashInsert(HandleCacheEntry *tce)
{
	int index = HashHandle(tce->handle) & (entryTableSize - 1);
	tce->hashLink = entryTable[index];
	entryTable[index] = tce;
}

Bool HandleCache::setTextureHandle(GFXTextureHandle handle)
{
	int index = HashHandle(handle) & (entryTableSize - 1);
	currentEntry = entryTable[index];
	while(currentEntry)
	{
		if(HandleEqual(currentEntry->handle, handle))
			break;
		currentEntry = currentEntry->hashLink;
	}
	if(currentEntry != NULL)
	{
		touch(currentEntry);
		return TRUE;
	}
	return FALSE;
}

HandleCache::HandleCache(int numHandles, int handleHashTableSize)
  : m_numHandles(numHandles)
{
	curCbKey = 1;

	array = new HandleCacheEntry[numHandles];
   int i;
	for(i = 0; i < numHandles;i++)
	{
		array[i].next = array + i + 1;
		array[i].prev = array + i - 1;
		array[i].lMap = NULL;
	}
	array[0].prev = NULL;
	array[numHandles-1].next = NULL;
		
	entryTableSize = handleHashTableSize;
	entryTable = new HandleCacheEntry*[entryTableSize];
	for(i = 0; i < entryTableSize; i++)
		entryTable[i] = NULL;
	freeListHead = array;
	freeListTail = array + numHandles - 1;
	currentEntry = NULL;
}

HandleCache::~HandleCache()
{
	delete[] array;
	delete[] entryTable;
}

void HandleCache::flush()
{
   currentEntry = NULL;
   int i;
   for(i = 0; i < entryTableSize;i++)
      entryTable[i] = NULL;
   
   // Whip through and release all the lightmaps...
   //
   for (i = 0; i < m_numHandles; i++) {
      if (array[i].lMap != NULL) {
		   gfxLightMapCache.release(array[i].lMap);
         array[i].lMap  = NULL;
         array[i].flags = 0;
      }
   }

#ifdef DEBUG
   GFXMetrics.numTMU0Wraps = 0;
   GFXMetrics.numTMU1Wraps = 0;
#endif
}

};