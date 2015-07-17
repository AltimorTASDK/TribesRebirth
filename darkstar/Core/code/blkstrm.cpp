//----------------------------------------------------------------------------

//	$Workfile:   blkstrm.cpp  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   25 Oct 1995 13:59:42  $

//	The block actually keeps track of it's current position in
//	the parent stream.  This could be removed to simplify the
//	code and probably increase performance.  Leaving it in allows
//	you to have multiple blocks open, and being acccessed at the
//	same time, on the same stream.

//	There is a known problem with declaring a block size larger than
//	what you write.  If the block is the last block in the file and
//	you do not write the total number of bytes, the block will come
//	only contain the bytes you wrote.  The block code seeks to the
//	correct location, but unless something is written, the file
//	system ignores it.  If there is a following block, no problem.

//	Initial Version: timg

//----------------------------------------------------------------------------

#include <blkstrm.h>

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

BlockStream::BlockStream()
{
	stream = 0;
	startPos = 0;
	currentPos = 0;
	strm_status = STRM_CLOSED;
}

BlockStream::~BlockStream()
{
}

//----------------------------------------------------------------------------

Int32 BlockStream::getCapabilities(StreamCap cap) const
{
	AssertFatal(stream != 0, "BlockStream::getCapabilities: Stream not open");
	return stream->getCapabilities(cap);
}


//----------------------------------------------------------------------------

Int32 BlockStream::getPosition() const
{
	AssertFatal(stream != 0,
		"BlockStream::getPosition: Stream not open");
	if (strm_status != STRM_OK && strm_status != STRM_EOS)
		return 0;
	// Pos is relative to the start of the block
	return currentPos - startPos - sizeof(Header);
}


Bool BlockStream::setPosition(Int32 pos)
{
	AssertFatal(stream != 0,
		"BlockStream::setPosition: Stream not open");
	if (strm_status != STRM_OK && strm_status != STRM_EOS)
		return false;
	if (pos < 0) {
		AssertFatal(0,"BlockStream::setPosition: Negative position");
		strm_status = STRM_ILLEGAL_CALL;
		return false;
	}
   Int32 dataSize = getDataSize();
	if (dataSize && (pos > dataSize) )
	{
		pos = dataSize;
		strm_status = STRM_EOS;
	}

	// Pos is relative to the start of the block
	pos += startPos + sizeof(Header);
	if (pos != currentPos) 
	{
		if (!stream->setPosition(pos)) 
		{
			currentPos = stream->getPosition();
			strm_status = STRM_UNKNOWN_ERROR;
			return false;
		}
		strm_status = stream->getStatus();
		currentPos = pos;
	}
	return true;
}


//----------------------------------------------------------------------------
Bool BlockStream::flush()
{
	AssertFatal(stream != 0,
		"BlockStream::flush: Stream not open");
	if (strm_status != STRM_OK && strm_status != STRM_EOS)
		return false;
	if (!stream->flush()) {
		strm_status = STRM_UNKNOWN_ERROR;
		return false;
	}
	return true;
}


//------------------------------------------------------------------------------
void* BlockStream::lock()
{
   if (stream) return (stream->lock());   
   return (NULL);
}   


//----------------------------------------------------------------------------
Bool BlockStream::write(int size,const void* d)
{
   AssertFatal(stream != 0,
		"BlockStream::write: Stream not open");
	if (strm_status != STRM_OK)
		return false;

	// Make sure the stream is where it's supposed to be
	// This could be time consuming
	if (stream->getPosition() != currentPos)
		if (!stream->setPosition(currentPos)) 
		{
			strm_status = STRM_UNKNOWN_ERROR;
			return false;
		}

   Int32 dataSize = getDataSize();
	if (dataSize && (currentPos + size > startPos + dataSize + (int)sizeof(Header)) ) 
	{
		AssertFatal(0,"BlockStream::write: Write passed end of block");
		strm_status = STRM_EOS;
		if ((size = startPos + dataSize + sizeof(Header) - currentPos) <= 0) 
		{
			lastBytes = 0;
			return false;
		}
	}
	if (!stream->write(size,d)) 
	{
		currentPos = stream->getPosition();
		strm_status = STRM_UNKNOWN_ERROR;
		lastBytes = stream->getLastSize();
		return false;
	}
	currentPos += size;
	lastBytes = size;
	return true;
}


//----------------------------------------------------------------------------
Bool BlockStream::read(int size,void* d)
{
	AssertFatal(stream != 0,
		"BlockStream::read: Stream not open");

	// Make sure the stream is where it's supposed to be
	// This could be time consuming
	if (stream->getPosition() != currentPos)
		if (!stream->setPosition(currentPos)) {
			strm_status = STRM_UNKNOWN_ERROR;
			return false;
		}

   Int32 dataSize = getDataSize();
	if (dataSize && currentPos + size > startPos + dataSize + (int)sizeof(Header)) 
	{
		strm_status = STRM_EOS;
		if ((size = startPos + dataSize + sizeof(Header) - currentPos) <= 0) {
			lastBytes = 0;
			return false;
		}
	}
	if (!stream->read(size,d)) 
	{
		currentPos = stream->getPosition();
		strm_status = STRM_UNKNOWN_ERROR;
		lastBytes = stream->getLastSize();
		return false;
	}
	currentPos += size;
	lastBytes = size;
	return true;
}


