//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gD3DPQueue.h"
#include "gD3DSfc.h"
#include "gD3DTxTrans.h"
#include "gD3DTxCache.h"


extern char *D3DAppErrorToString(HRESULT);

namespace Direct3D {

int __cdecl
queueCmp(const void* in_p1, const void* in_p2)
{
   // We need to insure that all lightmaps are drawn AFTER the polys they
   //  are applied to, so if a LMap mismatch occurs, this takes precedence
   //  over the texture key...
   //
   QueueEntry* p1 = (QueueEntry*)in_p1;
   QueueEntry* p2 = (QueueEntry*)in_p2;

   if (p1->primType != p2->primType) {
      return int(p1->primType - p2->primType);
   }

   if (p1->isLMap == true && p2->isLMap == false) {
      return 1;
   } else if (p2->isLMap == true && p1->isLMap == false) {
      return -1;
   } else {
      return (p1->textureKey - p2->textureKey);
   }
}



PolyQueue::PolyQueue(Int32 in_numEntries, Surface* io_pSurface)
 : m_numQueuedPolys(0),
   m_numQueueEntries(in_numEntries),
   pSurface(io_pSurface)
{
   m_pQueue = new QueueEntry[m_numQueueEntries];
}


PolyQueue::~PolyQueue()
{
   AssertFatal(m_numQueuedPolys == 0, "Error, deleting with queued polys");

   delete [] m_pQueue;
   m_pQueue = NULL;
   m_numQueuedPolys  = 0;
   m_numQueueEntries = 0;
   pSurface = NULL;
}


void 
PolyQueue::addPoly(const DWORD          in_texKey,
                   const bool           in_isLMap,
                   const bool           in_clamp,
                   const GFXShadeSource in_shadeSource,
                   const GFXAlphaSource in_alphaSource,
                   const GFXHazeSource  in_hazeSource,
                   const GFXFillMode    in_fillMode,
                   const int            in_zEnabled,
                   const bool           in_colorKeyEnabled,
                   const D3DTLVERTEX*   in_pVertices,
                   const Int32          in_numVertices,
                   const bool           in_turnOffFiltering)
{
   QueueEntry* pEntry = &m_pQueue[m_numQueuedPolys];

   pEntry->primType    = PolyPrim;
   pEntry->textureKey  = in_texKey;
   pEntry->isLMap      = in_isLMap;
   pEntry->isCallback  = false;
   pEntry->clamp       = in_clamp;
   pEntry->zEnabled    = in_zEnabled;
   pEntry->CKEnabled   = in_colorKeyEnabled;
   pEntry->cb          = NULL;
   pEntry->size        = -1;
   pEntry->shadeSource = in_shadeSource;
   pEntry->hazeSource  = in_hazeSource;
   pEntry->alphaSource = in_alphaSource;
   pEntry->fillMode    = in_fillMode;
   pEntry->filterOn    = !in_turnOffFiltering;

   pEntry->pVertices   = in_pVertices;
   pEntry->numVertices = in_numVertices;

   m_numQueuedPolys++;
   if (m_numQueuedPolys == m_numQueueEntries)
      pSurface->flushPolyQueue();
}


void 
PolyQueue::addPoly(const DWORD            in_texKey,
                   const bool             in_clamp,
                   const GFXCacheCallback in_cb,
                   const GFXTextureHandle in_texHandle,
                   const Int32            in_size,
                   const GFXShadeSource   in_shadeSource,
                   const GFXAlphaSource   in_alphaSource,
                   const GFXHazeSource    in_hazeSource,
                   const GFXFillMode      in_fillMode,
                   const int              in_zEnabled,
                   const D3DTLVERTEX*     in_pVertices,
                   const Int32            in_numVertices,
                   const bool             in_turnOffFiltering)
{
   QueueEntry* pEntry = &m_pQueue[m_numQueuedPolys];

   pEntry->primType    = PolyPrim;
   pEntry->textureKey  = in_texKey;
   pEntry->isLMap      = false;
   pEntry->isCallback  = true;
   pEntry->clamp       = in_clamp;
   pEntry->zEnabled    = in_zEnabled;
   pEntry->cb          = in_cb;
   pEntry->texHandle   = in_texHandle;
   pEntry->size        = in_size;
   pEntry->shadeSource = in_shadeSource;
   pEntry->hazeSource  = in_hazeSource;
   pEntry->alphaSource = in_alphaSource;
   pEntry->fillMode    = in_fillMode;
   pEntry->filterOn    = !in_turnOffFiltering;

   pEntry->pVertices   = in_pVertices;
   pEntry->numVertices = in_numVertices;

   m_numQueuedPolys++;
   if (m_numQueuedPolys == m_numQueueEntries)
      pSurface->flushPolyQueue();
}


void 
PolyQueue::addLine(const D3DTLVERTEX* in_pVertices,
                   const int          in_zEnable)
{
   if (m_numQueuedPolys == m_numQueueEntries)
      pSurface->flushPolyQueue();

   QueueEntry* pEntry  = &m_pQueue[m_numQueuedPolys++];
   pEntry->primType    = LinePrim;
   pEntry->pVertices   = in_pVertices;
   pEntry->zEnabled    = in_zEnable;
}

void 
PolyQueue::addPoint(const D3DTLVERTEX* in_pVertex,
                    const int          in_zEnable)
{
   QueueEntry* pEntry = &m_pQueue[m_numQueuedPolys++];
   pEntry->primType    = PointPrim;
   pEntry->pVertices   = in_pVertex;
   pEntry->zEnabled    = in_zEnable;

   m_numQueuedPolys++;
   if (m_numQueuedPolys == m_numQueueEntries)
      pSurface->flushPolyQueue();
}


void 
PolyQueue::flush(TextureCache*     io_pTextureCache,
                 IDirect3DDevice2* io_pD3DDevice2)
{
   // First, sort the queue...
   sortQueue();

   // and emit all sorted polys
   for (int i = 0; i < m_numQueuedPolys; i++) {
      if (m_pQueue[i].primType == PolyPrim) {
         emitQueuedPoly(io_pTextureCache,
                        io_pD3DDevice2, &m_pQueue[i]);
      } else if (m_pQueue[i].primType == LinePrim) {
         emitQueuedLine(io_pD3DDevice2, &m_pQueue[i]);
      } else {
         emitQueuedPoint(io_pD3DDevice2, &m_pQueue[i]);
      }
   }
   m_numQueuedPolys = 0;
}


void 
PolyQueue::sortQueue()
{
   if (m_numQueuedPolys <= 1)
      return;

   qsort(m_pQueue, m_numQueuedPolys, sizeof(QueueEntry), queueCmp);
}


void 
PolyQueue::emitQueuedLine(IDirect3DDevice2* io_pD3DDevice2,
                          const QueueEntry* in_pEntry)
{
   AssertFatal(io_pD3DDevice2 != NULL, "Error, no device");
   AssertFatal(in_pEntry != NULL, "Error, no entry...");

   pSurface->setRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL);
   HRESULT result = io_pD3DDevice2->DrawPrimitive(D3DPT_LINELIST,
                                                  D3DVT_TLVERTEX,
                                                  (void*)in_pEntry->pVertices, 2,
                                                  D3DDP_DONOTCLIP);
   AssertWarn(result == D3D_OK, avar("Problem emitting line: %s",
                                     D3DAppErrorToString(result)));
}

