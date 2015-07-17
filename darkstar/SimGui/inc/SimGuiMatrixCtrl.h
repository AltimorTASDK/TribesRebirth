//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUIMATRIXCTRL_H_
#define _SIMGUIMATRIXCTRL_H_

#include "simGuiScrollCtrl.h"
#include "g_barray.h"
#include "darkstar.strings.h"

namespace SimGui
{

// the function of the scroll content control class
// is to notify the parent class that children have resized.
// basically it just calls it's parent (enclosing) control's
// childResized method which turns around and computes new sizes
// for the scroll bars

class MatrixCtrl : public ScrollCtrl
{
private:
   typedef ScrollCtrl Parent;

protected:
	Point2I headerDim;

	void calcScrollRects(void);

public:

   MatrixCtrl();

   void inspectRead(Inspect *);
   void inspectWrite(Inspect *);

	void drawBorder(GFXSurface *sfc, const Point2I &offset);

   DECLARE_PERSISTENT(MatrixCtrl);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};   
};

#endif //_SIMGUIMATRIXCTRL_H_
