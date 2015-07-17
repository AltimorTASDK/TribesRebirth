//----------------------------------------------------------------------------

//	$Workfile:   gwtscan.h  $
//	$Version$
//	$Revision:   1.1  $
//	$Date:   28 Sep 1995 14:00:02  $

//----------------------------------------------------------------------------

#ifndef _GWTSCAN_H_
#define _GWTSCAN_H_

#include <gwcanvas.h>

#include <ts.h>
#include <ts_gfx.h>

//----------------------------------------------------------------------------

class GWTSCanvas: public GWCanvas
{
protected:
	typedef GWCanvas Parent;
	TSRenderInterface* pInterface;

private:
	static void init();
   int fracBits;

public:
	GWTSCanvas();
	~GWTSCanvas();
	TSRenderInterface* getInterface();

   void setFracBits( int i );
	virtual void lock();
	virtual void unlock();
	virtual void render();
};

inline void GWTSCanvas::setFracBits( int i )
{
   fracBits = i;
   if (pInterface) pInterface->setFracBits( fracBits );
}


#endif