void 
PolyQueue::emitQueuedPoint(IDirect3DDevice2* io_pD3DDevice2,
                           const QueueEntry* in_pEntry)
{
   AssertFatal(io_pD3DDevice2 != NULL, "Error, no device");
   AssertFatal(in_pEntry != NULL, "Error, no entry...");

   pSurface->setRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL);
   HRESULT result = io_pD3DDevice2->DrawPrimitive(D3DPT_POINTLIST,
                                                  D3DVT_TLVERTEX,
                                                  (void*)in_pEntry->pVertices, 1,
                                                  D3DDP_DONOTCLIP);
   AssertWarn(result == D3D_OK, avar("Problem emitting line: %s",
                                     D3DAppErrorToString(result)));
}

void 
PolyQueue::emitQueuedPoly(TextureCache*     io_pTextureCache,
                          IDirect3DDevice2* io_pD3DDevice2,
                          const QueueEntry* in_pEntry)
{
   AssertFatal(io_pTextureCache != NULL, "Error, no TCache");
   AssertFatal(io_pD3DDevice2 != NULL, "Error, no device");
   AssertFatal(in_pEntry != NULL, "Error, no entry...");

   // Set up the renderstate, and download the texture if necessary
   if (in_pEntry->fillMode == GFX_FILL_TWOPASS) {
      prepareTwoPassPoly(io_pTextureCache, in_pEntry);
   } else if (in_pEntry->fillMode == GFX_FILL_TEXTURE) {
      prepareTexturePoly(io_pTextureCache, in_pEntry);
   } else if (in_pEntry->fillMode == GFX_FILL_CONSTANT) {
      prepareConstantPoly(io_pTextureCache, in_pEntry);
   } else {
      AssertFatal(0, "Unknown poly type!");
   }

   // finalize render settings
   io_pTextureCache->activateCurrentTexture(pSurface);

   D3DPRIMITIVETYPE d3dpt;
   AssertFatal(in_pEntry->numVertices >= 3, "Error, not enough verts!");
   if (in_pEntry->numVertices == 3)
      d3dpt = D3DPT_TRIANGLELIST;
   else
      d3dpt = D3DPT_TRIANGLEFAN;

   HRESULT result = io_pD3DDevice2->DrawPrimitive(d3dpt,
                                                  D3DVT_TLVERTEX,
                                                  (void*)in_pEntry->pVertices,
                                                  in_pEntry->numVertices,
                                                  D3DDP_DONOTCLIP);
   AssertWarn(result == D3D_OK, avar("Problem emitting polygon: %s",
                                     D3DAppErrorToString(result)));
}


