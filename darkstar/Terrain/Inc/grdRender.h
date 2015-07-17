//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

#ifndef _GRDRENDER_H_
#define _GRDRENDER_H_

#include <tvector.h>
#include <BitGrid.h>
#include <Steppers.h>
#include <ts.h>
#include <ts_color.h>
#include "g_types.h"

#include "grdRange.h"
#include "grdBlock.h"
#include "grdFile.h"
#include "grdEdgeTable.h"


struct GFXLightMap;

//---------------------------------------------------------------------------

class DLLAPI GridRender {
public:
	enum Constants {
		MaxDetailLevel = 7,
      IsDynamicLit = (1<<30),
	};

	struct Detail {
		float detailDistance;		// Distance at which square decompresses
		float minDistance;			// Point interpolation min
		float maxDistance;			// Point interpolation max
		//
		float distanceScale;			// 1 / (maxDistance - minDistance)
		int level;

		struct Point {
			int x,y;
		} cameraPos;					// Offset from block

		// GridBlock::heightMap offsets
		int hpOffsetX;
		int hpOffsetY;
		int hpOffsetXY;
		int hpOffsetX2Y;
		int hpOffset2XY;

		// Buffer offsets
		int bpOffsetX;
		int bpOffsetY;
		int bpOffsetXY;
		int bpOffsetX2Y;
		int bpOffset2XY;
	};

   #ifdef DEBUG
    private:
      static GFXBitmap *defaultTexture;
    public:
      void setDefaultTexture(GFXBitmap *gbm) {defaultTexture=gbm;};
   #endif

private:
	// Detail settings
   bool isOrtho;
	Detail detailData[MaxDetailLevel+1];
	GridRange<float> heightRange;
	float visibleDistance;
	float perspectiveDistance;
	float hazeDistance;
	float screenSize;						// Size at which square decompress
	float hazeScale;						// 1 / (visibleDistance - hazeDistance)
	float hazeVerticalScale;
	GridRange<float> hazeVertical;
	bool customDetailTable;
	int scanDetailLevel;					// From visibleDistance
	int scale;
   int terrainTextureDetail;

	// Visible squares
	struct Square {
		GridBlock* block;
		GridBlock::Material* material;
		Point2I pos;
		float distance;
		const Detail* dp;
		const GridBlock::Height* hp;
		int *bp;
		int split;
      UInt16   flags;
      Point2F  blockOrg;
      enum  {  MaxLightLevelInc = 4, LevelIncMask = 0xf,
               SqrUnderDLight = 0x10, SqrDynamicLit = 0x20, SqrHasHighRes = 0x40, };
      void  checkHighRes ( void ) const { 
                  if(block->squareHasHighRes(pos, dp->level))
                     *(UInt16 *)(&flags) |= SqrHasHighRes;
               }
      void  stuffLevelInc(int l) const { *(UInt16 *)(&flags) |= (l&LevelIncMask);  }
      bool  isLit() const  { return(flags & SqrDynamicLit);   }
      bool  hasHighRes() const  { return(flags & SqrHasHighRes);   }
      bool  isUnderDLight () const  { return(flags & SqrUnderDLight);   }
      int   getLevelInc () const { return( flags & LevelIncMask);  }
      int   lightShift ( int txtrShift ) const;
	};
	typedef Vector<Square> SquareList;
	SquareList squareList;

   //---------------  Dynamic Lighting
   void        prepLightsRender (void);
   void        prepLightsBlock (void);
   void        prepLightsSquare (Square & square);

   // light merger routine and it's related data.  Info is set up in 
   //    prepLightsSquare().  
   struct DLightInfo {
      TSLight        *tsl;
      Box2I          squareBox;     // grid coordinates of the square affected.
      Box2I          xyGrid;        // grid coordinates to merge into
      };
   typedef Vector<DLightInfo>  DLightInfoList;
   Int16             htStepBits;       // These variables seem a little unclean, they're
   const Square      *renderSquare;    // active during the square prepare / render 
   Point3F           blockOrigin;      // process.  
   Int16             dlightMatFlags;
   DLightInfoList    dlightInfoList;
   
   bool              rangeOverlaps ( Box2I gbox, float zmin, float zmax );
   void              GetHeights ( float * hts, const Point2I & pos, int N );
   void              mergeDLight ( DLightInfo * dli, UInt16 * ldata, int lwidth );
   void              mergeAllDLights ( GFXLightMap* lightMap );

   enum  {
      lightBitsetMax = 8,
      MaxSquareLightLevel = 6,
      MaxLightMapWidth = (1 << MaxSquareLightLevel),
   };
   // typedef VectorPtr<TSLight *> TSLightBuffer;
   // TSLightBuffer     lightsOverBlock;
   BitSquare         *lightedSquares;
   UInt16            dLightCount;
   UInt16            dlBitsetSize;
   Int16             squareLightShift;
   bool              dlightsEnabled;
   bool              isUnderLights ( const Square & sqr );
   bool              isPossiblyLit () { return ( dLightCount != 0 );  }
   //Dynamic Lighting-----------

