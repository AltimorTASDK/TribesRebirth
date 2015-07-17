//================================================================
// 
// Initial Author: Rick Overman 
// 
// Description 
// 
// $Workfile:   g_ddraw.cpp  $
// $Revision:   1.8  $
// $Author  $
// $Modtime $
//
//================================================================

#include "g_ddraw.h"
#include "g_pal.h"
#include "g_bitmap.h"
#include "ddrawerr.h"
#include "r_clip.h"
#include "fn_table.h"
#include "console.h"

namespace {

PALETTEENTRY
applyGammaCorrection(const PALETTEENTRY& in_rRGB, const float in_gamma)
{
   PALETTEENTRY newEntry;

   float r = (float(in_rRGB.peRed)   / 255.0f);
   float g = (float(in_rRGB.peGreen) / 255.0f);
   float b = (float(in_rRGB.peBlue)  / 255.0f);

   newEntry.peRed   = UInt8((pow(r, 1.0f / in_gamma) * 255.0f) + 0.5f);
   newEntry.peGreen = UInt8((pow(g, 1.0f / in_gamma) * 255.0f) + 0.5f);
   newEntry.peBlue  = UInt8((pow(b, 1.0f / in_gamma) * 255.0f) + 0.5f);
   newEntry.peFlags = in_rRGB.peFlags;

   return newEntry;
}

} // namespace {}


//BWA -- handle to direct draw library...
static HINSTANCE hmodDD = NULL;

DDCAPS        GFXDDSurface::driverCaps;
DDCAPS        GFXDDSurface::helCaps;
IDirectDraw*  GFXDDSurface::pDD           = NULL;
IDirectDraw2* GFXDDSurface::pDD2          = NULL;
bool          GFXDDSurface::writeOnly     = FALSE;
bool          GFXDDSurface::sm_stretchBlt = false;
HWND          GFXDDSurface::DDSurfaceHWnd = NULL;

//BWA -- function pointer to dynamically load DirectDraw functions...
typedef HRESULT (WINAPI *DDrawFuncPtr)( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );

void GFXDDSurface::setSurfaceAlphaBlend(ColorF *color, float alpha)
{
   alphaColor = *color;
   alphaBlend = alpha;
}

bool GFXDDSurface::init(HWND hWnd)
{
   HRESULT result;
   
   AssertFatal( hWnd,  "GFXDDSurface::init: Invalid hWnd HANDLE.");
   
   //BWA -- load the function pointer from the DLL...
   hmodDD = LoadLibrary("DDRAW.DLL");
   AssertWarn( hmodDD, "GFXDDSurface::init: DDraw LoadLibrary Failed.");
   DDrawFuncPtr fpDirectDrawCreate = (DDrawFuncPtr)GetProcAddress(hmodDD, "DirectDrawCreate");
   AssertWarn( fpDirectDrawCreate, "GFXDDSurface::init: DDraw GetProcAddress Failed.");
   if (!fpDirectDrawCreate) {
      if ( hmodDD )
         FreeLibrary( hmodDD );
      return ( FALSE );
   }
   
   result = (fpDirectDrawCreate)( NULL, &pDD, NULL );  //call to function pointer
   AssertWarn(result == DD_OK, avar("GFXDDSurface::init: %s", DDRAW_ERROR(result)));
   if ( result != DD_OK ) return ( FALSE );

   result = pDD->QueryInterface(IID_IDirectDraw2, (LPVOID *) & pDD2);
   AssertWarn(result == DD_OK, avar("GFXDDSurface::init: %s", DDRAW_ERROR(result)));
   if ( result != DD_OK ) return ( FALSE );
   
   DDSurfaceHWnd = hWnd;
   driverCaps.dwSize = helCaps.dwSize = sizeof(DDCAPS);
   result = pDD2->GetCaps(&driverCaps, &helCaps);
   AssertWarn(result == DD_OK, avar("GFXDDSurface::init: %s", DDRAW_ERROR(result)));
   if ( result != DD_OK ) return ( FALSE );
   
   if(driverCaps.ddsCaps.dwCaps & DDSCAPS_WRITEONLY)
      writeOnly = TRUE;
   
   if (driverCaps.dwCaps & DDCAPS_BLTSTRETCH)
      sm_stretchBlt = true;
   else
      sm_stretchBlt = false;

   return TRUE;
}

