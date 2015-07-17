//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GD3DPQUEUE_H_
#define _GD3DPQUEUE_H_

//Includes

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#include "base.h"
#include "d_funcs.h"

#include <d3d.h>
#include <d3dcaps.h>
#include <d3dtypes.h>

namespace Direct3D {

class Surface;
class Munger;
class TextureMunger;
class TextureCache;

enum PrimType {
   PolyPrim   = 0,
   LinePrim   = 1,
   PointPrim  = 2,
   ForceDWORD = 0x3fffffff
};

struct QueueEntry {
   DWORD    textureKey;
   PrimType primType;
   
   int zEnabled;

   GFXCacheCallback cb;
   GFXTextureHandle texHandle;
   Int32            size;

   GFXShadeSource shadeSource;
   GFXAlphaSource alphaSource;
   GFXHazeSource  hazeSource;
   GFXFillMode    fillMode;

   const D3DTLVERTEX*   pVertices;
   Int32                numVertices;

   bool filterOn;
   bool isLMap;
   bool isCallback;
   bool clamp;
   bool CKEnabled;
};


class PolyQueue {
  private:
   Int32       m_numQueueEntries;
   QueueEntry* m_pQueue;
   
   Surface* pSurface;

   Int32 m_numQueuedPolys;

   void sortQueue();
   void emitQueuedPoly(TextureCache*     io_pTextureCache,
                               IDirect3DDevice2* io_pD3DDevice2,
                               const QueueEntry* in_pEntry);
   void emitQueuedLine(IDirect3DDevice2* io_pD3DDevice2,
                               const QueueEntry* in_pEntry);
   void emitQueuedPoint(IDirect3DDevice2* io_pD3DDevice2,
                                const QueueEntry* in_pEntry);

   void prepareTwoPassPoly(TextureCache*     io_pTextureCache,
                                   const QueueEntry* in_pEntry);
   void prepareTexturePoly(TextureCache*     io_pTextureCache,
                                   const QueueEntry* in_pEntry);
   void prepareConstantPoly(TextureCache*     io_pTextureCache,
                                    const QueueEntry* in_pEntry);
  public:
   PolyQueue(Int32 in_numEntries, Surface* io_pSurface);
   ~PolyQueue();

   void addPoly(const DWORD          in_texKey,
                        const bool           in_isLMap,
                        const bool           in_clamp,
                        const GFXShadeSource in_shadeSource,
                        const GFXAlphaSource in_alphaSource,
                        const GFXHazeSource  in_hazeSource,
                        const GFXFillMode    in_fillMode,
                        const int            in_zEnable,
                        const bool           in_colorKeyEnabled,
                        const D3DTLVERTEX*   in_pVertices,
                        const Int32          in_numVertices,
                        const bool           in_turnOffFiltering = false);

   void addPoly(const DWORD            in_texKey,
                        const bool             in_clamp,
                        const GFXCacheCallback in_cb,
                        const GFXTextureHandle in_texHandle,
                        const Int32            in_size,
                        const GFXShadeSource   in_shadeSource,
                        const GFXAlphaSource   in_alphaSource,
                        const GFXHazeSource    in_hazeSource,
                        const GFXFillMode      in_fillMode,
                        const int              in_zEnable,
                        const D3DTLVERTEX*     in_pVertices,
                        const Int32            in_numVertices,
                        const bool             in_turnOffFiltering = false);

   void addLine(const D3DTLVERTEX* in_pVertices,
                        const int          in_zEnable);
   void addPoint(const D3DTLVERTEX* in_pVertex,
                         const int          in_zEnable);

   void flush(TextureCache*     io_pTextureCache,
                      IDirect3DDevice2* io_pD3DDevice2);
};

}; // namespace Direct3D

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GD3DPQUEUE_H_
