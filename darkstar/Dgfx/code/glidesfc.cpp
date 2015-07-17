#include <stdlib.h>
#include "glidesfc.h"

#include "g_bitmap.h"
#include "g_pal.h"
#include "fn_all.h"
#include "fn_table.h"
#include "gfxmetrics.h"
#include "p_txcach.h"

//#pragma pack(8)
#define __MSC__
#define DYNAHEADER
#include "glide.h"
#include "glidetx.h"
#include "d_caps.h"
#include "r_clip.h"

int NearestPow(int size)
{
   int oneCount = 0;
   int shiftCount = -1;
   while(size)
   {
      if(size & 1)
         oneCount++;
      shiftCount++;
      size >>= 1;
   }
   if(oneCount > 1)
      shiftCount++;
   return (1 << shiftCount);
}
extern FunctionTable glide_table;
extern "C" {
   void __cdecl UnloadGlide();
}

int glideHackVal = 0;

namespace Glide 
{

extern bool allowSubSampling;
bool idle = true;
bool drawToFrontBuffer = false;
bool ztest = true;
bool queuePolys = false;

bool sg_bilinear = true;

GrTextureFilterMode_t sg_bilinearFilterMode   = GR_TEXTUREFILTER_BILINEAR;
GrTextureFilterMode_t sg_nonblinearFilterMode = GR_TEXTUREFILTER_POINT_SAMPLED;

const GFXBitmap *newTexture = NULL;

void plugCallback(const int   /*in_argc*/,
                  const char* in_argv[],
                  GFXSurface* pSurface)
{
   if(!strcmp(in_argv[0], "idle")) {
      idle = !idle;
   }
   else if (!strcmp(in_argv[0], "flushCache")) {
      pSurface->flushTextureCache();
   }
#ifdef DEBUG
   else if(!strcmp(in_argv[0], "q")) {
      queuePolys = !queuePolys;
   }
   else if(!strcmp(in_argv[0],"zt")) {
      ztest = !ztest;
   }
   else if(!strcmp(in_argv[0], "fb")) {
      drawToFrontBuffer = !drawToFrontBuffer;
      if(drawToFrontBuffer)
         grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      else
         grRenderBuffer(GR_BUFFER_BACKBUFFER);
   }
   else if (!strcmp(in_argv[0], "toggleBiLinear")) {
      sg_bilinear = (sg_bilinear == false);

      if (sg_bilinear == true) {
         sg_bilinearFilterMode = GR_TEXTUREFILTER_BILINEAR;
      } else {
         sg_bilinearFilterMode = GR_TEXTUREFILTER_POINT_SAMPLED;
      }
      grTexFilterMode(0, sg_bilinearFilterMode, sg_bilinearFilterMode);
      grTexFilterMode(1, sg_bilinearFilterMode, sg_bilinearFilterMode);
   }
#endif
}
   
struct PolyStore
{
   PolyStore *pnext, *pprev;
   int state;
   int visxstart;
   DWORD spanList;
   DWORD renderer; // unused...
   DWORD flags;
   DWORD curblock; // current block of spans (4 to a block)
   // step constants
   double dwdx, dwdy, w0;
   HandleCacheEntry *tce;
   GrVertex *vertBase;
   Point3F *tvertBase;
   int vertexCount;
};

struct edge_t {
   edge_t   *pnext, *pprev, *pnextremove;
   Int32 x;
   Int32 dx;
   Int32 leading;
   PolyStore *ptrailsurf, *pleadsurf;
};

};
//----------------------------------------------------------------------------

extern "C" const Point3F *vList[];
extern "C" const Point2F *tList[];
extern "C" const GFXColorInfoF *cList[];
extern "C" DWORD kList[]; // key list.
extern "C" Glide::PolyStore  surfstack;
extern "C" Glide::edge_t  *pavailedge;
extern "C" Glide::edge_t  edgehead;
extern "C" Glide::edge_t  edgetail;
extern "C" Glide::edge_t  edges[];
extern "C" int __cdecl DeltaCalcPolyW(void *calcStruct);
extern "C" int __cdecl gfx_add_edges(void *surface);
extern "C" int __cdecl gfx_scan_edges_bool(int dstY);
extern "C" int vertexCount;

//----------------------------------------------------------------------------

namespace Glide
{
   
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int CacheUsageCount;
TextureCache *tCache = NULL;
HandleCache *hCache = NULL;

static DWORD hazeColor;

void Surface::_lock(const GFXLockMode in_lockMode)
{
   AssertFatal(in_lockMode == GFX_NORMALLOCK,
               "Double lock not supported in glide mode");
   in_lockMode;
//   tCache = texCache;
//   hCache = hndCache;
}

void SetFillModeDualTMU(GFXSurface *, GFXFillMode fm);
void Draw3dBegin(GFXSurface *);
void InitVars();


Bool Surface::_setPalette(GFXPalette *lpPalette, Int32, Int32, bool)
{ 
   if(!lpPalette)
      return(true);

   pPalette = lpPalette;
   tCache->tmu[0].curPaletteIndex = 0xFFFFFFFF;
   tCache->tmu[1].curPaletteIndex = 0xFFFFFFFF;

   setHazeColor(lpPalette->hazeColor);

   GuTexTable t;
   int i;
   for(i = 0; i < 256; i++)
   {
      PALETTEENTRY color = lpPalette->palette[0].color[i];
      t.palette.data[i] = (UInt32(color.peRed)   << 16) | 
                          (UInt32(color.peGreen) << 8 ) | 
                          (UInt32(color.peBlue) );
   }
   grTexDownloadTable(0, GR_TEXTABLE_PALETTE, (void *) &t);
   grTexDownloadTable(1, GR_TEXTABLE_PALETTE, (void *) &t);
   GFXMetrics.numPaletteDLs++;
   return TRUE;
}

Surface::Surface()
{
   surfaceType = SURFACE_3DFX;
   functionTable = &glide_table;
   alphaBlend = 0;
   sstWinOpen = false;

   //nearDist = 0.2;
   //farDist  = 20000;
}

DWORD Surface::getCaps()
{
   return GFX_DEVCAP_SUPPORTS_CONST_ALPHA | GFX_DEVCAP_PALETTE_CHANGE_NOFLASH;
}


void Surface::drawSurfaceToBitmap(GFXBitmap *bmp)
{
   if(bmp->getWidth() != surfaceWidth || bmp->getHeight() != surfaceHeight ||
         bmp->bitDepth != 24)
      return;
   GrLfbInfo_t info;
   info.size = sizeof(info);
   
   grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER, 
         GR_LFBWRITEMODE_565, GR_ORIGIN_UPPER_LEFT, FXTRUE,
         &info);

   int i, j;
   for(j = 0; j < surfaceHeight; j++)
   {
      unsigned short *srcPtr = (unsigned short*)(((char*)info.lfbPtr) + j*info.strideInBytes);
      unsigned char *dstPtr = bmp->getAddress(0, j);
      for(i = 0; i < surfaceWidth; i++)
      {
         unsigned short pix = *srcPtr++;
         *dstPtr++ = (pix & 0x1F) << 3;
         *dstPtr++ = ((pix >> 5) & 0x3F) << 2;
         *dstPtr++ = (pix >> 11) << 3;
      }
   }
   grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_FRONTBUFFER);
}


int g_useCard = 0;

bool Surface::init(Vector<ResSpecifier>* resVector,
                   bool&                 windowed)
{
   windowed = false;
   resVector->setSize(0);
   static bool in      = FALSE;
   static int  highRes = 0;
   if(!in)
   {
      GrHwConfiguration hwconfig;
      if(!LoadGlide())
         return false;
      grSstQueryBoards(&hwconfig);
      if( hwconfig.num_sst < 1)
         return false;
      else if (hwconfig.num_sst > 1) {
         // Must select the card we're going to use...

         g_useCard = 0;
      } else {
         g_useCard = 0;
      }


      grGlideInit();
      grSstQueryHardware(&hwconfig);

      // Test for high res modes
      if (hwconfig.SSTs[g_useCard].type != GR_SSTTYPE_AT3D) {
         highRes = 0;

         // Both Voodoo and SST96 put the fbRam in the same place, so...
         if (hwconfig.SSTs[g_useCard].sstBoard.VoodooConfig.fbRam >= 4) {
            highRes = 1;   // low bit indicates 800x600

            if (hwconfig.SSTs[g_useCard].sstBoard.VoodooConfig.fbRam >= 8)
               highRes |= 2;
         }
         
         if (hwconfig.SSTs[g_useCard].sstBoard.VoodooConfig.sliDetect == 1) {
            // Test the SLI configuration
            if (hwconfig.SSTs[g_useCard].sstBoard.VoodooConfig.fbRam >= 2) {
               highRes |= 1;
            }
            
            // Second bit indicates 960x720 (1024?)
            if (hwconfig.SSTs[g_useCard].sstBoard.VoodooConfig.fbRam >= 4) {
               highRes |= 2;
            }
         }
      } else {
         AssertWarn(0, "What the heck is an AT3D?");
      }
         
      InitVars();
      in = true;
   }

   resVector->clear();
   resVector->push_back(ResSpecifier(Point2I(512, 384), false));
   resVector->push_back(ResSpecifier(Point2I(640, 400), false));
   resVector->push_back(ResSpecifier(Point2I(640, 480), false));
   if (highRes & 0x1) {
//      resVector->push_back(ResSpecifier(Point2I(856, 480), false));
      resVector->push_back(ResSpecifier(Point2I(800, 600), false));
   }
   if (highRes & 0x2) {
//      resVector->push_back(ResSpecifier(Point2I(960,  720), false));
      resVector->push_back(ResSpecifier(Point2I(1024, 768), false));
   }
   
   // if it's rush/voodoo2, do this:
   //windowed = true;
   
   return in;
}


