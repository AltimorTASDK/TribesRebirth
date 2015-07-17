//---------------------------------------------------------------------------
//	
//	   ts_color.h
//	
//---------------------------------------------------------------------------

#ifndef _TS_COLOR_H_
#define _TS_COLOR_H_

#include <types.h>
#include <tNTuple.h>

#define  RGB_2_MONO_FORMULA(r,g,b)        (0.3*(r) + 0.6*(g) + 0.1*(b))

namespace TS
{
   // Methods to quickly map distance^2 to a desired packed color intensity.  
   class Inten16Xlat 
   {
      public:   
         enum{ TableShift = 6,
               TableSize = (1 << TableShift),
               TableSizeMask = (TableSize - 1),
               AbsMaxUse = 28000,
               NumTables = 3,  };
               
         struct TableTables { 
            UInt8    tabs [ 15 ] [ TableSize ]; 
            UInt16 * fillArray ( UInt16 arr[TableSize], UInt16 maxColor ); 
            TableTables (); 
         }; 
         
         static TableTables      constTabs;
         struct RemapTable { 
            UInt16   dist2Color [ TableSize ]; 
            UInt16   maxColor; 
            Int16    usage; 
            float    scaleDistSquared; 
            void     incUsage () { if((usage+=2) > AbsMaxUse)  usage=AbsMaxUse; }
            RemapTable ();
            void init ( float maxDSqrd, UInt16 maxCol );
            void init ( float maxDSqrd )    {
                  scaleDistSquared = (float)TableSize / (maxDSqrd + 0.001f);
               }
            UInt16 operator[] ( float dSqrd )  {
                  int   i = int(dSqrd * scaleDistSquared);
                  if ( i < 0 )  
                     i = 0;
                  else if ( i >= TableSize )  
                     i = TableSize - 1;
                  return ( dist2Color [ i & TableSizeMask ] );
               }
         };//RemapTable
         
      private:
         static RemapTable       remapTabs [ NumTables ];
         static RemapTable       *curMapperPtr;

      public:
         static RemapTable & getMapper ( UInt16 maxColor, float maxDistSquared );
   };//Inten16Xlat
   

   // The following are catch-all classes for consolidating routines used 
   // in several places.  
   class Inten16 { 
      public:
         static UInt16 add ( UInt16 color, UInt16 inten );
         static UInt16 subtract ( UInt16 color, UInt16 inten );
         static UInt16 getPackedColor(float r, float g, float b );
   };//Inten16 
   class Color {
      public:
         static void saturate ( const ColorF & src, ColorF * dst );
   };//Color



   // Can expand packed intensity to do math on it (such as interpolation).  
   class ExpandedInten16 : public NTuple<4,Int32>
   {
      public:
         ExpandedInten16 ()   {}
         ExpandedInten16 ( UInt16 val )     {  (*this) = val;  }
         ExpandedInten16 ( Int32 x0, Int32 x1, Int32 x2, Int32 x3 )  {
               vec[0]=x0;  vec[1]=x1;  vec[2]=x2;  vec[3]=x3;  }
         
         // Do these operations without FOR loops as they happen often.  
         
         void operator = ( UInt16 val ) 
         {  // expand each 4 bit compoenent into high half of each vector entry
            vec [ 0 ] = (UInt32(val & 0xf)) << 0x10;
            vec [ 1 ] = (UInt32(val & 0xf0)) << 0x0C;
            vec [ 2 ] = (UInt32(val & 0xf00)) << 0x08;
            vec [ 3 ] = (UInt32(val & 0xf000)) << 0x04;
         }
         operator UInt16 ( ) const 
         {  // recombine back into 16 bit value with rounding.  
            return ( 
               ( (UInt16((vec[0]+32768) >> 0x10))  &  0xf)         |
               ( (UInt16((vec[1]+32768) >> 0x0C))  &  0xf0)        |
               ( (UInt16((vec[2]+32768) >> 0x08))  &  0xf00)       |
               ( (UInt16((vec[3]+32768) >> 0x04))  &  0xf000) );
         }
         ExpandedInten16 & operator += ( const ExpandedInten16 & ec )   {
            vec [ 0 ] += ec.vec [ 0 ];
            vec [ 1 ] += ec.vec [ 1 ];
            vec [ 2 ] += ec.vec [ 2 ];
            vec [ 3 ] += ec.vec [ 3 ];
            return ( *this );
         }
   };//ExpandedInten16


   inline UInt16 Inten16::add ( UInt16 color, UInt16 inten )
   {
      register UInt32   i32;
      register UInt16   m, i16;

      i32 = (color & ~0x0fff) + (inten & ~0x0fff);    // watch overflow -> 32.
      m = (i32 > 0xf000)? 0xf000: i32;

      i16 = (color & 0xf00) + (inten & 0xf00);        // prefer 16
      m |= (i16 > 0xf00)? 0xf00: i16;

      i16 = (color & 0xf0) + (inten & 0xf0);
      m |= (i16 > 0xf0)? 0xf0: i16;

      i16 = (color & 0xf) + (inten & 0xf);
      return ( m | ((i16 > 0xf)? 0xf: i16 ) );
   }

   inline UInt16 Inten16::getPackedColor(float r, float g, float b )
   {
      float    m = RGB_2_MONO_FORMULA ( r, g, b );
      UInt16 rNew = UInt16 ( r * 15.99 );
      UInt16 gNew = UInt16 ( g * 15.99 );
      UInt16 bNew = UInt16 ( b * 15.99 );
      UInt16 mNew = UInt16 ( m * 15.99 );
      return (mNew << 12) | (rNew << 8) | (gNew << 4) | (bNew << 0);
   }
   
   inline UInt16 Inten16::subtract ( UInt16 color, UInt16 inten )
   {
      register UInt16  m, i;
      
      if ( (m = (color & ~0x0fff) - (inten & ~0x0fff)) & 1 )
         m = 0;
      if ( Int16(i = (color & ~0xf0ff) - (inten & ~0xf0ff))  >  0  )
         m |= i;
      if ( Int16(i = (color & ~0xff0f) - (inten & ~0xff0f))  > 0  )
         m |= i;
      if ( Int16(i = (color & ~0xfff0) - (inten & ~0xfff0))  > 0  )
         m |= i;
      return m;
   }
   
   
   inline void Color::saturate ( const ColorF & src, ColorF * dst )
   {
      if( src.red > 1.0f )
         dst->red = 1.0f;
      else if (src.red < 0.0f)
         dst->red = 0.0f;
      else
         dst->red = src.red;
      if( src.green > 1.0f )
         dst->green = 1.0f;
      else if (src.green < 0.0f)
         dst->green = 0.0f;
      else
         dst->green = src.green;
      if( src.blue > 1.0f )
         dst->blue = 1.0f;
      else if (src.blue < 0.0f)
         dst->blue = 0.0f;
      else
         dst->blue = src.blue;
   }
   

};//namespace TS


typedef TS::Color                TSColor;
typedef TS::Inten16              TSInten16;
typedef TS::Inten16Xlat          TSInten16Xlat;
typedef TS::ExpandedInten16      TSExpandedInten16;


#endif