void 
PolyQueue::prepareTwoPassPoly(TextureCache*     io_pTextureCache,
                              const QueueEntry* in_pEntry)
{
   // Make sure the texture is set and on the card
   //
   if (in_pEntry->isLMap == false) {
      // it's the texture...
      //
      if (io_pTextureCache->setCurrentTexture(in_pEntry->textureKey) == false) {
         if (in_pEntry->isCallback == false) {
            // We know that for non-cb TWOPASS textures that:
            //  texture is always a normal bitmap
            //  The texture key is the bitmap pointer
            //
            io_pTextureCache->downloadBitmap(pSurface,
                                             (GFXBitmap*)in_pEntry->textureKey,
                                             in_pEntry->textureKey);
         } else {
            AssertFatal(in_pEntry->cb != NULL, "Error, null callback");
            io_pTextureCache->downloadBitmap(pSurface,
                                             in_pEntry->cb,
                                             in_pEntry->texHandle,
                                             in_pEntry->size,
                                             in_pEntry->textureKey);
         }
      }

      if (in_pEntry->shadeSource != GFX_SHADE_NONE)
         pSurface->setShadeEnable(true, true);
      else
         pSurface->setShadeEnable(false, true);

      if (in_pEntry->hazeSource != GFX_HAZE_NONE) {
         pSurface->setFogEnable(true, true);
         pSurface->setRenderState(D3DRENDERSTATE_FOGCOLOR, D3DRGB(pSurface->hazeColor.r,
                                                                  pSurface->hazeColor.g,
                                                                  pSurface->hazeColor.b));
      } else {
         pSurface->setFogEnable(false, true);
      }

      pSurface->setColorKey(false, true);
      pSurface->setFilterEnable(true, true);

      if (in_pEntry->clamp == true)
         pSurface->setWrapEnable(false, true);
      else
         pSurface->setWrapEnable(true, true);

      pSurface->setZEnable(true, true);
   } else {
      // It's a lightmap
      //
      if (io_pTextureCache->setCurrentTexture(in_pEntry->textureKey) == false) {
         // we know that
         //  texture key is the lightmap pointer
         //
         io_pTextureCache->downloadLightMap(pSurface,
                                            (GFXLightMap*)in_pEntry->textureKey,
                                            in_pEntry->textureKey);
      }

      if (in_pEntry->hazeSource != GFX_HAZE_NONE) {
         pSurface->setFogEnable(true, true);
         pSurface->setRenderState(D3DRENDERSTATE_FOGCOLOR, D3DRGB(1.0, 1.0, 1.0));
      } else {
         pSurface->setFogEnable(false, true);
      }
      
      // colorkey and wrapping always off...
      pSurface->setColorKey(false, true);
      pSurface->setWrapEnable(false, true);
      pSurface->setZEnable(true, true);
      pSurface->setFilterEnable(true, true);
   }
}


