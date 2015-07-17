//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _GFXWIN_H_
#define _GFXWIN_H_

/*
These will get included by stdafx.h
#include <ml.h>
#include <gfx.h>
#include <g_cds.h>
#include <ts.h>
*/

#include "m_point.h"
#include "g_surfac.h"
//#include "ts_gfx.h" // was ts_renderInterface
#include "g_cds.h"

//----------------------------------------------------------------------------

class GFXWindow
{
	void free();
	bool createBuffer(Point2I extent);

protected:
	HWND hWindow;
	GFXSurface* pBuffer;
	FunctionTable* fnTable;
	GFXPalette* pPalette;
	bool		edgeClipping;


public:
	GFXWindow(HWND window = 0);
	virtual ~GFXWindow();
	GFXSurface* getBuffer() const;

	virtual bool isAttached() const;
	virtual bool attach(HWND window);
	virtual void detach();

	virtual bool setSize(Point2I extent);
	virtual void setPalette(GFXPalette*);
	virtual void setFunctionTable(FunctionTable* fn);

	virtual void lock();
	virtual void unlock();
	virtual void update();

#if 0
	void resetEdgeList();
	void setEdgeClipping(bool on);
#endif
};

inline void GFXWindow::setFunctionTable(FunctionTable* fn)
{
	fnTable = fn;
}

inline GFXSurface* GFXWindow::getBuffer() const
{
	return pBuffer;
}


//----------------------------------------------------------------------------

class TSWindow: public GFXWindow
{
	typedef GFXWindow Parent;

	void free();

protected:
//	TSRenderInterface* pRender;

public:
	TSWindow(HWND window = 0);
	virtual ~TSWindow();

	virtual bool attach(HWND window);
	virtual void detach();

	virtual void setPalette(GFXPalette*);
	virtual bool setSize(Point2I extent);

	virtual void lock();
	virtual void unlock();

//    TSRenderInterface*  getRenderInterface();
};



#endif
