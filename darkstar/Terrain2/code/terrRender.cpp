#include "types.h"
#include "ml.h"
#include "ts_pointarray.h"
#include "g_surfac.h"
#include "m_mul.h"
#include "terrData.h"
#include "Console.h"

namespace Glide
{

bool hackTextureValid(GFXBitmap::CacheInfo *info, DWORD paletteIndex, GFXPalette *pal);
void hackDownloadBitmap(GFXBitmap *bmp, GFXPalette *pal, GFXBitmap::CacheInfo *info, bool conv=false);
void hackSetLightScaleOffset(Point2F scale, Point2F offset);
void hackMode(GFXBitmap *lightMap, bool additive, GFXPalette *pal);
void hackSetAlpha(bool first, float r, float g, float b, float a);
void hackUndoAlphaMode();
void hackDownloadLightmap(GFXBitmap *lm);
void hackSetLightmap(GFXBitmap *lm);
void hackReloadBitmap(GFXBitmap *lm, GFXPalette *pal);
};

extern void hackSetOutlineMode(bool mode);

struct CacheBitmap
{
   GFXBitmap *bitmap;
   int x, y, mipLevel, level;
   CacheBitmap *next;
};

struct SquareStackNode
{
   UInt16 level;
   UInt16 clipFlags;
   UInt32 lightMask;
   int top, right, bottom, left;
   Point2I pos;
};

struct WaterStackNode
{
   Point2F position, extent;
   int clipFlags;
   UInt32 lightMask;
   int top, right, bottom, left;
};

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

struct Detail
{
   float detailDistance;
   float maxDistance;
   float minDistance;
   float distanceScale;
   int level;
};

struct TerrLightInfo
{
   Point3F pos; // world position
   float radius; // radius of the light
   float radiusSquared; // radius^2
   float r, g, b;

   float distSquared; // distance to camera
};

enum
{
   MaxTerrainLights = 64,
   MaxVisibleLights = 31,
};

static TerrLightInfo terrainLights[MaxTerrainLights];

enum {
   ClampEdgeIndex = 0xFFFF,
   InvalidPointIndex = 0xFFFE,
   MaxScanPoints = 65535,
   MaxScanEdges = 65535,
   ScanEdgeLimit = 65520,
   ScanPointLimit = 65520,
   MaxEmitSquares = 16384,
   NumClipPlanes = 6, // near, left, right, top, bottom
   DynamicLightMapSize = 8,
   DynamicLightMapShift = 3,
   MaxWaterStackSize = 512,
   EdgeStackSize = 1024, // value for water/terrain edge stack size.
   MaxWaves = 8,
   MaxDetailLevel = 9,
   MaxMipLevel = 8,
};

struct ScanEdge
{
   UInt16 p1, p2, mp;
   UInt16 firstSubEdge;  // two sub edges for each edge, mp = InvalidPointIndex if none
};

struct ScanPoint
{
   float pointHeight, avgHeight;
   float minGrowFactor;
   UInt16 p1, p2; // points we use to compute averageHeight
};

struct EmitSquare
{
   UInt16 edge[4];
   UInt16 x, y;
   UInt8 level;
   UInt8 clipFlags;
   UInt8 flags;
   UInt8 mipLevel;
   UInt32 lightMask;
   enum {
      Split45 = 1,
      Recache = 2,
      DynamicallyLit = 4,
      DynamicLightFlagShift = 2,
   };
};

struct Color
{
   int r, g, b;
   float z;
};

struct TerrainRenderState
{
   Point2F lightCoords[TerrainBlock::SquareMaxPoints + 2];
   Point2F textureCoords[TerrainBlock::SquareMaxPoints + 2];
   CacheBitmap *activeTextureList;
   CacheBitmap *freeList[MaxMipLevel + 1];
   
   ScanEdge *edges;
   ScanPoint *points;

   Point2F *waterTexCoords;
   Point2F *envTexCoords;
   
   EmitSquare *squares;
   Point3F *emitPoints;
   TerrainBlock *currentBlock;
   int squareSize;
   float screenSize;
   UInt32 frameIndex;
   
   Point2F squareScale;
   
   float currentGrowFactor;
   Point2F blockPos;
   Point2I blockOffset;
   PlaneF clipPlane[NumClipPlanes];
   Point3F camPos;
   UInt32 dynamicLightCount;

   TSRenderContext *renderContext;
   TS::PointArray *pointArray;
   TS::Camera *camera;
   GFXSurface *gfxSurface;

   int pointCount;
   int edgeCount;
   int squareCount;
   int cameraSquare;
   
   float pixelError;
   float waterScale;
   float waterFreq;
   
   int waveCount;
   Point4F waveParameters[MaxWaves];

   float farDistance;
   float hazeDistance;
   float hazeScale;
   float minWaterSquareSize;
   Point2F texScale;
   Point2F LightTextureCoor[5];
   Point2F TextureCoor[8][5];
   Color lmaBuffer[DynamicLightMapSize + 1][DynamicLightMapSize + 1];  // light map accumulation buffer
   Detail detailData[MaxDetailLevel + 1];

   TerrainRenderState();
} trs;

inline void swap(Point2F& a,Point2F& b) {
	Point2F t = a; a = b; b = t;
}

TerrainRenderState::TerrainRenderState()
{
   for(int i = 0; i <= MaxMipLevel; i++)
      freeList[i] = NULL;
   activeTextureList = NULL;
   
   edges = new ScanEdge[MaxScanEdges];
   points = new ScanPoint[MaxScanPoints];
   waterTexCoords = new Point2F[MaxScanPoints];
   envTexCoords = new Point2F[MaxScanPoints];
   squares = new EmitSquare[MaxEmitSquares];
   emitPoints = new Point3F[MaxScanPoints];
   frameIndex = 0;

   LightTextureCoor[0].set(0, 1);   
   LightTextureCoor[1].set(1, 1);   
   LightTextureCoor[2].set(1, 0);   
   LightTextureCoor[3].set(0, 0);   
   LightTextureCoor[4].set(0, 1);   

   TextureCoor[0][0].set(0, 0);
   TextureCoor[0][1].set(1, 0);
   TextureCoor[0][2].set(1, 1);
   TextureCoor[0][3].set(0, 1);
   TextureCoor[0][4].set(0, 0);

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

static float screenError = 9;
static float screenSize = 45;
static float minSquareSize = 4;
static float farDistance = 500;
extern bool gRenderSimTerrain;

static int dynamicTextureCount;
static int unusedTextureCount;
static int textureSpaceUsed;
static int levelZeroCount;
static int fullMipCount;
static int staticTextureCount;
static int staticTSU;

void initTerrainVars()
{
   Console->addVariable(0, "T2::dynamicTextureCount", CMDConsole::Int, &dynamicTextureCount);
   Console->addVariable(0, "T2::unusedTextureCount", CMDConsole::Int, &unusedTextureCount);
   Console->addVariable(0, "T2::textureSpaceUsed", CMDConsole::Int, &textureSpaceUsed);

   Console->addVariable(0, "T2::staticTextureCount", CMDConsole::Int, &staticTextureCount);
   Console->addVariable(0, "T2::levelZeroCount", CMDConsole::Int, &levelZeroCount);
   Console->addVariable(0, "T2::fullMipCount", CMDConsole::Int, &fullMipCount);
   Console->addVariable(0, "T2::staticTSU", CMDConsole::Int, &staticTSU);

   Console->addVariable(0, "screenError", CMDConsole::Float, &screenError);
   Console->addVariable(0, "screenSize", CMDConsole::Float, &screenSize);
   Console->addVariable(0, "minSquareSize", CMDConsole::Float, &minSquareSize);
   Console->addVariable(0, "farDistance", CMDConsole::Float, &farDistance);
   Console->addVariable(0, "renderSimTerrain", CMDConsole::Bool, &gRenderSimTerrain);
}   

struct TerrainRender
{
   static void flushCache();
   static void cull();
   static int compareTexture(EmitSquare *sq, CacheBitmap *ci);
   static void sort();
   static void prepareLighting(EmitSquare *sq, float &scale, Point2F &offset);
   static int renderLevelZero();
   static void renderEdgedSquare(EmitSquare *sq);
   static int renderFullMip(int squareIndex);
   static int renderLevelNonzero(int squareIndex);
   static CacheBitmap *allocTexture(int mipLevel);
   static void freeTexture(CacheBitmap *cb);
   static void texture(int squareIndex);
      
   static void expandLightMap(int x, int y, int level);
   static void expandDynamicLight(TerrLightInfo &tl, int x, int y, int level);
   static void collapseLightMap();

   static void lightExpandHorz(float x, float ySquared, float dx, int j, int i, int shift);
   static void lightExpandVert(float y, float xSquared, float dy, int j, int i, int shift);
   static void setupColorAdder(float rSquared, float r, float g, float b);
   static void addColor(Color &c, float dSquared);
   static void lmExpandVert(int y, int x, int shift);
   static void lmExpandHorz(int y, int x, int shift);
   static inline void convertColor(Color &c, UInt16 lix);
   static inline UInt16 convertColorBack(Color &c);
   static UInt32 TestSquareLights(GridSquare *sq, int level, Point2I pos, UInt32 lightMask);
   static int TestSquareVisibility(Point3F &min, Point3F &max, int clipMask, float expand);
   static inline float getHaze(float dist);
   static void transformPoints();
   static void transformWaterPoints();
   static int allocInitialPoint(Point3F pos);
   static int allocPoint(Point2I pos, int p1, int p2);
   static int allocWaterPoint(Point2F pos, int p1, int p2);
   static int allocEdges(int count);
   static void clampEdge(int edge);
   static void subdivideEdge(int edge, Point2I pos);
   static void subdivideWaterEdge(int edge, Point2F pos);
   static float getSquareDistance(Point3F &minPoint, Point3F &maxPoint);
   
   static bool subdivideSquare(GridSquare *sq, int level, Point2I pos);
   static void emitSquare(SquareStackNode *n, int flags, int mipLevel, UInt32 lightMask);
   static void emitWaterSquare(WaterStackNode *n);
   static void emitTriFan(int *pointList, int *lightList, int vertexCount);
   static void emitTriFanReverse(int *pointList, int *lightList, int vertexCount);
   static void buildTextureCoor();
   static void renderCurrentBlock(int firstSquare, int lastSquare);
   static void renderWaterBlock(bool under);
   static void processCurrentBlock(int topEdge, int rightEdge, int bottomEdge, int leftEdge);
   static void buildLightArray();
   static float getWaterHeight(Point2F pos, int pointIndex);
   static void buildClippingPlanes();
   static void buildDetailTable();
   static void renderWater(TSRenderContext &rc, Point2F extent, Point2F texScale, float height, TSMaterialList *matList, Point4F *params, int paramCount, ColorF waterColor, float waterAlpha);
   static void processWaterBlock(Point2F extent, float height);
};

float currentR, currentG, currentB, currentRSq;

void TerrainRender::setupColorAdder(float rSquared, float r, float g, float b)
{
   currentR = r;
   currentG = g;
   currentB = b;
   currentRSq = rSquared;
}

void TerrainRender::addColor(Color &c, float dSquared)
{
   if(dSquared > currentRSq)
      return;
   float scale = 255.0f * ( 1 - (dSquared / currentRSq) );
   c.r += currentR * scale;
   c.g += currentG * scale;
   c.b += currentB * scale;
}

inline UInt16 TerrainRender::convertColorBack(Color &c)
{
   UInt16 ret;
   if(c.r > 255)
      ret = 0xF800;
   else
      ret = (c.r >> 3) << 11;
   if(c.g > 255)
      ret |= 0x07E0;
   else
      ret |= (c.g >> 2) << 5;
   
   if(c.b > 255)
      ret |= 0x001F;
   else
      ret |= c.b >> 3;
      
   return ret;
}

inline void TerrainRender::convertColor(Color &c, UInt16 lix)
{
   int r = lix >> 11;
   int g = (lix >> 5) & 0x3F;
   int b = lix & 0x1F;
   
   c.r = (r << 3) | ( r >> 2);
   c.g = (g << 2) | ( g >> 4);
   c.b = (b << 3) | ( b >> 2);
}

void TerrainRender::lightExpandHorz(float x, float ySquared, float dx, int j, int i, int shift)
{
   int size = 1 << shift;
   float scale = 1 / float(size);

   float z = trs.lmaBuffer[j][i].z;
   float dz = (trs.lmaBuffer[j][i+size].z - z) * scale;

   for(int count = 1; count < size; count++)
   {
      x += dx;
      z += dz;
      
      addColor(trs.lmaBuffer[j][i + count], x * x + z * z + ySquared);
   }
}

void TerrainRender::lightExpandVert(float y, float xSquared, float dy, int j, int i, int shift)
{
   int size = 1 << shift;
   float scale = 1 / float(size);

   float z = trs.lmaBuffer[j][i].z;
   float dz = (trs.lmaBuffer[j+size][i].z - z) * scale;

   for(int count = 1; count < size; count++)
   {
      y += dy;
      z += dz;
      
      trs.lmaBuffer[j+count][i].z = z;
      addColor(trs.lmaBuffer[j+count][i], y * y + z * z + xSquared);
   }
}

void TerrainRender::lmExpandHorz(int y, int x, int shift)
{
   int size = 1 << shift;
   Color d;
   Color c = trs.lmaBuffer[y][x];
   d.r = (trs.lmaBuffer[y][x+size].r - c.r) >> shift;
   d.g = (trs.lmaBuffer[y][x+size].g - c.g) >> shift;
   d.b = (trs.lmaBuffer[y][x+size].b - c.b) >> shift;
   
   for(int i = 1; i < size; i++)
   {
      c.r += d.r;
      c.g += d.g;
      c.b += d.b;
      
      trs.lmaBuffer[y][x+i] = c;
   }
}

void TerrainRender::lmExpandVert(int y, int x, int shift)
{
   int size = 1 << shift;
   Color d;
   Color c = trs.lmaBuffer[y][x];
   d.r = (trs.lmaBuffer[y+size][x].r - c.r) >> shift;
   d.g = (trs.lmaBuffer[y+size][x].g - c.g) >> shift;
   d.b = (trs.lmaBuffer[y+size][x].b - c.b) >> shift;
   
   for(int i = 1; i < size; i++)
   {
      c.r += d.r;
      c.g += d.g;
      c.b += d.b;
      
      trs.lmaBuffer[y+i][x] = c;
   }
}

void TerrainRender::expandDynamicLight(TerrLightInfo &tl, int x, int y, int level)
{
   int i, j;
   setupColorAdder(tl.radiusSquared, tl.r, tl.g, tl.b);
   
   if(DynamicLightMapShift - level <= 0)
   {
      int step = 1 << (level - DynamicLightMapShift);
      int count = DynamicLightMapSize;
      for(j = 0; j <= count; j++)
      {
         for(i = 0; i <= count; i++)
         {
            float xp = (x + i*step) * trs.squareSize - tl.pos.x;
            float yp = (y + j*step) * trs.squareSize - tl.pos.y;
            float zp = fixedToFloat(trs.currentBlock->getHeight((x + i) & TerrainBlock::BlockMask,
                                                                (y + j) & TerrainBlock::BlockMask)) - tl.pos.z;

            addColor(trs.lmaBuffer[j][i], xp * xp + yp * yp + zp * zp);
         }
      }
   }
   else
   {
      int delt = DynamicLightMapShift - level;
      int size = 1 << delt;
      int count = 1 << level;

      float delta = trs.squareSize / float(size);
      
      for(j = 0; j <= count; j++)
      {
         for(i = 0; i <= count; i++)
         {
            float xp = (x + i) * trs.squareSize - tl.pos.x;
            float yp = (y + j) * trs.squareSize - tl.pos.y;
            float zp = fixedToFloat(trs.currentBlock->getHeight((x + i) & TerrainBlock::BlockMask, (y + j) & TerrainBlock::BlockMask)) - tl.pos.z;
            trs.lmaBuffer[j << delt][i << delt].z = zp;

            addColor(trs.lmaBuffer[j << delt][i << delt], xp * xp + yp * yp + zp * zp);
            if(i != 0)
               lightExpandHorz(xp - trs.squareSize, yp * yp, delta, j << delt, (i - 1) << delt, delt);
            if(j != 0)
               lightExpandVert(yp - trs.squareSize, xp * xp, delta, (j-1) << delt, i << delt, delt);
            if(i != 0 && j != 0)
            {
               float xp = (x + i - 1) * trs.squareSize - tl.pos.x;
               for(int k = 1; k < size; k++)
               {
                  float yp = (y + j - 1) * trs.squareSize + k * delta - tl.pos.y;
                  lightExpandHorz(xp, yp * yp, delta,  ((j - 1) << delt) + k, (i-1) << delt, delt);
               }
            }
         }
      }
   }
}

void TerrainRender::expandLightMap(int x, int y, int level)
{
   int i, j;
   x &= TerrainBlock::BlockMask;
   y &= TerrainBlock::BlockMask;
   if(DynamicLightMapShift - level <= 0)
   {
      int step = (level - DynamicLightMapShift);
      int count = DynamicLightMapSize;
      int lightMask = trs.currentBlock->lightMap->getWidth() - 1;
      
      for(j = 0; j <= count; j++)
      {
         for(i = 0; i <= count; i++)
         {
            UInt16 lix = * ((UInt16 *) trs.currentBlock->lightMap->
               getAddress( (x + (i << step)) & lightMask,
                           (y + (j << step)) & lightMask ) );
            convertColor(trs.lmaBuffer[j][i], lix);
         }
      }
   }
   else
   {
      int delt = DynamicLightMapShift - level;
      int size = 1 << delt;
      int count = 1 << level;
      
      for(j = 0; j <= count; j++)
      {
         UInt16 *pixPtr = (UInt16 *) trs.currentBlock->lightMap->getAddress(x, y + j);
         for(i = 0; i <= count; i++)
         {
            convertColor(trs.lmaBuffer[j << delt][i << delt], *pixPtr++);
            if(i != 0)
               lmExpandHorz(j << delt, (i-1) << delt, delt);
            if(j != 0)
               lmExpandVert((j-1) << delt, i << delt, delt);
            if(i != 0 && j != 0)
            {
               for(int k = 1; k < size; k++)
                  lmExpandHorz(((j-1) << delt) + k, (i-1) << delt, delt);
            }
         }
      }
   }
}

void TerrainRender::collapseLightMap()
{
   int j, i;
   for(j = 0; j < 9; j++)
   {
      UInt16 *row = (UInt16 *) trs.currentBlock->dynamicLightMap->getAddress(0, j);
      Color *cwalk = trs.lmaBuffer[j];
      for(i = 0; i < 9; i++)
         *row++ = convertColorBack(*cwalk++);
   }
}

UInt32 TerrainRender::TestSquareLights(GridSquare *sq, int level, Point2I pos, UInt32 lightMask)
{
   UInt32 retMask = 0;
   float blockX = pos.x * trs.squareSize + trs.blockPos.x;
   float blockY = pos.y * trs.squareSize + trs.blockPos.y;
   float blockZ = fixedToFloat(sq->minHeight);

   float blockSize = trs.squareSize * (1 << level);
   float blockHeight = fixedToFloat(sq->maxHeight - sq->minHeight);

   Point3F vec;

   for(int i = 0; (lightMask >> i) != 0; i++)
   {
      
      if(lightMask & (1 << i))
      {
         Point3F *pos = &terrainLights[i].pos;
         // test the visibility of this light to box
         // find closest point on box to light and test
         
         if(pos->z < blockZ)
            vec.z = blockZ - pos->z;
         else if(pos->z > blockZ + blockHeight)
            vec.z = pos->z - (blockZ + blockHeight);
         else
            vec.z = 0;
         
         if(pos->x < blockX)
            vec.x = blockX - pos->x;
         else if(pos->x > blockX + blockSize)
            vec.x = pos->x - (blockX + blockSize);
         else
            vec.x = 0;

         if(pos->y < blockY)
            vec.y = blockY - pos->y;
         else if(pos->y > blockY + blockSize)
            vec.y = pos->y - (blockY + blockSize);
         else
            vec.y = 0;
         float dist = vec.len();
         if(dist < terrainLights[i].radius)
            retMask |= (1 << i);
      }
   }
   return retMask;
}

int TerrainRender::TestSquareVisibility(Point3F &min, Point3F &max, int mask, float expand)
{
   int retMask = 0;
   Point3F minPoint, maxPoint;
   for(int i = 0; i < NumClipPlanes; i++)
   {
      if(mask & (1 << i))
      {
         if(trs.clipPlane[i].x > 0)
         {
            maxPoint.x = max.x;
            minPoint.x = min.x;
         }
         else
         {
            maxPoint.x = min.x;
            minPoint.x = max.x;
         }
         if(trs.clipPlane[i].y > 0)
         {
            maxPoint.y = max.y;
            minPoint.y = min.y;
         }
         else
         {
            maxPoint.y = min.y;
            minPoint.y = max.y;
         }
         if(trs.clipPlane[i].z > 0)
         {
            maxPoint.z = max.z;
            minPoint.z = min.z;
         }
         else
         {
            maxPoint.z = min.z;
            minPoint.z = max.z;
         }
         float maxDot = m_dot(maxPoint, trs.clipPlane[i]);
         float minDot = m_dot(minPoint, trs.clipPlane[i]);
         float planeD = trs.clipPlane[i].d;
         if(maxDot <= -(planeD + expand))
            return -1;
         if(minDot <= -planeD)
            retMask |= (1 << i);
      }
   }
   return retMask;
}

inline float TerrainRender::getHaze(float dist)
{
   if(dist < trs.hazeDistance)
      return 0;
   else
   {
      float distFactor = (dist - trs.hazeDistance) * trs.hazeScale - 1.0;
      return 1.0 - distFactor * distFactor;
   }
}

void TerrainRender::transformPoints()
{
   trs.pointArray->reset();
   
   for(int i = 0; i < trs.pointCount;i++)
   {
      ScanPoint *p = trs.points + i;
      float avgHeight = (trs.points[p->p1].avgHeight + trs.points[p->p2].avgHeight) * .5;

      p->avgHeight = p->pointHeight + p->minGrowFactor * (avgHeight - p->pointHeight);

      trs.emitPoints[i].z = p->avgHeight;
      int index = trs.pointArray->addPoint(trs.emitPoints[i]);
	   TS::TransformedVertex& tv = trs.pointArray->getTransformedVertex(index);
      float dist = (trs.emitPoints[i] - trs.camPos).len();
      tv.fDist = dist;
      if(dist > trs.farDistance)
         tv.fStatus |= TS::ClipFarSphere;
      tv.fColor.haze = getHaze(dist);
   }
}


void TerrainRender::transformWaterPoints()
{
   trs.pointArray->reset();
   
   for(int i = 0; i < trs.pointCount;i++)
   {
      ScanPoint *p = trs.points + i;
      float avgHeight = (trs.points[p->p1].avgHeight + trs.points[p->p2].avgHeight) * 0.5;
      float avgX = (trs.envTexCoords[p->p1].x + trs.envTexCoords[p->p2].x) * 0.5;
      float avgY = (trs.envTexCoords[p->p1].y + trs.envTexCoords[p->p2].y) * 0.5;

      p->avgHeight = p->pointHeight + p->minGrowFactor * (avgHeight - p->pointHeight);
      trs.envTexCoords[i].x = trs.envTexCoords[i].x + p->minGrowFactor * (avgX - trs.envTexCoords[i].x);
      trs.envTexCoords[i].y = trs.envTexCoords[i].y + p->minGrowFactor * (avgY - trs.envTexCoords[i].y);
 
      trs.emitPoints[i].z = p->avgHeight;
      int index = trs.pointArray->addPoint(trs.emitPoints[i]);
	   TS::TransformedVertex& tv = trs.pointArray->getTransformedVertex(index);
      float dist = (trs.emitPoints[i] - trs.camPos).len();
      tv.fDist = dist;
      if(dist > trs.farDistance)
         tv.fStatus |= TS::ClipFarSphere;
      tv.fColor.haze = getHaze(dist);
   }
}

int TerrainRender::allocInitialPoint(Point3F pos)
{
   int ret = trs.pointCount++;
   trs.points[ret].minGrowFactor = 0;
   trs.points[ret].pointHeight = pos.z;
   trs.points[ret].avgHeight = pos.z;
   trs.emitPoints[ret].x = pos.x;
   trs.emitPoints[ret].y = pos.y;
   trs.points[ret].p1 = 0;
   trs.points[ret].p2 = 0;
   trs.waterTexCoords[ret].set(pos.x * trs.texScale.x, pos.y * trs.texScale.y);
   return ret;
}

int TerrainRender::allocWaterPoint(Point2F pos, int p1, int p2)
{
   int ret = trs.pointCount++;
   trs.points[ret].minGrowFactor = trs.currentGrowFactor;
   trs.points[ret].pointHeight = getWaterHeight(pos, ret);
   trs.points[ret].avgHeight = 0;
   trs.emitPoints[ret].x = pos.x;
   trs.emitPoints[ret].y = pos.y;
   trs.points[ret].p1 = p1;
   trs.points[ret].p2 = p2;
   trs.waterTexCoords[ret].set(pos.x * trs.texScale.x, pos.y * trs.texScale.y);
   return ret;
}

int TerrainRender::allocPoint(Point2I pos, int p1, int p2)
{
   int ret = trs.pointCount++;
   trs.points[ret].minGrowFactor = trs.currentGrowFactor;
   trs.points[ret].pointHeight = fixedToFloat(*(trs.currentBlock->heightMap + pos.x + pos.y + (pos.y << TerrainBlock::BlockShift)));
   trs.points[ret].avgHeight = 0;
   trs.emitPoints[ret].x = (trs.blockOffset.x + pos.x) * trs.squareScale.x;
   trs.emitPoints[ret].y = (trs.blockOffset.y + pos.y) * trs.squareScale.y;
   trs.points[ret].p1 = p1;
   trs.points[ret].p2 = p2;
   return ret;
}

int TerrainRender::allocEdges(int count)
{
   int ret = trs.edgeCount;
   trs.edgeCount += count;

   for(int i = ret; i < ret + count; i++)
      trs.edges[i].mp = InvalidPointIndex;
   return ret;
}

void TerrainRender::clampEdge(int edge)
{
   static int edgeStack[EdgeStackSize];
   int stackSize = 0;
   for(;;)
   {
      int mp = trs.edges[edge].mp;
      if(mp < InvalidPointIndex)
      {
         trs.points[mp].minGrowFactor = 1;
         edgeStack[stackSize++] = trs.edges[edge].firstSubEdge + 1;
         edge = trs.edges[edge].firstSubEdge;
         continue;
      }
      else
         trs.edges[edge].mp = ClampEdgeIndex;
      if(!stackSize--)
         break;
      edge = edgeStack[stackSize];
   }
}

void TerrainRender::subdivideEdge(int edge, Point2I pos)
{
   int mp = trs.edges[edge].mp;
   if(mp >= InvalidPointIndex)
   {
      int p1 = trs.edges[edge].p1;
      int p2 = trs.edges[edge].p2;
      int midPoint = allocPoint(pos, p1, p2);
      int fse = allocEdges(2);
      
      if(mp == ClampEdgeIndex)
      {
         trs.points[midPoint].minGrowFactor = 1;
         trs.edges[fse].mp = ClampEdgeIndex;
         trs.edges[fse+1].mp = ClampEdgeIndex;
      }
      trs.edges[edge].mp = midPoint;
      trs.edges[edge].firstSubEdge = fse;
      trs.edges[fse].p1 = trs.edges[edge].p1;
      trs.edges[fse].p2 = midPoint;
      trs.edges[fse+1].p1 = midPoint;
      trs.edges[fse+1].p2 = trs.edges[edge].p2;
   }
   else
   {
      trs.points[mp].minGrowFactor = max(trs.points[mp].minGrowFactor, trs.currentGrowFactor);
   }
}

void TerrainRender::subdivideWaterEdge(int edge, Point2F pos)
{
   int mp = trs.edges[edge].mp;
   if(mp >= InvalidPointIndex)
   {
      int p1 = trs.edges[edge].p1;
      int p2 = trs.edges[edge].p2;
      int midPoint = allocWaterPoint(pos, p1, p2);
      int fse = allocEdges(2);
      
      if(mp == ClampEdgeIndex)
      {
         trs.points[midPoint].minGrowFactor = 1;
         trs.edges[fse].mp = ClampEdgeIndex;
         trs.edges[fse+1].mp = ClampEdgeIndex;
      }
      trs.edges[edge].mp = midPoint;
      trs.edges[edge].firstSubEdge = fse;
      trs.edges[fse].p1 = trs.edges[edge].p1;
      trs.edges[fse].p2 = midPoint;
      trs.edges[fse+1].p1 = midPoint;
      trs.edges[fse+1].p2 = trs.edges[edge].p2;
   }
   else
   {
      trs.points[mp].minGrowFactor = max(trs.points[mp].minGrowFactor, trs.currentGrowFactor);
   }
}

float TerrainRender::getSquareDistance(Point3F &minPoint, Point3F &maxPoint)
{
   Point3F vec;
   if(trs.camPos.z < minPoint.z)
      vec.z = minPoint.z - trs.camPos.z;
   else if(trs.camPos.z > maxPoint.z)
      vec.z = trs.camPos.z - maxPoint.z;
   else
      vec.z = 0;

   if(trs.camPos.x < minPoint.x)
      vec.x = minPoint.x - trs.camPos.x;
   else if(trs.camPos.x > maxPoint.x)
      vec.x = trs.camPos.x - maxPoint.x;
   else
      vec.x = 0;

   if(trs.camPos.y < minPoint.y)
      vec.y = minPoint.y - trs.camPos.y;
   else if(trs.camPos.y > maxPoint.y)
      vec.y = trs.camPos.y - maxPoint.y;
   else
      vec.y = 0;

   return vec.len();
}

void TerrainRender::emitWaterSquare(WaterStackNode *n)
{
   if(trs.squareCount >= MaxEmitSquares)
      return;
   int square = trs.squareCount++;
   trs.squares[square].edge[0] = n->top;
   trs.squares[square].edge[1] = n->right;
   trs.squares[square].edge[2] = n->bottom;
   trs.squares[square].edge[3] = n->left;
}

void TerrainRender::emitSquare(SquareStackNode *n, int flags, int mipLevel, UInt32 lightMask)
{
   if(trs.squareCount >= MaxEmitSquares)
      return;
   int square = trs.squareCount++;
   if(lightMask)
      flags |= EmitSquare::DynamicallyLit;

   trs.squares[square].edge[0] = n->top;
   trs.squares[square].edge[1] = n->right;
   trs.squares[square].edge[2] = n->bottom;
   trs.squares[square].edge[3] = n->left;
   trs.squares[square].level = n->level;
   trs.squares[square].clipFlags = n->clipFlags;
   trs.squares[square].x = n->pos.x + trs.blockOffset.x;
   trs.squares[square].y = n->pos.y + trs.blockOffset.y;
   trs.squares[square].flags = flags;
   trs.squares[square].mipLevel = mipLevel;
   trs.squares[square].lightMask = lightMask;
}

void TerrainRender::emitTriFan(int *pointList, int *lightList, int vertexCount)
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
      trs.pointArray->drawPoly(3, vip, 0);
   }
}

