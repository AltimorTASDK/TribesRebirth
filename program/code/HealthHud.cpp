#include "fearGuiHudCtrl.h"
#include "Player.h"
#include "FearDcl.h"
#include "FearPlayerPSC.h"
#include "fearGlobals.h"

#include "g_surfac.h"
#include "g_bitmap.h"

#include "fearGuiShellPal.h"
#include "simResource.h"

namespace FearGui
{

class HealthHud : public HudCtrl
{
   typedef HudCtrl Parent;
   
   Resource<GFXBitmap> mBorderBmp;
   Resource<GFXBitmap> mIconGreenBmp;
   Resource<GFXBitmap> mIconRedBmp;
   Resource<GFXBitmap> mEndGreenBmp;
   Resource<GFXBitmap> mEndYellowBmp;
   Resource<GFXBitmap> mEndRedBmp;
   
   Resource<GFXBitmap> lrBorderBmp;
   Resource<GFXBitmap> lrIconGreenBmp;
   Resource<GFXBitmap> lrIconRedBmp;
   Resource<GFXBitmap> lrEndGreenBmp;
   Resource<GFXBitmap> lrEndYellowBmp;
   Resource<GFXBitmap> lrEndRedBmp;
   
   SimTime mStartFlashTime;
   
   bool mbLowRes;

public:
   bool onAdd(void);
   void onRender(GFXSurface *, Point2I, const Box2I &);

   DECLARE_PERSISTENT(HealthHud);
};

bool HealthHud::onAdd()
{
   if (! Parent::onAdd())
      return false;
    
   //load the bitmaps
   mBorderBmp = SimResource::get(manager)->load("hudTrans.bmp");
   AssertFatal(mBorderBmp, "Unable to load hudTrans.bmp");
   
   mIconGreenBmp = SimResource::get(manager)->load("HH_Icon_Green.bmp");
   AssertFatal(mIconGreenBmp, "Unable to load HH_Icon_Green.bmp");
   mIconGreenBmp->attribute |= BMA_TRANSPARENT;
   
   mIconRedBmp = SimResource::get(manager)->load("HH_Icon_Red.bmp");
   AssertFatal(mIconRedBmp, "Unable to load HH_Icon_Red.bmp");
   mIconRedBmp->attribute |= BMA_TRANSPARENT;
   
   mEndGreenBmp = SimResource::get(manager)->load("HH_End_Green.bmp");
   AssertFatal(mEndGreenBmp, "Unable to load HH_Icon_Red.bmp");
   mEndGreenBmp->attribute |= BMA_TRANSPARENT;
   
   mEndYellowBmp = SimResource::get(manager)->load("HH_End_Yellow.bmp");
   AssertFatal(mEndYellowBmp, "Unable to load HH_Icon_Red.bmp");
   mEndYellowBmp->attribute |= BMA_TRANSPARENT;
   
   mEndRedBmp = SimResource::get(manager)->load("HH_End_Red.bmp");
   AssertFatal(mEndRedBmp, "Unable to load HH_Icon_Red.bmp");
   mEndRedBmp->attribute |= BMA_TRANSPARENT;
   
   //lores versions
   lrBorderBmp = SimResource::get(manager)->load("lr_hudTrans.bmp");
   AssertFatal(lrBorderBmp, "Unable to load lr_hudTrans.bmp");
   
   lrIconGreenBmp = SimResource::get(manager)->load("LR_HH_Icon_Green.bmp");
   AssertFatal(lrIconGreenBmp, "Unable to load LR_HH_Icon_Green.bmp");
   lrIconGreenBmp->attribute |= BMA_TRANSPARENT;
   
   lrIconRedBmp = SimResource::get(manager)->load("LR_HH_Icon_Red.bmp");
   AssertFatal(lrIconRedBmp, "Unable to load LR_HH_Icon_Red.bmp");
   lrIconRedBmp->attribute |= BMA_TRANSPARENT;
   
   lrEndGreenBmp = SimResource::get(manager)->load("LR_HH_End_Green.bmp");
   AssertFatal(lrEndGreenBmp, "Unable to load LR_HH_Icon_Red.bmp");
   lrEndGreenBmp->attribute |= BMA_TRANSPARENT;
   
   lrEndYellowBmp = SimResource::get(manager)->load("LR_HH_End_Yellow.bmp");
   AssertFatal(lrEndYellowBmp, "Unable to load LR_HH_Icon_Red.bmp");
   lrEndYellowBmp->attribute |= BMA_TRANSPARENT;
   
   lrEndRedBmp = SimResource::get(manager)->load("LR_HH_End_Red.bmp");
   AssertFatal(lrEndRedBmp, "Unable to load LR_HH_Icon_Red.bmp");
   lrEndRedBmp->attribute |= BMA_TRANSPARENT;
   
   mbLowRes = FALSE;
   
   //set the extent
   extent.set(mBorderBmp->getWidth() + 3, mBorderBmp->getHeight() + 3);
   mStartFlashTime = -1;
   
   //make sure the HUD is onscreen
   snap();
   
   return true;
}

void HealthHud::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
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
   sfc->setAlphaSource(GFX_ALPHA_TEXTURE);
   sfc->drawBitmap2d(! mbLowRes ? mBorderBmp : lrBorderBmp, &Point2I(offset.x + 1, offset.y + 1));
   sfc->setAlphaSource(GFX_ALPHA_NONE);
   
