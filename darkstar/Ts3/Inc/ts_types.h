//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_TYPES_H_
#define _TS_TYPES_H_

#include <tsorted.h>
#include <talgorithm.h>
#include <ml.h>

namespace TS
{
   //---------------------------------------------------------------------------
   //------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a4
#pragma option -w-inl
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

   enum Constants
      {
      MaxNameSize = 24,
      };

   enum ClipCodes
      {
      ClipAllVis  = 0,     // completely visible
   	ClipNear    = 0x1,
      ClipFar     = 0x2,
      ClipFarSphere = 0x4,
      ClipLeft    = 0x8,
      ClipRight   = 0x10,
      ClipTop     = 0x20,
      ClipBottom  = 0x40,
      ClipNoneVis = 0x80,  // completely clipped
      ClipUnknown = 0x100,  // unknown; not yet tested
      ClipX       = ClipLeft | ClipRight,
      ClipY       = ClipTop | ClipBottom,
      ClipZ       = ClipNear | ClipFar | ClipFarSphere,
      ClipMask    = ClipX | ClipY | ClipZ,
     	};

   class Name
      {
      char name[MaxNameSize];
      public:

      Name( char * nm )
         {
         AssertFatal( strlen( nm ) < MaxNameSize,
               "TS::Name::Name: name too long" );
         strcpy( (char *)name, nm );
         }

      operator const char * () const
         {
         return name;
         }
      int operator < ( Name const & other ) const
         {
         return strcmp( name, other.name );
         }

      int operator != ( Name const & other ) const
         {
         return strcmp( name, other.name );
         }
      };

   //------------------------------------------------------------------

   class IndexList : public SortableVector<int>
      {
   public:
      void add( IndexList const & list )
         {
         if( size() )
            for( const_iterator item = list.begin(); item != list.end(); item++ )
               add( *item );
         else
            for( const_iterator item = list.begin(); item != list.end(); item++ )
               push_back( *item );
         }
      void add(int item)
         {
         // add only if not already in array:
         if( end() == find( item ) )
            push_back( item );
         }
      };

   //------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option -a.
#pragma option -winl.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


};  // namespace TS

#endif
