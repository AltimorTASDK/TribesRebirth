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
#include "fearguishellpal.h"
#include "fearGuiShapeView.h"

namespace SimGui {
extern Control *findControl(const char *name);
};

class FearGuiInventory : public SimGui::ArrayCtrl
{
   typedef SimGui::ArrayCtrl Parent;
   Resource<GFXFont> hFont;
   Resource<GFXFont> hFontHL;
   Resource<GFXFont> hFontNA;
   Resource<GFXFont> hFontMO;

   struct Cell
   {
      char heading[256];
      char string[256];
      int count;
      bool favorite;
      int item;
      bool enabled;
   };
   Vector<Cell> cells;
   Resource<GFXBitmap> statusFavorite;
   Point2I prevSelected;

   static bool initialized;
   static bool favorites[Player::MaxItemTypes];
   static const char *InvGetVisibleSet(CMDConsole *, int, int argc, const char **argv);
   static const char *InvSetFavorites(CMDConsole *, int, int argc, const char **argv);
   static void initStatics();

protected:
   bool checkShopping;

   void clearCells();
   void addCell(const char *heading, const char *string, int count, int item, bool enable);
public:
   bool cellSelected(Point2I cell);
   bool onAdd();
   void onWake();
   bool wantsTabListMembership();
   bool becomeFirstResponder();
   void onMouseDown(const SimGui::Event &event);
   const char *getScriptValue();
   void onPreRender();
   void checkSize();
   void onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool sel, bool mouseOver);
   DECLARE_PERSISTENT(FearGuiInventory);
};

bool FearGuiInventory::initialized = false;
bool FearGuiInventory::favorites[Player::MaxItemTypes];

const char *FearGuiInventory::InvGetVisibleSet(CMDConsole *, int, int argc, const char **argv)
{
   static char retBuffer[1024];
   char buf[24];

   if(argc != 2)
      return "";
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   if(!ctrl)
      return "";
   FearGuiInventory *inv = dynamic_cast<FearGuiInventory *>(ctrl);
   if(!inv)
      return "";
   Vector<Cell>::iterator i;
   retBuffer[0] = 0;

   for(i = inv->cells.begin(); i != inv->cells.end(); i++)
   {
      if(i->string[0])
      {
         sprintf(buf, "%d ", i->item);
         strcat(retBuffer, buf);
      }
   }
   return retBuffer;
}

const char *FearGuiInventory::InvSetFavorites(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
      return "False";
   const char *walk = argv[1];
   for(int i = 0; i < Player::MaxItemTypes; i++)
      favorites[i] = false;

   for(;;)
   {
      if(!*walk)
         return "True";
      else if(*walk == ' ')
         walk++;
      else if(*walk)
      {
         int idx = atoi(walk);
         if(idx >= 0 && idx < Player::MaxItemTypes)
            favorites[idx] = true;
         while(*walk && (*walk != ' '))
            walk++;
      }
   }
}

void FearGuiInventory::initStatics()
{
   if(initialized)
      return;
   Console->addCommand(0, "CmdInventory::getVisibleSet", InvGetVisibleSet);
   Console->addCommand(0, "CmdInventory::setFavorites", InvSetFavorites);
   for(int i = 0; i < Player::MaxItemTypes; i++)
      favorites[i] = false;
   initialized = true;
}

class FearGuiPurchase : public FearGuiInventory
{
public:
   void onPreRender();
   DECLARE_PERSISTENT(FearGuiPurchase);
};

IMPLEMENT_PERSISTENT_TAG(FearGuiInventory, FOURCC('F','g','I','d'));
IMPLEMENT_PERSISTENT_TAG(FearGuiPurchase, FOURCC('F','g','P','d'));

bool FearGuiInventory::becomeFirstResponder()
{
   return false;
}

bool FearGuiInventory::wantsTabListMembership()
{
   return false;
}

void FearGuiInventory::checkSize()
{
   if(size.y != cells.size())
      setSize(Point2I(1, cells.size()));
}

void FearGuiInventory::clearCells()
{
   cells.clear();
}

void FearGuiInventory::addCell(const char *heading, const char *string, int count, int item, bool enable)
{
   // do an insertion sort on the heading (if necessary) and the string

   Cell newCell;
   int i;
   for(i = 0; i < cells.size() && cells[i].heading[0] < heading[0]; i++)
      ;

   if(i == cells.size() || cells[i].heading[0] != heading[0])
   {
      strcpy(newCell.heading, heading);
      newCell.string[0] = 0;
      newCell.enabled = false;
      cells.insert(cells.begin() + i, newCell);
      i++;
      strcpy(newCell.string, string);
      newCell.count = count;
      newCell.item = item;
      newCell.enabled = enable;
      cells.insert(cells.begin() + i, newCell);
      return;
   }
   while(cells[i].heading[0] == heading[0] && i < cells.size())
      i++;
   strcpy(newCell.heading, heading);
   strcpy(newCell.string, string);
   newCell.count = count;
   newCell.item = item;
   newCell.enabled = enable;

   cells.insert(cells.begin() + i, newCell);
}

