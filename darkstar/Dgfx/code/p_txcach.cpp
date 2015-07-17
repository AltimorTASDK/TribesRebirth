//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <stdlib.h>
#include <m_random.h>

#include "p_tce.h"
#include "g_surfac.h"
#include "g_pal.h"
#include "g_bitmap.h"
#include "g_contxt.h"
#include "fn_all.h"
#include "rn_manag.h"
#include "p_txcach.h"
#include "gfxmetrics.h"

//----------------------------------------------------------------------------

static int sizes[] = { 32, 80, 200, 400, 1100, 5300, 22000, 0 };

GFXTextureCache gfxTextureCache(3000, 4096, 4096 * 1024);
GFXLightMapCache gfxLightMapCache(sizes);

extern "C" void __cdecl
gfx_cache_texture(BYTE *srcBits, BYTE *dstBits, UInt16 *lightBits,
                  int squareSize, int squareShift, int mapStride,
                  int bitmapStride, int cacheStride, BYTE *shadeMap,
                  int shadeShift);


static int getShift(int x)
{
   int i = 0;
   if (x)
      while (!(x & 1))
         i++, x >>= 1;
   return i;
}


// light and cache a texture at the same time.

const UInt32 sg_numRandomEntries = 2047;
extern "C" UInt32 sg_randomNoise[];
extern "C" UInt32 g_currNoiseIndex;

void
disableCacheNoise()
{
   memset(sg_randomNoise, 0, (sg_numRandomEntries * sizeof(UInt32)));
}

void
setupCacheNoise(const float in_linearBias)
{
   Random randGen;
   const float lmFixOne = float(0x1000);

   for (int i = 0; i < sg_numRandomEntries; i++) {
      float randVal = randGen.getFloat();
      if (randVal <= 1.0f/3.0f) {
         sg_randomNoise[i] = 0;
      } else if (randVal <= 2.0f/3.0f) {
         sg_randomNoise[i] = int(lmFixOne * in_linearBias);
      } else {
         sg_randomNoise[i] = int(-lmFixOne * in_linearBias);
      }
   }
}

inline int
getNextNoiseVal()
{
   // Slow slow slow!  Unfortunately, if the number of noise entries is a power
   //  of two, noticable aliasing appears on the surface.
   //
   g_currNoiseIndex++;
   if (g_currNoiseIndex >= sg_numRandomEntries)
      g_currNoiseIndex = 0;

   return sg_randomNoise[g_currNoiseIndex];
}

inline void
setNoiseStart(const DWORD in_noiseKey)
{
   // Since sg_numRandomEntries is (2^x - 1) it does quite nicely as a mod
   //  value.  Note that this function can return an out of bounds index,
   //  namely, sg_numRandomEntries, so we prime the value by tossing out
   //  a noise val...
   //
   g_currNoiseIndex = in_noiseKey & sg_numRandomEntries;
   getNextNoiseVal();
}


inline int GFXTextureCache::HashHandle(GFXTextureHandle tex)
{
   int k = tex.key[0] ^ tex.key[1];
   return (k >> 16) ^ (k & 0xFFFF);
}

void GFXTextureCache::HashInsert(GFXTextureCacheEntry *tce)
{
   int index = HashHandle(tce->handle) & (hashTableSize - 1);
   tce->hashLink = hashTable[index];
   hashTable[index] = tce;
}

inline Bool GFXTextureCache::HandleEqual(GFXTextureHandle t1, GFXTextureHandle t2)
{
   return (t1.key[0] == t2.key[0]) && (t1.key[1] == t2.key[1]);
}

GFXTextureCacheEntry* GFXTextureCache::getEntry(GFXTextureHandle tex)
{ 
   tex;
   return NULL;
}

//----------------------------------------------------------------------------

// cache and light a texture with no wrapping/offset

static void RecacheTexture(GFXTextureCacheEntry *tce, GFXPalette *palette)
{
   int mipLevel      = tce->mipLevel;
   GFXBitmap* srcTex = (GFXBitmap *) tce->bmp;
   AssertFatal(mipLevel < srcTex->detailLevels,
               "GFXTextureCache::RecacheTexture: Bitmap missing mip levels.");

   UInt8* srcBits = srcTex->pMipBits[mipLevel];
   
   // Setup the cache noise
   //
   setNoiseStart(DWORD(srcTex));
   
   // now prepare to light the texture.
   //
   UInt8 *shadeMap  = palette->findMultiPalette(srcTex->paletteIndex)->shadeMap;
   int shadeShift   = 16 - palette->shadeShift - PAL_SHIFT;
   UInt8 *bits      = (UInt8 *) tce->dataPtr;

   int dstStride;
   int srcStride = tce->csizeX;
   if (gfxTextureCache.isPadding() == false) {
      dstStride = srcStride;
   } else {
      dstStride = (srcStride + 0x3) & (~0x3);
   }
   dstStride;

   GFXLightMap *map = tce->lMap;

   int mapStride    = map->size.x;
   int mapShift     = getShift(mapStride - 1);

   int squareSize   = srcStride >> mapShift;
   int squareShift  = getShift(squareSize);
   UInt16 *light    = &(map->data);
   
   // jump into asm routine.
   //   
   gfx_cache_texture(srcBits, bits, light, squareSize, squareShift,
                     mapStride, srcStride, dstStride, shadeMap, shadeShift);
}

// build and light a texture by calling the callback and then
// lighting it.

static void RecacheCallbackTexture(GFXTextureCacheEntry *tce, GFXPalette *palette)
{
   // first create a bogus bmp to pass into the callback.
   //
   GFXBitmap bmp;
   bmp.stride = bmp.width = tce->csizeX;
   bmp.height = tce->csizeY;
   bmp.pBits  = tce->dataPtr;

   // call the callback to generate it.
   //
   tce->cb(tce->handle, &bmp, 0);
   
   // Setup the cache noise
   //
   setNoiseStart(DWORD(tce->handle.key[0]));

   // now prepare to light the texture.
   //
   UInt8 *shadeMap  = palette->findMultiPalette(bmp.paletteIndex)->shadeMap;
   int shadeShift   = 16 - palette->shadeShift - PAL_SHIFT;
   UInt8 *bits      = (UInt8 *) tce->dataPtr;
   GFXLightMap *map = tce->lMap;

   int dstStride;
   int srcStride = tce->csizeX;
   if (gfxTextureCache.isPadding() == false) {
      dstStride = srcStride;
   } else {
      dstStride = (srcStride + 0x3) & (~0x3);
   }
   dstStride;

   int mapStride    = map->size.x;
   int mapShift     = getShift(mapStride - 1);

   int squareSize   = srcStride >> mapShift;
   int squareShift  = getShift(squareSize);
   UInt16 *light    = &(map->data);
   
   // jump into asm routine.
   //   
   gfx_cache_texture(bits, bits, light,
                     squareSize, squareShift,
                     mapStride,
                     srcStride,
                     dstStride,
                     shadeMap, shadeShift);
}

