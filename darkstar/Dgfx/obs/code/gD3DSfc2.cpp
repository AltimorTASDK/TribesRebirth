//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "fn_table.h"
#include "gdmanag.h"
#include "gfxmetrics.h"
#include "d_caps.h"

#include "gD3DSfc.h"
#include "gD3DTxTrans.h"
#include "gD3DTxCache.h"
#include "gD3DHCache.h"
#include "gD3DPQueue.h"

// Convert hresult into string...
char* D3DAppErrorToString(HRESULT error);

// We want to be able to step thru utility functions in debug mode...
//
#ifndef DEBUG
#define GFXD3DINLINE inline
#else
#define GFXD3DINLINE
#endif

namespace Direct3D {

//------------------------------------------------------------------------------
// NAME 
//    ddEnumModesCallback(LPDDSURFACEDESC lpSurfaceDesc, LPVOID lpContext)
//    
// DESCRIPTION 
//    Simply places the supported modes into the passed resolution vector
//    
// ARGUMENTS 
//    lpSurfaceDesc - obv.
//    lpContext     - modeDescription vector to place surface desc in.
//
// NOTES 
//    
//------------------------------------------------------------------------------
HRESULT WINAPI
ddEnumModesCallback(LPDDSURFACEDESC lpSurfaceDesc, LPVOID lpContext)
{
   AssertFatal(lpContext != NULL, "Bad context passed to ddEnumModesCallback");
   
   Vector<modeDescription>* pResVector = (Vector<modeDescription>*)lpContext;
   
   Int32 height = lpSurfaceDesc->dwHeight;
   Int32 width  = lpSurfaceDesc->dwWidth;
   
   Int32 bitDepth = lpSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
   
   // Only take modes that are 16-bit for now...
   if (bitDepth == 16) {
      modeDescription mode;

      mode.res = Point2I(width, height);
      mode.bpp = bitDepth;
      memcpy(&mode.ddSDesc, lpSurfaceDesc, sizeof(DDSURFACEDESC));
      
      pResVector->push_back(mode);
   }
   
   return DDENUMRET_OK;
}



void 
Surface::drawSurfaceToBitmap(GFXBitmap* io_pBitmap)
{
   AssertFatal(pDDFront != NULL, "Error, no buffer to dump...");
   if(io_pBitmap->getWidth() != surfaceWidth ||
      io_pBitmap->getHeight() != surfaceHeight ||
      io_pBitmap->bitDepth != 24)
      return;

   DDSURFACEDESC ddsdTemp;
   ddsdTemp.dwSize = sizeof(DDSURFACEDESC);
   pDDFront->Lock(NULL, &ddsdTemp, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

   for (unsigned int h = 0; h < ddsdTemp.dwHeight; h++) {
      UInt16 *pSrc = (UInt16*)(((UInt8*)ddsdTemp.lpSurface) + (h*ddsdTemp.lPitch));
      for (unsigned int w = 0; w < ddsdTemp.dwWidth; w++) {
         UInt16 src = pSrc[w];

         io_pBitmap->pBits[(h*surfaceWidth + w)*3 + 0] = (src & 0x1f) << 3;
         io_pBitmap->pBits[(h*surfaceWidth + w)*3 + 1] = ((src >> 5) & 0x3f) << 2;
         io_pBitmap->pBits[(h*surfaceWidth + w)*3 + 2] = (src >> 11) << 3;
      }
   }
   pDDFront->Unlock(ddsdTemp.lpSurface);
}

//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::flushTextureCache()
//    
// DESCRIPTION 
//    obv.  Flushes handle cache too...
//    
//------------------------------------------------------------------------------
void 
Surface::flushTextureCache()
{
   AssertFatal(_pTextureCache != NULL, "Error, no texture cache attached to surface");
   AssertFatal(_pHandleCache != NULL, "Error, no handle cache attached to surface");

   _pTextureCache->flush();
   _pHandleCache->flush();
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::_setGamma()
//    
// DESCRIPTION 
//    Obv.
//    
// NOTES 
//    DMMNOTE In this driver, nothing is done with this call.  Change?
//------------------------------------------------------------------------------
void 
Surface::_setGamma()
{
   // What to do with this?
   //
   AssertWarn(0, "Confusing function Direct3D::Surface::_setGamma() called");
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::_lockDC()
//    void 
//    Surface::_unlockDC()
//    
// DESCRIPTION 
//    NOT used in Direct3D, there is never a call to lock the Windows Device
//   Context.  This should probably be an AssertFatal offense...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
Surface::_lockDC()
{
   AssertWarn(0, "Direct3D::Surface::_lockDC not implemented");
}


void 
Surface::_unlockDC()
{
   AssertWarn(0, "Direct3D::Surface::_unlockDC not implemented");
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::drawSurface(GFXSurface*    /*lpSourceSurface*/,
//                         const RectI*   /*in_subRegion*/,
//                         const Point2I* /*in_at*/)
//    
// DESCRIPTION 
//    Not implemented for hardware cards...
//    
//------------------------------------------------------------------------------
void 
Surface::drawSurface(GFXSurface*    /*lpSourceSurface*/,
                     const RectI*   /*in_subRegion*/,
                     const Point2I* /*in_at*/)
{
   AssertWarn(0, "DrawSurface not implemented in hardware");
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::draw3DBegin()
//    void 
//    Surface::draw3DEnd()
//    
// DESCRIPTION 
//    See functions in gD3DFn.cpp
//    
//------------------------------------------------------------------------------
void 
Surface::draw3DBegin()
{
   AssertFatal(draw3DBegun == false, "Error mismatched calls");
   AssertFatal(pD3DDevice2Vector[deviceMinorId] != NULL, "No device in draw3dbegin");

   draw3DBegun = true;
   GFXMetrics.reset();
#ifdef DEBUG
   GFXMetrics.textureSpaceUsed       = 0;
   GFXMetrics.textureBytesDownloaded = 0;
#endif
}

void 
Surface::draw3DEnd()
{
   AssertFatal(draw3DBegun == true, "Error mismatched 3d end calls");
   AssertFatal(pD3DDevice2Vector[deviceMinorId] != NULL, "No device in draw3dend");

   draw3DBegun = false;
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::setColorKey(const bool in_enabled)
//    
// DESCRIPTION 
//    Enable the color key on the surface if flag is true, disable otherwise...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
Surface::setColorKey(const bool in_enabled, const bool in_hardenState)
{
   if (in_hardenState == true)
      setRenderState(D3DRENDERSTATE_COLORKEYENABLE, in_enabled);
   else
      transparencyEnabled = in_enabled;
}


void 
Surface::setZEnable(const int in_state, const bool in_hardenState)
{
   if (in_hardenState == true) {
      if (in_state == GFX_NO_ZTEST) {
         setRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
         setRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
      } else if (in_state == GFX_ZTEST_AND_WRITE) {
         setRenderState(D3DRENDERSTATE_ZENABLE,      TRUE);
         setRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
         setRenderState(D3DRENDERSTATE_ZFUNC,        D3DCMP_GREATEREQUAL);
      } else if (in_state == GFX_ZTEST) {
         setRenderState(D3DRENDERSTATE_ZENABLE,      TRUE);
         setRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
         setRenderState(D3DRENDERSTATE_ZFUNC,        D3DCMP_GREATEREQUAL);
      } else { // GFX_ZWRITE
         setRenderState(D3DRENDERSTATE_ZENABLE,      TRUE);
         setRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
         setRenderState(D3DRENDERSTATE_ZFUNC,        D3DCMP_ALWAYS);
      }
   } else {
      isZEnabled = in_state;
   }
}


void 
Surface::setFogEnable(const bool in_enabled, const bool in_hardenState)
{
   if (in_hardenState == true)
      setRenderState(D3DRENDERSTATE_FOGENABLE, in_enabled);
   else
      isFogEnabled = in_enabled;
}

void 
Surface::setFilterEnable(const bool in_enabled, const bool in_hardenState)
{
   
   if (in_hardenState == true) {
      DWORD filterMode = (in_enabled == true) ? D3DFILTER_LINEAR : D3DFILTER_NEAREST;
      setRenderState(D3DRENDERSTATE_TEXTUREMAG, filterMode);
      setRenderState(D3DRENDERSTATE_TEXTUREMIN, filterMode);
   }
}


void 
Surface::setWrapEnable(const bool in_enabled, const bool in_hardenState)
{
   if (in_hardenState == true) {
      if (in_enabled == true) {
         setRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
      } else {
         setRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP);
      }
   } else {
      isWrapEnabled = in_enabled;
   }
}


void 
Surface::setShadeEnable(const bool in_enabled, const bool in_hardenState)
{
   if (in_hardenState == true) {
      if (in_enabled == true) {
         setRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);
      } else {
         setRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECAL);
      }
   } else {
      isShadeEnabled = in_enabled;
   }
}


void 
Surface::setTextureMap(const GFXBitmap* in_pTexture)
{
   // Because we are queueing all polys, we need to map this call into a
   //  registerTexture call with the handle cache...
   //
   AssertFatal(pTextureCache != NULL, "Error, surface not locked...");
   AssertFatal(pHandleCache != NULL, "Error, surface not locked...");

   GFXTextureHandle texHandle;
   texHandle.key[0] = DWORD(in_pTexture);
   texHandle.key[1] = 0;

   if (_pHandleCache->setTextureHandle(texHandle) == true) {
      // already registered, and now it's set...
      //
      return;
   }
  
   // Otherwise, we need to register the texture, which will also leave it
   //  set as the curreny texture...
   //
   registerTexture(texHandle,
                   in_pTexture->getWidth(), in_pTexture->getHeight(),
                   0, 0,
                   0, NULL,
                   in_pTexture, 1);
}


//------------------------------------------------------------------------------
// NAME 
//    bool Surface::describeDevice(int deviceNumber, GFXDeviceDescriptor *dev)
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
inline DWORD
vidMemRequired(const Point2I& in_size)
{
   // 16 bit, one back buf, and zBuf.
   return (in_size.x * in_size.y * 2 * 3);
}

bool 
Surface::describeDevice(int deviceNumber, GFXDeviceDescriptor *dev)
{
   AssertFatal(initialized == true, "Error describeDevice called before Direct3D::Surface::init()");

   // Some trickiness here.  deviceNumber actually refers to the n_th, _valid_
   //  device, rather than the device with deviceMinorId = deviceNumber...
   //
   int numValidDevices = 0;
   int deviceMinorId = -1;
   for (int i = 0; i < pD3D2Vector.size(); i++) {
      if ((pD3D2Vector[i] != NULL) && (D3DDevDescVector[i].valid == true))
         numValidDevices++;
         
      if ((numValidDevices - 1) == deviceNumber) {
         deviceMinorId = i;
         break;
      }
   }
   AssertWarn(deviceMinorId != -1, avar("Error, unable to find specified device: %d size: %d",
                                        deviceNumber, pD3D2Vector.size()));
   if (deviceMinorId == -1)
      return false;
      
   // Ok, we have the correct minorId, fill in the dev structure...
   //
   dev->flags         = GFXDeviceDescriptor::runsFullscreen;  // D3D Devices always run fullscreen for now
   dev->deviceId      = GFX_D3D_DEVICE;
   dev->deviceMinorId = deviceMinorId;
   dev->driverLibrary = NULL;
   dev->name          = D3DDevDescVector[deviceMinorId].devName;
   
   // Place the resolutions into the devices resolution list...
   getDeviceResolutions(deviceMinorId, DevResVector[deviceMinorId]);

   // If resVector's size is 0, then no valid modes were found
   if (DevResVector[deviceMinorId]->size() == 0) {
      return false;
   }
   
   // Place the resolutions into the device descriptor list.  Note: we don't sort
   //  them, this is handled by the GFXD3DDevice...
   //
   int k = 0;
   DWORD totalVidMem = getTotalSurfaceMemory(deviceNumber);
   dev->resolutionList = new Point2I[DevResVector[deviceMinorId]->size()];
   for (int j = 0; j < DevResVector[deviceMinorId]->size(); j++) {
      if (vidMemRequired((*DevResVector[deviceMinorId])[j].res) <= totalVidMem) {
         dev->resolutionList[k] = (*DevResVector[deviceMinorId])[j].res;
         k++;
      }
   }
   dev->resolutionCount = k;
   
   // Ok, the device is fully described, and if we're here, it's also valid
   //
   return true;
}


void 
Surface::getDeviceResolutions(Int32 in_deviceMinorId, Vector<modeDescription>* resVector)
{
   // Get the display modes associated with this device.  We require:
   // 16bit, Z-buffered, back-buffered, 3d accelerated
   DDSURFACEDESC ddSurfDesc;
   ddSurfDesc.dwSize  = sizeof(DDSURFACEDESC);
   
   ddSurfDesc.dwFlags = DDSD_BACKBUFFERCOUNT |
                        DDSD_CAPS;

   ddSurfDesc.dwBackBufferCount = 2;  // Require a back buffer

   ddSurfDesc.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | // Require 3d Acceleration
                               DDSCAPS_FLIP;      // Require flipping surface
   
   resVector->clear();
   pDD2Vector[in_deviceMinorId]->EnumDisplayModes(0, &ddSurfDesc,
                                                  (LPVOID)resVector,
                                                  ddEnumModesCallback);
}


//------------------------------------------------------------------------------
// NAME 
//    DWORD 
//    Surface::getCaps()
//    
// DESCRIPTION 
//    
// NOTES 
//    
//------------------------------------------------------------------------------
DWORD 
Surface::getCaps()
{
   return GFX_DEVCAP_SUPPORTS_CONST_ALPHA;
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::setRenderState(DWORD in_stateType, DWORD in_state)
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
GFXD3DINLINE void 
Surface::_setRenderState(const DWORD in_stateType,
                         const DWORD in_state,
                         DWORD* io_pCurrState)
{
   if (*io_pCurrState != in_state) {
      HRESULT result =
         pD3DDevice2Vector[deviceMinorId]->SetRenderState(in_stateType, in_state);
      AssertWarn(result == D3D_OK, "Unable to set renderstate...");

      if (result == D3D_OK)
         *io_pCurrState = in_state;
   }
}


void 
Surface::setRenderState(const DWORD in_stateType, const DWORD in_state)
{
   AssertFatal(pD3DDevice2Vector[deviceMinorId] != NULL,
               "No device to set renderstate to...");

   switch (in_stateType) {
     case D3DRENDERSTATE_TEXTUREHANDLE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.textureHandle);
      break;
     case D3DRENDERSTATE_TEXTUREADDRESS:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.textureAddress);
      break;
     case D3DRENDERSTATE_TEXTUREPERSPECTIVE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.texturePerspective);
      break;
     case D3DRENDERSTATE_WRAPU:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.wrapU);
      break;
     case D3DRENDERSTATE_WRAPV:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.wrapV);
      break;
     case D3DRENDERSTATE_ZENABLE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.zEnable);
      break;
     case D3DRENDERSTATE_FILLMODE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.fillMode);
      break;
     case D3DRENDERSTATE_SHADEMODE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.shadeMode);
      break;
     case D3DRENDERSTATE_ZWRITEENABLE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.zWriteEnable);
      break;
     case D3DRENDERSTATE_ALPHATESTENABLE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.alphaTestEnable);
      break;
     case D3DRENDERSTATE_SRCBLEND:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.srcBlend);
      break;
     case D3DRENDERSTATE_DESTBLEND:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.destBlend);
      break;
     case D3DRENDERSTATE_TEXTUREMAPBLEND:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.textureMapBlend);
      break;
     case D3DRENDERSTATE_CULLMODE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.cullMode);
      break;
     case D3DRENDERSTATE_ZFUNC:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.zFunc);
      break;
     case D3DRENDERSTATE_ALPHAREF:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.alphaRef);
      break;
     case D3DRENDERSTATE_ALPHAFUNC:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.alphaFunc);
      break;
     case D3DRENDERSTATE_FOGENABLE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.fogEnable);
      break;
     case D3DRENDERSTATE_ZVISIBLE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.zVisible);
      break;
     case D3DRENDERSTATE_FOGCOLOR:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.fogColor);
      break;
     case D3DRENDERSTATE_COLORKEYENABLE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.colorKeyEnable);
      break;
     case D3DRENDERSTATE_TEXTUREMAG:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.textureMag);
      break;
     case D3DRENDERSTATE_TEXTUREMIN:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.textureMin);
      break;

      // Special NOTE: 3DFx does not support specifically setting the blend
      //  enable for just the alpha blending at this time.  Use the big gun...
      //
     case D3DRENDERSTATE_BLENDENABLE:
      _setRenderState(in_stateType, in_state,
                      (DWORD*)&renderState.blendEnable);
      break;
     case D3DRENDERSTATE_ALPHABLENDENABLE:
      _setRenderState(D3DRENDERSTATE_BLENDENABLE, in_state,
                      (DWORD*)&renderState.alphaBlendEnable);
      break;


     default:
      AssertWarn(0, "Unknown render state set...");
      break;
   }
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::refreshRenderState()
//    
// DESCRIPTION 
//    Gets the current renderstate from the device, at least those that
//   we are interested in...
//    
// NOTES 
//    Assumes that all calls to GetRenderState succeed.
//------------------------------------------------------------------------------
void 
Surface::refreshRenderState()
{
   AssertFatal(pD3DDevice2Vector[deviceMinorId] != NULL,
               "Error, no device to query...");

   IDirect3DDevice2* pDevice = pD3DDevice2Vector[deviceMinorId];
   
   pDevice->GetRenderState(D3DRENDERSTATE_TEXTUREHANDLE,
                           (DWORD*)&renderState.textureHandle);
   pDevice->GetRenderState(D3DRENDERSTATE_TEXTUREADDRESS,
                           (DWORD*)&renderState.textureAddress);
   pDevice->GetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE,
                           (DWORD*)&renderState.texturePerspective);
   pDevice->GetRenderState(D3DRENDERSTATE_WRAPU,
                           (DWORD*)&renderState.wrapU);
   pDevice->GetRenderState(D3DRENDERSTATE_WRAPV,
                           (DWORD*)&renderState.wrapV);
   pDevice->GetRenderState(D3DRENDERSTATE_ZENABLE,
                           (DWORD*)&renderState.zEnable);
   pDevice->GetRenderState(D3DRENDERSTATE_FILLMODE,
                           (DWORD*)&renderState.fillMode);
   pDevice->GetRenderState(D3DRENDERSTATE_SHADEMODE,
                           (DWORD*)&renderState.shadeMode);
   pDevice->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE,
                           (DWORD*)&renderState.zWriteEnable);
   pDevice->GetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,
                           (DWORD*)&renderState.alphaTestEnable);
   pDevice->GetRenderState(D3DRENDERSTATE_SRCBLEND,
                           (DWORD*)&renderState.srcBlend);
   pDevice->GetRenderState(D3DRENDERSTATE_DESTBLEND,
                           (DWORD*)&renderState.destBlend);
   pDevice->GetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,
                           (DWORD*)&renderState.textureMapBlend);
   pDevice->GetRenderState(D3DRENDERSTATE_CULLMODE,
                           (DWORD*)&renderState.cullMode);
   pDevice->GetRenderState(D3DRENDERSTATE_ZFUNC,
                           (DWORD*)&renderState.zFunc);
   pDevice->GetRenderState(D3DRENDERSTATE_ALPHAREF,
                           (DWORD*)&renderState.alphaRef);
   pDevice->GetRenderState(D3DRENDERSTATE_ALPHAFUNC,
                           (DWORD*)&renderState.alphaFunc);
   pDevice->GetRenderState(D3DRENDERSTATE_FOGENABLE,
                           (DWORD*)&renderState.fogEnable);
   pDevice->GetRenderState(D3DRENDERSTATE_ZVISIBLE,
                           (DWORD*)&renderState.zVisible);
   pDevice->GetRenderState(D3DRENDERSTATE_FOGCOLOR,
                           (DWORD*)&renderState.fogColor);
   pDevice->GetRenderState(D3DRENDERSTATE_COLORKEYENABLE,
                           (DWORD*)&renderState.colorKeyEnable);
   pDevice->GetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
                           (DWORD*)&renderState.alphaBlendEnable);
   pDevice->GetRenderState(D3DRENDERSTATE_BLENDENABLE,
                           (DWORD*)&renderState.blendEnable);
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    Surface::setDefaultRenderState()
//    
// DESCRIPTION 
//    Gets the current renderstate from the device, at least those that
//   we are interested in...
//    
// NOTES 
//    Assumes that all calls to GetRenderState succeed.
//------------------------------------------------------------------------------
void 
Surface::setDefaultRenderState()
{
   setRenderState(D3DRENDERSTATE_TEXTUREHANDLE,
                  defaultRenderState.textureHandle);
   setRenderState(D3DRENDERSTATE_TEXTUREADDRESS,
                  defaultRenderState.textureAddress);
   setRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE,
                  defaultRenderState.texturePerspective);
   setRenderState(D3DRENDERSTATE_WRAPU,
                  defaultRenderState.wrapU);
   setRenderState(D3DRENDERSTATE_WRAPV,
                  defaultRenderState.wrapV);
   setRenderState(D3DRENDERSTATE_ZENABLE,
                  defaultRenderState.zEnable);
   setRenderState(D3DRENDERSTATE_FILLMODE,
                  defaultRenderState.fillMode);
   setRenderState(D3DRENDERSTATE_SHADEMODE,
                  defaultRenderState.shadeMode);
   setRenderState(D3DRENDERSTATE_ZWRITEENABLE,
                  defaultRenderState.zWriteEnable);
   setRenderState(D3DRENDERSTATE_ALPHATESTENABLE,
                  defaultRenderState.alphaTestEnable);
   setRenderState(D3DRENDERSTATE_SRCBLEND,
                  defaultRenderState.srcBlend);
   setRenderState(D3DRENDERSTATE_DESTBLEND,
                  defaultRenderState.destBlend);
   setRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,
                  defaultRenderState.textureMapBlend);
   setRenderState(D3DRENDERSTATE_CULLMODE,
                  defaultRenderState.cullMode);
   setRenderState(D3DRENDERSTATE_ZFUNC,
                  defaultRenderState.zFunc);
   setRenderState(D3DRENDERSTATE_ALPHAREF,
                  defaultRenderState.alphaRef);
   setRenderState(D3DRENDERSTATE_ALPHAFUNC,
                  defaultRenderState.alphaFunc);
   setRenderState(D3DRENDERSTATE_FOGENABLE,
                  defaultRenderState.fogEnable);
   setRenderState(D3DRENDERSTATE_FOGCOLOR,
                  defaultRenderState.fogColor);
   setRenderState(D3DRENDERSTATE_COLORKEYENABLE,
                  defaultRenderState.colorKeyEnable);
   setRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,
                  defaultRenderState.alphaBlendEnable);
   setRenderState(D3DRENDERSTATE_BLENDENABLE,
                  defaultRenderState.blendEnable);

   // Filter mode, device dependant, so not set from defaultRenderState...
   //
   setRenderState(D3DRENDERSTATE_TEXTUREMAG,
                  D3DDevDescVector[deviceMinorId].filterMode);
   setRenderState(D3DRENDERSTATE_TEXTUREMIN,
                  D3DDevDescVector[deviceMinorId].filterMode);
}

}; // namespace Direct3D


