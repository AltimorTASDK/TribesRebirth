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

#include "memory.h"

int MallocMin = 1024*1024*64;
int CommitMin = 1024*1024*2;
int ResizeMin = 1024*1024*2;

//---------------------------------------------------------------------------
// Default memory object used to override the standard
// library new, delete, malloc,calloc,realloc & free.

//int (far _cdecl *_malloc_handler)(void);


//---------------------------------------------------------------------------

Memory::Memory()
{
	// Skip init if this is the defualt memory object.
	// The Malloc object gets initialized before the
	// constructor is called by the startup code.
	//
	if (this == &Malloc)
		return;

	base = 0;
	head = 0;
	baseSize = memFree = 0;
	heapCheckMask = 0;
	heapCheckSize = 0;
}

Memory::~Memory()
{
// Don't delete critical section.
//	if (base)
//		DeleteCriticalSection(&lock);
}

//---------------------------------------------------------------------------
// Memory should be long word aligned.

void Memory::init(int size, int commit,void* ptr)
{
#ifdef DEBUG
	memset(ptr,0xCD,commit);
#endif
	base = ptr;
	baseSize = size;
   commitSize = commit;
	head = (MemoryHeader*) ptr;
	head->next = 0;
	head->size = size & MemoryBlockMask;
	memFree = head->size;
	*((int*) memPtr(head)) = MemoryFreeID;
	InitializeCriticalSection(&lock);
}


//---------------------------------------------------------------------------
// True if the header for the memory belongs to the memory
// pool and is marked as allocted.

bool Memory::isValidPtr(void* ptr)
{
	return int(ptr) >= int(base) &&
		int(ptr) < int(base) + baseSize &&
		headerPtr(ptr)->next == MemoryAllocID;
}


//---------------------------------------------------------------------------
// True if the header belongs to the memory pool and is
// marked as either allocated or free.

bool Memory::isValidHeader(const MemoryHeader* ptr)
{
	return int(ptr) >= int(base) && int(ptr) <= int(base) +
		baseSize - MemoryBlockSize - int(sizeof(MemoryHeader)) &&
		(*((int*) memPtr((MemoryHeader*)ptr)) == MemoryFreeID ||
		ptr->next == MemoryAllocID);
}


//---------------------------------------------------------------------------
// True if the header for the memory belongs to the memory
// pool, is marked as allocated, and the following header is
// also valid.

bool Memory::isValidBlock(void* ptr)
{
	if (!isValidPtr(ptr))
		return FALSE;

	MemoryHeader* header = headerPtr(ptr);
	MemoryHeader* next = nextIncHeader(header);
	if (next == (MemoryHeader*) ((char*)base) + baseSize)
		// End of memory pool
		return TRUE;

	return isValidHeader(header);
}


//---------------------------------------------------------------------------
// Return the largest allocatable block

int Memory::getLargestBlockSize()
{
	MemoryHeader* block = head;
	MemoryHeader* ptr;
	for (ptr = block->next; ptr; ptr = ptr->next)
		if (ptr->size > block->size)
			block = ptr;
	return block->size - sizeof(MemoryHeader);
}


//---------------------------------------------------------------------------

