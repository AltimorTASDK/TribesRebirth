//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "FearGuiGenderCombo.h"
#include "simGuiScrollCtrl.h"
#include "fearGuiScrollCtrl.h"
#include "simGuiArrayCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"

namespace FearGui
{

bool FGGenderPopUp::onAdd()
{
   //can't do the parent::onAdd(), since it also sets up sc, and ac
   if(!Parent::onAdd())
      return false;

   //create the popup menu
   sc = new FearGuiScrollCtrl();
   AssertFatal(sc, "could not allocate memory");
   sc->forceVScrollBar = SimGui::ScrollCtrl::ScrollBarDynamic;
   sc->forceHScrollBar = SimGui::ScrollCtrl::ScrollBarAlwaysOff;
   sc->pbaTag = IDPBA_SCROLL_SHELL;
   sc->willFirstRespond = false;
   sc->extent.set(218, 96);
   manager->addObject(sc);
   addObject(sc);
   
   ac = new GenderListCtrl();
   AssertFatal(ac, "could not allocate memory");
   ac->extent.set(218, 96);
   manager->addObject(ac);
   sc->addObject(ac);
   ac->message = IDSTR_STRING_DEFAULT; // we need the text list to
                                       // give the pop up ctrl some
                                       // message indicating that
                                       // mouseUp or keyUp has occured
   setBoarder();                                    
   return true;
}  

//------------------------------------------------------------------------------
bool FGGenderComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGGenderPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   return true;
}  
 
void FGGenderComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      // set up the data for the pop up ctrl
      GenderListCtrl *plCtrl = (GenderListCtrl*)getArrayCtrl();
      AssertFatal(plCtrl, "pop up control is missing a text list");
      plCtrl->onWake();
      Point2I globalPos = localToGlobalCoord(Point2I(0,0));
      popUpCtrl->setPos(globalPos.x - 1, globalPos.y + extent.y - 1);
      popUpCtrl->setDim(extent.x + 2, 96, 96);
      root->pushDialogControl(popUpCtrl);
      root->tabNext();
      popUpActive = true;
      root->mouseLock(popUpCtrl);
      updateFromArrayCtrl();
   }
}   

void FGGenderComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   GenderListCtrl *glCtrl = (GenderListCtrl*)(popUpCtrl->getArrayCtrl());
   int gender = glCtrl->getSelectedGender();
   setText(gender ? "FEMALE" : "MALE");
}

IMPLEMENT_PERSISTENT_TAG(FGGenderComboBox, FOURCC('F','G','g','d'));
 
IMPLEMENT_PERSISTENT_TAG(GenderListCtrl, FOURCC('F','G','g','l'));

bool GenderListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   return true;
}

void GenderListCtrl::onWake()
{
   setSize(Point2I( 1, 2));
}

int GenderListCtrl::getSelectedGender(void)
{
   return selectedCell.y;
}

const char* GenderListCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   return (cell.y ? "FEMALE" : "MALE");
}

};