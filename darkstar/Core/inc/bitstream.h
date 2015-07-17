#ifndef _H_BITSTREAM_
#define _H_BITSTREAM_

#include "streamio.h"

class Point3F;
class HuffmanProcessor;

class DLLAPI BitStream : public StreamIO
{
   BYTE *dataPtr;
   int bitNum;
   int bufSize;
   bool error;
   int maxReadBitNum;
   int maxWriteBitNum;

   friend HuffmanProcessor;
public:

   void setBuffer(void *bufPtr, int bufSize, int maxSize = 0);
   BYTE *getBytePtr();

   int  getCurPos() const;
   void setCurPos(const UInt32);

   BitStream(void *bufPtr, int bufSize, int maxSize = 0) { setBuffer(bufPtr, bufSize,maxSize); }
   void clear();
   
   void writeInt(int value, int bitCount);
   int readInt(int bitCount);

   void writeSignedInt(int value, int bitCount);
   int readSignedInt(int bitCount);

   // read and write floats... floats are 0 to 1 inclusive, signed floats are -1 to 1 inclusive

   float readFloat(int bitCount);
   float readSignedFloat(int bitCount);

   void writeFloat(float f, int bitCount);
   void writeSignedFloat(float f, int bitCount);

   // writes a normalized vector
   void writeNormalVector(Point3F *vec, int bitCount);
   void readNormalVector(Point3F *vec, int bitCount);
   
   // writes a normalized vector using alternate method
   void writeNormalVector(Point3F *vec, int angleBitCount, int zBitCount);
   void readNormalVector(Point3F *vec, int angleBitCount, int zBitCount);
   
   void writeBits(int bitCount, const void *bitPtr);
   void readBits(int bitCount, void *bitPtr);
   bool writeFlag(bool val);
   bool readFlag();

   void setBit(int bitCount, bool set);
   bool testBit(int bitCount);

   bool isFull() { return bitNum > (bufSize << 3); }
   bool isValid() { return !error; }

	using StreamIO::write;
	using StreamIO::read;

	Bool    write(int size,const void* d);
	Bool    read (int size,void* d);

   void readString(char stringBuf[256]);
   void writeString(const char *stringBuf, int maxLen=255);
};


//------------------------------------------------------------------------------
//-------------------------------------- INLINES
//
inline int
BitStream::getCurPos() const
{
   return bitNum;
}

inline void
BitStream::setCurPos(const UInt32 in_position)
{
   AssertFatal(in_position < (UInt32)(bufSize << 3), "Out of range bitposition");
   bitNum = in_position;
}

inline bool
BitStream::readFlag()
{
   if(bitNum > maxReadBitNum)
   {
      error = true;
      AssertFatal(false, "Out of range read");
      return false;
   }
   int mask = 1 << (bitNum & 0x7);
   bool ret = (*(dataPtr + (bitNum >> 3)) & mask) != 0;
   bitNum++;
   return ret;
}

#endif