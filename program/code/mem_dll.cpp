#include <windows.h>

// dll entry point - always returns success
bool WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID )
{
   return(true);
}

//---------------------------------------------------------------------------
// Override default stdlib memory management
//---------------------------------------------------------------------------
extern "C" {
   __declspec(dllexport) void * __cdecl MS_Malloc(size_t size)
   {
      return malloc(size);
   }

   __declspec(dllexport) void __cdecl MS_Free(void * mem)
   {
	   free(mem);
   }
   __declspec(dllexport) void * __cdecl MS_Realloc(void * mem, size_t size)
   {
	   return realloc(mem,size);
   }
   __declspec(dllexport) void * __cdecl MS_Calloc(size_t num, size_t size)
   {
      return calloc(num, size);
   }
}
