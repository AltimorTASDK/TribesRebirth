//================================================================
//
// Initial Author: Rick Overman
//
// Description
//
// $Workfile:   g_cds.cpp  $
// $Revision:   1.11  $
// $Author  $
// $Modtime $
//
//================================================================

#include "g_cds.h"
#include "g_bitmap.h"
#include "r_clip.h"
#include "fn_table.h"

namespace {

RGBQUAD
applyGammaCorrection(const RGBQUAD& in_pRGB, const float in_gamma)
{
   RGBQUAD newQuad;

   float r = (float(in_pRGB.rgbRed)   / 255.0f);
   float g = (float(in_pRGB.rgbGreen) / 255.0f);
   float b = (float(in_pRGB.rgbBlue)  / 255.0f);

   newQuad.rgbRed   = UInt8((pow(r, 1.0f / in_gamma) * 255.0f) + 0.5f);
   newQuad.rgbGreen = UInt8((pow(g, 1.0f / in_gamma) * 255.0f) + 0.5f);
   newQuad.rgbBlue  = UInt8((pow(b, 1.0f / in_gamma) * 255.0f) + 0.5f);

   return newQuad;
}

} // namespace {}


static BOOL is_win32s( void );

typedef BOOL (WINAPI *WINGRECOMMENDDIBFORMAT)( BITMAPINFO FAR *pFormat );
typedef HDC  (WINAPI *WINGCREATEDC)( void );
typedef HBITMAP (WINAPI *WINGCREATEBITMAP)( HDC WinGDC, BITMAPINFO const FAR *pHeader,
		  void FAR *FAR *ppBits );
typedef UINT (WINAPI *WINGSETDIBCOLORTABLE)( HDC WinGDC, UINT StartIndex,
		  UINT NumberOfEntries, RGBQUAD const FAR *pColors );
typedef UINT (WINAPI *WINGGETDIBCOLORTABLE)( HDC WinGDC, UINT StartIndex,
		  UINT NumberOfEntries, RGBQUAD FAR *pColors );
typedef BOOL (WINAPI *WINGBITBLT)( HDC hdcDest, int nXOriginDest,
		  int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc,
		  int nXOriginSrc, int nYOriginSrc );

typedef BOOL (WINAPI *WINGSTRETCHBLT)( HDC hdcDest, int nXOriginDest,
		  int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc,
		  int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc );

static BOOL fixedUp = FALSE;
static WINGCREATEDC            pfnWinGCreateDC=NULL;
static WINGCREATEBITMAP        pfnWinGCreateBitmap = NULL;
static WINGRECOMMENDDIBFORMAT  pfnWinGRecommendDIBFormat=NULL;
static WINGSETDIBCOLORTABLE    pfnWinGSetDIBColorTable=NULL;
static WINGGETDIBCOLORTABLE    pfnWinGGetDIBColorTable=NULL;
static WINGBITBLT              pfnWinGBitBlt=NULL;
static WINGSTRETCHBLT          pfnWinGStretchBlt=NULL;

static BOOL LoadWinG (void);
static void insteadWinGuseDIBS( void );

PaletteManager::PaletteManager()
{
	hPalette = NULL;
	hOld     = NULL;
}


void PaletteManager::destruct(HDC dc)
{
	if(hPalette) 
	{
		if ( hOld )
			SelectPalette(dc, hOld, false);
		DeleteObject(hPalette);
	}
	hOld = NULL;
	hPalette = NULL;
}