//------------------------------------------------------------------------------
// NAME 
//    char* D3DAppErrorToString(HRESULT error)
//    
// DESCRIPTION 
//    Stolen from Direct3D examples from MS to translate cryptic results...
//    
// NOTES 
//    Not always very helpful...
//------------------------------------------------------------------------------
char*
D3DAppErrorToString(HRESULT error)
{
    switch(error) {
        case DD_OK:
            return "No error.\0";
        case DDERR_ALREADYINITIALIZED:
            return "This object is already initialized.\0";
        case DDERR_BLTFASTCANTCLIP:
            return "Return if a clipper object is attached to the source surface passed into a BltFast call.\0";
        case DDERR_CANNOTATTACHSURFACE:
            return "This surface can not be attached to the requested surface.\0";
        case DDERR_CANNOTDETACHSURFACE:
            return "This surface can not be detached from the requested surface.\0";
        case DDERR_CANTCREATEDC:
            return "Windows can not create any more DCs.\0";
        case DDERR_CANTDUPLICATE:
            return "Can't duplicate primary & 3D surfaces, or surfaces that are implicitly created.\0";
        case DDERR_CLIPPERISUSINGHWND:
            return "An attempt was made to set a cliplist for a clipper object that is already monitoring an hwnd.\0";
        case DDERR_COLORKEYNOTSET:
            return "No src color key specified for this operation.\0";
        case DDERR_CURRENTLYNOTAVAIL:
            return "Support is currently not available.\0";
        case DDERR_DIRECTDRAWALREADYCREATED:
            return "A DirectDraw object representing this driver has already been created for this process.\0";
        case DDERR_EXCEPTION:
            return "An exception was encountered while performing the requested operation.\0";
        case DDERR_EXCLUSIVEMODEALREADYSET:
            return "An attempt was made to set the cooperative level when it was already set to exclusive.\0";
        case DDERR_GENERIC:
            return "Generic failure.\0";
        case DDERR_HEIGHTALIGN:
            return "Height of rectangle provided is not a multiple of reqd alignment.\0";
        case DDERR_HWNDALREADYSET:
            return "The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces or palettes created.\0";
        case DDERR_HWNDSUBCLASSED:
            return "HWND used by DirectDraw CooperativeLevel has been subclassed, this prevents DirectDraw from restoring state.\0";
        case DDERR_IMPLICITLYCREATED:
            return "This surface can not be restored because it is an implicitly created surface.\0";
        case DDERR_INCOMPATIBLEPRIMARY:
            return "Unable to match primary surface creation request with existing primary surface.\0";
        case DDERR_INVALIDCAPS:
            return "One or more of the caps bits passed to the callback are incorrect.\0";
        case DDERR_INVALIDCLIPLIST:
            return "DirectDraw does not support the provided cliplist.\0";
        case DDERR_INVALIDDIRECTDRAWGUID:
            return "The GUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier.\0";
        case DDERR_INVALIDMODE:
            return "DirectDraw does not support the requested mode.\0";
        case DDERR_INVALIDOBJECT:
            return "DirectDraw received a pointer that was an invalid DIRECTDRAW object.\0";
        case DDERR_INVALIDPARAMS:
            return "One or more of the parameters passed to the function are incorrect.\0";
        case DDERR_INVALIDPIXELFORMAT:
            return "The pixel format was invalid as specified.\0";
        case DDERR_INVALIDPOSITION:
            return "Returned when the position of the overlay on the destination is no longer legal for that destination.\0";
        case DDERR_INVALIDRECT:
            return "Rectangle provided was invalid.\0";
        case DDERR_LOCKEDSURFACES:
            return "Operation could not be carried out because one or more surfaces are locked.\0";
        case DDERR_NO3D:
            return "There is no 3D present.\0";
        case DDERR_NOALPHAHW:
            return "Operation could not be carried out because there is no alpha accleration hardware present or available.\0";
        case DDERR_NOBLTHW:
            return "No blitter hardware present.\0";
        case DDERR_NOCLIPLIST:
            return "No cliplist available.\0";
        case DDERR_NOCLIPPERATTACHED:
            return "No clipper object attached to surface object.\0";
        case DDERR_NOCOLORCONVHW:
            return "Operation could not be carried out because there is no color conversion hardware present or available.\0";
        case DDERR_NOCOLORKEY:
            return "Surface doesn't currently have a color key\0";
        case DDERR_NOCOLORKEYHW:
            return "Operation could not be carried out because there is no hardware support of the destination color key.\0";
        case DDERR_NOCOOPERATIVELEVELSET:
            return "Create function called without DirectDraw object method SetCooperativeLevel being called.\0";
        case DDERR_NODC:
            return "No DC was ever created for this surface.\0";
        case DDERR_NODDROPSHW:
            return "No DirectDraw ROP hardware.\0";
        case DDERR_NODIRECTDRAWHW:
            return "A hardware-only DirectDraw object creation was attempted but the driver did not support any hardware.\0";
        case DDERR_NOEMULATION:
            return "Software emulation not available.\0";
        case DDERR_NOEXCLUSIVEMODE:
            return "Operation requires the application to have exclusive mode but the application does not have exclusive mode.\0";
        case DDERR_NOFLIPHW:
            return "Flipping visible surfaces is not supported.\0";
        case DDERR_NOGDI:
            return "There is no GDI present.\0";
        case DDERR_NOHWND:
            return "Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND.\0";
        case DDERR_NOMIRRORHW:
            return "Operation could not be carried out because there is no hardware present or available.\0";
        case DDERR_NOOVERLAYDEST:
            return "Returned when GetOverlayPosition is called on an overlay that UpdateOverlay has never been called on to establish a destination.\0";
        case DDERR_NOOVERLAYHW:
            return "Operation could not be carried out because there is no overlay hardware present or available.\0";
        case DDERR_NOPALETTEATTACHED:
            return "No palette object attached to this surface.\0";
        case DDERR_NOPALETTEHW:
            return "No hardware support for 16 or 256 color palettes.\0";
        case DDERR_NORASTEROPHW:
            return "Operation could not be carried out because there is no appropriate raster op hardware present or available.\0";
        case DDERR_NOROTATIONHW:
            return "Operation could not be carried out because there is no rotation hardware present or available.\0";
        case DDERR_NOSTRETCHHW:
            return "Operation could not be carried out because there is no hardware support for stretching.\0";
        case DDERR_NOT4BITCOLOR:
            return "DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette.\0";
        case DDERR_NOT4BITCOLORINDEX:
            return "DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bit color index palette.\0";
        case DDERR_NOT8BITCOLOR:
            return "DirectDrawSurface is not in 8 bit color mode and the requested operation requires 8 bit color.\0";
        case DDERR_NOTAOVERLAYSURFACE:
            return "Returned when an overlay member is called for a non-overlay surface.\0";
        case DDERR_NOTEXTUREHW:
            return "Operation could not be carried out because there is no texture mapping hardware present or available.\0";
        case DDERR_NOTFLIPPABLE:
            return "An attempt has been made to flip a surface that is not flippable.\0";
        case DDERR_NOTFOUND:
            return "Requested item was not found.\0";
        case DDERR_NOTLOCKED:
            return "Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this process, has been attempted.\0";
        case DDERR_NOTPALETTIZED:
            return "The surface being used is not a palette-based surface.\0";
        case DDERR_NOVSYNCHW:
            return "Operation could not be carried out because there is no hardware support for vertical blank synchronized operations.\0";
        case DDERR_NOZBUFFERHW:
            return "Operation could not be carried out because there is no hardware support for zbuffer blitting.\0";
        case DDERR_NOZOVERLAYHW:
            return "Overlay surfaces could not be z layered based on their BltOrder because the hardware does not support z layering of overlays.\0";
        case DDERR_OUTOFCAPS:
            return "The hardware needed for the requested operation has already been allocated.\0";
        case DDERR_OUTOFMEMORY:
            return "DirectDraw does not have enough memory to perform the operation.\0";
        case DDERR_OUTOFVIDEOMEMORY:
            return "DirectDraw does not have enough memory to perform the operation.\0";
        case DDERR_OVERLAYCANTCLIP:
            return "The hardware does not support clipped overlays.\0";
        case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
            return "Can only have ony color key active at one time for overlays.\0";
        case DDERR_OVERLAYNOTVISIBLE:
            return "Returned when GetOverlayPosition is called on a hidden overlay.\0";
        case DDERR_PALETTEBUSY:
            return "Access to this palette is being refused because the palette is already locked by another thread.\0";
        case DDERR_PRIMARYSURFACEALREADYEXISTS:
            return "This process already has created a primary surface.\0";
        case DDERR_REGIONTOOSMALL:
            return "Region passed to Clipper::GetClipList is too small.\0";
        case DDERR_SURFACEALREADYATTACHED:
            return "This surface is already attached to the surface it is being attached to.\0";
        case DDERR_SURFACEALREADYDEPENDENT:
            return "This surface is already a dependency of the surface it is being made a dependency of.\0";
        case DDERR_SURFACEBUSY:
            return "Access to this surface is being refused because the surface is already locked by another thread.\0";
        case DDERR_SURFACEISOBSCURED:
            return "Access to surface refused because the surface is obscured.\0";
        case DDERR_SURFACELOST:
            return "Access to this surface is being refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have Restore called on it.\0";
        case DDERR_SURFACENOTATTACHED:
            return "The requested surface is not attached.\0";
        case DDERR_TOOBIGHEIGHT:
            return "Height requested by DirectDraw is too large.\0";
        case DDERR_TOOBIGSIZE:
            return "Size requested by DirectDraw is too large, but the individual height and width are OK.\0";
        case DDERR_TOOBIGWIDTH:
            return "Width requested by DirectDraw is too large.\0";
        case DDERR_UNSUPPORTED:
            return "Action not supported.\0";
        case DDERR_UNSUPPORTEDFORMAT:
            return "FOURCC format requested is unsupported by DirectDraw.\0";
        case DDERR_UNSUPPORTEDMASK:
            return "Bitmask in the pixel format requested is unsupported by DirectDraw.\0";
        case DDERR_VERTICALBLANKINPROGRESS:
            return "Vertical blank is in progress.\0";
        case DDERR_WASSTILLDRAWING:
            return "Informs DirectDraw that the previous Blt which is transfering information to or from this Surface is incomplete.\0";
        case DDERR_WRONGMODE:
            return "This surface can not be restored because it was created in a different mode.\0";
        case DDERR_XALIGN:
            return "Rectangle provided was not horizontally aligned on required boundary.\0";
        case D3DERR_BADMAJORVERSION:
            return "D3DERR_BADMAJORVERSION\0";
        case D3DERR_BADMINORVERSION:
            return "D3DERR_BADMINORVERSION\0";
        case D3DERR_EXECUTE_LOCKED:
            return "D3DERR_EXECUTE_LOCKED\0";
        case D3DERR_EXECUTE_NOT_LOCKED:
            return "D3DERR_EXECUTE_NOT_LOCKED\0";
        case D3DERR_EXECUTE_CREATE_FAILED:
            return "D3DERR_EXECUTE_CREATE_FAILED\0";
        case D3DERR_EXECUTE_DESTROY_FAILED:
            return "D3DERR_EXECUTE_DESTROY_FAILED\0";
        case D3DERR_EXECUTE_LOCK_FAILED:
            return "D3DERR_EXECUTE_LOCK_FAILED\0";
        case D3DERR_EXECUTE_UNLOCK_FAILED:
            return "D3DERR_EXECUTE_UNLOCK_FAILED\0";
        case D3DERR_EXECUTE_FAILED:
            return "D3DERR_EXECUTE_FAILED\0";
        case D3DERR_EXECUTE_CLIPPED_FAILED:
            return "D3DERR_EXECUTE_CLIPPED_FAILED\0";
        case D3DERR_TEXTURE_NO_SUPPORT:
            return "D3DERR_TEXTURE_NO_SUPPORT\0";
        case D3DERR_TEXTURE_NOT_LOCKED:
            return "D3DERR_TEXTURE_NOT_LOCKED\0";
        case D3DERR_TEXTURE_LOCKED:
            return "D3DERR_TEXTURELOCKED\0";
        case D3DERR_TEXTURE_CREATE_FAILED:
            return "D3DERR_TEXTURE_CREATE_FAILED\0";
        case D3DERR_TEXTURE_DESTROY_FAILED:
            return "D3DERR_TEXTURE_DESTROY_FAILED\0";
        case D3DERR_TEXTURE_LOCK_FAILED:
            return "D3DERR_TEXTURE_LOCK_FAILED\0";
        case D3DERR_TEXTURE_UNLOCK_FAILED:
            return "D3DERR_TEXTURE_UNLOCK_FAILED\0";
        case D3DERR_TEXTURE_LOAD_FAILED:
            return "D3DERR_TEXTURE_LOAD_FAILED\0";
        case D3DERR_MATRIX_CREATE_FAILED:
            return "D3DERR_MATRIX_CREATE_FAILED\0";
        case D3DERR_MATRIX_DESTROY_FAILED:
            return "D3DERR_MATRIX_DESTROY_FAILED\0";
        case D3DERR_MATRIX_SETDATA_FAILED:
            return "D3DERR_MATRIX_SETDATA_FAILED\0";
        case D3DERR_SETVIEWPORTDATA_FAILED:
            return "D3DERR_SETVIEWPORTDATA_FAILED\0";
        case D3DERR_MATERIAL_CREATE_FAILED:
            return "D3DERR_MATERIAL_CREATE_FAILED\0";
        case D3DERR_MATERIAL_DESTROY_FAILED:
            return "D3DERR_MATERIAL_DESTROY_FAILED\0";
        case D3DERR_MATERIAL_SETDATA_FAILED:
            return "D3DERR_MATERIAL_SETDATA_FAILED\0";
        case D3DERR_LIGHT_SET_FAILED:
            return "D3DERR_LIGHT_SET_FAILED\0";
        case D3DERR_SCENE_NOT_IN_SCENE:
            return "D3DERR_SCENE_NOT_IN_SCENE\0";
        default:
            return "Unrecognized error value.\0";
    }
}
