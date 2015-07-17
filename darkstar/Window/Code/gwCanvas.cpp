//--------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//--------------------------------------------------------------------------------
#include "gwBase.h"
#include "gwCanvas.h"
#include "gwDeviceManager.h"
#include "g_surfac.h"
#include "g_bitmap.h"

//----------------------------------------------------------------------------

GWWindowClass* GWCanvas::wClass = NULL;
int GWCanvas::wClassCanvasCount = 0;


//----------------------------------------------------------------------------
GWCanvas::GWCanvas()
{
	modeBPP = 8;
	sizeLocked = false;
   current = NULL;
   palette = NULL;
   windowedDevice = NULL;
   fullscreenDevice = NULL;
   ignoreSizeMessages = false;
	wClassCanvasCount++;
}


//------------------------------------------------------------------------------
GWCanvas::~GWCanvas()
{
	if (!--wClassCanvasCount) {
	   delete wClass;
		wClass = 0;
	}
	free();
}

//------------------------------------------------------------------------------
void GWCanvas::free()
{
	// We don't own the palette so DON'T delete it!
   if(fullscreenDevice) {
      fullscreenDevice->disable();
      deviceManager.freeDevice(fullscreenDevice);
   }
   if(windowedDevice) {
      windowedDevice->disable();
      deviceManager.freeDevice(windowedDevice);
   }

   current = windowedDevice = fullscreenDevice = NULL;
}


//------------------------------------------------------------------------------
GWWindowClass* GWCanvas::windowClass()
{
	// Default class used by all the canvases.
	if (!wClass) {
		wClass = new GWWindowClass;
		wClass->registerClass
			(CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
				0,LoadCursor(NULL,IDC_ARROW),0);
	}
	return wClass;
}


//------------------------------------------------------------------------------
bool GWCanvas::create(GWWindow* parent)
{
	free();
    return Parent::create(windowClass(),"GFX",
    	WS_CHILD | WS_VISIBLE,
		Point2I(0,0),parent->getSize(),
		parent,0);
}


//------------------------------------------------------------------------------
bool GWCanvas::create(LPCSTR wname, DWORD style,const Point2I& pos,
	const Point2I size, GWWindow* parent,int id, bool usePrefPos, bool usePrefSize)
{
   ignoreSizeMessages = true;
   bool ret = Parent::create(windowClass(),wname,style,pos,size,parent,id,usePrefPos, usePrefSize);
   if(ret)
   {
      initSurfaces();
      if ( current )
         current->enable( palette, GFX_DMF_RCLIP);
   }
   ignoreSizeMessages = false;
   return ret;
}


//------------------------------------------------------------------------------
bool GWCanvas::attach(HWND window)
{
	free();
	if (Parent::attach(window))
	{
      Point2I size=getClientSize();
		return setResolution( Point3I( size.x, size.y, 0 ) );
	}
	return false;
}


//------------------------------------------------------------------------------
void GWCanvas::detach()
{
	free();
	Parent::detach();
}


//------------------------------------------------------------------------------
void GWCanvas::destroyWindow()
{
	free();
	Parent::destroyWindow();
}


//------------------------------------------------------------------------------
void GWCanvas::initSurfaces()
{
   windowedDevice = deviceManager.allocateDevice("Software", getAppHandle(), getHandle());
   fullscreenDevice = deviceManager.allocateDevice("Software", getAppHandle(), getHandle());
   current = windowedDevice;
}

//------------------------------------------------------------------------------
int GWCanvas::getDeviceId(const char *name)
{
   return deviceManager.getDeviceId(name);
}  

//------------------------------------------------------------------------------
const char *GWCanvas::getDeviceName(int id)
{
   return deviceManager.getDeviceName(id);
}  
 
bool GWCanvas::setFullscreenDevice(const char *name)
{
   GFXPalette *palette = NULL;
   DWORD flags         = 0;

   if(!deviceManager.deviceNameValid(name))
      return false;

   // first check if we already have the desired fullscreenDevice
   // if so, we don't need to do any additional work
   if (fullscreenDevice)
   {
      const char *deviceName = getDeviceName(fullscreenDevice->getDeviceId());
      if (deviceName && !stricmp(deviceName, name) )
         return true;
   }

   bool fs = false;

   if(current->isFullscreen())
   {
      fs = true;
      if(current->getSurface())   
      {
         palette = current->getSurface()->getPalette();
         flags   = current->getSurface()->getFlags();
      }
      current->disable();
      current = NULL;
   }
   deviceManager.freeDevice(fullscreenDevice);
   fullscreenDevice = deviceManager.allocateDevice(name, getAppHandle(), getHandle());
   if(fs)
   {
      current = fullscreenDevice;
      current->modeSwitch(1);
      current->enable(palette, flags);
   }
   if(current && current->getSurface())
      current->getSurface()->flushTextureCache();
     
   surfaceChanged(); 
   return (fullscreenDevice != NULL);
}

