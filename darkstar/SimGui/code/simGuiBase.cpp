#include "simGuiBase.h"

namespace SimGui
{
Responder::Responder()
{
   nextResponder = NULL;
}

bool Responder::loseFirstResponder()
{
   return true;
}

bool Responder::becomeFirstResponder()
{
   return false;
}

void Responder::onMouseDown(const Event &event)
{
   if(nextResponder)
      nextResponder->onMouseDown(event);
}

void Responder::onMouseRepeat(const Event &event)
{
   if(nextResponder)
      nextResponder->onMouseRepeat(event);
}

void Responder::onMouseUp(const Event &event)
{
   if(nextResponder)
      nextResponder->onMouseUp(event);
}

void Responder::onMouseMove(const Event &event)
{
   if(nextResponder)
      nextResponder->onMouseMove(event);
}

void Responder::onMouseEnter(const Event &event)
{
   if(nextResponder)
      nextResponder->onMouseEnter(event);
}

void Responder::onMouseLeave(const Event &event)
{
   if(nextResponder)
      nextResponder->onMouseLeave(event);
}

void Responder::onMouseDragged(const Event &event)
{
   if(nextResponder)
      nextResponder->onMouseDragged(event);
}

void Responder::onRightMouseDown(const Event &event)
{
   if(nextResponder)
      nextResponder->onRightMouseDown(event);
}

void Responder::onRightMouseRepeat(const Event &event)
{
   if(nextResponder)
      nextResponder->onRightMouseRepeat(event);
}

void Responder::onRightMouseUp(const Event &event)
{
   if(nextResponder)
      nextResponder->onRightMouseUp(event);
}

void Responder::onRightMouseDragged(const Event &event)
{
   if(nextResponder)
      nextResponder->onRightMouseDragged(event);
}

void Responder::onKeyUp(const Event &event)
{
   if(nextResponder)
      nextResponder->onKeyUp(event);
}

void Responder::onKeyRepeat(const Event &event)
{
   if(nextResponder)
      nextResponder->onKeyRepeat(event);
}

void Responder::onKeyDown(const Event &event)
{
   if(nextResponder)
      nextResponder->onKeyDown(event);
}

DWORD Responder::onMessage(SimObject *sender, DWORD msg)
{
   if(nextResponder)
      return nextResponder->onMessage(sender, msg);
   return 0;
}

void Responder::setNextResponder(Responder *next)
{
   nextResponder = next;
}

Responder* Responder::getNextResponder()
{
   return nextResponder;
}

};