GFXSurface*
Surface::create(HWND  hwnd,
                Int32 in_width,
                Int32 in_height, bool fullscreen)
{
   Surface *ret = new Surface();
   if(!ret)
      return NULL;
   ret->surfaceWidth = in_width;
   ret->surfaceHeight = in_height;
   ret->surfaceStride = 2048;
   GrHwConfiguration hwconfig;
   grGlideInit();
   // qeury hardware
   if(!grSstQueryHardware(&hwconfig))
   {
      delete ret;
      UnloadGlide();
      return NULL;
   }
   ret->tmuCount = 1;
   if(hwconfig.SSTs[g_useCard].type == GR_SSTTYPE_VOODOO) {
      ret->tmuCount = hwconfig.SSTs[g_useCard].sstBoard.VoodooConfig.nTexelfx;
      if (hwconfig.SSTs[g_useCard].sstBoard.VoodooConfig.fbiRev > 200) {
         ret->isVoodoo2 = true;
      } else {
         ret->isVoodoo2 = false;
      }
   } else if(hwconfig.SSTs[g_useCard].type == GR_SSTTYPE_SST96) {
      ret->tmuCount = hwconfig.SSTs[g_useCard].sstBoard.SST96Config.nTexelfx;
   }

   grSstSelect(g_useCard);
   int mode;
   if(!fullscreen)
      mode = GR_RESOLUTION_NONE;

   else if (in_width == 512 && in_height == 384)
      mode = GR_RESOLUTION_512x384;
   else if (in_width == 640 && in_height == 400)
      mode = GR_RESOLUTION_640x400;
   else if (in_width == 640 && in_height == 480)
      mode = GR_RESOLUTION_640x480;
   else if (in_width == 800 && in_height == 600)
      mode = GR_RESOLUTION_800x600;
//   else if (in_width == 856 && in_height == 480)
//      mode = GR_RESOLUTION_856x480;
//   else if (in_width == 960 && in_height == 720)
//      mode = GR_RESOLUTION_960x720;
   else if (in_width == 1024 && in_height == 768)
      mode = GR_RESOLUTION_1024x768;
   else
   {
      delete ret;
      return NULL;
   }

   FxBool openSuccess;
   if (fullscreen == true) {
      openSuccess = grSstWinOpen((unsigned long)NULL,
                                 mode,
                                 GR_REFRESH_60Hz,
                                 GR_COLORFORMAT_ARGB,
                                 GR_ORIGIN_UPPER_LEFT,
                                 2, 1);
   } else {
      openSuccess = grSstWinOpen((unsigned long) hwnd,
                                 mode,
                                 GR_REFRESH_60Hz,
                                 GR_COLORFORMAT_ARGB,
                                 GR_ORIGIN_UPPER_LEFT,
                                 2, 1);
   }

   if(openSuccess == FALSE) {
      delete ret;
      return NULL;
   }
   ret->sstWinOpen = true;

   grGammaCorrectionValue(ret->gamma);
   grCullMode(GR_CULL_NEGATIVE);
   grTexFilterMode(0, sg_bilinearFilterMode, sg_bilinearFilterMode);
   grTexMipMapMode(0, GR_MIPMAP_NEAREST, FALSE);
   grTexClampMode(0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
   grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);   // this is a NOP on 3Dfx.
   grTexClampMode(0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

   CacheUsageCount++;
   if(!tCache) {
      tCache = new TextureCache(ret->tmuCount, 900);
      hCache = new HandleCache(2048, 2048);
   }
   tCache->flush();
   hCache->flush();
   if(ret->tmuCount > 1) {
      glide_table.fnSetFillMode = SetFillModeDualTMU;
      grTexFilterMode(1, sg_bilinearFilterMode, sg_bilinearFilterMode);
      grTexMipMapMode(1, GR_MIPMAP_NEAREST, FALSE);
   }
   return ret;
}

void Surface::setSurfaceAlphaBlend(ColorF *color, float alpha)
{
   alphaColor = *color;
   alphaBlend = alpha;
}

void Surface::_setGamma()
{
   grGammaCorrectionValue(gamma);
}

Surface::~Surface()
{
   if (!--CacheUsageCount) {
      delete tCache; tCache = 0;
      delete hCache; hCache = 0;
   }

   if (sstWinOpen == true)
      grSstWinClose();
   sstWinOpen = false;
}

void SetZTest(GFXSurface *, int enable)
{
   switch(enable)
   {
      case GFX_NO_ZTEST:
         grDepthBufferFunction(GR_CMP_ALWAYS);
         grDepthMask(FXFALSE);
         break;
      case GFX_ZTEST:
         grDepthBufferFunction(GR_CMP_LEQUAL);
         grDepthMask(FXFALSE);
         break;
      case GFX_ZTEST_AND_WRITE:
         grDepthBufferFunction(GR_CMP_LEQUAL);
         grDepthMask(FXTRUE);
         break;
      case GFX_ZALWAYSBEHIND:
      case GFX_ZWRITE:
         grDepthBufferFunction(GR_CMP_ALWAYS);
         grDepthMask(FXTRUE);
         break;
   }
}

void ClearZBuffer(GFXSurface *)
{
   // SLOW!  Use only in emergencies...
   grColorMask(FXFALSE, FXFALSE);
   grDepthMask(FXTRUE);
   grBufferClear(0, 0, GR_WDEPTHVALUE_FARTHEST);
   grColorMask(FXTRUE, FXFALSE);
   grDepthMask(FXTRUE);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


static GFXShadeSource curShadeSource;
static GFXHazeSource curHazeSource;
static GFXAlphaSource curAlphaSource;
static int curFillMode;
static Bool stateChanged = FALSE;
#define MAX_VERTS 20

GrVertex vertBase[8192];
Point3F texBase[8192];
GrVertex *verts = NULL;
Point3F *texCoords = NULL;
Point3F *curTex = NULL;
Point2F textureOffset, textureScale;


GrVertex *cur;

// New polygon interface functions...

static float vertex_snapper = (float) (1L << 19);
static float  constantHaze;
static float  constantAlpha;
static ColorF constantShade;
static ColorF constantFill;

typedef void (*fnVertexSetShade)(const GFXColorInfoF *);
typedef void (*fnVertexSetHaze)(const GFXColorInfoF *);
typedef void (*fnVertexSetAlpha)(const GFXColorInfoF *);

void fxSetShadeNone(const GFXColorInfoF *)
{
   cur->r = cur->g = cur->b = 255.0f;
}

void fxSetShadeConstant(const GFXColorInfoF *)
{
   cur->r = constantShade.red;
   cur->g = constantShade.green;
   cur->b = constantShade.blue;
}

void fxSetShadeVertex(const GFXColorInfoF *vert)
{
   cur->r = vert->color.red * 255.0f;
   cur->g = vert->color.green * 255.0f;
   cur->b = vert->color.blue * 255.0f;
}

fnVertexSetShade curVertexShader = NULL;

void fxSetHazeNone(const GFXColorInfoF *)
{
   cur->a = 0.0f;
}

void fxSetHazeConstant(const GFXColorInfoF *)
{
   // set to the constant haze value
   cur->a = constantHaze;
}

void fxSetHazeVertex(const GFXColorInfoF *ci)
{
   cur->a = ci->haze * 255.0f;
}

fnVertexSetHaze curVertexHazer = fxSetHazeNone;

void fxSetAlphaNone(const GFXColorInfoF *)
{
   // do nothing.
}

void fxSetAlphaConstant(const GFXColorInfoF *)
{
   // set to the constant haze value
//   cur->a = constantAlpha;
}

void fxSetAlphaVertex(const GFXColorInfoF *ci)
{
   cur->a = ci->alpha * 255.0f;
}

void fxSetAlphaVertHaze(const GFXColorInfoF *ci)
{
   cur->a = ci->haze * 255.0f;
}

fnVertexSetAlpha curVertexAlpher = fxSetAlphaNone;

void SetShadeSource(GFXSurface *, GFXShadeSource ss)
{
   switch(ss)
   {
      case GFX_SHADE_NONE:
         curVertexShader = fxSetShadeNone;
         break;
      case GFX_SHADE_CONSTANT:
         curVertexShader = fxSetShadeConstant;
         break;
      case GFX_SHADE_VERTEX:
         curVertexShader = fxSetShadeVertex;
         break;
   }
}

void SetHazeSource(GFXSurface *, GFXHazeSource hs)
{
   if(curHazeSource == hs)
      return;
   curHazeSource = hs;
   switch(hs)
   {
      case GFX_HAZE_NONE:
         grFogMode(GR_FOG_DISABLE);
         curVertexHazer = fxSetHazeNone;
         break;
      case GFX_HAZE_CONSTANT:
         grFogMode(GR_FOG_WITH_ITERATED_ALPHA);
         curVertexHazer = fxSetHazeConstant;
         break;
      case GFX_HAZE_VERTEX:
         grFogMode(GR_FOG_WITH_ITERATED_ALPHA);
         curVertexHazer = fxSetHazeVertex;
         break;
   }
}

void SetAlphaSource(GFXSurface * sfc, GFXAlphaSource as)
{
   if (as != GFX_ALPHA_CONSTANT)
      sfc->setConstantAlpha(1.0f);

   if(curAlphaSource == as)
      return;
   curAlphaSource = as;
   switch(as)
   {
      case GFX_ALPHA_NONE:
         curVertexAlpher = fxSetAlphaNone;
         grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
            GR_BLEND_ONE, GR_BLEND_ZERO);
         break;
      case GFX_ALPHA_CONSTANT:
         curVertexAlpher = fxSetAlphaConstant;
         grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, 
               GR_BLEND_ONE, GR_BLEND_ONE);
         grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FALSE);
         break;
      case GFX_ALPHA_VERTEX:
         curVertexAlpher = fxSetAlphaConstant;
         grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, 
               GR_BLEND_ONE, GR_BLEND_ONE);
         grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FALSE);
         break;
      case GFX_ALPHA_TEXTURE:
         curVertexAlpher = fxSetAlphaNone;
         grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, 
               GR_BLEND_ONE, GR_BLEND_ONE);
         grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
               GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_TEXTURE, FALSE);
         break;
      case GFX_ALPHA_FILL:
         curVertexAlpher = fxSetAlphaConstant;
         grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, 
               GR_BLEND_ONE, GR_BLEND_ONE);
         grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FALSE);
         break;
      case GFX_ALPHA_ADD:
         curVertexAlpher = fxSetAlphaConstant;
         grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE, 
                              GR_BLEND_ONE,       GR_BLEND_ONE);
         grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FALSE);
         break;
      case GFX_ALPHA_SUB:
         curVertexAlpher = fxSetAlphaConstant;
			curFillMode = -1;
         grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL_ALPHA,
                        GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
         grAlphaBlendFunction(GR_BLEND_ZERO, GR_BLEND_ONE_MINUS_SRC_COLOR, 
                              GR_BLEND_ONE,  GR_BLEND_ONE);
         grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, FALSE);
         break;
 
      //case GFX_ALPHA_TEXVERTEX:
		//	curVertexAlpher = fxSetAlphaVertHaze;
      //   grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, 
      //         GR_BLEND_ONE, GR_BLEND_ONE);
		//	grAlphaCombine(GR_COMBINE_FUNCTION_BLEND_OTHER,
		//						GR_COMBINE_FACTOR_LOCAL_ALPHA,
		//						GR_COMBINE_LOCAL_ITERATED,
		//						GR_COMBINE_OTHER_TEXTURE,
		//						FXFALSE);
			break;
 
      default:
         AssertFatal(0, "unknown blend mode");
   }
}

