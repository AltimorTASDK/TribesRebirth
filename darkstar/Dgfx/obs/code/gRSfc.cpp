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

#include "fn_table.h"
#include "g_pal.h"
#include "g_bitmap.h"
#include "p_txcach.h"
#include "gRDevice.h"
#include "gRSfc.h"
#include "renditionProxy.h"
#include "rn_manag.h"

extern void V_CDECL veriteErrorHandler(v_handle, v_routine, v_error, int);
extern void V_CDECL redlineErrorHandler(v_handle, vl_routine, vl_error, int);

namespace Rendition {

//-------------------------------------- Callback prototypes
//
v_cmdbuffer V_CDECL veriteCmdBuffOverflow(v_handle    io_vHandle,
                                          v_cmdbuffer io_overflowBuffer);
bool  Surface::sm_veriteRunning = false;

//------------------------------------------------------------------------------
// NAME 
//    Surface::Surface(const bool in_windowed)
//    
// DESCRIPTION 
//    Just a null'er c'tor
//------------------------------------------------------------------------------
Surface::Surface(const bool in_windowed)
 : m_pProxy(NULL),
   m_appHWnd(NULL),
   m_windowed(in_windowed),
   m_vHandle(NULL),
   m_pVSurface(NULL),
   m_pCurrentBuffer(NULL),
   m_currBuffIndex(0),
   m_filterType(V_SRCFILTER_BILINEAR),
   m_bilinearOffset(0.0f),
   m_draw3DBegun(0),
   m_pTextureCache(NULL)
{
   functionTable = &rendition_table;
   surfaceType   = SURFACE_RREDLINE;

   int i;
   for (i = 0; i < NUM_CMD_BUFFS; i++) {
      m_pCmdBuffs[i] = NULL;
   }

   m_waitForDisplaySwitch = true;
}

//------------------------------------------------------------------------------
// NAME 
//    Surface::~Surface()
//    
// DESCRIPTION 
//    Shuts down the verite connection associated with this surface in reverse
//   order of initialization...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
#include <stdio.h>

Surface::~Surface()
{
   AssertWarn(0, "Deleting surface");
   // We have to tell the render table that we'll no longer be handling span
   //  rendering...
   //
   g_renderTable.setIgnoreBadRenderers(false);

   // Flush out any remaining commands in the current buffer.
   //
   cmdBuffOverflow(m_vHandle, m_pCurrentBuffer, false);

   // Remove our command buffers...
   for (int i = 0; i < NUM_CMD_BUFFS; i++) {
      if (m_pCmdBuffs[i] != NULL)
         m_pProxy->V_DestroyCmdBuffer(m_pCmdBuffs[i]);
      m_pCmdBuffs[i] = NULL;
   }
   m_pCurrentBuffer = NULL;

   // Delete the texture cache
   delete m_pTextureCache;
   m_pTextureCache = NULL;

   // Delete the surfaces
   if (m_pVSurface != NULL)
      m_pProxy->VL_DestroySurface(m_vHandle, m_pVSurface);
   m_pVSurface = NULL;

   // Shutdown the verite for good...
   if (m_vHandle != NULL) {
      m_pProxy->V_DestroyVerite(m_vHandle);
   }
   m_vHandle = NULL;
   
   // Notify rest of class that a slot is available...
   sm_veriteRunning = false;
   
   // We don't own our Proxy object, so just NULL it...
   m_pProxy = NULL;
   
   gfxTextureCache.flushCache();
   AssertWarn(0, "Done Deleting surface");
}

//------------------------------------------------------------------------------
//-------------------------------------- Interface functions inherited from
//                                        GFXSurface
//
Bool 
Surface::_setPalette(GFXPalette *lpPalette,
                     Int32       /*in_start*/,
                     Int32       /*in_count*/,
                     Bool        /*in_rsvPal*/)
{
   if (lpPalette == NULL)
      return true;

   pPalette = lpPalette;
   m_pTextureCache->setPalette(lpPalette);

   PALETTEENTRY& rHaze = pPalette->color[(pPalette->getHazeColor()) & 0xff];
   ColorF hColor;
   hColor.red   = float(rHaze.peRed)   / 255.0f;
   hColor.green = float(rHaze.peGreen) / 255.0f;
   hColor.blue  = float(rHaze.peBlue)  / 255.0f;
   this->setHazeColor(&hColor);
   
   return true;
}

void 
Surface::flip()
{
   if (m_windowed == false) {
      // This is easy, just flip with the API call.  DMMNOTE: Since there is
      //  no convenient way for the surface to get messages to indicate that
      //  buffers need to be restored, we simply always call the restore
      //  function.  It may be that this should NOT be called on every frame...
      //
      m_pProxy->VL_RestoreSurface(m_vHandle, m_pVSurface);
      m_pProxy->VL_SwapDisplaySurface(&m_pCurrentBuffer, m_pVSurface);

      if (m_waitForDisplaySwitch == true) {
         m_pProxy->VL_WaitForDisplaySwitch(&m_pCurrentBuffer);
      }
      cmdBuffOverflow(m_vHandle, m_pCurrentBuffer, false);
   } else {
      v_rect bltRect;
      bltRect.left   = 0;
      bltRect.top    = 0;
      bltRect.right  = surfaceWidth  - 1;
      bltRect.bottom = surfaceHeight - 1;
      
      // Note that V_BltDisplayBuffer does NOT work through the FIFO, so we
      //  must flush the command buffers before calling it...
      //
      RECT cRect;
      GetClientRect(m_appHWnd, &cRect);
      ClientToScreen(m_appHWnd, (LPPOINT)&cRect);
      ClientToScreen(m_appHWnd, (LPPOINT)&cRect + 1);

      cmdBuffOverflow(m_vHandle, m_pCurrentBuffer, false);
      m_pProxy->V_BltDisplayBuffer(m_vHandle,
                                   m_pVSurface->buffer_group, 0,  // to front
                                   (v_rect*)&cRect,               // dst all
                                   m_pVSurface->buffer_group, 1,  // from back
                                   &bltRect);                     // blt all
   }
}

//------------------------------------------------------------------------------
//-------------------------------------- Rendition::Surface methods..
//
Surface* 
Surface::create(RenditionProxy* io_pProxy,
                HWND            io_appHWnd,
                const Int32     in_width,
                const Int32     in_height,
                const Int32     /*in_bpp*/,
                const bool      in_windowed)
{
   AssertFatal(io_pProxy != NULL, "Error, must have valid proxy object");

   if (sm_veriteRunning == true) {
      // Only one verite surface allowed
      //
      return NULL;
   }
   
   Surface* pRetSurf  = new Surface(in_windowed);
   pRetSurf->m_pProxy = io_pProxy;
   
   // We have to tell the render table that we'll be handling all span rendering
   //  from now on...
   //
   g_renderTable.setIgnoreBadRenderers(true);
   sm_veriteRunning = true;
   
   // If we break from this loop, it's for an error condition...
   //
   while (true) {
      v_error  vError;
      vl_error vlError;
      
      // Set our error "handlers"
      //
      vError  = io_pProxy->V_RegisterErrorHandler(veriteErrorHandler);
      vlError = io_pProxy->VL_RegisterErrorHandler(redlineErrorHandler);
      AssertFatal(vError == V_SUCCESS && vlError == VL_SUCCESS,
                  "Could not register error handlers...");

      // Open a verite connection...
      //
      vError = io_pProxy->V_CreateVerite(1, io_appHWnd, &pRetSurf->m_vHandle,
                                         GFXRenditionDevice::sm_pMicrocodeFile);
      if (vError != V_SUCCESS)
         break;
      pRetSurf->m_appHWnd = io_appHWnd;
   
      // Callbacks need to know which surface to forward overflow or
      //  error messages to, so we set the userData in the v_handle to point
      //  to the surface... (never fails...)
      //
      io_pProxy->V_SetUserPrivate(pRetSurf->m_vHandle, v_u32(pRetSurf));
      
      // Set app mode
      //
      v_u32 appType = (in_windowed == true) ? V_WINDOWED_APP :
                                              V_FULLSCREEN_APP;
      vlError = io_pProxy->V_SetDisplayType(pRetSurf->m_vHandle, appType);
      if (vlError != V_SUCCESS)
         break;

      // We may need to set the display mode for fullscreen...
      //  DMMNOTE: Any way to get highest possible refresh?
      //
      if (in_windowed == false) {
         vlError = io_pProxy->V_SetDisplayMode(pRetSurf->m_vHandle,
                                               in_width, in_height, 16, 60);
         if (vlError != V_SUCCESS)
            break;
      }
   
      // Create the command buffers, currently we use 3, more may be a good
      //  idea...
      //
      bool buffCreationSuccess = true;
      for (int i = 0; i < NUM_CMD_BUFFS; i++) {
         // We create buffers with the default size, we may want to override
         //  this with our own sizes eventually.  Command buffer callback
         //  requires that the Surface pointer be passed to the SetUserPrivate
         //  method for the v_handle
         //
         pRetSurf->m_pCmdBuffs[i] =
            io_pProxy->V_CreateCmdBuffer(pRetSurf->m_vHandle, 1024, 16*1024);
         if (pRetSurf->m_pCmdBuffs[i] == NULL) {
            buffCreationSuccess = false;
            break;
         }
         io_pProxy->V_SetCmdBufferCallBack(pRetSurf->m_pCmdBuffs[i],
                                           veriteCmdBuffOverflow);
      }
      if (buffCreationSuccess == false)
         break;
      pRetSurf->m_pCurrentBuffer = pRetSurf->m_pCmdBuffs[0];
      pRetSurf->m_currBuffIndex  = 0;
   
      // Create the rendering and front surface.  No ZBuffering is required.
      //
      vlError = io_pProxy->VL_CreateSurface(pRetSurf->m_vHandle,
                                            &pRetSurf->m_pVSurface,
                                            V_SURFACE_PRIMARY,     // Front surface is visible
                                            2,                     // Front/Back pair
                                            V_PIXFMT_565,          // 16-bit 565
                                            in_width, in_height);  // obv.
      if (vlError != VL_SUCCESS) {
         // This is a fairly critical and sensitive call, we'd like lots of
         //  failure information...
         //
         char buffer[512];
         io_pProxy->VL_GetErrorText(vlError, buffer, 511);
         AssertWarn(0, avar("Error creating Rendition::Surface %s", buffer));
         break;
      }
   
      // Install created surface as the rendering destination.  Note: call
      //  always succeeds...  Issue the command buffer synch'ly to make sure
      //  that the render buffer is set up before we enter any rending calls
      //
      io_pProxy->VL_InstallDstBuffer(&pRetSurf->m_pCurrentBuffer,
                                     pRetSurf->m_pVSurface);
      io_pProxy->VL_SetScissorX(&pRetSurf->m_pCurrentBuffer, in_width);
      io_pProxy->VL_SetScissorY(&pRetSurf->m_pCurrentBuffer, in_height);
      pRetSurf->cmdBuffOverflow(pRetSurf->m_vHandle,
                                pRetSurf->m_pCurrentBuffer,
                                false);
      
      // Successful surface creation!
      //
      pRetSurf->surfaceWidth    = in_width;
      pRetSurf->surfaceHeight   = in_height;
      pRetSurf->surfaceBitDepth = 16;
      
      RectI clipRect;
      clipRect.upperL.x = clipRect.upperL.y = 0;
      clipRect.lowerR.x = pRetSurf->surfaceWidth  - 1;
      clipRect.lowerR.y = pRetSurf->surfaceHeight - 1;
      pRetSurf->setClipRect(&clipRect);
      
      // Create our texture cache to attach to this surface...
      //
      pRetSurf->m_pTextureCache = new TextureCache(gfxTextureCache,
                                                   pRetSurf->m_pProxy,
                                                   pRetSurf->m_vHandle,
                                                   pRetSurf);
      // Flush the gfxtexture cache...
      //
      gfxTextureCache.flushCache();
      pRetSurf->setDefaultRenderState();
      return pRetSurf;
   }
   
   // Let surface destructor take care of shutting down partial initialization
   //
   delete pRetSurf;
   return NULL;
}


v_cmdbuffer 
Surface::cmdBuffOverflow(v_handle    io_vHandle,
                         v_cmdbuffer io_cmdBuffer,
                         const bool  in_issueAsync)
{
   AssertFatal(io_cmdBuffer == m_pCurrentBuffer,
               "Error, somehow a non-current buffer overflowed!");
   AssertFatal(io_vHandle == m_vHandle, "Why am I getting this notification?");
               
   // Asynch issue the current buffer...
   //
   v_error vError;
   if (in_issueAsync == true) {
      vError = m_pProxy->V_IssueCmdBufferAsync(io_vHandle, io_cmdBuffer);
      AssertWarn(vError == V_SUCCESS, "Something went wrong on async issue");
   } else {
      vError = m_pProxy->V_IssueCmdBuffer(io_vHandle, io_cmdBuffer);
      AssertWarn(vError == V_SUCCESS, "Something went wrong on sync issue");
   }
   
   // Advance to the next command buffer...
   //
   m_currBuffIndex += 1;
   m_currBuffIndex %= NUM_CMD_BUFFS;
   m_pCurrentBuffer = m_pCmdBuffs[m_currBuffIndex];
   
   while (m_pProxy->V_QueryCmdBuffer(io_vHandle, m_pCurrentBuffer) == V_CMDBUFFER_INUSE) {
      // Time is being wasted in here: maybe increase the number of buffers?
      ;
   }
   
   return m_pCurrentBuffer;
}


bool 
Surface::setTexture(const GFXTextureCacheEntry* in_tce)
{
   return m_pTextureCache->setTexture(in_tce);
}

bool 
Surface::setTexture(const BYTE* in_pTexture)
{
   return m_pTextureCache->setTexture(in_pTexture);
}

void
Surface::flushTexture(const GFXBitmap* in_pBitmap,
                      const bool       in_reload)
{
   m_pTextureCache->flushTexture(in_pBitmap, in_reload);
}

void 
Surface::downloadTexture(const GFXTextureCacheEntry* in_tce,
                         const bool                  in_enableAlpha,
                         const bool                  in_enableTransp)
{
   m_pTextureCache->downloadTexture(in_tce, in_enableAlpha, in_enableTransp);
}

void 
Surface::downloadTexture(BYTE*  in_pTexture,
                         const UInt32 in_stride,
                         const UInt32 in_height,
                         const bool   in_enableAlpha,
                         const bool   in_enableTransp)
{
   m_pTextureCache->downloadTexture(in_pTexture,
                                    in_stride,
                                    in_height,
                                    in_enableAlpha,
                                    in_enableTransp);
}
   
DWORD 
Surface::getCaps()
{
   return GFX_DEVCAP_SUPPORTS_CONST_ALPHA;
}

void 
Surface::setFilterEnable(const bool in_enable)
{
   if (in_enable == true) {
      if (currRenderState.srcFilter == m_filterType) {
         // already enabled...
         //
      } else {
         // If we are here, we are assured that the filtertype is bilinear...
         //
         currRenderState.srcFilter = V_SRCFILTER_BILINEAR;
         m_pProxy->VL_SetSrcFilter(getVCmdBuffer(), V_SRCFILTER_BILINEAR);
      }
   } else {
      if (currRenderState.srcFilter == V_SRCFILTER_POINT) {
         // already disabled...
         //
      } else {
         currRenderState.srcFilter = V_SRCFILTER_POINT;
         m_pProxy->VL_SetSrcFilter(getVCmdBuffer(), V_SRCFILTER_POINT);
      }
   }
}


//------------------------------------------------------------------------------
#define RENDERSTATE_MANIPULATION 1
//--------------------------------------
void 
Surface::setDefaultRenderState()
{
   currRenderState.srcFilter = V_SRCFILTER_POINT;
   m_pProxy->VL_SetSrcFilter(getVCmdBuffer(), V_SRCFILTER_POINT);
   currRenderState.srcFunc = V_SRCFUNC_REPLACE;
   m_pProxy->VL_SetSrcFunc(getVCmdBuffer(), V_SRCFUNC_REPLACE);
   currRenderState.srcFmt = 0;
   m_pProxy->VL_SetSrcFmt(getVCmdBuffer(), 0);

   currRenderState.chroma = 0;
   m_pProxy->VL_SetTranspReject(getVCmdBuffer(), 0);
   m_pProxy->VL_SetChromaKey(getVCmdBuffer(), 0);

   currRenderState.chromaMask      = 0;
   currRenderState.chromaColor     = 0;
   currRenderState.chromaColorPFmt = V_PIXFMT_565;
   currRenderState.chromaMaskPFmt  = V_PIXFMT_565;
   m_pProxy->VL_SetChromaMask(getVCmdBuffer(), 0xffff, V_PIXFMT_565);
   m_pProxy->VL_SetChromaColor(getVCmdBuffer(), 0, V_PIXFMT_565);

   currRenderState.dither = FALSE;
   m_pProxy->VL_SetDitherEnable(getVCmdBuffer(), FALSE);
   
   currRenderState.blendEnable = FALSE;
   m_pProxy->VL_SetBlendEnable(getVCmdBuffer(), FALSE);
   currRenderState.blendSrcFunc = V_BLENDSRCALPHA;
   m_pProxy->VL_SetBlendSrcFunc(getVCmdBuffer(), V_BLENDSRCALPHA);
   currRenderState.blendDstFunc = V_BLENDSRCALPHAINV;
   m_pProxy->VL_SetBlendDstFunc(getVCmdBuffer(), V_BLENDSRCALPHAINV);

   currRenderState.fogEnable = FALSE;
   m_pProxy->VL_SetFogEnable(getVCmdBuffer(), FALSE);
   currRenderState.fogAttrib = 0;
   m_pProxy->VL_SetF(getVCmdBuffer(), 0);
   currRenderState.fogColor = 0;
   m_pProxy->VL_SetFogColorRGB(getVCmdBuffer(), 0);
   
   currRenderState.fgColor = 0;
   m_pProxy->VL_SetFGColorARGB(getVCmdBuffer(), 0);
}


vl_error 
Surface::SetSrcFilter(v_u32 in_srcFilter)
{
   if (in_srcFilter != currRenderState.srcFilter) {
      currRenderState.srcFilter = in_srcFilter;
      return m_pProxy->VL_SetSrcFilter(getVCmdBuffer(), in_srcFilter);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetSrcFunc(v_u32 in_srcFunc)
{
   if (in_srcFunc != currRenderState.srcFunc) {
      currRenderState.srcFunc = in_srcFunc;
      return m_pProxy->VL_SetSrcFunc(getVCmdBuffer(), in_srcFunc);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetChromaKey(v_u32 in_chroma)
{
//   if (in_chroma != currRenderState.chroma) {
//      currRenderState.chroma = in_chroma;
      m_pProxy->VL_SetTranspReject(getVCmdBuffer(), in_chroma);
      return m_pProxy->VL_SetChromaKey(getVCmdBuffer(), in_chroma);
//   }
//   return VL_SUCCESS;
}

vl_error 
Surface::SetChromaMask(v_u32 in_chromaMask,
                       v_u32 in_pixelFmt)
{
   if (in_chromaMask != currRenderState.chromaMask ||
       in_pixelFmt   != currRenderState.chromaMaskPFmt) {
      currRenderState.chromaMask     = in_chromaMask;
      currRenderState.chromaMaskPFmt = in_pixelFmt;
      return m_pProxy->VL_SetChromaMask(getVCmdBuffer(), in_chromaMask, in_pixelFmt);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetChromaColor(v_u32 in_chromaColor,
                        v_u32 in_pixelFmt)
{
   if (in_chromaColor != currRenderState.chromaColor ||
       in_pixelFmt    != currRenderState.chromaColorPFmt) {
      currRenderState.chromaColor     = in_chromaColor;
      currRenderState.chromaColorPFmt = in_pixelFmt;
      return m_pProxy->VL_SetChromaColor(getVCmdBuffer(), in_chromaColor, in_pixelFmt);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetDitherEnable(v_u32 in_dither)
{
   if (in_dither != currRenderState.dither) {
      currRenderState.dither = in_dither;
      return m_pProxy->VL_SetDitherEnable(getVCmdBuffer(), in_dither);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetBlendEnable(v_u32 in_blendEnable)
{
   if (in_blendEnable != currRenderState.blendEnable) {
      currRenderState.blendEnable = in_blendEnable;
      return m_pProxy->VL_SetBlendEnable(getVCmdBuffer(), in_blendEnable);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetBlendSrcFunc(v_u32 in_blendSrcFunc)
{
   if (in_blendSrcFunc != currRenderState.blendSrcFunc) {
      currRenderState.blendSrcFunc = in_blendSrcFunc;
      return m_pProxy->VL_SetBlendSrcFunc(getVCmdBuffer(), in_blendSrcFunc);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetBlendDstFunc(v_u32 in_blendDstFunc)
{
   if (in_blendDstFunc != currRenderState.blendDstFunc) {
      currRenderState.blendDstFunc = in_blendDstFunc;
      return m_pProxy->VL_SetBlendDstFunc(getVCmdBuffer(), in_blendDstFunc);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetFogEnable(v_u32 in_fogEnable)
{
   if (in_fogEnable != currRenderState.fogEnable) {
      currRenderState.fogEnable = in_fogEnable;
      return m_pProxy->VL_SetFogEnable(getVCmdBuffer(), in_fogEnable);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetF(v_u32 in_fogAttrib)
{
   if (in_fogAttrib != currRenderState.fogAttrib) {
      currRenderState.fogAttrib = in_fogAttrib;
      return m_pProxy->VL_SetF(getVCmdBuffer(), in_fogAttrib);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetFGColorARGB(v_u32 in_fgColor)
{
   if (in_fgColor != currRenderState.fgColor) {
      currRenderState.fgColor = in_fgColor;
      return m_pProxy->VL_SetFGColorARGB(getVCmdBuffer(), in_fgColor);
   }
   return VL_SUCCESS;
}

vl_error 
Surface::SetFogColorRGB(v_u32 in_fogColor)
{
   if (in_fogColor != currRenderState.fogColor) {
      currRenderState.fogColor = in_fogColor;
      return m_pProxy->VL_SetFogColorRGB(getVCmdBuffer(), in_fogColor);
   }
   return VL_SUCCESS;
}


void 
Surface::processMessage(const int   in_argc,
                        const char* in_argv[])
{
   if (!stricmp(in_argv[0], "bilinear")) {
      m_filterType = (m_filterType == V_SRCFILTER_BILINEAR) ? V_SRCFILTER_POINT :
                                                              V_SRCFILTER_BILINEAR;
   } else if (!stricmp(in_argv[0], "flushCache")) {
      flushTextureCache();
   } else if (!strcmp(in_argv[0], "bOffset") && in_argc == 2) {
      float offsetVal;
      sscanf(in_argv[1], "%f", &offsetVal);
      m_bilinearOffset = offsetVal;
   }
}


//------------------------------------------------------------------------------
#define VERITE_CALLBACKS 1
//--------------------------------------
//------------------------------------------------------------------------------
// NAME 
//    v_cmdbuffer V_CDECL
//    veriteCmdBuffOverflow(v_handle    io_vHandle,
//                          v_cmdbuffer io_overflowBuffer)
//    
// DESCRIPTION 
//    Simply forwards the overflow message to the surface that owns the handle,
//   which is located in the userData of the v_handle...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
v_cmdbuffer V_CDECL
veriteCmdBuffOverflow(v_handle    io_vHandle,
                      v_cmdbuffer io_overflowBuffer)
{
   // It would be nice to use the proxy in the surface, but since the call
   //  to get the surface goes through the proxy, we're kinda stuck here...
   //
   RenditionProxy* pProxy = GFXRenditionDevice::getRenditionProxy();
   v_u32 userData = pProxy->V_GetUserPrivate(io_vHandle);
   AssertFatal(userData != 0, "Error, no valid pointer to surface!");

   Surface* pSurface = reinterpret_cast<Surface*>(userData);
   return pSurface->cmdBuffOverflow(io_vHandle, io_overflowBuffer);
}

//------------------------------------------------------------------------------
#define UNIMPLEMENTED_FUNCTIONS 1
//--------------------------------------
void 
Surface::_lock()
{
   // NOP, surface does not lock in any user settable way
}

void 
Surface::_unlock()
{
   // NOP, surface does not lock in any user settable way
}

void 
Surface::_setGamma()
{
   // Set gamma is a NOP for a rendition surface...
   AssertWarn(0, "Confusing message: _setGamma sent to Rendition::Surface");
}

void 
Surface::_lockDC()
{
   AssertFatal(0, "DC Locking not permitted on accelerated Rendition::Surface");
}

void 
Surface::_unlockDC()
{
   AssertFatal(0, "DC Locking not permitted on accelerated Rendition::Surface");
}

void 
Surface::drawSurface(GFXSurface*    /*lpSourceSurface*/,
                     const RectI*   /*in_subRegion*/,
                     const Point2I* /*in_at*/)
{
   AssertWarn(0, "drawSurface not implmented for Rendition::Surface");
}

void 
Surface::drawSurfaceToBitmap(GFXBitmap* io_pBmp)
{
   if(io_pBmp->getWidth()  != surfaceWidth  ||
      io_pBmp->getHeight() != surfaceHeight ||
      io_pBmp->bitDepth != 24) {
      return;
   }
   
   UInt16* bufferTemp = new UInt16[surfaceWidth * surfaceHeight];
   
   UInt16* pBuffer = (UInt16*)m_pProxy->V_LockBuffer(m_vHandle,
                                                     m_pVSurface->buffer_group, 1);
   memcpy(bufferTemp, pBuffer, sizeof(UInt16) * surfaceWidth * surfaceHeight);
   m_pProxy->V_UnlockBuffer(m_vHandle, m_pVSurface->buffer_group, 1);
   
   for (int i = 0; i < surfaceHeight; i++) {
      UInt16* srcPtr = bufferTemp + i*surfaceWidth;
      unsigned char *dstPtr = io_pBmp->getAddress(0, i);

      for (int j = 0; j < surfaceWidth; j++) {
         UInt16 pix = *srcPtr++;
         *dstPtr++ = (pix & 0x1F) << 3;
         *dstPtr++ = ((pix >> 5) & 0x3F) << 2;
         *dstPtr++ = (pix >> 11) << 3;
      }
   }
   
   delete [] bufferTemp;
}

}; // namespace Rendition