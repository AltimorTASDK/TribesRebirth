#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "simResource.h"
#include <g_font.h>
#include "fearGuiShellPal.h"
#include "fearGuiCommandTeamList.h"
#include "fearGuiTSCommander.h"
#include "fearGlobals.h"
#include "player.h"
#include "console.h"

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG(CommandTeamCtrl, FOURCC('F','G','c','T'));

void CommandTeamCtrl::setLowRes(bool value)
{
   mLowRes = value;
   if (! mLowRes)
      cellSize.set(640, hFont->getHeight() + 2);
   else
      cellSize.set(320, lr_hFont->getHeight() + 1);
}

void CommandTeamCtrl::onWake()
{
   Parent::onWake();
   setSize(Point2I(1,0));
   selectedCell.set(-1, -1);
}

bool CommandTeamCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   //set the player manager
   playerManager = PlayerManager::get(manager);

   //get the fonts
   hFont = SimResource::loadByTag(manager, IDFNT_HUD_10_HILITE, true);
   hFontWaitAckge = SimResource::loadByTag(manager, IDFNT_HUD_10_SPECIAL, true);
   hFontMuted = SimResource::loadByTag(manager, IDFNT_HUD_10_OTHER, true);
   lr_hFont = SimResource::loadByTag(manager, IDFNT_HUD_6_HILITE, true);
   lr_hFontWaitAckge = SimResource::loadByTag(manager, IDFNT_HUD_6_SPECIAL, true);
   
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
   
   //load the command status icons
   bmpCMDNone = SimResource::get(manager)->load("CMD_OrderNone.bmp");
   AssertFatal(bmpCMDNone, "Unable to load CMD_OrderNone.bmp");
   bmpCMDNone->attribute |= BMA_TRANSPARENT;
   
   bmpCMDPending = SimResource::get(manager)->load("CMD_OrderPending.bmp");
   AssertFatal(bmpCMDPending, "Unable to load CMD_OrderPending.bmp");
   bmpCMDPending->attribute |= BMA_TRANSPARENT;
   
   bmpCMDAcknowledged = SimResource::get(manager)->load("CMD_OrderAck.bmp");
   AssertFatal(bmpCMDAcknowledged, "Unable to load CMD_OrderAck.bmp");
   bmpCMDAcknowledged->attribute |= BMA_TRANSPARENT;
   
   //load the pba
   bma = SimResource::loadByTag(manager, IDPBA_COMMAND_LIST, true);
   lr_bma = SimResource::loadByTag(manager, IDPBA_LR_COMMAND_LIST, true);

   AssertFatal((bool)bma && (bool)lr_bma, "Unable to load bma or lr_bma");

   for (int i = 0; i < BmpCount; i++)
   {
      bma->getBitmap(i)->attribute |= BMA_TRANSPARENT;
      lr_bma->getBitmap(i)->attribute |= BMA_TRANSPARENT;
   }
   //set the lo res bool
   mLowRes = FALSE;
   
   prevSelectedCell.set(-1, -1);
   
   cellSize.set(640, hFont->getHeight() + 2);
   return true;
}

void CommandTeamCtrl::setBranchSelected(PlayerManager::ClientRep *head, bool value)
{
   //sanity check
   if (! head) return;
   
   //set the head
   cg.commandTS->setSelectedPlayer(head->id, value);
   
   //loop through the list of peons, and set each branch
   while (head)
   {
      setBranchSelected(head->firstPeon, value);
      head = head->nextPeon;
   }
}

void CommandTeamCtrl::buildPlayerDisplayList(PlayerManager::ClientRep *cl)
{
   while(cl)
   {
      if(cl->commander)
         cl->commandLevel = cl->commander->commandLevel + 1;
      else
         cl->commandLevel = 0;

      displayPlayers[displayPlayersCount++] = cl;
   
      //keep in sync with the muted status
      if (cl->selected && cl->mutedMe && cg.commandTS)
         cg.commandTS->setSelectedPlayer(cl->id, FALSE);
   
      if(cl->expanded)
         buildPlayerDisplayList(cl->firstPeon);
      cl = cl->nextPeon;
   }
}

void CommandTeamCtrl::onPreRender()
{
   PlayerManager::ClientRep *me = playerManager->findClient(manager->getId());
   if(!me)
      return;

   PlayerManager::TeamRep *team = playerManager->findTeam(me->team);
   displayPlayersCount = 0;
   playerManager->buildCommandTrees();
   
   if (team)
   {
      buildPlayerDisplayList(team->firstCommander);
   }

   setSize(Point2I(1, displayPlayersCount));
   setUpdate();
}

