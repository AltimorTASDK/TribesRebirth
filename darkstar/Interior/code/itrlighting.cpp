//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)

#pragma option -Jg
#endif

#include <talgorithm.h>
#include "itrlighting.h"
#include "crc16.h"

#define FileVersion  7


const UInt32 ITRLighting::csm_huffOptShift      = 10;
const UInt32 ITRLighting::csm_numHuffOptEntries = (1 << 10);

//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT(ITRLighting);


ITRLighting::ITRLighting()
 : m_huffmanCompressed(false)
{
   lightMapListSize = 0;
}


ITRLighting::~ITRLighting()
{
   //
}


//----------------------------------------------------------------------------
void ITRLighting::unpackColor( UInt16 color, float & red, float & green, float & blue, float & mono )
{
   int b = ( color & 0xf ); color >>= 4;
   int g = ( color & 0xf ); color >>= 4;
   int r = ( color & 0xf ); color >>= 4;
   int i = ( color & 0xf );
   
   red   = float( r / 15.0f );
   green = float( g / 15.0f );
   blue  = float ( b / 15.0f );
   mono  = float( i / 15.0f );   
}

//----------------------------------------------------------------------------

UInt16 ITRLighting::buildColor(float red,float green,float blue)
{
   int xr = (red < .0f)? 0:
      (red > 1.0f)? 15: int(red * 15) & 0xf;
   int xg = (green < .0f)? 0:
      (green > 1.0f)? 15: int(green * 15) & 0xf;
   int xb = (blue < .0f)? 0:
      (blue > 1.0f)? 15: int(blue * 15) & 0xf;
   float mono = red * 0.3f + green * 0.6f + blue * 0.1f;
   int xi = (mono < .0f)? 0:
      (mono > 1.0f)? 15: int(mono * 15) & 0xf;
   return (xi << 12) | (xr << 8) | (xg << 4) | xb;
}


//----------------------------------------------------------------------------

UInt16 ITRLighting::buildColorWOMono(float red,float green,float blue)
{
   int xr = (red < .0f)? 0:
      (red > 1.0f)? 15: int(red * 15) & 0xf;
   int xg = (green < .0f)? 0:
      (green > 1.0f)? 15: int(green * 15) & 0xf;
   int xb = (blue < .0f)? 0:
      (blue > 1.0f)? 15: int(blue * 15) & 0xf;
   return (xr << 8) | (xg << 4) | xb;
}


//----------------------------------------------------------------------------

UInt16 ITRLighting::buildColor(float red,float green,float blue,float mono)
{
   int xr = (red < .0f)? 0:
      (red > 1.0f)? 15: int(red * 15) & 0xf;
   int xg = (green < .0f)? 0:
      (green > 1.0f)? 15: int(green * 15) & 0xf;
   int xb = (blue < .0f)? 0:
      (blue > 1.0f)? 15: int(blue * 15) & 0xf;
   int xi = (mono < .0f)? 0:
      (mono > 1.0f)? 15: int(mono * 15) & 0xf;
   return (xi << 12) | (xr << 8) | (xg << 4) | xb;
}


//----------------------------------------------------------------------------

void ITRLighting::changeRGB(UInt16 *dst, float red,float green,float blue)
{
   int mono_comp = (*dst) & 0xf000;

   int xr = (red < .0f)? 0:
      (red > 1.0f)? 15: int(red * 15) & 0xf;
   int xg = (green < .0f)? 0:
      (green > 1.0f)? 15: int(green * 15) & 0xf;
   int xb = (blue < .0f)? 0:
      (blue > 1.0f)? 15: int(blue * 15) & 0xf;

   *dst = ((mono_comp) | (xr << 8) | (xg << 4) | xb);
}

//----------------------------------------------------------------------------

inline bool lightCompare(const ITRLighting::Light& a,
             const ITRLighting::Light& b)
{
   return a.id < b.id;
}  

int 
ITRLighting::findLight(int lightId)
{
   // Search lightList for light.  The light list is assumed
   // to have been sorted by ID.
   Light key;
   key.id = lightId;
   LightList::iterator itr = 
      binary_search(lightList.begin(), lightList.end(),
                    key, lightCompare);
   if (itr != lightList.end())
      return itr - lightList.begin();
   return -1;
}

