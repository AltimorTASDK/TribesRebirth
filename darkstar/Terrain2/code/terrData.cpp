#include "terrData.h"


extern void initTerrainVars();

TerrainBlock::TerrainBlock()
{
   lightMap = GFXBitmap::create(BlockSize - 1, BlockSize - 1, 16);
   baseTextureMap = GFXBitmap::create(BlockSize -1, BlockSize - 1);
   dynamicLightMap = GFXBitmap::create(16, 16, 16);
   
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
   initTerrainVars();
}

TerrainBlock::~TerrainBlock()
{
   delete lightMap;
   delete baseTextureMap;
   delete dynamicLightMap;
   
   delete[] compositeBuffer;
   delete[] gridMapBase;
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

void TerrainBlock::buildGridMap()
{
   for(int i = BlockShift; i >= 0; i--)
   {
      int squareCount = 1 << (BlockShift - i);
      int squareSize = (TerrainBlock::BlockSize - 1) / squareCount;
      
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
            bool empty = true;
            
            for(int sizeX = 0; sizeX <= squareSize; sizeX++)
            {
               for(int sizeY = 0; sizeY <= squareSize; sizeY++)
               {
                  int x = squareX * squareSize + sizeX;
                  int y = squareY * squareSize + sizeY;

                  if(sizeX != squareSize && sizeY != squareSize)
                  {
                     GridBlock::Material *mat = getMaterial(x, y);
                     if(mat->getEmptyLevel() == 0)
                        empty = false;
                  }
                  UInt16 ht = heightMap[x + y + (y << BlockShift)];

                  if(ht < min)
                     min = ht;
                  if(ht > max)
                     max = ht;
                  Point3F pt(sizeX, sizeY, fixedToFloat(ht));
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
            
            sq->flags = empty ? GridSquare::Empty : 0;
            
            bool shouldSplit45 = ((squareX ^ squareY) & 1) == 0;
            bool split45;
   
            //split45 = shouldSplit45;          
            if(i == 0)
               split45 = shouldSplit45;
            else if(i < 4 && shouldSplit45 == parentSplit45)
               split45 = shouldSplit45;
            else
               split45 = mindev45 < mindev135;
            
            //split45 = shouldSplit45;
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

TerrainFile::TerrainFile()
{
   squareSize = 8;
   
   visibleDistance = 500;
   hazeDistance = 250;
   
   int i, j;
   for(j = 0; j < FileSize; j++)
      for(i = 0; i < FileSize; i++)
         blockMap[j][i] = NULL;
}

TerrainFile::~TerrainFile()
{
   flushCache();
   delete blockMap[0][0];
}

TerrainBlock * TerrainFile::getBlock(const Point2I & fPos, Point2I * bPos)
{
   if(bPos)
   {
      bPos->x = fPos.x % (TerrainBlock::BlockSize - 1);
      bPos->y = fPos.y % (TerrainBlock::BlockSize - 1);
   }
   return(blockMap[fPos.x >> TerrainBlock::BlockShift][fPos.y >> TerrainBlock::BlockShift]);
}

void TerrainFile::setHeight(const Point2I & pos, float height)
{
   Point2I bPos;
   TerrainBlock * block = getBlock(pos, &bPos);
   UInt16 ht = floatToFixed(height);
   *((UInt16*)block->getHeightAddress(bPos.x, bPos.y)) = ht;
   
   Point3F p1, p2, p3, p4;
   PlaneF pl1, pl2, pl3, pl4;

   p1.set(0, 0, block->getHeight(pos.x, pos.y));
   p2.set(0, 1, block->getHeight(pos.x, pos.y + 1));
   p3.set(1, 1, block->getHeight(pos.x + 1, pos.y + 1));
   p4.set(1, 0, block->getHeight(pos.x + 1, pos.y));

   pl1.set(p1, p2, p3);
   pl2.set(p1, p3, p4);
   pl3.set(p1, p2, p4);
   pl4.set(p2, p3, p4);

   GridSquare * gs = block->findSquare(0, bPos);

   for(int sizeX = 0; sizeX <= 1; sizeX++)
      for(int sizeY = 0; sizeY <= 1; sizeY++)
      {
         int x = pos.x + sizeX;
         int y = pos.y + sizeY;
         UInt16 ht = block->getHeight(x, y);
         
         if(ht < gs->minHeight)
            gs->minHeight = ht;
         if(ht > gs->maxHeight)
            gs->maxHeight = ht;

         Point3F pt(sizeX, sizeY, fixedToFloat(ht));
            
         UInt16 dev, dev135;
         if(sizeX < sizeY)
            dev = calcDev(pl1, pt);
         else if(sizeX > sizeY)
            dev = calcDev(pl2, pt);
         else
            dev = Umax(calcDev(pl1, pt), calcDev(pl2, pt));   
         
         if(dev > gs->heightDeviance)
            gs->heightDeviance = dev;
      }
   
   for(int i = 1; i <= TerrainBlock::BlockShift; i++)
   {
      GridSquare * parent = block->findSquare(i, bPos);
      
      if(parent->minHeight > gs->minHeight)
         parent->minHeight = gs->minHeight;
      if(parent->maxHeight < gs->maxHeight)
         parent->maxHeight = gs->maxHeight;
      if(parent->heightDeviance < gs->heightDeviance)
         parent->heightDeviance = gs->heightDeviance;
   }
}

float TerrainFile::getHeight(const Point2I & pos)
{
   Point2I bPos;
   TerrainBlock * block = getBlock(pos, &bPos);
   return(fixedToFloat(block->getHeight(bPos.x, bPos.y)));
}

void TerrainFile::setMaterial(const Point2I & pos, const GridBlock::Material * material)
{
   Point2I bPos;
   TerrainBlock * block = getBlock(pos, &bPos);
   *block->getMaterial(bPos.x, bPos.y) = *material;
   bool gsEmpty = (block->findSquare(0, bPos)->flags & GridSquare::Empty) != 0;
   bool matEmpty = (material->flags & GridBlock::Material::EmptyMask) != 0;
   if(gsEmpty ^ matEmpty)
      block->buildGridMap();
}

GridBlock::Material * TerrainFile::getMaterial(const Point2I & pos)
{
   Point2I bPos;
   TerrainBlock * block = getBlock(pos, &bPos);
   return(block->getMaterial(bPos.x, bPos.y));
}

void TerrainFile::setLighting(const Point2I & pos, UInt16 color)
{
   Point2I bPos;
   TerrainBlock * block = getBlock(pos, &bPos);
   *((UInt16*)block->lightMap->getAddress(bPos.x, bPos.y)) = color;
}

UInt16 TerrainFile::getLighting(const Point2I & pos)
{
   Point2I bPos;
   TerrainBlock * block = getBlock(pos, &bPos);
   return(*((UInt16*)block->lightMap->getAddress(bPos.x, bPos.y)));
}