PaletteManager palManager;
HPALETTE PaletteManager::select(HDC dc, GFXPalette *pPalette)
{
	HPALETTE htemp;
	if(!hPalette && pPalette)
	{
		hPalette = CreatePalette( (LOGPALETTE *) &pPalette->version);
	}
	if(hPalette)
	{
		htemp = SelectPalette(dc, hPalette, FALSE);
		RealizePalette(dc);
	}
	return htemp;

//	if ( pPalette )
//	{
//		HPALETTE htemp;
// 		hPalette = CreatePalette( (LOGPALETTE *) &pPalette->version);
//		htemp		= SelectPalette(dc, hPalette, FALSE);
//		if ( !hOld )					//keep the first one we extract since it
//			hOld = htemp;				//belongs to the DC and needs to be
//		else								//selected back in before the window is destroyed
//			DeleteObject( htemp );
//		RealizePalette(dc);
//	}
//	else
//		if(hPalette)
//		{
//			SelectPalette(dc, hPalette, FALSE);
//			RealizePalette(dc);
//		}
}

void PaletteManager::animate16(HDC dc, LogPal *pal)
{
   HPALETTE htemp, old;
   
   htemp = CreatePalette( (LOGPALETTE *) pal);
   old = SelectPalette(dc, htemp, FALSE);
   RealizePalette(dc);
   if(old == hPalette)
      DeleteObject(old);
   hPalette = htemp;
}

void PaletteManager::animate(Int32 in_start, Int32 in_count, PALETTEENTRY colors[256])
{
	AssertFatal(hPalette, "NULL palette in animate");


	AnimatePalette(hPalette, in_start, in_count, &colors[in_start]);
}

void PaletteManager::animate(Int32 in_start, Int32 in_count, GFXPalette *pPalette)
{
	AssertFatal(hPalette, "NULL palette in animate");
	AnimatePalette(hPalette, in_start, in_count, &pPalette->palette[0].color[in_start]);
}

GFXCDSSurface::GFXCDSSurface()
{
	hWnd			= NULL;
	hWndDC		= NULL;
	hCDSDC		= NULL;
	hBitmap  	= NULL;
	hOldBitmap  = NULL;
	hOldPalette = NULL;
	pCDSBits 	= NULL;
	paletteSelected = FALSE;
	surfaceType = SURFACE_DIB_SECTION;
	functionTable = &rclip_table;
   alphaBlend = 0;
   lastAlphaBlend = 0;
}


GFXCDSSurface::~GFXCDSSurface()
{
	if ( isPrimary )
	{
		//select old stuff back in
		if (hBitmap)   SelectBitmap( hCDSDC, hOldBitmap );
		if (hOldPalette)
			SelectPalette(hWndDC, hOldPalette, FALSE);
		//palManager.destruct( hWndDC );
		DeleteDC( hCDSDC );
      ReleaseDC( hWnd, hWndDC);
	}
	if ( hBitmap ) DeleteObject( hBitmap );
}


void GFXCDSSurface::setHWND( HWND lhWnd )
{
	AssertFatal(isPrimary, "GFXCDSSurface::setHWND: only valid on primary surfaces.");

	if ( hCDSDC )
	{
		if (hBitmap) SelectBitmap( hCDSDC, hOldBitmap );
		DeleteDC( hCDSDC );
		hCDSDC = 0;
	}

   if ( hWndDC)
   {
      ReleaseDC(hWnd, hWndDC);
      hWndDC = NULL;
   }

	if ( lhWnd )
	{
		hWndDC = GetDC( lhWnd );		//assumes Window Class contains CS_OWNDC
		hWnd   = lhWnd;
      if(GetDeviceCaps(hWndDC, RASTERCAPS) & RC_PALETTE)
		   palManager.select(hWndDC, pPalette);
		hCDSDC = pfnWinGCreateDC( ); //hWndDC );
		if ( hBitmap )  hOldBitmap =  SelectBitmap( hCDSDC, hBitmap );
	}
}


