#include "SimGuiActiveCtrl.h"
#include "Fear.Strings.h"

#include "FearGuiHudCtrl.h"
#include "FearGuiDialog.h"
#include "fearGuiShellPal.h"

#include <g_bitmap.h>
#include <g_surfac.h>
#include <simResource.h>

namespace FearGui
{

bool FearGuiDialog::becomeFirstResponder()
{
   return TRUE;
}

bool FearGuiDialog::loseFirstResponder()
{
   return FALSE;
}

void FearGuiDialog::onWake(void)
{
   root->makeFirstResponder(this);
}

bool FearGuiDialog::onAdd(void)
{
   if (! Parent::onAdd())
      return FALSE;
   
   return TRUE;
}

void FearGuiDialog::parentResized(const Point2I &, const Point2I &newParentExtent)
{
   extent.x = max(min((int)(newParentExtent.x - 1), (int)fixedSize.x), 1);
   extent.y = max(min((int)(newParentExtent.y - 1), (int)fixedSize.y), 1);
      
   position.x = max((newParentExtent.x - extent.x) / 2, Int32(0));
   position.y = max((newParentExtent.y - extent.y) / 2, Int32(0));
}

void FearGuiDialog::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   //avoid the compiler warning
   updateRect;
   
   //draw the screen
   //initialize our vertices
   Point3F poly[4];
   for (int j = 0; j < 4; j++)
   {
      poly[j].z = 1.0f;
   }
   
   //DRAW THE TRANSPARENT BACKGROUND
   poly[0].x = offset.x;
   poly[0].y = offset.y;
   poly[1].x = offset.x + extent.x - 1;
   poly[1].y = offset.y;
   poly[2].x = offset.x + extent.x - 1;
   poly[2].y = offset.y + extent.y - 1;
   poly[3].x = offset.x;
   poly[3].y = offset.y + extent.y - 1;
   
   //draw the translucent box
   sfc->setFillMode(GFX_FILL_CONSTANT);
   sfc->setShadeSource(GFX_SHADE_NONE);
   sfc->setHazeSource(GFX_HAZE_NONE);
   sfc->setAlphaSource(GFX_ALPHA_FILL);
   sfc->setTransparency (false);
   sfc->setFillColor(254, ALPHAPAL_INDEX);

   for (int i = 0; i < 4; i++)
   {
      sfc->addVertex(poly + i);
   }
   sfc->emitPoly();
   sfc->setAlphaSource(GFX_ALPHA_NONE);
   
/*   
   //draw in the lines
   sfc->drawLine2d(&Point2I(offset.x + 1, offset.y + 1),
                     &Point2I(offset.x + extent.x - 3, offset.y + 1),
                     BLUE_HILITE);
   
   //only draw the bottom line if the window is full size
   if (extent.y == fixedSize.y)
   {
      sfc->drawLine2d(&Point2I(offset.x + 1, offset.y + extent.y - 3),
                        &Point2I(offset.x + extent.x - 3, offset.y + extent.y - 3),
                        BLUE_HILITE);
   }
*/
                     
   //render the chil'en                     
   renderChildControls(sfc, offset, updateRect);
}

void FearGuiDialog::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   insp->write( IDITG_MENU_FIXED_SIZE, fixedSize );
   
}

void FearGuiDialog::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read( IDITG_MENU_FIXED_SIZE, fixedSize );
   if (parent)
   {
      parentResized(parent->extent, parent->extent);
   }
}   

Persistent::Base::Error FearGuiDialog::write( StreamIO &sio, int a, int b)
{
   fixedSize.write(sio);
   return Parent::write(sio, a, b);
}

Persistent::Base::Error FearGuiDialog::read( StreamIO &sio, int a, int b)
{
   fixedSize.read(sio);
   return Parent::read(sio, a, b);
}

IMPLEMENT_PERSISTENT_TAG( FearGuiDialog, FOURCC('F','G','d','g'));

};