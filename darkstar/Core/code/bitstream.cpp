#include "types.h"
#include "bitstream.h"
#include "m_trig.h"
#include "tVector.h"
#include <ctype.h>

// dummy class for writing vectors
class Point3F
{
public:
   float x, y, z;
};

extern "C"
{
	float    __cdecl m_sqrt_RealF( const float a);
}


class HuffmanProcessor
{
   static const UInt32 csm_charFreqs[256];
   bool   m_tablesBuilt;

   void buildTables();

   struct HuffNode {
      UInt32 pop;

      Int16  index0;
      Int16  index1;
   };
   struct HuffLeaf {
      UInt32 pop;

      UInt8   numBits;
      UInt8   symbol;
      UInt32  code;   // no code should be longer than 32 bits.
   };
   // We have to be a bit careful with these, since they are pointers...
   struct HuffWrap {
      HuffNode* pNode;
      HuffLeaf* pLeaf;

     public:
      HuffWrap() : pNode(NULL), pLeaf(NULL) { }

      void set(HuffLeaf* in_leaf) { pNode = NULL; pLeaf = in_leaf; }
      void set(HuffNode* in_node) { pLeaf = NULL; pNode = in_node; }

      UInt32 getPop() { if (pNode) return pNode->pop; else return pLeaf->pop; }
   };

   Vector<HuffNode> m_huffNodes;
   Vector<HuffLeaf> m_huffLeaves;

   Int16 determineIndex(HuffWrap&);

   void generateCodes(BitStream&, Int32, Int32);

  public:
   HuffmanProcessor() : m_tablesBuilt(false) { }
   ~HuffmanProcessor();

   static HuffmanProcessor g_huffProcessor;

   bool readHuffBuffer(BitStream* pStream, char* out_pBuffer);
   bool writeHuffBuffer(BitStream* pStream, const char* out_pBuffer, int maxLen);
};

HuffmanProcessor HuffmanProcessor::g_huffProcessor;

HuffmanProcessor::~HuffmanProcessor()
{
#ifdef _MSC_VER
   int hc = _heapchk();
   AssertFatal((hc == _HEAPOK) || (hc == _HEAPEMPTY), "Heap error.");
#else
   int hc = heapcheck();
   AssertFatal(hc != _HEAPCORRUPT, "Heap error.");
#endif
}

void BitStream::setBuffer(void *bufPtr, int size, int maxSize)
{
   dataPtr = (BYTE *) bufPtr;
   bitNum = 0;
   bufSize = size;
   maxReadBitNum = size << 3;
   if(maxSize == 0)
      maxSize = size;
   maxWriteBitNum = maxSize << 3;
   error = false;
}

BYTE *BitStream::getBytePtr()
{
   return dataPtr + ((bitNum + 7) >> 3);
}

void BitStream::clear()
{
   memset(dataPtr, 0, bufSize);
}

void BitStream::writeBits(int bitCount, const void *bitPtr)
{
   if(!bitCount)
      return;

   if(bitCount + bitNum > maxWriteBitNum)
   {
      error = true;
      AssertFatal(false, "Out of range write");
      return;
   }
   const BYTE *ptr = (BYTE *) bitPtr;
   BYTE *stPtr = dataPtr + (bitNum >> 3);
   BYTE *endPtr = dataPtr + ((bitCount + bitNum - 1) >> 3);

   int upShift = bitNum & 0x7;
   int downShift = 8 - upShift;
   BYTE lastMask = 0xFF >> (7 - ((bitNum + bitCount - 1) & 0x7));
   BYTE startMask = 0xFF >> downShift;

   BYTE curB = *ptr++;
   *stPtr = (curB << upShift) | (*stPtr & startMask);

   stPtr++;
   while(stPtr <= endPtr)
   {
      BYTE nextB = *ptr++;
      *stPtr++ = (curB >> downShift) | (nextB << upShift);
      curB = nextB;
   }
   *endPtr &= lastMask;
   
   bitNum += bitCount;
}

void BitStream::setBit(int bitCount, bool set)
{
   if(set)
      *(dataPtr + (bitCount >> 3)) |= (1 << (bitCount & 0x7));
   else
      *(dataPtr + (bitCount >> 3)) &= ~(1 << (bitCount & 0x7));
}