Bool GFXCDSSurface::create(GFXSurface *&pSurface, Bool lPrimary, Int32 lWidth, Int32 lHeight, HWND hWnd, Point2I *offset, Int32 bitDepth)
{
	AssertFatal( lWidth > 0 && lHeight > 0,
      "GFXCDSSurface::create: Invalid width or height.");
	AssertFatal( hWnd,  "GFXCDSSurface::create: Invalid hWnd HANDLE.");

	GFXCDSSurface *lpSurface;
	BYTE data[sizeof(BITMAPINFO) + (sizeof(RGBQUAD)*256)];
	BITMAPINFO &bi = *(BITMAPINFO*)data;
	HDC hDC;

   if(!fixedUp)
   {
      if(is_win32s())
      {
         bool fWinGLoaded = LoadWinG() != 0;
         if(!fWinGLoaded)
 			{
 			   MessageBox( GetFocus(), "You need to install WinG graphics Libraries\n"
						" when running this application on Win32s",
						"Windows 3.1 initialization error",
						MB_ICONSTOP );
			   return FALSE;
         }
         fixedUp = TRUE;
      }
      else
      {
         insteadWinGuseDIBS();
         fixedUp = TRUE;
      }
   }
	//rather than GOTO's use while w/ breaks.
	while ( TRUE )
	{
		lpSurface = new GFXCDSSurface;
		if ( !lpSurface ) break;

		if(offset)
			lpSurface->m_offset = *offset;
		else
			lpSurface->m_offset = Point2I(0,0);
		hDC = GetDC( hWnd );
		if ( !hDC ) break;

		// fill in the bitmap info header info
		bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth    = lWidth;
		bi.bmiHeader.biHeight   = -lHeight;
		bi.bmiHeader.biPlanes   = 1;
		bi.bmiHeader.biBitCount = (unsigned short) bitDepth;

      if(bitDepth == 8)
      {
   		bi.bmiHeader.biCompression  = BI_RGB;
   		PALETTEENTRY blank = {0, 0, 0, 0};
         PALETTEENTRY *color = (PALETTEENTRY *) bi.bmiColors;

         int i;
        for (i = 0; i < 256; i++) color[i] = blank;
   		GetSystemPaletteEntries( hDC, 0, 256, color );
        for (i = 0; i < 256; i++)
        {
            UInt8 temp      = color[i].peRed;
            color[i].peRed  = color[i].peBlue;
            color[i].peBlue = temp;
         }
   	}
		bi.bmiHeader.biSizeImage    = 0;
		bi.bmiHeader.biXPelsPerMeter= 0;
		bi.bmiHeader.biYPelsPerMeter= 0;
		bi.bmiHeader.biClrUsed      = 0;
		bi.bmiHeader.biClrImportant = 0;

		//get the current system colors

		//create the DIB-Section
		lpSurface->hBitmap = pfnWinGCreateBitmap(
   	 	hDC, &bi, (VOID **) &lpSurface->pCDSBits);

		if ( !lpSurface->hBitmap ) break;
      ReleaseDC(hWnd, hDC);

		lpSurface->isPrimary    = lPrimary;
		lpSurface->surfaceWidth = lWidth;
		lpSurface->surfaceHeight= lHeight;
      lpSurface->surfaceBitDepth = bitDepth;

		if ( lPrimary ) lpSurface->setHWND( hWnd );
		lpSurface->setClipRect( &RectI(0, 0, lWidth - 1, lHeight - 1) );

		//return the surfce pointer (don't loop)
		pSurface = lpSurface;
		return ( TRUE );
	}

	//ERROR: If you get here something above failed
	AssertWarn(0, "GFXCDSSurface::create: Failed to create a surface.");
	if ( lpSurface ) delete lpSurface;
	pSurface = NULL;
	return ( FALSE );
}

void GFXCDSSurface::_lock(const GFXLockMode in_lockMode)
{
   AssertFatal(in_lockMode == GFX_NORMALLOCK,
               "Double locking not supported on CDS Surface");

	pSurfaceBits = pCDSBits;
	surfaceStride= (surfaceWidth*(surfaceBitDepth>>3)+3)&(~3);
}

void GFXCDSSurface::_unlock()
{
	pSurfaceBits = NULL;
	surfaceStride= 0;
}

