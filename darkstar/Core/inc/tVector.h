//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------

#ifndef _TVECTOR_H_
#define _TVECTOR_H_

#include <malloc.h>
#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

const int VectorDefaultBlockSize = 5;

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma warn -inl
#endif

//-----------------------------------------------------------------------------
// A dynamic array class.  The vector grows as you insert or append
// elements.  Insertion is fastest at the end of the array.  Resizing
// of the array can be avoided by pre-allocating space using the
// reserve() method.

// ***WARNING*** 
// This template does not initialize, construct or destruct any of
// it's elements.  This means don't use this template for elements
// (classes) that need these operations.  This template is intended
// to be used for simple structures that have no constructors or
// destructors.

template<class T>
class Vector
{
   enum Flags
      {
      Flags_OwnArray = 0x1,
      Flags_ReadOnly = 0x2,
      };

    int element_count;
    int array_size;
    short block_size;
    short flags;
    T* array;

    bool	resize(int);
 public:
    Vector(short bsize = VectorDefaultBlockSize);
    Vector(const Vector&);
    ~Vector();

	// STL interface
	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;

	typedef T* iterator;
	typedef const T* const_iterator;
	typedef int difference_type;
	typedef int size_type;


	void operator=(const Vector& p);

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;
   int  size() const;
	int  max_size() const;
	bool empty() const;

	void insert(iterator,const T&);
	void erase(iterator);

	T& front();
	const T& front() const;
	T& back();
	const T& back() const;
	void push_front(const T&);
	void push_back(const T&);
	void pop_front();
	void pop_back();
   T& operator[](int);
   const T& operator[](int) const;

   void reserve(int);
   int capacity() const;


	// Extended interface
	int  memSize() const;
   T*   address() const;
	int  setSize(int);
   void increment();
   void decrement();
   void insert(int);
   void erase(int);
	void clear();
	void compact();

	T& first();
	T& last();
	const T& first() const;
	const T& last() const;

   void setReadOnly();
   Bool isReadOnly() const;
   void referenceArray(T const * base, int size );

	// BJW 8/20/97
	// merge another vector into this one
	void merge(const Vector& p);
};


//-----------------------------------------------------------------------------

template<class T> Vector<T>::~Vector()
{
	if( array && (flags & Flags_OwnArray) )
		free(array);
}

template<class T> Vector<T>::Vector(short bsize)
{
   flags = 0;
	array = 0;
	element_count = 0;
	array_size = 0;
	block_size = bsize;
}

template<class T> Vector<T>::Vector(const Vector& p)
{
   flags = 0;
	array = 0;
	block_size = p.block_size;
	resize(p.element_count);
	if (p.element_count)
		memcpy(array,p.array,element_count * sizeof(value_type));
}


//-----------------------------------------------------------------------------

template<class T> void Vector<T>::referenceArray(T const * base, int size )
{
	if( array && (flags & Flags_OwnArray) )
		free(array);
   array = (T*)base;
	element_count = size;
   // by default, assume we don't own the array and shouldn't change it:
   flags = Flags_ReadOnly;
}	

//-----------------------------------------------------------------------------

template<class T> inline int Vector<T>::memSize() const
{
    return capacity() * sizeof(T);
}

template<class T> inline T* Vector<T>::address() const
{
	return array;
}

template<class T> inline int Vector<T>::setSize(int size)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::setSize: vector is read only" );
	if (size > array_size)
		resize(size);
	else
		element_count = size;
	return element_count;
}

template<class T> inline void Vector<T>::increment()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::increment: vector is read only" );
    if (++element_count > array_size)
		resize(element_count);
}

template<class T> inline void Vector<T>::decrement()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::decrement: vector is read only" );
    if (element_count > 0)
		element_count--;
}

template<class T> inline void Vector<T>::insert(int index)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::insert: vector is read only" );
	// Assert: index >= 0 && index < element_count
	increment();
	memmove(&array[index + 1],&array[index],
		(element_count - index - 1) * sizeof(value_type));
}

template<class T> inline void Vector<T>::erase(int index)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::erase: vector is read only" );
	// Assert: index >= 0 && index < element_count
	memmove(&array[index],&array[index + 1],
		(element_count - index - 1) * sizeof(value_type));
	decrement();
}

