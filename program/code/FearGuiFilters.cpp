#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "FearCSDelegate.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "fear.strings.h"
#include "FearGuiFilters.h"
#include "FearGuiShellPal.h"
#include "m_qsort.h"
#include "console.h"
#include "simResource.h"
#include "g_font.h"

namespace FearGui
{

static int __cdecl filterNameAscend(const void *a,const void *b)
{
   FilterListCtrl::FilterListInfo **entry_A = (FilterListCtrl::FilterListInfo **)(a);
   FilterListCtrl::FilterListInfo **entry_B = (FilterListCtrl::FilterListInfo **)(b);
   return (stricmp((*entry_A)->name, (*entry_B)->name));
} 

static int __cdecl filterNameDescend(const void *a,const void *b)
{
   FilterListCtrl::FilterListInfo **entry_A = (FilterListCtrl::FilterListInfo **)(a);
   FilterListCtrl::FilterListInfo **entry_B = (FilterListCtrl::FilterListInfo **)(b);
   return (stricmp((*entry_B)->name, (*entry_A)->name));
} 

enum {
   
   FLName = 0,
   FLCount
};

static FGArrayCtrl::ColumnInfo gFilterListInfo[FLCount] =
{
   { IDSTR_FT_FILTER,            10, 300,   0,   0,    TRUE, 200, filterNameAscend, filterNameDescend },
};
static FGArrayCtrl::ColumnInfo *gFInfoPtrs[FLCount];

static const char *sortAscendVariable = "pref::FLSortAscend";
bool FilterListCtrl::prefSortAscending = TRUE;

IMPLEMENT_PERSISTENT_TAG(FilterListCtrl, FOURCC('F','G','f','l'));

int FilterListCtrl::getCurFilter(void)
{
   if (selectedCell.y >= 0)
   {
      int index = 1;
      Vector<FilterListInfo>::iterator i;
      for (i = entries.begin(); i != entries.end(); i++)
      {
         if (i == entryPtrs[selectedCell.y])
         {
            return index;
         }
         index++;
      }
   }
   
   return -1;
}

int FilterListCtrl::addFilter(const char *filter)
{
   //find the next spot for Filter
   char varName[32];
   int i = 1;
   for(;;)
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
   
   //make sure it gets added to the list
   refresh = TRUE;
   setCurFilter = i;
   selectedCell.set(0, 0); //this is going to be overwritten in preRen, but
                           //the delegate won't 'unghost' buttons w/o this
   
   return i;
}

void FilterListCtrl::removeFilter(void)
{
   if (selectedCell.y < 0) return;
   char varToClear[32];
   char lastVar[32];
   int filterNum = getCurFilter();
   sprintf(varToClear, "pref::Filter%d", filterNum) ;
   sprintf(lastVar, "pref::Filter%d", entries.size()) ;
   
   if (filterNum < entries.size())
   {
      Console->setVariable(varToClear, Console->getVariable(lastVar));
   }
   Console->setVariable(lastVar, "");
   refresh = TRUE;
   setCurFilter = -1;
   selectedCell.set(-1, -1);
}

bool FilterListCtrl::cellSelected(Point2I cell)
{
   //used to unselect all rows, but don't want the Parent::() to resize and change offsets
   if((entries.size() <= cell.y))
      return false;
      
   return Parent::cellSelected(cell);
}

bool FilterListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //other misc vars
	columnsResizeable = FALSE;
	columnsSortable = FALSE;
   refresh = TRUE;
	numColumns = FLCount;
	columnInfo = gFInfoPtrs;
   setCurFilter = -1;
   
   //set the ptrs table
   for (int i = 0; i < FLCount; i++)
   {
      gFInfoPtrs[i] = &gFilterListInfo[i];
   }
   
   //pref vars
	CMDConsole::getLocked()->addVariable(0, sortAscendVariable, CMDConsole::Bool, &sortAscending);
   
   //set the cell dimensions
   int width = 0;
   for (int j = 0; j < FLCount; j++)
   {
      width += gFilterListInfo[j].width;
   }
   cellSize.set(width, hFont->getHeight() + 6);
   
   return true;
}

void FilterListCtrl::onWake()
{
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   setSize(Point2I( 1, 0));
   refresh = TRUE;
   setCurFilter = -1;
}

