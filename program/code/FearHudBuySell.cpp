#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "Player.h"
#include "FearItem.h"
#include "FearHudInvList.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "SimBase.h"
#include "fearGlobals.h"
#include "fearPlayerPSC.h"
#include "simResource.h"
#include "console.h"

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG(FGHInventory, FOURCC('F','G','i','v'));

enum {
   
   ILKey = 0,
   ILItem,
   ILQuantity,
   ILCount
};

static FGArrayCtrl::ColumnInfo gInventoryInfo[ILCount] =
{
   { IDSTR_KEY,   	         10, 300,   0,   0,    TRUE, 35, NULL, NULL },
   { IDSTR_CUR_ITEM,	         10, 300,   1,   1,    TRUE, 100, NULL, NULL },
   { IDSTR_QUANTITY,          10, 300,   2,   2,    TRUE, 35, NULL, NULL },
};                                       
static FGArrayCtrl::ColumnInfo *gInfoPtrs[ILCount];

bool FGHInventory::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
	numColumns = ILCount;
	columnInfo = gInfoPtrs;
   
   //set the ptrs table
   for (int i = 0; i < ILCount; i++)
   {
      gInfoPtrs[i] = &gInventoryInfo[i];
   }
   
   //set the cell dimensions
   int width = 0;
   for (int k = 0; k < ILCount; k++)
   {
      width += gInfoPtrs[k]->width;
   }
   cellSize.set(width, hFont->getHeight() + 6);
      
   return true;
}

void FGHInventory::onWake()
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
   for (int i = 0; i < InventoryManager::NUM_ITEMS; i++)
   {
      //make sure the item is a weapon we have
      if (ppsc->itemCount(i) > 0)
      {
         count++;
      }
   }
   
   setSize(Point2I( 1, count));
}

void FGHInventory::onPreRender()
{
   if(! cg.player)
   {
      setSize(Point2I( 1, 0));
      return;
   }
   
   //get the inventory manager
   PlayerPSC *ppsc = (PlayerPSC *) manager->findObject(PlayerPSCId);
   InventoryManager *man = cg.player->getInventory();
   
   //loop through the list, count the number of weapons, and store the indices
   int newSize = 0;
   for (int i = 0; i < InventoryManager::NUM_ITEMS; i++)
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
   
   for (int j = 0; j < InventoryManager::NUM_ITEMS; j++)
   {
      InventoryRep item;
      if (ppsc->itemCount(j))
      {
         strncpy(item.name, man->itemName(j), MaxInvNameLength);
         item.name[MaxInvNameLength] = '\0';
         item.count = ppsc->itemCount(j);
         item.index = j; 
         inventory.push_back(item);
      }
   }
   
   Parent::onPreRender();
}

static char buffer[256];
char* FGHInventory::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   switch (cell.x)
   {
      case ILKey:
         sprintf(buffer, "N/A");
         break;
         
      case ILQuantity:
         sprintf(buffer, "%d", inventory[cell.y].count);
         break;
         
      case ILItem:
         strcpy(buffer, inventory[cell.y].name);
         break;
   }
   return buffer;
}

void FGHInventory::dropSelected(void)
{
   if (selectedCell.y < 0 || selectedCell.y >= inventory.size()) return;
   Console->executef(4, "remoteEval", "2048", "drop", inventory[selectedCell.y].name);    
}

void FGHInventory::useSelected(void)
{
   if (selectedCell.y < 0 || selectedCell.y >= inventory.size()) return;
   
   //post the action to the PSC
   char buf[8], buf2[8];
   sprintf(buf, "%d", PlayerPSCId);
   sprintf(buf2, "%d", inventory[selectedCell.y].index);
   Console->executef(4, "postAction", buf, "IDACTION_USEITEM", buf2);
   
   //PlayerPSC *ppsc = (PlayerPSC *) manager->findObject(PlayerPSCId);
   //if (ppsc) ppsc->processAction(IDACTION_USEITEM
}

};
