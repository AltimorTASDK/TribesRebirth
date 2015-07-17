#ifndef _H_SIMGUIBASE
#define _H_SIMGUIBASE

#include <simBase.h>
#include <tVector.h>

#include "ml.h"

class SimCanvas;
class SimInputEvent;

namespace SimGui
{

struct Event : public SimEvent
{
   Event() { type = SimGuiEventType; }

   DWORD time;
   Point2I ptMouse;
   BYTE diKeyCode;
   BYTE ascii;
   BYTE modifier;
   BYTE mouseDownCount;
};

class DLLAPI Responder
{
private:
   Responder *nextResponder;
public:
   Responder();

   virtual void onMouseDown(const Event &event);
   virtual void onMouseRepeat(const Event &event);
   virtual void onMouseUp(const Event &event);
   virtual void onMouseMove(const Event &event);
   virtual void onMouseEnter(const Event &event);
   virtual void onMouseLeave(const Event &event);
   virtual void onMouseDragged(const Event &event);

   virtual void onRightMouseDown(const Event &event);
   virtual void onRightMouseRepeat(const Event &event);
   virtual void onRightMouseUp(const Event &event);
   virtual void onRightMouseDragged(const Event &event);

   virtual void onKeyUp(const Event &event);
   virtual void onKeyRepeat(const Event &event);
   virtual void onKeyDown(const Event &event);
   virtual DWORD onMessage(SimObject *sender, DWORD msg);
   virtual bool loseFirstResponder();
   virtual bool becomeFirstResponder();
   void setNextResponder(Responder *next);
   Responder* getNextResponder();
};


};
#endif
