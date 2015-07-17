#include "simGuiDelegate.h"
#include "simGuiBitmapCtrl.h"
#include "simResource.h"
#include "console.h"
#include "ircclient.h"
#include "fear.strings.h"
#include "FearGuiArrayCtrl.h"
#include "FearGuiBox.h"
#include "FearGuiUnivButton.h"
#include "fearGuiHelpCtrl.h"
#include "simGuiTextEdit.h"

namespace SimGui
{

//also defined in FearGuiRecList.cpp
const char gPlaybackExtention[] = "recordings\\*.rec";
const char gPlaybackDir[] = "recordings\\";

class CreateServerDelegate : public Delegate
{

private:
   typedef Delegate Parent;
   Control *curGui;
   
public:
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(CreateServerDelegate);
   CreateServerDelegate() { curGui = NULL; }
   
   DWORD onMessage(SimObject *sender, DWORD msg);
};

void CreateServerDelegate::onNewContentControl(Control *, Control *newGui)
{
   curGui = newGui;
   
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   if (root)
   {
      //set the help control
      SimGui::HelpCtrl *hc = root->getHelpCtrl();
      if (! hc)
      {
         FearGui::FGHelpCtrl *helpCtrl = new FearGui::FGHelpCtrl();
         root->setHelpCtrl(helpCtrl);
      }
   }
}

DWORD CreateServerDelegate::onMessage(SimObject *sender, DWORD msg)
{
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   else return Parent::onMessage(sender, msg);
   AssertFatal(root, "Unable to get the root");
   
   IRCClient *objIRCClient = 
      static_cast<IRCClient *>(IRCClient::find(manager));
   AssertFatal(objIRCClient, "JoinDelegate: could not locate IRC client");
      
   if (msg == IDCTG_SERVER_CREATE)
   {
      //join the game...
      const char *addr = Console->getVariable("Client::IPAddress");
      const char *port = Console->getVariable("$Server::Port");
      char addrBuf[256];
      sprintf(addrBuf, "%s:%s", addr, port);
      
      
      if (Console->getBoolVariable("$IRC::BroadcastIP"))
      {
         objIRCClient->onJoinServer(Console->getVariable("$pref::lastMission"), 
            Console->getVariable("$pref::hostName"), addrBuf,
            false, strcmp(Console->getVariable("$Server::Password"), ""));
      }
      Console->evaluatef("$SinglePlayer=false;createServer($pref::lastMission);");
      return -1;
   }
   return Parent::onMessage(sender, msg);
}

IMPLEMENT_PERSISTENT(CreateServerDelegate);

};