//----------------------------------------------------------------------------
//
//    BitGrid.h
//
//    Square bit grid which relies on invariant that (width == height == a power 
//    of two) for faster region masking.   Idea is derived classes (such as 
//    terrain region mgmt) will provide some more checking, shifting world values 
//    into bit space..
//
//----------------------------------------------------------------------------

#ifndef _BITGRID_H_
#define _BITGRID_H_

#include <types.h>

#ifdef __BORLANDC__
#pragma option -w-inl
#endif


class BitSquare 
{
   UInt8    *data;
   int      bitWidth;
   int      byteWidth;
   int      bitTotal;
   int      rowShift;
   int      dataCount ( void )         { return ( bitTotal >> 3 );  }

   // methods for finding bits.  Y gives row.  
   int      byteOff(int x, int y)   { return ( (y << rowShift) + (x >> 3)  );   }
   int      mask ( int x )          { return (  1 << (x & 7)  );     }
   
   
   // routines to set up loops for masking:  Assumes decent x and y.  The following
   // loopVars params are set up.  first setLoopWH is called to prep/clip w and h, then
   // setupLoop is called.  
   struct loopVars  {
      UInt8    mask1, mask2;
      int      offset, bytecnt;
      int      w, h;
   } l;
   void  setLoopWH ( int x, int y, int w, int h )  {
               l.w = x + w > bitWidth  ?  bitWidth - x  :  w;
               l.h = y + h > bitWidth  ?  bitWidth - y  :  h;   }
   bool  setupLoop ( int x, int y )    {
               if ( (x | y) & ~(bitWidth-1) )      // quick bounds check,
                  return false;                    // relies on size invariants
                  
               l.mask1 = ~ ( mask(x) - 1 );       // 0..01..1
               l.offset = byteOff ( x, y );
               l.bytecnt = ((x + l.w + 7) >> 3) - (x >> 3);
               if ( l.bytecnt > 1 )    {
                  if ( (l.mask2 = mask(x+l.w) - 1) == 0 )
                     l.mask2 = 0xff;
               }
               else    {                     // single byte row
                  UInt16    up = (1 << ((x & 7) + l.w)) - 1;
                  l.mask1 &= UInt8(up);
               }
               return true;
            }


public:
   // must construct with shift value.  Require at least a width of 8 so row lookup
   //       doesn't have negative shift
   BitSquare ( int widthShift = 3 )   {
         if ( widthShift < 3 )  widthShift = 3;
         bitWidth = 1 << widthShift;
         byteWidth = bitWidth >> 3;
         bitTotal = 1 << (widthShift * 2);
         rowShift = widthShift - 3;
         data = new UInt8 [ dataCount() ];
         zero ();
      }
   ~BitSquare ()     {  delete[] data;   }

   bool  set(int x, int y)    {  return (data [ byteOff (x,y) ] |= mask (x));      }
   bool  set ( void )         {  memset ( data, 0xff, dataCount() );  return true; }
   void  zero(int x, int y)   {  data [ byteOff (x,y) ] &= ~mask (x);   }
   void  zero ( void )        {  memset ( data, 0, dataCount() );       }
            
   bool  test ( int x, int y )
            {     
               return ( (data [ byteOff (x,y) ] & mask(x) ) != 0 );
            }
   
   
   // called when l.w and l.h is already set up using setLoopWH()
   bool  do_set_box8 ( const int x, const int y )   {
            if ( setupLoop ( x, y ) ) {
               if ( l.bytecnt == 2 )
                  while ( l.h-- )  { 
                     data [ l.offset + 0 ] |= l.mask1;
                     data [ l.offset + 1 ] |= l.mask2;
                     l.offset += byteWidth;
                  }
               else 
                  while ( l.h-- )   { 
                     data [ l.offset ] |= l.mask1;
                     l.offset += byteWidth;
                  }
               return true;
            }
            return false;
         }
   
            

