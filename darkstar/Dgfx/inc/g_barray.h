//================================================================
//   
// $Workfile:   g_barray.h  $
// $Version$
// $Revision:   1.2  $
//   
// DESCRIPTION:
//   Bitamp Array Container class
//   
//   GFXBitampArray provides a simple container for a list of bitmaps.
//   
//   Features:
//     optional index remapping:  if *indexRemap list exists the bitmaps
//       in the array can be referenced like a sparce array.
//   
//     optional associated user data: You can associate data with each bitamp
//       in the array.  Might be useful for animations, fonts, etc.
//       just set *userData and userDataSize to use.
//   
//     The load/save routines will preserve user and remap data.
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _G_BARRAY_H_
#define _G_BARRAY_H_


#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//Bitamp BMF_* flags are also valid for bitmap arrays
#define BAF_REMAP       (1<<31)
#define BAF_USERDATA    (1<<30)
#define BAF_FLATTEN_REMAP (1<<29)

class GFXBitmap;
class StreamIO;

class GFXBitmapArray
{
protected:
   Int32 getRealIndex(Int32 in_index) const;

public:
   GFXBitmap **array;         //pointer to a list of GFXBitmap pointers
   BYTE      *userData;       //pointer to a block of user defined data (optional)
   Int32     *indexRemap;     //pointer to an index remap table (optional)
   Int32     numBitmaps;            //number of actual bitamps in the array
   Int32     userDataSize;    //size of each userData record in bytes

   GFXBitmapArray();   
   ~GFXBitmapArray();

   GFXBitmap*  getBitmap(Int32 in_index) const;
   GFXBitmap*	operator[](Int32 in_index) const;
   void        setBitmap(Int32 in_index, GFXBitmap *in_pbm);
   void*       getUserData(Int32 in_index) const;
   inline Int32        getCount() const;

   void   free();

   static GFXBitmapArray* load(const char *in_filename, DWORD in_flags);
   static GFXBitmapArray* load(StreamIO *io_stream, DWORD in_flags);

   Bool   read(StreamIO *io_stream, DWORD in_flags);
   Bool   read(const char *in_filename, DWORD in_flags);

   Bool   write(const char *in_filename, DWORD in_flags);
   Bool   write(StreamIO *io_stream, DWORD in_flags);

   DWORD  getSaveSize();
   void   flattenRemap();
};

inline Int32 GFXBitmapArray::getCount() const
{ 
   return numBitmaps;
}

inline GFXBitmap* GFXBitmapArray::getBitmap(Int32 in_index) const
{
   if(!indexRemap)
      return array[in_index];
   else
   {
      int ndx = getRealIndex(in_index);
      if (ndx == -1)
         return (NULL);
      else
         return( array[ndx] );
   }
}

inline GFXBitmap* GFXBitmapArray::operator[](Int32 in_index) const
{
   return getBitmap(in_index);
}

inline void GFXBitmapArray::setBitmap(Int32 in_index, GFXBitmap *in_pbm)
{
   array[getRealIndex(in_index)] = in_pbm;
}   

inline void* GFXBitmapArray::getUserData(Int32 in_index) const
{
   if(!indexRemap)
      return userData + in_index * userDataSize;
   else
   {
      int ndx = getRealIndex(in_index);
      if (ndx == -1)
         return (NULL);
      else
         return ( userData + (ndx * userDataSize) );
   }
}


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_G_BARRAY_H_
