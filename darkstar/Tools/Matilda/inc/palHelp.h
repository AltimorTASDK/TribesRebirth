// Palhelp.h  Helper function prototypes, etc...
//

#ifndef _PALHELP_H_
#define _PALHELP_H_

#include "stdafx.h"

extern CPalette *gPalette;
extern CPalette  gMapPalette;

BOOL RealizeSysPalette(CClientDC &dc);
void initMapPalette();

#endif