   //  Function to fill in a box of width no more than 8 (hence at 
   //    most two masks wide).  x and y are assumed within bounds, else 
   //       false returned.  
   bool  setBox8 ( const int x, const int y, int w, const int h )  {
            if ( w > 8 ) 
               w = 8;
            setLoopWH ( x, y, w, h );
            return do_set_box8 ( x, y );
         }

   bool  set ( const int x, const int y, int w, const int h )  {
            setLoopWH ( x, y, w, h );
            if ( l.w <= 8 )  {
               if ( l.w == 1 && l.h == 1 )
                  return set ( x, y );
               else
                  return do_set_box8 ( x, y );
            }
            if ( setupLoop ( x, y ) )   {
               if ( l.bytecnt == 2 )  {   // is at least 2 since w >= 9.
                  while ( l.h-- )  {
                     data [ l.offset + 0 ] |= l.mask1;
                     data [ l.offset + 1 ] |= l.mask2;
                     l.offset += byteWidth;
                  }
               }
               else if ( l.bytecnt > 2 )  {
                  while ( l.h-- )  {
                     data [ l.offset ] |= l.mask1;
                     for ( int i = 1; i < l.bytecnt-1; i++ )
                        data [ l.offset + i ] = 0xff;
                     data [ l.offset + l.bytecnt - 1 ] |= l.mask2;
                     l.offset += byteWidth;
                  }
               }
            }
            return false;
         }


   // Test a box which is a power of two aligned:  the width is a power
   //    of two and X and Y are guaranteed to be multiples of the width.
   bool  testPwr2Sqr ( int x, int y, int pwr )  {
            if ( (x | y) & ~(bitWidth-1) ) 
               return false;
               
            if ( pwr == 0 )      // don't loop on width of 1
               return test ( x, y );

            l.h = 1 << pwr;
            l.offset = byteOff ( x, y );
            l.bytecnt = 1;
            
            if ( pwr < 3 )
               l.mask1 = ( (1<<(1<<pwr)) - 1) << (x&7);
            else
               l.bytecnt <<= (pwr-3);
            
            if ( pwr >= 3 )
               while ( l.h -- )   {
                  for ( int i = 0; i < l.bytecnt; i++ )
                     if ( data [ l.offset + i ] )
                        return true;
                  l.offset += byteWidth;
               }
            else
               while ( l.h -- )  {
                  if ( data [ l.offset ] & l.mask1 )
                     return true;
                  l.offset += byteWidth;
               }
            
            return false;
         }
         
         
#if 0
   bool  test ( int x, int y, int w )   {  return testBox8 ( x, y, w, w );   }
   bool  testBox8 ( const int x, const int y, int w, const int h )   {
            if ( w > 8 ) 
               w = 8;
            setLoopWH ( x, y, w, h );
            if ( setupLoop ( x, y )     {
               if ( l.bytecnt == 2 )
                  while ( l.h-- )  {
                     if ( (data [ l.offset ] & l.mask1)    || 
                              (data [ l.offset+1] & l.mask2)  )
                        return true;
                     l.offset += byteWidth;
                  }
               else 
                  while ( l.h-- )   {
                     if ( data [ l.offset ] & l.mask1 )
                        return true;
                     l.offset += byteWidth;
                  }
            }
            return false;
         }
#endif
         
};



// Do:
// -Convert to UInt32 and have a minimum of 32x32 or, ideally, templatize 
//       off of a supplied word type.  
// -A normal test routine.  
//
// Note:
// -The _ideal_ bit grid - at least for the purposes of testing on power of two 
//    boxes - would probably have some tree structure.  That is, an 8x8 square which
//    is power-of-two aligned could be test()ed against by just scanning 8 _contiguous_
//    bytes.  The power of two square one level up which contains this 8x8 square can
//    be tested by scanning 8 contiguous DWORDs, etc.  
// -Doing the set()ing is a little more involved, but the testing is what needs to be
//    quick in this case.  Plus, filtering down the tree during the set(x,y,w,h) phase
//    can probably be made pretty quick via some tricky checks at each level (relying
//    on the power-of-2 invariant).  



#ifdef __BORLANDC__
#pragma option -winl.
#endif


#endif
