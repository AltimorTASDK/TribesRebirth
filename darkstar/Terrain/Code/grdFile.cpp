//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

#include "grdFile.h"
#include "grdHeight.h"


#define TMPVOLNAME  "temp\\tmp$trn$vol"
//---------------------------------------------------------------------------

GridFile::GridFile()
{
	lastBlockId = 0;
	detailCount = 0;
	scale = 0;
	size.set(0,0);
	origin.set(0,0);
	materialList = 0;
	blockMap = 0;
	heightRange.fMin = 1.0E20f;
	heightRange.fMax = -1.0E20f;

   //persistent related stuff
   matListName = NULL;
   materialList = NULL;
   resManager = 0;
   blockListSize = 0;
   blockPattern = OneBlockMapsToAll;
}

GridFile::~GridFile()
{
	//
	delete [] blockMap;
	delete [] matListName;

}


//---------------------------------------------------------------------------
//-- Persistent & file i/o related stuff
IMPLEMENT_PERSISTENT_TAG(GridFile, FOURCC('G','F','I','L'));
static ResourceTypeGridFile _resdtf(".dtf");

//---------------------------------------------------------------------------
// adds the correct extension to the filename, creates a new char ptr which
// must eventually be deleted by caller
char* GridFile::fixName(const char *fname, const char *extn)
{
	if (fname)
	{
      int count=0;
		int len = strlen(fname);
		if (extn) {
			char *ext = (char*)fname + len-1;
			while (*ext != '.' && ext != fname)
				--ext, count++;
			if (ext == fname)
				len += strlen(extn);          //len of new extn
			else
				len += strlen(extn)-1-count;  //difference in lengths of extns
		}

		char *retStr = new char[len+1];
		strcpy(retStr, fname);
		if (extn)
			strcpy (&retStr[len-strlen(extn)], extn);
		return (retStr);
	}
	return (NULL);
}

//---------------------------------------------------------------------------

const char* GridFile::stripPath(const char* fname)
{
	// Return the file name portion without the path
	const char* ptr = fname + strlen(fname) - 1;
	for (; ptr != fname; ptr--)
		if (*ptr == '\\' || *ptr == '/' || *ptr == ':')
			return ptr + 1;
	return fname;
}


//---------------------------------------------------------------------------
// the fileName member is the name of the file within a volume & is used to
// build the names of the gridBlocks during file I/O
void GridFile::setFileName(const char *fname)
{
   blockList.setFileName(fname);
}

const char *GridFile::getFileName()
{
   return blockList.getFileName();
}


//---------------------------------------------------------------------------
void GridFile::setMaterialListName(const char *mListName)
{
	if (mListName != matListName) {
		delete [] matListName;
		matListName = mListName? fixName(mListName, ".dml"): 0;
	}
}

//---------------------------------------------------------------------------

bool GridFile::setResourceManager(ResourceManager* rm)
{
	// sets the resource manager for the file
   resManager = rm;
   blockList.setResourceManager(rm);
   return ( true );
}

//---------------------------------------------------------------------------
// Saves the GridFile and all the blocks in to the current volume.
bool GridFile::save(const char *volumeName)
{
   AssertFatal(resManager, "GridFile::save: resource manager not set");

   // we're doing something bad here...
   // closing the volume out from under the resource manager
   // this is why no mem-mapped files in terrain volumes

   // Write out the data to a temporary volume
   VolumeRWStream ioVol;
   ioVol.openVolume( TMPVOLNAME );
   bool success = false;

   // update the height ranges prior to saving
   updateHeightRange();
   
   if (ioVol.open(blockList.getFileName(), STRM_COMPRESS_NONE, 0)) {
      //write the gridFile and blockList
	   if (store(ioVol) == Persistent::Base::Ok) {
			ioVol.close();
         blockList.setVolume(&ioVol);
         success = blockList.save(); //save the blocks
	   }
   }

	// Transfer over to new volume
   resManager->removeVolume(volumeName);
	if (success) {
		if( !DeleteFile( volumeName ) )
      {
         // check that the file is new or not...
         FileRStream tmp( volumeName );
         if( tmp.getStatus() == STRM_OK )
            success = false;
      }
		ioVol.compactVolume( volumeName );
	}

   ioVol.closeVolume();
      
	DeleteFile( TMPVOLNAME );
	
   if (success) {
      resManager->addVolume(volumeName);
   }
   return success;
}

