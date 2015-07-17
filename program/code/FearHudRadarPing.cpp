#include "fearGuiHudCtrl.h"

#include "g_surfac.h"
#include "g_bitmap.h"
#include "g_font.h"
#include "simResource.h"
#include "fearGlobals.h"
#include "gameBase.h"
#include "player.h"
#include "FearPlayerPSC.h"

#define PixPerDegree 1

namespace FearGui
{

class FGHSensorPing : public HudCtrl
{
   typedef HudCtrl Parent;
   
   Resource<GFXBitmap> mBmpBase;
   Resource<GFXBitmap> mBmpPingedLo;
   Resource<GFXBitmap> mBmpPingedMed;
   Resource<GFXBitmap> mBmpPingedHi;
   Resource<GFXBitmap> mBmpSupressed;
   
   Resource<GFXBitmap> lrBmpBase;
   Resource<GFXBitmap> lrBmpPingedLo;
   Resource<GFXBitmap> lrBmpPingedMed;
   Resource<GFXBitmap> lrBmpPingedHi;
   Resource<GFXBitmap> lrBmpSupressed;
   
   SimTime startBlinkTime;
   bool mbLowRes;

public:
   bool onAdd(void);
   void onRender(GFXSurface *, Point2I, const Box2I &);

   DECLARE_PERSISTENT(FGHSensorPing);
};

bool FGHSensorPing::onAdd(void)
{
   if (! Parent::onAdd())
      return false;
      
   //load the bitmaps      
   mBmpBase = SimResource::get(manager)->load("ping.bmp");
   AssertFatal(mBmpBase, "Unable to load ping.bmp");
   
   mBmpPingedLo = SimResource::get(manager)->load("HP_PingedLo.bmp");
   AssertFatal(mBmpPingedLo, "Unable to load HP_PingedLow.bmp");
   mBmpPingedLo->attribute |= BMA_TRANSPARENT;
   
   mBmpPingedMed = SimResource::get(manager)->load("HP_PingedMed.bmp");
   AssertFatal(mBmpPingedMed, "Unable to Medad HP_PingedMedw.bmp");
   mBmpPingedMed->attribute |= BMA_TRANSPARENT;
   
   mBmpPingedHi = SimResource::get(manager)->load("HP_PingedHi.bmp");
   AssertFatal(mBmpPingedHi, "Unable to load HP_PingedHi.bmp");
   mBmpPingedHi->attribute |= BMA_TRANSPARENT;
   
   mBmpSupressed = SimResource::get(manager)->load("HP_Supressed.bmp");
   AssertFatal(mBmpSupressed, "Unable to load HP_Supressed.bmp");
   mBmpSupressed->attribute |= BMA_TRANSPARENT;
   
   //low res      
   lrBmpBase = SimResource::get(manager)->load("lr_ping.bmp");
   AssertFatal(lrBmpBase, "Unable to load lr_ping.bmp");
   
   lrBmpPingedLo = SimResource::get(manager)->load("LR_HP_PingedLo.bmp");
   AssertFatal(lrBmpPingedLo, "Unable to load LR_HP_PingedLow.bmp");
   lrBmpPingedLo->attribute |= BMA_TRANSPARENT;
   
   lrBmpPingedMed = SimResource::get(manager)->load("LR_HP_PingedMed.bmp");
   AssertFatal(lrBmpPingedMed, "Unable to Medad LR_HP_PingedMedw.bmp");
   lrBmpPingedMed->attribute |= BMA_TRANSPARENT;
   
   lrBmpPingedHi = SimResource::get(manager)->load("LR_HP_PingedHi.bmp");
   AssertFatal(lrBmpPingedHi, "Unable to load LR_HP_PingedHi.bmp");
   lrBmpPingedHi->attribute |= BMA_TRANSPARENT;
   
   lrBmpSupressed = SimResource::get(manager)->load("LR_HP_Supressed.bmp");
   AssertFatal(lrBmpSupressed, "Unable to load LR_HP_Supressed.bmp");
   lrBmpSupressed->attribute |= BMA_TRANSPARENT;
   
   //local members
   startBlinkTime = -1;
   
   mbLowRes = FALSE;
   
   //set the extent
   extent.set(32, 23);
   
   //make sure the hud in onscreen
   snap();
   
   return true;
}
void FGHSensorPing::onRender(GFXSurface *sfc, Point2I offset, const Box2I& /*updateRect*/)
{
   Point2I canvasSize(sfc->getWidth(), sfc->getHeight());
   
   //check for lowres
   if (canvasSize.x < 512 || canvasSize.y < 384)
   {
      if (! mbLowRes)
      {
         mbLowRes = TRUE;
         extent.set(20, 15);
         snap();
      }
   }
   else
   {
      if (mbLowRes)
      {
         mbLowRes = FALSE;
         extent.set(32, 23);
         snap();
         return;
      }
   }
   
   //make sure the HUD can fit on the canvas
   if ((canvasSize.x < extent.x) || (canvasSize.y < extent.y))
      return;
      
   if (!cg.psc || !cg.player || (cg.player != cg.psc->getControlObject() && !(cg.psc->getControlObject()->getType() & VehicleObjectType)))
   	return;
   
   //draw the base bmp   
   sfc->drawBitmap2d((! mbLowRes ? mBmpBase : lrBmpBase), &offset);
   
   //first, check the status
   GFXBitmap *bmp = NULL;
   
   //get the ping status
   BYTE pingStatus = cg.psc->getControlObject()->getSensorPinged(); // cg.player->getSensorPinged();
      
   //set the bitmap
   if (pingStatus == GameBase::PingStatusSupressed)
   {
      bmp = mBmpSupressed;
      startBlinkTime = -1;
   }
   
   else if (pingStatus == GameBase::PingStatusOn)
   {
      //find out how much time has passed since the light blinked
      SimTime curTime = manager->getCurrentTime();
      if (startBlinkTime <= 0) startBlinkTime = curTime;
      
      int bmpValue = int((curTime - startBlinkTime) * 10) % 6;
      if (bmpValue == 0)
      	bmp = (! mbLowRes ? mBmpPingedLo : lrBmpPingedLo);
	      else
	      	if (bmpValue == 1)
	      		bmp = (! mbLowRes ? mBmpPingedMed : lrBmpPingedMed);
		      else
		      	bmp = (! mbLowRes ? mBmpPingedHi : lrBmpPingedHi);
      
   }
   else
   {
      startBlinkTime = -1;
   }
   
   //now draw the bmp
   if (bmp)
   {
      sfc->drawBitmap2d(bmp, &Point2I(offset.x + (extent.x - bmp->getWidth()) / 2,
                                       offset.y + (extent.y - bmp->getHeight()) / 2));
   }
}

IMPLEMENT_PERSISTENT(FGHSensorPing);

};