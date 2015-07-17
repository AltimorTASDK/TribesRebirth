//================================================================
//   
// $Workfile:   lzhrstrm.cpp  $
// $Version$
// $Revision:   1.4  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include "lzhstrm.h"


//---------------------------------------------------------------------------
//	LZSS + Huffman Decompress
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

LZHRStream::LZHRStream()
{
	initialize();
   strm_status = STRM_FLTR_DETACHED;
}


//---------------------------------------------------------------------------

LZHRStream::LZHRStream(StreamIO& s)
{
	initialize();
   attach(s);
}


//---------------------------------------------------------------------------

void LZHRStream::initialize()
{
	d_stream = NULL;
	d_huff   = new LZHuffman;
	text_buf = new unsigned char[LZH_BUFF_SIZE + LZH_LOOKAHEAD - 1];

	AssertFatal(d_huff != 0,"LZHRStream:: Could not create LZHuffman.");
	AssertFatal(text_buf != 0,"LZHRStream:: Could not allocate buffers");

}


//---------------------------------------------------------------------------

LZHRStream::~LZHRStream()
{
   detach();
	delete d_huff;
	delete [] text_buf;
}


//----------------------------------------------------------------------------

void LZHRStream::attach(StreamIO &s)
{
   detach();
   d_stream = &s;
   d_huff->attach(s);

   for (int i = 0; i < LZH_BUFF_SIZE - LZH_LOOKAHEAD; i++)
	   text_buf[i] = ' ';

   DR = LZH_BUFF_SIZE - LZH_LOOKAHEAD;
   DJ = DK = 0;
   curPos = 0;

   strm_status = s.getStatus();
}


//----------------------------------------------------------------------------

void LZHRStream::detach()
{
   if (!d_stream) return;
   flush();
   d_stream = NULL;
   strm_status = STRM_FLTR_DETACHED;
}


void LZHRStream::close()
{
   detach();
}


//----------------------------------------------------------------------------

Int32 LZHRStream::getCapabilities(StreamCap cap) const
{
	switch(cap)
	{
		case STRM_CAP_TYPE:
			return STRM_LZH_R;

		case STRM_CAP_READ:
		case STRM_CAP_REPOSITION:
		case STRM_CAP_COMPRESS:
			return 1;
	}
	return 0;
}


//---------------------------------------------------------------------------

const int StackBufferSize = 500;

Bool LZHRStream::setPosition(Int32 pos)
{
	Int8	buff[StackBufferSize];

   if (strm_status == STRM_FLTR_DETACHED)
      return 0;

	if (pos < curPos)
	{
		// Reset parent stream back to the beginning.

		if (!d_stream->setPosition(0))
		{
			strm_status = d_stream->getStatus();
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


//---------------------------------------------------------------------------

Bool LZHRStream::read(int length,void * data)
{
	int  		c;
	Int32	   count = 0;
   unsigned char *pdata = (unsigned char*)data;

	// Check for stream error and zero length first.
        if (strm_status == STRM_FLTR_DETACHED)
                return 0;
        strm_status = d_stream->getStatus();

	if (strm_status != STRM_OK)
		return 0;

	if (!length)
		return 1;

	//

	while (1)
	{
		// Copy string from the buffer.

		while (DK < DJ)
		{
			c = text_buf[(DI + DK) & (LZH_BUFF_SIZE - 1)];

			*pdata++ = c;

			text_buf[DR++] = c;
			DR &= (LZH_BUFF_SIZE - 1);

			DK++;

			curPos++;
			if (++count == length)
				return 1;
		}

		// Copy individual chars.

		while ((c = d_huff->decodeChar()) < 256)
		{
			*pdata++ = c;

			text_buf[DR++] = c;
			DR &= (LZH_BUFF_SIZE - 1);

			curPos++;
			if (++count == length)
				return 1;
		}

		// Initialize the string copy.

		DI = (DR - d_huff->decodePosition() - 1) & (LZH_BUFF_SIZE - 1);
		DJ = c - 255 + LZH_THRESHOLD;
		DK = 0;
	}
}


//---------------------------------------------------------------------------

Bool LZHRStream::flush()
{
	return (strm_status != STRM_FLTR_DETACHED);
}


//---------------------------------------------------------------------------
Int32 LZHRStream::getPosition() const
{
	if (strm_status != STRM_FLTR_DETACHED)
      return curPos;
   else
      return 0;
}


//---------------------------------------------------------------------------
Bool LZHRStream::write(int, const void*)
{
	AssertFatal(0,"LZHRStream::write: Can on read from LZHRStream.");
   strm_status = STRM_ILLEGAL_CALL;
   return FALSE;
}