void TerrainRender::emitTriFanReverse(int *pointList, int *lightList, int vertexCount)
{
   TS::VertexIndexPair vip[3];

   vip[0].fVertexIndex = pointList[0];
   vip[0].fTextureIndex = lightList[0];

   for(int i = 2; i < vertexCount; i++)
   {
      vip[2].fVertexIndex = pointList[i-1];
      vip[2].fTextureIndex = lightList[i-1];
      vip[1].fVertexIndex = pointList[i];
      vip[1].fTextureIndex = lightList[i];
      trs.pointArray->drawPoly(3, vip, 0);
   }
}

////	{}, // Rotate
////	{}, // FlipX
////	{}, // Rotate | FlipX
////	{}, // FlipY
////	{}, // FlipY | Rotate
////	{}, // FlipY | FlipX
////	{}, // FlipY | Rotate | FlipX

extern DWORD gridCreateMipMap( int _mipLevel, Box2I& _area,
	 Point2I& _matMapSize, GridBlock::Material* _matMap,
	 TSMaterialList* _matList, int stride, BYTE *baseAddr, GFXBitmap* _defTex );

extern bool gPointArrayHack;
extern Point2F *gPointArrayHackCoords; // lighting map coordinates

// Backface cull all the squares:

void TerrainRender::cull()
{
   for(int i = 0; i < trs.squareCount;)
   {
      EmitSquare *sq = &trs.squares[i];

      int topEdge = sq->edge[0];
      int bottomEdge = sq->edge[2];
      
      // do backfacing test...

      float x = sq->x;
      float y = sq->y;
      float zTopLeft = trs.points[trs.edges[topEdge].p1].avgHeight;
      float zTopRight = trs.points[trs.edges[topEdge].p2].avgHeight;
      float zBottomLeft = trs.points[trs.edges[bottomEdge].p1].avgHeight;
      float zBottomRight = trs.points[trs.edges[bottomEdge].p2].avgHeight;
      
      Point3F normal1, normal2;
      Point3F squarePos, cp;

      float squareCSize = 1 << sq->level;
      if(sq->flags & GridSquare::Split45)
      {
         squarePos.set(x * trs.squareSize, y * trs.squareSize, zBottomLeft);
         normal1.set(zBottomLeft - zBottomRight, zBottomRight - zTopRight, trs.squareSize << sq->level);
         normal2.set(zTopLeft - zTopRight, zBottomLeft - zTopLeft, trs.squareSize << sq->level);
      }
      else
      {
         squarePos.set((x + squareCSize) * trs.squareSize, y * trs.squareSize, zBottomRight);
         normal1.set(zTopLeft - zTopRight, zBottomRight - zTopRight, trs.squareSize << sq->level);
         normal2.set(zBottomLeft - zBottomRight, zBottomLeft - zTopLeft, trs.squareSize << sq->level);
      }
      cp = trs.camPos - squarePos;
      
      if(m_dot(cp, normal1) < 0 && m_dot(cp, normal2) < 0)
      {
         trs.squareCount--;
         trs.squares[i] = trs.squares[trs.squareCount];
      }
      else
         i++;
   }
}

