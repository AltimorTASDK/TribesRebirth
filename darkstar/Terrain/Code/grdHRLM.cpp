//---------------------------------------------------------------------------
//	
//	   grdHRLM.cpp
//
//    High resolution light map management for terrain: Creation, access
//    (via quad tree database), and compression.  Most of the actual
//    unrolling and expanding code is in grdBlock.cpp.  
//
//---------------------------------------------------------------------------


#include "grdBlock.h"
#include <ts_color.h>
#include <bitset.h>


//----------------------------------------------------------------------------------------
//                 Data

int  GridHrlmList::PrefDetail = 1;
bool HiresLightMap::fillDone = false;
UInt16 HiresLightMap::decompressionBuffer[HiresLightMap::MaxSquareSize];

static int getPower(UInt16 X)
{
	// Returns log (base 2) of X.  (Position of highest set bit).  
	int i = 0;
	if (X)
		do
			i++;
		while (X >>= 1);
	return i;
}

// The bitset shift values specify the light square width, but are 1 over (since we
//    want zero to indicate a 1x1 square.  i.e.:
//       0  ->    1x1,     1  ->    2x2
//       2  ->    3x3,     3  ->    5x5
//       4  ->    9x9,     5  ->    17x17
//
// Note our offset table can use bytes if we stay below 17x17 as the maximum high res
//    light square resolution.  Else wasting some data.  
inline UInt16 HiresLightMap::squareWidth(int shift)
{
   AssertFatal( 0 <= shift && shift <= MaxWidthShift, "hrlm: bad sqr shift" );
   int   maxWidth = 1 << (MaxWidthShift - 1);
   return (maxWidth >> (MaxWidthShift - shift)) + 1;
}

inline UInt16 HiresLightMap::squareSize(int shift)
{ 
   UInt16 width = squareWidth(shift); 
   return( width * width );
}


#if 0 //(HiresLightMap::MaxWidthShift <= 5) && 0
typedef UInt8   LightDataOffset;
#else
typedef UInt16  LightDataOffset;
#endif
static const LightDataOffset   emptyEntry = (LightDataOffset)(-1);


static LightDataOffset 
dataOffsets[HiresLightMap::NumLightingCombinations][HiresLightMap::TotalResolutions];

void HiresLightMap::fillOffsets(void)
{
   if( ! fillDone ){
      LightDataOffset   accumulateOffset;
      int               combination, shift;
      
      for( combination = 0; combination < NumLightingCombinations; combination++ )
      {
         accumulateOffset = 0;
         for( shift = 0; shift <= MaxWidthShift; shift++ )
         {
            LightDataOffset   tableValue = emptyEntry;
            if( BIT(shift) & combination ){
               tableValue = accumulateOffset;
               accumulateOffset += squareSize(shift);
            }
            dataOffsets[combination][shift] = tableValue;
         }
      }
      fillDone = true;
   }
}

UInt16 HiresLightMap::getMapOffset(UInt16 whichRes)
{
   AssertFatal( fillDone, "hrlm: called too soon" );
   AssertFatal( whichRes < TotalResolutions, avar("hrlm: %d is bad res",whichRes) );
   UInt16   off = dataOffsets[ resolutions ][ whichRes ];
   AssertFatal( off != emptyEntry, avar("hrlm: %d:%d empty",resolutions,whichRes) );
   return off;
}


// How many total light lixels are in the given set of maps.  Note this equals offset (from 
// table) of largest map (highest bit), plus the size of largest map.  
UInt16 HiresLightMap::numTotalLixels( UInt16 whichResolutions )
{
   UInt16   shift = MaxWidthShift;
   
   do
      if( BIT(shift) & whichResolutions )
         return dataOffsets[ whichResolutions ][ shift ] + squareSize(shift);
   while( shift-- );
   
   return 0;
}

inline bool HiresLightMap::moreThanOneLixel(void)
{ 
   return ( resolutions > BIT(Square1x1) ); 
}


