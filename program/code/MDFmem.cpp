//---------------------------------------------------------------------------

// $Workfile:   memory.cpp  $
// $Version$
// $Revission$
// $Date:   16 Nov 1994 19:23:50  $
// $Log:   N:\arena\code\base\vcs\memory.cpv  $
//	

//---------------------------------------------------------------------------
 
//---------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <base.h>

#include <windows.h>
#include "types.h"

int CommitMin = 1024*1024*2;
int ResizeMin = 1024*1024*2;

enum {
   HeaderReserveSize = 200000,
   HeaderResizeAmt = 128,
   MinChunkSize = 8 * 1024 * 32, // size (in cache lines) of each Chunk - max 8 meg allocs
   ResizeLinesSize = 8 * 1024 * 32,
};

//---------------------------------------------------------------------------

struct CacheLine;

struct MemoryHeader
{
   MemoryHeader *next;
   MemoryHeader *prev;
   CacheLine *ptr;
   int size; // in cache lines
   MemoryHeader *nextInBucket;
   MemoryHeader *prevInBucket;
   int flags;
   int pad;
};

struct CacheLine
{
   MemoryHeader *header;
   int fill[7];
};

//---------------------------------------------------------------------------

extern "C" {
void __cdecl UnloadGlide();
};

static void MemoryError()
{
   MessageBox(NULL, "Error allocating virtual memory.  Make sure you have at least 120 MB free disk swapfile space.  Check http://www.tribesplayers.com for troubleshooting help.", "ERROR", MB_OK);
   UnloadGlide();
   exit(-1);
}

static void Heapcheck(MemoryHeader *head)
{
   while(head)
   {
      if(head->prev)
      {
         AssertFatal(head->prev->next == head, "DOH!");
         AssertFatal(head->prev->ptr + head->prev->size == head->ptr, "Dho!");
      }
      if(head->flags & 1)
      {
         AssertFatal(head->ptr->header == head, "Doh2!");
      }
      head = head->next;   
   }   
}

class Memory
{
	friend class MemoryIterator;

   enum {
      Allocated = 1,
      Array = 2,
      BucketCount = 25,
   };
   MemoryHeader *headerBase;
   MemoryHeader *buckets[BucketCount]; // max of 8-meg bucket

   MemoryHeader *headerFreeList;                   

   int            numCommittedHeaders;
   int            numUsedHeaders;
	CRITICAL_SECTION lock;
   static bool initialized;
public:
   void           bucketize(MemoryHeader *);
   void           removeFromBucket(MemoryHeader *);
   MemoryHeader*  newHeader();
   void           freeHeader(MemoryHeader *);
	void 				init();
	void*				alloc(int size, bool array);
	void* 			realloc(void* mem,int size);
	void				free(void *mem, bool array);
   void           allocChunk(DWORD size);
};

//---------------------------------------------------------------------------
// Default memory object used to override the standard
// library new, delete, malloc,calloc,realloc & free.

//int (far _cdecl *_malloc_handler)(void);


//---------------------------------------------------------------------------

extern Memory Malloc;
bool Memory::initialized = false;

//---------------------------------------------------------------------------
// Memory should be long word aligned.

static int countNextBits(int num)
{
   int temp = num;
   int count = 0;
   while((temp >>= 1) != 0)
      count++;

   if(num != (1 << count))
      return count + 1;
   return count;
}

static int countBits(int num)
{
   int temp = num;
   int count = 0;
   while((temp >>= 1) != 0)
      count++;

   return count;
}

void Memory::init()
{
   initialized = true;
   headerBase = (MemoryHeader *) VirtualAlloc(NULL, HeaderReserveSize * sizeof(MemoryHeader), MEM_RESERVE, PAGE_READWRITE);

   if(!headerBase)
      MemoryError();

   if(!VirtualAlloc(headerBase, sizeof(MemoryHeader), MEM_COMMIT, PAGE_READWRITE))
      MemoryError();

   numUsedHeaders = 0;
   numCommittedHeaders = 0;

   // alloc a big block:
   for(int i = 0; i < BucketCount; i++)
      buckets[i] = NULL;

   headerFreeList = NULL;
	InitializeCriticalSection(&lock);
}

