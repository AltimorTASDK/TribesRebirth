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
#include "FearGuiStandardCombo.h"
#include "FearGuiScrollCtrl.h"
#include "simGuiArrayCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "soundFX.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "m_qsort.h"
#include "console.h"

namespace FearGui
{

static Sfx::Profile base2dSfxProfile;

bool FGStandardPopUp::onAdd()
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
   
   ac = new StandardListCtrl();
   AssertFatal(ac, "could not allocate memory");
   ac->extent.set(640, 200);
   manager->addObject(ac);
   sc->addObject(ac);
   ac->message = IDSTR_STRING_DEFAULT; // we need the text list to
                                       // give the pop up ctrl some
                                       // message indicating that
                                       // mouseUp or keyUp has occured
                                       
   //copy the console command from the CB to the list control
   const char *conCmd = NULL;
   if (parentCb) conCmd = parentCb->getConsoleCommand();
   if (conCmd && conCmd[0])
   {
      ac->setConsoleCommand(conCmd);
   }                                       
   //setBoarder();                                       
   return true;
}  

void FGStandardPopUp::setSort(bool value)
{
   if (ac)
   {
      StandardListCtrl *my_ac = dynamic_cast<StandardListCtrl*>(ac);
      if (my_ac) my_ac->setSort(value);
   }
}

//------------------------------------------------------------------------------
bool FGStandardComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGStandardPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   
   //ensure the entries are sorted
   setSort(TRUE);
   
   //set the bmps
   char buf[256];
   sprintf(buf, "%s_DF.BMP", mBitmapRootName);
   mTitleBMP = SimResource::get(manager)->load(buf);
   sprintf(buf, "%s_BD.BMP", mBitmapRootName);
   mTitleGhostBMP = SimResource::get(manager)->load(buf);
   
   return true;
}  

void FGStandardComboBox::setSort(bool value)
{
   if (popUpCtrl)
   {
      FGStandardPopUp *myPopUp = dynamic_cast<FGStandardPopUp*>(popUpCtrl);
      if (myPopUp) myPopUp->setSort(value);
   }
}

void FGStandardComboBox::onMouseDown(const SimGui::Event &event)
{
   if (active)
   {
      //find the soundfx manager
      Sfx::Manager *sfx = Sfx::Manager::find(manager);
      if (sfx) sfx->play(stringTable.insert("Button3.wav"), 0, &base2dSfxProfile);
   }
   
   Parent::onMouseDown(event);
}

void FGStandardComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      //find the soundfx manager
      Sfx::Manager *sfx = Sfx::Manager::find(manager);
      if (sfx) sfx->play(stringTable.insert("Button5.wav"), 0, &base2dSfxProfile);
      
      // set up the data for the pop up ctrl
      StandardListCtrl *plCtrl = (StandardListCtrl*)getArrayCtrl();
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

void FGStandardComboBox::closePopUpCtrl()
{
   //find the soundfx manager
   Sfx::Manager *sfx = Sfx::Manager::find(manager);
   if (sfx) sfx->play(stringTable.insert("Button4.wav"), 0, &base2dSfxProfile);
   
   Parent::closePopUpCtrl();
}

void FGStandardComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   StandardListCtrl *plCtrl = (StandardListCtrl*)(popUpCtrl->getArrayCtrl());
   const char *myText;
   myText = plCtrl->getSelectedText();
   if (myText && myText[0]) setText(myText);
   else setText("");
   setUpdate();
}

void FGStandardComboBox::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
}

void FGStandardComboBox::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
}

