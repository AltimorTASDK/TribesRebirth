//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUITESTCHECK_H_
#define _SIMGUITESTCHECK_H_

#include "simGuiSimpleText.h"

namespace SimGui
{

class TestCheck : public SimpleText
{
private:
   typedef SimpleText Parent;

   bool set;

public:   
   TestCheck();

   void  onAction();
   void  onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

   const char *getScriptValue();
   void setScriptValue(const char *value);
   virtual void variableChanged(const char *newValue);

   DECLARE_PERSISTENT(TestCheck);
   Persistent::Base::Error write( StreamIO &sio, int version, int user);
   Persistent::Base::Error read( StreamIO &sio, int version, int user);
};

inline TestCheck::TestCheck()
{
   active = true;
   set = false;
}

};

#endif //_SIMGUITESTCHECK_H_
