//----------------------------------------------------------------------------
//
//	$Workfile:   memstrm.cpp  $
//	$Revision:   1.2  $
//	$Version:$
//	$Date:   15 Sep 1995 14:58:58  $  
//	
//----------------------------------------------------------------------------
//
// Notes:
// -----
//
//
//    Write() routines will write to anywhere it is positioned in the given
//  block.  If there is not enough room left to write the amount of data
//  given, it will write as much as possible, unless it is a single item.
//
//    Read() may read from anywhere within the used part of the block.  If it
//  tries to read more than the used part of the block, it will read as much 
//  as possible, unless it is a single item. 
//
//  **NOTE: When a status error occurs, no I/O is permitted until the status
//  error is cleared.  Write() and read() will return FALSE when there is an
//  error or when the status is set to something other than STRM_OK.
//
//    Both functions attempt to mimic fwrite() and fread() as close as
//  possible.  "Single item" means it will not read/write 1 byte of a 2 byte
//  number, for example.
//
//
//----------------------------------------------------------------------------


#include <string.h>
#include <base.h>
#include "memstrm.h"


//----------------------------------------------------------------------------

MemRWStream::MemRWStream()
{
	close();
}

MemRWStream::MemRWStream(UInt32 totalSize, void* frontPtr )
{
	open(totalSize, frontPtr);
}


void MemRWStream::open(UInt32 totalSize, void* frontPtr )
{
   mFrontPtr 	= (char*) frontPtr;
   mEndPtr  	= (char*) frontPtr + totalSize;
   mPosnPtr 	= (char*) frontPtr;
	strm_status	= STRM_OK;
   lockable    = true;
}

void MemRWStream::close()
{
   mFrontPtr 	= mEndPtr = mPosnPtr = NULL;
	strm_status	= STRM_CLOSED;
   lockable    = false;
}


//----------------------------------------------------------------------------
// The flag argument indicates the current operation
//		1 = write.
//		0 = read.
// flag was needed becuase one does not get an EOS when writing 
// to a file; at worst a DISK_FULL error, but not EOS.
// Always returns false

Bool MemRWStream::setStatus(Int8 flag)
{
   if (mPosnPtr == mEndPtr)
      if (flag)
         strm_status = STRM_IO_ERROR;
      else
         strm_status = STRM_EOS;
   else
      strm_status = STRM_IO_ERROR;

   return 0;
}


//----------------------------------------------------------------------------

Int32 MemRWStream::getCapabilities(StreamCap cap) const
{
	switch(cap)
	{
		case STRM_CAP_TYPE:
			return STRM_MEM_RW;

		case STRM_CAP_WRITE:
		case STRM_CAP_READ:
		case STRM_CAP_REPOSITION:
			return 1;

      case STRM_CAP_LOCKABLE:
         return (lockable);
	}
	return 0;
}


//----------------------------------------------------------------------------
// Return position of "read/write" head.

Int32 MemRWStream::getPosition() const
{
   return (mPosnPtr - mFrontPtr);
}


//----------------------------------------------------------------------------

Bool MemRWStream::setPosition(Int32 pos)
{
   if (strm_status == STRM_EOS)
      strm_status = STRM_OK;

   if (strm_status != STRM_OK)
      return 0;

   if (pos < 0 || pos > mEndPtr - mFrontPtr) 
      return setStatus(0);

	mPosnPtr = (mFrontPtr + pos);

   return 1;
}


//----------------------------------------------------------------------------

Bool MemRWStream::flush()
{
   return 1;
}


//----------------------------------------------------------------------------

Bool MemRWStream::write(int size,const void* d)
{
   if (strm_status != STRM_OK)
      return 0;

	// Find out how room we have to write to.

   int freSize = mEndPtr - mPosnPtr;

   if (size > freSize)
	{
	   memcpy(mPosnPtr, d, freSize);
		mPosnPtr += freSize;
      return setStatus(1);
	}

   memcpy(mPosnPtr, d, size);
	mPosnPtr += size;

   return 1;
}


//----------------------------------------------------------------------------

Bool MemRWStream::read(int size,void* d)
{
   if (strm_status != STRM_OK)
      return false;
	if (!size)
		return true;

   int rdSize = mEndPtr - mPosnPtr;

   if (size > rdSize)
	{
	   memcpy(d, mPosnPtr, rdSize);
	   mPosnPtr += rdSize;
      return setStatus(0);
	}

	AssertFatal(d,"MemRWStream::read: Null pointer");
   memcpy(d, mPosnPtr, size);
   mPosnPtr += size;
   
   return true;
}


//------------------------------------------------------------------------------
void* MemRWStream::lock()
{
   return ((lockable && strm_status == STRM_OK) ? mPosnPtr : NULL);
}   
