#include "g_surfac.h"
#include "g_bitmap.h"

#include "fearGuiHudCtrl.h"
#include "Player.h"
#include "FearPlayerPSC.h"
#include "fearHudPalTable.h"

#include "fearGlobals.h"

namespace FearGui
{

class FHInventory : public HudCtrl
{
   typedef HudCtrl Parent;
   
   Resource<GFXFont> textFont;
   Resource<GFXFont> selFont;
   
   Resource<GFXBitmap> mBitmap;
   Resource<GFXBitmap> mTopBitmap;
   Resource<GFXBitmap> mMid1Bitmap;
   Resource<GFXBitmap> mMid2Bitmap;
   Resource<GFXBitmap> mMidJoinBitmap;
   Resource<GFXBitmap> mBottomBitmap;
   
   Point2I centerPoint;
   
   int prevItemCount;
   
public:
   bool onAdd(void);
   void onRender(GFXSurface *, Point2I, const Box2I &);

   DECLARE_PERSISTENT(FHInventory);
};


bool FHInventory::onAdd()
{
   if (! Parent::onAdd())
      return false;
      
   textFont = SimResource::get(manager)->load("if_dg_8.pft");
   selFont = SimResource::get(manager)->load("if_g_10.pft");
   
   //load in the bitmaps
   mBitmap = SimResource::get(manager)->load("HW_CountOn.bmp");
   mTopBitmap = SimResource::get(manager)->load("HW_Top.bmp");
   mMid1Bitmap = SimResource::get(manager)->load("HW_Mid1.bmp");
   mMid2Bitmap = SimResource::get(manager)->load("HW_Mid2.bmp");
   mMidJoinBitmap = SimResource::get(manager)->load("HW_MidJoin.bmp");
   mBottomBitmap = SimResource::get(manager)->load("HW_Bottom.bmp");
   
   AssertFatal(mBitmap, "Unable to load count on bitmap");
   AssertFatal(mTopBitmap, "Unable to load top bitmap");
   AssertFatal(mMid1Bitmap, "Unable to load mid1 bitmap");
   AssertFatal(mMid2Bitmap, "Unable to load mid2 bitmap");
   AssertFatal(mMidJoinBitmap, "Unable to load join bitmap");
   AssertFatal(mBottomBitmap, "Unable to load bottom bitmap");
   
   mBitmap->attribute |= BMA_TRANSPARENT;
   mTopBitmap->attribute |= BMA_TRANSPARENT;
   mMid1Bitmap->attribute |= BMA_TRANSPARENT;
   mMid2Bitmap->attribute |= BMA_TRANSPARENT;
   mMidJoinBitmap->attribute |= BMA_TRANSPARENT;
   mBottomBitmap->attribute |= BMA_TRANSPARENT;
   
   //find out if it's to be displayed left or right
   //define the extent
   extent.set(1, 1);
   
   //make sure the HUD is visible
   snap();
   
   //initialize a local var
   prevWeapCount = -1;
   
   return true;
}

static const int gTransWidth = 36;
static const int gTransOffsets[8] = { 3, 13,  13, 3,  30, 3,  36, 9 };
static const int gIconSpacing = 4;
static const int gIconInset = 5;
static const int gIconOffsetX = 9;
static const int gIconOffsetY = 10;

static const int gTransTextOn[8] = { 1, 3,  3, 1,  21,1,  23, 3 };
static const int gTransTextOff[8] = { 0, 4,  2, 2,  18,2,  20, 4 };

void FHInventory::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   updateRect;

   //make sure we have a valid palette;   
   FearGui::InitializeHudColors(sfc->getPalette());

   if(! cg.player)
      return;

   //get the inventory manager
   InventoryManager *man = cg.player->getInventory();
   PlayerPSC *psc = (PlayerPSC *) manager->findObject(PlayerPSCId);
   
   //find the current ammo type
   int curWeap;
   curWeap = man->getSelected(InventoryManager::PRIMARY_WEAPON);
   
   //variables used for the display
   int weapons[InventoryManager::NUM_ITEMS];
   int dispCurWeap;
   
   //loop through the list, count the number of weapons, and store the indices
   int weapCount = 0;
   for (int i = 0; i < InventoryManager::NUM_ITEMS; i++)
   {
      //make sure the item is a weapon we have
      if ((man->itemCategory(i) == InventoryManager::WEAPON) && (psc->itemCount(i) > 0))
      {
         if (i == curWeap) dispCurWeap = weapCount;
         weapons[weapCount++] = i;
      }
   }
   
   //make sure we have a current weapon
   if (weapCount <= 0)
   {
      prevWeapCount = -1;
      extent.y = mBitmap->getHeight();
      return;
   }
   
   if (weapCount != prevWeapCount)
   {
      //calculate the new dimentions based on the number of weapons
      Point2I oldExtent = extent;
      extent.y = mTopBitmap->getHeight() + mBottomBitmap->getHeight() +
                  (weapCount * (mBitmap->getHeight() + gIconSpacing)) -
                  gIconSpacing - 2 * gIconInset;
      extent.x = gTransWidth + mBitmap->getWidth() + gIconSpacing / 2;
      snap();
      prevWeapCount = weapCount;
      return;
   }
   
   //make sure the HUD is visible
   if ((parent->extent.x < extent.x) || (parent->extent.y < extent.y))
      return;
      
   //DRAW THE TRANSPARENT BACKGROUND
   //initialize our vertices
   Point3F poly[8];
   for (int j = 0; j < 8; j++)
   {
      poly[j].z = 1.0f;
   }
   
