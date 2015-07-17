//---------------------------------------------------------------------------

// $Workfile:   memory.h  $
// $Version$
// $Revission$
// $Date:   01 Nov 1994 14:28:26  $
// $Log:   N:\arena\code\base\vcs\memory.h_v  $
//	

//---------------------------------------------------------------------------

#include <windows.h>
#include <alloc.h>
#include <mem.h>

#include "memory.h"

Memory Malloc;

//---------------------------------------------------------------------------
// Override default stdlib memory management
//---------------------------------------------------------------------------

void* _cdecl operator new(size_t size)
{
	return Malloc.alloc(size, false);
}

void* _cdecl operator new[](size_t size)
{
	return Malloc.alloc(size, true);
}

void* operator new(size_t size,Memory& mem)
{
	return mem.alloc(size, false);
}

void _cdecl operator delete(void* mem)
{
	Malloc.free(mem, false);
}

void _cdecl operator delete[](void* mem)
{
	Malloc.free(mem, true);
}



void * _cdecl malloc(size_t size)
{
	return Malloc.alloc(size, false);
}

void _cdecl free(void * mem)
{
	Malloc.free(mem, false);
}

void * _cdecl realloc(void * mem, size_t size)
{
	return Malloc.realloc(mem,size);
}

void * _cdecl calloc(size_t num, size_t size)
{
	void* mem;
	if ((mem = Malloc.alloc(num * size, false)) != 0)
		memset(mem,0,Malloc.getPtrSize(mem));
	return mem;
}



void * _cdecl std::malloc(size_t size)
{
	return Malloc.alloc(size, false);
}

void _cdecl std::free(void * mem)
{
	Malloc.free(mem, false);
}

void * _cdecl std::realloc(void * mem, size_t size)
{
	return Malloc.realloc(mem,size);
}

void * _cdecl std::calloc(size_t num, size_t size)
{
	void* mem;
	if ((mem = Malloc.alloc(num * size, false)) != 0)
		memset(mem,0,Malloc.getPtrSize(mem));
	return mem;
}
