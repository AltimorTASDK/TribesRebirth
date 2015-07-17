//---------------------------------------------------------------------------



//---------------------------------------------------------------------------

#include <stdlib.h>
#include "grdBlock.h"
#include "grdHeight.h"
#include "grdFile.h"
#include "grdCollision.h"
#include <bitset.h>
#include "console.h"
#include "gOGLSfc.h"

static ResourceTypeGridBlock _resdtb(".dtb");


//#define RANDOM_HEIGHTS
//#define RANDOM_ROTATIONS
//#define RANDOM_LIGHTING

namespace OpenGL {

extern OpenGL::Surface* g_surfaceActive;

} // namespace OpenGL


//---------------------------------------------------------------------------

namespace {

UInt32
getNextPow2(UInt32 size)
{
   int oneCount = 0;
   int shiftCount = -1;
   while(size) {
      if(size & 1)
         oneCount++;
      shiftCount++;
      size >>= 1;
   }
   if(oneCount > 1)
      shiftCount++;
   return (1 << shiftCount);
}

int
getShift(int x)
{
	int i = 0;
	if (x)
		while (!(x & 1))
			i++, x >>= 1;
	return i;
}

} // namespace {}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

GridBlock::GridBlock()
{
	id = 0;
	size.set(0,0);
	detailCount = 0;
	heightMap = 0;
	materialMap = 0;
	lightMap = 0;
   pSubLightmaps   = NULL;
   pSubLMCacheInfo = NULL;
   subLightmapSide = 0;
   hiresLightMap = 0;
   hiresLightMapSize = 0;
   hiresMode = false;
	lightScale = -1;
	heightRange.fMin = heightRange.fMax = .0f;
	nameId[0] = '\0';
   dirtyFlag = true;
	for (int i = 0; i < MaxDetailLevel + 1; i++)
		pinMap[i] = 0;

   m_pCoverBitmap = NULL;
}

GridBlock::~GridBlock()
{
	delete [] heightMap;
	delete [] materialMap;
	delete [] lightMap;

   if (OpenGL::g_surfaceActive != NULL) {
      OpenGL::g_surfaceActive->flushTextureCache();
   }
   deleteSubLightmaps();

   delete m_pCoverBitmap;
   m_pCoverBitmap = NULL;

   delete [] hiresLightMap;
	for (int i = 0; i < MaxDetailLevel + 1; i++)
		delete [] pinMap[i];
}

//---------------------------------------------------------------------------
// Persistent & related routines
IMPLEMENT_PERSISTENT_TAG(GridBlock, FOURCC('G','B','L','K'));

//---------------------------------------------------------------------------

int GridBlock::version()
{
	// 0:  Orignal
	// 1:  RickO Compression
	// 2:  Pin maps
   // 3:  High res light maps
   // 4:  LZH compress: heights, materials and vertex lighting
   // 5:  Alternative hires lightmap support
	return 5;
}


static inline void READ(char *&src, void *dst, int size)  
{
   memcpy(dst, src, size); 
   ((char*)src) += size;
}   

// --------------------------------------------------------------
// helper functions
//   -- a compressed block consists of a size then 
//      followed by the compressed data
void writeCompressedBlock( StreamIO& sio, int size, void * data )
{
   sio.write( size );
   
   // write out the compressed data
   LZHWStream lzStream;
   lzStream.attach( sio );
   lzStream.write( size, data );
   lzStream.detach();
}

void readCompressedBlock( StreamIO& sio, void * data )
{
   int size = 0;
   sio.read( &size );

   LZHRStream lzStream;
   lzStream.attach( sio );
   lzStream.read(size,data);
   lzStream.detach();
}

//---------------------------------------------------------------------------
Persistent::Base::Error GridBlock::read(StreamIO& sio, int version, int)
{
   dirtyFlag = false; // always false after read;

	//read persistent vars
   sio.read(sizeof(nameId), (void *)nameId);
   sio.read(&detailCount);
   sio.read(&lightScale);
   sio.read(sizeof(GridRange<float>), (void *) &heightRange);
   sio.read(sizeof(size), (void *) &size);

	setSize(size);

   if (version == 0)
   {
      AssertWarn(0, "GridBlock::read: Old version, resave GridFile to upgrade");
	   //read in the maps
	   int heightSize = sizeof(Height)*(size.x + 1) * (size.y + 1);
      sio.read(heightSize, (void *) heightMap);
   }
   else if( version < 4 )
   {  // Version 1 compressed block reader
      int rowSize = (size.x+1);
      
      // read TOP row
      sio.read(sizeof(Height) * rowSize, (void *) heightMap);

      // decompress middle rows
      for (int y=1; y<size.y; y++)
      {
         Height *idxT = heightMap + (y*rowSize);;
         Height *idxH = idxT + 1;;
         float scale;
         sio.read(sizeof(scale), (void *) &scale);        // read scale factor
         sio.read(sizeof(Height), (void *) &idxT->height);// read leading height
         for (int x=1; x < size.x; x++, idxT++, idxH++)
         {
            char b;
            sio.read(&b);
            idxH->height = idxT->height + float(b)*scale;
         }
         sio.read(sizeof(Height), (void *) &idxH->height);// read trailing height
      }
      // read in BOTTOM row
      sio.read(rowSize * sizeof(Height), (void *) (heightMap + (size.y * rowSize)));
   }
   else
      readCompressedBlock( sio, (void*)heightMap );

	// Material Map
   if( version < 4 )
   {
	   int matSize = sizeof(Material)*size.x * size.y;
      sio.read(matSize, (void *) materialMap);
   }
   else
      readCompressedBlock( sio, (void*)materialMap );

	// Pin Maps
	if (version >= 2) {
		for (int i = 0; i < MaxDetailLevel+1; i++) {
			UInt16 msize;
         sio.read(&msize);
			if (msize) {
				pinMap[i] = new UInt8[msize];
				sio.read(msize, (void *) pinMap[i]);
			}
		}
	}

	// Light Map
   if (lightScale != -1)                                   
   {                                                       
      if( version < 4 )
      {
   	   int mapWidth = getLightMapWidth();
		   int mapSize = mapWidth * mapWidth * sizeof(UInt16);
         sio.read(mapSize, (void *) lightMap);
      }
      else
         readCompressedBlock( sio, (void*)lightMap );

      buildSubLightmaps();
   }                                                       
   
   // read in the hires lightmap (already compressed)
   if(version > 4)
   {
      sio.read(&hiresLightMapSize);
      if(hiresLightMapSize)
      {
         hiresLightMap = new UInt8[hiresLightMapSize];
         sio.read(hiresLightMapSize, (void*)hiresLightMap);
      }
   }

   // hires stuff   
   hrLightMaps.setHighestDetail( detailCount - 1 );
   if( version >= 3 ){
      // Hi res light maps:
      int   numHRLMs, colorPoolSize, indexTableSize, treeTableSize, versionNumber;
      HiresLightMap  hrlm;
      
      hrLightMaps.clear();
      
      sio.read( &versionNumber );
      
      // Version # added after first tests (though nothing was saved), and there will likely
      //  be no backwards support, just there for development, and we skip the read if 
      //  the version is old.  
      if( versionNumber > 0 ){
         sio.read( &numHRLMs);
      }
      else
         numHRLMs = 0;
         
      if( versionNumber == GridHrlmList::Version )
      {
         sio.read( &colorPoolSize);
         sio.read( &indexTableSize);
         sio.read( &treeTableSize);

         for ( int i = 0; i < numHRLMs; i++ ){
            sio.read(sizeof(hrlm), &hrlm);
            bool could_push = hrLightMaps.push_back ( hrlm );
            AssertFatal ( could_push, "Couldn't push hrlm" );
         }
         
         hrLightMaps.colorPool.setSize ( colorPoolSize );
         hrLightMaps.indexTable.setSize ( indexTableSize );
         hrLightMaps.treeTable.setSize ( treeTableSize );
         
         if ( colorPoolSize )
            sio.read(colorPoolSize * sizeof(UInt16), hrLightMaps.colorPool.address());
         if ( indexTableSize )
            sio.read(indexTableSize * sizeof(UInt8), hrLightMaps.indexTable.address());
         if ( treeTableSize )
            sio.read(treeTableSize * sizeof(GridHrlmList::LNode), hrLightMaps.treeTable.address());

         // if no tree was stored (but there are maps), then build the tree.           
         if ( !treeTableSize && numHRLMs )
            hrLightMaps.BuildQuadTree();
            
         // hrLightMaps.dismantlePool();
      }
   }
   if( ! hrLightMaps.size() )
      doTestFillPattern ();

   return Ok;
}