void* Memory::alloc(int size, bool array)
{
	if (!base) {
		void *mem = VirtualAlloc(NULL, MallocMin, MEM_RESERVE, PAGE_READWRITE);
      VirtualAlloc(mem, CommitMin, MEM_COMMIT, PAGE_READWRITE);

		init(MallocMin,CommitMin,mem);
	}
#ifdef DEBUG
	if (heapCheckMask & Alloc)
		heapCheck();
#endif
	if (!size)
		return 0;
	EnterCriticalSection(&lock);

	// Expand size to include header and round up to
	// nearest block size boundry.
	//
	size = (size + sizeof(MemoryHeader) + MemoryBlockSize - 1) &
		MemoryBlockMask;

	// Search for the first free block in the chain
	// that will contain the requested memory.
	//
	MemoryHeader *prev = head,*ptr = head;
	for (; ptr; prev = ptr,ptr = ptr->next) {
		AssertFatal(isValidHeader(ptr),
			"Memory::alloc: Free list corrupted");
		if (ptr->size == size) {
			if (ptr == head)
				head = ptr->next;
			else
				prev->next = ptr->next;

			// Set allocated ID and clear the free ID
			//
			ptr->next = MemoryAllocID;
         ptr->arrayAlloc = (UInt32)array;
			int *mptr = (int*) memPtr(ptr);
			*mptr = 0;
			memFree -= size;
			LeaveCriticalSection(&lock);
			return mptr;
		}
		else
			// Make sure that after allocation the free 
			// block has the minimum of MemoryBlockSize worth
			// of space (including header).
			//
			if (ptr->size - int(sizeof(MemoryHeader) + MemoryBlockSize) > size) {
				// Take memory from the beginning of the
				// block.
				//
				
            // check for size overrun
            int totalSize = DWORD(ptr) - DWORD(base) + size + int(sizeof(MemoryHeader) + MemoryBlockSize) + 256;
            if(totalSize > commitSize)
            {
               while(commitSize <= totalSize)
                  commitSize += ResizeMin;

               VirtualAlloc(base, commitSize, MEM_COMMIT, PAGE_READWRITE);
            }
				
				MemoryHeader* npt = (MemoryHeader*)(((char*)ptr) + size);
				npt->size = ptr->size - size;
				npt->next = ptr->next;
				*(int*)memPtr(npt) = MemoryFreeID;
				if (ptr == head)
					head = npt;
				else
					prev->next = npt;

				ptr->size = size;
				ptr->next = MemoryAllocID;
            ptr->arrayAlloc = (UInt32)array;
				*(int*) memPtr(ptr) = 0;
				memFree -= size;
				LeaveCriticalSection(&lock);
				return memPtr(ptr);
			}
	}

	AssertWarn(0,"Memory::alloc: Allocation failed");
	LeaveCriticalSection(&lock);
	return 0;
}


//---------------------------------------------------------------------------

