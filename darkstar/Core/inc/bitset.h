//-----------------------------------------------------------------------------
//   This class maintains a bit mask set in an unsigned int.  There are
//   methods for setting, testing and clearing bits.  Two sets of methods
//   exist for accesing bits: bits can be accessed a.set(3) or by using
//   a mask a.set(0x030).  A macro is provided for converting
//   bit numbers into masks, BIT(2) will expand to 0x10 (100).
//
//   $Workfile:   bitset.h  $
//   $Version$
//   $Revision:   1.0  $
//   $Date:   30 Jul 1993 10:43:20  $
//   $log$
//-----------------------------------------------------------------------------

#ifndef _BITSET_H_
#define _BITSET_H_

//Includes
#include <types.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#define  BIT_BYTE_COUNT    8
#define  BIT_INT32_COUNT   (sizeof(UInt32) * BIT_BYTE_COUNT)

#define  BIT(s)            (1 << (s))


//-----------------------------------------------------------------------------

typedef UInt32 BitMask32;
typedef UInt16 BitMask16;

class BitSet32
{
   BitMask32   bset;

public:
   BitSet32()                             { bset = 0; }
   BitSet32(BitMask32 s)                  { bset = s; }

   operator    unsigned int() const       { return bset; }
   BitMask32   operator = (BitMask32 m)   { return bset = m; }
   BitMask32   operator |= (BitMask32 m)  { return bset |= m; }
   BitMask32   operator &= (BitMask32 m)  { return bset &= m; }
   BitMask32   operator ^= (BitMask32 m)  { return bset ^= m; }

   BitMask32   set()                      { return bset = ~0; }
   BitMask32   set(BitMask32 s)           { return bset |= s; }
   BitMask32   set(BitMask32 s, bool b)   { return bset = (bset&~s)|(b?s:0); }
   BitMask32   clear()                    { return bset = 0; }
   BitMask32   clear(BitMask32 s)         { return bset &= ~s; }
   UInt32      test(BitMask32 s) const    { return (bset & s) != 0; }
   BitMask32   toggle(BitMask32 s)        { return bset ^= s; }

   BitMask32   mask() const               { return bset; }
};


//-----------------------------------------------------------------------------


class BitSet16
{
   BitMask16   bset;

public:
   BitSet16()                             { bset = 0; }
   BitSet16(BitMask16 s)                  { bset = s; }

   operator    unsigned int() const       { return bset; }
   BitMask16   operator = (BitMask16 m)   { return bset = m; }

   BitMask16   set()                      { return bset = ~0; }
   BitMask16   set(BitMask16 s)           { return bset |= s; }
   BitMask16   clear()                    { return bset = 0; }
   BitMask16   clear(BitMask16 s)         { return bset &= BitMask16(~s); }
   UInt16      test(BitMask16 s) const    { return bset & s; }

   BitMask16   mask() const               { return bset; }
};


//-----------------------------------------------------------------------------

// Implement a packed array of numbers of bit length 1, 2, 4,.. (sticking to powers of 
// two for simplicity).  The bit width is specified with its log (0->1, 1->2, 2->4).  
template <size_t LogOfBitSize>
class BitSetX
{
 protected:
   UInt8   * vec;
   
 public:
   // enum{ 
         // Wanted to templatize off of Word type too, but can't do this:
         //    TrickyLimitedLogarithm = (sizeof(UnsignedInt) >> 1) + 3,
         //    DownShift = TrickyLimitedLogarithm - LogOfBitSize, 
         
   // OK, earlier BCC versions don't like these, try it longhand... 
   // NumBits = 1 << LogOfBitSize,  
   // Mask = (1 << NumBits) - 1,    
   // DownShift = 3 - LogOfBitSize, 
   // ShiftMask = (1 << DownShift) - 1, 
   // };
   void init(UInt16 * array)  { vec=reinterpret_cast<UInt8 *>(array);   }
   BitSetX(UInt16 * array)    { init(array); }
   BitSetX() {}
      
   UInt16 operator[]( int i ){        // Want this one to be efficient.  
      // return (vec[i>>DownShift] >> ((i & ShiftMask)<<LogOfBitSize)) & Mask;
      return (vec[i>>(3-LogOfBitSize)] >> ((i & ((1<<3-LogOfBitSize)-1))<<LogOfBitSize)) & ((1<<(1<<LogOfBitSize))-1);
   }
   void set ( int i, int value ){
      // int      upShift = (i & ShiftMask) << LogOfBitSize;
      // int      index = i >> DownShift;
      // UInt16   shiftedMask = Mask << upShift;
      // vec[index] &= ~shiftedMask;
      // vec[index] |= ((value << upShift) & shiftedMask);
      
      int      upShift = (i & ((1<<3-LogOfBitSize)-1)) << LogOfBitSize;
      int      index = i >> (3-LogOfBitSize);
      UInt16   shiftedMask = ((1<<(1<<LogOfBitSize))-1) << upShift;
      vec[index] &= ~shiftedMask;
      vec[index] |= ((value << upShift) & shiftedMask);
      
   }
};

class BitSetN 
{
   BitSetX<0>     single;     // unions don't work (ctor is ambiguous), but this 
   BitSetX<1>     bitpair;    // class is generally used in an autoclass (temporary) 
   BitSetX<2>     nybble;     // setting anyway and the slight space probably isn't 
   UInt8          * bytes;    // an issue.  Also: non-power-of-2 bit lengths might
   int            bitCount;   // be done using a combination of these.  
   
 public:
   BitSetN( int bcount, UInt16 * array );
   void set(int index, int value);
   void decompress ( UInt16 * dst, UInt16 lookup[], int N, int start=0 );
};


template <size_t numFlags>
class BitFlagArray : public BitSetX<0>
{
   UInt8    data[ (numFlags + 7) >> 3 ];
   
 public:
   BitFlagArray()
   {
      vec = data;
      clear();
   }
   void clear()
   {
      memset(data, 0, sizeof(data) );
   }
};

   

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_BITSET_H_
