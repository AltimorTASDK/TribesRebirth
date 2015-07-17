//================================================================
//   
// $Workfile:   streamio.h  $
// $Version$
// $Revision:   1.6  $
//	
// DESCRIPTION:
//
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _STREAMIO_H_
#define _STREAMIO_H_

//Includes
#include <types.h>
#include <base.h>


#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//---------------------------------------------------------------------------
// List of known stream types, compression types, capabilities, and statuses
//
enum StreamType
{
	STRM_BASE,

	STRM_FILE_RW,				// File read/write base of stdio.
	STRM_FILE_R,				// Read only.
	STRM_FILE_W,				// Write only.

	STRM_MEM_RW,				// IO to blocks of memory.
	STRM_MEM_R,					// Read only.
	STRM_MEM_W,					// Write only.

	STRM_RLE_R,					// Read RLE compressed.
	STRM_RLE_W,					// Write RLE compressed.

	STRM_LZH_R,					// Read LZH compressed.
	STRM_LZH_W,					// Write LZH compressed.

	STRM_BIT_R,					// Read bits.
	STRM_BIT_W,					// Write bits.

	STRM_BUFF_R,				// Read buffer stream.
   STRM_BUFF_W,				// Write buffer stream.
};

enum StreamCompressType
{
	// These enums are declared in the same order as the
	// 16bit packlib compress type.
	//
	STRM_COMPRESS_NONE,
	STRM_COMPRESS_RLE,
	STRM_COMPRESS_LZ,
	STRM_COMPRESS_LZH,
};

enum StreamCap
{
	STRM_CAP_TYPE,				// Stream type.
	STRM_CAP_WRITE,			// Write to stream.
	STRM_CAP_READ,				// Read from stream.
	STRM_CAP_REPOSITION,		// Can the stream be repositioned.
	STRM_CAP_COMPRESS,		// Is the stream compressed.
	STRM_CAP_LOCKABLE,		// The stream can be locked.
};

enum StreamStatus
{
   STRM_OK = 0,
   STRM_IO_ERROR,				// Read or Write error.
   STRM_EOS,					// End of Stream (normally during read).
   STRM_UNKNOWN_ERROR,		// ?
   STRM_ILLEGAL_CALL,		// Function not supported by stream or bad arguents.
	STRM_CLOSED,				// Stream not open for IO. 
	STRM_FLTR_DETACHED		// Filter Stream that is not attached to a base stream
};


//---------------------------------------------------------------------------
//
class String;

class DLLAPI StreamIO
{
protected:
   UInt32         lastBytes;
   StreamStatus   strm_status;

public:
   StreamIO();

	StreamStatus operator()() const;

   virtual StreamStatus	getStatus() const;
   void                 clrStatus();

	// Functions supplied by derived classes.

   virtual      ~StreamIO();
	virtual Int32   getPosition() const;
	virtual Bool    setPosition(Int32);

	virtual Int32   getCapabilities(StreamCap) const;
   virtual Bool    flush();
   UInt32          getLastSize() const;

	virtual Bool    write(int size,const void* d);
	virtual Bool    read (int size,void* d);
   virtual void*   lock();
   virtual void    close();

 	// Overloaded Read & Write.

#if (defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)) || (defined (_MSC_VER) && (_MSC_VER >= 1100))
	Bool 	write(bool d);
#endif
	Bool 	write(Int8   d);			     
	Bool 	write(Int16  d);			     
	Bool 	write(Int32  d);			     
	Bool 	write(UInt8  d);			     
	Bool 	write(UInt16 d);		     
	Bool 	write(UInt32 d);
	Bool 	write(RealF  d);
	Bool 	write(RealD  d);
   Bool  write(const String& in_rStr);

#if (defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)) || (defined (_MSC_VER) && (_MSC_VER >= 1100))
	Bool 	read(Bool* d);
#endif
	Bool 	read(Int8*   d); 	     
	Bool 	read(Int16*  d);		     
	Bool 	read(Int32*  d);		     
	Bool 	read(UInt8*  d);		     
	Bool 	read(UInt16* d);		     
	Bool 	read(UInt32* d);		     
	Bool 	read(RealF*  d);		     
	Bool 	read(RealD*  d);
   Bool  read(String* out_rStr);
   