// Used to allocate Ptr data for light maps, or reallocate if none present.  
// Note we must be in straight dump mode.  
//
// This function is the main workhorse routine as far as management of the 
// different sized maps goes (so of like resize() in the Vector<> class, for example).  
// All different combinations of light map resolutions can be present, and this routine
// does the resizing and copying over, etc.  
//
void HiresLightMap::resizeData( UInt8 flagsAfter )
{
   AssertFatal( compression == StraightDump, "only unpacked hrlms may be altered" );
   
   UInt8 flagsBefore = resolutions;
   
   if( flagsAfter == flagsBefore )
      return;
   
   // We have a special case when ONLY a 1x1 is present (single color):  we store the color
   //    instead of a pointer.  For simplicity here however, we just bracket the code with 
   //    a temporary allocation so that code in the middle is kept simple.  Likewise there 
   //    is a check at the end if only one color remains.  This 1x1 case DOES get well used
   //    by the mission lighter, so the space savings is worth it.  
   if( flagsBefore == BIT(Square1x1) ){
      UInt16   * tempPtr = new UInt16 [ 1 ];
      * tempPtr = color.Col;
      color.Ptr = tempPtr;
   }
   
   if( flagsAfter ){
      int      lixelCount = numTotalLixels(flagsAfter);
      UInt16   * newPtr = new UInt16 [ lixelCount ];
      memset( newPtr, 0, sizeof(UInt16) * lixelCount );
      
      if( flagsBefore ){
         AssertFatal( color.Ptr, "hrlm: invariant failure" );
         // copy over existing data into their slots.  
         for( int shift = 0; shift < TotalResolutions; shift++ )
            if( BIT(shift) & (flagsBefore & flagsAfter) )
               memcpy( newPtr + dataOffsets[ flagsAfter  ][ shift ], 
                   color.Ptr +  dataOffsets[ flagsBefore ][ shift ], 
                   squareSize( shift ) * sizeof(UInt16)   );
                   
         delete [] color.Ptr;
      }
      color.Ptr = newPtr;
   }
   else{//all are being cleared out. 
      if( color.Ptr ){
         delete [] color.Ptr;
         color.Ptr = NULL;
      }
   }

   // special case on the way out...
   if( flagsAfter == BIT(Square1x1) ){
      UInt16   theColor = color.Ptr[0];
      delete [] color.Ptr;
      color.Col = theColor;
   }
   
   resolutions = flagsAfter;
}

//----------------------------------------------------------------------------------------
//                Setting the map data, averaging.  

bool HiresLightMap::set ( int x, int y, UInt16 col )
{
   int   w = 17;
   AssertFatal(0,"hrlm: needed?" );
   if ( compression == StraightDump ){
      color.Ptr [ y * w + x ] = col;
      AssertFatal( (x|y) < w  &&  (x|y) >= 0, "hrlm: set()" );
      return true;
   }
   return false;
}
// Light this one at the maximum of the two.  
bool HiresLightMap::merge ( const HiresLightMap & inLightMap )
{
   int w = 17;
   if ( compression == StraightDump && inLightMap.compression == StraightDump ){
      for ( int i = w * w; i > 0; i-- ){
         TSExpandedInten16    cThis ( color.Ptr [ i ] );
         TSExpandedInten16    cThat ( inLightMap.color.Ptr [ i ] );
         for ( int j = 0; j < 4; j++ )
            if ( cThis[ j ] < cThat[ j ] )
               cThis[ j ] = cThat[ j ];
         color.Ptr[i] = cThis;
      }
      return true;
   }
   return false;
}


// Set the given color data in the light map.  whichRes is a shift value from the enum.  
bool HiresLightMap::setSquareData( UInt16 * colorData, Int16 whichResolution )
{
   if( ! (BIT(whichResolution) & resolutions) )
      resizeData( resolutions | BIT(whichResolution) );
   
   UInt16 * putWhere = getDataPointer( whichResolution );
   int      size = squareSize( whichResolution );
   memcpy( putWhere, colorData, size * sizeof(UInt16) );
   return true;
}

bool HiresLightMap::setSquareData( UInt16 singleColor )
{
   return setSquareData( & singleColor, Square1x1 );
}

// Remove the data for the specified resolutions.  Note this is a BITSET being
// passed in (whereas setSquareData() expects WHICH bit).  A possible solution to
// this easy mixup might be to define a BitSet8 type and use it....
// 
// If any of the specified resolutions are present, then remove those.
// 
bool HiresLightMap::axeSquareData( Int16 whichToAxe )
{
   // figure out the overlap (and ONLY the overlap, so the XOR works). 
   if( whichToAxe &= resolutions )
      resizeData( resolutions ^ whichToAxe );
      
   return true;
}



#define  SquareOffset(X,Y,W)     ( (Y) * (W) + (X) )
//
// average down one level from source to dest.  dst and src are assumed to have enough 
// space, (caller can use numTotalLixels(), or autoallocate MaxSquareSize words).  
//
// The 2x2 case needs a separate approach since otherwise the fractions aren't
// distributed completely, we just do a straight average.  
//
void HiresLightMap::averageDown ( UInt16 * dst, const UInt16 * src, UInt8 from )
{
   AssertFatal( from < TotalResolutions, avar("hrlm: can't go down from %d",from) );
   
   if( from == Square1x1 ){
      return;
   }
   else if( from == Square2x2 ){
      TSExpandedInten16  accumulate(0);
      for( int i = 0; i < 4; i++ ){
         TSExpandedInten16  c16( *src++ );
         accumulate += c16;
      }
      accumulate >>= 2;
      *dst = accumulate;
   }
   else{
      int   srcWidth = squareWidth( from );
      int   dstWidth = squareWidth( from - 1 );
      
      for( int y = 0; y < dstWidth; y++ ){
         for( int x = 0; x < dstWidth; x++ ){
            // We average down by taking a percentage of all nine (if in bounds) around the
            // the corresponding source light map.  Corners contribute 1/4, and up-down or
            // left-right interstices contribute 1/2.  With this scheme the total amount of 
            // average color will be maintained.  We'll see if this looks about right. 
            TSExpandedInten16    totalColor16(0);
            int srcX = x << 1, srcY = y << 1, totalScale = 0;

            for( int ud = -1; ud <= 1; ud++ ){
               for( int lr = -1; lr <= 1; lr++ ){
                  int   sx = srcX + lr,  sy = srcY + ud;
                  if( (sx>=0 && sx<srcWidth) && (sy>=0 && sy<srcWidth) ){
                     int   scaleFactor = 4;  //default for center = full scale
                     if( ud || lr )
                        scaleFactor = (ud && lr) ? 1 : 2;
                     TSExpandedInten16  col16 = src [ SquareOffset(sx, sy, srcWidth) ];
                     col16 *= scaleFactor;
                     totalColor16 += col16;
                     totalScale += scaleFactor;
                  }
               }//left-right
            }//up-down
            totalColor16 /= totalScale;
            dst[ SquareOffset(x, y, dstWidth) ] = totalColor16;
         }//X
      }//Y
   }//FromRes >= 3x3.
}