bool BitStream::testBit(int bitCount)
{
   return (*(dataPtr + (bitCount >> 3)) & (1 << (bitCount & 0x7))) != 0;
}

bool BitStream::writeFlag(bool val)
{
   if(bitNum + 1 > maxWriteBitNum)
   {
      error = true;
      AssertFatal(false, "Out of range write");
      return false;
   }
   if(val)
      *(dataPtr + (bitNum >> 3)) |= (1 << (bitNum & 0x7));
   else
      *(dataPtr + (bitNum >> 3)) &= ~(1 << (bitNum & 0x7));
   bitNum++;
   return (val);
}

void BitStream::readBits(int bitCount, void *bitPtr)
{
   if(!bitCount)
      return;
   if(bitCount + bitNum > maxReadBitNum)
   {
      error = true;
      AssertFatal(false, "Out of range read");
      return;
   }
   BYTE *stPtr = dataPtr + (bitNum >> 3);
   int byteCount = (bitCount + 7) >> 3;

   BYTE *ptr = (BYTE *) bitPtr;

   int downShift = bitNum & 0x7;
   int upShift = 8 - downShift;

   BYTE curB = *stPtr;
   while(byteCount--)
   {
      BYTE nextB = *++stPtr;
      *ptr++ = (curB >> downShift) | (nextB << upShift);
      curB = nextB;
   }

   bitNum += bitCount;
}

bool BitStream::read(int size, void *dataPtr)
{
   readBits(size << 3, dataPtr);
   return true;
}

bool BitStream::write(int size, const void *dataPtr)
{
   writeBits(size << 3, dataPtr);
   return true;
}

int BitStream::readInt(int bitCount)
{
   int ret = 0;
   readBits(bitCount, &ret);
   if(bitCount == 32)
      return ret;
   else
      ret &= (1 << bitCount) - 1;
   return ret;
}

void BitStream::writeInt(int val, int bitCount)
{
   writeBits(bitCount, &val);
}

void BitStream::writeFloat(float f, int bitCount)
{
   writeInt(f * ((1 << bitCount) - 1), bitCount);
}

float BitStream::readFloat(int bitCount)
{
   return readInt(bitCount) / float((1 << bitCount) - 1);
}

void BitStream::writeSignedFloat(float f, int bitCount)
{
   writeInt( ((f + 1) * .5) * ((1 << bitCount) - 1), bitCount);
}

float BitStream::readSignedFloat(int bitCount)
{
   return readInt(bitCount) * 2 / float((1 << bitCount) - 1) - 1.0f;
}

void BitStream::writeSignedInt(int value, int bitCount)
{
   if(writeFlag(value < 0))
      writeInt(-value, bitCount - 1);
   else
      writeInt(value, bitCount - 1);
}

int BitStream::readSignedInt(int bitCount)
{
   if(readFlag())
      return -readInt(bitCount - 1);
   else
      return readInt(bitCount - 1);
}

void BitStream::writeNormalVector(Point3F *vec, int bitCount)
{
   writeSignedFloat(vec->x, bitCount);
   writeSignedFloat(vec->y, bitCount);
   writeFlag(vec->z < 0);
}

void BitStream::readNormalVector(Point3F *vec, int bitCount)
{
   vec->x = readSignedFloat(bitCount);
   vec->y = readSignedFloat(bitCount);
   float zsquared = 1 - (vec->x * vec->x) - (vec->y * vec->y);
   if(zsquared < 0)
   {
      vec->z = 0;
      readFlag();
      return;
   }
   vec->z = m_sqrt_RealF(zsquared);
   if(readFlag())
      vec->z = -vec->z;
}

#define M_2PI       (2*3.14159265358979323846)

void BitStream::writeNormalVector(Point3F *vec, int angleBitCount, int zBitCount)
{
   writeSignedFloat( vec->z, zBitCount );
   
   // don't need to write x and y if they are both zero, which we can assess
   // by checking for |z| == 1
   if(!IsEqual(abs(vec->z), 1.0f))
   {
      writeSignedFloat( m_atan(vec->x,vec->y) / M_2PI, angleBitCount );
   }
}

