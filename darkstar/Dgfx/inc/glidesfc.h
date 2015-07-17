//----------------------------------------------------------------------------
//
//  $Workfile:   glidesfc.h  $
//  $Version$
//  $Revision:     $
//
//  DESCRIPTION:  3DFX card gfx interface surface and function table.
//
//  These functions are prototypes for the glue functions for all of the 
//  primitives that go into the function table in FN_TABLE.H
//  
//----------------------------------------------------------------------------

#ifndef _GLIDESFC_H_
#define _GLIDESFC_H_

//Includes

#include "g_surfac.h"
#include "gdevice.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//----------------------------------------------------------------------------
//             Forward references to classes and structures
//----------------------------------------------------------------------------

namespace Glide {

class TextureCache;
class HandleCache;

class Surface : public GFXSurface
{
   int tmuCount;
   TextureCache *texCache;
   HandleCache *hndCache;

   bool sstWinOpen;

   ColorF alphaColor;
   float alphaBlend;

	Bool _setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, bool);
   void _setGamma();
	void _lock(const GFXLockMode in_lockMode);
	void _unlock() {}
	public:

   void setSurfaceAlphaBlend(ColorF *color, float alpha);
   DWORD getCaps();
   static bool init(Vector<ResSpecifier>* resVector, bool &windowed);
	void drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at) {};
	void drawSurfaceToBitmap(GFXBitmap *bmp);
 	void flip();						   
	Surface();
	~Surface();
	static GFXSurface * create(HWND hwnd, Int32 in_width, Int32 in_height, bool fullscreen);

  public:
   bool isVoodoo2;
};

} // namespace Glide


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GLIDESFC_H_





