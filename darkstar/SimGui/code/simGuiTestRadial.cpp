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
#include "simGuiTestRadial.h"
#include "darkstar.strings.h"
#include "Console.h"

namespace SimGui
{

const char *TestRadial::getScriptValue()
{
   return set ? "True" : "False";
}

void TestRadial::setScriptValue(const char *val)
{
   if(CMDConsole::getBool(val) && !set)
   {
      set = true;
      messageSiblings(IDSYS_SIBLING_DISABLE);
   }
   else
      set = false;
   setIntVariable(set);
}

void TestRadial::variableChanged(const char *val)
{
   set = CMDConsole::getBool(val);
   if(set)
      messageSiblings(IDSYS_SIBLING_DISABLE);
}

void TestRadial::onAction()
{
   set = true;
   messageSiblings(IDSYS_SIBLING_DISABLE);
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

DWORD TestRadial::onMessage(SimObject *, DWORD msg)
{
   if (msg == IDSYS_SIBLING_DISABLE)
   {
      set = false;
      setIntVariable(set);
   }
   
   return msg;
}   

void TestRadial::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   GFXFont *useFont = (stateOver) ? hFontHL : hFont;
   Int32 knobColor = selectFillColor;
   Int32 backColor = fillColor;

   if (useFont)
   {
      // get dimensions of the radial knob
      RectI OutCircleBox(0, 0, 10, 10);
      RectI InCircleBox(1, 1, 9, 9);
      RectI DotBox(4, 4, 6, 6);
         
      // get draw position for text with consideration for
      // the radial knob and alignment
      Int32 txt_w = useFont->getStrWidth(text) + OutCircleBox.lowerR.x + 5;
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
      knobOffset.y += (extent.y - OutCircleBox.lowerR.y) / 2;
      
      // align the text
      localStart.x += OutCircleBox.lowerR.x + 5;
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
      OutCircleBox += knobOffset;
      InCircleBox += knobOffset;
      DotBox += knobOffset;
      
      sfc->drawRect2d(&OutCircleBox, knobColor);
      if(set)
         sfc->drawRect2d(&DotBox, knobColor);
      if (stateDepressed)
         sfc->drawRect2d(&InCircleBox, knobColor);
   }
   renderChildControls(sfc, offset, updateRect);
}

Persistent::Base::Error TestRadial::write( StreamIO &sio, int version, int user )
{
   sio.write(set);

   return Parent::write(sio, version, user);
}

Persistent::Base::Error TestRadial::read( StreamIO &sio, int version, int user)
{
   sio.read(&set);

   return Parent::read(sio, version, user);
}

};