int ITRLighting::findLight(const char* in_pLightName)
{
   for (int i = 0; i < lightList.size(); i++) {
      if (lightList[i].nameIndex != -1) {
         if (!strcmp(in_pLightName, &nameBuffer[lightList[i].nameIndex]))
            return i;
      }
   }

   return -1;
}

int ITRLighting::findLightId(const char* in_pLightName)
{
   for (int i = 0; i < lightList.size(); i++) {
      if (lightList[i].nameIndex != -1) {
         if (!strcmp(in_pLightName, &nameBuffer[lightList[i].nameIndex]))
            return lightList[i].id;
      }
   }

   return -1;
}

//----------------------------------------------------------------------------

template <class T>
inline void readSize(StreamIO& s,
         T&        array)
{
   Int32 size;
   s.read(&size);
   array.setSize(size);
}

template <class T>
inline void readArray(StreamIO& s,
          T&        array)
{
   s.read(array.size() * sizeof(T::value_type),
          (void*)array.address());
}  

template <class T>
inline void writeSize(StreamIO& s,T& array)
{
   s.write(Int32(array.size()));
}

template <class T>
inline void writeArray(StreamIO& s,
           T&        array)
{
   s.write(array.size() * sizeof(T::value_type),
           (void*)array.address());
}

void readNameBuffer(StreamIO& io_sio,
               ITRLighting::NameBuffer& out_rBuffer)
{
   UInt32 size;
   io_sio.read(&size);

   out_rBuffer.setSize(size);
   io_sio.read(size, out_rBuffer.address());
}

void writeNameBuffer(StreamIO& io_sio,
                const ITRLighting::NameBuffer& in_rBuffer)
{
   UInt32 size = in_rBuffer.size();
   io_sio.write(size);

   io_sio.write(size, in_rBuffer.address());
}

