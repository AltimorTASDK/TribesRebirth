/* This file can be compiled as C or C++             */
/* This library is meant to be thread safe           */
/* Any global variables should be stored as Thread   */
/* local variables or properly locked by Mutexs      */

#include <dsound.h>
#include "ia3d.h"
 
/* Use GUID_ instead of IID_ for both C or C++ */

#if !defined(__cplusplus) || defined(CINTERFACE)
#define GUID_IA3d			&IID_IA3d
#define GUID_IA3d2			&IID_IA3d2
#define GUID_IDirectSound	&IID_IDirectSound
#define GUID_CLSID_A3d		&CLSID_A3d
#else
#define GUID_IA3d			IID_IA3d
#define GUID_IA3d2			IID_IA3d2
#define GUID_IDirectSound	IID_IDirectSound
#define GUID_CLSID_A3d		CLSID_A3d
#endif

/*****************************************************************************
*
*  A3dInitialize()	You don't need to call this.  All functions in this module
*                   will call A3dInitialize before it does anything.  You can 
*                   call this to make your code look balanced with 
*					A3dUninitalized.
*
*  Return:
*
*    >= 0            COM success
*
*    S_FALSE        already initialized
*    S_OK			first time initialized
*
*    < 0            COM error
*
******************************************************************************/

HRESULT WINAPI A3dInitialize(void)
{
 HRESULT hr;

 hr = CoInitialize(NULL);

 return hr;
}

/*****************************************************************************
*
*  A3dUninitialize()	Uninitializes A3d Library.  Call at the exit 
*                       of your program if you use any function in this module.
*						Once COM is uninitialized, no further COM objects can
*						be used unil COM is initialized again.
*
******************************************************************************/

void WINAPI A3dUninitialize(void)
{

   CoUninitialize();

}

/*****************************************************************************
*
*  A3dCreate()		Creates a DirectSound interface.  The parameters for this
*                   are the same as DirectSoundCreate().  Basically uses COM
*					function CoCreateInstance() to initialize the IA3d2 inteface 
*                   and properly register it.  Then from the IA3d2 interface, get and
*                   initalize the A3d version of the DirectSound interface.
*                   If there are problems, this function returns the
*					DirectSound interface from Microsoft.
*
*  Return:
*
*   >= 0            COM success
*
*   A3D_OK          DirectSound is A3d DirectSound object
*
*   A3D_OK_OLD_DLL  Gets A3d Direct Sound but A3d.DLL is older than expected.  Be aware
*                    that sound results may be different than expected.  You should
*					 inform the user of this and ask him to download the latest 
*					 a3d.dll from www.aureal.com.
*
*   DS_OK           Microsoft DirectSound object is returned.  A3d Not present or
*					 failed to initialize.
*
*   < 0             COM error
*
* 	E_INVALIDARG	DirectSound pointer pointer is NULL.
*
******************************************************************************/

HRESULT WINAPI A3dCreate(LPGUID  lpGUID,	        /* in,  Prefered Driver Guid, NULL ok */
						  void **ppds,				/* out, Direct Sound pointer */
						  IUnknown FAR *pUnkOuter)	/* in,  Outer COM object, for Aggregate only NULL oK*/
{
	LPIA3D2       pIA3d2 = NULL;
    HRESULT       hr;
	LPDIRECTSOUND *ppDS = (LPDIRECTSOUND *)ppds;

	if (!ppDS)
	{
	 return E_INVALIDARG;
	}

	/* Defensive setting of the return DirectSound pointer */

	*ppDS = NULL;

    /* Just in case has not done A3dInitialzed has not been called. */

    A3dInitialize();

	/* Try to Get the IA3d2 Interface */

	if (SUCCEEDED(hr = CoCreateInstance(GUID_CLSID_A3d,			/* Class Id registerd to a3d.dll */
										 pUnkOuter,				/* Aggregate COM object */
										 CLSCTX_INPROC_SERVER,	/* Code runs on the same Machine */
										 GUID_IDirectSound,		/* Interface that we want */
										 (void **)ppDS)))		/* Pointer to the requested interface */
    {

     /* Assume old DLL until proven wrong */

     hr = A3D_OK_OLD_DLL;

     /* Try to get IA3D2 interface from a3d.dll */

	 if (SUCCEEDED(IDirectSound_QueryInterface(*ppDS, GUID_IA3d2, (void **)&pIA3d2)))
	 {
       A3DCAPS_SOFTWARE swCaps;
       
	   /* Get Dll Software CAP to get DLL version number */

       ZeroMemory(&swCaps,sizeof(swCaps));

       swCaps.dwSize = sizeof(swCaps);
       IA3d2_GetSoftwareCaps(pIA3d2,&swCaps);

       /* Compare version from a3d.dll to header version */
	   /* only return A3D_OK if dll version >= to header version */

	   if (swCaps.dwVersion >= A3D_CURRENT_VERSION)
	   {
		hr = A3D_OK;
	   }

	   /* Register our version for backwards compatibility with */
	   /* newer A3d.dll                                         */

       IA3d2_RegisterVersion(pIA3d2,A3D_CURRENT_VERSION);

     } 

	 /* We have DirectSound from A3d, so just initialize it */

	 if (*ppDS)
	 {
      if (SUCCEEDED(IDirectSound_Initialize(*ppDS, lpGUID )))
	  {
	   /* We have an A3d version of DirectSound! */
	   return hr;
	  }     		
     }

	}

    /* No IA3d interface or failed to initialize A3d DirectSound, 
	/* so it probably didn't come for Aureal */
    /* Just go with the MS DirectSound */

    return hr = DirectSoundCreate(lpGUID,ppDS,pUnkOuter);
}


