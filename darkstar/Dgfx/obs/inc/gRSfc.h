//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GRSFC_H_
#define _GRSFC_H_

//Includes
#include <tVector.h>

#include "d_caps.h"
#include "g_surfac.h"
#include "redline.h"
#include "verite.h"
#include "gRTxCache.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GFXTextureCacheEntry;

namespace Rendition {

class RenditionProxy;
class Surface : public GFXSurface {
   // Callback functions need to be friends of the class...
   //
   friend v_cmdbuffer V_CDECL veriteCmdBuffOverflow(v_handle, v_cmdbuffer);
   
   friend class TextureCache;
   
   // Private constants
  private:
   enum { NUM_CMD_BUFFS = 4 };

   // Callback forwarded messages...
  private:
   v_cmdbuffer cmdBuffOverflow(v_handle    io_vHandle,
                                       v_cmdbuffer io_overflowBuffer,
                                       const bool  in_issueAsynch = true);
  private:
   // Class variables
   //
   static bool  sm_veriteRunning;
   
   // Instance members
   //
   RenditionProxy* m_pProxy;
   HWND            m_appHWnd;
   const bool      m_windowed;

   // Texture cache...
   //
   TextureCache* m_pTextureCache;

   // Verite specific instance members
   //
   v_handle   m_vHandle;     // Handle to verite instance...
   v_surface* m_pVSurface;   // Pointer to verite surface structure..

   v_cmdbuffer m_pCmdBuffs[NUM_CMD_BUFFS];   // Command buffers...
   v_cmdbuffer m_pCurrentBuffer;
   Int32       m_currBuffIndex;
   Int32       m_cmdBuffMemOffset[NUM_CMD_BUFFS];

  public:
   bool m_waitForDisplaySwitch;

   // Rendition Surfaces can only be created through create(), but they can be
   //  deleted by anybody
   //
  protected:
   struct {
      v_u32 srcFilter;
      v_u32 srcFunc;
      v_u32 srcFmt;
      v_u32 chroma;
      v_u32 chromaMask;
      v_u32 chromaMaskPFmt;
      v_u32 chromaColor;
      v_u32 chromaColorPFmt;
      v_u32 dither;
      
      v_u32 blendEnable;
      v_u32 blendSrcFunc;
      v_u32 blendDstFunc;

      v_u32 fogEnable;
      v_u32 fogAttrib;
      
      v_u32 fogColor;
      v_u32 fgColor;
   } currRenderState;

   void setDefaultRenderState();

  public:
   vl_error SetFogColorRGB(v_u32);
   vl_error SetFGColorARGB(v_u32);

   vl_error SetSrcFilter(v_u32);
   vl_error SetSrcFunc(v_u32);
   vl_error SetChromaKey(v_u32);
   vl_error SetChromaMask(v_u32, v_u32);
   vl_error SetChromaColor(v_u32, v_u32);
   vl_error SetDitherEnable(v_u32);

   vl_error SetBlendEnable(v_u32);
   vl_error SetBlendDstFunc(v_u32);
   vl_error SetBlendSrcFunc(v_u32);

   vl_error SetFogEnable(v_u32);
   vl_error SetF(v_u32);
   
   Surface(const bool in_windowed);
  public:
   virtual ~Surface();
  
  public:
   // We keep track of the number of times that draw3DBegin/End are called,
   //  since the filled rectangle needs to be emitted in 3d mode...
   //
   int m_draw3DBegun;

   // Create sets up a verite connection, and switches the display mode.  Note
   //  that if any windowed surfaces are created, a full screen instance is
   //  disallowed, likewise, if a fullscreen instance has been created, no
   //  windowed surfaces are allowed.  Only one fullscreen instance is allowed
   //  at any given time, though multiple instances of a windowed surface are
   //  allowed.  DMMNOTE: Need to work out some way to split up available
   //  texture memory between windowed instances...
   //
   static Surface* create(RenditionProxy* io_pProxy,
                                  HWND            io_appHWnd,
                                  const Int32     in_width,
                                  const Int32     in_height,
                                  const Int32     in_bpp,
                                  const bool      in_windowed);
   void restoreDisplayMode();

   // Messages forwarded to the texture cache...
   //
  public:
   // inline forwards...
   //
   void flushTextureCache() { m_pTextureCache->flushCache(); }
   bool drawingTexture() { return m_pTextureCache->drawingTexture(); }
   void primeForDraw()
      { m_pTextureCache->primeForDraw(); }
   void primeForBitmap(const bool in_transparent,
                               const bool in_translucent)
      { m_pTextureCache->primeForBitmap(in_transparent, in_translucent); }
   
   bool setTexture(const GFXTextureCacheEntry* in_tce);
   bool setTexture(const BYTE*  in_pTexture);

   void flushTexture(const GFXBitmap*, const bool);
   void downloadTexture(const GFXTextureCacheEntry* in_tce,
                        const bool in_enableAlpha  = false,
                        const bool in_enableTransp = false);
   void downloadTexture(BYTE*        in_pTexture,
                        const UInt32 in_stride,
                        const UInt32 in_height,
                        const bool   in_enableAlpha  = false,
                        const bool   in_enableTransp = false);
   
   // Access functions for Function table members.  Probably not useful
   //  to anyone else...
   //
  public:
   v_handle        getVHandle()        { return m_vHandle; }
   v_surface*      getVSurface()       { return m_pVSurface; }
   v_cmdbuffer*    getVCmdBuffer()     { return &m_pCurrentBuffer; }
   RenditionProxy* getRenditionProxy() { return m_pProxy; }
   
   Int32 getVCmdBufferMemOffset() const;
   void  setVCmdBufferMemOffset(UInt32 numBytes);
   
   
   v_u32 m_filterType;
   v_u32 getFilterType() const { return m_filterType; }
   void  setFilterType(const v_u32 in_filterType) { m_filterType = in_filterType; }
   void  setFilterEnable(const bool in_enable);
   float m_bilinearOffset;
   
   
   DWORD getCaps();
   // Pure virtual overrides for GFXSurface...
   //
  protected:
   Bool _setPalette(GFXPalette *lpPalette,
                            Int32 in_start, Int32 in_count, Bool);
   void _setGamma();
   void _lock();
   void _unlock();
   void _lockDC();
   void _unlockDC();
  public:
   void flip();
   void drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at);
	void drawSurfaceToBitmap(GFXBitmap *bmp);

   // For messaging the surface from the console, not currently used...
   //
  public:
   void processMessage(const int in_argc,
                               const char* in_argv[]);
   
   // Internal state management
   //
  protected:
   void generateTranslationPalette();
};

inline Int32 
Surface::getVCmdBufferMemOffset() const
{
   return m_cmdBuffMemOffset[m_currBuffIndex];
}

inline void 
Surface::setVCmdBufferMemOffset(UInt32 in_numBytes)
{
   m_cmdBuffMemOffset[m_currBuffIndex] = in_numBytes;
}

}; // namespace Rendition

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GRSFC_H_
