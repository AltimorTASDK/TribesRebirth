// File: matsuprt.h
// Program: Matilda -- a material list editor
// Authors: John Leavens & Steve Nichols
// History:
//	11/18/95 -- code is checkpointed at external version 1.00;
//		internal SourceSafe version is 2.05
#ifndef _MATSUPRT_H_
#define _MATSUPRT_H_
// contains global support code for matilda

// system wide defines
const int	TMP_BSIZE =	128;
const int	RGB_MAX = 255;
const int	HLS_MAX = 240;


// global functions	
extern void glWarnFileNotFound( HWND hWnd, const char* pcFileName );
extern void glCouldntLoadBitmap( HWND hWnd, const char* pcFileName );
extern void glInitMapPalette();
extern BOOL glRealizeSysPalette( CClientDC &dc );

// global variables
extern CPalette* glPalette;
extern CPalette glMapPalette;


#endif

