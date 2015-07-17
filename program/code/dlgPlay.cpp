#include "simGame.h"
#include "simGuiDelegate.h"
#include "PlayerManager.h"
#include "console.h"
#include "fear.strings.h"
#include "FearGuiChat.h"
#include "FearGuiChatDisplay.h"
#include "FearGuiRadio.h"
#include "SimGuiCtrl.h"
#include "SimGuiBitmapCtrl.h"
#include "SimGuiCanvas.h"
#include "FearGuiTSCommander.h"
#include "FearPlayerPSC.h"
#include "simResource.h"
#include "fearGuiCommandTeamList.h"
#include "fearGuiCurCmd.h"
#include "fearGuiHelpCtrl.h"
#include "fearGlobals.h"
#include "player.h"
#include "fearHudInvList.h"
#include "dataBlockManager.h"
#include "fearGuiUnivButton.h"
#include "dlgPlay.h"
#include "simPalette.h"

namespace SimGui
{

void playDelegate::setScoresVisible(bool vis)
{
   //can't display both the scores and the hud list
   if (vis && hudListVisible)
   {
      setHudListVisible(FALSE);
   }   
   
   if(scoresVisible != vis)
   {
      scoresVisible = vis;
      setCurrentGuiMode(curMode);
   }
}

void playDelegate::setHudListVisible(bool vis)
{
   //can't display both the scores and the hud list
   if (vis && scoresVisible)
   {
      setScoresVisible(FALSE);
   }
   
   hudListVisible = vis;
   SimGui::Control *ctrl = curGui->findControlWithTag(IDCTG_HUD_LIST);
   if (ctrl) ctrl->setVisible(vis);
   Console->executef(2, vis ? "cursorOn" : "cursorOff", "MainWindow");
}

void playDelegate::setCurrentGuiMode(int newMode)
{
   if(!gameActive)
   {
      curMode = newMode;
      return;
   }
   bool cursorOn = true;
   Console->executef(3, "setCMMode", "PlayChatMenu", "0");
   Console->executef(3, "setCMMode", "CommandChatMenu", "0");
   bool setPalette = true;
   switch(newMode)
   {
      case PlayerPSC::PlayGuiMode:
         Console->executef(1, "loadPlayGui");
         cursorOn = false;
         break;
      case PlayerPSC::CommandGuiMode:
         Console->executef(3, "GuiLoadContentCtrl", "MainWindow", "command.gui");
         break;
      case PlayerPSC::VictoryGuiMode:
         Console->executef(3, "GuiLoadContentCtrl", "MainWindow", "gui\\CmdObjectives.gui");
         break;
      case PlayerPSC::ObjectiveGuiMode:
         Console->executef(3, "GuiLoadContentCtrl", "MainWindow", "gui\\CmdObjectives.gui");
         break;
      case PlayerPSC::InventoryGuiMode:
         Console->executef(3, "GuiLoadContentCtrl", "MainWindow", "gui\\CmdInventory.gui");
         break;
      case PlayerPSC::InitialGuiMode:
         setPalette = false;
         Console->executef(3, "GuiLoadContentCtrl", "MainWindow", "gui\\loading.gui");
         break;
   }

   if(setPalette && bool(hPal))
      SimGame::get()->getWorld()->setPalette(hPal);

   if (scoresVisible)
   {
      cursorOn = true;
      //see if we're in hi res or low res
      bool lowRes = FALSE;
      if (curGui)
      {
         lowRes = ((curGui->extent.x < 512) || (curGui->extent.y < 384));
      }
      if (lowRes)
      {
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\lr_score.gui");
      }
      else
      {
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\score.gui");
      }
   }
   Console->executef(2, cursorOn ? "cursorOn" : "cursorOff", "MainWindow");

   curMode = newMode;
}

void playDelegate::activate()
{
   gameActive = true;
   // see if there is a palette in the GhostGroup
   int mode = curMode;
                            
   setCurrentGuiMode(PlayerPSC::PlayGuiMode); // reactivate the play delegate
   if(mode != PlayerPSC::PlayGuiMode)
      setCurrentGuiMode(mode);
   Console->setBoolVariable("$InLobbyMode", false);
}

void playDelegate::reset()
{
   scoresVisible = false;
   hudListVisible = FALSE;
   gameActive = true;
   curMode = -1;
   mShellPage = -1;
   mbCommandMode = FALSE;
   for (int i = 0; i < PlayerPSC::ItemShoppingListWords; i++)
      shoppingList[i] = 0;
}

void playDelegate::onDeleteNotify(SimObject *obj)
{
   if(obj == currentPalette)
      currentPalette = NULL;
}

bool playDelegate::onAdd()
{
   if(!Parent::onAdd())
      return false;

   currentPalette = NULL;
   scoresVisible = false;
   hudListVisible = FALSE;
   gameActive = true;
   actionEventMap = SimResource::loadByName(manager, "actionMap.sae", true);
   playEventMap = SimResource::loadByName(manager, "playMap.sae", true);
   commandEventMap = SimResource::loadByName(manager, "pdaMap.sae", true);

   AssertFatal(bool(actionEventMap) && bool(playEventMap) && bool(commandEventMap), "Unable to load event maps.");

   commandGui = (SimGui::Control *) manager->findObject("commandGui");
   if(!commandGui)
   {
      commandGui = SimGui::Control::load(manager, "gui\\command.gui");
      manager->addObject(commandGui, "commandGui");
   }
   tsc = (FearGui::TSCommander *) commandGui->findControlWithTag(IDCTG_COMMAND_TS);
   
   //load the menu gui
   ResourceManager *rm = SimResource::get(manager);
   AssertFatal(rm, "Unable to get the resource manager");
   
   // local vars
   mShellPage = -1;
   mbCommandMode = FALSE;
   for (int i = 0; i < PlayerPSC::ItemShoppingListWords; i++)
   {
      shoppingList[i] = 0;
   }
   
   //set the global                                  
   cg.playDelegate = this;
   
   curMode = -1;
   
   return true;
}

void playDelegate::onNewContentControl(Control *oldGui, Control *newGui)
{
   curGui = newGui;
   bool newPlay = newGui->getTag() == IDCTG_PLAY_GUI || newGui->getTag() == IDCTG_COMMAND_GUI;
   bool oldPlay = false;
   if(oldGui)
      oldPlay = oldGui->getTag() == IDCTG_PLAY_GUI || oldGui->getTag() == IDCTG_COMMAND_GUI;

   if(newPlay && !oldPlay)
   {
		SimActionHandler::find (manager)->push (this, playEventMap);
		SimActionHandler::find (manager)->push (this, actionEventMap);
   }
   else if(!newPlay)
   {
		SimActionHandler::find (manager)->pop (this, playEventMap);
		SimActionHandler::find (manager)->pop (this, actionEventMap);
		SimActionHandler::find (manager)->pop (this, commandEventMap);
   }
   //make sure the hudList is off
   if(newGui->getTag() == IDCTG_PLAY_GUI)
   {
		SimActionHandler::find (manager)->pop (this, commandEventMap);
      setHudListVisible(FALSE);
      Console->setBoolVariable("ConnectedToServer", TRUE);
   }
   else if(newGui->getTag() == IDCTG_COMMAND_GUI)
   {
		SimActionHandler::find (manager)->push (this, commandEventMap);
   }

}

void playDelegate::onLoseDelegation(void)
{
	SimActionHandler::find (manager)->pop (this, playEventMap);
	SimActionHandler::find (manager)->pop (this, commandEventMap);
	SimActionHandler::find (manager)->pop (this, actionEventMap);
}

void playDelegate::preRender(void)
{
   if(!gameActive)
      return;

   // try to find a new palette to set in the game:
   if(!currentPalette)
   {
      SimGroup *group = (SimGroup *) manager->findObject("GhostGroup");
      if(group)
      {
         SimGroup::iterator i;
         for(i = group->begin(); i != group->end(); i++)
         {
            SimPalette *pal = dynamic_cast<SimPalette *>(*i);
            if(pal)
            {
               currentPalette = pal;
               deleteNotify(pal);
               hPal = pal->getPalette();
               pal->setInGame();
               break;
            }
         }  
      }
   }
}

bool playDelegate::onSimActionEvent (const SimActionEvent *event)
{
         
   SimGui::Canvas *root;
  	CMDConsole* console = CMDConsole::getLocked();
   int eventValue = (int)event->fValue;
   PlayerPSC *psc;
	switch(event->action)
	{
      case IDACTION_ESCAPE_PRESSED:
         if (hudListVisible)
            setHudListVisible(FALSE);
         else
         {
            gameActive = false;
            Console->executef(1, "EnterLobbyMode");
         }
         break;
         
		case IDACTION_CHAT:
         FearGui::FearGuiChat *chatHud;
         chatHud = (FearGui::FearGuiChat*)(curGui->findControlWithTag(IDCTG_HUD_CHAT));
         if (chatHud) {
            char buf[4];
            sprintf(buf, "%d", eventValue);
           	console->setVariable("pref::msgChannel", buf);
            chatHud->activate();
         }
         return true;
         
      case IDACTION_CHAT_DISP_SIZE:
         FearGui::FearGuiChatDisplay *chatDispHud;
         chatDispHud = (FearGui::FearGuiChatDisplay*)(curGui->findControlWithTag(IDCTG_HUD_CHAT_DISPLAY));
         if (chatDispHud) chatDispHud->setSize(eventValue);
         return true;
         
      case IDACTION_CHAT_DISP_PAGE:
         FearGui::FearGuiChatDisplay *chatDisplayHud;
         chatDisplayHud = (FearGui::FearGuiChatDisplay*)(curGui->findControlWithTag(IDCTG_HUD_CHAT_DISPLAY));
         if (chatDisplayHud) chatDisplayHud->page(eventValue);
         return true;

      case IDACTION_ZOOM_MODE_ON:
      {
         tsc->setZoomMode(TRUE);
         
         //find the control and set it
         FearGui::FGUniversalButton *ctrl = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_ZOOM_MODE));
         if (ctrl) ctrl->setMode(TRUE);
         
         return true;
      }
      case IDACTION_ZOOM_MODE_OFF:
      {
         tsc->setZoomMode(FALSE);
         
         //find the control and set it
         FearGui::FGUniversalButton *ctrl = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_ZOOM_MODE));
         if (ctrl) ctrl->setMode(FALSE);
         
         return true;
      }
      case IDACTION_ZOOM_IN:
         tsc->zoomIn();
         return true;
      case IDACTION_ZOOM_OUT:
         tsc->zoomOut();
         return true;
         
      case IDACTION_CENTER_MODE_ON:
      {
         tsc->setCenterMode(TRUE);
         
         //find the control and set it
         FearGui::FGUniversalButton *ctrl = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_CENTER_MODE));
         if (ctrl) ctrl->setMode(TRUE);
         
         return true;
      }
      case IDACTION_CENTER_MODE_OFF:
      {
         tsc->setCenterMode(FALSE);
         
         //find the control and set it
         FearGui::FGUniversalButton *ctrl = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_CENTER_MODE));
         if (ctrl) ctrl->setMode(FALSE);
         
         return true;
      }
         
      case IDACTION_MENU_PAGE:
      {
         if (eventValue == 1)
         {
            if (scoresVisible)
               console->executef(3, "remoteEval", "2048", "scoresOff");
            else
               console->executef(3, "remoteEval", "2048", "scoresOn");
         }
         else if (eventValue == 2)
         {
            setHudListVisible(! hudListVisible);
         }
      }
		default:
			// Anything we don't want get passed to the PSC
			if (cg.psc)
				cg.psc->onSimActionEvent(event);
			break;
   }
   return false;
}
      