bool GWCanvas::setWindowedDevice(const char *name)
{
   GFXPalette *palette = NULL;
   DWORD flags         = 0;

   if(!deviceManager.deviceNameValid(name))
      return false;

   bool win = false;

   if(!current->isFullscreen())
   {
      win = true;
      if(current->getSurface())   
      {
         palette = current->getSurface()->getPalette();
         flags   = current->getSurface()->getFlags();
      }
      ignoreSizeMessages = true;
      current->disable();
      ignoreSizeMessages = false;
      current = NULL;
   }
   deviceManager.freeDevice(windowedDevice);
   windowedDevice = deviceManager.allocateDevice(name, getAppHandle(), getHandle());
   if(win)
   {
      current = windowedDevice;
      ignoreSizeMessages = true;
      GWWindow::setClientSize(current->getResolution());
      ignoreSizeMessages = false;

      current->modeSwitch(0);
      current->enable(palette, flags);
   }
   if(current && current->getSurface()) {
      current->getSurface()->flushTextureCache();
   }
   
   surfaceChanged();
   return (windowedDevice != NULL);
}

//------------------------------------------------------------------------------
void GWCanvas::onActivate(UINT state, HWND hwndActDeact, BOOL fMinimized)
{
   Parent::onActivate(state, hwndActDeact, fMinimized);
}

void
GWCanvas::onActivateApp(BOOL fActivate, DWORD dwThreadId)
{
   if (fActivate == FALSE && (current && current->isFullscreen())) {
      SendMessage(getHandle(), WM_SYSCOMMAND, SC_MINIMIZE, 0);
   }
   Parent::onActivateApp(fActivate, dwThreadId);
}

//------------------------------------------------------------------------------
bool GWCanvas::setResolution(Point3I &size)
{
   if (!current)
   {
      initSurfaces();
      if ( current )
         current->enable( palette, GFX_DMF_RCLIP);
   }

   if ( current )
   {
      const Point2I &curRes = current->getResolution();
      Point2I newRes(size.x, size.y);
      if(newRes.x != curRes.x || newRes.y != curRes.y || current->isFullscreen() == false) {
         current->setResolution( newRes );
      }

      AssertWarn(current->getSurface(), "GWCanvas::setResolution: Unable to set video mode");
      return ( current->getSurface() != NULL );
   }

   AssertWarn(0,"GWCanvas::setResolution: NO CURRENT");
	detach();
	return false;
}


//------------------------------------------------------------------------------
void GWCanvas::lock()
{
	if ( current ) current->lock();
}


//------------------------------------------------------------------------------
void GWCanvas::unlock()
{
	if ( current ) current->unlock();
}


//------------------------------------------------------------------------------
void GWCanvas::flip()
{
   if ( current ) current->flip();
}


//------------------------------------------------------------------------------
void GWCanvas::fitParent()
{
	GWWindow* parent = getParent();
	if (parent) {
		Point2I size = parent->getSize();
		SetWindowPos(hWnd, 0,0,0, size.x, size.y, SWP_NOZORDER); // | SWP_NOACTIVATE);
	}
}


//------------------------------------------------------------------------------
void GWCanvas::paint()
{
	if (current->getSurface()) 
	{
		lock();
		render();
		unlock();
		flip();
	}
}


//------------------------------------------------------------------------------
void GWCanvas::render()
{
}


//------------------------------------------------------------------------------
void GWCanvas::setClientSize(Point2I size)
{
	if (!sizeLocked) {
      surfaceChanged();
		if (current && current->isFullscreen())
		   setResolution( Point3I(size.x, size.y, 0) );
		else
		   GWWindow::setClientSize(size);
	}
}


//------------------------------------------------------------------------------
void GWCanvas::setPosition(Point2I pos)
{
   GWWindow::setPosition( pos );
}


//----------------------------------------------------------------------------
BOOL GWCanvas::onWindowPosChanging(LPWINDOWPOS lpwpos)
{
	// Override default window handler when going to direct
	// draw, it wants to shrink the window so it will not hide the
   // task ba;
   if ( current && current->isFullscreen() )
      return ( false );

   if ( !(lpwpos->flags & SWP_NOSIZE) && sizeLocked )
   {
      // the window is trying to size when size is locked
      if ((lpwpos->cx != 0) || (lpwpos->cy != 0))
      {
         lpwpos->cx = sizeLockDim.x + (GetSystemMetrics(SM_CXFIXEDFRAME) << 1);
         lpwpos->cy = sizeLockDim.y + (GetSystemMetrics(SM_CYFIXEDFRAME) << 1) + GetSystemMetrics(SM_CYCAPTION);
      }
   }
	return Parent::onWindowPosChanging(lpwpos);
}