int TerrainRender::compareTexture(EmitSquare *sq, CacheBitmap *ci)
{
   if(sq->mipLevel != ci->mipLevel)
      return ci->mipLevel - int(sq->mipLevel);
   if(sq->level != ci->level)
      return int(sq->level) - ci->level;
   if(sq->x != ci->x)
      return int(sq->x) - ci->x;
   if(sq->y != ci->y)
      return int(sq->y) - ci->y;
   return 0;
}

// return negative if a < b in array order

static int _USERENTRY sortCompare(const void *va, const void *vb)
{
   const EmitSquare *a = (const EmitSquare *) va;
   const EmitSquare *b = (const EmitSquare *) vb;
   
   // we want mip 0s first.
   if(a->level == 0 && b->level != 0)
      return -1;
   if(a->level != 0 && b->level == 0)
      return 1;
      
   if(a->level == 0 && b->level == 0) // they're level 0 squares:
   {
      // material index is stored in mipLevel
      if(a->mipLevel != b->mipLevel)
         return int(a->mipLevel) - int(b->mipLevel);
      else
         return int(a->flags >> EmitSquare::DynamicLightFlagShift)
                 - int(b->flags >> EmitSquare::DynamicLightFlagShift);
   }

   // mipLevel sorts out the 255 mipLevel squares
   // sort in descending order on mipLevels.
   if(a->mipLevel != b->mipLevel)
      return int(b->mipLevel) - int(a->mipLevel);

   if(a->mipLevel == 255)
      return int(a->flags >> EmitSquare::DynamicLightFlagShift)
                 - int(b->flags >> EmitSquare::DynamicLightFlagShift);

   if(a->level != b->level)
      return int(a->level) - int(b->level);
   // hackytown... sort by x, y
   return *((int *) &a->x) - *((int *) &b->x);
}

void TerrainRender::sort()
{
   m_qsort(trs.squares, trs.squareCount, sizeof(EmitSquare), sortCompare);
}

void TerrainRender::prepareLighting(EmitSquare *sq, float &scale, Point2F &offset)
{
   float squareCSize = 1 << sq->level;

   if(sq->flags & EmitSquare::DynamicallyLit)
   {
      // draw the light maps:
      int lightIndex = 0;
      UInt32 mask = sq->lightMask;
      
      // build the base light map:
      
      expandLightMap(sq->x, sq->y, sq->level);

      for(;mask;mask >>= 1, lightIndex++)
      {
         if(!mask & 1)
            continue;
         // compute the lightTextures...
         expandDynamicLight(terrainLights[lightIndex], sq->x, sq->y, sq->level);
      }
      
      scale = 0.5 / squareCSize;
      offset.set(0.0625 * squareCSize, 0.0625 * squareCSize);
      
      collapseLightMap();
      Glide::hackDownloadLightmap(trs.currentBlock->dynamicLightMap);
   }
   else
   {
      scale = 1 / 256.0f;
      offset.set((sq->x & TerrainBlock::BlockMask) + 0.5,
                 (sq->y & TerrainBlock::BlockMask) + 0.5);
      Glide::hackSetLightmap(trs.currentBlock->lightMap);
   }
}

int TerrainRender::renderLevelZero()
{
   // set up sentinel end
   trs.squares[trs.squareCount].level = 1;
   int squareIndex;
   int lightBuffer[5];
   int pointBuffer[5];
   int prevIndex = -1;
      
   for(squareIndex = 0; trs.squares[squareIndex].level == 0; squareIndex++)
   {
      float lightScale;
      Point2F lightOffset;

      EmitSquare *sq = trs.squares + squareIndex;
      int topEdge = sq->edge[0];
      int bottomEdge = sq->edge[2];
      
      int topLeft = trs.edges[topEdge].p1;
      int topRight = trs.edges[topEdge].p2;
      int bottomLeft = trs.edges[bottomEdge].p1;
      int bottomRight = trs.edges[bottomEdge].p2;
      
      prepareLighting(sq, lightScale, lightOffset);
      int split45;
      
      if(sq->flags & EmitSquare::Split45)
      {
         split45 = 1;
         pointBuffer[0] = topRight;
         pointBuffer[1] = bottomRight;
         pointBuffer[2] = bottomLeft;
         pointBuffer[3] = topLeft;
      }
      else
      {
         split45 = 0;
         pointBuffer[0] = topLeft;
         pointBuffer[1] = topRight;
         pointBuffer[2] = bottomRight;
         pointBuffer[3] = bottomLeft;
      }
      int sqX = sq->x & TerrainBlock::BlockMask;
      int sqY = sq->y & TerrainBlock::BlockMask;
      
      GridBlock::Material *material = trs.currentBlock->getMaterial(sqX, sqY);
      GFXBitmap *textureMap = (GFXBitmap *)trs.currentBlock->materialList->getMaterial(material->index).getTextureMap();
      
      if(material->index != prevIndex)
      {
         staticTextureCount++;
         prevIndex = material->index;
         staticTSU += textureMap->imageSize;
      }
      
      if(!Glide::hackTextureValid(&(textureMap->getCacheInfo()), textureMap->paletteIndex, trs.gfxSurface->getPalette()))
         Glide::hackDownloadBitmap(textureMap, trs.gfxSurface->getPalette(), &(textureMap->getCacheInfo()));
      
      trs.pointArray->useTextures(trs.TextureCoor[material->flags & GridBlock::Material::RotateMask] + split45);
      for(int i = 0; i < 4; i++)
      {
         int idx = (i + split45) & 0x03;
         lightBuffer[i] = i;
         trs.lightCoords[i].set((lightOffset.x + trs.LightTextureCoor[idx].x) * lightScale,
                            (lightOffset.y + trs.LightTextureCoor[idx].y) * lightScale);
      }
      lightBuffer[4] = lightBuffer[0];
      emitTriFan(pointBuffer, lightBuffer, 4);  //(1, 2, 3) (1, 3, 4)
   }
   return squareIndex;
}