bool playDelegate::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
		onEvent (SimActionEvent);
		
		default:
			return Parent::processEvent (event);
	}
}

DWORD playDelegate::onMessage(SimObject *sender, DWORD msg)
{
   //SimGui::Canvas *root;
   //if (curGui) root = curGui->getCanvas();
   
   if (msg == IDACTION_ESCAPE_PRESSED)
   {
      gameActive = false;
      Console->executef(1, "EnterLobbyMode");
   }
         
   else if (msg == IDCTG_ZOOM_MODE)
   {
      FearGui::FGUniversalButton *ctrl = dynamic_cast<FearGui::FGUniversalButton*>(sender);
      if (ctrl)
      {
         tsc->setZoomMode(ctrl->isSet());
      }
      return -1;
   }
   
   else if (msg == IDCTG_CENTER_MODE)
   {
      FearGui::FGUniversalButton *ctrl = dynamic_cast<FearGui::FGUniversalButton*>(sender);
      if (ctrl)
      {
         tsc->setCenterMode(ctrl->isSet());
      }
      return -1;
   }
   
   else if (msg == IDCTG_SET_COMMANDER)
   {
      FearGui::CommandTeamCtrl *tc = dynamic_cast<FearGui::CommandTeamCtrl*>(curGui->findRootControlWithTag(IDCTG_COMMAND_PLAYER_LIST));
      if ((! tc) || (! cg.player)) return Parent::onMessage(sender, msg);
      
      
      int cmdrId = tc->getSelectedPlayerId();
      if (cmdrId < 0) return Parent::onMessage(sender, msg); 
      
      Console->evaluatef("remoteEval(2048, SelectCommander, %d);", cmdrId);
   }
   
   else if (msg == IDCTG_UNSET_COMMANDER)
   {
      if (! cg.player) return Parent::onMessage(sender, msg);
      
      char cmdBuf[1024];
      Console->executef(4, "remoteEval", "2048", "SelectCommander", "-1");
   }
   
   else if (msg == IDCTG_CMD_TAKE_CONTROL)
   {
      if (cg.commandTS)
      {
         int index = cg.commandTS->getSelectedTurret();
         if (index >= 0)
            Console->evaluatef("remoteEval(2048, CmdrMountObject, %d);", index);
      }
   }
   
   else if (msg == IDCTG_GS_RADIO_1)
   {
      if (cg.commandTS)
         cg.commandTS->setSelectType(DataBlockManager::PlayerDataType);
   }
   
   else if (msg == IDCTG_GS_RADIO_2)
   {
      if (cg.commandTS)
         cg.commandTS->setSelectType(DataBlockManager::TurretDataType);
   }
   
   else if (msg == IDCTG_GS_RADIO_3)
   {
      if (cg.commandTS)
         cg.commandTS->setSelectType(DataBlockManager::StaticShapeDataType);
   }
   
   else if (msg == IDCTG_INV_DROP)
   {
      FearGui::FGHInventory *inv = dynamic_cast<FearGui::FGHInventory*>(curGui->findControlWithTag(IDCTG_INV_CURRENT));
      if (inv)
         inv->dropSelected();
   }
   
   else if (msg == IDCTG_INV_USE)
   {
      FearGui::FGHInventory *inv = dynamic_cast<FearGui::FGHInventory*>(curGui->findControlWithTag(IDCTG_INV_CURRENT));
      if (inv)
         inv->useSelected();
   }
   
   else if (msg == IDCTG_INV_BUY)
   {
      FearGui::FGHBuySell *inv = dynamic_cast<FearGui::FGHBuySell*>(curGui->findControlWithTag(IDCTG_INV_AVAILABLE));
      if (inv)
         inv->buySelected();
   }
   
   else if (msg == IDCTG_INV_SELL)
   {
      FearGui::FGHInventory *inv = dynamic_cast<FearGui::FGHInventory*>(curGui->findControlWithTag(IDCTG_INV_CURRENT));
      if (inv)
         inv->sellSelected();
   }
   
   else if (msg == IDCTG_INV_SHOPPING_SET_FAVS)
   {
      FearGui::FGHInventory *inv = dynamic_cast<FearGui::FGHInventory*>(curGui->findControlWithTag(IDCTG_INV_CURRENT));
      if (inv)
         inv->setFavorites();
   }
   
   else if (msg == IDCTG_INV_SHOPPING_BUY_FAVS)
   {
      FearGui::FGHBuySell *inv = dynamic_cast<FearGui::FGHBuySell*>(curGui->findControlWithTag(IDCTG_INV_AVAILABLE));
      if (inv)
         inv->buyFavorites();
   }
   
   return Parent::onMessage(sender, msg);
}

void playDelegate::updateTeamEnergy(void)
{
   //update the team energy field
   SimGui::Canvas *root = curGui->getCanvas();
   if (! root)
      return;
   SimGui::Control *topCtrl = root->getTopDialog();
   if (! topCtrl)
      return;
   SimGui::SimpleText *ctrl = (SimGui::SimpleText*)(topCtrl->findControlWithTag(IDCTG_INV_TEAM_ENERGY));
   if (cg.playerManager && ctrl)
   {
      char buf[8];
      sprintf(buf, "%d", 0);
      ctrl->setText(buf);
   }
}


IMPLEMENT_PERSISTENT(playDelegate);

};