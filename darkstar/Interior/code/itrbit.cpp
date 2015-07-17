//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include "itrbit.h"


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

UInt8* ITRBitVector::clear(int count)
{
	count = (count >> 3) + 1;
	if (count > size())
		setSize(count);
	memset(begin(),0,count);
	return begin();
}


//----------------------------------------------------------------------------

void ITRBitVector::set(int index)
{
	AssertFatal(index >= 0,"ITRBitVector::set: Negative index value");
	int byte = index >> 3;
	int bit = 0x80 >> (index & 0x07);
	if (byte >= size()) {
		int osize = size();
		setSize(byte + 1);
		memset(&(*this)[osize],0,size() - osize);
	}
	(*this)[byte] |= bit;
}


//----------------------------------------------------------------------------

void ITRBitVector::set(ITRBitVector& bv)
{
	if (size() < bv.size()) {
		int osize = size();
		setSize(bv.size());
		memset(&(*this)[osize],0,size() - osize);
	}
	for (int i = 0; i < bv.size(); i++)
		(*this)[i] |= bv[i];
}


//----------------------------------------------------------------------------

int ITRBitVector::compress(Vector<UInt8>* list)
{
	// Remove duplicate zeros.
	int startSize = list->size();
	for (Parent::iterator itr = begin(); itr != end(); )
		if (!*itr) {
			int count = 1;
			for (itr++; !*itr && itr != end() && count < 255; )
				count++, itr++;
			list->push_back(0);
			list->push_back(count);
		}
		else
			list->push_back(*itr++);
	return list->size() - startSize;
}


//----------------------------------------------------------------------------

void ITRBitVector::uncompress(UInt8* src,int count)
{
	UInt8* dst = begin();
	for (UInt8* end = src + count; src != end; )
		if (!*src) {
			dst += src[1];
			src += 2;
		}
		else
			*dst++ |= *src++;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

bool ITRBitVector::iterator::next()
{
	if (!(bits & 0x7f)) {
		UInt8* start = pos;
		while (!*pos && pos < end)
			pos++;
		if (pos >= end)
			return false;
		bitCount = 0;
		byteCount += 8 + (pos - start) * 8;
		bits = *pos++;
		if (bits & 0x80)
			return true;
	}
	do
		bits <<= 1, bitCount++;
	while (!(bits & 0x80));
	return true;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

bool ITRCompressedBitVector::iterator::next()
{
	if (!(bits & 0x7f)) {
		if (pos < end)
			while (!*pos) {
				byteCount += pos[1] * 8;
				pos += 2;
			}
		if (pos >= end)
			return false;
		byteCount += 8;
		bitCount = 0;
		bits = *pos++;
		if (bits & 0x80)
			return true;
	}
	do
		bits <<= 1, bitCount++;
	while (!(bits & 0x80));
	return true;
}