void TerrainRender::renderEdgedSquare(EmitSquare *sq)
{
   static int pointBuffer[TerrainBlock::SquareMaxPoints + 2];
   static int lightBuffer[TerrainBlock::SquareMaxPoints + 2];

   static int edgeStack[TerrainBlock::BlockShift + 1];
   static int levelStack[TerrainBlock::BlockShift + 1];

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
   float lightScale;
   Point2F lightOffset;
   float squareCSize = 1 << sq->level;
   
   prepareLighting(sq, lightScale, lightOffset);
   float textureScale = 1 / squareCSize;

   int curPoint = 0;
   for(int squareEdge = 0; squareEdge < 4; squareEdge++)
   {
      // edge+1 if split45, else edge
      int edgeIndex;
      if(sq->flags & GridSquare::Split45)
         edgeIndex = (squareEdge + 1) & 0x3;
      else
         edgeIndex = squareEdge;
         
      // fill the point buffer with the recursed edges
      int stackSize = 0;
      int level = sq->level;
      int x = edgeStarts[edgeIndex][0] << level;
      int y = edgeStarts[edgeIndex][1] << level;
   
      int curEdge = sq->edge[edgeIndex];

      for(;;) {
         if(trs.edges[curEdge].mp < InvalidPointIndex)
         {
            // push the tail of the edge onto the stack
            // tail is firstSubEdge + 1 if top, right
            // or firstSubEdge if bottom, left
            level--;
            levelStack[stackSize] = level;
            if(edgeIndex < 2)
            {
               curEdge = trs.edges[curEdge].firstSubEdge;
               edgeStack[stackSize++] = curEdge + 1;
            }
            else
            {
               curEdge = trs.edges[curEdge].firstSubEdge + 1;
               edgeStack[stackSize++] = curEdge - 1;
            }
         }
         else
         {
            // top and right edges go p1->p2
            // bottom and left edges go p2->p1
            trs.lightCoords[curPoint].set((lightOffset.x + x) * lightScale, (lightOffset.y + y) * lightScale);
            trs.textureCoords[curPoint].set(x * textureScale, (squareCSize - y) * textureScale);
            pointBuffer[curPoint++] = 
               edgeIndex < 2 ? trs.edges[curEdge].p1 : trs.edges[curEdge].p2;
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
   // early out for strict square:
   for(int i = 0; i < curPoint; i++)
      lightBuffer[i] = i;
   // indices are in pointBuffer
   emitTriFan(pointBuffer, lightBuffer, curPoint);  //(1, 2, 3) (1, 3, 4)
}

int TerrainRender::renderFullMip(int squareIndex)
{
   GFXBitmap *textureMap = (GFXBitmap *)trs.currentBlock->baseTextureMap;
   if(!Glide::hackTextureValid(&(textureMap->getCacheInfo()), textureMap->paletteIndex, trs.gfxSurface->getPalette()))
      Glide::hackDownloadBitmap(textureMap, trs.gfxSurface->getPalette(), &(textureMap->getCacheInfo()));
   trs.pointArray->useTextures(trs.lightCoords);

   // set up sentinel end
   trs.squares[trs.squareCount].mipLevel = 0;
   
   for(;(trs.squares + squareIndex)->mipLevel == 255; squareIndex++)
      renderEdgedSquare(trs.squares + squareIndex);
   
   return squareIndex;
}

int TerrainRender::renderLevelNonzero(int squareIndex)
{
   CacheBitmap **walk = &trs.activeTextureList;
   
   trs.pointArray->useTextures(trs.textureCoords);
   dynamicTextureCount = trs.squareCount - squareIndex;
   
   for(;squareIndex < trs.squareCount;squareIndex++)
   {
      EmitSquare *sq = trs.squares + squareIndex;
      CacheBitmap *curTexture = *walk;
      if(sq->flags & EmitSquare::Recache)
      {
         CacheBitmap *newBitmap = allocTexture(sq->mipLevel);
         newBitmap->next = curTexture;
         *walk = newBitmap;
         curTexture = newBitmap;

         newBitmap->x = sq->x;
         newBitmap->y = sq->y;
         newBitmap->level = sq->level;

         int sqX = sq->x & TerrainBlock::BlockMask;
         int sqY = sq->y & TerrainBlock::BlockMask;

         GFXBitmap *bmp = newBitmap->bitmap;
      
         Point2I mapSize(TerrainBlock::BlockSize - 1, TerrainBlock::BlockSize - 1);
         Box2I mapArea;
         mapArea.fMin.x = sqX;
         mapArea.fMin.y = sqY;
         mapArea.fMax.x = sqX + (1 << sq->level) - 1;
         mapArea.fMax.y = sqY + (1 << sq->level) - 1;

         gridCreateMipMap(7 - (sq->mipLevel - sq->level), mapArea, mapSize, trs.currentBlock->materialMap, trs.currentBlock->materialList, bmp->stride, bmp->pMipBits[0], 0);
         
         for(int mip = 1; mip < bmp->detailLevels; mip++)
         {
            int size = 1 << (sq->mipLevel - mip);
            BYTE *dst = bmp->pMipBits[mip];
            BYTE *src = bmp->pMipBits[mip-1];
            for(int row = 0; row < size; row++)
            {
               for(int col = 0; col < size; col++)
               {
                  *dst++ = *src;
                  src += 2;
               }
               src += size << 1;
            }
         }
         bmp->paletteIndex = trs.currentBlock->paletteIndex;
         Glide::hackReloadBitmap(bmp, trs.gfxSurface->getPalette());
      }      

      GFXBitmap *bmp = curTexture->bitmap;
      textureSpaceUsed += bmp->imageSize;
      
      if(!Glide::hackTextureValid(&(bmp->getCacheInfo()), bmp->paletteIndex, trs.gfxSurface->getPalette()))
         Glide::hackDownloadBitmap(bmp, trs.gfxSurface->getPalette(), &(bmp->getCacheInfo()));
   
      renderEdgedSquare(sq);
      walk = &(curTexture->next);
   }
   return squareIndex;
}

void TerrainRender::flushCache()
{
   for(int i = 0; i <= MaxMipLevel; i++)
   {
      CacheBitmap *walk = trs.freeList[i];
      while(walk)
      {
         CacheBitmap *temp = walk->next;
         delete walk->bitmap;
         delete walk;
         walk = temp;
      }
   }
   CacheBitmap *walk = trs.activeTextureList;
   while(walk)
   {
      CacheBitmap *temp = walk->next;
      delete walk->bitmap;
      delete walk;
      walk = temp;
   }
}

CacheBitmap *TerrainRender::allocTexture(int mipLevel)
{
   if(trs.freeList[mipLevel])
   {
      CacheBitmap *ret = trs.freeList[mipLevel];
      trs.freeList[mipLevel] = ret->next;
      return ret;
   }
   CacheBitmap *ret = new CacheBitmap;
   int size = 1 << mipLevel;
   ret->mipLevel = mipLevel;
   
   GFXBitmap *bmp = new GFXBitmap;
   bmp->width = size;
   bmp->height = size;
   bmp->stride = size;
   bmp->bitDepth = 8;
   int tmp = size;
   bmp->imageSize = 0;
   int detailLevels;
   
   for(detailLevels = 0;tmp;tmp >>= 1, detailLevels++)
      bmp->imageSize += tmp * tmp;
   
   bmp->pBitsBase = new BYTE[bmp->imageSize + 2 * bmp->stride];
   bmp->pBits = bmp->pBitsBase + bmp->stride;
   bmp->attribute |= BMA_OWN_MEM;
   
   bmp->detailLevels = detailLevels;
   
   bmp->pMipBits[0] = bmp->pBits;
   for(int i = 1; i < detailLevels; i++)
   {
      int sz = size >> (i-1);
      bmp->pMipBits[i] = bmp->pMipBits[i-1] + sz * sz;
   }
   ret->bitmap = bmp;
   
   return ret;
}

void TerrainRender::freeTexture(CacheBitmap *cb)
{
   cb->next = trs.freeList[cb->mipLevel];
   trs.freeList[cb->mipLevel] = cb;
}

void TerrainRender::texture(int squareIndex)
{
   CacheBitmap **cbptr = &trs.activeTextureList;
   
   for(;squareIndex < trs.squareCount;)
   {
      EmitSquare *sq = trs.squares + squareIndex;
      CacheBitmap *curTex = *cbptr;
      
      int cmpVal;
      if(!curTex)
         cmpVal = -1;
      else
         cmpVal = compareTexture(sq, curTex);

      if(cmpVal < 0)
      {
         // flag the square as recache
         // we need to gen a new texture
         sq->flags |= EmitSquare::Recache;
         squareIndex++;
      }
      else if(cmpVal > 0)
      {
         *cbptr = curTex->next;
         freeTexture(curTex);
         unusedTextureCount++;
      }
      else
      {
         cbptr = &(curTex->next);
         squareIndex++;
      }
   }
   
   CacheBitmap *walk = *cbptr;
   *cbptr = NULL;
   
   // free all the textures at the end:
   while(walk)
   {
      CacheBitmap *temp = walk->next;
      freeTexture(walk);
      walk = temp;
   }
}

void TerrainRender::processCurrentBlock(int topEdge, int rightEdge, int bottomEdge, int leftEdge)
{
   SquareStackNode stack[TerrainBlock::BlockShift*4];
   Point3F minPoint, maxPoint;

   stack[0].level = TerrainBlock::BlockShift;
   stack[0].clipFlags = 0xFF;  // test all the planes
   stack[0].pos.set(0,0);
   stack[0].top = topEdge;
   stack[0].right = rightEdge;
   stack[0].bottom = bottomEdge;
   stack[0].left = leftEdge;
   stack[0].lightMask = (1 << trs.dynamicLightCount) - 1; // test all the lights
   
   int curStackSize = 1;
   float squareDistance;

   float ps = trs.screenSize / trs.camera->getPixelScale();
   float distanceScale = trs.camera->projectionDistance(ps, 1);
   
   while(curStackSize)
   {
      SquareStackNode *n = stack + curStackSize - 1;
      // see if it's visible
      int clipFlags = n->clipFlags;
      Point2I pos = n->pos;
      GridSquare *sq = trs.currentBlock->findSquare(n->level, pos);

         
      minPoint.set(trs.squareScale.x * pos.x + trs.blockPos.x,
                   trs.squareScale.y * pos.y + trs.blockPos.y,
                   fixedToFloat(sq->minHeight));
      maxPoint.set(minPoint.x + (trs.squareSize << n->level),
                   minPoint.y + (trs.squareSize << n->level),
                   fixedToFloat(sq->maxHeight));

      squareDistance = getSquareDistance(minPoint, maxPoint);

      if(squareDistance >= trs.farDistance || (sq->flags & GridSquare::Empty))
      {
         curStackSize--;
         continue;
      }
      if(clipFlags)
      {
         // zDelta for screen error height deviance.
         float zDelta = squareDistance * trs.pixelError;
         minPoint.z -= zDelta;
         maxPoint.z += zDelta;

         clipFlags = TestSquareVisibility(minPoint, maxPoint, clipFlags, trs.squareSize);
         if(clipFlags == -1)
         {
            // trivially rejected, so pop it off the stack
            curStackSize--;
            continue;
         }
      }
      UInt32 lightMask = n->lightMask;
      if(lightMask)
         lightMask = TestSquareLights(sq, n->level, n->pos, lightMask);
      
      // bottom level square or not subdivided means emit
      // the square.
      
      if(n->level == 0)
      {
         // store the material index in mipLevel for quicker sorting
         GridBlock::Material *material = trs.currentBlock->getMaterial(n->pos.x, n->pos.y);
         emitSquare(n, sq->flags, material->index, lightMask);
         curStackSize--;
         continue;
      }

      trs.currentGrowFactor = 0;
   
      bool subdivideSquare = false;
   
      if(squareDistance < 1)
         subdivideSquare = true;
      else
      {
         float squareSize = trs.squareSize << n->level;
         float detailDistance = (distanceScale * squareSize) - squareSize;

         if(squareDistance < detailDistance)
         {
            subdivideSquare = true;
            float clampDistance = 0.75 * detailDistance;
            
            if(squareDistance > clampDistance)
               trs.currentGrowFactor = (squareDistance - clampDistance) / (0.25 * detailDistance);
         }
         
//         float subdivideDistance = fixedToFloat(sq->heightDeviance) / trs.pixelError;
//         if(squareDistance < subdivideDistance)
//         {
//            subdivideSquare = true;
//            
//            float clampDistance = subdivideDistance * 0.8;
//            if(squareDistance > clampDistance)
//               trs.currentGrowFactor = (squareDistance - clampDistance) / (0.2 * subdivideDistance);
//         }
//      
//      

      }
      if(!subdivideSquare || trs.edgeCount > ScanEdgeLimit || trs.pointCount > ScanPointLimit)
      {
         clampEdge(n->top);
         clampEdge(n->right);
         clampEdge(n->bottom);
         clampEdge(n->left);

         int mipLevel;         
         if(squareDistance > 0.001)
         {
            int squareSz = trs.squareSize << n->level;
            int size = int(trs.camera->projectRadius(squareDistance + (squareSz >> 1), squareSz));
            mipLevel = getPower(size) - 1;
         }
         if(mipLevel > 6)
            mipLevel = 6;
         if(mipLevel <= n->level + 1)
            mipLevel = 0xFF;
         emitSquare(n, sq->flags, mipLevel, lightMask);
         curStackSize--; // we're good
         continue;
      }

      int p1, p2;
      int top = n->top;
      int right = n->right;
      int bottom = n->bottom;
      int left = n->left;

      // subdivide this square and throw it on the stack
      int squareOneSize = 1 << n->level;
      int squareHalfSize = squareOneSize >> 1;

      if(sq->flags & GridSquare::Split45)
      {
         p1 = trs.edges[top].p2;
         p2 = trs.edges[bottom].p1;
      }
      else
      {
         p1 = trs.edges[top].p1;
         p2 = trs.edges[bottom].p2;
      }
      int midPoint = allocPoint(Point2I(pos.x + squareHalfSize, pos.y + squareHalfSize), p1, p2);

      int nextLevel = n->level - 1;

      subdivideEdge(top, Point2I(pos.x + squareHalfSize, pos.y + squareOneSize));
      subdivideEdge(right, Point2I(pos.x + squareOneSize, pos.y + squareHalfSize));
      subdivideEdge(bottom, Point2I(pos.x + squareHalfSize, pos.y));
      subdivideEdge(left, Point2I(pos.x, pos.y + squareHalfSize));
   
      // cross edges go top, right, bottom, left
      int firstCrossEdge = allocEdges(4);
      
      trs.edges[firstCrossEdge].p1 = trs.edges[top].mp;
      trs.edges[firstCrossEdge].p2 = midPoint;
      trs.edges[firstCrossEdge+1].p1 = midPoint;
      trs.edges[firstCrossEdge+1].p2 = trs.edges[right].mp;
      trs.edges[firstCrossEdge+2].p1 = midPoint;
      trs.edges[firstCrossEdge+2].p2 = trs.edges[bottom].mp;
      trs.edges[firstCrossEdge+3].p1 = trs.edges[left].mp;
      trs.edges[firstCrossEdge+3].p2 = midPoint;
      
      for(int i = 0; i < 4; i++)
      {
         n[i].level = nextLevel;
         n[i].clipFlags = clipFlags;
         n[i].lightMask = lightMask;
      }

      n[0].pos = pos;
      n[0].top = firstCrossEdge + 3;
      n[0].right = firstCrossEdge + 2;
      n[0].bottom = trs.edges[bottom].firstSubEdge;
      n[0].left = trs.edges[left].firstSubEdge + 1;

      n[1].pos.set(pos.x + squareHalfSize, pos.y);
      n[1].top = firstCrossEdge + 1;
      n[1].right = trs.edges[right].firstSubEdge + 1;
      n[1].bottom = trs.edges[bottom].firstSubEdge + 1;
      n[1].left = firstCrossEdge + 2;

      n[2].pos.set(pos.x + squareHalfSize, pos.y + squareHalfSize);
      n[2].top = trs.edges[top].firstSubEdge + 1;
      n[2].right = trs.edges[right].firstSubEdge;
      n[2].bottom = firstCrossEdge + 1;
      n[2].left = firstCrossEdge;

      n[3].pos.set(pos.x, pos.y + squareHalfSize);
      n[3].top = trs.edges[top].firstSubEdge;
      n[3].right = firstCrossEdge;
      n[3].bottom = firstCrossEdge + 3;
      n[3].left = trs.edges[left].firstSubEdge;
      curStackSize += 3;
   }
}

void TerrainRender::buildLightArray()
{
   const TMat3F & mat = trs.camera->getTOW();
   TSSceneLighting * sceneLights = trs.renderContext->getLights();
   SphereF  instSphere ( mat.p, 1000000.0f );
   sceneLights->prepare ( instSphere, mat );

   int lightCount = 0;
      
   TSSceneLighting::iterator ptr;
   for ( ptr = sceneLights->begin(); ptr != sceneLights->end() && lightCount < MaxTerrainLights ; ptr++ )
   {
      TSLight *tsl = *ptr;
      if(tsl->fLight.fType == TS::Light::LightPoint && !tsl->isStaticLight())
      {
         terrainLights[lightCount].pos = tsl->fLight.fPosition;
         Point3F dVec = trs.camPos - terrainLights[lightCount].pos;

         terrainLights[lightCount].distSquared = m_dot(dVec, dVec);
         terrainLights[lightCount].radius = tsl->fLight.fRange;
         terrainLights[lightCount].radiusSquared = tsl->fLight.fRange * tsl->fLight.fRange;
         terrainLights[lightCount].r = tsl->fLight.fRed;
         terrainLights[lightCount].g = tsl->fLight.fGreen;
         terrainLights[lightCount].b = tsl->fLight.fBlue;
         lightCount++;
      }
   }
   trs.dynamicLightCount = min(lightCount, (int)MaxVisibleLights);
}

void TerrainRender::buildDetailTable()
{
	float ps = trs.screenSize / trs.camera->getPixelScale();
   for(int i = MaxDetailLevel; i >= 0; i--)
   {
		Detail& d = trs.detailData[i];
		float size = float(1 << i) * trs.squareSize;
		d.maxDistance = trs.camera->projectionDistance(ps,size);

		if (i < MaxDetailLevel - 1) {
			Detail& pd = trs.detailData[i+1];
         pd.minDistance = (d.maxDistance + pd.maxDistance) * 0.5;
		}

   }
   trs.detailData[0].minDistance = trs.detailData[0].maxDistance * 0.5;

   for(int i = MaxDetailLevel; i >= 0; i--)
   {
		Detail& d = trs.detailData[i];
		float delta = d.maxDistance - d.minDistance;
		d.distanceScale = (delta > .0f)? 1.0 / delta: 1.0f;
		d.level = i;
   }
}

void TerrainRender::buildClippingPlanes()
{
   // set up clipping planes:
   
	TMat3F camTCW = trs.camera->getTOC();
	camTCW.inverse();

	const RectF& vp = trs.camera->getWorldViewport();
	float cx = (vp.lowerR.x - vp.upperL.x) / 2.0f;
	float cy = (vp.upperL.y - vp.lowerR.y) / 2.0f;
	float nearDistance;
   
   nearDistance = trs.camera->getNearDist();

   Point3F origin(0,0,0);
   Point3F farOrigin(0,trs.farDistance,0);
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
   trs.clipPlane[0].set(xlUpperRight, xlUpperLeft, xlLowerRight); 

   // far plane is revers of near plane vector
   trs.clipPlane[1].x = -trs.clipPlane[0].x;
   trs.clipPlane[1].y = -trs.clipPlane[0].y;
   trs.clipPlane[1].z = -trs.clipPlane[0].z;

   // ax + by + cz + d = 0
   // n dot ptOnPlane + d = 0
   // d = - n dot ptOnPlane
   
   trs.clipPlane[1].d = - m_dot(xlFarOrigin, trs.clipPlane[1]);
   
   // left clip plane
   trs.clipPlane[2].set(xlLowerLeft, xlOrigin, xlUpperLeft); 
   // right clip plane
   trs.clipPlane[3].set(xlUpperRight, xlOrigin, xlLowerRight); 
   // top clip plane
   trs.clipPlane[4].set(xlUpperLeft, xlOrigin, xlUpperRight); 
   // bottom clip plane
   trs.clipPlane[5].set(xlLowerRight, xlOrigin, xlLowerLeft); 
}

void TerrainFile::flushCache()
{
   TerrainRender::flushCache();
}

void TerrainFile::render(TSRenderContext &rc)
{
   trs.frameIndex++;
   trs.texScale.set(0,0);
   trs.farDistance = visibleDistance;
   trs.hazeDistance = hazeDistance;
   trs.hazeScale = 1.0 / (trs.farDistance - trs.hazeDistance);

   trs.renderContext = &rc;
   trs.camera = rc.getCamera();
   trs.pointArray = rc.getPointArray();
   trs.gfxSurface = rc.getSurface();

	trs.gfxSurface->setFillMode(GFX_FILL_CONSTANT);
   trs.gfxSurface->setHazeSource(GFX_HAZE_NONE);
   trs.gfxSurface->setShadeSource(GFX_SHADE_NONE);
   trs.gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
   trs.gfxSurface->setTexturePerspective(FALSE);
	trs.gfxSurface->setTransparency(FALSE);

   trs.pointArray->useIntensities(false);
   trs.pointArray->useTextures(false);
   trs.pointArray->useHazes(true);
   trs.pointArray->setFarDist(trs.farDistance);
   trs.gfxSurface->setHazeSource(GFX_HAZE_VERTEX);
   trs.screenSize = screenSize;
   
   TMat3F mat = trs.camera->getTOC();
	mat.inverse();
	trs.camPos = mat.p;


   trs.pointCount = 0;
   trs.edgeCount = 0;
   trs.squareCount = 0;
   
   trs.squareSize = squareSize;
   trs.squareScale.set(squareSize, squareSize);
   
   trs.currentBlock = blockMap[0][0];
   trs.currentGrowFactor = 0;
   trs.blockPos.set(0,0);
   
   // compute pixelError
   if(screenError >= 0.001)
      trs.pixelError = 1 / trs.camera->projectRadius(screenError, 1);
   else
      trs.pixelError = 0.000001;
      
   TerrainRender::buildClippingPlanes();
   TerrainRender::buildLightArray();
   TerrainRender::buildDetailTable();
   
   
   int p0, p1, p2, p3;
   
   int blockPoints[FileSize+1][FileSize+1];
   int horzEdges[FileSize+1][FileSize];
   int vertEdges[FileSize+1][FileSize];
   
   int i, j;
   for(i = 0; i < FileSize + 1; i++)
   {
      for(j = 0; j < FileSize + 1; j++)
      {
         int x = i % FileSize, y = j % FileSize;
         UInt16 height = blockMap[x][y]->heightMap[0];
         blockPoints[i][j] = TerrainRender::allocInitialPoint(Point3F(i * trs.squareSize * (1 << TerrainBlock::BlockShift), j * trs.squareSize *(1 << TerrainBlock::BlockShift), fixedToFloat(height)));
      }
   }
   for(i = 0; i < FileSize + 1; i++)
   {
      for(j = 0; j < FileSize; j++)
      {
         int e = TerrainRender::allocEdges(1);
         trs.edges[e].p1 = blockPoints[j][i];
         trs.edges[e].p2 = blockPoints[j+1][i];
         horzEdges[i][j] = e;
         e = TerrainRender::allocEdges(1);
         trs.edges[e].p1 = blockPoints[i][j+1];
         trs.edges[e].p2 = blockPoints[i][j];
         vertEdges[i][j] = e;
      }
   }
   int lastSquare[FileSize][FileSize];
   // process all blocks
   trs.currentBlock = blockMap[0][0];
   for(i = 0; i < FileSize; i++)
   {
      for(j = 0; j < FileSize; j++)
      {
         trs.blockOffset.set(i << TerrainBlock::BlockShift, j << TerrainBlock::BlockShift);
         trs.blockPos.set(i * trs.squareSize * (1 << TerrainBlock::BlockShift), j * trs.squareSize * (1 << TerrainBlock::BlockShift));
         TerrainRender::processCurrentBlock(horzEdges[j+1][i], vertEdges[i+1][j], horzEdges[j][i], vertEdges[i][j]);
      }
   }

   // transform the points
   TerrainRender::transformPoints();

   TerrainRender::cull();
   TerrainRender::sort();

   // render all blocks

   unusedTextureCount = 0;
   textureSpaceUsed = 0;
   staticTextureCount = 0;
   staticTSU = 0;

   trs.pointArray->useTextures(true);
   gPointArrayHackCoords = trs.lightCoords;
   gPointArrayHack = true;
   trs.gfxSurface->setFillMode(GFX_FILL_TEXTURE);
   trs.gfxSurface->setTexturePerspective(true);

   Glide::hackMode(trs.currentBlock->lightMap, false, trs.gfxSurface->getPalette());

   int squareCount;
   squareCount = TerrainRender::renderLevelZero();
   levelZeroCount = squareCount;
   
   squareCount = TerrainRender::renderFullMip(squareCount);
   fullMipCount = squareCount - levelZeroCount;
   
   TerrainRender::texture(squareCount);
   TerrainRender::renderLevelNonzero(squareCount);

   gPointArrayHack = false;
   trs.gfxSurface->setTextureWrap(false);
   trs.gfxSurface->setHazeSource(GFX_HAZE_NONE);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------


float TerrainRender::getWaterHeight(Point2F pos, int pointIndex)
{
   float waveHeight = 0;
   Point3F normal(0,0,1);
   float sinAngle, cosAngle;
   
   for(int i = 0; i < trs.waveCount; i++)
   {
      Point4F &wave = trs.waveParameters[i];
      float temp = pos.x * wave.x + pos.y * wave.y + wave.z;
      m_sincos(temp, &sinAngle, &cosAngle);
      waveHeight += wave.w * sinAngle;
      normal.x += wave.w * wave.x * cosAngle;
      normal.y += wave.w * wave.y * cosAngle;
   }
   normal.normalize();
   
   Point3F vec(pos.x - trs.camPos.x, pos.y - trs.camPos.y, waveHeight - trs.camPos.z);
   vec += normal * (-2 * m_dot(vec, normal));
   vec.normalize(0.5);
   trs.envTexCoords[pointIndex].set(vec.x + 0.5, vec.y + 0.5);
   
   return waveHeight;
}

void TerrainRender::renderWaterBlock(bool underWater)
{
   static int pointBuffer[TerrainBlock::SquareMaxPoints + 2];
   static int lightBuffer[TerrainBlock::SquareMaxPoints + 2];
   int edgeStack[EdgeStackSize];

   for(int squareIndex = 0; squareIndex < trs.squareCount; squareIndex++)
   {
      EmitSquare *sq = trs.squares + squareIndex;

      // loop through the edges on the square
      
      int curPoint = 0;
   
      for(int squareEdge = 0; squareEdge < 4; squareEdge++)
      {
         // fill the point buffer with the recursed edges
         int stackSize = 0;
         int curEdge = sq->edge[squareEdge];

         for(;;) {
            if(trs.edges[curEdge].mp < InvalidPointIndex)
            {
               // push the tail of the edge onto the stack
               // tail is firstSubEdge + 1 if top, right
               // or firstSubEdge if bottom, left
               if(squareEdge < 2)
               {
                  curEdge = trs.edges[curEdge].firstSubEdge;
                  edgeStack[stackSize++] = curEdge + 1;
               }
               else
               {
                  curEdge = trs.edges[curEdge].firstSubEdge + 1;
                  edgeStack[stackSize++] = curEdge - 1;
               }
            }
            else
            {
               // top and right edges go p1->p2
               // bottom and left edges go p2->p1
               pointBuffer[curPoint++] = 
                  squareEdge < 2 ? trs.edges[curEdge].p1 : trs.edges[curEdge].p2;
               if(stackSize)
                  curEdge = edgeStack[--stackSize];
               else 
                  break;
            }
         }
      }
      // early out for strict square:
      for(int i = 0; i < curPoint; i++)
         lightBuffer[i] = pointBuffer[i];
      // indices are in pointBuffer
      if(underWater)
         emitTriFanReverse(pointBuffer, lightBuffer, curPoint);  //(1, 2, 3) (1, 3, 4)
      else
         emitTriFan(pointBuffer, lightBuffer, curPoint);  //(1, 2, 3) (1, 3, 4)
   }
}

void drawWaterSurface(TSRenderContext &rc, Point2F extent, Point2F texScale, float height, TSMaterialList *matList, Point4F *params, int paramCount, ColorF waterColor, float waterAlpha)
{
   TerrainRender::renderWater(rc, extent, texScale, height, matList, params, paramCount, waterColor, waterAlpha);
}
int index = 0;

void TerrainRender::renderWater(TSRenderContext &rc, Point2F extent, Point2F texScale, float height, TSMaterialList *matList, Point4F *params, int paramCount, ColorF waterColor, float waterAlpha)
{
   //trs.farDistance = visibleDistance;
   //trs.hazeDistance = hazeDistance;
   //trs.hazeScale = 1.0 / (trs.farDistance - trs.hazeDistance);

   trs.texScale = texScale;
   trs.renderContext = &rc;
   trs.camera = rc.getCamera();
   trs.pointArray = rc.getPointArray();
   trs.gfxSurface = rc.getSurface();
   
   if(trs.camera->getCameraType() == TS::OrthographicCameraType)
      return;

   trs.waterFreq = GetTickCount() / 1000.0f * M_2PI;
   trs.waterScale = 0.05 * M_PI;
   
   trs.waveCount = paramCount;
   for(int i= 0; i < paramCount; i++)
   {
      trs.waveParameters[i] = params[i];
      trs.waveParameters[i].w *= height;
      trs.waveParameters[i].x *= trs.waterScale;
      trs.waveParameters[i].y *= trs.waterScale;
      trs.waveParameters[i].z *= trs.waterFreq;
   }
   
   TMat3F mat = trs.camera->getTOC();
	mat.inverse();
	trs.camPos = mat.p;

   trs.minWaterSquareSize = minSquareSize;
   trs.squareScale.set(1, 1);
   
   trs.pointCount = 0;
   trs.edgeCount = 0;
   trs.squareCount = 0;
   
   TerrainRender::buildClippingPlanes();
   // compute pixelError
   if(screenError >= 0.001)
      trs.pixelError = 1 / trs.camera->projectRadius(screenError, 1);
   else
      trs.pixelError = 0.000001;
   processWaterBlock(extent, height);
   transformWaterPoints();

   bool underWater = false;
   if(trs.camPos.x >= 0 && trs.camPos.y >= 0 && trs.camPos.z <= -height &&
         trs.camPos.x <= extent.x && trs.camPos.y <= extent.y)
      underWater = true;
   else if(trs.cameraSquare != -1)
   {
      // see if the camera is above or under the water triangles of the square
      EmitSquare *esquare = trs.squares + trs.cameraSquare;
      Point3F *topLeft = trs.emitPoints + trs.edges[esquare->edge[0]].p1;
      Point3F *topRight = trs.emitPoints + trs.edges[esquare->edge[0]].p2;
      Point3F *bottomLeft = trs.emitPoints + trs.edges[esquare->edge[2]].p1;
      Point3F *bottomRight = trs.emitPoints + trs.edges[esquare->edge[2]].p2;
      
      Point3F normal;
      
      // check if the camera is in upper right corner or lower left (square is split 135)
      if( (trs.camPos.x - topLeft->x) / (bottomRight->x - topLeft->x) >
            (trs.camPos.y - topLeft->y) / (bottomRight->y - topLeft->y) )
         m_cross( *topLeft - *topRight, *bottomRight - *topRight, &normal);
      else
         m_cross( *bottomRight - *bottomLeft, *topLeft - *bottomLeft, &normal);

      if(m_dot(trs.camPos - *topLeft, normal) < 0)
         underWater = true;
   }

   if(matList)
   {
      enum { EnvironmentMapIndex, WaterMapIndex };
      
	   trs.gfxSurface->setFillMode(GFX_FILL_TEXTURE);
      GFXBitmap *environMap = (GFXBitmap *)(*matList)[EnvironmentMapIndex].getTextureMap();
      GFXBitmap *waterMap = (GFXBitmap *)(*matList)[WaterMapIndex].getTextureMap();

      // TMU 0 gets the water map
      // TMU 1 gets the environment map
      if(underWater)
      {
         trs.gfxSurface->setTextureMap(waterMap);
         trs.gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
         trs.pointArray->useTextures(trs.waterTexCoords);
      }
      else
      {
         trs.gfxSurface->setAlphaSource(GFX_ALPHA_CONSTANT);
         if(!Glide::hackTextureValid(&(environMap->getCacheInfo()), environMap->paletteIndex, trs.gfxSurface->getPalette()))
            Glide::hackDownloadBitmap(environMap, trs.gfxSurface->getPalette(), &(environMap->getCacheInfo()), true);
         Glide::hackMode(waterMap, true, trs.gfxSurface->getPalette());
         trs.gfxSurface->setConstantAlpha(waterAlpha);
         trs.pointArray->useTextures(trs.envTexCoords);
         gPointArrayHack = true;
         gPointArrayHackCoords = trs.waterTexCoords;
	   }
      trs.gfxSurface->setTextureWrap(true);
      trs.pointArray->useTextures(true);
   }
   else
   {
	   trs.gfxSurface->setFillMode(GFX_FILL_CONSTANT);
      trs.gfxSurface->setAlphaSource(GFX_ALPHA_CONSTANT);
      trs.gfxSurface->setConstantAlpha(0.5);
      trs.pointArray->useTextures(false);
      ColorF c(0, 0, 1);
      trs.gfxSurface->setFillColor(&c);
   }
   trs.gfxSurface->setHazeSource(GFX_HAZE_VERTEX);
   trs.gfxSurface->setShadeSource(GFX_SHADE_NONE);
   trs.gfxSurface->setTexturePerspective(FALSE);
	trs.gfxSurface->setTransparency(FALSE);

   trs.pointArray->useIntensities(false);
   trs.pointArray->useHazes(true);

   renderWaterBlock(underWater);
   if(underWater)
      trs.gfxSurface->setSurfaceAlphaBlend(&waterColor, 0.5);
   
   trs.gfxSurface->setTextureWrap(false);
   gPointArrayHack = false;
}

void TerrainRender::processWaterBlock(Point2F extent, float height)
{
   WaterStackNode stack[MaxWaterStackSize];

   int p1, p2, p3, p4;
   
   p1 = allocWaterPoint(Point2F(0, extent.y), 0, 0);
   p2 = allocWaterPoint(Point2F(extent.x, extent.y), 0, 0);
   p3 = allocWaterPoint(Point2F(extent.x, 0), 0, 0);
   p4 = allocWaterPoint(Point2F(0, 0), 0, 0);

   int e0 = allocEdges(4);
   trs.edges[e0].p1 = p1;
   trs.edges[e0].p2 = p2;
   trs.edges[e0+1].p1 = p2;
   trs.edges[e0+1].p2 = p3;
   trs.edges[e0+2].p1 = p4;
   trs.edges[e0+2].p2 = p3;
   trs.edges[e0+3].p1 = p1;
   trs.edges[e0+3].p2 = p4;

   stack[0].position.set(0,0);
   stack[0].extent = extent;
   stack[0].clipFlags = 0xFF;  // test all the planes
   stack[0].top = e0;
   stack[0].right = e0 + 1;
   stack[0].bottom = e0 + 2;
   stack[0].left = e0 + 3;
   
   int curStackSize = 1;
   trs.cameraSquare = -1;
   
   float ps = screenSize / trs.camera->getPixelScale();
   float distanceScale = trs.camera->projectionDistance(ps, 1);
   Point3F minPoint, maxPoint;
   
   // calculate detail distance:
   // dist = squareSize * distanceScale;

   while(curStackSize)
   {
      WaterStackNode *n = stack + curStackSize - 1;
      minPoint.set(n->position.x, n->position.y, -height);
      maxPoint.set(n->position.x + n->extent.x, n->position.y + n->extent.y, height);

      trs.currentGrowFactor = 0;
      int clipFlags = n->clipFlags;
      if(clipFlags)
      {
         clipFlags = TestSquareVisibility(minPoint, maxPoint, clipFlags, trs.minWaterSquareSize );
         if(clipFlags == -1)
         {
            curStackSize--;
            continue;
         }
      }
      Point2F pos = n->position, ext = n->extent;
      int top = n->top, right = n->right, bottom = n->bottom, left = n->left;
      
      if(n->extent.x >= n->extent.y * 2)
      {
         // divide into two squares horizontally and continue on.
         float midX = pos.x + ext.x * .5;
         subdivideWaterEdge(top, Point2F(midX, pos.y + ext.y));
         subdivideWaterEdge(bottom, Point2F(midX, pos.y));
      
         int crossEdge = allocEdges(1);
         trs.edges[crossEdge].p1 = trs.edges[top].mp;
         trs.edges[crossEdge].p2 = trs.edges[bottom].mp;
         
         n[0].position = pos;
         n[0].extent.set(midX - pos.x, ext.y);
         
         n[0].top = trs.edges[top].firstSubEdge;
         n[0].right = crossEdge;
         n[0].bottom = trs.edges[bottom].firstSubEdge;
         n[0].left = left;
         n[0].clipFlags = clipFlags;
         
         n[1].position.set(midX, pos.y);
         n[1].extent.set(pos.x + ext.x - midX, ext.y);
         
         n[1].top = trs.edges[top].firstSubEdge + 1;
         n[1].right = right;
         n[1].bottom = trs.edges[bottom].firstSubEdge+1;
         n[1].left = crossEdge;
         n[1].clipFlags = clipFlags;
         
         curStackSize++;
         continue;
      }
      if(n->extent.y >= n->extent.x * 2)
      {
         // divide into two squares horizontally and continue on.
         float midY = pos.y + ext.y * .5;
         subdivideWaterEdge(left, Point2F(pos.x, midY));
         subdivideWaterEdge(bottom, Point2F(pos.x + ext.x, midY));
      
         int crossEdge = allocEdges(1);
         trs.edges[crossEdge].p1 = trs.edges[left].mp;
         trs.edges[crossEdge].p2 = trs.edges[right].mp;
         
         n[0].position = pos;
         n[0].extent.set(ext.x, midY - pos.y);
         
         n[0].top = crossEdge;
         n[0].right = trs.edges[right].firstSubEdge + 1;
         n[0].bottom = bottom;
         n[0].left = trs.edges[left].firstSubEdge + 1;
         n[0].clipFlags = clipFlags;
         
         n[1].position.set(pos.x, midY);
         n[1].extent.set(ext.x, pos.y + ext.y - midY);
         
         n[1].top = top;
         n[1].right = trs.edges[right].firstSubEdge;
         n[1].bottom = crossEdge;
         n[1].left = trs.edges[left].firstSubEdge;
         n[1].clipFlags = clipFlags;
         
         curStackSize++;
         continue;
      }
      // check if squareSize is less than min square size

      float blockDistance = getSquareDistance(minPoint, maxPoint);
      
      if(trs.edgeCount > ScanEdgeLimit || trs.pointCount > ScanPointLimit || ext.x <= trs.minWaterSquareSize || ext.y <= trs.minWaterSquareSize)
      {
         // check if the camera is in the square:
         // save the square number off for future use if it is
         if(blockDistance < 0.01)
            trs.cameraSquare = trs.squareCount;
            
         
         // emit the square.
         emitWaterSquare(n);
         curStackSize--;
         continue;
      }

      // mandatory divisions are done - now gotta do size check vs. screen error.
      float squareSize = ext.x > ext.y ? ext.x : ext.y;
      float detailDistance = distanceScale * squareSize;
      if(blockDistance >= detailDistance)
      {
         clampEdge(top);
         clampEdge(right);
         clampEdge(bottom);
         clampEdge(left);

         // emit the square.
         emitWaterSquare(n);
         curStackSize--;
         continue;
      }
      
      float clampDistance = detailDistance * 0.4;
      if(blockDistance > clampDistance)
         trs.currentGrowFactor = (blockDistance - clampDistance) / (0.6 * detailDistance);

      // split up the square
      
      float midX = pos.x + ext.x * 0.5;
      float midY = pos.y + ext.y * 0.5;
      
      // always split135
      int p1 = trs.edges[top].p1;
      int p2 = trs.edges[bottom].p2;
      
      int midPoint = allocWaterPoint(Point2F(midX, midY), p1, p2);

      subdivideWaterEdge(top, Point2F(midX, pos.y + ext.y));
      subdivideWaterEdge(right, Point2F(pos.x + ext.x, midY));
      subdivideWaterEdge(bottom, Point2F(midX, pos.y));
      subdivideWaterEdge(left, Point2F(pos.x, midY));
   
      // cross edges go top, right, bottom, left
      int firstCrossEdge = allocEdges(4);
      
      trs.edges[firstCrossEdge].p1 = trs.edges[top].mp;
      trs.edges[firstCrossEdge].p2 = midPoint;
      trs.edges[firstCrossEdge+1].p1 = midPoint;
      trs.edges[firstCrossEdge+1].p2 = trs.edges[right].mp;
      trs.edges[firstCrossEdge+2].p1 = midPoint;
      trs.edges[firstCrossEdge+2].p2 = trs.edges[bottom].mp;
      trs.edges[firstCrossEdge+3].p1 = trs.edges[left].mp;
      trs.edges[firstCrossEdge+3].p2 = midPoint;
      
      n[0].position = pos;
      n[0].extent.set(midX - pos.x, midY - pos.y);
      n[0].top = firstCrossEdge + 3;
      n[0].right = firstCrossEdge + 2;
      n[0].bottom = trs.edges[bottom].firstSubEdge;
      n[0].left = trs.edges[left].firstSubEdge + 1;

      n[1].position.set(midX, pos.y);
      n[1].extent.set(pos.x + ext.x - midX, midY - pos.y);
      n[1].top = firstCrossEdge + 1;
      n[1].right = trs.edges[right].firstSubEdge + 1;
      n[1].bottom = trs.edges[bottom].firstSubEdge + 1;
      n[1].left = firstCrossEdge + 2;

      n[2].position.set(midX, midY);
      n[2].extent.set(pos.x + ext.x - midX, pos.y + ext.y - midY);
      n[2].top = trs.edges[top].firstSubEdge + 1;
      n[2].right = trs.edges[right].firstSubEdge;
      n[2].bottom = firstCrossEdge + 1;
      n[2].left = firstCrossEdge;

      n[3].position.set(pos.x, midY);
      n[3].extent.set(midX - pos.x, pos.y + ext.x - midY);
      n[3].top = trs.edges[top].firstSubEdge;
      n[3].right = firstCrossEdge;
      n[3].bottom = firstCrossEdge + 3;
      n[3].left = trs.edges[left].firstSubEdge;

      for(int i = 0; i < 4; i++)
         n[i].clipFlags = clipFlags;

      curStackSize += 3;
   }
}

