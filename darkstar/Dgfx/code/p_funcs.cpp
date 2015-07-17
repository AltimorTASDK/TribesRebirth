//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <stdio.h>

#include "g_surfac.h"
#include "g_pal.h"
#include "g_bitmap.h"
#include "g_contxt.h"
#include "fn_all.h"
#include "rn_manag.h"
#include "p_txcach.h"
#include "gfxmetrics.h"
#include "fn_table.h"
#include "gsdevice.h"
#include "p_funcs.h"
#include "p_edge.h"

#define GFX_MAX_POLY_VERTEX 100


//------------------------------------------------------------------------------
//-------------------------------------- Globals
// Exported globals...
//
GFXFillMode    g_fillMode           = ( GFXFillMode )-1;//GFX_FILL_CONSTANT; // Exported in p_funcs.h
GFXShadeSource g_shadeSource        = GFX_SHADE_NONE;
GFXHazeSource  g_hazeSource         = GFX_HAZE_NONE;
GFXAlphaSource g_alphaSource        = GFX_ALPHA_NONE;
Bool           g_transparent        = FALSE;
Bool           g_texturePerspective = FALSE;
bool           g_triError           = false;
bool           g_wBuffer            = true;
float g_shadeLevel = 0.0f;
float g_hazeLevel  = 0.0f;

rendererManager g_renderTable;                           // exported in rn_manag.h

GFXMetricsInfo GFXMetrics;                               // exported in gfxmetrics.h
GFXPrefInfo    GFXPrefs;                                 // exported in gfxmetrics.h

// Unexported globals... use of static keyword is depricated as of ANSI draft 2,
//  unnamed namespace preferred...
//
namespace {
   Int32 sg_hazeOffset      = 0;
   Int32 sg_hazeTableOffset = 0;
   Int32 sg_shadeOffset     = 0;

   GFXPolyDrawContext *sg_curPoly = &gfxPDC;

   int   sg_zsortEnable   = false;
   bool  sg_outlineMode   = false;
   bool  sg_wrapEnable    = false;
   bool  sg_triFanEnabled = false;

inline void
evalTriFanFlag()
{
   sg_triFanEnabled = ((g_shadeSource == GFX_SHADE_VERTEX) ||
                       (g_hazeSource == GFX_HAZE_VERTEX)   ||
                       (g_fillMode != GFX_FILL_CONSTANT && g_texturePerspective == false));
}


Int32 sg_triFanSeqKey = -1;
inline Int32
getNextTriFanKey()
{
   return sg_triFanSeqKey--;
}
   
};


//------------------------------------------------------------------------------
//-------------------------------------- ASM imports...
//
extern "C" const  Point3F *vList[];
extern "C" const  Point2F *tList[];
extern "C" const  GFXColorInfoF *cList[];
extern "C" UInt32 kList[];

extern "C" int vertexCount;
int iy[GFX_MAX_POLY_VERTEX];

int nextLefts[GFX_MAX_POLY_VERTEX];
int nextRights[GFX_MAX_POLY_VERTEX];

extern int g_polyFlags;


