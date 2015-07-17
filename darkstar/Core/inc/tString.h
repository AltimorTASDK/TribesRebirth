//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _TSTRING_H_
#define _TSTRING_H_

//Includes
#include <persist.h>
#include <base.h>
// Don't want STL string.h, but do want the standard one?
//#include <string.h>


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
// Not really a template.  Just a simple not very efficient string
// class.  Mainly built so it could be used in the Map template as
// either a Key or Value. NULL string support is shaky.

// Part of the ever-growing support for STL--

class String
{
	char* string;

	void assign(const char* p);

public:
	String();
	String(const char*);
	String(const String&);
	~String();

	typedef char value_type;
	typedef char& reference;
	typedef const char& const_reference;

	typedef char* iterator;
	typedef const char* const_iterator;
	typedef int difference_type;
	typedef int size_type;

	const char* c_str() const { return string; }

	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	size_type length() const;
	size_type size()   const;
	bool      empty()  const;

	char& front();
	const char& front() const;
	char& back();
	const char& back() const;

   char& operator[](int);
   const char& operator[](int) const;

	void operator=(const char* p);
	bool operator==(const char* p) const;
	bool operator!=(const char* p) const;
	bool operator<(const char* p) const;

	void operator=(const String& p);
	bool operator==(const String& p) const;
	bool operator!=(const String& p) const;
	bool operator<(const String& p) const;
};


//-----------------------------------------------------------------------------

inline void String::assign(const char* p)
{
	if (p) {
		string = new char[strlen(p)+1];
		strcpy(string,p);
	}
	else
		string = 0;
}

inline String::String()
{
	string = 0;
}

inline String::String(const char* p)
{
	assign(p);
}

inline String::String(const String& s)
{
	assign(s.string);
}

inline String::~String()
{
	delete [] string;
}

inline String::iterator String::begin()
{
	return string;
}

inline String::const_iterator String::begin() const
{
	return string;
}

inline String::iterator String::end()
{
	return string? &string[length()]: 0;
}

inline String::const_iterator String::end() const
{
	return string? &string[length()]: 0;
}

inline String::size_type String::length () const
{
	return string? strlen(string): 0;
}

inline String::size_type String::size() const
{
	return length();
}

inline bool String::empty() const
{
	return length() == 0;
}

inline char& String::front()
{
	AssertFatal(string != 0,"String::front: Null string");
	return string[0];
}

inline const char& String::front() const
{
	AssertFatal(string != 0,"String::front: Null string");
	return string[0];
}

inline char& String::back()
{
	AssertFatal(string != 0,"String::back: Null string");
	return *(end() + 1);
}

inline const char& String::back() const
{
	AssertFatal(string != 0,"String::back: Null string");
	return *(end() + 1);
}

inline char& String::operator[](int index)
{
	AssertFatal(string != 0,"String::operator[]: Null string");
	return string[index];
}

inline const char& String::operator[](int index) const
{
	AssertFatal(string != 0,"String::operator[]: Null string");
	return string[index];
}

inline void String::operator=(const char* p)
{
	if (p != string) {
		delete [] string;
		assign(p);
	}
}

inline bool String::operator==(const char* p) const
{
	AssertFatal(string != 0,"String::operator==: Null string");
	return !stricmp(string,p);
}

inline bool String::operator!=(const char* p) const
{
	AssertFatal(string != 0,"String::operator!=: Null string");
	return !operator==(p);
}

inline bool String::operator<(const char* p) const
{
	AssertFatal(string != 0,"String::operator<: Null string");
	return stricmp(string,p) < 0;
}

inline void String::operator=(const String& p)
{
	if (p.string != string) {
		delete [] string;
		assign(p.string);
	}
}

inline bool String::operator==(const String& p) const
{
	AssertFatal(string != 0,"String::operator==: Null string");
	return !stricmp(string,p.string);
}

inline bool String::operator!=(const String& p) const
{
	AssertFatal(string != 0,"String::operator!=: Null string");
	return !operator==(p);
}

inline bool String::operator<(const String& p) const
{
	AssertFatal(string != 0,"String::operator<: Null string");
	return stricmp(string,p.string) < 0;
}


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_TSTRING_H_

