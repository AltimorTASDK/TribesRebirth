#include "simGuiArrayCtrl.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "FearGuiMissionList.h"
#include "FearGuiMissionCombo.h"

// Currently defined the same as in dlgMissionSelect
const char gMissionExtention[] = "missions\\*.mis";

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG(MissionListCtrl, FOURCC('F','G','m','l'));

bool MissionListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   return true;
}

void MissionListCtrl::onWake()
{
   int count = 0;
   WIN32_FIND_DATA data;
   HANDLE handle = FindFirstFile(gMissionExtention, &data);

   if (handle != INVALID_HANDLE_VALUE)
   {
      do
      {
         MissionRep mission;
         data.cFileName[strlen(data.cFileName) - 4] = '\0';
         strcpy(mission.name, data.cFileName);
         missions.push_back(mission);
         count++;
      }
      while (FindNextFile(handle,&data));
      FindClose(handle);
   }
   setSize(Point2I( 1, count));
}

void MissionListCtrl::setSelectedMission(const char *missionName)
{
   int cellNum = 0;
   Vector<MissionRep>::iterator i;
   for (i = missions.begin(); i != missions.end(); i++)
   {
      if (! stricmp(i->name, missionName))
      {
         setSelectedCell(Point2I(0, cellNum));
         break;
      }
      cellNum++;
   }
}

const char* MissionListCtrl::getSelectedMission(void)
{
   Point2I selected = Parent::getSelectedCell();
   if (selected.y >= 0)
   {
      return &(missions[selected.y].name[0]);
   }
   else return NULL;
}

const char* MissionListCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   if (cell.y < missions.size())
   {
      return &(missions[cell.y].name[0]);
   }
   else
   {
      return NULL;
   }
}

};
