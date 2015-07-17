//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gD3DTxCache.h"
#include "gD3DSfc.h"
#include "g_pal.h"
#include "gfxmetrics.h"


// We want to be able to step thru utility functions in debug mode...
//
#ifdef DEBUG
#define GFXD3DINLINE
#else
#define GFXD3DINLINE inline
#endif

// Imported to get error strings...
//
char *D3DAppErrorToString(HRESULT);

namespace Direct3D {

// In gD3DFn.cpp
//
extern Int32 NearestPow(Int32);

//------------------------------------------------------------------------------
//-------------------------------------- TextureCache
//

TextureCache::TextureCache(const Int32        in_numEntries,
                           TextureTranslator* in_pTexTranslator)
 : frameU(1),
   numCacheEntries(in_numEntries),
   pTexTranslator(in_pTexTranslator),
   pCurrentTexture(NULL),
   pPalette(NULL),
   pDDPalette(NULL)
{
   Int32 i;

   // Set up the free list
   //
   allocArray = new TextureCacheEntry[numCacheEntries];
   memset(allocArray, 0, sizeof(TextureCacheEntry) * numCacheEntries);

   for (i = 0; i < numCacheEntries; i++) {
      allocArray[i].freeLink = &allocArray[i + 1];
   }
   allocArray[numCacheEntries - 1].freeLink = NULL;
   freeList.freeLink = allocArray;

   ageList.agePrev = NULL;
   ageList.ageNext = &ageTail;
   ageTail.agePrev = &ageList;
   ageTail.ageNext = NULL;

   // Hash table
   //
   hashTable = new TextureCacheEntry*[hashTableSize];
   memset(hashTable, 0, sizeof(TextureCacheEntry*) * hashTableSize);

   // Allocate buffer for downloading callback textures... (256x256 largest
   //  allowed size...)
   //
   pCallBackBuffer = new UInt8[256*256];
   
   for (i = 0; i < 4; i++) {
      for (int j = 0; j < sm_numBucketEntries; j++) {
         BucketSurface& rWorking = m_sysmemCache[i][j];
         
         rWorking.widthHeight = 0x80000000;
         rWorking.lastUsed    = -1;
         rWorking.pSurface = NULL;
         rWorking.pTexture = NULL;
      }
   }

   m_bucketTime = 0;
}



TextureCache::~TextureCache()
{
   for (int i = 0; i < 4; i++) {
      for (int j = 0; j < sm_numBucketEntries; j++) {
         BucketSurface& rWorking = m_sysmemCache[i][j];
         
         if (rWorking.pTexture != NULL) rWorking.pTexture->Release();
         if (rWorking.pSurface != NULL) rWorking.pSurface->Release();
      }
   }

   flush();
   delete [] allocArray;

   freeList.freeLink = NULL;
   ageList.ageNext   = NULL;
   pCurrentTexture   = NULL;

   memset(hashTable, 0, sizeof(TextureCacheEntry*) * hashTableSize);
   delete [] hashTable;

   if (pDDPalette) {
      pDDPalette->Release();
      pDDPalette = NULL;
   }
   pPalette = NULL;

   if (pTexTranslator != NULL)
      delete pTexTranslator;
   pTexTranslator = NULL;

   delete [] pCallBackBuffer;
   pCallBackBuffer = NULL;
}



//------------------------------------------------------------------------------
// NAME 
//    GFXD3DINLINE bool 
//    TextureCache::texturesRemainingInCache()
//    
// DESCRIPTION 
//    Simply returns true if anything is on the cache...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
GFXD3DINLINE bool 
TextureCache::texturesRemainingInCache() const
{
   return (ageList.ageNext != &ageTail);

}


//------------------------------------------------------------------------------
// NAME 
//    GFXD3DINLINE TextureCacheEntry* 
//    getFreeEntry()
//    
// DESCRIPTION 
//    Returns a free cache entry.
//    
// NOTES 
//    Does NOT place the entry on the cacheList.
//------------------------------------------------------------------------------
GFXD3DINLINE TextureCacheEntry* 
TextureCache::getFreeEntry()
{
   TextureCacheEntry* pRetEntry = freeList.freeLink;

   // Make sure we actually got one, unloading oldest texture if not...
   //
   if (pRetEntry == NULL) {
      AssertFatal(texturesRemainingInCache() == true, "Serious goof-up!");
      AssertWarn(0, "Warning, removing texture from cache in getFreeEntry");
      unloadOldestTexture();
      pRetEntry = freeList.freeLink;
      AssertFatal(pRetEntry != NULL,
                  "Error, unable to get entry after unloading, probable error");
   }

   freeList.freeLink   = pRetEntry->freeLink;
   pRetEntry->freeLink = NULL;

   return pRetEntry;
}


//------------------------------------------------------------------------------
// NAME 
//    GFXD3DINLINE void 
//    TextureCache::releaseEntry(TextureCacheEntry* io_pTrash)
//    
// DESCRIPTION 
//    Places an entry on the free list...
//    
// NOTES 
//    Assumes that there are no d3d structures attached to the entry...
//------------------------------------------------------------------------------
GFXD3DINLINE void 
TextureCache::releaseEntry(TextureCacheEntry* io_pTrash)
{
   AssertFatal(io_pTrash != NULL, "Error, no trash to take out!");
   AssertFatal(io_pTrash->pSurface     == NULL, "Error, unreleased entry...");
   AssertFatal(io_pTrash->pTexture     == NULL, "Error, unreleased entry...");
   AssertFatal(io_pTrash->d3dTexHandle == NULL, "Error, unreleased entry...");
   AssertFatal(io_pTrash->cacheNext == NULL && io_pTrash->cachePrev == NULL,
               "Error, entry still on the cache");
   AssertFatal(io_pTrash->ageNext == NULL && io_pTrash->agePrev == NULL,
               "Error, entry still on age list...");

   io_pTrash->freeLink = freeList.freeLink;
   freeList.freeLink   = io_pTrash;
}


//------------------------------------------------------------------------------
// NAME 
//    GFXD3DINLINE void 
//    TextureCache::placeInCache(TextureCacheEntry* io_pEntry)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
GFXD3DINLINE UInt16 
TextureCache::hashKey(const DWORD in_key) const
{
   DWORD modKey = in_key;
   return UInt16(((modKey >> 16) ^ (modKey & 0xffff)) % hashTableSize);
}


GFXD3DINLINE UInt16 
TextureCache::hashEntry(const TextureCacheEntry* in_pEntry) const
{
   return hashKey(in_pEntry->key);
}


GFXD3DINLINE void 
TextureCache::placeInCache(TextureCacheEntry* io_pEntry)
{
   AssertFatal(io_pEntry != NULL, "Error, cannot make null entry current");
   AssertFatal(io_pEntry->cacheNext == NULL && io_pEntry->cachePrev == NULL,
               "Error, entry is in the cache...");
   AssertFatal(io_pEntry->ageNext == NULL && io_pEntry->agePrev == NULL,
               "Error, entry on ageList...");
   DWORD hashKey = hashEntry(io_pEntry);
   
   // Place in the proper hash bin...
   //
   if (hashTable[hashKey] != NULL) {
      hashTable[hashKey]->cachePrev = io_pEntry;
   }
   io_pEntry->cacheNext = hashTable[hashKey];
   hashTable[hashKey] = io_pEntry;

   // Place on the ageList...
   //
   if (ageList.ageNext != NULL)
      ageList.ageNext->agePrev = io_pEntry;
      
   io_pEntry->ageNext = ageList.ageNext;
   io_pEntry->agePrev = &ageList;
   ageList.ageNext = io_pEntry;
}


//------------------------------------------------------------------------------
// NAME 
//    GFXD3DINLINE void 
//    TextureCache::removeFromCache(TextureCacheEntry* io_pRemove)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
GFXD3DINLINE void 
TextureCache::removeFromCache(TextureCacheEntry* io_pRemove)
{
   AssertFatal(io_pRemove != NULL, "Error, cannot remove null entry");

   DWORD hashKey = hashEntry(io_pRemove);

   // Remove from hash table...
   //
   if (io_pRemove->cacheNext != NULL) {
      io_pRemove->cacheNext->cachePrev = io_pRemove->cachePrev;
   }
   if (io_pRemove->cachePrev != NULL) {
      // Entry is not first on hash list...
      //
      io_pRemove->cachePrev->cacheNext = io_pRemove->cacheNext;
   } else {
      // entry is first on hash list
      //
      hashTable[hashKey] = io_pRemove->cacheNext;
   }
   
   io_pRemove->cacheNext = NULL;
   io_pRemove->cachePrev = NULL;

   // Remove from ageList (these pointers are guaranteed to be non-NULL because
   //  of the head and tail sentries...
   //
   io_pRemove->ageNext->agePrev = io_pRemove->agePrev;
   io_pRemove->agePrev->ageNext = io_pRemove->ageNext;

   io_pRemove->agePrev = io_pRemove->ageNext = NULL;
}


//------------------------------------------------------------------------------
// NAME 
//    GFXD3DINLINE void 
//    TextureCache::makeCurrentEntry(TextureCacheEntry* io_pCurrent)
//    
// DESCRIPTION 
//    obv.
//    
// NOTES 
//    
//------------------------------------------------------------------------------
GFXD3DINLINE void 
TextureCache::makeCurrentEntry(TextureCacheEntry* io_pCurrent)
{
   AssertFatal(io_pCurrent != NULL, "Error, cannot make null entry current");
   pCurrentTexture = io_pCurrent;
}


//------------------------------------------------------------------------------
// NAME 
//    void TextureCache::unloadEntry(TextureCacheEntry* io_pTex)
//    
// DESCRIPTION 
//    Simply offloads from the card and releases the associated interfaces
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureCache::unloadEntry(TextureCacheEntry* io_pEntry)
{
   AssertFatal(io_pEntry != NULL, "Error, null textureCacheEntry");
   AssertFatal(io_pEntry->pSurface != NULL, "Huh?");
   AssertFatal(io_pEntry->pTexture != NULL, "Huh?");
   AssertWarn(io_pEntry->pCreator, "Odd, no creator...");

   // Release Direct3D Structures, and unload the texture
   //
   io_pEntry->d3dTexHandle = NULL;

   io_pEntry->pTexture->Release();
   io_pEntry->pTexture = NULL;

   io_pEntry->pSurface->Release();
   io_pEntry->pSurface = NULL;
   GFXMetrics.incReleasedTextures();

   removeFromCache(io_pEntry);

   // Key and Creator info
   io_pEntry->key      = DWORD(0);
   io_pEntry->pCreator = NULL;

   // Move to free list
   releaseEntry(io_pEntry);
}


//------------------------------------------------------------------------------
// NAME 
//    void TextureCache::setPalette(GFXPalette* pGFXPalette,
//                                          IDirectDrawPalette* pDDPalette)
//    
// DESCRIPTION 
//    Does what it says...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureCache::setPalette(GFXPalette*         io_pGFXPalette,
                         IDirectDrawPalette* io_pDDPalette)
{
   // We don't own our GFXPalette, so overwrite the pointer
   //
   pPalette = io_pGFXPalette;

   // We DO however, add a reference to our directdraw palette, so if we
   //  already own one, release it before setting the member.  Increment
   //  the new DDPalette's reference count;
   //
   if (pDDPalette != NULL)
      pDDPalette->Release();
   pDDPalette = io_pDDPalette;
   pDDPalette->AddRef();

   // Let the TextureMungers know that the palette has changed...
   //
   pTexTranslator->setPalette(io_pGFXPalette);
}


