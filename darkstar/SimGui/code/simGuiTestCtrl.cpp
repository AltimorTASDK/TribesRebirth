#include "simGuiTestCtrl.h"
#include "g_surfac.h"

namespace SimGui
{

void TestControl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   RectI r(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1);
   sfc->drawRect2d_f(&r, hilight + 1);
   renderChildControls(sfc, offset, updateRect);
}

void TestControl::onMouseEnter(const Event &event)
{
   event;
   hilight = 1;
}

void TestControl::onMouseLeave(const Event &event)
{
   event;
   hilight = 0;
}

};