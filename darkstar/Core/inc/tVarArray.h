//------------------------------------------------------------------------------
//    tVarArray.h
//
//    Modified tString.h to have a sort of string of variable type.  Size must
//    be given.  Comparison is left out, a CmpVarArray could be derived from 
//    this one for types T warranted to have < defined.  
//------------------------------------------------------------------------------

#ifndef _TVARARRAY_H_
#define _TVARARRAY_H_

#include <persist.h>
#include <base.h>
#include <bitset.h>


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

// Implement an array with a length that's stored here.  

template <class T>
class VarArray
{
   int            len;
   T              * array;

	void assign ( const T * inArr, int N, bool ownIt = false );

public:
   BitSet16       flags;
   enum  { OwnsData = 1, };
   
	VarArray()  { array = NULL; len = 0; flags.clear(); }
	VarArray(const T * inArr, int N, bool ownIt = false );
	VarArray(const VarArray&, bool ownIt = false );
	~VarArray();
   

	typedef T value_type;
	typedef T & reference;
	typedef const T & const_reference;

	typedef T * iterator;
	typedef const T * const_iterator;
	typedef int difference_type;
	typedef int size_type;

	const T * data() const { return array; }

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	size_type length() const;
	size_type size()   const;
	bool      empty()  const;

	T & front();
	const T & front() const;

   // this is undefined memory:
	// T & back();
	// const T & back() const;

   T & operator[](int);
   operator int() const   { return len; }
   const T & operator[](int) const;

	void operator= (const VarArray& p);
	bool operator== (const VarArray& p) const;
	bool operator!= (const VarArray& p) const;
	// bool operator< (const VarArray& p) const;
};


//-----------------------------------------------------------------------------

template <class T> 
inline void VarArray<T>::assign(const T * inData, int N, bool ownsIt )
{
   flags.clear();
	if(inData != NULL) {
      if( ownsIt ){
         array = const_cast<T *>(inData);
         flags.set(OwnsData);
      }
      else{
		   array = new T [ N ];
		   memcpy( array, inData, sizeof(T) * N );
      }
      len = N;
	}
	else
		array = NULL, len = 0;
}

template <class T> 
inline VarArray<T>::VarArray(const T * data, int N, bool ownsIt )
{
	assign(data,N,ownsIt);
}

template <class T> 
inline VarArray<T>::VarArray(const VarArray& va, bool ownsIt )
{
	assign(va.array, va.len, ownsIt );
}

template <class T> 
inline VarArray<T>::~VarArray()
{
   if( !flags.test(OwnsData) )
	   delete [] array;
   len = 0;
   array = NULL;
   flags.clear();
}

template <class T> 
inline VarArray<T>::iterator VarArray<T>::begin()
{
	return array;
}

template <class T> 
inline VarArray<T>::const_iterator VarArray<T>::begin() const
{
	return array;
}

template <class T> 
inline VarArray<T>::iterator VarArray<T>::end()
{
	return array != NULL ? &array[len]: 0;
}

template <class T> 
inline VarArray<T>::const_iterator VarArray<T>::end() const
{
	return array != NULL ? &array[len]: 0;
}

template <class T> 
inline VarArray<T>::size_type VarArray<T>::length () const
{
	return array ? len : 0;
}

template <class T> 
inline VarArray<T>::size_type VarArray<T>::size() const
{
	return length();
}

template <class T> 
inline bool VarArray<T>::empty() const
{
	return length() == 0;
}

template <class T> 
inline T & VarArray<T>::front()
{
	AssertFatal(array != 0,"VA:front1" );
	return array[0];
}

template <class T> 
inline const T & VarArray<T>::front() const
{
	AssertFatal(array != 0,"VA:front2" );
	return array[0];
}

template <class T> 
inline T & VarArray<T>::operator[](int index)
{
	AssertFatal(array != NULL && index<len ,"VA:operator[]");
	return array[index];
}

template <class T> 
inline const T & VarArray<T>::operator[](int index) const
{
	AssertFatal(array != NULL && index<len ,"VA:operator[]");
	return array[index];
}

template <class T> 
inline void VarArray<T>::operator=(const VarArray& inVA )
{
	if (inVA.array != array) {
      if( !flags.test(OwnsData) )
		   delete [] array;
		assign(inVA.array, inVA.len, inVA.flags.test(OwnsData) );
	}
}

template <class T> 
inline bool VarArray<T>::operator==(const VarArray& p) const
{
	AssertFatal(array != 0,"VA:operator==()");
   if( len == N )
	   return !memcmp( array, data, len*sizeof(T) );
   return false;
}

template <class T> 
inline bool VarArray<T>::operator!=(const VarArray& p) const
{
	AssertFatal(array != 0,"VA:operator!=()");
	return !operator==(data,N);
}

// inline bool VarArray::operator<(const VarArray& p) const
// {
// 	AssertFatal(string != 0,"VarArray::operator<: Null string");
// 	return stricmp(string,p.string) < 0;
// }

// inline bool VarArray::operator<(const T * data, int N) const
// {
// 	AssertFatal(array != 0,"VA::operator< ()");
// 	return memcmp(string,p) < 0;
// }

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_TVARARRAY_H_