static void 
RecacheWrappingTexture(GFXTextureCacheEntry *tce, GFXPalette *palette)
{
   int mipLevel    = tce->mipLevel;
   int offsetX     = tce->offsetX >> mipLevel;
   int offsetY     = tce->offsetY >> mipLevel;
   int lightScale  = tce->lightScale - mipLevel;
   GFXBitmap* srcTex = (GFXBitmap *) tce->bmp;

   UInt8 *shadeMap  = palette->findMultiPalette(srcTex->paletteIndex)->shadeMap;
   int shadeShift  = 16 - palette->shadeShift - PAL_SHIFT;

   int lightSize   = 1 << lightScale;

   UInt8* dstBits  = (UInt8*) tce->dataPtr;

   int dstStride;
   if (gfxTextureCache.isPadding() == false) {
      dstStride = tce->csizeX;
   } else {
      dstStride = (tce->csizeX + 0x3) & (~0x3);
   }

   AssertFatal(mipLevel < srcTex->detailLevels,
      "GFXTextureCache::RecacheWrappingTexture: Bitmap missing mip levels.");
   UInt8* srcBits = srcTex->pMipBits[mipLevel];

   // Setup the cache noise
   //
   setNoiseStart(DWORD(srcTex));

   int sxMask  = (srcTex->width >> mipLevel) - 1;
   int syMask  = (srcTex->height >> mipLevel) - 1;
   int sxShift = getShift(srcTex->stride >> mipLevel);

   GFXLightMap *map = tce->lMap;
   int mipRound     = mipLevel? 1 << (mipLevel - 1): 0;

   UInt16* mapBits = &map->data;
   int mapStride   = map->size.x;
   int mapOffsetX  = (map->offset.x + mipRound) >> mipLevel;
   int mapOffsetY  = (map->offset.y + mipRound) >> mipLevel;
   int alphaMask   = ~((1 << 12) - 1);

   // Scan throught the lightmap..
   for (int y = 0; y < map->size.y - 1; y++) {
      for (int x = 0; x < map->size.x - 1; x++) {
         // Get corner intensity values and stepping
         // values for left and right edge.
         UInt16* mb = mapBits + (y * mapStride) + x;
         int lightLeft = mb[0] & alphaMask;
         int lightLeftStep = ((mb[mapStride] & alphaMask) - lightLeft) >> lightScale;
         int lightRight = mb[1] & alphaMask;
         int lightRightStep = ((mb[mapStride + 1] & alphaMask) - lightRight) >> lightScale;

         // Convert light map coor to texture coor.
         int dx = (x << lightScale) - mapOffsetX;
         int dy = (y << lightScale) - mapOffsetY;
         UInt8* dst = dstBits + (dy * dstStride) + dx;

         // Get Source texture coor.
         int sxStart = dx + offsetX;
         int syStart = dy + offsetY;

         // Presteping and overun control Y
         int startY = 0;
         if (dy < 0) {
            startY = -dy;
            lightLeft  += lightLeftStep * startY;
            lightRight += lightRightStep * startY;
            dst        += dstStride * startY;
         }
         int endY = lightSize;
         if (dy + endY > tce->csizeY)
            endY = tce->csizeY - dy;

         // Presteping and overun control X
         int startX = 0;
         if (dx < 0) {
            startX = -dx;
            dst += startX;
         }
         int endX = lightSize;
         if (dx + endX > tce->csizeX)
            endX = tce->csizeX - dx;

         // Actual core loops.  Steps left & right edge down
         // and interpolate values across.
         for (int iy = startY; iy < endY; iy++) {
            // Setup for most inner loop
            int lightStep = (lightRight - lightLeft) >> lightScale;
            int light = startX? lightLeft + lightStep * startX: lightLeft;
            UInt8* sy = srcBits + (((syStart + iy) & syMask) << sxShift);
            UInt8* dp = dst;

            // Horizontal scan.
            for (int ix = startX; ix < endX; ix++) {
               Int32 lightNew = getNextNoiseVal();
               lightNew += light;
               if (lightNew > Int32(0xf000)) lightNew = Int32(0xf000);
               if (lightNew < 0)             lightNew = 0;

               *dp++ = shadeMap[((lightNew >> shadeShift) & ~((1 << PAL_SHIFT) - 1)) +
                                *(sy + ((sxStart + ix) & sxMask))];
               light += lightStep;
            }

            // Y Stepping.
            dst += dstStride;
            lightLeft += lightLeftStep;
            lightRight += lightRightStep;
         }
      }
   }
}


//----------------------------------------------------------------------------

void GFXTextureCache::flushCache(void)
{
   // this just needs to whip through the hash table and
   // invalidate all the entries.
   //
   int i;
   for(i = 0; i < hashTableSize; i++)
      hashTable[i] = 0;
      
   // And whip through and release all the associated lightmaps...
   //
   for (i = 0; i < arraySize; i++) {
      if (array[i].lMap != NULL) {
         gfxLightMapCache.release(array[i].lMap);
         array[i].lMap = NULL;
      }
   }
}

void* GFXTextureCache::allocateChunk(int size)
{
   // Align the returned pointer on a DWord boundary...
   //
   curDataPtr = reinterpret_cast<BYTE*>((UInt32(curDataPtr) + 0x3) & ~0x3);

   if(curDataPtr + size > bitmapData + dataCacheSize)
   {
      curDataPtr = bitmapData;
      wrapCount++;
   }
   void *ret = curDataPtr;
   curDataPtr += size;
   return ret;
}

