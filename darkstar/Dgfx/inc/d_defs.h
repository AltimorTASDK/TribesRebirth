//================================================================
//   
// $Workfile:   d_defs.h  $
// $Version$
// $Revision:   1.6  $
//   
// DESCRIPTION:
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _D_DEFS_H_
#define _D_DEFS_H_


#include <types.h>

typedef DWORD MGRType;
#define MGR_UPDATE   0
#define MGR_DIRECT   1

// DMMNOTE: The FILL_LIGHTMAP and TEXTUREP1 modes are ONLY valid for the OpenGL surface.
//
enum GFXShadeSource { GFX_SHADE_NONE = 1, GFX_SHADE_CONSTANT = 2, GFX_SHADE_VERTEX = 4};
enum GFXHazeSource  { GFX_HAZE_NONE = 8, GFX_HAZE_CONSTANT = 16, GFX_HAZE_VERTEX = 32 };
enum GFXAlphaSource { GFX_ALPHA_NONE, GFX_ALPHA_CONSTANT, GFX_ALPHA_VERTEX, GFX_ALPHA_TEXTURE, GFX_ALPHA_FILL, GFX_ALPHA_ADD, GFX_ALPHA_SUB };
enum GFXFillMode    { GFX_FILL_CONSTANT, GFX_FILL_TEXTURE, GFX_FILL_TWOPASS, GFX_FILL_LIGHTMAP, GFX_FILL_TEXTUREP1 };
enum GFXZBufferMode { GFX_NO_ZTEST = 0, GFX_ZTEST_AND_WRITE=1, GFX_ZTEST=2, GFX_ZWRITE = 3, GFX_ZALWAYSBEHIND };
enum GFXLockMode    { GFX_NORMALLOCK = 0, GFX_DOUBLELOCK = 1 };

typedef DWORD GFXMode;
const GFXMode MAX_GFX_MODES = 32;

typedef DWORD GFXFlipFlag;
const GFXFlipFlag GFX_FLIP_NONE = 0;    // do not flip
const GFXFlipFlag GFX_FLIP_X    = 1<<0; // Flip left to right
const GFXFlipFlag GFX_FLIP_Y    = 1<<1; // Flip top to bottom
const GFXFlipFlag GFX_FLIP_XY   = (GFX_FLIP_X|GFX_FLIP_Y);

//Device Manager Flags
typedef DWORD GFX_DMFlag;
const GFX_DMFlag GFX_DMF_RCLIP      = 1<<0;  //enable rect clipping if set
const GFX_DMFlag GFX_DMF_ECLIP      = 1<<1;  //enable edge clipping if set
const GFX_DMFlag GFX_DMF_CLIP       = GFX_DMF_RCLIP | GFX_DMF_ECLIP;  //enable edge clipping if set
const GFX_DMFlag GFX_DMF_WAITVBLANK = 1<<2;	 //wait for the vertical blank (if supported)
const GFX_DMFlag GFX_DMF_WRITEONLY  = 1<<3;  //this surface is write only
const GFX_DMFlag GFX_DMF_ZCLIP      = 1<<4;  //uses z-buffering
const GFX_DMFlag GFX_DMF_USES_SOFTWARE_CURSOR = 1<<5;
const GFX_DMFlag GFX_DMF_BACKPAGE_CARDMEM = 1<<6;
enum GFX_SURFACE_TYPE
{
	SURFACE_DIRECT_DRAW,	
	SURFACE_DIB_SECTION,
	SURFACE_MEMORY,
	SURFACE_SUBSURFACE,	
	SURFACE_DIRECT_3D,
	SURFACE_3DFX,	
	SURFACE_RREDLINE,	
};


//bits 7-0  : are pixel color depth in pixels
//bits 31-8 : mode number
#define GFX_NxNx8         0x008
#define GFX_320x200x8     0x108
#define GFX_640x480x8     0x208
#define GFX_800x600x8     0x308
#define GFX_1024x768x8    0x408
#define GFX_1280x1024x8   0x508
#define GFX_NxNx16        0x010
#define GFX_320x200x16    0x110
#define GFX_640x480x16    0x210
#define GFX_800x600x16    0x310                 
#define GFX_1024x768x16   0x410
#define GFX_1280x1024x16  0x510
#define GFX_NxNx24        0x018
#define GFX_320x200x24    0x118
#define GFX_640x480x24    0x218
#define GFX_800x600x24    0x318 
#define GFX_1024x768x24   0x418
#define GFX_1280x1024x24  0x518



#endif //_D_DEFS_H_


