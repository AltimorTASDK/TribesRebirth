//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <string.h>
#include <g_surfac.h>
#include "g_bitmap.h"
#include "darkstar.strings.h"
#include "fear.strings.h"
#include "FearGuiRadio.h"
#include "simResource.h"

namespace FearGui
{

bool FearGuiRadio::becomeFirstResponder()
{
   return false;
}

bool FearGuiRadio::onAdd(void)
{
   if(!Parent::onAdd())
      return false;

   //load the .BMPs
   mOnBitmap = SimResource::get(manager)->load("i_bigfear_on.bmp");
   AssertFatal(mOnBitmap, "Unable to load bitmap");
   mOnBitmap->attribute |= BMA_TRANSPARENT;
   mOffBitmap = SimResource::get(manager)->load("i_bigfear_off.bmp");
   AssertFatal(mOffBitmap, "Unable to load bitmap");
   mOffBitmap->attribute |= BMA_TRANSPARENT;

   //define the extent - the crosshair doesn't get any events
   extent.x = mOnBitmap->getWidth();
   extent.y = mOnBitmap->getHeight();
   
   return true;
}
   
void FearGuiRadio::onAction()
{
   set = true;
   messageSiblings(IDSYS_SIBLING_DISABLE);
   messageSiblings(getMessage());   
   Parent::onAction();
}   

bool FearGuiRadio::onSimActionEvent (const SimActionEvent *event)
{
   SimActionEvent tempEvent(*event);
   tempEvent.type = event->type;
	switch(event->action)
   {
		case IDACTION_TOGGLE:
         if (set)
         {
            set = FALSE;
            tempEvent.action = IDACTION_TURN_OFF;
         }
         else
         {
            set = TRUE;
            onAction();
            tempEvent.action = IDACTION_TURN_ON;
         }
			break;
         
		case IDACTION_TURN_ON:
         set = TRUE;
			onAction();
			break;
		case IDACTION_TURN_OFF:
         set = FALSE;
			break;
   
   }
   
   return true;
}

bool FearGuiRadio::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
		onEvent (SimActionEvent);
      
      default:
         return Parent::processEvent(event);
	}
}

DWORD FearGuiRadio::onMessage(SimObject *sender, DWORD msg)
{
   if (msg == IDSYS_SIBLING_DISABLE)
      set = false;
   
   return Parent::onMessage(sender, msg);
}   

void FearGuiRadio::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   GFXBitmap *bmp = (set ? mOnBitmap : mOffBitmap);
   
   //draw in the button
   if (bmp) sfc->drawBitmap2d(bmp, &offset);
}

void FearGuiRadio::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   insp->write( IDSTR_AI_ACTIVE,            bool(set) );
   
}

void FearGuiRadio::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   bool Set;
   insp->read( IDSTR_AI_ACTIVE,            bool(Set) );
   set = Set;
}   

Persistent::Base::Error FearGuiRadio::write( StreamIO &sio, int version, int user )
{
   sio.write(set);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FearGuiRadio::read( StreamIO &sio, int version, int user)
{
   sio.read(&set);
   
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG(FearGuiRadio, FOURCC('F','G','r','b'));

};