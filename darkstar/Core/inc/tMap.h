//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------

#ifndef _TMAP_H_
#define _TMAP_H_

#include <tsorted.h>

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
// Not very efficient right now on insertion, sorts after every insert.
// The Key and Value types may be classes with construtors etc.  The
// Map class correctly initializes and destroys its element, unlike
// the Vector template.

// The Key and Value classes must have a default constructor.  The
// sorting is done by using != and < operators on the Key.

// Find currently copies the key to build an element to search with,
// so it's best if the key has a light weight copy operation.

template<class Key, class Value>
class Map
{
public:
	struct Element {
		Key first;
		Value second;
		Element() {}
		Element(const Element& elm) : first(elm.first), second(elm.second) {}
		Element(const Key& k, const Value& val) : first(k), second(val) {}
		~Element() {}
		bool operator<(const Element& e) const { return first < e.first; }
	};

private:
	Element searchKey;
	SortableVector<Element> data;

public:
	typedef Element value_type;
	typedef Element& reference;
	typedef const Element& const_reference;

	typedef typename SortableVector<Element>::iterator iterator;
	typedef typename SortableVector<Element>::const_iterator const_iterator;
	typedef int difference_type;
	typedef int size_type;

	Map();
	~Map();

	iterator begin()             { return data.begin(); }
	const_iterator begin() const { return data.begin(); }
	iterator end()               { return data.end(); }
	const_iterator end() const   { return data.end(); }

	size_type size() const;
	bool empty() const;

	value_type& front();
	const value_type& front() const;
	value_type& back();
	const value_type& back() const;

	void erase(const iterator itr);

	iterator find(const Key&);
	void insert(const value_type& val);

	void insert(const Key&, const Value& );
};


//-----------------------------------------------------------------------------

template<class Key, class Value>
inline Map<Key,Value>::Map()
{
}

template<class Key, class Value>
Map<Key,Value>::~Map()
{
	// Call destructors for all elements
	for (iterator itr = begin(); itr != end(); itr++)
		destroy(itr);
}

template<class Key, class Value>
inline typename Map<Key,Value>::size_type Map<Key,Value>::size () const
{
	return data.size();
}

template<class Key, class Value>
inline bool Map<Key,Value>::empty () const
{
	return size() != 0;
}

template<class Key, class Value>
inline typename Map<Key,Value>::value_type& Map<Key,Value>::front()
{
	return data.front();
}

template<class Key, class Value>
inline const typename Map<Key,Value>::value_type& Map<Key,Value>::front() const
{
	return data.front();
}

template<class Key, class Value>
inline typename Map<Key,Value>::value_type& Map<Key,Value>::back()
{
	return data.back();
}

template<class Key, class Value>
inline const typename Map<Key,Value>::value_type& Map<Key,Value>::back() const
{
	return data.back();
}

template<class Key, class Value>
inline void Map<Key,Value>::erase(const iterator itr)
{
	destroy(itr);
	data.erase(itr);
}

template<class Key, class Value>
inline typename Map<Key,Value>::iterator Map<Key,Value>::find(const Key& first)
{
	searchKey.first = first;
	return data.find(searchKey);
}

template<class Key, class Value>
inline void Map<Key,Value>::insert(const value_type& val)
{
	data.increment();
	new(&data.last()) Element(val);
	data.sort();
}

template<class Key, class Value>
inline void Map<Key,Value>::insert(const Key& k, const Value& v)
{
	data.increment();
	new(&data.last()) Element(k,v);
	data.sort();
}

#endif
