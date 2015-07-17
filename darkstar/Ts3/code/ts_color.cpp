//---------------------------------------------------------------------------
//	
//	   ts_color.cpp
//
//	   Methods for dealing with coloring, light map intensity combining.  
//    Created 5/8/98, largely as a gathering place for similar code scattered
//    in several places.  
//	
//---------------------------------------------------------------------------


#include "ts_color.h"

namespace TS
{

Inten16Xlat::TableTables    Inten16Xlat::constTabs;
Inten16Xlat::RemapTable     Inten16Xlat::remapTabs[NumTables];
Inten16Xlat::RemapTable * Inten16Xlat::curMapperPtr = Inten16Xlat::remapTabs;


Inten16Xlat::RemapTable::RemapTable ()
{     // default table works for zero.  
   memset ( dist2Color, 0, sizeof(dist2Color) );
   maxColor = 0;
   usage = 0;
   scaleDistSquared = 0.0;
}


void Inten16Xlat::RemapTable::init ( float maxDSqrd, UInt16 maxCol )
{
   init ( maxDSqrd );
   constTabs.fillArray ( dist2Color, maxCol );
   incUsage ();
   maxColor = maxCol;
}


Inten16Xlat::RemapTable & 
Inten16Xlat::getMapper ( UInt16 maxColor, float maxDistSquared )
{
   Int16          minUsed = AbsMaxUse + 1;
   RemapTable     *rt;
   
   // We need to gradually erode the usage counts, otherwise high use 
   //    entries which then don't get used for a long time (if ever)
   //       can wind up filling up slots while others can't get a 
   //       foothold (the competition is greater due to less slots). 
   // Note the incUsage() method adds by 2.
   {
      static char which = 0;
      rt = &remapTabs[ which = (which+1) % NumTables ];
      if( --(rt->usage) < 0 )
         rt->usage = 0;
   }
   
   // find a table that already maps this color for us, or find the least-used free
   //    table.  The idea is that colors frequently mapped will not reuse the remap
   //    tables.  
   for ( int i = 0; i < NumTables; i++ )
      if( (rt = &remapTabs[i])->maxColor == maxColor)  {
         rt->init ( maxDistSquared );
         rt->incUsage();
         curMapperPtr = rt;
         return ( *curMapperPtr );
      }
      else if ( rt->usage < minUsed )
         minUsed = (curMapperPtr = rt)->usage;
   
   // replacing existing table, clear the usage count.  
   curMapperPtr->usage = 0;
   curMapperPtr->init ( maxDistSquared, maxColor );
   return ( *curMapperPtr );
}



// These are some tables to help us, uh, build our tables.  We have 16 tables to 
// map distance^2 to range.  Using some space here to avoid TableSize*4 square
// roots when a new light intensity is needed for the lighting.  They could be 
// packed into nybbles too... 
Inten16Xlat::TableTables::TableTables()
{
   for ( int i = 1; i < 16; i++ )   {
      UInt8 *tab = tabs[i-1];
      float maxd = sqrt(TableSize + .001f);
      float maxc = (float)i + 0.5;
      for ( int j = 0; j < TableSize; j++ ) 
         tab[j] = UInt8(maxc - (sqrt(j) / maxd) * (float)i);
   }
}

UInt16* Inten16Xlat::TableTables::fillArray( UInt16 arr[TableSize], UInt16 maxColor )
{
   register int   i;
   UInt8    *ft[4];
   UInt16   mask = 0xffff, col16;
   
   // Set up for our table building loop.  Zero isn't in our tables, so that's handled
   // by the mask.  
   for ( i = 0; i < 4; i++ ) 
      if ( (col16 = (maxColor >> (i << 2)) & 0xf)   !=   0  )
         ft[i] = tabs [ col16 - 1 ];
      else
         mask &= ~(0xf << (i << 2)), ft[i] = tabs[0];

   for ( i = 0; i < TableSize; i++ ) 
      arr[i] = (ft[0][i] | (ft[1][i]<<4) | (ft[2][i]<<8) | (ft[3][i]<<12)) & mask;
      
   return ( arr );
}


};  // namespace TS