extern "C" void __cdecl NewRasterizer();
extern "C" int  __cdecl DeltaCalcTriUVQWH(void *calcStruct);
extern "C" int  __cdecl DeltaCalcTriUVWH(void *calcStruct);
extern "C" int  __cdecl DeltaCalcTriWH(void *calcStruct);
extern "C" int  __cdecl DeltaCalcPolyUVQW(void *calcStruct);
extern "C" int  __cdecl DeltaCalcTriUVQW(void *calcStruct);
extern "C" int  __cdecl DeltaCalcTriUVQWS(void *calcStruct);
extern "C" int  __cdecl DeltaCalcTriUVW(void *calcStruct);
extern "C" int  __cdecl DeltaCalcPolyW(void *calcStruct);
extern "C" int  __cdecl DeltaCalcTriUVWS(void *calcStruct);
extern "C" int  __cdecl DeltaCalcTriWS(void *calcStruct);
extern "C" void __cdecl GFXRenderSpanUV();
extern "C" void __cdecl GFXRenderSpanUVC();
extern "C" void __cdecl GFXRenderSpanUVT();
extern "C" void __cdecl GFXRenderSpanUVCT();
extern "C" void __cdecl GFXRenderSpanUVA();
extern "C" void __cdecl GFXRenderSpanUVS();
extern "C" void __cdecl GFXRenderSpanUVST();
extern "C" void __cdecl GFXRenderSpanUVSR();
extern "C" void __cdecl GFXRenderSpanUVW();
extern "C" void __cdecl GFXRenderSpanUVWR();
extern "C" void __cdecl GFXRenderSpanUVWTR();
extern "C" void __cdecl GFXRenderSpanUVWC();
extern "C" void __cdecl GFXRenderSpanUVWA();
extern "C" void __cdecl GFXRenderSpanUVWS();
extern "C" void __cdecl GFXRenderSpanUVWCT();
extern "C" void __cdecl GFXRenderSpanUVWST();
extern "C" void __cdecl GFXRenderSpanUVWCR();
extern "C" void __cdecl GFXRenderSpanUVWCTR();
extern "C" void __cdecl GFXRenderSpan();
extern "C" void __cdecl GFXRenderSpanA();
extern "C" void __cdecl GFXRenderSpanC();
extern "C" void __cdecl GFXRenderSpanS();

//------------------------------------------------------------------------------
//-------------------------------------- RENDERMANAGER IMPLEMENTATION
//
void __cdecl
badRendererCalled()
{
   AssertFatal(0, "You should not have reached here!");
}

int __cdecl
badDeltaCalcCalled(void*)
{
   AssertFatal(0, "You should not have reached here!");
   return -1;
}


