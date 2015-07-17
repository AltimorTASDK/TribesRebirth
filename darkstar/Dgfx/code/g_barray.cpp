//================================================================
//   
// $Workfile:   g_barray.cpp  $
// $Version$
// $Revision:   1.0  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================


#include <filstrm.h>
#include "g_barray.h"
#include "g_bitmap.h"
#include "resManager.h"

struct Header
{
   DWORD ver_nc;
   DWORD numBitmaps;
};

//increment high byte if bitmap array format becomes incompatible with
//the old reader.
#define BMARRAY_VERSION 0x00000000


//================================================================
// NAME
//   GFXBitmapArray::GFXBitmapArray
//   
// DESCRIPTION
//   Constructor
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   initialized, empty, GFXBitamp array.
//   
// NOTES 
//   
//================================================================
GFXBitmapArray::GFXBitmapArray()
{
   array       = NULL;
   userData    = NULL;
   indexRemap  = NULL;
   numBitmaps  = 0;
   userDataSize= 0;
}


//================================================================
// NAME
//   GFXBitmapArray::~GFXBitmapArray
//   
// DESCRIPTION
//   Destructor
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   The GFXBitampArray ASSUMES all the data it holds it is responsible
//   for, do it deletes it all.  Make sure you remove and zero out
//   things you don't want deleted before deleting the GFXBitampArray.
//   
//================================================================
GFXBitmapArray::~GFXBitmapArray()
{
   free();
}   


//================================================================
// NAME
//   GFXBitmapArray::free
//   
// DESCRIPTION
//   
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   
//   
// NOTES 
//   
//================================================================
void GFXBitmapArray::free()
{
   if (array)
   {
      GFXBitmap **temp = array;
      for (Int32 ndx=0; ndx<numBitmaps; ndx++, temp++)
      {   
        if (*temp)
         {
            delete *temp;
            *temp = NULL;
         }
      }
      delete [] array;
      array = NULL;
   }
   if (userData)
   {
      delete [] userData;
      userData = NULL;
   }
   if (indexRemap) 
   {
      delete [] indexRemap;
      indexRemap = NULL;
   }
}


//================================================================
// NAME
//   GFXBitmapArray::getRealIndex
//   
// DESCRIPTION
//   Given a bitmap index return its real index in the array.
//   If a indexReamp table exists convert the passed index
//   into the actual index in the array.
//   Otherwise simply return in_index.
//   
// ARGUMENTS 
//   in_index  - bitmap index to remap
//   
// RETURNS 
//   a valid array index
//   OR -1 if index does not exist
//   
// NOTES 
//   This preforms a classic binary search on a sorted indexReamap
//   table.    
//================================================================
Int32 GFXBitmapArray::getRealIndex(Int32 in_index) const
{
   register Int32  lo, hi, index;
   Int32   val;

   //if there is no remap table return in_index
   if (!indexRemap) return (in_index);
   AssertFatal( in_index >= 0, "GFXBitmapArray::getRealIndex: in_index out of range");

   lo = 0;                                //head of array
   hi = numBitmaps - 1;                   //tail of array
   
   while (lo <= hi)
   {
      index = (lo+hi) >> 1;               //get midpoint, round down
      val = indexRemap[index];
      if (in_index == val) return(index); //FOUND, outta here
      else                                //no, we'll lets zero in on it
      {   
      	if (in_index < val) 
      		hi = index-1;
         else lo = index+1;
   	}
   }
   //if we get here we could not locate the requested index
   //AssertMessage(val == in_index, avar("GFXBitmapArray::getRealIndex: Requested index (%d) does not exist in array", in_index));
   return(-1); 
}   


//================================================================
// NAME
//   GFXBitmapArray::load
//   
// DESCRIPTION
//   
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   
//   
// NOTES 
//   
//================================================================
GFXBitmapArray* GFXBitmapArray::load(const char *in_filename, DWORD in_flags)
{
   GFXBitmapArray *bma = new GFXBitmapArray;
   if (bma)
   {
      if (bma->read(in_filename, in_flags))
         return bma;
      else
         delete bma;
   }
   return (NULL);
}


