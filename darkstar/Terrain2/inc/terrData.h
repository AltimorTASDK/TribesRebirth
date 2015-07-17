#include "types.h"
#include "g_bitmap.h"
#include "grdBlock.h"


struct GridSquare
{
   UInt16 minHeight;
   UInt16 maxHeight;
   UInt16 heightDeviance;
   UInt16 flags;
   enum {
      Split45 = 1,
      Empty = 2,
   };
};

struct TerrainBlock
{
   enum {
      BlockSize = 257,
      SquareMaxPoints = 1024,
      BlockShift = 8,
      BlockMask = 255,
      GridMapSize = 0x15555,
      MaxMipLevel = 6,
   };

   GridBlock::Material *materialMap;
   TSMaterialList *materialList;

   GFXBitmap *baseTextureMap;
   GFXBitmap *lightMap;
   GFXBitmap *dynamicLightMap;
   
   UInt8 *compositeBuffer;
   
   // fixed point height values
   UInt16 heightMap[BlockSize * BlockSize];
   GridSquare *gridMap[BlockShift+1];
   GridSquare *gridMapBase;
   DWORD paletteIndex;

   TerrainBlock();
   ~TerrainBlock();
   void buildGridMap();
   GridBlock::Material *getMaterial(int x, int y);
   GridSquare *findSquare(int level, Point2I pos);
   UInt16 getHeight(int x, int y) { return heightMap[x + y + (y << BlockShift)]; }
   UInt16 *getHeightAddress(int x, int y) { return heightMap + x + y + (y << BlockShift); }
   void checkActiveList(UInt32 frameIndex);
};

inline GridSquare *TerrainBlock::findSquare(int level, Point2I pos)
{
   return gridMap[level] + (pos.x >> level) + ((pos.y>>level) << (BlockShift - level));
}

inline GridBlock::Material *TerrainBlock::getMaterial(int x, int y)
{
   return materialMap + x + (y << BlockShift);
}

struct TerrainFile
{
   enum {
      FileSize = 3,
   };
   int squareSize;         // length/width scale factors
   float visibleDistance;
   float hazeDistance;
   float screenSize;

   void setVisibleDistance(float dist) { visibleDistance = dist; }
   void setHazeDistance(float dist) { hazeDistance = dist; }

   // editing interfacce functions
   TerrainBlock * getBlock(const Point2I & fPos, Point2I * bPos = 0);
   void setHeight(const Point2I & pos, float height);
   float getHeight(const Point2I & pos);
   void setMaterial(const Point2I & pos, const GridBlock::Material * material);
   GridBlock::Material * getMaterial(const Point2I & pos);
   void setLighting(const Point2I & pos, UInt16 color);
   UInt16 getLighting(const Point2I & pos);
   void setScreenSize(float size) { screenSize = size; }
   
   TerrainBlock *blockMap[FileSize][FileSize];
   void render(TSRenderContext &rc);
   void flushCache();

   TerrainFile();
   ~TerrainFile();
};

// 11.5 fixed point - gives us a height range from 0->2048 in 1/32 inc

inline float fixedToFloat(UInt16 val)
{
   return float(val) * 0.03125;
}
inline UInt16 floatToFixed(float val)
{
   return UInt16(val * 32.0);
}

