#include "console.h"
#include "fear.strings.h"
#include "simInput.h"
#include "simGuiDelegate.h"
#include "simGuiCtrl.h"
#include "simGuiTextEdit.h"
#include "fearGuiCombo.h"
#include "fearGuiStandardCombo.h"
#include "fearGuiArrayCtrl.h"
#include "FearGuiFilterCombo.h"
#include "FearGuiFilters.h"
#include "FearGuiServerFilterCB.h"
#include "FearGuiUnivButton.h"
#include "FearGuiCFGButton.h"
#include "fearGuiBox.h"
#include "gwDeviceManager.h"
#include "gwCanvas.h"


namespace SimGui
{

class OptionsDelegate : public Delegate
{

private:
   typedef Delegate Parent;
   Control *curGui;
   SimGui::Canvas *root;
   
   void initScreenTypeComboBoxes();
   void initScreenResComboBox(bool usePref = FALSE);
   bool mbChangingVideoModes;
   
   //filters
   int curFilter;
   bool verifyFilterSelected(void);
   
   //control config vars
   void setInfoText(const char *name, const char *map);
   void setCancelButtonState(bool value);
   FearGui::CFGButton *ctrlCFG;
   Vector<FearGui::CFGButton *> cfgButtons;
   
public:
   OptionsDelegate() { curGui = NULL; root = NULL; }
   void onNewContentControl(Control *oldGui, Control *newGui);
   void onLoseDelegation(void);
   DWORD onMessage(SimObject *sender, DWORD msg);
   void preRender(void);
   
   bool processQuery(SimQuery *query);
   bool processEvent(const SimEvent *event);
   bool onSimInputEvent(const SimInputEvent *event);
   
   void defineNewAction(const SimInputEvent *event);