void InitVars()
{
   verts = vertBase;
   cur = verts;
   texCoords = texBase;
   curTex = texCoords;
   pavailedge = edges; // replace ClearEdgeLists()
   curVertexShader = fxSetShadeNone;
   curVertexHazer = fxSetHazeNone;
   curVertexAlpher = fxSetAlphaNone;
}   

void SetConstantHaze(GFXSurface *, float haze)
{
   // we haze with iterated alpha, so...
   constantHaze = 255.0f * haze;
}

void SetConstantShadeCF(GFXSurface *, const ColorF *col)
{
   constantShade.red   = col->red   * 255.0f;
   constantShade.green = col->green * 255.0f;
   constantShade.blue  = col->blue  * 255.0f;
}

void SetConstantShadeF(GFXSurface *, float intensity)
{
   constantShade.red = constantShade.green = constantShade.blue = intensity * 255.0f;
}

void SetConstantAlpha(GFXSurface *, float alpha)
{
   constantAlpha = 255.0f * alpha;
   UInt32 c = (UInt32(constantAlpha)      << 24) |
              (UInt32(constantFill.red)   << 16) |
              (UInt32(constantFill.green) << 8)  |
              (UInt32(constantFill.blue)  << 0);
   grConstantColorValue((int) c);
}

void SetFillColorCF(GFXSurface *, const ColorF *col)
{
   constantFill.red   = col->red   * 255.0f;
   constantFill.green = col->green * 255.0f;
   constantFill.blue  = col->blue  * 255.0f;
   
   UInt32 c = (UInt32(constantAlpha)      << 24) |
              (UInt32(constantFill.red)   << 16) |
              (UInt32(constantFill.green) << 8)  |
              (UInt32(constantFill.blue)  << 0);
   grConstantColorValue(c);
}

void SetFillColorI(GFXSurface *io_pSurface, Int32 index, DWORD paletteIndex)
{
   PALETTEENTRY color;
   GFXPalette *pal = io_pSurface->getPalette();
   int i;
   for(i = pal->numPalettes - 1; i > 0; i--)
   {
      if(pal->palette[i].paletteIndex == paletteIndex)
         break;
   }

   color = pal->palette[i].color[index];
   if (curAlphaSource == GFX_ALPHA_FILL) 
      constantAlpha = float(color.peFlags);

   constantFill.red   = color.peRed;
   constantFill.green = color.peGreen;
   constantFill.blue  = color.peBlue;
   UInt32 c = (UInt32(constantAlpha)      << 24) |
              (UInt32(constantFill.red)   << 16) |
              (UInt32(constantFill.green) << 8)  |
              (UInt32(constantFill.blue)  << 0);
   grConstantColorValue((int) c);
}

void SetHazeColorCF(GFXSurface *, const ColorF *col)
{
   UInt32 colr = (UInt32(col->red * 255.0f) << 16) | (UInt32(col->green * 255.0f) << 8) | UInt32(col->blue * 255.0f);
   hazeColor = colr;
   grFogColorValue(colr);
}

void SetHazeColorI(GFXSurface *io_pSurface, Int32 index)
{
   PALETTEENTRY color = io_pSurface->getPalette()->palette[0].color[index];
   UInt32 c = (UInt32(color.peRed) << 16) | (UInt32(color.peGreen) << 8) | UInt32(color.peBlue);
   hazeColor = c;
   grFogColorValue((int) c);
}

// grDrawTriangle(

static int testcolor = 0;
#define MAX_POLYS          3000

PolyStore fxSurfs[MAX_POLYS];
static PolyStore *curSurf;


void Draw3dBegin(GFXSurface *)
{
   AssertFatal(hCache && tCache, "Error - surface not locked");
   GFXMetrics.reset();

   //grBufferClear(0,0,GR_WDEPTHVALUE_FARTHEST);
   verts = vertBase;
   cur = verts;
   texCoords = texBase;
   curTex = texCoords;
   curSurf = fxSurfs;
   pavailedge = edges; // replace ClearEdgeLists()
#ifdef DEBUG
   GFXMetrics.textureSpaceUsed = 0;
   GFXMetrics.textureBytesDownloaded = 0;
   tCache->frameU++;
#endif
}

void Draw3dEnd(GFXSurface *surf)
{
   surf;
   return;
}

void QueuePoly(void)
{
   // add the polygon to the edge table, etc...
   if(DeltaCalcPolyW(&(curSurf->dwdx)))
   {
      curSurf->flags = 0;
      curSurf->spanList = 0;
      curSurf->tce = hCache->getCurrentEntry();
      curSurf->vertBase = verts;
      curSurf->tvertBase = texCoords;
      verts = cur;
      texCoords = curTex;
      curSurf->vertexCount = vertexCount;
      gfx_add_edges(curSurf); 
      curSurf++;      
   }
   else
   {
      cur = verts;
      curTex = texCoords;
   }
   vertexCount = 0;
}

//----------------------------------------------------------------------------

