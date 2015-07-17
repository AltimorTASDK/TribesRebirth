//================================================================
//   
// $Workfile:   rlerstrm.cpp  $
// $Version$
// $Revision:   1.2  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================


#include "rlestrm.h"


//----------------------------------------------------------------------------

RLERStream::RLERStream()
{
   initialize();
   strm_status = STRM_FLTR_DETACHED;
}


//----------------------------------------------------------------------------

RLERStream::RLERStream(StreamIO& stream)
{
   initialize();
   attach(stream);
}


//----------------------------------------------------------------------------

void RLERStream::initialize()
{
   ts = NULL;
   goalByte = byteCount = buffLen = 0;
	curPos  = 0;
}


//----------------------------------------------------------------------------

void RLERStream::attach(StreamIO &s)
{
   detach();
   ts = &s;
   goalByte = byteCount = buffLen = 0;
	curPos  = 0;
   strm_status = s.getStatus();
}


//----------------------------------------------------------------------------

void RLERStream::detach()
{
   if (!ts) return;
   flush();
   ts = NULL;
   strm_status = STRM_FLTR_DETACHED;
}


void RLERStream::close()
{
   detach();
}

//----------------------------------------------------------------------------

Int32 RLERStream::getCapabilities(StreamCap cap) const
{
	switch(cap)
	{
		case STRM_CAP_TYPE:
			return STRM_RLE_R;

		case STRM_CAP_READ:
		case STRM_CAP_COMPRESS:
		case STRM_CAP_REPOSITION:
			return 1;
	}
	return 0;
}


//----------------------------------------------------------------------------

Int32 RLERStream::getPosition() const
{
	if (strm_status != STRM_FLTR_DETACHED)
      return curPos;
   else
      return 0;
}


//----------------------------------------------------------------------------

const int StackBufferSize = 500;

Bool RLERStream::setPosition(Int32 pos)
{
	Int8	buff[StackBufferSize];

   if (strm_status != STRM_OK)
      return 0;

	if (pos < curPos)
	{
		// Reset parent stream back to the beginning.

		if (!ts->setPosition(0))
		{
			strm_status = ts->getStatus();
			return 0;
		}

		curPos = 0;
	}
	else
		pos -= curPos;

	// Read untill we get to the desired position.

	for (int i = pos / StackBufferSize; i > 0; --i)
		read(StackBufferSize,buff);

	return read(pos % StackBufferSize,buff);
}


//----------------------------------------------------------------------------

Bool RLERStream::flush()
{
	return (strm_status != STRM_FLTR_DETACHED);
}


//----------------------------------------------------------------------------

Bool RLERStream::write(int, const void*)
{
	AssertFatal(0,"RLERStream::write: Cannot write to RLERStream.");
   strm_status = STRM_ILLEGAL_CALL;
	return FALSE;
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

Bool RLERStream::read(int size, void* d)
{
   Int8* posnPtr = (Int8*)d;  // position pointer into given array
   Int8  tlen;                // transfer length

	// Check error status and for zero length first.

	if (strm_status != STRM_OK)
		return FALSE;

	if (!size)
		return TRUE;

   while(1)
   {
		// Read data either from the current run of compressed
		// or uncompressed data. A buffLen of zero indicates a
		// compressed run.

      if (!buffLen)
         for (; byteCount && size; byteCount--,size--,curPos++)
            *posnPtr++ = goalByte;
      else
      {
			// Read uncompressed runs directly from the
			// source stream.

         tlen = (buffLen < size) ? buffLen : size;

         if (!ts->read(tlen,posnPtr))
			{
				strm_status = ts->getStatus();
				return FALSE;
			}

         buffLen  -= tlen;
         size     -= tlen;
         posnPtr  += tlen;
			curPos   += tlen;
      }

		// See if we are done, if not read in the next run.

      if (!size)
			return TRUE;

		if (!ts->read(&byteCount))
		{
			strm_status = ts->getStatus();
			return FALSE;
		}
		//curPos++;

		// High bit indicates a repeat byte run.

      if (byteCount & 128)
      {
			// Extract the count, and read in the repeat byte.

         byteCount &= 127;

         if (!ts->read(&goalByte))
			{
				strm_status = ts->getStatus();
				return FALSE;
			}
			curPos++;
      }
      else
      {
			// The following data is uncompressed,
			// so set the buffer length.

         buffLen = byteCount;
         byteCount = 0;
      }
   }
}

