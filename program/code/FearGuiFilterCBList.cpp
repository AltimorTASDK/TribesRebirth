#include "simGuiArrayCtrl.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "FearGuiFilterCBList.h"
#include "console.h"
#include "m_qsort.h"

namespace FearGui
{

static int __cdecl filterNameAscend(const void *a,const void *b)
{
   FGFilterCBListCtrl::FilterRep **entry_A = (FGFilterCBListCtrl::FilterRep **)(a);
   FGFilterCBListCtrl::FilterRep **entry_B = (FGFilterCBListCtrl::FilterRep **)(b);
   return (stricmp((*entry_A)->name, (*entry_B)->name));
} 

IMPLEMENT_PERSISTENT_TAG(FGFilterCBListCtrl, FOURCC('F','G','f','b'));

bool FGFilterCBListCtrl::onAdd()
{
   if (! Parent::onAdd()) return FALSE;
   onWake();
   return TRUE;
}

void FGFilterCBListCtrl::onWake()
{
   filters.clear();
   filterPtrs.clear();
   
   //find the next spot for Filter
   char varName[32];
   int i = 1;
   while (1)
   {
      sprintf(varName, "pref::Filter%d", i);
      const char *filter = Console->getVariable(varName);

      //if we found a filter, add it into the list
      if(filter && (filter[0] != 0))
      {
         FilterRep info;
         info.number = i;
         strncpy(info.name, filter, 255);
         info.name[255] = '\0';
         char *temp = strchr(info.name, ':');
         if (! temp)
         {
            i++;
            continue;
         }
         else
         {
            *temp = '\0';
            filters.push_back(info);
         }
      }
      else break;
         
      i++;
   }
   
   //now that they're all added, build the table of pointers
   for (i = 0; i < filters.size(); i++)
   {
      filterPtrs.push_back(&filters[i]);
   }
   
   //now sort the filters by name
   if (filterPtrs.size() > 0)
   {
      m_qsort((void *)&filterPtrs[0], filterPtrs.size(),
                     sizeof(FGFilterCBListCtrl::FilterRep *), filterNameAscend);
   }
  
   //now push an <empty> onto the front
   FilterRep info = {0, "<None>"};
   filters.push_back(info);
   filterPtrs.push_front(&filters[filters.size() - 1]);
   
   //set the var
   setSize(Point2I(1, filters.size()));
   
   //find the prev selected
   selectedCell.set(0, 0);
   int num = atoi(Console->getVariable("pref::UseFilter"));
   for (i = 0; i < filterPtrs.size(); i++)
   {
      if (filterPtrs[i]->number == num)
      {
         selectedCell.set(0, i);
         break;
      }
   }
}

const char* FGFilterCBListCtrl::getCurFilter(int &number)
{
   if (selectedCell.y >= 0)
   {
      number = filterPtrs[selectedCell.y]->number;
      return filterPtrs[selectedCell.y]->name;
   }
   else
   {
      number = -1;
      return NULL;
   }
}

const char* FGFilterCBListCtrl::getFilterName(int number)
{
   for (int i = 0; i < filters.size(); i++)
   {
      if (filters[i].number == number) return filters[i].name;
   }
   
   //nothing was found, return NULL
   return NULL;
}

int FGFilterCBListCtrl::addFilter(const char *filter)
{
   //find the next spot for Filter
   char varName[32];
   int i = 1;
   while (1)
   {
      sprintf(varName, "pref::Filter%d", i);
      const char *filter = Console->getVariable(varName);

      if((! filter) || (filter[0] == 0))
         break;
         
      i++;
   }
   if (filter && filter[0])
   {
      char buf[256];
      sprintf(buf, "%s:", filter);
      Console->setVariable(varName, buf);
   }
   
   //now push the new filter 
   FilterRep newFilter;
   newFilter.number = i;
   strncpy(newFilter.name, filter, 255);
   newFilter.name[255] = '\0';
   filters.push_back(newFilter);
   filterPtrs.push_back(&filters[filters.size() - 1]);
   
   //set the vars
   setSize(Point2I(1, filters.size()));
   selectedCell.set(0, filters.size() - 1);
   
   return i;
}

void FGFilterCBListCtrl::removeFilter(int filterNum)
{
   if (filterNum < 0) return;
   char varToClear[32];
   char lastVar[32];
   sprintf(varToClear, "pref::Filter%d", filterNum);
   
   int i = 1;
   while (1)
   {
      sprintf(lastVar, "pref::Filter%d", i);
      const char *filter = Console->getVariable(lastVar);

      //find the last stored filter
      if (!filter || (filter[0] == 0)) break;
         
      i++;
   }
   i--;
   
   sprintf(lastVar, "pref::Filter%d", i);
   if (filterNum < i)
   {
      Console->setVariable(varToClear, Console->getVariable(lastVar));
   }
   Console->setVariable(lastVar, "");
   
   //recreate the list
   onWake();
}

const char* FGFilterCBListCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   if (cell.y < filterPtrs.size())
   {
      return filterPtrs[cell.y]->name;
   }
   else
   {
      return NULL;
   }
}

};
