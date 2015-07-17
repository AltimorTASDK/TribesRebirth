//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------

#ifndef _TIO_H_
#define _TIO_H_

#include <malloc.h>
#include <base.h>
#include <streamio.h>

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma warn -inl
#endif


//-----------------------------------------------------------------------------

template <class T>
inline Bool readVector( StreamIO & sio, int count, T *vec )
{
   vec->setSize( count );
   return sio.read( vec->size() * sizeof(T::value_type), (char*)vec->address() );   
}

//-----------------------------------------------------------------------------

template <class T>
inline Bool writeVector( StreamIO & sio, T const &vec )
{
   return sio.write( vec.size() * sizeof(T::value_type), (char*)vec.address() );   
}

//-----------------------------------------------------------------------------

template <class T>
inline Bool lockVector( StreamIO & sio, int count, T *vec )
{
   #if 0
   if( sio.MemoryMapEnabled() )
      {
      vec->referenceArray( count, (T::value_type*)sio.getMapPointer() );
      return sio.setPosition( sio.getPosition() + vec->size() * sizeof(T) );
      }
   else
   #endif
      return readVector( sio, count, vec );
}

//-----------------------------------------------------------------------------

#endif
