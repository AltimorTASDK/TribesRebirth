//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "chunkedBitmap.h"


//------------------------------------------------------------------------------
ChunkedBitmap::ChunkedBitmap()
{
   source = NULL;
}   

//------------------------------------------------------------------------------
ChunkedBitmap::ChunkedBitmap(GFXBitmap *bmp, bool _transparent)
{
   source = NULL;
   set(bmp, _transparent);
}   

//------------------------------------------------------------------------------
ChunkedBitmap::~ChunkedBitmap()
{
   clear();
}     

//------------------------------------------------------------------------------
void ChunkedBitmap::clear()
{
   GFXBitmap *bmp;

   while (!chunks.empty())
   {
      bmp = chunks.front();
      chunks.pop_front();
      delete bmp;
   }
   source = NULL;   
}   

//------------------------------------------------------------------------------
void ChunkedBitmap::copy(GFXBitmap *bmp, GFXBitmap *chunk, Point2I &offset)
{
   char *src = (char*)bmp->getAddress(&offset);
   char *dst = (char*)chunk->pBits;

   for (int y=0; y < chunk->getHeight(); y++)
   {
      memcpy(dst, src, chunk->getWidth());
      src += bmp->stride;
      dst += chunk->stride;   
   }   
}   

//------------------------------------------------------------------------------
void ChunkedBitmap::set(GFXBitmap *bmp, bool _transparent)
{
   if (source != bmp)
      clear();


   xChunkCnt = (bmp->width + 255) / 256;
   yChunkCnt = (bmp->height + 255) / 256; 

   AssertWarn (xChunkCnt && yChunkCnt, "Trying to chunk an empty bitmap");

   int x, y; // looping through chunkWidth, and chunkHeight

   // dimensions of new bitmap
   chunkWidth = bmp->width / xChunkCnt;      
   chunkHeight = bmp->height / yChunkCnt;

   if (source == bmp)
   {
      if ( (xChunkCnt * yChunkCnt != chunks.size()) || (chunkWidth != chunks[0]->width) || (chunkHeight != chunks[0]->height) )
      {
         clear();
         source = NULL;
      }
   }

   for (y = 0; y < yChunkCnt; y++)
   {
      for (x = 0; x < xChunkCnt; x++)        
      {
         GFXBitmap *chunk;
         if (bmp == source)
            chunk = chunks[x+(y*xChunkCnt)];   
         else
         {
            chunk = GFXBitmap::create(chunkWidth, chunkHeight);
            chunk->paletteIndex = bmp->paletteIndex;
            chunks.push_back(chunk);
         }
         
         copy(bmp, chunk, Point2I(x*chunkWidth, y*chunkHeight));
      }
   }
   source = bmp;  
   setTransparent(_transparent);
}   


//------------------------------------------------------------------------------
void ChunkedBitmap::flush(GFXSurface *surface, bool reload)
{
   Vector<GFXBitmap*>::iterator i = chunks.begin();
   for (; i != chunks.end(); i++)
   {
      surface->setTextureMap( *i );
      surface->flushTexture( *i, reload);
      surface->setTextureMap( *i );
   }
}   


//------------------------------------------------------------------------------
void ChunkedBitmap::setTransparent(bool tf)
{
   Int32 limit = yChunkCnt * xChunkCnt;

   for (int i = 0; i < limit; i++) {
      if (tf)
         chunks[i]->attribute |= BMA_TRANSPARENT;   
      else
         chunks[i]->attribute &= ~BMA_TRANSPARENT;
   }
}  
 
//------------------------------------------------------------------------------
void ChunkedBitmap::drawBitmap(GFXSurface *sfc)
{
   if (chunks.size() == 0) return;

   Box2I entireScreen;
   entireScreen.fMin.set( 0, 0 );
   entireScreen.fMax.set( sfc->getWidth(),
                          sfc->getHeight() );
   
   drawBitmap(sfc, entireScreen);
}   

//------------------------------------------------------------------------------
void ChunkedBitmap::drawBitmap(GFXSurface *sfc, Box2I &region)
{
   if (chunks.size() == 0) return;

   Int32 width = region.fMax.x - region.fMin.x;
   Int32 height = region.fMax.y - region.fMin.y;
   
   Point2I stretch(width / xChunkCnt, height / yChunkCnt);
   
   int x, y;

   for (y = 0; y < yChunkCnt; y++)
   {
      for (x = 0; x < xChunkCnt; x++)
      {
         Point2I ptStart = region.fMin;
         Point2I ptEnd = stretch;

         ptStart.x += (x * stretch.x);
         ptStart.y += (y * stretch.y);
         
         if ((x + 1) == xChunkCnt)
         {
            ptEnd.x = region.fMax.x - ptStart.x;
         }
         if ((y + 1) == yChunkCnt)
         {
            ptEnd.y = region.fMax.y - ptStart.y;
         }

         sfc->drawBitmap2d( chunks[xChunkCnt * y + x], &ptStart, &ptEnd, GFX_FLIP_NONE );
      }
   }
}   

//------------------------------------------------------------------------------
void ChunkedBitmap::drawBitmap(GWCanvas *canvas)
{
   drawBitmap(canvas->getSurface());
}   

//------------------------------------------------------------------------------
void ChunkedBitmap::drawBitmap(GWCanvas *canvas, Box2I &region)
{
   drawBitmap(canvas->getSurface(), region );
}   
