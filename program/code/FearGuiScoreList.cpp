#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "g_font.h"
#include "simResource.h"
#include "fearguishellpal.h"
#include "console.h"
#include "fearglobals.h"
#include "fearPlayerPSC.h"
#include <m_qsort.h>

namespace FearGui
{

class ScoreListCtrl : public SimGui::ArrayCtrl
{
   typedef SimGui::ArrayCtrl Parent;
   Resource<GFXFont> hFont;
   Resource<GFXFont> hFontHL;
   
   enum {
      MaxColumns = 16,
   };
   Vector<PlayerManager::TeamRep *> teams;
   Vector<PlayerManager::ClientRep *> players;
   int teamTabStops[MaxColumns];
   int playerTabStops[MaxColumns];
   int selectedClient;
   int teamHeadingLine;
   int clientHeadingLine;
   int firstTeamLine;
   int firstClientLine;
   bool drawTeamScores;

   PlayerManager *playerManager;
   
   bool mbLowRes;
   
public:
   bool onAdd();
   void onWake();
   void onPreRender();
   void onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool selected, bool mouseOver);
   bool cellSelected(Point2I cell);
   bool becomeFirstResponder();

   DECLARE_PERSISTENT(ScoreListCtrl);
};

IMPLEMENT_PERSISTENT_TAG(ScoreListCtrl, FOURCC('F','G','s','c'));

bool ScoreListCtrl::becomeFirstResponder()
{
   return false;
}


bool ScoreListCtrl::cellSelected(Point2I cell)
{
   int pidx = cell.y - firstClientLine;
   if(pidx >= 0 && pidx < players.size())
   {
      PlayerManager::ClientRep *cr = players[pidx];
      Console->evaluatef("remoteEval(2048, SelectClient, %d);", cr->id);
      selectedClient = cr->id;
   }
   return true;
}

bool ScoreListCtrl::onAdd()
{
   active = true;
   selectedClient = 0;
   if(!Parent::onAdd())
      return false;
      
   //set the player manager
   playerManager = PlayerManager::get(manager);

   //hard coded low-res version
   if (getName() && (! stricmp(getName(), "LowResScore")))
   {
      hFont = SimResource::get(manager)->load("sf_white_6.pft");
      AssertFatal(hFont.operator bool(), "Failed to load sf_white_6.pft");
      hFontHL = SimResource::get(manager)->load("sf_yellow_6.pft");
      AssertFatal(hFontHL.operator bool(), "Failed to load sf_yellow_6.pft");
      hFont->fi.flags |= FONT_LOWERCAPS;
      hFontHL->fi.flags |= FONT_LOWERCAPS;
      mbLowRes = TRUE;
   }
   else
   {
      hFont = SimResource::get(manager)->load("sf_white_10b.pft");
      AssertFatal(hFont.operator bool(), "Failed to load sf_white_10b.pft");
      hFontHL = SimResource::get(manager)->load("sf_yellow_10b.pft");
      AssertFatal(hFontHL.operator bool(), "Failed to load sf_yellow_10b.pft");
      mbLowRes = FALSE;
   }
   
   cellSize.set(640, hFont->getHeight() + 2);
   return true;
}

static int __cdecl playerScoreCompare(const void *a,const void *b)
{
   PlayerManager::ClientRep **entry_A = (PlayerManager::ClientRep **)(a);
   PlayerManager::ClientRep **entry_B = (PlayerManager::ClientRep **)(b);
   return ((*entry_B)->scoreSortValue - (*entry_A)->scoreSortValue);
} 

static int __cdecl teamScoreCompare(const void *a,const void *b)
{
   PlayerManager::TeamRep **entry_A = (PlayerManager::TeamRep **)(a);
   PlayerManager::TeamRep **entry_B = (PlayerManager::TeamRep **)(b);
   return ((*entry_B)->scoreSortValue - (*entry_A)->scoreSortValue);
} 

void ScoreListCtrl::onWake()
{
   setSize(Point2I( 1, playerManager->getNumTeams() + playerManager->getNumClients()));
}

