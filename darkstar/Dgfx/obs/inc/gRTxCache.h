//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GRTXCACHE_H_
#define _GRTXCACHE_H_

//Includes
#include <base.h>

#include "verite.h"
#include "redline.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class  GFXTextureCache;
class GFXTextureCacheEntry;
class  GFXPalette;

namespace Rendition {

class  RenditionProxy;
struct TextureCacheEntry;
class  Surface;

class TextureCache {
   static const UInt32 sm_hashTableSize;
   
   enum { NUM_TXCACHE_ENTRIES = 1024 };
   friend class Surface;

  public:
   TextureCache(GFXTextureCache& io_rTxCache,
                           RenditionProxy*  io_pProxy,
                           v_handle         io_vHandle,
                           Surface*         io_pSurface);
   ~TextureCache();
   
   void setPalette(const GFXPalette* io_pPalette);

   void flushCache();
   void flushTexture(const GFXBitmap*, const bool);
   bool drawingTexture() { return (m_currentEntry != NULL); }
   
   // Higher level utility functions...
   //
  protected:
   void placeHashEntry(TextureCacheEntry* io_pPlace);
   void removeHashEntry(TextureCacheEntry* io_pRemove);

   void unloadOldestEntry();
   void touchEntry(TextureCacheEntry* io_pTouch);
   bool texturesRemainInCache() const;

   // Worker functions
   //
  protected:
   void unloadEntry(TextureCacheEntry*     io_pUnload);
   void removeFromCache(TextureCacheEntry* io_pRemove);
   void placeInCache(TextureCacheEntry*    io_pPlace);
   TextureCacheEntry* findInCache(const UInt32 in_key1,
                                  const UInt32 in_key2);

   TextureCacheEntry* getFreeEntry();
   void placeOnFreeList(TextureCacheEntry* io_pFree);

   bool setTexture(const GFXTextureCacheEntry* in_tce);
   bool setTexture(const BYTE* in_pTexture);
   
   void downloadTexture(const GFXTextureCacheEntry* in_tce,
                        const bool                  in_enableAlpha,
                        const bool                  in_enableTransp);
   void downloadTexture(BYTE*  in_pTexture,
                        const UInt32 in_stride,
                        const UInt32 in_height,
                        const bool   in_enableAlpha,
                        const bool   in_enableTransp);

   v_surface* createRenditionSurface(const UInt32 in_stride,
                                     const UInt32 in_height,
                                     const v_u32  in_format);
   TextureCacheEntry* primeEntry(BYTE*  io_pBits,
                                 UInt32 in_stride,
                                 UInt32 in_height,
                                 v_u32  in_format);
   void primeForDraw();
   void primeForBitmap(const bool in_transparent,
                       const bool in_translucent);
  private:
   // Texture cache and surface to download textures from...
   //
   GFXTextureCache& m_gfxTxCache;
   Surface*         m_pSurface;

   // Rendition interface objects...
   //
   RenditionProxy* m_pProxy;
   v_handle        m_vHandle;
   v_memory        m_cacheVMemory;
   
   // Translation 8bit -> 16bit buffers...
   //
   v_surface* m_pNormalSurface;
   v_surface* m_pAlphaSurface;
   v_memory   m_normalMemory;
   v_memory   m_alphaMemory;

   UInt16*     m_pNormalTranslation;
   UInt16*     m_pAlphaTranslation;
   GFXPalette* m_pPalette;
   
   // Actual texture cache...
   //
   TextureCacheEntry*  m_pEntryBuffer;      // don't touch...
   TextureCacheEntry** m_hashTable;

   TextureCacheEntry& m_rFreeList;
   TextureCacheEntry& m_rAgeHead;
   TextureCacheEntry& m_rAgeTail;
   TextureCacheEntry* m_currentEntry;
};

}; // namspace Rendition


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GRTXCACHE_H_