rendererManager::rendererManager()
 : m_ignoreBadRenderers(false)
{
	int i;
	flags = 0;
	for(i = 0; i < 256; i++)
		spanRenderTable[i] = badRendererCalled;
	for(i = 0; i < 16; i++)
	{
		deltaCalcTriTable[i]  = badDeltaCalcCalled;
		deltaCalcPolyTable[i] = badDeltaCalcCalled;
	}
	installTriDeltaFunction(renderInterpUV | renderInterpW, DeltaCalcTriUVQW);
	installTriDeltaFunction(renderInterpUV | renderInterpW | renderInterpS, DeltaCalcTriUVQWS);
	installTriDeltaFunction(renderInterpUV | renderInterpW | renderInterpH, DeltaCalcTriUVQWH);
	installTriDeltaFunction(renderInterpUV, DeltaCalcTriUVW);
	installTriDeltaFunction(renderInterpUV | renderInterpS, DeltaCalcTriUVWS);
	installTriDeltaFunction(renderInterpUV | renderInterpH, DeltaCalcTriUVWH);
	installTriDeltaFunction(renderInterpS, DeltaCalcTriWS);
	installTriDeltaFunction(renderInterpH, DeltaCalcTriWH);
	installTriDeltaFunction(0, DeltaCalcPolyW);

	installPolyDeltaFunction(renderInterpUV | renderInterpW, DeltaCalcPolyUVQW);
	installPolyDeltaFunction(0, DeltaCalcPolyW);

	installFunction(0, (renderSpanFunc) GFXRenderSpan);
	installFunction(renderTransparent, (renderSpanFunc) GFXRenderSpan);
	installFunction(renderInterpS, 
					(renderSpanFunc) GFXRenderSpanS);
	installFunction(renderInterpH,
					(renderSpanFunc) GFXRenderSpanS);
	installFunction(renderInterpH | renderConstantS,
					(renderSpanFunc) GFXRenderSpanS);
	installFunction(renderConstantA, (renderSpanFunc)GFXRenderSpanA);

   installFunction(renderConstantS,
	       (renderSpanFunc) GFXRenderSpanC);

	installFunction(renderInterpUV,
					(renderSpanFunc) GFXRenderSpanUV);
	installFunction(renderInterpUV | renderInterpS,
					(renderSpanFunc) GFXRenderSpanUVS);
	installFunction(renderInterpUV | renderInterpH,
					(renderSpanFunc) GFXRenderSpanUVS);
	installFunction(renderInterpUV | renderInterpW | renderTransparent,
					(renderSpanFunc) GFXRenderSpanUVWTR);
	installFunction(renderInterpUV | renderInterpW | renderTransparent | renderWrapped,
					(renderSpanFunc) GFXRenderSpanUVWTR);
	installFunction(renderInterpUV | renderTransparent, 
					(renderSpanFunc) GFXRenderSpanUVT);
	installFunction(renderInterpUV | renderConstantA, 
					(renderSpanFunc) GFXRenderSpanUVA);
	installFunction(renderInterpUV | renderConstantA | renderConstantS, 
					(renderSpanFunc) GFXRenderSpanUVA);
	installFunction(renderInterpUV | renderConstantA | renderInterpW, 
					(renderSpanFunc) GFXRenderSpanUVWA);
	installFunction(renderInterpUV | renderConstantA | renderInterpW | renderConstantS, 
					(renderSpanFunc) GFXRenderSpanUVWA);
	installFunction(renderInterpUV | renderConstantS,
					(renderSpanFunc) GFXRenderSpanUVC);
	installFunction(renderInterpUV | renderConstantS | renderTransparent,
					(renderSpanFunc) GFXRenderSpanUVCT);
	installFunction(renderInterpUV | renderInterpS | renderTransparent,
					(renderSpanFunc) GFXRenderSpanUVST);
	installFunction(renderInterpUV | renderInterpH | renderTransparent,
					(renderSpanFunc) GFXRenderSpanUVST);

	installFunction(renderInterpUV | renderInterpW,
					(renderSpanFunc) GFXRenderSpanUVW);
	installFunction(renderInterpUV | renderInterpW | renderWrapped,
					(renderSpanFunc) GFXRenderSpanUVWR);
	installFunction(renderInterpUV | renderInterpW | renderConstantS,
					(renderSpanFunc) GFXRenderSpanUVWC);
	installFunction(renderInterpUV | renderInterpW | renderConstantS | renderWrapped,
					(renderSpanFunc) GFXRenderSpanUVWCR);
	installFunction(renderInterpUV | renderInterpW | renderConstantS | renderTransparent,
					(renderSpanFunc) GFXRenderSpanUVWCT);
	installFunction(renderInterpUV | renderInterpW | renderConstantS | renderTransparent | renderWrapped,
					(renderSpanFunc) GFXRenderSpanUVWCTR);
	installFunction(renderInterpUV | renderInterpW | renderInterpS,
					(renderSpanFunc) GFXRenderSpanUVWS);
	installFunction(renderInterpUV | renderInterpW | renderInterpH,
					(renderSpanFunc) GFXRenderSpanUVWS);
	installFunction(renderInterpUV | renderInterpW | renderInterpS | renderTransparent,
					(renderSpanFunc) GFXRenderSpanUVWST);
	installFunction(renderInterpUV | renderInterpW | renderInterpH | renderTransparent,
					(renderSpanFunc) GFXRenderSpanUVWST);
}


//------------------------------------------------------------------------------
//-------------------------------------- GFX Functions...
//

GFXPalette::MultiPalette *curMultiPalette = NULL;

void clearMultiPalette()
{
   curMultiPalette = NULL;
}   

void GFXSetupShadeMap(GFXSurface *)
{
   if(!curMultiPalette)
      return;
   switch (g_shadeSource | g_hazeSource)
   {
     case GFX_SHADE_CONSTANT | GFX_HAZE_NONE:
      gfxPDC.shadeMap = curMultiPalette->shadeMap + sg_shadeOffset;
	   break;
     case GFX_SHADE_NONE | GFX_HAZE_CONSTANT:
	   gfxPDC.shadeMap = curMultiPalette->hazeMap + sg_hazeOffset;
	   break; 
     case GFX_SHADE_CONSTANT | GFX_HAZE_CONSTANT:
	   gfxPDC.shadeMap = curMultiPalette->shadeMap + sg_shadeOffset + sg_hazeTableOffset;
	   break;
     case GFX_SHADE_VERTEX | GFX_HAZE_NONE:
	   gfxPDC.shadeMap = curMultiPalette->shadeMap;
	   break;
     case GFX_SHADE_VERTEX | GFX_HAZE_CONSTANT:
	   gfxPDC.shadeMap = curMultiPalette->shadeMap + sg_hazeTableOffset;
	   break;
     case GFX_SHADE_NONE | GFX_HAZE_VERTEX:
	   gfxPDC.shadeMap = curMultiPalette->hazeMap;
	   break;
   }
}