void Memory::allocChunk(DWORD chunkSize)
{
   if(chunkSize < MinChunkSize)
      chunkSize = MinChunkSize;
      
   void *base = VirtualAlloc(NULL, (chunkSize + 1) << 5, MEM_RESERVE, PAGE_READWRITE);
   if(!VirtualAlloc(base, (chunkSize + 1) << 5, MEM_COMMIT, PAGE_READWRITE))
      MemoryError();

   MemoryHeader *hdr = newHeader();
   hdr->flags = 0;
   hdr->ptr = (CacheLine *) (DWORD(base) + 28);
   hdr->size = chunkSize;
   hdr->next = NULL;
   hdr->prev = NULL;
   bucketize(hdr);
}

MemoryHeader *Memory::newHeader()
{
   if(headerFreeList)
   {
      MemoryHeader *temp = headerFreeList;
      headerFreeList = headerFreeList->next;
      return temp;
   }
   else
   {
      MemoryHeader *ret = headerBase + numUsedHeaders;
      numUsedHeaders++;
      if(numUsedHeaders <= numCommittedHeaders)
         return ret;

      numCommittedHeaders += HeaderResizeAmt;
      if(!VirtualAlloc(headerBase, sizeof(MemoryHeader) * numCommittedHeaders, MEM_COMMIT, PAGE_READWRITE))
         MemoryError();
      return ret;
   }
}

void Memory::freeHeader(MemoryHeader *header)
{
   header->next = headerFreeList;
   headerFreeList = header;
}

//---------------------------------------------------------------------------

void* Memory::alloc(int size, bool array)
{
	if (!initialized)
      init();
	if (!size)
		return 0;
	EnterCriticalSection(&lock);
	// Expand size to include header and round up to
	// nearest block size boundry.
	//

   size = (size + 4 + 31) >> 5; // compute cache line size.

   int bucket = countNextBits(size);
   int i;

   for(i = bucket; i < BucketCount; i++)
      if(buckets[i])
         break;
   
   MemoryHeader *header;
   if(i == BucketCount)
   {
      // allocate a bigblock...
      allocChunk(size);
      for(i = bucket; i < BucketCount;i++)
         if(buckets[i])
            break;
      
      AssertFatal(i != BucketCount, "Internal memory error.");
   }

   // unlink the first guy in bucket[i]
   header = buckets[i];
   buckets[i] = header->nextInBucket;
   if(buckets[i])
      buckets[i]->prevInBucket = NULL;

   header->ptr->header = header;
   header->flags = array ? Allocated | Array : Allocated;

   void *ret = &(header->ptr->fill[0]);

   if(header->size != size)
   {
      // there's free stuff left over at the end.
      MemoryHeader *nHeader = newHeader();

      nHeader->size = header->size - size;
      header->size = size;

      nHeader->flags = 0;

      nHeader->prev = header;
      nHeader->next = header->next;
      header->next = nHeader;
      nHeader->ptr = header->ptr + header->size;

      if(nHeader->next)
         nHeader->next->prev = nHeader;

      bucketize(nHeader);
   }
	LeaveCriticalSection(&lock);
   return ret;
}

//---------------------------------------------------------------------------

void Memory::free(void *mem, bool array)
{
	if (!mem)
		return;
   
	EnterCriticalSection(&lock);
   CacheLine *ptr = (CacheLine *) (DWORD(mem) - 4);
   MemoryHeader *hdr = ptr->header;

   AssertFatal(hdr >= headerBase && hdr < headerBase + numUsedHeaders, "Invalid freed block.");
   AssertFatal(hdr->ptr == ptr, "Bogus header pointer.");
   AssertFatal(hdr->flags & Allocated, "Not an allocated block!");
   AssertFatal(bool(hdr->flags & Array) == array, "Array alloc mismatch.");

   hdr->flags = 0;

   // fill the block with the fill value
   
#ifdef DEBUG
   memset(ptr, 0xCC, hdr->size * 32);
#endif

   // see if we can merge hdr with the block after it.

   MemoryHeader *next = hdr->next;
   if(next && next->flags == 0)
   {
      removeFromBucket(next);
      hdr->size += next->size;
      hdr->next = next->next;
      if(next->next)
         next->next->prev = hdr;
      freeHeader(next);
   }   

   // see if we can merge hrd with the block before it.
   MemoryHeader *prev = hdr->prev;

   if(prev && prev->flags == 0)
   {
      removeFromBucket(prev);
      prev->size += hdr->size;
      prev->next = hdr->next;
      if(hdr->next)
         hdr->next->prev = prev;
      freeHeader(hdr);

      hdr = prev;
   }
   
   // throw this puppy in a bucket!
   bucketize(hdr);
	LeaveCriticalSection(&lock);
}