//---------------------------------------------------------------------------
Persistent::Base::Error GridBlock::write(StreamIO& sio, int, int)
{
	//write persistent vars
	sio.write(sizeof(nameId),(void*)nameId);
	sio.write(detailCount);
	sio.write(lightScale);
	sio.write(sizeof(GridRange<float>), (void*)&heightRange);
	sio.write(sizeof(size), (void*)&size);

   int heightSize = sizeof(Height)*(size.x + 1) * (size.y + 1);
   writeCompressedBlock( sio, heightSize, (void*)heightMap );
   
   int matSize = sizeof( Material ) * size.x * size.y;
   writeCompressedBlock( sio, matSize, (void*)materialMap );

	// Pin Maps
	for (int i = 0; i < MaxDetailLevel+1; i++)
		if (!pinMap[i])
			sio.write(UInt16(0));
		else {
			int msize;
			if (i == 0)
				// The height pin map is calculated differently
				// than the square pin maps.
				msize = getHeightMapWidth();
			else
				// size.x and size.y should be the same.
				msize = size.x >> (i - 1);
            
         // need to add on some extra space - wasted on height pin map... 
         msize = ((msize * msize) >> 3) + ( ( 2 * msize ) >> 3 ) + 1;
         
         // check if no bits were set ( possible because of clearing away pin'd terrain... cheaper
         // to do the check here )
         int j;
         for( j = 0; j < msize; j++ )
            if( pinMap[i][j] )
               break;
         if( j == msize )
         {
            delete [] pinMap[i];
            sio.write( UInt16( 0 ) );
         }
         else
         {
			   sio.write(UInt16(msize));
			   sio.write(msize,(void*)pinMap[i]);
         }
		}
      
	// Light Map
 	if (lightScale != -1)                                   
 	{  
  	   int mapWidth = getLightMapWidth();
	   int mapSize = mapWidth * mapWidth * sizeof(UInt16);

      // write out compressed
      writeCompressedBlock( sio, mapSize, lightMap );
 	}        

   // write out the hires lightmap (already compressed)
   sio.write(hiresLightMapSize);
   if(hiresLightMapSize)
      sio.write(hiresLightMapSize, (void*)hiresLightMap);

   // Hi res light maps:   
   hrLightMaps.compressLightMaps();
   {
      int   versionNumber  =  hrLightMaps.version;
      int   numHRLMs       =  hrLightMaps.size();
      int   colorPoolSize  =  hrLightMaps.colorPool.size();
      int   indexTableSize =  hrLightMaps.indexTable.size();
      int   treeTableSize  =  hrLightMaps.treeTable.size();
      
      sio.write( versionNumber );
      sio.write( numHRLMs );
      sio.write( colorPoolSize );
      sio.write( indexTableSize );
      sio.write( treeTableSize );
      
      sio.write( numHRLMs * sizeof(HiresLightMap), hrLightMaps.address() );
      sio.write( colorPoolSize * sizeof(UInt16), hrLightMaps.colorPool.address() );
      sio.write( indexTableSize * sizeof(UInt8), hrLightMaps.indexTable.address() );
      sio.write( treeTableSize * sizeof(GridHrlmList::LNode), hrLightMaps.treeTable.address() );
   }
   // hrLightMaps.dismantlePool();
 	                                               
   return (sio.getStatus() != STRM_OK)? WriteError: Ok;
}


//---------------------------------------------------------------------------
// creates a GridBlock with data from the named file in the volume
GridBlock* GridBlock::load(const char *blkname, VolumeStream *vol)
{
	AssertFatal(blkname && vol, 
		"GridBlock::load: blockName or Vol is null");

	if (vol->open(blkname))
	{
      Persistent::Base::Error err;
		Persistent::Base* ptr = Persistent::Base::load(*vol, &err);
		vol->close();
		AssertFatal(ptr, "GridBlock::load: file 'blkname' not found");
		GridBlock *gb = dynamic_cast<GridBlock*>(ptr);
		AssertFatal(gb, "GridBlock::load: file 'blkname' not a"
			" valid GridBlock");
		return gb;
	}
	return NULL;
}