void BitStream::readNormalVector(Point3F * vec, int angleBitCount, int zBitCount)
{
   vec->z = readSignedFloat(zBitCount);
   
   // check to see if |z| == 1.  If so, then we don't read x and y (they're zero)
   if(!IsEqual(abs(vec->z), 1.0f))
   {
      float angle = M_2PI * readSignedFloat(angleBitCount);
   
      float mult = m_sqrt_RealF(1.0f - vec->z * vec->z);
      vec->x = mult * cos(angle);
      vec->y = mult * sin(angle);
   }
   else
   {
      // no need to read, we know
      vec->x = 0.0f;
      vec->y = 0.0f;
   }
}

void
BitStream::readString(char buf[256])
{
   HuffmanProcessor::g_huffProcessor.readHuffBuffer(this, buf);
}

void
BitStream::writeString(const char *string, int maxLen)
{
   HuffmanProcessor::g_huffProcessor.writeHuffBuffer(this, string, maxLen);
}

const static UInt32 csg_probBoost = 1;

void
HuffmanProcessor::buildTables()
{
   AssertFatal(m_tablesBuilt == false, "Cannot build tables twice!");
   m_tablesBuilt = true;

   Int32 i;

   // First, construct the array of wraps...
   //
   m_huffLeaves.setSize(256);
   m_huffNodes.reserve(256);
   m_huffNodes.increment();
   for (i = 0; i < 256; i++) {
      HuffLeaf& rLeaf = m_huffLeaves[i];

      rLeaf.pop    = csm_charFreqs[i] + (isalnum(i) ? csg_probBoost : 0) + csg_probBoost;
      rLeaf.symbol = UInt8(i);

      memset(&rLeaf.code, 0, sizeof(rLeaf.code));
      rLeaf.numBits = 0;
   }

   Int32 currWraps = 256;
   HuffWrap* pWrap = new HuffWrap[256];
   for (i = 0; i < 256; i++) {
      pWrap[i].set(&m_huffLeaves[i]);
   }

   while (currWraps != 1) {
      UInt32 min1 = 0xfffffffe, min2 = 0xffffffff;
      Int32 index1 = -1, index2 = -1;

      for (i = 0; i < currWraps; i++) {
         if (pWrap[i].getPop() < min1) {
            min2   = min1;
            index2 = index1;

            min1   = pWrap[i].getPop();
            index1 = i;
         } else if (pWrap[i].getPop() < min2) {
            min2   = pWrap[i].getPop();
            index2 = i;
         }
      }
      AssertFatal(index1 != -1 && index2 != -1 && index1 != index2, "hrph");
      
      // Create a node for this...
      m_huffNodes.increment();
      HuffNode& rNode = m_huffNodes.last();
      rNode.pop    = pWrap[index1].getPop() + pWrap[index2].getPop();
      rNode.index0 = determineIndex(pWrap[index1]);
      rNode.index1 = determineIndex(pWrap[index2]);

      Int32 mergeIndex = index1 > index2 ? index2 : index1;
      Int32 nukeIndex  = index1 > index2 ? index1 : index2;
      pWrap[mergeIndex].set(&rNode);

      if (index2 != (currWraps - 1)) {
         pWrap[nukeIndex] = pWrap[currWraps - 1];
      }
      currWraps--;
   }
   AssertFatal(currWraps == 1, "wrong wraps?");
   AssertFatal(pWrap[0].pNode != NULL && pWrap[0].pLeaf == NULL, "Wrong wrap type!");

   // Ok, now we have one wrap, which is a node.  we need to make sure that this
   //  is the first node in the node list.
   m_huffNodes[0] = *(pWrap[0].pNode);
   delete [] pWrap;

   UInt32 code = 0;
   BitStream bs(&code, 4);

   generateCodes(bs, 0, 0);
}

void
HuffmanProcessor::generateCodes(BitStream& rBS,
                                Int32      index,
                                Int32      depth)
{
   if (index < 0) {
      // leaf node, copy the code in, and back out...
      HuffLeaf& rLeaf = m_huffLeaves[-(index + 1)];

      memcpy(&rLeaf.code, rBS.dataPtr, sizeof(rLeaf.code));
      rLeaf.numBits = depth;
   } else {
      HuffNode& rNode = m_huffNodes[index];

      int pos = rBS.getCurPos();

      rBS.writeFlag(false);
      generateCodes(rBS, rNode.index0, depth + 1);

      rBS.setCurPos(pos);
      rBS.writeFlag(true);
      generateCodes(rBS, rNode.index1, depth + 1);

      rBS.setCurPos(pos);      
   }
}

