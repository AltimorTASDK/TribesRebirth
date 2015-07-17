
#include "g_surfac.h"
#include "g_bitmap.h"
#include "g_font.h"
#include "simResource.h"
#include "fearGuiHudCtrl.h"
#include "fearGuiShellPal.h"
#include "fearglobals.h"

#define PixPerDegree 1

namespace FearGui
{

class ClockHud : public HudCtrl
{
   typedef HudCtrl Parent;
   
   Resource<GFXFont> textFont;
   Resource<GFXFont> lowResFont;

   bool mLowRes;
   Point2I hrExtent;
   Point2I lrExtent;
   
public:
   bool onAdd(void);
   void onRender(GFXSurface *, Point2I, const Box2I &);

   DECLARE_PERSISTENT(ClockHud);
};

bool ClockHud::onAdd(void)
{
   if (! Parent::onAdd())
      return false;
      
   textFont = SimResource::get(manager)->load("if_w_10.pft");
   lowResFont = SimResource::get(manager)->load("sf_white_6.pft");

   //define the extent
   mLowRes = FALSE;
   extent.x = textFont->getStrWidth("88:88:88.8") + 12;
   extent.y = textFont->getHeight() + 6;
   
   hrExtent = extent;
   lrExtent.x = lowResFont->getStrWidth("88:88:88.8") + 8; 
   lrExtent.y = lowResFont->getHeight() + 4; 
   
   snap();
   
   return true;
}
extern void drawBracket(GFXSurface *sfc, int x, int ty, int by, int dx);

void ClockHud::onRender(GFXSurface *sfc, Point2I offset, const Box2I& /*updateRect*/)
{
   if ((! mLowRes) && (parent->extent.x < 512 || parent->extent.y < 384))
   {
      mLowRes = TRUE;
      extent = lrExtent;
   }
   else if (mLowRes && (parent->extent.x >= 512 && parent->extent.y >= 384))
   {
      mLowRes = FALSE;
      extent = hrExtent;
      snap();
      return;
   }
   
   //make sure the HUD can fit on the canvas
   if (parent && ((parent->extent.x < extent.x) || (parent->extent.y < extent.y)))
      return;
      
   //draw the bounding rect
   Point2I bottomRight, topLeft;
   topLeft.x = offset.x;
   topLeft.y = offset.y;
   bottomRight.x = offset.x + extent.x;
   bottomRight.y = offset.y + extent.y;
   
   //DRAW THE TRANSPARENT BACKGROUND
   //initialize our vertices
   Point3F poly[8];
   for (int j = 0; j < 8; j++)
      poly[j].z = 1.0f;
   
   GFXFont *font = (! mLowRes ? textFont : lowResFont);
   
   //set the vertices
   poly[0].x = topLeft.x;
   poly[0].y = topLeft.y;
   poly[1].x = bottomRight.x;
   poly[1].y = topLeft.y;
   poly[2].x = bottomRight.x;
   poly[2].y = bottomRight.y;
   poly[3].x = topLeft.x;
   poly[3].y = bottomRight.y;
   
   //draw the translucent box
   sfc->setFillMode(GFX_FILL_CONSTANT);
   sfc->setShadeSource(GFX_SHADE_NONE);
   sfc->setHazeSource(GFX_HAZE_NONE);
   sfc->setAlphaSource(GFX_ALPHA_FILL);
   sfc->setTransparency (false);
   sfc->setFillColor(254, ALPHAPAL_INDEX);

   for (int i = 0; i < 4; i++)
   {
      sfc->addVertex(poly + i);
   }
   sfc->emitPoly();
   sfc->setAlphaSource(GFX_ALPHA_NONE);
   
   drawBracket(sfc, topLeft.x, topLeft.y, bottomRight.y-1, 2);
   drawBracket(sfc, bottomRight.x-1, topLeft.y, bottomRight.y-1, -2);
      
   float curTime = cg.clockTime;
   if(curTime < 0)
      curTime = -curTime;
   int secondsLeft, hours, mins, secs, tenths;
   secondsLeft = (int)curTime;
   hours = secondsLeft / 3600;
   secondsLeft -= hours * 3600;
   mins = secondsLeft / 60; 
   secondsLeft -= mins * 60;
   secs = secondsLeft;
   tenths = (int)((curTime - (int)curTime) * 10);
   
   char buf[256];
   sprintf(buf, "%02d:%02d:%02d.%1d", hours, mins, secs, tenths);
   if (mLowRes) topLeft.y += 1;
   sfc->drawText_p(font, &Point2I(topLeft.x + 4, topLeft.y), buf);
}

IMPLEMENT_PERSISTENT(ClockHud);

};