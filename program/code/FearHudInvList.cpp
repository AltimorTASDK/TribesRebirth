#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "Player.h"
#include "Item.h"
#include "FearHudInvList.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "SimBase.h"
#include "fearGlobals.h"
#include "fearPlayerPSC.h"
#include "simResource.h"
#include "console.h"
#include "simGuiDelegate.h"
#include "dlgPlay.h"
#include "fearGuiUnivButton.h"
#include "dataBlockManager.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fearGuiShapeView.h"

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG(FGHInventory, FOURCC('F','G','i','v'));

enum {
   
   ILFavorite = 0,
   ILItem,
   ILQuantity,
   ILCount
};

static FGArrayCtrl::ColumnInfo gInventoryInfo[ILCount] =
{
   { -1,          	         10, 300,   0,   0,    TRUE,  25, NULL, NULL },
   { IDSTR_INV_CUR_ITEM,	   10, 300,   1,   1,    TRUE, 152, NULL, NULL },
   { IDSTR_QUANTITY,          10, 300,   2,   2,    TRUE,  60, NULL, NULL },
};                                       
static FGArrayCtrl::ColumnInfo *gInfoPtrs[ILCount];

bool FGHInventory::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   //status bitmaps
   statusFavorite = SimResource::get(manager)->load("I_PingRed.bmp");
   AssertFatal(statusFavorite, "Unable to load statusPingRed bitmap");
   statusFavorite->attribute |= BMA_TRANSPARENT;
   
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
}

void FGHInventory::onPreRender()
{
   if(! cg.player)
   {
      setSize(Point2I( 1, 0));
      return;
   }
   
   //
   inventory.clear();
	int itemTypeCount = cg.dbm->getDataGroupSize(DataBlockManager::ItemDataType);
   for (int j = 0; j < itemTypeCount; j++)
      if (cg.psc->itemCount(j))
      {
			Item::ItemData* data = static_cast<Item::ItemData*>
				(cg.dbm->lookupDataBlock(j,DataBlockManager::ItemDataType));
	      if (data->showInventory)
	      {
		      InventoryRep item;
	         strncpy(item.name, data->description , MaxInvNameLength);
		      item.name[MaxInvNameLength] = '\0';
		      strncpy(item.shape, data->fileName , MaxInvNameLength);
	         item.shape[MaxInvNameLength] = '\0';
            if ((! strstr(item.shape, ".dts")) && (strlen(item.shape) < MaxInvNameLength - 4))
            {
               sprintf(&item.shape[strlen(item.shape)], ".dts");
            }
	         item.count = cg.psc->itemCount(j);
	         item.index = j;
            item.favorite = FALSE;
	         inventory.push_back(item);
	      }
	   }

  	if (inventory.size() != size.y)
		setSize(Point2I(1, inventory.size()));
   
   //now toggle the favorite flag   
   char buf[256];
   int counter = 0;
   while (1)
   {
      sprintf(buf, "pref::itemFavorite%d", counter);
      const char *fav = Console->getVariable(buf);
      if (fav && fav[0])
      {
         //see if it's for sale...
         Vector<InventoryRep>::iterator i;
         for (i = inventory.begin(); i < inventory.end(); i++)
         {
            if (! stricmp(fav, i->name))
            {
               i->favorite = TRUE;
               break;
            }
         }
         counter++;
      }
      else break;
   }
      
   Parent::onPreRender();
}

