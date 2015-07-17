//---------------------------------------------------------------------------



//---------------------------------------------------------------------------

#ifndef _GRDBLOCK_H_
#define _GRDBLOCK_H_

#include <ml.h>
#include <m_collision.h>
#include <tvector.h>
#include <ts.h>
#include <ts_color.h>

#include "grdRange.h"
#include "grdHRLM.h"
#include "ts_shadow.h" // for PolyList...will eventually be moved
#include "lock.h"
#include "g_bitmap.h"
#include "g_types.h"
//---------------------------------------------------------------------------

class GridFile;
class GridRender;
struct GridHeightInfo;


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

class GridBlock : public Persistent::VersionedBase
{
	friend GridFile;
	friend GridRender;

public:
	struct Material {
		enum Flags {
			// Three bits for texture orientation
			Plain =  0,
			Rotate = 1,
			FlipX =  2,
			FlipY =  4,
			RotateMask = 7,
			// Three bits for empty squares
			EmptyShift = 3,
			EmptyMask =  7,
			// Editing flags
			Edit = 64,
         Corner = 128,
		};
		UInt8 flags;
		UInt8 index;

		//
		int getEmptyLevel()  {
			return (flags >> EmptyShift) & EmptyMask;
		}
	};

	struct Height {
		float height;
	};

	//
	enum Constants {
		MaxNameIdLen = 15,
		MaxDetailLevel = 10,
	};
   typedef int Id;

// B-listed
private:
   UInt8 *        hiresLightMap;
   int            hiresLightMapSize;
   bool           hiresMode;
public:
   void enableHires(bool hires);
   void setHiresLightMap(UInt16* map);

private:
   Id id;                     // Assigned by GridBlockList class
   char nameId[MaxNameIdLen+1];// Used mainly by ted plugin
	Point2I size;					// Block size
	int detailCount;				// Size shift value + 1
	int lightScale;				// Scale of height coors.
	GridRange<float> heightRange;

	Height* heightMap;			// Size{xy} + 1
	Material* materialMap;		// Size{xy} + 1
	UInt8* pinMap[MaxDetailLevel+1];
   GridHrlmList   hrLightMaps;

	UInt16*               lightMap;
   GFXLightMap**         pSubLightmaps;
   GFXBitmap::CacheInfo* pSubLMCacheInfo;
   UInt32                subLightmapSide;

   GFXBitmap*            m_pCoverBitmap;

   void deleteSubLightmaps();
   void buildSubLightmaps();
   void buildCoverBitmap();

	// Data only valid during rendering...
	// MaterialList is also set before calls to getSurfaceInfo
	TSMaterialList* materialList;
	Point2I bufferOrigin;
	Point3F origin;				// Origin in object space
	int textureScale;				// Size (shift value) of texture at level 0
	float textureWorldScale;
   bool dirtyFlag; // has the block been modified?
public:
	GridBlock();
	~GridBlock();
	Id getId();
   void setId( Id val ){ id = val; };
   void setNameId( const char * txt );
	Height* getHeight(int detail, const Point2I& pos);
	Material* getMaterial(int detail, const Point2I& pos);
	UInt16* getLightMap(int detail, const Point2I& pos);

	Height* getHeightMap();
	Material* getMaterialMap();
	UInt16* getLightMap();

	int getHeightMapWidth();
	int getMaterialMapWidth();
	int getLightMapWidth();

	bool isHeightPinned(const Height* hp);
	bool isSquarePinned(int detail, const Point2I& pos);
   
   GridHrlmList * getHRLMList(){ return( &hrLightMaps ); };
   
   // callback structure used for undo/redo in ted
   struct pinCallbackInfo
   {
      // set before pin
      void ( * func )( struct pinCallbackInfo * info );
      void * obj;
      
      // info taken back
      int detail;
      Point2I pos;
      bool val;
      GridBlock * gridBlock;
   };
   void pinSquare( int detail, Point2I pos, int maxDetail = MaxDetailLevel, bool set = true, 
      pinCallbackInfo * callback = NULL );
      
   void clearPinMaps();

   void setDirtyFlag() { dirtyFlag = true; }
   bool isDirty() { return dirtyFlag; }
	void setScale(int ss);
	int getSizeShift();
	void setSize(Point2I ss);	// Shift values
   const Point2I &getSize() { return size; }
	void setLightScale(int ls);
	int getLightScale();
	void setHeightRange(float min,float max);
	void updateHeightRange();
	GridRange<float> getHeightRange();

	bool getSurfaceInfo(int scale,const Point2F& pos,CollisionSurface* info);
	bool getSurfaceHeight(int scale,const Point2F& pos,float* height, bool ignoreEmptyTiles = false);
	void getPolys(int scale,const Box2F & offsetBox,Point2F & blockOffset,
	              const TMat3F & transBack, TS::PolyList & pl, int maxPolys);
	Material *getMaterial(int scale, int detail, const Point2F& pos);
	void buildLightMapTest(GridFile *,TSSceneLighting *sceneLighting,Point2F groundOrigin );
   // light map methods:
   struct Step {  int Size, Inc, Adj;
                  UInt16   *Data;  };
   bool  addHRLM ( UInt16 *data, const Point2I & pos );
   bool  addHRLM ( UInt16 singleColor, const Point2I & pos );
   static Step  howToStep ( int srcWidth, int sourceSize, UInt16 * ls, int flags );
   static void  lmExpand ( TSExpandedInten16 * expandedColors, 
               const UInt16 * ls, int srcInc, int N, int shift );
   static void  lmInterpolate ( TSExpandedInten16 * expandedColors, 
               TSExpandedInten16 begin, TSExpandedInten16 end,int shift );
   static void  lmCollapse( const TSExpandedInten16 * expandedColors, 
                    UInt16 * ld, int dstInc, int N );
   UInt16 *fillLightMap( UInt16 *lmDst, const Point2I & pos, 
               int lev, int levInc, int flags, bool hasHighRes );