void
GWCanvas::onWindowPosChanged(const LPWINDOWPOS lpwpos)
{
	Parent::onWindowPosChanged(lpwpos);
   // Do nothing?
}




//------------------------------------------------------------------------------
void GWCanvas::onPaletteChanged(HWND hwndPaletteChange)
{
	if (hwndPaletteChange != hWnd)
		// Not just responding to own message,
		onQueryNewPalette();
}


//------------------------------------------------------------------------------
BOOL GWCanvas::onQueryNewPalette()
{
	// Assumes class CS_OWNDC
	HDC hDC = GetDC(hWnd);
	if (current && current->getSurface())
		// Reselect the HPALETTE
		current->getSurface()->setPalette(NULL,0,1);
	if (RealizePalette(hDC)) {
		// Palette has changed
		InvalidateRect(hWnd, NULL, TRUE);
		return true;
	}
	return false;
}


//------------------------------------------------------------------------------
void GWCanvas::onPaint()
{
   Parent::onPaint();
	if (current && current->getSurface()) 
	{
		ValidateRect( hWnd, NULL );
		paint();
	}
}


//------------------------------------------------------------------------------
void GWCanvas::onSize(UINT state, int cx, int cy)
{
   // Ignore these messages if we are in the process of switching devices...
   if (ignoreSizeMessages ||
      (current && current->isFullscreen()))
      return;

   setResolution( Point3I(cx, cy, 0) );
   Parent::onSize( state, cx, cy);
}


//------------------------------------------------------------------------------
BOOL GWCanvas::onEraseBkgnd(HDC hDC)
{
   return (Parent::onEraseBkgnd(hDC));
}

void GWCanvas::nextRes()
{
   if(current && !sizeLocked)
      current->nextRes();
}

void GWCanvas::prevRes()
{
   if(current && !sizeLocked)
      current->prevRes();
}

//------------------------------------------------------------------------------
Point2I newSize(0, 0);
Point2I newPosition(0, 0);
bool usePosition = false;

void GWCanvas::surfaceSwap()
{
   ignoreSizeMessages = true;
   GFXPalette *pPalette = NULL;
   DWORD flags         = 0;

   if ( current && current->getSurface() )   
   {
      pPalette = current->getSurface()->getPalette();
      flags    = current->getSurface()->getFlags();
   }

   current->disable();
   if(current->isFullscreen()) {
      current = windowedDevice;
      current->modeSwitch(0);
      current->enable(pPalette, flags);
   } else {
      current = fullscreenDevice;
      current->modeSwitch(1);
      current->enable(pPalette, flags);
   }
   
   if ( current )
   {
      if (current->isFullscreen())
      {
		   if (sizeLocked)
		      setResolution( Point3I(sizeLockDim.x, sizeLockDim.y, 0) );
      }
      else {
         if (usePosition) {
            setPosition(newPosition);
            setClientSize(newSize);
            usePosition = false;
            setResolution(Point3I(newSize.x, newSize.y, 0));
         } else {
            GWWindow::setClientSize( current->getResolution() );
         }
      }
   }

   if(current && current->getSurface())
      current->getSurface()->flushTextureCache();
   
   palette = pPalette;
   setPalette(palette);
   surfaceChanged();
   ignoreSizeMessages = false;
}


//------------------------------------------------------------------------------
void GWCanvas::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch( vk ) {
		case VK_LWIN:
		case VK_RWIN:
		case VK_APPS:
			break;
      case VK_F10:                  // in Win95/NT4, F10 activates system menu, this prevents it
         break;

   	default:
			Parent::onKey(vk,fDown,cRepeat,flags);
         break;
	}
}

//------------------------------------------------------------------------------
void GWCanvas::onSysKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch( vk ) {
		case VK_RETURN:
		  	if (!fDown)
            setFullScreen( !current->isFullscreen() );   // toggle full screen state
			break;

      case VK_MENU:
      case VK_LMENU:
      case VK_RMENU:
         break;

      default:
			Parent::onSysKey(vk,fDown,cRepeat,flags);
         break;
	}
}

//------------------------------------------------------------------------------

void GWCanvas::onCancelMode()
{
//   setFullScreen(false);   // doesn't do anything if not fullScreen
   Parent::onCancelMode();
}   