Int16
HuffmanProcessor::determineIndex(HuffWrap& rWrap)
{
   if (rWrap.pLeaf != NULL) {
      AssertFatal(rWrap.pNode == NULL, "um, never.");

      return -((rWrap.pLeaf - m_huffLeaves.address()) + 1);
   } else {
      AssertFatal(rWrap.pNode != NULL, "um, never.");

      return rWrap.pNode - m_huffNodes.address();
   }
}

bool
HuffmanProcessor::readHuffBuffer(BitStream* pStream,
                                 char*      out_pBuffer)
{
   if (m_tablesBuilt == false)
      buildTables();

   if (pStream->readFlag()) {
      Int32 len = pStream->readInt(8);
      for (int i = 0; i < len; i++) {
         Int32 index = 0;
         while (true) {
            if (index >= 0) {
               if (pStream->readFlag() == true) {
                  index = m_huffNodes[index].index1;
               } else {
                  index = m_huffNodes[index].index0;
               }
            } else {
               out_pBuffer[i] = m_huffLeaves[-(index+1)].symbol;
               break;
            }
         }
      }
      out_pBuffer[len] = '\0';
      return true;
   } else {
      // Uncompressed string...
      UInt32 len = pStream->readInt(8);
      pStream->read(len, out_pBuffer);
      out_pBuffer[len] = '\0';
      return true;
   }
}

bool
HuffmanProcessor::writeHuffBuffer(BitStream* pStream, const char* out_pBuffer, int maxLen)
{
   if (m_tablesBuilt == false)
      buildTables();

   Int32 len = out_pBuffer ? strlen(out_pBuffer) : 0;
   AssertWarn(len <= 255, "String TOO long for writeString");
   AssertWarn(len <= 255, out_pBuffer);
   if (len > maxLen)
      len = maxLen;

   Int32 numBits = 0;
   Int32 i;
   for (i = 0; i < len; i++)
      numBits += m_huffLeaves[(unsigned char)out_pBuffer[i]].numBits;

   if (numBits >= (len * 8)) {
      pStream->writeFlag(false);
      pStream->writeInt(len, 8);
      pStream->write(len, out_pBuffer);
   } else {
      pStream->writeFlag(true);
      pStream->writeInt(len, 8);
      for (i = 0; i < len; i++) {
         HuffLeaf& rLeaf = m_huffLeaves[((unsigned char)out_pBuffer[i])];
         pStream->writeBits(rLeaf.numBits, &rLeaf.code);
      }
   }

   return true;
}

const UInt32 HuffmanProcessor::csm_charFreqs[256] = {
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
329   ,
21    ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
2809  ,
68    ,
0     ,
27    ,
0     ,
58    ,
3     ,
62    ,
4     ,
7     ,
0     ,
0     ,
15    ,
65    ,
554   ,
3     ,
394   ,
404   ,
189   ,
117   ,
30    ,
51    ,
27    ,
15    ,
34    ,
32    ,
80    ,
1     ,
142   ,
3     ,
142   ,
39    ,
0     ,
144   ,
125   ,
44    ,
122   ,
275   ,
70    ,
135   ,
61    ,
127   ,
8     ,
12    ,
113   ,
246   ,
122   ,
36    ,
185   ,
1     ,
149   ,
309   ,
335   ,
12    ,
11    ,
14    ,
54    ,
151   ,
0     ,
0     ,
2     ,
0     ,
0     ,
211   ,
0     ,
2090  ,
344   ,
736   ,
993   ,
2872  ,
701   ,
605   ,
646   ,
1552  ,
328   ,
305   ,
1240  ,
735   ,
1533  ,
1713  ,
562   ,
3     ,
1775  ,
1149  ,
1469  ,
979   ,
407   ,
553   ,
59    ,
279   ,
31    ,
0     ,
0     ,
0     ,
68    ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     ,
0     
};