void CommandTeamCtrl::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool, bool)
{
   Point2I cellOffset = offset;
   
   GFXBitmapArray *myBma = (! mLowRes ? bma : lr_bma);
   GFXBitmap *bmp;
   int bmpIndex;
   int statusWidth = myBma->getBitmap(BmpParentContinue)->getWidth();
   
   PlayerManager::ClientRep *cl = displayPlayers[cell.y];
   PlayerManager::ClientRep *prev = cell.y ? displayPlayers[cell.y-1] : NULL;
   
   //if (prev && prev->nextPeon != cl)
   //   prev = NULL;
   
   //bool pending = (cl->curCommand >= 0 &&
   //               cl->commandStatus == PlayerManager::CMD_RECEIVED);

   //draw the background rectangle
   if (cl->selected)
   {
      RectI selRect(cellOffset, cellOffset);
      selRect.lowerR.x += extent.x - 1;
      selRect.lowerR.y += extent.y - 1;
      selRect.upperL.x += 2 + bmpDamageHigh->getWidth() + 1 + bmpCMDAcknowledged->getWidth();
      
      sfc->drawRect2d_f(&selRect, GREEN);
   }

   if(!cg.commandTS)
      return;
      
   //draw the players health status
   int damageLevel = cg.commandTS->getPlayerDamage(cl->id);
   if (damageLevel ==  2) bmp = bmpDamageHigh;
   else  if (damageLevel == 1) bmp = bmpDamageMed;
   else bmp = bmpDamageLow;
   
   //draw the bitmap
   cellOffset.x += 2;
   if (! bmp) return;
   sfc->drawBitmap2d(bmp, &Point2I(cellOffset.x, cellOffset.y + 1));
   cellOffset.x += bmp->getWidth() + 1;
   
   //draw the command status
   int cmdStatus = cg.commandTS->getPlayerStatus(cl->id) - 1;
   if (cmdStatus == PlayerManager::CMD_RECEIVED) bmp = bmpCMDPending;
   else if (cmdStatus == PlayerManager::CMD_ACKNOWLEDGED) bmp = bmpCMDAcknowledged;
   //else bmp = bmpCMDNone;
   else bmp = NULL;
   
   //draw the bitmap
   if (bmp)
   {
      sfc->drawBitmap2d(bmp, &Point2I(cellOffset.x, cellOffset.y + 1));
   }
   cellOffset.x += bmpCMDPending->getWidth() + 2;
   
   //draw in my own status
   //find the starting index point
   if (! cl->firstPeon) bmpIndex = BmpNone;
   else if (cl->expanded) bmpIndex = BmpParentOpen;
   else bmpIndex = BmpParentClosed;
   
   //add to the index based on the status
   if (! prev)
   {
      if (cl->nextPeon)
         bmpIndex += 1;
   }
   else if (!cl->nextPeon)
      bmpIndex += 2;
   else
      bmpIndex += 3;
   
   //now we have our bmpIndex, draw the status
   if (bmpIndex >= 0)
   {
      bmp = myBma->getBitmap(bmpIndex);
      sfc->drawBitmap2d(bmp, &Point2I(cellOffset.x + (cl->commandLevel * statusWidth), cellOffset.y));
   }
   
   //draw in all the required continuation lines
   bmp = myBma->getBitmap(BmpParentContinue);
   PlayerManager::ClientRep *commander = cl->commander;
   while(commander)
   {
      if (commander->nextPeon)
         sfc->drawBitmap2d(bmp, &Point2I(cellOffset.x + (commander->commandLevel * statusWidth), cellOffset.y));
      commander = commander->commander;
   }
   
   GFXFont *font;
   //if (! mLowRes) font = (cl->mutedMe ? hFontMuted : (pending ? hFontWaitAckge : hFont));
   //else font = (pending ? lr_hFontWaitAckge : lr_hFont);
   if (! mLowRes) font = (cl->mutedMe ? hFontMuted : hFont);
   else font = lr_hFont;
   
   sfc->drawText_p(font, &Point2I(cellOffset.x + ((cl->commandLevel + 1) * statusWidth), cellOffset.y - 2), cl->name);
}