//------------------------------------------------------------------------------
void GWCanvas::onSysCommand(UINT cmd, int x, int y)
{
   static   bool    wasFullScreen = false;
   static   Point2I oldPosition(0, 0);
   static   Point2I oldSize(0, 0);
   switch (cmd & 0xFFF0)
   {
	   case SC_SCREENSAVE:	   // disable the Screen Saver
	   case SC_MONITORPOWER:	// disable the Monitor Power Down
         // if we are the top-most window disable
	      if (GetForegroundWindow() == getHandle())
            return;
      case SC_MINIMIZE:
         wasFullScreen = current->isFullscreen();
         if (current->isFullscreen()) {
            surfaceSwap();
         } else {
         }
         break;

      case SC_MAXIMIZE:
         if (!current->isFullscreen()) {
            Point2I test = getPosition();
            if (test.x >= 0 && test.y >= 0 && test.x < 2000 && test.y < 2000) { // HACKHACKHACK
               oldPosition = getPosition();
               oldSize     = getClientSize();
            }
            surfaceSwap();
         }
         return;

      case SC_RESTORE:
         if (wasFullScreen && !current->isFullscreen()) {
            newPosition = oldPosition;
            newSize     = oldSize;
            usePosition = true;
            setFullScreen(true);
         } else if ( current->isFullscreen() ) {
            newPosition = oldPosition;
            newSize     = oldSize;
            usePosition = true;
            surfaceSwap();
         }
         wasFullScreen = false;
         break;
   }
   Parent::onSysCommand( cmd, x, y);    
}

//------------------------------------------------------------------------------

void GWCanvas::onGetMinMaxInfo(LPMINMAXINFO lpMinMaxInfo)
{
   WINDOWPLACEMENT wp;
   wp.length = sizeof(WINDOWPLACEMENT);
   GetWindowPlacement(getHandle(),&wp);

   lpMinMaxInfo->ptMinTrackSize.x = minSize.x;
   lpMinMaxInfo->ptMinTrackSize.y = minSize.y;
   lpMinMaxInfo->ptMaxTrackSize.x = maxSize.x;
   lpMinMaxInfo->ptMaxTrackSize.y = maxSize.y;

   lpMinMaxInfo->ptMaxSize.x     = wp.rcNormalPosition.right  - wp.rcNormalPosition.left;
   lpMinMaxInfo->ptMaxSize.y     = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
   lpMinMaxInfo->ptMaxPosition.x = wp.rcNormalPosition.left;
   lpMinMaxInfo->ptMaxPosition.y = wp.rcNormalPosition.top;
}   

//------------------------------------------------------------------------------

void GWCanvas::setBPP(int depth)
{
	modeBPP = depth;
}

int GWCanvas::getBPP()
{
	return modeBPP;
}


//------------------------------------------------------------------------------

void GWCanvas::lockSize()
{
	if (!sizeLocked) {
		int style = GetWindowLong(hWnd,GWL_STYLE);
		style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
		SetWindowLong(hWnd,GWL_STYLE,style);
      sizeLockDim = getClientSize();
		if (!current->isFullscreen()) {
			// The border size changes, so we need to force a reset.
			setClientSize(getClientSize());
		}
		sizeLocked = true;
	}
}

void GWCanvas::unlockSize()
{
	if (sizeLocked) {
		sizeLocked = false;
		int style = GetWindowLong(hWnd,GWL_STYLE);
		style |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
		SetWindowLong(hWnd,GWL_STYLE,style);
		if (!current->isFullscreen()) {
			// The border size changes, so we need to force a reset.
			setClientSize(getClientSize());
		}
	}
}

//------------------------------------------------------------------------------

GFXSurface* GWCanvas::getSurface()
{
	return current ? current->getSurface() : NULL;
}

Point2I GWCanvas::getClientSize()
{
   if ( !(current && current->getSurface()) ) return ( Parent::getClientSize() );
   return ( Point2I( current->getSurface()->getWidth(), current->getSurface()->getHeight() ) );
}

void GWCanvas::setPalette( GFXPalette *_palette )
{
   if ( current && current->getSurface() )   
      current->getSurface()->setPalette( _palette );
   else
      palette = _palette;
}


bool
GWCanvas::isGfxDriver(const char* in_pName)
{
   if(!deviceManager.deviceNameValid(in_pName))
      return false;
   return true;   
}

void
GWCanvas::setEnforceCanvasSize(const Point2I& in_rMin,
                               const Point2I& in_rMax)
{
   minSize = in_rMin;
   maxSize = in_rMax;

   Int32 capY = ::GetSystemMetrics(SM_CYSIZEFRAME)*2 + ::GetSystemMetrics(SM_CYCAPTION);
   Int32 capX = ::GetSystemMetrics(SM_CXSIZEFRAME)*2;

   minSize.x += capX;
   maxSize.x += capX;
   minSize.y += capY;
   maxSize.y += capY;
}

void
GWCanvas::setFullScreen(bool flag)
{
   if ( current->isFullscreen() == flag)
      return;  // (isFullScreen xor flag) condition is met

   if (current->isFullscreen())
      SendMessage(getHandle(), WM_SYSCOMMAND, SC_RESTORE, 0);
   else
      PostMessage(getHandle(), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
}

