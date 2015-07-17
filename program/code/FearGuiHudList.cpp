#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "FearCSDelegate.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "fear.strings.h"
#include "FearGuiHudList.h"
#include "FearGuiShellPal.h"
#include "m_qsort.h"
#include "console.h"
#include "g_barray.h"
#include "g_font.h"
#include "SimResource.h"

namespace FearGui
{

enum {
   
   HIName = 0,
   HIActive,
   HICount
};

static FGArrayCtrl::ColumnInfo gColumnInfo[HICount] =
{
   { -1,     10, 300,   0,   0,    TRUE, 110, NULL, NULL },
   { -1,     10, 300,   1,   1,    TRUE, 20, NULL, NULL },
};
static FGArrayCtrl::ColumnInfo *gInfoPtrs[HICount];                                         

struct HudDisplayInfo
{
   Int32 tag;
   bool  active;
};
static const int gNumHuds = 10;
static HudDisplayInfo gHuds[gNumHuds] =
{
   { IDCTG_HUD_CLOCK,         TRUE  },
   { IDCTG_HUD_COMPASS,       TRUE  },
   { IDCTG_HUD_WEAPON,        TRUE  },
   { IDCTG_HUD_CROSSHAIR,     TRUE  },
   { IDCTG_HUD_SNIPER_CROSS,  TRUE  },
   { IDCTG_HUD_HEALTH,        TRUE  },
   { IDCTG_HUD_JETPACK,       TRUE  },
   { IDCTG_HUD_CHAT_DISPLAY,  TRUE  },
   { IDCTG_HUD_SENSOR,        TRUE  },
   { IDCTG_HUD_LOGO,          TRUE  },
};

IMPLEMENT_PERSISTENT_TAG(HudListCtrl, FOURCC('F','G','h','l'));

Int32 HudListCtrl::getMouseCursorTag(void)
{
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
   if (pt.x > gColumnInfo[HIName].width) return IDBMP_CURSOR_HAND;
   return Parent::getMouseCursorTag();
}

void HudListCtrl::onMouseDown(const SimGui::Event &event)
{
   if(!active)
   {
      Parent::onMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   Point2I cell((pt.x < 0 ? -1 : pt.x / cellSize.x), (pt.y < 0 ? -1 : pt.y / cellSize.y));
   if (cell.x >= 0 && cell.x < size.x && cell.y >= 0 && cell.y < size.y)
   {
      //exception - the sniper crosshair is a pref toggle, not a hud
      if (gHuds[cell.y].tag == IDCTG_HUD_SNIPER_CROSS)
      {
         if (! stricmp(Console->getVariable("pref::SniperCrosshair"), "FALSE"))
         {
            Console->setVariable("pref::SniperCrosshair", "TRUE");
         }
         else
         {
            Console->setVariable("pref::SniperCrosshair", "FALSE");
         }
      }
      else
      {
         //find the associated hud, and toggle the visible flag
         SimGui::Control *ctrl = findRootControlWithTag(gHuds[cell.y].tag);
         if (ctrl) ctrl->setVisible(! ctrl->isVisible());
      }
   }
}

bool HudListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   //load the fonts
   hFont = SimResource::get(manager)->load("sf_Orange255_10.pft");
   AssertFatal(hFont.operator bool(), "Failed to load sf_Orange255_10.pft");
   hFontShadow = SimResource::get(manager)->load("sf_black_10.pft");
   AssertFatal(hFontShadow.operator bool(), "Failed to load sf_black_10.pft");
   
   lrFont = SimResource::get(manager)->load("sf_Yellow_6.pft");
   AssertFatal(lrFont.operator bool(), "Failed to load sf_Yellow_6.pft");
   lrFont->fi.flags |= FONT_LOWERCAPS;
   
   lrFontShadow = SimResource::get(manager)->load("sf_black_6.pft");
   AssertFatal(lrFontShadow.operator bool(), "Failed to load sf_black_6.pft");
   lrFontShadow->fi.flags |= FONT_LOWERCAPS;
      
   //load the bitmaps
   mHudOnBmp = SimResource::get(manager)->load("HUD_Checkbox_ON.bmp");
   AssertFatal(mHudOnBmp, "Unable to load HUD_Checkbox_ON bitmap");
   mHudOnBmp->attribute |= BMA_TRANSPARENT;

   mHudOffBmp = SimResource::get(manager)->load("HUD_Checkbox_DF.bmp");
   AssertFatal(mHudOffBmp, "Unable to load HUD_Checkbox_DF bitmap");
   mHudOffBmp->attribute |= BMA_TRANSPARENT;
   
   lrHudOnBmp = SimResource::get(manager)->load("LR_HUD_Checkbox_ON.bmp");
   AssertFatal(lrHudOnBmp, "Unable to load LR_HUD_Checkbox_ON bitmap");
   lrHudOnBmp->attribute |= BMA_TRANSPARENT;

   lrHudOffBmp = SimResource::get(manager)->load("LR_HUD_Checkbox_DF.bmp");
   AssertFatal(lrHudOffBmp, "Unable to load LR_HUD_Checkbox_DF bitmap");
   lrHudOffBmp->attribute |= BMA_TRANSPARENT;
   
   //set the console var - the sniper crosshair refers to a var, not a hud ctrl directly
   if (stricmp(Console->getVariable("pref::SniperCrosshair"), "FALSE"))
   {
      Console->setVariable("pref::SniperCrosshair", "TRUE");
   }
            
   //other misc vars
	columnsResizeable = FALSE;
	columnsSortable = FALSE;
	numColumns = HICount;
	columnInfo = gInfoPtrs;
   
   //set the ptrs table
   for (int j = 0; j < HICount; j++)
   {
      gInfoPtrs[j] = &gColumnInfo[j];
   }
   
   //set the cell dimensions
   mbLowRes = FALSE;
   int width = 0;
   for (int i = 0; i < HICount; i++)
   {
      width += gInfoPtrs[i]->width;
   }
   cellSize.set(width, hFont->getHeight() + 12);
   setSize(Point2I(1, gNumHuds));
   extent.set(width, gNumHuds * cellSize.y);
   
   return true;
}

void HudListCtrl::onWake(void)
{
   setSize(Point2I(1, gNumHuds));
}

void HudListCtrl::onPreRender(void)
{
   SimGui::Control *playGui = (SimGui::Control*)(manager->findObject("playGui"));
   if (parent && playGui)
   {
      if (playGui->extent.x < 512 || playGui->extent.y < 384)
      {
         mbLowRes = TRUE;
         gInfoPtrs[HIName]->width = 70;
         gInfoPtrs[HIActive]->width = 15;
         cellSize.set(85, lrFont->getHeight() + 6);
         extent.set(85, gNumHuds * cellSize.y);
         
         //change the parent extent
         Point2I newParentPosition;
         newParentPosition.x = (playGui->extent.x - (extent.x + 24)) / 2;
         newParentPosition.y = (playGui->extent.y - (extent.y + 24)) / 2;
         parent->resize(newParentPosition, Point2I(extent.x + 24, extent.y + 24));
         position.set(12, 12);
      }
      else
      {
         mbLowRes = FALSE;
         gInfoPtrs[HIName]->width = 110;
         gInfoPtrs[HIActive]->width = 20;
         cellSize.set(130, hFont->getHeight() + 12);
         extent.set(130, gNumHuds * cellSize.y);
         position.set(24, 24);
         
         //change the parent extent
         Point2I newParentPosition;
         newParentPosition.x = (playGui->extent.x - (extent.x + 48)) / 2;
         newParentPosition.y = (playGui->extent.y - (extent.y + 48)) / 2;
         parent->resize(newParentPosition, Point2I(extent.x + 48, extent.y + 48));
      }
   }
   setUpdate();
}

void HudListCtrl::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool, bool)
{
   const char *name = SimTagDictionary::getString(manager, gHuds[cell.y].tag);
   GFXFont *font;
   font = (! mbLowRes ? hFontShadow : lrFontShadow);
   sfc->drawText_p(font, &Point2I(offset.x - 1, offset.y + 1 - (! mbLowRes ? 2 : 0)), name);
   font = (! mbLowRes ? hFont : lrFont);
   sfc->drawText_p(font, &Point2I(offset.x, offset.y - (! mbLowRes ? 2 : 0)), name);
   
   //exception - the sniper crosshair is a pref toggle, not a hud
   if (gHuds[cell.y].tag == IDCTG_HUD_SNIPER_CROSS)
   {
      if (! stricmp(Console->getVariable("pref::SniperCrosshair"), "FALSE"))
      {
         sfc->drawBitmap2d((! mbLowRes ? mHudOffBmp : lrHudOffBmp), &Point2I(offset.x + gColumnInfo[HIName].width, offset.y)); 
      }
      else
      {
         sfc->drawBitmap2d((! mbLowRes ? mHudOnBmp : lrHudOnBmp), &Point2I(offset.x + gColumnInfo[HIName].width, offset.y)); 
      }
   }
   else
   {
      //see if the cell's hud display is visible
      SimGui::Control *ctrl = findRootControlWithTag(gHuds[cell.y].tag);
      if (ctrl && ctrl->isVisible())
      {
         sfc->drawBitmap2d((! mbLowRes ? mHudOnBmp : lrHudOnBmp), &Point2I(offset.x + gColumnInfo[HIName].width, offset.y)); 
      }
      else
      {
         sfc->drawBitmap2d((! mbLowRes ? mHudOffBmp : lrHudOffBmp), &Point2I(offset.x + gColumnInfo[HIName].width, offset.y)); 
      }
   }
}

void HudListCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   Grandparent::onRender(sfc, offset, updateRect);
}


};
