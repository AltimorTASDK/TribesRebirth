/**********************************************************************
 *<
	FILE: helpers.h

	DESCRIPTION: Helper object header file

	CREATED BY: Tom Hudson

	HISTORY: Created 31 January 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#ifndef __HELPERS__H
#define __HELPERS__H
#define DLLEXPORT __declspec(dllexport)

#include "Max.h"
#include "resource.h"

TCHAR *GetString(int id);

extern ClassDesc* GetSequenceDesc();
extern ClassDesc* GetTransitionDesc();

#endif // __HELPERS__H