template<class T> inline T& Vector<T>::first()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::first: vector is read only" );
	return array[0];
}

template<class T> inline const T& Vector<T>::first() const
{
	return array[0];
}

template<class T> inline T& Vector<T>::last()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::last: vector is read only" );
	return array[element_count - 1];
}

template<class T> inline const T& Vector<T>::last() const
{
	return array[element_count - 1];
}

template<class T> inline void Vector<T>::clear()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::clear: vector is read only" );
	element_count = 0;
}

template<class T> inline void Vector<T>::compact()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::compact: vector is read only" );
	resize(element_count);
}


//-----------------------------------------------------------------------------

template<class T> void inline Vector<T>::operator=(const Vector& p)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::operator=: vector is read only" );
	resize(p.element_count);
	block_size = p.block_size;
	if (p.element_count)
		memcpy(array,p.array,element_count * sizeof(value_type));
}	

template<class T> inline typename Vector<T>::iterator Vector<T>::begin()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::begin: vector is read only" );
	return &array[0];
}

template<class T> inline typename Vector<T>::const_iterator Vector<T>::begin() const
{
	return &array[0];
}

template<class T> inline typename Vector<T>::iterator Vector<T>::end()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::end: vector is read only" );
	return &array[element_count];
}

template<class T> inline typename Vector<T>::const_iterator Vector<T>::end() const
{
	return &array[element_count];
}

template<class T> inline int Vector<T>::size() const
{
    return element_count;
}

template<class T> inline int Vector<T>::max_size() const
{
    return M_MAX_INT >> 2;
}

template<class T> inline bool Vector<T>::empty() const
{
    return element_count == 0;
}

template<class T> inline void Vector<T>::insert(iterator p,const T& x)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::insert: vector is read only" );
	int index = p - array;
	insert(index);
	array[index] = x;
}

template<class T> inline void Vector<T>::erase(iterator q)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::erase: vector is read only" );
	erase(q - array);
}

template<class T> inline T& Vector<T>::front()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::front: vector is read only" );
	return *begin();
}

template<class T> inline const T& Vector<T>::front() const
{
	return *begin();
}

template<class T> inline T& Vector<T>::back()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::back: vector is read only" );
	return *end();
}

template<class T> inline const T& Vector<T>::back() const
{
	return *end();
}

template<class T> inline void Vector<T>::push_front(const T& x)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::push_front: vector is read only" );
	insert(0);
	array[0] = x;
}

template<class T> inline void Vector<T>::push_back(const T& x)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::push_back: vector is read only" );
	increment();
	array[element_count - 1] = x;
}

template<class T> inline void Vector<T>::pop_front()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::pop_front: vector is read only" );
	erase(0);
}

template<class T> inline void Vector<T>::pop_back()
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::pop_back: vector is read only" );
	decrement();
}

template<class T> inline T& Vector<T>::operator[](int index)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::operator[]: vector is read only" );
	return array[index];
}

template<class T> inline const T& Vector<T>::operator[](int index) const
{
	return array[index];
}

template<class T> inline int Vector<T>::capacity() const
{
    return array_size;
}


//-----------------------------------------------------------------------------

template<class T> void Vector<T>::reserve(int size)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::reserve: vector is read only" );
	if (size > array_size) {
		int ec = element_count;
		if (resize(size))
			element_count = ec;
	}
}

template<class T> bool Vector<T>::resize(int ecount)
{
   AssertFatal( !(flags & Flags_ReadOnly), 
      "Vector::resize: vector is read only" );
   AssertFatal( !array || (flags & Flags_OwnArray),
      "Vector::resize: don't own array; can't realloc" );
    if (ecount > 0) {
		int blocks = ecount / block_size;
		if (ecount % block_size)
		    blocks++;
		int mem_size = blocks * block_size * sizeof(T);
		array = array? (T*) realloc((char *) array,mem_size):
			(T*) malloc(mem_size);
		if (!array) {
		    element_count = 0;
		    array_size = 0;
		    return false;
		}
	    element_count = ecount;
	    array_size = blocks * block_size;
       flags = Flags_OwnArray;
	    return true;
    }
	if (array) {
		free(array);
		array = 0;
	}
	array_size = 0;
	element_count = 0;
	return true;
}