void GFXTextureCache::allocate(GFXTextureCacheEntry *ent)
{
   // implements the wrap-around cache.
   // if it's a callback texture then csizex and csizeY are
   // already set - all we have to do is allocate the data.
   //
   if(!ent->testFlag(GFXTextureCacheEntry::isCallbackTexture))
   {
      if(ent->mipLevel > ent->lightScale)
         ent->mipLevel = ent->lightScale;
      ent->csizeX = ent->sizeX >> ent->mipLevel;
      ent->csizeY = ent->sizeY >> ent->mipLevel;
   }

   // Align the returned pointer on a DWord boundary...
   //
   curDataPtr = reinterpret_cast<BYTE*>((UInt32(curDataPtr) + 0x3) & ~0x3);

   int xSize;
   if (isPadding() == false) {
      xSize = ent->csizeX;
   } else {
      xSize = (ent->csizeX + 0x3) & (~0x3);
   }

   int size = xSize * ent->csizeY;
   if(curDataPtr + size > bitmapData + dataCacheSize)
   {
      curDataPtr = bitmapData;
      wrapCount++;
   }

   ent->dataPtr   = curDataPtr;
   ent->wrapCount = wrapCount;
   curDataPtr += size;
}

inline Bool GFXTextureCache::overCached(GFXTextureCacheEntry *ent)
{
   if(ent->wrapCount == wrapCount)
      return FALSE;
   if((ent->wrapCount == wrapCount - 1) && (ent->dataPtr >= curDataPtr))
      return FALSE;
   // otherwise this has been written over in the cache.
   return TRUE;
}

void GFXTextureCache::prepareForDraw(GFXTextureCacheEntry *tce, GFXPalette *palette)
{
   AssertFatal(tce, "Invalid texture cache entry.");
   if(overCached(tce)) {
      allocate(tce);
      tce->setFlag(GFXTextureCacheEntry::needsRecache);
   }
   if(tce->testFlag(GFXTextureCacheEntry::needsRecache)) {

#ifdef DEBUG
      GFXMetrics.addToTextureBytesDL(tce->csizeX * tce->csizeY);
#endif
      GFXMetrics.incRecachedSurfaces();
      tce->cf(tce, palette);
      tce->clearFlag(GFXTextureCacheEntry::needsRecache);
   }
   gfxPDC.textureMap       = tce->dataPtr;
   gfxPDC.textureMapStride = tce->csizeX;
   gfxPDC.textureMapHeight = tce->csizeY;
   
#ifdef DEBUG
   if (tce->frameUsed != frameU) {
      tce->frameUsed = frameU;
      GFXMetrics.addToTextureSU(tce->csizeX * tce->csizeY);
   }
#endif
}

GFXLightMap* GFXTextureCache::getLightMap()
{
   AssertFatal(currentEntry, "Invalid texture cache entry.");
   return currentEntry->lMap;
}

void GFXTextureCache::setLightMap(int lightScale, GFXLightMap *map)
{
   // add the current light map to the free list for its size.
   if(currentEntry->lMap && currentEntry->lMap != map) {
      gfxLightMapCache.release(currentEntry->lMap);
   }

   currentEntry->setFlag(GFXTextureCacheEntry::needsRecache);
   currentEntry->lMap = map;
   currentEntry->lightScale = lightScale;
   if(lightScale < currentEntry->mipLevel) {
      // force reallocate - light scale changed.
      currentEntry->mipLevel = lightScale;
      currentEntry->wrapCount = wrapCount - 2;
   }
}

void GFXTextureCache::setTextureMap(const GFXBitmap *bmp)
{
   currentEntry->setFlag(GFXTextureCacheEntry::needsRecache);
   currentEntry->bmp = bmp;
}

void GFXTextureCache::setMipLevel(int mipLevel)
{
   if(mipLevel > currentEntry->lightScale)
      mipLevel = currentEntry->lightScale;

   if(currentEntry->mipLevel != mipLevel) {
      // force a reallocation of memory.
      currentEntry->wrapCount = wrapCount - 2;
      if(mipLevel < currentEntry->bmp->detailLevels)
         currentEntry->mipLevel   = mipLevel;
      else
         currentEntry->mipLevel = currentEntry->bmp->detailLevels - 1;
   }
}