void EmitPolyTwoPass(GFXSurface* surf)
{
   int i;
   AssertFatal(hCache && tCache, "Error - surface not locked");
   
   GFXMetrics.incEmittedPolys();

   HandleCacheEntry *ent;
   ent = hCache->getCurrentEntry();
   grTexClampMode(0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
   grFogColorValue(0xFFFFFFFF);

   int valid = ent->flags & LightMapValid;

   if(!valid || !tCache->tmu[0].setCurrentTexture(ent->lMapInfo, 0xFFFFFFFF, NULL))
   {
      tCache->tmu[0].downloadLightMap(ent->lMapInfo, ent->lMap, ent->lmapSize.x, ent->lmapSize.y);
      ent->flags |= LightMapValid;
   }

   for(i = 0; i < vertexCount; i++)
   {
      verts[i].tmuvtx[0].sow = (texCoords[i].x * 
         ent->lmapScale.x + ent->lmapTrans.x) * verts[i].oow;
      verts[i].tmuvtx[0].tow = (texCoords[i].y * 
         ent->lmapScale.y + ent->lmapTrans.y) * verts[i].oow;
   }
   grAlphaBlendFunction(GR_BLEND_ONE,
                        GR_BLEND_ZERO,
                        GR_BLEND_ONE,
                        GR_BLEND_ZERO);

   grDrawPolygonVertexList(vertexCount, verts);

   if(!ent->clamp)
   {
      grTexClampMode(0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
   }
   GFXPalette *pal = surf->getPalette();

   if (ent->bmp != NULL)
   {
      GFXBitmap *ebmp = ent->bmp;

      // HACK for true color remapped bmps.
      if (ebmp->pPalette == NULL) {
         if (!tCache->tmu[0].setCurrentTexture(ebmp->getGlideCacheInfo(), ebmp->paletteIndex, pal))
            tCache->tmu[0].downloadBitmap(ebmp, pal, ebmp->getGlideCacheInfo());
      } else {
         if (!tCache->tmu[0].setCurrentTexture(ebmp->getGlideCacheInfo(), ebmp->paletteIndex, ebmp->pPalette))
            tCache->tmu[0].downloadBitmap(ebmp,  ebmp->pPalette, ebmp->getGlideCacheInfo());
      }
   }
   else
   {
      // Callback textures treated slightly differently...
      if (!tCache->tmu[0].setCurrentTexture(ent->tMapInfo, ent->tMapPaletteIndex, pal))
      {
         GFXBitmap bmp;
         bmp.height = bmp.stride = bmp.width = ent->size;
         bmp.pBits = tCache->downloadBuffer;
         bmp.pMipBits[0] = bmp.pBits;
         bmp.detailLevels = 1;
         bmp.bitDepth = 8;
         ent->cb(ent->handle, &bmp, 0);
         ent->tMapPaletteIndex = bmp.paletteIndex;
         tCache->tmu[0].downloadBitmap(&bmp, pal, ent->tMapInfo);
      }
   }

   // here's where we should scale this stuff.
   // oh well.
   // it'll work for the terrain and shapes as is.

   grAlphaTestFunction( GR_CMP_ALWAYS );
   grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE, 
                  GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
   guTexCombineFunction(GR_TMU0, GR_TEXTURECOMBINE_DECAL);
   grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                  GR_COMBINE_FACTOR_ONE,
                  GR_COMBINE_LOCAL_NONE,
                  GR_COMBINE_OTHER_CONSTANT,
                  FXFALSE);
   grAlphaBlendFunction(GR_BLEND_ZERO,
                        // source is lighting map.  Don't add this color, use for scale
                        GR_BLEND_SRC_COLOR,
                        // dest is base map, so scale by lighting map src
                        GR_BLEND_ONE,
                        // use source alpha in lighting map if it exists
                        GR_BLEND_ZERO);
                        // assume there is no alpha buffer

   if(curHazeSource == GFX_HAZE_NONE)
      grFogMode(GR_FOG_DISABLE);
   else
   {
      grFogMode(GR_FOG_WITH_ITERATED_ALPHA);
   }
   grFogColorValue(hazeColor);


   for(i = 0; i < vertexCount; i++)
   {
      verts[i].tmuvtx[0].sow = (texCoords[i].x * 
         ent->coordScale.x + ent->coordTrans.x) * verts[i].oow;
      verts[i].tmuvtx[0].tow = (texCoords[i].y * 
         ent->coordScale.y + ent->coordTrans.y) * verts[i].oow;
   }

   grDrawPolygonVertexList(vertexCount, verts);
   grFogColorValue(hazeColor);
   
    grAlphaBlendFunction(GR_BLEND_ONE,         
                      GR_BLEND_ZERO,    
                      GR_BLEND_ONE,  
                      GR_BLEND_ZERO);
   if(!ent->clamp)
   {
      grTexClampMode(0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
   }

#ifdef DEBUG
   float width = surf->getWidth();
   GrVertex *prev = verts + vertexCount - 1;

   for(i = 0; i < vertexCount; i++)
   {
      GrVertex *cur = verts + i;
      prev->x -= float(3 << 18);
      prev->y -= float(3 << 18);
      cur->x -= float(3 << 18);
      cur->y -= float(3 << 18);
      if(cur->y != prev->y)
         GFXMetrics.numPixelsRendered += 2 * (width - (cur->x + prev->x) * .5) * (prev->y - cur->y);

      prev->x += float(3 << 18);
      prev->y += float(3 << 18);
      cur->x += float(3 << 18);
      cur->y += float(3 << 18);

      prev = cur;
   }
#endif

   cur = verts;
   curTex = texCoords;
   vertexCount = 0;
}

Point2F lightMapScale;
Point2F lightMapOffset;

void hackSetLightScaleOffset(Point2F scale, Point2F offset)
{
   lightMapScale = scale;
   lightMapOffset = offset;
   lightMapScale *= 256;
   lightMapOffset *= 256;
}

bool hackTextureValid(GFXBitmap::CacheInfo *info, DWORD paletteIndex, GFXPalette *pal)
{
   newTexture = NULL;
   if(!tCache)
      return false;
   textureOffset.set(0, 0);
   textureScale.set(256, 256);

   return tCache->tmu[1].setCurrentTexture(*((GFXBitmap::GlideCacheInfo *)info), paletteIndex, pal);
}

void hackSetLightmap(GFXBitmap *lm)
{
   if(!tCache->tmu[0].setCurrentTexture(lm->getGlideCacheInfo(), 0xFFFFFFFF, NULL))
      tCache->tmu[0].downloadBitmap(lm, NULL, lm->getGlideCacheInfo());
}

void hackDownloadLightmap(GFXBitmap *lm)
{
   tCache->tmu[0].flushTexture(lm, NULL, true);
   if(!tCache->tmu[0].setCurrentTexture(lm->getGlideCacheInfo(), 0xFFFFFFFF, NULL))
      tCache->tmu[0].downloadBitmap(lm, NULL, lm->getGlideCacheInfo());
}

void hackReloadBitmap(GFXBitmap *lm, GFXPalette *pal)
{
   tCache->tmu[1].flushTexture(lm, pal, true);
}

static UInt16 remapTable[256];

static GFXBitmap *convertTo16Bit(GFXBitmap *lightMap, GFXPalette *pal)
{
   static GFXBitmap tempBmp;
   GFXPalette::MultiPalette *mp = pal->findMultiPalette(lightMap->paletteIndex);
   
   for(int j = 0; j < 256; j++)
      remapTable[j] = (int(mp->color[j].peRed >> 3) << 11) |
                      (int(mp->color[j].peGreen >> 2) << 5) |
                      int(mp->color[j].peBlue >> 3);
   
   tempBmp.height = lightMap->height;
   tempBmp.width = lightMap->width;
   tempBmp.stride = lightMap->stride * 2;
   tempBmp.pBits = tCache->downloadBuffer;
   
   BYTE *ptr = tCache->downloadBuffer;
   int width = lightMap->width;
   int height = lightMap->height;
   for(int i = 0; i < lightMap->detailLevels; i++)
   {
      tempBmp.pMipBits[i] = ptr;
      
      UInt16 *dest = (UInt16 *) ptr;
      const BYTE *source = lightMap->getDetailLevel(i);
      ptr += (width * height * 2);
      
      while(dest != (UInt16 *) ptr)
         *dest++ = remapTable[*source++];
      
      width >>= 1;
      height >>= 1;
   }
   tempBmp.detailLevels = lightMap->detailLevels;
   tempBmp.bitDepth = 16;
   tempBmp.paletteIndex = 0xFFFFFFFF;
   return &tempBmp;
}

void hackDownloadBitmap(GFXBitmap *bmp, GFXPalette *pal, GFXBitmap::CacheInfo *info, bool trans)
{
   if(tCache)
   {
      // tmu 1 - if trans is set, it's the environment map
      // so convert it to 16-bit first (stupid no duplicate tex table)
      GFXBitmap *dl = bmp;
      if(trans)
         dl = convertTo16Bit(bmp, pal);
//      if(trans)
//         tCache->tmu[1].downloadTranslucentBitmap(bmp, pal, *((GFXBitmap::GlideCacheInfo *)info));
//      else
      tCache->tmu[1].downloadBitmap(dl, pal, *((GFXBitmap::GlideCacheInfo *)info));
   }
}

void HackEmitPoly(GFXSurface *surf)
{
   GFXMetrics.incEmittedPolys();
   grDrawPolygonVertexList(vertexCount, verts);
   GFXMetrics.incRenderedPolys();

   cur = verts;
   vertexCount = 0;
}

void HackAddVertex(GFXSurface *, const Point3F *in_vert, const Point2F *in_tvert, const GFXColorInfoF *in_color, DWORD /*in_key*/)
{
   cur->x = in_vert->x + float(3<<18);
   cur->y = in_vert->y + float(3<<18);
   cur->oow = in_vert->z;
   
   cur->tmuvtx[1].sow = (in_tvert->x * 256) * cur->oow;
   cur->tmuvtx[1].tow = (in_tvert->y * 256) * cur->oow;
   
   curVertexShader(in_color);
   curVertexHazer(in_color);
   curVertexAlpher(in_color);
      
   cur->tmuvtx[0].sow = in_color->color.red * 256 * cur->oow;
   cur->tmuvtx[0].tow = in_color->color.green * 256 * cur->oow;
   
   vertexCount++;
   cur++;
   curTex++;
}

void hackSetAlpha(bool first, float r, float g, float b, float a)
{
   if(first)
      grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);
   else
      grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO);
   
   UInt32 c = (UInt32(a * 255.0) << 24) |
              (UInt32(r * a * 255.0) << 16) |
              (UInt32(g * a * 255.0) << 8)  |
              (UInt32(b * a * 255.0) << 0);
   grConstantColorValue(c);

   grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_LOCAL,
                  GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_NONE, false);
   grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                  GR_COMBINE_FACTOR_LOCAL,
                  GR_COMBINE_LOCAL_CONSTANT,
                  GR_COMBINE_OTHER_TEXTURE,
                  false );
}

void hackUndoAlphaMode()
{
   curAlphaSource = (GFXAlphaSource)-1;
   grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ZERO, GR_BLEND_ZERO);
   grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
}