void	GFXCDSSurface::flip()
{
	AssertFatal(0, "GFXCDSSurface::flip: not a valid operation for this type of surface.");
}

void GFXCDSSurface::update(const RectI *copyBounds)
{
	AssertFatal(isPrimary, "GFXCDSSurface::update: only valid on primary surfaces.");

   if(alphaBlend != 0 || lastAlphaBlend != 0)
   {
      if(alphaBlend != lastAlphaBlend || alphaColor.red != lastAlphaColor.red
         || alphaColor.green != lastAlphaColor.green || alphaColor.blue != lastAlphaColor.blue)
      {
         lastAlphaBlend = alphaBlend;
         lastAlphaColor = alphaColor;

         int i;
         float ared = alphaColor.red * alphaBlend * 255.0;
         float agreen = alphaColor.green * alphaBlend * 255.0;
         float ablue = alphaColor.blue * alphaBlend * 255.0;

         float mulfactor = 1 - alphaBlend;

         RGBQUAD rgb[256];
         for(i = 0; i < 256; i++)
         {
            rgb[i].rgbRed = min(int(ared + pPalette->palette[0].color[i].peRed * mulfactor), 255);
            rgb[i].rgbGreen = min(int(agreen + pPalette->palette[0].color[i].peGreen * mulfactor), 255);
            rgb[i].rgbBlue = min(int(ablue + pPalette->palette[0].color[i].peBlue * mulfactor), 255);

            rgb[i] = applyGammaCorrection(rgb[i], gamma);
         }
			SelectBitmap(hCDSDC, hBitmap );
   		pfnWinGSetDIBColorTable(hCDSDC, 0, 256, rgb);

         if(GetDeviceCaps(hWndDC, RASTERCAPS) & RC_PALETTE)
         {
            LogPal pal;
            pal.palVersion = 0x300;
            pal.palNumEntries = 256;
            for(i = 0; i < 256; i++)
            {
               pal.color[i].peFlags = PC_RESERVED;
               pal.color[i].peRed = min(int(ared + pPalette->palette[0].color[i].peRed * mulfactor), 255);
               pal.color[i].peGreen = min(int(agreen + pPalette->palette[0].color[i].peGreen * mulfactor), 255);
               pal.color[i].peBlue = min(int(ablue + pPalette->palette[0].color[i].peBlue * mulfactor), 255);
            }
            palManager.animate(1, 255, pal.color);
         }
      }
   }

	RectI bounds(0, 0, surfaceWidth - 1, surfaceHeight - 1);
   if(!copyBounds)
      copyBounds = &bounds;

   pfnWinGBitBlt(hWndDC, copyBounds->upperL.x + m_offset.x, copyBounds->upperL.y + m_offset.y,
         copyBounds->lowerR.x - copyBounds->upperL.x + 1,
         copyBounds->lowerR.y - copyBounds->upperL.y + 1,
         hCDSDC,copyBounds->upperL.x, copyBounds->upperL.y);

}

