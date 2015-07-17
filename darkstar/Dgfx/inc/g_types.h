//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _G_TYPES_H_
#define _G_TYPES_H_

//Includes
#include "types.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GFXBitmap;

struct GFXTextureHandle
{
	DWORD key[2];
};

typedef void (*GFXCacheCallback)(GFXTextureHandle tex, GFXBitmap *bmp, int numMipLevels);

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

// Lightmaps must be byte aligned...
//
#ifdef __BORLANDC__
#pragma option -a1
#endif
#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct GFXLightMap
{
	enum Constants {
		// Size of header only:
		// sizeof(LightMap) - sizeof(UInt16)
		HeaderSize = 6,
	};
	struct Point {
		UInt8 x,y;
	};
	Point size;					// size of the lmap.
	Point offset;				// Offset in texture coor.
	BYTE userdata;				// 8 bits of user accessable data.
	BYTE index;					// used by allocater.
	UInt16 data;				// Start of data
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif //_G_TYPES_H_
