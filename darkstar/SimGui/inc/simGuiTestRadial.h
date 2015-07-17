//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUITESTRADIAL_H_
#define _SIMGUITESTRADIAL_H_

#include "simGuiSimpleText.h"

namespace SimGui
{

class TestRadial : public SimpleText
{
private:
   typedef SimpleText Parent;

   bool set;

public:   
   TestRadial();

   void  onAction();
   DWORD onMessage(SimObject *sender, DWORD msg);
   void  onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

   const char *getScriptValue();
   void setScriptValue(const char *value);
   virtual void variableChanged(const char *newValue);

   DECLARE_PERSISTENT(TestRadial);
   Persistent::Base::Error TestRadial::write( StreamIO &sio, int version, int user);
   Persistent::Base::Error TestRadial::read( StreamIO &sio, int version, int user);
};

inline TestRadial::TestRadial()
{
   active = true;
   set = false;
}

};

#endif //_SIMGUITESTRADIAL_H_