void 
PolyQueue::prepareTexturePoly(TextureCache*     io_pTextureCache,
                              const QueueEntry* in_pEntry)
{
   AssertFatal(in_pEntry->isLMap == false, "Error, lightmaps can't be textures");

   // Make sure the texture is set and on the card
   //
   if (io_pTextureCache->setCurrentTexture(in_pEntry->textureKey) == false) {
      // We know that:
      //  textured polys are not callback
      //  texture key is the bitmap pointer...
      //
      if (in_pEntry->CKEnabled == true) {
         io_pTextureCache->downloadTransparentBitmap(pSurface,
                                                     (GFXBitmap*)in_pEntry->textureKey,
                                                     in_pEntry->textureKey);
      } else if (in_pEntry->alphaSource == GFX_ALPHA_TEXTURE) {
         io_pTextureCache->downloadAlphaBitmap(pSurface,
                                               (GFXBitmap*)in_pEntry->textureKey,
                                               in_pEntry->textureKey);
      } else {
         io_pTextureCache->downloadBitmap(pSurface,
                                          (GFXBitmap*)in_pEntry->textureKey,
                                          in_pEntry->textureKey);
      }
   }

   if (in_pEntry->shadeSource != GFX_SHADE_NONE)
      pSurface->setShadeEnable(true, true);
   else
      pSurface->setShadeEnable(false, true);

   pSurface->setSpecularEnable(false, true);
   
   pSurface->setFilterEnable(in_pEntry->filterOn, true);
   
   if (in_pEntry->hazeSource != GFX_HAZE_NONE) {
      pSurface->setFogEnable(true, true);
   } else {
      pSurface->setFogEnable(false, true);
   }

   if (in_pEntry->clamp == true)
      pSurface->setWrapEnable(false, true);
   else
      pSurface->setWrapEnable(true, true);

   pSurface->setZEnable(in_pEntry->zEnabled, true);

   // colorkey is handled by the texture munger when
   //  the texture is activated
}


void 
PolyQueue::prepareConstantPoly(TextureCache*     io_pTextureCache,
                               const QueueEntry* in_pEntry)
{
   io_pTextureCache->setCurrentTexture(0);

   if (pSurface->shadeSource != GFX_SHADE_NONE)
      pSurface->setShadeEnable(true, true);
   else
      pSurface->setShadeEnable(false, true);

   pSurface->setSpecularEnable(false);
   if (in_pEntry->hazeSource != GFX_HAZE_NONE)
      pSurface->setFogEnable(true, true);
   else
      pSurface->setFogEnable(false, true);
   
   if (in_pEntry->alphaSource == GFX_ALPHA_CONSTANT) {
      pSurface->setRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA);
      pSurface->setRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
      pSurface->setRenderState(D3DRENDERSTATE_BLENDENABLE,      true);
      pSurface->setRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
   } else {
      pSurface->setRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
      pSurface->setRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
      pSurface->setRenderState(D3DRENDERSTATE_BLENDENABLE,      false);
      pSurface->setRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
   }

   pSurface->setZEnable(in_pEntry->zEnabled, true);
}

}; // namespace Direct3D