void GFXDDSurface::free(void)
{
   if ( pDD2 != NULL ) {
      pDD2->Release();
      pDD2 = NULL;
   }
   if ( pDD != NULL ) {
      pDD->Release();
      pDD = NULL;
   }
   if ( hmodDD ) {
      FreeLibrary( hmodDD );
      hmodDD = NULL;
   }
}

GFXDDSurface::GFXDDSurface()
{
   pSurface        = NULL;
   pBackSurface    = NULL;
   pDDPalette      = NULL;
   pFullMemSurface = NULL;
   pHalfMemSurface = NULL;
   pStretchSurface = NULL;

   surfaceType = SURFACE_DIRECT_DRAW;  
   functionTable = &rclip_table;

   m_coopSet    = false;
   m_vidModeSet = false;
}

GFXDDSurface::~GFXDDSurface()
{
   if (pStretchSurface != NULL)
      pStretchSurface->Release();
   pStretchSurface = NULL;

   if (pDDPalette)
      pDDPalette->Release();
   pDDPalette = NULL;

   if (pSurface != NULL && isPrimary == true)
      pSurface->Release();
   pSurface     = NULL;
   pBackSurface = NULL;

   if (m_vidModeSet == true)
      pDD2->RestoreDisplayMode();
   if (m_coopSet == true)
      pDD2->SetCooperativeLevel(DDSurfaceHWnd, DDSCL_NORMAL);

   delete pFullMemSurface;
   pFullMemSurface = NULL;
   delete pHalfMemSurface;
   pHalfMemSurface = NULL;
}

