//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include <ts.h>
#include "itrcache.h"


//----------------------------------------------------------------------------

static int getShift(int x)
{
	int i = 0;
	if (x)
		while (!(x & 1))
			i++, x >>= 1;
	return i;
}


//----------------------------------------------------------------------------

ITRTextureCache::ITRTextureCache(int bufferSize)
{
	bitmapIndex = 0;
	bufferIndex = TagSize;
	buffer.setSize(bufferSize);
	// A handle of 0 is always invalid.
	getHeader(0)->tag = 0;
}

ITRTextureCache::~ITRTextureCache()
{
	for (int i = 0; i < bitmaps.size(); i++)
		delete bitmaps[i];
}


//----------------------------------------------------------------------------

GFXBitmap* ITRTextureCache::nextBitmap()
{
	if (bitmapIndex < bitmaps.size() - 1)
		return bitmaps[bitmapIndex++];

	// Allocate a new bitmap
   GFXBitmap *bm = new GFXBitmap;
   bm->stride = 256;
   bm->bitDepth = 8;
	bm->imageSize = 256 * 256;

	bitmaps.push_back(bm);
	bitmapIndex++;
	return bm;
}


//----------------------------------------------------------------------------

ITRTextureCache::Handle ITRTextureCache::allocate(int dsize)
{
	// Implements simple wrap-around cache.
	int size = dsize + Header::Size;
	if (bufferIndex + size > buffer.size())
		bufferIndex = TagSize;
	while (isValid(bufferIndex)) {
		clearHeader(bufferIndex);
		bufferIndex += TagSize;
		if (bufferIndex + size > buffer.size())
			bufferIndex = TagSize;
	}
	Handle h = bufferIndex;
	getHeader(h)->tag = HeaderTag;
	bufferIndex += size;
	return h;
}


//----------------------------------------------------------------------------

GFXBitmap* ITRTextureCache::getTexture(Handle h)
{
	AssertFatal(isValid(h),"ITRTextureCache::getTexture: Invalid handle");
	Header* header = getHeader(h);
	GFXBitmap* dm = nextBitmap();
	dm->width = header->width + 1;
	dm->height = header->height + 1;
	dm->stride = dm->width;
	dm->imageSize = dm->width * dm->height;
	dm->pBits = (BYTE*) &header->data;
	return dm;
}	


//----------------------------------------------------------------------------
// Builds a texture in the cache.
// Copies the source texture with wrapping, and lights it
// with a constant intensity.
//
ITRTextureCache::Handle ITRTextureCache::buildTexture(Handle handle,
	int sizeX,int sizeY,int offsetX,int offsetY,
	TSMaterial* material,UInt16 color)
{
	if (!isValid(handle))
		handle =  allocate(sizeX * sizeY);
	Header* header = getHeader(handle);
	header->width = sizeX - 1;
	header->height = sizeY - 1;
	header->mipLevel = 0;

	color &= ~((1 << ITRLighting::LightMap::AlphaShift) - 1);
	UInt8* shadeMap = palette->getShadeMap(color);

	const GFXBitmap* srcTex = material->getTextureMap();
	UInt8* srcBits = srcTex->pBits;
	int sxMask = srcTex->width - 1;
	int syMask = srcTex->height - 1;
	int sxShift = getShift(srcTex->stride);
	int sxStart = offsetX;
	int syStart = offsetY;

	UInt8* dp = (UInt8*) &header->data;
	for (int y = 0; y < sizeY; y++) {
		UInt8* sy = srcBits + (((syStart + y) & syMask) << sxShift);
		for (int x = 0; x < sizeX; x++)
			*dp++ = shadeMap[*(sy + ((sxStart + x) & sxMask))];
	}
	return handle;
}


