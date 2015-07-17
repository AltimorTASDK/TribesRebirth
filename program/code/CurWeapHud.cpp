#include "g_surfac.h"
#include "g_bitmap.h"

#include "fearGuiHudCtrl.h"
#include "Player.h"
#include "FearPlayerPSC.h"
#include "dataBlockManager.h"

#include "fearGlobals.h"
#include "simResource.h"
#include "g_font.h"
#include "fearGuiShellPal.h"

namespace FearGui
{

static const int gTransWidth = 36;
static const int gTransOffsets[8] = { 2, 8,  8, 2,  24, 2,  30, 8 };
static const int gIconSpacing = 4;
static const int gIconInset = 5;
static const int gIconOffsetX = 4;
static const int gIconOffsetY = 7;

static const int lr_gTransWidth = 26;
static const int lr_gTransOffsets[8] = { 1, 6,  6, 1,  16, 1,  18, 4 };
static const int lr_gIconSpacing = 2;
static const int lr_gIconInset = 2;
static const int lr_gIconOffsetX = 4;
static const int lr_gIconOffsetY = 6;

class CurrentWeaponHud : public HudCtrl
{
   typedef HudCtrl Parent;

   Resource<GFXFont> textFont;
   Resource<GFXFont> ammoFont;
   
   Resource<GFXBitmap> mBitmap;
   Resource<GFXBitmap> mDefaultWeapon;
   
   Resource<GFXFont> lr_textFont;
   Resource<GFXFont> lr_ammoFont;
   
   Resource<GFXBitmap> lr_Bitmap;
   Resource<GFXBitmap> lrDefaultWeapon;
   
   Resource<GFXBitmap> mInfinityOn;
   Resource<GFXBitmap> mInfinityOff;
   
   Resource<GFXBitmap> lrInfinityOn;
   Resource<GFXBitmap> lrInfinityOff;
   
   bool mbLowRes;
   
   Point2I centerPoint;
   
   int prevWeapCount;
   
public:
   bool onAdd(void);
   void onRender(GFXSurface *, Point2I, const Box2I &);

   DECLARE_PERSISTENT(CurrentWeaponHud);
};


bool CurrentWeaponHud::onAdd()
{
   if (! Parent::onAdd())
      return false;
      
   textFont = SimResource::get(manager)->load("if_dg_8.pft");
   ammoFont = SimResource::get(manager)->load("if_g_8.pft");
   
   //load in the bitmaps
   
   //mBitmap = SimResource::get(manager)->load("HW_CountOn.bmp");
   mBitmap = SimResource::get(manager)->load("ammoSh.bmp");
   AssertFatal(mBitmap, "Unable to load count on bitmap");
//   mBitmap->attribute |= BMA_TRANSPARENT;
   
   mDefaultWeapon = SimResource::get(manager)->load("I_weapon_off.bmp");
   AssertFatal(mDefaultWeapon, "Unable to load mDefaultWeapon bitmap");
   mDefaultWeapon->attribute |= BMA_TRANSPARENT;
   
   mInfinityOn = SimResource::get(manager)->load("I_Infinity_on.bmp");
   AssertFatal(mInfinityOn.operator bool(), "Unable to load count I_Infinity_on.bmp");
   mInfinityOn->attribute |= BMA_TRANSPARENT;
   
   mInfinityOff = SimResource::get(manager)->load("I_Infinity_off.bmp");
   AssertFatal(mInfinityOff.operator bool(), "Unable to load count I_Infinity_off.bmp");
   mInfinityOff->attribute |= BMA_TRANSPARENT;
   
   //low res resources...
   lr_textFont = SimResource::get(manager)->load("sf_dk_green_5.pft");
   lr_ammoFont = SimResource::get(manager)->load("sf_green_5.pft");
   
   lrDefaultWeapon = SimResource::get(manager)->load("lr_weapon_off.bmp");
   AssertFatal(lrDefaultWeapon, "Unable to load lr_weapon_off bitmap");
   lrDefaultWeapon->attribute |= BMA_TRANSPARENT;
   
   lr_Bitmap = SimResource::get(manager)->load("lr_ammo.bmp");
   AssertFatal(lr_Bitmap, "Unable to load lr_ammo.bmp");
//   lr_Bitmap->attribute |= BMA_TRANSPARENT;
   
   lrInfinityOn = SimResource::get(manager)->load("lr_Infinity_on.bmp");
   AssertFatal(lrInfinityOn.operator bool(), "Unable to load count lr_Infinity_on.bmp");
   lrInfinityOn->attribute |= BMA_TRANSPARENT;
   
   lrInfinityOff = SimResource::get(manager)->load("lr_Infinity_off.bmp");
   AssertFatal(lrInfinityOff.operator bool(), "Unable to load count lr_Infinity_off.bmp");
   lrInfinityOff->attribute |= BMA_TRANSPARENT;
   
   //define the extent
   extent.set(1, 1);
   
   mbLowRes = FALSE;
   
   //make sure the HUD is visible
   snap();
   
   //initialize a local var
   prevWeapCount = -1;
   
   return true;
}

void CurrentWeaponHud::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   Point2I canvasSize(sfc->getWidth(), sfc->getHeight());
   