//---------------------------------------------------------------------------
// saves the GridBlock data under the given name into the volume
bool GridBlock::save(const char *blkname, VolumeRWStream *vol)
{
	AssertFatal(blkname && vol, "blkname or vol ptr is null");

   // update the height range prior to saving out
   updateHeightRange();

	if (vol->open(blkname, STRM_COMPRESS_NONE, 0))
	{
		store(*vol);
		vol->close();
      dirtyFlag = false;
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

GridBlock::Height* GridBlock::getHeight(int detail, const Point2I& pos)
{
	int index = ((pos.y << detail) * (getHeightMapWidth())) +
		(pos.x << detail);
	return &heightMap[index];
}

GridBlock::Material* GridBlock::getMaterial(int detail, const Point2I& pos)
{
	int index = ((pos.y << detail) * (getMaterialMapWidth())) +
		(pos.x << detail);
	return &materialMap[index];
}

UInt16* GridBlock::getLightMap(int detail, const Point2I& pos)
{
   detail += lightScale;
	int index = ((pos.y << detail) * (getLightMapWidth())) +
		(pos.x << detail);
	return &lightMap[index];
}


//---------------------------------------------------------------------------

bool GridBlock::isHeightPinned(const Height* hp)
{
	register UInt8* pm = pinMap[0];
	if (pm) {
		register int index = hp - heightMap;
		return pm[index >> 3] & (1 << (index & 0x7));
	}
	return false;
}

bool GridBlock::isSquarePinned(int detail, const Point2I& pos)
{
	register UInt8* pm = pinMap[detail+1];
	if (pm) {
		register int index = (pos.y << (detailCount - detail - 1)) + pos.x;
		return pm[index >> 3] & (1 << (index & 0x7));
	}
	return false;
}

// this function now handles clearing of the pin flag as well ( through the SET variable ) - callback
// is used for undo'n in ted
void GridBlock::pinSquare( int detail, Point2I pos, int maxDetail, bool set, 
   pinCallbackInfo * callback )
{
	AssertFatal( detail < MaxDetailLevel,
		"GridBlock::pinSquare: Detail level out of range");
      
	if (!pinMap[detail+1]) {

		// Allocate the memory, +1 on width & height to keep
		// it the same as the height map. *** This size is also
		// calculated on the write method.
		Point2I ss((size.x >> detail),(size.y >> detail));
		int size = ((ss.x * ss.y) >> 3);
      size += ( ( ss.x + ss.y ) >> 3 ) + 1;
		pinMap[detail+1] = new UInt8[size];
		memset(pinMap[detail+1],0,size);

		// The first entry in the pinMap is the flags used
		// for height pinning.  *** This size is also calculated
		// on the write method.
		if (!pinMap[0]) {
			int size = getHeightMapWidth();
			size = ((size * size) >> 3);
         size += ( ( 2 * size ) >> 3 ) + 1;
			pinMap[0] = new UInt8[size];
			memset(pinMap[0],0,size);
		}
	}

	int index = pos.y * (size.x >> detail) + pos.x;
   
   // check if should call callback - on undo/redo will call pinsquare with maxdetail set to detail
   if( callback )
   {
      callback->detail = detail;
      callback->pos = pos;
      callback->val = pinMap[detail+1][ index >> 3 ] & ( 1 << ( index & 0x7 ) );
      callback->func( callback );
   }
      
	// Pin the square
   if( set )
	   pinMap[detail+1][index >> 3] |= 1 << (index & 0x7);
   else
	   pinMap[detail+1][index >> 3] &= ~( 1 << (index & 0x7) );

	// Pin the heights
	int ds = 1 << (detail - 1);
	Point2I inc(ds,ds * getHeightMapWidth());
	Height* hp = getHeight(detail,pos);
	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 3; y++) {
			int index = &hp[x * inc.x + y * inc.y] - heightMap;
         
         // call the undo callback function
         if( callback )
         {
            callback->detail = detail;
            callback->pos = pos;
            callback->val = pinMap[0][ index >> 3 ] & ( 1 << ( index & 0x7 ) );
            callback->func( callback );
         }
         
         if( set )
   			pinMap[0][index >> 3] |= 1 << (index & 0x7);
         else
   			pinMap[0][index >> 3] &= ~( 1 << (index & 0x7) );
		}

	// To pin this square, we need to make sure all the
	// higher up squares are also pinned.
	++detail;
	if (detail < maxDetail && detail < detailCount) {
		Point2I npos = pos;
		npos >>= 1;
		pinSquare(detail,npos,maxDetail,set,callback);
		if (pos.x & 1) {
			if (npos.x < (size.x >> detail) - 1)
				pinSquare(detail,Point2I(npos.x+1,npos.y),maxDetail,set,callback);
		}
		else
			if (npos.x > 0)
				pinSquare(detail,Point2I(npos.x-1,npos.y),maxDetail,set,callback);
		if (pos.y & 1) {
			if (npos.y < (size.y >> detail) - 1)
				pinSquare(detail,Point2I(npos.x,npos.y+1),maxDetail,set,callback);
		}
		else
			if (npos.y > 0)
				pinSquare(detail,Point2I(npos.x,npos.y-1),maxDetail,set,callback);
	}
}

// remove pinned spaces in a certain detail... -1 clears all
void GridBlock::clearPinMaps()
{
   // delete them all
	for( int i = 0; i < MaxDetailLevel + 1; i++ )
   {
		delete [] pinMap[i];
      pinMap[i] = NULL;
   }
}


void
GridBlock::deleteSubLightmaps()
{
   for (UInt32 i = 0; i < subLightmapSide * subLightmapSide; i++) {
      GFXLightMap* pLightmap = pSubLightmaps[i];
      UInt8* pDelete = (UInt8*)pLightmap;
      delete [] pDelete;
   }
   delete [] pSubLightmaps;
   pSubLightmaps   = NULL;
   subLightmapSide = 0;

   delete [] pSubLMCacheInfo;
   pSubLMCacheInfo = NULL;
}

void
GridBlock::buildSubLightmaps()
{
   // NOTE: Big hack on the cache info.  C++ deficiency.  No arguments allowed
   //  to array constructors...

   deleteSubLightmaps();
   if (getLightMapWidth() > 256) {
      AssertFatal(getLightMapWidth() == 257, "Error, can't handle anything but 257x257 yet");

      // We want to break this up into 9 lightmaps of 128x128 size.  First, allocate
      //  those lightmaps...
      //
      subLightmapSide = 2;
      pSubLightmaps = new GFXLightMap*[4];
      pSubLMCacheInfo = new GFXBitmap::CacheInfo[4];
      for (int i = 0; i < 4; i++) {
         pSubLightmaps[i] = (GFXLightMap*)(new UInt8[GFXLightMap::HeaderSize +
                                                     (256 * 256) * 2]);
         new (&pSubLMCacheInfo[i]) GFXBitmap::CacheInfo(0xfffffffd);
      }

      const UInt32 coordSpots[2] = { 0, 1 };

      for (int y = 0; y < 2; y++) {
         for (int x = 0; x < 2; x++) {
            GFXLightMap* pLightMap = pSubLightmaps[y * 2 + x];
            UInt16* pDst           = &(pLightMap->data);

            UInt32 startX = coordSpots[x];
            UInt32 startY = coordSpots[y];

            pLightMap->size.x   = 256;
            pLightMap->size.y   = 256;
            pLightMap->offset.x = 0;
            pLightMap->offset.y = 0;
            pLightMap->index    = 0xff;

            for (UInt32 cy = startY; cy < startY + 256; cy++) {
               for (UInt32 cx = startX; cx < startX + 256; cx++) {
                  pDst[(cy - startY) * 256 + (cx - startX)] =
                     lightMap[cy * getLightMapWidth() + cx];
               }
            }
         }
      }
   } else {
      // Lightmap is perfectly acceptable as is, bump it up to a
      //  power of 2...
      UInt32 widthPad = getNextPow2(getLightMapWidth());
      subLightmapSide = 1;
      pSubLightmaps = new GFXLightMap*[1];

      pSubLightmaps[0] = (GFXLightMap*)(new UInt8[GFXLightMap::HeaderSize +
                                                  (widthPad * widthPad) * 2]);
      pSubLightmaps[0]->size.x   = widthPad;
      pSubLightmaps[0]->size.y   = widthPad;
      pSubLightmaps[0]->offset.x = 0;
      pSubLightmaps[0]->offset.y = 0;

      UInt16* pDest = &(pSubLightmaps[0]->data);
      UInt16* pSrc  = lightMap;
      for (int y = 0; y < getLightMapWidth(); y++) {
         for (int x = 0; x < getLightMapWidth(); x++) {
            pDest[y * widthPad + x] = pSrc[y * getLightMapWidth() + x];
         }
      }

      pSubLMCacheInfo = new GFXBitmap::CacheInfo[1];
      new (pSubLMCacheInfo) GFXBitmap::CacheInfo(0xfffffffd);
   }
}

void
GridBlock::buildCoverBitmap()
{
   if (m_pCoverBitmap != NULL)
      return;

   m_pCoverBitmap = GFXBitmap::create(size.x, size.y);
   TS::Material& rKeyBM = (*materialList)[materialMap[0].index];
   m_pCoverBitmap->paletteIndex = rKeyBM.getTextureMap()->paletteIndex;


   for (int y = 0; y < m_pCoverBitmap->getWidth(); y++) {
      for (int x = 0; x < m_pCoverBitmap->getHeight(); x++) {
         UInt8* pBit = (UInt8*)m_pCoverBitmap->getDetailLevel(0) +
                       (y * m_pCoverBitmap->getStride()) + x;

         TS::Material& rMaterial = (*materialList)[materialMap[y * size.x + x].index];
         const GFXBitmap* pBitmap = rMaterial.getTextureMap();
         if (pBitmap == NULL) {
            *pBit = 0;
         } else {
            *pBit = *(pBitmap->getDetailLevel(pBitmap->getNumDetailLevels() - 1));
         }
      }
   }
}

//---------------------------------------------------------------------------

void GridBlock::setSize(Point2I ss)
{
	delete [] heightMap;
	delete [] materialMap;
	delete [] lightMap;
	lightMap = 0;

   deleteSubLightmaps();

	size = ss;
	Point2I shift;
	shift.x = getShift(ss.x);
	shift.y = getShift(ss.y);
	AssertFatal((1 << shift.x) == size.x && (1 << shift.y) == size.y,
			"GridBlock::setSize: Size must be power of 2");
	AssertFatal(shift.x == shift.y,
			"GridBlock::setSize: Size must be square");
	detailCount = shift.x + 1;

	// Allocate height map
	int heightSize = (size.x + 1) * (size.y + 1);
	heightMap = new Height[heightSize];
	memset(heightMap,0,heightSize * sizeof(Height));

#ifdef RANDOM_HEIGHTS
	// Random heights
	for ( i=0; i<heightSize; i++ )
		heightMap[i].height = (float(rand()) / RAND_MAX) * 200;
	updateHeightRange();
#endif

	// Allocat material map
	int matSize = size.x * size.x;
	materialMap = new Material[matSize];
	memset(materialMap,0,matSize * sizeof(Material));

#ifdef RANDOM_ROTATIONS
	// Random rotations
	for ( i=0; i<matSize; i++ )
		materialMap[i].flags = int((float(rand()) / RAND_MAX) * 8) & 0x7;
#endif

	// Update other data dependant on size
	if (lightScale != -1)
		setLightScale(lightScale);
}


//---------------------------------------------------------------------------

void GridBlock::setLightScale(int ls)
{
	AssertFatal(size.x != 0 && size.y != 0,
		"GridBlock::setLightScale: Must set map size first");
	lightScale = ls;
	delete lightMap;
	int mapWidth = getLightMapWidth();
	int mapSize = mapWidth * mapWidth;
	lightMap = new UInt16[mapSize];

	// Light map defaults to full intensity
	memset(lightMap,-1,mapSize * sizeof(UInt16));

   buildSubLightmaps();

#ifdef RANDOM_LIGHTING
	// Random lighting
	for (int i = 0; i < mapSize; i++)
		lightMap[i] = (rand() & 1)? -1: 0;
#endif
}


//---------------------------------------------------------------------------

void GridBlock::updateHeightRange()
{
	// Scan through the height map and update the height range.
	heightRange.fMin = +1.0E20f;
	heightRange.fMax = -1.0E20f;
	int width = getHeightMapWidth();
	Height* hp = heightMap;
	Height* ep = hp + width * width;
	for (; hp != ep; hp++) {
		heightRange.fMin = min(hp->height,heightRange.fMin);
		heightRange.fMax = max(hp->height,heightRange.fMax);
	}
}


//---------------------------------------------------------------------------

bool GridBlock::getSurfaceInfo(int scale,const Point2F& pos,CollisionSurface* info)
{
	// Determin which square it's on
	Point2I spos;
	spos.x = int(pos.x) >> scale;
	spos.y = int(pos.y) >> scale;

	if (spos.x >= 0 && spos.x < size.x && spos.y >= 0 && spos.y < size.y) {
		Material* material = getMaterial(0,spos);
		// Make sure the the square is not empty space.
		if (!material->getEmptyLevel()) {
			// Use the height struct to extract info
			GridHeight ht;
			ht.width = float(1 << scale);
			ht.heights = getHeight(0,spos);
			ht.split = ((spos.x ^ spos.y) & 1)? GridHeight::Split135:
				GridHeight::Split45;
			ht.dx = 1;
			ht.dy = getHeightMapWidth();

			Point2F hpos;
			hpos.x = pos.x - float(spos.x << scale);
			hpos.y = pos.y - float(spos.y << scale);

			info->position.x = pos.x;
			info->position.y = pos.y;
			info->position.z = ht.getHeight(hpos);
			ht.getNormal(hpos,&info->normal);
			info->normal.normalize();
			info->material = int(&(*materialList)[material->index]);
			info->distance =  0.0f;
			info->surface = 0;
			info->time = 1.0f;
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------------

bool GridBlock::getSurfaceHeight(int scale,const Point2F& pos,float *height, bool ignoreEmptyTiles)
{
	// Determin which square it's on
	Point2I spos;
	spos.x = int(pos.x) >> scale;
	spos.y = int(pos.y) >> scale;

	if (spos.x >= 0 && spos.x < size.x && spos.y >= 0 && spos.y < size.y)
   {
      if(!ignoreEmptyTiles)
      {
		   Material* material = getMaterial(0,spos);
   		// Make sure the the square is not empty space.
   		if (material->getEmptyLevel())
            return false;
      }
      
		// Use the height struct to extract info
		GridHeight ht;
		ht.width = float(1 << scale);
		ht.heights = getHeight(0,spos);
		ht.split = ((spos.x ^ spos.y) & 1)? GridHeight::Split135:
			GridHeight::Split45;
		ht.dx = 1;
		ht.dy = getHeightMapWidth();

		Point2F hpos;
		hpos.x = pos.x - float(spos.x << scale);
		hpos.y = pos.y - float(spos.y << scale);

		*height = ht.getHeight(hpos);
		return true;
	}
	return false;
}

void GridBlock::getPolys(int scale,const Box2F & box,Point2F & blockOffset,
                         const TMat3F & transBack, TS::PolyList & pl, int maxPolys)
{
   if (pl.size()>=maxPolys)
      return;

	Point2I minS,maxS;
	int minSx,minSy,maxSx,maxSy;
	minSx = int(box.fMin.x) >> scale;
	minSy = int(box.fMin.y) >> scale;
	maxSx = int(box.fMax.x) >> scale;
	maxSy = int(box.fMax.y) >> scale;

	Point2I spos;
	for (spos.x=minSx;spos.x<=maxSx;spos.x++)
		for (spos.y=minSy;spos.y<=maxSy;spos.y++)
			if (spos.x>=0 && spos.x<size.x && spos.y>=0 && spos.y<size.y)
			{
				// if the square is empty, skip these polys
				Material* material = getMaterial(0,spos);
				if (material->getEmptyLevel())
               continue;

				// get 4 corners and put into original space
				Point3F pointLL,pointLR,pointUL,pointUR,tmp;

				tmp.x = float(spos.x << scale) + blockOffset.x;
				tmp.y = float(spos.y << scale) + blockOffset.y;
				tmp.z = getHeight(0,spos)->height;
				m_mul(tmp,transBack,&pointLL);

				spos.x++;
				tmp.x = float(spos.x << scale) + blockOffset.x;
				tmp.y = float(spos.y << scale) + blockOffset.y;
				tmp.z = getHeight(0,spos)->height;
				m_mul(tmp,transBack,&pointLR);

				spos.y++;
				tmp.x = float(spos.x << scale) + blockOffset.x;
				tmp.y = float(spos.y << scale) + blockOffset.y;
				tmp.z = getHeight(0,spos)->height;
				m_mul(tmp,transBack,&pointUR);

				spos.x--;
				tmp.x = float(spos.x << scale) + blockOffset.x;
				tmp.y = float(spos.y << scale) + blockOffset.y;
				tmp.z = getHeight(0,spos)->height;
				m_mul(tmp,transBack,&pointUL);

				spos.y--; // back to LL

				if ((spos.x ^ spos.y) & 1) // GridHeight::Split135
				{
					// poly: LL, UL, LR
					pl.increment();
					pl.last() = new TS::Poly;
					pl.last()->set(pointLL,pointUL,pointLR);

               if (pl.size()>=maxPolys)
                  return;

					// poly: LR, UL, UR
					pl.increment();
					pl.last() = new TS::Poly;
					pl.last()->set(pointLR,pointUL,pointUR);
				}
				else // GridHeight::Split45
				{
					// poly: LL, UR, LR
					pl.increment();
					pl.last() = new TS::Poly;
					pl.last()->set(pointLL,pointUR,pointLR);

               if (pl.size()>=maxPolys)
                  return;

					// poly: LL, UL, UR
					pl.increment();
					pl.last() = new TS::Poly;
					pl.last()->set(pointLL,pointUL,pointUR);
				}
            if (pl.size()>=maxPolys)
               return;
			}

	// special case -- if whole box is in only one square, check to see if we
	// can get rid of one of the polys
	if (minSx==maxSx && minSy==maxSy && pl.size()==2)
	{
		if ((minSx ^ minSy) & 1) // GridHeight::Split135
		{
			// LL and UR corners of box..on same side of negative diagonal?
			Point2F LL,UR;
			LL.x = box.fMin.x - float(minSx << scale);
			LL.y = box.fMin.y - float(minSy << scale);
			UR.x = box.fMax.x - float(minSx << scale);
			UR.y = box.fMax.y - float(minSy << scale);
			float w = float (1 << scale);
			if ( (w-LL.x)>=LL.y && (w-UR.x)>=UR.y ) // both below diagonal
			{
				// get rid of second entry
				delete pl[1];
				pl.decrement();
			}
			else if ( (w-LL.x)<=LL.y && (w-UR.x)<=UR.y ) // both above diagonal
			{
				// get rid of first entry
				delete pl[0];
				pl[0]=pl[1];
				pl.decrement();
			}
		}
		else // GridHeight::Split45
		{
			// UL and LR on same side of positive diagonal?
			Point2F UL,LR;
			UL.x = box.fMin.x - float(minSx << scale);
			UL.y = box.fMax.y - float(minSy << scale);
			LR.x = box.fMax.x - float(minSx << scale);
			LR.y = box.fMin.y - float(minSy << scale);
			if ( UL.x<=UL.y && LR.x<=LR.y ) // both above diagonal
			{
				// get rid of first entry
				delete pl[0];
				pl[0]=pl[1];
				pl.decrement();
			}
			else if ( UL.x>=UL.y && LR.x>=LR.y ) // both below diagonal
			{
				// get rid of second entry
				delete pl[1];
				pl.decrement();
			}
		}
	}
}


//---------------------------------------------------------------------------

GridBlock::Material *GridBlock::getMaterial(int scale, int detail, 
   const Point2F& pos)
{
	// Determin which square it's on
	Point2I spos;
	spos.x = int(pos.x) >> scale;
	spos.y = int(pos.y) >> scale;

	if (spos.x >= 0 && spos.x < size.x && spos.y >= 0 && spos.y < size.y) 
		return getMaterial(detail, spos);
   
   return NULL;
}
	
//---------------------------------------------------------------------------

void GridBlock::buildLightMapTest(GridFile *file,TSSceneLighting *sceneLighting,
	Point2F groundOrigin  )
{
	int scale = file->getScale();
	int mapWidth = getLightMapWidth();
	int mapSize = mapWidth * mapWidth;

	// If there are no lights passed in, just do the plaid pattern
	if(!sceneLighting) {
		// Build lightmap test pattern
		for (int i = 0; i < mapSize; i++)
			lightMap[i] = (i & 1)? 0: -1;
	}
	else {
		int i, j;
		int tscale = (mapWidth - 1) / size.x;
		// get the shift value for the map coords.
		int mapShift = 0;
		while((tscale >>= 1))
			mapShift++;
		//
		TS::Vertex v;
      GridCollision coll(file, 0);
		for(j = 0; j < mapWidth; j++) {
			for(i = 0; i < mapWidth; i++) {
            int gridWidth = size.x + 1;
				int x = i >> mapShift, y = j >> mapShift;
      		Height *hptr = heightMap + y * gridWidth + x;
				v.fPoint.x = groundOrigin.x + float((i << scale) >> mapShift);
				v.fPoint.y = groundOrigin.y + float((j << scale) >> mapShift);
				v.fPoint.z = hptr->height;

				// size of square * 2 (since x and y are sums - that
				// way we don't have to divide them by two.
				v.fNormal.z = 1 << (scale + 1);
			
				// Build vertex normal by averaging adjacent heights
				// (looks at adjacent heights in current detail level,
				// so it may skip lower detail heights).
				v.fNormal.x = x > 0 ? (hptr - 1)->height : hptr->height;
				v.fNormal.x -= x < size.x ? (hptr + 1)->height : hptr->height;
				v.fNormal.y = y > 0 ? (hptr - size.x - 1)->height : hptr->height;
				v.fNormal.y -= y < size.y ? (hptr + size.x + 1)->height : hptr->height;

				// Unfortunalty, normal comes out unnormalized.
				v.fNormal.normalizef(1.0f);
				
				// now calculate the color.
				ColorF col;
				ColorF vi = sceneLighting->getAmbientIntensity();

				//sceneLighting->calcIntensity(v, &col);
				
				TSSceneLighting::const_iterator light;

            for(light = sceneLighting->begin(); light !=
                sceneLighting->end(); light++)
            {
               Point3F start;
               start = v.fPoint;
               start.z += .5;
               bool visible = true;
               TSLight::LightInfo *l = &((*light)->fLight);
               if(l->fType == TSLight::LightDirectional)
               {
                  Point3F end = l->fWAim;
                  end *= -1000;
                  end += start;
                  
                  visible = !coll.collide(start, end);
               }

               if(visible) 
                  (*light)->calcIntensity(v, &vi);
            }
				TSColor::saturate (vi, &col);
				
				
				
				float inten = col.red * .22 + col.green * .67 + col.blue * .11;
				if(inten > 1.0f)
					inten = 1.0f;
				int c = int(inten * 65535) & 0xF000;
				c |= int(col.red * 15.9) << 8;
            c |= int(col.green * 15.9) << 4;
				c |= int(col.blue * 15.9);
				lightMap[j * mapWidth + i] = c;
			}
		}

	}

   buildSubLightmaps();
}


//---------------------------------------------------------------------------





// Fill in  SRC  i  i  i  SRC  i  i  i  SRC  i  i  i  SRC  i  ....  
// where the i values are interpolated from the source data.  The number of 
// interpolations is (2^shift-1).  See ts_color.h and tNTuple.h for defn
// of TSExpandedInten16.  This creates only expanded data in the supplied 
// array which is assumed to be large enough.  
void GridBlock::lmExpand ( TSExpandedInten16 * expandedColors, 
         const UInt16 * ls, int srcInc, 
         int N, int shift )
{
   UInt16   cur = *ls;
   TSExpandedInten16  curColor ( cur );
   *expandedColors++ = curColor;
   while ( N-- )
   {
      UInt16   next = *(ls += srcInc);
      TSExpandedInten16  nextColor ( next );
      
      if ( cur != next )  {
         lmInterpolate ( expandedColors, curColor, nextColor, shift );
         expandedColors += ((1 << shift) - 1);
         cur = next;
      }
      else 
         for ( int i = (1<<shift)-1; i > 0; i-- )
            *expandedColors++ = curColor;

      *expandedColors++ = (curColor = nextColor);
   }
}


// Fill in 2^shift-1 interpolated values between begin and end.  
void GridBlock::lmInterpolate ( TSExpandedInten16 * expandedColors, 
         TSExpandedInten16 begin, TSExpandedInten16 end,
         int shift )
{
   end -= begin;
   end >>= shift;
   for ( int i = (1<<shift)-1;  i > 0; i-- )
      *expandedColors++ = (begin += end);
}

// Collapse the array of N expanded colors back into packed ones.  Within the 
// loop the operator UInt16() cast performs the collapse on each element.  
void GridBlock::lmCollapse ( const TSExpandedInten16 * expandedColors, 
         UInt16 * ld, int dstInc, 
         int N )
{
   while ( N-- )
      *ld  = UInt16 (*expandedColors++ ), ld += dstInc;
}



#define  MaxLightMapWidth  128
#define  MaxLightLevelInc  4
#define  MaxExpandCells    64

// This is our workhorse routine that gives us an expanded lightmap for a square 
// which has hi res level-0 squares contained somewhere within.  This does what 
// the normal lo res expander does, but then inserts the high res data for those 
// hi-res squares.  To understand this routine - first understand the Low Res 
// expander - then this one just adds in high resolution square management.  
//
// We're using the step data, but actually this routine assumes the data is just
// flipped in y - basically a square with level>0.  So we step through the squares
// in reverse.  

void GridBlock::expandHRLMSquare ( UInt16 *lmapData, 
            const Point2I & pos, int level, int levelInc,  
            const Step & src
            ) 
{
   TSExpandedInten16    downwardExpand [ 1 << MaxLightLevelInc ];
   TSExpandedInten16    expCur[MaxLightMapWidth+1], expNxt[MaxLightMapWidth+1];
   TSExpandedInten16    *rowExpandCur  = expCur, *temp;
   TSExpandedInten16    *rowExpandNext = expNxt;

   Int16    rowHRList[MaxExpandCells];
   int      ind;
   Point2I  side = pos;
   UInt16   *ld = lmapData, *ls = src.Data;
   int      numThisRow = 0;
   int      numCells = (1 << level);
   int      sourceSize = numCells + 1;
   int      destSize = (numCells << levelInc) + 1;
   int      nBetween = (1 << levelInc) - 1;
   
   int      numHiDefs = hrLightMaps.getSubSquareList ( pos, level );
   int      remainingHiDefs = numHiDefs;
   
   int   srcRowAdd = (src.Inc * sourceSize) + src.Adj;
   int   dstRowAdd = destSize << levelInc;
   side <<= level;
   
   memset( rowHRList, 0xff, numCells * sizeof(Int16) );
   
   AssertFatal ( numHiDefs > 0, "should exist HRLMs" );
   AssertFatal ( numCells <= MaxExpandCells, "bad EXPAND param" );
   AssertFatal ( levelInc && levelInc <= MaxLightLevelInc, "levinc" );

   side.y += (numCells-1);
   lmExpand ( rowExpandCur, ls, src.Inc, numCells, levelInc );
   lmCollapse( rowExpandCur, ld, 1, destSize );
	for( int y = numCells; y > 0; y--, ld += dstRowAdd, side.y-- ){
      lmExpand ( rowExpandNext, ls += srcRowAdd, src.Inc, numCells, levelInc );
      lmCollapse( rowExpandNext, ld + dstRowAdd, 1, destSize );

      // Get a list of the high res light maps for this row.
      if ( remainingHiDefs )
         for( int i = 0; i < numHiDefs; i++ )
            if ( hrLightMaps[ind = hrLightMaps.hiDefList[i]].pos.y == side.y ){
               int   xIdx = hrLightMaps[ind].pos.x - side.x;
               AssertFatal( xIdx>=0 && xIdx<MaxExpandCells, "BadXIDX" );
               rowHRList[ xIdx ] = ind;
               numThisRow++;
            }
      
      // If we have high defs in this row, then we do a square by square loop, seeing
      //    if each square is in our list.  Note that we only reset variables if 
      //    we know they were changed.  Also, left-right order important: probably
      //    want the high res maps to overwrite interpolated edges.  
      if ( numThisRow ){
         lmInterpolate( downwardExpand, rowExpandCur[0], rowExpandNext[0], levelInc );
         lmCollapse( downwardExpand, ld + destSize, destSize, nBetween );
   		for( int xOut = 0; xOut < numCells; xOut++ ){
            int   dataOff = (xOut << levelInc);
            if(  (ind = rowHRList[xOut])  >=  0  ){
               // hrLightMaps[ ind ].fetchLightMap ( ld + dataOff, destSize, levelInc );
               hrLightMaps.fetchLightMap ( ld + dataOff, destSize, levelInc, ind );
               rowHRList[ xOut ] = -1;  //reset to null 
            }
            else{//do regular interpolation on this square.
               int   xLast = dataOff + nBetween + 1;
         		for ( int x = dataOff + 1; x <= xLast ; x++ ){
                  lmInterpolate ( downwardExpand, rowExpandCur[x], 
                        rowExpandNext[x], levelInc );
                  lmCollapse ( downwardExpand, ld + x + destSize, destSize, nBetween );
               }
            }
         }
         remainingHiDefs -= numThisRow;
         numThisRow = 0;
         AssertFatal(remainingHiDefs>=0,"BadJibe");
      }
      else//regular downward expansion for entire row of cells.
   		for( int x = destSize - 1; x >= 0; x-- ){
            lmInterpolate ( downwardExpand, 
                  rowExpandCur[x], rowExpandNext[x], levelInc );
            lmCollapse ( downwardExpand, ld + x + destSize, destSize, nBetween );
         }
      
      temp = rowExpandCur;
      rowExpandCur = rowExpandNext;
      rowExpandNext = temp;
	}//for each row of squares
   AssertFatal( remainingHiDefs == 0, "EvenOut(Not)");
}



void GridBlock::expandLowResData ( UInt16 *lmapData, 
            int level, int levelInc, const Step & step ) 
{
   TSExpandedInten16    downwardExpand [ 1 << MaxLightLevelInc ];
   TSExpandedInten16    expCur[MaxLightMapWidth+1], expNxt[MaxLightMapWidth+1];
   TSExpandedInten16    *rowExpandCur  = expCur, *temp;
   TSExpandedInten16    *rowExpandNext = expNxt;
   int      sourceSize = (1 << level) + 1;
   int      destSize = ((step.Size - 1) << levelInc) + 1;

   int srcRowAdd = (step.Inc * sourceSize) + step.Adj;
   int dstRowAdd = destSize << levelInc;
   
   UInt16 * ld = lmapData, *ls = step.Data;
   lmExpand ( rowExpandCur, step.Data, step.Inc, sourceSize-1, levelInc );
	for (int y = sourceSize - 1; y > 0; y--, ld += dstRowAdd)   {
      lmExpand ( rowExpandNext, ls += srcRowAdd, step.Inc, sourceSize-1, levelInc );

      // Interpolate all the new rows between two expanded ones.   After 
      // interpolating, then collapse.  
		for ( int x = destSize - 1; x >= 0; x-- ) {
         lmInterpolate ( downwardExpand, 
               rowExpandCur[x], rowExpandNext[x], levelInc );
         lmCollapse ( downwardExpand, ld + x + destSize, destSize, (1<<levelInc)-1 );
      }
      // collapse cur row, and then make the next one into the current row
      // for the next iteration of the loop, if any.  
      lmCollapse ( temp = rowExpandCur, ld, 1, destSize );
      rowExpandCur = rowExpandNext;  
      rowExpandNext = temp;
	}
   // collapse our last row (pointed to by cur from our last swap).  
   lmCollapse ( rowExpandCur, ld, 1, destSize );
}


GridBlock::Step GridBlock::howToStep ( int srcWidth, int sourceSize, 
      UInt16 * ls, int flags )
{
   Step  step;
   
   step.Size = sourceSize;
	step.Inc = 1;
   step.Adj = 0;
   step.Data = ls;

	switch (flags & Material::RotateMask) {
		case Material::Plain:
			step.Data += srcWidth * (sourceSize - 1);
			step.Adj = -(srcWidth + sourceSize);
			break;
		case Material::FlipX:
			step.Data += srcWidth * (sourceSize - 1) + sourceSize - 1;
			step.Inc = -1;
			step.Adj = -srcWidth + sourceSize;
			break;
		case Material::FlipY:
			step.Adj = srcWidth - sourceSize;
			break;
		case Material::Rotate:
			step.Data += srcWidth * (sourceSize - 1) + (sourceSize - 1);
			step.Inc = -srcWidth;
			step.Adj = srcWidth * sourceSize - 1;
			break;
		case Material::FlipX | Material::FlipY:
			step.Data += sourceSize - 1;
			step.Inc = -1;
			step.Adj = sourceSize + srcWidth;
			break;
		case Material::FlipX | Material::Rotate:
			step.Data += sourceSize - 1;
			step.Inc = srcWidth;
			step.Adj = -srcWidth * sourceSize - 1;
			break;
		case Material::FlipY | Material::Rotate:
			step.Data += srcWidth * (sourceSize - 1);
			step.Inc = -srcWidth;
			step.Adj = srcWidth * sourceSize + 1;
			break;
		case Material::FlipX | Material::FlipY | Material::Rotate:
			step.Inc = srcWidth;
			step.Adj = -srcWidth * sourceSize + 1;
			break;
	}
   return step;
}


void GridBlock::dumpLowResGrid ( UInt16 *ld, const Step & step )
{
   UInt16   *ls = step.Data;
   for (int y = step.Size; y > 0; y--) {
      for (int x = step.Size; x > 0; x--) 
         *ld++ = *ls, ls += step.Inc;
      ls += step.Adj;
   }
}



#define  COLOR_SQUARE_TEST    0


// Add this data to the high res light map pool.  
bool GridBlock::addHRLM ( UInt16 *data, const Point2I & pos )
{
   HiresLightMap  hrlm( pos, HiresLightMap::Square17x17, data );
   
   // return true;
   
   UInt16   downFrom = HiresLightMap::Square17x17;
   UInt16   downward [ HiresLightMap::MaxSquareSize ];
   while( downFrom )
   {
      UInt16 * sourceData = hrlm.getDataPointer( downFrom );
      AssertFatal( sourceData, "grdBlock: no source data" );
      hrlm.averageDown( downward, sourceData, downFrom );
      hrlm.setSquareData( downward, --downFrom );
   }

#  if (COLOR_SQUARE_TEST == 3) && 0
   {
      // This removes SOME of the averaged down versions for testing.  We may want to 
      // actually do this for those averaged down versions which are "close" to 
      // the original (having a certain minimum max variation, and havig a total color
      // sum that is close to the 17x17).  Note the 17x17 is required to be present. 
      static UInt16  cycle;
      
      UInt16  allLowerMaps = (1 << HiresLightMap::MaxWidthShift) - 1;
      // hrlm.axeSquareData( cycle++ & allLowerMaps );
      hrlm.axeSquareData( allLowerMaps );
   }
#  endif
   
   // Gather stats here?  
   
   return  hrLightMaps.push_back( hrlm ) != NULL;
}

// Add a single color hrlm.  
bool GridBlock::addHRLM ( UInt16 singleColor, const Point2I & pos )
{
   HiresLightMap  hrlm ( pos, singleColor );
   return  hrLightMaps.push_back( hrlm ) != NULL;
}



//---------------------------------------------------------------------------
//       Miscellaneous tests for high res light maps.  



int GridBlock::doTestFillPattern( void )
{
#if   _DEBUG_LIGHT_DUMP  

#if (COLOR_SQUARE_TEST == 3)

// Fill up our table with random stuff.  

   #define  Il       0xFFFF
   #define  mm       0x8888
   #define  DD       0x1111
   
   UInt16  test1[17*17] = {
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,DD,
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,DD,DD,
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,DD,DD,DD,
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,DD,DD,DD,DD, 
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,DD,DD,DD,DD,DD, 
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,DD,DD,DD,DD,DD,DD, 
   Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,DD,DD,DD,DD,DD,DD,DD, 
   Il,Il,Il,Il,Il,Il,Il,Il,DD,DD,DD,DD,DD,DD,DD,DD,DD, 
   Il,Il,Il,Il,Il,Il,Il,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD, 
   Il,Il,Il,Il,Il,Il,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD, 
   Il,Il,Il,Il,Il,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD, 
   Il,Il,Il,Il,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD, 
   Il,Il,Il,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,
   Il,Il,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD, 
   Il,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD,DD, 
   };
   
   UInt16  test2[17*17] = {
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,
   Il,Il,Il,Il,Il,mm,DD,DD,DD,DD,DD,mm,Il,Il,Il,Il,Il,
   Il,Il,Il,mm,DD,DD,Il,Il,Il,Il,Il,DD,DD,mm,Il,Il,Il,
   Il,Il,mm,DD,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,mm,Il,Il,
   Il,Il,DD,Il,Il,DD,DD,mm,Il,mm,DD,DD,Il,Il,DD,Il,Il,
   Il,mm,DD,Il,DD,DD,DD,DD,Il,DD,DD,DD,DD,Il,DD,mm,Il,
   Il,DD,Il,Il,DD,DD,DD,DD,Il,DD,DD,DD,DD,Il,Il,DD,Il,
   Il,DD,Il,Il,mm,DD,DD,mm,Il,mm,DD,DD,mm,Il,Il,DD,Il,
   Il,DD,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,Il,
   Il,DD,Il,DD,DD,Il,Il,Il,Il,Il,Il,Il,DD,DD,Il,DD,Il,
   Il,DD,Il,mm,DD,DD,Il,Il,Il,Il,Il,mm,DD,mm,Il,DD,Il,
   Il,mm,DD,Il,mm,DD,DD,DD,Il,Il,DD,DD,mm,Il,DD,mm,Il,
   Il,Il,DD,Il,Il,Il,mm,DD,DD,DD,DD,mm,Il,Il,DD,Il,Il,
   Il,Il,mm,DD,Il,Il,Il,Il,Il,Il,Il,Il,Il,DD,mm,Il,Il,
   Il,Il,Il,mm,DD,DD,mm,Il,Il,Il,mm,DD,DD,mm,Il,Il,Il,
   Il,Il,Il,Il,Il,mm,DD,DD,DD,DD,DD,mm,Il,Il,Il,Il,Il,
   Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,Il,
   };
   
   
   UInt16  test3[17*17];
   UInt16  test4[17*17];
   
   for( int x = 0; x < 17*17; x++ ){         //    test:
      test3[x] = x * (65535 / 289);           // straight dump compression
      test4[x] = (x & 63) * (65535 / 63);    // byte pack compression
   }
   
   UInt16         *tests[] = {   test1, test2, test1, 
                                 test2, test1, test3, test4 };
   UInt16         colors[4] = {  0xFFFF, 0xA1E4, 0xEEEE, 0xDDD3 };
   UInt16         *data;
   int            whichTest = 0, doSolidSometimes = 0;

   int            worldW = (1 << detailCount-1);
   Point2I        center ( worldW >> 1, worldW >> 1 ); 
   int            w = (worldW >> 1) - 5, stepAmt;
   if ( detailCount < 7 )
      stepAmt = 4;
   else
      stepAmt = 1 << (detailCount - 7);      // -7.., or -6 is sparser
      
   int            gmask = worldW - 1;
   UInt16         col, buff[17*17];
   UInt16         randomSolid = 1129;     // prime
   
   for ( int x = center.x - w; x < center.x + w; x += stepAmt )
      for ( int y = center.y - w; y < center.y + w; y += stepAmt )
         if ( (x ^ y) & 0xf ) {
            Point2I  pos ( x & gmask, y & gmask );
            data = tests[ whichTest % (sizeof(tests)/sizeof(tests[0])) ];
            col = colors[ 0 ];
            
            for ( int i = 0; i < 17*17; i++ )
               buff[i] = data[i] & col;

            // Test:
            //   -The averager should create smaller versions all of the same color,
            //   -The compressor should detect one color and pack.  
            if( ! (++doSolidSometimes & 0xff) )
               memset( buff, 0xff, sizeof(buff) );
               
            addHRLM ( buff, pos );
            
            randomSolid += 5005; // relatively prime to above and to 2.  

               // surround the high res ones with a random solid color.  
            if( pos.x > 0 && pos.y > 0 && pos.x < worldW-1 && pos.y < worldW-1 )
               for( int ud = -1; ud <= 1; ud++ )
                  for( int lr = -1; lr <= 1; lr++ )
                     if( (lr || ud) && (lr + ud <= 0 ) ) {
                        Point2I  newPoint( ud, lr );
                        newPoint += pos;
                        addHRLM ( randomSolid, newPoint );
                     }
                     
            whichTest++;
         }//if

   hrLightMaps.BuildQuadTree ();
   
   hrLightMaps.compressLightMaps ();
   
   return 0;
#else
   return 0;
#endif
#endif
}

//   ^^^ Test code ^^^
//---------------------------------------------------------------------------


// 
// Build a light map for the give square (pos and level give square), expanding if 
// levelInc specifies to do so.  The caller passes in the light map destination data 
// which is gauranteed to be large enough to hold data for a square 
// of width (1 << level + levelInc) + 1.  
//
UInt16 * GridBlock::fillLightMap ( UInt16 *lmapData, 
            const Point2I & pos, int level, 
            int levelInc, int flags, bool hasHighRes )
{
   AssertFatal ( levelInc <= MaxLightLevelInc, "bad levinc" );
   
   Step     step;

   if ( hasHighRes && level == 0 ){
      int   ind = hrLightMaps.findHighResNode ( pos, 0 );
      AssertFatal ( ind, "grd: ind" );
      HiresLightMap & h = hrLightMaps[ ind >> 4 ];
      
      if( h.resolutions == BIT(HiresLightMap::Square1x1) )
      {
         for( int i = 0; i < 17*17; i++ )
            lmapData[ i ] = h.color.Col;
      }
      else
      {
         // UInt16   * dataPtr = h.color.Ptr;
         // if( h.compression == HiresLightMap::ColorDump )
         //    dataPtr = & hrLightMaps.colorPool [ h.color.Idx ];
         // step = howToStep ( 17, 17, dataPtr, flags );
         // dumpLowResGrid( lmapData, step );
         UInt16   * ptr = hrLightMaps.getHrlmData( h, HiresLightMap::Square17x17 );
         step = howToStep ( 17, 17, ptr, flags );
         dumpLowResGrid( lmapData, step );
      }
   }
   else
   {
      step = howToStep ( getLightMapWidth(), (1 << level) + 1, 
                           getLightMap(level, pos), flags );
      
      if( levelInc != 0 ){
         if ( hasHighRes )
            expandHRLMSquare ( lmapData, pos, level, levelInc, step );
         else
            expandLowResData ( lmapData, level, levelInc, step );
      }
      else
   	   dumpLowResGrid ( lmapData, step );
   }
	return lmapData;
}

//---------------------------------------------------------------------------

void GridBlock::enableHires(bool hires)
{
   // check if not changing anything
   if(!(hiresMode ^ hires) || !hiresLightMap)
      return;
      
   MemRWStream stream(hiresLightMapSize, hiresLightMap);
   
   LZHRStream lzStream;
   lzStream.attach(stream);

   // read in the info
   int size = getLightMapWidth() * getLightMapWidth();
   UInt16 * data = new UInt16[size];
   lzStream.read(size * sizeof(UInt16),data);
   lzStream.detach();
   
   // adjust the lightmap
   for(int i = 0; i < size; i++)
      lightMap[i] ^= data[i];

   hiresMode = hires;
         
   delete [] data;
}

//---------------------------------------------------------------------------
// the map passed in is a full uncompressed lightmap
void GridBlock::setHiresLightMap(UInt16* map)

{
   AssertFatal(map, "GridBlock::setHiresLightMap - map is null");
   if(hiresLightMap)
      delete [] hiresLightMap;
   hiresLightMap = NULL;
   
   // xor the contents.. lots of zeros we hope
   int size = getLightMapWidth() * getLightMapWidth();
   for(int i = 0; i < size; i++)
      map[i] ^= lightMap[i];

   // adjust the size to bytes
   size *= sizeof(UInt16);
   
   // create a buffer with a little extra room in case
   char * buffer = new char[size + 4096];
   
   MemRWStream stream(size + 4096, buffer);
   
   LZHWStream lzStream;
   lzStream.attach(stream);
   lzStream.write(size, map);
   lzStream.detach();
   
   // grab the size - in bytes
   hiresLightMapSize = stream.getPosition();
   
   // copy this
   hiresLightMap = new UInt8[hiresLightMapSize];
   memcpy(hiresLightMap, buffer, hiresLightMapSize);
   
   delete [] map;
   delete [] buffer;
}

//---------------------------------------------------------------------------

GridBlockList::GridBlockList()
{
   ioVol=0;
   fileName = 0;
   resManager = NULL;
}

//---------------------------------------------------------------------------
GridBlockList::~GridBlockList()
{
   delete [] fileName;

   for ( iterator itr = begin(); itr != end(); itr++ ) {
      delete *itr;
   }
}

//---------------------------------------------------------------------------
//works correctly if fname == GridBlockList::fileName
void GridBlockList::setFileName(const char *fname)
{
   const char *newFname = fname? GridFile::fixName(fname, ".dtf"): 0;
	delete [] fileName;
  	fileName = (char *)newFname;
}

//---------------------------------------------------------------------------
void GridBlockList::setVolume(VolumeRWStream *vol)
{
   AssertFatal(vol, "GridFile::setVolume: vol ptr is null");

	ioVol = vol;
}

//---------------------------------------------------------------------------
void GridBlockList::closeVolume()
{
	if (ownVolume) {
	  delete ioVol;
     ioVol =0;  
	}
}

//---------------------------------------------------------------------------
void GridBlockList::addBlock(GridBlock* block)
{
   AssertFatal(block, "GridBlockList::addBlock: block ptr null");
	char* file = makeBlockName( block->getId(), fileName);
   resManager->add( ResourceType::typeof(file), file, block);

   GridBlockListElem *elem = new GridBlockListElem;
   elem->block = resManager->load(file);
   elem->blockId = block->getId();
   elem->name = new char[strlen(block->getNameId())+1];
   strcpy(elem->name, block->getNameId());
   push_back(elem);
}


//---------------------------------------------------------------------------
// searches the list for a block, loads it if necessary
GridBlock* GridBlockList::findBlock(GridBlock::Id id)
{
	for (iterator itr = begin(); itr != end(); itr++)
		if ((*itr)->blockId == id)
		{ 
			if (!bool((*itr)->block))
			{
				findBlockLock.acquire();
            // if another thread snuck in here before we got the lock and loaded 
            // our block for us, just return it.
				if(bool((*itr)->block))
            {
               findBlockLock.release();
               return (*itr)->block;
            }
				AssertFatal( bool((*itr)->block) || (resManager && fileName),
					"GridBlockList::findBlock: resManager or fileName not set");
				loadBlock(**itr);
				findBlockLock.release();
			}
			return (*itr)->block;
      }

	return 0;
}


//---------------------------------------------------------------------------
bool GridBlockList::findBlock(GridBlock* blk)
{
   for (iterator itr = begin(); itr !=end(); itr++)
      if (((GridBlock *) (*itr)->block) == blk)
         return true;
   return false;
}

// just clears the current hires lightmaps
void GridBlockList::clearHiresLightmaps()
{
   for( iterator itr = begin(); itr != end(); itr++ )
   {
      if( !bool((*itr)->block))
         loadBlock(**itr);
      (*itr)->block->getHRLMList()->clear();
   }
}

//---------------------------------------------------------------------------
GridBlock* GridBlockList::findBlock(char *nameid)
{
   GridBlock *ret = NULL;
   findBlockLock.acquire();
	for ( iterator itr = begin(); itr != end(); itr++ )
	{
		GridBlockListElem& elem = **itr;
		if (bool(elem.block))
		{
			if (!strcmp(nameid,elem.block->getNameId()))
			{
				ret = elem.block;
            break;
         }
		}
		else
			if (!strcmp(nameid,elem.name))
			{
				loadBlock(elem);
				ret = elem.block;
			   break;
			}
	}
   findBlockLock.release();
   return ret;
}


//---------------------------------------------------------------------------
//deletes 1st block with the name, returns the block id of the deleted block
GridBlock::Id GridBlockList::deleteBlock(char *nameid)
{
   AssertFatal(nameid, "GridBlockList::deleteBlock: nameid ptr null");
	iterator itr;
	for (itr = begin(); itr != end(); itr++ ) {
		GridBlockListElem *elem = *itr;
		if (!strcmp(
         elem->block?elem->block->getNameId() : elem->name,  nameid) )
	   		break;
	}

   GridBlock::Id id = -1;
   if ( itr != end() ) {
      id= (*itr)->blockId;
      delete *itr;
      erase(itr);
   }
   return ( id );
}


//---------------------------------------------------------------------------
// builds up a blockname with GridFile::fileName and blockId
char *GridBlockList::makeBlockName(GridBlock::Id Id, const char *gridFileName)
{
   static char buf[255];
	AssertFatal(gridFileName, "GridFile::makeBlockName:: filename not set.");
   char *pt;
   strcpy(buf, gridFileName);
   pt = strrchr(buf, '.');
   if(pt)
      sprintf(pt, "#%d%s", Id, GridBlock::getExt());
   else
      sprintf(buf, "%s#%d%s", gridFileName, Id, GridBlock::getExt());

   return buf;
} 

//---------------------------------------------------------------------------
void GridBlockList::markLoadedBlocksAsDirty()
{
   for (iterator itr = begin(); itr != end(); itr++) {
		GridBlockListElem& elm = **itr;
		if ((bool)elm.block)
		   elm.block->setDirtyFlag();
  	}
} 

//---------------------------------------------------------------------------
void GridBlockList::loadBlock(GridBlockListElem& elem)
{
  AssertFatal(resManager, "GridBlockList::loadBlock: no resManager");
  elem.block = resManager->load(makeBlockName(elem.blockId, fileName));
  elem.block->setId( elem.blockId );
  elem.block->setNameId( elem.name ? elem.name : avar( "block-%d", elem.blockId ) );
}	

//---------------------------------------------------------------------------
// unloads the block from an element in the list
void GridBlockList::unloadBlock(GridBlockListElem& elem)
{
   AssertFatal( ioVol && fileName,
		"GridBlockList::unloadBlock: Volume or fileName not set");

   if ( bool(elem.block) )
   {
      if(!elem.block->isDirty())
      {
      	// Copy the block name into the GridBlockListElem name.
         delete [] elem.name;
         elem.name = new char[strlen(elem.block->getNameId())+1];
         strcpy(elem.name, elem.block->getNameId());
         elem.block.unlock();
      }
   }
}

//---------------------------------------------------------------------------

void GridBlockList::updateHeightRange()
{
   for (iterator itr = begin(); itr != end(); itr++) 
   {
		GridBlockListElem& elm = **itr;
		if( ( bool )elm.block )
		   elm.block->updateHeightRange();
  	}
}

//---------------------------------------------------------------------------
// Save all currently loaded blocks
bool GridBlockList::save()
{
   AssertFatal(ioVol && fileName, "GridBlockList::save: vol and gridFileName"
      " must be set");
   bool res=true;
  	for (iterator itr = begin(); itr != end(); itr++) 
   {
		GridBlockListElem& elm = **itr;
		if (!bool(elm.block))
         loadBlock(elm);
      const char * blockName = makeBlockName( elm.blockId, fileName );

      // make sure that the block resource is valid???
      AssertFatal( bool( elm.block ), avar( "GridBlockList::save: failed to get block '%s'.", blockName ) );
      
      res &= elm.block->save( blockName, ioVol);
  	}
   return res;
}	

//---------------------------------------------------------------------------
bool GridBlockList::write(StreamIO& sio)
{
   // make sure lores lightmap is where it should be
   enableHires(false);

   sio.write( size() );
   for ( iterator itr = begin(); itr != end(); itr++ )
   {
     GridBlockListElem& elm = **itr;
     sio.write( elm.blockId );
     GridFile::writeString(elm.name, sio);
   }
   return (bool)(sio.getStatus() == STRM_OK);
}


//---------------------------------------------------------------------------
bool GridBlockList::read(StreamIO& sio)
{
   int size;
   if (!sio.read(&size))
      return false;

   for ( int i=0; i < size; i++ )
   {
      GridBlockListElem *elm = new GridBlockListElem;
      sio.read(&elm->blockId); 
      elm->name = GridFile::readString(sio);
      
   	push_back(elm);
   }
   return (bool)(sio.getStatus() == STRM_OK);
}

void GridBlockList::enableHires(bool hires)
{
   // send this off to all the blocks
   for (iterator itr = begin(); itr != end(); itr++) 
   {
      GridBlockListElem& elm = **itr;
      if(!bool(elm.block))
         loadBlock(elm);
    
      elm.block->enableHires(hires);     
   }
   
   // set the var
   GridHrlmList::PrefDetail = (int)hires;
   
   // flush the texture cache
   CMDConsole::getLocked()->evaluate("flushTextureCache();", false);
}

