#include "fearGuiHudCtrl.h"
#include "Player.h"
#include "FearDcl.h"
#include "FearPlayerPSC.h"
#include "fearGlobals.h"

#include "g_surfac.h"
#include "g_bitmap.h"
#include "simResource.h"
#include "fearGuiShellPal.h"

namespace FearGui
{

class JetPackHud : public HudCtrl
{
   typedef HudCtrl Parent;
   
   Resource<GFXBitmap> mBorderBmp;
   Resource<GFXBitmap> mIconGreenBmp;
   Resource<GFXBitmap> mEndGreenBmp;

   Resource<GFXBitmap> lrBorderBmp;
   Resource<GFXBitmap> lrIconGreenBmp;
   Resource<GFXBitmap> lrEndGreenBmp;

   bool mbLowRes;
   
public:
   bool onAdd(void);
   void onRender(GFXSurface *, Point2I, const Box2I &);

   DECLARE_PERSISTENT(JetPackHud);
};

bool JetPackHud::onAdd()
{
   if (! Parent::onAdd())
      return false;
    
   //load the bitmaps
   mBorderBmp = SimResource::get(manager)->load("hudTrans.bmp");
   AssertFatal(mBorderBmp, "Unable to load HH_Border.bmp");
   
   mIconGreenBmp = SimResource::get(manager)->load("HJ_Icon_Green.bmp");
   AssertFatal(mIconGreenBmp, "Unable to load HJ_Icon_Green.bmp");
   mIconGreenBmp->attribute |= BMA_TRANSPARENT;
   
   mEndGreenBmp = SimResource::get(manager)->load("HJ_End_Green.bmp");
   AssertFatal(mEndGreenBmp, "Unable to load HJ_End_Green.bmp");
   mEndGreenBmp->attribute |= BMA_TRANSPARENT;
   
   lrBorderBmp = SimResource::get(manager)->load("lr_hudTrans.bmp");
   AssertFatal(lrBorderBmp, "Unable to load lr_hudTrans.bmp");
   
   lrIconGreenBmp = SimResource::get(manager)->load("LR_HJ_Icon_Green.bmp");
   AssertFatal(lrIconGreenBmp, "Unable to load LR_HJ_Icon_Green.bmp");
   lrIconGreenBmp->attribute |= BMA_TRANSPARENT;
   
   lrEndGreenBmp = SimResource::get(manager)->load("LR_HJ_End_Green.bmp");
   AssertFatal(lrEndGreenBmp, "Unable to load LR_HJ_End_Green.bmp");
   lrEndGreenBmp->attribute |= BMA_TRANSPARENT;
   
   //set the extent
   extent.set(mBorderBmp->getWidth() + 3, mBorderBmp->getHeight() + 3);
   
   mbLowRes = FALSE;
   
   //make sure the HUD is onscreen
   snap();
   
   return true;
}

extern void drawBracket(GFXSurface *sfc, int x, int ty, int by, int dx);

void JetPackHud::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
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
      
   //make sure we have a player and a psc
   if(!cg.psc) return;
   
   //draw the border
   if (! mbLowRes)
   {
      sfc->drawBitmap2d(mBorderBmp, &Point2I(offset.x + 1, offset.y + 1), GFX_FLIP_Y);
   }
   else
   {
      sfc->drawBitmap2d(lrBorderBmp, &Point2I(offset.x + 1, offset.y - 11), GFX_FLIP_Y);
   }
   
   float energyLevel = 1.0 - cg.psc->getEnergyLevel();
   int energyWidth = int(energyLevel * 85.0f);
   if (energyLevel > 0.0) energyWidth = max(energyWidth, 1);
   int color = LIGHT_BLUE;
   if (mbLowRes) energyWidth = int(energyLevel * 51.0f);
   
   //draw the icon
   if (! mbLowRes)
   {
      sfc->drawBitmap2d(mIconGreenBmp, &Point2I(offset.x + 5, offset.y + 8));
   }
   else
   {
      sfc->drawBitmap2d(lrIconGreenBmp, &Point2I(offset.x + 3, offset.y + 4));
   }
   
   //stop here if there is no energy to draw
   if (energyLevel == 0) return;
   
   //draw the energy level
   int barWidth;
   Point2I bmpOffset = offset;
   if (! mbLowRes)
   {
      barWidth = 84;
      bmpOffset.x += 37;
      bmpOffset.y += 11;
   }
   else
   {
      barWidth = 52;
      bmpOffset.x += 23;
      bmpOffset.y += 7;
   }
   RectI energyClip;
   energyClip.upperL = bmpOffset;
   energyClip.lowerR.x = bmpOffset.x + energyWidth;
   energyClip.lowerR.y = bmpOffset.y + (! mbLowRes ? mEndGreenBmp->getHeight() : lrEndGreenBmp->getHeight());
   sfc->setClipRect(&energyClip);
   
   GFXBitmap *endBmp = (! mbLowRes ? mEndGreenBmp : lrEndGreenBmp);
   sfc->drawBitmap2d(endBmp, &bmpOffset);
   bmpOffset.x += endBmp->getWidth();
   int rectWidth = barWidth - (2 * endBmp->getWidth());
   sfc->drawRect2d_f(&RectI(bmpOffset.x, bmpOffset.y, bmpOffset.x + rectWidth, bmpOffset.y + endBmp->getHeight() - 1), color);
   bmpOffset.x += rectWidth;
   sfc->drawBitmap2d(endBmp, &bmpOffset, GFX_FLIP_X);
   
}

IMPLEMENT_PERSISTENT(JetPackHud);

};

