
#include "windows.h"
#include "winbase.h"
#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "FearCSDelegate.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "fear.strings.h"
#include "FearGuiColumns.h"
#include "FearGuiShellPal.h"
#include "m_qsort.h"
#include "console.h"
#include "g_barray.h"
#include "g_font.h"
#include "FearGuiRecList.h"

namespace FearGui
{

const char gPlaybackExtention[] = "recordings\\*.rec";
const char gPlaybackDir[] = "recordings\\";

static char gPlaybackString[256];

enum {
   
   RCName = 0,
   RCCount
};

static FGArrayCtrl::ColumnInfo gColumnInfo[RCCount] =
{
   { IDSTR_RECORDINGS,     10, 300,   0,   0,    TRUE, 200, NULL, NULL },
};
static FGArrayCtrl::ColumnInfo *gInfoPtrs[RCCount];                                         


IMPLEMENT_PERSISTENT_TAG(FGRecordingListCtrl, FOURCC('F','G','r','c'));

#define MaxRecNameLength 256   
struct PlaybackRep
{
   char name[MaxRecNameLength + 1];
	bool alreadyPlayed;
};
static Vector<PlaybackRep> recordings;
static Vector<PlaybackRep*> recordingPtrs;

static int __cdecl recNameAscend(const void *a,const void *b)
{
   PlaybackRep **entry_A = (PlaybackRep **)(a);
   PlaybackRep **entry_B = (PlaybackRep **)(b);
   return (stricmp((*entry_A)->name, (*entry_B)->name));
} 

bool FGRecordingListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //other misc vars
	columnsResizeable = FALSE;
	columnsSortable = FALSE;
	numColumns = RCCount;
	columnInfo = gInfoPtrs;
   
   //set the ptrs table
   for (int j = 0; j < RCCount; j++)
   {
      gInfoPtrs[j] = &gColumnInfo[j];
   }
   
   //set the cell dimensions
   int width = 0;
   int i;
   for (i = 0; i < RCCount; i++)
   {
      width += gInfoPtrs[i]->width;
   }
   cellSize.set(width, hFont->getHeight() + 6);
   
   //seed the random number generator
   rand.setSeed((unsigned int)(manager->getCurrentTime()));
   
   //if we've already created the list, return true
   if (recordings.size() > 0) return TRUE;
   
   WIN32_FIND_DATA data;
   HANDLE handle = FindFirstFile(gPlaybackExtention, &data);

   if (handle != INVALID_HANDLE_VALUE)
   {
      do
      {
         PlaybackRep recording;
         data.cFileName[strlen(data.cFileName) - 4] = '\0';
         strcpy(recording.name, data.cFileName);
         recording.alreadyPlayed = FALSE;
         recordings.push_back(recording);
      }
      while (FindNextFile(handle,&data));
      FindClose(handle);
   }
   
   for (i = 0; i < recordings.size(); i++)
   {
      recordingPtrs.push_back(&recordings[i]);
   }
   
   //sort the list;
   m_qsort((void *)&recordingPtrs[0], recordingPtrs.size(),
                  sizeof(PlaybackRep *),
                  recNameAscend);
   
   return true;
}


void FGRecordingListCtrl::onWake()
{
   //seed the random number generator
   rand.setSeed((unsigned int)(manager->getCurrentTime()));
   
   setSize(Point2I( 1, recordings.size()));
}

char* FGRecordingListCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   if (cell.y >= recordingPtrs.size() || cell.y < 0) return NULL;
   return recordingPtrs[cell.y]->name;
}

void FGRecordingListCtrl::renameSelected(const char *newName)
{
   if (newName && newName[0])
   {
      if (selectedCell.y >= 0 && selectedCell.y < recordingPtrs.size())
      {
         char oldFileName[256];
         char newFileName[256];
         sprintf(oldFileName, "%s%s.rec", gPlaybackDir, &recordingPtrs[selectedCell.y]->name[0]);
         sprintf(newFileName, "%s%s.rec", gPlaybackDir, newName);
         
         //first copy the old to the new
         if (CopyFile(oldFileName, newFileName, FALSE))
         {
            //then delete the old
            DeleteFile(oldFileName);
            
            //then replace the entry in the list
            strcpy(recordingPtrs[selectedCell.y]->name, newName);
         }
      }
   }
}

void FGRecordingListCtrl::removeSelected(void)
{
}

void FGRecordingListCtrl::setSelectedRecording(const char *recordingName)
{
   int cellNum = 0;
   Vector<PlaybackRep*>::iterator i;
   for (i = recordingPtrs.begin(); i != recordingPtrs.end(); i++)
   {
      if (! stricmp((*i)->name, recordingName))
      {
         setSelectedCell(Point2I(0, cellNum));
         break;
      }
      cellNum++;
   }
}

const char* FGRecordingListCtrl::getSelectedRecording(void)
{
   Point2I selected = Parent::getSelectedCell();
   if (selected.y >= 0)
   {
      recordingPtrs[selected.y]->alreadyPlayed = TRUE;
      sprintf(gPlaybackString, "%s%s.rec", gPlaybackDir, &recordingPtrs[selected.y]->name[0]);
      return (&gPlaybackString[0]);
   }
   else return NULL;
}

const char* FGRecordingListCtrl::getSelectedRecordingName(void)
{
   Point2I selected = Parent::getSelectedCell();
   if (selected.y >= 0)
   {
      return &recordingPtrs[selected.y]->name[0];
   }
   else return NULL;
}

void FGRecordingListCtrl::selectRandomRecording(void)
{
   //if there are no recordings, zip...
   if (recordingPtrs.size() == 0) return;
   
   Vector<int> randomList;
   randomList.clear();
   int count = 0;
   int counter = 0;
   
   //create the list of unplayed recordings
   Vector<PlaybackRep*>::iterator i;
   for (i = recordingPtrs.begin(); i != recordingPtrs.end(); i++)
   {
      if (! (*i)->alreadyPlayed)
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
      for (i = recordingPtrs.begin(); i != recordingPtrs.end(); i++)
      {
         (*i)->alreadyPlayed = FALSE;
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

void FGRecordingListCtrl::selectNextRecording(void)
{
   //if there are no recordings, zip...
   if (recordingPtrs.size() == 0) return;
   
   //find the next unplayed recording
   bool found = FALSE;
   int index = 0;
   Vector<PlaybackRep*>::iterator i;
   for (i = recordingPtrs.begin(); i != recordingPtrs.end(); i++)
   {
      if (! (*i)->alreadyPlayed)
      {
         found = TRUE;
         break;
      }
      index++;
   }
   
   //if we haven't got any unplayed, reset the list and pick one
   if(! found)
   {
      for (i = recordingPtrs.begin(); i != recordingPtrs.end(); i++)
      {
         (*i)->alreadyPlayed = FALSE;
      }
      
      index = 0;
   }
   
   //return the name of the recording
   selectedCell.set(0, index);
}

};
