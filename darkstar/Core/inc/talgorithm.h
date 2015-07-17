//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------

#ifndef _TALGORITHM_H_
#define _TALGORITHM_H_

#include <malloc.h>
#include <base.h>

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma warn -inl
#endif


//-----------------------------------------------------------------------------

#include <new.h>

// Borland also defines this in it's new.h file
//#if defined(__BORLANDC__)
//#include <new.h>
//#else
//inline void* operator new(size_t , void* p)
//{
//	return p;
//}
//#endif

template <class T>
inline void destroy(T* pointer)
{
    pointer->~T();
}


//-----------------------------------------------------------------------------

template <class Iterator, class T>
Iterator find(Iterator first, Iterator last, const T& x)
{
   while (first != last && *first != x)
      ++first;
   return first;
}

//-----------------------------------------------------------------------------

template <class Iterator, class T>
Iterator binary_search(Iterator first, Iterator last, const T& x)
{
   int len = last - first;

   while (len > 0)
   {
       int half = len / 2;
       Iterator middle = first + half;
       if(*middle < x)
       {
           first = middle;
           ++first;
           len = len - half - 1;
       }
       else
           len = half;
   }
   return (first != last && !(x < *first)) ? first : last;
}

template <class Iterator, class T, class LessThan>
Iterator binary_search(Iterator first, Iterator last, const T& x, LessThan compare)
{
   int len = last - first;

   while (len > 0)
   {
       int half = len / 2;
       Iterator middle = first + half;
       if(compare(*middle, x))
       {
           first = middle;
           ++first;
           len = len - half - 1;
       }
       else
           len = half;
   }
   return (first != last && !compare(x,*first)) ? first : last;
}

#endif