//-------------------------------------------------------------------------------------
//                Construction, initialization.  


void HiresLightMap::init(void)
{
   pos.x = pos.y = 0;
   nColors = flags = resolutions = 0;
   compression = StraightDump;
   color.Idx = 0;
}


HiresLightMap::HiresLightMap()
{
   init();
   compression = ColorDump;
}

// copy constructor
HiresLightMap::HiresLightMap( const HiresLightMap & h )
{
   * this = h;
   if( h.compression==StraightDump && h.resolutions>BIT(Square1x1) )
   {
      int   nLixels = h.numTotalLixels();
      color.Ptr = new UInt16 [ nLixels ];
      memcpy( color.Ptr, h.color.Ptr, nLixels * sizeof(UInt16) );
   }
}

HiresLightMap::HiresLightMap ( const Point2I & gridPos, UInt16 * copyData )
{
   init();
   pos = gridPos;
   resolutions = BIT(Square17x17);
   
   color.Ptr = new UInt16 [ 289 ];
   int   dataSz = 289 * sizeof(UInt16);
   if ( copyData != NULL )
      memcpy ( color.Ptr, copyData, dataSz );
   else
      memset ( color.Ptr, 0, dataSz );
}

HiresLightMap::HiresLightMap ( const Point2I & gridPos, Int16 whichRes, UInt16 * copyData )
{
   init();
   pos = gridPos;
   AssertFatal( copyData, "hrlm: this ctor expects data" );
   setSquareData( copyData, whichRes );
}

HiresLightMap::HiresLightMap ( const Point2I & gridPos, UInt16 singleColor )
{
   init();
   pos = gridPos;
   setSquareData( singleColor );
}


// When we (IF we) delete, see if memory needs to be freed.  
HiresLightMap::~HiresLightMap ()
{
   if ( compression == StraightDump ){
      if( resolutions > BIT(Square1x1) )
         delete [] color.Ptr;
   }else{
      AssertFatal( compression == ColorDump || isBitPacked(), "hrlm: dump sanitation" );
   }
}


//-------------------------------------------------------------------------------------
//                Rendering: 



// Return the pointer to the light map data for the given resolution.  This data must 
// have already been set aside- the "slot" must already exist, and we must be in 
// StraightDump mode (sort of the working mode for generating maps).  Note our special
// case for the single color square.  In case you're wondering why there even exist 
// single color "high res" light maps: the low res grid NEAR high res light maps doesn't 
// jibe with the high res scheme exactly since it needs to average differently for the 
// case when the high resolution maps are turned off (as a detail setting).  
UInt16 * HiresLightMap::getDataPointer( UInt8 whichResolution ) 
{
   AssertFatal( compression == StraightDump, "hrlm: wrong compress mode" );
   AssertFatal( resolutions & BIT(whichResolution), "hrlm: slot not present" );
   AssertFatal( whichResolution < TotalResolutions, "hrlm: getDataPtr() bad arg" );

   if( resolutions == BIT(Square1x1) )
      return( & color.Col );
   else
      return( & color.Ptr[ getMapOffset(whichResolution) ] );
}


// step fetch using Adj field on dest data (but it's just inc'd in inner loop).  
static void stepFetchAdjDst ( const GridBlock::Step & dst, const GridBlock::Step & src )
{
   UInt16   *ls = src.Data,  *ld = dst.Data;
   
   for( int y = src.Size; y > 0; y-- ){
      for (int x = src.Size; x > 0; x--)
         *ld++ = *ls, ls += src.Inc;
      ls += src.Adj;
      ld += dst.Adj;
   }
}


// For bit-packed data this method tells us how many UInt16's are needed for the given
// number of lixels.  
int HiresLightMap::bitPackedLixelWordUse( int nLixels )
{
   return ((nLixels + (1 << compression) - 1) >> compression);
}


