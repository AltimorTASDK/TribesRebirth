//-------------------------------------------------------------------------------------
//	         grdHrlm.h 
//
//    High res light maps, see .cpp file.  
//-------------------------------------------------------------------------------------


#ifndef _GRDHRLM_H_
#define _GRDHRLM_H_

#include <tvector.h>
#include <types.h>
#include <ml.h>


#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


#define  _DEBUG_LIGHT_DUMP    1


class GridHrlmList;


class HiresLightMap 
{
   friend class GridHrlmList;
   
   static void    fillOffsets(void);
   static UInt16  numTotalLixels( UInt16 whichResolutions );
   static UInt16  squareWidth(int shift);
   static UInt16  squareSize(int shift);
   
   bool     merge ( const HiresLightMap & inLightMap );
   UInt16   getMapOffset(UInt16 whichRes);
   void     resizeData( UInt8 flagsAfter );
   void     invalidate () { flags |= Invalid;  }
   int      bitPackedLixelWordUse( int nLixels );
   UInt16 * decompress( UInt16 * data, int whichRes, UInt16 * unrollBuf = 0 );
   
   
public:
   enum{  
      // packing methods.
      StraightDump = 0,       // straight alloc: it owns data.  
      BitPack8 = 1,           // compressing in 8 bits or less per lixel.  These
      BitPack4 = 2,           //    enum values are important - they give a 
      BitPack2 = 3,           //    shiftAdjust value for array size math.  
      BitPack1 = 4,
      ColorDump,              // entries are dumped in color pool. 
      // flags, miscellaneous
      Invalid = 1 << 0,
      MaxBitPackColors = 255,
   };
   int bitsPerLixel() { return (1 << (4 - compression)); }
   
   enum{
      Square1x1,           // these defines are one over actual shift value
      Square2x2,           // in formula - so ZERO gives ONE-by-ONE square.
      Square3x3,
      Square5x5,
      Square9x9,
      Square17x17,
      MaxWidthShift = Square17x17,
      Square33x33,                     // prob'ly won't go here
      Square65x65,
      AbsMaxWidthShift = Square65x65,  // never will go past here. 
      TotalResolutions = MaxWidthShift + 1,
      MaxSquareWidth = ((1<< (MaxWidthShift-1)) + 1),
      MaxSquareSize = (MaxSquareWidth * MaxSquareWidth),
      // Rough approximation of sum.  Summing a series with r about.25 so sum is 1+.33, 
      // so 1+.5 is safe upper bound.  Used for autoclass arrays in the compression:
      CrudeTotalLixels = MaxSquareSize + (MaxSquareSize >> 1),
      // An HRLM can have ANY combination of maps up to the highest res.  
      NumLightingCombinations = (1 << TotalResolutions),
   };
   
private:
   static bool    fillDone;
   static UInt16  decompressionBuffer[MaxSquareSize];

public:
   // Data members- about 12 bytes. This COULD go as low as 8 by making the position 
   // into a pair of bytes AND combining nColors to also have some compression codes.  
   // Both ideas seem too little limitting and the space is probably worth it.  
   Point2S  pos;                 // grid position.  
   UInt8    flags;
   UInt8    compression;         // method of compression.
   UInt8    resolutions;         // bitset of which resolution hrlms are present.
   UInt8    nColors;             // number of colors if bit-packed.  
   union{                        
      UInt16   * Ptr;            // Pool lookups, saved as index. Mission lighting uses
      UInt32   Idx;              //    pointer since data may get resized.  
      UInt16   Col;              // Color goes here for single color square case.  
   }color; 
   
   HiresLightMap ( const Point2I & gridPos, UInt16 * copyData = 0 );  //makes 17x17
   HiresLightMap ( const Point2I & gridPos, Int16 whichRes, UInt16 * copyData );
   HiresLightMap ( const Point2I & gridPos, UInt16 singleColor );
   HiresLightMap ( const HiresLightMap & h );
   HiresLightMap();
   void init(void);
   ~HiresLightMap ();
   
   bool moreThanOneLixel(void);
   bool setSquareData( UInt16 singleColor );
   bool setSquareData( UInt16 * colorData, Int16 whichRes );
   void averageDown ( UInt16 * dst, const UInt16 * src, UInt8 from );
   bool axeSquareData( Int16 bitsetSpecifyingWhichOnes );
   
