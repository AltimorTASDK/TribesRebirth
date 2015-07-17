//----------------------------------------------------------------------------
//
//  $Workfile:   fn_all.h  $
//  $Version$
//  $Revision:   1.2  $
//
//  DESCRIPTION:  Function prototypes for Glue Code Functions.
//
//  These functions are prototypes for the glue functions for all of the 
//  primitives that go into the function table in FN_TABLE.H
//  
//----------------------------------------------------------------------------

#ifndef _FN_ALL_H_
#define _FN_ALL_H_

//----------------------------------------------------------------------------
//             Forward references to classes and structures
//----------------------------------------------------------------------------

#include "types.h"
#include "g_types.h"
#include "d_defs.h"
#include "m_point.h"
#include "m_rect.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class  GFXSurface;
class  GFXFont;
class  GFXBitmap;

//----------------------------------------------------------------------------
//                                Clear Screen
//----------------------------------------------------------------------------

extern void GFXClearScreen(GFXSurface *io_pSurface, DWORD color);

//----------------------------------------------------------------------------
//                                 Draw Funcs
//----------------------------------------------------------------------------

extern void GFXDrawPoint2d   ( GFXSurface *io_pSurface, const Point2I *in_at, float in_w, DWORD in_color );
extern void GFXDrawLine2d    (GFXSurface *io_pSurface, const Point2I *in_st, const Point2I *in_en, DWORD in_color );
extern void GFXDrawRect2d     ( GFXSurface *io_pSurface, const RectI *in_rect, DWORD in_color );
extern void GFXDrawRect2d_f   ( GFXSurface *io_pSurface, const RectI *in_rect, float w, DWORD in_color );

//----------------------------------------------------------------------------
//                                Draw Text
//----------------------------------------------------------------------------

extern Int32 GFXDrawText_p( GFXSurface *, GFXFont *, const Point2I *, const void * );  // draws a string at the given point
extern Int32 GFXDrawText_r( GFXSurface *, GFXFont *, const RectI *, const void * );   // draws a string bounded by rectangle

//----------------------------------------------------------------------------
//                                 DrawBitmap
//----------------------------------------------------------------------------

extern void GFXDrawBitmap2d_f    (GFXSurface *io_pSurface, const GFXBitmap *in_pBM, const Point2I *in_at, GFXFlipFlag in_flip);
extern void GFXDrawBitmap2d_rf   (GFXSurface *io_pSurface, const GFXBitmap *in_pBM, const RectI *in_subRegion, const Point2I *in_at, GFXFlipFlag in_flip);
extern void GFXDrawBitmap2d_sf   (GFXSurface *io_pSurface, const GFXBitmap *in_pBM, const Point2I *in_at, const Point2I *in_stretch, GFXFlipFlag in_flip);
extern void GFXDrawBitmap2d_af   (GFXSurface *io_pSurface, const GFXBitmap *in_pBM, const Point2I *in_at, const Point2I *in_stretch, Int32 in_angle);
extern void GFXDrawBitmap2d_f_ec (GFXSurface *io_pSurface, const GFXBitmap *in_pBM, const Point2I *in_at, GFXFlipFlag in_flip);
extern void GFXDrawBitmap2d_rf_ec(GFXSurface *io_pSurface, const GFXBitmap *in_pBM, const RectI *in_subRegion, const Point2I *in_at, GFXFlipFlag in_flip);
extern void GFXDrawBitmap2d_sf_ec(GFXSurface *io_pSurface, const GFXBitmap *in_pBM, const Point2I *in_at, const Point2I *in_stretch, GFXFlipFlag in_flip);
extern void GFXDrawBitmap2d_af_ec(GFXSurface *io_pSurface, const GFXBitmap *in_pBM, const Point2I *in_at, const Point2I *in_stretch, Int32 in_angle);

//----------------------------------------------------------------------------
//                                Draw Poly
//----------------------------------------------------------------------------