// Perform the bit-pack decompression.  Each color in the light map is coded into a 
// few bits, and this smaller number then does a lookup into a pool of which colors
// are present.  This pool is kept at the end of the bit packed data.  
//
// This routine is also used by the poolDisabling code, which wants to unroll all the 
// data (required if changes are to be made to the maps).  This use is signaled by a 
// non-NULL last parameter, which supplies a buffer to unroll the data to.  
UInt16 * HiresLightMap::decompress( UInt16 * data, int whichRes, UInt16 * unrollBuf ) 
{
   AssertFatal( isBitPacked(), "hrlm: assuming bit packing" );
   
   BitSetN  bitPackedData( bitsPerLixel(), data );
   int      totalLixels = numTotalLixels();
   UInt16 * lookup = & data[ bitPackedLixelWordUse( totalLixels ) ];
   
   int      start, N;
   UInt16   * wherePut;

   if( unrollBuf == NULL ){   
      start = getMapOffset( whichRes );
      N = squareSize( whichRes );
      wherePut = decompressionBuffer;
   }else{
      start = 0;
      N = totalLixels;
      wherePut = unrollBuf;
   }
   
   bitPackedData.decompress( wherePut, lookup, N, start );
   
   return wherePut;
}

static void fillSolid ( const GridBlock::Step & dst, UInt16 c16 )
{
   UInt16   *ld = dst.Data;
   for( int y = dst.Size; y > 0; y--, ld += dst.Adj )
      for (int x = dst.Size; x > 0; x--)
         *ld++ = c16;
}

// The main expansion/fetch/decompression routine.  This fetches data from the high
// res light map from the bottom up, and is used by the level>0 decompression.  If the
// light map data is compressed, it first unpacks it to a buffer.  
void HiresLightMap::fetchLightMap( UInt16 * ld, int dstInc, int levelInc, UInt16 * data )
{
   GridBlock::Step   src, dst;
   int               squareSideLength = (1 << levelInc) + 1;

   AssertFatal( resolutions >= BIT(Square1x1), "hrlm: map has no data" );
   if( resolutions > BIT(Square1x1) )
   {
      if( !data )
         data = color.Ptr;
   
      // Find the closest resolution map to what we want (greater than or equal to 
      // levelInc).  Note our internal level is one higher.  
      int   closestResMap = levelInc + 1;
      while( ! ( BIT(closestResMap) & resolutions ) )
      {
         closestResMap++;
         AssertFatal( closestResMap<TotalResolutions, "hrlm: missing map data" );
      }
   
      if( isBitPacked() )
         data = decompress( data, closestResMap );
      else
         data += getMapOffset( closestResMap );
         
      int  w = squareWidth( closestResMap );
      
      src.Data = & data [ (w-1) * w ];
      src.Inc = 1 << ((closestResMap - 1) - levelInc);
      src.Size = squareSideLength;
      src.Adj = -(src.Inc * src.Size) - (src.Inc * w);
   }

   dst.Data = ld;
   dst.Adj = dstInc - (dst.Size = squareSideLength);
   
   if( resolutions == BIT(Square1x1) )
      fillSolid ( dst, color.Col );
   else
      stepFetchAdjDst( dst, src );
}



// Need a method on the LIST since IT has the color pool.  
//
// DO: the fetching of data happens in too many places here, and the meaning of 
//    the compression flag is not well defined.  
void  GridHrlmList::fetchLightMap( UInt16 * ld, int dstInc, int levelInc, int hrlmIndex )
{
   HiresLightMap  & h = (*this)[ hrlmIndex ];

   UInt16    *data = 0;
   if( h.compression != HiresLightMap::StraightDump )
      data = & colorPool[ h.color.Idx ];
      
   h.fetchLightMap ( ld, dstInc, levelInc, data );
}

// Get a square's worth of data for this resolution.  This will decompress into 
// a buffer if the data is packed.  
UInt16 * GridHrlmList::getHrlmData( HiresLightMap & h, UInt8 whichRes )
{
   AssertFatal( h.resolutions & BIT(whichRes), "hrlm: res not there" );
   
   if( h.compression == HiresLightMap::StraightDump ){
      return h.getDataPointer( whichRes );
   }
   else if ( h.compression == HiresLightMap::ColorDump ){
      if( h.resolutions == BIT(HiresLightMap::Square1x1) )
         return( & h.color.Col );
      else
         return( & colorPool[ h.color.Idx + h.getMapOffset( whichRes ) ] );
   }
   else{//decompress
      AssertFatal( h.isBitPacked(), "hrlm: expecting compressed data" );
      return h.decompress( & colorPool[ h.color.Idx ], whichRes );
   }
}

//*************************************************************************************
//          Pool consolidation / compression                                          *
//*************************************************************************************


typedef NTuple< (1<<16), UInt8 >    TSColorSet;