Bool GFXDDSurface::create(GFXSurface*& io_rpSurface, UInt32 in_width, UInt32 in_height)
{
   DDSURFACEDESC       ddsd;
   PALETTEENTRY        pe[256] = {0,};
   HRESULT             result;
   bool success = true;

   AssertFatal(pDD2 != NULL, "GFXDDSurface::create: init first.");
   
   // Set up the new surface...
   io_rpSurface = new GFXDDSurface;
   success &= (io_rpSurface != NULL);
   
   GFXDDSurface* pDDSurface = ((GFXDDSurface*)io_rpSurface);

   while ( success )
   {
      if (writeOnly)
         io_rpSurface->setFlag(GFX_DMF_WRITEONLY);

      // Set exclusive mode
      result = pDD2->SetCooperativeLevel(DDSurfaceHWnd, DDSCL_EXCLUSIVE       |
                                                        DDSCL_FULLSCREEN      |
                                                        DDSCL_NOWINDOWCHANGES);
      AssertWarn(result == DD_OK, avar("GFXDDSurface::create: %s", DDRAW_ERROR(result)));
      if (result != DD_OK)
         break;
      pDDSurface->m_coopSet = true;


      result = pDD2->SetDisplayMode(in_width, in_height, 8, 0, 0);
      AssertWarn(result == DD_OK, avar("GFXDDSurface::create: %s", DDRAW_ERROR(result)));
      if (result != DD_OK)
         break;
      pDDSurface->m_vidModeSet = true;
      
      // Create the surface
      memset( &ddsd, 0, sizeof( ddsd ) );
      ddsd.dwSize         = sizeof( ddsd );
      ddsd.dwFlags        = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
      ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                            DDSCAPS_FLIP           |
                            DDSCAPS_COMPLEX;
      ddsd.dwBackBufferCount = 1;
      result = pDD2->CreateSurface( &ddsd, &(pDDSurface->pSurface), NULL );
      AssertWarn(result == DD_OK, avar("GFXDDSurface::create: %s", DDRAW_ERROR(result)));
      if( result != DD_OK )
         break;

      ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
      result = pDDSurface->pSurface->GetAttachedSurface(&ddsd.ddsCaps, &(pDDSurface->pBackSurface) );
      AssertWarn(result == DD_OK, avar("GFXDDSurface::create: getting back buffer: %s", DDRAW_ERROR(result)));
      if( result != DD_OK )
         break;
      
      //create a palette for the primary surface
      result = pDD2->CreatePalette( DDPCAPS_8BIT, pe, &pDDSurface->pDDPalette, NULL );
      AssertWarn(result == DD_OK, avar("GFXDDSurface::create: %s", DDRAW_ERROR(result)));
      if ( result != DD_OK ) break;

      result = pDDSurface->pSurface->SetPalette(pDDSurface->pDDPalette);
      AssertWarn(result == DD_OK, avar("GFXDDSurface::create: %s", DDRAW_ERROR(result)));
      if ( result != DD_OK ) break;

      pDDSurface->isPrimary       = true;
      pDDSurface->surfaceHeight   = in_height;
      pDDSurface->surfaceWidth    = in_width;
      pDDSurface->surfaceBitDepth = 8;
      pDDSurface->setFlag(GFX_DMF_BACKPAGE_CARDMEM);
      
      RectI r;
      io_rpSurface->getClientRect(&r);
      io_rpSurface->setClipRect(&r);

      // Ok, we have the primary surface.  Now we need to create the secondary
      //  ddraw surface, if possible, and the memory surfaces...
      //
      pDDSurface->pFullMemSurface = (GFXMemSurface*)GFXMemSurface::create(pDDSurface->surfaceWidth,
                                                                          pDDSurface->surfaceHeight);
      pDDSurface->pHalfMemSurface = (GFXMemSurface*)GFXMemSurface::create(pDDSurface->surfaceWidth  / 2,
                                                                          pDDSurface->surfaceHeight / 2);
      AssertWarn(pDDSurface->pFullMemSurface && pDDSurface->pHalfMemSurface,
                 "WTF!  Can't create mem surfaces?");
      if (pDDSurface->pFullMemSurface == NULL || pDDSurface->pHalfMemSurface == NULL)
         break;

      // If the driver supports it, create the stretching surface...
      //
      if (sm_stretchBlt) {
         memset( &ddsd, 0, sizeof( ddsd ) );
         ddsd.dwSize = sizeof( ddsd );
         ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
         ddsd.dwWidth  = pDDSurface->surfaceWidth  / 2;
         ddsd.dwHeight = pDDSurface->surfaceHeight / 2;
         ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
         result = pDD2->CreateSurface( &ddsd, &pDDSurface->pStretchSurface, NULL );

         if (result != DD_OK) {
            pDDSurface->pStretchSurface = NULL;
         }
      } else {
         pDDSurface->pStretchSurface = NULL;
      }

      return (TRUE);
   }

   //if we break out of the above while loop
   //an error occured and we need to clean up the mess
   delete io_rpSurface;
   io_rpSurface = NULL;

   return FALSE;
}


void
GFXDDSurface::_lock(const GFXLockMode in_lockMode)
{
   GFXMemSurface* pMemSurface;
   if (in_lockMode == GFX_NORMALLOCK)
      pMemSurface = pFullMemSurface;
   else 
      pMemSurface = pHalfMemSurface;

   m_lastLockMode = in_lockMode;
   m_pLockSurface = pMemSurface;

   pMemSurface->lock();
   pSurfaceBits  = pMemSurface->getAddress();
   surfaceHeight = pMemSurface->getHeight();
   surfaceWidth  = pMemSurface->getWidth();
   surfaceStride = pMemSurface->getStride();
}


void GFXDDSurface::_unlock()
{
   m_pLockSurface->unlock();
   m_pLockSurface = NULL;

   pSurfaceBits  = NULL;
   surfaceStride = 0;
   surfaceHeight = pFullMemSurface->getHeight();
   surfaceWidth  = pFullMemSurface->getWidth();
}