void GFXTextureCache::setTextureSize(int size)
{
   if(currentEntry->csizeX != size)
   {
      currentEntry->csizeX = size;
      currentEntry->csizeY = size;
      // force reallocation of textures.
      currentEntry->wrapCount = wrapCount - 2;
   }
}

GFXTextureCache::GFXTextureCache(int numElems, int htSize, int dataSize)
 : frameU(1),
   dwordPadTextures(false)
{
   arraySize = numElems;
   array = new GFXTextureCacheEntry[numElems];
   int i;
   for(i = 0; i < numElems; i++)
   {
      array[i].flags = 0;
      array[i].next = array + i + 1;
      array[i].prev = array + i - 1;
      array[i].handle.key[0] = 0;
      array[i].handle.key[1] = 0;
      array[i].lMap = NULL;
   }
   array[0].prev = NULL;
   array[numElems-1].next = NULL;

   hashTableSize = htSize;
   hashTable = new GFXTextureCacheEntry*[htSize];
   for(i = 0; i < htSize; i++)
   {
      hashTable[i] = NULL;
   }
   freeListHead = array;
   freeListTail = array + numElems - 1;
   bitmapDataBase = new BYTE[dataSize + 512000]; // give us some pad...
   bitmapData = bitmapDataBase + 256000;
   dataCacheSize = dataSize;
   wrapCount = 0;
   curDataPtr = bitmapData;
   currentEntry = NULL;
}

GFXTextureCache::~GFXTextureCache()
{
   delete[] array;
   delete[] bitmapDataBase;
   delete[] hashTable;
}

void 
GFXTextureCache::enablePadding(const bool in_enable)
{
   if (in_enable == dwordPadTextures) {
      return;
   } else {
      dwordPadTextures = in_enable;
   }
}


void GFXTextureCache::touch(GFXTextureCacheEntry *entry)
{
   entry->next->prev = entry->prev;
   entry->prev->next = entry->next;

   entry->next = freeListTail;
   entry->prev = freeListTail->prev;

   freeListTail->prev = entry;
   entry->prev->next  = entry;
}

GFXTextureCacheEntry* GFXTextureCache::getFreeEntry()
{
   // take the free entry from the head of the free list.
   GFXTextureCacheEntry *ret = freeListHead->next;

#ifdef DEBUG
   ret->frameUsed = 0;
#endif

   // Add the expired light map to the free list for its size.
   if (ret->lMap) {
      gfxLightMapCache.release(ret->lMap);
      ret->lMap = 0;
   }

   // gotta remove this puppy from it's current hash bucket...
   //
   int index = HashHandle(ret->handle) & (hashTableSize - 1);
   GFXTextureCacheEntry **walk = &hashTable[index];
   while(*walk) {
      if(*walk == ret) {
         *walk = ret->hashLink;
         break;
      }
      walk = &((*walk)->hashLink);
   }
   
   ret->wrapCount = wrapCount - 2; // force allocation...
   touch(ret);
   return ret;
}

void GFXTextureCache::registerTexture(GFXTextureHandle tex, int sizeX, int sizeY,
      int offsetX, int offsetY, int lightScale, GFXLightMap *map,
      const GFXBitmap *texture, int mipLevel)
{
   GFXTextureCacheEntry *ent = getFreeEntry();

   if(sizeX == texture->getWidth() && sizeY == texture->getHeight() &&
      offsetX == 0 && offsetY == 0 &&
      map->offset.x == 0 && map->offset.y == 0 && 
      ((map->size.x - 1) << lightScale) == sizeX &&
      ((map->size.y - 1) << lightScale) == sizeY)
      ent->cf = RecacheTexture;
   else
      ent->cf = RecacheWrappingTexture;

   ent->flags      = 0;
   ent->handle     = tex;
   ent->sizeX      = sizeX;
   ent->sizeY      = sizeY;
   ent->offsetX    = offsetX;
   ent->offsetY    = offsetY;
   ent->lightScale = lightScale;

   if(mipLevel < texture->detailLevels)
      ent->mipLevel   = mipLevel;
   else
      ent->mipLevel = texture->detailLevels - 1;

   ent->dataPtr    = 0;
   ent->lMap       = map;
   ent->bmp        = texture;

   HashInsert(ent);
   currentEntry = ent;
}

