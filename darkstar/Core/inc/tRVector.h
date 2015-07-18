//-----------------------------------------------------------------------------
//    tRVector.h
//    
//    Derived from tVector, tRVector is a "re-useable" vector.  These vectors
//    are re-useable in the sense that memory doesn't have to be allocated
//    every time one of these is created.  Instead, it borrows the memory from
//    a pool and returns it when it is through.
//
//-----------------------------------------------------------------------------

#ifndef _T_RVECTOR_H_
#define _T_RVECTOR_H_
#include <tVector.h>
#include <RVecAlloc.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma warn -inl
#endif

extern RVectorAlloc rvAlloc;

//-----------------------------------------------------------------------------

template<class T> class RVector
{
   RVectorAlloc::Node * node;

   // 
   int element_count;
   int array_size;
   T* array;

   bool resize(int);
   void setArray(RVectorAlloc::Node *);
   
public:

   RVector();
   RVector(const RVector&);
   ~RVector();

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
   int  size() const;
	int  max_size() const;
	bool empty() const;

	void operator=(const RVector& p);

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

   void reserve(int);inline
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
   void compact() {} // don't do this

	T& first();
	T& last();
	const T& first() const;
	const T& last() const;

	// merge another vector into this one
	void merge(const RVector& p);
};

//-----------------------------------------------------------------------------
// These are the methods that differ from the standard Vector class
//-----------------------------------------------------------------------------

template<class T> RVector<T>::RVector()
{
   node = rvAlloc.getArray();
   setArray(node);
   element_count = 0;
}

template<class T> RVector<T>::RVector(const RVector& p)
{
   node = rvAlloc.getArray();
   setArray(node);
   element_count = 0;
   resize(p.element_count);
   if (p.element_count)
		memcpy(array,p.array,element_count * sizeof(T));
}

template<class T> RVector<T>::~RVector()
{
   rvAlloc.release(node);
   array = NULL;
}

template<class T> bool RVector<T>::resize(int ecount)
{
   element_count = ecount;

   if (ecount <= array_size)
      return true;
   
   node = rvAlloc.resize(node,ecount*sizeof(T));
   setArray(node);

   return array!=NULL;
}

template<class T> void RVector<T>::setArray(RVectorAlloc::Node * node)
{
   if (!node)
   {
      // problems...
      array = NULL;
      element_count = 0;
      return;
   }
   
   array = (T*)&node->arrayStart;
   array_size = (node->size - RVectorAlloc::ArrayOffset) / sizeof(T);
}

//-----------------------------------------------------------------------------
// These methods are simply copied from tVector.h
//-----------------------------------------------------------------------------

template<class T> void inline RVector<T>::operator=(const RVector& p)
{
	resize(p.element_count);
	if (p.element_count)
		memcpy(array,p.array,element_count * sizeof(value_type));
}	

template<class T> inline void RVector<T>::increment()
{
    if (++element_count > array_size)
		resize(element_count);
}

template<class T> inline void RVector<T>::decrement()
{
    if (element_count > 0)
		element_count--;
}

template<class T> inline void RVector<T>::insert(int index)
{
	increment();
	memmove(&array[index + 1],&array[index],
		(element_count - index - 1) * sizeof(value_type));
}

template<class T> inline void RVector<T>::insert(iterator p,const T& x)
{
	int index = p - array;
	insert(index);
	array[index] = x;
}

template<class T> inline void RVector<T>::erase(int index)
{
	memmove(&array[index],&array[index + 1],
		(element_count - index - 1) * sizeof(value_type));
	decrement();
}

template<class T> inline void RVector<T>::erase(iterator q)
{
	erase(q - array);
}

template<class T> inline T& RVector<T>::first()
{
	return array[0];
}

template<class T> inline const T& RVector<T>::first() const
{
	return array[0];
}

template<class T> inline T& RVector<T>::last()
{
	return array[element_count - 1];
}

template<class T> inline const T& RVector<T>::last() const
{
	return array[element_count - 1];
}

template<class T> inline typename RVector<T>::iterator RVector<T>::begin()
{
	return &array[0];
}

template<class T> inline typename RVector<T>::const_iterator RVector<T>::begin() const
{
	return &array[0];
}

template<class T> inline typename RVector<T>::iterator RVector<T>::end()
{
	return &array[element_count];
}

template<class T> inline typename RVector<T>::const_iterator RVector<T>::end() const
{
	return &array[element_count];
}

template<class T> inline void RVector<T>::clear()
{
	element_count = 0;
}

template<class T> inline T& RVector<T>::front()
{
	return *begin();
}