void GFXDDSurface::updateAlphaBlend()
{
   if ((alphaBlend != 0 || lastAlphaBlend != 0) && pPalette != NULL)
   {
      if(alphaBlend != lastAlphaBlend || alphaColor.red != lastAlphaColor.red
         || alphaColor.green != lastAlphaColor.green || alphaColor.blue != lastAlphaColor.blue)
      {
         lastAlphaBlend = alphaBlend;
         lastAlphaColor = alphaColor;

         int i;
         float ared   = alphaColor.red   * alphaBlend * 255.0;
         float agreen = alphaColor.green * alphaBlend * 255.0;
         float ablue  = alphaColor.blue  * alphaBlend * 255.0;

         float mulfactor = 1 - alphaBlend;

         PALETTEENTRY color[256];
         for(i = 0; i < 256; i++)
         {
            color[i].peRed   = min(int(ared   + pPalette->palette[0].color[i].peRed   * mulfactor), 255);
            color[i].peGreen = min(int(agreen + pPalette->palette[0].color[i].peGreen * mulfactor), 255);
            color[i].peBlue  = min(int(ablue  + pPalette->palette[0].color[i].peBlue  * mulfactor), 255);
            color[i].peFlags = 0;

            color[i] = applyGammaCorrection(color[i], gamma);
         }
         HRESULT result;

         if (pDDPalette) {
            result = pDDPalette->SetEntries(0, 0, 256, color);
            AssertWarn(result == DD_OK, avar("GFXDDSurface::updateAlphaBlend: %s", DDRAW_ERROR(result)));
         } else {
            AssertWarn(0, "No directdraw palette?");
         }
      }
   }
}

void GFXDDSurface::update(const RectI *copyRect)
{
   AssertFatal(isPrimary == true, "GFXDDSurface::update: Must be called on the primary (front) surface.");
   AssertFatal(pages > 1, "GFXDDSurface::update: No back page.");
   updateAlphaBlend();
      
   if(copyRect)
      drawSurface(next, copyRect, &(copyRect->upperL));
   else
      drawSurface(next, &RectI(0,0,surfaceWidth-1, surfaceHeight-1), &Point2I(0,0));
}


extern "C" {
void __cdecl
gfx_stretch2_blit(BYTE*  pDst, UInt32 dstStride,
                  BYTE*  pSrc, UInt32 srcStride,
                  UInt32 height);
}

void
GFXDDSurface::flipDouble()
{
   HRESULT result;

   DDSURFACEDESC ddsd;
   memset( &ddsd, 0, sizeof( ddsd ) );
   ddsd.dwSize  = sizeof( ddsd );

   if(pSurface->IsLost() == DDERR_SURFACELOST)
      pSurface->Restore();

   if (pStretchSurface == NULL) {
      result = pBackSurface->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL );

      AssertWarn(result == DD_OK, avar("GFXDDSurface::lock: %s", DDRAW_ERROR(result)));
      if ( result == DD_OK ) {
         BYTE*  pDest   = (BYTE*)ddsd.lpSurface;
         Int32 iStride = ddsd.lPitch;
   
         pHalfMemSurface->lock();

         gfx_stretch2_blit(pDest, iStride,
                           pHalfMemSurface->getAddress(),
                           pHalfMemSurface->getWidth(),
                           pHalfMemSurface->getHeight());

         pHalfMemSurface->unlock();
         result = pBackSurface->Unlock( NULL );
         AssertWarn(result == DD_OK, avar("GFXDDSurface::unlock: %s", DDRAW_ERROR(result)));
      }
   } else {
      if (pStretchSurface->IsLost() == DDERR_SURFACELOST)
         pStretchSurface->Restore();

      DDSURFACEDESC ddsd;
      memset( &ddsd, 0, sizeof( ddsd ) );
      ddsd.dwSize  = sizeof( ddsd );

      result = pStretchSurface->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL );
      AssertWarn(result == DD_OK, avar("GFXDDSurface::lock: %s", DDRAW_ERROR(result)));
      if (result == DD_OK) {
         BYTE*  pDest   = (BYTE*)ddsd.lpSurface;
         Int32 iStride = ddsd.lPitch;
         
         pHalfMemSurface->lock();
         if (iStride == pHalfMemSurface->getStride()) {
            memcpy(pDest, pHalfMemSurface->getSurfaceBitmap()->getAddress(0, 0),
                   iStride * pHalfMemSurface->getHeight());
         } else {
            BYTE* pStart = pHalfMemSurface->getSurfaceBitmap()->getAddress(0, 0);
            Int32 sStride = pHalfMemSurface->getSurfaceBitmap()->getStride();
            for (int i = 0; i < pHalfMemSurface->getHeight(); i++) {
               memcpy(pDest + (i * iStride), pStart + (i * sStride),
                      sStride);
            }
         }
         result = pStretchSurface->Unlock( NULL );
         AssertWarn(result == DD_OK, avar("GFXDDSurface::unlock: %s", DDRAW_ERROR(result)));
         pHalfMemSurface->unlock();

         pBackSurface->Blt(NULL, pStretchSurface, NULL, DDBLT_WAIT, NULL);
      }
   }


   if(pSurface->IsLost() == DDERR_SURFACELOST)
      pSurface->Restore();
}