//------------------------------------------------------------------------------
// NAME 
//    bool 
//    TextureCache::getD3DTexture(DDSURFACEDESC*        in_pDDSD,
//                                IDirectDrawSurface2** out_ppDDSurface2,
//                                IDirect3DTexture2**   out_ppD3DTexture2)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool 
TextureCache::getD3DTexture(const Surface*        in_pSurface,
                            DDSURFACEDESC*        in_pDDSD,
                            IDirectDrawSurface2** out_ppDDSurface2,
                            IDirect3DTexture2**   out_ppD3DTexture2,
                            const bool            in_unloadIfFailed)
{
   IDirectDrawSurface* pTempDDSurface = NULL;
   while (true) {
      // Get the IDirectDrawSurface & IDirectDrawSurface2 Interfaces...
      //
      HRESULT result;
      while (true) {
         result = (Surface::pDD2Vector[in_pSurface->deviceMinorId])->CreateSurface(in_pDDSD,
                                                                                   &pTempDDSurface,
                                                                                   NULL);
         if (result == DD_OK) {
            // surface was created ok,
            //
            GFXMetrics.incCreatedTextures();
            break;
         } else if (in_unloadIfFailed == true && texturesRemainingInCache() == true) {
            unloadOldestTexture();
         } else {
            AssertFatal(0, avar("Unable to create texture surface.  UnloadifFailed: %s.  Error: %s",
                                (in_unloadIfFailed == true) ? "true" : "false",
                                D3DAppErrorToString(result)));
         }
      }
      if (result != DD_OK) break;
      
      result = pTempDDSurface->QueryInterface(IID_IDirectDrawSurface2,
                                              (LPVOID*)out_ppDDSurface2);
      AssertWarn(result == S_OK, "Error creating surface2");
      if (result != DD_OK) break;

      // we can get rid of the IDirectDrawSurface once we have idds2
      //
      pTempDDSurface->Release();
      pTempDDSurface = NULL;

      // Get the texture interface...
      //
      result = (*out_ppDDSurface2)->QueryInterface(IID_IDirect3DTexture2,
                                                   (LPVOID*)out_ppD3DTexture2);
      AssertWarn(result == S_OK, "Error getting texture interface");
      if (result != DD_OK) break;

      return true;
   }
   
   if (pTempDDSurface != NULL)
      pTempDDSurface->Release();
   
   if ((*out_ppDDSurface2) != NULL) {
      (*out_ppDDSurface2)->Release();
      (*out_ppDDSurface2) = NULL;
   }

   if ((*out_ppD3DTexture2) != NULL) {
      (*out_ppD3DTexture2)->Release();
      (*out_ppD3DTexture2) = NULL;
   }

   return false;
}