const char *FearGuiInventory::getScriptValue()
{
   static char buf[24];
   if(selectedCell.y < 0 || selectedCell.y >= cells.size())
      strcpy(buf, "-1");
   else
      sprintf(buf, "%d", cells[selectedCell.y].item);
   return buf;
}

bool FearGuiInventory::cellSelected(Point2I cell)
{
   if(cell.y >= 0 && cell.y < cells.size() && cells[cell.y].enabled)
   {
      Parent::cellSelected(cell);
      const char *name = getName();
      if(name)
      {
         char buf[256];
         sprintf(buf, "%s::onSelect", name);
         if(Console->isFunction(buf))
            Console->executef(2, buf, getScriptValue());
      }
      return true;
   }
   selectedCell.y = -1;
   return false;
}

void FearGuiInventory::onMouseDown(const SimGui::Event &event)
{
   Parent::onMouseDown(event);
   if(event.mouseDownCount > 1 && selectedCell.y != -1)
   {
      if(selectedCell.y == prevSelected.y)
      {
         const char *name = getName();
         if(name)
         {
            char buf[256];
            sprintf(buf, "%s::onDoubleClick", name);
            if(Console->isFunction(buf))
               Console->executef(2, buf, getScriptValue());
         }
      }
   }
   prevSelected = selectedCell;
}

void FearGuiInventory::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool sel, bool mouseOver)
{
   if(cell.y >= cells.size())
      return;

   Cell *c = &cells[cell.y];
   if(c->string[0])
   {
      // it's not a heading
      GFXFont *fnt;
      if(!c->enabled)
         fnt = hFontNA;
      else if(sel)
         fnt = hFontHL;
      else if(mouseOver)
         fnt = hFontMO;
      else
         fnt = hFont;
      sfc->drawRect2d_f(&RectI(offset.x, offset.y, offset.x + cellSize.x - 1, offset.y + cellSize.y - 1), 0);
      sfc->drawText_p(fnt, &Point2I(offset.x + 15, offset.y), c->string);
      char buf[32];
      sprintf(buf, "%d", c->count);
      sfc->drawText_p(fnt, &Point2I(offset.x + 158, offset.y), buf);

      if(favorites[c->item])
         sfc->drawBitmap2d(statusFavorite, &Point2I(offset.x + 3, offset.y + ((cellSize.y - statusFavorite->getHeight()) >> 1)));
   }
   else
   {
      sfc->drawRect2d(&RectI(offset.x-1, offset.y, offset.x + cellSize.x - 1, offset.y + cellSize.y - 1), FearGui::HILITE_COLOR);
      sfc->drawText_p(hFontHL, &Point2I(offset.x + 2, offset.y), c->heading + 1);
   }
}

void FearGuiInventory::onWake()
{
   setSize(Point2I(1, 0));
}

bool FearGuiInventory::onAdd()
{
   if(!Parent::onAdd())
      return false;

   initStatics();
   statusFavorite = SimResource::loadByName(manager, "I_Favorite.bmp", true);
   statusFavorite->attribute |= BMA_TRANSPARENT;
   hFont = SimResource::loadByTag(manager, IDFNT_9_STANDARD, true);
   hFontHL = SimResource::loadByTag(manager, IDFNT_9_SELECTED, true);
   hFontNA = SimResource::loadByTag(manager, IDFNT_9_DISABLED, true);
   hFontMO = SimResource::loadByTag(manager, IDFNT_9_HILITE, true);

   cellSize.set(300, hFont->getHeight() + 6);
   return true;
}

void FearGuiInventory::onPreRender()
{
   clearCells();
   if(!cg.psc)
      return;

	int itemTypeCount = cg.dbm->getDataGroupSize(DataBlockManager::ItemDataType);
   for (int j = 0; j < itemTypeCount; j++)
   {
      if (cg.psc->itemCount(j))
      {
			Item::ItemData* data = static_cast<Item::ItemData*>
				(cg.dbm->lookupDataBlock(j,DataBlockManager::ItemDataType));
	      if (data->showInventory)
            addCell(data->typeString, data->description, cg.psc->itemCount(j), j, true);
	   }
   }
   checkSize();
   
   setUpdate();
}

void FearGuiPurchase::onPreRender()
{
   clearCells();
   if(!cg.psc)
      return;

	int itemTypeCount = cg.dbm->getDataGroupSize(DataBlockManager::ItemDataType);
   for (int j = 0; j < itemTypeCount; j++)
   {
		if (cg.psc->isItemShoppingOn(j))
	   {
			Item::ItemData* data = static_cast<Item::ItemData*>
				(cg.dbm->lookupDataBlock(j,DataBlockManager::ItemDataType));
			if (data->showInventory)
            addCell(data->typeString, data->description, data->price, j, cg.psc->isItemBuyOn(j));
	   }
   }
   checkSize();
}