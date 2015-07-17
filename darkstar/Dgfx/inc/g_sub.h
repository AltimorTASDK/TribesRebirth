//================================================================
//   
// $Workfile:   g_sub.h  $
// $Version$
// $Revision:   1.4  $
//   
// DESCRIPTION:
//   
//================================================================

#ifndef _G_SUB_H_
#define _G_SUB_H_


#include "g_surfac.h"
#include "d_funcs.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


class GFXSubSurface : public GFXSurface
{
protected:
   Int32    x0, y0;
   GFXSurface *pParent;
   
   Bool _setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, Bool in_rsvPal = true);
   void _setGamma();
	void _lock(const GFXLockMode in_lockMode);
	void _unlock();

public:
	GFXSubSurface();

	static Bool create(GFXSurface *&pSurface, GFXSurface *parent, const RectI *in_subRegion);
   void setParent(GFXSurface *newParent);
   void setRegion(const RectI *in_subRegion);

 	void flip();
	void update(const RectI *rect);
	void drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at);
   Bool getRGBFormat(RGBFormat *);
};

inline void GFXSubSurface::_setGamma()
{
   
}   

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_G_SUB_H_