//----------------------------------------------------------------------------
Persistent::Base::Error 
ITRLighting::read(StreamIO& s,
                  int       version,
                  int       /*user*/)
{
   AssertFatal(version == FileVersion || version == 6,
               "ITRLighting::read: Incorrect file version");

   // Header, or in memory data first.
   s.read(&geometryBuildId);
   s.read(&lightScaleShift);
   s.read(&lightMapListSize);
   readSize(s,stateList);
   readSize(s,stateDataList);
   readSize(s,lightList);
   readSize(s,surfaceList);
   readSize(s,mapData);

   // Memory mapped portion.
   readArray(s,stateList);
   readArray(s,stateDataList);
   readArray(s,lightList);
   readArray(s,surfaceList);
   readArray(s,mapData);

   readNameBuffer(s, nameBuffer);

   // Read in the compression tables...
   //
   s.read(&m_huffmanCompressed);
   if (m_huffmanCompressed == true) {
      readSize(s, huffman16Nodes);
      readSize(s, huffman16Leaves);
      readArray(s, huffman16Nodes);
      readArray(s, huffman16Leaves);

//      if (version >= 7) {
//         readSize(s, huffman8Nodes);
//         readSize(s, huffman8Leaves);
//         readArray(s, huffman8Nodes);
//         readArray(s, huffman8Leaves);
//      }
   }

   return (s.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error 
ITRLighting::write(StreamIO& s,
                   int       version,
                   int       /*user*/)
{
   AssertFatal(version == FileVersion, "ITRLighting::write: Incorrect file version");

   // Header, or in memory data first.
   s.write(geometryBuildId);
   s.write(lightScaleShift);
   s.write(lightMapListSize);
   writeSize(s,stateList);
   writeSize(s,stateDataList);
   writeSize(s,lightList);
   writeSize(s,surfaceList);
   writeSize(s,mapData);

   // Memory mapped portion.
   writeArray(s,stateList);
   writeArray(s,stateDataList);
   writeArray(s,lightList);
   writeArray(s,surfaceList);
   writeArray(s,mapData);

   writeNameBuffer(s, nameBuffer);

   // Write out the compression tables...
   //
   s.write(m_huffmanCompressed);
   if (m_huffmanCompressed == true) {
      writeSize(s, huffman16Nodes);
      writeSize(s, huffman16Leaves);
      writeArray(s, huffman16Nodes);
      writeArray(s, huffman16Leaves);

//      writeSize(s, huffman8Nodes);
//      writeSize(s, huffman8Leaves);
//      writeArray(s, huffman8Nodes);
//      writeArray(s, huffman8Leaves);
   }

   return (s.getStatus() == STRM_OK)? Ok: WriteError;
}

int ITRLighting::version()
{
   return FileVersion;
}  


//-------------------------------------- Lightmap decompression.  out_pBuffer must
//                                        have no stride padding on entry...
//
UInt16
ITRLighting::decNextColor16(BitStream& io_rBS)
{
   AssertFatal(m_huffmanCompressed != 0, "No huffman tables to work from!");

   int currNode;
   UInt16 optRead = io_rBS.readInt(csm_huffOptShift);
   Huff16OptTableEntry& rEntry = m_huff16OptTable[optRead];

   if ((rEntry.goCode & (1 << 15)) != 0) {
      // Ok, the code was longer than 10 bits, set the current node, and keep
      //  moving...
      //
      currNode = rEntry.val;
   } else {
      // Hah!  Less than 10 Bits.  Reset the bitstreams position, and
      //  return the color...
      //
      io_rBS.setCurPos(io_rBS.getCurPos() - (csm_huffOptShift - rEntry.goCode));
      return rEntry.val;

   }

   while (true) {
      bool next = io_rBS.readFlag();

      if (next) currNode = huffman16Nodes[currNode].indexOne;
      else      currNode = huffman16Nodes[currNode].indexZero;

      if (currNode < 0)
         return huffman16Leaves[-(currNode+1)].color;
   }
}

UInt16
ITRLighting::decodeNextColor16(BitStream& io_rBS)
{
   AssertFatal(m_huffmanCompressed != 0, "No huffman tables to work from!");

   int currNode = 0;
   while (true) {
      bool next = io_rBS.readFlag();

      if (next) currNode = huffman16Nodes[currNode].indexOne;
      else      currNode = huffman16Nodes[currNode].indexZero;

      if (currNode < 0)
         return huffman16Leaves[-(currNode+1)].color;
   }
}

UInt16
ITRLighting::findNthColor16(BitStream& io_rBS, UInt16 index)
{
   int currNode  = 0;
   int currColor = 0;

   while (true) {
      bool next = io_rBS.readFlag();

      if (next) currNode = huffman16Nodes[currNode].indexOne;
      else      currNode = huffman16Nodes[currNode].indexZero;

      if (currNode < 0) {
         if (currColor++ == index) {
            return huffman16Leaves[-(currNode+1)].color;
         } else {
            currNode = 0;
         }
      }
   }
}

void
ITRLighting::decompressLightMap16(UInt8*   lMapData,
                                  UInt32   in_sizeX,
                                  UInt32   in_sizeY,
                                  UInt16*  out_pBuffer)
{
   // First, attach a bitstream to the input buffer for huffman decoding.  We
   //  know that the size of the compressed data will never exceed the size
   //  of the lightmap in 16 bit colors, since it would otherwise have remained
   //  uncompressed.
   //
   BitStream bs(lMapData, in_sizeX * in_sizeY * 2);
   
   for (UInt32 y = 0; y < in_sizeY; y++) {
      for (UInt32 x = 0; x < in_sizeX; x++) {
         int currNode = 0;
         while (true) {
            bool next = bs.readFlag();

            if (next) currNode = huffman16Nodes[currNode].indexOne;
            else      currNode = huffman16Nodes[currNode].indexZero;

            if (currNode < 0) {
               *out_pBuffer++ = huffman16Leaves[-(currNode+1)].color;
               break;
            }
         }
      }
   }
}

void
ITRLighting::table16Recurse(BitStream& codeBuffer, int node, int depth)
{
   if (depth > csm_huffOptShift && node > 0) {
      // oops, went to far.  Enter the node index into the table,
      //  and skeedadle..
      //
      int pos = codeBuffer.getCurPos();
      codeBuffer.setCurPos(0);
      Huff16OptTableEntry& rEntry = m_huff16OptTable[codeBuffer.readInt(csm_huffOptShift)];
      AssertFatal(rEntry.val == 0xffff && rEntry.goCode == 0xffff,
                  "Entry has already been set??");
      rEntry.goCode = 0x8000;
      rEntry.val    = node;
      codeBuffer.setCurPos(pos);
      return;
   }

   if (node < 0) {
      // Ok, we have a leaf here, we need to fill in all possible table entries...
      //

   }
}

void
ITRLighting::constructHuff16Table()
{
   AssertFatal(m_huffmanCompressed != 0, "Error, no huffman tree to construct table from");

   m_huff16OptTable.setSize(csm_numHuffOptEntries);
   memset(m_huff16OptTable.address(), 0xff,
          sizeof(Huff16OptTableEntry) * csm_numHuffOptEntries);

   UInt16 codeBuffer = 0;
   BitStream bStream(&codeBuffer, 2);

   table16Recurse(bStream, 0, 0);
}

ITRLighting::LightMap * ITRLighting::getLightMap( Int32 index )
{
   return( (ITRLighting::LightMap*)&mapData[index] );
}

//-----------------------------------------------------------------

IMPLEMENT_PERSISTENT(ITRMissionLighting);

ITRMissionLighting::ITRMissionLighting()
{
   //
}

//-----------------------------------------------------------------

ITRMissionLighting::~ITRMissionLighting()
{
   //
}

//-----------------------------------------------------------------

int ITRMissionLighting::version()
{
   return( FileVersion );
}  

//-----------------------------------------------------------------

Persistent::Base::Error ITRMissionLighting::write(StreamIO &sio, 
   int version, int /*user*/ )
{
   Persistent::Base::Error err = ITRLighting::write(sio,version,0);
   if( err != Ok )
      return( err );

   // write out our index pairs
   writeSize( sio, m_indexArray );
   
   LZHWStream lzStream;
   lzStream.attach(sio);
   lzStream.write( sizeof(IndexEntry) * m_indexArray.size(), &m_indexArray[0] );
   lzStream.detach();
   
   return( err );
}

//-----------------------------------------------------------------

Persistent::Base::Error ITRMissionLighting::read(StreamIO &sio,
   int version, int /*user*/ )
{
   Persistent::Base::Error err = ITRLighting::read(sio,version,0);
   if( err != Ok )
      return( err );
      
   // read in the arrays for the tables
   readSize( sio, m_indexArray );
   
   LZHRStream lzStream;
   lzStream.attach(sio);
   lzStream.read( sizeof(IndexEntry) * m_indexArray.size(), &m_indexArray[0] );
   lzStream.detach();
   
   // write out the index pairs
   return( err );
}

//-----------------------------------------------------------------

ITRLighting::LightMap * ITRMissionLighting::getLightMap( Int32 index )
{
   // get the index price-is-right style
   int min = 0;
   int max = m_indexArray.size();
   if( !max )
      return( NULL );
      
   max--;
   int step = max / 2;
   
   // half the step each time through and adjust the min/max
   while( step )
   {
      int cur = min + step;
      if( index == m_indexArray[cur].srcIndex )
         return( (ITRLighting::LightMap*)&mapData[m_indexArray[cur].destIndex] );
      else if( index > m_indexArray[cur].srcIndex )
      {
         min = cur + 1;
         step = ( max - min ) / 2;
      }
      else
      {
         max = cur - 1;
         step = ( max - min ) / 2;
      }
   }   
   
   // just walk the few left
   while( min <= max )
   {
      if( index == m_indexArray[min].srcIndex )
         return( (ITRLighting::LightMap*)&mapData[m_indexArray[min].destIndex] );
      min++;
   }

   return( NULL );
}


UInt32
ITRLighting::getCheckVal() const
{
   UInt32 retVal = ((computeCRC16(mapData.address(), mapData.size()) << 16) |
                    computeCRC16((const unsigned char *)nameBuffer.address(), nameBuffer.size()));

   return retVal;
}