//----------------------------------------------------------------------------
void BlockStream::undoOpen()
{
	if (!stream)
		return;

	// close the stream and reposition where we originally opened it
	stream->setPosition(startPos);
	stream      = NULL;
	currentPos  = 0;
	startPos    = 0;
	strm_status = STRM_CLOSED;
}

//----------------------------------------------------------------------------
// Write block
// Creates a new block in the output stream when opened.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

BlockRWStream::BlockRWStream()
{
}

BlockRWStream::~BlockRWStream()
{
	close();
}


//----------------------------------------------------------------------------

Bool BlockRWStream::open(StreamIO& s,Int32 id,Int32 size, Bool dwordAlign)
{
	close();
	if (s.getStatus() != STRM_OK)
		return false;
	stream = &s;
	header.id = id;
	header.size = dwordAlign ? (size | ALIGN_DWORD) : size;
	AssertFatal(size != 0 ||
		stream->getCapabilities(STRM_CAP_REPOSITION),
		"BlockRWStream::open: Cannot open a block with undefined size "
		"on a stream that is not repositionable");
	startPos = stream->getPosition();
	if (!stream->write(sizeof(Header),&header)) 
	{
		stream = 0;
		startPos = 0;
		return false;
	}
	currentPos  = startPos + sizeof(Header);
	strm_status = STRM_OK;
	return true;
}


//----------------------------------------------------------------------------

Bool BlockRWStream::open(StreamIO& s,Int32 id)
{
	// Make sure the block opened is the one
	// we are looking for
	if (open(s) && header.id == id)
		return true;
	close();
	return false;
}


//----------------------------------------------------------------------------

Bool BlockRWStream::open(StreamIO& s)
{
	close();
	if (s.getStatus() != STRM_OK)
		return false;
	stream = &s;
	startPos = stream->getPosition();
	if (!stream->read(sizeof(Header),&header)) {
		stream = 0;
		startPos = 0;
		return false;
	}
	currentPos = startPos + sizeof(Header);
	strm_status = STRM_OK;
	return true;
}

//----------------------------------------------------------------------------

void BlockRWStream::close()
{
	if (!stream) return;

	Bool ok = true;
	if ( !getDataSize() )
	{
		// No size was given
      // if align on DWORD header.size == ALIGN_DWORD otherwise 0
		header.size = header.size | (currentPos - startPos - sizeof(Header));
		if (!stream->setPosition(startPos) ||
			!stream->write(sizeof(Header),&header))
			ok = false;
		currentPos = startPos + sizeof(Header);
	}

	if (ok) {
		Int32 endPos = startPos + getBlockSize();
		if (currentPos != endPos) {
			if (stream->getCapabilities(STRM_CAP_REPOSITION)) {
				stream->setPosition(endPos);
				currentPos = stream->getPosition();
			}
			if (currentPos != endPos)
				// Either we can't reposition or we've been
				// stopped because we are at the end of the file,
				// so let's fill with 0
				while (currentPos++ < endPos)
					// Ouch, this is slow
					if (!stream->write(Int8(0)))
						break;
		}
	}

	stream = 0;
	currentPos = startPos = 0;
	strm_status = STRM_CLOSED;
}


//----------------------------------------------------------------------------
// Read block
// Opens an existing block in the input stream
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

BlockRStream::BlockRStream()
{
}

BlockRStream::~BlockRStream()
{
	close();
}


//----------------------------------------------------------------------------

Bool BlockRStream::open(StreamIO& s,Int32 id)
{
	// Make sure the block opened is the one
	// we are looking for
	if (open(s) && header.id == id)
		return true;
	close();
	return false;
}


//----------------------------------------------------------------------------

Bool BlockRStream::open(StreamIO& s)
{
	close();
	if (s.getStatus() != STRM_OK)
		return false;
	stream = &s;
	startPos = stream->getPosition();
	if (!stream->read(sizeof(Header),&header)) {
		stream = 0;
		startPos = 0;
		return false;
	}
	currentPos = startPos + sizeof(Header);
	strm_status = STRM_OK;
	return true;
}


//----------------------------------------------------------------------------

void BlockRStream::close()
{
	if (!stream) return;

	Int32 endPos = startPos + getBlockSize();
	if (currentPos != endPos)
		if (stream->getCapabilities(STRM_CAP_REPOSITION))
			stream->setPosition(endPos);
		else
			while (currentPos++ < endPos) {
				Int8 ch;
				if (!stream->read(&ch))
					break;
			}
	stream = 0;
	currentPos = startPos = 0;
	strm_status = STRM_CLOSED;
}