//------------------------------------------------------------------------------
// NAME 
//    bool 
//    TextureCache::getD3DTexture(DDSURFACEDESC*        in_pDDSD,
//                                IDirectDrawSurface2** out_ppDDSurface2,
//                                IDirect3DTexture2**   out_ppD3DTexture2)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
int TextureCache::sm_numBucketEntries = 256;

bool 
TextureCache::getBucketTexture(const Surface*        in_pSurface,
                               Munger*               io_pMunger,
                               DDSURFACEDESC*        in_pDDSD,
                               IDirectDrawSurface2** out_ppDDSurface2,
                               IDirect3DTexture2**   out_ppD3DTexture2,
                               int                   in_type)
{
   Int32 height      = in_pDDSD->dwHeight;
   Int32 width       = in_pDDSD->dwWidth;
   Int32 mipLevels   = in_pDDSD->dwMipMapCount;
   AssertFatal(width <= 256 && height <= 256, "Out of range size...");

   Int32 wHMip = (mipLevels << 16) | (width << 8) | height;
   
   Int32 leastTime  = 0x7fffffff;
   Int32 leastIndex = -1;
   
   for (int i = 0; i < sm_numBucketEntries; i++) {
      if (m_sysmemCache[in_type][i].widthHeight == wHMip) {
         m_sysmemCache[in_type][i].lastUsed = m_bucketTime++;

         *out_ppDDSurface2  = m_sysmemCache[in_type][i].pSurface;
         *out_ppD3DTexture2 = m_sysmemCache[in_type][i].pTexture;
         
         return true;
      }
      
      if (m_sysmemCache[in_type][i].lastUsed < leastTime) {
         leastTime  = m_sysmemCache[in_type][i].lastUsed;
         leastIndex = i;
      }
   }

   // If we made it here, then we need to toss out the least recently used
   //  entry
   //
   AssertFatal(leastIndex != -1, "impossible");
   BucketSurface& rWorking = m_sysmemCache[in_type][leastIndex];
   if (rWorking.pTexture != NULL) rWorking.pTexture->Release();
   if (rWorking.pSurface != NULL) rWorking.pSurface->Release();
   GFXMetrics.incReleasedTextures();

   // Now create the new surface...
   //
   bool success = getD3DTexture(in_pSurface, in_pDDSD,
                                &rWorking.pSurface,
                                &rWorking.pTexture,
                                false);
   AssertFatal(success == true, "error creating bucket surface...");

   // If necessary, attach the palette to the texture...
   //
   if (io_pMunger->attachPalette() == true) {
      AssertFatal(pDDPalette != NULL, "Error, no palette to attach...");
      rWorking.pSurface->SetPalette(pDDPalette);
   }

   if (io_pMunger->setColorKey() == true) {
      DDCOLORKEY ddCKey;
      ddCKey.dwColorSpaceLowValue  = 0;
      ddCKey.dwColorSpaceHighValue = 0;

      rWorking.pSurface->SetColorKey(DDCKEY_SRCBLT, &ddCKey);
   }

   rWorking.lastUsed    = m_bucketTime++;
   rWorking.widthHeight = wHMip;
   
   *out_ppDDSurface2  = rWorking.pSurface;
   *out_ppD3DTexture2 = rWorking.pTexture;

   return success;
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    TextureCache::unloadOldestTexture()
//    
// DESCRIPTION 
//
// NOTES 
//------------------------------------------------------------------------------
void 
TextureCache::unloadOldestTexture()
{
   AssertWarn(texturesRemainingInCache() == true,
              "Warning, attempted to unload texture from empty cache...");
   if (texturesRemainingInCache() == false)
      return;

   // Get Oldest pointer at the back of the list...
   //
   TextureCacheEntry* pEntry = ageTail.agePrev;
   unloadEntry(pEntry);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    TextureCache::_bmDownload(GFXSurface*    io_pSurface,
//                              GFXBitmap*     io_pTexture,
//                              const DWORD    in_key,
//                              TextureMunger* io_pMunger)
//    
// DESCRIPTION 
//    Downloads a bitmap, given the proper munger...
//    
// NOTES 
//    DMMNOTE: Needs some way of gracefully failing...
//    There are two possible places that a card may indicate that there is
//     no room on the card to accept a texture: first, when the texture surface
//     is created, and second, when ->Load() is called.  To handle this, both
//     locations have routines to unload textures from the card.  The second case
//     is handled below, at the Load() call.  The first is handled in
//     getD3DTexture, when the flag passed to this function is raised.
//------------------------------------------------------------------------------
void 
TextureCache::_bmDownload(GFXSurface*      io_pSurface,
                          const GFXBitmap* io_pTexture,
                          const DWORD      in_key,
                          TextureMunger*   io_pMunger)
{
   AssertFatal(io_pSurface != NULL, "Error, bad surface passed");
   AssertFatal(io_pTexture != NULL, "Error, bad bitmap passed");
   AssertFatal(io_pMunger  != NULL, "Error, no munger!");

   Surface* pSurface      = (Surface*)io_pSurface;
   Int32    deviceMinorId = pSurface->deviceMinorId;

   Int32 width     = io_pTexture->getWidth();
   Int32 height    = io_pTexture->getHeight();
   Int32 mipLevels = 1; //io_pTexture->detailLevels;

   // Resize to pow of 2 square...
   //
   width  = NearestPow(width);
   height = NearestPow(height);

   DDSURFACEDESC ddsd;
   io_pMunger->getSurfDesc(&ddsd);

   if (mipLevels != 1 &&
       Surface::D3DDevDescVector[deviceMinorId].canDoMipmaps == true) {
      // Set up the mipmap description
      //
      ddsd.dwFlags        |= DDSD_MIPMAPCOUNT;
      ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
      ddsd.dwMipMapCount   = mipLevels;
   }
   
   ddsd.dwFlags |= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   ddsd.dwHeight = height;
   ddsd.dwWidth  = width;

   // Create the DDSurface and D3DTexture
   //
   IDirectDrawSurface2* pDDSurface2Src = NULL;
   IDirect3DTexture2*   pD3DTextureSrc = NULL;

   // Get the system memory source texture...
   //
   ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY | DDSCAPS_ALLOCONLOAD);
   ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
   bool success = getBucketTexture(pSurface, io_pMunger, &ddsd,
                                   &pDDSurface2Src,
                                   &pD3DTextureSrc,
                                   io_pMunger->getTextureType());
   AssertFatal(success == true, "Error Getting Source texture, how to handle? DMM");

   // Get the video memory destination texture
   //
   TextureCacheEntry* pEntry = NULL;
   ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_SYSTEMMEMORY);
   ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY | DDSCAPS_ALLOCONLOAD;
   scavengeTextureEntry(pSurface, pEntry,
                        io_pMunger, &ddsd);

   // If necessary, attach the palette to the texture...
   //
   if (io_pMunger->attachPalette() == true) {
      AssertFatal(pDDPalette != NULL, "Error, no palette to attach...");
      pEntry->pSurface->SetPalette(pDDPalette);
   }

   if (io_pMunger->setColorKey() == true) {
      DDCOLORKEY ddCKey;
      ddCKey.dwColorSpaceLowValue  = 0;
      ddCKey.dwColorSpaceHighValue = 0;

      pEntry->pSurface->SetColorKey(DDCKEY_SRCBLT, &ddCKey);
   }

   // Call the texture translator on each of the mip levels for the source
   //  (sys mem) texture)
   //
   IDirectDrawSurface2* pMipLevel = pDDSurface2Src;
   DDSCAPS ddscaps;
   ddscaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY |
                    DDSCAPS_MIPMAP  | DDSCAPS_COMPLEX;
#ifdef DEBUG
   Int32 cardSize = 0;
#endif
   for (int i = 0; i < mipLevels; i++) {
      // Lock the surface, and translate the bitmap
      // - Wait for lock, and we will only write to the surface...
      DDSURFACEDESC ddsDesc;
      ddsDesc.dwSize = sizeof(DDSURFACEDESC);
      HRESULT result = pMipLevel->Lock(NULL, &ddsDesc, 0, NULL);
      AssertWarn(result == DD_OK, avar("Error locking surface: %s",
                                       D3DAppErrorToString(result)));
      if (result != DD_OK)
         break;

#ifdef DEBUG
      GFXMetrics.textureBytesDownloaded += (ddsDesc.dwHeight * ddsDesc.lPitch);
      cardSize += (ddsDesc.dwHeight * ddsDesc.lPitch);
#endif
      io_pMunger->translateBitmap(io_pTexture,        // texture
                                  i,                  // miplevel
                                  ddsDesc.lpSurface,  // surface ptr
                                  ddsDesc.lPitch);    // surface stride
      result = pMipLevel->Unlock(ddsDesc.lpSurface);
      AssertWarn(result == DD_OK, avar("Error UNLOCKING surface???: %s",
                                       D3DAppErrorToString(result)));

      // Set to the next mipmap.  The condition has the nice property that
      //  if the texture is not mipmaped, (mipLevels = 1), it won't try to
      //  extract the next miplevel surface
      //
      if ((i != (mipLevels - 1)) &&
          Surface::D3DDevDescVector[deviceMinorId].canDoMipmaps == true) {

         IDirectDrawSurface2* temp = NULL;
         HRESULT result = pMipLevel->GetAttachedSurface(&ddscaps, &temp);
         AssertWarn(result == DD_OK, "Error getting attached mipmap");
         if (result != DD_OK)
            break;

         pMipLevel = temp;
      }
   }

   // At this point, all miplevels of the texture should be translated into the
   //  system memory source texture.  Call Load() to blit from the source
   //  texture to the destination texture on the card, unloading old textures
   //  from the cache as necessary...
   //
   D3DTEXTUREHANDLE tempTexHandle  = NULL;
   while (true) {
      HRESULT result = pEntry->pTexture->Load(pD3DTextureSrc);

      if (result == D3D_OK) {
         // ok, get a texture handle, and return success...
         //
         pEntry->pTexture->GetHandle(Surface::pD3DDevice2Vector[deviceMinorId],
                                     &tempTexHandle);
         break;
      } else if (texturesRemainingInCache() == true) {
         unloadOldestTexture();
      } else {
         AssertFatal(0, avar("Bogus!  Couldn't get a texture into the cache!  %p (%p): %s",
                             pEntry->pTexture, pEntry->agePrev, D3DAppErrorToString(result)));
      }
   }

   while (tempTexHandle == NULL) {
      if (texturesRemainingInCache() == true) {
         unloadOldestTexture();
         pEntry->pTexture->GetHandle(Surface::pD3DDevice2Vector[deviceMinorId],
                                     &tempTexHandle);
      } else {
         releaseEntry(pEntry);
         AssertFatal(0, "Unable to get a texture handle, what to do?");
      }
   }

   // If we successfully loaded the texture, place it at the head of the
   //  cache.  NOTE: We set it to be the current texture.
   //
   pEntry->key          = in_key;
   pEntry->d3dTexHandle = tempTexHandle;
   pEntry->width        = UInt16(width);
   pEntry->height       = UInt16(height);
   pEntry->mipLevels    = mipLevels;
#ifdef DEBUG
   pEntry->cardSize     = cardSize;
   GFXMetrics.incNumTexturesDownloaded();
#endif
   placeInCache(pEntry);
   makeCurrentEntry(pEntry);
}


//------------------------------------------------------------------------------
// NAME 
//void 
//TextureCache::_bmDownload(GFXSurface*      io_pSurface,
//                          const GFXCacheCallback in_cb,
//                          const GFXTextureHandle               in_texHandle,
//                          const Int32                          in_size,
//                          const DWORD      in_key,
//                          TextureMunger*   io_pMunger)
//
// DESCRIPTION 
//    Downloads a bitmap, given the proper munger...
//    
// NOTES 
//    DMMNOTE: Needs some way of gracefully failing...
//    There are two possible places that a card may indicate that there is
//     no room on the card to accept a texture: first, when the texture surface
//     is created, and second, when ->Load() is called.  To handle this, both
//     locations have routines to unload textures from the card.  The second case
//     is handled below, at the Load() call.  The first is handled in
//     getD3DTexture, when the flag passed to this function is raised.
//------------------------------------------------------------------------------
void 
TextureCache::_bmDownload(GFXSurface*            io_pSurface,
                          const GFXCacheCallback in_cb,
                          const GFXTextureHandle in_texHandle,
                          const Int32            in_size,
                          const DWORD            in_key,
                          TextureMunger*         io_pMunger)
{
   AssertFatal(io_pSurface != NULL, "Error, bad surface passed");
   AssertFatal(io_pMunger  != NULL, "Error, no munger!");

   Surface* pSurface      = (Surface*)io_pSurface;
   Int32    deviceMinorId = pSurface->deviceMinorId;

   DDSURFACEDESC ddsd;
   io_pMunger->getSurfDesc(&ddsd);

   ddsd.dwFlags |= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   ddsd.dwHeight = in_size;
   ddsd.dwWidth  = in_size;

   // Create the DDSurface and D3DTexture
   //
   IDirectDrawSurface2* pDDSurface2Src = NULL;
   IDirect3DTexture2*   pD3DTextureSrc = NULL;
   
   // Get the system memory source texture...
   //
   ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY | DDSCAPS_ALLOCONLOAD);
   ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
   bool success = getBucketTexture(pSurface, io_pMunger, &ddsd,
                                   &pDDSurface2Src,
                                   &pD3DTextureSrc,
                                   io_pMunger->getTextureType());
   AssertFatal(success == true, "Error Getting Source texture, how to handle? DMM");

   // Get the video memory destination texture
   //
   TextureCacheEntry* pEntry = NULL;
   ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_SYSTEMMEMORY);
   ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY | DDSCAPS_ALLOCONLOAD;
   scavengeTextureEntry(pSurface, pEntry,
                        io_pMunger, &ddsd);

   // If necessary, attach the palette to the texture...
   //
   if (io_pMunger->attachPalette() == true) {
      AssertFatal(pDDPalette != NULL, "Error, no palette to attach...");
      pEntry->pSurface->SetPalette(pDDPalette);
   }

   if (io_pMunger->setColorKey() == true) {
      DDCOLORKEY ddCKey;
      ddCKey.dwColorSpaceLowValue  = 0;
      ddCKey.dwColorSpaceHighValue = 0;

      pEntry->pSurface->SetColorKey(DDCKEY_SRCBLT, &ddCKey);
   }

   // If we need to create the callback texture in memory, we need to bite
   //  the bullet here, before we lock the surface...
   //
   GFXBitmap bmp;
   if (io_pMunger->isSurfaceGFXFormat() == false) {
      bmp.width  = in_size;
      bmp.height = in_size;
      bmp.stride = in_size;
		bmp.pBits        = pCallBackBuffer;
      bmp.pMipBits[0]  = pCallBackBuffer;
		bmp.detailLevels = 1;
		bmp.bitDepth     = 8;
		in_cb(in_texHandle, &bmp, 0);
   }

   // Lock the surface, and translate the bitmap
   //
   DDSURFACEDESC ddsDesc;
   ddsDesc.dwSize = sizeof(DDSURFACEDESC);
   
   HRESULT result = pDDSurface2Src->Lock(NULL, &ddsDesc, 0, NULL);
   AssertWarn(result == DD_OK, avar("Error locking surface: %s",
                                    D3DAppErrorToString(result)));

