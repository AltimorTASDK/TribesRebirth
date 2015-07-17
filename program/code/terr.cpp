#include "types.h"
#include "ml.h"
#include "simbase.h"
#include "simRenderGrp.h"
#include "ts_pointarray.h"
#include "g_surfac.h"
#include "fearGlobals.h"
#include "simterrain.h"
#include "m_mul.h"

namespace Glide
{

bool hackTextureValid(GFXBitmap::CacheInfo *info, DWORD paletteIndex, GFXPalette *pal);
void hackDownloadBitmap(GFXBitmap *bmp, GFXPalette *pal, GFXBitmap::CacheInfo *info);
void hackSetLightScaleOffset(Point2F scale, Point2F offset);
void hackMode(GFXBitmap *lightMap);

};

float squareColors[8][2][3] = 
{
  { { 1, 0, 0 }, { 0, 1, 0 } },
  { { 1, 1, 0 }, { 0, 0, 1 } },
  { { 0.5, 0, 0 }, { 0, 0.5, 0 } },
  { { 0.5, 0.5, 0 }, { 0, 0, 0.5 } },
  { { 1, 0, 1 }, { 0, 1, 1 } },
  { { 0.5, 0, 0.5 }, { 0, 0.5, 0.5 } },
  { { 1, 0, 0 }, { 0, 1, 0 } },
  { { 1, 0, 0 }, { 0, 1, 0 } },
};

enum {
   ClampEdgeIndex = 0xFFFF,
   InvalidPointIndex = 0xFFFE,
   MaxScanEdges = 65535,
   MaxScanPoints = 65535,
   MaxEmitSquares = 16384,
   NumClipPlanes = 6, // near, left, right, top, bottom
};

struct ScanEdge
{
   UInt16 p1, p2, mp;
   UInt16 firstSubEdge;  // two sub edges for each edge, mp = InvalidPointIndex if none
};

struct ScanPoint
{
   UInt16 pointHeight, avgHeight;
   float minGrowFactor;
   UInt16 x, y;
   UInt16 p1, p2; // points we use to compute averageHeight
};

struct EmitSquare
{
   UInt16 edge[4];
   UInt8 x, y, level, clipFlags;
   UInt16 centerPoint;
   UInt8 flags;
   UInt8 mipLevel;
};

struct GridSquare
{
   UInt16 minHeight;
   UInt16 maxHeight;
   UInt16 heightDeviance;
   UInt16 flags;
   enum {
      Split45 = 1,
   };
};

struct Block
{
   enum {
      BlockSize = 257,
      SquareMaxPoints = 1024,
      BlockShift = 8,
      GridMapSize = 0x15555,
      TextureCacheEntrySize = 128 * 128 * 2,
   };

   struct TextureCacheEntry
   {
      GFXBitmap::CacheInfo info;
      int mipLevel;
      int detailLevel;
   };

   TextureCacheEntry textureCache[TextureCacheEntrySize];

   GridBlock::Material *materialMap;
   TSMaterialList *materialList;

   GFXBitmap *lightMap;
   UInt8 *compositeBuffer;
   
   UInt16 heightMap[BlockSize * BlockSize];
   GridSquare *gridMap[BlockShift+1];
   GridSquare *gridMapBase;
   DWORD paletteIndex;

   Block();
   ~Block();
   void buildGridMap();
   GridBlock::Material *getMaterial(int x, int y);
   GridSquare *findSquare(int level, Point2I pos);
   UInt16 getHeight(int x, int y) { return heightMap[x + y + (y << BlockShift)]; }
   TextureCacheEntry *findEntry(int x, int y, int detailLevel, int mipLevel);
};

Block::Block()
{
   lightMap = GFXBitmap::create(BlockSize - 1, BlockSize - 1, 16);
   compositeBuffer = new UInt8[65536];
   
   gridMapBase = new GridSquare[GridMapSize];
   int i;
   GridSquare * gs = gridMapBase;
   materialMap = NULL;
   
   for(i = BlockShift; i >= 0; i--)
   {
      gridMap[i] = gs;
      gs += 1 << (2 * (BlockShift - i));
   }
}

Block::~Block()
{
   delete lightMap;
   delete[] compositeBuffer;
   delete[] gridMapBase;
}

GridSquare *Block::findSquare(int level, Point2I pos)
{
   return gridMap[level] + (pos.x >> level) + ((pos.y>>level) << (BlockShift - level));
}

GridBlock::Material *Block::getMaterial(int x, int y)
{
   return materialMap + x + (y << BlockShift);
}

Block::TextureCacheEntry *Block::findEntry(int x, int y, int detailLevel, int mipLevel)
{
   TextureCacheEntry *ent = textureCache + (x >> 1) + ((y >> 1) << 7) + (detailLevel & 1) * 16384;
   if(ent->detailLevel != detailLevel || ent->mipLevel != mipLevel)
   {
      ent->info.clear();
      ent->detailLevel = detailLevel;
      ent->mipLevel = mipLevel;
   }
   return ent;
}

static int getPower(int x)
{
	// Returns 2^n (the highest bit).
	int i = 0;
	if (x)
		do
			i++;
		while (x >>= 1);
	return i;
}

