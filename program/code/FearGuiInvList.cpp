#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "Player.h"
#include "Item.h"
#include "FearGuiInvList.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "SimBase.h"
#include "fearGlobals.h"
#include "fearPlayerPSC.h"
#include "simResource.h"
#include "dataBlockManager.h"
namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG(InventoryListCtrl, FOURCC('F','G','i','l'));

bool InventoryListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   hFont = SimResource::loadByTag(manager, IDFNT_W_10, true);
   cellSize.set(640, hFont->getHeight() + 2);
   return true;
}

void InventoryListCtrl::onWake()
{
   //see if we can find the item
   if (! cg.player)
   {
      setSize(Point2I( 1, 0));
      return;
   }
   
   //get the inventory manager
   PlayerPSC *ppsc = (PlayerPSC *) manager->findObject(PlayerPSCId);

   //loop through the list, count the number of weapons, and store the indices
   int count = 0;
	int itemTypeCount = cg.dbm->getDataGroupSize(DataBlockManager::ItemDataType);
   for (int i = 0; i < itemTypeCount; i++)
   {
      //make sure the item is a weapon we have
      if (ppsc->itemCount(i) > 0)
      {
         count++;
      }
   }
   setSize(Point2I( 1, count));
}

void InventoryListCtrl::onPreRender()
{
   if(! cg.player)
   {
      setSize(Point2I( 1, 0));
      return;
   }

   //get the inventory manager
   PlayerPSC *ppsc = (PlayerPSC *) manager->findObject(PlayerPSCId);
   
   //loop through the list, count the number of weapons, and store the indices
   int newSize = 0;
	int itemTypeCount = cg.dbm->getDataGroupSize(DataBlockManager::ItemDataType);
   for (int i = 0; i < itemTypeCount; i++)
   {
      //make sure the item is a weapon we have
      if (ppsc->itemCount(i) > 0)
      {
         newSize++;
      }
   }
   if(newSize != size.y)
   {
      setSize(Point2I(1, newSize));
   }

   //clear the vector
   inventory.clear();
   
   for (int j = 0; j < itemTypeCount; j++)
   {
      if (ppsc->itemCount(j))
      {
	      InventoryRep item;
			Item::ItemData* data = static_cast<Item::ItemData*>
				(cg.dbm->lookupDataBlock(j,DataBlockManager::ItemDataType));
         strncpy(item.name, data->description, MaxInvNameLength);
         item.name[MaxInvNameLength] = '\0';
         item.count = ppsc->itemCount(j); 
         inventory.push_back(item);
      }
   }
}

void InventoryListCtrl::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool, bool)
{
   char buf[8];
   Point2I clipMax = root->getContentControl()->extent;
   clipMax.x -= 1;
   clipMax.y -= 1;
   
   //find the inventory name
   offset.x += 60;
   sfc->setClipRect(&RectI(offset.x, offset.y,
                   min(clipMax.x, offset.x + 110),
                   min(clipMax.y, offset.y + 2 * hFont->getHeight())));
   sfc->drawText_p(hFont, &offset, inventory[cell.y].name);
   
   //and the count
   offset.x += 120;
   sprintf(buf, "%d", inventory[cell.y].count);
   sfc->setClipRect(&RectI(offset.x, offset.y,
                   min(clipMax.x, offset.x + 60),
                   min(clipMax.y, offset.y + 2 * hFont->getHeight())));
   sfc->drawText_p(hFont, &offset, buf);
}

bool InventoryListCtrl::becomeFirstResponder()
{
   return false;
}

};