static char buffer[256];
char* FGHInventory::getCellText(GFXSurface *sfc, const Point2I &cell, const Point2I &cellOffset, const Point2I &cellExtent)
{
   switch (cell.x)
   {
      case ILFavorite:
         if (inventory[cell.y].favorite)
         {
            //favorite
            Point2I bmpOffset;
            bmpOffset.x = cellOffset.x + 5;
            bmpOffset.y = cellOffset.y + (cellExtent.y - statusFavorite->getHeight()) / 2 + 1;
            sfc->drawBitmap2d(statusFavorite, &bmpOffset);
            //bmpOffset.x += bmp->getWidth() + 3;
         }
         return NULL;
         
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
	char buff[16];
	sprintf(buff,"%d",inventory[selectedCell.y].index);
   Console->executef(4, "remoteEval","2048","dropItem", buff);
}

void FGHInventory::useSelected(void)
{
   if (selectedCell.y < 0 || selectedCell.y >= inventory.size()) return;
	char buff[16];
	sprintf(buff,"%d",inventory[selectedCell.y].index);
   Console->executef(4, "remoteEval","2048","useItem", buff);
}

void FGHInventory::sellSelected(void)
{
   if (selectedCell.y < 0 || selectedCell.y >= inventory.size()) return;
	char buff[16];
	sprintf(buff,"%d",inventory[selectedCell.y].index);
   Console->executef(4, "remoteEval","2048","sellItem", buff);
}

void FGHInventory::setFavorites(void)
{
   char buf[256];
   int counter = 0;
   Vector<InventoryRep>::iterator i;
   for (i = inventory.begin(); i < inventory.end(); i++)
   {
      sprintf(buf, "$pref::itemFavorite%d = \"%s\";", counter, i->name);
      Console->evaluate(buf, false);
      counter++;
   }
   
   //now clear the unused items
   while (1)
   {
      sprintf(buf, "pref::itemFavorite%d", counter);
      const char *fav = Console->getVariable(buf);
      if (fav && fav[0])
      {
         sprintf(buf, "$pref::itemFavorite%d = \"\";", counter);
         Console->evaluate(buf, false);
         counter++;
      }
      else break;
   }
}

void FGHInventory::onMouseDown(const SimGui::Event &event)
{
   Parent::onMouseDown(event);
   if (event.mouseDownCount > 1)
   {
      if (selectedCell.y == prevSelected.y)
      {
         //find out whether use or sell should be used
         FearGui::FGUniversalButton *ctrl = (FearGui::FGUniversalButton*)findRootControlWithTag(IDCTG_INV_USE);
         if (ctrl) useSelected();
         
         ctrl = (FearGui::FGUniversalButton*)findRootControlWithTag(IDCTG_INV_SELL);
         if (ctrl) sellSelected();
      }
   }
   prevSelected = selectedCell;
}

bool FGHInventory::cellSelected(Point2I cell)
{
   Parent::cellSelected(cell);
   if (cell.y >= 0 && cell.y < inventory.size())
   {
      ShapeView *view = dynamic_cast<ShapeView*>(findRootControlWithTag(IDCTG_INV_ITEM_SHAPE_VIEW));
      if (view)
      {
         view->loadShape(inventory[cell.y].shape);
      }
      
      FGHBuySell *buysell = dynamic_cast<FGHBuySell*>(findRootControlWithTag(IDCTG_INV_AVAILABLE));
      if (buysell)
      {
         buysell->cellSelected(Point2I(0, -1));
      }
      return TRUE;
   }
   return FALSE;
}


// ------------------------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(FGHBuySell, FOURCC('F','G','b','s'));

enum {
   
   BSFavorite = 0,
   BSItem,
   BSCost,
   BSCount
};

static FGArrayCtrl::ColumnInfo gBSInventoryInfo[BSCount] =
{
   { -1,                      10, 300,   0,   0,    TRUE,  25, NULL, NULL },
   { IDSTR_INV_AVAIL_ITEM,    10, 300,   1,   1,    TRUE, 152, NULL, NULL },
   { IDSTR_INV_COST,          10, 300,   2,   2,    TRUE,  60, NULL, NULL },
};                                       
static FGArrayCtrl::ColumnInfo *gBSInfoPtrs[BSCount];

bool FGHBuySell::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
	numColumns = BSCount;
	columnInfo = gBSInfoPtrs;
   
   //set the ptrs table
   for (int i = 0; i < BSCount; i++)
   {
      gBSInfoPtrs[i] = &gBSInventoryInfo[i];
   }
   
   //set the cell dimensions
   int width = 0;
   for (int k = 0; k < BSCount; k++)
   {
      width += gBSInfoPtrs[k]->width;
   }
   cellSize.set(width, hFont->getHeight() + 6);
      
   return true;
}

void FGHBuySell::onWake()
{
}