   DECLARE_PERSISTENT(OptionsDelegate);
};

void OptionsDelegate::onNewContentControl(Control *, Control *newGui)
{
   curGui = newGui;
   
   if (curGui->getTag() == IDGUI_OPTIONS)
   {
      //build the list of CFG buttons
      cfgButtons.clear();
      
      //first find all the pages where they exist
      SimGui::Control *page7 = curGui->findControlWithTag(IDCTG_GS_PAGE_1);
      if (page7)
      {
         SimObject **i;
         for (i = page7->begin(); i != page7->end(); i++)
         {
            FearGui::CFGButton *ctrl = dynamic_cast<FearGui::CFGButton *>(*i);
            if (ctrl)
            {
               cfgButtons.push_back(ctrl);
            }
         }
      }
      
      SimGui::Control *page8 = curGui->findControlWithTag(IDCTG_GS_PAGE_2);
      if (page8)
      {
         SimObject **i;
         for (i = page8->begin(); i != page8->end(); i++)
         {
            FearGui::CFGButton *ctrl = dynamic_cast<FearGui::CFGButton *>(*i);
            if (ctrl)
            {
               cfgButtons.push_back(ctrl);
            }
         }
      }
      
      SimGui::Control *page9 = curGui->findControlWithTag(IDCTG_GS_PAGE_3);
      if (page9)
      {
         SimObject **i;
         for (i = page9->begin(); i != page9->end(); i++)
         {
            FearGui::CFGButton *ctrl = dynamic_cast<FearGui::CFGButton *>(*i);
            if (ctrl)
            {
               cfgButtons.push_back(ctrl);
            }
         }
      }
      
      SimGui::Control *page11 = curGui->findControlWithTag(IDCTG_GS_PAGE_4);
      if (page11)
      {
         SimObject **i;
         for (i = page11->begin(); i != page11->end(); i++)
         {
            FearGui::CFGButton *ctrl = dynamic_cast<FearGui::CFGButton *>(*i);
            if (ctrl)
            {
               cfgButtons.push_back(ctrl);
            }
         }
      }
      
      //get the root
      root = curGui->getCanvas();
      AssertFatal(root, "Unable to get the root");
      
      initScreenTypeComboBoxes();
      
      verifyFilterSelected();
      
      //filter vars
      curFilter = 0;
      
      //control config vars
      ctrlCFG = NULL;
      setInfoText("", "");
      
      //disable the cancel button
      setCancelButtonState(FALSE);
      
      //write out the action maps to a temporary file
      Console->executef(5, "saveActionMap", "temp\\tempCFG.cs", "actionMap.sae", "playMap.sae", "pdaMap.sae");
      
      //re-evaluate the search path
      Console->evaluate("EvalSearchPath();", FALSE);
      
   }
   else
   {
      //write out the action maps
      Console->executef(5, "saveActionMap", "config\\config.cs", "actionMap.sae", "playMap.sae", "pdaMap.sae");
   }
   
   //check the video mode
   mbChangingVideoModes = FALSE;
   bool fs = FALSE;
   if (root) fs = root->isFullScreen();
   Console->setBoolVariable("$pref::VideoFullScreen", fs);
   Console->executef(1, "OptionsVideoMode::update");
}

void OptionsDelegate::onLoseDelegation(void)
{
   //write out the action maps
   Console->executef(5, "saveActionMap", "config\\config.cs", "actionMap.sae", "playMap.sae", "pdaMap.sae");
}

void OptionsDelegate::preRender(void)
{
   //unless we actually click on a radio button to change modes, reflect the current
   if (! mbChangingVideoModes)
   {
      bool fs = FALSE;
      if (root) fs = root->isFullScreen();
      Console->setBoolVariable("$pref::VideoFullScreen", fs);
      Console->executef(1, "OptionsVideoMode::update");
   }
}

bool OptionsDelegate::verifyFilterSelected(void)
{
   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   else return FALSE;
   AssertFatal(root, "Unable to get the root");
   
   //enable the join game button
   FearGui::FGServerFilterComboBox *filters = dynamic_cast<FearGui::FGServerFilterComboBox*>(curGui->findControlWithTag(IDCTG_FILTER_LIST));
   FearGui::FilterCtrl *conditions = dynamic_cast<FearGui::FilterCtrl*>(curGui->findControlWithTag(IDCTG_CONDITION_LIST));
   FearGui::FearGuiBox *condBox = dynamic_cast<FearGui::FearGuiBox*>(curGui->findControlWithTag(IDCTG_CONDITION_BOX));
   FearGui::FGUniversalButton *removeFilter = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_REMOVE_FILTER));
   FearGui::FGUniversalButton *addCond = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_NEW_CONDITION));
   FearGui::FGUniversalButton *removeCond = dynamic_cast<FearGui::FGUniversalButton*>(curGui->findControlWithTag(IDCTG_REMOVE_CONDITION));
   if (filters && conditions && condBox&& removeFilter && addCond && removeCond)
   {
      int filterSel = filters->getCurrentFilter();
      if (filterSel > 0)
      {
         conditions->setCurEntry(filterSel);
         conditions->active = TRUE;
         condBox->setGhosted(FALSE);
         removeFilter->active = TRUE;
         addCond->active = TRUE;
         
         Point2I condSel = conditions->getSelectedCell();
         if (condSel.y >= 0)
         {
            removeCond->active = TRUE;
         }
         else
         {
            removeCond->active = FALSE;
         }
      }
      else
      {
         conditions->active = FALSE;
         condBox->setGhosted(TRUE);
         removeFilter->active = FALSE;
         addCond->active = FALSE;
         removeCond->active = FALSE;
      }
      conditions->setUpdate();
      condBox->setUpdate();
      removeFilter->setUpdate();
      addCond->setUpdate();
      removeCond->setUpdate();
   }
   return FALSE;
}

