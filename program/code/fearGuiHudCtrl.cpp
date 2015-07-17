//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "fearGuiHudCtrl.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "simGame.h"
#include "fear.strings.h"
#include "fearGlobals.h"
#include "player.h"
#include "fearGuiShellPal.h"

namespace FearGui
{

//----------------------------------------------------------------------------

const char * hudDirSprintfString = "Huds\\%s.hud";
const char * hudGuiBaseName = "playGui";

//----------------------------------------------------------------------------

HudCtrl::HudCtrl()
{
   stateOver = stateDragging = statePressed = false;
   
   fracPos.set(0,0);
}

HudCtrl::~HudCtrl()
{

}

//----------------------------------------------------------------------------

void HudCtrl::onMouseDown(const SimGui::Event &event)
{
   event;
   hitPt = globalToLocalCoord(event.ptMouse);
   stateDragging = true;
   mouseLock();
}

void HudCtrl::onMouseDragged(const SimGui::Event &event)
{
   event;
   Point2I delta = event.ptMouse;
   delta -= hitPt;

   position = parent->globalToLocalCoord(event.ptMouse);
   position -= hitPt;

   snap();
}

void HudCtrl::onMouseUp(const SimGui::Event &event)
{
   event;
   event;
   stateDragging = false;
   mouseUnlock();
}

void HudCtrl::snap()
{
   if (! parent) return;
   Point2I oldPosition = position;
   
   if(position.x < SnapDistance)
      position.x = 0;
   else if((position.x + extent.x) > (parent->extent.x - SnapDistance))
      position.x = parent->extent.x - extent.x;
   if(position.y < SnapDistance)
      position.y = 0;
   else if((position.y + extent.y) > (parent->extent.y - SnapDistance))
      position.y = parent->extent.y - extent.y;
      
   //recalculate only if the position changed
   if (position.x != oldPosition.x || position.y != oldPosition.y)
   {
      fracPos.x = position.x / float(parent->extent.x - extent.x);
      fracPos.y = position.y / float(parent->extent.y - extent.y);
   }
}

void HudCtrl::parentResized(const Point2I &, const Point2I &newParentExtent)
{
   position.x = fracPos.x * (newParentExtent.x - extent.x);
   position.y = fracPos.y * (newParentExtent.y - extent.y);
}

void HudCtrl::resize(const Point2I &, const Point2I &newExtent)
{
   extent = newExtent;
   snap();
}

bool HudCtrl::onAdd(void)
{
   if(!Parent::onAdd())
      return false;
   return true;
}
   
void HudCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   updateRect;
   RectI frame(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1);
   sfc->drawRect2d(&frame, stateDragging ? 255 : stateOver ? 254 : 253);
}

void HudCtrl::onRemove()
{
   detach();

   Parent::onRemove();
}

void HudCtrl::detach()
{
   if(!isDeleted() && !isRemoved()) 
      deleteObject();
}

void HudCtrl::onWake()
{
   //position.x = fracPos.x * (parent->extent.x - extent.x);
   //position.y = fracPos.y * (parent->extent.y - extent.y);
   
   if (cg.player)
   {
      clearNotify(cg.player);
      deleteNotify(cg.player);
   }
}

bool HudCtrl::onSimActionEvent (const SimActionEvent *event)
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
      
bool HudCtrl::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
		onEvent (SimActionEvent);
		
		default:
			return Parent::processEvent (event);
	}
}

Persistent::Base::Error HudCtrl::write( StreamIO &sio, int a, int b)
{
   fracPos.write(sio);
   extent.write(sio);
   int output = isVisible();
   sio.write(output);
   return Parent::write(sio, a, b);
}

Persistent::Base::Error HudCtrl::read( StreamIO &sio, int a, int b)
{
   fracPos.read(sio);
   extent.read(sio);
   int input;
   sio.read(&input);
   if (input) 
   {
      setVisible(true);
   }
   else
   {
      setVisible(false);
   }
   return Parent::read(sio, a, b);
}

IMPLEMENT_PERSISTENT_TAG(HudCtrl, FOURCC('F','G','h','c'));

};