// Maybe a better name for this would be decompressLightMaps().  
void GridHrlmList::dismantlePool(void)
{
   for ( iterator itr = begin(); itr < end(); itr++ )
      if ( itr->compression != HiresLightMap::StraightDump )
         if( itr->moreThanOneLixel() )
         {
            int      nLixels = itr->numTotalLixels();
            UInt16   * data = new UInt16 [ nLixels ];
            
            if( itr->isBitPacked() )
               itr->decompress( & colorPool[itr->color.Idx], 0, data );
            else
               memcpy ( data, &colorPool[itr->color.Idx], nLixels * sizeof(UInt16) );
               
            itr->compression = HiresLightMap::StraightDump;
            itr->color.Ptr = data;
            itr->nColors = 0;
         }
   colorPool.clear ();
}


void GridHrlmList::compressLightMaps(void)
{
   TSColorSet  colorSet;
   
   memset(colorSet.vec, 0, sizeof(colorSet.vec) );
   
   dismantlePool();
   for ( iterator itr = begin(); itr != end(); itr++ )
   {
      if( itr->moreThanOneLixel() )
      {
         AssertFatal( itr->compression == HiresLightMap::StraightDump, 
                  "hrlm: strait is best" );
         
         Int32       poolIndex = colorPool.size();
         UInt16      * src = itr->color.Ptr;
         int         nLixels = itr->numTotalLixels(), i, nDistinct, j;
         
         UInt16   result[ HiresLightMap::CrudeTotalLixels+256 ];
         UInt16   colorList[256], resultSize = nLixels;
         itr->compression = HiresLightMap::ColorDump;

         // Here's our simple compression method, bitpack the whole set of lixels.
         //    this is less than optimal since the averaged down versions will likely up 
         //    the bit count, so a better method will be to individually bit pack each
         //    separate map.  Extraction is more involved in that case, so we'll start
         //    with this case.  Another good improvement would be to allow 3/5/6/7 bit-
         //    per-lixel compression 
         {
            for( i = 0; i < nLixels; i++ )                  // flag the distinct colors.
               colorSet[ src[ i ] ] = 1;
               
            for( i = nDistinct = 0; i < (1<<16); i++ )      // count the number of distinct
               if( colorSet[i] ){                           // colors and convert colors vector 
                  colorSet[i] = nDistinct++;                // into a remap array.  
                  if( nDistinct <= HiresLightMap::MaxBitPackColors )
                     colorList[ nDistinct - 1 ] = i;
               }
     
            if(nDistinct > HiresLightMap::MaxBitPackColors) 
            {                                // 1. NO COMPRESSION (DEFAULT).
               memcpy( result, src, resultSize * sizeof(UInt16) );
            }
            else if ( nDistinct > 1 )        // 2. COMPRESSION W/LESS BITS PER LIXEL.
            {       
               // Figure out how much we can bit-compress.  
               // The actual list of colors is then put at the end of the 
               //    array of data.  The bitpacked compression enum ID also
               //    gives us the logarithm of how many elements pack into
               //    our basic type, which is used then for the shift math. 
               //    (see bitPackedLixelWordUse(), for example).  
               int bitsPerLixel;
                  
               if( nDistinct > 16 ){                  // 2A--> BYTE-PER-LIXEL
                  bitsPerLixel = 8;
                  itr->compression = HiresLightMap::BitPack8;
               }
               else if( nDistinct > 4 ){  
                  bitsPerLixel = 4;                   // 2B--> NYBBLE
                  itr->compression = HiresLightMap::BitPack4;
               }
               else if( nDistinct > 2 ){              // 2C--> BIT PAIR
                  bitsPerLixel = 2;
                  itr->compression = HiresLightMap::BitPack2;
               }else{                                 // 2D--> SINGLE BIT. 
                  bitsPerLixel = 1;
                  itr->compression = HiresLightMap::BitPack1;
               }

               BitSetN  packer ( bitsPerLixel, result );
               for( j = 0; j < nLixels; j++ )
                  packer.set( j, colorSet[ src[ j ] ] );
            
               resultSize = itr->bitPackedLixelWordUse( nLixels );
               memcpy( result + resultSize, colorList, nDistinct * sizeof(UInt16) );
               resultSize += nDistinct;
               
               itr->nColors = nDistinct;
            }
            else                             // 3. MONO-COLOR, SAVE AS 1X1.
            {
               // Not really sure what the compression field should be.  The field defns
               // are a little awkward.  Better would be for StraightDump to be an
               // ownsData bit in the flags or something instead.. 
               itr->compression = HiresLightMap::StraightDump;
               itr->resolutions = BIT(HiresLightMap::Square1x1);
               itr->nColors = 0;
               itr->color.Col = colorList[0];
               resultSize = 0;
            }

            // The speed probably isn't appreciable, but we'll save some time and 
            //    only reset the non-zero elements of the color set.  
            if( nDistinct > HiresLightMap::MaxBitPackColors ){
               for( i = 0; i < nLixels; i++ )
                  colorSet[ src[i] ] = 0;
            }else{
               for( i = 0; i < nDistinct; i++ )
                  colorSet[ colorList[i] ] = 0;
            }
         }

         for ( i = 0; i < resultSize; i++ )
            colorPool.push_back ( result[i] );
            
         delete [] src;
         if( resultSize )
            itr->color.Idx = poolIndex;
      }
   }
}


