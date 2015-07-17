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
#include "g_ddraw.h"

//----------------------------------------------------------------------------

GWCanvas::GWCanvas()
{
	pSurface = 0;
	fnTable     = &software_rclip_table;
	ddrawfnTable= &ddraw_rclip_table;
	modeBPP = 8;

	//	gfx_set_sub_shift_enable(false);
}

GWCanvas::~GWCanvas()
{
	free();
}

void GWCanvas::free()
{
	// We don't own the palette
	manager.setMode(GFX_SM_MODE_NONE);
	pSurface = 0;
}

GWWindowClass* GWCanvas::windowClass()
{
	// Default class used by all the canvases.
	static GWWindowClass* wClass = NULL;
	if (!wClass) {
		wClass = new GWWindowClass;
		wClass->registerClass
			(CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
				0,0,HBRUSH(HOLLOW_BRUSH));
	}
	return wClass;
}

bool GWCanvas::create(GWWindow* parent)
{
	free();
    return Parent::create(windowClass(),"GFX",
    	WS_CHILD | WS_VISIBLE,
		Point2I(0,0),parent->getSize(),
		parent,0);
}

bool GWCanvas::create(LPCSTR wname, DWORD style,const Point2I& pos,
	const Point2I size, GWWindow* parent,int id)
{
    return Parent::create(windowClass(),wname,style,pos,size,parent,id);
}

bool GWCanvas::createMDIChild(LPCSTR wname, DWORD style,const Point2I& pos,
	const Point2I size, GWWindow* parent,int id)
{
    return Parent::createMDIChild(windowClass(),wname,style,pos,size,parent,id);
}

bool GWCanvas::attach(HWND window)
{
	free();
	if (Parent::attach(window))
		return createSurface(getSize());
	return false;
}

void GWCanvas::detach()
{
	free();
	Parent::detach();
}

void GWCanvas::destroyWindow()
{
	free();
	Parent::destroyWindow();
}

void GWCanvas::setMode(GFX_SM_MODE mode)
{
	RECT rect;
   Point2I size;

   if (manager.getMode() == mode) return;

	pSurface = 0;
	switch (mode) 
	{
		case GFX_SM_MODE_DIRECT:
			GetWindowRect(hWnd,&windowRect);
			windowStyle = GetWindowLong(hWnd,GWL_STYLE);
			manager.setMode(mode);
         manager.getSize( &size );
         setClientSize( size );
			break;

		case GFX_SM_MODE_WINDOW:
			rect = windowRect;
			manager.setMode(mode);
         manager.getSize( &size );
			SetWindowLong(hWnd,GWL_STYLE,windowStyle);
         setClientSize( size );
         setPosition( Point2I(rect.left, rect.top) );
			break;
	}
	if (!pSurface) 
	{
		// pSurface will be NULL if Windows hasn't sent us
		// a size message.
		Point2I size = getSize();
		onSize(SIZE_RESTORED,size.x,size.y);
	}
}

void GWCanvas::setEdgeClipping(bool on)
{
	if (on) 
	{
		fnTable     = &software_eclip_table;
		ddrawfnTable= &ddraw_eclip_table;
		if (pSurface)
			pSurface->setFlag(GFX_DMF_ECLIP);
	} 
	else 
	{
		fnTable     = &software_rclip_table;
		ddrawfnTable= &ddraw_rclip_table;
		if (pSurface)
			pSurface->clearFlag(GFX_DMF_ECLIP);
	}
	GFXCDSSurface::setFunctionTable(fnTable);
	GFXDDSurface::setFunctionTable(ddrawfnTable);
 	GFXMemSurface::setFunctionTable(fnTable);
}


bool GWCanvas::createSurface(Point2I size)
{
//	if (manager.getAppHandle() == hWnd) 
	if (manager.getSurfaceHandle() == hWnd) 
	{
		// Resize existing window.
		if (manager.setResolution(size, modeBPP)) 
		{
			pSurface = manager.getBackSurface();
         manager.getSize( &size );
         if (manager.getMode() == GFX_SM_MODE_DIRECT)
            setClientSize( size );
			return true;
		}
	}
	else 
	{
		// Bind surface manager to the window.
		manager.init(getAppHandle(), getHandle());
		manager.setResolution(size, modeBPP);

		GFXCDSSurface::setFunctionTable(fnTable);
		GFXDDSurface::setFunctionTable(ddrawfnTable);
	 	GFXMemSurface::setFunctionTable(fnTable);

		if (manager.setMode(GFX_SM_MODE_WINDOW)) {
			pSurface = manager.getBackSurface();
			pSurface->createRasterList( 3000 );
			return true;
		}
	}
	detach();
	return false;
}


