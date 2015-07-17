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
#include "FearGuiServerFilterCB.h"
#include "simGuiScrollCtrl.h"
#include "fearGuiScrollCtrl.h"
#include "fearGuiFilterCBList.h"
#include "simGuiArrayCtrl.h"
#include "soundFX.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "console.h"

namespace FearGui
{

static Sfx::Profile base2dSfxProfile;

static const char *curFilterVariable = "pref::UseFilter";
int FGServerFilterComboBox::mCurFilter = 0;
 
bool FGServerFilterPopUp::onAdd()
{
   //can't do the parent::onAdd(), since it also sets up sc, and ac
   if(!Parent::onAdd())
      return false;

   //initialize the 2d sound profile
   base2dSfxProfile.flags = 0;
   base2dSfxProfile.baseVolume = 0;
   base2dSfxProfile.coneInsideAngle = 90;
   base2dSfxProfile.coneOutsideAngle= 180; 
   base2dSfxProfile.coneVector.set(0.0f, -1.0f, 0.0f);
   base2dSfxProfile.coneOutsideVolume = 0.0f;  
   base2dSfxProfile.minDistance = 30.0f;     
   base2dSfxProfile.maxDistance = 300.0f;     
   
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
   
   ac = new FGFilterCBListCtrl();
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

//------------------------------------------------------------------------------
bool FGServerFilterComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGServerFilterPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   
	Console->addVariable(0, curFilterVariable, CMDConsole::Int, &mCurFilter);
   
   //set the title bmp
   mTitleBMP = SimResource::get(manager)->load("POP_Filter_DF.BMP");
   
   return true;
}  
 
void FGServerFilterComboBox::onWake()
{
   FGFilterCBListCtrl *flCtrl = (FGFilterCBListCtrl*)getArrayCtrl();
   AssertFatal(flCtrl, "pop up control is missing a text list");
   
   flCtrl->onWake();
   
   const char *temp = flCtrl->getFilterName(mCurFilter);
   if (temp) setText(temp);
   else setText("<None>");
   
   char buf[64];
   sprintf(buf, "%d", mCurFilter);
   setVariable(buf);
}  

void FGServerFilterComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      //find the soundfx manager
      Sfx::Manager *sfx = Sfx::Manager::find(manager);
      if (sfx) sfx->play(stringTable.insert("Button5.wav"), 0, &base2dSfxProfile);
      
      // set up the data for the pop up ctrl
      FGFilterCBListCtrl *flCtrl = (FGFilterCBListCtrl*)getArrayCtrl();
      AssertFatal(flCtrl, "pop up control is missing a text list");
      flCtrl->onWake();
      Point2I globalPos = localToGlobalCoord(Point2I(0,0));
      popUpCtrl->setPos(globalPos.x - 1, globalPos.y + extent.y - 1);
      SimGui::ArrayCtrl *ac = popUpCtrl->getArrayCtrl();
      if (ac)
      {
         popUpCtrl->setDim(extent.x + 2, ac->extent.y, 240);
      }
      else
      {
         popUpCtrl->setDim(extent.x + 2, 240, 240);
      }
      root->pushDialogControl(popUpCtrl);
      root->tabNext();
      popUpActive = true;
      root->mouseLock(popUpCtrl);
      updateFromArrayCtrl();
   }
}   

void FGServerFilterComboBox::closePopUpCtrl()
{
   root->mouseUnlock();
   if (popUpCtrl == root->getTopDialog())
   {
      // get data from the pop up ctrl
      updateFromArrayCtrl();
      root->popDialogControl();
      root->makeFirstResponder(this);
      popUpActive = false;
      GreatGrandParent::onMessage(this, message);    
   }
}  

void FGServerFilterComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   FGFilterCBListCtrl *flCtrl = (FGFilterCBListCtrl*)(popUpCtrl->getArrayCtrl());
   const char *temp = flCtrl->getCurFilter(mCurFilter);
   if (temp) setText(temp);
   else setText("<None>");
   
   char buf[64];
   sprintf(buf, "%d", mCurFilter);
   setVariable(buf);
}

int FGServerFilterComboBox::addFilter(const char *filter)
{
   FGFilterCBListCtrl *flCtrl = (FGFilterCBListCtrl*)(popUpCtrl->getArrayCtrl());
   if (! flCtrl) return -1;
   
   int returnVal = flCtrl->addFilter(filter);
   mCurFilter = returnVal;
   if (returnVal >= 0)
   {
      setText(filter);
   }
   return returnVal;
   
}

void FGServerFilterComboBox::removeFilter(void)
{
   FGFilterCBListCtrl *flCtrl = (FGFilterCBListCtrl*)(popUpCtrl->getArrayCtrl());
   if (! flCtrl) return;
   flCtrl->removeFilter(mCurFilter);
   setText("<None>");
   mCurFilter = 0;
}
   
IMPLEMENT_PERSISTENT_TAG(FGServerFilterComboBox, FOURCC('F','G','s','x'));
 
};