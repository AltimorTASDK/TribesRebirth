//----------------------------------------------------------------------------

//	$Workfile:   gwcanvas.h  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:00  $

//----------------------------------------------------------------------------

#ifndef _GWCANVAS_H_
#define _GWCANVAS_H_

#include <gfx.h>
#include <g_cds.h>

#include <gwBase.h>
#include <gwSMan.h>


//----------------------------------------------------------------------------

class GWCanvas: public GWWindow
{
protected:
	typedef GWWindow Parent;
	GWSurfaceManager manager;
	GFXSurface* pSurface;
	FunctionTable* fnTable;
	FunctionTable* ddrawfnTable;
	RECT windowRect;
	LONG windowStyle;
	int modeBPP;	// Window Bits Per Pixel

private:
	GWWindowClass* windowClass();
	bool createSurface(Point2I size);
	void free();

public:
	GWCanvas();
	~GWCanvas();

	GFXSurface* getSurface();
   bool create(LPCSTR wname, DWORD style,const Point2I& pos, const Point2I size, GWWindow* parent,int id);
   bool createMDIChild(LPCSTR wname, DWORD style,const Point2I& pos, const Point2I size, GWWindow* parent,int id);
	bool attach(HWND);
	void detach();
	void destroyWindow();
	void fitParent();
	using GWWindow::create;
	bool create(GWWindow* parent);

	void setBPP(int depth);
	int  getBPP();
	void setFunctionTable(FunctionTable* fn);
	FunctionTable* getFunctionTable();
	void setEdgeClipping(bool on);
	void setMode(GFX_SM_MODE mode);
	GFX_SM_MODE getMode();
	void flip();
   void adjSurfaceSize(int adj);
   const Resolution* getResolution();
   const Resolution* enumResolution(bool reset);

	virtual void paint();
	virtual void lock();
	virtual void unlock();
	virtual void render();

	virtual void setClientSize(Point2I size);
	virtual void setPosition(Point2I size);

	// Windows events
	virtual void onPaletteChanged(HWND hwndPaletteChange);
	virtual BOOL onQueryNewPalette();
	virtual void onPaint();
	virtual void onSize(UINT state, int cx, int cy);
	virtual BOOL onEraseBkgnd(HDC hdc);
	virtual BOOL onWindowPosChanging(LPWINDOWPOS lpwpos);
	virtual void onSysKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
};


inline void GWCanvas::setBPP(int depth)
{
	modeBPP = depth;
}

inline int GWCanvas::getBPP()
{
	return modeBPP;
}

inline void GWCanvas::setFunctionTable(FunctionTable* fn)
{
	fnTable = fn;
}

inline FunctionTable* GWCanvas::getFunctionTable()
{
	return fnTable;
}

inline GFXSurface* GWCanvas::getSurface()
{
	return pSurface;
}

inline GFX_SM_MODE GWCanvas::getMode()
{
	return manager.getMode();
}

inline const Resolution* GWCanvas::getResolution()
{
   return manager.getResolution(0);
}

inline const Resolution* GWCanvas::enumResolution(bool reset)
{
   return (manager.enumResolution(reset));
}


#endif