//================================================================
// void GFXCDSSurface::_setPalette(GFXPalette *in_pPal, Int32 in_start, Int32 in_count, BOOL )
//
//================================================================
// Description:
//		Sets the surfaces palette
//
//		If (in_pPal == NULL) update the DC with the current LOCICAL PALETTE
//		If (in_start == 0 and in_count == 256) install in_pPal pointer into the surface
//		Otherwise use in_pPal to update the existing pPalette entries
//
// Returns:
//		none
//
// USAGE:
//
//		case WM_PALETTECHANGED:
//	   	if ((HWND)wParam == hWnd)       			// If responding to own message,
//	      	break;						  				// Nothing to do.
//		 	//allow to fall through
//		case WM_QUERYNEWPALETTE:
//		{
//			BOOL changed;
//			HDC hDC = GetDC(hWnd);						// Assumes class CS_OWNDC
//		 	if (pSurface) pSurface->setPalette(NULL,0,0,0);	//re-Select the HPALETTE
//			changed = RealizePalette(hDC);			// realize
//	    	if (changed)              					// Did the realization change?
//	    		InvalidateRect(hWnd, NULL, TRUE); 	// Yes, so force a repaint.
//	    	return(changed);
//		}
//    case WM_PAINT:
//    	BeginPaint( hWnd, &ps );
//			if (pSurface) pSurface->update();		//update or flip whatever, just redraw
//      	return 1;
//
//================================================================
// Programmed By Rick Overman
// Modified:  Dave Moore 6/6/96 - Added flag to mark palette for animation
//                                 or to leave unmarked to allow multiple
//                                 windows...
//================================================================
Bool GFXCDSSurface::_setPalette(GFXPalette *in_pPal, Int32 in_start, Int32 in_count, Bool in_rsvPal)
{
   AssertFatal(in_start >= 0 && in_start < 256,
      "GFXCDSSurface::setPalette: start out of range.");
   AssertFatal(in_count > 0 && in_count <= 256,
      "GFXCDSSurface::setPalette: count out of range.");
   AssertFatal( in_start+in_count <= 256,
      "GFXCDSSurface::setPalette: start+count out of range.");

	GFXCDSSurface *cds;
	GFXCDSSurface *primary;
	int i;

	//special case for Windowed apps
	//if in_pPal is NULL re-Select the hPalette into the HDC
	//this is needed to be a 'behaved' palletized app.
   extern void clearMultiPalette();
   clearMultiPalette();
	if ( !in_pPal )
	{
      if(GetDeviceCaps(hWndDC, RASTERCAPS) & RC_PALETTE)
		   palManager.select(hWndDC, NULL);
	 	return TRUE;
	}

	if ( in_start == 0 && in_count == 256 )
		pPalette = in_pPal;
	else
	{
		AssertFatal(pPalette && surfaceBitDepth == 8, "GFXCDSSurface::setPalette: you must set a FULL palette before you can change a range of it.");
		if(pPalette != in_pPal)
		   memcpy( &pPalette->palette[0].color[in_start], &in_pPal->palette[0].color[in_start], sizeof(PALETTEENTRY)*in_count);
	}

	if ( isPrimary || pages>1 )
	{
		RGBQUAD rgb[256];
		for ( i=0, cds = this; i < pages; i++, cds = (GFXCDSSurface*)cds->next)
		{
			cds->pPalette = pPalette;
			if ( cds->isPrimary ) primary = cds;
		}

      if(surfaceBitDepth != 8)
         return TRUE;

      //System colors must have peFlags = 0
      //Any colors that match the upper 10 colors peFlags = PC_RESERVED
      //All other entries peFlags = PC_NOCOLLAPSE
		//  step 2:
      for (i=0; i<256; i++)
      {
         /*BYTE flag = PC_NOCOLLAPSE;
         for (Int32 pi=246; pi<256; pi++)
         {
            if (pPalette->color[i].peRed == pPalette->color[pi].peRed
                  && pPalette->color[i].peGreen == pPalette->color[pi].peGreen
                  && pPalette->color[i].peBlue == pPalette->color[pi].peBlue)
               flag = PC_RESERVED;
         }
         pPalette->color[i].peFlags = flag;
         */
         if (in_rsvPal == true) {
            pPalette->palette[0].color[i].peFlags = PC_RESERVED; // for palette animation...
         } else {
            pPalette->palette[0].color[i].peFlags = PC_NOCOLLAPSE;
         }
      }

		//set the DIB color table to match the newly created identity palette
		//  step 3:
		//  we have to set the DIB color table in each hBitmap BUT the only way
		//  to do that is to select the bitmap into a DC and set the color table
		//  for the hBitmap in a DC... so we loop through... (what were they thinking?)
      PALETTEENTRY *pe = pPalette->palette[0].color;
      RGBQUAD *out_rgb = rgb;

      for(i=0; i<256; i++, pe++, out_rgb++)
      {
         // copy from the out_rgb_quad color table to the new palette
         // palette_entry table

         out_rgb->rgbRed    = pe->peRed;
         out_rgb->rgbGreen  = pe->peGreen;
         out_rgb->rgbBlue   = pe->peBlue;
         out_rgb->rgbReserved = 0;
      }
      
      // apply gamma correction to RGB
      RGBQUAD *pRGB=rgb;
      for ( i=0; i<256; i++, pRGB++ )
         *pRGB = applyGammaCorrection(*pRGB, gamma);

		cds = (GFXCDSSurface*)cds->next;
		for ( i=0; i < pages; i++, cds = (GFXCDSSurface*)cds->next)
		{
			SelectBitmap(primary->hCDSDC, cds->hBitmap );
			pfnWinGSetDIBColorTable(primary->hCDSDC, in_start, in_count, &rgb[in_start]);
		}
   	bool ret;
   	HDC hDC;
   	hDC = GetDC(NULL);
   	ret = (GetDeviceCaps(hDC,RASTERCAPS) & RC_PALETTE) != 0;
   	ReleaseDC(NULL,hDC);

      if(ret)
      {
   		if(!paletteSelected)
   		{
   			paletteSelected = true;
   			hOldPalette = palManager.select(primary->hWndDC, pPalette);
   		}
   		else
   			palManager.animate(in_start, in_count, pPalette);
      }
   }
	// Check if the device actually supports palettized stuff.
	// if not, a blit will be required to change the colors of this
	// surface.

	return true;
}