void GFXShadeHazeChanged(GFXSurface *io_pSurface)
{
   gfxPDC.shadeScale = float((io_pSurface->getPalette()->getMaxShade() << 16) | 0xFF00);
   GFXSetupShadeMap(io_pSurface);
   switch(g_shadeSource | g_hazeSource)
   {
      case GFX_SHADE_NONE | GFX_HAZE_NONE:
	 g_renderTable.clearRenderFlag(renderInterpS);
	 g_renderTable.clearRenderFlag(renderInterpH);
	 g_renderTable.clearRenderFlag(renderConstantS);
	 break;
      case GFX_SHADE_CONSTANT | GFX_HAZE_CONSTANT:
      case GFX_SHADE_CONSTANT | GFX_HAZE_NONE:
      case GFX_SHADE_NONE | GFX_HAZE_CONSTANT:
	 g_renderTable.clearRenderFlag(renderInterpS);
	 g_renderTable.clearRenderFlag(renderInterpH);
	 g_renderTable.setRenderFlag(renderConstantS);
	 break;
      case GFX_SHADE_VERTEX | GFX_HAZE_NONE:
      case GFX_SHADE_VERTEX | GFX_HAZE_CONSTANT:
	 g_renderTable.clearRenderFlag(renderInterpH);
	 g_renderTable.clearRenderFlag(renderConstantS);
	 g_renderTable.setRenderFlag(renderInterpS);
	 break;
      case GFX_SHADE_NONE | GFX_HAZE_VERTEX:
	 g_renderTable.clearRenderFlag(renderInterpS);
	 g_renderTable.clearRenderFlag(renderConstantS);
	 g_renderTable.setRenderFlag(renderInterpH);
   }
}

void GFXSetShadeSource(GFXSurface *io_pSurface, GFXShadeSource src)
{
	if(src == g_shadeSource)
		return;
	g_shadeSource = src;
   GFXShadeHazeChanged(io_pSurface);
   evalTriFanFlag();
}


void GFXSetHazeSource(GFXSurface *io_pSurface, GFXHazeSource src)
{
	if(src == g_hazeSource)
		return;
	g_hazeSource = src;
   GFXShadeHazeChanged(io_pSurface);
   evalTriFanFlag();
}


void GFXSetAlphaSource(GFXSurface *, GFXAlphaSource src)
{
	g_alphaSource = src;
	switch(src)
	{
		case GFX_ALPHA_NONE:
			g_renderTable.clearRenderFlag(renderConstantA);
			break;
		case GFX_ALPHA_CONSTANT:
			g_renderTable.setRenderFlag(renderConstantA);
			break;
		case GFX_ALPHA_VERTEX:
			// unsupported option in software...
			g_renderTable.clearRenderFlag(renderConstantA);
			break;
		case GFX_ALPHA_ADD:
		case GFX_ALPHA_SUB:
		case GFX_ALPHA_TEXTURE:
//         if(curMultiPalette) {
//	         gfxPDC.shadeMap = curMultiPalette->transMap;
//            int test = toupper(*((BYTE*)gfxPDC.shadeMap));
//            test;
//         }
			g_renderTable.setRenderFlag(renderConstantA);
			break;
		case GFX_ALPHA_FILL:
         if(curMultiPalette)
	         gfxPDC.shadeMap = curMultiPalette->transMap;
			g_renderTable.setRenderFlag(renderConstantA);
			break;
	}       
}

void GFXSetZMode(GFXSurface *, bool wBuffer)
{
   g_wBuffer = wBuffer;
}


