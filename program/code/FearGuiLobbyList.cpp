#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "FearCSDelegate.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "fearGlobals.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "m_qsort.h"
#include "console.h"
#include "simResource.h"
#include "g_font.h"
#include "fearGuiLobbyList.h"

namespace FearGui
{

static int __cdecl playerNameAscend(const void *a,const void *b)
{
   PlayerManager::ClientRep **entry_A = (PlayerManager::ClientRep **)(a);
   PlayerManager::ClientRep **entry_B = (PlayerManager::ClientRep **)(b);
   return (stricmp((*entry_A)->name, (*entry_B)->name));
} 

static int __cdecl playerNameDescend(const void *a,const void *b)
{
   PlayerManager::ClientRep **entry_A = (PlayerManager::ClientRep **)(a);
   PlayerManager::ClientRep **entry_B = (PlayerManager::ClientRep **)(b);
   return (stricmp((*entry_B)->name, (*entry_A)->name));
} 

static int __cdecl scoreAscend(const void *a,const void *b)
{
   PlayerManager::ClientRep **entry_A = (PlayerManager::ClientRep **)(a);
   PlayerManager::ClientRep **entry_B = (PlayerManager::ClientRep **)(b);
   return ((*entry_A)->scoreSortValue - (*entry_B)->scoreSortValue);
} 

static int __cdecl scoreDescend(const void *a,const void *b)
{
   PlayerManager::ClientRep **entry_A = (PlayerManager::ClientRep **)(a);
   PlayerManager::ClientRep **entry_B = (PlayerManager::ClientRep **)(b);
   return ((*entry_B)->scoreSortValue - (*entry_A)->scoreSortValue);
} 

static int __cdecl teamAscend(const void *a,const void *b)
{
   PlayerManager::ClientRep **entry_A = (PlayerManager::ClientRep **)(a);
   PlayerManager::ClientRep **entry_B = (PlayerManager::ClientRep **)(b);
   if ((*entry_A)->team == (*entry_B)->team)
   {
      return ((*entry_A)->scoreSortValue - (*entry_B)->scoreSortValue);
   }
   else
   {
      return ((*entry_A)->team - (*entry_B)->team);
   }
} 

static int __cdecl teamDescend(const void *a,const void *b)
{
   PlayerManager::ClientRep **entry_A = (PlayerManager::ClientRep **)(a);
   PlayerManager::ClientRep **entry_B = (PlayerManager::ClientRep **)(b);
   if ((*entry_A)->team == (*entry_B)->team)
   {
      return ((*entry_A)->scoreSortValue - (*entry_B)->scoreSortValue);
   }
   else
   {
      return ((*entry_B)->team - (*entry_A)->team);
   }
} 

enum {
   
   LLName = 0,
   LLTeam,
   LLScore,
   LLCount
};

static FGArrayCtrl::ColumnInfo gPlayerListInfo[LLCount] =
{
   { IDSTR_SD_PLAYER,            10, 300,   0,   0,    TRUE, 200, playerNameAscend, playerNameDescend },
   { IDSTR_SD_TEAM,              10, 300,   1,   1,    TRUE, 200, teamDescend, teamAscend },
   { IDSTR_SD_SCORE,             10, 300,   2,   2,    TRUE,  60, scoreAscend, scoreDescend },
};
static FGArrayCtrl::ColumnInfo *gInfoPtrs[LLCount];

IMPLEMENT_PERSISTENT_TAG(FGLobbyList, FOURCC('F','G','l','l'));

bool FGLobbyList::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //other misc vars
	columnsResizeable = FALSE;
	columnsSortable = TRUE;
	numColumns = LLCount;
	columnInfo = gInfoPtrs;
   
   //set the cell dimensions
   int width = 0;
   for (int j = 0; j < LLCount; j++)
   {
      gInfoPtrs[j] = &gPlayerListInfo[j];
      width += gPlayerListInfo[j].width;
   }
   cellSize.set(width, hFont->getHeight() + 6);
   
   return true;
}

void FGLobbyList::onWake()
{
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   setSize(Point2I( 1, 0));
   refresh = TRUE;
}

void FGLobbyList::buildPlayerDisplayList(PlayerManager::ClientRep *cl)
{
   while(cl)
   {
      if(cl->commander)
         cl->commandLevel = cl->commander->commandLevel + 1;
      else
         cl->commandLevel = 0;

      displayPlayers[displayPlayersCount++] = cl;
   
      buildPlayerDisplayList(cl->firstPeon);
      cl = cl->nextPeon;
   }
}

void FGLobbyList::onPreRender()
{
   if (! cg.playerManager) return;
   PlayerManager::ClientRep *me = cg.playerManager->findClient(manager->getId());
   if(!me)
      return;

   displayPlayersCount = 0;
   cg.playerManager->buildCommandTrees();
   for (int i = 0; i < cg.playerManager->getNumTeams(); i++)
   {
      PlayerManager::TeamRep *team = cg.playerManager->findTeam(i);
      buildPlayerDisplayList(team->firstCommander);
   }

   setSize(Point2I(1, displayPlayersCount));
      
   //sort
   if (columnToSort < 0) return;
   
   if (sortAscending && gPlayerListInfo[columnToSort].sortAscend)
   {
      m_qsort((void *)&displayPlayers[0], displayPlayersCount,
                     sizeof(PlayerManager::ClientRep *),
                     gPlayerListInfo[columnToSort].sortAscend);
   }
   else if (gPlayerListInfo[columnToSort].sortDescend)
   {
      m_qsort((void *)&displayPlayers[0], displayPlayersCount,
                     sizeof(PlayerManager::ClientRep *),
                     gPlayerListInfo[columnToSort].sortDescend);
   }
}

char* FGLobbyList::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   if (! cg.playerManager) return NULL;
   
   char buffer[256];
   switch (cell.x)
   {
      case LLName:
         return (char*)(displayPlayers[cell.y]->name);
         
      case LLTeam:
      {
         PlayerManager::TeamRep *team = cg.playerManager->findTeam(displayPlayers[cell.y]->team);
         return (char*)(team->name);
      }
   
      case LLScore:
         return displayPlayers[cell.y]->scoreString;
   }
   return NULL;
}

};
