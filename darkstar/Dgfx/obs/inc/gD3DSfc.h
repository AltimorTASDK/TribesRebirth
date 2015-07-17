//----------------------------------------------------------------------------
//
//  $Workfile:   gd3dsfc.h  $
//  $Version$
//  $Revision:     $
//
//  DESCRIPTION:  Direct3D gfx interface surface and function table.
//
//  These functions are prototypes for the glue functions for all of the 
//  primitives that go into the function table in FN_TABLE.H.  This surface is
//  modeled on the Glide 3DFx surface in gglidesfc.h.  The Direct3D Functions
//  currently use the DirectPrimitive Interface
//  
//----------------------------------------------------------------------------

#ifndef _GD3DSFC_H_
#define _GD3DSFC_H_

//Includes
#include <base.h>
#include <tVector.h>

#include <d3d.h>
#include <d3dcaps.h>
#include <d3dtypes.h>

#include "g_surfac.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


// Forward declarations
struct GFXDeviceDescriptor;

namespace Direct3D {


// Function pointer types to set shade, haze, and alpha values...
//
class Surface;
typedef void (*fnVertexSetAlpha)(Surface*, D3DTLVERTEX*, const GFXColorInfoF*);
typedef void (*fnVertexSetShade)(Surface*, D3DTLVERTEX*, const GFXColorInfoF*);
typedef void (*fnVertexSetHaze)(Surface*,  D3DTLVERTEX*, const GFXColorInfoF*);

void setHazeNone(Surface*,      D3DTLVERTEX*, const GFXColorInfoF*);
void setHazeConstant(Surface*,  D3DTLVERTEX*, const GFXColorInfoF*);
void setHazeVertex(Surface*,    D3DTLVERTEX*, const GFXColorInfoF*);
void setShadeNone(Surface*,     D3DTLVERTEX*, const GFXColorInfoF*);
void setShadeConstant(Surface*, D3DTLVERTEX*, const GFXColorInfoF*);
void setShadeVertex(Surface*,   D3DTLVERTEX*, const GFXColorInfoF*);
void setAlphaNone(Surface*,     D3DTLVERTEX*, const GFXColorInfoF*);
void setAlphaConstant(Surface*, D3DTLVERTEX*, const GFXColorInfoF*);
void setAlphaVertex(Surface*,   D3DTLVERTEX*, const GFXColorInfoF*);


// Used in the DevResVector to map width-height resolutions to their DDraw
//  surface descriptions
typedef struct _modeDescription {
   Point2I res;
   Int32   bpp;
   
   DDSURFACEDESC ddSDesc;
} modeDescription;


//------------------------------------------------------------------------------
//-------------------------------------- Direct3D::Surface
//

class TextureCache;
class HandleCache;

class Surface : public GFXSurface
{
   friend class TextureCache;

   struct D3DDeviceDescriptor {
      bool          valid;
      GUID          deviceGUID;
      D3DDEVICEDESC devDesc;
      char*         devName;
      
      bool  canDoMipmaps;
      DWORD filterMode;

      bool  colorLightMaps;
      bool  lightMapInverted;
      DWORD lightMapMode;

      bool canFilter;
      bool canDoTwoPass;
      bool canDoColorKey;
      bool driverAlphaInverted;
      bool driverAlphaCmp;
   };

   struct RenderState {
      D3DTEXTUREHANDLE  textureHandle;
      D3DTEXTUREADDRESS textureAddress;
      BOOL              texturePerspective;
      BOOL              wrapU;
      BOOL              wrapV;
      BOOL              zEnable;
      D3DFILLMODE       fillMode;
      D3DSHADEMODE      shadeMode;
      BOOL              zWriteEnable;
      BOOL              alphaTestEnable;
      D3DTEXTUREFILTER  textureMag;
      D3DTEXTUREFILTER  textureMin;
      D3DBLEND          srcBlend;
      D3DBLEND          destBlend;
      D3DTEXTUREBLEND   textureMapBlend;
      D3DCULL           cullMode;
      D3DCMPFUNC        zFunc;
      D3DFIXED          alphaRef;
      D3DCMPFUNC        alphaFunc;
      BOOL              blendEnable;
      BOOL              fogEnable;
      BOOL              zVisible;
      D3DCOLOR          fogColor;
      BOOL              colorKeyEnable;
      BOOL              alphaBlendEnable;
   };