static Point2F lightCoords[Block::SquareMaxPoints + 2];
static Point2F textureCoords[Block::SquareMaxPoints + 2];
static ScanEdge edges[MaxScanEdges];
static ScanPoint points[MaxScanPoints];
static EmitSquare squares[MaxEmitSquares];
static Block *currentBlock;
static int squareSize;
static float heightScale;
static float currentGrowFactor;
static Point2F blockPos;
static Point2I blockOffset;
static PlaneF clipPlane[NumClipPlanes];
static Point3F camPos;

static TS::PointArray *pointArray;
static TS::Camera *camera;
static GFXSurface *gfxSurface;

static int pointCount;
static int edgeCount;
static int squareCount;
static float pixelError;
static float squareDistance;

static float hazeDistance;
static float hazeScale;

extern float screenError;
extern float farDistance;
struct File
{
   enum {
      FileSize = 3,
   };
   int squareSize;       // length/width scale factors
   float heightScale;      // height scale factors

   Block *blockMap[FileSize][FileSize];
};

int TestSquareVisibility(GridSquare *square, int level, int mask, Point2I pos)
{
   int retMask = 0;
   for(int i = 0; i < NumClipPlanes; i++)
   {
      if(mask & (1 << i))
      {
         Point3F minPoint, maxPoint;
         minPoint.x = maxPoint.x = squareSize * pos.x + blockPos.x;
         minPoint.y = maxPoint.y = squareSize * pos.y + blockPos.y;

         if(clipPlane[i].x > 0)
            maxPoint.x += squareSize << level;
         else
            minPoint.x += squareSize << level;
         if(clipPlane[i].y > 0)
            maxPoint.y += squareSize << level;
         else
            minPoint.y += squareSize << level;
         if(clipPlane[i].z > 0)
         {
            minPoint.z = square->minHeight * heightScale;
            maxPoint.z = square->maxHeight * heightScale;
         }
         else
         {
            maxPoint.z = square->minHeight * heightScale;
            minPoint.z = square->maxHeight * heightScale;
         }
         float maxDot = m_dot(maxPoint, clipPlane[i]);
         float minDot = m_dot(minPoint, clipPlane[i]);
         float planeD = clipPlane[i].d;
         if(maxDot <= -planeD)
            return -1;
         if(minDot <= -planeD)
            retMask |= (1 << i);
      }
   }
   return retMask;
}

struct SquareStackNode
{
   int level;
   int clipFlags;
   int top, right, bottom, left;
   Point2I pos;
};

static inline float getHaze(float dist)
{
   if(dist < hazeDistance)
      return 0;
   else
   {
      float distFactor = (dist - hazeDistance) * hazeScale - 1.0;
      return 1.0 - distFactor * distFactor;
   }
}

static void transformPoints()
{
   pointArray->reset();
   ScanPoint *p = points;
   ScanPoint *end = points + pointCount;

   for(;p != end;p++)
   {
      UInt16 avgHeight = (points[p->p1].avgHeight + points[p->p2].avgHeight) >> 1;
      if(p->avgHeight == InvalidPointIndex)
         p->avgHeight = avgHeight;
      else
         p->avgHeight = p->pointHeight + p->minGrowFactor * (avgHeight - p->pointHeight);
      Point3F pos(p->x * squareSize, p->y * squareSize, p->avgHeight * heightScale);
      int index = pointArray->addPoint(pos);
	   TS::TransformedVertex& tv = pointArray->getTransformedVertex(index);
      float dist = (pos - camPos).len();
      tv.fDist = dist;
      if(dist > farDistance)
         tv.fStatus |= TS::ClipFarSphere;
      tv.fColor.haze = getHaze(dist);
   }
}

static int allocInitialPoint(Point3I pos)
{
   int ret = pointCount++;
   points[ret].minGrowFactor = 0;
   points[ret].pointHeight = pos.z;
   points[ret].avgHeight = pos.z;
   points[ret].x = pos.x;
   points[ret].y = pos.y;
   points[ret].p1 = 0;
   points[ret].p2 = 0;
   return ret;
}

static int allocPoint(Point2I pos, int p1, int p2)
{
   int ret = pointCount++;
   points[ret].minGrowFactor = currentGrowFactor;
   points[ret].pointHeight = *(currentBlock->heightMap + pos.x + pos.y + (pos.y << Block::BlockShift));
   points[ret].avgHeight = 0;
   points[ret].x = blockOffset.x + pos.x;
   points[ret].y = blockOffset.y + pos.y;
   points[ret].p1 = p1;
   points[ret].p2 = p2;
   return ret;
}

static int allocEdges(int count)
{
   int ret = edgeCount;
   edgeCount += count;

   for(int i = ret; i < ret + count; i++)
      edges[i].mp = InvalidPointIndex;
   return ret;
}

static void clampEdge(int edge)
{
   static int edgeStack[Block::BlockShift + 1];
   int stackSize = 0;
   for(;;)
   {
      int mp = edges[edge].mp;
      if(mp < InvalidPointIndex)
      {
         points[mp].minGrowFactor = 1;
         edgeStack[stackSize++] = edges[edge].firstSubEdge + 1;
         edge = edges[edge].firstSubEdge;
         continue;
      }
      else
         edges[edge].mp = ClampEdgeIndex;
      if(!stackSize--)
         break;
      edge = edgeStack[stackSize];
   }
}

