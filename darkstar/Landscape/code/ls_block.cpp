//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   LS_Block.cpp  $
//北  $Version$
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北        Fills a grid block height map from stack(0) and sub-region
//北        
//北  (c) Copyright 1996, 1997, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#include "ls.h"      // base class defined

// fills the grd height map from the sub region of stack[0] specified
// by _box.  this assumes that box is likely to be xy+1

void  LandScape::setGrdHeightMap( float *_pGrdHeightMap, Box2I& _box )
{
   if (!stack.size())
      return;

   int mask = stack[0]->size - 1;
   Point2I start = _box.fMin;
   Point2I end = _box.fMax;
   
   for ( int y=start.y; y < end.y; y++ )
   {
      float *basePtr = stack[0]->real + ((y-1) & mask) * stack[0]->size;

      for ( int x=start.x; x < end.x; x++ )
         *_pGrdHeightMap++ = *(basePtr + ((x) & mask));
   }
}
