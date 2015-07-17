#include "PlayerManager.h"
#include "FearCSDelegate.h"
#include "netCSDelegate.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "FearGuiFilters.h"
#include "m_qsort.h"
#include "FearGuiArrayCtrl.h"
#include "FearGuiServerDetail.h"
#include "console.h"
#include "g_font.h"

namespace FearGui
{

static int __cdecl playerNameAscend(const void *a,const void *b)
{
   FearCSDelegate::PlayerInfo **entry_A = (FearCSDelegate::PlayerInfo **)(a);
   FearCSDelegate::PlayerInfo **entry_B = (FearCSDelegate::PlayerInfo **)(b);
   return (stricmp((*entry_A)->name, (*entry_B)->name));
} 

static int __cdecl playerNameDescend(const void *a,const void *b)
{
   FearCSDelegate::PlayerInfo **entry_A = (FearCSDelegate::PlayerInfo **)(a);
   FearCSDelegate::PlayerInfo **entry_B = (FearCSDelegate::PlayerInfo **)(b);
   return (stricmp((*entry_B)->name, (*entry_A)->name));
} 

static int __cdecl playerTeamAscend(const void *a,const void *b)
{
   FearCSDelegate::PlayerInfo **entry_A = (FearCSDelegate::PlayerInfo **)(a);
   FearCSDelegate::PlayerInfo **entry_B = (FearCSDelegate::PlayerInfo **)(b);
   return (stricmp((*entry_A)->team, (*entry_B)->team));
} 

static int __cdecl playerTeamDescend(const void *a,const void *b)
{
   FearCSDelegate::PlayerInfo **entry_A = (FearCSDelegate::PlayerInfo **)(a);
   FearCSDelegate::PlayerInfo **entry_B = (FearCSDelegate::PlayerInfo **)(b);
   return (stricmp((*entry_B)->team, (*entry_A)->team));
} 

static int __cdecl scoreAscend(const void *a,const void *b)
{
   FearCSDelegate::PlayerInfo **entry_A = (FearCSDelegate::PlayerInfo **)(a);
   FearCSDelegate::PlayerInfo **entry_B = (FearCSDelegate::PlayerInfo **)(b);
   return ((*entry_A)->score - (*entry_B)->score);
} 

static int __cdecl scoreDescend(const void *a,const void *b)
{
   FearCSDelegate::PlayerInfo **entry_A = (FearCSDelegate::PlayerInfo **)(a);
   FearCSDelegate::PlayerInfo **entry_B = (FearCSDelegate::PlayerInfo **)(b);
   return ((*entry_B)->score - (*entry_A)->score);
} 

static int __cdecl pingAscend(const void *a,const void *b)
{
   FearCSDelegate::PlayerInfo **entry_A = (FearCSDelegate::PlayerInfo **)(a);
   FearCSDelegate::PlayerInfo **entry_B = (FearCSDelegate::PlayerInfo **)(b);
   return ((*entry_A)->ping - (*entry_B)->ping);
} 

static int __cdecl pingDescend(const void *a,const void *b)
{
   FearCSDelegate::PlayerInfo **entry_A = (FearCSDelegate::PlayerInfo **)(a);
   FearCSDelegate::PlayerInfo **entry_B = (FearCSDelegate::PlayerInfo **)(b);
   return ((*entry_B)->ping - (*entry_A)->ping);
} 

enum {
   
   SDName = 0,
   SDTeam,
   SDScore,
   SDPing,
   SDCount
};

static FGArrayCtrl::ColumnInfo gPlayerInfo[SDCount] =
{
   { IDSTR_SD_PLAYER,          10, 300,   0,   0,    TRUE,   140, playerNameAscend, playerNameDescend },
   { IDSTR_SD_TEAM,            10, 300,   1,   1,    TRUE,   130, playerTeamAscend, playerTeamDescend },
   { IDSTR_SD_SCORE,           10, 300,   2,   2,    TRUE,    50, scoreAscend, scoreDescend },
   { IDSTR_SD_PING,            10, 300,   3,   3,    TRUE,    45, pingAscend, pingDescend },
};
static FGArrayCtrl::ColumnInfo *gInfoPtrs[SDCount];

static const char *sortColumnVariable = "pref::SDSortColumn";
static const char *sortAscendVariable = "pref::SDSortAscend";

int ServerDetailCtrl::prefColumnToSort = -1;
bool ServerDetailCtrl::prefSortAscending = TRUE;
   
IMPLEMENT_PERSISTENT_TAG(ServerDetailCtrl, FOURCC('F','G','s','d'));

bool ServerDetailCtrl::cellSelectable(const Point2I &)
{
   return FALSE;
}

bool ServerDetailCtrl::cellSelected(Point2I)
{
   return FALSE;
}

bool ServerDetailCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //set the delegate
   delegate = (FearCSDelegate *) manager->findObject(SimCSDelegateId);