void FGHBuySell::onPreRender()
{
   if (! cg.player)
   {
      setSize(Point2I( 1, 0));
      return;
   }

   //
   inventory.clear();
	int maxType = cg.dbm->getDataGroupSize(DataBlockManager::ItemDataType);
   for (int j = 0; j < maxType; j++)
		if (cg.psc->isItemShoppingOn(j))
	   {
	      InventoryRep item;
			Item::ItemData* data = static_cast<Item::ItemData*>
				(cg.dbm->lookupDataBlock(j,DataBlockManager::ItemDataType));
			if (data->showInventory) {
		      strncpy(item.name, data->description , MaxInvNameLength);
		      item.name[MaxInvNameLength] = '\0';
		      strncpy(item.shape, data->fileName , MaxInvNameLength);
	         item.shape[MaxInvNameLength] = '\0';
            if ((! strstr(item.shape, ".dts")) && (strlen(item.shape) < MaxInvNameLength - 4))
            {
               sprintf(&item.shape[strlen(item.shape)], ".dts");
            }
		      item.count = cg.psc->itemCount(j);
		      item.index = j;
		      item.cost = data->price;
	         item.favorite = FALSE;
		      inventory.push_back(item);
			}
	   }

  	if (inventory.size() != size.y)
		setSize(Point2I(1, inventory.size()));
      
   //now toggle the favorite flag   
   char buf[256];
   int counter = 0;
   while (1)
   {
      sprintf(buf, "pref::itemFavorite%d", counter);
      const char *fav = Console->getVariable(buf);
      if (fav && fav[0])
      {
         //see if it's for sale...
         Vector<InventoryRep>::iterator i;
         for (i = inventory.begin(); i < inventory.end(); i++)
         {
            if (! stricmp(fav, i->name))
            {
               i->favorite = TRUE;
               break;
            }
         }
         counter++;
      }
      else break;
   }
      
   Grandparent::onPreRender();
}

static char BSbuffer[256];
char* FGHBuySell::getCellText(GFXSurface *sfc, const Point2I &cell, const Point2I &cellOffset, const Point2I &cellExtent)
{
   switch (cell.x)
   {
      case BSFavorite:
         if (inventory[cell.y].favorite)
         {
            //favorite
            Point2I bmpOffset;
            bmpOffset.x = cellOffset.x + 5;
            bmpOffset.y = cellOffset.y + (cellExtent.y - statusFavorite->getHeight()) / 2 + 1;
            sfc->drawBitmap2d(statusFavorite, &bmpOffset);
            //bmpOffset.x += bmp->getWidth() + 3;
         }
         return NULL;
      
      case BSItem:
         strcpy(BSbuffer, inventory[cell.y].name);
         break;
         
      case BSCost:
         sprintf(BSbuffer, "%d", inventory[cell.y].cost);
         break;
   }
   return BSbuffer;
}

void FGHBuySell::buySelected(void)
{
   if (selectedCell.y < 0 || selectedCell.y >= inventory.size()) return;
	char buff[16];
	sprintf(buff,"%d",inventory[selectedCell.y].index);
   Console->executef(4, "remoteEval","2048","buyItem", buff);
}

void FGHBuySell::buyFavorites(void)
{
   //char buf[256];
   //int counter = 0;
   Console->evaluate("buyFavorites();", FALSE);
/*   
   //buy the favorites
   while (1)
   {
      sprintf(buf, "pref::itemFavorite%d", counter);
      const char *fav = Console->getVariable(buf);
      if (fav && fav[0])
      {
         //see if it's for sale...
         Vector<InventoryRep>::iterator i;
         for (i = inventory.begin(); i < inventory.end(); i++)
         {
            if (! stricmp(fav, i->name))
            {
               char idx[16];
               sprintf(idx, "%d", i->index);
               Console->executef(4, "remoteEval","2048","buyItem", idx);
               break;
            }
         }
         counter++;
      }
      else break;
   }
*/
}

void FGHBuySell::onMouseDown(const SimGui::Event &event)
{
   Grandparent::onMouseDown(event);
   if (event.mouseDownCount > 1)
   {
      if (selectedCell.y == prevSelected.y)
      {
         buySelected();
      }
   }
   prevSelected = selectedCell;
}

bool FGHBuySell::cellSelected(Point2I cell)
{
   Grandparent::cellSelected(cell);
   if (cell.y >= 0 && cell.y < inventory.size())
   {
      ShapeView *view = dynamic_cast<ShapeView*>(findRootControlWithTag(IDCTG_INV_ITEM_SHAPE_VIEW));
      if (view)
      {
         view->loadShape(inventory[cell.y].shape);
      }
      
      FGHInventory *inv = dynamic_cast<FGHInventory*>(findRootControlWithTag(IDCTG_INV_CURRENT));
      if (inv)
      {
         inv->cellSelected(Point2I(0, -1));
      }
      return TRUE;
   }
   return FALSE;
}

};