void ScoreListCtrl::onPreRender()
{
   int stop = 1;
   teamTabStops[0] = 0;
   playerTabStops[0] = 0;

   BYTE *s = (BYTE *) playerManager->teamScoreHeading;
   while(*s)
   {
      if(*s == '\t' && *(s+1))
      {
         teamTabStops[stop++] = *(s+1);
         s+=2;
      }
      else
         s++;
   }
   teamTabStops[stop] = 640;
   
   s = (BYTE *) playerManager->clientScoreHeading;
   stop = 1;
   while(*s)
   {
      if(*s == '\t' && *(s+1))
      {
         playerTabStops[stop++] = *(s+1);
         s+=2;
      }
      else
         s++;
   }
   playerTabStops[stop] = 640;
   
   int newSize;
   int cgmode = 0;
   if(cg.psc)
      cgmode = cg.psc->getCurrentGuiMode();
   if(playerManager->teamScoreHeading[0] && cgmode != PlayerPSC::VictoryGuiMode && cgmode != PlayerPSC::ObjectiveGuiMode)
   {
      firstTeamLine = 1;
      teamHeadingLine = 0;
      clientHeadingLine = firstTeamLine + playerManager->getNumTeams();
      firstClientLine = clientHeadingLine + 1;
      newSize = playerManager->getNumTeams() + playerManager->getNumClients() + 2;
   }
   else
   {
      firstTeamLine = -1;
      teamHeadingLine = -1;
      clientHeadingLine = 0;
      firstClientLine = 1;
      newSize = playerManager->getNumClients() + 1;
   }
   if(newSize != size.y)
      setSize(Point2I(1, newSize));
      
   //clear the vectors
   teams.clear();
   players.clear();
   
   int i;
   for(i = 0; i < playerManager->getNumTeams(); i++)
      teams.push_back(playerManager->findTeam(i));

   //sort the teams by score
   m_qsort((void *)&teams[0], teams.size(), sizeof(PlayerManager::TeamRep *), teamScoreCompare);

   for(PlayerManager::ClientRep *j = playerManager->getClientList(); j; j = j->nextClient)
      players.push_back(j);

   //sort the teams by score
   m_qsort((void *)&players[0], players.size(), sizeof(PlayerManager::ClientRep *), playerScoreCompare);
}

void ScoreListCtrl::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool, bool)
{
   char buf[1024];

   bool tabSkipOne = false;
   int *tabStops;
   buf[0] = 0;
   bool selected = false;

   if(cell.y >= firstClientLine)
   {
      tabStops = playerTabStops;
      int pidx = cell.y - firstClientLine;
      const char *s = players[pidx]->scoreString;
      if(players[pidx]->id == selectedClient)
         selected = true;
      char *bufptr = buf;

      while(*s)
      {
         if(*s == '%' && *(s+1) == 'n')
         {
            strcpy(bufptr, players[pidx]->name);
            bufptr += strlen(bufptr);
            s += 2;
         }
         else if(*s == '%' && *(s+1) == 't')
         {
            int team = players[pidx]->team;
            if(team != -1)
            {
               // this is BAD!
               PlayerManager::TeamRep *teamRep = playerManager->teamList + (team+1);
               strcpy(bufptr, teamRep->name);
               bufptr += strlen(bufptr);
            }
            s += 2;
         }
         else if(*s == '%' && *(s+1) == 'p')
         {
            sprintf(bufptr, "%d", players[pidx]->ping);
            bufptr += strlen(bufptr);
            s += 2;
         }
         else if(*s == '%' && *(s+1) == 'l')
         {
            sprintf(bufptr, "%d", int(players[pidx]->packetLoss * 100));
            bufptr += strlen(bufptr);
            s += 2;
         }
         else
            *bufptr++ = *s++;
      }
      *bufptr = 0;
   }
   else if (cell.y == clientHeadingLine)
   {
      tabStops = playerTabStops;
      tabSkipOne = true;
      strcpy(buf, playerManager->clientScoreHeading);
   }
   else if (cell.y >= firstTeamLine)
   {
      tabStops = teamTabStops;
      int team = cell.y - firstTeamLine;
      const char *s = teams[team]->scoreString;
      char *bufptr = buf;

      while(*s)
      {
         if(*s == '%' && *(s+1) == 't')
         {
            strcpy(bufptr, teams[team]->name);
            bufptr += strlen(bufptr);
            s += 2;
         }
         else
            *bufptr++ = *s++;
      }
      *bufptr = 0;
   }
   else if (cell.y == teamHeadingLine)
   {
      // team heading
      tabStops = teamTabStops;
      tabSkipOne = true;
      strcpy(buf, playerManager->teamScoreHeading);
   }


   char *s = buf;
   int stop = 0;
   if(selected)
      sfc->drawRect2d_f(&RectI(offset.x, offset.y, offset.x + 640, offset.y + hFont->getHeight() + 2), GREEN_78);
   for(;;)
   {
      int tabStop = (! mbLowRes ? tabStops[stop] : tabStops[stop] * 70 / 100);
      
      char buft[256];
      int i = 0;
      while(*s && *s != '\t')
         buft[i++] = *s++;
      buft[i] = 0;
      if(i)
      {
         int nextTabStop = (! mbLowRes ? tabStops[stop + 1] : tabStops[stop + 1] * 70 / 100);
         RectI cr = *sfc->getClipRect();
         sfc->setClipRect(
            &RectI(max(offset.x + tabStop, cr.upperL.x),
                   max(offset.y, cr.upperL.y),
                   min(offset.x + nextTabStop - 2, cr.lowerR.x),
                   min(offset.y + 2 * hFont->getHeight(), cr.lowerR.y) ) );

         sfc->drawText_p((tabSkipOne ? hFont : hFontHL), &Point2I(offset.x + 2 + tabStop, offset.y - (! mbLowRes ? 2 : 1)), buft);
         sfc->setClipRect(&cr);
      }
      if(*s == '\t' && *(s+1) && tabSkipOne)
         s += 2;
      else if(*s == '\t')
         s++;

      if(!*s)
         break;
      stop++;
   }
}

};
