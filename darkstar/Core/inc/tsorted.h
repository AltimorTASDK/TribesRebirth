//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------

#ifndef _TSORTED_H_
#define _TSORTED_H_

#include <stdlib.h>
#include <tvector.h>
#include <talgorithm.h>
#include <m_qsort.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma warn -inl
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<class T> 
class SortableVector : public Vector<T>
{
   // class T must implement operator <
protected:
public:
   static int __cdecl compare( const void *elem1, const void *elem2 );
public:
   void sort();
   iterator find( T const & );
};

template<class T> 
int __cdecl SortableVector<T>::compare( void const *elem1, void const *elem2 )
{
   if( *((T const *)elem1) < *((T const *)elem2) )
      return -1;
   else if( *((T const *)elem2) < *((T const *)elem1) )
      return 1;
   else
      return 0;
}

template<class T> inline void SortableVector<T>::sort()
   {
   m_qsort( address(), size(), sizeof(T), compare );
   }

template<class T> inline typename SortableVector<T>::iterator SortableVector<T>::find( T const & x)
   {
   return binary_search( begin(), end(), x );
   }

//-----------------------------------------------------------------------------

template<class T> 
class SortableVectorPtr : public VectorPtr<T>
{
   // class T must implement operator <
protected:
public:
   static int __cdecl qsortCompare( const void *elem1, const void *elem2 );
   static bool lessThan( T const &, T const & );
public:
   void sort();
   iterator find( T const & );
};

template<class T> 
int __cdecl SortableVectorPtr<T>::qsortCompare( void const *elem1, void const *elem2 )
{
   if( **((T const *)elem1) < **((T const *)elem2) )
      return -1;
   else if( **((T const *)elem2) < **((T const *)elem1) )
      return 1;
   else
      return 0;
}

template<class T> inline void SortableVectorPtr<T>::sort()
   {
   m_qsort( address(), size(), sizeof(T), qsortCompare );
   }

template<class T> inline bool SortableVectorPtr<T>::lessThan( T const & elem1, T const & elem2 )
   {
   return (*elem1 < *elem2);
   }

template<class T> inline typename SortableVectorPtr<T>::iterator SortableVectorPtr<T>::find( T const & x)
   {
   return binary_search( begin(), end(), x, lessThan );
   }

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