void GFXCDSSurface::setSurfaceAlphaBlend(ColorF *color, float alpha)
{
   alphaColor = *color;
   alphaBlend = alpha;
}

void GFXCDSSurface::_setGamma()
{
   _setPalette( pPalette, 0, 256 );   
}   

void GFXCDSSurface::drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at)
{
	GFXBitmap bmp;

	AssertFatal(!lpSourceSurface->isLocked(), "GFXCDSSurface::drawSurface: Source surface is locked.");

	lpSourceSurface->lock();
	lpSourceSurface->getBitmap(&bmp);
	Int32 flags = getFlags();
   clearFlag(GFX_DMF_ECLIP);
	drawBitmap2d( &bmp, in_subRegion, in_at );
	setFlags(flags);
	lpSourceSurface->unlock();
}

//############################################################################
//############################################################################
//
// NAME
//   dibWINGRECOMMENDDIBFORMAT
//   dibWINGCREATEDC
//   dibWINGCREATEBITMAP
//   dibWINGSETDIBCOLORTABLE
//   dibWINGGETDIBCOLORTABLE
//   dibWINGBITBLT
//   dibWINGSTRETCHBLT
//   LoadWinG
//   insteadWinGuseDIBS
//   is_winnt
//   is_win32s
//   is_win95
//
// DESCRIPTION
//   The following functions are a collection of functions that are
//   helper functions to support WinG type graphics under Win95
//   and Win32s.  The functions that start with dib are Win95 functions
//   that replace the WinG functions.  The other functions are to help
//   handle the detection of the Windows version and the loading of
//   WinG under Win32s.
//
// NOTES
//   In your code, wherever you would use a WinG function you should
//   replace that with a function reference.  Ex.
//
//       (*pfnWinGRecommendDIBFormat)((BITMAPINFO*)&bi)
//
//############################################################################
//############################################################################

static BOOL WINAPI dibWINGRECOMMENDDIBFORMAT( BITMAPINFO FAR *pFormat )
{
	pFormat->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pFormat->bmiHeader.biPlanes = 1;
	pFormat->bmiHeader.biCompression = BI_RGB;
	pFormat->bmiHeader.biSizeImage = 0;
	pFormat->bmiHeader.biClrUsed = 0;
	pFormat->bmiHeader.biHeight = -1;
	return TRUE;
}