//================================================================
// NAME
//   GFXBitmapArray::load
//   
// DESCRIPTION
//   
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   
//   
// NOTES 
//   
//================================================================
GFXBitmapArray* GFXBitmapArray::load(StreamIO *in_stream, DWORD in_flags)
{
   GFXBitmapArray *bma = new GFXBitmapArray;
   if (bma)
   {
      if (bma->read(in_stream, in_flags))
         return bma;
      else
         delete bma;
   }
   return (NULL);
}  

  
//================================================================
// NAME
//   GFXBitmapArray::read
//   
// DESCRIPTION
//   
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   
//   
// NOTES 
//   
//================================================================
Bool GFXBitmapArray::read(const char *in_filename, DWORD in_flags)
{
   Bool   result;
   HANDLE fileHandle;
   fileHandle = CreateFile(in_filename,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL
                           );
   if (fileHandle == INVALID_HANDLE_VALUE)
   {
      AssertWarn(0, avar("GFXBitmapArray::load: Unable to open file %s, CreateFile for reading failed.", in_filename));
      return (FALSE);
   }

   FileRStream frs(fileHandle);
   result = read(&frs, in_flags);
   frs.close();
   CloseHandle(fileHandle);

   return(result);
}


//================================================================
// NAME
//   GFXBitmapArray::read
//   
// DESCRIPTION
//   Read a bitmap array and it associated parts into the current
//   GFXBitmap array object.  
//   
//   The contents of the object freed first then the new object is
//   read in from the stream.
//   
// ARGUMENTS 
//   in_stream - Read stream positioned at a GFXBitampArray
//   in_flags  - BAF_* flags
//   
// RETURNS 
//   TRUE  if the array read successfully.  The stream is now 
//         positioned on the byte following the array.
//   
//   FALSE if the read failed.  The stream position is unknown.
//   
// NOTES 
//   
//================================================================
Bool GFXBitmapArray::read(StreamIO *in_stream, DWORD in_flags)
{
   DWORD    data;
   DWORD    size;
   Int32    bmindex = 0;
   Header   header;
   Int32    numChunks = -1;

   free();                             //release current array contents

   while (numChunks--)
   {
      in_stream->read(&data);
      in_stream->read(&size);
      switch (data)
      {
         case FOURCC('P','B','M','A'):
            break;

         case FOURCC('h','e','a','d'):
            AssertFatal(sizeof(header) == size, "GFXBitmapArray::read: Header size different than expected.");
            in_stream->read(size, (BYTE*)&header);
            numChunks= header.ver_nc & 0x00ffffff;    //mask off the version
            AssertFatal((header.ver_nc & 0xff000000) == BMARRAY_VERSION, "GFXBitmapArray::read: incorrect file version.");
            numBitmaps = header.numBitmaps;
            array = new GFXBitmap*[numBitmaps];
            if (!array)
            {
               AssertWarn(array, avar("GFXBitmapArray::read: new failed on %d bytes", size));
               free();
               return (FALSE);
            }
            break;

         case FOURCC('u','s','e','r'):
            userData = new BYTE[size];
            if (!userData)
            {
               AssertWarn(userData, avar("GFXBitmapArray::read: new failed on %d bytes", size));
               free();
               return (FALSE);
            }
            in_stream->read(size, (BYTE*)userData);   
            userDataSize = size/numBitmaps;
            break;

         case FOURCC('r','m','a','p'):
            indexRemap = (Int32*) new BYTE[size];
            if (!indexRemap)
            {
               AssertWarn(indexRemap, avar("GFXBitmapArray::read: new failed on %d bytes", size));
               free();
               return (FALSE);
            }
            in_stream->read(size, (BYTE*)indexRemap);   
            break;

         case FOURCC('P','B','M','P'):
            AssertFatal(bmindex < numBitmaps, "GFXBitmapArray::read: more bitmaps in array file than header claimed.");
            array[bmindex] = GFXBitmap::load(in_stream, in_flags);
            bmindex++;
            break;

         default:
            if (numChunks != -1)
               in_stream->setPosition(in_stream->getPosition()+size);
            else
            {
               AssertWarn(0, "GFXBitmapArray::read: not a GFXBitmapArray.");
               free();                    //clean up our mess
               return (FALSE);
            }
            break;
      }
   }
   AssertFatal(bmindex == numBitmaps, "GFXBitmapArray::read: bitmaps in array file does not match header.");
   if(in_flags & BAF_FLATTEN_REMAP)
   {
      flattenRemap();
   }
   return (TRUE);
}