static void subdivideEdge(int edge, Point2I pos)
{
   int mp = edges[edge].mp;
   if(mp >= InvalidPointIndex)
   {
      int p1 = edges[edge].p1;
      int p2 = edges[edge].p2;
      int midPoint = allocPoint(pos, p1, p2);
      int fse = allocEdges(2);
      
      if(mp == ClampEdgeIndex)
      {
         points[midPoint].minGrowFactor = 1;
         edges[fse].mp = ClampEdgeIndex;
         edges[fse+1].mp = ClampEdgeIndex;
      }

      //points[p1].minGrowFactor = max(points[p1].minGrowFactor, currentGrowFactor);
      //points[p2].minGrowFactor = max(points[p2].minGrowFactor, currentGrowFactor);

      edges[edge].mp = midPoint;
      edges[edge].firstSubEdge = fse;
      edges[fse].p1 = edges[edge].p1;
      edges[fse].p2 = midPoint;
      edges[fse+1].p1 = midPoint;
      edges[fse+1].p2 = edges[edge].p2;
   }
   else
   {
      points[mp].minGrowFactor = max(points[mp].minGrowFactor, currentGrowFactor);
   }
}

static bool subdivideSquare(GridSquare *sq, int level, Point2I pos)
{
   currentGrowFactor = 0;

   float blockX = pos.x * squareSize + blockPos.x;
   float blockY = pos.y * squareSize + blockPos.y;
   float blockZ = sq->minHeight * heightScale;
   
   float blockSize = squareSize * (1 << level);
   float blockHeight = (sq->maxHeight - sq->minHeight) * heightScale;

   Point3F vec;
   if(camPos.z < blockZ)
      vec.z = blockZ - camPos.z;
   else if(camPos.z > blockZ + blockHeight)
      vec.z = camPos.z - (blockZ + blockHeight);
   else
      vec.z = 0;

   if(camPos.x < blockX)
      vec.x = blockX - camPos.x;
   else if(camPos.x > blockX + blockSize)
      vec.x = camPos.x - (blockX + blockSize);
   else
      vec.x = 0;

   if(camPos.y < blockY)
      vec.y = blockY - camPos.y;
   else if(camPos.y > blockY + blockSize)
      vec.y = camPos.y - (blockY + blockSize);
   else
      vec.y = 0;

   float dist = vec.len();

   squareDistance = dist;
   
   if(dist < 1)
      return true;
   
   float projectedError = sq->heightDeviance * heightScale / dist;
   if(projectedError > pixelError)
   {
      if(projectedError < pixelError * 1.25)
         currentGrowFactor = 1 - (projectedError - pixelError) * 4 / pixelError;
      return true;
   }
   return false;
}

static void emitSquare(SquareStackNode *n, int centerPoint, int flags, int mipLevel)
{
   if(squareCount >= MaxEmitSquares)
      return;
   int square = squareCount++;
   squares[square].edge[0] = n->top;
   squares[square].edge[1] = n->right;
   squares[square].edge[2] = n->bottom;
   squares[square].edge[3] = n->left;
   squares[square].level = n->level;
   squares[square].clipFlags = n->clipFlags;
   squares[square].x = n->pos.x;
   squares[square].y = n->pos.y;
   squares[square].centerPoint = centerPoint;
   squares[square].flags = flags;
   squares[square].mipLevel = mipLevel;
}

void emitTriFan(int *pointList, int *lightList, int vertexCount)
{
   TS::VertexIndexPair vip[3];

   vip[0].fVertexIndex = pointList[0];
   vip[0].fTextureIndex = lightList[0];

   for(int i = 2; i < vertexCount; i++)
   {
      vip[1].fVertexIndex = pointList[i-1];
      vip[1].fTextureIndex = lightList[i-1];
      vip[2].fVertexIndex = pointList[i];
      vip[2].fTextureIndex = lightList[i];
      pointArray->drawPoly(3, vip, 0);
   }
}

////	{}, // Rotate
////	{}, // FlipX
////	{}, // Rotate | FlipX
////	{}, // FlipY
////	{}, // FlipY | Rotate
////	{}, // FlipY | FlipX
////	{}, // FlipY | Rotate | FlipX
Point2F LightTextureCoor[5] =
 { Point2F(0, 1), Point2F(1, 1), Point2F(1, 0), Point2F(0, 0), Point2F(0, 1) };

Point2F TextureCoor[8][5] =
{
 { Point2F(0, 0), Point2F(1, 0), Point2F(1, 1), Point2F(0, 1), Point2F(0, 0) },
};
   
inline void swap(Point2F& a,Point2F& b) {
	Point2F t = a; a = b; b = t;
}
static void buildTextureCoor()
{
	for (int i = 1; i < 8; i++) {
		Point2F* tex = TextureCoor[i];
			// Copy the unrotated to start with
		for (int p = 0; p < 4; p++)
			tex[p] = TextureCoor[0][p];
		//
		if (i & GridBlock::Material::FlipX) {
			swap(tex[0],tex[1]);
			swap(tex[2],tex[3]);
		}
		if (i & GridBlock::Material::FlipY) {
			swap(tex[0],tex[3]);
			swap(tex[1],tex[2]);
		}
		if (i & GridBlock::Material::Rotate) {
         Point2F temp = tex[0];
         tex[0] = tex[3];
         tex[3] = tex[2];
         tex[2] = tex[1];
         tex[1] = temp;
		}
      tex[4] = tex[0];
	}
}

extern void gridCreateMipMap( int _mipLevel, Box2I& _area,
	 Point2I& _matMapSize, GridBlock::Material* _matMap,
	 TSMaterialList* _matList, GFXBitmap* _pDstBM, GFXBitmap* _defTex );