   virtual void readString(char stringBuf[256]);
   virtual void writeString(const char *stringBuf, int maxLen=255);
   const char *readSTString(bool casesens = false);

   //
   Bool   writeCharString(const char* str);
   Bool   readCharString(char** str);

	//
	Bool 	write(int d);
	Bool 	write(unsigned int d);
	Bool   read(int* d);
	Bool   read(unsigned int* d);
}; 


//----------------------------------------------------------------------------

inline void StreamIO::clrStatus()
{
   strm_status = STRM_OK;
}

inline StreamStatus StreamIO::operator()() const
{
	return getStatus();
}


//----------------------------------------------------------------------------
//                          - write -
//----------------------------------------------------------------------------


#if (defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)) || (defined(_MSC_VER) && (_MSC_VER >= 1100))
inline Bool StreamIO::write(Bool d)
{
   UInt8 sb = static_cast<UInt8>(d);
   return (write(sizeof(sb), (const void *) &sb));
}
#endif

inline Bool StreamIO::write(Int8 d)
{
   return (write(sizeof(Int8), (const void *) &d));
}


inline Bool StreamIO::write(Int16 d)
{
   return (write(sizeof(Int16), (const void *) &d));
}


inline Bool StreamIO::write(Int32 d)
{
   return (write(sizeof(Int32), (const void *) &d));
}


//............................................................................


inline Bool StreamIO::write(UInt8 d)
{
   return (write(sizeof(UInt8), (const void *) &d));
}


inline Bool StreamIO::write(UInt16 d)
{
   return (write(sizeof(UInt16), (const void *) &d));
}


inline Bool StreamIO::write(UInt32 d)
{
   return (write(sizeof(UInt32), (const void *) &d));
}

//............................................................................

inline Bool StreamIO::write(RealF d)
{
   return (write(sizeof(RealF), (const void *) &d));
}

inline Bool StreamIO::write(RealD d)
{
   return (write(sizeof(RealD), (const void *) &d));
}

//----------------------------------------------------------------------------
//                            - read -
//----------------------------------------------------------------------------

#if (defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)) || (defined(_MSC_VER) && (_MSC_VER >= 1100))
inline Bool StreamIO::read(bool *d)
{
   UInt8 sb = 0;
   read(sizeof(sb), (void *) &sb);
   *d = (sb != 0);
   return strm_status == STRM_OK;
}
#endif

inline Bool StreamIO::read(Int8* d)
{
   return (read(sizeof(Int8), (void *) d));
}


inline Bool StreamIO::read(Int16* d)
{
   return (read(sizeof(Int16), (void *) d));
}


inline Bool StreamIO::read(Int32* d)
{
   return (read(sizeof(Int32), (void *) d));
}


//............................................................................


inline Bool StreamIO::read(UInt8* d)
{
   return (read(sizeof(UInt8), (void *) d));
}


inline Bool StreamIO::read(UInt16* d)
{
   return (read(sizeof(UInt16), (void *) d));
}


inline Bool StreamIO::read(UInt32* d)
{
   return (read(sizeof(UInt32), (void *) d));
}

//............................................................................

inline Bool StreamIO::read(RealF* d)
{
   return (read(sizeof(RealF), (void *) d));
}

inline Bool StreamIO::read(RealD* d)
{
   return (read(sizeof(RealD), (void *) d));
}

//............................................................................


inline Bool StreamIO::write(int d)
{
   return (write(sizeof(d), (const void *) &d));
}

inline Bool StreamIO::write(unsigned int d)
{
   return (write(sizeof(d), (const void *) &d));
}

inline Bool StreamIO::read(int * d)
{
   return read(sizeof(*d), (void *) d);
}

inline Bool StreamIO::read(unsigned int * d)
{
   return read(sizeof(*d), (void *) d);
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif //_STREAMIO_H_
