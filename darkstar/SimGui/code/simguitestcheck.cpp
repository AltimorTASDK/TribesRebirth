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
#include "simGuiTestCheck.h"
#include "Console.h"

namespace SimGui
{

const char *TestCheck::getScriptValue()
{
   return set ? "True" : "False";
}

void TestCheck::setScriptValue(const char *val)
{
   set = CMDConsole::getBool(val);
   setIntVariable(set);
}

void TestCheck::variableChanged(const char *val)
{
   set = CMDConsole::getBool(val);
}

void TestCheck::onAction()
{
   set = !set;
   const char *name = getName();
   setIntVariable(set);
   if(name)
   {
      char buf[256];
      sprintf(buf, "%s::onAction", name);
      if(Console->isFunction(buf))
         Console->executef(1, buf);
   }
   if(consoleCommand[0])
      CMDConsole::getLocked()->evaluate(consoleCommand, false);
}   

void TestCheck::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   GFXFont *useFont = (stateOver) ? hFontHL : hFont;
   Int32 knobColor = selectFillColor;
   Int32 backColor = fillColor;

   if (useFont)
   {
      // get dimensions of the radial knob
      RectI OutBoxBox(0, 0, 10, 10);
      RectI InBoxBox(1, 1, 9, 9);
         
      // get draw position for text with consideration for
      // the radial knob and alignment
      Int32 txt_w = useFont->getStrWidth(text) + OutBoxBox.lowerR.x + 5;
      Point2I localStart;
      switch (alignment)
      {
         case RightJustify:
            localStart.x = extent.x - txt_w;  
            break;
         case CenterJustify:
            localStart.x = (extent.x - txt_w) / 2;
            break;
         default:
            // LeftJustify
            localStart.x = 0;
      }

      // align the radialKnob
      Point2I knobOffset = offset;
      knobOffset.x += localStart.x;
      knobOffset.y += (extent.y - OutBoxBox.lowerR.y) / 2;
      
      // align the text
      localStart.x += OutBoxBox.lowerR.x + 5;
      localStart.y = (extent.y - useFont->getHeight()) / 2;
      Point2I globalStart = localToGlobalCoord(localStart);

      // first draw the background
      RectI r(offset.x, offset.y, offset.x + extent.x, offset.y + extent.y);
      sfc->drawRect2d_f(&r, backColor);

      // now draw the border if we are the firstResponder
      // to designate focus
      if (root->getFirstResponder() == this) 
      {
         r.upperL.x++; r.upperL.y++;       
         r.lowerR.x--; r.lowerR.y--;
         sfc->drawRect2d(&r, knobColor);
      }

      // draw the text
      globalStart.y += textVPosDelta;
      sfc->drawText_p(useFont, &globalStart, text);

      // finally draw the knob
      OutBoxBox += knobOffset;
      InBoxBox += knobOffset;
      sfc->drawRect2d(&OutBoxBox, knobColor);
      if(stateDepressed)
         sfc->drawRect2d(&InBoxBox, knobColor);
      if(set)
      {
         sfc->drawLine2d(&OutBoxBox.upperL, &OutBoxBox.lowerR, knobColor);
         sfc->drawLine2d(&Point2I(OutBoxBox.upperL.x, OutBoxBox.lowerR.y), 
                         &Point2I(OutBoxBox.lowerR.x, OutBoxBox.upperL.y),
                         knobColor);
      }
   }
   renderChildControls(sfc, offset, updateRect);
}

Persistent::Base::Error TestCheck::write( StreamIO &sio, int version, int user )
{
   sio.write(set);

   return Parent::write(sio, version, user);
}

Persistent::Base::Error TestCheck::read( StreamIO &sio, int version, int user)
{
   sio.read(&set);

   return Parent::read(sio, version, user);
}

};