extern bool gPointArrayHack;
extern Point2F *gPointArrayHackCoords; // lighting map coordinates

static void renderCurrentBlock(int firstSquare, int lastSquare)
{
   gPointArrayHack = true;
   static int pointBuffer[Block::SquareMaxPoints + 2];
   static int lightBuffer[Block::SquareMaxPoints + 2];
   
   static float lightScale = 1 / 256.0f;
   
   static int edgeDelta[4][2] =
   {
      {  1, 0  },
      {  0, -1 },
      { -1, 0  },
      {  0, 1  },
   };

   static int edgeStarts[4][2] =
   {
      { 0, 1 },
      { 1, 1 },
      { 1, 0 },
      { 0, 0 },
   };
   
   
   pointArray->useTextures(true);
   gPointArrayHackCoords = lightCoords;
   gfxSurface->setFillMode(GFX_FILL_TEXTURE);
   gfxSurface->setTexturePerspective(true);
   //gfxSurface->setTextureMap(currentBlock->lightMap);
   //gfxSurface->setTextureWrap(true);
   
   Glide::hackMode(currentBlock->lightMap);
   
   for(int squareIndex = firstSquare; squareIndex < lastSquare; squareIndex++)
   {
      EmitSquare *sq = squares + squareIndex;

      // loop through the edges on the square
      
      int edgePointIndex[4];  // index of the corner points
      int edgeStack[Block::BlockShift + 1];
      int levelStack[Block::BlockShift + 1];

      int curPoint = 1;  // optimization below - reserve space at beginning

      // if the square is split45, process it rotate process 90 deg,
      // so it goes right, bottom - left, top for triangles.
      
      int split45 = 0;
      if(sq->flags & GridSquare::Split45)
         split45 = 1;
      float textureScale = 1 / float(1 << sq->level);
      if(sq->level == 0)
      {
         GridBlock::Material *material = currentBlock->getMaterial(sq->x, sq->y);

         GFXBitmap *textureMap = (GFXBitmap *)currentBlock->materialList->getMaterial(material->index).getTextureMap();
         if(!Glide::hackTextureValid(&(textureMap->getCacheInfo()), textureMap->paletteIndex, gfxSurface->getPalette()))
            Glide::hackDownloadBitmap(textureMap, gfxSurface->getPalette(), &(textureMap->getCacheInfo()));
      
         pointArray->useTextures(TextureCoor[material->flags & GridBlock::Material::RotateMask] + split45);
         int topEdge = sq->edge[0];
         int bottomEdge = sq->edge[2];
         if(split45)
         {
            pointBuffer[0] = edges[topEdge].p2;
            pointBuffer[1] = edges[bottomEdge].p2;
            pointBuffer[2] = edges[bottomEdge].p1;
            pointBuffer[3] = edges[topEdge].p1;
         }
         else
         {
            pointBuffer[0] = edges[topEdge].p1;
            pointBuffer[1] = edges[topEdge].p2;
            pointBuffer[2] = edges[bottomEdge].p2;
            pointBuffer[3] = edges[bottomEdge].p1;
         }
         for(int i = 0; i < 4; i++)
         {
            int idx = (i + split45) & 0x03;
            lightBuffer[i] = i;
            lightCoords[i].set((sq->x + LightTextureCoor[idx].x) * lightScale,
                               (sq->y + LightTextureCoor[idx].y) * lightScale);
         }
         lightBuffer[4] = lightBuffer[0];
         emitTriFan(pointBuffer, lightBuffer, 4);  //(1, 2, 3) (1, 3, 4)
      }
      else
      {
         Block::TextureCacheEntry *textureCacheEntry
             = currentBlock->findEntry(sq->x, sq->y, sq->level, sq->mipLevel);
         
         if(!Glide::hackTextureValid(&textureCacheEntry->info, currentBlock->paletteIndex, gfxSurface->getPalette()))
         {
            GFXBitmap bmp;
            bmp.height = bmp.stride = bmp.width = 1 << sq->mipLevel;
            bmp.pBits = currentBlock->compositeBuffer;
            bmp.pMipBits[0] = bmp.pBits;
            bmp.detailLevels = 1;
            bmp.bitDepth = 8;
            Point2I mapSize(Block::BlockSize - 1, Block::BlockSize - 1);
            Box2I mapArea;
            mapArea.fMin.x = sq->x;
            mapArea.fMin.y = sq->y;
            mapArea.fMax.x = sq->x + (1 << sq->level) - 1;
            mapArea.fMax.y = sq->y + (1 << sq->level) - 1;
            gridCreateMipMap(7 - (sq->mipLevel - sq->level), mapArea, mapSize, currentBlock->materialMap, currentBlock->materialList, &bmp, 0);
            bmp.paletteIndex = currentBlock->paletteIndex;
            Glide::hackDownloadBitmap(&bmp, gfxSurface->getPalette(), &textureCacheEntry->info);
         }
         pointArray->useTextures(textureCoords);
         int squareSize = 1 << sq->level;
         for(int squareEdge = 0; squareEdge < 4; squareEdge++)
         {
            // edge+1 if split45, else edge
            int edgeIndex = (squareEdge + split45) & 0x3;
            // fill the point buffer with the recursed edges
            edgePointIndex[squareEdge] = curPoint;
            int stackSize = 0;
            int level = sq->level;
            int x = edgeStarts[edgeIndex][0] << level;
            int y = edgeStarts[edgeIndex][1] << level;
         
            int curEdge = sq->edge[edgeIndex];

            for(;;) {
               if(edges[curEdge].mp < InvalidPointIndex)
               {
                  // push the tail of the edge onto the stack
                  // tail is firstSubEdge + 1 if top, right
                  // or firstSubEdge if bottom, left
                  level--;
                  levelStack[stackSize] = level;
                  if(edgeIndex < 2)
                  {
                     curEdge = edges[curEdge].firstSubEdge;
                     edgeStack[stackSize++] = curEdge + 1;
                  }
                  else
                  {
                     curEdge = edges[curEdge].firstSubEdge + 1;
                     edgeStack[stackSize++] = curEdge - 1;
                  }
               }
               else
               {
                  // top and right edges go p1->p2
                  // bottom and left edges go p2->p1
                  lightCoords[curPoint].set((sq->x + x) * lightScale, (sq->y + y) * lightScale);
                  textureCoords[curPoint].set(x * textureScale, (squareSize - y) * textureScale);
                  pointBuffer[curPoint++] = 
                     edgeIndex < 2 ? edges[curEdge].p1 : edges[curEdge].p2;
                  x += edgeDelta[edgeIndex][0] << level;
                  y += edgeDelta[edgeIndex][1] << level;
                  if(stackSize)
                  {
                     curEdge = edgeStack[--stackSize];
                     level = levelStack[stackSize];
                  }
                  else 
                     break;
               }
            }
         }
         ColorF c;
         float *co = squareColors[sq->level][split45];
         c.red = co[0];
         c.green = co[1];
         c.blue = co[2];
         gfxSurface->setFillColor(&c);

         // early out for strict square:
         for(int i = 0; i < curPoint; i++)
            lightBuffer[i] = i;
         if(curPoint == 5) // four point square
         {
            // indices are in pointBuffer
            emitTriFan(pointBuffer+1, lightBuffer+1, 4);  //(1, 2, 3) (1, 3, 4)
         }
         else
         {
            int centerOffset = 1 << (sq->level - 1);
            pointBuffer[0] = sq->centerPoint;
            lightCoords[0].set((sq->x + centerOffset) * lightScale, (sq->y + centerOffset) * lightScale );
            textureCoords[0].set(centerOffset * textureScale, centerOffset * textureScale );
            pointBuffer[curPoint] = pointBuffer[1];
            lightBuffer[curPoint] = 1;
            emitTriFan(pointBuffer, lightBuffer, curPoint+1);
         }
      }
   }
   gPointArrayHack = false;
   gfxSurface->setTextureWrap(false);
}

