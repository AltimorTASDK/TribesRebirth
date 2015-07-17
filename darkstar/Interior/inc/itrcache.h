//---------------------------------------------------------------------------

//	$Workfile$
//	$Revision$
//	$Version$
//	$Date$

//---------------------------------------------------------------------------

#ifndef _ITRCACHE_H_
#define _ITRCACHE_H_

#include <ts_material.h>
#include <tvector.h>
#include <itrlighting.h>

//---------------------------------------------------------------------------

class GFXBitmap;


//---------------------------------------------------------------------------

class ITRTextureCache
{
public:
	typedef int Handle;
	struct LightMap {
		enum Constants {
			// Size of header only:
			// sizeof(LightMap) - sizeof(UInt16)
			HeaderSize = 4,
		};
		struct Point {
			UInt8 x,y;
		};
		Point size;					//
		Point offset;				// Offset in texture coor.
		UInt16 data;				// Start of data
	};

private:
	enum Constants {
		HeaderTag = 'HTAG',
		TagSize = sizeof(Int32),
		BufferSize = 4000 * 1024,
		InvalidHandle = 0,
	};
	struct Header {
		enum Constants {
			Size = 8, // sizeof(Header) - sizeof(data);
		};
		Int32 tag;
		Int16 mipLevel;
		UInt8 width;	// Stored width-1
		UInt8 height;	// Stored height-1
		UInt8 data;
	};
	GFXPalette* palette;

	Vector<GFXBitmap*> bitmaps;
	int bitmapIndex;

	Vector<UInt8> buffer;
	int bufferIndex;

	GFXBitmap* nextBitmap();
	void clearHeader(Handle);
	Header* getHeader(Handle);
	Handle allocate(int size);
public:

	ITRTextureCache(int buffSize = BufferSize);
	~ITRTextureCache();

	void reset() { bitmapIndex = 0; }
	bool isValid(Handle);
	bool isValid(Handle,int mipLevel);
	void setPalette(GFXPalette* pal);

	GFXBitmap* getTexture(ITRTextureCache::Handle h);
	int getMipLevel(ITRTextureCache::Handle h);
	Handle buildTexture(Handle h,int sizeX,int sizeY,int offsetX,int offsetY,
		TSMaterial*,UInt16 color);
	Handle buildTexture(Handle h,int sizeX,int sizeY,int offsetX,int offsetY,
		int scale,TSMaterial*,LightMap* map,int mipLevel);
};


//---------------------------------------------------------------------------

inline ITRTextureCache::Header* ITRTextureCache::getHeader(Handle h)
{
	return (Header*) &buffer[h];
}

inline bool ITRTextureCache::isValid(Handle h)
{
	return getHeader(h)->tag == HeaderTag;
}

inline bool ITRTextureCache::isValid(Handle h,int mipLevel)
{
	Header* header = getHeader(h);
	return header->tag == HeaderTag && header->mipLevel == mipLevel;
}

inline void ITRTextureCache::clearHeader(Handle h)
{
	getHeader(h)->tag = 0;
}

inline int ITRTextureCache::getMipLevel(ITRTextureCache::Handle h)
{
	return getHeader(h)->mipLevel;
}	

inline void ITRTextureCache::setPalette(GFXPalette* pal)
{
	palette = pal;
}	

#endif