void
GFXDDSurface::flip()
{
   AssertFatal(isPrimary == true, "GFXDDSurface::flip: Must be called from the Primary surface.");

   HRESULT result;
   if (m_lastLockMode == GFX_DOUBLELOCK) {
      flipDouble();
   } else {
      DDSURFACEDESC ddsd;
      memset( &ddsd, 0, sizeof( ddsd ) );
      ddsd.dwSize  = sizeof( ddsd );

      if(pSurface->IsLost() == DDERR_SURFACELOST)
         pSurface->Restore();
      result = pBackSurface->Lock( NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL );

      AssertWarn(result == DD_OK, avar("GFXDDSurface::lock: %s", DDRAW_ERROR(result)));
      if ( result == DD_OK ) {
         BYTE*  pDest   = (BYTE*)ddsd.lpSurface;
         Int32 iStride = ddsd.lPitch;
   
         if (iStride == pFullMemSurface->getSurfaceBitmap()->getStride()) {
            memcpy(pDest, pFullMemSurface->getSurfaceBitmap()->getAddress(0, 0), iStride * getHeight());
         } else {
            BYTE* pStart = pFullMemSurface->getSurfaceBitmap()->getAddress(0, 0);
            Int32 sStride = pFullMemSurface->getSurfaceBitmap()->getStride();
            for (int i = 0; i < getHeight(); i++) {
               memcpy(pDest + (i * iStride), pStart + (i * sStride),
                      sStride);
            }
         }

         result = pBackSurface->Unlock( NULL );
         AssertWarn(result == DD_OK, avar("GFXDDSurface::unlock: %s", DDRAW_ERROR(result)));
      }
   }

   if(pSurface->IsLost() == DDERR_SURFACELOST)
      pSurface->Restore();
   updateAlphaBlend();
   result = pSurface->Flip( NULL, DDFLIP_WAIT );
   AssertWarn(result == DD_OK, avar("GFXDDSurface::flip: %s", DDRAW_ERROR(result)));
}

void GFXDDSurface::easyFlip()
{
   flip();
   if(popCount && (flags & GFX_DMF_BACKPAGE_CARDMEM))
   {
      popCount--;
      next->lock();
      next->drawSurface(this, &RectI(0,0,surfaceWidth-1,surfaceHeight-1),&Point2I(0,0));
      next->unlock();
   } 
}


Bool GFXDDSurface::_setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, Bool /*in_rsvPal*/)
{
   AssertFatal(in_start >= 0 && in_start < 256, 
      "GFXDDSurface::setPalette: start out of range.");
   AssertFatal(in_count > 0 && in_count <= 256, 
      "GFXDDSurface::setPalette: count out of range.");
   AssertFatal( in_start+in_count <= 256, 
      "GFXDDSurface::setPalette: start+count out of range.");

   HRESULT result;

   extern void clearMultiPalette();
   clearMultiPalette();
   if ( !lpPalette ) return TRUE;

   pPalette = lpPalette;

   PALETTEENTRY color[256];
   for ( int i=0; i<256; i++ ) {
      color[i]         = applyGammaCorrection(pPalette->palette[0].color[i], gamma);
      color[i].peFlags = 0;
   }

   if(pDDPalette) {
      pDD2->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
      result = pDDPalette->SetEntries(0, 0, 256, color);
      AssertWarn(result == DD_OK, avar("GFXDDSurface::setPalette: %s", DDRAW_ERROR(result)));
   } else {
      AssertWarn(0, "Error, unable to get a palette pointer for DDraw surface?");
   }
   return TRUE;
}

