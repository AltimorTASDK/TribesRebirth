//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <g_surfac.h>
#include <inspect.h>
#include "simGuiSlider.h"
#include "editor.strings.h"   

namespace SimGui
{

//------------------------------------------------------------------------------
Slider::Slider()
{
   active = true;
   minVal = 0.0f;
   maxVal = 1.0f;
   curVal = 0.0f;
   lineVal = 0.05f;
   pageVal = 0.10f;

   dragginThumb = false;
   pageLeft = true;
   thumbWidth = 6;
}

//------------------------------------------------------------------------------
void Slider::setCurVal(float val)  
{
   curVal = val; 
   if (curVal < minVal) curVal = minVal;
   if (curVal > maxVal) curVal = maxVal; 
   setFloatVariable(curVal);
}

//------------------------------------------------------------------------------
void Slider::onKeyDown(const Event &event)
{
   setUpdate();
   if (onKeyLine(event))
      return;

   Parent::onKeyDown(event);
}   

//------------------------------------------------------------------------------
void Slider::onKeyRepeat(const Event &event)
{
   setUpdate();
   if (onKeyLine(event))
      return;

   Parent::onKeyRepeat(event);
} 

//------------------------------------------------------------------------------
bool Slider::onKeyLine(const Event &event)
{
   if (active) 
   {
      switch(event.diKeyCode)
      {
         case DIK_RIGHT:
            setCurVal(curVal + lineVal);
            setUpdate();
            return true;
         case DIK_LEFT:
            setCurVal(curVal - lineVal);            
            setUpdate();
            return true;
      }
   }
   return false;
}     

//------------------------------------------------------------------------------
void Slider::onMouseDown(const Event &event)
{
   if (active)
   {
      Point2I localPt = globalToLocalCoord(event.ptMouse);
      dragginThumb = inThumb(localPt);
      
      if (!dragginThumb)
      {
         pageLeft = (localPt.x < getThumbPos());
         onMousePage(event);
      }
   }
   setUpdate();
   Parent::onMouseDown(event);
}

//------------------------------------------------------------------------------
void Slider::onMouseUp(const Event &event)
{
   dragginThumb = false;
   setUpdate();
   Parent::onMouseUp(event);
}

//------------------------------------------------------------------------------
void Slider::onMouseRepeat(const Event &event)
{
   onMousePage(event);
   setUpdate();
   Parent::onMouseRepeat(event);
}   

//------------------------------------------------------------------------------
void Slider::onMousePage(const Event &event)
{
   if (dragginThumb)
      return;

   setUpdate();
   Point2I localPt = globalToLocalCoord(event.ptMouse);
   int thumbLeftSide = getThumbPos();
   int thumbRightSide = thumbLeftSide + thumbWidth;
   if ( (localPt.x < thumbLeftSide) && pageLeft)
   {
      setCurVal(curVal - pageVal*(maxVal - minVal));
   }
   else
   {
      if ((localPt.x > thumbRightSide) && !pageLeft)
         setCurVal(curVal + pageVal*(maxVal - minVal));
   }
}   

//------------------------------------------------------------------------------
Int32 Slider::getThumbPos()
{
   float trackLen = (float)(extent.x - thumbWidth);
   return (Int32)(((curVal - minVal) / (maxVal - minVal)) * trackLen);
}  

//------------------------------------------------------------------------------
void Slider::setThumbPos(Int32 localPt_x)
{
   float trackLen = (float)(extent.x - thumbWidth);
   float newCurVal = (localPt_x - (thumbWidth > 1)) / trackLen * (maxVal - minVal) + minVal;
   
   setCurVal(newCurVal);
}   

//------------------------------------------------------------------------------
Bool Slider::inThumb(Point2I pos)
{
   RectI thumbRect;
   thumbRect.upperL.set(getThumbPos(), 0);
   thumbRect.lowerR.set(thumbRect.upperL.x + thumbWidth , extent.y);
   if ( (pos.x >= thumbRect.upperL.x) && (pos.x <= thumbRect.lowerR.x) 
         && (pos.y >= thumbRect.upperL.y) && (pos.y <= thumbRect.lowerR.y) )
      return true;
   else
      return false;  
}   

//------------------------------------------------------------------------------
void Slider::onMouseDragged(const Event &event)
{
   if (!dragginThumb)
      return;

   if(root->getMouseLockedControl() == this)
   {
      Point2I localPt = globalToLocalCoord(event.ptMouse);
      setThumbPos(localPt.x);
   }

   setUpdate();
   Parent::onMouseDragged(event);
}  

//------------------------------------------------------------------------------
void Slider::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   Int32 halfThumbWidth = thumbWidth >> 1;
   Int32 halfHeight = extent.y >> 1;

   RectI track(halfThumbWidth, halfHeight, extent.x - halfThumbWidth, halfHeight);
   track.upperL += offset;
   track.lowerR += offset;
   sfc->drawLine2d(&track.upperL, &track.lowerR, 252);

   RectI thumbRect(offset.x + getThumbPos(), offset.y, thumbWidth + offset.x + getThumbPos(), extent.y + offset.y);
   sfc->drawRect2d_f(&thumbRect, (stateDepressed) ? 249 : 250);
}

//------------------------------------------------------------------------------
void Slider::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);

   insp->write(IDITG_MIN_VALUE, minVal);
   insp->write(IDITG_MAX_VALUE, maxVal);
}   

//------------------------------------------------------------------------------
void Slider::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   float newMinVal, newMaxVal;
   insp->read(IDITG_MIN_VALUE, newMinVal);
   insp->read(IDITG_MAX_VALUE, newMaxVal);

   if (newMinVal == newMaxVal)
      return;

   if (newMinVal < newMaxVal)
   {
      minVal = newMinVal;
      maxVal = newMaxVal;
   }
   else
   {
      minVal = newMaxVal;
      maxVal = newMinVal;
   }
}   

//------------------------------------------------------------------------------
Persistent::Base::Error Slider::write( StreamIO &sio, int version, int user )
{
   sio.write(minVal);
   sio.write(maxVal);
   sio.write(curVal);
   
   return Parent::write(sio, version, user);
}

//------------------------------------------------------------------------------
Persistent::Base::Error Slider::read( StreamIO &sio, int version, int user)
{
   sio.read(&minVal);
   sio.read(&maxVal);
   sio.read(&curVal);

   return Parent::read(sio, version, user);
}

};
