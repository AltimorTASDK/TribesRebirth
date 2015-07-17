#include "simGuiArrayCtrl.h"
#include "g_surfac.h"
#include "m_random.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "FearGuiRecordings.h"
#include "fearGuiScrollCtrl.h"

// Currently defined the same as in dlgMissionSelect
const char gPlaybackExtention[] = "recordings\\*.rec";
const char gPlaybackDir[] = "recordings\\";

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG(FGRecordingList, FOURCC('F','G','r','l'));

static Vector<FGRecordingList::PlaybackRep> recordings;

bool FGRecordingList::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   //seed the random number generator
   rand.setSeed((unsigned int)(manager->getCurrentTime()));
   
   //if we've already created the list, return true
   if (recordings.size() > 0) return TRUE;
   
   int count = 0;
   WIN32_FIND_DATA data;
   HANDLE handle = FindFirstFile(gPlaybackExtention, &data);

   if (handle != INVALID_HANDLE_VALUE)
   {
      do
      {
         FGRecordingList::PlaybackRep recording;
         data.cFileName[strlen(data.cFileName) - 4] = '\0';
         strcpy(recording.name, data.cFileName);
         recording.alreadyPlayed = FALSE;
         recordings.push_back(recording);
         count++;
      }
      while (FindNextFile(handle,&data));
      FindClose(handle);
   }
   setSize(Point2I( 1, count));
   
   return true;
}

void FGRecordingList::onWake()
{
   //seed the random number generator
   rand.setSeed((unsigned int)(manager->getCurrentTime()));
}

void FGRecordingList::setSelectedRecording(const char *recordingName)
{
   int cellNum = 0;
   Vector<PlaybackRep>::iterator i;
   for (i = recordings.begin(); i != recordings.end(); i++)
   {
      if (! stricmp(i->name, recordingName))
      {
         setSelectedCell(Point2I(0, cellNum));
         break;
      }
      cellNum++;
   }
}

const char* FGRecordingList::getSelectedRecording(void)
{
   char buffer[256];

   Point2I selected = Parent::getSelectedCell();
   if (selected.y >= 0)
   {
      recordings[selected.y].alreadyPlayed = TRUE;
      sprintf(buffer, "%s%s.rec", gPlaybackDir, &recordings[selected.y].name[0]);
      return (&buffer[0]);
   }
   else return NULL;
}

const char* FGRecordingList::getSelectedRecordingName(void)
{
   Point2I selected = Parent::getSelectedCell();
   if (selected.y >= 0)
   {
      return &recordings[selected.y].name[0];
   }
   else return NULL;
}

void FGRecordingList::selectRandomRecording(void)
{
   //if there are no recordings, zip...
   if (recordings.size() == 0) return;
   
   Vector<int> randomList;
   randomList.clear();
   int count = 0;
   int counter = 0;
   
   //create the list of unplayed recordings
   Vector<PlaybackRep>::iterator i;
   for (i = recordings.begin(); i != recordings.end(); i++)
   {
      if (! i->alreadyPlayed)
      {
         randomList.push_back(counter);
         count++;
      }
      counter++;
   }
   
   int index;
   
   //if we haven't got any unplayed, reset the list and pick one
   if(count == 0)
   {
      for (i = recordings.begin(); i != recordings.end(); i++)
      {
         i->alreadyPlayed = FALSE;
      }
      
      index = rand.getInt() % recordings.size();
   }
   
   //else choose a random one
   else
   {
      index = randomList[rand.getInt() % count];
   }
   
   //return the name of the recording
   selectedCell.set(0, index);
}
   
const char* FGRecordingList::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   if (cell.y < recordings.size())
   {
      return &(recordings[cell.y].name[0]);
   }
   else
   {
      return NULL;
   }
}

 
bool FGRecordingPopUp::onAdd()
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
   
   ac = new FGRecordingList();
   AssertFatal(ac, "could not allocate memory");
   ac->extent.set(218, 96);
   manager->addObject(ac);
   sc->addObject(ac);
   ac->message = IDSTR_STRING_DEFAULT; // we need the text list to
                                       // give the pop up ctrl some
                                       // message indicating that
                                       // mouseUp or keyUp has occured
   return true;
}  

const char* FGRecordingPopUp::getSelectedRecording(void)
{
   const char *recording = NULL;
   FGRecordingList *list = (FGRecordingList*)ac;
   if (list) recording = list->getSelectedRecording();
   return recording;
}

void FGRecordingPopUp::selectRandomRecording(void)
{
   FGRecordingList *list = (FGRecordingList*)ac;
   if (list) list->selectRandomRecording();
}

//------------------------------------------------------------------------------
bool FGRecordingComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new FGRecordingPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   
   //set the text
   setText("");
   
   return true;
}  
 
void FGRecordingComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      // set up the data for the pop up ctrl
      FGRecordingList *rlCtrl = (FGRecordingList*)getArrayCtrl();
      AssertFatal(rlCtrl, "pop up control is missing a text list");
      rlCtrl->onWake();
      Point2I globalPos = localToGlobalCoord(Point2I(0,0));
      popUpCtrl->setPos(globalPos.x - 1, globalPos.y + extent.y - 1);
      popUpCtrl->setDim(extent.x + 2, 96, 96);
      root->pushDialogControl(popUpCtrl);
      root->tabNext();
      popUpActive = true;
      root->mouseLock(popUpCtrl);
   }
}   


const char* FGRecordingComboBox::getSelectedRecording(void)
{
   const char *recording = NULL;
   FGRecordingPopUp *popUp = (FGRecordingPopUp *)(popUpCtrl);
   if (popUp) recording = popUp->getSelectedRecording();
   return recording;
}

void FGRecordingComboBox::selectRandomRecording(void)
{
   FGRecordingPopUp *popUp = (FGRecordingPopUp *)(popUpCtrl);
   if (popUp) popUp->selectRandomRecording();
   updateFromArrayCtrl();
}

void FGRecordingComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   FGRecordingList *rlCtrl = (FGRecordingList*)(popUpCtrl->getArrayCtrl());
   setText(rlCtrl->getSelectedRecordingName());
}

IMPLEMENT_PERSISTENT_TAG(FGRecordingComboBox, FOURCC('F','G','r','c'));
 
};
