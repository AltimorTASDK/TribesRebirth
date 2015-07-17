//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _ITRBIT_H_
#define _ITRBIT_H_

#include <types.h>
#include <tvector.h>

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

class ITRBitVector: private Vector<UInt8>
{
	typedef Vector<UInt8> Parent;

public:
	class iterator {
	protected:
		UInt8 *pos,*end;
		UInt8 bits;
		int byteCount;
		int bitCount;
	public:
		iterator();
		iterator(ITRBitVector& bv);
		void clear();
		bool next();
		int operator*();
		bool operator++();
	};

	ITRBitVector() {}
	UInt8* begin() { return Parent::begin(); }
	UInt8* end()   { return Parent::end(); }
	int size()     { return Parent::size(); }
	void setSize(int s) { Parent::setSize(s); }
	void clear()   { Parent::clear(); }

	//
	void reserve(int numBits) { Parent::reserve((numBits >> 3) + 1); }
	UInt8* clear(int count);
	void set(int bit);
	void clr(int bit);
	void set(ITRBitVector& bv);
	bool ftest(int bit);
	bool test(int bit);
	bool test(int index,int bit);
	int compress(Vector<UInt8>* list);
	void uncompress(UInt8*,int count);
};


//----------------------------------------------------------------------------

inline bool ITRBitVector::ftest(int bit)
{
	return begin()[bit >> 3] & (0x80 >> (bit & 0x07));
}

inline bool ITRBitVector::test(int bit)
{
	int index = bit >> 3;
	if (index > size() - 1)
		return false;
	return begin()[index] & (0x80 >> (bit & 0x07));
}

inline bool ITRBitVector::test(int index, int bit)
{
	if (index > size() - 1)
		return false;
	return begin()[index] & bit;
}

inline void ITRBitVector::clr(int bit)
{
	int index = bit >> 3;
	if (index < size())
		begin()[index] &= ~(0x80 >> (bit & 0x07));
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

inline ITRBitVector::iterator::iterator()
{
}	

inline ITRBitVector::iterator::iterator(ITRBitVector& bv)
{
	pos = bv.begin();
	end = bv.end();
	bits = 0;
	byteCount = -8;
	bitCount = 0;
}

inline void ITRBitVector::iterator::clear()
{
	AssertFatal(byteCount >= 0,
		"ITRBitVector::iterator::clear: must call ++ first");
	*(pos - 1) &= ~(0x80 >> bitCount);
}

inline int ITRBitVector::iterator::operator*()
{
	AssertFatal(byteCount >= 0,
		"ITRBitVector::iterator::operator*: must call ++ first");
	return byteCount + bitCount;
}

inline bool ITRBitVector::iterator::operator++()
{
	return next();
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

class ITRCompressedBitVector
{
public:
	class iterator: public ITRBitVector::iterator {
		typedef ITRBitVector::iterator Parent;
	public:
		iterator(UInt8* start,UInt8* end);
		bool operator++();
		bool next();
	};
};

inline ITRCompressedBitVector::iterator::iterator(UInt8* sp,UInt8* ep)
{
	pos = sp;
	end = ep;
	bits = 0;
	byteCount = -8;
	bitCount = 0;
}

inline bool ITRCompressedBitVector::iterator::operator++()
{
	return next();
}


#endif