void GFXSetTexturePerspective(GFXSurface * /*io_pSurface*/, Bool persp)
{
	if(g_texturePerspective == persp)
		return;
   if(g_wBuffer)
	   g_texturePerspective = persp;
   else
      g_texturePerspective = false;
	if(g_texturePerspective)
		g_renderTable.setRenderFlag(renderInterpW);
	else
		g_renderTable.clearRenderFlag(renderInterpW);

   evalTriFanFlag();
}


void GFXSetTransparency(GFXSurface *, Bool t)
{
	if(t == g_transparent)
		return;
   g_transparent = t;
	if(t)
		g_renderTable.setRenderFlag(renderTransparent);
	else
		g_renderTable.clearRenderFlag(renderTransparent);
}

void GFXSetFillMode(GFXSurface * io_pSurface, GFXFillMode fm)
{
	if(fm == g_fillMode)
		return;
	g_fillMode = fm;
   evalTriFanFlag();
	if(fm == GFX_FILL_CONSTANT)
	{
		g_renderTable.clearRenderFlag(renderInterpUV);
		g_renderTable.clearRenderFlag(renderInterpW);
		g_renderTable.clearRenderFlag(renderTransparent);
	}
	else
	{
		g_renderTable.setRenderFlag(renderInterpUV);
		if(g_texturePerspective)
			g_renderTable.setRenderFlag(renderInterpW);
		if(g_transparent)
			g_renderTable.setRenderFlag(renderTransparent);
      if(fm == GFX_FILL_TWOPASS)
         curMultiPalette = io_pSurface->getPalette()->palette;
	}
}

void GFXSetHazeColorCF(GFXSurface *, const ColorF *)
{
	// can't set the haze color in software.
}

void GFXSetHazeColorI(GFXSurface *, Int32 /*index*/) 
{
	// can't set the haze color in software.
}

void GFXSetConstantHaze(GFXSurface *surf, float haze)
{
	GFXPalette *pal = surf->getPalette();
   g_hazeLevel   = haze;
   sg_hazeOffset = Int32(pal->getMaxShade() * haze) << PAL_SHIFT;
   sg_hazeTableOffset = Int32(pal->getMaxHaze() * haze) << (PAL_SHIFT + pal->shadeShift);
   GFXSetupShadeMap(surf);
}

void GFXSetConstantAlpha(GFXSurface* /*io_pSurface*/,
                                 float       /*alpha*/)
{
   // not supported in software...
}

void GFXSetConstantShadeF(GFXSurface *surf, float shade)
{
	if(shade > 1.0f)
		shade = 1.0f;
	else if(shade < 0.0f)
		shade = 0.0f;
	GFXPalette *pal = surf->getPalette();
   g_shadeLevel  = shade;
   sg_shadeOffset = Int32(pal->getMaxShade() * shade) << PAL_SHIFT;
   GFXSetupShadeMap(surf);
}

void GFXSetConstantShadeCF(GFXSurface *io_pSurface, const ColorF *cf)
{
	GFXSetConstantShadeF(io_pSurface, float(cf->red * .22 + cf->green * .67 + cf->blue * .11));
}

Bool GFXSetTextureHandle(GFXSurface * /*io_pSurface*/, GFXTextureHandle tex)
{
	return gfxTextureCache.setTextureHandle(tex);
}

void GFXRegisterTextureCB(GFXSurface *, GFXTextureHandle tex,
		GFXCacheCallback cb, int csizeX, 
		int csizeY, int lightScale, GFXLightMap *map)
{
	gfxTextureCache.registerTexture(tex, cb, csizeX, csizeY, lightScale, map);
}

void GFXRegisterTexture(GFXSurface *, GFXTextureHandle th,
		int sizeX, int sizeY, int offsetX, int offsetY, int lightScale,
		GFXLightMap *map, const GFXBitmap *tex, int mipLevel)
{
	gfxTextureCache.registerTexture(th, sizeX, sizeY, offsetX, offsetY,
			lightScale, map, tex, mipLevel);
}

GFXLightMap * GFXAllocateLightMap(GFXSurface *, int pixelSize)
{
	return gfxLightMapCache.allocate(pixelSize);
}

void GFXHandleSetLightMap(GFXSurface *, int lightScale, GFXLightMap *lMap)
{
	gfxTextureCache.setLightMap(lightScale, lMap);
}

