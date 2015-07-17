//----------------------------------------------------------------------------
//
// bitVector.h
//
//----------------------------------------------------------------------------

#ifndef _TBITVECTOR_H_
#define _TBITVECTOR_H_

#include <types.h>
#include <tvector.h>

#ifdef __BORLANDC__
#pragma option -w-inl
#endif


//----------------------------------------------------------------------------

//
// Bits in the bit vector are numberered 0 to (n-1)
//

class BitVector : private Vector<unsigned int>
{
	typedef Vector<unsigned int> Parent;

public:
	BitVector(): Vector<unsigned int>(1) {}
	BitVector(int s): Vector<unsigned int>(1) { setSize(s); }
	unsigned int* begin() { return Parent::begin(); }
	unsigned int* end()   { return Parent::end(); }
	int  size()    { return Parent::size(); }
	void setSize(int s);

   void zero();
	void zero(int bit);
	void zero(int a, int b);

	void set();
	void set(int bit);
	void set(int a, int b);

	bool test();
	bool test(int bit);

	void not();
	void and(BitVector &bv);
	void or(BitVector &bv);
   void xor(BitVector &bv);

};


//----------------------------------------------------------------------------

inline void BitVector::setSize(int s)
{
   s = (s+31)&~31;   // round up to nearest DWORD bits
   Parent::setSize(s >> 5);
}

inline bool BitVector::test()
{
   for (iterator i=begin(); i != end(); i++)
      if (*i) return (true);
   return (false);
}

inline bool BitVector::test(int bit)
{
	return (begin()[bit >> 5] & (0x80000000 >> (bit & 0x01f))) != 0;
}


//----------------------------------------------------------------------------
inline void BitVector::zero()
{
   memset(begin(), 0, size()*sizeof(unsigned int));
}

inline void BitVector::zero(int bit)
{
   begin()[bit >> 5] &= ~(0x80000000 >> (bit & 0x01f));
}

inline void BitVector::zero(int a, int b)
{
   unsigned int mask;
   if (a>b) SWAP(a,b);
   if ((a>>5)!=(b>>5))
   {
      for (int i=(a>>5)+1; i< (b>>5); i++)
         begin()[i] = 0;

      //clear start
      mask = 0xffffffff >> (a & 0x01f);
      begin()[a >> 5] &= ~mask;

      //clear tail
      mask = 0xffffffff << (31 - (b & 0x1f));
      begin()[b >> 5] &= ~mask;
   }
   else
   {
      mask = 0xffffffff << (31-(b-a));
      mask >>= (a & 0x1f);
      begin()[a >> 5] &= ~mask;
   }
}   


//----------------------------------------------------------------------------
inline void BitVector::set()
{
   memset(begin(), 0xff, size()*sizeof(unsigned int));
}

inline void BitVector::set(int bit)
{
   begin()[bit >> 5] |= (0x80000000 >> (bit & 0x01f));
}

inline void BitVector::set(int a, int b)
{
   unsigned int mask;
   if (a>b) SWAP(a,b);
   if ((a>>5)!=(b>>5))
   {
      for (int i=(a>>5)+1; i< (b>>5); i++)
         begin()[i] = 0xffffffff;

      //set start
      mask = 0xffffffff >> (a & 0x01f);
      begin()[a >> 5] |= mask;

      //set tail
      mask = 0xffffffff << (31 - (b & 0x1f));
      begin()[b >> 5] |= mask;
   }
   else
   {
      mask = 0xffffffff << (31-(b-a));
      mask >>= (a & 0x1f);
      begin()[a >> 5] |= mask;
   }
}   


//------------------------------------------------------------------------------
inline void BitVector::not()
{
   iterator i = begin();
   for (; i != end(); i++)
      *i = ~(*i);
}   

inline void BitVector::and(BitVector &bv)
{
   iterator a = begin();
   iterator b = bv.begin();
   for (; a != end() && b != bv.end(); a++,b++)
      *a &= *b;
}   

inline void BitVector::or(BitVector &bv)
{
   iterator a = begin();
   iterator b = bv.begin();
   for (; a != end() && b != bv.end(); a++,b++)
      *a |= *b;
}   

inline void BitVector::xor(BitVector &bv)
{
   iterator a = begin();
   iterator b = bv.begin();
   for (; a != end() && b != bv.end(); a++,b++)
      *a ^= *b;
}  

#ifdef __BORLANDC__
#pragma option -winl.
#endif


//----------------------------------------------------------------------------
#endif
