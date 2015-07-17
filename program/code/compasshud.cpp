#include "fearGuiHudCtrl.h"
#include "Player.h"

#include "g_surfac.h"
#include "g_bitmap.h"
#include "fearGlobals.h"
#include "g_font.h"
#include "simResource.h"
#include "fearGuiShellPal.h"
#include "fearPlayerPsc.h"
#include "ts_pointArray.h"

namespace FearGui
{

class CompassHud : public HudCtrl
{
   typedef HudCtrl Parent;
   
   Resource<GFXFont> textFont;
   Resource<GFXFont> lrTextFont;
   
   Resource<GFXBitmap> mBorderBmp;
   Resource<GFXBitmap> lrBorderBmp;
   
   bool mbLowRes;
   
public:
   bool onAdd(void);
   void onRender(GFXSurface *, Point2I, const Box2I &);

   DECLARE_PERSISTENT(CompassHud);
};

static float gCompassPoints[] =
{
   //'N'
   4.0f,     -2.0f, -18.0f,     -2.0f, -24.0f,      2.0f, -18.0f,     2.0f, -24.0f,
   
   //'E'
   4.0f,     18.0f,   2.0f,     18.0f,  -2.0f,     24.0f,  -2.0f,    24.0f,   2.0f,
   2.0f,     21.0f,  -2.0f,     21.0f,   2.0f,
   
   //'W'
   5.0f,    -24.0f,   4.0f,    -18.0f,  2.0f,     -24.0f,   0.0f,    -18.0f, -2.0f,   -24.0f, -4.0f,
   
   //'S'
   12.0f,     2.0f,  20.0f,      1.0f,  19.0f,     -1.0f,  19.0f,     -2.0f, 20.0f,    -2.0f, 21.0f,     -1.0f, 22.0f,
              1.0f,  22.0f,      2.0f,  23.0f,      2.0f,  24.0f,      1.0f, 25.0f,    -1.0f, 25.0f,     -2.0f, 24.0f,
   
   //4 marks
   2.0f,     15.0f, -14.0f,     19.0f, -18.0f,
   2.0f,     15.0f,  14.0f,     19.0f,  18.0f,
   2.0f,    -14.0f,  14.0f,    -18.0f,  18.0f,
   2.0f,    -14.0f, -14.0f,    -18.0f, -18.0f,
      
   //EOF
   -1.0f
};

static float lrCompassPoints[] =
{
   //'N'
   4.0f,     -1.5f, -10.5f,     -1.5f, -13.5f,      1.5f, -10.5f,     1.5f, -13.5f,
   
   //'E'
   4.0f,     9.5f,    0.5f,      9.5f,  -1.5f,     13.5f,  -1.5f,    13.5f,   0.5f,
   2.0f,     11.5f,  -1.5f,     11.5f,  -0.5f,
   
   //'W'
   7.0f,    -13.5f,   2.5f,    -11.5f,   2.5f,    -10.5f,   1.5f,   -11.5f,   1.5f,    -10.5f,  -0.5f,   -11.5f,  -1.5f,   -13.5f,  -1.5f,
   
   //'S'
   6.0f,      1.5f,   9.5f,      0.5f,   9.5f,     -0.5f,  10.5f,     1.5f,  12.5f,      0.5f,  13.5f,    -0.5f,  13.5f,
   
   //4 marks
   2.0f,      7.5f,  -7.5f,      9.5f,  -9.5f,
   2.0f,      7.5f,   7.5f,      9.5f,   9.5f,
   2.0f,     -7.5f,   7.5f,     -9.5f,   9.5f,
   2.0f,     -7.5f,  -7.5f,     -9.5f,  -9.5f,
      
   //EOF
   -1.0f 
};

bool CompassHud::onAdd()
{
   if (! Parent::onAdd())
      return false;
   
   //load the font   
   textFont = SimResource::get(manager)->load("if_g_8.pft");
   lrTextFont = SimResource::get(manager)->load("sf_green_5.pft");
   
   //load the bitmaps
   mBorderBmp = SimResource::get(manager)->load("Compass.bmp");
   AssertFatal(mBorderBmp, "Unable to load Compass.bmp");
   
   lrBorderBmp = SimResource::get(manager)->load("lr_Compass.bmp");
   AssertFatal(lrBorderBmp, "Unable to load lr_Compass.bmp");
   
   mbLowRes = FALSE;

   //define the extent
   extent.x = mBorderBmp->getWidth();
   extent.y = mBorderBmp->getHeight();
   snap();

   return true;
}


static Point2F tex[4];

void CompassHud::onRender(GFXSurface *sfc, Point2I offset, const Box2I& /*updateRect*/)
{
   Point2I canvasSize(sfc->getWidth(), sfc->getHeight());
   
   //check for lowres
   if (canvasSize.x < 512 || canvasSize.y < 384)
   {
      if (! mbLowRes)
      {
         mbLowRes = TRUE;
         extent.set((mBorderBmp->getWidth() * 6 / 10) + 2, (mBorderBmp->getHeight() * 6 / 10) + 2);
         snap();
         return;
      }
   }
   else
   {
      if (mbLowRes)
      {
         mbLowRes = FALSE;
         extent.set(mBorderBmp->getWidth() + 3, mBorderBmp->getHeight() + 3);
         snap();
         return;
      }
   }
   
   //make sure the HUD is visible
   if ((canvasSize.x < extent.x) || (canvasSize.y < extent.y))
      return;
   
   if(!cg.psc) return;
   
	GameBase* cp = cg.psc->getControlObject();
	if (!cp)
		return;
   Vector3F playerDirection = cp->getCompassRotation();

   //draw the background
   sfc->drawBitmap2d((! mbLowRes ? mBorderBmp : lrBorderBmp), &offset);
   
   Point2F centerPt;
   if (! mbLowRes)
   {
      centerPt.x = 31.5f + float(offset.x);
      centerPt.y = 31.5f + float(offset.y);
   }
   else
   {
      centerPt.x = 18.5f + float(offset.x);
      centerPt.y = 18.5f + float(offset.y);
   }
   
   //calculate the degree range where the middle range is the players
   
   //rotate the needle around the compass
   float cosTheta, sinTheta;
   m_sincos(playerDirection.z, &sinTheta, &cosTheta);

   //loop through and rotate the compass markings
   Point2F markPoints[12];
   float *points;
   if (! mbLowRes) points = &gCompassPoints[0];
   else points = &lrCompassPoints[0];
   while (*points > 0.0f)
   {
      int j;
      for (j = 0; j < *points; j++)
      {
         markPoints[j].x = (points[2 * j + 1] * cosTheta) - (points[2 * j + 2] * sinTheta);
         markPoints[j].y = (points[2 * j + 1] * sinTheta) + (points[2 * j + 2] * cosTheta);
      }
      
      //now draw the lines
      for (j = 0; j < *points - 1; j++)
      {
         Point2I tempStart;
         tempStart.x = int(markPoints[j].x + centerPt.x);
         tempStart.y = int(markPoints[j].y + centerPt.y);
         Point2I tempEnd;
         tempEnd.x = int(markPoints[j + 1].x + centerPt.x);
         tempEnd.y = int(markPoints[j + 1].y + centerPt.y);
         sfc->drawLine2d(&tempStart, &tempEnd, GREEN);
      }
      
      //now increment the points pointer
      points += (int(*points) * 2) + 1;
   }
   
   //now draw the red mark
   if (! mbLowRes)
   {
      sfc->drawLine2d(&Point2I(centerPt.x, centerPt.y - 14), &Point2I(centerPt.x, centerPt.y - 29), RED);
   }
   else
   {
      sfc->drawLine2d(&Point2I(centerPt.x, centerPt.y - 8), &Point2I(centerPt.x, centerPt.y - 15), RED);
   }

   //calculate the wayPoint mark
   Point3F poly[3];
   Point3F myPosition = cp->getLinearPosition();
   Point2F mp(myPosition.x, myPosition.y);
        
   //if we're already at the target, don't bother with the waypoint mark
   if (cg.curCommand != -1 && ((abs(myPosition.x - cg.wayPoint.x) > 2) ||
                              (abs(myPosition.y - cg.wayPoint.y) > 2)))
   {
      //first find the distance to target
      int dist = m_distf(mp, cg.wayPoint);
      RealF wayPointDirection;

      if (int(myPosition.x - cg.wayPoint.x) == 0)
      {
         if (myPosition.y > cg.wayPoint.y) wayPointDirection = -M_PI / 2.0f;
         else wayPointDirection = M_PI / 2.0f;
      }
      else
      {
         wayPointDirection = m_atan(cg.wayPoint.x - myPosition.x,
                                  cg.wayPoint.y - myPosition.y);
      }
      wayPointDirection = wayPointDirection - playerDirection.z;
      
      //rotate a triangle to point 
      m_sincos((M_PI / 2.0f) - wayPointDirection, &sinTheta, &cosTheta);
      float triangleX = 6.0f;
      float triangleY1 = -24.0f; 
      float triangleY2 = -16.0f; 
      if (mbLowRes)
      {
         triangleX = triangleX * 0.6f;
         triangleY1 = triangleY1 * 0.6f;
         triangleY2 = triangleY2 * 0.6f;
      }
      poly[0].x = ( 0.0f * cosTheta) - (triangleY1 * sinTheta);
      poly[0].y = ( 0.0f * sinTheta) + (triangleY1 * cosTheta);
      poly[1].x = ( triangleX * cosTheta) - (triangleY2 * sinTheta);
      poly[1].y = ( triangleX * sinTheta) + (triangleY2 * cosTheta);
      poly[2].x = (-triangleX * cosTheta) - (triangleY2 * sinTheta);
      poly[2].y = (-triangleX * sinTheta) + (triangleY2 * cosTheta);
      
      int i;
      for (i = 0; i < 3; i++)
      {
         poly[i].x += float(centerPt.x);
         poly[i].y += float(centerPt.y);
         poly[i].z  = 1.0f;
      }
      
      //draw the arrow interior
      sfc->setFillMode(GFX_FILL_CONSTANT);
      sfc->setShadeSource(GFX_SHADE_NONE);
      sfc->setHazeSource(GFX_HAZE_NONE);
      sfc->setAlphaSource(GFX_ALPHA_NONE);
      sfc->setTransparency (FALSE);
      sfc->setFillColor(GREEN);
      for (i = 0; i < 3; i++)
      {
         sfc->addVertex(poly + i);
      }
      sfc->emitPoly();
      
      //print out the distance
      GFXFont *font = (! mbLowRes ? textFont : lrTextFont);
      char distStr[12];
      sprintf(distStr, "%d", min(999, dist));
      sfc->drawText_p(font, &Point2I(centerPt.x + 1 - (font->getStrWidth(distStr) / 2),
                                          centerPt.y - font->getHeight() + 3), distStr);
        
   }
   else
   {
      GFXFont *font = (! mbLowRes ? textFont : lrTextFont);
      char distStr[12];
      sprintf(distStr, "000");
      sfc->drawText_p(font, &Point2I(centerPt.x + 1 - (font->getStrWidth(distStr) / 2),
                                          centerPt.y - font->getHeight() + 3), distStr);
   }
}

IMPLEMENT_PERSISTENT(CompassHud);

};