   UInt16 get ( int x, int y );
   bool  set ( int x, int y, UInt16 color );
   bool  invalid ()   { return ((flags & Invalid) != 0);   }
   bool  isBitPacked(void) { return compression<=BitPack1 && compression>=BitPack8; }
   bool  valid() { return ! invalid(); }
   UInt16 numTotalLixels(void) const    { return numTotalLixels( this->resolutions ); }
   UInt16 * getDataPointer( UInt8 whichResolution );
   
   void fetchLightMap ( UInt16 * ld, int dstInc, int levelInc, UInt16 * data = 0 );
};



class GridHrlmList : private Vector<HiresLightMap> 
{
	   typedef Vector<HiresLightMap>    Base;
      
   public:
      GridHrlmList ( int detailMax = 0 );
      ~GridHrlmList ();
   
      typedef  UInt32            LNode;
      typedef  Vector<LNode>     NodeTable;
      typedef  Vector<UInt16>    Table16;
      typedef  Vector<UInt8>     IndexTable;
      
      enum{
         Version = 3,
      };

   private:
      enum  {  MaximumDetail = 8, 
               MaximumLightMaps = 50000,
               MaxGridWidth = (1<<MaximumDetail),
            };
      int      maxLightMaps;
      int      highestLevel;
      int      gridW;
      UInt16   **buildHelper;
      UInt16   *helperTable;
      
      void     removeInvalidSquares ( void );
      void     prepareQuadBuildHelp ( void );
      void     dismantleQuadBuildHelp ( void );
      UInt16   gridIndex ( const Point2S & s );
      void     getQuadrants ( Point2S quads[4], const Point2S & q0, int level );
      void     getSubSquareRecurse ( const Point2S & square, int level, LNode ln );
      LNode    BuildRecurse ( const Point2S & square, int level );


   public:
      // Along with the vector itself, the following three pools are what gets 
      // stored as part of the block. 
      NodeTable      treeTable;
      Table16        colorPool;
      IndexTable     indexTable;
      int            version;

      // This controls the highest level of HRLM that will be used in the terrain 
      // rendering.  It is a shift amount, and the formula for the HRLM square width 
      // resolution is like so:  ((1 << PrefDetail-1) + 1).  One gives a default 2x2 
      // square.  HiresLightMap::MaxWidthShift gives the maximum (-> 17x17 now).  
      static int  PrefDetail;

      // getSubSquareList() builds list of high res squares within the target
      // square.  list is put into hiDefList, count is returned.  
      Table16  hiDefList;
      int      getSubSquareList( Point2I targetSquare, int targetLevel );

      // Tree must be (re)built for any newly added light maps to be rendered.
      void     BuildQuadTree( void );
      
      // finds the quad tree node associated with the given square IFF the given
      // square contains any high res light maps.  
      LNode    findHighResNode ( Point2I targetSquare, int targetLevel );
      void     setHighestDetail ( int detLev );
      void     setMaxLMaps ( int maxLms ){ 
                     if ( (maxLightMaps = maxLms) > MaximumLightMaps )
                        maxLightMaps = MaximumLightMaps;  }
                  
      void     dismantlePool(void);
      void     compressLightMaps(void);
      void     fetchLightMap( UInt16 * ld, int dstInc, int levelInc, int ind );
      UInt16   * getHrlmData( HiresLightMap & h, UInt8 whichRes );

      
      /* Provide public interface to SOME of the vector functionality.  Light maps must 
      ** be deconstructed, and we may also need to perform some list consolidation when 
      ** an erase happens.  
      */
      HiresLightMap * push_back ( const HiresLightMap & hrlm );
      void erase (int index)   {
   		operator[] (index).~HiresLightMap ();
         Base::erase(index);  
      }
      void clear( void );
   	HiresLightMap& operator[](UInt16 index) { 
         return Base::operator[](index);
      }
   	typedef HiresLightMap * iterator;
   	iterator begin()                    {  return Base::begin();      }
   	iterator end()                      {  return Base::end();        }
      int  size() const                   {  return Base::size();       }
   	bool empty() const                  {  return Base::empty();      }
   	HiresLightMap & last()              {  return Base::last();       }
      HiresLightMap * address() const     {  return Base::address();    }
};



#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif

