//================================================================
//
// $Workfile:   filstrm.h  $
// $Version$
// $Revision:   1.5  $
//
// DESCRIPTION:
//
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _FILSTRM_H_
#define _FILSTRM_H_

#include <filedefs.h>
#include <streamio.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#define STRM_BUFFER_SIZE (1024*2)

//---------------------------------------------------------------------------
//	File Read/Write
//
// Note: the hFile members of these classes are marked as mutable in order
//  to maintain maximal logical const'ness of the classes.
//---------------------------------------------------------------------------


class DLLAPI FileRWStream: public StreamIO
{
   mutable HANDLE hFile;   // Current file handle
	Bool		isMine;        // Did I open this file ?
   bool assertIsv;
	// Methods:

   Bool setStatus();

public:

   FileRWStream(bool assertIfInvalid = true);
   FileRWStream(HANDLE in_hFile, bool assertIfInvalid = true);
   FileRWStream(const char* in_filename, bool assertIfInvalid = true);

	Bool 	open(const char* in_filename, bool readOnly=FALSE);
	void 	close();
	void 	setHandle(HANDLE in_hFile);
	HANDLE getHandle() const;
	UInt32 getSize() const;

	// Virtual methods:

   ~FileRWStream();
	Int32  getPosition() const;
	Bool   setPosition(Int32 pos);
	Int32	 getCapabilities(StreamCap) const;
   Bool   flush();

	using StreamIO::write;
	using StreamIO::read;

	// Overloaded Read & Write.
	Bool 	write(int size,const void* d);
	Bool 	read (int size,void* d);
};


inline HANDLE 
FileRWStream::getHandle() const
{
	return hFile;
}


//---------------------------------------------------------------------------
// File Read Stream
//---------------------------------------------------------------------------

class DLLAPI FileRStream: public StreamIO
{
   UInt8    buffer[STRM_BUFFER_SIZE];
   Int32    buffHead;
   Int32    buffTail;
   Int32    buffPos;
   mutable HANDLE   hFile;    // Current file.
	Bool		isMine;			   // Did I open this file ?

	// Methods:

   Bool fillCache(int in_newHead);
   Bool setStatus();

public:

   FileRStream();
   FileRStream(HANDLE in_hFile);
   FileRStream(const char* in_filename);

	Bool 	open(const char* in_filename);
	void 	close();
	void   setHandle(HANDLE in_hFile);
	HANDLE getHandle() const;
	UInt32 getSize() const;

	// Virtual methods:

   ~FileRStream();
	Int32  getPosition() const;
	Bool   setPosition(Int32 pos);
	Int32		getCapabilities(StreamCap) const;
   Bool   flush();

	using StreamIO::write;
	using StreamIO::read;

	// Overloaded Read.
	Bool 	write(int size,const void* d);
	Bool 	read(int size,void* d);
};


inline HANDLE 
FileRStream::getHandle() const
{
	return hFile;
}



//---------------------------------------------------------------------------
//	File Write
//---------------------------------------------------------------------------


class DLLAPI FileWStream : public StreamIO
{
   mutable HANDLE hFile;   // Current file.
	Bool		isMine;			// Did I open this file ?
   char     strm_buf;

	// Methods:

   Bool setStatus();
   bool assertIsv;

public:

   FileWStream(bool assertIfInvalid = true);
   FileWStream(HANDLE in_hFile, bool assertIfInvalid = true);
   FileWStream(const char* in_filename, bool assertIfInvalid = true);

	Bool 	open(const char* in_filename);
	void 	close();
	void 	setHandle(HANDLE in_hFile);
	HANDLE getHandle() const;
	UInt32 getSize() const;

	// Virtual methods:

   ~FileWStream();
	Int32  getPosition() const;
	Bool   setPosition(Int32 pos);
	Int32		getCapabilities(StreamCap) const;
   Bool   flush();

	using StreamIO::write;
	using StreamIO::read;

	// Overloaded Write.
	Bool 			write(int size,const void* d);
	Bool 			read(int size,void* d);
};


inline HANDLE 
FileWStream::getHandle() const
{
	return hFile;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif //_FILSTRM_H_
