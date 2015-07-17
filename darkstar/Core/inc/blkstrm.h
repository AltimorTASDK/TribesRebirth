//================================================================
//   
//	$Workfile:   blkstrm.h  $
//	$Version$
//	$Revision:   1.2  $
//
// DESCRIPTION:
//
//	The block actually keeps track of it's current position in
//	the parent stream.  This could be removed to simplify the
//	code and probably increase performance.  Leaving it in allows
//	you to have multiple blocks open, and being acccessed at the
//	same time, on the same stream.
//
//	There is a known problem with declaring a block size larger than
//	what you write.  If the block is the last block in the file and
//	you do not write the total number of bytes, the block will come
//	only contain the bytes you wrote.  The block code seeks to the
//	correct location, but unless something is written, the file
//	system ignores it.  If there is a following block, no problem.
//
//	Initial Version: timg
//
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _BLOCKIO_H_
#define _BLOCKIO_H_

#include <streamio.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


//----------------------------------------------------------------------------

#define ALIGN_DWORD 0x80000000

class BlockStream: public StreamIO
{
public:
	struct Header {
		Int32 id;
		Int32 size;
	};

protected:
	StreamIO* stream;	// Parent stream
	Header header;		// Current block header
	int currentPos;	// Position for read/write
	int startPos;		// Start of the block's header

	Int32 alignSize(Int32 size, Bool alignDword) const;

public:
	BlockStream();
	virtual ~BlockStream();

	Int32   getID() const;
	Int32   getDataSize() const;
	Int32   getBlockSize() const;

	virtual Int32 getCapabilities(StreamCap) const;
	virtual Int32 getPosition() const;
	virtual Bool  setPosition(Int32);
	virtual Bool  flush();

   virtual void* lock();

	// close the stream and reposition where we originally opened it
	void undoOpen();

	// Read or write, based on capabilities of
	// the parent stream.
	using StreamIO::write;
	using StreamIO::read;

	virtual Bool write(int size,const void* d);
	virtual Bool read (int size,void* d);
};


inline Int32 BlockStream::alignSize(Int32 size, Bool alignDword) const
{
   if ( alignDword )
	   return ( (size + 3) & (~3) );    //DWORD Align
   else
	   return ( (size + 1) & (~1) );    //WORD Align
}


inline Int32 BlockStream::getID() const
{
	return header.id;
}


inline Int32 BlockStream::getDataSize() const
{
   return ( header.size & ~ALIGN_DWORD);
}


inline Int32 BlockStream::getBlockSize() const
{
   if ( header.size & ALIGN_DWORD )
	   return ( alignSize(getDataSize(), true) + sizeof(Header) );    //DWORD Align
   else
	   return ( alignSize(getDataSize(), false)+ sizeof(Header) );    //WORD Align
}


//----------------------------------------------------------------------------
// Creates a block in the output stream when opened.
//
class BlockRWStream: public BlockStream
{
public:
	BlockRWStream();
	~BlockRWStream();

	Bool open(StreamIO& s,Int32 id,Int32 size, Bool dwordAlign);
	Bool open(StreamIO& s,Int32 id);
	Bool open(StreamIO& s);
	void close();
};


//----------------------------------------------------------------------------
// Opens an existing block in the the output stream
//
class BlockRStream: public BlockStream
{
public:
	BlockRStream();
	~BlockRStream();

	Bool open(StreamIO& s);
	Bool open(StreamIO& s,Int32 id);
	void close();
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif
