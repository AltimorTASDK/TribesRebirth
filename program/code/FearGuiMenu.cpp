#include "SimGuiActiveCtrl.h"
#include "FearGuiHudCtrl.h"
#include "Fear.Strings.h"
#include "fearGuiShellPal.h"

#include <g_bitmap.h>
#include <g_surfac.h>
#include <simResource.h>

namespace FearGui
{

class FearGuiMenu : public SimGui::Control
{
   typedef SimGui::Control Parent;
   
   Point2I fixedSize;
   
public:
   FearGuiMenu();
   
   bool onAdd(void);
   void onPreRender() { setUpdate(); }
   void onRender(GFXSurface *, Point2I, const Box2I &);

   bool becomeFirstResponder();

   bool onSimActionEvent (const SimActionEvent *event);
	bool processEvent(const SimEvent* event);
   
   void inspectWrite(Inspect* insp);
   void inspectRead(Inspect *insp);
   
   DECLARE_PERSISTENT(FearGuiMenu);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

FearGuiMenu::FearGuiMenu(void)
{
   extent.set(1, 1);
   fixedSize.set(1, 1);
}

bool FearGuiMenu::becomeFirstResponder()
{
   return false;
}

bool FearGuiMenu::onSimActionEvent (const SimActionEvent *event)
{
	switch(event->action)
	{
		case IDACTION_TOGGLE:
         setVisible(! isVisible());
			break;
		case IDACTION_TURN_ON:
         setVisible(TRUE);
			break;
		case IDACTION_TURN_OFF:
         setVisible(FALSE);
			break;
   }
   return true;
}
      
bool FearGuiMenu::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
		onEvent (SimActionEvent);
		
		default:
			return Parent::processEvent (event);
	}
}

bool FearGuiMenu::onAdd(void)
{
   if (! Parent::onAdd())
      return FALSE;
      
   return TRUE;
}

extern void drawBracket(GFXSurface *sfc, int x, int ty, int by, int dx);

void FearGuiMenu::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
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

   drawBracket(sfc, offset.x, offset.y, offset.y + extent.y - 1, 6);
   drawBracket(sfc, offset.x+extent.x - 1, offset.y, offset.y + extent.y - 1, -6);


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

void FearGuiMenu::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   insp->write( IDITG_MENU_FIXED_SIZE, fixedSize );
   
}

void FearGuiMenu::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read( IDITG_MENU_FIXED_SIZE, fixedSize );
   if (parent)
   {
      parentResized(parent->extent, parent->extent);
   }
}   

Persistent::Base::Error FearGuiMenu::write( StreamIO &sio, int a, int b)
{
   fixedSize.write(sio);
   return Parent::write(sio, a, b);
}

Persistent::Base::Error FearGuiMenu::read( StreamIO &sio, int a, int b)
{
   fixedSize.read(sio);
   return Parent::read(sio, a, b);
}

IMPLEMENT_PERSISTENT_TAG( FearGuiMenu, FOURCC('F','G','m','u'));

};