   void _setRenderState(const DWORD in_stateType, const DWORD in_state,
                                DWORD* io_pCurrState);

   static bool initialized;
   
   // DirectX Interface objects, class Level, initialized by init(), except for
   //  pD3DDevice2Vector
   static Vector<char *>            pDDNameVector;
   static Vector<IDirectDraw2*>     pDD2Vector;
   static Vector<LPSTR>             pD3DNameVector;
   static Vector<IDirect3D2*>       pD3D2Vector;
   static Vector<IDirect3DDevice2*> pD3DDevice2Vector;

   // Since we get the GUIDs for a device when init() is called,
   //  but cannot _create_ the devices until we create the DDSurface
   //  in create(), we need a place to store the GUIDs for easy retreval
   static Vector<D3DDeviceDescriptor> D3DDevDescVector;
   
   // Holds allowed video modes for each device, after describeDevices is called
   static Vector<Vector<modeDescription>*> DevResVector;

   // State variables used to keep track of vertices and polys emitted from GFX,
   //  which are then passed to D3D...
   //
   enum { MAX_NUM_VERTICES = 100 };
   bool        lightMapsEnabled;
   bool        polyBegun;
   Int32       numVerticesEmitted;
   D3DTLVERTEX pD3DTLVertexBuffer[MAX_NUM_VERTICES];

   Int32        numQueueVerts;
   D3DTLVERTEX* pD3DTLQueueBuffer;

   static HWND D3DSurfaceHWnd;
   static bool cooperativeLevelSet;
   
   static Int32 modeWidth;
   static Int32 modeHeight;
   static Int32 modeDepth;

   // object Level interfaces, initialized by create()
   IDirectDrawPalette*  pDDPalette;
   IDirectDrawSurface2* pDDFront;

   IDirectDrawSurface2* pDDBack;
   IDirectDrawSurface2* pZBuffer;
   IDirect3DViewport2*  pViewport;

   IDirect3DMaterial2*  pBGMaterial;
   D3DMATERIALHANDLE    hBGMaterial;
   PALETTEENTRY         currBGColor;

   Int32                deviceMinorId;

   // Texture cache, and our storage of the current renderstate...
   TextureCache* _pTextureCache;
   HandleCache*  _pHandleCache;

   // To let the texture cache know how much memory is left...
   //
   DWORD getFreeTextureMemory();
   static DWORD getTotalSurfaceMemory(const Int32 in_deviceMinorId);

   // Required virtual functions for implementing GFXSurface interface...
   //
   Bool _setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, bool);
   void _setGamma();
   void _lock();
   void _unlock();
   void _lockDC();
   void _unlockDC();


   void emitTexturePoly(DWORD      in_key,
                                const bool in_bitmapOverride,
                                const bool in_clamp);
   void emitTwoPassPoly();
   void emitConstantPoly();

  public:
   Surface(Int32 in_deviceMinorId);
   ~Surface();

   // Hack to allow direct access to drawprim for lines and points...
   //
   IDirect3DDevice2* getD3DDevice2() const { return pD3DDevice2Vector[deviceMinorId]; }

   void  processMessage(const int   in_argc,
                                const char* in_argv[]);
   DWORD getCaps();

   static GFXSurface * create(HWND hwnd, Int32 in_width, Int32 in_height,
                                      Int32 deviceMinorId);

