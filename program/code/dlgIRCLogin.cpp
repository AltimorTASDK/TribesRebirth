#include "console.h"
#include "fear.strings.h"
#include "fearCSDelegate.h"
#include "simGuiDelegate.h"
#include "simGuiCtrl.h"
#include "simGuiTextEdit.h"
//#include "fearGuiArrayCtrl.h"
//#include "fearGuiCombo.h"
//#include "fearGuiIRCServersCB.h"
#include "fearGuiUnivButton.h"

namespace SimGui
{

class IRCLoginDelegate : Delegate
{

private:
   typedef Delegate Parent;
   Control *curGui;
   
public:
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(IRCLoginDelegate);
   IRCLoginDelegate() { curGui = NULL; }
   
   DWORD onMessage(SimObject *sender, DWORD msg);
};

void IRCLoginDelegate::onNewContentControl(Control *, Control *newGui)
{
   curGui = newGui;
}

DWORD IRCLoginDelegate::onMessage(SimObject *sender, DWORD msg)
{
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   else return Parent::onMessage(sender, msg);
   AssertFatal(root, "Unable to get the root");
   
   if (msg == IDCTG_IRC_NICK_TEXT)
   {
      SimGui::TextEdit *nickTextCtrl = dynamic_cast<SimGui::TextEdit*>(curGui->findControlWithTag(IDCTG_IRC_NICK_TEXT));
      if (! nickTextCtrl) return -1;
      FearGui::FGUniversalButton *connectBtn = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_IRC_CONNECT));
      if (! connectBtn) return -1;
      
      bool foundText = FALSE;
      char buf[256], *bufPtr = &buf[0];
      
      //see if there is any text in the nickTextCtrl
      nickTextCtrl->getText(buf);
      while (*bufPtr != '\0')
      {
         if (*bufPtr != ' ')
         {
            foundText = TRUE;
            break;
         }
         bufPtr++;
      }
      
      //set the done button
      connectBtn->active = foundText;
      connectBtn->setUpdate();
      
      return -1;
   }
   
   return Parent::onMessage(sender, msg);
}

IMPLEMENT_PERSISTENT(IRCLoginDelegate);

};