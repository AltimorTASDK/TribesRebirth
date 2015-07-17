//================================================================
//   
// $Workfile:   g_mem.h  $
// $Version$
// $Revision:   1.4  $
//   
// DESCRIPTION:
//   
//================================================================

#ifndef _G_MEM_H_
#define _G_MEM_H_


#include "g_surfac.h"
#include "d_funcs.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GFXMemSurface : public GFXSurface
{
protected:
   GFXBitmap *pBitmap;
   Bool bmpIsMine;   // if the memory is ours, we deallocate on destruction

   Bool _setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, Bool in_rsvPal = true);
   void _setGamma();
	void _lock(const GFXLockMode in_lockMode);
	void _unlock();

public:
	GFXMemSurface();
	~GFXMemSurface();

   void setMemoryOwner(Bool sfcOwns);
   
   virtual GFXBitmap * getSurfaceBitmap();
   void setBitmap(GFXBitmap *bmp);
   
	static GFXSurface * create(GFXBitmap *pBitmap);
	static GFXSurface * create(Int32 lWidth, Int32 lHeight, Int32 bitDepth=8);

   Bool getRGBFormat(RGBFormat *rgb);
 	void flip();
	void update(const RectI *rect);
	void drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at);
};

inline void GFXMemSurface::_setGamma()
{
   
}   

inline void GFXMemSurface::setMemoryOwner(Bool sfcOwns)
{
   bmpIsMine = sfcOwns;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_G_MEM_H_
