//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   LS_Fill.cpp  $
//北  $Version$
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北        LandScape matrix filling methods
//北        
//北  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#include "LS.h"
#include <m_trig.h>

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#define El(vv, xq, yq)  vv[((yq) * size) + xq]  /* elmt of array */
#define minf(x,y) (((x)<(y))? (x):(y))

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北  
//北  Fill matrix with 1/f Gaussian noise.  Fill all four quadrants
//北  
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void LandScape::fillNormal( float _height )
{
   if (stack.size() < 2)
   {
       sprintf( mExecString, "FILL_NORMAL: not enough items on the stack to fillNormal (need 2)");
       mExecStatus = EXEC_FAILURE;
       return;
   }
   if (stack[0]->size != stack[1]->size)
   {
       sprintf( mExecString, "FILL_NORMAL: different sized matrices");
       mExecStatus = EXEC_FAILURE;
       return;
   }

   float    radius=0;
   float    phase;

   int size = stack[0]->size;
  
   for ( int y = 0; y < size>>1; y++ )
   {
      for ( int x = 0; x < size>>1; x++ )
      {
//         if ( (x|y) ) radius = pow((double) (x*x + y*y), -_height) * rand.getNormal();
//         if ( (x|y) ) radius = _height * (1.0/(x*x+y*y)) * rand.getNormal();

         // fill quadrant 2
         if ( (x|y) ) radius = float(pow((double)(x*x + y*y),(double)-_height) * rand.getNormal());
         phase = float(M_2PI * rand.getFloat());
         // real part is 2nd on stack
         El(stack[1]->real, x, y) = float(radius * cos(phase)); 
         // imag part is top of stack
         El(stack[0]->real, x, y) = float(radius * sin(phase));

         // fill quadrant 4
         if ( (x|y) ) radius = float(pow((double)(x*x + y*y),(double)-_height) * rand.getNormal());
         phase = float(M_2PI * rand.getFloat());
         El(stack[1]->real, size-x-1, size-y-1) = float(radius * cos(phase));
         El(stack[0]->real, size-x-1, size-y-1) = float(radius * sin(phase));

         // fill quadrant 3
         if ( (x|y) ) radius = float(pow((double)(x*x + y*y),(double)-_height) * rand.getNormal());
         phase = float(M_2PI * rand.getFloat());
         El(stack[1]->real, x, size-y-1) = float(radius * cos(phase)); 
         El(stack[0]->real, x, size-y-1) = float(radius * sin(phase));

         // fill quadrant 1
         if ( (x|y) ) radius = float(pow((double)(x*x + y*y),(double)-_height) * rand.getNormal());
         phase = float(M_2PI * rand.getFloat());
         El(stack[1]->real, size-x-1, y)  = float(radius * cos(phase));
         El(stack[0]->real, size-x-1, y)  = float(radius * sin(phase));
      }
   }
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北  
//北  Fill matrix with 1/f Gaussian noise.  Fill only Quadrants 1 and 3
//北  
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北


#if 0
void LandScape::fillNormalHalf( float _height )
{
   int      rankmax;
   float    radius=0;
   float    phase, rcos, rsin;

   rankmax = minf( (width / 2.0 - 0.5),(height / 2.0 - 0.5) );

   for ( int y=0; y<=rankmax; y++ )
   {
      for ( int x=0; x<=y; x++ )
      {
         if ( (x|y) )
            radius = _height * (1.0/(x*x+y*y)) * rand.getNormal();

         // fill quadrant 2 & 4 symmetrically
         phase = M_2PI * rand.getFloat();
         rcos = radius * cos(phase);
         rsin = radius * sin(phase);

         El(stack[0]->real, x, y) = rcos; 
         El(stack[0]->imag, x, y) = rsin;
         El(stack[0]->real, width-x-1, height-y-1) = rcos;
         El(stack[0]->imag, width-x-1, height-y-1) = rsin;

         // fill quadrant 1 & 3 symmetrically
         phase = M_2PI * rand.getFloat();
         rcos = radius * cos(phase); 
         rsin = radius * sin(phase);

         El(stack[0]->real, x, height-y-1) = rcos; 
         El(stack[0]->imag, x, height-y-1) = rsin;
         El(stack[0]->real, width-x-1, y)  = rcos;
         El(stack[0]->imag, width-x-1, y)  = rsin;
      }
   }
}
#endif

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北  
//北  Fill matrix with 1/f Gaussian noise.  Radial from center
//北  
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

//void LandScape::fillNormal( float _height )
//{
//   float    radius=0;
//   float    phase;
//
//   for ( int y=0; y<height>>1; y++ )
//   {
//      for ( int x=0; x<width>>1; x++ )
//      {
//         if ( (x|y) )
//            radius = _height * (1.0/(x*x+y*y)) * rand.getNormal();
//
//         // fill quadrant 2
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, x, y) = radius * cos(phase); 
//         El(stack[0]->imag, x, y) = radius * sin(phase);
//
//         // fill quadrant 4
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, width-x-1, height-y-1) = radius * cos(phase);
//         El(stack[0]->imag, width-x-1, height-y-1) = radius * sin(phase);
//
//         // fill quadrant 3
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, x, height-y-1) = radius * cos(phase); 
//         El(stack[0]->imag, x, height-y-1) = radius * sin(phase);
//
//         // fill quadrant 1
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, width-x-1, y)  = radius * cos(phase);
//         El(stack[0]->imag, width-x-1, y)  = radius * sin(phase);
//      }
//   }
//}

