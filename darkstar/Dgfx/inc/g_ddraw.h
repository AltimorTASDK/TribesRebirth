//================================================================
//	
// Initial Author: Rick Overman 
//	
// Description 
//	
// $Workfile:   g_ddraw.h  $
// $Revision:   1.6  $
// $Author  $
// $Modtime $
//
//================================================================

#ifndef _G_DDRAW_H_
#define _G_DDRAW_H_

#include "g_surfac.h"
#include "g_mem.h"
#include "d_funcs.h"
#include <ddraw.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

struct RGBFormat;

//Direct Draw surface
class GFXDDSurface: public GFXSurface
{
protected:
	static IDirectDraw	*pDD;
	static IDirectDraw2	*pDD2;
   
   static DDCAPS          driverCaps, helCaps;
   static bool            writeOnly;
   static bool            sm_stretchBlt;
   ColorF lastAlphaColor, alphaColor;
   float  lastAlphaBlend, alphaBlend;

   static HWND DDSurfaceHWnd;

   bool   m_coopSet;
   bool   m_vidModeSet;
   bool   m_doubleCapable;

	IDirectDrawSurface* pSurface;
	IDirectDrawSurface* pBackSurface;
   IDirectDrawPalette* pDDPalette;

	IDirectDrawSurface* pStretchSurface;

   GFXMemSurface* pFullMemSurface;
   GFXMemSurface* pHalfMemSurface;

   GFXLockMode    m_lastLockMode;
   GFXMemSurface* m_pLockSurface;

	friend long FAR PASCAL enumCallback(LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC, LPVOID lpContext);
	friend void GFXDDClearScreen(GFXSurface *io_pSurface, DWORD in_color);
	friend void GFXDDDrawRect2d_f(GFXSurface *io_pSurface,  const RectI *in_rect, float w, DWORD in_color );
   Bool _setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, Bool in_rsvPal = true);
   void _setGamma();
	void _lock(const GFXLockMode in_lockMode);
	void _unlock();

public:
   bool supportsDoubleLock() const;
   static bool init(HWND hWnd);
   static void free(void);
   static BOOL isWriteOnly() { return writeOnly; }
   static const DDCAPS* GetDriverCaps() { return &driverCaps; }
   static const DDCAPS* GetHELCaps()    { return &helCaps;    }
	GFXDDSurface();
	~GFXDDSurface();

	static bool create(GFXSurface*& io_rpSurface, UInt32 width, UInt32 height);

 	void flipDouble();
 	void flip();
   void easyFlip();
   void update(const RectI *rect=NULL);
	void drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at);
   static IDirectDraw*  GetDD()  { return pDD; }
   static IDirectDraw2* GetDD2() { return pDD2; }
   IDirectDrawSurface * GetDDSurface() { return pSurface; }
   void restore();
//   void makeGDIBackPage();
   void setSurfaceAlphaBlend(ColorF *color, float alpha);
   void updateAlphaBlend();
};

inline void GFXDDSurface::restore()
{
   if(pSurface->IsLost() != DD_OK)
      pSurface->Restore();
}

//------------------------------ CUSTOM DIRECT DRAW FUNCTION TABLE CODE
void GFXDDClearScreen(GFXSurface *io_pSurface, DWORD in_color);
void GFXDDDrawRect2d_f(GFXSurface *io_pSurface,  const RectI *in_rect, float w, DWORD in_color );

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_G_DDRAW_H_
