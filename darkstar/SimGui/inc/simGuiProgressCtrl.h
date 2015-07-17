//------------------------------------------------------------------------------
// Description 
//    Simple progress control
//    
// $Workfile$ SimGuiProgressCtrl.h
// $Revision$ 1.0
// $Author  $ Robert Mobbs (robert.mobbs@dynamix.com)
// $Modtime $ 5/15/98
//
//------------------------------------------------------------------------------

#ifndef _H_SIMGUIPROGRESSCTRL
#define _H_SIMGUIPROGRESSCTRL

#include <simguiactivectrl.h>

#define PROGRESSCTRL_MAX_RANGE 2000
#define PROGRESSCTRL_MIN_RANGE    0

namespace SimGui
{
   class ProgressCtrl : public ActiveCtrl
   {
      private   :
         int iRangeHi, iRangeLo;
         int iPos, iIncrement;

         typedef ActiveCtrl Parent;

      protected :
      public    :
         ProgressCtrl();

         // Progress bar interactive controls
         void setStep(int);
         void setRange(int, int);
         int  stepIt();
         int  offsetPos(int);

			void setPercentDone(float percentDone);  //a float between 0.0f and 1.0f

         void onRender(GFXSurface *, Point2I, const Box2I &);
  
         void inspectRead (Inspect *);
         void inspectWrite(Inspect *);

         DECLARE_PERSISTENT(ProgressCtrl);

         Persistent::Base::Error read (StreamIO &, int, int);
         Persistent::Base::Error write(StreamIO &, int, int);

			//script function method
		   virtual void setScriptValue(const char *value);
   };   
};

#endif //_H_SIMGUIPROGRESSCTRL