// this one is the original one that does a good job of 
// distributing the points.
//            radius = pow((double) (x*x + y*y), -_height) * rand.getNormal();

// this one fills from the center out but doesn't look good.
//            radius = pow((double) ( ((width>>1)-x)*((width>>1)-x) + ((height>>1)-y)*((height>>1)-y)), -_height) * rand.getNormal();

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

//// This routine works very good as a substitute normal randomizer
//void LandScape::fillNormal( float _height )
//{
//   float    radius=0;
//   float    phase;
//
//   for ( int y=0; y<height>>1; y++ )
//   {
//      for ( int x=0; x<width>>1; x++ )
//      {
//         if ( (x&&y) )
//            radius = _height * (1.0/(x*x+y*y)) * rand.getNormal();
//
//         // fill quadrant 2
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, x, y) = radius * cos(phase); 
//         El(stack[0]->imag, x, y) = radius * sin(phase);
//
//         // fill quadrant 4
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, width-x-1, height-y-1) = radius * cos(phase);
//         El(stack[0]->imag, width-x-1, height-y-1) = radius * sin(phase);
//
//         // fill quadrant 3
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, x, height-y-1) = radius * cos(phase); 
//         El(stack[0]->imag, x, height-y-1) = radius * sin(phase);
//
//         // fill quadrant 1
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, width-x-1, y)  = radius * cos(phase);
//         El(stack[0]->imag, width-x-1, y)  = radius * sin(phase);
//      }
//   }
//}

////北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//
//// Produces a pattern that looks like a muslin weave
//void LandScape::fillNormal( float _height )
//{
//   float    radius=0;
//   float    phase;
//   float    scale=1;
//
//   _height=1;
//   for ( int y=0; y<height>>1; y++ )
//   {
//      for ( int x=0; x<width>>1; x++ )
//      {
//         if ( (x&&y) )
//            radius = _height * (1/(x*x)+1/(y*y)) * rand.getNormal();
//
//         // fill quadrant 2
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, x, y) = radius * cos(phase); 
//         El(stack[0]->imag, x, y) = radius * sin(phase);
//
//         // fill quadrant 4
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, width-x-1, height-y-1) = radius * cos(phase);
//         El(stack[0]->imag, width-x-1, height-y-1) = radius * sin(phase);
//
//         // fill quadrant 3
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, x, height-y-1) = radius * cos(phase); 
//         El(stack[0]->imag, x, height-y-1) = radius * sin(phase);
//
//         // fill quadrant 1
//         phase = M_2PI * rand.getFloat();
//         El(stack[0]->real, width-x-1, y)  = radius * cos(phase);
//         El(stack[0]->imag, width-x-1, y)  = radius * sin(phase);
//      }
//   }
//}

