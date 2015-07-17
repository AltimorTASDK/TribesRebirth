//================================================================
//
// $Workfile:   rlestrm.h  $
// $Version$
// $Revision:   1.2  $
//
// DESCRIPTION:
//
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _RLESTRM_H_
#define _RLESTRM_H_


#include <streamio.h>

#ifdef __BORLANDC__
#pragma option -a1
#endif
#ifdef _MSC_VER
#pragma pack(push,1)
#endif

//---------------------------------------------------------------------------

class RLERStream : public StreamIO
{
private:
   StreamIO*	ts;      		// stream RLE reads/writes from/to.
   Int8			goalByte;		// byte we are comparing to
   Int8			byteCount;		// number of times byte was found
   Int8			buffLen;			// length of buffer used.
   Int8			buffer[127];	// for runs of non-matching bytes
	Int32 		curPos;			// Current position.

   void initialize();

public:

   RLERStream();
   RLERStream(StreamIO& stream);

   void  attach(StreamIO &s);
   void  detach();
   void  close();

	// Virtual methods:

	Int32	getPosition() const;
	Bool setPosition(Int32 pos);
	Int32	getCapabilities(StreamCap) const;
   Bool flush();

	using StreamIO::write;
	using StreamIO::read;

	Bool   write(int size,const void* d);
	Bool   read(int size,void* d);
};


//---------------------------------------------------------------------------

class RLEWStream : public StreamIO
{
private:
   StreamIO*	ts;				// stream RLE reads/writes from/to.
   Int8			goalByte;		// byte we are comparing to
   Int8			byteCount;		// number of times byte was found
   Int8			buffLen;			// length of buffer used.
   Int8			buffer[127];	// for runs of non-matching bytes
	Int32		   curPos;			// Current position.

   void initialize();

public:

   RLEWStream();
   RLEWStream(StreamIO& stream);

   void  attach(StreamIO &s);
   void  detach();
   void  close();

	// Virtual methods:

   ~RLEWStream();
	Int32	 getPosition() const;
	Bool 	setPosition(Int32 pos);
	Int32	getCapabilities(StreamCap) const;
   Bool flush();

	using StreamIO::write;
	using StreamIO::read;

	Bool write(int size,const void* d);
	Bool read(int size,void* d);
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_RLESTRM_H_
