//----------------------------------------------------------------------------
//   
//  $Workfile:   LS_cmplx.cpp  $
//  $Version$
//  $Revision:   1.00  $
//    
//  DESCRIPTION:
//        LandScape complex or compund methods
//        
//  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
// 
//----------------------------------------------------------------------------

#include "LS.h"
#include <m_trig.h>

//----------------------------------------------------------------------------
// for every pixel in stack(0) < threshold, copy same pixel from stack(_n)

void LandScape::overlay( int _n, float _thresh )
{
   if ( stack.size() < _n+1 )
   {
      sprintf( mExecString, "OVERLAY: Not enough items on the stack to overlay" );
      mExecStatus = EXEC_FAILURE;
      return;
   }
    // assumes both matrices are the same size
    int size = stack[0]->size;

    for (int y = 0; y < size; y++)
        for (int x = 0; x < size; x++)
            if (stack[0]->real[y*size + x] <= _thresh)
                stack[0]->real[y*size + x] = stack[_n]->real[y*size + x];

   delete stack[_n];
   stack.erase( stack.begin()+_n );
}

//----------------------------------------------------------------------------
// this is a very complex operation.  It merges stack(0) with a mask
// image in stack(_n).  Forcing pixels in (0) that are within a distance
// _dist of a non-zero pixel in (_n) to migrate towards the pixel value
// in (_n).  The amount of migration is based on the average value of its
// nearest neighbors and a percentage of the pixels relative distance 
// from the pixel in (_n) and _dist.
//
// the goal is to bend the slopes in (0) towards the non-zero values in (_n)
// from a given disance surrounding the non-zero pixels in (_n).  This
// allows you to specify fixed artifacts in (_n) and apply them to
// a random terrain in (0).

void LandScape::blend( int _n, int _dist )
{
   if ( stack.size() < _n+1 )
   {
      sprintf( mExecString, "BLEND: not enough items on the stack to blend");
      mExecStatus = EXEC_FAILURE;
      return;
   }

   int   x,y, r,c, i,j;
   float avPix;
   int   avTot,avIdx;

    // assumes both matrices are the same size
    int size = stack[0]->size;

   for ( y=0; y<size*size; y+=size )  // for every pixel
   {
      for ( x=0; x<size; x++ )
      {
         if ( stack[_n]->real[ y+x ] )
         {
            for ( r=-_dist; r<=_dist; r++ )     // for every pix within _dist of pixel
            {
               for ( c=-_dist; c<=_dist; c++ )
               {
                  avPix = avTot = 0;
                  if ( !stack[_n]->real[x+c+y+r*size] )
                  {
                     for ( i=-size; i<=size; i+=size )   // averag pix with neighbors
                     {
                        for( j=-1; j<=1; j++ )
                        {
                           avIdx = x+c+j+y+r*size+i;
                           if ( (avIdx >= 0) && (avIdx < size*size) )
                           {
                              if ( !stack[_n]->real[ avIdx ] )    // if pix[0](xcj,yri) is != 0, average it instead
                              {
                                 avPix += stack[0]->real[ avIdx ];
                                 avTot++;
                              }
                           }
                        }
                     }
                     if ( avTot )
                     {
                        avPix = avPix/avTot;
                        // then store adjusted average based on percentage relative to distance
                        stack[0]->real[x+c+y+r*size] = avPix -( ((avPix - stack[_n]->real[x+y]) * ((float)(2*_dist)+1-abs(r)-abs(c)))/((float)(2*_dist)+1) );
                     }
                  }
               }
            }
            stack[0]->real[ y+x ] = stack[_n]->real[ y+x ];
         }
      }
   }
}


//void LandScape::blend( int _n, int _dist )
//{
//   if ( stack.size() < _n+1 )
//      return;
//
//   int   x,y, r,c, i,j;
//   float avPix;
//   int   avTot,avIdx;
//
//   for ( y=0; y<width*height; y+=width )  // for every pixel
//   {
//      for ( x=0; x<width; x++ )
//      {
//         if ( stack[_n]->real[ y+x ] )
//         {
//            for ( r=-_dist; r<=_dist; r++ )     // for every pix within _dist of pixel
//            {
//               for ( c=abs(r)-_dist; c<=_dist-abs(r); c++ )
//               {
//                  avPix = avTot = 0;
//                  for ( i=-width; i<=width; i+=width )   // averag pix with neighbors
//                  {
//                     for( j=-1; j<=1; j++ )
//                     {
//                        avIdx = x+c+j+y+r*width+i;
//                        if ( (avIdx >= 0) && (avIdx < width*height) )
//                        {
//                           if ( stack[_n]->real[ avIdx ] )    // if pix[0](xcj,yri) is != 0, average it instead
//                              avPix += stack[_n]->real[ avIdx ];
//                           else
//                              avPix += stack[0]->real[ avIdx ];
//                           avTot++;
//                        }
//                     }
//                  }
//                  if ( avTot )
//                  {
//                     avPix = avPix/avTot;
//                     // then store adjusted average based on percentage relative to distance
//                     stack[0]->real[x+c+y+r*width] = avPix -( ((avPix - stack[_n]->real[x+y]) * ((float)_dist+1-abs(r)-abs(c)))/((float)_dist+1) );
//                  }
//               }
//            }
//            stack[0]->real[ y+x ] = stack[_n]->real[ y+x ];
//         }
//      }
//   }
//}

//----------------------------------------------------------------------------

void LandScape::flipX()
{
   int width = stack[0]->size;

   if (width < 1)
   {
      sprintf( mExecString, "FLIPX: Nothing on the stack");
      mExecStatus = EXEC_FAILURE;
      return;
   }

   float *realPtr = stack[0]->real;

   for ( int y=0; y<sq(width); y+=width )
      for ( int x=0; x<(width>>1); x++ )
      {
         float tmp = realPtr[y+x];
         realPtr[y+x] = realPtr[y+width-1-x];
         realPtr[y+width-1-x] = tmp;
      }
}   

//----------------------------------------------------------------------------

void LandScape::flipY()
{
   int width = stack[0]->size;

   if (width < 1)
   {
      sprintf( mExecString, "FLIPY: Nothing on the stack");
      mExecStatus = EXEC_FAILURE;
      return;
   }

   float * realPtr = stack[0]->real;

   for ( int y=0; y<(sq(width)>>1); y+=width )
      for ( int x=0; x<width; x++ )
      {
         float tmp = realPtr[y+x];
         realPtr[y+x] = realPtr[sq(width)-y-width+x];
         realPtr[sq(width)-width-y+x] = tmp;
      }
}   