void GFXBitmapArray::flattenRemap(void)
{   
   if(indexRemap)
   {
      Int32 maxIndex = indexRemap[numBitmaps-1];
      BYTE *newUserData = new BYTE[(maxIndex+1) * userDataSize];
      GFXBitmap **newArray = new GFXBitmap*[maxIndex+1];
      Int32 walk = 0;
      Int32 i;
      for(i = 0; i <= maxIndex; i++)
      {
         if(indexRemap[walk] == i)
         {
            newArray[i] = array[walk];
            memcpy(newUserData + i * userDataSize, userData + walk * userDataSize,
                   userDataSize);
            walk++;
         }
         else
         {
            newArray[i] = NULL;
         }
      }
      delete[] userData;
      delete[] indexRemap;
      delete[] array;
      array = newArray;
      userData = newUserData;
      indexRemap = NULL;
   }
}


//================================================================
// NAME
//   GFXBitmapArray::write
//   
// DESCRIPTION
//   
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   
//   
// NOTES 
//   
//================================================================
Bool GFXBitmapArray::write(const char *in_filename, DWORD in_flags)
{
   FileWStream fws;
   if (fws.open(in_filename) == false) {
      AssertWarn(0, avar("GFXBitmapArray::write: failed to open file %s", in_filename));
      return false;
   }

   Bool result = write(&fws, in_flags);
   fws.close();

   return(result);
}


//================================================================
// NAME
//   GFXBitmapArray::write
//   
// DESCRIPTION
//   write the bitmap array to the stream.
//   
// ARGUMENTS 
//   in_stream - a writable file stream
//   in_flags  - BAF_* flags
//   
// RETURNS 
//   TRUE  if array written successfully
//   FALSE if error writing array (stream position may be modified)
//   
// NOTES 
//   
//   
// FILE FORMAT  
//   
//================================================================
Bool GFXBitmapArray::write(StreamIO *in_stream, DWORD in_flags)
{
   Header header;

   header.ver_nc = 
      BMARRAY_VERSION + 
      numBitmaps +
      ((userData) ? 1 : 0) +
      ((indexRemap) ? 1 : 0);
   header.numBitmaps = numBitmaps;

   in_stream->write(FOURCC('P','B','M','A'));      // PBMA:sz
   in_stream->write((DWORD)getSaveSize());

   in_stream->write(FOURCC('h','e','a','d'));      // head:sz
   in_stream->write((DWORD)sizeof(Header));
   in_stream->write((DWORD)sizeof(Header), (BYTE*)&header);

   if (userData)
   {
      in_stream->write(FOURCC('u','s','e','r'));   // user:sz
      in_stream->write((DWORD)(numBitmaps*userDataSize));
      in_stream->write((numBitmaps*userDataSize), (BYTE*)userData);
   }

   if (indexRemap)
   {
      in_stream->write(FOURCC('r','m','a','p'));   // rmap:sz
      in_stream->write((DWORD)(numBitmaps*sizeof(DWORD)));
      in_stream->write((numBitmaps*sizeof(DWORD)), (BYTE*)indexRemap);
   }

   for (Int32 ndx = 0; ndx<numBitmaps ; ndx++ )   // PBMP:sz
   {
      AssertFatal(array[ndx], "GFXBitmapArray::write: NULL bitmap pointer in array.");
      array[ndx]->write(in_stream, in_flags);
   }

   return(in_stream->getStatus() == STRM_OK);
}


//================================================================
// NAME
//   GFXBitmapArray::getSaveSize
//   
// DESCRIPTION
//   get the size of the PBMA chunk on disk
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   size of the PBMA chunk
//   
// NOTES 
//   assumes if indexRemap and userData exist they will be written to disk
//   
//================================================================
DWORD GFXBitmapArray::getSaveSize()
{
   //magic number 8 is size of chunk_ID and chunk_size
   DWORD size;
   size = 8 + sizeof(Header);
   if (indexRemap) size += (8 + (numBitmaps * sizeof(DWORD)) );
   if (userData)   size += (8 + (numBitmaps * userDataSize) );

   for (Int32 ndx = 0; ndx<numBitmaps ; ndx++ )
   {
      AssertFatal(array[ndx], "GFXBitmapArray::getSaveSize: NULL bitmap pointer in array.");
      size += array[ndx]->getSaveSize();
   }

   return (size);
}
