//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GD3DHCACHE_H_
#define _GD3DHCACHE_H_

//Includes
#include "g_types.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace Direct3D {

struct HandleCacheEntry {
   bool inUse;

   DWORD            key;
   GFXTextureHandle handle;
   DWORD            flags;
   
   const GFXBitmap *bmp;
   GFXLightMap     *lMap;

   // For callback textures, the size of one side.
   Int32 size;

   Point2F texScale,  texOffset;
   Point2F lMapScale, lMapOffset;
   Point2I lMapSize;
   
   GFXCacheCallback cb;   // if it has one of these, we have to regen
                          //  the bitmap when it's recached.
   bool clamp;

   HandleCacheEntry *hashLink;
   HandleCacheEntry *freePrev, *freeNext;
};

class Surface;

class HandleCache {
  private:
   Surface* pSurface;

   enum { hashTableSize = 512 };
   Int32 numHandles;

   DWORD currCBKey;
   
//   // For touch marks...
//   bool *touchArray;
   
   // Not to be touched except by destructor
   HandleCacheEntry* allocArray;

   // Head and tail sentries for the free list
   HandleCacheEntry*  freeHead;
   HandleCacheEntry*  freeTail;
   
   // Hash table
   HandleCacheEntry** hashArray;
   
   // obv.
   HandleCacheEntry* currentEntry;
   
   UInt16 hashHandle(const GFXTextureHandle& in_handle);
   void   touchEntry(HandleCacheEntry* io_pEntry);
   bool   isHandleEqual(const GFXTextureHandle,
                                const GFXTextureHandle) const;
   void   removeFromHashTable(HandleCacheEntry* io_pEntry);
  public:
   HandleCache(const Int32 in_numHandles,
                          Surface*    io_pSurface);
   ~HandleCache();

   void  hashInsert(HandleCacheEntry* io_pEntry);
   DWORD getNextCBKey() { DWORD temp = currCBKey; currCBKey += 4; return temp; }

   HandleCacheEntry* getFreeEntry();
   HandleCacheEntry* getCurrEntry() { return currentEntry; }

   Bool setTextureHandle(const GFXTextureHandle& tex);
   void markCurrentEntry();

   void handleSetTextureMap(const GFXBitmap* in_pTexture);
   void handleSetLightMap(GFXLightMap* in_pLMap);

//   void clearTouchMarks();

   void flush();
};


};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GD3DHCACHE_H_