   //check for lowres
   if (canvasSize.x < 512 || canvasSize.y < 384)
   {
      if (! mbLowRes)
      {
         mbLowRes = TRUE;
         prevWeapCount = -1;
      }
   }
   else
   {
      if (mbLowRes)
      {
         mbLowRes = FALSE;
         prevWeapCount = -1;
         return;
      }
   }
   
   if (!cg.psc || !cg.player || cg.player != cg.psc->getControlObject())
      return;
      
   //variables used for the display
	int itemTypeCount = cg.dbm->getDataGroupSize(DataBlockManager::ItemDataType);
   int  weapCount = 0;
   
   int j;
   for (j = 0; j < itemTypeCount; j++)
   {
      if (cg.psc->itemCount(j))
      {
			Item::ItemData* data = static_cast<Item::ItemData*>
				(cg.dbm->lookupDataBlock(j,DataBlockManager::ItemDataType));
	      if (data->hudIcon && data->showWeaponBar)
	      {
            weapCount++;
	      }
	   }
   }
      
   //make sure we have a current weapon
   if (weapCount <= 0)
   {
      prevWeapCount = -1;
      extent.y = mDefaultWeapon->getHeight();
      return;
   }
   
   if (weapCount != prevWeapCount)
   {
      //calculate the new dimentions based on the number of weapons
      
      if (mbLowRes)
      {
        extent.x = lr_gTransWidth + lrDefaultWeapon->getWidth() + lr_gIconSpacing / 2;
        extent.y = (weapCount * (lrDefaultWeapon->getHeight() + lr_gIconSpacing)) - lr_gIconSpacing + (2 * lr_gIconOffsetY);
      }
      else
      {
        extent.x = gTransWidth + mDefaultWeapon->getWidth() + gIconSpacing / 2;
        extent.y = (weapCount * (mDefaultWeapon->getHeight() + gIconSpacing)) - gIconSpacing + (2 * gIconOffsetY);
      }
      
      snap();
      prevWeapCount = weapCount;
      return;
   }
   
   //make sure the HUD is visible
   if ((parent->extent.x < extent.x) || (parent->extent.y < extent.y))
      return;
      
   //set the local tables
   const int *curTransOffsets = (mbLowRes ? &lr_gTransOffsets[0] : &gTransOffsets[0]);
   
      
   //DRAW THE TRANSPARENT BACKGROUND
   //initialize our vertices
   Point3F poly[8];
   for (j = 0; j < 8; j++)
   {
      poly[j].z = 1.0f;
   }
   
   //set the vertices
   for (int k = 0; k < 4; k++)
   {
      poly[k].x = offset.x + curTransOffsets[2 * k] + (mbLowRes ? 1 : 0);
      poly[k].y = offset.y + curTransOffsets[2 * k + 1];
      poly[7 - k].x = offset.x + curTransOffsets[2 * k] + (mbLowRes ? 1 : 0);
      poly[7 - k].y = offset.y + extent.y - curTransOffsets[2 * k + 1];
   }
   
   //draw the translucent box
   sfc->setFillMode(GFX_FILL_CONSTANT);
   sfc->setShadeSource(GFX_SHADE_NONE);
   sfc->setHazeSource(GFX_HAZE_NONE);
   sfc->setAlphaSource(GFX_ALPHA_FILL);
   sfc->setTransparency (false);
   sfc->setFillColor(254, ALPHAPAL_INDEX);

   for (int m = 0; m < 8; m++)
   {
      sfc->addVertex(poly + m);
   }
      
   sfc->emitPoly();
   sfc->setAlphaSource(GFX_ALPHA_NONE);

   //draw the frame
   Point2I tl = offset;
   Point2I br = offset + extent - 1;
   
   //top corner
   sfc->drawLine2d(&Point2I(tl.x + 0, tl.y + 7), &Point2I(tl.x + 7, tl.y + 0), GREEN);
   sfc->drawLine2d(&Point2I(tl.x + 0, tl.y + 8), &Point2I(tl.x + 8, tl.y + 0), GREEN);
   sfc->drawLine2d(&Point2I(tl.x + 0, tl.y + 9), &Point2I(tl.x + 9, tl.y + 0), GREEN);
   
   sfc->drawLine2d(&Point2I(tl.x + 7, tl.y + 0), &Point2I(tl.x + (mbLowRes ? 11 : 17), tl.y + 0), GREEN);
   sfc->drawLine2d(&Point2I(tl.x + 7, tl.y + 1), &Point2I(tl.x + (mbLowRes ? 11 : 17), tl.y + 1), GREEN);

