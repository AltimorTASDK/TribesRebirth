//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _CHUNKEDBITMAP_H_
#define _CHUNKEDBITMAP_H_

#include <g_bitmap.h>
#include <g_surfac.h>
#include <tvector.h>
#include <gwcanvas.h>
#include <m_box.h>

class ChunkedBitmap 
{
private:
   Vector<GFXBitmap *> chunks;

   int   xChunkCnt;
   int   yChunkCnt;
   int   chunkWidth;
   int   chunkHeight;
   GFXBitmap *source;
   bool transparent;

   void copy(GFXBitmap *bmp, GFXBitmap *chunk, Point2I &offset);

public:
   ChunkedBitmap();
   ChunkedBitmap(GFXBitmap *, bool _transparent = false);
   ~ChunkedBitmap();

   void setTransparent(bool tf);
   void clear();
   void set(GFXBitmap *, bool _transparent = false);
   void flush(GFXSurface *surface, bool reload);
   void drawBitmap(GWCanvas *canvas); 
   void drawBitmap(GWCanvas *canvas, Box2I &region);  
   void drawBitmap(GFXSurface *sfc);
   void drawBitmap(GFXSurface *sfc, Box2I &region); 
};

#endif //_CHUNKEDBITMAP_H_