void GWCanvas::lock()
{
	AssertFatal(hWnd != 0, "GWCanvas::lock: Window not initialized");
	GFXCDSSurface::setFunctionTable(fnTable);
	GFXDDSurface::setFunctionTable(ddrawfnTable);
 	GFXMemSurface::setFunctionTable(fnTable);
	pSurface->lock();
}

void GWCanvas::unlock()
{
	pSurface->unlock();
}

void GWCanvas::flip()
{
	manager.getFrontSurface()->easyFlip();
}


void GWCanvas::adjSurfaceSize(int adj)
{
   if (manager.getMode() != GFX_SM_MODE_DIRECT) return;
   const Resolution *res = manager.getResolution(adj);
   createSurface(res->size);
}


void GWCanvas::fitParent()
{
	GWWindow* parent = getParent();
	if (parent) {
		Point2I size = parent->getSize();
		SetWindowPos(hWnd,0,0,0,size.x,size.y,
			SWP_NOZORDER); // | SWP_NOACTIVATE);
	}
}

void GWCanvas::paint()
{
	if (pSurface) {
		lock();
		render();
		unlock();
		flip();
	}
}

void GWCanvas::render()
{
//	pSurface->clear(0);
}

void GWCanvas::setClientSize(Point2I size)
{
	RECT rect;
   switch (manager.getMode())
   {
      case GFX_SM_MODE_DIRECT:
	      SetRect(&rect,0,0,size.x, size.y);
	      AdjustWindowRectEx(&rect,
	      	GetWindowLong(hWnd, GWL_STYLE),
	      	GetMenu(hWnd) != NULL,
	      	GetWindowLong(hWnd, GWL_EXSTYLE));
	      SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, 
	      	rect.right - rect.left,rect.bottom - rect.top,
	      	SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
         break;

		case GFX_SM_MODE_WINDOW:
      default:
         GWWindow::setClientSize( size );
         break;
   }
}

void GWCanvas::setPosition(Point2I pos)
{
	RECT rect;
   Point2I size;
   switch (manager.getMode())
   {
      case GFX_SM_MODE_DIRECT:
         //always force to force to upper left in direct
         size = getSize();
	      SetRect(&rect,0,0,size.x, size.y);
	      AdjustWindowRectEx(&rect,
	      	GetWindowLong(hWnd, GWL_STYLE),
	      	GetMenu(hWnd) != NULL,
	      	GetWindowLong(hWnd, GWL_EXSTYLE));
	      SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, 
	      	0, 0,
	      	SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS | SWP_NOSIZE);
         break;

		case GFX_SM_MODE_WINDOW:
	      SetWindowPos(getHandle(), HWND_NOTOPMOST, pos.x, pos.y, 0, 0, SWP_NOSIZE);
         break;

      default:
         GWWindow::setPosition( pos );
         break;
   }
}

//----------------------------------------------------------------------------

BOOL GWCanvas::onWindowPosChanging(LPWINDOWPOS lpwpos)
{
	// Override default window handler when going to direct
	// draw, it wants to shrink the window for some reason.
	if (manager.getMode() == GFX_SM_MODE_DIRECT)
		return false;
	return Parent::onWindowPosChanging(lpwpos);
}

void GWCanvas::onPaletteChanged(HWND hwndPaletteChange)
{
	if (hwndPaletteChange != hWnd)
		// Not just responding to own message,
		onQueryNewPalette();
}

BOOL GWCanvas::onQueryNewPalette()
{
	// Assumes class CS_OWNDC
	HDC hDC = GetDC(hWnd);
	if (pSurface)
		// Reselect the HPALETTE
		pSurface->setPalette(NULL,0,1);
	if (RealizePalette(hDC)) {
		// Palette has changed
		InvalidateRect(hWnd, NULL, TRUE);
		return true;
	}
	return false;
}

void GWCanvas::onPaint()
{
   Parent::onPaint();
	if (pSurface) {
		ValidateRect( hWnd, NULL );
		paint();
	}
}

void GWCanvas::onSize(UINT, int cx, int cy)
{
	if (cx > 0 && cy > 0)
	   createSurface(Point2I(cx, cy));
}

BOOL GWCanvas::onEraseBkgnd(HDC)
{
	// No background erase
	// Hollow brush doesn't seem to work.
	return false;
}

void GWCanvas::onSysKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch( vk ) {
   	case VK_TAB:
      case VK_ESCAPE:
	     	if (!fDown)
				setMode(GFX_SM_MODE_WINDOW);
         break;
		case VK_RETURN:
		  	if (!fDown)
	     		if (getMode() == GFX_SM_MODE_WINDOW)
					setMode(GFX_SM_MODE_DIRECT);
				else
					setMode(GFX_SM_MODE_WINDOW);
			break;
      default:
			Parent::onSysKey(vk,fDown,cRepeat,flags);
         break;
	}
}