void Memory::free(void *mem, bool array)
{
#ifdef DEBUG
	if (heapCheckMask & Free)
		heapCheck();
#endif
	if (!mem)
		return;

	AssertFatal(isValidPtr(mem),"Memory::free: Bad pointer");
	AssertFatal(isValidBlock(mem),"Memory::free: Block overrun");
	MemoryHeader* block = headerPtr(mem);
   AssertFatal(block->arrayAlloc == (UInt32)array, "Memory::free: inconsistent [] usage");

	memFree += block->size;

	*((int*)mem) = MemoryFreeID;
	EnterCriticalSection(&lock);

	// If list is empty, then simple
	//
	if (!head) {
		block->next = 0;
		head = block;
		return;
	}

	// Find insertion point for the block and link
	// it in to the free list.
	//
	MemoryHeader *prev = head,*ptr = head;
	for (; ptr; prev = ptr,ptr = ptr->next) {
		AssertFatal(isValidHeader(ptr),
			"Memory::free: Free list corrupted");
		if (((int) block) < ((int) ptr)) {
			if (ptr == head) {
				block->next = head;
				head = block;
			}
			else {
				block->next = ptr;
				prev->next = block;
			}
			break;
		}
	}

	if (!ptr) {
		prev->next = ptr = block;
		block->next = 0;
	}

	// Attempt to merge the current block with the
	// previous block.
	//
	if (nextIncHeader(prev) == block) {
		prev->next = block->next;
		prev->size += block->size;
		*((int*) memPtr(block)) = 0;
		block = prev;
	}

	// Attempt to merge the current block with the
	// next block.
	//
	if (nextIncHeader(block) == ptr) {
		block->next = ptr->next;
		block->size += ptr->size;
		*((int*) memPtr(ptr)) = 0;
	}

#ifdef DEBUG
	mem = memPtr(block);
	//memset(mem,0xCD,block->size - sizeof(MemoryHeader));
	*((int*)mem) = MemoryFreeID;
#endif
	LeaveCriticalSection(&lock);
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
      return malloc(size);
	EnterCriticalSection(&lock);

	AssertFatal(isValidPtr(mem),"Memory::realloc:: Bad pointer");
	AssertFatal(isValidBlock(mem),"Memory::realloc: Block overrun");
	MemoryHeader* block = headerPtr(mem);
   AssertFatal(block->arrayAlloc == 0, "Memory:realloc: bad things happen when you move new[]'ed ptrs");

	void* np = mem;

	// Get next block and make sure it's not the end of
	// the memory pool
	//
	MemoryHeader* nptr = nextIncHeader(block);
	if (!isValidHeader(nptr))
		nptr = 0;

	// Expand size to include header and round up to
	// nearest block size boundry.
	//
	int bsize = (size + sizeof(MemoryHeader) + MemoryBlockSize - 1) &
		MemoryBlockMask;

	// Determin wether we can expand/shrink block in place
	// or if we have to move it.
	//
	if (bsize < block->size) {
		int shrink = block->size - bsize;

		if (nptr && !isAllocatedHeader(nptr)) {
			//
			// Add memory to following block
			//
			MemoryHeader* nnptr = (MemoryHeader*)(int(nptr) - shrink);
			nnptr->size = nptr->size + shrink;
			nnptr->next = nptr->next;
         nnptr->arrayAlloc = 0;
			*((int*) memPtr(nnptr)) = MemoryFreeID;
			*((int*) memPtr(nptr)) = 0;
			if (nptr == head)
				head = nnptr;
			else {
				MemoryHeader *prev = prevLinkedHeader(nptr);
				prev->next = nnptr;
			}
			block->size -= shrink;
			memFree += shrink;
		}
		else
			if (shrink > sizeof(MemoryHeader) + MemoryBlockSize) {
				//
				// Convert the freed memory into a free block
				//
				block->size -= shrink;
				MemoryHeader* fptr = nextIncHeader(block);
				fptr->size = shrink;
				fptr->next = MemoryAllocID;
            fptr->arrayAlloc = 0;
				free(memPtr(fptr), false);
			}
			else
				// Have to move it.
				//
				if ((np = alloc(size, false)) != 0) {
					memcpy(np,mem,size);
					free(mem, false);
				}
	}
	else
		if (bsize > block->size) {
			int expand = bsize - block->size;

			if (nptr && !isAllocatedHeader(nptr) && nptr->size -
				int(sizeof(MemoryHeader) + MemoryBlockSize) >= expand) {

            int totalSize = DWORD(mem) - DWORD(base) + bsize + int(sizeof(MemoryHeader) + MemoryBlockSize) + 256;
            if(totalSize > commitSize)
            {
               while(commitSize <= totalSize)
                  commitSize += ResizeMin;

               VirtualAlloc(base, commitSize, MEM_COMMIT, PAGE_READWRITE);
            }

				//
				// Take memory from following free block
				//
				MemoryHeader* nnptr = (MemoryHeader*)(int(nptr) + expand);
				nnptr->size = nptr->size - expand;
				nnptr->next = nptr->next;
				*((int*) memPtr(nnptr)) = MemoryFreeID;
				if (expand > sizeof(MemoryHeader))
					*((int*) memPtr(nptr)) = 0;
				if (nptr == head)
					head = nnptr;
				else {
					MemoryHeader *prev = prevLinkedHeader(nptr);
					prev->next = nnptr;
				}
				block->size += expand;
				memFree -= expand;
			}
			else
				// Have to move it.
				//
				if ((np = alloc(size, false)) != 0) {
					memcpy(np,mem,getPtrSize(mem));
					free(mem, false);
				}
		}

	LeaveCriticalSection(&lock);
	return np;
}


//----------------------------------------------------------------------------

