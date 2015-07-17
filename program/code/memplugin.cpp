#include <windows.h>
#include <alloc.h>
#include <mem.h>
#include "base.h"
#include "memory.h"

//-----------------------------------------------------------------
// Override default stdlib memory management by using a memory dll
//    - debug version adds header/footer sentinals to the allocated
//      memory for tracking double deletions over/underunds and such
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
		memset(mem,0,Malloc.getPtrSize(mem));
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
		memset(mem,0,Malloc.getPtrSize(mem));
	return mem;
}
