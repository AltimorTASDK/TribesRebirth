//================================================================
//   
// $Workfile:   d_funcs.h  $
// $Version$
// $Revision:   1.0  $
//   
// DESCRIPTION:
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _D_FUNCS_H_
#define _D_FUNCS_H_


#include <types.h>
#include "d_defs.h"
#include "m_point.h"
#include "m_rect.h"
#include "g_types.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//================================================================
//
// Rendering Attributes:
//    (default: Outline)
//
//    f - filled
//    g - gouraud shaded
//    s - shade-mapped
//    t - textured (implies filled)
//
// ---------------------------------------------------------------
// Attributes controlled by Polygon or Bitmap switches:
//    p - perspective correction for textures
//    x - Tiling for textures
//    0 - Transparency in textures
//
// Attributes controlled by changing the contents of the table:
//    z - z-buffering
//    zc - z-buffer as a clipping process
//    ec - edge-table clipping
//    (ezc - edge plus z clipping)
//    (default is rectangle clipping)
//    ... any direct-to-surface rendering (vs. Rasterization pipeline)
//    ... any customized or user-created rendering
//
//================================================================


class  GFXSurface;
class  GFXFont;
class  GFXBitmap;

struct GFXLightMap;

struct FunctionTable
{
	void (*fnClear)(GFXSurface *io_pSurface, DWORD in_color);

	void (*fnDrawPoint)(GFXSurface *io_pSurface, const Point2I *in_pt, float in_w, DWORD in_color);
	void (*fnDrawLine2d)(GFXSurface *io_pSurface, const Point2I *in_st, const Point2I *in_en, DWORD in_color );

	void (*fnDrawRect2d) (GFXSurface *io_pSurface, const RectI *in_rect, DWORD in_color );
	void (*fnDrawRect_f) (GFXSurface *io_pSurface, const RectI *in_rect, float in_w, DWORD in_color );

	void (*fnDrawBitmap2d_f)(GFXSurface *io_pSurface, const GFXBitmap *in_pBM, 
	               const Point2I *in_at, GFXFlipFlag in_flip);
	void (*fnDrawBitmap2d_rf)(GFXSurface *io_pSurface, const GFXBitmap *in_pBM, 
	               const RectI *in_subRegion, const Point2I *in_at, GFXFlipFlag in_flip);
	void (*fnDrawBitmap2d_sf)(GFXSurface *io_pSurface, const GFXBitmap *in_pBM, 
	               const Point2I *in_at, const Point2I *in_stretch, GFXFlipFlag in_flip);
	void (*fnDrawBitmap2d_af)(GFXSurface *io_pSurface, const GFXBitmap *in_pBM, const Point2I *in_at, 
	               const Point2I *in_stretch, Int32 rotate);

	Int32 (*fnDrawText_p)(GFXSurface *io_pSurface, GFXFont *in_font, const Point2I *in_atPt, const void *in_string );
	Int32 (*fnDrawText_r)(GFXSurface *io_pSurface, GFXFont *in_font, const RectI *in_atRect, const void *in_string );

	void (*fnDraw3DBegin)(GFXSurface *io_pSurface);
	void (*fnDraw3DEnd)(GFXSurface *io_pSurface);
	
	// New function interface:

	void (*fnAddVertexV)(GFXSurface *io_pSurface, const Point3F *, DWORD);
	void (*fnAddVertexVT)(GFXSurface *io_pSurface, const Point3F *, const Point2F *, DWORD);
	void (*fnAddVertexVC)(GFXSurface *io_pSurface, const Point3F *, const GFXColorInfoF *, DWORD);
	void (*fnAddVertexVTC)(GFXSurface *io_pSurface, const Point3F *, const Point2F *, const GFXColorInfoF *, DWORD);
	void (*fnEmitPoly)(GFXSurface *io_pSurface);

	void (*fnSetShadeSource)(GFXSurface *io_pSurface, GFXShadeSource ss);
	void (*fnSetHazeSource)(GFXSurface *io_pSurface, GFXHazeSource hs);
	void (*fnSetAlphaSource)(GFXSurface *io_pSurface, GFXAlphaSource as);
	void (*fnSetFillColorCF)(GFXSurface *io_pSurface, const ColorF *c);
	void (*fnSetFillColorI)(GFXSurface *io_pSurface, Int32 index, DWORD paletteIndex);
	void (*fnSetHazeColorCF)(GFXSurface *io_pSurface, const ColorF *c);
	void (*fnSetHazeColorI)(GFXSurface *io_pSurface, Int32 index);
	void (*fnSetConstantShadeCF)(GFXSurface *io_pSurface, const ColorF *c);
	void (*fnSetConstantShadeF)(GFXSurface *io_pSurface, float c);
	void (*fnSetConstantHaze)(GFXSurface *io_pSurface, float h);
	void (*fnSetConstantAlpha)(GFXSurface *io_pSurface, float h);
	void (*fnSetTransparency)(GFXSurface *io_pSurface, Bool transFlag);
	void (*fnSetTextureMap)(GFXSurface *io_pSurface, const GFXBitmap *pTextureMap);
	void (*fnSetFillMode)(GFXSurface *io_pSurface, GFXFillMode fm);
	void (*fnSetTexturePerspective)(GFXSurface *io_pSurface, Bool perspTex);

	void (*fnRegisterTexture)(GFXSurface *, GFXTextureHandle,
		int sizeX, int sizeY, int offsetX, int offsetY, int lightScale,
		GFXLightMap *map, const GFXBitmap *tex, int mipLevel);
   void (*fnRegisterTextureCB)(GFXSurface *, GFXTextureHandle,
                                       GFXCacheCallback cb, int csizeX, 
		int csizeY, int lightScale, GFXLightMap *map);
	Bool (*fnSetTextureHandle)(GFXSurface *, GFXTextureHandle);
	GFXLightMap * (*fnAllocateLightMap)(GFXSurface *, int pixelSize);
	GFXLightMap * (*fnHandleGetLightMap)(GFXSurface *);
	void (*fnHandleSetLightMap)(GFXSurface *, int lightScale, GFXLightMap *map);
	void (*fnHandleSetTextureMap)(GFXSurface *, const GFXBitmap *);
	void (*fnHandleSetMipLevel)(GFXSurface *, int mipLevel);
	void (*fnHandleSetTextureSize)(GFXSurface *, int newSize);
	void (*fnFlushTextureCache)(GFXSurface *);

	void (*fnSetZTest)(GFXSurface *, int enable);
	void (*fnClearZBuffer)(GFXSurface *);
	void (*fnSetTextureWrap)(GFXSurface *, Bool wrapEnable);
   
   void (*fnFlushTexture)(GFXSurface*, const GFXBitmap* in_pTexture, const bool in_reload);
	void (*fnSetZMode)(GFXSurface *, bool wBuffer);

   void (*fnSetClipPlanes)(GFXSurface*, const float, const float);

   GFXHazeSource (*fnGetHazeSource)(GFXSurface*);
   float         (*fnGetConstantHaze)(GFXSurface*);
};

struct GFXPolyRenderState
{
	GFXShadeSource shadeSource;
	GFXHazeSource hazeSource;
	GFXAlphaSource alphaSource;
	GFXFillMode fillMode;
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_D_FUNCS_H_