static void processCurrentBlock(int topEdge, int rightEdge, int bottomEdge, int leftEdge)
{
   SquareStackNode stack[Block::BlockShift*4];

   stack[0].level = Block::BlockShift;
   stack[0].clipFlags = 0xFF;  // test all the planes
   stack[0].pos.set(0,0);
   stack[0].top = topEdge;
   stack[0].right = rightEdge;
   stack[0].bottom = bottomEdge;
   stack[0].left = leftEdge;
   
   int curStackSize = 1;
   while(curStackSize)
   {
      SquareStackNode *n = stack + curStackSize - 1;
      // see if it's visible
      int clipFlags = n->clipFlags;
      Point2I pos = n->pos;
      GridSquare *sq = currentBlock->findSquare(n->level, pos);
      if(clipFlags)
      {
         clipFlags = TestSquareVisibility(sq, n->level, n->clipFlags, n->pos);
         if(clipFlags == -1)
         {
            // trivially rejected, so pop it off the stack
            curStackSize--;
            continue;
         }
      }
      // bottom level square or not subdivided means emit
      // the square.
      
      if(n->level == 0)
      {
         emitSquare(n, InvalidPointIndex, sq->flags, 0);
         curStackSize--;
         continue;
      }
      
      int p1, p2;
      int top = n->top;
      int right = n->right;
      int bottom = n->bottom;
      int left = n->left;

      // subdivide this square and throw it on the stack
      int squareSize = 1 << n->level;
      int squareHalfSize = squareSize >> 1;

      if(sq->flags & GridSquare::Split45)
      {
         p1 = edges[top].p2;
         p2 = edges[bottom].p1;
      }
      else
      {
         p1 = edges[top].p1;
         p2 = edges[bottom].p2;
      }
      
      bool subdiv = subdivideSquare(sq, n->level, n->pos);
      int midPoint = allocPoint(Point2I(pos.x + squareHalfSize, pos.y + squareHalfSize), p1, p2);

      if(!subdiv)
      {
         points[midPoint].avgHeight = InvalidPointIndex;
         clampEdge(top);
         clampEdge(right);
         clampEdge(bottom);
         clampEdge(left);
         int mipLevel = 7;
         
         if(squareDistance > 0.001)
         {
            int size = int(camera->projectRadius(squareDistance, squareSize << n->level));
            mipLevel = getPower(size + (size >> 1));
         }
         if(mipLevel < n->level)
            mipLevel = n->level;
         else if(mipLevel > 7)
            mipLevel = 7;
         emitSquare(n, midPoint, sq->flags, mipLevel);
         curStackSize--; // we're good
         continue;
      }

      int nextLevel = n->level - 1;

      subdivideEdge(top, Point2I(pos.x + squareHalfSize, pos.y + squareSize));
      subdivideEdge(right, Point2I(pos.x + squareSize, pos.y + squareHalfSize));
      subdivideEdge(bottom, Point2I(pos.x + squareHalfSize, pos.y));
      subdivideEdge(left, Point2I(pos.x, pos.y + squareHalfSize));
   
      // cross edges go top, right, bottom, left
      int firstCrossEdge = allocEdges(4);
      
      edges[firstCrossEdge].p1 = edges[top].mp;
      edges[firstCrossEdge].p2 = midPoint;
      edges[firstCrossEdge+1].p1 = midPoint;
      edges[firstCrossEdge+1].p2 = edges[right].mp;
      edges[firstCrossEdge+2].p1 = midPoint;
      edges[firstCrossEdge+2].p2 = edges[bottom].mp;
      edges[firstCrossEdge+3].p1 = edges[left].mp;
      edges[firstCrossEdge+3].p2 = midPoint;
      
      for(int i = 0; i < 4; i++)
      {
         n[i].level = nextLevel;
         n[i].clipFlags = clipFlags;
      }

      n[0].pos = pos;
      n[0].top = firstCrossEdge + 3;
      n[0].right = firstCrossEdge + 2;
      n[0].bottom = edges[bottom].firstSubEdge;
      n[0].left = edges[left].firstSubEdge + 1;

      n[1].pos.set(pos.x + squareHalfSize, pos.y);
      n[1].top = firstCrossEdge + 1;
      n[1].right = edges[right].firstSubEdge + 1;
      n[1].bottom = edges[bottom].firstSubEdge + 1;
      n[1].left = firstCrossEdge + 2;

      n[2].pos.set(pos.x + squareHalfSize, pos.y + squareHalfSize);
      n[2].top = edges[top].firstSubEdge + 1;
      n[2].right = edges[right].firstSubEdge;
      n[2].bottom = firstCrossEdge + 1;
      n[2].left = firstCrossEdge;

      n[3].pos.set(pos.x, pos.y + squareHalfSize);
      n[3].top = edges[top].firstSubEdge;
      n[3].right = firstCrossEdge;
      n[3].bottom = firstCrossEdge + 3;
      n[3].left = edges[left].firstSubEdge;
      curStackSize += 3;
   }
}