//*************************************************************************************
//                 Hires Light Map List methods.                                      *
//*************************************************************************************



GridHrlmList::GridHrlmList ( int detailMax ) :
      Base ( 20 ),
      treeTable ( 100 ),
      colorPool ( 400 ),
      indexTable ( 400 )
{
   AssertFatal ( detailMax <= MaximumDetail, "hrlm block dimensions" );
   setHighestDetail ( detailMax );
   buildHelper = NULL;
   helperTable = NULL;
   maxLightMaps = MaximumLightMaps;
   version = Version;
   HiresLightMap::fillOffsets();
}

GridHrlmList::~GridHrlmList ()
{
   clear ();
}

void GridHrlmList::clear( void )
{
	for (iterator itr = begin(); itr != end(); itr++)
		(*itr).~HiresLightMap ();
   Base::clear();
   
   buildHelper = NULL;
   helperTable = NULL;
   treeTable.clear();
   colorPool.clear();
   indexTable.clear();
}


void  GridHrlmList::setHighestDetail ( int detLev )  
{ 
   highestLevel = detLev; 
   gridW = (1 << highestLevel);
}


// This will rebuild the high definition light map quad tree lookup table.  For:
//    1)  Quickly unrolling light maps as needed.  
//    2)  Iterating for compressing light maps - color pool consolidation efforts
//          will probably have better results if we are iterating through the 
//          hi definition light maps in a tree traversal.
void GridHrlmList::BuildQuadTree ( void )
{
   prepareQuadBuildHelp ();

   treeTable.clear ();
   LNode  lnode = BuildRecurse ( Point2S(0,0), highestLevel );
   if ( lnode )
      treeTable.push_back ( lnode );

   dismantleQuadBuildHelp ();
}

// Build the hi def lookup node for this square in the tree (the corner and level
//    combine to give the square).  Return the node or zero if none.  For leaf
//    nodes this means we put a 1 into the low nybble to make it non-zero.  All
//    non-leaf nodes are by definition non-zero (i.e. they have at least one
//    quadrant bit set).  
GridHrlmList::LNode GridHrlmList::BuildRecurse ( const Point2S & square, int level )
{
   if ( level == 0 )  {
      UInt16   index = gridIndex ( square );
      if ( index != 0xFFFF )
         return (index << 4) | 1;
   }
   else{
      Point2S  quads[4];
      LNode    ln[4], qb = 0;
      
      getQuadrants ( quads, square, --level );
      
      for ( int i = 0; i < 4; i++ )
         qb |= ((ln[i] = BuildRecurse( quads[i], level )) != 0) << i;
      
      if( qb )    // sub nodes?  put the subnodes into the tree, and build
      {           // this node for our caller to put in tree.  
         qb |= treeTable.size() << 4;
         for ( int i = 0; i < 4; i++ )
            if ( qb & BIT(i) )
               treeTable.push_back ( ln [ i ] );
         return qb;
      }
   }
   return 0;
}

// See if the given square (point and level determine square) has a node 
// associated with it - i.e. has high definition light maps below it.  Note that
// we expect the target square to still need shifting to get it's 'absolute'
// grid location - i.e. we expect it be as the terrain system keeps it.  
GridHrlmList::LNode GridHrlmList::findHighResNode( Point2I trgtSqr, int targetLevel )
{
   if( ! PrefDetail )
      return 0;
      
   if( treeTable.empty () )
      return 0;
      
   // For given quad bits, and a bit position of one of them, give it's number. 
   #define  QbBitPosition(qb,shift)     (_qb_bitpos[(qb)&7][shift])
   static const char _qb_bitpos[8][4] = { 
            {0,0,0,0}, {0,0,0,1}, {0,0,0,1}, {0,1,0,2},
            {0,0,0,1}, {0,0,1,2}, {0,0,1,2}, {0,1,2,3} };
            
   Point2S  targetSquare( trgtSqr.x, trgtSqr.y );

   targetSquare <<= targetLevel;

   // set up the loop.  our root is at the _end_.  
   LNode    node = treeTable.last(); 
   int      curLevel = highestLevel;
   Point2S  iterSquare ( 0, 0 );
   
   // Note high bits of node gives index to sub nodes or (at lowest level)
   //    index to square, low 4 bits (when level > 0) tell if which quadrants
   //    have sub-nodes (i.e. which contain hi def maps).  
   while( curLevel-- != targetLevel ){
      UInt16   shift = 0;
      int      halfWid = 1 << curLevel;
      if( targetSquare.x >= iterSquare.x + halfWid )
         shift += 1, iterSquare.x += halfWid;
      if( targetSquare.y >= iterSquare.y + halfWid )
         shift += 2, iterSquare.y += halfWid;
      if( node & BIT(shift) )
         node = treeTable [ (node >> 4) + QbBitPosition(node, shift) ];
      else
         return 0;
   }
   return node;
}


