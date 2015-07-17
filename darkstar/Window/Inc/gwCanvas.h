//----------------------------------------------------------------------------

//	$Workfile:   gwcanvas.h  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:00  $

//----------------------------------------------------------------------------

#ifndef _GWCANVAS_H_
#define _GWCANVAS_H_

#include <gwBase.h>
#include "gdevice.h"
#include "gdmanag.h"

//----------------------------------------------------------------------------

class GFXSurface;

class GWCanvas: public GWWindow
{
private:
	static GWWindowClass* wClass;
	static int wClassCanvasCount;

protected:
	typedef GWWindow Parent;
	int modeBPP;	// Window Bits Per Pixel
	bool sizeLocked;
   Point2I sizeLockDim;

   //--------------------------------------
   GFXDevice *fullscreenDevice, *windowedDevice, *current;
   GFXPalette *palette;

   // True if we are in the process of switching so we can properly
   //  ignore onSize messages...
   //
   bool ignoreSizeMessages;
private:

	GWWindowClass* windowClass();
	bool setResolution(Point3I &size);
   void initSurfaces();

	void free();
   void surfaceSwap();

public:
   void swapSurfaces();
   void nextRes();
   void prevRes();

	GWCanvas();
	~GWCanvas();

   const char *getDeviceName(int id);
   int  getDeviceId(const char *name);
   bool setFullscreenDevice(const char *name);
   bool setWindowedDevice(const char *name);
   bool isGfxDriver(const char* in_pName);

   bool isFullScreen();
   void setFullScreen(bool flag);

   GFXDevice*  getCurrentDevice() { return current; }
	GFXDevice*  getWindowedDevice() { return windowedDevice; }
   GFXDevice*  getFullScreenDevice() { return fullscreenDevice; }
	GFXSurface* getSurface();
   bool create(LPCSTR wname, DWORD style,const Point2I& pos, 
                       const Point2I size, GWWindow* parent,int id, 
                       bool usePrefPos = TRUE, bool usePrefSize = FALSE);
	bool attach(HWND);
	void detach();
	void destroyWindow();
	void fitParent();
	using GWWindow::create;
	bool create(GWWindow* parent);

	void setBPP(int depth);
	int  getBPP();
	void lockSize();
	void unlockSize();
	bool isSizeLocked();

	virtual void paint();
	virtual void lock();
	virtual void unlock();
	virtual void render();
	virtual void flip();

   virtual void surfaceChanged() { } 

	virtual void setClientSize(Point2I size);
	virtual void setPosition(Point2I size);
	Point2I getClientSize();
   void    setPalette( GFXPalette *palette );

   void setEnforceCanvasSize(const Point2I& in_rMin,
                             const Point2I& in_rMax);

	// Windows events
	virtual void onActivate(UINT state, HWND hwndActDeact, BOOL fMinimized);
   virtual void onActivateApp(BOOL fActivate, DWORD dwThreadId);
	virtual void onPaletteChanged(HWND hwndPaletteChange);
	virtual BOOL onQueryNewPalette();
	virtual void onPaint();
	virtual void onSize(UINT state, int cx, int cy);
	virtual BOOL onEraseBkgnd(HDC hdc);
	virtual BOOL onWindowPosChanging(LPWINDOWPOS lpwpos);
   virtual void  onWindowPosChanged(const LPWINDOWPOS lpwpos);
   virtual void onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
	virtual void onSysKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
	virtual void onSysCommand(UINT cmd, int x, int y);
   virtual void onCancelMode();
   virtual void onGetMinMaxInfo(LPMINMAXINFO lpMinMaxInfo);
};

inline bool GWCanvas::isFullScreen()
{
   if (current) 
      return (current == fullscreenDevice);
   return false;
}

inline bool GWCanvas::isSizeLocked()
{
	return sizeLocked;
}

inline void GWCanvas::swapSurfaces()
{
   setFullScreen( !current->isFullscreen() );
}
#endif
