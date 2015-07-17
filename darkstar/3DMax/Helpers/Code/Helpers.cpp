/**********************************************************************
 *<
	FILE: helpers.cpp

	DESCRIPTION:   DLL implementation of primitives

	CREATED BY: Dan Silva

	HISTORY: created 12 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "helpers.h"

HINSTANCE hInstance    = NULL;
BOOL      controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	switch(fdwReason) 
    {
		case DLL_PROCESS_ATTACH:
	        if (controlsInit == FALSE && hinstDLL) 
            {
    	        hInstance    = hinstDLL;
		        controlsInit = TRUE;
	        
                InitCommonControls();
                InitCustomControls(hInstance);
            }
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			break;
    }
	
    return(TRUE);
}

LPCTSTR DLLEXPORT LibDescription() 
{ 
    return _T("Sequence helper objects"); 
}

INT DLLEXPORT LibNumberClasses() 
{
    return (2);
}

ClassDesc * DLLEXPORT LibClassDesc(int iIndex) 
{
	switch(iIndex)
    {
        case 0 : 
            return (GetSequenceDesc());
        case 1 : 
            return (GetTransitionDesc());
        default: 
            return (0);
    }
}

// Return version so can detect obsolete DLLs
ULONG DLLEXPORT LibVersion() 
{ 
    return (VERSION_3DSMAX); 
}

LPTSTR GetString(int iStringID)
{
	static TCHAR szBuf[BUFSIZ];

	if (hInstance)
    {
		return (LoadString(hInstance, iStringID, szBuf, BUFSIZ) ? szBuf : NULL);
    }

	return (NULL);
}