void OptionsDelegate::initScreenTypeComboBoxes()
{
   //set up the video drivers combo
   FearGui::FGStandardComboBox *windowed_cb = static_cast<FearGui::FGStandardComboBox *>(root->getContentControl()->findControlWithTag(IDCTG_OPTS_VID_WINDOW));
   if (! windowed_cb) return;
   windowed_cb->clear();
   
   FearGui::FGStandardComboBox *fullscreen_cb = static_cast<FearGui::FGStandardComboBox *>(root->getContentControl()->findControlWithTag(IDCTG_OPTS_VID_FULLSCRN));
   if (! fullscreen_cb) return;
   fullscreen_cb->clear();
   
   GWCanvas *gwc = dynamic_cast<GWCanvas *>(manager->findObject("MainWindow"));
   AssertFatal(gwc, "No MainWindow!!  What in the world?");
   GFXDevice *winDev = gwc->getWindowedDevice();
   GFXDevice *fsDev = gwc->getFullScreenDevice();
   Int32 winDeviceId = (winDev) ? winDev->getDeviceId() : GFX_SOFTWARE_DEVICE;
   Int32 fsDeviceId = (fsDev) ? fsDev->getDeviceId() : GFX_SOFTWARE_DEVICE;

   GFXDeviceDescriptor *deviceList;
   int deviceCount;
   deviceManager.enumerateDevices(deviceCount, deviceList);
   for(int i = 0; i < deviceCount; i++)
   {
      char nameBuffer[256];
      strcpy(nameBuffer, deviceList[i].name);
      if (deviceList[i].pRendererName != NULL) {
         strcat(nameBuffer, " [");
         strcat(nameBuffer, deviceList[i].pRendererName);
         strcat(nameBuffer, "]");
      }

      if (deviceList[i].flags & GFXDeviceDescriptor::runsFullscreen)
      {
         fullscreen_cb->addEntry(nameBuffer, deviceList[i].deviceId);
      }
      if (deviceList[i].flags & GFXDeviceDescriptor::runsWindowed)
      {
         windowed_cb->addEntry(nameBuffer, deviceList[i].deviceId);
      }   
   }
   
   //now set the combo boxes
   int tempID = -1;
   const char *deviceName = Console->getVariable("pref::VideoFullScreenDriver");
   if (deviceName[0]) tempID = fullscreen_cb->findEntry(deviceName);
   if (tempID < 0) tempID = fsDeviceId;
   fullscreen_cb->setSelectedEntry(tempID);
   
   tempID = -1;
   deviceName = Console->getVariable("pref::VideoWindowedDriver");
   if (deviceName[0]) tempID = windowed_cb->findEntry(deviceName);
   if (tempID < 0) tempID = winDeviceId;
   windowed_cb->setSelectedEntry(tempID);
   
   //now initialize the res combo box
   initScreenResComboBox(TRUE);
}   

void OptionsDelegate::initScreenResComboBox(bool usePref)
{
   FearGui::FGStandardComboBox *windowed_cb = static_cast<FearGui::FGStandardComboBox *>(root->getContentControl()->findControlWithTag(IDCTG_OPTS_VID_WINDOW));
   if (! windowed_cb) return;
   
   FearGui::FGStandardComboBox *fullscreen_cb = static_cast<FearGui::FGStandardComboBox *>(root->getContentControl()->findControlWithTag(IDCTG_OPTS_VID_FULLSCRN));
   if (! fullscreen_cb) return;
   
   FearGui::FGStandardComboBox *fs_res_cb = static_cast<FearGui::FGStandardComboBox *>(root->getContentControl()->findControlWithTag(IDCTG_OPTS_VID_RES));
   if (! fs_res_cb) return;
   
   char tempRes[256];
   const char *resString;
   if (usePref)
   {
      resString = Console->getVariable("pref::VideoFullScreenRes");
   }
   else
   {
      resString = fs_res_cb->getSelectedText();
   }
   if (resString)
   {
      strcpy(tempRes, resString);
   }
   else
   {
      tempRes[0] = '\0';
   }
   fs_res_cb->clear();
   
   char buffer[256];
   int temp = fullscreen_cb->getSelectedEntry();
   if (temp >= 0)
   {
      GFXDeviceDescriptor *deviceList;
      int deviceCount;

      char buffer[256];
      strcpy(buffer, fullscreen_cb->getSelectedText());
      char* pSpace = strchr(buffer, ' ');
      if (pSpace != NULL)
         *pSpace = '\0';

      deviceManager.enumerateDevices(deviceCount, deviceList);
      for(int i = 0; i < deviceCount; i++)
      {
         if (!strcmp(deviceList[i].name, buffer))
         {
            for (int j =0; j < deviceList[i].resolutionCount; j++)
            {
               sprintf(buffer, "%dx%d", deviceList[i].resolutionList[j].res.x, deviceList[i].resolutionList[j].res.y);
               if (deviceList[i].resolutionList[j].isVirtual)
                  strcat(buffer, "(V)");
               fs_res_cb->addEntry(buffer, j);
            }
         }
      }
   }
   
   //now set the resolution to the same as you had before...
   temp = -1;
   if (tempRes[0])
   {
      temp = fs_res_cb->findEntry(tempRes);
   }
   if (temp < 0) temp = 0;
   fs_res_cb->setSelectedEntry(temp);
}  