//---------------------------------------------------------------------------

int GridFile::version()
{
	return 1;
}

//---------------------------------------------------------------------------
//reads a length, then reads a string into newly allocated storage
char *GridFile::readString(StreamIO& sio)
{
   int len;
   char *str;
   if (sio.read(&len) && len)
   {
      str = new char[len+1];
      if (sio.read(len,(void*)str)) {
			str[len] = '\0';
	      return ( str );
      }
   }
   return ( NULL );
}

//---------------------------------------------------------------------------
//always writes a length, then writes a string if len > 0
void GridFile::writeString(const char *str, StreamIO& sio)
{
   int len= str ? strlen(str) : 0;
   sio.write(len);
   if ( len )
      sio.write(len, (void*)str);
}

//---------------------------------------------------------------------------
Persistent::Base::Error GridFile::read(StreamIO& sio, int ver, int)
{
   //read matListName
   char *mlname = readString(sio);
   setMaterialListName(mlname);
   delete [] mlname;

   //read persistent vars
   sio.read(&lastBlockId);
   sio.read(&detailCount);
   sio.read(&scale);

   sio.read(sizeof(bounds), (void*) &bounds);
   sio.read(sizeof(origin), (void*) &origin);
   sio.read(sizeof(GridRange<float>), (void*)&heightRange);

   if (!sio.read(sizeof(size), (void*)&size))
      return (ReadError);        //we can't continue

   AssertWarn( ver, "GridFile::read:  Old terrain version, using default block pattern.");

   if ( ver == 1 )
      sio.read(sizeof(GBPatternMap),&blockPattern);

   //now read the map of Block IDs
   if ( blockMap )               delete [] blockMap;
   blockMap = new Block[size.x*size.y];          
   sio.read(size.x*size.y*sizeof(GridFile::Block), (void*)blockMap);

   return ( (!blockList.read(sio) || (sio.getStatus() != STRM_OK)))?
             ReadError: Ok;
}

//---------------------------------------------------------------------------
Persistent::Base::Error GridFile::write(StreamIO& sio, int, int)
{
   writeString(matListName, sio);

   //write persistent vars
   sio.write(lastBlockId);
   sio.write(detailCount);
   sio.write(scale);

   sio.write(sizeof(bounds), (void*) &bounds);
   sio.write(sizeof(origin), (void*) &origin);
   sio.write(sizeof(GridRange<float>), (void*)&heightRange);
   sio.write(sizeof(size), (void*) &size);
   sio.write(sizeof(GBPatternMap), (void*)&blockPattern);

   int blocklen = size.x*size.y;
   if ( !blockMap )
   {
	  //partial check of blockMap integrity
      if ( blocklen )
         return ( WriteError );
   }
   else
   {
	   //now write the map of Block IDs
		sio.write(blocklen*sizeof(GridFile::Block), (void*)blockMap);
   }

   return ( !(blockList.write(sio) || (sio.getStatus() != STRM_OK)))?
          WriteError: Ok;
}  
 
//---------------------------------------------------------------------------

GridFile::Block* GridFile::getBlockMap(const Point2I& pos)
{
	AssertFatal(pos.x >= 0 && pos.y >= 0 && pos.x < size.x && pos.y < size.y,
		"GridFile::getRootBlock: Out of bounds");
	return &blockMap[pos.y * size.x + pos.x];
}

//---------------------------------------------------------------------------
//searches through the blockMap and zeroes any Id corresponding to theId
void GridFile::invalidateId(GridBlock::Id theId)
{
   for ( int y=0; y<size.y; y++ )
      for ( int x=0; x<size.x; x++ )
      {
         GridFile::Block *blk = getBlockMap( Point2I(x,y) );
         if ( blk && (blk->blockId==theId) )
            blk->blockId = 0;
      }
}