void Memory::removeFromBucket(MemoryHeader *hdr)
{
   MemoryHeader *prev = hdr->prevInBucket;
   MemoryHeader *next = hdr->nextInBucket;
   
   if(prev)
      prev->nextInBucket = next;
   else
   {
      // find out which bucket this one is in.
      int bucket = countBits(hdr->size);
      AssertFatal(buckets[bucket] == hdr, "Bad bucket pointer.");
      buckets[bucket] = next;
   }
   if(next)
      next->prevInBucket = prev;
}

void Memory::bucketize(MemoryHeader *hdr)
{
   int bucket = countBits(hdr->size);
   hdr->nextInBucket = buckets[bucket];
   hdr->prevInBucket = NULL;

   if(buckets[bucket])
      buckets[bucket]->prevInBucket = hdr;
   buckets[bucket] = hdr;
   //memset(&(hdr->ptr->fill[0]), 0xCD, (hdr->size << 5) - 4);
   hdr->ptr->header = hdr;
}


//---------------------------------------------------------------------------
// First attempts to resize the block where it is, if that
// fails it will try to move it.  If the request fails, the
// original block is left alone.
//
void* Memory::realloc(void* mem,int size)
{
	if (!size) {
		free(mem, false);
		return 0;
	}
   if(!mem)
      return alloc(size, false);
	EnterCriticalSection(&lock);

   CacheLine *ptr = (CacheLine *) (DWORD(mem) - 4);
   MemoryHeader *hdr = ptr->header;

   AssertFatal(hdr >= headerBase && hdr < headerBase + numUsedHeaders, "Invalid freed block.");
   AssertFatal(hdr->ptr == ptr, "Bogus header pointer.");
   AssertFatal((hdr->flags & Allocated) == Allocated, "Bad block flags.");

   size = (size + 4 + 31) >> 5; // compute cache line size.

   void *ret;

   if(size <= hdr->size)
      ret = mem;
   else
   {
      MemoryHeader *next = hdr->next;
      if(next && next->flags == 0 && next->size + hdr->size >= size)
      {
         // we can merge with the next dude.
         removeFromBucket(next);
         int delta = size - hdr->size;
         next->size -= delta;
         next->ptr += delta;

         hdr->size = size;
         if(next->size)
            bucketize(next);
         else
         {
            hdr->next = next->next;
            if(next->next)
               next->next->prev = hdr;
            freeHeader(next);
         }
         ret = mem;
      }
      else
      {
         ret = alloc(size << 5, false);
         memcpy(ret, mem, hdr->size << 5);
         free(mem, false);
      }
   }
	LeaveCriticalSection(&lock);
	return ret;
}

//----------------------------------------------------------------------------
//-----------------------------------------------------------------
// Throw in the microsoft memory library if MEM_DLLNAME is found,
// otherwise use the stuff above
//-----------------------------------------------------------------

#define MemFreeID          UInt32('FBLK')
#define MemAllocID         UInt32('ABLK')

#define MEM_DLLNAME        "mem.dll"
#define MALLOC_FUNC_NAME   "MS_Malloc"
#define FREE_FUNC_NAME     "MS_Free"
#define REALLOC_FUNC_NAME  "MS_Realloc"
#define CALLOC_FUNC_NAME   "MS_Calloc"

class MemPlugin
{
   private:
      
   public:
      static HINSTANCE  dllInstance;
   
      struct Header
      {
         UInt32   memTag;
         UInt32   size;
         UInt32   stackPtr;
         UInt32   __padding;
      };
      
      struct Footer
      {
         UInt32   memTag;
      };
      
      typedef void * (_cdecl * MALLOCFUNC)(size_t size);
      typedef void (_cdecl * FREEFUNC)(void * mem);
      typedef void * (_cdecl * REALLOCFUNC)(void * mem, size_t size);
      typedef void * (_cdecl * CALLOCFUNC)(size_t num, size_t size);