bool OptionsDelegate::processQuery(SimQuery *query)
{
   if (query->type == SimInputPriorityQueryType)
   {
      SimInputPriorityQuery *q = (SimInputPriorityQuery*)query;
      q->priority = SI_PRIORITY_INPUT_DETECT;
      return (true);
   }
   return Parent::processQuery(query);
}   

bool OptionsDelegate::processEvent(const SimEvent *event)
{
   switch(event->type)
   {
      onEvent(SimInputEvent);
      default:
         return Parent::processEvent(event);
   }
}

bool OptionsDelegate::onSimInputEvent(const SimInputEvent *event)
{
   switch(event->objType)
   {
      case SI_POV:  
      case SI_XPOV: 
      case SI_YPOV:
      case SI_XAXIS:
      case SI_YAXIS:
      case SI_ZAXIS:  
      case SI_RXAXIS: 
      case SI_RYAXIS: 
      case SI_RZAXIS:  
      case SI_SLIDER:
         return FALSE;
         
      case SI_KEY:
         switch (event->objInst)
         {
            case DIK_GRAVE:
               return FALSE;
            case DIK_ESCAPE:
               defineNewAction(NULL);
               return TRUE;
               
            case DIK_LSHIFT:   
            case DIK_RSHIFT:   
            case DIK_LCONTROL: 
            case DIK_RCONTROL:  
            case DIK_LMENU:    
            case DIK_RMENU:    
            default:
               if (event->action == SI_BREAK)
               {
                  break;
               }
               else return TRUE;
         }
         break;
         
      default:
         break;
   }
   
   //if the event made it this far without exiting, define the new action
   defineNewAction(event);
   
   return TRUE;
}

//this is *really* lame - but the only way to keep all 4 options pages synched with the same message
void OptionsDelegate::setInfoText(const char *name, const char *map)
{
   SimGui::SimpleText *nameCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_OPTS_CTRL_CFG_NAME_1));
   SimGui::SimpleText *mapCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_OPTS_CTRL_CFG_MAP_1));
   if (nameCtrl && mapCtrl)
   {
      mapCtrl->setText(map);
      nameCtrl->setText(name);
      nameCtrl->position.x = mapCtrl->position.x + mapCtrl->extent.x;
   }
   nameCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_OPTS_CTRL_CFG_NAME_2));
   mapCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_OPTS_CTRL_CFG_MAP_2));
   if (nameCtrl && mapCtrl)
   {
      mapCtrl->setText(map);
      nameCtrl->setText(name);
      nameCtrl->position.x = mapCtrl->position.x + mapCtrl->extent.x;
   }
   nameCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_OPTS_CTRL_CFG_NAME_3));
   mapCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_OPTS_CTRL_CFG_MAP_3));
   if (nameCtrl && mapCtrl)
   {
      mapCtrl->setText(map);
      nameCtrl->setText(name);
      nameCtrl->position.x = mapCtrl->position.x + mapCtrl->extent.x;
   }
   nameCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_OPTS_CTRL_CFG_NAME_4));
   mapCtrl = dynamic_cast<SimGui::SimpleText*>(curGui->findControlWithTag(IDCTG_OPTS_CTRL_CFG_MAP_4));
   if (nameCtrl && mapCtrl)
   {
      mapCtrl->setText(map);
      nameCtrl->setText(name);
      nameCtrl->position.x = mapCtrl->position.x + mapCtrl->extent.x;
   }
}