   GFXBitmap* getCoverBitmap() { return m_pCoverBitmap; }

   static void  dumpLowResGrid ( UInt16 *lmapData, const Step & step );
   void  expandHRLMSquare ( UInt16 *lmapData, const Point2I & pos, 
               int level, int levelInc, const Step & step );
   static void  expandLowResData ( UInt16 *lmapData, 
               int level, int levelInc, const Step & step );
   bool  squareHasHighRes ( Point2I pos, int level ) {
               return ( hrLightMaps.findHighResNode( pos, level ) != 0 );  }
   int doTestFillPattern( void );
   

	//persistent and file i/o 
	DECLARE_PERSISTENT(GridBlock);
	int version();
	Persistent::Base::Error read(StreamIO&, int version, int user);
	Persistent::Base::Error write(StreamIO&, int version, int user);

	static GridBlock* load(const char *blkname, VolumeStream *vol);
	bool save(const char *blkname, VolumeRWStream *vol);
	static const char *getExt();

   //named id methods
   const char *getNameId();
};

//---------------------------------------------------------------------------
class ResourceTypeGridBlock: public ResourceType
{
public:
	ResourceTypeGridBlock(const char *ext = ".dtb"):
		ResourceType(ResourceType::typeof(ext)) {}
    void* construct (StreamIO *stream,int)
		{
         Persistent::Base::Error err;
			Persistent::Base* obj = Persistent::Base::load(*stream, &err);
			AssertFatal(dynamic_cast<GridBlock*>(obj) != 0,
				"ResourceTypeGridBlock::construct: "
				".dtb file dos not containe a GridBlock object");
			return (void*) obj;
		}
	void destruct(void *p) {delete (GridBlock*)p;}
};

//---------------------------------------------------------------------------
inline const char *GridBlock::getNameId()
{
   return nameId;
}

inline void GridBlock::setNameId(const char *txt)
{
   if (txt) {
      strncpy(nameId, txt, MaxNameIdLen);
      nameId[MaxNameIdLen] = '\0';
   }
	else
      nameId[0] = '\0';
}

inline const char *GridBlock::getExt()
{
	return ".dtb";
}

inline GridBlock::Id GridBlock::getId()
{
	return id;
}

inline int GridBlock::getHeightMapWidth()
{
	return size.x + 1;
}

inline int GridBlock::getMaterialMapWidth()
{
	return size.x;
}

inline int GridBlock::getLightMapWidth()
{
	return (size.x << lightScale) + 1;
}

inline GridBlock::Height* GridBlock::getHeightMap()
{
	return heightMap;
}

inline GridBlock::Material* GridBlock::getMaterialMap()
{
	return materialMap;
}

inline UInt16* GridBlock::getLightMap()
{
	return lightMap;
}

inline int GridBlock::getLightScale()
{
	return lightScale;
}	

inline void GridBlock::setHeightRange(float min,float max)
{
	heightRange.fMin = min;
	heightRange.fMax = max;
}

inline GridRange<float> GridBlock::getHeightRange()
{
	return heightRange;
}	

inline int GridBlock::getSizeShift()
{
   return detailCount - 1;
}

//---------------------------------------------------------------------------

// class GridBlockList;                     
// struct GridBlockList::GridBlockListElem; 

struct GridBlockListElem {
	Resource<GridBlock> block;
	GridBlock::Id blockId;
	char* name;

   GridBlockListElem() { name = NULL; }
   ~GridBlockListElem() { delete[] name; }
};

class GridBlockList: public Vector<GridBlockListElem*>
{
public:

private:
   bool ownVolume;
   VolumeRWStream *ioVol;
   char *fileName;

   void loadBlock(GridBlockListElem& element);
   void unloadBlock(GridBlockListElem& element);
   GridBlock::Id deleteBlock(GridBlockListElem& elm);

   ResourceManager *resManager;
   Lock findBlockLock;

public:
	GridBlockList();
	~GridBlockList();
   void enableHires(bool hires);
   char * makeBlockName(GridBlock::Id Id, const char *gridFileName);
   void setResourceManager(ResourceManager *rm) { resManager = rm; }
	void addBlock(GridBlock*);

   void setVolume(VolumeRWStream *vol);
   const char *getFileName();
   void setFileName(const char *fname);
   void updateHeightRange();
   
   bool findBlock(GridBlock* blk);
   GridBlock* findBlock(char *nameid);
   GridBlock::Id deleteBlock(char *nameid);
   GridBlock* findBlock(GridBlock::Id id);

   void markLoadedBlocksAsDirty();
   void clearHiresLightmaps();

   //file i/o
   void closeVolume();
   bool save();
   bool read(StreamIO&);
   bool write(StreamIO&);
};

//---------------------------------------------------------------------------
inline const char *GridBlockList::getFileName()
{
	return fileName;
}


#endif
