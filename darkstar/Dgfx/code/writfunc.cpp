//================================================================
//   
// $Workfile:   writfunc.cpp  $
// $Version$
// $Revision:   1.1  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include <windows.h>


extern "C" void __cdecl
markFunctionWritable(void *startOfFunc, unsigned long fLength);


extern "C" void __cdecl
markFunctionWritable(void *startOfFunc, unsigned long fLength)
{
   DWORD dummy;
   VirtualProtect(startOfFunc, fLength, PAGE_EXECUTE_READWRITE, &dummy);
}
