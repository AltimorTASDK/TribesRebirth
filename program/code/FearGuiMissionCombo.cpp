//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simResource.h"
#include "FearGuiMissionCombo.h"
#include "simGuiScrollCtrl.h"
#include "fearGuiScrollCtrl.h"
#include "fearGuiMissionList.h"
#include "simGuiArrayCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "console.h"

namespace FearGui
{
 
bool FGMissionPopUp::onAdd()
{
   //can't do the parent::onAdd(), since it also sets up sc, and ac
   if(!Parent::onAdd())
      return false;

   //create the popup menu
   FearGuiScrollCtrl *fgSC = new FearGuiScrollCtrl();
   AssertFatal(fgSC, "could not allocate memory");
   fgSC->setBorderThickness(6);
   fgSC->forceVScrollBar = SimGui::ScrollCtrl::ScrollBarDynamic;
   fgSC->forceHScrollBar = SimGui::ScrollCtrl::ScrollBarAlwaysOff;
   fgSC->pbaTag = IDPBA_SCROLL_SHELL;
   fgSC->willFirstRespond = false;
   fgSC->extent.set(218, 96);
   sc = fgSC;
   manager->addObject(sc);
   addObject(sc);
   SimGui::Control *ctrl = (SimGui::Control*)(sc->getScrollContentCtrl());
   if (ctrl)
   {
      ctrl->mbBoarder = TRUE;
      ctrl->boarderColor = GREEN_78;
      ctrl->mbOpaque = TRUE;
      ctrl->fillColor = GREEN_78;
   }
   
   ac = new MissionListCtrl();
   AssertFatal(ac, "could not allocate memory");
   ac->extent.set(640, 96);
   manager->addObject(ac);
   sc->addObject(ac);
   ac->message = IDSTR_STRING_DEFAULT; // we need the text list to
                                       // give the pop up ctrl some
                                       // message indicating that
                                       // mouseUp or keyUp has occured
   //setBoarder();
   return true;
}  

//also declared in FearGuiMissionList.cpp
const char gMissionExtention[] = "missions\\*.mis";
const char gMissionDir[] = "missions\\";

//------------------------------------------------------------------------------
bool FGMissionComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGMissionPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   
   //set the text
   char buffer[256];
   const char *lastMisName;
   
   lastMisName = Console->getVariable("pref::lastMission");
   sprintf(buffer, "%s%s.mis", gMissionDir, lastMisName);
   
   WIN32_FIND_DATA data;
   HANDLE handle = FindFirstFile(buffer, &data);
   if (handle != INVALID_HANDLE_VALUE)
   {
      setText(lastMisName);
      setVariable(data.cFileName);
   }
   else handle = FindFirstFile(gMissionExtention, &data);
   if (handle != INVALID_HANDLE_VALUE)
   {
      data.cFileName[strlen(data.cFileName) - 4] = '\0';
      setText(data.cFileName);
      setVariable(data.cFileName);
   }
   
   //set the title bmp
   mTitleBMP = SimResource::get(manager)->load("POP_Mission.BMP");
   
   return true;
}  
 
void FGMissionComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      // set up the data for the pop up ctrl
      MissionListCtrl *mlCtrl = (MissionListCtrl*)getArrayCtrl();
      AssertFatal(mlCtrl, "pop up control is missing a text list");
      mlCtrl->onWake();
      Point2I globalPos = localToGlobalCoord(Point2I(0,0));
      popUpCtrl->setPos(globalPos.x - 1, globalPos.y + extent.y - 1);
      popUpCtrl->setDim(extent.x + 2, 96, 96);
      root->pushDialogControl(popUpCtrl);
      root->tabNext();
      popUpActive = true;
      root->mouseLock(popUpCtrl);
   }
}   

void FGMissionComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   MissionListCtrl *mlCtrl = (MissionListCtrl*)(popUpCtrl->getArrayCtrl());
   setText(mlCtrl->getSelectedMission());
}

IMPLEMENT_PERSISTENT_TAG(FGMissionComboBox, FOURCC('F','G','m','c'));
 
};