void hackMode(GFXBitmap *lightMap, bool additive, GFXPalette *pal)
{
   if(!tCache)
      return;
   grTexClampMode(1, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
   grTexClampMode(0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

   // HACK for trucolor remaps
   //
   GFXPalette* pUsePal = pal;
   if (lightMap->pPalette != NULL)
      pUsePal = lightMap->pPalette;

   if(!tCache->tmu[0].setCurrentTexture(lightMap->getGlideCacheInfo(), lightMap->paletteIndex, pUsePal ))
   {
      // tmu 0 - if it's 8-bit, it's translucent (water map)
      GFXBitmap *dl = lightMap;
      if(lightMap->bitDepth == 8)
         tCache->tmu[0].downloadTranslucentBitmap(dl, pal, lightMap->getGlideCacheInfo());
      else
         tCache->tmu[0].downloadBitmap(dl, NULL, lightMap->getGlideCacheInfo());
   }
   curFillMode = -1;

   glide_table.fnAddVertexVTC = HackAddVertex;
   glide_table.fnEmitPoly = HackEmitPoly;
   grFogColorValue(hazeColor);
   grHints(GR_HINT_STWHINT, GR_STWHINT_ST_DIFF_TMU1);

   grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
   if(additive)
   {
      if(glideHackVal == 0)
         grTexCombine(  0, GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_LOCAL_ALPHA,
            GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE,
            FXFALSE, FXFALSE ); 
      else if(glideHackVal == 1)
         grTexCombine(  0, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
            GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE,
            FXFALSE, FXFALSE ); 
      else if(glideHackVal == 2)
         grTexCombine(  0, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
            GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE,
            FXFALSE, FXFALSE ); 
   }
   else
      grTexCombine(  0, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
         GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
         FXFALSE, FXFALSE ); 
   
   grTexCombine(  1, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
      FXFALSE, FXFALSE ); 
}

void EmitPoly(GFXSurface* surf)
{
   GFXBitmap *tx = (GFXBitmap *) newTexture;
   GFXPalette *pal = surf->getPalette();
   if(newTexture)
   {
      if (tx->pPalette != NULL)
         pal = tx->pPalette;

      if(!tCache->tmu[0].setCurrentTexture(tx->getGlideCacheInfo(), tx->paletteIndex, pal))
      {
         if(curAlphaSource == GFX_ALPHA_TEXTURE /* || curAlphaSource == GFX_ALPHA_TEXVERTEX*/)
            tCache->tmu[0].downloadTranslucentBitmap(tx, pal, tx->getGlideCacheInfo());
         else
            tCache->tmu[0].downloadBitmap(tx, pal, tx->getGlideCacheInfo());
      }
      newTexture = NULL;
   }

   grDrawPolygonVertexList(vertexCount, verts);
   GFXMetrics.incRenderedPolys();

#ifdef DEBUG
   float width = surf->getWidth();
   GrVertex *prev = verts + vertexCount - 1;
   int i;
   for(i = 0; i < vertexCount; i++)
   {
      GrVertex *cur = verts + i;
      prev->x -= float(3 << 18);
      prev->y -= float(3 << 18);
      cur->x -= float(3 << 18);
      cur->y -= float(3 << 18);
      if(cur->y != prev->y)
         GFXMetrics.numPixelsRendered += (width - (cur->x + prev->x) * .5) * (prev->y - cur->y);

      prev->x += float(3 << 18);
      prev->y += float(3 << 18);
      cur->x += float(3 << 18);
      cur->y += float(3 << 18);

      prev = cur;
   }
#endif

   cur = verts;
   curTex = texCoords;
   vertexCount = 0;
}

void AddVertex2pDualTMU(GFXSurface *, const Point3F *in_vert, const Point2F *in_tvert, const GFXColorInfoF *in_color, DWORD /*in_key*/)
{
   HandleCacheEntry *curEntry = hCache->getCurrentEntry();
   
   cur->x = in_vert->x + float(3 << 18);
   cur->y = in_vert->y + float(3 << 18);
   cur->oow = in_vert->z;

   cur->tmuvtx[1].sow = (in_tvert->x * curEntry->lmapScale.x + curEntry->lmapTrans.x) * in_vert->z;
   cur->tmuvtx[1].tow = (in_tvert->y * curEntry->lmapScale.y + curEntry->lmapTrans.y) * in_vert->z;
   cur->tmuvtx[0].sow = (in_tvert->x * curEntry->coordScale.x + curEntry->coordTrans.x) * in_vert->z;
   cur->tmuvtx[0].tow = (in_tvert->y * curEntry->coordScale.y + curEntry->coordTrans.y) * in_vert->z;

   curVertexShader(in_color);
   curVertexHazer(in_color);
   curVertexAlpher(in_color);
   vertexCount++;
   cur++;
}

void EmitPoly2pDualTMU(GFXSurface *surf)
{
   GFXMetrics.incEmittedPolys();

   HandleCacheEntry *ent;
   ent = hCache->getCurrentEntry();

   if(!ent->clamp)
      grTexClampMode(0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

   int valid = ent->flags & LightMapValid;
   if(!valid || !tCache->tmu[1].setCurrentTexture(ent->lMapInfo, 0xFFFFFFFF, NULL))
   {
      tCache->tmu[1].downloadLightMap(ent->lMapInfo, ent->lMap, ent->lmapSize.x, ent->lmapSize.y);
      ent->flags |= LightMapValid;
   }
   GFXPalette *pal = surf->getPalette();

   if (ent->bmp != NULL)
   {
      GFXBitmap *ebmp = ent->bmp;
      if (ebmp->pPalette != NULL) {
         if (!tCache->tmu[0].setCurrentTexture(ebmp->getGlideCacheInfo(), ebmp->paletteIndex, ebmp->pPalette))
            tCache->tmu[0].downloadBitmap(ebmp, ebmp->pPalette, ebmp->getGlideCacheInfo());
      } else {
         if (!tCache->tmu[0].setCurrentTexture(ebmp->getGlideCacheInfo(), ebmp->paletteIndex, pal))
            tCache->tmu[0].downloadBitmap(ebmp, pal, ebmp->getGlideCacheInfo());
      }
   }
   else
   {
      // Callback textures treated slightly differently...
      if (!tCache->tmu[0].setCurrentTexture(ent->tMapInfo, ent->tMapPaletteIndex, pal))
      {
         GFXBitmap bmp;
         bmp.height = bmp.stride = bmp.width = ent->size;
         bmp.pBits = tCache->downloadBuffer;
         bmp.pMipBits[0] = bmp.pBits;
         bmp.detailLevels = 1;
         bmp.bitDepth = 8;
         ent->cb(ent->handle, &bmp, 0);
         ent->tMapPaletteIndex = bmp.paletteIndex;
         tCache->tmu[0].downloadBitmap(&bmp, pal, ent->tMapInfo);
      }
   }
   grDrawPolygonVertexList(vertexCount, verts);
   
   if(!ent->clamp)
      grTexClampMode(0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);

   GFXMetrics.incRenderedPolys();

   cur = verts;
   vertexCount = 0;
}

void AddVertexV(GFXSurface *, const Point3F *in_vert, DWORD)
{
   cur->x = in_vert->x + float(3<<18);
   cur->y = in_vert->y + float(3<<18);
   cur->oow = in_vert->z;

   curVertexShader(NULL);
   curVertexHazer(NULL);
   curVertexAlpher(NULL);
   vertexCount++;
   cur++;
}

void AddVertexVT(GFXSurface *, const Point3F *in_vert, const Point2F *in_tvert, DWORD)
{
   cur->x = float(int(in_vert->x * 16.0f)) / 16.0f;
   cur->y = float(int(in_vert->y * 16.0f)) / 16.0f;
// cur->x = (in_vert->x + vertex_snapper); cur->x -= vertex_snapper;
// cur->y = (in_vert->y + vertex_snapper); cur->y -= vertex_snapper;
   //cur->oow = 1 / (in_vert->z / in_vert->w);
   //cur->ooz = 65535.0f * cur->oow;
   
   cur->oow = in_vert->z;
   if(in_tvert)
   {
      cur->tmuvtx[0].sow = (in_tvert->x * 255.0f + 0.5f) * cur->oow;
      cur->tmuvtx[0].tow = (in_tvert->y * 255.0f + 0.5f) * cur->oow;
   }
   curVertexShader(NULL);
   curVertexHazer(NULL);
   curVertexAlpher(NULL);
   vertexCount++;
   cur++;
}

void AddVertexVC(GFXSurface *, const Point3F *in_vert, const GFXColorInfoF *in_color, DWORD)
{
   cur->x = float(int(in_vert->x * 16.0f)) / 16.0f;
   cur->y = float(int(in_vert->y * 16.0f)) / 16.0f;
// cur->x = (in_vert->x + vertex_snapper); cur->x -= vertex_snapper;
// cur->y = (in_vert->y + vertex_snapper); cur->y -= vertex_snapper;
   cur->oow = in_vert->z;
   //cur->oow = 1 / (in_vert->z / in_vert->w);
   cur->ooz = 65535.0f * cur->oow;
   
   curVertexShader(in_color);
   curVertexHazer(in_color);
   curVertexAlpher(in_color);
   
   vertexCount++;
   cur++;
}

void AddVertexVTC_NoTex(GFXSurface *, const Point3F *in_vert, const Point2F * /*in_tvert*/, const GFXColorInfoF *in_color, DWORD /*in_key*/)
{
   cur->x = in_vert->x + float(3<<18);
   cur->y = in_vert->y + float(3<<18);

   cur->oow = in_vert->z;
   curVertexShader(in_color);
   curVertexHazer(in_color);
   curVertexAlpher(in_color);
      
   vertexCount++;
   cur++;
   curTex++;
}   

void AddVertexVTC_Textured(GFXSurface *, const Point3F *in_vert, const Point2F *in_tvert, const GFXColorInfoF *in_color, DWORD /*in_key*/)
{
   cur->x = in_vert->x + float(3<<18);
   cur->y = in_vert->y + float(3<<18);
   cur->oow = in_vert->z;
   
   cur->tmuvtx[0].sow = (in_tvert->x * textureScale.x + textureOffset.x) * cur->oow;
   cur->tmuvtx[0].tow = (in_tvert->y * textureScale.y + textureOffset.y) * cur->oow;
   
   curVertexShader(in_color);
   curVertexHazer(in_color);
   curVertexAlpher(in_color);
      
   vertexCount++;
   cur++;
   curTex++;
}

void AddVertexVTC_TwoPass(GFXSurface *, const Point3F *in_vert, const Point2F *in_tvert, const GFXColorInfoF *in_color, DWORD /*in_key*/)
{
   cur->x = in_vert->x + float(3<<18);
   cur->y = in_vert->y + float(3<<18);

   cur->oow = in_vert->z;
   
   curTex->z = in_vert->z;
   curTex->x = in_tvert->x;
   curTex->y = in_tvert->y;
   
   curVertexShader(in_color);
   curVertexHazer(in_color);
   curVertexAlpher(in_color);
      
   vertexCount++;
   cur++;
   curTex++;
}

//----------------------------------------------------------------------------

void SetTexturePerspective(GFXSurface *, Bool )
{
   // This is a NOP on the 3Dfx... all texturing is perspective
}

void SetFillMode(GFXSurface *, GFXFillMode fm)
{
   //fm = GFX_FILL_CONSTANT;
   //grConstantColorValue(0xFFFFFFFF);
   if(curFillMode == fm)
      return;
   curFillMode = fm;
   switch(fm)
   {
      case GFX_FILL_CONSTANT:
         newTexture = NULL;
         glide_table.fnAddVertexVTC = AddVertexVTC_NoTex;
         glide_table.fnEmitPoly = EmitPoly;
         grColorCombine(GR_COMBINE_FUNCTION_BLEND_OTHER, GR_COMBINE_FACTOR_LOCAL,
            GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_CONSTANT, FALSE);
         break;
      case GFX_FILL_TEXTURE:
         glide_table.fnAddVertexVTC = AddVertexVTC_Textured;
         glide_table.fnEmitPoly = EmitPoly;
         grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
            GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
         guTexCombineFunction(0, GR_TEXTURECOMBINE_DECAL);
         break;
      case GFX_FILL_TWOPASS:
         glide_table.fnAddVertexVTC = AddVertexVTC_TwoPass;
         glide_table.fnEmitPoly = EmitPolyTwoPass;
         grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
            GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
         guTexCombineFunction(0, GR_TEXTURECOMBINE_DECAL);
         break;
   }
}

void SetFillModeDualTMU(GFXSurface *, GFXFillMode fm)
{
   if(curFillMode == fm)
      return;
   curFillMode = fm;
   glide_table.fnEmitPoly = EmitPoly;
   grHints(GR_HINT_STWHINT, 0);
   switch(fm)
   {
      case GFX_FILL_CONSTANT:
         newTexture = NULL;
         glide_table.fnAddVertexVTC = AddVertexVTC_NoTex;
         grColorCombine(GR_COMBINE_FUNCTION_BLEND_OTHER, GR_COMBINE_FACTOR_LOCAL,
            GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_CONSTANT, FALSE);
         break;
      case GFX_FILL_TEXTURE:
         glide_table.fnAddVertexVTC = AddVertexVTC_Textured;
         grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
            GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
         grTexCombine( 0, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
            GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
            FXFALSE, FXFALSE ); 
         break;
      case GFX_FILL_TWOPASS:
         glide_table.fnEmitPoly = EmitPoly2pDualTMU;
         glide_table.fnAddVertexVTC = AddVertex2pDualTMU;
         grFogColorValue(hazeColor);
         grHints(GR_HINT_STWHINT, GR_STWHINT_ST_DIFF_TMU1);

         grTexClampMode(1, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
         grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
            GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
         grTexCombine(  0, GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
            GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
            FXFALSE, FXFALSE ); 
         grTexCombine(  1, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
            GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
            FXFALSE, FXFALSE ); 
         break;
   }
}

void SetTransparency(GFXSurface* /*io_pSurface*/, Bool enable)
{
//   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   if(enable)
   {
//      if (pSurface->isVoodoo2 == false)
         grTexFilterMode(0, sg_nonblinearFilterMode, sg_nonblinearFilterMode);
      grChromakeyValue(0x00000000);
      grChromakeyMode(GR_CHROMAKEY_ENABLE);
   }
   else
   {
      grChromakeyMode(GR_CHROMAKEY_DISABLE);
//      if (pSurface->isVoodoo2 == false)
         grTexFilterMode(0, sg_bilinearFilterMode, sg_bilinearFilterMode);
   }

}

void SetTextureMap(GFXSurface *, const GFXBitmap *texture)
{
   AssertFatal(hCache && tCache, "Error - surface not locked");
   int longside = max(texture->getWidth(), texture->getHeight());

   float scale = 256.0f / float(longside);

   textureOffset.set(0, 0);
   textureScale.set(scale * texture->getWidth(), scale * texture->getHeight());

   newTexture = texture;
}

static void setChromaKey(Bool chroma)
{
   static Bool chromaState = FALSE;
   if(chroma)
   {
      grTexFilterMode(0, sg_nonblinearFilterMode, sg_nonblinearFilterMode);
   }
   if(chroma == chromaState)
      return;
   if(chroma)
   {
      grChromakeyValue(0x00FFFFFF);
      grChromakeyMode(GR_CHROMAKEY_ENABLE);
   }
   else
   {  
      grChromakeyMode(GR_CHROMAKEY_DISABLE);
   }
   chromaState = chroma;
}

int numPendingFlips()
{
   return (grSstStatus() >> 28) & 0x7;
}

static Point3F alphaPolyPts[4];
void Surface::flip()
{
   if(alphaBlend != 0)
   {
      alphaPolyPts[0].set(0,0,1);
      alphaPolyPts[1].set(getWidth(),0,1);
      alphaPolyPts[2].set(getWidth(),getHeight(),1);
      alphaPolyPts[3].set(0,getHeight(),1);

      setAlphaSource(GFX_ALPHA_CONSTANT);
      setHazeSource(GFX_HAZE_NONE);
      setShadeSource(GFX_SHADE_NONE);
      setFillMode(GFX_FILL_CONSTANT);
      setConstantAlpha(alphaBlend);
      setFillColor(&alphaColor); 
      addVertex(alphaPolyPts);
      addVertex(alphaPolyPts+1);
      addVertex(alphaPolyPts+2);
      addVertex(alphaPolyPts+3);
      emitPoly();
   }

   while(numPendingFlips() > 1)
   {
      Sleep(10); // wait 10 ms.
   }
   grSstIdle();

   if (GFXPrefs.waitForVSync == true)
      grBufferSwap(1);
   else
      grBufferSwap(0);
}

void ClearScreen(GFXSurface *io_pSurface, DWORD in_color)
{
   PALETTEENTRY color;
   int c;
   if(io_pSurface->getPalette())
   {
      
      color = (io_pSurface->getPalette())->palette[0].color[in_color];
      c = (UInt32(color.peRed) << 16) | (UInt32(color.peGreen) << 8) | UInt32(color.peBlue);
   }
   else
      c = 0;
   grBufferClear(c, 0, 0);
}

Bool SetTextureHandle(GFXSurface *, GFXTextureHandle tex)
{
   bool ret = hCache->setTextureHandle(tex);
#ifdef DEBUG
   if(!ret)
      return ret;

   HandleCacheEntry *ent = hCache->getCurrentEntry();
   if(ent->cb == 0)
   {
      const GFXBitmap *tex = ent->bmp;
      AssertFatal(tex->getWidth() <= 256 && tex->getWidth() > 0 && tex->getHeight() <= 256
               && tex->getHeight() > 0, "Invalid texture size");
   }

#endif
   return ret;
}

void validateLightMapSize(Point2I *size)
{
   while((size->x << 3) < size->y)
      size->x <<= 1;
   while((size->y << 3) < size->x)
      size->y <<= 1;
}

void 
RegisterTexture(GFXSurface*      /*io_pSfc*/,
                GFXTextureHandle th,
                int              sizeX,
                int              sizeY,
                int              offsetX,
                int              offsetY,
                int              lightScale,
                GFXLightMap*     map,
                const GFXBitmap* tex,
                int              /*mipLevel*/)
{
   AssertFatal(tex->getWidth() <= 256 && tex->getWidth() > 0 && tex->getHeight() <= 256
               && tex->getHeight() > 0, "Invalid texture size");

   AssertFatal(hCache && tCache, "Error - surface not locked");
   HandleCacheEntry *ent = hCache->getFreeEntry(); // should be get free entry...
   
   ent->handle = th;
   ent->key    = (DWORD) tex; // key used for cacheing.
   ent->cb     = 0;
   ent->size   = 0;
   ent->bmp    = (GFXBitmap *) tex;
   ent->lMap   = map;
   ent->lMapInfo.driverSeqCount = 0xFFFFFFFF;
   
   ent->flags  = 0;

   ent->lmapSize.set(NearestPow(map->size.x),
                     NearestPow(map->size.y));
   validateLightMapSize(&ent->lmapSize);

   int longside = max(tex->getWidth(), tex->getHeight());

   ent->coordScale.set(sizeX * 256.0f / float(longside),
                       sizeY * 256.0f / float(longside));

   ent->coordTrans.set(offsetX * 256.0f / float(longside),
                       offsetY * 256.0f / float(longside));

   float scx = float(sizeX) / float(1 << lightScale);
   float scy = float(sizeY) / float(1 << lightScale);

   longside = max(ent->lmapSize.x, ent->lmapSize.y); 
   float baseoff = 128.0f / float(longside);

   ent->lmapScale.set(256.0f * scx / float(longside),
                      256.0f * scy / float(longside));

   ent->lmapTrans.set(baseoff + ent->lmapScale.x * map->offset.x / float(sizeX),
                      baseoff + ent->lmapScale.y * map->offset.y / float(sizeY));

   if(sizeX == tex->getWidth() && sizeY == tex->getHeight() &&
      offsetX == 0       && offsetY == 0         &&
      map->offset.x == 0 && map->offset.y == 0   &&
      ((map->size.x - 1) << lightScale) == sizeX &&
      ((map->size.y - 1) << lightScale) == sizeY)
   {
      ent->clamp = true;
   } else {
      ent->clamp = false;
   }

   hCache->HashInsert(ent);
}

void 
RegisterTextureCB(GFXSurface*      /*io_pSfc*/,
                  GFXTextureHandle tex,
                  GFXCacheCallback cb,
                  int              csizeX, 
                  int              /*csizeY*/,
                  int              /*lightScale*/,
                  GFXLightMap*     map)
{
   AssertFatal(hCache && tCache, "Error - surface not locked");
   HandleCacheEntry *ent = hCache->getFreeEntry(); // should be get free entry...
   
   ent->handle = tex;
   ent->key    = hCache->GetNextCbKey();
   ent->cb     = cb;
   ent->size   = csizeX;
   ent->lMap   = map;
   ent->flags  = 0;
   ent->lMapInfo.driverSeqCount = 0xFFFFFFFF;
   ent->tMapInfo.driverSeqCount = 0xFFFFFFFF;

   ent->coordScale.set(256.0, 256.0);
   ent->coordTrans.set(0, 0);

   ent->lmapSize.set(NearestPow(map->size.x), NearestPow(map->size.y));

   float spow   = float(NearestPow(map->size.x));
   float scale  = (map->size.x - 1) / spow;
   float offset = 128.0f / spow; // half a texel offset;

   scale *= 256.0f;// - 2 * offset;

   ent->lmapScale.set(scale,  scale);
   ent->lmapTrans.set(offset, offset);
   ent->clamp = true;

   hCache->HashInsert(ent);
}

void 
HandleSetLightMap(GFXSurface*  /*io_pSurface*/,
                  int          /*io_lightScale*/,
                  GFXLightMap* io_pLMap)
{
   if ((hCache->getCurrentEntry())->lMap != NULL)
      gfxLightMapCache.release((hCache->getCurrentEntry())->lMap);
   
   (hCache->getCurrentEntry())->lMap = io_pLMap;
   (hCache->getCurrentEntry())->flags &= ~LightMapValid;
}

GFXLightMap * HandleGetLightMap(GFXSurface *)
{
   return (hCache->getCurrentEntry())->lMap;
}

void HandleSetTextureMap(GFXSurface *, const GFXBitmap *texture)
{
   HandleCacheEntry *ent = hCache->getCurrentEntry();
   AssertFatal(ent->cb == NULL, "cannot set the texture on a callback handle");

   ent->bmp = (GFXBitmap *) texture;
   ent->key = DWORD(texture);
}

void HandleSetMipLevel(GFXSurface* /*io_pSurface*/,
                               int         /*io_mipLevel*/)
{
   //fxTextureCache.setMipLevel(tex, mipLevel);
}
 
void HandleSetTextureSize(GFXSurface *, int newSize)
{
   HandleCacheEntry *ent = hCache->getCurrentEntry();

   if(ent->cb && ent->size != newSize)
   {
      ent->key = hCache->GetNextCbKey();
      ent->size = newSize;
   }
}

void FlushTextureCache(GFXSurface *)
{
   tCache->flush();
   hCache->flush();
}
 
void SetTextureWrap(GFXSurface *, Bool wrap)
{
   if(wrap)
      grTexClampMode(0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
   else
      grTexClampMode(0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// other gfx functions

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void 
DrawPoint(GFXSurface*    io_pSurface,
          const Point2I* in_at,
          float          in_w,
          DWORD          in_color)
{

   RectI *clipRect = io_pSurface->getClipRect();
   if ( (in_at->x < clipRect->upperL.x) 
       ||(in_at->x > clipRect->lowerR.x)
       ||(in_at->y < clipRect->upperL.y)
       ||(in_at->y > clipRect->lowerR.y) )
          return;

   curFillMode = -1;
   SetFillColorI(io_pSurface, in_color, 0xFFFFFFFF);
   grColorCombine(GR_COMBINE_FUNCTION_BLEND_OTHER, GR_COMBINE_FACTOR_ONE,
                  GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FALSE);

   cur->x = float(in_at->x) + 0.5;
   cur->y = float(in_at->y) + 0.5;
   cur->oow = in_w;

   grDrawPoint(cur);
}

void 
DrawLine2d(GFXSurface*    io_pSurface,
           const Point2I* in_st,
           const Point2I* in_en,
           DWORD          in_color)
{
   Point2I start = *in_st, end = *in_en;

   if(!rectClip(&start, &end, io_pSurface->getClipRect() ))
      return;

   curFillMode = -1;
   SetFillColorI(io_pSurface, in_color, 0xFFFFFFFF);
   grColorCombine(GR_COMBINE_FUNCTION_BLEND_OTHER, GR_COMBINE_FACTOR_ONE,
                  GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FALSE);

   cur = verts;

   cur->x = start.x + .5f;
   cur->y = start.y + .5f;
   cur->oow = 1.0f;
   cur->ooz = 1.0f;
   cur->a = constantAlpha;
   (cur+1)->x = end.x + .5f;
   (cur+1)->y = end.y + .5f;
   (cur+1)->oow = 1.0f;
   (cur+1)->ooz = 1.0f;
   (cur+1)->a = constantAlpha;
   grDrawLine(cur, cur+1);

   // Have to deadjust the points for drawing begin/end point pair...
   //
//   cur->x     -= .5f;
//   cur->y     -= .5f;
//   (cur+1)->x -= .5f;
//    (cur+1)->y -= .5f;
   grDrawPoint(cur);
   grDrawPoint(cur+1);
}

void 
DrawRect2d(GFXSurface*  io_pSurface,
           const RectI* in_rect,
           DWORD        in_color)
{
   Point2I a;

   a(in_rect->lowerR.x, in_rect->upperL.y);
   DrawLine2d(io_pSurface, &in_rect->upperL, &a, in_color);       //top
   DrawLine2d(io_pSurface, &a, &in_rect->lowerR, in_color);       //right

   a(in_rect->upperL.x, in_rect->lowerR.y);
   DrawLine2d(io_pSurface, &in_rect->upperL, &a, in_color);       //left
   DrawLine2d(io_pSurface, &a, &in_rect->lowerR, in_color);       //bottom
}

void 
DrawRect_f(GFXSurface*  io_pSurface,
           const RectI* in_rect,
           float        in_w,
           DWORD        in_color)
{
   RectI clippedRect = in_rect;
   if(!rectClip(&clippedRect, io_pSurface->getClipRect()))
      return;

   curFillMode = -1;
   SetFillColorI(io_pSurface, in_color, 0xFFFFFFFF);
   grColorCombine(GR_COMBINE_FUNCTION_BLEND_OTHER, GR_COMBINE_FACTOR_ONE,
                  GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FALSE);
   cur[0].x = clippedRect.upperL.x + .5f;
   cur[0].y = clippedRect.upperL.y + .5f;
   cur[0].oow = in_w;
   cur[0].a = constantAlpha;
   cur[1].x = clippedRect.lowerR.x + 1.5f;
   cur[1].y = clippedRect.upperL.y + .5f;
   cur[1].oow = in_w;
   cur[1].a = constantAlpha;
   cur[2].x = clippedRect.lowerR.x + 1.5f;
   cur[2].y = clippedRect.lowerR.y + 1.5f;
   cur[2].oow = in_w;
   cur[2].a = constantAlpha;
   cur[3].x = clippedRect.upperL.x + .5f;
   cur[3].y = clippedRect.lowerR.y + 1.5f;
   cur[3].oow = in_w;
   cur[3].a = constantAlpha;

   grDrawTriangle(cur, cur + 1, cur + 2);
   grDrawTriangle(cur, cur + 2, cur + 3);
}

void bitmapDraw(GFXSurface*      io_pSfc,
                const GFXBitmap* in_pBM)
{
   int wpow = NearestPow(in_pBM->getWidth());
   int hpow = NearestPow(in_pBM->getHeight());

   int longside = max(wpow, hpow);
   int i, j;

   float offset = 0.0f;
   float scale = 256.0f / float(longside);

   textureOffset.set(offset, offset);
   textureScale.set(scale,scale);

   GFXBitmap *pbmp = (GFXBitmap *) in_pBM;
   GFXPalette *palette = io_pSfc->getPalette();
   if (pbmp->pPalette != NULL)
      palette = pbmp->pPalette;

   if(!tCache->tmu[0].setCurrentTexture(pbmp->getGlideCacheInfo(), pbmp->paletteIndex, palette))
   {
      allowSubSampling = false;
      if(wpow == in_pBM->getWidth() && hpow == in_pBM->getHeight()) {
         if ((in_pBM->attribute & BMA_TRANSLUCENT) == 0)
            tCache->tmu[0].downloadBitmap(pbmp, palette, pbmp->getGlideCacheInfo());
         else
            tCache->tmu[0].downloadTranslucentBitmap(pbmp, palette, pbmp->getGlideCacheInfo());
      } else {
         GFXBitmap bmp;
         bmp.height = hpow;
         bmp.stride = bmp.width = wpow;
         bmp.pBits = tCache->downloadBuffer;
         bmp.pMipBits[0] = bmp.pBits;
         bmp.detailLevels = 1;
         bmp.bitDepth = 8;
         bmp.paletteIndex = in_pBM->paletteIndex;
         for(j = 0; j < in_pBM->getHeight(); j++)
         {
            BYTE *src = in_pBM->getAddress(0, j);
            BYTE *dst = bmp.getAddress(0, j);
            BYTE *dstLast = dst + in_pBM->getWidth();
            while(dst < dstLast)
            {
               *dst++ = *src++;
            }
         }

         if ((in_pBM->attribute & BMA_TRANSLUCENT) == 0)
            tCache->tmu[0].downloadBitmap(&bmp, palette, pbmp->getGlideCacheInfo());
         else
            tCache->tmu[0].downloadTranslucentBitmap(&bmp, io_pSfc->getPalette(), pbmp->getGlideCacheInfo());
      }
      allowSubSampling = true;
   }
   curFillMode = GFX_FILL_TEXTURE;
   grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE);
   glide_table.fnAddVertexVTC = AddVertexVTC_Textured;
   guTexCombineFunction(0, GR_TEXTURECOMBINE_DECAL);
   
   if(in_pBM->attribute & BMA_TRANSPARENT)
   {
      grTexFilterMode(0, sg_nonblinearFilterMode, sg_nonblinearFilterMode);
      grChromakeyValue(0x00000000);
      grChromakeyMode(GR_CHROMAKEY_ENABLE);
   }
   if (in_pBM->attribute & BMA_TRANSLUCENT)
   {
      grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, 
                           GR_BLEND_ONE, GR_BLEND_ONE);
      grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                     GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_TEXTURE, FALSE);
   }
   
   for(i = 0; i < vertexCount; i++)
   {
      verts[i].tmuvtx[0].sow = (texCoords[i].x * 
         textureScale.x + textureOffset.x) * verts[i].oow;
      verts[i].tmuvtx[0].tow = (texCoords[i].y * 
         textureScale.y + textureOffset.y) * verts[i].oow;
      verts[i].a = constantAlpha;
   }

   grDrawPolygonVertexList(vertexCount, verts);

   cur = verts;
   curTex = texCoords;
   vertexCount = 0;
   if (in_pBM->attribute & BMA_TRANSLUCENT)
   {
      grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                           GR_BLEND_ONE, GR_BLEND_ZERO);
      curAlphaSource = (GFXAlphaSource)-1;
   }
   if(in_pBM->attribute & BMA_TRANSPARENT)
   {
      grChromakeyMode(GR_CHROMAKEY_DISABLE);
      grTexFilterMode(0, sg_bilinearFilterMode, sg_bilinearFilterMode);
   }
}

float lscale;
float rscale;
float tscale;
float bscale;

bool setupBitmapPoints(GFXSurface *sfc,
                       int x0, int y0,
                       int x1, int y1)
{
   lscale = tscale = 0.0f;
   rscale = bscale = 1.0f;

   if(sfc->getFlags() & GFX_DMF_RCLIP) {
      RectI *cr = sfc->getClipRect();
      float le = float(cr->upperL.x);
      float te = float(cr->upperL.y);
      float re = float(cr->lowerR.x) + 1.0f;
      float be = float(cr->lowerR.y) + 1.0f;

      if(x0 >= re || (x0 + x1) <= le || y0 >= be || (y0 + y1) <= te)
         return false;

      if(x0 < le)
         lscale = (le - x0) / x1;
      if(y0 < te)
         tscale = (te - y0) / y1;
      if(x0 + x1 > re)
         rscale = (re - x0) / x1;
      if(y0 + y1 > be)
         bscale = (be - y0) / y1;
   }

   cur[0].x = float(x0) + float(lscale * x1);
   cur[0].y = float(y0) + float(tscale * y1);
   cur[1].x = float(x0) + float(rscale * x1);
   cur[1].y = float(y0) + float(tscale * y1);
   cur[2].x = float(x0) + float(rscale * x1);
   cur[2].y = float(y0) + float(bscale * y1);
   cur[3].x = float(x0) + float(lscale * x1);
   cur[3].y = float(y0) + float(bscale * y1);

   int i;
   for(i = 0; i < 4; i++) {
      cur->oow = 1.0f;
      cur->r = cur->g = cur->b = cur->a = 1.0f;
      cur++;
   }

   vertexCount = 4;
   return true;
}

void
setupBitmapTexCoords(float x0, float y0,
                     float x1, float y1)
{
   curTex[0].x = float(x0) + float(x1 - x0) * lscale;
   curTex[0].y = float(y0) + float(y1 - y0) * tscale;
   curTex[0].z = 1.0f;
   curTex[1].x = float(x0) + float(x1 - x0) * rscale;
   curTex[1].y = curTex[0].y;
   curTex[1].z = 1.0f;
   curTex[2].x = curTex[1].x;
   curTex[2].y = float(y0) + float(y1 - y0) * bscale;
   curTex[2].z = 1.0f;
   curTex[3].x = curTex[0].x;
   curTex[3].y = curTex[2].y;
   curTex[3].z = 1.0f;
   curTex += 4;
}

void 
DrawBitmap2d_f(GFXSurface*      io_pSurface,
               const GFXBitmap* in_pBM,
               const Point2I*   in_at,
               GFXFlipFlag      in_flip)
{
   cur = verts;
   curTex = texCoords;
   vertexCount = 0;

   if(!setupBitmapPoints(io_pSurface,
                         in_at->x, in_at->y,
                         in_pBM->getWidth(), in_pBM->getHeight())) {
      return;
   }
   
   if (in_flip != GFX_FLIP_NONE) {
      float coords[4];
      coords[0] = 0.0f;
      coords[1] = 0.0f;
      coords[2] = float(in_pBM->getWidth());
      coords[3] = float(in_pBM->getHeight());
      
      float temp;
      if ((in_flip & GFX_FLIP_X) != 0) {
         temp = coords[2];
         coords[2] = coords[0];
         coords[0] = temp;
      }
      if ((in_flip & GFX_FLIP_Y) != 0) {
         temp = coords[1];
         coords[1] = coords[3];
         coords[3] = temp;
      }
      setupBitmapTexCoords(coords[0], coords[1], coords[2], coords[3]);
   } else {
      setupBitmapTexCoords(0.0f, 0.0f, float(in_pBM->getWidth()), float(in_pBM->getHeight()));
   }
   
   bitmapDraw(io_pSurface, in_pBM);
}

void 
DrawBitmap2d_rf(GFXSurface*      io_pSurface,
                const GFXBitmap* in_pBM,
                const RectI*     in_subRegion,
                const Point2I*   in_at,
                GFXFlipFlag      in_flip)
{
   cur = verts;
   curTex = texCoords;
   vertexCount = 0;

   if(!setupBitmapPoints(io_pSurface,
                         in_at->x, in_at->y,
                         in_subRegion->len_x() + 1, in_subRegion->len_y() + 1)) {
      return;
   }
   
   if (in_flip != GFX_FLIP_NONE) {
      float coords[4];
      coords[0] = float(in_subRegion->upperL.x);
      coords[1] = float(in_subRegion->upperL.y);
      coords[2] = float(in_subRegion->lowerR.x + 1);
      coords[3] = float(in_subRegion->lowerR.y + 1);
      
      float temp;
      if ((in_flip & GFX_FLIP_X) != 0) {
         temp = coords[2];
         coords[2] = coords[0];
         coords[0] = temp;
      }
      if ((in_flip & GFX_FLIP_Y) != 0) {
         temp = coords[1];
         coords[1] = coords[3];
         coords[3] = temp;
      }
      setupBitmapTexCoords(coords[0], coords[1], coords[2], coords[3]);
   } else {
      setupBitmapTexCoords(float(in_subRegion->upperL.x), float(in_subRegion->upperL.y),
                           float(in_subRegion->lowerR.x) + 1, float(in_subRegion->lowerR.y + 1));
   }

   bitmapDraw(io_pSurface, in_pBM);
}

void 
DrawBitmap2d_sf(GFXSurface*      io_pSurface,
                const GFXBitmap* in_pBM,
                const Point2I*   in_at,
                const Point2I*   in_stretch,
                GFXFlipFlag      in_flip)
{
   cur = verts;
   curTex = texCoords;
   vertexCount = 0;

   if(!setupBitmapPoints(io_pSurface,
                         in_at->x, in_at->y,
                         in_stretch->x, in_stretch->y)) {
      return;
   }
   
   if (in_flip != GFX_FLIP_NONE) {
      float coords[4];
      coords[0] = 0.0f;
      coords[1] = 0.0f;
      coords[2] = float(in_pBM->getWidth());
      coords[3] = float(in_pBM->getHeight());
      
      float temp;
      if ((in_flip & GFX_FLIP_X) != 0) {
         temp = coords[2];
         coords[2] = coords[0];
         coords[0] = temp;
      }
      if ((in_flip & GFX_FLIP_Y) != 0) {
         temp = coords[1];
         coords[1] = coords[3];
         coords[3] = temp;
      }
      setupBitmapTexCoords(coords[0], coords[1], coords[2], coords[3]);
   } else {
      setupBitmapTexCoords(0.0f, 0.0f, float(in_pBM->getWidth()), float(in_pBM->getHeight()));
   }

   bitmapDraw(io_pSurface, in_pBM);
}


void 
FlushTexture(GFXSurface*      io_pSurface,
             const GFXBitmap* in_pTexture,
             const bool       in_reload)
{
   tCache->tmu[0].flushTexture((GFXBitmap *) in_pTexture, io_pSurface->getPalette(), in_reload);
}

void
SetZMode(GFXSurface *, bool wBuffer)
{
   wBuffer;
}

void
SetClipPlanes(GFXSurface* io_pSurface,
              const float in_nearDist,
              const float in_farDist)
{
   Surface* pSurface = static_cast<Surface*>(io_pSurface);

   //pSurface->nearDist = in_nearDist;
   //pSurface->farDist  = in_farDist;
}

GFXHazeSource
GetHazeSource(GFXSurface* /*io_pSurface*/)
{
   return curHazeSource;
}

float
GetConstantHaze(GFXSurface* /*io_pSurface*/)
{
   return constantHaze / 255.0f;
}

void
hackDrawPoints(GFXSurface*      io_pSurface,
               const UInt32     in_numPoints,
               const Point3F*   in_pPoints,
               const float*     in_pPointsSize,
               const ColorF*    in_pPointsColor,
               const float*     in_pPointsAlpha,
               const GFXBitmap* in_pBitmap)
{
   AssertFatal(in_pBitmap != NULL, "No bitmap?");

   curFillMode    = -1;
   curAlphaSource = GFXAlphaSource(-1);

   grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                  GR_COMBINE_FACTOR_LOCAL,
                  GR_COMBINE_LOCAL_ITERATED,
                  GR_COMBINE_OTHER_TEXTURE,
                  FXFALSE);

   GFXBitmap* pBitmap = (GFXBitmap*)in_pBitmap;
   GFXPalette* pal    = io_pSurface->getPalette();
   if (pBitmap->pPalette != NULL)
      pal = pBitmap->pPalette;

   if(!tCache->tmu[0].setCurrentTexture(pBitmap->getGlideCacheInfo(), pBitmap->paletteIndex, pal))
   {
      if(pBitmap->attribute & BMA_TRANSLUCENT)
         tCache->tmu[0].downloadTranslucentBitmap(pBitmap, pal, pBitmap->getGlideCacheInfo());
      else
         tCache->tmu[0].downloadBitmap(pBitmap, pal, pBitmap->getGlideCacheInfo());
   }

   if ((pBitmap->attribute & BMA_ADDITIVE) != 0) {
      grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                     GR_COMBINE_FACTOR_NONE,
                     GR_COMBINE_LOCAL_ITERATED,
                     GR_COMBINE_OTHER_NONE,
                     FXFALSE);
      grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE, 
                           GR_BLEND_ONE,       GR_BLEND_ONE);
   } else {
      grAlphaCombine(GR_COMBINE_FUNCTION_BLEND_OTHER,
                     GR_COMBINE_FACTOR_LOCAL,
                     GR_COMBINE_LOCAL_ITERATED,
                     GR_COMBINE_OTHER_TEXTURE,
                     FXFALSE);
      grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, 
                           GR_BLEND_ONE,       GR_BLEND_ONE);
   }

   grDepthBufferFunction(GR_CMP_LEQUAL);
   grDepthMask(FXFALSE);

   GrVertex pVerts[4];

   for (UInt32 i = 0; i < in_numPoints; i++) {

      for (int j = 0; j < 4; j++) {
         pVerts[j].x   = in_pPoints[i].x;
         pVerts[j].y   = in_pPoints[i].y;
         pVerts[j].oow = in_pPoints[i].z;
         pVerts[j].r   = in_pPointsColor[i].red   * 255.0f;
         pVerts[j].g   = in_pPointsColor[i].green * 255.0f;
         pVerts[j].b   = in_pPointsColor[i].blue  * 255.0f;
         pVerts[j].a   = in_pPointsAlpha[i]       * 255.0f;
         pVerts[j].tmuvtx[0].sow = in_pPoints[i].z;
         pVerts[j].tmuvtx[0].tow = in_pPoints[i].z;
         pVerts[j].tmuvtx[0].oow = 1.0f;
      }

      float size = in_pPointsSize[i] * 0.5f;
      
      pVerts[0].x -= size;
      pVerts[0].y -= size;
      pVerts[0].tmuvtx[0].oow = 1.0f;
      pVerts[1].x += size;
      pVerts[1].y -= size;
      pVerts[1].tmuvtx[0].oow = 1.0f;
      pVerts[2].x += size;
      pVerts[2].y += size;
      pVerts[2].tmuvtx[0].oow = 1.0f;
      pVerts[3].x -= size;
      pVerts[3].y += size;
      pVerts[3].tmuvtx[0].oow = 1.0f;

      pVerts[0].tmuvtx[0].sow *= 0.5f;
      pVerts[0].tmuvtx[0].tow *= 0.5f;
      pVerts[1].tmuvtx[0].sow *= 255.5f;
      pVerts[1].tmuvtx[0].tow *= 0.5f;
      pVerts[2].tmuvtx[0].sow *= 255.5f;
      pVerts[2].tmuvtx[0].tow *= 255.5f;
      pVerts[3].tmuvtx[0].sow *= 0.5f;
      pVerts[3].tmuvtx[0].tow *= 255.5f;

      for (int i = 0; i < 4; i++) {
         pVerts[i].x += float(3<<18);
         pVerts[i].y += float(3<<18);
      }

      grDrawTriangle(pVerts, pVerts + 1, pVerts + 3);
      grDrawTriangle(pVerts + 1, pVerts + 2, pVerts + 3);
   }

   grDepthBufferFunction(GR_CMP_LEQUAL);
   grDepthMask(FXTRUE);
}




}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// glide function table

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