//----------------------------------------------------------------------------
// Builds a texture in the cache.
// Copies the source texture with wrapping, and lights it
// according to the lightmap.
//
ITRTextureCache::Handle ITRTextureCache::buildTexture(Handle handle,
	int sizeX,int sizeY,int offsetX,int offsetY,
	int lightScale,TSMaterial* material,
	LightMap* map,int mipLevel)
{
	if (mipLevel > lightScale)
		mipLevel = lightScale;
	lightScale -= mipLevel;

	int mipRound = mipLevel? 1 << (mipLevel - 1): 0;
	int mipUp = (1 << mipLevel) - 1;
	sizeX = (sizeX + mipUp) >> mipLevel;
	sizeY = (sizeY + mipUp) >> mipLevel;
	offsetX = (offsetX + mipRound) >> mipLevel;
	offsetY = (offsetY + mipRound) >> mipLevel;

	if (!isValid(handle))
		handle =  allocate(sizeX * sizeY);
	Header* header = getHeader(handle);
	header->width = sizeX - 1;
	header->height = sizeY - 1;
	header->mipLevel = mipLevel;

	//
	UInt8* shadeMap = palette->getShadeMap(0);
	int shadeShift = 16 - palette->shadeShift - PAL_SHIFT;

	int lightSize = 1 << lightScale;
	int lightHalf = lightSize >> 1;

	UInt8* dstBits = (UInt8*) &header->data;
	int dstStride = header->width + 1;

	const GFXBitmap* srcTex = material->getTextureMap();
	AssertFatal(mipLevel < srcTex->detailLevels,
		"ITRTextureCache::buildTexture: Bitmap missing mip levels.");
	UInt8* srcBits = srcTex->pMipBits[mipLevel];
	int sxMask = (srcTex->width >> mipLevel) - 1;
	int syMask = (srcTex->height >> mipLevel) - 1;
	int sxShift = getShift(srcTex->stride >> mipLevel);

	UInt16* mapBits = &map->data;
	int mapStride = map->size.x;
	int mapOffsetX = (map->offset.x + mipRound) >> mipLevel;
	int mapOffsetY = (map->offset.y + mipRound) >> mipLevel;
	int alphaMask = ~((1 << ITRLighting::LightMap::AlphaShift) - 1);

	// Scan throught the lightmap..
	for (int y = 0; y < map->size.y - 1; y++) {
		for (int x = 0; x < map->size.x - 1; x++) {
			// Get corner intensity values and stepping
			// values for left and right edge.
			UInt16* mb = mapBits + (y * mapStride) + x;
			int lightLeft = mb[0] & alphaMask;
			int lightLeftStep = ((mb[mapStride] & alphaMask) - 
				lightLeft) >> lightScale;
			int lightRight = mb[1] & alphaMask;
			int lightRightStep = ((mb[mapStride + 1] & alphaMask) - 
				lightRight) >> lightScale;

			// Convert light map coor to texture coor.
			int dx = (x << lightScale) - mapOffsetX - lightHalf;
			int dy = (y << lightScale) - mapOffsetY - lightHalf;
			UInt8* dst = dstBits + (dy * dstStride) + dx;

			// Get Source texture coor.
			int sxStart = dx + offsetX;
			int syStart = dy + offsetY;

			// Presteping and overun control Y
			int startY = 0;
			if (dy < 0) {
				startY = -dy;
				lightLeft += lightLeftStep * startY;
				lightRight += lightRightStep * startY;
				dst += dstStride * startY;
			}
			int endY = lightSize;
			if (dy + endY > sizeY)
				endY = sizeY - dy;

			// Presteping and overun control X
			int startX = 0;
			if (dx < 0) {
				startX = -dx;
				dst += startX;
			}
			int endX = lightSize;
			if (dx + endX > sizeX)
				endX = sizeX - dx;

			// Actual core loops.  Steps left & right edge down
			// and interpolate values across.
			for (int iy = startY; iy < endY; iy++) {
				// Setup for most inner loop
				int lightStep = (lightRight - lightLeft) >> lightScale;
				int light = startX? lightLeft + lightStep * startX: lightLeft;
				UInt8* sy = srcBits + (((syStart + iy) & syMask) << sxShift);
				UInt8* dp = dst;

				// Horizontal scan.
				for (int ix = startX; ix < endX; ix++) {
					*dp++ = shadeMap[
						((light >> shadeShift) & ~((1 << PAL_SHIFT) - 1)) +
						*(sy + ((sxStart + ix) & sxMask))];
					light += lightStep;
				}

				// Y Stepping.
				dst += dstStride;
				lightLeft += lightLeftStep;
				lightRight += lightRightStep;
			}
		}
	}
	return handle;
}