int CommandTeamCtrl::getSelectedPlayerId(void)
{
   if (! cg.player) return -1;
   int myId = manager->getId();
   
   int i;
   for(i = 0; i < displayPlayersCount;i++)
   {
      PlayerManager::ClientRep *cl = displayPlayers[i];
      if(cl->selected && cl->id != myId && !playerManager->isInCommandChain(cl->id, myId))
         return cl->id;
   }
   return -1;
}

Int32 CommandTeamCtrl::getMouseCursorTag(void)
{
   if (cg.commandTS && cg.commandTS->inWaypointMode())
   {
      return IDBMP_CURSOR_WAYPOINT_ARROW;
   }
   else return Parent::getMouseCursorTag();
}

void CommandTeamCtrl::onMouseDown(const SimGui::Event &event)
{
   if((!active) || (! cg.commandTS))
   {
      Parent::onMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   
   GFXBitmapArray *myBma = (! mLowRes ? bma : lr_bma);
   
   bool doubleClick = (event.mouseDownCount > 1);
   
   //find out which cell was hit
   Point2I cell((pt.x < 0 ? -1 : pt.x / cellSize.x), (pt.y < 0 ? -1 : pt.y / cellSize.y));
   if(cell.x >= 0 && cell.x < size.x && cell.y >= 0 && cell.y < size.y)
   {
      //see where in the cell the hit happened
      PlayerManager::ClientRep *me = displayPlayers[cell.y];
      int statusWidth = myBma->getBitmap(BmpParentContinue)->getWidth() + 2;
      int statusOffset = 2 + bmpDamageHigh->getWidth() + 1 + bmpCMDAcknowledged->getWidth() + 1 +
                           (statusWidth * me->commandLevel);
      
      //if we clicked on the expanded icon
      if (pt.x >= statusOffset && pt.x <= statusOffset + statusWidth && me->firstPeon)
      {
         me->expanded = !me->expanded;
      }
      
      //if we clicked on the players name...
      else if (pt.x >= 2 + ((me->commandLevel + 1) * statusWidth))
      {
         //if we're in waypoint mode - select a player as a target
         if (cg.commandTS && cg.commandTS->inWaypointMode())
         {
            Console->setVariable("CommandTargetName", displayPlayers[cell.y]->name);
            Console->setVariable("CommandTarget", "friendly player");
            int targIndex = cg.commandTS->getPlayerTargIndex(displayPlayers[cell.y]->id);
            if (targIndex >= 0)
            {
               cg.commandTS->setCommandWaypointItem(targIndex);
            	cg.commandTS->setMouseMode(TSCommander::SelectingMode);
            }
         }
         else
         {
            //make sure you double click on the same person
            if (cell.y != prevSelectedCell.y) doubleClick = FALSE;
            
            //first, deselect all turrets and objects
            cg.commandTS->setSelectedTurret(-1);
            cg.commandTS->setSelectedObject(-1);
            
            //find the player and set the selected cell(s)
            if(event.modifier & SI_SHIFT)
            {
               if (selectedCell.y < 0) selectedCell = cell;
               
               //go through the list and unselect everyone
               cg.commandTS->setSelectedPlayer(-1, FALSE);
               
               //now select the players between the first selected cell, and the current
               for (int i = min(selectedCell.y, cell.y); i <= max(selectedCell.y, cell.y); i++)
               {
                  if (! displayPlayers[i]->mutedMe)
                  {
                     cg.commandTS->setSelectedPlayer(displayPlayers[i]->id, TRUE);
                  }
               }
            }
            
            //toggle the player selected
            else if (event.modifier & SI_CTRL)
            {
               if (! me->mutedMe)
               {
                  if (doubleClick)
                  {
                     setBranchSelected(me, !me->selected);
                  }
                  else
                  {
                     cg.commandTS->setSelectedPlayer(me->id, !me->selected);
                  }
               }
            }
               
            else
            {
               //set the first cell selected
               selectedCell = cell;
               
               //go through the list and unselect everyone
               cg.commandTS->setSelectedPlayer(-1, FALSE);
               
               //select the player
               if (! displayPlayers[selectedCell.y]->mutedMe)
               {
                  if (doubleClick)
                  {
                     setBranchSelected(me, !me->selected);
                  }
                  else
                  {
                     cg.commandTS->setSelectedPlayer(me->id, TRUE);
                  }
               }
            }
            prevSelectedCell = cell;
         }
      }
   }
}

};
