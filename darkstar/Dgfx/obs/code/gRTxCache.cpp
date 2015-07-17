//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "g_pal.h"
#include "p_funcs.h"
#include "p_txcach.h"
#include "p_tce.h"

#include "gRTxCache.h"
#include "renditionProxy.h"
#include "gRSfc.h"

#ifdef DEBUG
#define GRINLINE
#else
#define GRINLINE inline
#endif


namespace Rendition {

const UInt32 TextureCache::sm_hashTableSize = 257;


// Try not to change the size, this is exactly one cache line...
//
struct TextureCacheEntry {
   v_surface*  pTxSurf;
   
   // pRepresent points to either a texture cache entry, or the bytes
   //  of the texture map.  We can determine the context only at the
   //  setTexture(...) and downloadTexture(...) call sites...
   //
   UInt32 key1;
   UInt32 key2;

   TextureCacheEntry* pHashNext;
   
   TextureCacheEntry* pAgeNext;
   TextureCacheEntry* pAgePrev;
   TextureCacheEntry* pFreeNext;

   UInt16 stride;
   UInt16 height;
};


//------------------------------------------------------------------------------
// NAME 
//    TextureCache::TextureCache(...)
//    
// NOTES 
//    We do some trickiness here to enable us to not include the definition of
//   the texture cache entries in the header.  Namely, we initialize the
//   ageHead and tail, and free list references with (*(new TCE)), which means
//   we have to delete them in the d'tor...
//------------------------------------------------------------------------------
TextureCache::TextureCache(GFXTextureCache& io_rTxCache,
                           RenditionProxy*  io_pProxy,
                           v_handle         io_vHandle,
                           Surface*         io_pSurface)
 : m_gfxTxCache(io_rTxCache),
   m_pProxy(io_pProxy),
   m_vHandle(io_vHandle),
   
   m_rFreeList(*(new TextureCacheEntry)),
   m_rAgeHead(*(new TextureCacheEntry)),
   m_rAgeTail(*(new TextureCacheEntry)),
   m_currentEntry(NULL),

   m_pNormalTranslation(NULL),
   m_pNormalSurface(NULL),
   m_pAlphaTranslation(NULL),
   m_pAlphaSurface(NULL),
   