GFXLightMap * GFXHandleGetLightMap(GFXSurface *)
{
	return gfxTextureCache.getLightMap();
}

void GFXHandleSetTextureMap(GFXSurface *, const GFXBitmap *texture)
{
	gfxTextureCache.setTextureMap(texture);
}

void GFXHandleSetMipLevel(GFXSurface *, int mipLevel)
{
	gfxTextureCache.setMipLevel(mipLevel);
}
 
void GFXHandleSetTextureSize(GFXSurface *, int newSize)
{
	gfxTextureCache.setTextureSize(newSize);
}

void GFXFlushTextureCache(GFXSurface *)
{
	gfxTextureCache.flushCache();
}
 
void GFXSetTextureMap(GFXSurface * io_pSurface, const GFXBitmap *pTexture)
{
	sg_curPoly->textureMap       = pTexture->getAddress(0,0);
	sg_curPoly->textureMapStride = pTexture->getStride();
	sg_curPoly->textureMapHeight = pTexture->getHeight();
   
   if(!curMultiPalette || curMultiPalette->paletteIndex != pTexture->paletteIndex)
   {
      curMultiPalette = io_pSurface->getPalette()->findMultiPalette(pTexture->paletteIndex);
      if(g_alphaSource == GFX_ALPHA_TEXTURE ||
         g_alphaSource == GFX_ALPHA_ADD     ||
         g_alphaSource == GFX_ALPHA_SUB)
	      ;//gfxPDC.shadeMap = curMultiPalette->transMap;
      else
         GFXSetupShadeMap(io_pSurface);
   }
   if(g_alphaSource == GFX_ALPHA_TEXTURE ||
      g_alphaSource == GFX_ALPHA_ADD     ||
      g_alphaSource == GFX_ALPHA_SUB)
	   gfxPDC.shadeMap = curMultiPalette->transMap;
}

void GFXSetFillColorCF(GFXSurface *, const ColorF *)
{
	// not supported in software.
}

void GFXSetFillColorI(GFXSurface * io_pSurface, Int32 index, DWORD paletteIndex)
{
   if(!curMultiPalette || paletteIndex != curMultiPalette->paletteIndex)
   {
      curMultiPalette = io_pSurface->getPalette()->findMultiPalette(paletteIndex);
      GFXSetupShadeMap(io_pSurface);
      if(g_alphaSource == GFX_ALPHA_FILL)
         gfxPDC.shadeMap = curMultiPalette->transMap;
   }
   if (((g_shadeSource | g_hazeSource) == (GFX_SHADE_CONSTANT | GFX_HAZE_NONE)) ||
       ((g_shadeSource | g_hazeSource) == (GFX_SHADE_NONE | GFX_HAZE_CONSTANT)) ||
       ((g_shadeSource | g_hazeSource) == (GFX_SHADE_CONSTANT | GFX_HAZE_CONSTANT)))
      sg_curPoly->color = gfxPDC.shadeMap[index];
   else
   {
      if(paletteIndex == 0xFFFFFFFF || g_alphaSource == GFX_ALPHA_FILL)
         sg_curPoly->color = index;
      else
      {
         Int32 offset = (io_pSurface->getPalette()->getMaxShade() << 8) + index;
         sg_curPoly->color = curMultiPalette->shadeMap[offset];
      }
   }
}

Bool VertexValid(GFXSurface *io_pSurface, const Point3F *vert, const Point2F *tex)
{
	float width = (float) io_pSurface->getWidth();
	float height = (float) io_pSurface->getHeight();
	if(tex && g_fillMode != GFX_FILL_CONSTANT)
	{
		if(tex->x < 0.0f || tex->y < 0.0f || tex->x > 1.0f || tex->y > 1.0f)
			return FALSE;
	}
	if(vert->x < -0.5f || vert->y < -0.5f || vert->x > width || vert->y > height)
		return FALSE;
	return TRUE;            
}

