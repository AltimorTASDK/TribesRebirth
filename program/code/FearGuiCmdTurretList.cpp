#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "simResource.h"
#include <g_font.h>
#include "fearGuiShellPal.h"
#include "fearGuiCmdTurretList.h"
#include "fearGuiTSCommander.h"
#include "sensorManager.h"
#include "fearGlobals.h"
#include "staticBase.h"
#include "dataBlockManager.h"
#include "player.h"
#include "console.h"

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG(CommandTurretCtrl, FOURCC('F','G','t','I'));

void CommandTurretCtrl::setLowRes(bool value)
{
   mLowRes = value;
   if (! mLowRes)
      cellSize.set(640, hFont->getHeight() + 2);
   else
      cellSize.set(320, lr_hFont->getHeight() + 1);
}

void CommandTurretCtrl::onWake()
{
   Parent::onWake();
   setSize(Point2I(1,0));
   selectedCell.set(-1, -1);
}

bool CommandTurretCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   //get the fonts
   hFont = SimResource::loadByTag(manager, IDFNT_HUD_10_HILITE, true);
   hFontInUse = SimResource::loadByTag(manager, IDFNT_HUD_10_OTHER, true);
   lr_hFont = SimResource::loadByTag(manager, IDFNT_HUD_6_HILITE, true);
   
   //load the damage icons
   bmpDamageLow = SimResource::get(manager)->load("CMD_DamageLow.bmp");
   AssertFatal(bmpDamageLow, "Unable to load CMD_DamageLow.bmp");
   bmpDamageLow->attribute |= BMA_TRANSPARENT;
   
   bmpDamageMed = SimResource::get(manager)->load("CMD_DamageMed.bmp");
   AssertFatal(bmpDamageMed, "Unable to load CMD_DamageMed.bmp");
   bmpDamageMed->attribute |= BMA_TRANSPARENT;
   
   bmpDamageHigh = SimResource::get(manager)->load("CMD_DamageHigh.bmp");
   AssertFatal(bmpDamageHigh, "Unable to load CMD_DamageHigh.bmp");
   bmpDamageHigh->attribute |= BMA_TRANSPARENT;
   
   bmpActive = SimResource::get(manager)->load("CMD_ItemActive.bmp");
   AssertFatal(bmpActive, "Unable to load CMD_ItemActive.bmp");
   bmpActive->attribute |= BMA_TRANSPARENT;
   
   bmpInactive = SimResource::get(manager)->load("CMD_ItemInactive.bmp");
   AssertFatal(bmpInactive, "Unable to load CMD_ItemInactive.bmp");
   bmpInactive->attribute |= BMA_TRANSPARENT;
   
   //set the lo res bool
   mLowRes = FALSE;
   
   cellSize.set(640, hFont->getHeight() + 2);
   return true;
}

Int32 CommandTurretCtrl::getMouseCursorTag(void)
{
   if (cg.commandTS && cg.commandTS->inWaypointMode())
   {
      return IDBMP_CURSOR_WAYPOINT_ARROW;
   }
   else return Parent::getMouseCursorTag();
}
   
void CommandTurretCtrl::onMouseDown(const SimGui::Event &event)
{
   if (active && cg.commandTS && cg.commandTS->inWaypointMode())
   {
      Point2I pt = globalToLocalCoord(event.ptMouse);
      
      //find out which cell was hit
      Point2I cell((pt.x < 0 ? -1 : pt.x / cellSize.x), (pt.y < 0 ? -1 : pt.y / cellSize.y));
      if (cell.x >= 0 && cell.x < size.x && cell.y >= 0 && cell.y < size.y)
      {
         Console->setVariable("CommandTargetName", itemList[cell.y].name);
         cg.commandTS->setCommandWaypointItem(itemList[cell.y].index);
      	cg.commandTS->setMouseMode(TSCommander::SelectingMode);
      }
   }
   else
   {
      Parent::onMouseDown(event);
      if (event.mouseDownCount > 1)
      {
         if (selectedCell.y == prevSelected.y)
         {
            if (cg.commandTS)
            {
               int index = cg.commandTS->getSelectedTurret();
               if (index >= 0)
               {
                  char cmdBuf[1024];
                  sprintf(cmdBuf, "remoteEval(2048, CmdrMountObject, %d);", index);
                  Console->evaluate(cmdBuf, false);
               }
            }
         }
      }
      prevSelected = selectedCell;
   }
}