#ifdef DEBUG
   GFXMetrics.textureBytesDownloaded += (ddsDesc.dwHeight * ddsDesc.lPitch);
   Int32 cardSize = (ddsDesc.dwHeight * ddsDesc.lPitch);
#endif

   if (result == DD_OK) {
      if (io_pMunger->isSurfaceGFXFormat() == true) {
         bmp.width  = in_size;
         bmp.height = in_size;
         bmp.stride = ddsDesc.lPitch;
         bmp.pBits        = static_cast<UInt8*>(ddsDesc.lpSurface);
         bmp.pMipBits[0]  = static_cast<UInt8*>(ddsDesc.lpSurface);
         bmp.detailLevels = 1;
         bmp.bitDepth     = 8;
		   in_cb(in_texHandle, &bmp, 0);
      } else {
         io_pMunger->translateBitmap(&bmp,               // texture
                                     0,                  // miplevel
                                     ddsDesc.lpSurface,  // surface ptr
                                     ddsDesc.lPitch);    // surface stride
      }
      result = pDDSurface2Src->Unlock(ddsDesc.lpSurface);
      AssertWarn(result == DD_OK, avar("Error UNLOCKING surface???: %s",
                                       D3DAppErrorToString(result)));
   }

   // At this point, all miplevels of the texture should be translated into the
   //  system memory source texture.  Call Load() to blit from the source
   //  texture to the destination texture on the card, unloading old textures
   //  from the cache as necessary...
   //
   D3DTEXTUREHANDLE     tempTexHandle  = NULL;
   while (true) {
      HRESULT result = pEntry->pTexture->Load(pD3DTextureSrc);

      if (result == D3D_OK) {
         // ok, get a texture handle, and return success...
         //
         pEntry->pTexture->GetHandle(Surface::pD3DDevice2Vector[deviceMinorId],
                                     &tempTexHandle);
         break;
      } else if (texturesRemainingInCache() == true) {
         unloadOldestTexture();
      } else {
         AssertFatal(0, "Bogus!  Couldn't get a texture into the cache!  How to handle?");
      }
   }

   while (tempTexHandle == NULL) {
      if (texturesRemainingInCache() == true) {
         unloadOldestTexture();
         pEntry->pTexture->GetHandle(Surface::pD3DDevice2Vector[deviceMinorId],
                                     &tempTexHandle);
      } else {
         releaseEntry(pEntry);
         AssertFatal(0, "Unable to get a texture handle, what to do?");
      }
   }

   // If we successfully loaded the texture, place it at the head of the
   //  cache.  NOTE: We set it to be the current texture.
   //
   pEntry->key          = in_key;
   pEntry->d3dTexHandle = tempTexHandle;
   pEntry->width        = UInt16(in_size);
   pEntry->height       = UInt16(in_size);
   pEntry->mipLevels    = 1;