void 
GFXAddVertexVTC(GFXSurface*          io_pSurface,
                const Point3F*       vert,
                const Point2F*       tex,
                const GFXColorInfoF* c,
                DWORD                k)
{
	AssertFatal(vertexCount < GFX_MAX_POLY_VERTEX, "GFXAddVertexVTC: Exceeded max vertex count");

//   AssertFatal(vert->z != 0, "Error, not possible...");

	if(!VertexValid(io_pSurface, vert, tex))
      g_triError = true;
	//AssertFatal(VertexValid(io_pSurface, vert, tex), "Invalid vertex in call to GFXAddVertex");

   // Automatic trifan handling...
   //
   if (vertexCount      == 3 &&
       sg_triFanEnabled == true) {
      // Replace the edge key for the fan with a sequence key and emit the
      //  fan poly
      //
      UInt32 kTemp = kList[2];
      kList[2]     = UInt32(getNextTriFanKey());
      io_pSurface->emitPoly();

      // Reset the vertex list...
      //
      vertexCount = 2;
      vList[1]    = vList[2];
      tList[1]    = tList[2];
      cList[1]    = cList[2];
      kList[1]    = kTemp;
      kList[0]    = kList[2];
   }
	
   vList[vertexCount] = vert;
	tList[vertexCount] = tex;
	cList[vertexCount] = c;
	kList[vertexCount] = k;
	vertexCount++;
}

void GFXAddVertexV(GFXSurface *io_pSurface, const Point3F *vert, DWORD k)
{
	GFXAddVertexVTC(io_pSurface, vert, NULL, NULL, k);
}

void GFXAddVertexVT(GFXSurface *io_pSurface, const Point3F *vert, const Point2F *tex, DWORD k)
{
	GFXAddVertexVTC(io_pSurface, vert, tex, NULL, k);
}

void GFXAddVertexVC(GFXSurface *io_pSurface, const Point3F *vert, const GFXColorInfoF *c, DWORD k)
{
	GFXAddVertexVTC(io_pSurface, vert, NULL, c, k);
}

void GFXEmitPolyLines(GFXSurface *io_pSurface)
{
	AssertFatal(vertexCount >= 3, "Too few vertices in polygon");
	GFXMetrics.incEmittedPolys();

   if(g_triError)
   {
      g_triError = false;
      vertexCount = 0;
      return;
   }

	if(g_renderTable.calcDelta(&gfxPDC, vertexCount == 3))
	{
		GFXMetrics.incRenderedPolys();

		int i;
		const Point3F *start = vList[vertexCount - 1];
		for(i = 0; i < vertexCount; i++)
		{
			const Point3F *end = vList[i];
			io_pSurface->drawLine2d(&Point2I(Int32(start->x), Int32(start->y)), 
                                 &Point2I(Int32(end->x),   Int32(end->y)), 255);
			start = end;
		}
	}
	vertexCount = 0;
}

void GFXEmitPoly(GFXSurface *io_pSurface)
{
	AssertFatal(vertexCount >= 3, "Too few vertices in polygon");
	gfxPDC.rowStride = io_pSurface->getStride();
	gfxPDC.rowAddr   = io_pSurface->getAddress(0, 0);
	GFXMetrics.incEmittedPolys();

   if(g_triError)
   {
      g_triError = false;
      vertexCount = 0;
      return;
   }

	if(g_renderTable.calcDelta(&gfxPDC, vertexCount == 3))
	{
      GFXSetupShadeMap(io_pSurface);
		if(g_fillMode == GFX_FILL_TWOPASS)
			gfxTextureCache.prepareForDraw(gfxTextureCache.getCurrentEntry(), io_pSurface->getPalette());

		g_renderTable.getRenderer()();
		NewRasterizer();
		GFXMetrics.incRenderedPolys();
	}
	vertexCount = 0;
}

void GFXDraw3DBeginLines(GFXSurface *io_pSurface)
{
	GFXMetrics.reset();
	io_pSurface->clear(0);
   
#ifdef DEBUG
   GFXMetrics.textureSpaceUsed       = 0;
   GFXMetrics.textureBytesDownloaded = 0;
   gfxTextureCache.frameU++;
#endif
}

void GFXDraw3DEnd(GFXSurface *)
{

}

