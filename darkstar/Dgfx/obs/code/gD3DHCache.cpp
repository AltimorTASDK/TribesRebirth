//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "g_bitmap.h"

#include "gD3DHCache.h"
#include "gD3DTxCache.h"
#include "gD3DSfc.h"
#include "p_txcach.h"

#define GFXD3DINLINE inline

namespace Direct3D {

//------------------------------------------------------------------------------
//-------------------------------------- HandleCache
//

HandleCache::HandleCache(const Int32 in_numHandles,
                         Surface*    io_pSurface)
 : numHandles(in_numHandles),
   pSurface(io_pSurface)
{
//   touchArray = new bool[numHandles];
   allocArray = new HandleCacheEntry[numHandles];
   hashArray  = new HandleCacheEntry*[hashTableSize];

   // Just to be sure...
   memset(allocArray, 0, sizeof(HandleCacheEntry)  * numHandles);
   memset(hashArray,  0, sizeof(HandleCacheEntry*) * hashTableSize);

   // Set up the free list
   for (int i = 0; i < numHandles; i++) {
      allocArray[i].inUse    = false;

      allocArray[i].freeNext = allocArray + i + 1;
      allocArray[i].freePrev = allocArray + i - 1;
   }
   allocArray[0].freePrev              = NULL;
   allocArray[numHandles - 1].freeNext = NULL;

   freeHead = &allocArray[0];
   freeTail = &allocArray[numHandles - 1];

   // No entry for the moment...
   currentEntry = NULL;

   // Must be non-zero
   currCBKey = 1;
}



HandleCache::~HandleCache()
{
   flush();
   delete [] allocArray;
   delete [] hashArray;

   pSurface = NULL;
}


//------------------------------------------------------------------------------
// NAME 
//    Int32 HandleCache::hashHandle(const GFXTextureHandle in_handle)
//    
// DESCRIPTION 
//    gets a hash value from a textureHandle, in the range 0-65535
//    
//------------------------------------------------------------------------------
GFXD3DINLINE UInt16 
HandleCache::hashHandle(const GFXTextureHandle& in_handle)
{
   // Note: uses same hash as Glide::HandleCache::HashHandle
   //
   UInt32 hashVal = in_handle.key[0] ^ in_handle.key[1];
   return static_cast<UInt16>((hashVal >> 16) ^ (hashVal & 0xffff));
}


//------------------------------------------------------------------------------
// NAME 
//    bool isHandleEqual(const GFXTextureHandle, const GFXTextureHandle)
//    
// DESCRIPTION 
//    returns true if two handles are equivalent
//    
//------------------------------------------------------------------------------
GFXD3DINLINE bool 
HandleCache::isHandleEqual(const GFXTextureHandle in_t1,
                           const GFXTextureHandle in_t2) const
{
	return ((in_t1.key[0] == in_t2.key[0]) &&
	        (in_t1.key[1] == in_t2.key[1]));
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    HandleCache::removeFromHashTable(HandleCacheEntry* io_pEntry)
//    
// DESCRIPTION 
//    removes from the hash linked list...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
HandleCache::removeFromHashTable(HandleCacheEntry* io_pEntry)
{
   AssertFatal(io_pEntry != NULL, "No entry in removeFromHashTable");

   UInt32 hashVal = hashHandle(io_pEntry->handle) & (hashTableSize - 1);
   AssertFatal(hashVal < hashTableSize && hashVal >= 0,
               avar("Error: bad hash value in removeFromHashTable: %u", hashVal));

   HandleCacheEntry* prevEntry  = NULL;
   HandleCacheEntry* probeEntry = hashArray[hashVal];
   
   // Check that its not the first hashtable entry...
   //
   AssertFatal(probeEntry != NULL, "Error, no hash entries is expected location");
   if (isHandleEqual(probeEntry->handle, io_pEntry->handle) == true) {
      hashArray[hashVal] = probeEntry->hashLink;
      return;
   }

   // Scan the list...
   //
   while (probeEntry->hashLink != NULL) {
      if (isHandleEqual(probeEntry->hashLink->handle, io_pEntry->handle) == true) {
         prevEntry = probeEntry;
         break;
      }
      probeEntry = probeEntry->hashLink;
   }
   AssertFatal(prevEntry != NULL, "Error, entry not found in proper location!");

   prevEntry->hashLink = io_pEntry->hashLink;
   io_pEntry->hashLink = NULL;
}


//------------------------------------------------------------------------------
// NAME 
//    void HandleCache::touchEntry(HandleCacheEntry* io_pEntry)
//    
// DESCRIPTION 
//    Moves the input entry to the front of the cache, which corresponds
//     to the back of the free list
//    
// NOTES 
//    DMMNOTE: Should move to front of hashTable entry too?
//------------------------------------------------------------------------------
GFXD3DINLINE void 
HandleCache::touchEntry(HandleCacheEntry* io_pEntry)
{
   AssertFatal(io_pEntry != NULL,
               "Error null cache entry in HandleCache::touchEntry");

   // Remove the entry from the list
   //
   io_pEntry->freePrev->freeNext = io_pEntry->freeNext;
   io_pEntry->freeNext->freePrev = io_pEntry->freePrev;

   // And place just behind the freeList tail sentry
   io_pEntry->freeNext = freeTail;
   io_pEntry->freePrev = freeTail->freePrev;
   freeTail->freePrev->freeNext = io_pEntry;
   freeTail->freePrev = io_pEntry;
}


//------------------------------------------------------------------------------
// NAME 
//    void HandleCache::hashInsert(HandleCacheEntry* io_pEntry)
//    
// DESCRIPTION 
//    Enters a cache entry into the appropriate hash table slot.
//    
// NOTES 
//    Assumes that the entry has been moved "out of danger" in the free list
//     by getFreeEntry
//------------------------------------------------------------------------------
void 
HandleCache::hashInsert(HandleCacheEntry* io_pEntry)
{
   AssertFatal(io_pEntry != NULL,
               "Error null cache entry in HandleCache::hashInsert");

   io_pEntry->inUse = true;
   UInt32 hashVal = hashHandle(io_pEntry->handle) & (hashTableSize - 1);
   AssertFatal(hashVal < hashTableSize && hashVal >= 0,
               avar("Error: bad hash value in hashInsert: %u", hashVal));

   io_pEntry->hashLink = hashArray[hashVal];
   hashArray[hashVal]  = io_pEntry;
}


//------------------------------------------------------------------------------
// NAME 
//    HandleCacheEntry* HandleCache::getFreeEntry()
//    
// DESCRIPTION 
//    Returns a free cache entry, and sets it to the current entry.  Also
//     moves it to the front of the cache
//    
// NOTES 
//    
//------------------------------------------------------------------------------
HandleCacheEntry* 
HandleCache::getFreeEntry()
{
   HandleCacheEntry* retEntry = freeHead->freeNext;

   if (retEntry->inUse == true) {
      // If the handle has a lightmap associated w/ it, release it from the
      //  gfx cache
      if (retEntry->lMap != NULL) {
         gfxLightMapCache.release(retEntry->lMap);
         retEntry->lMap = NULL;
      }
      removeFromHashTable(retEntry);
      retEntry->inUse = false;
   }

   // Move it to the front of the cache, set it as the current entry,
   //  and return...
   touchEntry(retEntry);
   currentEntry = retEntry;
   return retEntry;
}


//------------------------------------------------------------------------------
// NAME 
//    Bool HandleCache::setTextureHandle(GFXTextureHandle tex)
//    
// DESCRIPTION 
//    sets the currentEntry field by handle
//    
// RETURNS 
//    true if the texture was in the cache, false otherwise
//    
// NOTES 
//    if the routine fails, the currentEntry pointer is NULL
//------------------------------------------------------------------------------
Bool 
HandleCache::setTextureHandle(const GFXTextureHandle& in_texHandle)
{
   UInt32 hashVal = hashHandle(in_texHandle) & (hashTableSize - 1);
   AssertFatal(hashVal < hashTableSize && hashVal >= 0,
               avar("Error: bad hash value in hashInsert: %u", hashVal));
   
   currentEntry = hashArray[hashVal];
   while (currentEntry != NULL) {
      if (isHandleEqual(currentEntry->handle, in_texHandle) == true)
         break;

      currentEntry = currentEntry->hashLink;
   }

   if (currentEntry != NULL) {
      // Found the texture, move it to the front, and 
      //
      touchEntry(currentEntry);
      return true;
   } else {
      // Texture entry is not in the HandleCache
      //
      return false;
   }
}


void 
HandleCache::handleSetTextureMap(const GFXBitmap* in_pTexture)
{
   AssertFatal(currentEntry != NULL, "Error, no entry to set texture for...");
   
   if (currentEntry->bmp) {
      AssertFatal(currentEntry->bmp->getStride() == in_pTexture->getStride(),
                  "Mismatched X dimension...");
      AssertFatal(currentEntry->bmp->getHeight() == in_pTexture->getHeight(),
                  "Mismatched Y dimension...");
   }

   // If the texture is currently in the texturecache, we need to flag it as
   //  needing a recache...
   //
   AssertFatal(pSurface->pTextureCache != NULL, "Error, surface not locked...");
   currentEntry->bmp = in_pTexture;
   if (pSurface->pTextureCache->setCurrentTexture(DWORD(in_pTexture)) == true) {
      pSurface->pTextureCache->flagCurrentForRecache();
   }
}

void 
HandleCache::handleSetLightMap(GFXLightMap* in_pLMap)
{
   AssertFatal(currentEntry != NULL, "Error, no entry to set texture for...");

   if (currentEntry->lMap != NULL)
		gfxLightMapCache.release(currentEntry->lMap);
   
   // If the texture is currently in the texturecache, we need to flag it as
   //  needing a recache...
   //
   AssertFatal(pSurface->pTextureCache != NULL, "Error, surface not locked...");
   currentEntry->lMap = in_pLMap;
   if (pSurface->pTextureCache->setCurrentTexture(DWORD(in_pLMap)) == true) {
      pSurface->pTextureCache->flagCurrentForRecache();
   }
}

//------------------------------------------------------------------------------
// NAME 
//    void HandleCache::flush()
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
HandleCache::flush()
{
   for (int i = 0; i < numHandles; i++) {
      HandleCacheEntry* pEntry = &allocArray[i];

      if (pEntry->inUse == true) {
         // If the handle has a lightmap associated w/ it, release it from the
         //  gfx cache
         if (pEntry->lMap != NULL) {
            gfxLightMapCache.release(pEntry->lMap);
            pEntry->lMap = NULL;
         }
         removeFromHashTable(pEntry);
         pEntry->inUse = false;
      }
   }
}

};