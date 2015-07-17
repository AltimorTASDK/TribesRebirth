//================================================================
//   
// $Workfile:   g_cds.h  $
// $Version$
// $Revision:   1.5  $
//   
// DESCRIPTION:
//   
//================================================================

#ifndef _G_CDS_H_
#define _G_CDS_H_


#include "g_surfac.h"
#include "d_funcs.h"
#include "g_pal.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#ifndef STRICT
#error STRICT must be defined!!!!!
#endif

//CreateDIBSection surface
class GFXCDSSurface: public GFXSurface
{
protected:
	HBITMAP	   hBitmap;
	BYTE		   *pCDSBits;
	//NOTE: the following items are only valid on the primary surface
	HDC			hWndDC;			//orginal window DC (Assumed to be CS_OWNDC)
	HDC			hCDSDC;			//compatible DC for CreateDIBSection blits
	HBITMAP		hOldBitmap;
	HPALETTE	   hOldPalette;
	HWND		   hWnd;
 	Bool		   paletteSelected;
	Point2I		m_offset;
   ColorF      alphaColor;
   float       alphaBlend;

   ColorF      lastAlphaColor;
   float       lastAlphaBlend;

   Bool _setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, Bool in_rsvPal = true);
   void _setGamma();
	void _lock(const GFXLockMode in_lockMode);
	void _unlock();

public:

	GFXCDSSurface();
	~GFXCDSSurface();

	static Bool create(GFXSurface *&pSurface, Bool lPrimary, Int32 lWidth, Int32 lHeight, HWND hWnd, Point2I *off, Int32 bpp=8);
   
	void setHWND( HWND hWnd );
   void setSurfaceAlphaBlend(ColorF *color, float alpha);

 	void flip();
	void update(const RectI *rect = NULL);
   void easyFlip();
	void drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at);
};

inline void GFXCDSSurface::easyFlip()
{
   update();
}

struct LogPal
{
    WORD         palVersion; 
    WORD         palNumEntries; 
    PALETTEENTRY color[256]; 
};

class PaletteManager
{
	public:
	HPALETTE hPalette;
	HPALETTE hOld;

	PaletteManager();
	void destruct(HDC dc);
	HPALETTE select(HDC dc, GFXPalette *pPalette);
   void animate16(HDC dc, LogPal *pal);
	void animate(Int32 in_start, Int32 in_count, GFXPalette *pPalette);
	void animate(Int32 in_start, Int32 in_count, PALETTEENTRY colors[256]);
	HPALETTE getPalette() { return hPalette; }
};

extern PaletteManager palManager;

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_G_CDS_H_
