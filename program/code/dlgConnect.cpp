#include "simGuiDelegate.h"
#include "simGuiBitmapCtrl.h"
#include "simResource.h"
#include "console.h"
#include "fear.strings.h"
#include "FearGuiArrayCtrl.h"
#include "FearGuiBox.h"
#include "fearGuiHelpCtrl.h"
#include "FearGuiUnivButton.h"

namespace SimGui
{

//also defined in FearGuiRecList.cpp
const char gPlaybackExtention[] = "recordings\\*.rec";
const char gPlaybackDir[] = "recordings\\";

class ConnectDelegate : public Delegate
{

private:
   typedef Delegate Parent;
   Control *curGui;
   
public:
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(ConnectDelegate);
   ConnectDelegate() { curGui = NULL; }
   
   DWORD onMessage(SimObject *sender, DWORD msg);
};

void ConnectDelegate::onNewContentControl(Control *, Control *newGui)
{
   curGui = newGui;
   FearGui::FGUniversalButton *radio;
   
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
   
   const char *nextMenu = Console->getVariable("pref::PlayGameMode");
   if (! stricmp(nextMenu, "HOST"))
   {
      radio = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDGUI_CREATE_SERVER));
      if (radio) radio->onAction();
   }
   else if (! stricmp(nextMenu, "TRAIN"))
   {
      radio = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDGUI_TEMP));
      if (radio) radio->onAction();
   }
   else
   {
      radio = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDGUI_JOIN_GAME));
      if (radio) radio->onAction();
   }
   
   const char *joinMode = Console->getVariable("pref::JoinGameMode");
   if (! stricmp(joinMode, "LAN"))
   {
      radio = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_SERVER_JOIN_LAN));
      if (radio) radio->onAction();
   }
   else
   {
      radio = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_SERVER_JOIN_INTERNET));
      if (radio) radio->onAction();
   }
}

DWORD ConnectDelegate::onMessage(SimObject *sender, DWORD msg)
{
/*
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   else return Parent::onMessage(sender, msg);
   AssertFatal(root, "Unable to get the root");
   
   if (msg == IDCTG_SHELL_NEXT)
   {
      const char *nextMenu = Console->getVariable("pref::PlayGameMode");
      if (! nextMenu || (! nextMenu[0]) || (! stricmp(nextMenu, "HOST")))
      {
         Parent::onMessage(sender, IDGUI_CREATE_SERVER);
         return -1;
      }
      else if (! stricmp(nextMenu, "JOIN"))
      {
         Parent::onMessage(sender, IDGUI_JOIN_GAME);
         return -1;
      }
   }
*/   
   return Parent::onMessage(sender, msg);
}

IMPLEMENT_PERSISTENT(ConnectDelegate);

};