   static bool setDisplayMode(Int32 in_width, Int32 in_height,
                                      Int32 in_bitDepth, Int32 in_deviceMinorId);
   static void restoreDisplayMode(HWND in_hWnd, Int32 in_deviceMinorId);

   static BOOL    enumDDCallback(LPGUID lpGUID, LPSTR lpDriverDescription,
                                         LPSTR lpDriverName);
   static HRESULT enumD3DCallback(LPGUID, LPSTR, LPSTR, LPD3DDEVICEDESC,
                                          LPD3DDEVICEDESC, LPVOID);
   static bool init(HWND hWnd);
   static void free();

   //-------------------------------------- Render state variables
   //
   // D3D Render states
   RenderState        renderState;
   static RenderState defaultRenderState;

   // GFX Renderstate variables
   bool          draw3DBegun;

   bool   transparencyEnabled;
   int    isZEnabled;
   bool   isFogEnabled;
   bool   isWrapEnabled;
   bool   isShadeEnabled;

   D3DCOLORVALUE fillColor;
   D3DCOLORVALUE hazeColor;
   D3DCOLORVALUE shadeColor;

   float  constantAlpha;
   float  constantHaze;

   fnVertexSetAlpha  vertexAlpher;
   fnVertexSetShade  vertexShader;
   fnVertexSetHaze   vertexHazer;

   GFXColorModel     colorModel;
   GFXShadeSource    shadeSource;
   GFXAlphaSource    alphaSource;
   GFXHazeSource     hazeSource;
   GFXFillMode       fillMode;

   // Internal state maintanence functions...
   const RenderState* getRenderState() const { return &renderState; }
   void               setRenderState(const DWORD in_stateType, const DWORD in_state);
   void               refreshRenderState();
   void               setDefaultRenderState();

   void setColorKey(const bool in_enabled,       const bool in_hardenState = false);
   void setFogEnable(const bool in_enabled,      const bool in_hardenState = false);
   void setFilterEnable(const bool in_enabled,   const bool in_hardenState = false);
   void setZEnable(const int in_state,           const bool in_hardenState = false);
   void setWrapEnable(const bool in_enabled,     const bool in_hardenState = false);
   void setShadeEnable(const bool in_enabled,    const bool in_hardenState = false);

   //-------------------------------------- Function table mappings
   //
   void clearScreen(DWORD in_color);
   void clearZBuffer();
   void draw3DBegin();
   void draw3DEnd();

   // User-usable versions of the caches, only valid when surface is locked...
   //
   TextureCache* pTextureCache;
   HandleCache*  pHandleCache;

   //-------------------------------------- Function table helper functions...
   //
   bool isDrawing3D() { return draw3DBegun; }
   void setTextureMap(const GFXBitmap* in_pTexture);
   bool isAlphaInverted() { return D3DDevDescVector[deviceMinorId].driverAlphaInverted; }

   // Functions to manipulate the vertex buffer...
   //
   D3DTLVERTEX* getPCurrVertex();
   void         incCurrVertex() { numVerticesEmitted++; }
   void         emitPoly(const bool  in_bitmapOverride = false,
                                 const DWORD in_bitmapKey      = 0);
   void         emitLine();
   void         emitPoint();

   static Int32 getNumDevices();
   static bool  describeDevice(int deviceNumber, GFXDeviceDescriptor *dev);
   static void  getDeviceResolutions(Int32 in_minorDeviceNumber,
                                             Vector<modeDescription>* resVector);

   void drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at);
   void drawSurfaceToBitmap(GFXBitmap* io_pBitmap);
   void flip();						   

   void flushTextureCache();
};


inline D3DTLVERTEX* 
Surface::getPCurrVertex()
{
   AssertFatal(numVerticesEmitted < MAX_NUM_VERTICES,
               "Error, too many vertices!");

   return &pD3DTLVertexBuffer[numVerticesEmitted];
}

}; // namespace Direct3D


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // _GD3DSFC_H_