   //set the vertices
   for (int k = 0; k < 4; k++)
   {
      poly[k].x = offset.x + gTransOffsets[2 * k];
      poly[k].y = offset.y + gTransOffsets[2 * k + 1];
      poly[7 - k].x = offset.x + gTransOffsets[2 * k];
      poly[7 - k].y = offset.y + extent.y - gTransOffsets[2 * k + 1];
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
   
   //draw the top
   Point2I bmpOffset = offset;
   sfc->drawBitmap2d(mTopBitmap, &bmpOffset);
   bmpOffset.y += mTopBitmap->getHeight();
   
   //draw the middle
   int stretchMid = extent.y - mTopBitmap->getHeight() - mBottomBitmap->getHeight();
   if (stretchMid < 30)
   {
      sfc->drawBitmap2d(mMid1Bitmap, &bmpOffset, &Point2I(mMid1Bitmap->getWidth(), stretchMid));
      bmpOffset.y += stretchMid;
   }
   else
   {
      int stretch3rd = (stretchMid / 3) - 2 * mMidJoinBitmap->getHeight();
      sfc->drawBitmap2d(mMid1Bitmap, &bmpOffset, &Point2I(mMid1Bitmap->getWidth(), stretch3rd));
      bmpOffset.y += stretch3rd;
      sfc->drawBitmap2d(mMidJoinBitmap, &bmpOffset);
      bmpOffset.y += mMidJoinBitmap->getHeight();
      sfc->drawBitmap2d(mMid2Bitmap, &bmpOffset, &Point2I(mMid1Bitmap->getWidth(),
                          stretchMid - 2 * stretch3rd - 2 * mMidJoinBitmap->getHeight()));
      bmpOffset.y += stretchMid - 2 * stretch3rd - 2 * mMidJoinBitmap->getHeight();
      sfc->drawBitmap2d(mMidJoinBitmap, &bmpOffset);
      bmpOffset.y += mMidJoinBitmap->getHeight();
      sfc->drawBitmap2d(mMid1Bitmap, &bmpOffset, &Point2I(mMid1Bitmap->getWidth(), stretch3rd));
      bmpOffset.y += stretch3rd;
   }
   
   //draw the bottom
   sfc->drawBitmap2d(mBottomBitmap, &bmpOffset);
      
   //draw the icons
   Point2I tempOffset;
   tempOffset.x = offset.x + gIconOffsetX;
   tempOffset.y = offset.y + gIconOffsetY;
   
   for (int n = 0; n < weapCount; n++)
   {
      int iconBmpTag;
      const char *iconBmpName;
      if (n == dispCurWeap)
      {
         iconBmpTag = man->itemHudIconOnTag(weapons[n]);
      }
      else
      {
         iconBmpTag = man->itemHudIconOffTag(weapons[n]);
      }
      
	   iconBmpName = SimTagDictionary::getString(manager,iconBmpTag);
      Resource<GFXBitmap> weaponBmp;
      if (iconBmpName)
      {
         weaponBmp = SimResource::get(manager)->load(iconBmpName);
         AssertFatal(weaponBmp, "Unable to load weaponBmp.");
         weaponBmp->attribute |= BMA_TRANSPARENT;
      }
      else
      {
         weaponBmp = mBitmap;
      }
      sfc->drawBitmap2d(weaponBmp, &tempOffset);

      // draw the text backdrop
      Point2I textOffset;
      textOffset.x = tempOffset.x + weaponBmp->getWidth() + 3 + gIconSpacing / 2;
      textOffset.y = tempOffset.y;
      
      //set the vertices
      const int *offsetTable = (n == dispCurWeap ? gTransTextOn : gTransTextOff);
      for (int p = 0; p < 4; p++)
      {
         poly[p].x = textOffset.x + offsetTable[2 * p];
         poly[p].y = textOffset.y + offsetTable[2 * p + 1];
         poly[7 - p].x = textOffset.x + offsetTable[2 * p];
         poly[7 - p].y = textOffset.y + mBitmap->getHeight() - offsetTable[2 * p + 1];
      }
      
      //draw the translucent box
      sfc->setFillMode(GFX_FILL_CONSTANT);
      sfc->setShadeSource(GFX_SHADE_NONE);
      sfc->setHazeSource(GFX_HAZE_NONE);
      sfc->setAlphaSource(GFX_ALPHA_FILL);
      sfc->setTransparency (false);
      sfc->setFillColor(254, ALPHAPAL_INDEX);

      for (int q = 0; q < 8; q++)
      {
         sfc->addVertex(poly + q);
      }
         
      sfc->emitPoly();
      sfc->setAlphaSource(GFX_ALPHA_NONE);
      
      //draw the text bitmap
      if (n == dispCurWeap)
      {
         sfc->drawBitmap2d(mBitmap, &textOffset);
         textOffset.x += gIconSpacing / 2;
      }
      else
      {
         textOffset.x += gIconSpacing / 4;
      }
      
      //draw the text
      char buf[8] = "888";
      GFXFont *font = (n == dispCurWeap ? selFont : textFont);
		int count = min(man->getCount(man->itemAmmo(weapons[n])), 999);
      if (count < 0) count = 999;
      sprintf(buf, "%d", count);
      textOffset.x += (font->getStrWidth("888") - font->getStrWidth(buf)) / 2;
      sfc->drawText_p(font, &textOffset, buf);
      
      //now update the y for the next icon
      tempOffset.y += mBitmap->getHeight() + gIconSpacing;
   }
}

IMPLEMENT_PERSISTENT(FHInventory);

};