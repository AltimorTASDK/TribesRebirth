#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "FearCSDelegate.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "fear.strings.h"
#include "FearGuiMasterList.h"
#include "FearGuiShellPal.h"
#include "m_qsort.h"
#include "console.h"
#include "g_barray.h"
#include "g_font.h"

namespace SimGui {
extern Control *findControl(const char *name);
};

namespace FearGui
{

enum {
   
   MSName = 0,
   MSAddr,
   MSCount
};

static int __cdecl masterNameAscend(const void *a,const void *b)
{
   MasterServerList::MasterRep **entry_A = (MasterServerList::MasterRep **)(a);
   MasterServerList::MasterRep **entry_B = (MasterServerList::MasterRep **)(b);
   return (stricmp((*entry_A)->name, (*entry_B)->name));
} 

static FGArrayCtrl::ColumnInfo gColumnInfo[MSCount] =
{
   { IDSTR_SI_MASTER_SERVER,   10, 300,   0,   0,    TRUE,  122, NULL, NULL },
   { IDSTR_SI_ADDRESS,         10, 300,   1,   1,    TRUE,  122, NULL, NULL },
};
static FGArrayCtrl::ColumnInfo *gInfoPtrs[MSCount];                                         

IMPLEMENT_PERSISTENT_TAG(MasterServerList, FOURCC('F','G','m','s'));

static bool gScriptFunctionsAdded = FALSE;

static const char *FGMasterListClear(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MasterServerList *ac = NULL;
   if (ctrl) ac = dynamic_cast<MasterServerList *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   ac->scriptClear();
   return "true";
}

static const char *FGMasterListAddEntry(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 5)
   {
      Console->printf("%s(control, name, addr, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MasterServerList *ac = NULL;
   if (ctrl) ac = dynamic_cast<MasterServerList *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   ac->scriptAddEntry(argv[2], argv[3], argv[4]);
   return "true";
}

static const char *FGMasterListDeleteEntry(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MasterServerList *ac = NULL;
   if (ctrl) ac = dynamic_cast<MasterServerList *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   ac->scriptDeleteEntry(atoi(argv[2]));
   return "true";
}

static const char *FGMasterListSetSelected(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MasterServerList *ac = NULL;
   if (ctrl) ac = dynamic_cast<MasterServerList *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   ac->scriptSetSelected(atoi(argv[2]));
   return "true";
}

static const char *FGMasterListGetSelected(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "-1";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MasterServerList *ac = NULL;
   if (ctrl) ac = dynamic_cast<MasterServerList *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "-1";
   }
   return avar("%d", ac->scriptGetSelected());
}

bool MasterServerList::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //other misc vars
	columnsResizeable = FALSE;
	columnsSortable = FALSE;
	numColumns = MSCount;
	columnInfo = gInfoPtrs;
   
   //set the ptrs table
   for (int j = 0; j < MSCount; j++)
   {
      gInfoPtrs[j] = &gColumnInfo[j];
   }
   
   //set the cell dimensions
   int width = 0;
   for (int i = 0; i < MSCount; i++)
   {
      width += gInfoPtrs[i]->width;
   }
   cellSize.set(width, hFont->getHeight() + 6);
   
   //add the script functions
   if (! gScriptFunctionsAdded)
   {
      gScriptFunctionsAdded = TRUE;
      Console->addCommand(0, "FGMasterList::addEntry", FGMasterListAddEntry);
      Console->addCommand(0, "FGMasterList::deleteEntry", FGMasterListDeleteEntry);
      Console->addCommand(0, "FGMasterList::clear", FGMasterListClear);
      Console->addCommand(0, "FGMasterList::setSelected", FGMasterListSetSelected);
      Console->addCommand(0, "FGMasterList::getSelected", FGMasterListGetSelected);
   }
   
   return true;
}

void MasterServerList::onWake()
{
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   setSize(Point2I(1, 0));
}

bool MasterServerList::cellSelected(Point2I cell)
{
   if (cell.y < 0 || cell.y >= entryPtrs.size()) return FALSE;
   selectedCell.set(0, cell.y);
   Console->setVariable("Server::CurrentMaster", avar("%d", entryPtrs[cell.y]->id));
   Console->setVariable("Server::Master1", entryPtrs[cell.y]->address);
   return TRUE;
}
   
char* MasterServerList::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   if (cell.y >= entryPtrs.size()) return NULL;
   
   switch (cell.x)
   {
      case MSName:
         return entryPtrs[cell.y]->name;
      case MSAddr:
         return entryPtrs[cell.y]->address;
   }
   
   return NULL;
}

void MasterServerList::scriptClear(void)
{
   entries.clear();
   entryPtrs.clear();
   setSize(Point2I(1, 0));
   selectedCell.set(-1, -1);
}

void MasterServerList::scriptAddEntry(const char *name, const char *addr, const char *id)
{
	MasterRep newMaster;
   strncpy(newMaster.name, name, 255);
   newMaster.name[255] = '\0';
   strncpy(newMaster.address, addr, 255);
   newMaster.address[255] = '\0';
   newMaster.id = atoi(id);
   
   entries.push_back(newMaster);
   
   entryPtrs.clear();
   for (int i = 0; i < entries.size(); i++)
   {
      entryPtrs.push_back(&entries[i]);
   }
   if (entryPtrs.size() > 0)
   {
      m_qsort((void *)&entryPtrs[0], entryPtrs.size(),
                     sizeof(MasterServerList::MasterRep *),
                     masterNameAscend);
   }
   
   setSize(Point2I(1, entries.size()));
}

void MasterServerList::scriptDeleteEntry(int id)
{
   if (selectedCell.y >= 0 && selectedCell.y < entryPtrs.size())
   {
      for (int i = 0; i < entries.size(); i++)
      {
         if (entries[i].id == id)
         {
            entries.erase(i);
         }
      }
   }
   
   entryPtrs.clear();
   for (int i = 0; i < entries.size(); i++)
   {
      entryPtrs.push_back(&entries[i]);
   }
   if (entryPtrs.size() > 0)
   {
      m_qsort((void *)&entryPtrs[0], entryPtrs.size(),
                     sizeof(MasterServerList::MasterRep *),
                     masterNameAscend);
   }
   setSize(Point2I(1, entries.size()));
   cellSelected(Point2I(0, 0));
}

void MasterServerList::scriptSetSelected(int id)
{
   for (int i = 0; i < entryPtrs.size(); i++)
   {
      if (entryPtrs[i]->id == id)
      {
         cellSelected(Point2I(0, i));
         break;
      }
   }
}

int MasterServerList::scriptGetSelected(void)
{
   if (selectedCell.y >= 0 && selectedCell.y < entryPtrs.size())
   {
      return entryPtrs[selectedCell.y]->id;
   }
   else return -1;
}

};
