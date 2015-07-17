//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#include <ml.h>
#include "grdRender.h"
#include "g_bitmap.h"

// you need these function prototypes
extern "C"  void __cdecl MipBlt( int src_start,int src_inc,int dst_start,int dst_width,int src_width,int src_adjust,int dst_adjust );


//---------------------------------------------------------------------------

DWORD gridCreateMipMap(
	int                  _mipLevel,     // mip level you want to use
	Box2I&               _area,         // area of _matMap you want
   Point2I&             _matMapSize,   // size of the _matMap
   GridBlock::Material* _matMap,
   TSMaterialList*      _matList,
   int                  stride,
   BYTE*                baseAddr,
   GFXBitmap*           _defTex       )
{
   int size = stride / (_area.len_x()+1);
	_matMap += (_area.fMax.y * _matMapSize.x) + _area.fMin.x;
   DWORD paletteIndex;
   
   for ( int y=0,yd=0,yo=0; y<=_area.len_y(); y++, yd+=size, yo-=_matMapSize.x )
   {
      for ( int x=0,xd=0,xo=0; x<=_area.len_x(); x++, xd+=size, xo++ )
      {
         UInt32 mapI = yo + xo;
         if (_matMap[mapI].index == 0xff)
            continue;
#ifdef DEBUG
			const GFXBitmap *pSrcBM;
			if (_matMap[mapI].index < _matList->getMaterialsCount()) {
				pSrcBM = (*_matList)[ _matMap[mapI].index ].getTextureMap();
			}
			else {
				static bool warned = 0;
				AssertWarn(warned,avar("gridCreateMipMap: Material index out of range. requested %d max %d.", 
				   _matMap[mapI].index, _matList->getMaterialsCount()) );
				AssertFatal(_defTex,"gridCreateMipMap: No default texture");
				warned = true;
				pSrcBM = _defTex;
			}
#else
			const GFXBitmap *pSrcBM = (*_matList)[ _matMap[mapI].index ].getTextureMap();
#endif
         paletteIndex = pSrcBM->paletteIndex;

         int   srcWidth = pSrcBM->width >> _mipLevel;
         int   src_start = (int)pSrcBM->pMipBits[_mipLevel];
         int   src_inc   = 1;
         int   src_adj   = 0;

         int   dst_start = yd*stride + xd + (int)baseAddr;
         int   dst_adj   = stride-srcWidth;
         int   dst_width = srcWidth>>2;           // div by 4 for dword count

         if ( srcWidth == 1 )
         {
            *(BYTE*)dst_start = *(BYTE*)src_start;
         }
         else
         {
            switch( _matMap[mapI].flags & GridBlock::Material::RotateMask )
            {
               case GridBlock::Material::Plain:
                  break;

               case GridBlock::Material::FlipX:
                  src_start += srcWidth-1;
                  src_inc = -1;
                  src_adj = 2*srcWidth;
                  break;

               case GridBlock::Material::FlipY:
                  src_start += srcWidth*srcWidth-srcWidth;
                  src_adj = -2*srcWidth;
                  break;

               case GridBlock::Material::Rotate:
                  src_start += srcWidth*srcWidth-srcWidth;
                  src_inc = -srcWidth;
                  src_adj = srcWidth*srcWidth+1;
                  break;

               case GridBlock::Material::FlipX|GridBlock::Material::FlipY:
                  src_start += srcWidth*srcWidth-1;
                  src_inc = -1;
                  break;

               case GridBlock::Material::FlipX|GridBlock::Material::Rotate:
                  src_start += srcWidth*srcWidth-1;
                  src_inc = -srcWidth;
                  src_adj = srcWidth*srcWidth-1;
                  break;

               case GridBlock::Material::FlipY|GridBlock::Material::Rotate:
                  src_inc = srcWidth;
                  src_adj = -(srcWidth*srcWidth-1);
                  break;

               case GridBlock::Material::FlipX|
               	GridBlock::Material::FlipY|GridBlock::Material::Rotate:
                  src_start += srcWidth-1;
                  src_inc = srcWidth;
                  src_adj = -((srcWidth*srcWidth)+1);
                  break;
            }
            // assembly language blitter.
            MipBlt(src_start,src_inc,dst_start,dst_width,srcWidth,src_adj,dst_adj);
         }
      }
   }
   return paletteIndex;
}
