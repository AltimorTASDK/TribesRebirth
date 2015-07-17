#include "console.h"
#include "netPacketStream.h"
#include "fearGlobals.h"
#include "fear.strings.h"
#include "fearCSDelegate.h"
#include "simGuiCtrl.h"
#include "simGuiTextEdit.h"
#include "simGuiProgressCtrl.h"
#include "fearGuiArrayCtrl.h"
#include "fearGuiCombo.h"
#include "fearGuiServerList.h"
#include "fearGuiPlayerList.h"
#include "fearGuiUnivButton.h"
#include "fearGuiTextFormat.h"
#include "fearGuiBox.h"
#include "fearGuiServerInfo.h"
#include "fearGuiHelpCtrl.h";
#include "dlgJoinGame.h"
#include "ircclient.h"

namespace SimGui
{

static bool gbAutoRefreshDone = FALSE;
static const char *curFilterVariable = "pref::UseFilter";

void JoinGameDelegate::onNewContentControl(Control *, Control *newGui)
{
   if(!newGui)
      return;
   if(newGui->getTag() != IDGUI_JOIN_GAME)
      return;

   curGui = newGui;
   
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   else return;
   if(!root)
      return;
   
   //set the help control
   SimGui::HelpCtrl *hc = root->getHelpCtrl();
   if (! hc)
   {
      FearGui::FGHelpCtrl *helpCtrl = new FearGui::FGHelpCtrl();
      root->setHelpCtrl(helpCtrl);
   }
   
   //add a var so we can keep an eye on the filter
   CMDConsole* console = CMDConsole::getLocked();
   mCurFilter = atoi(console->getVariable(curFilterVariable));
   
   //disable the join button until a game has been selected
   verifyServer();
   
   //check for a new master MOTD
   verifyMasterMOTD();
   
   //hide the filter combo if the connection is a LAN
   FearGui::FGComboBox *combo = dynamic_cast<FearGui::FGComboBox*>(curGui->findControlWithTag(IDCTG_FILTER_LIST));
   if (! stricmp(Console->getVariable("Pref::JoinGameMode"), "LAN"))
   {
      if (combo) combo->setVisible(FALSE);
   }
   else if (combo) combo->setVisible(TRUE);
   
   //create the var
   if (! stricmp(Console->getVariable("pref::AutoRefresh"), ""))
   {
      Console->setVariable("pref::AutoRefresh", "TRUE");
   }
   
   //see if we should do an auto refresh
   if (! stricmp(Console->getVariable("pref::AutoRefresh"), "FALSE"))
   {
      gbAutoRefreshDone = TRUE;
   }
   else if (! gbAutoRefreshDone)
   {
      onMessage(this, IDCTG_SERVER_REFRESH_LIST);
      gbAutoRefreshDone = TRUE;
   }
}

void JoinGameDelegate::onLoseDelegation(void)
{
   FearGui::ServerListCtrl *slCtrl = (FearGui::ServerListCtrl*)curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST);
   if (! slCtrl) return;
   slCtrl->writeScriptFile();
}

bool JoinGameDelegate::verifyServer(void)
{
   if (! curGui) return FALSE;
   CMDConsole* console = CMDConsole::getLocked();
   int prevFilter = atoi(console->getVariable(curFilterVariable));
   
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   else return FALSE;
   if (! root) return FALSE;
   
   //enable the join game button
   FearGui::FGUniversalButton *joinButton = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_SERVER_JOIN));
   FearGui::FGUniversalButton *infoButton = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_SERVER_QUERY));
   FearGui::ServerListCtrl *slCtrl = (FearGui::ServerListCtrl*)curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST);
   FearCSDelegate::ServerInfo *info = NULL;
   bool infoAvail;
   if (slCtrl) info = slCtrl->getServerSelected(infoAvail);
   if (joinButton && infoButton)
   {
      if ((prevFilter == mCurFilter) && info) // deactivate with a new filter
      {
         joinButton->active = TRUE;
         infoButton->active = infoAvail;
      }
      else
      {
         mCurFilter = prevFilter;
         joinButton->active = FALSE;
         infoButton->active = FALSE;
      }
      joinButton->setUpdate();
      infoButton->setUpdate();
      return joinButton->active;
   }
   return FALSE;
}

