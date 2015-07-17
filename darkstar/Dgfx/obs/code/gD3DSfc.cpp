//------------------------------------------------------------------------------
// Description Direct3D Surface Implementation
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

// Initialized Direct3D GUIDs
//
#define INITGUID

#include "fn_table.h"
#include "gdmanag.h"
#include "gfxmetrics.h"

#include "gD3DSfc.h"
#include "gD3DTxTrans.h"
#include "gD3DTxCache.h"
#include "gD3DHCache.h"


// Size of the vertex queue...
//
#define D3D_SIZEQUEUEBUFFER 4096

// Uncomment to cause GFX to assert the reason a card does not meet
//  minimum requirements
//#define DEBUG_BAD_CARD

// Copied from D3D Examples for debugging...
//
extern char* D3DAppErrorToString(HRESULT error);


namespace Direct3D {

//-------------------------------------- Function Pointer Typedefs
//
typedef HRESULT (WINAPI *DDrawFuncPtr)(GUID FAR *lpGUID,
                                       LPDIRECTDRAW FAR *lplpDD,
                                       IUnknown FAR *pUnkOuter);
typedef HRESULT (WINAPI *DDrawEnumFuncPtr)(LPDDENUMCALLBACK lpCallback,
                                          LPVOID lpContext);


//-------------------------------------- Static Class Members
//
static DDrawFuncPtr     g_lpDirectDrawCreate    = NULL;
static DDrawEnumFuncPtr g_lpDirectDrawEnumerate = NULL;
static HINSTANCE hmodDD = NULL;

bool Surface::initialized = false;

Vector<char *>             Surface::pDDNameVector;
Vector<IDirectDraw2*>      Surface::pDD2Vector;
Vector<IDirect3D2*>        Surface::pD3D2Vector;
Vector<IDirect3DDevice2*>  Surface::pD3DDevice2Vector;

Vector<Surface::D3DDeviceDescriptor> Surface::D3DDevDescVector;

Vector<Vector<modeDescription>*> Surface::DevResVector;

HWND Surface::D3DSurfaceHWnd      = NULL;
bool Surface::cooperativeLevelSet = false;

Int32 Surface::modeWidth  = -1;
Int32 Surface::modeHeight = -1;
Int32 Surface::modeDepth  = -1;

Surface::RenderState Surface::defaultRenderState = {
   NULL,                // textureHandle
   D3DTADDRESS_CLAMP,   // textureAddress
   TRUE,                // texturePerspective;
   FALSE,               // wrapU;
   FALSE,               // wrapV;
   TRUE,                // zEnable;
   D3DFILL_SOLID,       // fillMode;
   D3DSHADE_GOURAUD,    // shadeMode;
   TRUE,                // zWriteEnable;
   FALSE,               // alphaTestEnable;
   D3DFILTER_LINEAR,    // textureMag (not set by defaultRenderState)
   D3DFILTER_LINEAR,    // textureMin (not set by defaultRenderState)
   D3DBLEND_ONE,        // srcBlend;
   D3DBLEND_ZERO,       // destBlend;
   D3DTBLEND_MODULATE,  // textureMapBlend;
   D3DCULL_CCW,         // cullMode;
   D3DCMP_ALWAYS,       // zFunc;
   0,                   // alphaRef;
   D3DCMP_LESSEQUAL,    // alphaFunc;
   FALSE,               // blendEnable
   FALSE,               // fogEnable;
   FALSE,               // zVisible
   D3DRGB(0, 0, 0),     // fogColor;
   FALSE,               // colorKeyEnable;
   FALSE                // alphaBlendEnable;
};                                                  


#define CALLBACK_FUNCTIONS
//------------------------------------------------------------------------------
// NAME 
//    enumDDCallBack(LPGUID lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID)
//    
// DESCRIPTION 
//    See Surface::enumDDCallBack
//------------------------------------------------------------------------------
BOOL WINAPI
enumDDCallback(LPGUID lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID)
{
   // Stub callback.  Map to class function
   return Surface::enumDDCallback(lpGUID, lpDriverDescription, lpDriverName);
}


//------------------------------------------------------------------------------
// NAME 
//    enumDDCallBack(LPGUID lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID)
//    
// DESCRIPTION 
//    See Surface::enumD3DDevCallBack
//------------------------------------------------------------------------------
HRESULT WINAPI
enumD3DCallback(LPGUID lpGUID,
                LPSTR lpDeviceDescription, LPSTR lpDeviceName,
                LPD3DDEVICEDESC lpD3DHWDeviceDesc,
                LPD3DDEVICEDESC lpD3DHELDeviceDesc,
                LPVOID lpUserArg)
{
   // Stub callback.  Map to class function
   return Surface::enumD3DCallback(lpGUID, lpDeviceDescription, lpDeviceName,
                                   lpD3DHWDeviceDesc, lpD3DHELDeviceDesc,
                                   lpUserArg);
}


//------------------------------------------------------------------------------
// NAME 
//    HRESULT WINAPI d3dEnumTexFmtCallback(LPDDSURFACEDESC lpDdsd, LPVOID lpUserArg)
//    
// DESCRIPTION 
//    Passes the texture formats enumerate to the TxTranslatorClassFact.
//    
// ARGUMENTS 
//    lpDdsd    - obv.
//    lpUserArg - NULL
//
//    
// NOTES 
//    Assumes that the caller of EnumTextureFormats has notified
//   TxTranslatorClassFact
//------------------------------------------------------------------------------
HRESULT WINAPI d3dEnumTexFmtCallback(LPDDSURFACEDESC lpDdsd,
                                     LPVOID          /*lpUserArg*/)
{
   TxTranslatorClassFact::addFormat(lpDdsd);

   return D3DENUMRET_OK;
}


DWORD 
Surface::getFreeTextureMemory()
{
   AssertFatal(pDD2Vector[deviceMinorId] != NULL, "No DD2 Interface to query...");

   DDSCAPS ddsCaps;
   ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;

   DWORD total, free;
   HRESULT result = 
      pDD2Vector[deviceMinorId]->GetAvailableVidMem(&ddsCaps, &total, &free);
   AssertWarn(result == DD_OK, avar("Error getting avail texmem: %s",
                                    D3DAppErrorToString(result)));
   return free;
}


DWORD 
Surface::getTotalSurfaceMemory(const Int32 in_deviceMinorId)
{
   AssertFatal(pDD2Vector[in_deviceMinorId] != NULL, "No DD2 Interface to query...");

   DDSCAPS ddsCaps;
   ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;

   DWORD total, free;
   HRESULT result = 
      pDD2Vector[in_deviceMinorId]->GetAvailableVidMem(&ddsCaps, &total, &free);
   AssertWarn(result == DD_OK, avar("Error getting avail texmem: %s",
                                    D3DAppErrorToString(result)));
   return total;
}



Surface::Surface(Int32 in_deviceMinorId)
 : deviceMinorId(in_deviceMinorId),
   pZBuffer(NULL),
   pDDFront(NULL),
   pDDBack(NULL),
   pViewport(NULL),
   _pTextureCache(NULL),
   _pHandleCache(NULL),
   pTextureCache(NULL),
   pHandleCache(NULL),
   pDDPalette(NULL),
   lightMapsEnabled(true),
   polyBegun(false),
   numVerticesEmitted(0),
   pBGMaterial(NULL),
   hBGMaterial(NULL),
   draw3DBegun(false),
   transparencyEnabled(false),
   isFogEnabled(false),
   isZEnabled(false),
   isShadeEnabled(false),
   isWrapEnabled(false),
   constantHaze(0.0f),
   vertexShader(setShadeNone),
   vertexHazer(setHazeNone),
   vertexAlpher(setShadeNone),
   hazeSource(GFX_HAZE_NONE),
   alphaSource(GFX_ALPHA_NONE),
   numQueueVerts(0)
{
   functionTable = &direct3d_table;

   fillColor.r  = fillColor.g  = fillColor.b  = fillColor.a  = 1.0f;
   shadeColor.r = shadeColor.g = shadeColor.b = shadeColor.a = 1.0f;
   
   hazeColor.r = hazeColor.b = hazeColor.a = 1.0f;
   hazeColor.g = 0.0f;

   // Hard-coded value for number of vertices right now, may want to make this
   //  a little more flexible later...
   //
   pD3DTLQueueBuffer = new D3DTLVERTEX[D3D_SIZEQUEUEBUFFER];
}



Surface::~Surface()
{
   if (pD3DTLQueueBuffer != NULL) {
      delete [] pD3DTLQueueBuffer;
      pD3DTLQueueBuffer = NULL;
   }

   if (pDDPalette != NULL) {
      pDDPalette->Release();
      pDDPalette = NULL;
   }
   pPalette = NULL;

   if (_pTextureCache != NULL) {
      delete pTextureCache;
      _pTextureCache = NULL;
      pTextureCache = NULL;
   }

   if (_pHandleCache != NULL) {
      delete pHandleCache;
      _pHandleCache = NULL;
      pHandleCache = NULL;
   }

   if (pBGMaterial != NULL) {
      pBGMaterial->Release();
      pBGMaterial = NULL;
      hBGMaterial = NULL;
      memset(&currBGColor, 0xff, sizeof(PALETTEENTRY));
   }

   if (pViewport != NULL) {
      pViewport->Release();
      pViewport = NULL;
   }

   if (pD3DDevice2Vector[deviceMinorId] != NULL) {
      pD3DDevice2Vector[deviceMinorId]->Release();
      pD3DDevice2Vector[deviceMinorId] = NULL;
   }
   
   if (pZBuffer) {
      pDDBack->DeleteAttachedSurface(0, pZBuffer);
      pZBuffer->Release();
      pZBuffer = NULL;
   }
   if (pDDFront != NULL) {
      pDDFront->Release();
      pDDFront = NULL;
      pDDBack = NULL;
   }

}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::flip()
//    
// DESCRIPTION 
//    Obv.
//    
// NOTES 
//    Check into asnyc flip in DX5? DMMNOTE
//------------------------------------------------------------------------------
void 
Surface::flip()
{
   AssertFatal(pDDFront != NULL, "Error, no flipping surface");
   AssertWarn(lockCount == 0,
              "Flipping locked surface, EndScene likely not called...");

   HRESULT result = pDDFront->Flip(NULL, DDFLIP_WAIT);
   AssertWarn(result == DD_OK, "Error flipping surface!");
}


//------------------------------------------------------------------------------
// NAME 
//    Bool Surface::_setPalette(GFXPalette *lpPalette, Int32, Int32, bool)
//    
// DESCRIPTION 
//    Sets the palette on the surface
//    
// NOTES 
//    The most important part here is the notification to the texture cache,
//   which filters down to the texture translators.  The surface object really
//   doesn't "care" what the palette is, except for clears, and setting the
//   haze color...
//------------------------------------------------------------------------------
Bool 
Surface::_setPalette(GFXPalette *lpPalette, Int32, Int32, bool)
{
   if (!lpPalette)
      return true;

   AssertFatal(pDD2Vector[deviceMinorId] != NULL, "Error, no DD2 Device...");

   // Create a directdraw palette...
   //
   IDirectDrawPalette* pTempDDPalette = NULL;
   HRESULT result = pDD2Vector[deviceMinorId]->CreatePalette(DDPCAPS_8BIT,
                                                             lpPalette->color,
                                                             &pTempDDPalette,
                                                             NULL);
   AssertWarn(result == DD_OK, "Unable to create a DD palette object...");
   if (result != DD_OK)
      return false;

   // Release any old palettes lying around...
   //
   if (pDDPalette != NULL)
      pDDPalette->Release();

   pDDPalette = pTempDDPalette;
   pPalette   = lpPalette;

   // Let the texture cache know that the palette has changed...
   //
   if (_pTextureCache != NULL)
      _pTextureCache->setPalette(lpPalette, pDDPalette);

   ColorF hazeColor;
   PALETTEENTRY* pHaze = &lpPalette->color[lpPalette->hazeColor & 0xff];
   hazeColor.red   = float(pHaze->peRed)   / 255.0f;
   hazeColor.green = float(pHaze->peGreen) / 255.0f;
   hazeColor.blue  = float(pHaze->peBlue)  / 255.0f;
   setHazeColor(&hazeColor);

   return true;
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::_lock()
//    void 
//    Surface::_unlock()
//    
// DESCRIPTION 
//    Locks the surface, and validates the texture and handle cache members
//    
// NOTES 
//------------------------------------------------------------------------------
void 
Surface::_lock()
{
   AssertFatal(pD3DDevice2Vector[deviceMinorId] != NULL,
               "Direct3D::Surface::_lock no device");
   AssertFatal(_pTextureCache != NULL, "Error, no texture cache...");
   AssertFatal(_pHandleCache != NULL, "Error, no handle cache...");

   // Since the lock count is incremented BEFORE GFXSurface::lock calls this
   //  member, we must check this against 1.
   //
   if (lockCount == 1) {
      // Since we draw ALL prims as polys, we need to begin the scene here...
      //
      HRESULT result = pD3DDevice2Vector[deviceMinorId]->BeginScene();
      if (result == DDERR_SURFACELOST) {
         AssertWarn(result == D3D_OK, avar("BeginScene try1: %s", D3DAppErrorToString(result)));
         pDDFront->Restore();
         pDDBack->Restore();
         pZBuffer->Restore();
         flushTextureCache();
         result = pD3DDevice2Vector[deviceMinorId]->BeginScene();
      }
      AssertWarn(result == D3D_OK, avar("BeginScene: %s", D3DAppErrorToString(result)));
   }

   pTextureCache = _pTextureCache;
   pHandleCache  = _pHandleCache;
}


void 
Surface::_unlock()
{
   AssertFatal(pD3DDevice2Vector[deviceMinorId] != NULL,
               "Direct3D::Surface::_lock no device");

   // As above, this member is affected BEFORE this function is called, so here
   //  we check agaist 0...
   //
   if (lockCount == 0) {
      HRESULT result = pD3DDevice2Vector[deviceMinorId]->EndScene();
      AssertWarn(result == D3D_OK, avar("EndScene: %s", D3DAppErrorToString(result)));

      pTextureCache = NULL;
      pHandleCache  = NULL;
   }
}


//========================================================================
//
// init()
//
// Description:
//    Allocates Direct3D structures, but does not switch video modes.
//    Note that ALL possible HW D3D Devices are enumerated, so they can
//    be properly represented to the Device Manager
//
// Arguments:
//    hWnd - Window handle of the calling process
//
// Notes:
//    GOTO's are used here to handle the problem of error handling in the
//     progressive initialization of the D3D Libs without duplicating code.
//     Arg. DMM 6/4/97
//========================================================================
bool 
Surface::init(HWND hWnd)
{ 
   HRESULT result;
   int i;
   
   if (initialized == true) {
      AssertWarn(0, "Attempted to re-init Direct3D::Surface");
      return true;
   }

   AssertFatal(hWnd, "Error: bad hwnd passed to Direct3D::Surface::init()");
   D3DSurfaceHWnd = hWnd;
   
   // Load the DirectX DLL, set the pointer to the DirectDrawEnumerate Function
   //
   hmodDD = LoadLibrary("DDRAW.DLL");
   AssertWarn( hmodDD, "GFXDDSurface::init: DDraw LoadLibrary Failed.");

   g_lpDirectDrawCreate =
         (DDrawFuncPtr)GetProcAddress(hmodDD, "DirectDrawCreate");
   g_lpDirectDrawEnumerate =
         (DDrawEnumFuncPtr)GetProcAddress(hmodDD, "DirectDrawEnumerateA");
   AssertWarn(g_lpDirectDrawEnumerate, "GFXDDSurface::init: DDraw GetProcAddress Failed for DDEnum.");
   AssertWarn(g_lpDirectDrawCreate, "GFXDDSurface::init: DDraw GetProcAddress Failed for DDCreate.");

   if (!g_lpDirectDrawEnumerate || !g_lpDirectDrawCreate)
         goto ErrorLibraryLoaded;
	
   result = (g_lpDirectDrawEnumerate)(Direct3D::enumDDCallback, NULL);  //call to function pointer
	AssertWarn(result == DD_OK, avar("GFXDDSurface::init: %s",
	                                 D3DAppErrorToString(result)));
	if ( result != DD_OK ) {
      // It is possible that some DD objects were created in the above call,
      //  so we cannot assume they are NULL...
      goto ErrorDDInitialized;
   }

   // Now we need to get the IDirect3D2 interface, and find the first hardware
   //  device attached to each.  First make sure the D3D Vectors are set to
   //  the proper size...
   //
   pD3D2Vector.setSize(pDD2Vector.size());
   D3DDevDescVector.setSize(pDD2Vector.size());
   
   for (i = 0; i < pDD2Vector.size(); i++) {
      IDirect3D2* pD3D2 = NULL;
      
      result = pDD2Vector[i]->QueryInterface(IID_IDirect3D2, (LPVOID *)&pD3D2);
      AssertWarn(result == S_OK, "Error Query for IDirect3D2 failed");
      // We'd _like_ to have the interface, but we need to handle correctly
      //  the case in which multiple DD interfaces are present, but only
      //  one has a Direct3D extension, so we'll push back whatever we get
      //  from the above, and handle NULL cases down the line when we are
      //  asked to enumerate possible D3D devices.
      //
      pD3D2Vector[i] = pD3D2;
      
      //  Now, if we were successful above, enumerate the devices attached
      // to this interface, passing the location of the D3DDeviceDescriptor
      // to be filled in...
      //
      D3DDevDescVector[i].valid   = false;
      D3DDevDescVector[i].devName = pDDNameVector[i];
      if (pD3D2Vector[i] != NULL) {
         result = pD3D2Vector[i]->EnumDevices(Direct3D::enumD3DCallback,
                                              &(D3DDevDescVector[i]));
         AssertWarn(result == D3D_OK,
                    avar("Error enumerating devices in Direct3D::Surface::init(): %s",
                         D3DAppErrorToString(result)));
      }
      
      // If we found a valid device, get rid of the superfluous copy of the name
      //
      if (D3DDevDescVector[i].valid == true)
         pDDNameVector[i] = NULL;
   }

   // Should be done.  At this point, the following structures should be
   //  successfully initialized:
   //   pDD2Vector,
   //   pD3D2Vector,
   //   D3DDevDescVector
   // Note that the pD3DDevice2Vector is NOT initialized, but we need place-
   //  holder elements so we can map 1 to 1 with the pDD2Vector, so we simply
   //  set the size before returning, and set all elements to NULL.  We need
   //  also the Resolution Vectors...
   //
   pD3DDevice2Vector.setSize(pDD2Vector.size());
   for (i = 0; i < pD3DDevice2Vector.size(); i++)
      pD3DDevice2Vector[i] = NULL;
   
   DevResVector.setSize(pDD2Vector.size());
   for (i = 0; i < DevResVector.size(); i++)
      DevResVector[i] = new Vector<modeDescription>;
   
   initialized = true;
   return true;

//-------------------------------------- ERROR HANDLERS
// Error target after pDD2Vector could potentially
//  contain IDirectDraw2 pointers
ErrorDDInitialized:
   int j;
   for (j = 0; j < pDD2Vector.size(); j++) {
      if (pDD2Vector[j] != NULL)
         pDD2Vector[j]->Release();
      pDD2Vector[j] = NULL;
   }
   pDD2Vector.clear();

// Error target after DDRAW.DLL is loaded
ErrorLibraryLoaded:
   if ( hmodDD )
      FreeLibrary( hmodDD );
      
   D3DSurfaceHWnd = NULL;
   
   return false;
}


//------------------------------------------------------------------------------
// NAME 
//    Surface::free()
//    
// DESCRIPTION 
//    Frees all DirectX Interface objects associated with this class
//    
// ARGUMENTS 
//    -
//    
// RETURNS 
//    -
//    
// NOTES 
//    We assume that all screens, and therefore all initialized devices in
//     pD3DDevice2Vector have been destroyed and free'd before this function
//     is called.
//------------------------------------------------------------------------------
void 
Surface::free()
{
   int i;

   // We need to Release() our DirectX object in the reverse order that
   //  we created them...

   // These should all be NULL at this point.
   pD3DDevice2Vector.clear();
   
   for (i = 0; i < DevResVector.size(); i++) {
      DevResVector[i]->clear();
      delete DevResVector[i];
      DevResVector[i] = NULL;
   }
   DevResVector.clear();
   
   for (i = 0; i < D3DDevDescVector.size(); i++) {
      Surface::D3DDeviceDescriptor* pDesc = &D3DDevDescVector[i];

      if (pDesc->devName != NULL) {
         AssertFatal(pDesc->valid == true,
                     "Deleting name from invalid device descriptor");
         delete [] pDesc->devName;
      }
      pDesc->devName = NULL;
      pDesc->valid   = false;
   }
   D3DDevDescVector.clear();
   
   for (i = 0; i < pD3D2Vector.size(); i++) {
      if (pD3D2Vector[i] != NULL)
         pD3D2Vector[i]->Release();
      pD3D2Vector[i] = NULL;
   }
   pD3D2Vector.clear();
   
   for (i = 0; i < pDD2Vector.size(); i++) {
      if (pDD2Vector[i] != NULL)
         pDD2Vector[i]->Release();
      pDD2Vector[i] = NULL;   
   }
   pDD2Vector.clear();
   
   for (i = 0; i < pDDNameVector.size(); i++) {
      if (pDDNameVector[i] != NULL) {
         delete [] pDDNameVector[i];
         pDDNameVector[i] = NULL;
      }
   }
   pDDNameVector.clear();

   // ...NULL our function pointers, and release DDRAW.DLL
   //
   g_lpDirectDrawCreate    = NULL;
   g_lpDirectDrawEnumerate = NULL;
   
   if (hmodDD)
      FreeLibrary(hmodDD);
   hmodDD = NULL;
   
   initialized = false;
}


//------------------------------------------------------------------------------
// NAME 
//    void Surface::restoreDisplayMode()
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
Surface::restoreDisplayMode(HWND in_hWnd, Int32 in_deviceMinorId)
{
   AssertFatal(pDD2Vector[in_deviceMinorId] != NULL,
               "Bad device minorId passed to D3D::Surface::restoreDisplayMode");
   HRESULT result;
   result = pDD2Vector[in_deviceMinorId]->SetCooperativeLevel(in_hWnd, DDSCL_NORMAL);
   AssertWarn(result == DD_OK, avar("Error restoring display mode: %s",
                                    D3DAppErrorToString(result)));
}


//------------------------------------------------------------------------------
// NAME 
//    GFXSurface* Surface::create()
//    
// DESCRIPTION 
//    Creates a Direct3D Surface and returns it.  Note that this will fail
//   unless the Display mode has been set to in_width x in_height x 16
//    
// ARGUMENTS 
//    hWnd      - Window handle to call SetCooperativeLevel on
//    in_width  - width of display
//    in_height - height ""
//    minorDeviceNumber - Device number to create screen from...
//    
// RETURNS 
//    Surface ptr.
//    
// NOTES 
//------------------------------------------------------------------------------
GFXSurface*                                                                           
Surface::create(HWND hWnd, Int32 in_width, Int32 in_height, Int32 deviceMinorId)              
{                                                                                             
   // Make sure this is a valid device                                                        
   //                                                                                         
   if (pD3D2Vector[deviceMinorId] == NULL) {                                                  
      AssertWarn(0, "Error, Attempted to allocate an invalid minor device surface");          
      return NULL;                                                                            
   }                                                                                          
                                                                                              
   // make sure nothing else is using the device...                                           
   //
   if (pD3DDevice2Vector[deviceMinorId] != NULL) {
      AssertWarn(0, "Error, attempted to create a surface on a device already in use");
      return NULL;
   }
   
   // Set Exclusive mode on this hWnd
   HRESULT result;
   result = pDD2Vector[deviceMinorId]->SetCooperativeLevel(hWnd,
                                                           DDSCL_ALLOWMODEX  |
                                                           DDSCL_ALLOWREBOOT |
                                                           DDSCL_EXCLUSIVE   |
                                                           DDSCL_FULLSCREEN  |
                                                           DDSCL_NOWINDOWCHANGES);
   AssertWarn(result == DD_OK, "Warning, SetCooperativeLevel Error in Direct3D::Surface::create");
   if (result != DD_OK && result != DDERR_EXCLUSIVEMODEALREADYSET) {
      AssertWarn(0, "Could Not set exclusive mode in Direct3D::Surface::create()");
      return NULL;
   }
   
   Surface *pRetSurf = new Surface(deviceMinorId);
   
   // Find the modeDescription for this video mode
   //
   Int32 modeNumber = -1;
   for (int i = 0; i < (DevResVector[deviceMinorId])->size(); i++) {
      if ((*DevResVector[deviceMinorId])[i].res.x == in_width &&
          (*DevResVector[deviceMinorId])[i].res.y == in_height) {
         modeNumber = i;
         break;
      }
   }
   AssertWarn(modeNumber != -1,
              avar("Error, could not match mode (%d x %d)", in_width, in_height));
   if (modeNumber == -1) {
      delete pRetSurf;
      return NULL;
   }

   // Create a description of the Surface:
   DDSURFACEDESC ddsd;
   memset(&ddsd, 0, sizeof(DDSURFACEDESC));
   ddsd.dwSize  = sizeof(DDSURFACEDESC);
   ddsd.dwFlags = DDSD_BACKBUFFERCOUNT |
                  DDSD_CAPS;

   ddsd.dwWidth  = in_width;
   ddsd.dwHeight = in_height;
   ddsd.dwBackBufferCount = 1;

   ddsd.ddsCaps.dwCaps  = DDSCAPS_PRIMARYSURFACE |
                          DDSCAPS_VIDEOMEMORY    |
                          DDSCAPS_COMPLEX        |
                          DDSCAPS_FLIP           |
                          DDSCAPS_3DDEVICE;

   // Create the Surface.  We need IDirectDrawSurface3 Interfaces for both
   //  the front and back surface. (DirectX v5+)
   //
   IDirectDrawSurface* pTempSurface     = NULL;
   IDirectDrawSurface* pTempBackSurface = NULL;
   result = pDD2Vector[deviceMinorId]->CreateSurface(&ddsd,
                                                     &pTempSurface,
                                                     NULL);
   AssertWarn(result == DD_OK, D3DAppErrorToString(result));
   if (result != DD_OK) {
      AssertWarn(0, avar("Unable to create Flipping structure for Direct3D:Surface. (%d x %d) Error: %u",
                         in_width, in_height, result));
      delete pRetSurf;
      return NULL;
   }
   result = pTempSurface->QueryInterface(IID_IDirectDrawSurface2,
                                         (LPVOID *)&pRetSurf->pDDFront);
   if (result != S_OK) {
      AssertWarn(0, "Unable to create IDirectDrawSurface3 Interface");
      pTempSurface->Release();
      
      delete pRetSurf;
      return NULL;
   }
   
   // Get the back buffer
   DDSCAPS ddsCaps;
   ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
   
   result = pTempSurface->GetAttachedSurface(&ddsCaps, &pTempBackSurface);
   if (result != DD_OK) {
      AssertWarn(0, "Unable to obtain back buffer from DirectDraw Surface");
      pTempSurface->Release();

      delete pRetSurf;
      return NULL;
   }
   result = pTempBackSurface->QueryInterface(IID_IDirectDrawSurface2,
                                             (LPVOID *)&pRetSurf->pDDBack);
   if (result != S_OK) {
      AssertWarn(0, "Unable to create IDirectDrawSurface3 Interface");
      pTempSurface->Release();
      pTempBackSurface->Release();
      
      delete pRetSurf;
      return NULL;
   }
   
   // Since QueryInterface, if successful, increases an objects ref. count,
   //  we can Release() our TempSurfaces, and they will be deallocated when
   //  the surface releases the pDDSurface3 objects...
   //
   pTempSurface->Release();
   pTempBackSurface->Release();
   pTempSurface = pTempBackSurface = NULL;
   

   // Ok, now we need the ZBuffer...
   //
   IDirectDrawSurface* pTempZBuf = NULL;
   memset(&ddsd, 0, sizeof(DDSURFACEDESC));
   ddsd.dwSize  = sizeof(DDSURFACEDESC);
   ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT |
                  DDSD_ZBUFFERBITDEPTH     |
                  DDSD_CAPS;

   ddsd.dwWidth  = in_width;
   ddsd.dwHeight = in_height;

   DWORD ddEncDepth =
         D3DDevDescVector[deviceMinorId].devDesc.dwDeviceZBufferBitDepth;

   if ((ddEncDepth & DDBD_32) != 0) {
      ddsd.dwZBufferBitDepth = 32;
   } else if ((ddEncDepth & DDBD_24) != 0) {
      ddsd.dwZBufferBitDepth = 24;
   } else if ((ddEncDepth & DDBD_16) != 0) {
      ddsd.dwZBufferBitDepth = 16;
   } else {
      AssertWarn(0, "Unsupported ZBuffer bit depth");

      delete pRetSurf;
      return NULL;
   }

   ddsd.ddsCaps.dwCaps  = DDSCAPS_ZBUFFER |
                          DDSCAPS_VIDEOMEMORY;
   result = pDD2Vector[deviceMinorId]->CreateSurface(&ddsd,
                                                     &pTempZBuf,
                                                     NULL);
   AssertWarn(result == DD_OK,
              avar("Error creating ZBuffer in Direct3D::Surface::create(): %s",
                   D3DAppErrorToString(result)));
   if (result != DD_OK) {
      delete pRetSurf;
      return NULL;
   }
   
   // Get the IDirectDraw3 version of the ZBuffer interface, and attach it to
   //  the back buffer...
   //
   result = pTempZBuf->QueryInterface(IID_IDirectDrawSurface2,
                                      (LPVOID *)&pRetSurf->pZBuffer);
   AssertWarn(result == DD_OK, avar("Error creating ZBuffer in Direct3D::Surface::create(): %s",
                                    D3DAppErrorToString(result)));
   if (result != DD_OK) {
      pTempZBuf->Release();

      delete pRetSurf;
      return NULL;
   }
   // As before, we can safely release the pTempZBuf
   //
   pTempZBuf->Release();

   result = pRetSurf->pDDBack->AddAttachedSurface(pRetSurf->pZBuffer);
   AssertWarn(result == DD_OK, avar("Error creating ZBuffer in Direct3D::Surface::create(): %s",
                                    D3DAppErrorToString(result)));
   if (result != DD_OK) {
      delete pRetSurf;
      return NULL;
   }


   // Since we were successful in creating all the surfaces that we need, we
   //  can set up some of the return surface's variables...
   //
   DDSURFACEDESC strideDDSD;
   strideDDSD.dwSize = sizeof(DDSURFACEDESC);
   pRetSurf->pDDBack->GetSurfaceDesc(&strideDDSD);
   pRetSurf->surfaceWidth  = in_width;
   pRetSurf->surfaceHeight = in_height;
   pRetSurf->surfaceStride = strideDDSD.lPitch;

   // Ok, now that we have our front, back, and Z- buffers, we need to
   //  attach the desired IDirect3DDevice interface to them.  This involves:
   //   Querying the Surface for the Device
   //   Enumerating the supported texture formats, and getting a translator
   //   
   //
   GUID devGUID = D3DDevDescVector[deviceMinorId].deviceGUID;

   // We need a IDirect3DDevice2 interface for the drawprim methods,
   //  which we obtain, God knows why, from a completely different place than
   //  we used to get the IDirect3DDevice interface...
   //
   IDirectDrawSurface* pTSurface;
   result = pRetSurf->pDDBack->QueryInterface(IID_IDirectDrawSurface,
                                              (LPVOID*)&pTSurface);
   if (result != S_OK) {
      delete pRetSurf;
      return NULL;
   }

   IDirect3DDevice2* pTempDevice2 = NULL;
   result = pD3D2Vector[deviceMinorId]->CreateDevice(devGUID,
                                                     pTSurface,
                                                     &pTempDevice2);
   if (result != S_OK) {
      delete pRetSurf;
      return NULL;
   }
   pD3DDevice2Vector[deviceMinorId] = pTempDevice2;

   // Ok, we have the device, start a texture enumeration so we can
   //  retreive the optimal texture format and translator...
   //
   AssertFatal(pD3DDevice2Vector[deviceMinorId] != NULL, "Bogus!");
   TxTranslatorClassFact::beginTextureEnum();
   pD3DDevice2Vector[deviceMinorId]->EnumTextureFormats(d3dEnumTexFmtCallback,
                                                       NULL);
   TxTranslatorClassFact::endTextureEnum();

   // Create the TextureTranslator and TextureCache this surface will use
   //
   TextureTranslator* pTexTranslator =
         TxTranslatorClassFact::getBestTranslator(D3DDevDescVector[deviceMinorId].driverAlphaInverted,
                                                  D3DDevDescVector[deviceMinorId].driverAlphaCmp,
                                                  D3DDevDescVector[deviceMinorId].canDoColorKey,
                                                  D3DDevDescVector[deviceMinorId].colorLightMaps,
                                                  D3DDevDescVector[deviceMinorId].lightMapInverted,
                                                  D3DDevDescVector[deviceMinorId].lightMapMode);
   if (pTexTranslator == NULL) {
      AssertWarn(0, "Unable to get texture translator!");

      delete pRetSurf;
      return NULL;
   }

   HandleCache* pHandleCache = new HandleCache(4096, pRetSurf);
   pRetSurf->_pHandleCache   = pHandleCache;
   
   TextureCache* pTextureCache = new TextureCache(4096, pTexTranslator);
   pRetSurf->_pTextureCache    = pTextureCache;

   // With texture translator in hand, we need now to set up the viewport
   //  for D3D.  Since it has a nicer form, we use the IDirect3DViewport2
   //  interface...
   //
   D3DVIEWPORT2 d3dViewport2;
   
   result = pD3D2Vector[deviceMinorId]->CreateViewport(&pRetSurf->pViewport, NULL);
   AssertWarn(result == D3D_OK, "Could not create viewport");
   if (result != D3D_OK) {
      delete pRetSurf;
      return NULL;
   }
   result = pD3DDevice2Vector[deviceMinorId]->AddViewport(pRetSurf->pViewport);
   AssertWarn(result == D3D_OK, "Could not attach viewport to device");
   if (result != D3D_OK) {
      delete pRetSurf;
      return NULL;
   }

   // Set up the viewport parameters...
   //
   d3dViewport2.dwSize       = sizeof(D3DVIEWPORT2);
   d3dViewport2.dwX          = 0;
   d3dViewport2.dwY          = 0;
   d3dViewport2.dwWidth      = in_width;
   d3dViewport2.dwHeight     = in_height;
   d3dViewport2.dvClipX      = 0.0f;
   d3dViewport2.dvClipY      = 0.0f;
   d3dViewport2.dvClipWidth  = D3DVALUE(in_width);
   d3dViewport2.dvClipHeight = D3DVALUE(in_height);
   d3dViewport2.dvMinZ       = D3DVALUE(0.0f);
   d3dViewport2.dvMaxZ       = D3DVALUE(1.0f);
   
   result = pRetSurf->pViewport->SetViewport2(&d3dViewport2);
   AssertWarn(result == D3D_OK, "Could not set viewport2 params");
   if (result != D3D_OK) {
      delete pRetSurf;
      return NULL;
   }

   // and make it the current viewport for the device...
   //
   result = pD3DDevice2Vector[deviceMinorId]->SetCurrentViewport(pRetSurf->pViewport);
   AssertWarn(result == D3D_OK, "Could not make viewport2 ccurrent on the device");
   if (result != D3D_OK) {
      delete pRetSurf;
      return NULL;
   }

   // Get the renderstate for the surface...
   //
   pRetSurf->setDefaultRenderState();
   pRetSurf->refreshRenderState();

   // For some reason, on at least the 3dfx, if the zbuffer is not cleared
   //  before the first poly is rendered, Bad Things can happen...
   // 
   pRetSurf->clearZBuffer();

   // Set up the clipping rect...
   //
   RectI clipRect;
   clipRect.upperL.x = 0; clipRect.upperL.y = 0;
   clipRect.lowerR.x = in_width;
   clipRect.lowerR.y = in_height;
   pRetSurf->setClipRect(&clipRect);

   // Done and successful!
   //
   return pRetSurf;
}

//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::emitPoly()
//    
// DESCRIPTION 
//    Sends the submitted vertices to the PolyQueue, breaking it up into
//   texture/lightmap components if required...
//
// NOTES 
//------------------------------------------------------------------------------
void 
Surface::emitPoly(const bool  in_bitmapOverride,
                  const DWORD in_bitmapKey)
{
   AssertFatal(numVerticesEmitted >= 3, "Error, not enough vertices...");

   GFXMetrics.incEmittedPolys();

   GFXFillMode useFillMode;
   if (in_bitmapOverride == true)
      useFillMode = GFX_FILL_TEXTURE;
   else
      useFillMode = fillMode;

   // Now add a delayed render request to the buffer, making sure that the
   //  added vertices in pD3DTLVertexBuffer are properly transformed before
   //  finalized in pD3DTLQueueBuffer...
   //
   if (useFillMode == GFX_FILL_TWOPASS) {
      emitTwoPassPoly();
   } else if (useFillMode == GFX_FILL_TEXTURE) {
      HandleCacheEntry* pEntry = _pHandleCache->getCurrEntry();
      DWORD key;
      bool  clamp;
      if (in_bitmapOverride == true) {
         key = in_bitmapKey;
         clamp = true;
      } else {
         key = DWORD(pEntry->bmp);
         clamp = pEntry->clamp;
      }

      emitTexturePoly(key, in_bitmapOverride, clamp);
   } else if (useFillMode == GFX_FILL_CONSTANT) {
      emitConstantPoly();
   } else {
      AssertFatal(0, "Unknown fill mode...");
   }

   // Poly has been queued, we can zero the emitted vertices...
   //
   numVerticesEmitted = 0;
   GFXMetrics.incRenderedPolys();
}


void 
Surface::emitTwoPassPoly()
{
   D3DTLVERTEX lmapVertBuff[MAX_NUM_VERTICES];
   HandleCacheEntry* pEntry = _pHandleCache->getCurrEntry();

   // Copy and Scale vertices...
   //
   memcpy(lmapVertBuff, pD3DTLVertexBuffer, numVerticesEmitted * sizeof(D3DTLVERTEX));
   
   for (int i = 0; i < numVerticesEmitted; i++) {
      pD3DTLVertexBuffer[i].tu *= pEntry->texScale.x;
      pD3DTLVertexBuffer[i].tu += pEntry->texOffset.x;
      pD3DTLVertexBuffer[i].tv *= pEntry->texScale.y;
      pD3DTLVertexBuffer[i].tv += pEntry->texOffset.y;

      lmapVertBuff[i].tu *= pEntry->lMapScale.x;
      lmapVertBuff[i].tu += pEntry->lMapOffset.x;
      lmapVertBuff[i].tv *= pEntry->lMapScale.y;
      lmapVertBuff[i].tv += pEntry->lMapOffset.y;
   }
}


void 
Surface::emitTexturePoly(DWORD      in_key,
                         const bool in_bitmapOverride,
                         const bool in_clamp)
{
   HandleCacheEntry* pEntry = _pHandleCache->getCurrEntry();

   // Scale vertices...
   //
   if (in_bitmapOverride == false) {
      for (int i = 0; i < numVerticesEmitted; i++) {
         pD3DTLVertexBuffer[i].tu *= pEntry->texScale.x;
         pD3DTLVertexBuffer[i].tu += pEntry->texOffset.x;
         pD3DTLVertexBuffer[i].tv *= pEntry->texScale.y;
         pD3DTLVertexBuffer[i].tv += pEntry->texOffset.y;
      }
   }

   // Make sure the texture is set and on the card
   //
   if (_pTextureCache->setCurrentTexture(in_key) == false) {
      // We know that:
      //  textured polys are not callback
      //  texture key is the bitmap pointer...
      //
      if (transparencyEnabled == true) {
         _pTextureCache->downloadTransparentBitmap(this, (GFXBitmap*)in_key, in_key);
      } else if (alphaSource == GFX_ALPHA_TEXTURE) {
         _pTextureCache->downloadAlphaBitmap(this, (GFXBitmap*)in_key, in_key);
      } else {
         pTextureCache->downloadBitmap(this, (GFXBitmap*)in_key, in_key);
      }
   }

   if (shadeSource != GFX_SHADE_NONE)
      setShadeEnable(true, true);
   else
      setShadeEnable(false, true);

   if (hazeSource != GFX_HAZE_NONE)
      setFogEnable(true, true);
   else
      setFogEnable(false, true);

   if (in_clamp == true)
      setWrapEnable(false, true);
   else
      setWrapEnable(true, true);

   setZEnable(isZEnabled, true);

   // If this is a bitmap, draw wo/ filtering
   //
   setFilterEnable(!in_bitmapOverride, true);

   // finalize render settings
   _pTextureCache->activateCurrentTexture(this);

   HRESULT result =
      pD3DDevice2Vector[deviceMinorId]->DrawPrimitive(D3DPT_TRIANGLEFAN,
                                                      D3DVT_TLVERTEX,
                                                      (void*)pD3DTLVertexBuffer,
                                                      numVerticesEmitted,
                                                      D3DDP_DONOTCLIP);
   AssertWarn(result == D3D_OK, avar("Problem emitting polygon: %s",
                                     D3DAppErrorToString(result)));
}


void 
Surface::emitConstantPoly()
{
   if (shadeSource != GFX_SHADE_NONE)
      setShadeEnable(true, true);
   else
      setShadeEnable(false, true);

   if (hazeSource != GFX_HAZE_NONE)
      setFogEnable(true, true);
   else
      setFogEnable(false, true);
   
   if (alphaSource == GFX_ALPHA_CONSTANT ||
       alphaSource == GFX_ALPHA_FILL) {
      setRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA);
      setRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
      setRenderState(D3DRENDERSTATE_BLENDENABLE,      true);
      setRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
   } else {
      setRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
      setRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
      setRenderState(D3DRENDERSTATE_BLENDENABLE,      false);
      setRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
   }

   setZEnable(isZEnabled, true);
   setRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL);
   HRESULT result =
      pD3DDevice2Vector[deviceMinorId]->DrawPrimitive(D3DPT_TRIANGLEFAN,
                                                      D3DVT_TLVERTEX,
                                                      (void*)pD3DTLVertexBuffer,
                                                      numVerticesEmitted,
                                                      D3DDP_DONOTCLIP);
   AssertWarn(result == D3D_OK, avar("Problem emitting polygon: %s",
                                     D3DAppErrorToString(result)));
}