// This routine is what we use to expand lightmaps for a given target level
// square.  It builds a list of all high res light maps that are contained within
// that square.  The list of indices is built in the member variable hiDefList.  
int GridHrlmList::getSubSquareList( Point2I trgtSqr, int targetLevel )
{
   hiDefList.clear();
   
   LNode    ln = findHighResNode ( trgtSqr, targetLevel );
   
   if( ln )
   {
      Point2S  targetSquare( trgtSqr.x, trgtSqr.y );
      targetSquare <<= targetLevel;
      getSubSquareRecurse( targetSquare, targetLevel, ln );
   }

   return hiDefList.size();
}



// Build a list of high resolution squares contained below the given square.  The square
// coordinates have already been shifted up.  The squares which are found are pushed
// onto hiDefList.  getSubSquareList() above sets up the recursion.  
void GridHrlmList::getSubSquareRecurse ( const Point2S & square, int level, LNode ln )
{
   LNode    index = (ln >> 4);
   
   if ( level == 0 )
      hiDefList.push_back ( index );
   else{
      Point2S  quads[4];
      getQuadrants ( quads, square, --level );
      for ( int i = 4; i > 0; i--, ln >>= 1 )
         if ( ln & 1 )
            getSubSquareRecurse( quads[i], level, treeTable [ index++ ] );
   }
}


// Adds the given HiresLightMap data to the list, if that square has not already been
// put in the list.  We may want a different behavior for when this happens.  It may
// be well to provide some random access capabilities (ability to poke values anywhere
// into the gridBlock array).  
HiresLightMap * GridHrlmList::push_back( const HiresLightMap & inLightMap ) 
{
   // The map is copied into the list, return the one actually on the list:
   HiresLightMap *   newMap = NULL;
   
   if( size() < maxLightMaps )
   {
      HiresLightMap  * sameSquare = NULL;
      for ( iterator itr = begin(); itr < end(); itr++ ) {
         if ( itr->pos == inLightMap.pos ) {
            sameSquare = itr;
            break;
         }
      }
      if ( sameSquare == NULL ){
         // allocate the space and invoke the copy constructor.  
         Base::increment();
         new(end()-1) HiresLightMap(inLightMap);
         newMap = & last();
      }
   }
      
   return newMap;
}


//****************************************************************************
//      Internal utility methods used in tree building process              **
//****************************************************************************


void GridHrlmList::removeInvalidSquares ()
{
   // compact invalid squares out of list.  
}


// We need to build a big temporary grid to make the build fast.  buildHelper[x][y]
// will give the square index if there's one there, or -1.  This is also where
// we can trim out duplicates from the list.  We make a first pass to invalidate
// the duplicates, and then we compact and build the indices again.  
void GridHrlmList::prepareQuadBuildHelp ( void )
{
   AssertFatal( !helperTable && !buildHelper, "hrlm: unbalanced quad build prep" );
   AssertFatal( size() <= 65535, avar("hrlm: %s exceeds hi def lim (%s)",size(),65535) );

   helperTable = new UInt16 [ gridW * gridW ];
   buildHelper = new UInt16 * [ gridW ];
   
   for ( int i = 0; i < gridW; i++ )
      buildHelper[ i ] = &helperTable[ i * gridW ];
      
   for ( int pass = 0; pass < 1; pass++ ) {
      // removeInvalidSquares ();
      memset ( helperTable, 0xff, gridW * gridW * sizeof(UInt16) );
      for ( iterator itr = begin(); itr < end(); itr++ ) {
         int   x = itr->pos.x, y = itr->pos.y;
         AssertFatal ( (x|y) < gridW && (x|y) >= 0, "HRLM bounds" );
         UInt16   index = buildHelper[ x ] [ y ];
         if ( index != 0xFFFF ) {
            (*this)[ index ].invalidate ();
            AssertFatal ( 0, "hrlm: culls dups" );
         }
         buildHelper[ x ] [ y ] = (itr - begin());
      }
   }
}

void GridHrlmList::dismantleQuadBuildHelp ( void )
{
   delete [] helperTable;
   delete [] buildHelper;
   helperTable = NULL;
   buildHelper = NULL;
}


UInt16 GridHrlmList::gridIndex ( const Point2S & s )
{
   AssertFatal ( buildHelper != NULL, "hrlm: temp mgmt" );
   AssertFatal ( (s.x|s.y) < gridW && (s.x|s.y) >= 0, "hrlm: bounds" );
   return ( buildHelper[ s.x ][ s.y ] );
}