static void renderFile(File *file)
{
   pointCount = 0;
   edgeCount = 0;
   squareCount = 0;
   
   squareSize = file->squareSize;
   heightScale = file->heightScale;
   
   currentBlock = file->blockMap[0][0];
   currentGrowFactor = 0;
   blockPos.set(0,0);
   
   // compute pixelError
   if(screenError >= 0.001)
      pixelError = 1 / camera->projectRadius(screenError, 1);
   else
      pixelError = 0;
      
   // set up clipping planes:
   
	TMat3F camTCW = camera->getTOC();
	camTCW.inverse();

	const RectF& vp = camera->getWorldViewport();
	float cx = (vp.lowerR.x - vp.upperL.x) / 2.0f;
	float cy = (vp.upperL.y - vp.lowerR.y) / 2.0f;
	float nearDistance;
   
   nearDistance = camera->getNearDist();

   Point3F origin(0,0,0);
   Point3F farOrigin(0,farDistance,0);
   Point3F upperLeft(-cx, nearDistance, cy);
   Point3F upperRight(cx, nearDistance, cy);
   Point3F lowerLeft(-cx, nearDistance, -cy);
   Point3F lowerRight(cx, nearDistance, -cy);
           
   Point3F xlOrigin, xlFarOrigin, xlUpperLeft, xlUpperRight, xlLowerLeft, xlLowerRight;

   m_mul(origin, camTCW, &xlOrigin);
   m_mul(farOrigin, camTCW, &xlFarOrigin);
   m_mul(upperLeft, camTCW, &xlUpperLeft);
   m_mul(upperRight, camTCW, &xlUpperRight);
   m_mul(lowerLeft, camTCW, &xlLowerLeft);
   m_mul(lowerRight, camTCW, &xlLowerRight);

   // build the 6 planes:
   
   // near plane
   clipPlane[0].set(xlUpperRight, xlUpperLeft, xlLowerRight); 

   // far plane is revers of near plane vector
   clipPlane[1].x = -clipPlane[0].x;
   clipPlane[1].y = -clipPlane[0].y;
   clipPlane[1].z = -clipPlane[0].z;

   // ax + by + cz + d = 0
   // n dot ptOnPlane + d = 0
   // d = - n dot ptOnPlane
   
   clipPlane[1].d = - m_dot(xlFarOrigin, clipPlane[1]);
   
   // left clip plane
   clipPlane[2].set(xlLowerLeft, xlOrigin, xlUpperLeft); 
   // right clip plane
   clipPlane[3].set(xlUpperRight, xlOrigin, xlLowerRight); 
   // top clip plane
   clipPlane[4].set(xlUpperLeft, xlOrigin, xlUpperRight); 
   // bottom clip plane
   clipPlane[5].set(xlLowerRight, xlOrigin, xlLowerLeft); 

   int p0, p1, p2, p3;
   
   int blockPoints[File::FileSize+1][File::FileSize+1];
   int horzEdges[File::FileSize+1][File::FileSize];
   int vertEdges[File::FileSize+1][File::FileSize];
   
   int i, j;
   for(i = 0; i < File::FileSize + 1; i++)
   {
      for(j = 0; j < File::FileSize + 1; j++)
      {
         int x = i % File::FileSize, y = j % File::FileSize;
         UInt16 height = file->blockMap[x][y]->heightMap[0];
         blockPoints[i][j] = allocInitialPoint(Point3I(i << Block::BlockShift, j << Block::BlockShift, height));
      }
   }
   for(i = 0; i < File::FileSize + 1; i++)
   {
      for(j = 0; j < File::FileSize; j++)
      {
         int e = allocEdges(1);
         edges[e].p1 = blockPoints[j][i];
         edges[e].p2 = blockPoints[j+1][i];
         horzEdges[i][j] = e;
         e = allocEdges(1);
         edges[e].p1 = blockPoints[i][j+1];
         edges[e].p2 = blockPoints[i][j];
         vertEdges[i][j] = e;
      }
   }
   int lastSquare[File::FileSize][File::FileSize];
   // process all blocks
   for(i = 0; i < File::FileSize; i++)
   {
      for(j = 0; j < File::FileSize; j++)
      {
         blockOffset.set(i << Block::BlockShift, j << Block::BlockShift);
         blockPos.set(i * squareSize * (1 << Block::BlockShift), j * squareSize * (1 << Block::BlockShift));
         currentBlock = file->blockMap[i][j];
         processCurrentBlock(horzEdges[j+1][i], vertEdges[i+1][j], horzEdges[j][i], vertEdges[i][j]);
         lastSquare[i][j] = squareCount;
      }
   }

   // transform the points
   transformPoints();

   // render all blocks
   int firstSquare = 0;
   for(i = 0; i < File::FileSize; i++)
   {
      for(j = 0;j < File::FileSize; j++)
      {
         currentBlock = file->blockMap[i][j];
         renderCurrentBlock(firstSquare, lastSquare[i][j]);
         firstSquare = lastSquare[i][j];
      }
   }
}

