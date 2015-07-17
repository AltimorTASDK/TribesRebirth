//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ITRLIGHTING_H_
#define _ITRLIGHTING_H_

//Includes
#include <base.h>
#include <ml.h>
#include <tvector.h>
#include <tMap.h>
#include <tString.h>
#include <persist.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

// light animation flags...
//
static const UInt32 ITRLIGHTANIM_AUTOSTART        = 1 << 0;
static const UInt32 ITRLIGHTANIM_LOOPTOENDFRAME   = 1 << 1;
static const UInt32 ITRLIGHTANIM_RANDOMFRAME      = 1 << 2;    // not implemented...
static const UInt32 ITRLIGHTANIM_MANAGEDBYMEDITOR = 1 << 3;

// Mask for special behaivors...
//
static const UInt32 ITRLIGHTANIM_SPECIALANIM = ITRLIGHTANIM_RANDOMFRAME;

//----------------------------------------------------------------------------

class BitStream;

class ITRLighting: public Persistent::VersionedBase
{
   static const UInt32 csm_huffOptShift;
   static const UInt32 csm_numHuffOptEntries;

  public:
   struct LightMap
   {
      enum Constants {
         // Size of header only:
         // sizeof(LightMap) - sizeof(UInt16)
         HeaderSize = 0,
         AlphaShift = 12,
      };
      // The size and offset of the light map are
      // stored with the surface as it's the same
      // for all light maps on that surface.
      // Light map data is 4:4:4:4, Alpha,RGB
      // Start of following data
      UInt16 data;
   };
   struct StateData
   {
      // There will be a statedata entry for each
      // surface affected by the light.  The lightIndex
      // value is the same index as that stored in
      // the Surface.
      Int16 surface;          // Affected surface.
      Int16 lightIndex;       // Index into instance lightMapList
      Int32 mapIndex;         // Index into lightMapData (-1 for none)
   };
   struct State
   {
      // Pos, etc. for TS?

      // Color components are stored as 8:8 from 255:0 - 0:0
      UInt16 red, green, blue, mono;
      
      // Time at which this state becomes the active one.
      float  animationTime;
      
      Int16 dataCount;
      Int16 dataIndex;        // Index into stateDataList
   };
   struct Light
   {
      Int32  id;
      Int32  nameIndex;
      Int32  stateCount;
      Int32  stateIndex;       // Index into stateList

      float  animationDuration;
      UInt32 animationFlags;
   };
   struct Surface
   {
      struct Point {
         UInt8 x, y;
      };

      // MapIndex is either an index into mapData or
      // a UInt16 4:4:4 color value. The high bit is set
      // to indicate a color.
      union {
         Int32 mapIndex;         // Static light map if high bit set
                                 //  if 2nd highest bit set, is compressed
         Int32 mapColor;         // 4:4:4:4 Color in lower 16bits
      };
      Int16 lightCount;       // Number of lights that affect surface
      Int16 lightIndex;       // Index into instance lightMapList
      Point mapSize;          // Light map size
      Point mapOffset;        // Light map offset
   };

   typedef Vector<UInt8>      MapData;
   typedef Vector<State>      StateList;
   typedef Vector<StateData>  StateDataList;
   typedef Vector<Light>      LightList;
   typedef Vector<Surface>    SurfaceList;
   typedef Vector<char>       NameBuffer;

   struct Huff16OptTableEntry {
      UInt16 val;
      UInt16 goCode;
   };
   struct Huffman16Node {
      Int32 indexOne;
      Int32 indexZero;
   };
   struct Huffman16Leaf {
      UInt32 color;
   };

   typedef Vector<Huffman16Node>       Huff16NodeList;
   typedef Vector<Huffman16Leaf>       Huff16LeafList;
   typedef Vector<Huff16OptTableEntry> Huff16OptTable;

   //
   NameBuffer    nameBuffer;
   LightList     lightList;
   StateList     stateList;
   StateDataList stateDataList;
   SurfaceList   surfaceList;
   MapData       mapData;

   UInt16        decNextColor16(BitStream& in_rBS);
   UInt16        decodeNextColor16(BitStream& in_rBS);
   UInt16        findNthColor16(BitStream& in_rBS, UInt16 index);
   UInt8         decodeNextColor8(BitStream& in_rBS);
   UInt8         findNthColor8(BitStream& in_rBS, UInt16 index);
   bool          m_huffmanCompressed;
   void          constructHuff16Table();
   void          table16Recurse(BitStream& codeBuffer, int node, int depth);

   virtual ITRLighting::LightMap * getLightMap( Int32 index );

   Huff16NodeList huffman16Nodes;
   Huff16LeafList huffman16Leaves;
   Huff16OptTable m_huff16OptTable;

   Int32         lightMapListSize;
   Int32         lightScaleShift;      // Texture to light coor.
   Int32         geometryBuildId;      // Build ID from geometry file.

   //
   ITRLighting();
   virtual ~ITRLighting();
   static UInt16 buildColor(float red,float green,float blue);
   static UInt16 buildColorWOMono(float red,float green,float blue);
   static UInt16 buildColor(float red,float green,float blue,float mono);
   static void ITRLighting::unpackColor( UInt16 color, float & red, float & green, float & blue, float & mono );
   static void   changeRGB(UInt16 *dst, float red,float green,float blue);

   const char* getName(const int in_nameIndex) { return &nameBuffer[in_nameIndex]; }

   void decompressLightMap16(UInt8*   lMapData,
                             UInt32   in_sizeX,
                             UInt32   in_sizeY,
                             UInt16*  out_pBuffer);
   void decompressLightMap8(UInt8*   lMapData,
                            UInt32   in_sizeX,
                            UInt32   in_sizeY,
                            UInt8*   out_pBuffer);

   //
   int findLight(int lightId);                 // Returns lightList index
   int findLight(const char* in_pLightName);   // Returns lightList index
   int findLightId(const char* in_pLightName); // Returns lightId for name

   UInt32 getCheckVal() const;

   // Persistent IO
   DECLARE_PERSISTENT(ITRLighting);
   Error read(StreamIO &,  int version, int );
   Error write(StreamIO &, int version, int );
   int   version();
};

//-----------------------------------------------------------------

class ITRMissionLighting : public ITRLighting
{
   public:
   
      struct IndexEntry
      {  
         Int32    srcIndex;
         Int32    destIndex;
      };
      
   private:
   
      Vector<IndexEntry>      m_indexArray;
      
   public:
   
      void addEntry( IndexEntry & entry ){ m_indexArray.push_back(entry); }
      
      ITRMissionLighting();
      ~ITRMissionLighting();
   
      ITRLighting::LightMap * getLightMap( Int32 index );
      
      // Persistent IO
      DECLARE_PERSISTENT(ITRMissionLighting);
      Error read( StreamIO &,  int version, int );
      Error write( StreamIO &, int version, int );
      int version();
};

//-----------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_ITRLIGHTING_H_

