//================================================================
//   
// $Workfile:   lzhwstrm.cpp  $
// $Version$
// $Revision:   1.2  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include "lzhstrm.h"


//---------------------------------------------------------------------------
// LZSS + Huffman Compression
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

LZHWStream::LZHWStream()
{
   initialize();
   strm_status = STRM_FLTR_DETACHED;
}


//---------------------------------------------------------------------------

LZHWStream::LZHWStream(StreamIO& s)
{
   initialize();
   attach(s);
}


//---------------------------------------------------------------------------

LZHWStream::~LZHWStream()
{
	detach();

	delete d_huff;
	delete [] text_buf;
	delete [] lson;
	delete [] rson;
	delete [] dad;
}


//----------------------------------------------------------------------------

void LZHWStream::initialize()
{
	d_stream = NULL;
	d_huff   = new LZHuffman;
	text_buf = new unsigned char[LZH_BUFF_SIZE + LZH_LOOKAHEAD - 1];
	lson = new short[LZH_BUFF_SIZE + 1];
	rson = new short[LZH_BUFF_SIZE + 257];
	dad  = new short[LZH_BUFF_SIZE + 1];

	AssertFatal(d_huff != 0,"LZHWStream:: Could not create LZHuffman.");
	AssertFatal(text_buf != 0 && lson != 0 && rson != 0 && dad != 0,
		"LZHWStream:: Could not allocate buffers");
}


//----------------------------------------------------------------------------

void LZHWStream::attach(StreamIO &s)
{
   detach();
   d_stream = &s;
   strm_status = s.getStatus();

   d_huff->attach(s);

	int  i;

	// Initialize the LZ tree.

	for (i = LZH_BUFF_SIZE + 1; i <= LZH_BUFF_SIZE + 256; i++)
		rson[i] = LZH_NIL;			   /* root */
	for (i = 0; i < LZH_BUFF_SIZE; i++)
		dad[i] = LZH_NIL;			      /* node */

	//

	ES = 0;
	ER = LZH_BUFF_SIZE - LZH_LOOKAHEAD;

	for (i = ES; i < ER; i++)
		text_buf[i] = ' ';

	EFULL = 0;
	ELEN = 0;
	ELM = ELF = 0;
	curPos = 0;
   iFileSize = 0;

}


//----------------------------------------------------------------------------

void LZHWStream::detach()
{
   if (!d_stream) return;
   flush();
   d_stream = NULL;
   strm_status = STRM_FLTR_DETACHED;
}


void LZHWStream::close()
{
   detach();
}

//----------------------------------------------------------------------------

Int32 LZHWStream::getCapabilities(StreamCap cap) const
{
	switch(cap)
	{
		case STRM_CAP_TYPE:
			return STRM_LZH_W;

		case STRM_CAP_WRITE:
		case STRM_CAP_COMPRESS:
			return 1;
	}
	return 0;
}


//---------------------------------------------------------------------------
// Insert to tree

void LZHWStream::insertNode(int r)
{
	int  i, p, cmp, c;
	unsigned char  *key;

	cmp = 1;
	key = &text_buf[r];
	p = LZH_BUFF_SIZE + 1 + key[0];
	rson[r] = lson[r] = LZH_NIL;
	match_length = 0;
	for ( ; ; ) {
		if (cmp >= 0) {
			if (rson[p] != LZH_NIL)
				p = rson[p];
			else {
				rson[p] = r;
				dad[r] = p;
				return;
			}
		} else {
			if (lson[p] != LZH_NIL)
				p = lson[p];
			else {
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}
		for (i = 1; i < LZH_LOOKAHEAD; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)
				break;
		if (i > LZH_THRESHOLD) {
			if (i > match_length) {
				match_position = ((r - p) & (LZH_BUFF_SIZE - 1)) - 1;
				if ((match_length = i) >= LZH_LOOKAHEAD)
					break;
			}
			if (i == match_length) {
				if ((c = ((r - p) & (LZH_BUFF_SIZE - 1)) - 1) < match_position) {
					match_position = c;
				}
			}
		}
	}
	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];
	dad[lson[p]] = r;
	dad[rson[p]] = r;
	if (rson[dad[p]] == p)
		rson[dad[p]] = r;
	else
		lson[dad[p]] = r;
	dad[p] = LZH_NIL;  // Remove p
}


//---------------------------------------------------------------------------
// Remove from tree

