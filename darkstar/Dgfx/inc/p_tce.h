//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _P_TCE_H_
#define _P_TCE_H_

//Includes
#include "g_bitmap.h"
#include "p_txcach.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GFXTextureCacheEntry
{
public:
   enum Flags {
      needsRecache      = 1 << 0,
      isCallbackTexture = 1 << 1
   };
   
   GFXTextureHandle handle;
   DWORD            flags;
   
   GFXTextureCacheEntry *hashLink;  // link for chaining in hash buckets.
   GFXTextureCacheEntry *next;      // links for lists.
   GFXTextureCacheEntry *prev;

   // Size in the cache = size[XY] >> mipLevel.
   //  boundary...
   //
   int csizeX, csizeY;

   const GFXBitmap *bmp;   // source texture
   GFXCacheCallback cb;
   GFXTextureCache::CacheFunction cf;
   int sizeX;
   int sizeY;
   int offsetX;
   int offsetY;
   int mipLevel;

   BYTE *dataPtr; // pointer to the data in the cache
   int wrapCount; // wrap count when this texture was added to the cache - used
               // to determine if the texture is still valid.

   int lightScale;
   GFXLightMap *lMap;   // light map for the surface

#ifdef DEBUG
   UInt32 frameUsed;
#endif

   inline void setFlag(const Flags in_flag)        { flags |= in_flag; }
   inline void clearFlag(const Flags in_flag)      { flags &= ~in_flag; }
   inline Bool testFlag(const Flags in_flag) const { return ((flags & in_flag) != 0); }
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_P_TCE_H_
