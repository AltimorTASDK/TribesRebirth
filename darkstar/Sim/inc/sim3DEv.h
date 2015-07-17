#ifndef _H_SIM3DEV
#define _H_SIM3DEV

#include "simEv.h"

struct Sim3DMouseEvent: public SimEvent
{
public:
   
   SimObject *sender;   // object that generated the event
   Point3F startPt;     // start point of mouse ray - usually on the projection plane
   Point3F direction;   // direction vector of ray from startPt
   enum { MouseDown, MouseUp, MouseMove, MouseDragged, 
          RightMouseDown, RightMouseUp, RightMouseDragged
   } meType;
   BYTE modifier;       // keyboard modifier

	Sim3DMouseEvent() { type = Sim3DMouseEventType; }
};

#endif
