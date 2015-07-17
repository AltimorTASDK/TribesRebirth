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
#include "simGuiActiveCtrl.h"
#include "g_surfac.h"
#include "inspect.h"
#include "editor.strings.h"
#include "console.h"

namespace SimGui
{

ActiveCtrl::ActiveCtrl()
{
   stateDepressed = false;
   stateOver = false;
   onSelectedCursorTag = 0;

   active = false;
   message = 0;
}

void ActiveCtrl::setMessage(DWORD msg)
{
   message = msg;
}

DWORD ActiveCtrl::getMessage(void)
{
   return message;
}

bool ActiveCtrl::inHitArea(const Point2I &pt)
{
   return ( (pt.x < extent.x) && (pt.y < extent.y) 
            && (pt.x >= 0) && (pt.y >= 0) );
}

void ActiveCtrl::onMouseMove(const Event &event)
{
   if(!active)
   {
      mouseCursorTag = 0;
      Parent::onMouseMove(event);
      return;
   }
   bool oldOver = stateOver;

   Point2I localPt = globalToLocalCoord(event.ptMouse);
   stateOver = inHitArea(localPt);
   if(oldOver != stateOver)
      setUpdate();
   mouseCursorTag = stateOver ? onSelectedCursorTag : 0;
}   

void ActiveCtrl::onAction()
{
   if(consoleCommand[0])
      CMDConsole::getLocked()->evaluate(consoleCommand, false);
   const char *name = getName();
   if(name)
   {
      char buf[256];
      sprintf(buf, "%s::onAction", name);
      if(Console->isFunction(buf))
         Console->executef(1, buf);
   }
   if(message)
      Parent::onMessage(this, message);
}

void ActiveCtrl::onMouseLeave(const Event &event)
{
   if (! active)
   {
      Parent::onMouseLeave(event);
      return;
   }
   stateOver = false;
   setUpdate();
}

void ActiveCtrl::onMouseDown(const Event &event)
{
   if (! active)
   {
      Parent::onMouseDown(event);
      return;
   }
   if(stateOver)
   {
      root->mouseLock(this); 
      root->makeFirstResponder(this);
      stateDepressed = true;
   }
   setUpdate();
}

void ActiveCtrl::onMouseUp(const Event &event)
{
   if (! active)
   {
      Parent::onMouseUp(event);
      return;
   }

   if(root->getMouseLockedControl() == this)
      root->mouseUnlock();

   setUpdate();
   if (stateDepressed)
   {
      stateDepressed = false;
      onAction();   
   }
}

void ActiveCtrl::onKeyDown(const Event &event)
{
   if ( (! active) || (event.diKeyCode != DIK_RETURN) )
   {
      Parent::onKeyDown(event);
      return;
   }

   if (root->getMouseLockedControl() != this)
      stateDepressed = true;
   setUpdate();
}

void ActiveCtrl::onKeyUp(const Event &event)
{
   if ( (! active) || (event.diKeyCode != DIK_RETURN) )
   {
      Parent::onKeyUp(event);
      return;
   }

   if (root->getMouseLockedControl() != this && stateDepressed)
   {
      stateDepressed = false;
      onAction();
   }
   setUpdate();
}

void ActiveCtrl::onMouseDragged(const Event &event)
{
   if (! active)
   {
      Parent::onMouseDragged(event);
   }

   if(root->getMouseLockedControl() == this)
   {
      bool oldDepressed = stateDepressed;
      Point2I localPt = globalToLocalCoord(event.ptMouse);
      stateDepressed = inHitArea(localPt);
      if(oldDepressed != stateDepressed)
         setUpdate();
   }
}

void ActiveCtrl::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);

   insp->write(IDITG_ACTIVE, active);
   insp->write(IDITG_MESSAGE_TAG, true, "*", (Int32)message);
}   

void ActiveCtrl::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   insp->read(IDITG_ACTIVE, active);

   Int32 temp;
   insp->read(IDITG_MESSAGE_TAG, NULL, NULL, temp);
   message = DWORD(temp);
}   

bool ActiveCtrl::wantsTabListMembership()
{
   return true;
}

bool ActiveCtrl::loseFirstResponder()
{
   setUpdate();
   return !stateDepressed;
}   

bool ActiveCtrl::becomeFirstResponder()
{
   setUpdate();
   return (active && isVisible());
}

Persistent::Base::Error ActiveCtrl::write( StreamIO &sio, int version, int user )
{
   sio.write(active);
   sio.write(message);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error ActiveCtrl::read( StreamIO &sio, int version, int user)
{
   sio.read(&active);
   sio.read(&message);

   return Parent::read(sio, version, user);
}

};