   m_pSurface(io_pSurface)
{
   // allocate our texture entries, hard coded for now to 1024 in class def.,
   //  and set up the free list...
   //
   m_pEntryBuffer = new TextureCacheEntry[NUM_TXCACHE_ENTRIES];
   memset(m_pEntryBuffer, 0, sizeof(TextureCacheEntry) * NUM_TXCACHE_ENTRIES);
   for (int i = 0; i < (NUM_TXCACHE_ENTRIES - 1); i++) {
      m_pEntryBuffer[i].pFreeNext = &m_pEntryBuffer[i + 1];
   }
   m_pEntryBuffer[NUM_TXCACHE_ENTRIES - 1].pFreeNext = NULL;
   m_rFreeList.pFreeNext = &m_pEntryBuffer[0];

   // Set up the ageList sentries...
   //
   m_rAgeHead.pAgeNext = &m_rAgeTail;
   m_rAgeTail.pAgePrev = &m_rAgeHead;
   m_rAgeHead.pAgePrev    =
      m_rAgeTail.pAgeNext = NULL;

   // Set up the hash table...
   //
   m_hashTable = new TextureCacheEntry*[sm_hashTableSize];
   memset(m_hashTable, 0, sizeof(TextureCacheEntry*) * sm_hashTableSize);

   // this sets our v_memory object up...
   //
   v_u32* pBase     = reinterpret_cast<v_u32*>(io_rTxCache.getBaseAddress());
   UInt32 cacheSize = io_rTxCache.getCacheSize();
   
   AssertFatal((UInt32(pBase) & 0x3) == 0, "error, cache not properly aligned!");
   m_cacheVMemory = m_pProxy->V_AllocMemoryObject(m_vHandle,
                                                  pBase, cacheSize);
   v_error vError =
      m_pProxy->V_LockPages(m_vHandle, m_cacheVMemory);
   AssertFatal(vError == V_SUCCESS, "Error, unable to lock cache pages.");
}

TextureCache::~TextureCache()
{
   // Flush out any textures on the card...
   //
   flushCache();

   // Destroy our translation surfaces...
   //
   if (m_pNormalSurface != NULL) {
      m_pProxy->VL_DestroySurface(m_vHandle, m_pNormalSurface);
      m_pNormalSurface = NULL;
   }
   
   if (m_pAlphaSurface != NULL) {
      m_pProxy->VL_DestroySurface(m_vHandle, m_pAlphaSurface);
      m_pAlphaSurface = NULL;
   }

   // And the buffers...
   //
   m_pProxy->V_FreeLockedMem(m_vHandle, m_normalMemory);
   m_pProxy->V_FreeLockedMem(m_vHandle, m_alphaMemory);
   m_pNormalTranslation   =
      m_pAlphaTranslation = NULL;

   v_error vError =
      m_pProxy->V_UnlockPages(m_vHandle, m_cacheVMemory);
   AssertFatal(vError == V_SUCCESS, "Error, unable to UNlock cache pages?");
   
   m_pProxy->V_FreeMemoryObject(m_vHandle, m_cacheVMemory);
   m_cacheVMemory = NULL;
   
   // Lose the hash table..
   //
   delete [] m_hashTable;
   m_hashTable = NULL;

   // set notes in c'tor
   //
   delete &m_rFreeList;
   delete &m_rAgeHead;
   delete &m_rAgeTail;
   
   delete [] m_pEntryBuffer;
   m_pEntryBuffer = NULL;
}


//------------------------------------------------------------------------------
//-------------------------------------- Translation texture setup...
namespace {

inline UInt16
munge565(const PALETTEENTRY& in_rColor)
{
   return ((UInt16(in_rColor.peRed   >> 3) << 11) |
           (UInt16(in_rColor.peGreen >> 2) << 5)  |
           (UInt16(in_rColor.peBlue  >> 3) << 0));

}

inline UInt16
munge4444(const PALETTEENTRY& in_rColor)
{
   return ((UInt16(in_rColor.peFlags >> 4) << 12) |
           (UInt16(in_rColor.peRed   >> 4) << 8)  |
           (UInt16(in_rColor.peGreen >> 4) << 4)  |
           (UInt16(in_rColor.peBlue  >> 4) << 0));
}

GRINLINE UInt16
hashKeys(const UInt32 in_key1,
         const UInt32 in_key2)
{
   UInt32 returnKey  = (in_key1 >> 16) ^ in_key1;
   returnKey        ^= (in_key2 >> 16) ^ in_key2;
   
   return UInt16(returnKey & 0xFFFF);
}

}; // namespace {}


void 
TextureCache::setPalette(const GFXPalette* in_pPalette)
{
   // we need to create 256x1 textures on the card for
   //  the entries of the palette converted to 565 sixteen
   //  bit, and 4444 alpha...
   //
   if (m_pNormalTranslation == NULL) {
      m_normalMemory = m_pProxy->V_AllocLockedMem(m_vHandle,
                                                  sizeof(UInt16) * 256);
      m_pNormalTranslation =
         (UInt16*)m_pProxy->V_GetMemoryObjectAddress(m_normalMemory);
   }
   if (m_pAlphaTranslation == NULL) {
      m_alphaMemory = m_pProxy->V_AllocLockedMem(m_vHandle,
                                                 sizeof(UInt16) * 256);
      m_pAlphaTranslation =
         (UInt16*)m_pProxy->V_GetMemoryObjectAddress(m_alphaMemory);
   }

   for (int i = 0; i < 256; i++) {
      m_pNormalTranslation[i] = munge565(in_pPalette->color[i]);
      m_pAlphaTranslation[i]  = munge4444(in_pPalette->transColor[i]);
   }

   // Last color require to be black, first color required to be white...
   //  (alpha last excluded)
   //
   m_pAlphaTranslation[0]    = 0;
   m_pNormalTranslation[0]   = 0;
   m_pNormalTranslation[255] = 0xffff;
   
   // Now create textures out of these buffers, and download them to the
   //  card.  Note that to ensure success, we flush the texture cache, since
   //  all textures that have been downloaded with previous palettes are
   //  invalid...
   //
   flushCache();
   
   vl_error vlError;
   if (m_pNormalSurface == NULL) {
      vlError = m_pProxy->VL_CreateSurface(m_vHandle,
                                           &m_pNormalSurface,
                                           0, 1,               // no attributes, 1 buffer
                                           V_PIXFMT_565,       // normal 16 bit
                                           256, 1);            // LUtex aspect
      AssertFatal(vlError == VL_SUCCESS, "Error creating LUTexture");
   }
   if (m_pAlphaSurface == NULL) {
      vlError = m_pProxy->VL_CreateSurface(m_vHandle,
                                           &m_pAlphaSurface,
                                           0, 1,               // no attributes, 1 buffer
                                           V_PIXFMT_4444,      // alpha 16 bit
                                           256, 1);            // LUtex aspect
      AssertFatal(vlError == VL_SUCCESS, "Error creating LUTexture");
   }
   
   // Load the textures with our translation data...
   //
   vlError = m_pProxy->VL_LoadBuffer(m_pSurface->getVCmdBuffer(),
                                     m_pNormalSurface, 0,   //  sfc/buffer
                                     sizeof(UInt16)*256,    // data linebytes
                                     256, 1,
                                     m_normalMemory, NULL);
   AssertFatal(vlError == VL_SUCCESS, "Error loading LUTexture");
   vlError = m_pProxy->VL_LoadBuffer(m_pSurface->getVCmdBuffer(),
                                     m_pAlphaSurface, 0,   //  sfc/buffer
                                     sizeof(UInt16)*256,    // data linebytes
                                     256, 1,
                                     m_alphaMemory, NULL);
   AssertFatal(vlError == VL_SUCCESS, "Error loading LUTexture");
}


//------------------------------------------------------------------------------
//-------------------------------------- Texture Cache Functionality
GRINLINE void 
TextureCache::placeHashEntry(TextureCacheEntry* io_pPlace)
{
   UInt16 hashKey = UInt16(hashKeys(io_pPlace->key1, io_pPlace->key2) %
                           sm_hashTableSize);
   
   io_pPlace->pHashNext = m_hashTable[hashKey];
   m_hashTable[hashKey] = io_pPlace;
}

GRINLINE void 
TextureCache::removeHashEntry(TextureCacheEntry* io_pRemove)
{
   UInt16 hashKey = UInt16(hashKeys(io_pRemove->key1, io_pRemove->key2) %
                           sm_hashTableSize);
   
   TextureCacheEntry* pEntry = m_hashTable[hashKey];
   
   // Since m_hashTable is an array of pointers rather than sentries, we must
   //  check for the special case...
   //
   if (pEntry == io_pRemove) {
      m_hashTable[hashKey]  = pEntry->pHashNext;
      io_pRemove->pHashNext = NULL;
      return;
   }

   while (pEntry->pHashNext != NULL) {
      if (pEntry->pHashNext == io_pRemove) {
         pEntry->pHashNext     = pEntry->pHashNext->pHashNext;
         io_pRemove->pHashNext = NULL;
         return;
      }

      pEntry = pEntry->pHashNext;
   }
   
   AssertWarn(0, "Entry not found in expected hash list...");
}


GRINLINE bool 
TextureCache::texturesRemainInCache() const
{
   return (m_rAgeHead.pAgeNext != &m_rAgeTail);
}


GRINLINE void 
TextureCache::unloadOldestEntry()
{
   if (texturesRemainInCache() == false) {
      AssertWarn(0, "tried to remove oldest texture from empty cache...");
      return;
   }

   unloadEntry(m_rAgeHead.pAgeNext);
}

void 
TextureCache::flushCache()
{
   while (texturesRemainInCache())
      unloadOldestEntry();
}


GRINLINE void 
TextureCache::removeFromCache(TextureCacheEntry* io_pRemove)
{
   AssertFatal((io_pRemove->pAgeNext != NULL &&
                io_pRemove->pAgePrev != NULL),
               "Error, TCE isn't ON cache");
   
   io_pRemove->pAgePrev->pAgeNext = io_pRemove->pAgeNext;
   io_pRemove->pAgeNext->pAgePrev = io_pRemove->pAgePrev;
   
   io_pRemove->pAgeNext    =
      io_pRemove->pAgePrev = NULL;
   
   removeHashEntry(io_pRemove);
}

GRINLINE void 
TextureCache::placeInCache(TextureCacheEntry* io_pPlace)
{
   AssertFatal((io_pPlace->pAgeNext == NULL && io_pPlace->pAgePrev == NULL),
               "Error, TCE is ALREADY on cache");
   AssertFatal(io_pPlace->pFreeNext == NULL,
               "Error, TCE still on free list...");

   // place at tail...
   //
   io_pPlace->pAgePrev = m_rAgeTail.pAgePrev;
   io_pPlace->pAgeNext = &m_rAgeTail;

   io_pPlace->pAgePrev->pAgeNext = io_pPlace;
   m_rAgeTail.pAgePrev           = io_pPlace;
   
   m_currentEntry = io_pPlace;
   
   placeHashEntry(io_pPlace);
}


GRINLINE void 
TextureCache::touchEntry(TextureCacheEntry* io_pTouch)
{
   // Code is replicated here from place and remove..Cache to avoid affecting
   //  the hash entries...
   //
   io_pTouch->pAgePrev->pAgeNext = io_pTouch->pAgeNext;
   io_pTouch->pAgeNext->pAgePrev = io_pTouch->pAgePrev;
   
   io_pTouch->pAgePrev = m_rAgeTail.pAgePrev;
   io_pTouch->pAgeNext = &m_rAgeTail;

   io_pTouch->pAgePrev->pAgeNext = io_pTouch;
   m_rAgeTail.pAgePrev           = io_pTouch;
   
   m_currentEntry = io_pTouch;
}


GRINLINE void 
TextureCache::placeOnFreeList(TextureCacheEntry* io_pFree)
{
   AssertFatal((io_pFree->pAgeNext  == NULL &&
                io_pFree->pAgePrev  == NULL &&
                io_pFree->pHashNext == NULL),
               "Error, TCE is still on cache");
   AssertFatal(io_pFree->pFreeNext == NULL, "Error, already on free list...");
   
   io_pFree->pFreeNext   = m_rFreeList.pFreeNext;
   m_rFreeList.pFreeNext = io_pFree;
}


TextureCacheEntry* 
TextureCache::getFreeEntry()
{
   if (m_rFreeList.pFreeNext != NULL) {
      TextureCacheEntry* pReturn = m_rFreeList.pFreeNext;
      m_rFreeList.pFreeNext = pReturn->pFreeNext;
      pReturn->pFreeNext = NULL;
      return pReturn;
   } else {
      AssertWarn(0, "Ran out of free TCEs, increase size in class definition.");
      unloadOldestEntry();
      return getFreeEntry();
   }
}

void 
TextureCache::unloadEntry(TextureCacheEntry* io_pUnload)
{
   removeFromCache(io_pUnload);
   
   m_pProxy->VL_DestroySurface(m_vHandle, io_pUnload->pTxSurf);
   io_pUnload->pTxSurf = NULL;
   
   placeOnFreeList(io_pUnload);
}

TextureCacheEntry* 
TextureCache::findInCache(const UInt32 in_key1,
                          const UInt32 in_key2)
{
   UInt16 hashKey = UInt16(hashKeys(in_key1, in_key2) %
                           sm_hashTableSize);
   
   TextureCacheEntry* pEntry = m_hashTable[hashKey];
   while (pEntry != NULL) {
      if (pEntry->key1 == in_key1 &&
          pEntry->key2 == in_key2)
         return pEntry;
      pEntry = pEntry->pHashNext;
   }

   // Not found...
   return NULL;
}

bool 
TextureCache::setTexture(const GFXTextureCacheEntry* in_tce)
{
   AssertFatal(in_tce != NULL, "Error, null texture cache entry...");

   TextureCacheEntry* pEntry = findInCache(UInt32(in_tce->dataPtr),
                                           UInt32((in_tce->mipLevel << 28) ^
                                                  in_tce->wrapCount));
   if (pEntry != NULL)
      touchEntry(pEntry);

   return (pEntry != NULL);
}


bool 
TextureCache::setTexture(const BYTE* in_pTexture)
{
   // Solid color poly case...
   //
   if (in_pTexture == NULL) {
      m_currentEntry = NULL;
      return true;
   }

   TextureCacheEntry* pEntry = findInCache(UInt32(in_pTexture), 0UL);

   if (pEntry != NULL)
      touchEntry(pEntry);

   return (pEntry != NULL);
}


v_surface* 
TextureCache::createRenditionSurface(const UInt32 in_stride,
                                     const UInt32 in_height,
                                     const v_u32  in_format)
{
   AssertFatal((in_stride & 0x3) == 0, "Error, not 4 pixel padded...");

   v_surface* retSurf = NULL;
   vl_error vlError;
   
   // For some bizarre reason, there are some height 0
   //  bitmaps coming through... DMMNOTE: BOGUS, this is actually a bug that needs
   //  fixing in gfx or itr.
   //
   UInt32 height;
   UInt32 stride;
   if (in_height == 0) height = 1;
   else                height = in_height;
   
   if (in_stride < 4) stride = 4;
   else               stride = in_stride;
   
   
   AssertFatal(height > 0 && height <= 256, "Error, bogus height...");
   
   while (true) {
      vlError = m_pProxy->VL_CreateSurface(m_vHandle,
                                           &retSurf,
                                           0, 1,
                                           in_format,
                                           stride, height);
      if (vlError == VL_SUCCESS) {
         break;
      } else if (vlError == VL_OUT_OF_VIDEO_MEMORY) {
         if (texturesRemainInCache() == true)
            unloadOldestEntry();
         else
            AssertFatal(0, "Huh? can't get texture into cache...");
      } else {
         AssertFatal(0, "Unrecoverable error loading texture");
      }
   }

   AssertFatal(retSurf != NULL, "Error, something went wrong...");
   return retSurf;
}

void
padBitmap(const BYTE*  in_pBits,
          const UInt32 in_bmpStride,
          const UInt32 in_height,
          BYTE*        out_pBits,
          const UInt32 in_dstStride)
{
   AssertFatal((in_dstStride & 0x3) == 0, "Error, not properly padded...");
   
   const BYTE* pSrcBits = in_pBits;
   BYTE* pDstBits       = out_pBits;
   for (UInt32 y = 0; y < in_height; y++) {
      for (UInt32 x = 0; x < in_bmpStride; x++) {
         pDstBits[x] = pSrcBits[x];
      }
      pDstBits += in_dstStride;
      pSrcBits += in_bmpStride;
   }
}

TextureCacheEntry* 
TextureCache::primeEntry(BYTE*  io_pBits,
                         UInt32 in_stride,
                         UInt32 in_height,
                         v_u32  in_format)
{
   AssertFatal((UInt32(io_pBits) & 0x3) == 0, "Error, data not aligned!");

   UInt32 stride;
   BYTE* pBits;
   
   if ((in_stride & 0x3) != 0) {
      stride = (in_stride + 0x3) & ~0x3;
      pBits  = (BYTE*)m_gfxTxCache.allocateChunk(stride * in_height);
      padBitmap(io_pBits, in_stride, in_height, pBits, stride);
   } else {
      stride = in_stride;
      pBits  = io_pBits;
   }

   TextureCacheEntry* pEntry = getFreeEntry();
   pEntry->pTxSurf = createRenditionSurface(stride,
                                            in_height,
                                            in_format);
   pEntry->stride = UInt16(stride);
   pEntry->height = UInt16(in_height);
   
   v_surface* pTranslationSurface;
   if (in_format == V_PIXFMT_565)
      pTranslationSurface = m_pNormalSurface;
   else if (in_format == V_PIXFMT_4444)
      pTranslationSurface = m_pAlphaSurface;
   else
      AssertFatal(0, avar("Error, non-supported format: %d", in_format));
   AssertFatal(pTranslationSurface != NULL, "Error, no palette set...");

   // Set up translation render state...
   //
   m_pProxy->VL_InstallTextureMapBasic(m_pSurface->getVCmdBuffer(),
                                       pTranslationSurface);
   m_pProxy->VL_InstallDstBuffer(m_pSurface->getVCmdBuffer(),
                                 pEntry->pTxSurf);
   m_pSurface->setFilterEnable(false);
   m_pSurface->SetSrcFunc(V_SRCFUNC_REPLACE);
   m_pProxy->VL_SetSrcFmt(m_pSurface->getVCmdBuffer(),in_format);
   m_pProxy->VL_SetDstFmt(m_pSurface->getVCmdBuffer(), in_format);
   m_pSurface->SetChromaKey(V_CHROMAKEY_DISABLE);
   m_pSurface->SetDitherEnable(V_DITHER_DISABLE);
   m_pSurface->SetBlendEnable(V_BLEND_DISABLE);
   m_pSurface->SetFogEnable(V_FOG_DISABLE);

   // translation...
   //
   m_pProxy->VL_Lookup(m_pSurface->getVCmdBuffer(),
                       0, 0,
                       UInt16(in_stride), UInt16(in_height),
                       m_cacheVMemory,
                       (v_u32*)pBits);

   // Reset destination state...
   //
   m_pProxy->VL_InstallDstBuffer(m_pSurface->getVCmdBuffer(),
                                 m_pSurface->getVSurface());
   m_pProxy->VL_SetDstFmt(m_pSurface->getVCmdBuffer(), V_PIXFMT_565);

   
   m_pSurface->cmdBuffOverflow(m_pSurface->m_vHandle,
                               m_pSurface->m_pCurrentBuffer);


   return pEntry;
}
                         

void 
TextureCache::downloadTexture(const GFXTextureCacheEntry* in_tce,
                              const bool                  in_enableAlpha,
                              const bool                  in_enableTransp)
{
   v_u32 pixFmt = (in_enableAlpha == false) ? V_PIXFMT_565 :
                                              V_PIXFMT_4444;
   TextureCacheEntry* pEntry = primeEntry(in_tce->dataPtr,
                                          in_tce->csizeX,
                                          in_tce->csizeY,
                                          pixFmt);
   if (in_enableTransp) {
      UInt16 chromaColor = munge565(m_pSurface->getPalette()->color[0]);
      
      m_pProxy->VL_SetSurfaceChromaColor(pEntry->pTxSurf, chromaColor);
      m_pProxy->VL_SetSurfaceChromaMask(pEntry->pTxSurf, 0xFFFF);
      m_pProxy->VL_SetSurfaceChromaKey(pEntry->pTxSurf, V_CHROMAKEY_ENABLE);
   }

   pEntry->key1 = UInt32(in_tce->dataPtr);
   pEntry->key2 = UInt32((in_tce->mipLevel << 28) ^ in_tce->wrapCount);
   
   placeInCache(pEntry);
}


void 
TextureCache::downloadTexture(BYTE*        in_pTexture,
                              const UInt32 in_stride,
                              const UInt32 in_height,
                              const bool   in_enableAlpha,
                              const bool   in_enableTransp)
{
   // We need to place the bitmap into the texture cache...
   //
   BYTE* pDump = (BYTE*)m_gfxTxCache.allocateChunk(in_stride * in_height);
   AssertFatal((UInt32(pDump) & 0x3) == 0, "Error, unaligned pointer returned");
   
   // Copy the bitmap into the cache...
   //
   memcpy(pDump, in_pTexture, in_stride * in_height);

   v_u32 pixFmt = (in_enableAlpha == false) ? V_PIXFMT_565 :
                                              V_PIXFMT_4444;
   TextureCacheEntry* pEntry = primeEntry(pDump,
                                          in_stride,
                                          in_height,
                                          pixFmt);
   if (in_enableTransp) {
      m_pProxy->VL_SetSurfaceChromaColor(pEntry->pTxSurf, 0);
      m_pProxy->VL_SetSurfaceChromaMask(pEntry->pTxSurf, 0xFFFFFFFF);
      m_pProxy->VL_SetSurfaceChromaKey(pEntry->pTxSurf, V_CHROMAKEY_ENABLE);
   }

   pEntry->key1 = UInt32(in_pTexture);
   pEntry->key2 = UInt32(0);           // bitmaps have zero wrapcount && mipLevel

   placeInCache(pEntry);
}


void 
TextureCache::primeForDraw()
{
   TextureCacheEntry* pEntry = m_currentEntry;
   
   if (pEntry != NULL) {
      // textured spans...
      //
      vl_error vlError =
         m_pProxy->VL_InstallTextureMap(m_pSurface->getVCmdBuffer(),
                                        pEntry->pTxSurf);
      AssertFatal(vlError == VL_SUCCESS,
                  avar("Error installing texture: %p", pEntry->pTxSurf));

      m_pSurface->setFilterEnable(true);
   }
}


void 
TextureCache::primeForBitmap(const bool in_transparent,
                             const bool /*in_translucent*/)
{
   AssertFatal(m_currentEntry != NULL, "Error, no curent bitmap...");

   TextureCacheEntry* pEntry = m_currentEntry;
   
   vl_error vlError =
      m_pProxy->VL_InstallTextureMap(m_pSurface->getVCmdBuffer(),
                                     pEntry->pTxSurf);
   AssertFatal(vlError == VL_SUCCESS,
               avar("Error installing texture: %p", pEntry->pTxSurf));
   
   m_pSurface->SetSrcFunc(V_SRCFUNC_REPLACE);
   m_pSurface->setFilterEnable(false);
   m_pSurface->SetFGColorARGB(0xFFFFFFFF);
   m_pSurface->SetFogEnable(V_FOG_DISABLE);
   
   if (in_transparent == true)
      m_pSurface->SetChromaKey(V_CHROMAKEY_ENABLE);
   else
      m_pSurface->SetChromaKey(V_CHROMAKEY_DISABLE);
      
}

void
TextureCache::flushTexture(const GFXBitmap* in_pBitmap,
                           const bool       in_reload)
{
   in_pBitmap; in_reload;
//   
//   TextureCacheEntry* pEntry = findInCache(UInt32(in_pBitmap), 0);
//   
//   if (pEntry == NULL) {
//      // Texture no longer in cache, return
//      return;
//   }
//   
//   unloadEntry(pEntry);
//
//
//   UInt32 stride;
//   BYTE*  pBits  = in_pBitmap->getAddress(0, 0);
//   
//   if ((in_pBitmap->getStride() & 0x3) != 0) {
//      stride = (in_pBitmap->getStride() + 0x3) & ~0x3;
//      pBits  = (BYTE*)m_gfxTxCache.allocateChunk(stride * in_pBitmap->getHeight());
//      padBitmap(in_pBitmap->getAddress(0, 0),
//                in_pBitmap->getStride(),
//                in_pBitmap->getHeight(),
//                pBits, stride);
//   }
//
//   v_surface* pTranslationSurface;
//   if (pEntry->pTxSurf->pixel_fmt == V_PIXFMT_565)
//      pTranslationSurface = m_pNormalSurface;
//   else if (pEntry->pTxSurf->pixel_fmt == V_PIXFMT_4444)
//      pTranslationSurface = m_pAlphaSurface;
//   else
//      AssertFatal(0, avar("Error, non-supported format: %d", pEntry->pTxSurf->pixel_fmt));
//   AssertFatal(pTranslationSurface != NULL, "Error, no palette set...");
//
//   // Set up translation render state...
//   //
//   m_pProxy->VL_InstallTextureMapBasic(m_pSurface->getVCmdBuffer(),
//                                       pTranslationSurface);
//   m_pProxy->VL_InstallDstBuffer(m_pSurface->getVCmdBuffer(),
//                                 pEntry->pTxSurf);
//   m_pSurface->setFilterEnable(false);
//   m_pSurface->SetSrcFunc(V_SRCFUNC_REPLACE);
//   m_pProxy->VL_SetSrcFmt(m_pSurface->getVCmdBuffer(), pEntry->pTxSurf->pixel_fmt);
//   m_pProxy->VL_SetDstFmt(m_pSurface->getVCmdBuffer(), pEntry->pTxSurf->pixel_fmt);
//   m_pSurface->SetChromaKey(V_CHROMAKEY_DISABLE);
//   m_pSurface->SetDitherEnable(V_DITHER_DISABLE);
//   m_pSurface->SetBlendEnable(V_BLEND_DISABLE);
//   m_pSurface->SetFogEnable(V_FOG_DISABLE);
//
//   // translation...
//   //
//   m_pProxy->VL_Lookup(m_pSurface->getVCmdBuffer(),
//                       0, 0,
//                       UInt16(in_pBitmap->getStride()), UInt16(in_pBitmap->getHeight()),
//                       m_cacheVMemory,
//                       (v_u32*)pBits);
//
//   // Reset destination state...
//   //
//   m_pProxy->VL_InstallDstBuffer(m_pSurface->getVCmdBuffer(),
//                                 m_pSurface->getVSurface());
//   m_pProxy->VL_SetDstFmt(m_pSurface->getVCmdBuffer(), V_PIXFMT_565);
//   
//   m_pSurface->cmdBuffOverflow(m_pSurface->m_vHandle,
//                               m_pSurface->m_pCurrentBuffer);
}

}; // namespace Rendition