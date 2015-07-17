//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include "stdafx.h"
#include "gfxwin.h"
#include "fn_table.h"

//----------------------------------------------------------------------------

GFXWindow::GFXWindow(HWND window)
{
	pBuffer = 0;
	pPalette = 0;
	hWindow = 0;
	fnTable = &rclip_table;
	if (window)
		attach(window);
}

GFXWindow::~GFXWindow()
{
	detach();
}

void GFXWindow::free()
{
	// We don't own the palette or the function table
	delete pBuffer;
	pBuffer = 0;
}

bool GFXWindow::isAttached() const
{
	return hWindow != 0;
}

bool GFXWindow::attach(HWND window)
{
	if (window == hWindow)
		return true;
	detach();
	hWindow = window;

	RECT rect;
	GetClientRect(hWindow,&rect);
	Point2I extent(rect.right - rect.left,rect.bottom - rect.top);
	if (!createBuffer(extent)) {
		hWindow = 0;
		return false;
	}
	return true;
}

bool GFXWindow::createBuffer(Point2I extent)
{
// dp-1/3	GFXCDSSurface::setFunctionTable(fnTable);
	// DMMSOLID added point
	Point2I temp(0,0);
	if (GFXCDSSurface::create(pBuffer, YES, extent.x, extent.y,
		hWindow, &temp))
	{
		if (pPalette)
			pBuffer->setPalette(pPalette, false);
		update();
		return true;
	}
	free();
	return false;
}

void GFXWindow::detach()
{
	hWindow = 0;
	free();
}

bool GFXWindow::setSize(Point2I extent)
{
	if(!isAttached()) {
		AfxMessageBox("GFXWindow::resize: Object not attached", MB_OK, 0);
		return false;
	}
	free();
	if (!createBuffer(extent)) {
		detach();
		return false;
	}
	return true;
}

void GFXWindow::setPalette(GFXPalette* pal)
{
	pPalette = pal;
	if (pBuffer)
		pBuffer->setPalette(pPalette, false);
}

void GFXWindow::lock()
{
// dp - 1/3	GFXCDSSurface::setFunctionTable(fnTable);
//    if (!pBuffer->isLocked())
	  pBuffer->lock();
}

void GFXWindow::unlock()
{
	update();
	pBuffer->unlock();
}

void GFXWindow::update()
{
	pBuffer->update();
}


//----------------------------------------------------------------------------

TSWindow::TSWindow(HWND window): Parent(window)
{
//	pRender = 0;
}

TSWindow::~TSWindow()
{
	free();
}

void TSWindow::free()
{
//	delete pRender;
//	pRender = 0;
}

bool TSWindow::attach(HWND window)
{
	free();
	return Parent::attach(window);
}

void TSWindow::detach()
{
	free();
	Parent::detach();
}

void TSWindow::setPalette(GFXPalette* pal)
{
	Parent::setPalette(pal);
	free();
}

bool TSWindow::setSize(Point2I extent)
{
	free();
	return Parent::setSize(extent);
}

void TSWindow::lock()
{
#if 0 // dp - 1/3
	if (pRender && pRender->getBuffer() != pBuffer) {
		delete pRender;
		pRender = 0;
	}
	if (!pRender && isAttached())
		pRender = new TSRenderInterface(pBuffer);
	if(!pRender) {
		AfxMessageBox("TSWindow::lock: Object not attached", MB_OK, 0);
		return;
	}

	// TSRenderInterface will lock the surface and
	// surfaces currently don't support nested locks
	//Parent::lock();

//    if (!pRender->isLocked())
	  pRender->lock();
#endif
}

void TSWindow::unlock()
{
#if 0 // dp - 1/3
	if(!pRender) {
		AfxMessageBox("TSWindow::unlock: Object not attached", MB_OK, 0);
		return;
	}
	pRender->unlock();
#endif
	update();
	// TSRenderInterface will unlock the surface and
	// surfaces currently don't support nested locks
	//Parent::unlock();
}

#if 0
TSRenderInterface* TSWindow::getRenderInterface()
{
  return pRender;
}
#endif


#if 0
// reset the edge list
void GFXWindow::resetEdgeList()
{
	AssertFatal(edgeClipping, "GFXWindow::resetEdgeList: edge clipping not active");

//    if (!pBuffer->isLocked())
	  pBuffer->lock();
//    else
//      return;
	pBuffer->clear(0);
	GFXEdgeList *edgeList = pBuffer->getEdgeList();
	edgeList->createFromSurface(pBuffer, 0);
	edgeList->pack();
	pBuffer->unlock();
	pBuffer->setEdgeList(edgeList);
}

void GFXWindow::setEdgeClipping(bool on)
{
	if (on == edgeClipping) return;  // already there, ignore it.
	
	if (on && ! pBuffer->getEdgeList())
	{
		// haven't created an edge list yet, make one now
		GFXEdgeList* edgeList = new GFXEdgeList(6000, 10000);
//		pBuffer->setEdgeList(edgeList);

//        if (!pBuffer->isLocked())
		  pBuffer->lock();
//        else
//          return;
		pBuffer->clear(0);
		edgeList->createFromSurface(pBuffer, 0);
		edgeList->pack();
		pBuffer->unlock();
		pBuffer->setEdgeList(edgeList);
		//setEdgeClipping(true);
	
		pBuffer->setFlag(GFX_DMF_ECLIP);
		fnTable     = &eclip_table;
	}
	else 
	{
		fnTable     = &rclip_table;
		if (pBuffer)
			pBuffer->clearFlag(GFX_DMF_ECLIP);
	}

	GFXCDSSurface::setFunctionTable(fnTable);
	GFXDDSurface::setFunctionTable(fnTable);

	edgeClipping = on;

	//if (on) resetEdgeList();	
}
#endif