#ifdef DEBUG
   pEntry->cardSize     = cardSize;
   GFXMetrics.incNumTexturesDownloaded();
#endif
   placeInCache(pEntry);
   makeCurrentEntry(pEntry);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    TextureCache::downloadBitmap(GFXSurface*    io_pSurface,
//                                 GFXBitmap*     io_pTexture,
//                                 const DWORD    in_key)
//    
// DESCRIPTION 
//    Downloads a normal (non-alpha, non-transp) bitmap
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureCache::downloadBitmap(GFXSurface*      io_pSurface,
                             const GFXBitmap* io_pTexture,
                             const DWORD      in_key)
{
   AssertFatal(io_pSurface != NULL, "Error, bad surface passed");
   AssertFatal(io_pTexture != NULL, "Error, bad bitmap passed");
   AssertFatal(pTexTranslator != NULL, "Error, no translator");
   AssertFatal(pTexTranslator->pNormalMunger != NULL, "Error, no normal munger!");
   AssertFatal(in_key != 0, "Error, 0 key not allowed...");

   _bmDownload(io_pSurface, io_pTexture, 
               in_key, pTexTranslator->pNormalMunger);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    TextureCache::downloadBitmap(GFXSurface*            io_pSurface,
//                                 const GFXCacheCallback in_cb,
//                                 const GFXTextureHandle in_texHandle,
//                                 const Int32            in_size,
//                                 const DWORD            in_key)
//    
// DESCRIPTION 
//    Downloads a normal (non-alpha, non-transp) bitmap
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureCache::downloadBitmap(GFXSurface*            io_pSurface,
                             const GFXCacheCallback in_cb,
                             const GFXTextureHandle in_texHandle,
                             const Int32            in_size,
                             const DWORD            in_key)
{
   AssertFatal(io_pSurface != NULL, "Error, bad surface passed");
   AssertFatal(in_cb != NULL, "Error, no callback...");
   AssertFatal(pTexTranslator != NULL, "Error, no translator");
   AssertFatal(pTexTranslator->pNormalMunger != NULL, "Error, no normal munger!");
   AssertFatal(in_key != 0, "Error, 0 key not allowed...");

   _bmDownload(io_pSurface, in_cb, in_texHandle, in_size,
               in_key, pTexTranslator->pNormalMunger);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    TextureCache::downloadAlphaBitmap(GFXSurface* io_pSurf,
//                                      GFXBitmap*  io_pTexture,
//                                      const DWORD in_key)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureCache::downloadAlphaBitmap(GFXSurface*      io_pSurface,
                                  const GFXBitmap* io_pTexture,
                                  const DWORD      in_key)
{
   AssertFatal(io_pSurface != NULL, "Error, bad surface passed");
   AssertFatal(io_pTexture != NULL, "Error, bad bitmap passed");
   AssertFatal(pTexTranslator != NULL, "Error, no translator");
   AssertFatal(pTexTranslator->pAlphaMunger != NULL, "Error, no normal munger!");
   AssertFatal(in_key != 0, "Error, 0 key not allowed...");

   _bmDownload(io_pSurface, io_pTexture,
               in_key, pTexTranslator->pAlphaMunger);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    TextureCache::downloadTransparentBitmap(GFXSurface* io_pSurface,
//                                            GFXBitmap*  io_pTexture,
//                                            const DWORD in_key)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureCache::downloadTransparentBitmap(GFXSurface*      io_pSurface,
                                        const GFXBitmap* io_pTexture,
                                        const DWORD      in_key)
{
   AssertFatal(io_pSurface != NULL, "Error, bad surface passed");
   AssertFatal(io_pTexture != NULL, "Error, bad bitmap passed");
   AssertFatal(pTexTranslator != NULL, "Error, no translator");
   AssertFatal(pTexTranslator->pTransMunger != NULL, "Error, no normal munger!");
   AssertFatal(in_key != 0, "Error, 0 key not allowed...");

   _bmDownload(io_pSurface, io_pTexture,
               in_key, pTexTranslator->pTransMunger);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    TextureCache::downloadLightMap(GFXSurface*  io_pSurface,
//                                   GFXLightMap* io_pLightmap,
//                                   const DWORD  in_key)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    See note for _bmDownload on texture load failure
//------------------------------------------------------------------------------
void 
TextureCache::scavengeTextureEntry(const Surface*      in_pSurface,
                                   TextureCacheEntry*& out_pEntry,
                                   Munger*             io_pMunger,
                                   DDSURFACEDESC*      io_pDDSD)
{
   AssertFatal(out_pEntry == NULL, "Error, overwriting cacheEntry");

   // Create the DDSurface and D3DTexture
   //
   out_pEntry = getFreeEntry();
   IDirectDrawSurface2* pDDSurface2 = NULL;
   IDirect3DTexture2*   pD3DTexture = NULL;

   bool success = getD3DTexture(in_pSurface, io_pDDSD,
                                &pDDSurface2,
                                &pD3DTexture,
                                true);
   AssertFatal(success == true,
               "Error Getting Destination texture, how to handle? DMM");

   out_pEntry->pSurface = pDDSurface2;
   out_pEntry->pTexture = pD3DTexture;
   out_pEntry->pCreator = io_pMunger;
}


void 
TextureCache::downloadLightMap(GFXSurface*        io_pSurface,
                               const GFXLightMap* io_pLightmap,
                               const DWORD        in_key)
{
   AssertFatal(io_pSurface != NULL, "Error, bad surface passed");
   AssertFatal(io_pLightmap != NULL, "Error, bad bitmap passed");
   AssertFatal(pTexTranslator != NULL, "Error, no translator");
   AssertFatal(pTexTranslator->pLightMunger != NULL,
               "Error, no Transparent munger!");
   AssertFatal(in_key != 0, "Error, 0 key not allowed...");

   Surface* pSurface      = (Surface*)io_pSurface;
   Int32    deviceMinorId = pSurface->deviceMinorId;

   Int32 width     = NearestPow(io_pLightmap->size.x);
   Int32 height    = NearestPow(io_pLightmap->size.y);

   DDSURFACEDESC ddsd;
   pTexTranslator->pLightMunger->getSurfDesc(&ddsd);

   ddsd.dwFlags |= DDSD_HEIGHT | DDSD_WIDTH;
   ddsd.dwWidth  = width;
   ddsd.dwHeight = height;

   // Create the DDSurface and D3DTexture
   //
   IDirectDrawSurface2* pDDSurface2Src = NULL;
   IDirect3DTexture2*   pD3DTextureSrc = NULL;

   // Get the system memory source texture...
   //
   ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_VIDEOMEMORY | DDSCAPS_ALLOCONLOAD);
   ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
   bool success = getBucketTexture(pSurface, pTexTranslator->pLightMunger, &ddsd,
                                   &pDDSurface2Src,
                                   &pD3DTextureSrc,
                                   pTexTranslator->pLightMunger->getTextureType());
   AssertFatal(success == true,
               "Error Getting Source texture, how to handle? DMM");

   // Get the video memory destination texture
   //
   ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_SYSTEMMEMORY);
   ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY | DDSCAPS_ALLOCONLOAD;
   TextureCacheEntry* pEntry = NULL;
   scavengeTextureEntry(pSurface, pEntry,
                        pTexTranslator->pLightMunger, &ddsd);

   // Call the texture translator
   //
#ifdef DEBUG
   Int32 cardSize = 0;
#endif
   while (true) {
      DDSURFACEDESC ddsDesc;
      ddsDesc.dwSize = sizeof(DDSURFACEDESC);
      HRESULT result = pDDSurface2Src->Lock(NULL, &ddsDesc,
                                            0, NULL);
      AssertWarn(result == DD_OK, avar("Error locking surface: %s",
                                       D3DAppErrorToString(result)));
      if (result != DD_OK)
         break;

#ifdef DEBUG
      GFXMetrics.textureBytesDownloaded += (ddsDesc.dwHeight * ddsDesc.lPitch);
      cardSize += (ddsDesc.dwHeight * ddsDesc.lPitch);
#endif
      pTexTranslator->pLightMunger->translateLightmap(io_pLightmap,
                                                      ddsDesc.lpSurface,
                                                      ddsDesc.lPitch);
      result = pDDSurface2Src->Unlock(ddsDesc.lpSurface);
      AssertWarn(result == DD_OK, avar("Error UNLOCKING surface???: %s",
                                       D3DAppErrorToString(result)));

      // Lightmaps have only one mipLevel
      break;
   }

   // At this point, all miplevels of the texture should be translated into the
   //  system memory source texture.  Call Load() to blit from the source
   //  texture to the destination texture on the card, unloading old textures
   //  from the cache as necessary...
   //
   D3DTEXTUREHANDLE tempTexHandle = NULL;
   while (true) {
      HRESULT result = pEntry->pTexture->Load(pD3DTextureSrc);

      if (result == D3D_OK) {
         // ok, get a texture handle, and return success...
         //
         pEntry->pTexture->GetHandle(Surface::pD3DDevice2Vector[deviceMinorId],
                                     &tempTexHandle);
         break;
      } else if (texturesRemainingInCache() == true) {
         unloadOldestTexture();
      } else {
         AssertFatal(0, avar("Couldn't get a texture in the cache! %s",
                             D3DAppErrorToString(result)));
      }
   }

   while (tempTexHandle == NULL) {
      if (texturesRemainingInCache() == true) {
         unloadOldestTexture();
         pEntry->pTexture->GetHandle(Surface::pD3DDevice2Vector[deviceMinorId],
                                     &tempTexHandle);
      } else {
         releaseEntry(pEntry);
         AssertFatal(0, "Unable to get a texture handle, what to do?");
      }
   }

   // If we successfully loaded the texture, place it at the head of the
   //  cache.  NOTE: We set it to be the current texture.
   //
   pEntry->key          = in_key;
   pEntry->d3dTexHandle = tempTexHandle;
   pEntry->width        = UInt16(width);
   pEntry->height       = UInt16(height);
   pEntry->mipLevels    = 1;
#ifdef DEBUG
   pEntry->cardSize     = cardSize;
   GFXMetrics.incNumTexturesDownloaded();
#endif
   placeInCache(pEntry);
   makeCurrentEntry(pEntry);
}


//------------------------------------------------------------------------------
// NAME 
//    bool TextureCache::setCurrentTexture(DWORD key)
//    
// DESCRIPTION 
//    checks for texture identified by in_key in cache.  If it exists, it is
//   set to the current texture, and setCurr... returns true.  If not, false
//   is returned, and the caller is responsible for calling
//   download.*[Bitmap|Lightmap] to place the texture in the cache...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool 
TextureCache::setCurrentTexture(const DWORD in_key)
{
   // Are we setting up for solid fill polys?
   //
   if (in_key == 0) {
      pCurrentTexture = NULL;
   }

   // First check to see if it's the current texture
   //
   if (pCurrentTexture != NULL) {
      if (pCurrentTexture->key == in_key) {
         return true;
      }
   }

   // Ok, not the current texture, go to the cache...
   //
   DWORD hashValue = hashKey(in_key);

   TextureCacheEntry* pEntry = hashTable[hashValue];
   while (pEntry != NULL) {
      if (pEntry->key == in_key) {
         makeCurrentEntry(pEntry);
         return true;
      }
      pEntry = pEntry->cacheNext;
   }

   // Didn't find it in the cache...
   //
   return false;
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    TextureCache::activateCurrentTexture(GFXSurface* io_pSurface)
//    
// DESCRIPTION 
//    Sets the render state of the surface to that required by the current
//   texture.
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureCache::activateCurrentTexture(GFXSurface* io_pSurface)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);
   AssertFatal(pSurface != NULL,
               "Error, null surface ptr passed to setRenderState()");

   if (pCurrentTexture == NULL) {
      // No current texture, null the texturehandle for the device, and set
      //  renderstate to solid fill defaults...
      pSurface->setRenderState(D3DRENDERSTATE_TEXTUREHANDLE,   NULL);
      pSurface->setRenderState(D3DRENDERSTATE_SRCBLEND,        D3DBLEND_ONE);
      pSurface->setRenderState(D3DRENDERSTATE_DESTBLEND,       D3DBLEND_ZERO);
      pSurface->setRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);
      pSurface->setRenderState(D3DRENDERSTATE_BLENDENABLE,     false);
      pSurface->setRenderState(D3DRENDERSTATE_COLORKEYENABLE,  false);

      return;
   }

#ifdef DEBUG
   if (pCurrentTexture->frameU < frameU) {
      GFXMetrics.textureSpaceUsed += pCurrentTexture->cardSize;
      pCurrentTexture->frameU = frameU;
   }
#endif


   const D3DTexRenderState*    pTexRS  = pCurrentTexture->pCreator->getRenderState();
   
   // Set the texture handle...
   //
   pSurface->setRenderState(D3DRENDERSTATE_TEXTUREHANDLE,
                            pCurrentTexture->d3dTexHandle);

   // If we mismatch any render states, set the card up for this texture...
   //
   pSurface->setRenderState(D3DRENDERSTATE_SRCBLEND,        pTexRS->srcBlend);
   pSurface->setRenderState(D3DRENDERSTATE_DESTBLEND,       pTexRS->destBlend);
   pSurface->setRenderState(D3DRENDERSTATE_BLENDENABLE,
                            pTexRS->enableAlphaBlend);
   pSurface->setRenderState(D3DRENDERSTATE_COLORKEYENABLE,
                            pCurrentTexture->pCreator->setColorKey());
}

//------------------------------------------------------------------------------
// NAME 
//    void 
//    TextureCache::flagCurrentForRecache()
//    
// DESCRIPTION 
//    "flags" for recache by removing the texture from the cache entirely
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureCache::flagCurrentForRecache()
{
   if (pCurrentTexture != NULL) {
      unloadEntry(pCurrentTexture);
      pCurrentTexture = NULL;
   }
}

//------------------------------------------------------------------------------
// NAME 
//    void TextureCache::flush()
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
TextureCache::flush()
{
   while (texturesRemainingInCache() == true)
      unloadOldestTexture();

   memset(hashTable, 0, sizeof(TextureCacheEntry*) * hashTableSize);
   pCurrentTexture = NULL;
}

}; // namespace Direct3D