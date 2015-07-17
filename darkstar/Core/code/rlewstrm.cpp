//================================================================
//   
// $Workfile:   rlewstrm.cpp  $
// $Version$
// $Revision:   1.2  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================


#include "rlestrm.h"


//----------------------------------------------------------------------------

RLEWStream::RLEWStream()
{
   initialize();
   strm_status = STRM_CLOSED;
}


//----------------------------------------------------------------------------

RLEWStream::RLEWStream(StreamIO& stream)
{
   initialize();
   attach(stream);
}


//----------------------------------------------------------------------------

RLEWStream::~RLEWStream()
{
	detach();
}


//----------------------------------------------------------------------------

void RLEWStream::initialize()
{
   ts = NULL;
   goalByte = byteCount = buffLen = 0;
	curPos  = 0;
}


//----------------------------------------------------------------------------

void RLEWStream::attach(StreamIO &s)
{
   detach();
   ts = &s;
   goalByte = byteCount = buffLen = 0;
	curPos  = 0;
   strm_status = s.getStatus();
}


//----------------------------------------------------------------------------

void RLEWStream::detach()
{
   if (!ts) return;
   flush();
   ts = NULL;
   strm_status = STRM_CLOSED;
}


void RLEWStream::close()
{
   detach();
}

//----------------------------------------------------------------------------

Int32 RLEWStream::getCapabilities(StreamCap cap) const
{
	switch(cap)
	{
		case STRM_CAP_TYPE:
			return STRM_RLE_W;

		case STRM_CAP_WRITE:
		case STRM_CAP_COMPRESS:
			return 1;
	}
	return 0;
}


//----------------------------------------------------------------------------

Int32 RLEWStream::getPosition() const
{
   if (strm_status != STRM_FLTR_DETACHED)
      return curPos;
   else
      return 0;
}


//----------------------------------------------------------------------------

Bool RLEWStream::setPosition(Int32 pos)
{
	if (pos != curPos)
	{
		AssertFatal(0,"RLEWStream::setPosition: Cannot reposition RLEWStream.");
	   strm_status = STRM_ILLEGAL_CALL;
		return FALSE;
	}
	return TRUE;
}


//----------------------------------------------------------------------------

Bool RLEWStream::flush()
{
   if (strm_status != STRM_OK)
      return 0;

   // Bytes in byte count aren't worth compressing, so buffer them before
   // flushing everything.

   if (byteCount < 3)
   {
		// Make sure buffer isn't full yet!

      if (buffLen >= 126)
      {
         if (!ts->write(buffLen) || !ts->write(buffLen,buffer))
			{
				strm_status = ts->getStatus();
				return 0;
			}

         buffLen = 0;
      }
      
		// Buffer byte

      buffer[buffLen++] = goalByte;
      byteCount--;

      if (byteCount)
      {
         buffer[buffLen++] = goalByte;
         byteCount--;
      }
   }

	// Flush the buffer to disk.

   if (buffLen)
      if (!ts->write(buffLen) || ! ts->write(buffLen,buffer))
		{
			strm_status = ts->getStatus();
			return 0;
		}

	// Flush the compressed data to disk.

   if (byteCount)
      if (!ts->write((Int8)(byteCount | 128)) || !ts->write(goalByte))
		{
			strm_status = ts->getStatus();
			return 0;
		}


   byteCount = 0;
   buffLen = 0;

   return 1;
}


//----------------------------------------------------------------------------
// RLE will look for a run of bytes that are the same, if it finds more than
// 2 bytes the same in a row, it will record the number of bytes found, up to
// 127, with the high bit set.  If it finds a series of bytes that are
// different, again up to 127, it will record the number found, and then write
// out the series of different bytes.  So, in an example output file you may
// find (in hex): 84 00 02 01 02 this would uncompress to: 00 00 00 00 01 02,
// i.e. 4 0's and 01 02 <-- series of unrelated data.
//

Bool RLEWStream::write(int size,const void* d)
{
   const Int8* posnPtr = (const Int8*)d;

   if (strm_status != STRM_OK) return FALSE;

	// Check for zero length first.

	if (!size)
		return TRUE;

	//

   if (!byteCount)
   {
      goalByte = *posnPtr;
      posnPtr++;
      byteCount++;
      size--;
		curPos++;
   }

   while(size)
   {
	   // The bytes are the same, keep reading them in and incrementing
	   // the counter.

      if ((goalByte == *posnPtr) && (byteCount < 127))
      {
         byteCount++;
         posnPtr++;
         size--;
			curPos++;
      }  
      else
      {
         // The byteCount is less than three, not enough bytes worth
         // compressing, so buffer the bytes and continue the search.

         if (byteCount < 3)
         {
            if (buffLen >= 126) // make sure buffer isn't full yet!
            {
		         if (!ts->write(buffLen) || !ts->write(buffLen,buffer))
					{
						strm_status = ts->getStatus();
						return FALSE;
					}

		         buffLen = 0;
            }
          
				// Buffer byte

            buffer[buffLen++] = goalByte;

            if ((--byteCount) != 0)
               buffer[buffLen++] = goalByte;
         }

         else
         {   
	         // Byte count is greater than 3, so write out the buffer if there is
	         // one, and then write out the byte count and byte.  Reset the
	         // buffer. 

            if (buffLen)
            {
               if (!ts->write(buffLen) || !ts->write(buffLen,buffer))
					{
						strm_status = ts->getStatus();
						return FALSE;
					}

               buffLen = 0;
            }
   
            if (!ts->write((Int8)(byteCount | 128)) || !ts->write(goalByte))
				{
					strm_status = ts->getStatus();
					return FALSE;
				}
         }

         goalByte  = *posnPtr;
         posnPtr++;
         byteCount = 1;
         size--;
			curPos++;
      }
   }

	return TRUE;
}


//----------------------------------------------------------------------------
Bool RLEWStream::read(int, void*)
{
	AssertFatal(0,"RLEWStream::read: Cannot read from RLEWStream.");
   strm_status = STRM_ILLEGAL_CALL;
	return FALSE;
}

