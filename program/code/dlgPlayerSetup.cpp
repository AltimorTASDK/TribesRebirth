#include "console.h"
#include "tsfx.h"
#include "sound.h"
#include "fear.strings.h"
#include "fearCSDelegate.h"
#include "simGuiDelegate.h"
#include "simGuiCtrl.h"
#include "simGuiTextEdit.h"
#include "fearGuiStandardCombo.h"
#include "fearGuiUnivButton.h"
#include "fearGuiHelpCtrl.h"

namespace SimGui
{

static Sfx::Profile base2dSfxProfile;

class PlayerSetupDelegate : public Delegate
{

private:
   typedef Delegate Parent;
   Control *curGui;
   
public:
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(PlayerSetupDelegate);
   PlayerSetupDelegate() { curGui = NULL; }
   
   DWORD onMessage(SimObject *sender, DWORD msg);
};

void PlayerSetupDelegate::onNewContentControl(Control *, Control *newGui)
{
   curGui = newGui;
   
   //initialize the 2d sound profile
   base2dSfxProfile.flags = 0;
   base2dSfxProfile.baseVolume = 0;
   base2dSfxProfile.coneInsideAngle = 90;
   base2dSfxProfile.coneOutsideAngle= 180; 
   base2dSfxProfile.coneVector.set(0.0f, -1.0f, 0.0f);
   base2dSfxProfile.coneOutsideVolume = 0.0f;  
   base2dSfxProfile.minDistance = 30.0f;     
   base2dSfxProfile.maxDistance = 300.0f;     
   
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

DWORD PlayerSetupDelegate::onMessage(SimObject *sender, DWORD msg)
{
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   else return Parent::onMessage(sender, msg);
   AssertFatal(root, "Unable to get the root");

   if (msg == IDCTG_PLYR_CFG_TEXT_EDIT)
   {
      SimGui::TextEdit *teCtrl = dynamic_cast<SimGui::TextEdit*>(root->getTopDialog()->findControlWithTag(IDCTG_PLYR_CFG_TEXT_EDIT));
      if (! teCtrl) return -1;
      FearGui::FGUniversalButton *doneBtn = dynamic_cast<FearGui::FGUniversalButton*>(root->getTopDialog()->findControlWithTag(IDCTG_NEW_PLAYER));
      if (! doneBtn) return -1;
      
      bool foundText = FALSE;
      char buf[256], *bufPtr = &buf[0];
      teCtrl->getText(buf);
      
      //see if there is any text in it yet
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
      doneBtn->active = foundText;
      doneBtn->setUpdate();
   }
   
   else if (msg == IDCTG_NEW_PLAYER)
   {
      SimGui::TextEdit *ctrl = dynamic_cast<SimGui::TextEdit*>(root->getTopDialog()->findControlWithTag(IDCTG_PLYR_CFG_TEXT_EDIT));
      if (ctrl)
      {
         char newPlayer[255];
         ctrl->getText(newPlayer);
         if (newPlayer[0] != '\0')
         {
            Console->executef(2, "AddPlayerConfig", newPlayer);
         }
      }
      
      root->makeFirstResponder(NULL);
      root->popDialogControl();
      
      return -1;
   }
   
   else if (msg == IDCTG_PLYR_CFG_VOICE_TEST)
   {
      FearGui::FGStandardComboBox *voiceCB = dynamic_cast<FearGui::FGStandardComboBox*>(curGui->findControlWithTag(IDCTG_PLYR_CFG_VOICE));
      if (voiceCB)
      {
         char sndBuf[256];
         const char *voiceBase = voiceCB->getText();
         if (voiceBase)
         {
            //create the sound string
            sprintf(sndBuf, "%s.whello.wav", voiceBase);
            
            //add it to the soundfx manager
            Sfx::Manager *sfx = Sfx::Manager::find(manager);
            sfx->play(stringTable.insert(sndBuf), 0, &base2dSfxProfile);
         }
      }
   }
   
   
   return Parent::onMessage(sender, msg);
}

IMPLEMENT_PERSISTENT(PlayerSetupDelegate);

};