//---------------------------------------------------------------------------
// assigns the block's id to a position in the blockMap
bool GridFile::positionBlock(const Point2I& pos, GridBlock* block)
{
   Block* blk = getBlockMap(pos);
   if ( blk && block )
   {
      blk->blockId = block->getId();
      return ( true );
   }
   return ( false );
}

//---------------------------------------------------------------------------
void GridFile::setMaterialList(Resource<TSMaterialList>& mlist)
{
   materialHandle = mlist;
	materialList = materialHandle;
}

//---------------------------------------------------------------------------
void GridFile::setScale(int ss)
{
	scale = ss;
}

//---------------------------------------------------------------------------
void GridFile::setSize(Point2I ss)                
{                                                 
  Block* newMap = new Block[ss.x*ss.y];          
  if ( !newMap )                                 
     return;                                     
  memset(newMap, 0, ss.x*ss.y*sizeof(Block));    
                                                 
  if ( blockMap )                                
  {                                              
   for ( int y=0; y < min(ss.y, size.y); y++ )   
     for ( int x=0; x< min(ss.x, size.x); x++ )  
       newMap[y*ss.y+x] = blockMap[y*size.y+x]; 
  }                                              
                                                 
  delete [] blockMap;                            
  size = ss;                                     
  blockMap = newMap; 
}                                                 

//---------------------------------------------------------------------------
void GridFile::addBlock(GridBlock* blk)
{
   if ( blk )
   {
      AssertFatal(!blockList.findBlock(blk), "GridFile::addBlock: block has"
            " already been added.");

   	blk->id = lastBlockId++;
   	detailCount = max(detailCount,blk->detailCount);
   	heightRange.fMin = min(heightRange.fMin,blk->heightRange.fMin);
   	heightRange.fMax = max(heightRange.fMax,blk->heightRange.fMax);
   	blockList.addBlock(blk);
   }
}

GridBlock* GridFile::getBlock(const Point2I& pos)
{
	return blockList.findBlock(getBlockMap(pos)->blockId);
}

void GridFile::loadAllBlocks()
{
   Point2I pos;
   for (int i= 0; i < size.x; i++)
      for (int j = 0; j < size.y; j++)
         getBlock(pos.set(i,j));
}   

void GridFile::markLoadedBlocksAsDirty()
{
   getBlockList()->markLoadedBlocksAsDirty();      
}   


//---------------------------------------------------------------------------

void GridFile::updateHeightRange()
{
   // update the block's height's
   blockList.updateHeightRange();
   
	// Scan through the height map and update the height range.
	heightRange.fMin = +1.0E20f;
	heightRange.fMax = -1.0E20f;
	for (GridBlockList::iterator itr = blockList.begin();
			itr != blockList.end(); itr++) {
		if (bool((*itr)->block)) {
			heightRange.fMin = min((*itr)->block->heightRange.fMin,heightRange.fMin);
			heightRange.fMax = max((*itr)->block->heightRange.fMax,heightRange.fMax);
		}
	}
}


//---------------------------------------------------------------------------

bool GridFile::pinSquare(int detail, const Point2F& pos)
{
	if (detail >= 1 && detail < detailCount) {
	   GridBlock* block;
		Point2I bpos;
		int groundShift = (detailCount - 1) + scale;
		bpos.x = int(pos.x) >> groundShift;
		bpos.y = int(pos.y) >> groundShift;

		if (bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y)
			if ((block = getBlock(bpos)) != 0) {
				Point2F spos;
				spos.x = pos.x - float(bpos.x << groundShift);
				spos.y = pos.y - float(bpos.y << groundShift);

				Point2I bpos;
				bpos.x = int(spos.x) >> (scale + detail);
				bpos.y = int(spos.y) >> (scale + detail);
				block->pinSquare(detail,bpos);
				return true;
			}
	}
	return false;
}


//---------------------------------------------------------------------------

bool GridFile::getSurfaceInfo(const Point2F& pos,CollisionSurface* info)
{
   GridBlock* block;
	Point2I bpos;
	int groundShift = (detailCount - 1) + scale;
	bpos.x = int(pos.x) >> groundShift;
	bpos.y = int(pos.y) >> groundShift;

	if (bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y)
		if ((block = getBlock(bpos)) != 0) {
			// Pass position in block space
			Point2F spos;
			spos.x = pos.x - float(bpos.x << groundShift);
			spos.y = pos.y - float(bpos.y << groundShift);
			block->materialList = materialList;
			if (block->getSurfaceInfo(scale,spos,info)) {
				// Reset info pos back into file space
				info->position.x = pos.x;
				info->position.y = pos.y;
				info->part = block->getId();
				return true;
			}
		}
	return false;
}

//---------------------------------------------------------------------------

bool GridFile::getSurfaceHeight(const Point2F& pos, float* height, bool ignoreEmptyTiles)
{
   GridBlock* block;
	Point2I bpos;
	int groundShift = (detailCount - 1) + scale;
	bpos.x = int(pos.x) >> groundShift;
	bpos.y = int(pos.y) >> groundShift;

	if (bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y)
		if ((block = getBlock(bpos)) != 0) {
			// Pass position in block space
			Point2F spos;
			spos.x = pos.x - float(bpos.x << groundShift);
			spos.y = pos.y - float(bpos.y << groundShift);
			block->materialList = materialList;
			return (block->getSurfaceHeight(scale,spos,height, ignoreEmptyTiles));
  		}
	return false;
}

//---------------------------------------------------------------------------

GridBlock::Material *GridFile::getMaterial(const Point2F& pos)
{
   GridBlock* block;
	Point2I bpos;
	int groundShift = (detailCount - 1) + scale;
	bpos.x = int(pos.x) >> groundShift;
	bpos.y = int(pos.y) >> groundShift;

	if (bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y)
		if ( (block = getBlock(bpos)) != NULL )
		{
			// Pass position in block space
			Point2F spos;
			spos.x = pos.x - float(bpos.x << groundShift);
			spos.y = pos.y - float(bpos.y << groundShift);
         return block->getMaterial(scale, 0, spos);
		}
	return NULL;
}


//---------------------------------------------------------------------------

bool GridFile::getBlockInfo(const Point2F& pos,GridBlockInfo* info)
{
	Point2I bpos;
	int groundShift = (detailCount - 1) + scale;
	bpos.x = int(pos.x) >> groundShift;
	bpos.y = int(pos.y) >> groundShift;

	if (bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y)
		if ( (info->block = getBlock(bpos)) != NULL )
		{
			info->pos.x = pos.x - float(bpos.x << groundShift);
			info->pos.y = pos.y - float(bpos.y << groundShift);
			return true;
		}
	return false;
}

//---------------------------------------------------------------------------

bool GridFile::getLighting(const Point2F& pos, UInt16 & lighting)
{
   GridBlock* block;
	Point2I bpos;
	int groundShift = (detailCount - 1) + scale;
	bpos.x = int(pos.x) >> groundShift;
	bpos.y = int(pos.y) >> groundShift;

	if (bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y)
		if ( (block = getBlock(bpos)) != NULL )
		{
			// get offset into block
			Point2F blockOffset;
			blockOffset.x = pos.x - float(bpos.x << groundShift);
			blockOffset.y = pos.y - float(bpos.y << groundShift);

			// get square position in block
			Point2I spos;
			spos.x = blockOffset.x;
			spos.y = blockOffset.y;
			spos >>= scale;

			// offset into square
			Point2F squareOffset = blockOffset;
			squareOffset.x -= spos.x << scale;
			squareOffset.y -= spos.y << scale;

			// get lightmap grid position in square
			Point2I lpos;
			lpos.x = squareOffset.x;
			lpos.y = squareOffset.y;
			lpos >>= scale-block->lightScale;

			// get offset into lightmap grid
			Point2F lgridOffset = squareOffset;
			lgridOffset.x -= lpos.x << (scale - block->lightScale);
			lgridOffset.y -= lpos.y << (scale - block->lightScale);

			// get lightmap
			int index = (lpos.y + (spos.y << block->lightScale)) * block->getLightMapWidth() +
			            (lpos.x + (spos.x << block->lightScale));
			UInt16 * lmap = &block->lightMap[index];

			// lmap now points to lower-left corner of lightmap grid location around our point
			// size of lightmap grid depends on block->lightScale
			// if block->lightScale=0 then lightmap grid is 1-1 with terrain squares (height field)

			// get four corners of lightmap grid
			UInt16 LL = lmap[0];
			UInt16 LR = lmap[1];
			UInt16 UL = lmap[block->getLightMapWidth()];
			UInt16 UR = lmap[block->getLightMapWidth()+1];

			// how big is one lightmap grid?
			// scale better not be bigger than lightscale...
			if (scale<block->lightScale)
				return false;  // AssertFatal?
			float lgridSize = 1 << (scale - block->lightScale);

			// get interpolation coeffs
			float px = lgridOffset.x/lgridSize;
			float py = lgridOffset.y/lgridSize;

			// interpolate 4 bits at a time...
			UInt16 L  = (UInt16)( ((float)(LL &    0xF))*px + ((float)(LR &    0xF)*(1-px)) );
			L        |= (UInt16)( ((float)(LL &   0xF0))*px + ((float)(LR &   0xF0)*(1-px)) ) & 0xF0;
			L        |= (UInt16)( ((float)(LL &  0xF00))*px + ((float)(LR &  0xF00)*(1-px)) ) & 0xF00;
			L        |= (UInt16)( ((float)(LL & 0xF000))*px + ((float)(LR & 0xF000)*(1-px)) ) & 0xF000;

			UInt16 U  = (UInt16)( ((float)(UL &    0xF))*px + ((float)(UR &    0xF)*(1-px)) );
			U        |= (UInt16)( ((float)(UL &   0xF0))*px + ((float)(UR &   0xF0)*(1-px)) ) & 0xF0;
			U        |= (UInt16)( ((float)(UL &  0xF00))*px + ((float)(UR &  0xF00)*(1-px)) ) & 0xF00;
			U        |= (UInt16)( ((float)(UL & 0xF000))*px + ((float)(UR & 0xF000)*(1-px)) ) & 0xF000;

			lighting  = (UInt16)( ((float)( L &    0xF))*py + ((float)( U &    0xF)*(1-py)) );
			lighting |= (UInt16)( ((float)( L &   0xF0))*py + ((float)( U &   0xF0)*(1-py)) ) & 0xF0;
			lighting |= (UInt16)( ((float)( L &  0xF00))*py + ((float)( U &  0xF00)*(1-py)) ) & 0xF00;
			lighting |= (UInt16)( ((float)( L & 0xF000))*py + ((float)( U & 0xF000)*(1-py)) ) & 0xF000;

			return true;

		}
	return false;
}

//---------------------------------------------------------------------------

void GridFile::getPolys(const Box2F& box, const TMat3F & transBack, TS::PolyList & pl,int maxPolys)
{
	Point2I bpos, bposMin, bposMax;
	int groundShift = (detailCount - 1) + scale;
	bposMin.x = int (box.fMin.x) >> groundShift; // use min to get block
	bposMin.y = int (box.fMin.y) >> groundShift;

	bposMax.x = int (box.fMax.x) >> groundShift; // use max to get block
	bposMax.y = int (box.fMax.y) >> groundShift;

	GridBlock * block;
   for (bpos.x=bposMin.x; bpos.x <= bposMax.x; bpos.x++)
      for (bpos.y=bposMin.y; bpos.y <= bposMax.y; bpos.y++)
      {
      	if (bpos.x>=0 && bpos.x<size.x &&
             bpos.y>=0 && bpos.y<size.y &&
             (block=getBlock(bpos))!=0)
      	{
		      Point2F blockOffset;
   	   	blockOffset.x = float(bpos.x << groundShift);
	   	   blockOffset.y = float(bpos.y << groundShift);

      		Box2F offsetBox = box;
	      	offsetBox.fMin -= blockOffset;
		      offsetBox.fMax -= blockOffset;

      		block->getPolys(scale,offsetBox,blockOffset,transBack,pl,maxPolys);

            if (pl.size()>=maxPolys)
               return;
      	}
      }
}

