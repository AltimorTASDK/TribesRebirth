#include "simGuiTextFormat.h"
#include "g_surfac.h"
#include "FearGuiHudCtrl.h"
#include "FearGuiChatDisplay.h"
#include "simResource.h"
#include "g_font.h"
#include "chatmenu.h"
#include "fearGuiShellPal.h"
#include "fear.strings.h"

namespace FearGui
{
   void drawBracket(GFXSurface *sfc, int x, int ty, int by, int dx);
};
extern char centerPrintBuffer[];
extern int centerPrintPos;

class CenterPrint : public SimGui::Control
{
   typedef SimGui::Control Parent;
   bool render;
public:
   SimGui::TextFormat tf;
   void onPreRender();
   bool onAdd();
   bool pointInControl(Point2I &pt);

   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   DECLARE_PERSISTENT(CenterPrint);
};

IMPLEMENT_PERSISTENT_TAG(CenterPrint, FOURCC('F','C','p','R'));

bool CenterPrint::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   //set the font flags
   Resource<GFXFont> font;
   font = SimResource::get(manager)->load("sf_orange_6.pft");
   if (bool(font)) font->fi.flags |= FONT_LOWERCAPS;
   font = SimResource::get(manager)->load("sf_yellow_6.pft");
   if (bool(font)) font->fi.flags |= FONT_LOWERCAPS;
   font = SimResource::get(manager)->load("sf_white_6.pft");
   if (bool(font)) font->fi.flags |= FONT_LOWERCAPS;

   render = false;
   return true;
}

bool CenterPrint::pointInControl(Point2I &)
{
   return false;
}

void CenterPrint::onPreRender()
{
   render = centerPrintBuffer[0] != 0 && centerPrintPos >= 0 && centerPrintPos <= 2;

   if(render)
   {
      Point2I parentExtent = parent->getExtent();
      bool lowRes = FALSE;
      if (parentExtent.x < 512 || parentExtent.y < 384) lowRes = TRUE;
      if (! lowRes)
      {
         tf.setFont(0, "sf_orange214_10.pft");
         tf.setFont(1, "sf_orange255_10.pft");
         tf.setFont(2, "sf_white_9b.pft");
         parentExtent.x -= 70;
      }
      else
      {
         tf.setFont(0, "sf_orange_6.pft");
         tf.setFont(1, "sf_yellow_6.pft");
         tf.setFont(2, "sf_white_6.pft");
         parentExtent.x -= 30;
      }
      
      tf.formatControlString(centerPrintBuffer, parentExtent.x - 4, false, true);
      int height = tf.getHeight() + 8;

      int posOffset = (! lowRes ? 35 : 15);
      Point2I pos;
      Point2I extent(parentExtent.x, height);
      if(centerPrintPos == 0)
         pos.set(posOffset, (parentExtent.y - height) >> 1);
      else if(centerPrintPos == 1)
         pos.set(posOffset, parentExtent.y - height - posOffset);
      else
         pos.set(posOffset, posOffset);
      resize(pos, extent);
   }
}

void CenterPrint::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   if(!render)
      return;

   Point2I topLeft = offset;
   Point2I bottomRight = offset + extent;

   Point3F poly[4];
   for (int j = 0; j < 4; j++)
      poly[j].z = 1.0f;
   //DRAW THE TRANSPARENT BACKGROUND
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
      sfc->addVertex(poly + i);

   sfc->emitPoly();
   sfc->setAlphaSource(GFX_ALPHA_NONE);
   FearGui::drawBracket(sfc, topLeft.x, topLeft.y, bottomRight.y - 1, 4);
   FearGui::drawBracket(sfc, bottomRight.x - 1, topLeft.y, bottomRight.y - 1, -4);
   tf.onRender(sfc, offset + Point2I(2,2), updateRect);
}