      // the memory functions
      static MALLOCFUNC    Malloc;
      static FREEFUNC      Free;
      static REALLOCFUNC   Realloc;
      static CALLOCFUNC    Calloc;
      static bool          skipPlugin;
      
#ifdef DEBUG
      static MALLOCFUNC    dllMalloc;
      static FREEFUNC      dllFree;
      static REALLOCFUNC   dllRealloc;
      static CALLOCFUNC    dllCalloc;
      
      static void * debugMalloc(size_t size);
      static void debugFree(void * mem);
      static void * debugRealloc(void * mem, size_t size);
      static void * debugCalloc(size_t num, size_t size);
#endif
      static void init();
};

HINSTANCE               MemPlugin::dllInstance;
bool                    MemPlugin::skipPlugin;
MemPlugin::MALLOCFUNC   MemPlugin::Malloc;
MemPlugin::FREEFUNC     MemPlugin::Free;
MemPlugin::REALLOCFUNC  MemPlugin::Realloc;
MemPlugin::CALLOCFUNC   MemPlugin::Calloc;

#ifdef DEBUG
MemPlugin::MALLOCFUNC   MemPlugin::dllMalloc;
MemPlugin::FREEFUNC     MemPlugin::dllFree;
MemPlugin::REALLOCFUNC  MemPlugin::dllRealloc;
MemPlugin::CALLOCFUNC   MemPlugin::dllCalloc;
#endif

Memory Malloc;

//---------------------------------------------------------------------------

void MemPlugin::init()
{
   // load up the dll
   dllInstance = LoadLibrary(MEM_DLLNAME);
   
   if(dllInstance == NULL)
   {
      skipPlugin = true;
      return;
   }
   
   // grab the functions
   Malloc = (MALLOCFUNC)GetProcAddress(dllInstance, MALLOC_FUNC_NAME);
   Free = (FREEFUNC)GetProcAddress(dllInstance, FREE_FUNC_NAME);
   Realloc = (REALLOCFUNC)GetProcAddress(dllInstance, REALLOC_FUNC_NAME);
   Calloc = (CALLOCFUNC)GetProcAddress(dllInstance, CALLOC_FUNC_NAME);

   // make sure grabbed the function pointers
   if(!(Malloc && Free && Realloc && Calloc))
   {
      FreeLibrary(dllInstance);
      dllInstance = NULL;
      AssertISV(dllInstance, "Failed to load memory library.");
   }
   
#ifdef DEBUG
   dllMalloc = Malloc;
   dllFree = Free;
   dllRealloc = Realloc;
   dllCalloc = Calloc;
   
   Malloc = &debugMalloc;
   Free = &debugFree;
   Realloc = &debugRealloc;
   Calloc = &debugCalloc;
#endif
}

//---------------------------------------------------------------------------

#ifdef DEBUG
void * MemPlugin::debugMalloc(size_t size)
{
   UInt32 here;
   
   Header * header = (Header*)dllMalloc(size + sizeof(Header) + sizeof(Footer));
   header->memTag = MemAllocID;
   header->size = size;
   header->stackPtr = (UInt32)(((UInt32*)&here)[5]);
   
   Footer * footer = (Footer*)(((char*)header) + sizeof(Header) + size);
   footer->memTag = MemAllocID;
   
   return((void*)(header + 1));
}

//---------------------------------------------------------------------------

void MemPlugin::debugFree(void * mem)
{
   if(mem)
   {
      Header * header = ((Header*)mem) - 1;

      AssertISV(header->memTag != MemFreeID, "MemPlugin::debugFree - memory already deleted");
      AssertISV(header->memTag == MemAllocID, "MemPlugin::debugFree - invalid memory header");

      Footer * footer = (Footer*)(((char*)header) + sizeof(Header) + header->size);
      AssertISV(footer->memTag == MemAllocID, "MemPlugin::debugFree - buffer overrun");

      header->memTag = MemFreeID;

      dllFree((void*)header);
   }
}

//---------------------------------------------------------------------------

