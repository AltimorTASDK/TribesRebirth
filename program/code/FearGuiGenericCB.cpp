//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "FearGuiGenericCB.h"
#include "simGuiScrollCtrl.h"
#include "fearGuiScrollCtrl.h"
#include "simGuiArrayCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"

namespace FearGui
{

void FGGenericPopUp::clear(void)
{
   FearGui::GenericListCtrl *myList = dynamic_cast<FearGui::GenericListCtrl*>(ac);
   if (myList) myList->clear();
}

Point2I FGGenericPopUp::findEntry(const char *entry)
{
   FearGui::GenericListCtrl *myList = dynamic_cast<FearGui::GenericListCtrl*>(ac);
   if (myList) return myList->findEntry(entry);
   else return (Point2I(-1, -1));
}

void FGGenericPopUp::addEntry(const Int32 id, const char *entry)
{
   FearGui::GenericListCtrl *myList = dynamic_cast<FearGui::GenericListCtrl*>(ac);
   if (myList) myList->addEntry(id, entry);
}

void FGGenericPopUp::setSelection(const char *entry)
{
   FearGui::GenericListCtrl *myList = dynamic_cast<FearGui::GenericListCtrl*>(ac);
   if (myList) myList->setSelection(entry);
}

const char*  FGGenericPopUp::getSelectedText(Point2I &cell)
{
   FearGui::GenericListCtrl *myList = dynamic_cast<FearGui::GenericListCtrl*>(ac);
   if (myList) return myList->getSelectedText(cell);
   else return NULL;
}
   
bool FGGenericPopUp::onAdd()
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
   
   ac = new GenericListCtrl();
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

void FGGenericComboBox::clear(void)
{
   FearGui::FGGenericPopUp *myPopUp = dynamic_cast<FearGui::FGGenericPopUp*>(popUpCtrl);
   if (myPopUp) myPopUp->clear();
}

Point2I FGGenericComboBox::findEntry(const char *entry)
{
   FearGui::FGGenericPopUp *myPopUp = dynamic_cast<FearGui::FGGenericPopUp*>(popUpCtrl);
   if (myPopUp) return myPopUp->findEntry(entry);
   else return (Point2I(-1, -1));
}

void FGGenericComboBox::addEntry(const Int32 id, const char *entry)
{
   FearGui::FGGenericPopUp *myPopUp = dynamic_cast<FearGui::FGGenericPopUp*>(popUpCtrl);
   if (myPopUp) myPopUp->addEntry(id, entry);
}

void FGGenericComboBox::setSelection(const char *entry)
{
   FearGui::FGGenericPopUp *myPopUp = dynamic_cast<FearGui::FGGenericPopUp*>(popUpCtrl);
   if (myPopUp) {
      myPopUp->setSelection(entry);
      updateFromArrayCtrl();
   }
}

const char* FGGenericComboBox::getSelectedText(Point2I &cell)
{
   FearGui::FGGenericPopUp *myPopUp = dynamic_cast<FearGui::FGGenericPopUp*>(popUpCtrl);
   if (myPopUp) return myPopUp->getSelectedText(cell);
   else return NULL;
}

bool FGGenericComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGGenericPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   return true;
}  
 
void FGGenericComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      // set up the data for the pop up ctrl
      GenericListCtrl *plCtrl = (GenericListCtrl*)getArrayCtrl();
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

void FGGenericComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   GenericListCtrl *glCtrl = (GenericListCtrl*)(popUpCtrl->getArrayCtrl());
   Point2I temp;
   const char *puText = glCtrl->getSelectedText(temp);
   setText(puText);
   setUpdate();
}

IMPLEMENT_PERSISTENT_TAG(FGGenericComboBox, FOURCC('F','G','g','c'));
 
IMPLEMENT_PERSISTENT_TAG(GenericListCtrl, FOURCC('F','G','g','k'));

void GenericListCtrl::clear(void)
{
   entries.clear();
   setSize(Point2I(1, 0));
}
   
Point2I GenericListCtrl::findEntry(const char *entry)
{
   Vector<Entry>::iterator i;
   int index = 0;
   for (i = entries.begin(); i != entries.end(); i++)
   {
      if (! (stricmp(i->name, entry))) break;
      index++;
   }
   if (i != entries.end()) return (Point2I(0, index));
   else return (Point2I(-1, -1));
}

void GenericListCtrl::addEntry(const Int32 id, const char *entry)
{
   //don't add the entry twice
   Point2I found = findEntry(entry);
   if (found.y >= 0) return;
   
   Entry info;
   strncpy(info.name, entry, MAX_ENTRY_LENGTH);
   info.id = id;
   info.name[MAX_ENTRY_LENGTH] = '\0';
   entries.push_back(info);
   setSize(Point2I(1, entries.size()));
}

void GenericListCtrl::setSelection(const char *entry)
{
   Point2I found = findEntry(entry);
   if (found.y >= 0) selectedCell.y = found.y;
}

bool GenericListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   
   entries.clear();
   setSize(Point2I(0, 1));
   
   return true;
}

void GenericListCtrl::onWake()
{
}

const char* GenericListCtrl::getSelectedText(Point2I &cell)
{
   cell = selectedCell;
   if (selectedCell.y >= 0)
   {
      return entries[selectedCell.y].name;
   }
   else return NULL;
}

const char* GenericListCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   return entries[cell.y].name;
}

};