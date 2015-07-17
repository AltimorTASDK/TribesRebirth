//================================================================
//	
// Initial Author: Rick Overman 
//	
// Description 
//	
// $Workfile:   ddrawerr.h  $
// $Revision:   1.0  $
// $Author  $
// $Modtime $
//
//================================================================

#ifndef _DDRAWERR_H_
#define _DDRAWERR_H_

#include <ddraw.h>
#include <types.h>

#ifdef DEBUG
const char* DDRAW_ERROR(HRESULT error);
#else
#define DDRAW_ERROR(x) 0
#endif


#endif //_DDRAWERR_H_