void * MemPlugin::debugRealloc(void * mem, size_t size)
{
   if(!mem)
      return(debugMalloc(size));
      
   if(!size)
   {
      debugFree(mem);
      return(NULL);                   
   }

   Header * header = ((Header*)mem) - 1;
   
   AssertISV(header->memTag != MemFreeID, "MemPlugin::debugRealloc - memory already deleted");
   AssertISV(header->memTag == MemAllocID, "MemPlugin::debugRealloc - invalid memory header");

   Footer * footer = (Footer*)(((char*)header) + sizeof(Header) + header->size);
   AssertISV(footer->memTag == MemAllocID, "MemPlugin::debugRealloc - buffer overrun");

	UInt32 there = header->stackPtr;
   header->size = size;
   header = (Header *)dllRealloc(header, size + sizeof(Header) + sizeof(Footer));
	header->stackPtr = there;
   footer = (Footer*)(((char*)header) + sizeof(Header) + header->size);
   footer->memTag = MemAllocID;

   
   return((void*)(header + 1));
}

//---------------------------------------------------------------------------

void * MemPlugin::debugCalloc(size_t num, size_t size)
{
   Header * header = (Header*)dllCalloc(num, size + sizeof(Header) + sizeof(Footer));
   header->memTag = MemAllocID;
   header->size = num * size;
   
   Footer * footer = (Footer*)(((char*)header) + sizeof(Header) + header->size);
   footer->memTag = MemAllocID;
   
   return((void*)(header + 1));
}
#endif

//---------------------------------------------------------------------------

void * _cdecl operator new(size_t size)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
   
   if(!MemPlugin::skipPlugin)
      return MemPlugin::Malloc(size);
      
   return Malloc.alloc(size,false);
}

//---------------------------------------------------------------------------

void * _cdecl operator new[](size_t size)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
      
   if(!MemPlugin::skipPlugin)
      return MemPlugin::Malloc(size);
      
   return Malloc.alloc(size, true);
}

//---------------------------------------------------------------------------

void* operator new(size_t size,Memory& mem)
{
	return mem.alloc(size, false);
}

//---------------------------------------------------------------------------

void _cdecl operator delete(void* mem)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
      
   if(!MemPlugin::skipPlugin)
   {
      MemPlugin::Free(mem);
      return;
   }
      
   Malloc.free(mem, false);
}

//---------------------------------------------------------------------------

void _cdecl operator delete[](void* mem)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();

   if(!MemPlugin::skipPlugin)
   {
      MemPlugin::Free(mem);
      return;
   }

	Malloc.free(mem, true);
}

//---------------------------------------------------------------------------

void * _cdecl malloc(size_t size)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
      
   if(!MemPlugin::skipPlugin)
      return MemPlugin::Malloc(size);
   
   return Malloc.alloc(size, false);
}

//---------------------------------------------------------------------------

void _cdecl free(void * mem)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
      
   if(!MemPlugin::skipPlugin)
   {
      MemPlugin::Free(mem);
      return;
   }
      
   Malloc.free(mem, false);
}

//---------------------------------------------------------------------------

void * _cdecl realloc(void * mem, size_t size)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
      
   if(!MemPlugin::skipPlugin)
   	return MemPlugin::Realloc(mem, size);
   
	return Malloc.realloc(mem,size);
}

//---------------------------------------------------------------------------

void * _cdecl calloc(size_t num, size_t size)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
   
   if(!MemPlugin::skipPlugin)
      return MemPlugin::Calloc(num, size);
      
	void* mem;
	if ((mem = Malloc.alloc(num * size, false)) != 0)
		memset(mem,0,num * size);
	return mem;
}

//---------------------------------------------------------------------------

void * _cdecl std::malloc(size_t size)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
      
   if(!MemPlugin::skipPlugin)
      return MemPlugin::Malloc(size);
      
	return Malloc.alloc(size, false);
}

//---------------------------------------------------------------------------

void _cdecl std::free(void * mem)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
      
   if(!MemPlugin::skipPlugin)
   {
      MemPlugin::Free(mem);
      return;
   }
   
	Malloc.free(mem, false);
}

//---------------------------------------------------------------------------

void * _cdecl std::realloc(void * mem, size_t size)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
      
   if(!MemPlugin::skipPlugin)
   	return MemPlugin::Realloc(mem,size);
      
	return Malloc.realloc(mem,size);
}

//---------------------------------------------------------------------------

void * _cdecl std::calloc(size_t num, size_t size)
{
   if(!MemPlugin::skipPlugin && (MemPlugin::dllInstance == NULL))
      MemPlugin::init();
      
   if(!MemPlugin::skipPlugin)
      return MemPlugin::Calloc(num, size);
      
	void* mem;
	if ((mem = Malloc.alloc(num * size, false)) != 0)
		memset(mem,0,num * size);
	return mem;
}