void Memory::heapCheck()
{
	EnterCriticalSection(&lock);
	MemoryIterator mem;
	for (; mem.current(); mem.next()) {
		MemoryHeader* hdr = mem.current();
		AssertISV(isValidHeader(hdr),"Memory::heapCheck: Invalid memory header");
		if (!isAllocatedHeader(hdr) && hdr->size < heapCheckSize) {
			UInt8* ptr = ((UInt8*) memPtr(hdr)) + sizeof(int);
			UInt8* end = ptr + (hdr->size - sizeof(MemoryHeader) - sizeof(int));
			while (ptr != end) {
				AssertISV(*ptr++ == 0xCD,"Memory::heapCheck: Invalid free block contents");
			}
		}
	}
	LeaveCriticalSection(&lock);
}


//---------------------------------------------------------------------------
// Display memory map on mono screen.
// The mono display is drawn from 0,0 to the given width,height

void Memory::monoDump(int px,int py,int width,int height)
{
#ifdef MONO_OUT
	MemoryIterator mem;
	int cSize = coreSize() / (width * height);
	int bSize = mem.current()->size;
	int per;

	for (int i = 0; i < height; i++)
		for (int b = 0; b < width; ++b) {
			if ((bSize -= cSize) < 0) {
				int alloc = isAllocatedHeader(mem.current())? -bSize: 0;
				do {
					bSize += mem.next()->size;
					if (isAllocatedHeader(mem.current()))
							alloc += mem.current()->size;
				}
				while (bSize < 0);
				if (isAllocatedHeader(mem.current()))
					alloc -= bSize;
				per = (alloc * 100) / cSize;
			}
			else
				per = isAllocatedHeader(mem.current())? 100: 0;

			// Display symbol according to percentage of cSize
			// allocated.
			//
			mono_printf(px + b,py + i,"%c",
				(per == 0)?  '\007':
				(per < 50)?  '\261':
				(per < 100)? '\262':
				'\333');
		}
#else
	px,py,width,height;
#endif
}


//---------------------------------------------------------------------------

MemoryHeader* Memory::prevLinkedHeader(const MemoryHeader* ptr)
{
	AssertFatal(!isAllocatedHeader(ptr),
		"Memory::prevLinkedHeader: Block is not free");

	if (ptr == head)
		return 0;
	MemoryHeader *prev = head;
	for (; prev; prev = prev->next)
		if (prev->next == ptr)
			break;
	AssertFatal(prev,"Memory::prevLinkedHeader: Free list corrupted");
	return prev;
}


////---------------------------------------------------------------------------
//// Expand the base memory block by the given amount.
//// Allocates in increments of "increment"
//// Returns the amount actually expanded.
////
//int Memory::expand(int amount,int increment)
//{
//	MemoryHeader* ptr = (MemoryHeader*) (increment);
//	AssertISV(((char*)ptr) == ((char*)base) + baseSize,
//		"Memory::expand: Hole found in memory expansion");
//	EnterCriticalSection(&lock);
//	if (ptr) {
//		int count = 1;
//		int maxCount = amount / increment;
//		while ((count < maxCount) &&
//			(GlobalAlloc(GMEM_FIXED,increment) != (void*)-1))
//			count++;
//
//		// Fix up new memory so we can free it
//		//
//		int memSize = count * increment;
//		ptr->next = MemoryAllocID;
//		ptr->size = memSize;
//		baseSize += memSize;
//		free(memPtr(ptr));
//		LeaveCriticalSection(&lock);
//		return memSize;
//	}
//	LeaveCriticalSection(&lock);
//	return 0;
//}


//---------------------------------------------------------------------------
// Memory Iterator
//---------------------------------------------------------------------------

MemoryIterator::MemoryIterator(Memory& mem)
{
	memory = &mem;
	header = (MemoryHeader*) memory->base;
}

MemoryHeader* MemoryIterator::next()
{
	if (!header)
		return 0;

	header = memory->nextIncHeader(header);
	if (int(header) >= int(((char*)memory->base) + memory->baseSize))
		header = 0;

	return header;
}
