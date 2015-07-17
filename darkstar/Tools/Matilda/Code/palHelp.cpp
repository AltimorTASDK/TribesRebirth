// Some palette helper functions...
//

#include "stdafx.h"
#include "palHelp.h"

#include <types.h>


CPalette   *gPalette	= NULL;
CPalette	gMapPalette;


// =====
// Helper function to swap palettes...
// ==
BOOL
RealizeSysPalette(CClientDC &dc)
{
	// if user loaded palette not loaded no realize
	if( gPalette == NULL )
		return FALSE;

	CPalette* pOldPal = dc.SelectPalette( gPalette, 0);
	if( pOldPal != NULL ) {
		int rCnt = dc.RealizePalette();

		dc.SelectPalette(pOldPal, 1);

		if( rCnt != 0 )
			afxDump << "QUERYNEWPALETTE  - The number of colors realized " << rCnt << "\n";
		else
			return FALSE;
	}
	else
		return FALSE;

	// we realized our palette so cause a redraw
	if( dc.GetWindow() != NULL )
		dc.GetWindow()->Invalidate(TRUE);

	// indicates that we realized our palette
	return TRUE;
}


void initMapPalette()
{
	LOGPALETTE* pPal = (LOGPALETTE*) malloc(sizeof(LOGPALETTE) 
	                         + 256 * sizeof(PALETTEENTRY));
	pPal->palVersion = 0x300;  // Windows 3.0
	pPal->palNumEntries = 256; // table size
	for (int i=0; i<256; i++)  {
		pPal->palPalEntry[i].peRed = (UInt8)i;
		pPal->palPalEntry[i].peGreen = 0;
		pPal->palPalEntry[i].peBlue = 0;
		pPal->palPalEntry[i].peFlags = PC_EXPLICIT;
	}
	gMapPalette.CreatePalette(pPal);
	free (pPal);
}

