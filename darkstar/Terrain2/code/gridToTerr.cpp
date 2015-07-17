#include "types.h"
#include "grdFile.h"
#include "grdBlock.h"
#include "terrData.h"

TerrainFile *createFileFromGridFile(GridFile *gf)
{
   TerrainFile *ret = new TerrainFile;
   ret->squareSize = 8;
   
   int i, j;

   GridBlock *block = gf->getBlock(Point2I(0,0));
   BYTE textureVals[256];
   TSMaterialList *materialMap = gf->getMaterialList();

   for(int i = 0; i < 256; i++)
   {
      if(materialMap->getMaterialsCount() <= i)
         break;
         
      GFXBitmap *textureMap = (GFXBitmap *)materialMap->getMaterial(i).getTextureMap();
      if(textureMap)
         textureVals[i] = *(textureMap->getDetailLevel(textureMap->detailLevels - 1));
      else
         textureVals[i] = 0;
   }

   float minHeight = gf->getHeightRange(false).fMin;
   float minAdd = (minHeight < 0.f) ? -minHeight : 0.f;
   
   TerrainBlock *blk = new TerrainBlock;
   UInt16 *lm = block->getLightMap();
   GFXBitmap *lightMap = blk->lightMap;
   GFXBitmap *baseTextureMap = blk->baseTextureMap;
   
   blk->materialMap = block->getMaterialMap();
   blk->materialList = gf->getMaterialList();
   blk->paletteIndex = blk->materialList->getMaterial(1).getTextureMap()->paletteIndex;
   baseTextureMap->paletteIndex = blk->paletteIndex;
   blk->dynamicLightMap->paletteIndex = blk->paletteIndex;
         
   int numMaterials = blk->materialList->getMaterialsCount();
   
   for(i = 0; i < TerrainBlock::BlockSize; i++)
   {
      for(j = 0; j < TerrainBlock::BlockSize; j++)
      {
         float height = block->getHeight(0, Point2I(i, j))->height;
         
         // ensure the height is withing our range (0->2047) 2^11-1
         height += minAdd;
         if(height >= 2047.0f)
            height = 2047.0f;
            
         // add the height as fixedpoint
         blk->heightMap[i + j + (j << TerrainBlock::BlockShift)] = floatToFixed(height);
         UInt16 lmsrc = lm[i + j + (j << TerrainBlock::BlockShift)];
         
         int r = (lmsrc >> 8) & 0xF;
         int g = (lmsrc >> 4) & 0xF;
         int b = lmsrc & 0xF;
         
         *((UInt16*)lightMap->getAddress(i,j)) =
               (r << 12) | ((r << 8) & 0x8) |
               (g << 7) | ((g << 3) & 0xC) |
               (b << 1) | (b >> 3);
                           

         BYTE *tMapPtr = baseTextureMap->getAddress(i,j);
         GridBlock::Material *material = blk->getMaterial(i, j);

         if(material->index >= numMaterials)
         {  
            AssertWarn(0, avar("Material index found to be out of range: %d/%d at %d-%d. Setting to 0.",
               material->index, numMaterials, i, j));
            material->index = 0;
         }
         *tMapPtr = textureVals[material->index];
      }
   }
   blk->buildGridMap();
   for(i = 0; i < TerrainFile::FileSize; i ++)
      for(j = 0; j < TerrainFile::FileSize; j ++)
         ret->blockMap[i][j] = blk;
   return ret;
}

