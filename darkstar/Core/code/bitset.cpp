//
//    BITSET.CPP
// 
// Simple classes for managing an array of 1-, 2-, or 4- bit values, packed into 
// short integers.  Implemented for the high resolution light map compression, and 
// is oriented around fast decompression.  
//
// The basic BitSetX<> is templatized so the decompression can be reasonably fast,
// then the BitSetN wraps and switches.  Additional bit sizes _could_ be implemented
// efficiently by using two or more arrays.  i.e. a 5-bit values could be packed
// packed using the BitSetX<0> and BitSetX<2> and the unpack would avoid any modulo
// or divide arithmetic or non-word-aligned fetches.  
// 



#include "bitset.h"


BitSetN::BitSetN( int bcount, UInt16 * array )
{
   switch( bitCount = bcount )
   {
      case  1:
         single.init( array );
         break;
      case  2:
         bitpair.init( array );
         break;
      case  4:
         nybble.init( array );
         break;
      case  8:
         bytes = reinterpret_cast<UInt8 *>(array);
         break;
   }
}
      
void BitSetN::set(int index, int value)
{
   switch( bitCount )
   {
      case  1:
         single.set(index, value);
         break;
      case  2:
         bitpair.set(index, value);
         break;
      case  4:
         nybble.set(index, value);
         break;
      case  8:
         bytes[index] = value;
         break;
   }
}

// Decompress an array of values using the supplied lookup table.  This just iterates
// through our bit packed array and uses the values as lookups from the supplied
// lookup table.  We switch OUTSIDE of the loop, since this is our main bottleneck.
void BitSetN::decompress ( UInt16 * dst, UInt16 lookup[], int N, int start )
{
   register int i;
   N += start;
   switch( bitCount )
   {
      case  1:
         for( i = start; i < N; i++ )
            *dst++ = lookup[ single[i] ];
         break;
      case  2:
         for( i = start; i < N; i++ )
            *dst++ = lookup[ bitpair[i] ];
         break;
      case  4:
         for( i = start; i < N; i++ )
            *dst++ = lookup[ nybble[i] ];
         break;
      case  8:
         N += start;
         for( i = start; i < N; i++ )
            *dst++ = lookup[ bytes[i] ];
         break;
   }
}