void 
Surface::emitLine()
{
   setRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL);
   HRESULT result =
      pD3DDevice2Vector[deviceMinorId]->DrawPrimitive(D3DPT_LINELIST,
                                                      D3DVT_TLVERTEX,
                                                      (void*)pD3DTLVertexBuffer, 2,
                                                      D3DDP_DONOTCLIP);
   AssertWarn(result == D3D_OK, avar("Problem emitting line: %s",
                                     D3DAppErrorToString(result)));

   // Point has been queued, we can zero the emitted vertices...
   //
   numVerticesEmitted = 0;
   GFXMetrics.incRenderedPolys();
}

void 
Surface::emitPoint()
{
   setRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL);
   HRESULT result =
      pD3DDevice2Vector[deviceMinorId]->DrawPrimitive(D3DPT_POINTLIST,
                                                      D3DVT_TLVERTEX,
                                                      (void*)pD3DTLVertexBuffer, 1,
                                                      D3DDP_DONOTCLIP);
   AssertWarn(result == D3D_OK, avar("Problem emitting line: %s",
                                     D3DAppErrorToString(result)));

   // Point has been queued, we can zero the emitted vertices...
   //
   numVerticesEmitted = 0;
   GFXMetrics.incRenderedPolys();
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::clearScreen(GFXSurface *io_pSurface, DWORD in_color)
//    
// DESCRIPTION 
//    Simple (hah!) clearscreen function.  Must regenerate the D3DMaterial
//   used to clear the screen if the previous clear was a different color...
//    
// NOTES 
//    Does NOT clear the ZBuffer currently...
//------------------------------------------------------------------------------
void 
Surface::clearScreen(DWORD in_color)
{
   AssertFatal(pPalette != NULL, "Error, no palette on surface!");
   AssertFatal(in_color >= 0 && in_color <= 255,
               "Error, invalid color in ClearScreen");
   AssertFatal(pD3D2Vector[deviceMinorId] != NULL, "Error, no IDirect3D2");
   AssertFatal(pD3DDevice2Vector[deviceMinorId] != NULL,
               "Error, no IDirect3DDevice2");
   AssertFatal(pViewport != NULL, "Error, no viewport to clear...");


   if (!((memcmp(&pPalette->color[in_color], &currBGColor, sizeof(PALETTEENTRY)) == 0) &&
         pBGMaterial != NULL)) {
      // Need to regen the background material...
      //
      IDirect3DMaterial2 *pTempMaterial = NULL;
      D3DMATERIAL         tempMat;

      PALETTEENTRY* pBGColor = &pPalette->color[in_color];
      D3DCOLORVALUE d3dCVal;

      if (pBGMaterial != NULL) {
         // Get rid of any old bg material...
         //
         pBGMaterial->Release();
         pBGMaterial = NULL;
         hBGMaterial = NULL;
      }
      
      memcpy(&currBGColor, &pPalette->color[in_color],
             sizeof(PALETTEENTRY));

      d3dCVal.r = D3DVALUE(pBGColor->peRed)   / D3DVALUE(255);
      d3dCVal.g = D3DVALUE(pBGColor->peGreen) / D3DVALUE(255);
      d3dCVal.b = D3DVALUE(pBGColor->peBlue)  / D3DVALUE(255);
      d3dCVal.a = D3DVALUE(0.0);
      
      memset(&tempMat, 0, sizeof(D3DMATERIAL));

      tempMat.dwSize  = sizeof(D3DMATERIAL);
      tempMat.diffuse = d3dCVal;
      
      // Create the material...
      //
      HRESULT result = pD3D2Vector[deviceMinorId]->CreateMaterial(&pTempMaterial,
                                                                  NULL);
      AssertWarn(result == D3D_OK, "Error, could not create new bg material");
      if (result != D3D_OK)
         return;

      pBGMaterial = pTempMaterial;
      pBGMaterial->SetMaterial(&tempMat);
      
      result = pBGMaterial->GetHandle(pD3DDevice2Vector[deviceMinorId],
                                      &hBGMaterial);
      AssertWarn(result == D3D_OK, "Could not get handle for bg material");
      if (result != D3D_OK) {
         pBGMaterial->Release();
         pBGMaterial = NULL;
         return;
      }

      result = pViewport->SetBackground(hBGMaterial);
      AssertWarn(result == D3D_OK, "Could not set bg handle for viewport");
      if (result != D3D_OK) {
         pBGMaterial->Release();
         pBGMaterial = NULL;
         hBGMaterial = NULL;
         return;
      }
   }

   // Create a dummy clear rect, and clear the screen...
   //
   D3DRECT dummyRect;
   dummyRect.x1 = 0;
   dummyRect.y1 = 0;
   dummyRect.x2 = surfaceWidth;
   dummyRect.y2 = surfaceHeight;
   
   pViewport->Clear(1, &dummyRect, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::clearZBuffer()
//    
// ARGUMENTS 
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
Surface::clearZBuffer()
{
   AssertFatal(pViewport != NULL, "Error, cannot clear without a viewport...");

   D3DRECT dummyRect;

   dummyRect.x1 = 0;
   dummyRect.y1 = 0;
   dummyRect.x2 = surfaceWidth - 1;
   dummyRect.y2 = surfaceHeight - 1;
   
   HRESULT result = pViewport->Clear(1, &dummyRect, D3DCLEAR_ZBUFFER);
   AssertWarn(result == D3D_OK, avar("Error clearing ZBuffer: %s",
                                     D3DAppErrorToString(result)));
}


//------------------------------------------------------------------------------
// NAME 
//    Int32 Surface::getNumDevices()
//    
// ARGUMENTS 
//    
// NOTES 
//    
//------------------------------------------------------------------------------
Int32 
Surface::getNumDevices()
{
   AssertFatal(initialized == true, "Error getNumDevices called before Direct3D::Surface::init()");
   
   // Number of devices is the number of non-NULL pointers in
   //  the pD3D2Vector...
   //
   int numDevices = 0;
   for (int i = 0; i < pD3D2Vector.size(); i++) {
      if ((pD3D2Vector[i] != NULL) && (D3DDevDescVector[i].valid == true))
         numDevices++;
   }
   
   return numDevices;
}


//------------------------------------------------------------------------------
// NAME 
//    bool 
//    Surface::setDisplayMode(Int32 in_width, Int32 in_height, Int32 in_bitDepth)
//    
// ARGUMENTS 
//    
// NOTES 
//    
//------------------------------------------------------------------------------
bool 
Surface::setDisplayMode(Int32 in_width, Int32 in_height,
                        Int32 in_bitDepth, Int32 in_deviceMinorId)
{
   HRESULT result;

   AssertFatal(D3DSurfaceHWnd,
               "Error, called D3D::Surface::setDisplayMode w/ bad HWND");

   if (cooperativeLevelSet == false) {
      
      result = pDD2Vector[in_deviceMinorId]->SetCooperativeLevel(D3DSurfaceHWnd,
                                                                 DDSCL_NOWINDOWCHANGES | 
                                                                 DDSCL_EXCLUSIVE | 
                                                                 DDSCL_FULLSCREEN | 
                                                                 DDSCL_ALLOWMODEX | 
                                                                 DDSCL_ALLOWREBOOT); 
      AssertWarn(result == DD_OK, "Error setting CoopLevel in Direct3D::Surface::setDisplayMode");
      if (result != DD_OK)
         return false;
      
      cooperativeLevelSet = true;
   };
   
	result = pDD2Vector[in_deviceMinorId]->SetDisplayMode(in_width, in_height,
                                                         in_bitDepth, 0, 0);
	AssertWarn(result == DD_OK,
              "Direct3D::Surface::setDisplayMode: Error setting display mode");
	if ( result != DD_OK )
      return false;
   
   modeWidth  = in_width;
   modeHeight = in_height;
   modeDepth  = in_bitDepth;
   
   return true;
}


void 
Surface::processMessage(const int   /*in_argc*/,
                        const char* in_argv[])
{
   if (strcmp(in_argv[0], "lightMaps") == 0) {
      lightMapsEnabled = !lightMapsEnabled;
   } else if (strcmp(in_argv[0], "flushCache") == 0) {
      flushTextureCache();
   }
   
}


//------------------------------------------------------------------------------
// NAME 
//    enumDDCallBack(LPGUID lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName)
//    
// DESCRIPTION 
//    Class function to handle enumeration of DirectDraw Devices.
//    
// ARGUMENTS 
//    lpGUID              - GUID of DirectDraw Device
//    lpDriverDescription - obv.
//    lpDriverName        - obv.
//    
// RETURNS 
//    DDENUMRET_OK     - Continue enumerating DD devices
//    DDENUMRET_CANCEL - Stop enumerating devices
//    
// NOTES 
//------------------------------------------------------------------------------
BOOL 
Surface::enumDDCallback(LPGUID lpGUID,
                        LPSTR /*lpDriverDescription*/,
                        LPSTR lpDriverName)
{
   HRESULT result;
   DDCAPS  ddHWCaps;
   DDCAPS  ddHELCaps;
   
   AssertFatal(g_lpDirectDrawCreate != NULL, "Error, bad function pointer in enumDDCallback");

   IDirectDraw*   pDD  = NULL;
   IDirectDraw2*  pDD2 = NULL;
   
   // Get the IDirectDraw interface for the GUID
   //
   result = g_lpDirectDrawCreate(lpGUID, &pDD, NULL);
   if (result != DD_OK) {
      AssertWarn(0, "DirectDrawCreate Failed");
      return DDENUMRET_OK;
   }

   // From this, get the IDD2 interface
   result = pDD->QueryInterface(IID_IDirectDraw2, (LPVOID *)&pDD2);
   if (result != DD_OK) {
      AssertWarn(0, "Failed to obtain IDirectDraw2 interface");
      pDD->Release();

      return DDENUMRET_OK;
   }
   
   // Ok, got both of what we need, check to see if these support a Direct3D
   // device...
   //
   ddHWCaps.dwSize  = sizeof(DDCAPS);
   ddHELCaps.dwSize = sizeof(DDCAPS);
   pDD2->GetCaps(&ddHWCaps, &ddHELCaps);
   
   if (!(ddHWCaps.dwCaps & DDCAPS_3D)) {
      // DirectDraw device is not 3d accelerated
      //
      pDD2->Release();
      pDD->Release();
      return DDENUMRET_OK;
   }

   // Ok, driver has a 3d accelerator, place it in the list (We no longer need
   //  the IDirectDraw* interface...)
   //
   char *ddName = new char[strlen("D3D-") + strlen(lpDriverName) + 2];
   strcpy(ddName, "D3D-");
   strcat(ddName, lpDriverName);

   pDD->Release();
   pDD2Vector.push_back(pDD2);
   pDDNameVector.push_back(ddName);

   // And Continue Enumerating...
   //
   return DDENUMRET_OK;
}



//------------------------------------------------------------------------------
// NAME 
//    enumD3DCallBack()
//    
// DESCRIPTION 
//    Class function to handle enumeration of Direct3D Devices.
//    
// ARGUMENTS 
//    lpGUID              - GUID of DirectDraw Device
//    lpDriverDescription - obv.
//    lpDriverName        - obv.
//    lpD3DHWDeviceDesc   - description of hardware caps of device
//    lpD3DHELDeviceDesc  - description of software ""
//    lpUserArg           - Address of a D3DDeviceDescriptor structure
//                           in which to place information about the device
//    
// RETURNS 
//    DDENUMRET_OK     - Continue enumerating D3D devices
//    DDENUMRET_CANCEL - Stop enumerating devices
//    
// NOTES
//    Checking the HWDevDesc for COLOR_RGB seems to be a poor way of checking
//   if the described device is hardware, but MS's example code seems to prefer
//   it.  Maybe a better way is possible?
//
//    --=-- GFX 3D Card Minimum Requirements encoded below --=--
//     - ZBuffer Bit depth of at least 16 bits
//     - ZBuffer Less/Equal Comparison (Two pass texturing requires LE cmp)
//     - Vertex Fogging
//     - Gouraud Fogging
//     - Alpha blending (INVSRCALPHA?)
//     - Gouraud Shading and interpolation of RGB lighting
//     - Perspective, alpha, and transparent texturing
//     - Two pass texturing ability
//------------------------------------------------------------------------------
inline void 
debugBadCard(const char *in_pError)
{
#ifdef DEBUG_BAD_CARD
   AssertWarn(0, in_pError);
#endif
}

HRESULT 
Surface::enumD3DCallback(LPGUID          lpGUID,
                         LPSTR           lpDeviceDescription,
                         LPSTR           lpDeviceName,
                         LPD3DDEVICEDESC lpD3DHWDeviceDesc,
                         LPD3DDEVICEDESC /*lpD3DHELDeviceDesc*/,
                         LPVOID          lpUserArg)
{
   D3DDeviceDescriptor* pDevDesc = (D3DDeviceDescriptor*)lpUserArg;

   if (lpD3DHWDeviceDesc->dcmColorModel == D3DCOLOR_RGB) {
      // Device is hardware, check for minimum standards.
      //
      debugBadCard("Found hardware card, beginning capability scan...");
      debugBadCard(lpDeviceDescription);
      debugBadCard(lpDeviceName);
      bool success = true;
      if ((lpD3DHWDeviceDesc->dwFlags & D3DDD_DEVICEZBUFFERBITDEPTH) == 0) {
         // device has no zBuffer...
         AssertWarn(0, "No ZBuffer  (Fatal)");
         success = false;
      }
      
      if ((lpD3DHWDeviceDesc->dwDeviceZBufferBitDepth != DDBD_16) &&
          (lpD3DHWDeviceDesc->dwDeviceZBufferBitDepth != DDBD_24) &&
          (lpD3DHWDeviceDesc->dwDeviceZBufferBitDepth != DDBD_32)) {
         // device's zbuffer does not meet minimum requirements
         debugBadCard("Insufficient ZBuffer bit depth  (Fatal)");
         success = false;
      }

      if ((lpD3DHWDeviceDesc->dpcTriCaps.dwZCmpCaps & D3DPCMPCAPS_LESSEQUAL) == 0) {
         // device cannot perform proper zbuf'ing
         debugBadCard("Does not have Less/Equal Z Comparison  (Fatal)");
         success = false;
      }

      if ((lpD3DHWDeviceDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX) == 0) {
         // device does not support vertex fogging
         debugBadCard("No vertex fogging  (Fatal)");
         success = false;
      }
      
      if ((lpD3DHWDeviceDesc->dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_FOGGOURAUD) == 0) {
         // no fogging caps
         debugBadCard("Does not gouraud interpolate fog  (Fatal)");
         success = false;
      }
      
      if (((lpD3DHWDeviceDesc->dpcTriCaps.dwSrcBlendCaps  & D3DPBLENDCAPS_SRCALPHA)    == 0) ||
          ((lpD3DHWDeviceDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) == 0)) {
         if (((lpD3DHWDeviceDesc->dpcTriCaps.dwSrcBlendCaps  & D3DPBLENDCAPS_INVSRCALPHA) == 0) ||
             ((lpD3DHWDeviceDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCALPHA)    == 0)) {
            debugBadCard("Card has backwards alpha, still usable  (Warning)");
            pDevDesc->driverAlphaInverted = true;
         } else {
            // device is alpha unaware
            debugBadCard("Lacks proper alpha support (Source: SRCALPHA && Dest: INVSRCALPHA)  (Fatal)");
            success = false;
         }
      } else {
         pDevDesc->driverAlphaInverted = false;
      }
      
      if (pDevDesc->driverAlphaInverted == false) {
         if ((lpD3DHWDeviceDesc->dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_LESSEQUAL) == 0) {
            debugBadCard("Card cannot do alpha cmp, transparencies will probably smudge  (Warning)");
            pDevDesc->driverAlphaCmp = false;
         } else {
            pDevDesc->driverAlphaCmp = true;
         }
      } else {
         if ((lpD3DHWDeviceDesc->dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL) == 0) {
            debugBadCard("Card cannot do alpha cmp, transparencies will probably smudge  (Warning)");
            pDevDesc->driverAlphaCmp = false;
         } else {
            pDevDesc->driverAlphaCmp = true;
         }
      }

      if ((lpD3DHWDeviceDesc->dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB) == 0) {
         // device cannot do gouraud shading...
         debugBadCard("Cannot gouraud interpolate vertex lighting  (Fatal)");
         success = false;
      }
      
      UInt32 mipModes = D3DPTFILTERCAPS_MIPLINEAR        | 
                        D3DPTFILTERCAPS_LINEARMIPNEAREST |
                        D3DPTFILTERCAPS_LINEARMIPLINEAR  |
                        D3DPTFILTERCAPS_MIPNEAREST;
      if ((lpD3DHWDeviceDesc->dpcTriCaps.dwTextureFilterCaps & mipModes) != 0) {
         pDevDesc->canDoMipmaps = true;

         // We need to set the filter mode we use in the driver, in order of
         //  preference.  Currently we choose the best mode available, probably
         //  there needs to be some clean way to make this user-selectable...
         // DMMNOTE DMMALARM!
         //
//         DWORD filterCaps = lpD3DHWDeviceDesc->dpcTriCaps.dwTextureFilterCaps;
//         if ((filterCaps & D3DPTFILTERCAPS_LINEARMIPLINEAR) != 0) {
//            // Tri-linear filtering...
//            //
//            pDevDesc->filterMode = D3DFILTER_LINEARMIPLINEAR;
//         } else if ((filterCaps & D3DPTFILTERCAPS_LINEARMIPNEAREST) != 0) {
//            // Bilinear filtering on the best available mipmap
//            //
//            pDevDesc->filterMode = D3DFILTER_LINEARMIPNEAREST;
//         }
//         else if ((filterCaps & D3DPTFILTERCAPS_MIPLINEAR) != 0) {
//            // Weighted average of the closest two mipmaps
//            //
//            pDevDesc->filterMode = D3DFILTER_MIPLINEAR;
//         }
//         else {
//            // Nearest texel in best mipmap
//            //
//            pDevDesc->filterMode = D3DFILTER_MIPNEAREST;
//         }
         pDevDesc->filterMode   = D3DFILTER_LINEAR;
      } else {
         debugBadCard("Cannot mipmap: still usable  (Warning)");
         pDevDesc->canDoMipmaps = false;

         // If the card can't mip, at least it should be bilinear capable, which
         //  we require for two-pass texturing
         pDevDesc->filterMode   = D3DFILTER_LINEAR;
      }

      DWORD reqTexCaps = D3DPTEXTURECAPS_ALPHA       |
                         D3DPTEXTURECAPS_PERSPECTIVE;
      if ((lpD3DHWDeviceDesc->dpcTriCaps.dwTextureCaps & reqTexCaps) != reqTexCaps) {
         // device does not support minimum texturing requrements
         debugBadCard("Missing Alpha or perspective correction  (Fatal)");
         success = false;
      }

      if ((lpD3DHWDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY) == 0) {
         debugBadCard("Does not support color keying: attempting to emulate w/ alpha surface  (Warning)");
         pDevDesc->canDoColorKey = false;
      } else {
         pDevDesc->canDoColorKey = true;
      }
      
      if ((lpD3DHWDeviceDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATE) == 0) {
         debugBadCard("Inadequate shading support: D3DPTFILTERCAPS_MODULATE  (Fatal)");
         success = false;
      }

      // Currently requiring two-pass texturing ability from cards. set bool
      //  in DevDescriptor structure to indicate if device is capable. May be
      //  desirable to add support for cards that cannot do this later?
      // Note: these logical ops are a nightmare to read, clean this up if
      //  possible DMMNOTE
      //
      if ((((lpD3DHWDeviceDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR)    != 0) ||
           ((lpD3DHWDeviceDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR) != 0)) &&
          ((lpD3DHWDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) != 0)) {

         pDevDesc->canDoTwoPass = true;
         if ((lpD3DHWDeviceDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR) != 0) {
            pDevDesc->colorLightMaps   = true;
            pDevDesc->lightMapMode     = D3DBLEND_SRCCOLOR;
            pDevDesc->lightMapInverted = false;
         } else {
            pDevDesc->colorLightMaps   = true;
            pDevDesc->lightMapMode     = D3DBLEND_INVSRCCOLOR;
            pDevDesc->lightMapInverted = true;
         }
      } else {
         debugBadCard("No color two-pass texturing  (Warning)");
        
         if ((((lpD3DHWDeviceDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCALPHA)    != 0) ||
              ((lpD3DHWDeviceDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) != 0)) &&
             ((lpD3DHWDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) != 0)) {

            debugBadCard("Card can support only mono lightmaps  (Warning)");
            pDevDesc->canDoTwoPass = true;
            if ((lpD3DHWDeviceDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR) != 0) {
               pDevDesc->colorLightMaps   = false;
               pDevDesc->lightMapMode     = D3DBLEND_SRCALPHA;
               pDevDesc->lightMapInverted = false;
            } else {
               pDevDesc->colorLightMaps   = false;
               pDevDesc->lightMapMode     = D3DBLEND_INVSRCALPHA;
               pDevDesc->lightMapInverted = true;
            }
         } else {
            debugBadCard("Card cannot support mono lightmaps, Should support?  (Fatal)");
            pDevDesc->canDoTwoPass = false;
            success = false;
         }
      }
      
      // If we missed for any reason above, keep enumerating
      //
      if (success == false) {
         debugBadCard("Unusable card");

         // Prevent name from being deleted on device shutdown...
         //
         pDevDesc->devName = NULL;
         return D3DENUMRET_OK;
      } else {
         debugBadCard("Usable card");
      }

      // If we're here, then the device passed our requirements.  Mark it as
      //  valid, and set up the description.
      //
      pDevDesc->valid      = true;
      pDevDesc->deviceGUID = *lpGUID;
      memcpy(&pDevDesc->devDesc, lpD3DHWDeviceDesc, sizeof(D3DDEVICEDESC));
      
      // Set up the name of the device.  Note that no spaces are currently
      //  allowed...
      //
      Int32 nameLen = strlen(pDevDesc->devName) + 1;
      for (int i = 0; i < nameLen; i++) {
         if (pDevDesc->devName[i] == ' ')
            pDevDesc->devName[i] = '-';
      }
      
      //  We support only the first D3DDevice attached to each Direct3D
      // interface for now.  If we found a valid device above, we're done
      // enumerating.
      //
      return D3DENUMRET_CANCEL;
   } else {
      // Device is software, keep looking
      //
      return D3DENUMRET_OK;
   }
}


}; // namespace Direct3D