extern void GFXDraw3DBegin(GFXSurface *sfc);
extern void GFXDraw3DEnd(GFXSurface *sfc);

//----------------------------------------------------------------------------
//                                New Poly Interface - state based.
//----------------------------------------------------------------------------

void GFXSetShadeSource(GFXSurface *io_pSurface, GFXShadeSource src);
void GFXSetHazeSource(GFXSurface *io_pSurface, GFXHazeSource src);
void GFXSetAlphaSource(GFXSurface *io_pSurface, GFXAlphaSource src);
void GFXSetTexturePerspective(GFXSurface *io_pSurface, Bool persp);
void GFXSetTransparency(GFXSurface *, Bool t);
void GFXSetFillMode(GFXSurface *io_pSurface, GFXFillMode fm);
void GFXSetHazeColorCF(GFXSurface *, const ColorF *);
void GFXSetHazeColorI(GFXSurface *, Int32 index);
void GFXSetConstantHaze(GFXSurface *, float haze);
void GFXSetConstantAlpha(GFXSurface *, float alpha);
void GFXSetConstantShadeF(GFXSurface *, float shade);
void GFXSetConstantShadeCF(GFXSurface *io_pSurface, const ColorF *cf);
void GFXSetTextureMap(GFXSurface *io_pSurface, const GFXBitmap *pTexture);
void GFXSetTextureWrap(GFXSurface *io_pSurface, Bool);
void GFXSetFillColorCF(GFXSurface *, const ColorF *);
void GFXSetFillColorI(GFXSurface *, Int32 index, DWORD paletteIndex);
void GFXAddVertexVTC(GFXSurface *io_pSurface, const Point3F *vert, const Point2F *tex, const GFXColorInfoF *c, DWORD key);
void GFXAddVertexV(GFXSurface *io_pSurface, const Point3F *vert, DWORD key);
void GFXAddVertexVT(GFXSurface *io_pSurface, const Point3F *vert, const Point2F *tex, DWORD key);
void GFXAddVertexVC(GFXSurface *io_pSurface, const Point3F *vert, const GFXColorInfoF *c, DWORD key);
void GFXEmitPoly(GFXSurface *io_pSurface);
void GFXSort3dBegin(GFXSurface *io_pSurface);
void GFXSort3dEnd(GFXSurface *io_pSurface);
void GFXSortEmitPoly(GFXSurface *);

void GFXRegisterTextureCB(GFXSurface *,GFXTextureHandle,
		GFXCacheCallback cb, int csizeX, 
		int csizeY, int lightScale, GFXLightMap *map);
void GFXRegisterTexture(GFXSurface *, GFXTextureHandle,
		int sizeX, int sizeY, int offsetX, int offsetY, int lightScale,
		GFXLightMap *map, const GFXBitmap *tex, int mipLevel);
Bool GFXSetTextureHandle(GFXSurface *, GFXTextureHandle);
GFXLightMap * GFXAllocateLightMap(GFXSurface *, int pixelSize);
GFXLightMap * GFXHandleGetLightMap(GFXSurface *);
void GFXHandleSetLightMap(GFXSurface *, int lightScale, GFXLightMap *map);
void GFXHandleSetTextureMap(GFXSurface *, const GFXBitmap *);
void GFXHandleSetMipLevel(GFXSurface *, int mipLevel);
void GFXHandleSetTextureSize(GFXSurface *, int newSize);
void GFXFlushTextureCache(GFXSurface *);
extern void GFXSetZTest(GFXSurface *, int);
extern void GFXClearZBuffer(GFXSurface *);

void GFXFlushTexture(GFXSurface*, const GFXBitmap*, const bool);

void GFXSetClipPlanes(GFXSurface*, const float, const float);

GFXHazeSource GFXGetHazeSource(GFXSurface*);
float         GFXGetConstantHaze(GFXSurface*);

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_FN_ALL_H_