   float energyLevel = 1.0 - cg.psc->getDamageLevel();
   int energyWidth = int(energyLevel * 85.0f);
   if (energyLevel > 0.0) energyWidth = max(energyWidth, 1);
   int color = energyWidth >= 57 ? GREEN : (energyWidth >= 28 ? YELLOW : RED);
   if (mbLowRes) energyWidth = int(energyLevel * 51.0f);
   
   //draw the icon
   GFXBitmap *iconBmp;
   Point2I iconOffset;
   if (! mbLowRes)
   {
      if (energyWidth >= 1 && energyWidth <= 27)
      {
         SimTime curTime = GetTickCount();
         if (mStartFlashTime < 0) mStartFlashTime = curTime;
         SimTime timeElapsed = curTime - mStartFlashTime;
         iconBmp = ((int(timeElapsed) >> 9) % 2 ? mIconGreenBmp : mIconRedBmp);
      }
      else if (energyWidth >= 28) iconBmp = mIconGreenBmp;
      else iconBmp = mIconRedBmp;
      iconOffset.set(5, 3);
   }
   else
   {
      if (energyWidth >= 1 && energyWidth <= 16)
      {
         SimTime curTime = GetTickCount();
         if (mStartFlashTime < 0) mStartFlashTime = curTime;
         SimTime timeElapsed = curTime - mStartFlashTime;
         iconBmp = ((int(timeElapsed) >> 9) % 2 ? lrIconGreenBmp : lrIconRedBmp);
      }
      else if (energyWidth >= 17) iconBmp = lrIconGreenBmp;
      else iconBmp = lrIconRedBmp;
      iconOffset.set(4, 4);
   }
   sfc->drawBitmap2d(iconBmp, &Point2I(offset.x + iconOffset.x, offset.y + iconOffset.y));
   
   //finish here if there is no energy to draw
   if (energyLevel == 0) return;
   
   //draw the energy level
   int barWidth;
   Point2I bmpOffset = offset;
   if (! mbLowRes)
   {
      barWidth = 84;
      bmpOffset.x += 37;
      bmpOffset.y += 10;
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
   
   GFXBitmap *endBmp;
   if (! mbLowRes)
   {
      endBmp = (color == GREEN ? mEndGreenBmp : (color == YELLOW ? mEndYellowBmp : mEndRedBmp));
   }
   else
   {
      endBmp = (color == GREEN ? lrEndGreenBmp : (color == YELLOW ? lrEndYellowBmp : lrEndRedBmp));
   }
   sfc->drawBitmap2d(endBmp, &bmpOffset);
   bmpOffset.x += endBmp->getWidth();
   int rectWidth = barWidth - (2 * endBmp->getWidth());
   sfc->drawRect2d_f(&RectI(bmpOffset.x, bmpOffset.y, bmpOffset.x + rectWidth, bmpOffset.y + endBmp->getHeight() - 1), color);
   bmpOffset.x += rectWidth;
   sfc->drawBitmap2d(endBmp, &bmpOffset, GFX_FLIP_X);
}

IMPLEMENT_PERSISTENT(HealthHud);

};