extern void GFXSortDrawPoint3d(GFXSurface *io_pSurface, const Point2I *in_at, float in_w, DWORD in_color);
extern void GFXSortDrawRect3d(GFXSurface *io_pSurface, const RectI *in_at, float in_w, DWORD in_color);

void GFXSetZTest(GFXSurface *, int enable)
{
	if(enable == sg_zsortEnable)
		return;
   if(enable != -1) {
	   sg_zsortEnable = enable;
   } else {
      rclip_table.fnDraw3DBegin = GFXDraw3DBegin;
   }
   
	if(sg_outlineMode)
      return;
	else if(sg_zsortEnable == GFX_ZTEST  ||
           sg_zsortEnable == GFX_ZWRITE ||
           sg_zsortEnable == GFX_ZTEST_AND_WRITE ||
           sg_zsortEnable == GFX_ZALWAYSBEHIND) {
		rclip_table.fnDraw3DEnd  = GFXSort3dEnd;
		rclip_table.fnEmitPoly   = GFXSortEmitPoly;
		rclip_table.fnDrawPoint  = GFXSortDrawPoint3d;
		rclip_table.fnDrawRect_f = GFXSortDrawRect3d;
      if(sg_zsortEnable == GFX_ZALWAYSBEHIND)
         g_polyFlags = FLAG_ALWAYS_BEHIND;
      else if(sg_zsortEnable == GFX_ZWRITE)
         g_polyFlags = FLAG_ALWAYS_BEHIND;
      else
         g_polyFlags = 0;
	}
	else if(sg_zsortEnable == GFX_NO_ZTEST) {
      rclip_table.fnDraw3DEnd  = GFXDraw3DEnd;
      rclip_table.fnEmitPoly   = GFXEmitPoly;
      rclip_table.fnDrawPoint  = GFXDrawPoint2d;
      rclip_table.fnDrawRect_f = GFXDrawRect2d_f;
	}
}


void
GFXSoftwareDevice::messageSurface(const int   in_argc,
                                  const char* in_argv[])
{
   if (!strcmp(in_argv[0], "flushCache")) {
      gfxTextureCache.flushCache();
   }
   else if (!strcmp(in_argv[0], "enableCacheNoise")) {
      extern void setupCacheNoise(const float);

      if (in_argc == 1) {
         setupCacheNoise(0.5f);
      } else {
         float newBias = atof(in_argv[1]);
         setupCacheNoise(newBias);
      }
   }
   else if (!strcmp(in_argv[0], "disableCacheNoise")) {
      extern void disableCacheNoise();
      disableCacheNoise();
   }
#ifdef DEBUG
   else if (!strcmp(in_argv[0], "outline")) {
      sg_outlineMode = !sg_outlineMode;

      if(sg_outlineMode) {
         rclip_table.fnDraw3DBegin = GFXDraw3DBeginLines;
         rclip_table.fnDraw3DEnd   = GFXDraw3DEnd;
         rclip_table.fnEmitPoly    = GFXEmitPolyLines;
         rclip_table.fnDrawPoint   = GFXDrawPoint2d;
         rclip_table.fnDrawRect_f  = GFXDrawRect2d_f;
      }
      else {
         GFXSetZTest(NULL, -1);
      }
   }
#endif
}


void GFXClearZBuffer(GFXSurface *)
{

}

void GFXSetTextureWrap(GFXSurface *, Bool wrap)
{
	if(sg_wrapEnable == wrap)
		return;
	sg_wrapEnable = wrap;
	if(wrap)
		g_renderTable.setRenderFlag(renderWrapped);
	else
		g_renderTable.clearRenderFlag(renderWrapped);
}

void 
GFXFlushTexture(GFXSurface*, const GFXBitmap*, const bool)
{
   //
}

void
GFXSetClipPlanes(GFXSurface* /*io_pSurface*/,
                 const float /*in_nearDist*/,
                 const float /*in_farDist*/)
{
   //
}

GFXHazeSource
GFXGetHazeSource(GFXSurface* /*io_pSurface*/)
{
   return g_hazeSource;
}

float
GFXGetConstantHaze(GFXSurface* /*io_pSurface*/)
{
   return g_hazeLevel;
}