bool CommandTurretCtrl::cellSelected(Point2I cell)
{
   if (cell.y >= 0 && cell.y < itemList.size())
   {
      if (cg.commandTS)
      {
         //deselect all players
         cg.commandTS->setSelectedPlayer(-1, FALSE);
         
         //deselect all static objects
         cg.commandTS->setSelectedObject(-1);
         
         //selecte the turret
         cg.commandTS->setSelectedTurret(itemList[cell.y].index);
         return Parent::cellSelected(cell);
      }
   }
   return FALSE;
}
   
void CommandTurretCtrl::onPreRender()
{
   itemList.clear();
   
   if (! cg.playerManager) return;
   PlayerManager::ClientRep *me = cg.playerManager->findClient(manager->getId());
   if (! me) return;
   
   if (cg.commandTS)
   {
      TSCommander::SensorTargetInfo *targ = &(cg.commandTS->targs[0]);
      for (int i = 0; i < TSCommander::MaxNumTargs; i++, targ++)
      {
         if (targ->team == me->team &&
				 (targ->datGroup == DataBlockManager::TurretDataType ||
				  targ->datGroup == DataBlockManager::FlierDataType ||
				  targ->datGroup == DataBlockManager::TankDataType ||
				  targ->datGroup == DataBlockManager::VehicleDataType))
         {
            //find the item's name
            if(targ->datId == -1)
               continue;
            const char *itemName = NULL;
            if (targ->name && targ->name[0])
            	itemName = targ->name;
            else
            {
         		GameBase::GameBaseData* data = (GameBase::GameBaseData*)(cg.dbm->lookupDataBlock(targ->datId,targ->datGroup));
               if (! data) continue;
               
               //push the item into the list
               if (data->description && data->description[0]) itemName = data->description;
            }
            
            //add the item to the list
            if (itemName)
            {
               ItemRep newItem;
               newItem.index = i;
               strncpy(newItem.name, itemName, MaxItemNameLength);
               newItem.name[MaxItemNameLength] = '\0';
               itemList.push_back(newItem);
            }
         }
      }
   }

   setSize(Point2I(1, itemList.size()));
   setUpdate();
}

void CommandTurretCtrl::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool, bool)
{
   Point2I cellOffset = offset;
   
   if (! cg.commandTS) return;
   if (cg.commandTS->targs[itemList[cell.y].index].selected == TRUE)
   {
      RectI selRect(cellOffset, cellOffset);
      selRect.lowerR.x += extent.x - 1;
      selRect.lowerR.y += extent.y - 1;
      selRect.upperL.x += 2 + bmpDamageHigh->getWidth() + 1 + bmpActive->getWidth();
      
      sfc->drawRect2d_f(&selRect, GREEN);
   }
   
   //draw the items damage status
   if (! cg.commandTS) return;
   int damageLevel = cg.commandTS->getTargetDamage(itemList[cell.y].index);
   GFXBitmap *healthBmp;
   if (damageLevel ==  2) healthBmp = bmpDamageHigh;
   else  if (damageLevel == 1) healthBmp = bmpDamageMed;
   else healthBmp = bmpDamageLow;
   
   //draw the bitmap
   cellOffset.x += 2;
   if (! healthBmp) return;
   sfc->drawBitmap2d(healthBmp, &Point2I(cellOffset.x, cellOffset.y + 1));
   cellOffset.x += healthBmp->getWidth() + 1;
   
   //draw then item's active status
   if (! cg.commandTS) return;
   int activeStatus = cg.commandTS->getTargetStatus(itemList[cell.y].index);
   GFXBitmap *bmp;
   if (activeStatus) bmp = bmpActive;
   else bmp = bmpInactive;
   
   //draw the bitmap
   sfc->drawBitmap2d(bmp, &Point2I(cellOffset.x, cellOffset.y + 1));
   cellOffset.x += bmp->getWidth() + 2;
   
   GFXFont *font;
   if (mLowRes) font = lr_hFont;
   else if (cg.commandTS->targs[itemList[cell.y].index].inUse != TRUE)
   {
      font = hFontInUse;
   }
   else font = hFont;
   sfc->drawText_p(font, &Point2I(cellOffset.x, cellOffset.y - 2), itemList[cell.y].name);
}

};