File *createTestFile()
{
   buildTextureCoor();
   File *ret = new File;
   ret->squareSize = 8;
   ret->heightScale = 1;
   
   int i, j;

   SimTerrain *t = (SimTerrain *) cg.manager->findObject(8);
   if(t)
   {
      GridFile *gf = t->getGridFile();
      GridBlock *block = gf->getBlock(Point2I(0,0));
      float max = gf->getHeightRange(false).fMax;
      ret->heightScale = max / 65535.0f;
      Block *blk = new Block;
      UInt16 *lm = block->getLightMap();
      GFXBitmap *bmp = blk->lightMap;
      blk->materialMap = block->getMaterialMap();
      blk->materialList = gf->getMaterialList();
      blk->paletteIndex = blk->materialList->getMaterial(1).getTextureMap()->paletteIndex;
            
      for(i = 0; i < Block::BlockSize; i++)
      {
         for(j = 0; j < Block::BlockSize; j++)
         {
            float height = block->getHeight(0, Point2I(i, j))->height / ret->heightScale;
            if(height > 65535.0f)
               height = 65535.0f;
                           
            blk->heightMap[i + j + (j << Block::BlockShift)] = height;
            UInt16 *ptr = (UInt16 *) bmp->getAddress(i,j);
            *ptr = lm[i + j + (j << Block::BlockShift)];
         }
      }
      blk->buildGridMap();
      for(i = 0; i < File::FileSize; i ++)
         for(j = 0; j < File::FileSize; j ++)
            ret->blockMap[i][j] = blk;
   }
   else
   {
      // create circular cone in the middle of the map:
      Block *blk = new Block;

      for(i = 0; i < Block::BlockSize; i++)
      {
         for(j = 0; j < Block::BlockSize; j++)
         {
            float dist = sqrt((64 - i) * (64 - i) + (64 - j) * (64 - j));
            dist /= 64.0f;
         
            if(dist > 1)
               dist = 1;
   
            blk->heightMap[i + j + (j << Block::BlockShift)] = (1 - dist) * (1 - dist) * 300;
         }
      }
      blk->buildGridMap();
      for(i = 0; i < File::FileSize;i++)
         for(j = 0; j < File::FileSize; j++)
            ret->blockMap[i][j] = blk;
   }
   return ret;
}

static UInt16 calcDev(PlaneF &pl, Point3F &pt)
{
   float z = (pl.d + pl.x * pt.x + pl.y * pt.y) / -pl.z;
   float diff = z - pt.z;
   if(diff < 0)
      diff = -diff;
      
   if(diff > 0xFFFF)
      return 0xFFFF;
   else
      return UInt16(diff);
}

UInt16 Umax(UInt16 u1, UInt16 u2)
{
   return u1 > u2 ? u1 : u2;
}

void Block::buildGridMap()
{
   for(int i = BlockShift; i >= 0; i--)
   {
      int squareCount = 1 << (BlockShift - i);
      int squareSize = (Block::BlockSize - 1) / squareCount;
      
      for(int squareX = 0; squareX < squareCount; squareX++)
      {
         for(int squareY = 0; squareY < squareCount; squareY++)
         {
            UInt16 min = 0xFFFF;
            UInt16 max = 0;
            UInt16 mindev45 = 0;
            UInt16 mindev135 = 0;
            
            Point3F p1, p2, p3, p4;

            // determine max error for both possible splits.
            PlaneF pl1, pl2, pl3, pl4;


            p1.set(0, 0, getHeight(squareX * squareSize, squareY * squareSize));
            p2.set(0, squareSize, getHeight(squareX * squareSize, squareY * squareSize + squareSize));
            p3.set(squareSize, squareSize, getHeight(squareX * squareSize + squareSize, squareY * squareSize + squareSize));
            p4.set(squareSize, 0, getHeight(squareX * squareSize + squareSize, squareY * squareSize));

            // pl1, pl2 = split45, pl3, pl4 = split135
            pl1.set(p1, p2, p3);
            pl2.set(p1, p3, p4);
            pl3.set(p1, p2, p4);
            pl4.set(p2, p3, p4);
            bool parentSplit45 = false;
            GridSquare *parent = NULL;
            if(i < BlockShift)
            {
               parent = findSquare(i+1, Point2I(squareX * squareSize, squareY * squareSize));
               parentSplit45 = parent->flags & GridSquare::Split45;
            }
            for(int sizeX = 0; sizeX <= squareSize; sizeX++)
            {
               for(int sizeY = 0; sizeY <= squareSize; sizeY++)
               {
                  int x = squareX * squareSize + sizeX;
                  int y = squareY * squareSize + sizeY;
                  
                  UInt16 ht = heightMap[x + y + (y << BlockShift)];

                  if(ht < min)
                     min = ht;
                  if(ht > max)
                     max = ht;
                  Point3F pt(sizeX, sizeY, ht);
                  UInt16 dev, dev135;

                  if(sizeX < sizeY)
                     dev = calcDev(pl1, pt);
                  else if(sizeX > sizeY)
                     dev = calcDev(pl2, pt);
                  else
                     dev = Umax(calcDev(pl1, pt), calcDev(pl2, pt));

                  if(dev > mindev45)
                     mindev45 = dev;
                     
                  if(sizeX + sizeY < squareSize)
                     dev = calcDev(pl3, pt);
                  else if(sizeX + sizeY > squareSize)
                     dev = calcDev(pl4, pt);
                  else
                     dev = Umax(calcDev(pl3, pt), calcDev(pl4, pt));
                  
                  if(dev > mindev135)
                     mindev135 = dev;
               }
            }
            GridSquare *sq = findSquare(i, Point2I(squareX * squareSize, squareY * squareSize));
            sq->minHeight = min;
            sq->maxHeight = max;
            sq->flags = 0;
            
            bool shouldSplit45 = ((squareX ^ squareY) & 1) == 0;
            bool split45;
   
            //split45 = shouldSplit45;          
            if(i == 0)
               split45 = shouldSplit45;
            else if(i < 4 && shouldSplit45 == parentSplit45)
               split45 = shouldSplit45;
            else
               split45 = mindev45 < mindev135;
            
            if(split45)
            {
               sq->flags |= GridSquare::Split45;
               sq->heightDeviance = mindev45;
            }
            else
               sq->heightDeviance = mindev135;
            if(parent)
               if(parent->heightDeviance < sq->heightDeviance)
                  parent->heightDeviance = sq->heightDeviance;
         }
      }
   }
}

class SimT2 : public SimObject, SimRenderImage
{
public:
   bool onAdd();
   File *file;
   void onRemove();
   bool processQuery(SimQuery *query);
   void render(TSRenderContext &rc);
   void onDeleteNotify(SimObject *);
   TMat3F transform;
   
   DECLARE_PERSISTENT(SimT2);
};

IMPLEMENT_PERSISTENT(SimT2);

extern bool gRenderSimTerrain;

bool SimT2::onAdd()
{
   file = createTestFile();
   itype = SimRenderImage::Normal;
   SimTerrain *terr = (SimTerrain*) manager->findObject(8);
   if(terr)
   {
      transform = terr->getTransform();
      farDistance = terr->getVisibleDistance();
      hazeDistance = terr->getHazeDistance();
      hazeScale = 1.0 / (farDistance - hazeDistance);
      deleteNotify(terr);
   }
   gRenderSimTerrain = false;
   addToSet(SimRenderSetId);
   return SimObject::onAdd();
}

void SimT2::onRemove()
{
   gRenderSimTerrain = true;
   SimObject::onRemove();
}

void SimT2::onDeleteNotify(SimObject *)
{
   deleteObject();
}

bool SimT2::processQuery(SimQuery *query)
{
   switch (query->type) {
      case SimRenderQueryImageType: {
         SimRenderQueryImage *q = static_cast<SimRenderQueryImage *>(query);
         if (q->containerRenderQuery == false) 
         {
            // Container system is querying...
            q->image[0] = this;
            q->count = 1;
         }
         break;
      }
      default:
         return false;
   }
   return true;
}

void SimT2::render(TSRenderContext &rc)
{
   pointArray = rc.getPointArray();
   gfxSurface = rc.getSurface();
   camera = rc.getCamera();
   camera->pushTransform(transform);

	gfxSurface->setFillMode(GFX_FILL_CONSTANT);
   gfxSurface->setHazeSource(GFX_HAZE_NONE);
   gfxSurface->setShadeSource(GFX_SHADE_NONE);
   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
   gfxSurface->setTexturePerspective(FALSE);
	gfxSurface->setTransparency(FALSE);

   pointArray->useIntensities(false);
   pointArray->useTextures(false);
   pointArray->useHazes(true);
   pointArray->setFarDist(farDistance);
   gfxSurface->setHazeSource(GFX_HAZE_VERTEX);

   TMat3F mat = camera->getTOC();
	mat.inverse();
	camPos = mat.p;

   renderFile(file);
 	camera->popTransform();
   gfxSurface->setHazeSource(GFX_HAZE_NONE);
}   