void LZHWStream::deleteNode(int p)
{
	int  q;

	if (dad[p] == LZH_NIL)
		return;				// Not registered
	if (rson[p] == LZH_NIL)
		q = lson[p];
	else
	if (lson[p] == LZH_NIL)
		q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != LZH_NIL) {
			do {
				q = rson[q];
			} while (rson[q] != LZH_NIL);
			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}
		rson[q] = rson[p];
		dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p)
		rson[dad[p]] = q;
	else
		lson[dad[p]] = q;
	dad[p] = LZH_NIL;
}


//---------------------------------------------------------------------------

Bool LZHWStream::write(int length, const void *data)
{
	unsigned char cc;
   const unsigned char *pdata = (const unsigned char*)data;

   iFileSize += length;

   if (strm_status == STRM_FLTR_DETACHED)
      return 0;

	// Check for zero length first.

	if (!length)
		return 1;

	// Fill up the buffer first.

	if (!EFULL)
	{
		while (ELEN < LZH_LOOKAHEAD)
		{
			text_buf[ER + ELEN++] = *pdata++;

			curPos++;
			if (!--length)
				return 1;
		}

		// Once the buffer is full, call InsertNode for
		// all the data.

		for (int i = 1; i <= LZH_LOOKAHEAD; i++)
			insertNode(ER - i);
		insertNode(ER);

		EFULL = 1;
	}

	// Output the compressed data.

	while (1)
	{
		// If we are not in the middle of refilling the buffer
		// then write out the current match position.

		if (ELF >= ELM)
		{
			if (match_length > ELEN)
				match_length = ELEN;

			if (match_length <= LZH_THRESHOLD)
			{
				match_length = 1;
				d_huff->encodeChar(text_buf[ER]);
			}
			else
			{
				d_huff->encodeChar(255 - LZH_THRESHOLD + match_length);
				d_huff->encodePosition(match_position);
			}

			// Set up the refill variables.

			ELF = 0;
			ELM = match_length;
		}

		// Replace the bytes matched previously.

		while (ELF++ < ELM)
		{
			cc = *pdata++;

			deleteNode(ES);
			text_buf[ES] = cc;

			if (ES < LZH_LOOKAHEAD - 1)
				text_buf[ES + LZH_BUFF_SIZE] = cc;

			ES = (ES + 1) & (LZH_BUFF_SIZE - 1);
			ER = (ER + 1) & (LZH_BUFF_SIZE - 1);

			insertNode(ER);

			curPos++;
			if (!--length)
				return 1;
		}
	}
}


//--------------------------------------------------------------------------

Bool LZHWStream::flush()
{
	if (strm_status == STRM_FLTR_DETACHED)
		return 0;

	if (!EFULL)
	{
		// The buffer was never filled, so call InsertNode for
		// whatever is there.

		for (int i = 1; i <= LZH_LOOKAHEAD; i++)
			insertNode(ER - i);
		insertNode(ER);
	}

	while (ELEN)
	{
		// If we are not in the middle of refilling the buffer
		// then write out the current match position.

		if (ELF >= ELM)
		{
			if (match_length > ELEN)
				match_length = ELEN;

			if (match_length <= LZH_THRESHOLD)
			{
				match_length = 1;
				d_huff->encodeChar(text_buf[ER]);
			}
			else
			{
				d_huff->encodeChar(255 - LZH_THRESHOLD + match_length);
				d_huff->encodePosition(match_position);
			}

			// Set up the refill variables.

			ELM = match_length;
			ELF = 0;
		}

		while (ELF++ < ELM)
		{
			deleteNode(ES);

			ES = (ES + 1) & (LZH_BUFF_SIZE - 1);
			ER = (ER + 1) & (LZH_BUFF_SIZE - 1);

			curPos++;
			if (--ELEN)
				insertNode(ER);
		}
	}

	// Flush out the huffman stream.

	d_huff->flush();

	// Cannote write to the stream after it has been flushed. so detach.

   d_stream = NULL;
   strm_status = STRM_FLTR_DETACHED;

	return 1;
}


//---------------------------------------------------------------------------

Int32 LZHWStream::getPosition() const
{
   return iFileSize;
   /*
	if (strm_status != STRM_FLTR_DETACHED)
      return curPos;
   else
      return 0;
   */
}


//----------------------------------------------------------------------------

Bool LZHWStream::setPosition(Int32 pos)
{
	if (pos != curPos)
	{
		AssertFatal(0,"LZHWStream::setPosition: Cannot reposition LZHWStream.");
	   strm_status = STRM_ILLEGAL_CALL;
		return 0;
	}
	return 1;
}


//----------------------------------------------------------------------------

Bool LZHWStream::read(int, void*)
{
	AssertFatal(0,"LZHWStream::read: Can on write to LZHWStream.");
   strm_status = STRM_ILLEGAL_CALL;
	return FALSE;
}