bool JoinGameDelegate::verifyMasterMOTD(void)
{
   if (! curGui) return FALSE;
   //find the MOTD button
   FearGui::FGUniversalButton *button = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_MASTER_MOTD));
   if (! button) return FALSE;
   
   bool foundMOTD;
   bool newMOTD;
   const char *oldMOTD;
   const char *msgMOTD;
   oldMOTD = Console->getVariable("pref::MSprevMOTD");
   msgMOTD = Console->getVariable("pref::MSMOTD");
   
   if ((! msgMOTD) || (! msgMOTD[0])) foundMOTD = FALSE;
   else foundMOTD = TRUE;
   
   if (! strcmp(oldMOTD, msgMOTD)) newMOTD = FALSE;
   else newMOTD = TRUE;
   
   //also disable the MOTD button if we're on a LAN
   if (! stricmp(Console->getVariable("Pref::JoinGameMode"), "LAN"))
   {
      foundMOTD = FALSE;
      newMOTD = FALSE;
   }
   
   //set the buttons visiblity
   button->active = foundMOTD;
   button->setFlashing(newMOTD);
   button->setUpdate();
   
   return foundMOTD;
}

void JoinGameDelegate::masterTimeout(const char *address)
{
   const char *curMaster = Console->getVariable("Server::CurrentMaster");
   const char *masterName = NULL;
   if (curMaster[0]) masterName = Console->getVariable(avar("Server::MasterName%d", atoi(curMaster)));
   char masterStr[256];
   if (masterName && masterName[0]) sprintf(masterStr, " (%s)", masterName);
   else masterStr[0] = '\0';
   char buf[512];
   sprintf(buf, "Internet Connection Error\nCould not contact any master server.  Please verify your internet connection.", address, masterStr);
   Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\MessageDialog.gui");
   Console->executef(3, "Control::setValue", "MessageDialogTextFormat", buf); 
   mbMasterTimedOut = TRUE;
}

void JoinGameDelegate::updatePingLists(int numToPing, int numPinged, bool firstPhase)
{
   if (! curGui) return;
   
   if ((! numToPing) && (! numPinged))
   {
      queryFinished();
      return;
   }
   
   SimGui::ProgressCtrl *prog = dynamic_cast<SimGui::ProgressCtrl*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_PROGRESS));
   if (prog)
   {
      float percentDone;
      if (numPinged + numToPing == 0) percentDone = 1.0f;
      else percentDone = float(numPinged) / (2.0f * float(numPinged + numToPing));
      if (! firstPhase) percentDone = percentDone + 0.5f;
      prog->setPercentDone(percentDone);
   }
   
   SimGui::SimpleText *textCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_TEXT));
   char buf[64];
   sprintf(buf, "Servers left to %s: %d", (firstPhase ? "ping" : "query"), numToPing); 
   if (textCtrl) textCtrl->setText(buf);
   
}

void JoinGameDelegate::queryFinished(void)
{
   //find the game list, and signal the end of the query
   if (cg.gameServerList)
   {
      cg.gameServerList->rebuildFinished(mbMasterTimedOut);
   }
   
   if (! curGui) return;
   
   //reverify our selected server
   verifyServer();
   
   //hide the progress controls
   FearGui::FearGuiBox *boxCtrl = dynamic_cast<FearGui::FearGuiBox*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_BOX));
   if (boxCtrl)
   {
      boxCtrl->setVisible(FALSE);
      boxCtrl->setUpdate();
   }
   
   //reset the refresh button
   SimGui::ActiveCtrl *ctrl = dynamic_cast<SimGui::ActiveCtrl*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_LIST));
   if (ctrl)
   {
      ctrl->active = TRUE;
      ctrl->setUpdate();
   }
   ctrl = dynamic_cast<SimGui::ActiveCtrl*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_VISIBLE));
   if (ctrl)
   {
      ctrl->active = TRUE;
      ctrl->setUpdate();
   }
}