void FilterListCtrl::onPreRender()
{
   if (refresh)
   {
      //build up the list of FilterListes
      entries.clear();
      entryPtrs.clear();
      
      //find the next spot for filter
      char varName[32];
      int i = 1;
      for(;;)
      {
         sprintf(varName, "pref::Filter%d", i);
         const char *filter = Console->getVariable(varName);

         if((! filter) || (filter[0] == 0)) break;
            
         //add the Filter to the vector
         FilterListInfo info;
         
         strncpy(info.name, filter, Inspect::MAX_STRING_LEN - 1);
         info.name[Inspect::MAX_STRING_LEN - 1] = '\0';
         
         //now make sure we only copy up to the ':'
         char *temp = strchr(info.name, ':');
         if (! temp) break;
         *temp = '\0';
         
         entries.push_back(info);
            
         i++;
      }
      setSize(Point2I( 1, entries.size()));
      
      //setup the pointers array
      for (int j = 0; j < entries.size(); j++)
      {
         entryPtrs.push_back(&entries[j]);
      }
      
      //refresh
      if (sortAscending && gFilterListInfo[0].sortAscend)
      {
         m_qsort((void *)&entryPtrs[0], entryPtrs.size(),
                        sizeof(FilterListCtrl::FilterListInfo *),
                        gFilterListInfo[0].sortAscend);
      }
      
      //make sure the cell selected matches the current filter
      if (setCurFilter >= 1)
      {
         int index = 1;
         Vector<FilterListInfo*>::iterator i;
         for (i = entryPtrs.begin(); i != entryPtrs.end(); i++)
         {
            if ((*i) == &entries[setCurFilter - 1])
            {
               cellSelected(Point2I(0, index - 1));
               break;
            }
            index++;
         }
      }
   
      refresh = FALSE;
   }
}

char* FilterListCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   char buffer[256];
   switch (cell.x)
   {
      case FLName:
         //NAME
         return entryPtrs[cell.y]->name;
   }
   return NULL;
}

// ----------------------------------------------------------------------------

enum {
   
   FTVariable = 0,
   FTCondition,
   FTValue,
   FTFiller,
   FTCount
};

static FGArrayCtrl::ColumnInfo gFilterInfo[FTCount] =
{
   { IDSTR_FT_VAR,             10, 300,   0,   0,    TRUE, 70, NULL, NULL },
   { IDSTR_FT_COND,            10, 300,   1,   1,    TRUE, 87, NULL, NULL },
   { IDSTR_FT_VALUE,           10, 300,   2,   2,    TRUE, 67, NULL, NULL },
   { -1,                       10, 300,   3,   3,    TRUE,  32, NULL, NULL },
};
static FGArrayCtrl::ColumnInfo *gInfoPtrs[FTCount];

const char *gFilterVariable = "pref::Filter";

const int gNumFilterVars = 8;
FilterCtrl::FilterVars gFilterVars[gNumFilterVars] =
{
   { IDSTR_FT_VAR_PING,		      FilterCtrl::FT_COND_NUMERICAL, },
   { IDSTR_FT_VAR_PLAYERS,       FilterCtrl::FT_COND_NUMERICAL, },
   { IDSTR_FT_VAR_MISSION,       FilterCtrl::FT_COND_TEXT,      },
   { IDSTR_FT_VAR_SERVER,	      FilterCtrl::FT_COND_TEXT,      },
   { IDSTR_FT_VAR_MISSION_TYPE,  FilterCtrl::FT_COND_TEXT,      },
   { IDSTR_FT_VAR_SERVER_TYPE,   FilterCtrl::FT_COND_TEXT,      },
   { IDSTR_FT_VAR_IS_FAVORITE,	FilterCtrl::FT_COND_BOOL,      },
   { IDSTR_FT_VAR_IS_DEDICATED,	FilterCtrl::FT_COND_BOOL,      },
};

const int gNumFilterNumberConds = 4;
FilterCtrl::FilterVars gFilterNumberConds[gNumFilterNumberConds] =
{
   { IDSTR_FT_COND_LT,    -1, },
   { IDSTR_FT_COND_GT,    -1, },
   { IDSTR_FT_COND_EQ,    -1, },
   { IDSTR_FT_COND_NE,    -1, },
};

const int gNumFilterTextConds = 2;
FilterCtrl::FilterVars gFilterTextConds[gNumFilterTextConds] =
{
   { IDSTR_FT_COND_HAS,        -1, },
   { IDSTR_FT_COND_HAS_NOT,    -1, },
};

const int gNumFilterBoolConds = 2;
FilterCtrl::FilterVars gFilterBoolConds[gNumFilterBoolConds] =
{
   { IDSTR_FT_COND_TRUE,        -1, },
   { IDSTR_FT_COND_FALSE,       -1, },
};

IMPLEMENT_PERSISTENT_TAG(FilterCtrl, FOURCC('F','G','f','t'));

void FilterCtrl::setCurEntry(int entryNum)
{
   if (mCurEntry != entryNum)
   {
      mCurEntry = entryNum;
      readEntryVar(mCurEntry);
   }
}

void FilterCtrl::readEntryVar(int entryNum)
{
   //clear the array
   entries.clear();
   
   if (entryNum < 0) return;
   
   //get the variable
   char buf[256];
   sprintf(buf, "%s%d", gFilterVariable, max(0, entryNum));
   const char *var = Console->getVariable(buf);
   
   //if we found a variable
   if (var)
   {
      strncpy(buf, var, 255);
      buf[255] = '\0';
      
      //scan past the filter name
      char *temp = strchr(buf, ':');
      if (! temp) return;
      
      //store the entry name (save space for a ':')
      *temp = '\0';
      strncpy(mCurEntryName, buf, 254);
      mCurEntryName[254] = '\0';
      temp++;
      
      //loop through and read in the conditions
      char *end;
      while (temp)
      {
         FilterInfo info;
         
         //find the variable
         end = strchr(temp, ',');
         if (! end) break;
         *end = '\0';
         info.variable = atoi(temp);
         temp = end + 1;
         
         //find the condition
         end = strchr(temp, ',');
         if (! end) break;
         *end = '\0';
         info.condition = atoi(temp);
         temp = end + 1;
         
         //find the value
         end = strchr(temp, ':');
         if (end) *end = '\0';
         if (*temp == '\0') break;
         strncpy(info.value, temp, 255);
         info.value[255] = '\0';
         if (end) temp = end + 1;
         else temp = NULL;
         
         //push the filter condition
         entries.push_back(info);
      }
   } 
}

