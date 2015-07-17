//----------------------------------------------------------------------------

//	$Workfile:   diction.h  $
//	$Version$
//	$Revision:   1.0  $
//	$Date:   16 Oct 1995 16:39:40  $

//----------------------------------------------------------------------------


#ifndef _DICTION_H_
#define _DICTION_H_

#include <base.h>
#include <m_qsort.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//----------------------------------------------------------------------------
// Map of names to IDs

template<class IDValue>
class TPage {
public:
	struct Entry {
		IDValue id;
		char* name;
	};
	enum Constants {
		DefaultSize = 40,
		DefaultBufferSize = DefaultSize * 8,
	};
	TPage();
	TPage* next;
	void defragment();
	int entryCount;
	int bufferEnd;
	int bufferAvail;
	Entry entries[DefaultSize];
	char stringBuffer[DefaultBufferSize];
};


template<class IDValue>
TPage<IDValue>::TPage()
{
	next = 0;
	entryCount = 0;
	bufferEnd = 0;
	bufferAvail = DefaultBufferSize;
}

template<class IDValue>
void TPage<IDValue>::defragment()
{
	// compact the string buffer so that all available space
	// is at the end of the buffer.

	// note: bufferAvail doesn't change, but bufferEnd does

   char temp[DefaultBufferSize];
   char *next = temp;
   bufferEnd = 0;
   for( int e = 0; e < entryCount; e++ ) {
      int size = strlen( entries[e].name ) + 1;
      strcpy( next, entries[e].name );
      entries[e].name = &stringBuffer[bufferEnd];
      bufferEnd += size;
      next += size;
   }
	memcpy( stringBuffer, temp, bufferEnd );
}

template<class IDValue>
class Dictionary
{
   typedef TPage<IDValue>  Page;
   typedef Page::Entry  Entry;

	Page* firstPage;

	static __cdecl compare(const void*,const void*);
public:
	Dictionary();
	~Dictionary();
	void add(const char* name, const IDValue & id);
	Bool remove(const IDValue & id);
	Bool remove(const char * name);
	int	getCount() const;
	const IDValue * map(const char* name);
	const char * find(const IDValue & id);
};


template<class IDValue>
Dictionary<IDValue>::Dictionary()
{
	firstPage = 0;
}

template<class IDValue>
Dictionary<IDValue>::~Dictionary()
{
	for (Page *page = firstPage; page; ) {
		Page* delPage = page;
		page = page->next;
		delete delPage;
	}
	firstPage = 0;
}

template<class IDValue>
int Dictionary<IDValue>::getCount() const
{
	int	count = 0;
	for( Page * page = firstPage; page; page = page->next )
		count += page->entryCount;
	return count;
}

template<class IDValue>
int __cdecl Dictionary<IDValue>::compare(const void* a,const void* b)
{
	return strcmp(reinterpret_cast<const Entry*>(a)->name,
		reinterpret_cast<const Entry*>(b)->name);
}

template<class IDValue>
void Dictionary<IDValue>::add(const char* name, const IDValue &id)
{
	int size = strlen(name) + 1;
	AssertFatal(size <= Page::DefaultBufferSize,
		"Dictionary::add: String name longer than max size");

	// Search for the first page with enough room to
	// store the string.
	Page* page = firstPage;
	if (!page)
		page = firstPage = new Page;
	else
		for(; page->entryCount == Page::DefaultSize ||
			page->bufferAvail < size;
			page = page->next)
			if (!page->next) {
				page->next = new Page;
				page = page->next;
				break;
			}

	// if there is enough space, but not all at the end of
	// the buffer, defragment the buffer:

	if( page->bufferEnd + size > Page::DefaultBufferSize )
		page->defragment();


	// Add entry and sort
	Entry& de = page->entries[page->entryCount++];
	de.id = id;
	de.name = &page->stringBuffer[page->bufferEnd];
	strcpy(de.name,name);
	page->bufferEnd += size;
	page->bufferAvail -= size;

	m_qsort(page->entries,page->entryCount,sizeof(Entry),
		Dictionary::compare);
}

template<class IDValue>
const char * Dictionary<IDValue>::find( const IDValue &id )
{
	for (Page *page = firstPage; page; page = page->next)
		{
		Entry *pe = page->entries;
		for( int e = 0; e < page->entryCount; e++, pe++ )
			if( pe->id == id )
				return pe->name;
		}
	return 0;
}

template<class IDValue>
Bool Dictionary<IDValue>::remove( const IDValue &id )
{
	for (Page *page = firstPage; page; page = page->next)
		{
		Entry *pe = page->entries;
		for( int e = 0; e < page->entryCount; e++, pe++ )
			if( pe->id == id )
				{
				page->bufferAvail += strlen( pe->name ) + 1;
				if( --page->entryCount )
					{
					// copy the last entry to this spot:
					*pe = page->entries[page->entryCount];

					// resort entries:
					m_qsort(pe,page->entryCount - e,sizeof(Entry),
						Dictionary::compare);
					}
				return TRUE;
				}
		}
	return FALSE;
}

template<class IDValue>
Bool Dictionary<IDValue>::remove( const char * name )
{
	Entry key;
	key.name = const_cast<char*>(name);
	for (Page *page = firstPage; page; page = page->next)
		{
      void* vp = bsearch(&key,page->entries,page->entryCount,
			sizeof(Entry),Dictionary::compare);
		if (vp)
			{
			page->bufferAvail += strlen( name ) + 1;

			Entry * pe = reinterpret_cast<Entry*>(vp);
			*pe = page->entries[--page->entryCount];

			m_qsort(page->entries,page->entryCount,sizeof(Entry),
				Dictionary::compare);
			return TRUE;
			}
      }
	return FALSE;
}

template<class IDValue>
const IDValue * Dictionary<IDValue>::map(const char* name )
{
	Entry key;
	key.name = const_cast<char*>(name);
	for (Page *page = firstPage; page; page = page->next)
      {
		void* vp = bsearch(&key,page->entries,page->entryCount,
			sizeof(Entry),Dictionary::compare);
      if( vp )
			{
			return &reinterpret_cast<Entry*>(vp)->id;
			}
      }
	return 0;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