   //bottom corner
   sfc->drawLine2d(&Point2I(tl.x + 0, br.y - 7), &Point2I(tl.x + 7, br.y - 0), GREEN);
   sfc->drawLine2d(&Point2I(tl.x + 0, br.y - 8), &Point2I(tl.x + 8, br.y - 0), GREEN);
   sfc->drawLine2d(&Point2I(tl.x + 0, br.y - 9), &Point2I(tl.x + 9, br.y - 0), GREEN);
   
   sfc->drawLine2d(&Point2I(tl.x + 7, br.y - 0), &Point2I(tl.x + (mbLowRes ? 11 : 17), br.y - 0), GREEN);
   sfc->drawLine2d(&Point2I(tl.x + 7, br.y - 1), &Point2I(tl.x + (mbLowRes ? 11 : 17), br.y - 1), GREEN);
   
   //left edge
   sfc->drawLine2d(&Point2I(tl.x + 0, tl.y + 7), &Point2I(tl.x + 0, br.y - 7), GREEN);
   sfc->drawLine2d(&Point2I(tl.x + 1, tl.y + 7), &Point2I(tl.x + 1, br.y - 7), GREEN);
   
   //draw the icons
   Point2I tempOffset;
   tempOffset.x = offset.x + (mbLowRes ? lr_gIconOffsetX : gIconOffsetX);
   tempOffset.y = offset.y + (mbLowRes ? lr_gIconOffsetY : gIconOffsetY);
   
   for (j = 0; j < itemTypeCount; j++)
   {
      //make sure we have the item
      if (cg.psc->itemCount(j) <= 0) continue;
		Item::ItemData* data = static_cast<Item::ItemData*>
                           				(cg.dbm->lookupDataBlock(j,DataBlockManager::ItemDataType));
                                       
      //make sure the item has both an icon, and is supposed to appear in the weapons hud                                 
      if (!(data->hudIcon && data->showWeaponBar)) continue;
      
      char iconBmpName[256];
      char defaultIconName[256];
      bool weapSelected = FALSE;
      
      for (int k = 0; k < Player::MaxItemImages; k++)
      {
         int itemType = cg.player->getMountedItem(k);
         if (itemType == j)
         {
            //if it's hilileOnActive we need to check if it's actually active
            if (data->hiliteOnActive)
            {
               weapSelected = cg.player->isItemFiring(k);
            }
            else  weapSelected = TRUE;
            break;
         }
      }
      GFXBitmap *weaponBmp = mbLowRes ?
         (weapSelected ? data->hudLROnBitmap : data->hudLROffBitmap) :
         (weapSelected ? data->hudIOnBitmap : data->hudIOffBitmap);

      if(!weaponBmp)
         weaponBmp = mbLowRes ? lrDefaultWeapon : mDefaultWeapon;
      
      sfc->drawBitmap2d(weaponBmp, &tempOffset);
      
      //find out if the weapon uses ammo                               `
      const Item::ItemData* itemData = Player::getItemData(j);
		if (itemData->imageId != -1)
		{
	      int ammoCount = -1;
	      const Player::ItemImageData* imageData = Player::getItemImageData(itemData->imageId);
	      if (imageData->ammoType >= 0)
	      {
	         ammoCount = cg.psc->itemCount(imageData->ammoType);
	      }

	      // draw the text backdrop
	      Point2I textOffset;
	      textOffset.x = tempOffset.x + weaponBmp->getWidth() + (mbLowRes ?  2 + lr_gIconSpacing / 2 : 2 + gIconSpacing / 2);
	      textOffset.y = tempOffset.y;
         sfc->drawBitmap2d((mbLowRes ? lr_Bitmap : mBitmap), &textOffset);
         textOffset.x += (mbLowRes ? 3 : 2);
         textOffset.y += (mbLowRes ? 0 : -1);
	      
         //draw the text
	      char buf[8] = "888";
	      
	      GFXFont *font;
	      if (mbLowRes) font = (weapSelected ? lr_ammoFont : lr_textFont);
	      else font = (weapSelected ? ammoFont : textFont);
	      
			int count = min(ammoCount, 999);
         
         //draw the infinity icon
	      if (count < 0)
         {
   	      textOffset.x += (mbLowRes ? 1 : 2);
   	      textOffset.y += (mbLowRes ? 3 : 6);
            GFXBitmap *bmp;
            if (mbLowRes) bmp = weapSelected ? lrInfinityOn : lrInfinityOff;
            else bmp = weapSelected ? mInfinityOn : mInfinityOff;
            sfc->drawBitmap2d(bmp, &textOffset);
         }
         else
         {
   	      sprintf(buf, "%d", count);
   	      textOffset.x += (font->getStrWidth("888") - font->getStrWidth(buf)) / 2;
   	      sfc->drawText_p(font, &textOffset, buf);
         }
		}
      
      //now update the y for the next icon
      tempOffset.y += (mbLowRes ? lrDefaultWeapon->getHeight() + lr_gIconSpacing : mDefaultWeapon->getHeight() + gIconSpacing);
   }
}

IMPLEMENT_PERSISTENT(CurrentWeaponHud);

};