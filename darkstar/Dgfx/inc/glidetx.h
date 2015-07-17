#ifndef _H_GLIDETX
#define _H_GLIDETX

#include "g_types.h"
#include "g_bitmap.h"

class GFXPalette;

namespace Glide {

struct TMU
{
   Int32 wrapCount;
   DWORD curTexAddress;
   DWORD minTexAddress;
   DWORD maxTexAddress;
   Int32 curTextureWrapCount;
   DWORD curTextureAddress;
   DWORD curPaletteIndex;
   DWORD transRemapIndex;
   DWORD transRemapTable[256];
   int tmuId;

   bool setCurrentTexture(GFXBitmap::GlideCacheInfo &ci, DWORD paletteIndex, GFXPalette *palette);
   void invalidateAllTextures();
   void flushTexture(GFXBitmap* texture, GFXPalette* io_pPal,const bool in_reload);
   void advanceCurrentTexturePtr(GFXBitmap::GlideCacheInfo &ci);
   void calcLodAndAspectRatio(GFXBitmap::GlideCacheInfo &ci, int width, int height, int mipMapLevels);
   void downloadData(GFXBitmap::GlideCacheInfo &ci, const void *srcPtr);
   void downloadTranslucentBitmap88(GFXBitmap *bmp, GFXPalette *pal, GFXBitmap::GlideCacheInfo &ci);
   void downloadTranslucentBitmap(GFXBitmap *bmp, GFXPalette *pal, GFXBitmap::GlideCacheInfo &ci);
   void downloadBitmap(GFXBitmap *bmp, GFXPalette *pal, GFXBitmap::GlideCacheInfo &ci);
   void downloadLightMap(GFXBitmap::GlideCacheInfo &ci, GFXLightMap *map, int lmapWidth, int lmapHeight);
   void activate(GFXBitmap::GlideCacheInfo &ci, DWORD paletteIndex, GFXPalette *pal);
   TMU();
};

class TextureCache
{
private:
   enum { MAX_TMUS = 3 };

   int curTMU;
   int tableSize;
   int tmuCount;
public:
   int frameU;
	BYTE *downloadBuffer; // in case we have to convert to 16-bit before DL.
	TextureCache(int tmuCount, int tableSize);
	~TextureCache();
   void flush();

   TMU tmu[MAX_TMUS];
};		

enum HandleFlags { LightMapValid = 1 };

struct HandleCacheEntry
{
	GFXTextureHandle handle;
	DWORD flags;
   bool clamp;
	HandleCacheEntry *hashLink, *next, *prev;
	Point2F coordScale, coordTrans;
   Point2F lmapScale, lmapTrans;
   Point2I lmapSize;
	GFXBitmap *bmp;
	GFXLightMap *lMap;
   GFXBitmap::GlideCacheInfo lMapInfo;
   GFXBitmap::GlideCacheInfo tMapInfo;
   DWORD tMapPaletteIndex;
	int size;
	DWORD key;
	GFXCacheCallback cb; // if it has one of these, we have to regen
										// the bitmap when it's recached.
};

class HandleCache
{
private:
	int                  entryTableSize;
   int                  m_numHandles;
	DWORD                curCbKey;
	HandleCacheEntry*    currentEntry;
	HandleCacheEntry**   entryTable;
	HandleCacheEntry*    freeListTail;
   HandleCacheEntry*    freeListHead;
	HandleCacheEntry*    array;

	void touch(HandleCacheEntry *entry);
	int  HashHandle(GFXTextureHandle);
	Bool HandleEqual(GFXTextureHandle, GFXTextureHandle);
public:
	HandleCache(int numHandles, int handleTableSize);
   ~HandleCache();
	void  HashInsert(HandleCacheEntry *);
	DWORD GetNextCbKey() { int temp = curCbKey; curCbKey += 4; return temp; }
	HandleCacheEntry* getFreeEntry();
	HandleCacheEntry* getCurrentEntry() { return currentEntry; }
	GFXLightMap* getLightMap();
	void setLightMap(int lightScale, GFXLightMap *map);
	void setTextureMap(const GFXBitmap *bmp);
	void setMipLevel(int mipLevel);
	Bool setTextureHandle(GFXTextureHandle tex);
   void flush();
};


}

#endif
