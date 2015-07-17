#ifndef _H_SIMGUITESTCTRL
#define _H_SIMGUITESTCTRL

#include "simGuiCtrl.h"

namespace SimGui
{

class TestControl : public Control
{
private:
   typedef Control Parent;
   int hilight;
public:   
   DECLARE_PERSISTENT(TestControl);
   TestControl() {hilight = 0;};
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   void onMouseEnter(const Event &event);
   void onMouseLeave(const Event &event);
};

};

#endif
