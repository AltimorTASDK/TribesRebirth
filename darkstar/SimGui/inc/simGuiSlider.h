//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUISLIDER_H_
#define _SIMGUISLIDER_H_

#include "simGuiActiveCtrl.h"

namespace SimGui
{

class Slider : public ActiveCtrl
{
private:
   typedef ActiveCtrl Parent;

protected:
   float minVal, maxVal, curVal, lineVal, pageVal;
   Int32 thumbWidth;
   bool dragginThumb;
   bool pageLeft;

   Int32 getThumbPos();
   void setThumbPos(Int32 localPt_x);
   virtual Bool inThumb(Point2I pos);

public:
   Slider();

   void setMinVal(float val)  { if (val < maxVal) minVal = val; setCurVal(curVal); }
   void setMaxVal(float val)  { if (val > minVal) maxVal = val; setCurVal(curVal); }
   void setPageVal(float val) { pageVal = val; }
   void setLineVal(float val) { lineVal = val; }
   void setCurVal(float val);
   float getCurVal()          {  return curVal; }
   
   void onKeyDown(const Event &event);
   void onKeyRepeat(const Event &event);
   bool onKeyLine(const Event &event);
   
   void onMouseDown(const Event &event);
   void onMouseUp(const Event &event);
   void onMouseRepeat(const Event &event);
   void onMousePage(const Event &event);
   
   void onMouseDragged(const Event &event);

   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   
   void inspectRead(Inspect *);
   void inspectWrite(Inspect *);

   DECLARE_PERSISTENT(Slider);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};   

};

#endif //_SIMGUISLIDER_H_