//-----------------------------------------------------------------------------

template<class T>
inline void Vector<T>::setReadOnly()
   {
   flags |= Flags_ReadOnly;
   }

template<class T>
inline Bool Vector<T>::isReadOnly() const
   {
   return flags & Flags_ReadOnly;
   }



// BJW 8/20/97
// code to merge a vector into this one
template<class T>
inline void Vector<T>::merge(const Vector& p)
{
	if (p.size()) {
		int oldsize = size();
		resize(oldsize + p.size());
		memcpy( &array[oldsize], p.address(), p.size() * sizeof(T) );
	}
}

//-----------------------------------------------------------------------------
// Template for vectors of pointers.
//-----------------------------------------------------------------------------

template <class T> 
class VectorPtr : public Vector<void*>
{
public:
	// STL interface
	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;

	typedef T* iterator;
	typedef const T* const_iterator;
	typedef int difference_type;
	typedef int size_type;

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	void insert(iterator,const T&);
	void erase(iterator);

	T& front();
	const T& front() const;
	T& back();
	const T& back() const;
	void push_front(const T&);
	void push_back(const T&);

   T& operator[](int);
   const T& operator[](int) const;

	// Extended interface
	typedef Vector<void*> Parent;
	T& first();
	T& last();
	const T& first() const;
	const T& last() const;
};


//-----------------------------------------------------------------------------

template<class T> inline T& VectorPtr<T>::first()
{
	return (T&)Parent::first();
}

template<class T> inline const T& VectorPtr<T>::first() const
{
	return (const T)Parent::first();
}

template<class T> inline T& VectorPtr<T>::last()
{
	return (T&)Parent::last();
}

template<class T> inline const T& VectorPtr<T>::last() const
{
	return (const T&)Parent::last();
}


//-----------------------------------------------------------------------------

#if 1
template<class T> inline typename VectorPtr<T>::iterator VectorPtr<T>::begin()
{
	return (iterator)Parent::begin();
}

template<class T> inline typename VectorPtr<T>::const_iterator VectorPtr<T>::begin()
const
{
	return (const_iterator)Parent::begin();
}

template<class T> inline typename VectorPtr<T>::iterator VectorPtr<T>::end()
{
	return (iterator)Parent::end();
}

template<class T> inline typename VectorPtr<T>::const_iterator VectorPtr<T>::end() const
{
	return (const_iterator)Parent::end();
}
#else
template<class T> inline typename Vector<T>::iterator VectorPtr<T>::begin()
{
	return (iterator)Parent::begin();
}

template<class T> inline typename Vector<T>::const_iterator VectorPtr<T>::begin() const
{
	return (const_iterator)Parent::begin();
}

template<class T> inline typename Vector<T>::iterator VectorPtr<T>::end()
{
	return (iterator)Parent::end();
}

template<class T> inline typename Vector<T>::const_iterator VectorPtr<T>::end() const
{
	return (const_iterator)Parent::end();
}
#endif

template<class T> inline void VectorPtr<T>::insert(iterator i,const T& x)
{
   Parent::insert( (Parent::iterator)i, (Parent::reference)x );   
}

template<class T> inline void VectorPtr<T>::erase(iterator i)
{
   Parent::erase( (Parent::iterator)i );   
}

template<class T> inline T& VectorPtr<T>::front()
{
	return *begin();
}

template<class T> inline const T& VectorPtr<T>::front() const
{
	return *begin();
}

template<class T> inline T& VectorPtr<T>::back()
{
	return *end();
}

template<class T> inline const T& VectorPtr<T>::back() const
{
	return *end();
}

template<class T> inline void VectorPtr<T>::push_front(const T& x)
{
	Parent::push_front((Parent::const_reference)x);
}

template<class T> inline void VectorPtr<T>::push_back(const T& x)
{
	Parent::push_back((Parent::const_reference)x);
}

template<class T> inline T& VectorPtr<T>::operator[](int index)
{
	return (T&)Parent::operator[](index);
}

template<class T> inline const T& VectorPtr<T>::operator[](int index) const
{
	return (const T&)Parent::operator[](index);
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif

