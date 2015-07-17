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
#include "FearGuiPlayerCombo.h"
#include "FearGuiScrollCtrl.h"
#include "fearGuiPlayerList.h"
#include "simGuiArrayCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "console.h"

namespace FearGui
{

bool FGPlayerPopUp::onAdd()
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
   fgSC->extent.set(218, 200);
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
   
   ac = new PlayerListCtrl();
   AssertFatal(ac, "could not allocate memory");
   ac->extent.set(640, 200);
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
bool FGPlayerComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGPlayerPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   
   //set the title bmp
   mTitleBMP = SimResource::get(manager)->load("POP_Player_DF.BMP");
   mTitleGhostBMP = SimResource::get(manager)->load("POP_Player_BD.BMP");
   
   return true;
}  
 
void FGPlayerComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      // set up the data for the pop up ctrl
      PlayerListCtrl *plCtrl = (PlayerListCtrl*)getArrayCtrl();
      AssertFatal(plCtrl, "pop up control is missing a text list");
      plCtrl->onWake();
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

void FGPlayerComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   PlayerListCtrl *plCtrl = (PlayerListCtrl*)(popUpCtrl->getArrayCtrl());
   const char *player;
   player = plCtrl->getSelectedPlayer();
   if (player && player[0]) setText(player);
   else setText("");
   setUpdate();
}

IMPLEMENT_PERSISTENT_TAG(FGPlayerComboBox, FOURCC('F','G','p','c'));
 
};