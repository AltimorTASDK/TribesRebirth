//================================================================
//
//	$Workfile:   memstrm.h  $
//	$Version: Beta1.7.2 $
//	$Revision:   1.4  $
//
// DESCRIPTION:
//
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _MEMSTRM_H_
#define _MEMSTRM_H_

#include <streamio.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//---------------------------------------------------------------------------

class MemRWStream : public StreamIO
{
   char*			mFrontPtr;     // Beginning of allocated block.
   char*			mEndPtr;       // End of allocated block.
   char*			mPosnPtr;      // Current position of I/O "head".
   bool        lockable;      // is the memory stream lockable?

	// Methods:

   Bool setStatus(Int8 flag);

public:
	MemRWStream();
   MemRWStream(UInt32 totalSize, void* frontPtr);

   void setLockable(bool tf);

	// Virtual methods:

	Int32 getPosition() const;
	Bool 	setPosition(Int32 pos);
	Int32	getCapabilities(StreamCap) const;
   Bool 	flush();
	Bool write(int size,const void* d); 
	Bool read(int size,void* d); 
   void*  lock();

	void open(UInt32 totalSize, void* frontPtr);
	void close();

	// Overloaded Read & Write.

	Bool   write(Int8 d);			     
	Bool   write(Int16 d);			     
	Bool   write(Int32 d);			     
	Bool   write(UInt8 d);			     
	Bool   write(UInt16 d);		     
	Bool   write(UInt32 d);
	Bool   write(RealF d);
	Bool   write(RealD d);

	Bool   write(int size,const Int8* d);   
	Bool   write(int size,const Int16* d);  
	Bool   write(int size,const Int32* d);  
	Bool   write(int size,const UInt8* d);  
	Bool   write(int size,const UInt16* d);  
	Bool   write(int size,const UInt32* d);  
	Bool   write(int size,const RealF* d);  
	Bool   write(int size,const RealD* d);  

	Bool   read(Int8* d); 	     
	Bool   read(Int16* d);		     
	Bool   read(Int32* d);		     
	Bool   read(UInt8* d);		     
	Bool   read(UInt16* d);		     
	Bool   read(UInt32* d);		     
	Bool   read(RealF* d);		     
	Bool   read(RealD* d);		     
   
	Bool   read(int size,Int8* d);   
	Bool   read(int size,Int16* d);   
	Bool   read(int size,Int32* d);   
	Bool   read(int size,UInt8* d);   
	Bool   read(int size,UInt16*d);   
	Bool   read(int size,UInt32*d); 
	Bool   read(int size,RealF* d);   
	Bool   read(int size,RealD* d);   
};


//------------------------------------------------------------------------------
inline void MemRWStream::setLockable(bool tf)
{
   lockable = tf;
}   


//----------------------------------------------------------------------------
//                          - write -
//----------------------------------------------------------------------------


inline Bool MemRWStream::write(Int8 d)
{
   return (write(sizeof(Int8), (const void *) &d));
}


inline Bool MemRWStream::write(Int16 d)
{
   return (write(sizeof(Int16), (const void *) &d));
}


inline Bool MemRWStream::write(Int32 d)
{
   return (write(sizeof(Int32), (const void *) &d));
}


//............................................................................


inline Bool MemRWStream::write(UInt8 d)
{
   return (write(sizeof(UInt8), (const void *) &d));
}


inline Bool MemRWStream::write(UInt16 d)
{
   return (write(sizeof(UInt16), (const void *) &d));
}


inline Bool MemRWStream::write(UInt32 d)
{
   return (write(sizeof(UInt32), (const void *) &d));
}

//............................................................................

inline Bool MemRWStream::write(RealF d)
{
   return (write(sizeof(RealF), (const void *) &d));
}

inline Bool MemRWStream::write(RealD d)
{
   return (write(sizeof(RealD), (const void *) &d));
}

//............................................................................


inline Bool MemRWStream::write(int size,const Int8* d)
{
   return (write((size * sizeof(Int8)), (const void *) d));
}


inline Bool MemRWStream::write(int size,const Int16* d)
{
   return (write((size * sizeof(Int16)), (const void *) d));
}


inline Bool MemRWStream::write(int size,const Int32* d)
{
   return (write((size * sizeof(Int32)), (const void *) d));
}


//............................................................................


inline Bool MemRWStream::write(int size,const UInt8* d)
{
   return (write((size * sizeof(UInt8)), (const void *) d));
}


inline Bool MemRWStream::write(int size,const UInt16* d)
{
   return (write((size * sizeof(UInt16)), (const void *) d));
}


inline Bool MemRWStream::write(int size,const UInt32* d)
{
   return (write((size * sizeof(UInt32)), (const void *) d));
}

//............................................................................

inline Bool MemRWStream::write(int size,const RealF* d)
{
   return (write((size * sizeof(RealF)), (const void *) d));
}

inline Bool MemRWStream::write(int size,const RealD* d)
{
   return (write((size * sizeof(RealD)), (const void *) d));
}


//----------------------------------------------------------------------------
//                            - read -
//----------------------------------------------------------------------------

inline Bool MemRWStream::read(Int8* d)
{
   return (read(sizeof(Int8), (void *) d));
}


inline Bool MemRWStream::read(Int16* d)
{
   return (read(sizeof(Int16), (void *) d));
}


inline Bool MemRWStream::read(Int32* d)
{
   return (read(sizeof(Int32), (void *) d));
}


//............................................................................


inline Bool MemRWStream::read(UInt8* d)
{
   return (read(sizeof(UInt8), (void *) d));
}


inline Bool MemRWStream::read(UInt16* d)
{
   return (read(sizeof(UInt16), (void *) d));
}


inline Bool MemRWStream::read(UInt32* d)
{
   return (read(sizeof(UInt32), (void *) d));
}

//............................................................................

inline Bool MemRWStream::read(RealF* d)
{
   return (read(sizeof(RealF), (void *) d));
}

inline Bool MemRWStream::read(RealD* d)
{
   return (read(sizeof(RealD), (void *) d));
}

//............................................................................


inline Bool MemRWStream::read(int size,Int8* d)
{
   return (read((size * sizeof(Int8)), (void *) d));
}


inline Bool MemRWStream::read(int size,Int16* d)
{
   return (read((size * sizeof(Int16)), (void *) d));
}


inline Bool MemRWStream::read(int size,Int32* d)
{
   return (read((size * sizeof(Int32)), (void *) d));
}


//............................................................................


inline Bool MemRWStream::read(int size,UInt8* d)
{
   return (read((size * sizeof(UInt8)), (void *) d));
}


inline Bool MemRWStream::read(int size,UInt16* d)
{
   return (read((size * sizeof(UInt16)), (void *) d));
}


inline Bool MemRWStream::read(int size,UInt32* d)
{
   return (read((size * sizeof(UInt32)), (void *) d));
}

//............................................................................

inline Bool MemRWStream::read(int size,RealF* d)
{
   return (read((size * sizeof(RealF)), (void *) d));
}
#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif


