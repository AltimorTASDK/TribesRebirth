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
#include "simGuiTestButton.h"

namespace SimGui
{

TestButton::TestButton()
{
   active = true;
   extent.set(140, 30);
   alignment = CenterJustify;
}

void TestButton::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   GFXFont *useFont = (stateOver) ? hFontHL : hFont;
   Int32 backColor = 249;
   Int32 insideBorderColor = (root->getFirstResponder() == this) ? 245 : 255; 
   Int32 outsideBorderColor = 249;

   if (useFont)
   {
      Int32 txt_w = useFont->getStrWidth(text);

      Point2I localStart;
      // align the horizontal
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

      // center the vertical
      localStart.y = (extent.y - useFont->getHeight()) / 2;

      // first draw the background
      RectI r(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1);
      r.upperL+=2;
      r.lowerR-=2;
      sfc->drawRect2d_f(&r, backColor);

      // draw the inside border
      r.upperL-=1; 
      r.lowerR+=1;
      drawBorder(sfc, r, insideBorderColor);

      // draw the oustside border
      r.upperL-=1;
      r.lowerR+=1;
      drawBorder(sfc, r, outsideBorderColor);

      // finally draw the text
      localStart.y += textVPosDelta;
      if (stateDepressed)
         localStart+=1;
      Point2I globalStart = localToGlobalCoord(localStart);
      sfc->drawText_p(useFont, &globalStart, text);
   }
   renderChildControls(sfc, offset, updateRect);
}

void TestButton::drawBorder(GFXSurface *sfc, RectI &r, int color)
{
   Point2I p1, p2;

   p1.x = r.upperL.x + 2;
   p2.x = r.lowerR.x - 2;
   p1.y = p2.y = r.upperL.y;
   sfc->drawLine2d( &p1, &p2, color);     // top  
   p1.y = p2.y = r.lowerR.y;
   sfc->drawLine2d( &p1, &p2, color);     // bottom

   p1.y = r.upperL.y + 2;
   p2.y = r.lowerR.y - 2;
   p1.x = p2.x = r.upperL.x;
   sfc->drawLine2d( &p1, &p2, color);     // left
   p1.x = p2.x = r.lowerR.x;
   sfc->drawLine2d( &p1, &p2, color);     // right

   p1.x = p2.x = r.upperL.x + 1;
   p1.y = p2.y = r.upperL.y + 1;
   sfc->drawLine2d( &p1, &p2, color);     // left top dot
   p1.x = p2.x = r.lowerR.x - 1;
   sfc->drawLine2d( &p1, &p2, color);     // right top dot

   p1.x = p2.x = r.upperL.x + 1;
   p1.y = p2.y = r.lowerR.y - 1;
   sfc->drawLine2d( &p1, &p2, color);     // left bottom dot
   p1.x = p2.x = r.lowerR.x - 1;
   sfc->drawLine2d( &p1, &p2, color);     // right bottom dot
}   



};