//----------------------------------------------------------------------------

//	$Workfile:   gwtscan.cpp  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:02  $

//----------------------------------------------------------------------------

#include "gwbase.h"
#include "gwtscan.h"


//----------------------------------------------------------------------------

GWTSCanvas::GWTSCanvas()
{
	pInterface = 0;
   fracBits = 3;
}

GWTSCanvas::~GWTSCanvas()
{
	delete pInterface;
}

TSRenderInterface* GWTSCanvas::getInterface()
{
	AssertFatal(hWnd != 0, "GWTSCanvas::lock: Window not initialized");
	if (!pInterface || pInterface->getBuffer() != pSurface) {
		delete pInterface;
		pInterface = new TSRenderInterface(pSurface);
      pInterface->setFracBits( fracBits );
	}
	return pInterface;
}

void GWTSCanvas::lock()
{
	AssertFatal(hWnd != 0, "GWTSCanvas::lock: Window not initialized");
	Parent::lock();
	getInterface()->lock();
}

void GWTSCanvas::unlock()
{
	pInterface->unlock();
	Parent::unlock();
}

void GWTSCanvas::render()
{
	pSurface->clear(0);
}

