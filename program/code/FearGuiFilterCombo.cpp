//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "SoundFX.h"
#include "FearGuiFilterCombo.h"
#include "simGuiScrollCtrl.h"
#include "fearGuiScrollCtrl.h"
#include "simGuiArrayCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "FearGuiFilters.h"

namespace FearGui
{
 
static Sfx::Profile base2dSfxProfile;

bool FGFilterPopUp::onAdd()
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
   
   ac = new FilterVarCtrl();
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
Point2I FGFilterComboBox::getSelected(void)
{
   if ((! popUpCtrl) || (! popUpCtrl->getArrayCtrl()))
   {
      return Point2I(-1, -1);
   }
   else return popUpCtrl->getArrayCtrl()->getSelectedCell();
}

void FGFilterComboBox::setPopUpMessage(Int32 msg)
{
   if (popUpCtrl && popUpCtrl->getArrayCtrl())
   {
      popUpCtrl->getArrayCtrl()->setMessage(msg);
   }
}

void FGFilterComboBox::setTable(FilterCtrl::FilterVars *table, int tableSize)
{
   //set the table
   mTable = table;
   
   if (popUpCtrl && popUpCtrl->getArrayCtrl())
   {
      FilterVarCtrl *myAC = dynamic_cast<FilterVarCtrl*>(popUpCtrl->getArrayCtrl());
      if (myAC) myAC->setTable(table, tableSize);
      updateFromArrayCtrl();
   }
}

bool FGFilterComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGFilterPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   
   mTable = NULL;
   
   //set the text
   updateFromArrayCtrl();                                    
   return true;
}  
 
void FGFilterComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      //find the soundfx manager
      Sfx::Manager *sfx = Sfx::Manager::find(manager);
      if (sfx) sfx->play(stringTable.insert("Button5.wav"), 0, &base2dSfxProfile);
      
      // set up the data for the pop up ctrl
      FilterVarCtrl *mlCtrl = (FilterVarCtrl*)getArrayCtrl();
      AssertFatal(mlCtrl, "pop up control is missing a text list");
      mlCtrl->onWake();
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

void FGFilterComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   FilterVarCtrl *fvCtrl = (FilterVarCtrl*)(popUpCtrl->getArrayCtrl());
   Point2I selected = fvCtrl->getSelectedCell();
   if (selected.y >= 0 && mTable)
   {
      setText(SimTagDictionary::getString(manager, mTable[selected.y].tag));
   }
   else
   {
      setText("");
   }
}

IMPLEMENT_PERSISTENT_TAG(FGFilterComboBox, FOURCC('F','G','f','c'));
 
//------------------------------------------------------------------------------

bool FilterVarCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   mTable = NULL;
   setSize(Point2I( 1, 0));
   
   return true;
}

const char* FilterVarCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   return SimTagDictionary::getString(manager, mTable[cell.y].tag);
}

void FilterVarCtrl::setTable(FilterCtrl::FilterVars *table, int tableSize)
{
   if (table != mTable)
   {
      mTable = table;
      setSize(Point2I(1, tableSize));
      cellSelected(Point2I(0, 0));
   }
}

IMPLEMENT_PERSISTENT_TAG(FilterVarCtrl, FOURCC('F','G','f','v'));

};