DWORD JoinGameDelegate::onMessage(SimObject *sender, DWORD msg)
{  
   //get the root
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   else return Parent::onMessage(sender, msg);
   if (! root) return -1;
   
   IRCClient *objIRCClient = 
      static_cast<IRCClient *>(IRCClient::find(manager));
   AssertFatal(objIRCClient, "JoinDelegate: could not locate IRC client");
      
   //check for a new MOTD
   verifyMasterMOTD();
   
   if (msg == IDCTG_SERVER_REFRESH_LIST)
   {
      mbMasterTimedOut = FALSE;
      Console->evaluate("rebuildServerList();", FALSE);
      if (cg.gameServerList) cg.gameServerList->rebuildList();
      
      //grey out the refresh button
      SimGui::ActiveCtrl *ctrl = dynamic_cast<SimGui::ActiveCtrl*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_LIST));
      if (ctrl) ctrl->active = FALSE;
      ctrl = dynamic_cast<SimGui::ActiveCtrl*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_VISIBLE));
      if (ctrl) ctrl->active = FALSE;
      
      //initialize the progress controls
      FearGui::FearGuiBox *boxCtrl = dynamic_cast<FearGui::FearGuiBox*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_BOX));
      if (boxCtrl)
      {
         boxCtrl->setVisible(TRUE);
         boxCtrl->setUpdate();
      }
      
      SimGui::ProgressCtrl *prog = dynamic_cast<SimGui::ProgressCtrl*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_PROGRESS));
      if (prog) prog->setPercentDone(0.0f);
      
      SimGui::SimpleText *textCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_TEXT));
      if (textCtrl)
      {
         bool lanOnly = Console->getBoolVariable("pref::lanOnly", false) ||
            Console->getIntVariable("Server::NumMasters", 0) == 0;
         if (lanOnly)
            textCtrl->setText("Searching for LAN servers");
         else
            textCtrl->setText("Querying master servers");
      }
      
      return -1;
   }
   
   else if (msg == IDCTG_SERVER_REFRESH_VISIBLE)
   {
      mbMasterTimedOut = TRUE;
      bool result = FALSE;
      if (cg.gameServerList)
      {
         result = cg.gameServerList->refreshVisible();
      }
      if (! result) return -1;
      
      //grey out the refresh button
      SimGui::ActiveCtrl *ctrl = dynamic_cast<SimGui::ActiveCtrl*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_LIST));
      if (ctrl) ctrl->active = FALSE;
      ctrl = dynamic_cast<SimGui::ActiveCtrl*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_VISIBLE));
      if (ctrl) ctrl->active = FALSE;
      
      //initialize the progress controls
      FearGui::FearGuiBox *boxCtrl = dynamic_cast<FearGui::FearGuiBox*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_BOX));
      if (boxCtrl)
      {
         boxCtrl->setVisible(TRUE);
         boxCtrl->setUpdate();
      }
      
      SimGui::ProgressCtrl *prog = dynamic_cast<SimGui::ProgressCtrl*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_PROGRESS));
      if (prog) prog->setPercentDone(0.0f);
      
      SimGui::SimpleText *textCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_SERVER_REFRESH_TEXT));
      if (textCtrl)
      {
         textCtrl->setText("Pinging visible servers");
      }
      
      return -1;
   }
   
   else if (msg == IDCTG_SERVER_REFRESH_CANCEL)
   {
      if (cg.csDelegate)
      {
		   cg.csDelegate->clearPingRequestList();
		   cg.csDelegate->clearGameRequestList();
      }
      if (cg.gameServerList) cg.gameServerList->rebuildCancel();
      mbMasterTimedOut = TRUE;
      queryFinished();
      
      return -1;
   }
   
   else if (msg == IDDLG_MASTER_MOTD)
   {
      char buf[1024];
      const char *newMOTD;
      const char *name;
      newMOTD = Console->getVariable("pref::MSMOTD");
      name = Console->getVariable("pref::MSName");
      
      //push the dialog
      Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\MasterMOTD.gui");
      
      //set the server name in the dialog box
      sprintf(buf, "<f1>Message of the day from: %s<f0>\n\n", name);
      int length = 1023 - strlen(buf);
      strncpy(&buf[strlen(buf)], newMOTD, length);
      buf[1023] = '\0';
      
      //set the message of the day in the dialog box
      FearGui::FGTextFormat *motd = dynamic_cast<FearGui::FGTextFormat*>(root->getTopDialog()->findControlWithTag(IDCTG_MASTER_MOTD));
      if (motd) motd->formatControlString(buf, motd->extent.x);
      
      //now update the pref variable
      Console->setVariable("pref::MSprevMOTD", newMOTD);
      
      verifyMasterMOTD();
      
      return -1;
   }
   
   else if (msg == IDCTG_SERVER_JOIN)
   {
      if (cg.csDelegate)
      {
		   cg.csDelegate->clearPingRequestList();
		   cg.csDelegate->clearGameRequestList();
      }
      if (cg.gameServerList) cg.gameServerList->rebuildCancel();
      mbMasterTimedOut = TRUE;
      queryFinished();
      
      //find the server list ctrl, and the server info
      FearGui::ServerListCtrl *slCtrl = (FearGui::ServerListCtrl*)curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST);
      FearCSDelegate::ServerInfo *info = NULL;
      bool infoAvail;
      if (slCtrl) info = slCtrl->getServerSelected(infoAvail);
      if (! info) return -1;
      
      //see if the server requires a password
      if (info->password)
      {
         CMDConsole::getLocked()->executef(3, "GuiPushDialog", "MainWindow", "gui\\Password.gui");
      }
      else
      {
         //join the game...
         if (Console->getBoolVariable("$IRC::BroadcastIP"))
         {
            objIRCClient->onJoinServer(info->missionName, 
               info->name, Console->getVariable("$Server::Address"),
               false, info->password);
         }
         Console->evaluate("JoinGame();", FALSE);
      }
      
      return -1;
   }
   
   else if (msg == IDCTG_SERVER_JOIN_PASSWORD)
   {
      //kill the dialog
      root->makeFirstResponder(NULL);
      root->popDialogControl();
      
      //join the game...
      if (Console->getBoolVariable("$IRC::BroadcastIP"))
      {
         //find the server list ctrl, and the server info
         FearGui::ServerListCtrl *slCtrl = (FearGui::ServerListCtrl*)curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST);
         FearCSDelegate::ServerInfo *info = NULL;
         bool infoAvail;
         if (slCtrl) info = slCtrl->getServerSelected(infoAvail);
         if (info)
         {
            objIRCClient->onJoinServer(info->missionName, 
               info->name, Console->getVariable("$Server::Address"),
               false, info->password);
         }
      }
      Console->evaluate("JoinGame();", FALSE);
   }
   
   //used to quit while in the process of joining a server
   else if (msg == IDCTG_SERVER_JOIN_CANCEL)
   {
      if(cg.packetStream)
      {
         cg.packetStream->disconnect();
         manager->unregisterObject(cg.packetStream);
         delete cg.packetStream;
         cg.packetStream = NULL;
      }
      else if (cg.csDelegate)
      {
         cg.csDelegate->cancelDuringConnection();
      }
      
      Console->executef(3, "GuiLoadContentCtrl", "MainWindow", "gui\\JoinGame.gui");
   }
   
   else if (msg == IDDLG_SERVER_INFO)
   {
      //find the server list dialog
      FearGui::ServerListCtrl *slCtrl = (FearGui::ServerListCtrl*)curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST);
      
      //push the dialog
      const char *dlgName = SimTagDictionary::getString(manager, msg);
      CMDConsole::getLocked()->executef(3, "GuiPushDialog", "MainWindow", dlgName);
      
      //get the info
      char buf[256];
      SimGui::SimpleText *ctrl;
      FearGui::FGUniversalButton *btnCtrl;
      FearCSDelegate::ServerInfo *info = NULL;
      bool infoAvail;
      if (slCtrl) info = slCtrl->getServerSelected(infoAvail);
      
      //plug the info into the various controls
      if (info && infoAvail)
      {
         //server name
         ctrl = (SimGui::SimpleText*)root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_NAME);
         if (ctrl) ctrl->setText(info->name);
            
         //server address
         ctrl = (SimGui::SimpleText*)root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_ADDRESS);
         if (ctrl) ctrl->setText(info->transportAddress);
         
         //server version
         ctrl = (SimGui::SimpleText*)root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_VERSION);
         if (ctrl) ctrl->setText(info->version);
         
         //server Ping
         ctrl = (SimGui::SimpleText*)root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_PING);
         if (ctrl) {
            sprintf(buf, "%d", info->pingTime);
            ctrl->setText(buf);
         }
            
         //server dedicated
         ctrl = (SimGui::SimpleText*)root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_DEDICATED);
         if (ctrl) ctrl->setText((info->dedicated ? "YES" : "NO"));
         
         //server game
         ctrl = (SimGui::SimpleText*)root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_GAME);
         if (ctrl) ctrl->setText(info->modName);
         
         //server mission
         ctrl = (SimGui::SimpleText*)root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_MISSION);
         if (ctrl) ctrl->setText(info->missionName);
         
         //server password
         ctrl = (SimGui::SimpleText*)root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_PASSWORD);
         if (ctrl) ctrl->setText((info->password ? "YES" : "NO"));
         
         //server favorite
         btnCtrl = (FearGui::FGUniversalButton*)root->getTopDialog()->findControlWithTag(IDCTG_SERVER_INFO_FAVORITE);
         if (btnCtrl) btnCtrl->setMode(info->favorite);
         
         //server neverPing
         btnCtrl = (FearGui::FGUniversalButton*)root->getTopDialog()->findControlWithTag(IDCTG_SERVER_INFO_NEVER_PING);
         if (btnCtrl) btnCtrl->setMode(info->neverPing);
         
         //server text
         Console->executef(3, "Control::setValue", "ServerInfoText", info->hostInfo);
         
         //server info list
         FearGui::ServerInfoCtrl *siCtrl = dynamic_cast<FearGui::ServerInfoCtrl*>(root->getTopDialog()->findControlWithTag(IDCTG_SRVR_INFO_LIST));
         if (siCtrl) siCtrl->setServerInfo(info);
      }
      
      return -1;
   }
   
   else if (msg == IDCTG_SERVER_INFO_FAVORITE)
   {
      //find the server list dialog
      FearGui::ServerListCtrl *slCtrl = (FearGui::ServerListCtrl*)curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST);
      
      //get the info
      FearGui::FGUniversalButton *btnCtrl;
      FearCSDelegate::ServerInfo *info = NULL;
      bool infoAvail;
      if (slCtrl) info = slCtrl->getServerSelected(infoAvail);
      btnCtrl = (FearGui::FGUniversalButton*)root->getTopDialog()->findControlWithTag(IDCTG_SERVER_INFO_FAVORITE);
      
      if (info && infoAvail && btnCtrl)
      {
         slCtrl->setFavorite(info->transportAddress, btnCtrl->isSet());
      }
      
      return -1;
   }
   
   else if (msg == IDCTG_SERVER_INFO_NEVER_PING)
   {
      //find the server list dialog
      FearGui::ServerListCtrl *slCtrl = (FearGui::ServerListCtrl*)curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST);
      
      //get the info
      FearGui::FGUniversalButton *btnCtrl;
      FearCSDelegate::ServerInfo *info = NULL;
      bool infoAvail;
      if (slCtrl) info = slCtrl->getServerSelected(infoAvail);
      btnCtrl = (FearGui::FGUniversalButton*)root->getTopDialog()->findControlWithTag(IDCTG_SERVER_INFO_NEVER_PING);
      
      if (info && infoAvail && btnCtrl)
      {
         slCtrl->setNeverPing(info->transportAddress, btnCtrl->isSet());
      }
      
      return -1;
   }
   
   else if (msg == IDCTG_SERVER_INFO_REFRESH)
   {
      //find the server list dialog
      FearGui::ServerListCtrl *slCtrl = (FearGui::ServerListCtrl*)curGui->findControlWithTag(IDCTG_SERVER_SELECT_LIST);
      
      //get the info
      FearCSDelegate::ServerInfo *info = NULL;
      bool infoAvail;
      if (slCtrl) info = slCtrl->getServerSelected(infoAvail);
      
      if (info && infoAvail && cg.csDelegate)
      {
         cg.csDelegate->pushPingInfoRequest(info->transportAddress);
      }
   }
   
   else
   {
      //since there are more than a few things that alter the current server...
      verifyServer();
   }
   
   return Parent::onMessage(sender, msg);
}

IMPLEMENT_PERSISTENT(JoinGameDelegate);

};