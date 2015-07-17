#include "simGuiDelegate.h"
#include "FearGuiTestEdit.h"
#include "console.h"
#include "fear.strings.h"
#include "simGuiTextList.h"

namespace SimGui
{

class MultiSetupDelegate : Delegate
{

private:
   typedef Delegate Parent;
   Control *curGui;
   
public:
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(MultiSetupDelegate);
   MultiSetupDelegate() { curGui = NULL; }
   DWORD onMessage(SimObject *sender, DWORD msg);
};

void MultiSetupDelegate::onNewContentControl(Control *, Control *newGui)
{
   CMDConsole* console = CMDConsole::getLocked();
   char buf[256];
   curGui = newGui;
   
   FearGui::TestEdit *edit = (FearGui::TestEdit *)(curGui->findControlWithTag(IDCTG_PLAYER_TS));
   if(edit)
   {
      const char *var = console->getVariable("Pref::PlayerName");
      edit->setText(var);
   }
}

DWORD MultiSetupDelegate::onMessage(SimObject *sender, DWORD msg)
{
   if(msg == IDGUI_MULTI_PLAYER)
   {
      FearGui::TestEdit *edit = (FearGui::TestEdit *)(curGui->findControlWithTag(IDCTG_PLAYER_TS));
      if(edit)
      {
         char buf[Inspect::MAX_STRING_LEN + 1];
         edit->getText(buf);
        	CMDConsole* console = CMDConsole::getLocked();
        	console->setVariable("Pref::PlayerName", buf);
      }
   }
   return Parent::onMessage(sender, msg);
}

IMPLEMENT_PERSISTENT(MultiSetupDelegate);

};