void GFXDDSurface::_setGamma()
{
   _setPalette( pPalette, 0, 256 );   
}   

bool
GFXDDSurface::supportsDoubleLock() const
{
   if (pHalfMemSurface == NULL) {
      AssertWarn(0, "This should really never happen");
      return false;
   }
   if (pHalfMemSurface->getWidth() >= 320 && pHalfMemSurface->getHeight() >= 240)
      return true;
   else
      return false;
}

void GFXDDSurface::drawSurface(GFXSurface*    /*lpSourceSurface*/,
                               const RectI*   /*in_subRegion*/,
                               const Point2I* /*in_at*/)
{
//   HRESULT result;
//   GFXBitmap bmp;
//   RectI sub_r = *in_subRegion;
//   
//   AssertFatal(!lpSourceSurface->isLocked(), "GFXDDSurface::drawSurface: Source surface is locked.");
//   AssertFatal(!lpSourceSurface->isDCLocked(), "GFXDDSurface::drawSurface: Source surface is DC locked.");
//
//   if ( lpSourceSurface->getType() == SURFACE_DIRECT_DRAW )
//   {
//      sub_r.lowerR.x++, sub_r.lowerR.y++;
//      
//      result = pSurface->BltFast(in_at->x, in_at->y, ((GFXDDSurface*)lpSourceSurface)->pSurface, (RECT*)&sub_r, DDBLTFAST_NOCOLORKEY);
//      if ( result == DD_OK ) return;
//   }
//
//   //if the blit failed or the source is not another DirectDraw surface blit it in code
//   lpSourceSurface->lock();
//   lpSourceSurface->getBitmap(&bmp);
//   Int32 flags = getFlags();
//   clearFlag(GFX_DMF_ECLIP);
//   drawBitmap2d( &bmp, in_subRegion, in_at );
//   setFlags(flags);
//   lpSourceSurface->unlock();
}


//------------------------------ CUSTOM DIRECT DRAW FUNCTION TABLE CODE
extern void GFXClearScreen(GFXSurface *io_pSurface, DWORD in_color);

void GFXDDClearScreen(GFXSurface *io_pSurface, DWORD in_color)
{
   HRESULT result;
   DDBLTFX ddbltfx;

   AssertFatal( io_pSurface->getType() == SURFACE_DIRECT_DRAW, "GFXDDClearScreen: Custom clear only works on direct draw surfaces" );  

   ddbltfx.dwSize = sizeof(DDBLTFX);
   ddbltfx.dwFillColor = in_color;
   io_pSurface->unlock();
   result = ((GFXDDSurface*)io_pSurface)->pSurface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );
   io_pSurface->lock();
   if ( result != DD_OK )
      GFXClearScreen(io_pSurface, in_color);
}


//------------------------------ 
extern void GFXDrawRect2d_f(GFXSurface *io_pSurface, const RectI *in_rect, float in_w, DWORD in_color );
void GFXDDDrawRect2d_f(GFXSurface *io_pSurface,  const RectI *in_rect, float in_w, DWORD in_color )
{
   HRESULT result;
   DDBLTFX ddbltfx;
   RectI  src = *in_rect;

   AssertFatal( io_pSurface->getType() == SURFACE_DIRECT_DRAW, "GFXDDDrawRect3d_f: Custom clear only works on direct draw surfaces" ); 

   // clip rectangle to destination clip region
   if ( io_pSurface->getFlag(GFX_DMF_RCLIP) )
      if ( !rectClip( &src, io_pSurface->getClipRect()) )
         return;

   io_pSurface->setLastRect(&src);                   

   result = ((GFXDDSurface*)io_pSurface)->pSurface->Blt((RECT*)&src, NULL, NULL, DDBLT_COLORFILL, &ddbltfx );
   if ( result != DD_OK )
      GFXDrawRect2d_f(io_pSurface, &src, in_w, in_color);
}