//       Y
//
//       ^
//       |
//       |  Quad 2   Quad 3
//       |
//       |  Quad 0   Quad 1
//    ---+--------------------> X
//       |
// 
void GridHrlmList::getQuadrants ( Point2S quads[4], const Point2S & q0, int level )
{
   quads[0]( q0.x + 0,           q0.y + 0 );
   quads[1]( q0.x + (1<<level),  q0.y + 0 );
   quads[2]( q0.x + 0,           q0.y + (1<<level) );
   quads[3]( quads[1].x,         quads[2].y );
}




#if 0

         NOTES

 Square compression method ideas:

      Straight (non compress) dump.  Fastest and fattest. 
      Bit packed with 1/2/4 bits for a pool lookup.
      Bit packed with 3/5/6 bits.
      Modified bit packing:  i.e. suppose there are 25 colors, but the least frequently
            occuring 9 of them account for only a dozen squares or such.  Then we 
            do a 1/2/4 bit compress, and just dump the rest as 16 bit pool indices 
            (which will probably be better than using offset/UInt16 bit pairs on 
            average since the pool will have lots of common data..).  
      Quad tree bit streamed. 


 Here's the order of the data withing a compressed square: first the level ZERO
 data, then the level One, the level 2, and so on.  Each square will store data down
 to the level it needs, in this order.  


  ZERO  4  l3  4  L2  4   l3  4  One   4   l3  4   L2   4   l3 4  ZERO
   l3      l3     l3      l3      l3       l3      l3       l3    l3
   L2      l3     L2      l3     L2        l3      L2       l3    L2
   l3      l3     l3      l3      l3       l3      l3       l3    l3
   One     l3     L2      l3     One       l3      L2       l3    One
   l3      l3     l3      l3      l3       l3      l3       l3    l3
   L2      l3     L2      l3     L2        l3      L2       l3    L2
   l3      l3     l3      l3      l3       l3      l3       l3    l3
  ZERO     l3     L2      l3     One       l3      L2       l3    ZERO
  

#if 0
//    10/11/98: 
//    Code for compression was suspended sort of halfway through development.  
// 
//    A straightforward bit packing compression will be employed instead of the more 
//       involved ideas worked up before (below), and will likely suffice.  
// 
#define  NStats         (1<<16)
#define  MaxHrlmShift   5
#define  MaxByteDumps   (MaxHrlmShift-1)
#define  MaxHrlmSize    ( (1<<MaxHrlmShift) + 1 )
#define  MaxSubSample   ( (1<<MaxHrlmShift-1) + 1 )
#define  MaxHrlmPool    ( 2 * MaxHrlmSize * MaxHrlmSize )

namespace {
// Function to compare pool entries.  
int _USERENTRY compareObject(const void* a,const void* b)
{
   PoolEntry   *  p1 = reinterpret_cast<const PoolEntry*>(a);
   PoolEntry   *  p2 = reinterpret_cast<const PoolEntry*>(b);
}
enum CompressionInfo{
   // 8 methods used to compress sub squares within grid.  
   PredefSolidColor, 
   SingleStreamColor,
   PredefBitArray,
   OtherMethods,
   TwoStreamColors,
   FourStreamColors,
   StreamBitArray,
   // square compressed stream definitions:
   InterpolateUp = 253,
   InterpolateDown = 0,
   PoolDumpSquare = 255,
};
};//namespace

// Table tells where to put data for decompression routines (it's data is 
// kept in order of decompression.  The next table is the inverse and is used 
// internally to figure out how to feed that data to the light map compression.  
// The drawBuffer is where toWhere points at.  So decompression algorithms all 
// just scan the required amount of data and store in the drawBuffer using 
// toWhere.  We then fetch out of the drawBuffer with the requisite flips.  
// We're using an extra pass in order to take the flipping and stepping out of 
// the compressor.  Given that the passes will be more streamlined, it probably 
// costs at most little, and gives more flexibility.  Presumably the drawBuffer 
// will be cached in the expansion loop too...  
static UInt16 * toWhere [ MaxHrlmSize * MaxHrlmSize ];
static short fromWhere [ MaxHrlmSize * MaxHrlmSize ];
static UInt16 drawBuffer [ MaxHrlmSize * MaxHrlmSize ];

void fillWhereTables ( int maxShift )
{
   AssertFatal ( maxShift<=MaxHrlmShift && maxShift>2, "HRLM Shifty" );
   int   x, y, W, sh, offset, width = (1 << maxShift);
   
   memset ( fromWhere, 0xff, sizeof(fromWhere) );
   UInt16   where = 0;
   for( sh = maxShift; sh >= 0; sh-- )
      for( y = 0, W = (1 << sh); y <= width; y += W ){
         offset = y * (W + 1);
         for( x = 0; x <= width; x += W, offset += W )
            if ( fromWhere[offset] < 0 )
               fromWhere[offset] = where++;
      }
   
   for( offset = (width+1)*(width+1)-1; offset >= 0; offset-- ){
      AssertFatal ( fromWhere[offset] >= 0, "what's up with fromWhere?" );
      toWhere[fromWhere[offset]] = drawBuffer + offset;
   }
}
#endif
#endif
