//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GD3DTXCACHE_H_
#define _GD3DTXCACHE_H_

//Includes
#include "gD3DTxTrans.h"
#include "p_txcach.h"


#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


class GFXSurface;
class GFXPalette;

namespace Direct3D {

struct BucketSurface {
   Int32 widthHeight;  // width << 16 | height
   Int32  lastUsed;
   
   IDirectDrawSurface2* pSurface;
   IDirect3DTexture2*   pTexture;
};

struct TextureCacheEntry {
   DWORD key;

   D3DTEXTUREHANDLE     d3dTexHandle;
   IDirectDrawSurface2* pSurface;
   IDirect3DTexture2*   pTexture;
   
   Munger* pCreator;

   Int16 width;
   Int16 height;
   Int32 mipLevels;  

   TextureCacheEntry* freeLink;
   TextureCacheEntry* cacheNext;
   TextureCacheEntry* cachePrev;
   TextureCacheEntry* ageNext;
   TextureCacheEntry* agePrev;

#ifdef DEBUG
   Int32 frameU;
   Int32 cardSize;
#endif

   ~TextureCacheEntry() { AssertWarn(pSurface == NULL && pTexture == NULL,
                                                "Error, deleting loaded texture!"); }
};

class Surface;

class TextureCache {
  private:
   enum { hashTableSize = 256 };

   // Translators for downloading textures
   TextureTranslator* pTexTranslator;
   
   // Current Palettes
   GFXPalette*         pPalette;
   IDirectDrawPalette* pDDPalette;

   // don't touch!
   const Int32        numCacheEntries;
   TextureCacheEntry* allocArray;
   
   // Free list sentry
   TextureCacheEntry freeList;

   // Currently loaded textures
   TextureCacheEntry ageList;
   TextureCacheEntry ageTail;

   TextureCacheEntry**  hashTable;
   TextureCacheEntry*   pCurrentTexture;

   // Bucket Surface Textures...
   //
   static int    sm_numBucketEntries;
   int           m_bucketTime;
   BucketSurface m_sysmemCache[4][256];

   TextureCacheEntry* getFreeEntry();
   void               unloadEntry(TextureCacheEntry* io_pTex);
   void               releaseEntry(TextureCacheEntry* io_pTrash);

   void   placeInCache(TextureCacheEntry* io_pEntry);
   void   removeFromCache(TextureCacheEntry* io_pRemove);
   void   makeCurrentEntry(TextureCacheEntry* io_pCurrent);
   UInt16 hashEntry(const TextureCacheEntry* in_pEntry) const;
   UInt16 hashKey(const DWORD in_key) const;
   bool   texturesRemainingInCache() const;

   // Private download functions
   //
   bool getD3DTexture(const Surface*        io_pSurface,
                              DDSURFACEDESC*        io_pDDSD,
                              IDirectDrawSurface2** out_ppDDSurface2,
                              IDirect3DTexture2**   out_ppD3DTexture2,
                              const bool            in_unloadIfFailed);
   bool getBucketTexture(const Surface*        in_pSurface,
                                 Munger*             io_pMunger,
                                 DDSURFACEDESC*        in_pDDSD,
                                 IDirectDrawSurface2** out_ppDDSurface2,
                                 IDirect3DTexture2**   out_ppD3DTexture2,
                                 int                   in_type);
   
   void scavengeTextureEntry(const Surface*      in_pSurface,
                                     TextureCacheEntry*& out_pEntry,
                                     Munger*             io_pMunger,
                                     DDSURFACEDESC*      io_pDDSD);
   void _bmDownload(GFXSurface*      io_pSurf,
                            const GFXBitmap* io_pTexture,
                            const DWORD      in_key,
                            TextureMunger*   io_pMunger);
   void _bmDownload(GFXSurface*            io_pSurf,
                            const GFXCacheCallback in_cb,
                            const GFXTextureHandle in_texHandle,
                            const Int32            in_size,
                            const DWORD            in_key,
                            TextureMunger*         io_pMunger);
  public:
   Int32 frameU;

   void setPalette(GFXPalette*         pGFXPalette,
                           IDirectDrawPalette* pDDPalette);

   void downloadBitmap(GFXSurface*            io_pSurface,
                               const GFXCacheCallback in_cb,
                               const GFXTextureHandle in_texHandle,
                               const Int32            in_size,
                               const DWORD            in_key);
   void downloadBitmap(GFXSurface*      io_pSurf,
                               const GFXBitmap* io_pTexture,
                               const DWORD      in_key);
   void downloadAlphaBitmap(GFXSurface*      io_pSurf,
                                    const GFXBitmap* io_pTexture,
                                    const DWORD      in_key);
   void downloadTransparentBitmap(GFXSurface*      io_pSurf,
                                          const GFXBitmap* io_pTexture,
                                          const DWORD      in_key);
   void downloadLightMap(GFXSurface*        io_pSurf,
                                 const GFXLightMap* in_pLightMap,
                                 const DWORD        in_key);

   bool setCurrentTexture(const DWORD in_key);
   void activateCurrentTexture(GFXSurface* io_pSurface);
   void flagCurrentForRecache();

   void unloadOldestTexture();

   void flush();

   TextureCache(const Int32        in_numEntries,
                           TextureTranslator* in_pTexTranslator);
   ~TextureCache();

   UInt8* pCallBackBuffer;
};


}; // namespace Direct3D

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GD3DTXCACHE_H_


