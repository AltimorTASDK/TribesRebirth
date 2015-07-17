#include "simGuiArrayCtrl.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "FearGuiPlayerList.h"
#include "FearGuiPlayerCombo.h"
#include "m_qsort.h"
#include "console.h"

namespace FearGui
{                    

IMPLEMENT_PERSISTENT_TAG(PlayerListCtrl, FOURCC('F','G','p','l'));

static int __cdecl playerNameAscend(const void *a,const void *b)
{
   PlayerListCtrl::ClientRep **entry_A = (PlayerListCtrl::ClientRep **)(a);
   PlayerListCtrl::ClientRep **entry_B = (PlayerListCtrl::ClientRep **)(b);
   return (stricmp((*entry_A)->name, (*entry_B)->name));
} 

//also defined in FearGuiPlayerCombo.cpp
static const char *gPlayerName = "pref::PlayerName";
static const char *gPlayerGender = "pref::PlayerGender";
static const char *gPlayerVoice = "pref::PlayerVoice";
static const char *gPlayerSkinBase = "pref::PlayerSkinBase";

static const char gPlayerExtention[] = "players\\*.cs";
static const char gPlayerDir[] = "players\\";

//console function members
void PlayerListCtrl::addEntry(const char *buf, int id)
{
   ClientRep newPlayer;
   newPlayer.consoleIndex = id;
   strncpy(newPlayer.name, buf, MaxClientNameLength);
   newPlayer.name[MaxClientNameLength] = '\0';
   players.push_back(newPlayer);
   
   //resort
   playerPtrs.clear();
   for (int j = 0; j < players.size(); j++)
   {
      playerPtrs.push_back(&players[j]);
   }
   m_qsort((void *)&playerPtrs[0], playerPtrs.size(),
                  sizeof(PlayerListCtrl::ClientRep *),
                  playerNameAscend);
   
   //set the size and selected cell
   setSize(Point2I(1, playerPtrs.size()));
   setSelected(id);
}

void PlayerListCtrl::deleteEntry(int id)
{
   for (int i = 0; i < playerPtrs.size(); i++)
   {
      if (players[i].consoleIndex == id)
      {
         players.erase(i);
         break;
      }
   }
   
   //resort
   playerPtrs.clear();
   for (int j = 0; j < players.size(); j++)
   {
      playerPtrs.push_back(&players[j]);
   }
   m_qsort((void *)&playerPtrs[0], playerPtrs.size(),
                  sizeof(PlayerListCtrl::ClientRep *),
                  playerNameAscend);
   
   //set the size and selected cell
   setSize(Point2I(1, playerPtrs.size()));
   selectedCell.set(-1, -1);
}

void PlayerListCtrl::clear(void)
{
   players.clear();
   playerPtrs.clear();
   selectedCell.set(-1, -1);
}

int PlayerListCtrl::getSelectedEntry(void)
{
   if (selectedCell.y < 0) return -1;
   else return playerPtrs[selectedCell.y]->consoleIndex;
}

void PlayerListCtrl::setSelectedEntry(int id)
{
   for (int i = 0; i < playerPtrs.size(); i++)
   {
      if (playerPtrs[i]->consoleIndex == id)
      {
         selectedCell.set(0, i);
         break;
      }
   }
}

bool PlayerListCtrl::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   setTag(IDCTG_PLYR_CFG_LIST);
   
   players.clear();
   playerPtrs.clear();
   
   setSize(Point2I(1, 0));
   
   return TRUE;
}
 
bool PlayerListCtrl::cellSelected(Point2I cell)
{
   if (cell.y < 0 || cell.y >= playerPtrs.size()) return FALSE;
   Parent::cellSelected(cell);
   Console->executef(1, "SelectePlayerConfig");
   return TRUE;
}

const char* PlayerListCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   if (cell.y < players.size())
   {
      return &(playerPtrs[cell.y]->name[0]);
   }
   else
   {
      return NULL;
   }
}

const char *PlayerListCtrl::getSelectedPlayer(void)
{
   if (selectedCell.y >= 0)
   {
      return playerPtrs[selectedCell.y]->name;
   }
   return NULL;
}

};
