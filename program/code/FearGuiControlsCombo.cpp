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
#include "FearGuiControlsCombo.h"
#include "simGuiScrollCtrl.h"
#include "fearGuiScrollCtrl.h"
#include "simGuiArrayCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "console.h"

namespace FearGui
{
 
bool FGControlsPopUp::onAdd()
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
      ctrl->boarderColor = STANDARD_COLOR;
      ctrl->mbOpaque = TRUE;
      ctrl->fillColor = STANDARD_COLOR;
   }
   
   ac = new ControlsListCtrl();
   AssertFatal(ac, "could not allocate memory");
   ac->extent.set(218, 96);
   manager->addObject(ac);
   sc->addObject(ac);
   ac->message = IDSTR_STRING_DEFAULT; // we need the text list to
                                       // give the pop up ctrl some
                                       // message indicating that
                                       // mouseUp or keyUp has occured
   //setBoarder();
   return true;
}  

const char gControlsExtention[] = "*.cfg";

//------------------------------------------------------------------------------
bool FGControlsComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGControlsPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   
   //set the text
   char buffer[256];
   const char *lastCtrlsName;
   
   lastCtrlsName = Console->getVariable("pref::lastControls");
   sprintf(buffer, "%s.cfg", lastCtrlsName);
   
   WIN32_FIND_DATA data;
   HANDLE handle = FindFirstFile(buffer, &data);
   if (handle != INVALID_HANDLE_VALUE)
   {
      setText(lastCtrlsName);
      setVariable(data.cFileName);
   }
   else handle = FindFirstFile(gControlsExtention, &data);
   if (handle != INVALID_HANDLE_VALUE)
   {
      data.cFileName[strlen(data.cFileName) - 4] = '\0';
      setText(data.cFileName);
      setVariable(data.cFileName);
   }
   
   //set the title bmp
   mTitleBMP = SimResource::get(manager)->load("POP_Controls_DF.BMP");
   mTitleGhostBMP = SimResource::get(manager)->load("POP_Controls_BD.BMP");
   
   return true;
}  
 
void FGControlsComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      // set up the data for the pop up ctrl
      ControlsListCtrl *clCtrl = (ControlsListCtrl*)getArrayCtrl();
      AssertFatal(clCtrl, "pop up control is missing a text list");
      clCtrl->onWake();
      Point2I globalPos = localToGlobalCoord(Point2I(0,0));
      popUpCtrl->setPos(globalPos.x - 1, globalPos.y + extent.y - 2);
      popUpCtrl->setDim(extent.x + 2, 240, 240);
      root->pushDialogControl(popUpCtrl);
      root->tabNext();
      popUpActive = true;
      root->mouseLock(popUpCtrl);
   }
}   

void FGControlsComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   ControlsListCtrl *clCtrl = (ControlsListCtrl*)(popUpCtrl->getArrayCtrl());
   setText(clCtrl->getSelectedControls());
}

IMPLEMENT_PERSISTENT_TAG(FGControlsComboBox, FOURCC('F','G','q','c'));

//--------------------------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(ControlsListCtrl, FOURCC('F','G','q','l'));

bool ControlsListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   return true;
}

void ControlsListCtrl::onWake()
{
   int count = 0;
   WIN32_FIND_DATA data;
   HANDLE handle = FindFirstFile(gControlsExtention, &data);

   if (handle != INVALID_HANDLE_VALUE)
   {
      do
      {
         ControlsRep cfgFileRep;
         data.cFileName[strlen(data.cFileName) - 4] = '\0';
         strcpy(cfgFileRep.name, data.cFileName);
         cfgFiles.push_back(cfgFileRep);
         count++;
      }
      while (FindNextFile(handle,&data));
      FindClose(handle);
   }
   setSize(Point2I( 1, count));
}

void ControlsListCtrl::setSelectedControls(const char *ControlsName)
{
   int cellNum = 0;
   Vector<ControlsRep>::iterator i;
   for (i = cfgFiles.begin(); i != cfgFiles.end(); i++)
   {
      if (! stricmp(i->name, ControlsName))
      {
         setSelectedCell(Point2I(0, cellNum));
         break;
      }
      cellNum++;
   }
}

const char* ControlsListCtrl::getSelectedControls(void)
{
   Point2I selected = Parent::getSelectedCell();
   if (selected.y >= 0)
   {
      return &(cfgFiles[selected.y].name[0]);
   }
   else return NULL;
}

const char* ControlsListCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   if (cell.y < cfgFiles.size())
   {
      return &(cfgFiles[cell.y].name[0]);
   }
   else
   {
      return NULL;
   }
}

};