static HDC WINAPI dibWINGCREATEDC( void )
{
	return CreateCompatibleDC(NULL);
}

static HBITMAP WINAPI dibWINGCREATEBITMAP( HDC hDC, BITMAPINFO const FAR *pHeader,
		  void FAR *FAR *ppBits )
{
	return CreateDIBSection( hDC, pHeader,
		DIB_RGB_COLORS, (void FAR * FAR *) ppBits, NULL, 0 );
}

static UINT WINAPI dibWINGSETDIBCOLORTABLE( HDC WinGDC, UINT StartIndex,
		  UINT NumberOfEntries, RGBQUAD const FAR *pColors )
{
	return SetDIBColorTable(WinGDC, StartIndex, NumberOfEntries, pColors );
}

static UINT WINAPI dibWINGGETDIBCOLORTABLE( HDC WinGDC, UINT StartIndex,
		  UINT NumberOfEntries, RGBQUAD FAR *pColors )
{
	return GetDIBColorTable(WinGDC, StartIndex, NumberOfEntries, pColors );
}

static BOOL WINAPI dibWINGBITBLT( HDC hdcDest, int nXOriginDest,
		  int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc,
		  int nXOriginSrc, int nYOriginSrc )
{
	return (BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest,
                  nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY) != 0);
}

static BOOL WINAPI dibWINGSTRETCHBLT( HDC hdcDest, int nXOriginDest,
		  int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc,
		  int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc )
{
	return StretchBlt( hdcDest, nXOriginDest,
		nYOriginDest, nWidthDest, nHeightDest, hdcSrc,
		nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY );
}

BOOL LoadWinG (void)
{
	HMODULE hWinG;
	BOOL bResult = FALSE;
	hWinG = LoadLibrary ("WING32.DLL");
	if (hWinG) {
		bResult = TRUE;
		bResult = bResult && (BOOL)(pfnWinGBitBlt = (WINGBITBLT)GetProcAddress(hWinG, "WinGBitBlt"));
		bResult = bResult && (BOOL)(pfnWinGCreateBitmap = (WINGCREATEBITMAP)GetProcAddress(hWinG, "WinGCreateBitmap"));
		bResult = bResult && (BOOL)(pfnWinGRecommendDIBFormat = (WINGRECOMMENDDIBFORMAT)GetProcAddress(hWinG, "WinGRecommendDIBFormat"));
		bResult = bResult && (BOOL)(pfnWinGCreateDC = (WINGCREATEDC)GetProcAddress(hWinG, "WinGCreateDC"));
		bResult = bResult && (BOOL)(pfnWinGSetDIBColorTable = (WINGSETDIBCOLORTABLE)GetProcAddress(hWinG, "WinGSetDIBColorTable"));
		bResult = bResult && (BOOL)(pfnWinGGetDIBColorTable = (WINGGETDIBCOLORTABLE)GetProcAddress(hWinG, "WinGGetDIBColorTable"));
		bResult = bResult && (BOOL)(pfnWinGStretchBlt = (WINGSTRETCHBLT)GetProcAddress(hWinG, "WinGStretchBlt"));
	}
 //	else
	return bResult;
}

void insteadWinGuseDIBS()
{
	pfnWinGBitBlt = dibWINGBITBLT;
	pfnWinGCreateBitmap = dibWINGCREATEBITMAP;
	pfnWinGRecommendDIBFormat = dibWINGRECOMMENDDIBFORMAT;
	pfnWinGCreateDC = dibWINGCREATEDC;
	pfnWinGSetDIBColorTable = dibWINGSETDIBCOLORTABLE;
	pfnWinGGetDIBColorTable = dibWINGGETDIBCOLORTABLE;
	pfnWinGStretchBlt = dibWINGSTRETCHBLT;
}

BOOL is_win32s()
{
   Int32 vers = GetVersion();

   return ((vers & 0xFF) < 4) && (vers & 0x80000000);
}