bool FilterCtrl::writeEntryVar(int entryNum)
{
   if (entryNum < 0) return TRUE;
   
   //get the variable name
   char varName[64];
   sprintf(varName, "%s%d", gFilterVariable, entryNum);
   
   //fill in the buffer
   char buf[256];
   char *temp = &buf[0];
   int lengthLeft = 255;
   
   //write out the entry name
   sprintf(temp, "%s:", mCurEntryName);
   int length = strlen(mCurEntryName);
   lengthLeft -= length;
   temp += length;
   
   Vector<FilterInfo>::iterator i;
   for (i = entries.begin(); i != entries.end(); i++)
   {
      //make sure the condition won't overflow the Console variable
      int length;
      char temp2[256];
      sprintf(temp2, ":%d,%d,",(*i).variable, (*i).condition);
      length = strlen(temp2) + strlen((*i).value);
      //return FALSE;
      AssertFatal(length < lengthLeft, "Too many conditions");
      
      //add the condition to the end of the console var
      sprintf(temp, "%s%s", temp2, (*i).value);
      lengthLeft -= length;
      temp += length;
   }
   
   //now write out the console var
   Console->setVariable(varName, buf);
   
   //success
   return TRUE;
}
   
void FilterCtrl::addCondition(int var, int cond, const char *value)
{
   if (var >= 0)
   {
      FilterInfo info;
      info.variable = var;
      info.condition = cond;
      strncpy(info.value, value, 255);
      info.value[255] = '\0';
      
      //push the filter condition
      entries.push_back(info);
   }
   
   bool success = writeEntryVar(mCurEntry);
   if (! success)
   {
      //remove the last entry added
      entries.pop_back();
   }
}

void FilterCtrl::removeSelected(void)
{
   if (entries.size() <= 0) return;
   
   if (selectedCell.y >= 0)
   {
      entries.erase(selectedCell.y);
   }
   
   //write the entry var
   writeEntryVar(mCurEntry);
   
   if (! cellSelected(Point2I(0, 0)))
   {
      selectedCell.set(-1, -1);
   }
}

bool FilterCtrl::cellSelected(Point2I cell)
{
   //used to unselect all rows, but don't want the Parent::() to resize and change offsets
   if((entries.size() <= cell.y))
      return false;
      
   return Parent::cellSelected(cell);
}

bool FilterCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //set the misc vars
	columnsResizeable = FALSE;
	columnsSortable = FALSE;
	numColumns = FTCount;
	columnInfo = gInfoPtrs;
   mCurEntry = -1;
   
   //set the ptrs table
   for (int i = 0; i < FTCount; i++)
   {
      gInfoPtrs[i] = &gFilterInfo[i];
   }
      
   //set the cell dimensions
   int width = 0;
   for (int j = 0; j < FTCount; j++)
   {
      width += gFilterInfo[j].width;
   }
   cellSize.set(width, hFont->getHeight() + 6);
   
   return true;
}

void FilterCtrl::onWake()
{
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   setSize(Point2I( 1, 0));
   setCurEntry(0);
}

void FilterCtrl::onPreRender()
{
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   setSize(Point2I( 1, entries.size()));
}

char* FilterCtrl::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   int varIndex = entries[cell.y].variable;
   const char *cond;
   switch (cell.x)
   {
      case FTVariable:
         //NAME
         return (char*)SimTagDictionary::getString(manager, gFilterVars[varIndex].tag);
         
         
      case FTCondition:
         //CONDITION
         if (gFilterVars[varIndex].type == FT_COND_NUMERICAL)
         {
            cond = SimTagDictionary::getString(manager, gFilterNumberConds[entries[cell.y].condition].tag);
         }
         else if (gFilterVars[varIndex].type == FT_COND_TEXT)
         {
            cond = SimTagDictionary::getString(manager, gFilterTextConds[entries[cell.y].condition].tag);
         }
         else
         {
            cond = SimTagDictionary::getString(manager, gFilterBoolConds[entries[cell.y].condition].tag);
         }
         return (char*)cond;
         
      case FTValue:
         //VALUE
         if (gFilterVars[varIndex].type != FT_COND_BOOL)
         {
            return entries[cell.y].value;
         }
         else
         {
            return "";
         }
         
      case FTFiller:
         //FILLER
         return (cell.y < entries.size() - 1 ? "AND" : NULL);
   }
   return NULL;
}

};
