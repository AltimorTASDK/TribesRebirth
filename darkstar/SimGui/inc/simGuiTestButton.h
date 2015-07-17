//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUITESTBUTTON_H_
#define _SIMGUITESTBUTTON_H_

#include "simGuiSimpleText.h"

namespace SimGui
{

class DLLAPI TestButton : public SimpleText
{
private:
   typedef SimpleText Parent;

public:   
   TestButton();

   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   void drawBorder(GFXSurface *sfc, RectI &r, int color);
   
   DECLARE_PERSISTENT(TestButton);
};

};

#endif //_SIMGUITESTBUTTON_H_
