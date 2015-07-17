//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _P_TXCACH_H_
#define _P_TXCACH_H_

//Includes

#include "types.h"
#include "g_types.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GFXPalette;
class GFXBitmap;
class GFXFont;

class GFXTextureCacheEntry;

class GFXLightMapCache
{
	struct data_page {
		data_page *next;
		int data;
	};
	enum {
		PageSize = 20,
	};
	struct arrayHeadTag {
		int maxSize;
		int stackSize;
		int stackCount;
		void **stack;
		data_page *link;
	};
	int maxMaxSize; // largest acceptable light map.
	int numArrays;
	arrayHeadTag *arrays;

	void allocateMore(arrayHeadTag *head);
public:
	GFXLightMapCache(int *sizeList);
	~GFXLightMapCache();
	GFXLightMap* allocate(int numPixels);
	void release(GFXLightMap *map);
};


class GFXTextureCache
{
	BYTE *bitmapData;	// data block from which textures are allocated.
   BYTE *bitmapDataBase;
	int wrapCount;		// how many times the cache has wrapped around.
	int dataCacheSize;	// size in bytes of the data cache.
	BYTE *curDataPtr;	// current pointer into the cache.

   bool dwordPadTextures;

	int arraySize; 		// size of the array.
	int hashTableSize;	// must be a power of 2.
	GFXTextureCacheEntry **hashTable;
	GFXTextureCacheEntry *array; 			// array of entries.
	GFXTextureCacheEntry *freeListHead; 	// where the entries are taken from
	GFXTextureCacheEntry *freeListTail; 	// where the entries are moved to when used.
	GFXTextureCacheEntry *currentEntry;

	GFXTextureCacheEntry* getFreeEntry(); 	// return a new free entry.
	void touch(GFXTextureCacheEntry *ent);	// move to the end of the free list.
	inline Bool overCached(GFXTextureCacheEntry *ent);
	void allocate(GFXTextureCacheEntry *tce);
	inline GFXTextureCacheEntry* getEntry(GFXTextureHandle tex);
	void HashInsert(GFXTextureCacheEntry *tce);
	int HashHandle(GFXTextureHandle tex);
	Bool HandleEqual(GFXTextureHandle tex1, GFXTextureHandle tex2);
public:
   UInt32 frameU;

   typedef void (*CacheFunction)(GFXTextureCacheEntry *tce, GFXPalette *pal);
	GFXTextureCache(int numTCEntries, int htSize, int dataSize);
	~GFXTextureCache();
	
   void enablePadding(const bool in_enable);
   bool isPadding() const { return dwordPadTextures; }
   
   void* allocateChunk(int size);
	Bool  setTextureHandle(GFXTextureHandle tex);
	GFXTextureCacheEntry* getCurrentEntry() { return currentEntry; }
	void registerTexture(GFXTextureHandle tex, int sizeX, int sizeY, 
		int offsetX, int offsetY, int lightScale, GFXLightMap *map,
		const GFXBitmap *texture, int mipLevel);
	void registerTexture(GFXTextureHandle tex, GFXCacheCallback cb, int csizeX, 
		int csizeY, int lightScale, GFXLightMap *map);

	void setLightMap(int lightScale, GFXLightMap *map);
	void setTextureMap(const GFXBitmap *bmp);
	void setMipLevel(int mipLevel);
	void setTextureSize(int newSize);
	void prepareForDraw(GFXTextureCacheEntry *ent, GFXPalette *palette);
	GFXLightMap* getLightMap();
	void flushCache(void);
   
   // For the Rendition driver, in order to lock the memory for the cache...
   //
   BYTE*  getBaseAddress() const { return bitmapData; }
   UInt32 getCacheSize()   const { return dataCacheSize; }
};

extern GFXTextureCache gfxTextureCache;
extern GFXLightMapCache gfxLightMapCache;



#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_P_TXCACH_H_