template<class T> inline const T& RVector<T>::front() const
{
	return *begin();
}

template<class T> inline T& RVector<T>::back()
{
	return *end();
}

template<class T> inline const T& RVector<T>::back() const
{
	return *end();
}

template<class T> inline void RVector<T>::push_front(const T& x)
{
	insert(0);
	array[0] = x;
}

template<class T> inline void RVector<T>::push_back(const T& x)
{
	increment();
	array[element_count - 1] = x;
}

template<class T> inline void RVector<T>::pop_front()
{
	erase(0);
}

template<class T> inline void RVector<T>::pop_back()
{
	decrement();
}

template<class T> inline T& RVector<T>::operator[](int index)
{
	return array[index];
}

template<class T> inline const T& RVector<T>::operator[](int index) const
{
	return array[index];
}

//-----------------------------------------------------------------------------

template<class T> inline int RVector<T>::memSize() const
{
    return capacity() * sizeof(T);
}

template<class T> inline T* RVector<T>::address() const
{
	return array;
}

template<class T> inline int RVector<T>::setSize(int size)
{
	if (size > array_size)
		resize(size);
	else
		element_count = size;
	return element_count;
}

template<class T> inline int RVector<T>::size() const
{
    return element_count;
}

template<class T> inline int RVector<T>::max_size() const
{
    return M_MAX_INT >> 2;
}

template<class T> inline bool RVector<T>::empty() const
{
    return element_count == 0;
}

template<class T> inline int RVector<T>::capacity() const
{
    return array_size;
}


//-----------------------------------------------------------------------------

template<class T> void RVector<T>::reserve(int size)
{
	if (size > array_size) {
		int ec = element_count;
		if (resize(size))
			element_count = ec;
	}
}

//-----------------------------------------------------------------------------

// code to merge a RVector into this one
template<class T>
inline void RVector<T>::merge(const RVector& p)
{
	if (p.size()) {
		int oldsize = size();
		resize(oldsize + p.size());
		memcpy( &array[oldsize], p.address(), p.size() * sizeof(T) );
	}
}

//-----------------------------------------------------------------------------
// RVectorPtr: for vectors of pointers.
//-----------------------------------------------------------------------------

template <class T> 
class RVectorPtr : public RVector<void*>
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
	typedef RVector<void*> Parent;
	T& first();
	T& last();
	const T& first() const;
	const T& last() const;
};


//-----------------------------------------------------------------------------

template<class T> inline T& RVectorPtr<T>::first()
{
	return (T&)Parent::first();
}

template<class T> inline const T& RVectorPtr<T>::first() const
{
	return (const T)Parent::first();
}

template<class T> inline T& RVectorPtr<T>::last()
{
	return (T&)Parent::last();
}

template<class T> inline const T& RVectorPtr<T>::last() const
{
	return (const T&)Parent::last();
}


//-----------------------------------------------------------------------------

template<class T> inline typename RVectorPtr<T>::iterator RVectorPtr<T>::begin()
{
	return (iterator)Parent::begin();
}

template<class T> inline typename RVectorPtr<T>::const_iterator RVectorPtr<T>::begin() const
{
	return (const_iterator)Parent::begin();
}

template<class T> inline typename RVectorPtr<T>::iterator RVectorPtr<T>::end()
{
	return (iterator)Parent::end();
}

template<class T> inline typename RVectorPtr<T>::const_iterator RVectorPtr<T>::end() const
{
	return (const_iterator)Parent::end();
}

template<class T> inline void RVectorPtr<T>::insert(iterator i,const T& x)
{
   Parent::insert( (Parent::iterator)i, (Parent::reference)x );   
}

template<class T> inline void RVectorPtr<T>::erase(iterator i)
{
   Parent::erase( (Parent::iterator)i );   
}

template<class T> inline T& RVectorPtr<T>::front()
{
	return *begin();
}

template<class T> inline const T& RVectorPtr<T>::front() const
{
	return *begin();
}

template<class T> inline T& RVectorPtr<T>::back()
{
	return *end();
}

template<class T> inline const T& RVectorPtr<T>::back() const
{
	return *end();
}

template<class T> inline void RVectorPtr<T>::push_front(const T& x)
{
	Parent::push_front((Parent::const_reference)x);
}

template<class T> inline void RVectorPtr<T>::push_back(const T& x)
{
	Parent::push_back((Parent::const_reference)x);
}

template<class T> inline T& RVectorPtr<T>::operator[](int index)
{
	return (T&)Parent::operator[](index);
}

template<class T> inline const T& RVectorPtr<T>::operator[](int index) const
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