//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

#ifndef _GRDFILE_H_
#define _GRDFILE_H_

#include <ts.h>

#include <ResManager.h>
#include "grdBlock.h"
#include "ts_shadow.h" // for PolyList -- will eventually be moved

//---------------------------------------------------------------------------

class GridRender;

struct GridBlockInfo
{
	GridBlock* block;
	Point2F pos;
};


//---------------------------------------------------------------------------

class DLLAPI GridFile : public Persistent::VersionedBase
{
public:
   enum GBPatternMap {
      OneBlockMapsToAll = 0,
      EachBlockUnique,
      MosaicBlockPattern1,
	};

private:
	friend GridRender;
  	struct Block {
  		GridBlock::Id blockId;
   };
   int lastBlockId;					// Last unassigned blockid.
   int detailCount;					// Range covered by details
   int scale;							// Scale of detail 0 (shift value)
	Point2I size;						// File size (NOT shift values)
   Box3F bounds;						// World bounds of file
	Point2I origin;					// Center of file blockMap
	GridRange<float> heightRange;
	Block* blockMap;
   Resource<TSMaterialList> materialHandle;
	TSMaterialList* materialList; //dereferenced materialHandle
  	Block* getBlockMap(const Point2I& pos);
   GridBlockList blockList;		// List of sub blocks
   GBPatternMap blockPattern;   // type of grid block layout pattern

   //file io related members
   char *matListName;
   VolumeStream *ioVol;
   ResourceManager *resManager;
   int blockListSize;
   bool readFile(VolumeStream *vol);
	static const char* stripPath(const char* fname);

public:
	GridFile();
	~GridFile();
	void setSize(Point2I ss);		// Not shift values
   Point2I getSize() {return (size);};
   int getDetailCount() {return ( detailCount);};
	void setMaterialList(Resource<TSMaterialList>& mlist);
   TSMaterialList* getMaterialList();
	GridRange<float> getHeightRange(bool rebuild);
	void addBlock(GridBlock* blk);
	void setScale(int ss);
	int getScale(void);
	void updateHeightRange();
   GridBlockList *getBlockList();
   GBPatternMap getBlockPattern() { return blockPattern; }
   void setBlockPattern( GBPatternMap pm ){ blockPattern = pm; }
   
   //assigns block to/from a position in the blockMap
   bool positionBlock(const Point2I& pos, GridBlock *block);
	GridBlock* getBlock(const Point2I& pos);
   void loadAllBlocks();
   void markLoadedBlocksAsDirty();

	// Pin the square at detail which contains the point.
	// pos is in grid file space
	bool pinSquare(int detail, const Point2F& pos);

   //zeroes out any occurance of an Id in the blockMap
   void invalidateId(GridBlock::Id);

	bool getBlockInfo(const Point2F& pos, GridBlockInfo* info);
	bool getSurfaceInfo(const Point2F& pos,CollisionSurface* info);
	bool getSurfaceHeight(const Point2F& pos,float* height, bool ignoreEmptyTiles = false);
	bool getLighting(const Point2F & pos, UInt16 & lighting);
	void getPolys(const Box2F& box, const TMat3F & transBack, TS::PolyList & pl,int maxPolys);
	GridBlock::Material *getMaterial(const Point2F& pos);
	int getBlockSizeShift() { return detailCount - 1; }

   //Persistent and related functions
   DECLARE_PERSISTENT(GridFile);
   Persistent::Base::Error read(StreamIO&, int version, int user );
   Persistent::Base::Error write(StreamIO&, int version, int user);
   int version();

   static void writeString(const char *str, StreamIO& sio);
   static char *readString(StreamIO& sio);
   static char *fixName(const char *fname, const char *extn);

   void setFileName (const char *fname);
   const char *getFileName();
   void setMaterialListName (const char *mListName);
   const char *getMaterialListName();

   bool setResourceManager(ResourceManager* rm);
   bool save(const char *volumeName);
};

//---------------------------------------------------------------------------
class ResourceTypeGridFile: public ResourceType
{
public:
	ResourceTypeGridFile(const char *ext = ".dtf"):
		ResourceType(ResourceType::typeof(ext)) {}
	void* construct (StreamIO *stream,int)
		{
         Persistent::Base::Error err;
			Persistent::Base* obj = Persistent::Base::load(*stream, &err);
			AssertFatal(dynamic_cast<GridFile*>(obj) != 0,
				"ResourceTypeGridFile::construct: "
				".dtf file dos not containe a GridFile object");
			return (void*) obj;
		}
	void destruct (void *p) {delete (GridFile*)p;}
};
		
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
inline const char *GridFile::getMaterialListName()
{
   return ( matListName );
}

//---------------------------------------------------------------------------
inline GridBlockList *GridFile::getBlockList()
{
   return &blockList;
}

//---------------------------------------------------------------------------
inline int GridFile::getScale()
{
	return scale;
}

//---------------------------------------------------------------------------
inline GridRange<float> GridFile::getHeightRange(bool rebuild = false)
{
	if (rebuild)
		updateHeightRange();
	return heightRange;
}	

//---------------------------------------------------------------------------
inline TSMaterialList*  GridFile::getMaterialList() 
{
   return materialList;
}

#endif