FunctionTable glide_table =
{
   Glide::ClearScreen,           // Clear

   Glide::DrawPoint,      // Draw Point 2d
   Glide::DrawLine2d,       // Draw Line 2d
   Glide::DrawRect2d,       // Draw Rectangle 2d
   Glide::DrawRect_f,      // Draw Rectangle 2d, Filled

   Glide::DrawBitmap2d_f,     // Draw Bitmap 2d
   Glide::DrawBitmap2d_rf,     // Draw Bitmap 2d, Region
   Glide::DrawBitmap2d_sf,  // Draw Bitmap 2d, Stretch
   NULL,                   // Draw Bitmap 2d, Stretch, Rotate

   GFXDrawText_p,          // Draw Text String at Given Point
   GFXDrawText_r,          // Draw Text String at Given Rectangle

   Glide::Draw3dBegin, // Draw 3d scene begin.
   Glide::Draw3dEnd, // Draw 3d scene end.
   Glide::AddVertexV, // Add vertex with no shade/texture
   Glide::AddVertexVT,
   Glide::AddVertexVC,
   Glide::AddVertexVTC_NoTex,
   Glide::EmitPoly,
   Glide::SetShadeSource,
   Glide::SetHazeSource,
   Glide::SetAlphaSource,
   Glide::SetFillColorCF,
   Glide::SetFillColorI,
   Glide::SetHazeColorCF,
   Glide::SetHazeColorI,
   Glide::SetConstantShadeCF,
   Glide::SetConstantShadeF,
   Glide::SetConstantHaze,
   Glide::SetConstantAlpha,
   Glide::SetTransparency,
   Glide::SetTextureMap,
   Glide::SetFillMode,
   Glide::SetTexturePerspective,

   Glide::RegisterTexture,
   Glide::RegisterTextureCB,
   Glide::SetTextureHandle,
   GFXAllocateLightMap,
   Glide::HandleGetLightMap,
   Glide::HandleSetLightMap,
   Glide::HandleSetTextureMap,
   Glide::HandleSetMipLevel,
   Glide::HandleSetTextureSize,
   Glide::FlushTextureCache,
   Glide::SetZTest,
   Glide::ClearZBuffer,
   Glide::SetTextureWrap,
   
   Glide::FlushTexture,
   Glide::SetZMode,

   Glide::SetClipPlanes,

   Glide::GetHazeSource,
   Glide::GetConstantHaze
};