Persistent::Base::Error FGStandardComboBox::write( StreamIO &sio, int version, int user )
{
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FGStandardComboBox::read( StreamIO &sio, int version, int user)
{
   return Parent::read(sio, version, user);
}


IMPLEMENT_PERSISTENT_TAG(FGStandardComboBox, FOURCC('F','G','s','j'));

//------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(StandardListCtrl, FOURCC('F','G','s','y'));

static int __cdecl standardNameAscend(const void *a,const void *b)
{
   StandardListCtrl::EntryRep **entry_A = (StandardListCtrl::EntryRep **)(a);
   StandardListCtrl::EntryRep **entry_B = (StandardListCtrl::EntryRep **)(b);
   return (stricmp((*entry_A)->name, (*entry_B)->name));
} 

//console function members
void StandardListCtrl::addEntry(const char *buf, int id)
{
   EntryRep newEntry;
   newEntry.consoleIndex = id;
   strncpy(newEntry.name, buf, MaxEntryNameLength);
   newEntry.name[MaxEntryNameLength] = '\0';
   entries.push_back(newEntry);
   
   //resort
   entryPtrs.clear();
   for (int j = 0; j < entries.size(); j++)
   {
      entryPtrs.push_back(&entries[j]);
   }
   
   if (mbSortEntries)
   {
      m_qsort((void *)&entryPtrs[0], entryPtrs.size(),
                     sizeof(StandardListCtrl::EntryRep *),
                     standardNameAscend);
   }
   
   //set the size and selected cell
   setSize(Point2I(1, entryPtrs.size()));
   setSelected(id);
}

void StandardListCtrl::deleteEntry(int id)
{
   for (int i = 0; i < entryPtrs.size(); i++)
   {
      if (entries[i].consoleIndex == id)
      {
         entries.erase(i);
         break;
      }
   }
   
   //resort
   entryPtrs.clear();
   for (int j = 0; j < entries.size(); j++)
   {
      entryPtrs.push_back(&entries[j]);
   }
   m_qsort((void *)&entryPtrs[0], entryPtrs.size(),
                  sizeof(StandardListCtrl::EntryRep *),
                  standardNameAscend);
   
   //set the size and selected cell
   setSize(Point2I(1, entryPtrs.size()));
   selectedCell.set(-1, -1);
}

int StandardListCtrl::findEntry(const char *buf)
{
   for (int i = 0; i < entryPtrs.size(); i++)
   {
      if (! stricmp(buf, entryPtrs[i]->name))
      {
         return entryPtrs[i]->consoleIndex;
      }
   }
   return -1;
}

void StandardListCtrl::clear(void)
{
   entries.clear();
   entryPtrs.clear();
   setSize(Point2I(1, 0));
   selectedCell.set(-1, -1);
}

int StandardListCtrl::getSelectedEntry(void)
{
   if (selectedCell.y < 0) return -1;
   else return entryPtrs[selectedCell.y]->consoleIndex;
}

void StandardListCtrl::setSelectedEntry(int id)
{
   for (int i = 0; i < entryPtrs.size(); i++)
   {
      if (entryPtrs[i]->consoleIndex == id)
      {
         selectedCell.set(0, i);
         break;
      }
   }
}

bool StandardListCtrl::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   setTag(IDCTG_PLYR_CFG_LIST);
   
   entries.clear();
   entryPtrs.clear();
   
   setSize(Point2I(1, 0));
   
   return TRUE;
}
 
bool StandardListCtrl::cellSelected(Point2I cell)
{
   if (cell.y < 0 || cell.y >= entryPtrs.size()) return FALSE;
   Parent::cellSelected(cell);
   if (consoleCommand[0]) Console->evaluate(consoleCommand, false);
   return TRUE;
}

const char* StandardListCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   if (cell.y >= 0 && cell.y < entries.size())
   {
      return &(entryPtrs[cell.y]->name[0]);
   }
   else
   {
      return NULL;
   }
}

const char *StandardListCtrl::getSelectedText(void)
{
   if (selectedCell.y >= 0 && selectedCell.y < entryPtrs.size())
   {
      return entryPtrs[selectedCell.y]->name;
   }
   return NULL;
}

void StandardListCtrl::selectPrev(void)
{
   if (selectedCell.y >= 1)
   {
      if (selectedCell.y < entryPtrs.size())
      {
         cellSelected(Point2I(0, selectedCell.y - 1));
      }
      else
      {
         cellSelected(Point2I(0, 0));
      }
   }
   else
   {
      cellSelected(Point2I(0, entryPtrs.size() - 1));
   }
}

void StandardListCtrl::selectNext(void)
{
   if (selectedCell.y < entryPtrs.size() - 1)
   {
      if (selectedCell.y >= 0)
      {
         cellSelected(Point2I(0, selectedCell.y + 1));
      }
      else
      {
         cellSelected(Point2I(0, 0));
      }
   }
   else
   {
      cellSelected(Point2I(0, 0));
   }
}

 
};