   //set the local vars
   refresh = TRUE;
   columnToResize = -1;
	numColumns = SDCount;
	columnInfo = gInfoPtrs;
   
   //pref vars
	Console->addVariable(0, sortAscendVariable, CMDConsole::Bool, &prefSortAscending);
	Console->addVariable(0, sortColumnVariable, CMDConsole::Int, &prefColumnToSort);
   
   //setup the ptrs table
   for (int j = 0; j < SDCount; j++)
   {
      gInfoPtrs[j] = &gPlayerInfo[j];
   }
   
   //set the cell dimensions
   int width = 0;
   for (int i = 0; i < SDCount; i++)
   {
      if (gInfoPtrs[i]->active)
      {
         width += gInfoPtrs[i]->width;
      }
   }
   cellSize.set(width, hFont->getHeight() + 6);
   
   return true;
}

void ServerDetailCtrl::onWake()
{
   //set the player list index
   playerListIndex = -1;
   const char *addr = Console->getVariable("Server::Address");
   if (delegate && addr && addr[0])
   {
      delegate->getPlayerList(playerListIndex, addr);
   }
   
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   setSize(Point2I( 1, 0));
   refresh = TRUE;
   mNumInfoPackets = 0;
}

void ServerDetailCtrl::onPreRender()
{
   //keep the pref vars up to date
   prefColumnToSort = columnToSort;
   prefSortAscending = sortAscending;
   
   Vector<FearCSDelegate::PlayerInfo> *playerList = delegate->getPlayerList(playerListIndex, NULL);
   if (! playerList) refresh = FALSE;
   else if (playerList->size() != size.y) refresh = TRUE;
   
   //see if we need to refresh the list
   if (refresh)
   {
      players.clear();
      playerPtrs.clear();
      selectedCell.set(-1, -1);
   
      //set the size
      setSize(Point2I(1, playerList->size()));
      
      Vector<FearCSDelegate::PlayerInfo>::iterator i;
      for (i = playerList->begin(); i != playerList->end(); i++)
      {
         //copy the player into the players list
         players.increment();
         new(&players.last()) FearCSDelegate::PlayerInfo;
         FearCSDelegate::PlayerInfo *newPlayer = &players.last();
         *newPlayer = *i;
      }
      
      //do it *after* the servers *resizeable - and moveable* vector is complete
      for (int j = 0; j < size.y; j++)
      {
         playerPtrs.push_back(&players[j]);
      }
      
      //resort
      if (columnToSort >= 0)
      {
         //sort the column
         if (sortAscending && gInfoPtrs[columnToSort]->sortAscend)
         {
            m_qsort((void *)&playerPtrs[0], playerPtrs.size(),
                           sizeof(FearCSDelegate::PlayerInfo *),
                           gInfoPtrs[columnToSort]->sortAscend);
         }
         else if ((! sortAscending) && gInfoPtrs[columnToSort]->sortDescend)
         {
            m_qsort((void *)&playerPtrs[0], playerPtrs.size(),
                           sizeof(FearCSDelegate::PlayerInfo *),
                           gInfoPtrs[columnToSort]->sortDescend);
         }
      }
      refresh = FALSE;
   }
   Parent::onPreRender();
}

static char buffer[256];
char* ServerDetailCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   switch (cell.x)
   {
      case SDName:
         //SERVER NAME
         {
            char *name = playerPtrs[cell.y]->name;
            if (name[0]) return name;
            else return "N/A";
         }
         
      case SDTeam:
         //MISSION NAME
         {
            char *team = playerPtrs[cell.y]->team;
            if (team[0]) return team;
            else return "N/A";
         }
         
      case SDScore:
         //PING
         sprintf(buffer, "%d", playerPtrs[cell.y]->score);
         return &buffer[0];
         
      case SDPing:
         //Players
         {
            int ping = playerPtrs[cell.y]->score;
            if (ping >= 0)
            {
               sprintf(buffer, "%d", playerPtrs[cell.y]->ping);
               return &buffer[0];
            }
            else return "N/A";
         }
   }
   return NULL;
}

};