//another lame related function
void OptionsDelegate::setCancelButtonState(bool value)
{
   SimGui::Control *page7 = curGui->findControlWithTag(IDCTG_MENU_PAGE_07);
   if (page7)
   {
      FearGui::FGUniversalButton *cancelButton = dynamic_cast<FearGui::FGUniversalButton*>(page7->findControlWithTag(IDCTG_OPTS_CTRL_CFG_REVERT));
      if (cancelButton)
      {
         cancelButton->active = value;
         cancelButton->setUpdate();
      }
   }
   SimGui::Control *page8 = curGui->findControlWithTag(IDCTG_MENU_PAGE_08);
   if (page8)
   {
      FearGui::FGUniversalButton *cancelButton = dynamic_cast<FearGui::FGUniversalButton*>(page8->findControlWithTag(IDCTG_OPTS_CTRL_CFG_REVERT));
      if (cancelButton)
      {
         cancelButton->active = value;
         cancelButton->setUpdate();
      }
   }
   SimGui::Control *page9 = curGui->findControlWithTag(IDCTG_MENU_PAGE_09);
   if (page9)
   {
      FearGui::FGUniversalButton *cancelButton = dynamic_cast<FearGui::FGUniversalButton*>(page9->findControlWithTag(IDCTG_OPTS_CTRL_CFG_REVERT));
      if (cancelButton)
      {
         cancelButton->active = value;
         cancelButton->setUpdate();
      }
   }
   SimGui::Control *page11 = curGui->findControlWithTag(IDCTG_MENU_PAGE_11);
   if (page11)
   {
      FearGui::FGUniversalButton *cancelButton = dynamic_cast<FearGui::FGUniversalButton*>(page11->findControlWithTag(IDCTG_OPTS_CTRL_CFG_REVERT));
      if (cancelButton)
      {
         cancelButton->active = value;
         cancelButton->setUpdate();
      }
   }
}


void OptionsDelegate::defineNewAction(const SimInputEvent *event)
{
   //remove from the input consumer set
   removeFromSet(SimInputConsumerSetId);
   
   //clear the info text
   setInfoText("", "");
   
   //make sure we actually are redefining an action
   if (! ctrlCFG) return;
   
   //unpop the control
   ctrlCFG->setDefine(FALSE);
   
   //if no event, assume we are cancelling
   if (! event) return;
   
   //first see if any of the other cfgButtons have this event defined
   FearGui::CFGButton *conflictCFG = NULL;
   for (int i = 0; i < cfgButtons.size(); i++)
   {
      if ((cfgButtons[i] != ctrlCFG) && cfgButtons[i]->bindingConflict(event))
      {
         conflictCFG = cfgButtons[i];
         break;
      }
   }

   //if conflict, remove its binding
   if (conflictCFG)
   {
      //set the info text
      setInfoText(" is no longer bound.", conflictCFG->getText());
      conflictCFG->removeBinding();
   }
   
   //set the new binding
   ctrlCFG->newBinding(event);
   ctrlCFG = NULL;
   
   //enable the cancel button
   setCancelButtonState(TRUE);
}

