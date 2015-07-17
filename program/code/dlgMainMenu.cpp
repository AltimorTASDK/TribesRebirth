#include "simGuiDelegate.h"
#include "simGuiBitmapCtrl.h"
#include "simGuiHelpCtrl.h"
#include "simGuiSimpleText.h"
#include "simResource.h"
#include "console.h"
#include "fear.strings.h"
#include "version.h"
#include "FearGuiArrayCtrl.h"
#include "FearGuiRecList.h"
#include "fearGuiHelpCtrl.h"

namespace SimGui
{

//also defined in FearGuiRecList.cpp
const char gPlaybackExtention[] = "recordings\\*.rec";
const char gPlaybackDir[] = "recordings\\";

class MainMenuDelegate : public Delegate
{

private:
   typedef Delegate Parent;
   Control *curGui;
   
public:
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(MainMenuDelegate);
   MainMenuDelegate() { curGui = NULL; }
};

void MainMenuDelegate::onNewContentControl(Control *, Control *newGui)
{
   //CMDConsole* console = CMDConsole::getLocked();
   curGui = newGui;
   
   //set the help control
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   if (root)
   {
      SimGui::HelpCtrl *hc = root->getHelpCtrl();
      if (! hc)
      {
         FearGui::FGHelpCtrl *helpCtrl = new FearGui::FGHelpCtrl();
         root->setHelpCtrl(helpCtrl);
      }
   }
   
   //set the version text
   SimGui::SimpleText *vc = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_SRVR_INFO_VERSION));
   if (vc)
   {
      vc->setText(avar("Version %s", FearVersion));
   }
}

IMPLEMENT_PERSISTENT(MainMenuDelegate);

};