void GFXTextureCache::registerTexture(GFXTextureHandle tex, GFXCacheCallback cb,
      int csizeX, int csizeY, int lightScale, GFXLightMap *map)
{
   GFXTextureCacheEntry *ent = getFreeEntry();
   
   ent->cf = RecacheCallbackTexture;
   ent->flags   = GFXTextureCacheEntry::isCallbackTexture;
   ent->handle  = tex;
   ent->csizeX  = csizeX;
   ent->csizeY  = csizeY;
   ent->cb      = cb;
   ent->dataPtr = 0;
   ent->lightScale = lightScale;
   ent->lMap       = map;
   HashInsert(ent);
   currentEntry = ent;
}

Bool GFXTextureCache::setTextureHandle(GFXTextureHandle tex)
{
   int index = HashHandle(tex) & (hashTableSize - 1);
   currentEntry = hashTable[index];

   while(currentEntry) {
      // check if keys are equal...
      //
      if(HandleEqual(currentEntry->handle, tex))
         break;
      currentEntry = currentEntry->hashLink;
   }
   if(currentEntry != NULL) {
      touch(currentEntry);
      return TRUE;
   }
   return FALSE;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// implementation for the GFXLightMapCache

GFXLightMapCache::GFXLightMapCache(int *sizeList)
{
   // setup metrics for light maps.
   GFXMetrics.numLightMaps = GFXMetrics.lightMapsInUse = 0;

   // sizeList is a zero-terminated array of maximum sized light maps.
   int i = 0;
   arrayHeadTag *tag;
   while(sizeList[i])
      i++;
   arrays = (arrayHeadTag *) malloc(i * sizeof(arrayHeadTag));
   i = 0;
   while(sizeList[i])
   {
      tag = arrays + i;
      tag->maxSize = sizeList[i];
      tag->stackSize = 0;
      tag->stackCount = 0;
      tag->stack = (void **) malloc(4);
      tag->link = NULL;
      i++;
   }
   maxMaxSize = tag->maxSize;
   numArrays = i;
}

GFXLightMapCache::~GFXLightMapCache()
{
   int i;
   for(i = 0; i < numArrays; i++)
   {
      arrayHeadTag *tag = arrays + i;
      free(tag->stack);
      data_page *walk = tag->link;
      while(walk)
      {
         data_page *temp = walk;
         walk = walk->next;
         free(temp);
      }
   }
   free(arrays);
}

void GFXLightMapCache::allocateMore(arrayHeadTag *head)
{
   // increment global lightMap metrics.
   GFXMetrics.numLightMaps += PageSize;

   // Make sure stack is large enough
   head->stackSize = head->stackSize + PageSize;
   head->stack = (void **) realloc(head->stack,
      sizeof(void *) * head->stackSize);

   // Allocate a new page
   int recSize = (head->maxSize << 1) + sizeof(GFXLightMap);
   data_page *newdata = (data_page *)
      malloc(recSize * PageSize + sizeof(data_page*));
   newdata->next = head->link;
   head->link = newdata;

   // Add new maps from page onto stack
   for (int i = 0; i < PageSize; i++)
      head->stack[head->stackCount++] = (void *)
         (((BYTE *) &newdata->data) + recSize * i);
}

GFXLightMap* GFXLightMapCache::allocate(int numPixels)
{
   int count = 0;
   AssertFatal(numPixels <= maxMaxSize, "Light map too large.");
   //  Find smallest lightmap buffer
   while (arrays[count].maxSize < numPixels)
      count++;
   arrayHeadTag *a = arrays + count;
   if(a->stackCount == 0)
      allocateMore(a);
   GFXLightMap *ret = (GFXLightMap *) a->stack[--a->stackCount];
   ret->index = (BYTE) count;

   GFXMetrics.lightMapsInUse++;
   return ret;
}

void GFXLightMapCache::release(GFXLightMap *lmap)
{
   int count = lmap->index;
   AssertFatal(count < numArrays, "Error, out of range index");

   arrayHeadTag *a = arrays + count;
   a->stack[a->stackCount] = (void *) lmap;
   a->stackCount++;
   GFXMetrics.lightMapsInUse--;
}