DWORD OptionsDelegate::onMessage(SimObject *sender, DWORD msg)
{
   SimGui::Canvas *root;
   
   if (curGui) root = curGui->getCanvas();
   else return Parent::onMessage(sender, msg);
   AssertFatal(root, "Unable to get the root");
   
   //--------------------------------------//
   //     FILTERS SECTION                  // 
   //--------------------------------------//
   //keep the filters up to date
   verifyFilterSelected();
   
   if (msg == IDDLG_ADD_CONDITION)
   {
      //push the dialog
      Console->executef(3, "GuiPushDialog", "MainWindow", SimTagDictionary::getString(manager, msg));
      
      //now set the conditional tables
      FearGui::FGFilterComboBox *filterVar = dynamic_cast<FearGui::FGFilterComboBox*>(root->getTopDialog()->findControlWithTag(IDCTG_FILTER_VAR));
      AssertFatal(filterVar, "Unable to find the filterVar popup.");
      FearGui::FGFilterComboBox *filterCond = dynamic_cast<FearGui::FGFilterComboBox*>(root->getTopDialog()->findControlWithTag(IDCTG_FILTER_COND));
      AssertFatal(filterCond, "Unable to find the filterVar popup.");
      SimGui::TextEdit *ctrl = dynamic_cast<SimGui::TextEdit*>(root->getTopDialog()->findControlWithTag(IDCTG_NEW_CONDITION_TEXT));
      AssertFatal(ctrl, "Unable to find the filter value testEdit.");
      
      //first set the table for filterVar
      filterVar->setTable(&FearGui::gFilterVars[0], FearGui::gNumFilterVars);
      filterVar->setMessage(IDCTG_FILTER_CHANGE_VAR);
      
      //next set the table for the conditions based on the variable from filterVar
      Point2I varSel = filterVar->getSelected();
      if (varSel.y >= 0)
      {
         if (FearGui::gFilterVars[varSel.y].type == FearGui::FilterCtrl::FT_COND_NUMERICAL)
         {
            ctrl->active = TRUE;
            filterCond->setTable(&FearGui::gFilterNumberConds[0], FearGui::gNumFilterNumberConds);
            ctrl->setText("0");
         }
         else if (FearGui::gFilterVars[varSel.y].type == FearGui::FilterCtrl::FT_COND_TEXT)
         {
            ctrl->active = TRUE;
            filterCond->setTable(&FearGui::gFilterTextConds[0], FearGui::gNumFilterTextConds);
            ctrl->setText("");
         }
         else
         {
            ctrl->active = FALSE;
            filterCond->setTable(&FearGui::gFilterBoolConds[0], FearGui::gNumFilterBoolConds);
            ctrl->setText("");
         }
      }
      
      return -1;
   }
   
   else if (msg == IDCTG_FILTER_CHANGE_VAR)
   {
      //find the conditional tables
      FearGui::FGFilterComboBox *filterVar = dynamic_cast<FearGui::FGFilterComboBox*>(root->getTopDialog()->findControlWithTag(IDCTG_FILTER_VAR));
      AssertFatal(filterVar, "Unable to find the filterVar popup.");
      FearGui::FGFilterComboBox *filterCond = dynamic_cast<FearGui::FGFilterComboBox*>(root->getTopDialog()->findControlWithTag(IDCTG_FILTER_COND));
      AssertFatal(filterCond, "Unable to find the filterVar popup.");
      SimGui::TextEdit *ctrl = dynamic_cast<SimGui::TextEdit*>(root->getTopDialog()->findControlWithTag(IDCTG_NEW_CONDITION_TEXT));
      AssertFatal(ctrl, "Unable to find the filter value testEdit.");
      
      //next set the table for the conditions based on the variable from filterVar
      Point2I varSel = filterVar->getSelected();
      if (varSel.y >= 0)
      {
         if (FearGui::gFilterVars[varSel.y].type == FearGui::FilterCtrl::FT_COND_NUMERICAL)
         {
            ctrl->active = TRUE;
            filterCond->setTable(&FearGui::gFilterNumberConds[0], FearGui::gNumFilterNumberConds);
            ctrl->setText("0");
         }
         else if (FearGui::gFilterVars[varSel.y].type == FearGui::FilterCtrl::FT_COND_TEXT)
         {
            ctrl->active = TRUE;
            filterCond->setTable(&FearGui::gFilterTextConds[0], FearGui::gNumFilterTextConds);
            ctrl->setText("");
         }
         else
         {
            ctrl->active = FALSE;
            filterCond->setTable(&FearGui::gFilterBoolConds[0], FearGui::gNumFilterBoolConds);
            ctrl->setText("");
         }
      }
   }
   
   else if (msg == IDCTG_NEW_CONDITION)
   {
      //find the conditional tables
      FearGui::FGFilterComboBox *filterVar = dynamic_cast<FearGui::FGFilterComboBox*>(root->getTopDialog()->findControlWithTag(IDCTG_FILTER_VAR));
      AssertFatal(filterVar, "Unable to find the filterVar popup.");
      FearGui::FGFilterComboBox *filterCond = dynamic_cast<FearGui::FGFilterComboBox*>(root->getTopDialog()->findControlWithTag(IDCTG_FILTER_COND));
      AssertFatal(filterCond, "Unable to find the filterVar popup.");
      SimGui::TextEdit *ctrl = dynamic_cast<SimGui::TextEdit*>(root->getTopDialog()->findControlWithTag(IDCTG_NEW_CONDITION_TEXT));
      AssertFatal(ctrl, "Unable to find the filter value testEdit.");
      FearGui::FilterCtrl *conditions = dynamic_cast<FearGui::FilterCtrl*>(curGui->findControlWithTag(IDCTG_CONDITION_LIST));
      AssertFatal(conditions, "Unable to find the conditions list.");
      
      //get the var and cond selected
      Point2I varSel = filterVar->getSelected();
      Point2I condSel = filterCond->getSelected();
      
      //get the entered value
      char value[255];
      ctrl->getText(value);
      
      //make sure if we're doing a numerical condition, the text is a "number"
      if (varSel.y >= 0)
      {
         if (FearGui::gFilterVars[varSel.y].type == FearGui::FilterCtrl::FT_COND_NUMERICAL)
         {
            int tempValue = atoi(value);
            sprintf(value, "%d", tempValue);
            
            //add the new condition
            conditions->addCondition(varSel.y, condSel.y, &value[0]);
         }
         else if (FearGui::gFilterVars[varSel.y].type == FearGui::FilterCtrl::FT_COND_BOOL)
         {
            //add the new condition
            conditions->addCondition(varSel.y, condSel.y, "-1");
         }
         else
         {
            //add the new condition
            conditions->addCondition(varSel.y, condSel.y, &value[0]);
         }
      }
      
      //close the dialog
      root->makeFirstResponder(NULL);
      root->popDialogControl();
      
      //keep the edit and remove buttons up to date
      verifyFilterSelected();
      
      return -1;
   }
   
   else if (msg == IDCTG_REMOVE_CONDITION)
   {
      FearGui::FilterCtrl *conditions = dynamic_cast<FearGui::FilterCtrl*>(curGui->findControlWithTag(IDCTG_CONDITION_LIST));
      if (conditions)
      {
         conditions->removeSelected();
         
         //keep the filters up to date
         verifyFilterSelected();
      }
   }
   
   else if (msg == IDCTG_FILTER_LIST)
   {
      FearGui::FGServerFilterComboBox *filters = dynamic_cast<FearGui::FGServerFilterComboBox*>(curGui->findControlWithTag(IDCTG_FILTER_LIST));
      FearGui::FilterCtrl *conditions = dynamic_cast<FearGui::FilterCtrl*>(curGui->findControlWithTag(IDCTG_CONDITION_LIST));
      if (filters)
      {
         conditions->setCurEntry(filters->getCurrentFilter());
         
         //keep the edit and remove buttons up to date
         verifyFilterSelected();
      }
   }
   
   else if (msg == IDCTG_NEW_FILTER)
   {
      //find the conditional tables
      SimGui::TextEdit *ctrl = dynamic_cast<SimGui::TextEdit*>(root->getTopDialog()->findControlWithTag(IDCTG_NEW_FILTER_TEXT));
      AssertFatal(ctrl, "Unable to find the filter value testEdit.");
      FearGui::FGServerFilterComboBox *filters = dynamic_cast<FearGui::FGServerFilterComboBox*>(curGui->findControlWithTag(IDCTG_FILTER_LIST));
      AssertFatal(filters, "Unable to find the filters list.");
      FearGui::FilterCtrl *conditions = dynamic_cast<FearGui::FilterCtrl*>(curGui->findControlWithTag(IDCTG_CONDITION_LIST));
      AssertFatal(conditions, "Unable to find the conditions list.");
      
      char newFilter[255];
      ctrl->getText(newFilter);
      if (newFilter[0] != '\0')
      {
         int filterNum = filters->addFilter(newFilter);
         conditions->setCurEntry(filterNum);
      }
      
      root->makeFirstResponder(NULL);
      root->popDialogControl();
      
      //keep the edit and remove buttons up to date
      verifyFilterSelected();
      
      return -1;
   }
   
   else if (msg == IDCTG_REMOVE_FILTER)
   {
      FearGui::FGServerFilterComboBox *filters = dynamic_cast<FearGui::FGServerFilterComboBox*>(curGui->findControlWithTag(IDCTG_FILTER_LIST));
      FearGui::FilterCtrl *conditions = dynamic_cast<FearGui::FilterCtrl*>(curGui->findControlWithTag(IDCTG_CONDITION_LIST));
      if (filters)
      {
         filters->removeFilter();
         conditions->setCurEntry(-1);
         
         //keep the edit and remove buttons up to date
         verifyFilterSelected();
      }
   }
   
   //--------------------------------------//
   //     CONTROL CONFIG SECTION           // 
   //--------------------------------------//
   else if (msg == IDCTG_OPTS_CTRL_CFG_SELECT)
   {
      //make sure the sender is a CFGButton
      ctrlCFG = dynamic_cast<FearGui::CFGButton *>(sender);
      if (! ctrlCFG) return -1;
      
      //set the state
      ctrlCFG->setDefine(TRUE);
      
      //set the info text
      setInfoText("Press the new key or button...", "");
      
      //push the delegate to the top level of receiving events
      addToSet(SimInputConsumerSetId);
      
      return -1;
   }
   
   else if (msg == IDCTG_OPTS_CTRL_CFG_REVERT)
   {
      Console->executef(2, "exec", "tempCFG.cs");
      Console->executef(2, "exec", "config.cs");
      
      //now refresh all the bindings
      for (int i = 0; i < cfgButtons.size(); i++)
      {
         cfgButtons[i]->refreshBinding();
      }
      
      //disable the cancel button
      setCancelButtonState(FALSE);
   }
   
   else if (msg == IDCTG_OPTS_CTRL_CFG_DEFAULTS)
   {
      Console->executef(2, "exec", "sae.cs");
      
      //now refresh all the bindings
      for (int i = 0; i < cfgButtons.size(); i++)
      {
         cfgButtons[i]->refreshBinding();
      }
      
      //disable the cancel button
      //setCancelButtonState(FALSE);
      
      //set the text
      setInfoText("", "");
      
   }
   
   //--------------------------------------//
   //     IRC OPTIONS SECTION              // 
   //--------------------------------------//
   else if (msg == IDCTG_IRC_NEW_SERVER)
   {
      SimGui::TextEdit *ctrl = dynamic_cast<SimGui::TextEdit*>(root->getTopDialog()->findControlWithTag(IDCTG_IRC_SERVER_DESC));
      if (ctrl)
      {
         char newServer[255];
         ctrl->getText(newServer);
         if (newServer[0] != '\0')
         {
            Console->executef(2, "IRCOptions::Add", newServer);
         }
      }
      
      root->makeFirstResponder(NULL);
      root->popDialogControl();
      
      return -1;
   }
   
   //--------------------------------------//
   //     VIDEO OPTIONS SECTION            // 
   //--------------------------------------//
   else if (msg == IDCTG_OPTS_VID_FULLSCRN)
   {
      initScreenResComboBox(FALSE);
      
      //also, update the openGL combo
      Console->executef(1, "OptionsVideo::OpenGLsetup");
      
      return -1;
   }
   else if (msg == IDSTR_VID_FULLSCRN_MODE)
   {
      mbChangingVideoModes = TRUE;
   }
   
   return Parent::onMessage(sender, msg);
}

IMPLEMENT_PERSISTENT(OptionsDelegate);

};