	// Point list & buffer
	struct Point {
		int paIndex;						// Cleared to -1
		int* buffer;						// Buffer index to clear
		float distance;
	};
	typedef Vector<Point> PointList;
	typedef Vector<int> Buffer;		// Index into PointList

	PointList pointList;
	Point2I bufferOrigin;				// In object detail 0 space
	Point2I bufferSize;
	Buffer  buffer;

	// Context Data
	TSRenderContext* renderContext;
	TSPointArray* pointArray;
	GFXSurface* gfxSurface;
	TSCamera* camera;
	Point3F cameraPos;					// Pos in object space
	Box2I visibleBox;						// ScanDetailLevel object space
	GridEdgeTable edgeTables;

	// Render data
	GridFile* file;						// File being rendered
	GridBlock* block;						// Block being rendered

	// Methods
	int* getBuffer(int detail, const Point2I& pos);
	void scanBlock(GridBlock* blck,const Point2I& pos);

	// Point transformation
	float interpolateHeight(const Point2I& pos,const Detail*,
		const GridBlock::Height *hp,float frac);
	void transform(const Point3F& vector,float distance,int* bp);
	void transform(const Point2I& pos,const Detail*,
		const GridBlock::Height*,int* bp);
	void transformCross(const Point2I& pos,const Detail*,
		const GridBlock::Height*,int* bp);
	void transformRow(Point2I pos,int endx,const Detail* dp,
		const GridBlock::Height* hp,int *bp);

	// Recursive square subdivision
	bool isVisible(const Point2I& pos,const Detail*);
	float getDistance(const Detail*,const int* bp);
	void testSquare(Point2I,const Detail*,const GridBlock::Height* hp,int* bp);

	// Square rendering
	int getSquarePoints(const Detail*,const int* bp,Point** points);
   int getTextureShift( int blkTxtrScale, int mipLevel, int detail );
	void drawSquare(const Square& square);
	void drawSquareOGL(const Square& square, bool drawPass);
   void finishSquareAsTempOGL(const Square& square,
                              int,
                              int*,
                              TS::VertexIndexPair*);
	int faceVisibility(int a,int b,int c);
   int buildPolyList( const Square & square, int * polyVis,
      TS::VertexIndexPair * ilist );
   void drawPolys( const int numVisPolys, const int * polyVis,
      const TS::VertexIndexPair * ilist );
   
	// Caching.
   void buildLightMapOGL(const Square&          square,
                         int                    flags,
                         int                    txtShift,
                         GFXLightMap*&          out_rpLightMap,
                         GFXBitmap::CacheInfo*& out_rpCacheInfo,
                         RectI&                 out_rSubRegion);
	GFXLightMap* buildLightMap(const Square& square,int flags, int maxTxtShift=1000);
	static void buildTexture(GFXTextureHandle,GFXBitmap*,int numMipLevels);

public:
	GridRender();
	~GridRender();

	void setDetailTable(const Detail dt[MaxDetailLevel]);
	void setVisibleDistance(float dist);
	void setScreenSize(float sz);
	void setHazeDistance(float dist);
   inline float getHazeDistance() { return hazeDistance; }
	void setHazeVertical(float minDist,float maxDist);
	void setPerspectiveDistance(float dist);
   inline float getPerspectiveDistance();
	void setContext(TSRenderContext& rc);
	void setScale(int ss);
	void setHeightRange(GridRange<float> hr);
   void setTerrainTextureDetail(int level);
	void render(GridFile* aFile);
   void dlightsEnable(void); 
   void dlightsDisable(void);

	float getVisibleDistance() const {return visibleDistance;}
   float getHaze(float distance);
   float getScreenSize() { return screenSize; }
};


//---------------------------------------------------------------------------

inline void GridRender::setScale(int ss)
{
	scale = ss;
}

inline void GridRender::setHeightRange(GridRange<float> hr)
{
	heightRange = hr;
}

inline void GridRender::setPerspectiveDistance(float dist)
{
	perspectiveDistance = dist;
}	

inline float GridRender::getPerspectiveDistance()
{
   return (perspectiveDistance);
}

inline float GridRender::getHaze(float distance)
{
	// Haze values are inset a little to avoid some interpolation
	// problems with GFX.
	if (distance < hazeDistance)
      return (0.01f);
	else {
		// Distance haze
		float distFactor = (distance - hazeDistance) * hazeScale - 1.0;
		return 1.0-(distFactor*distFactor);      // simple quadratic
#if 0
	   return m_fog(distFactor,3.5f);
#endif
#if 0
		// Vertical haze
		float vertFactor;
		if (vector.z > hazeVertical.fMin) {
			if (vector.z < hazeVertical.fMax)
				vertFactor = 0.01f + (hazeVertical.fMax - vector.z) * hazeVerticalScale;
			else
				vertFactor = 0.01f;
		}
		else
			vertFactor = 1.0f;
		tv.fColor.haze *= vertFactor;
#endif
	}
}   


#endif
