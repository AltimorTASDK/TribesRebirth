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

namespace FearGui
{

enum {
   
   CIName = 0,
   CIDesc,
   CIActive,
   CICount
};

static FGArrayCtrl::ColumnInfo gColumnInfo[CICount] =
{
   { IDSTR_CI_COLUMN_NAME,     10, 300,   0,   0,    TRUE,  70, NULL, NULL },
   { IDSTR_CI_DESC,            10, 300,   1,   1,    TRUE, 161, NULL, NULL },
   { IDSTR_CI_ACTIVE,          10, 300,   2,   2,    TRUE,  36, NULL, NULL },
};
static FGArrayCtrl::ColumnInfo *gInfoPtrs[CICount];                                         

//there is a mirror of these vars in FearGuiServerList.cpp
static char *columnDisplayVariables = "pref::ServerColumnDisplay";
static const int gNumColumns = 12;
struct ColumnDisplayInfo
{
   Int32 nameTag;
   Int32 descTag;
   int   position;
   bool  active;
   int   width;
};
static ColumnDisplayInfo gColumns[gNumColumns] =
{
   { IDSTR_SI_FAVORITES,      IDSTR_CI_FAVORITES,     0,    TRUE,     52},
   { IDSTR_SI_CONNECTION,     IDSTR_CI_CONNECTION,    1,    TRUE,     52},
   { IDSTR_SI_STATUS,         IDSTR_CI_STATUS,		   2,    TRUE,     54},
   { IDSTR_SI_NAME,           IDSTR_CI_NAME,			   3,    TRUE,    105},
   { IDSTR_SI_PING,				IDSTR_CI_PING,			   4,    TRUE,     43},
   { IDSTR_SI_MTYPE,	         IDSTR_CI_MTYPE,		   5,    TRUE,    100},
   { IDSTR_SI_MISSION,			IDSTR_CI_MISSION,		   6,    TRUE,     90},
   { IDSTR_054,               IDSTR_CI_PLAYERSAVAIL,  7,    TRUE,     59},
   { IDSTR_SI_CPU,				IDSTR_CI_CPU,			   8,    TRUE,     33},
   { IDSTR_SI_ADDRESS,			IDSTR_CI_ADDRESS,		   9,    TRUE,    102},
   { IDSTR_SRVR_INFO_VERSION,	IDSTR_CI_VERSION,		   10,   TRUE,    100},
   { IDSTR_SI_MOD,	         IDSTR_CI_MOD,   		   11,   TRUE,    100},
};

IMPLEMENT_PERSISTENT_TAG(ColumnListCtrl, FOURCC('F','G','c','l'));

Int32 ColumnListCtrl::getMouseCursorTag(void)
{
   Point2I cursorPos = root->getCursorPos();
   
   Point2I pt = globalToLocalCoord(cursorPos);
   
   //if the point is within the column header
   if (pt.y < headerDim.y)
   {
      return (cellToMove >= 0 ? IDBMP_CURSOR_GRAB : 0);
   }
   else
   {
      if (cellToMove >= 0) return IDBMP_CURSOR_GRAB;
      else
      {
         //see if the cursor is in the "display?" column
         int width = 0;
         
         //see if it's in the name column
         for (int i = 0; i < CIName; i++) width += gColumnInfo[i].width;
         if (pt.x >= width && pt.x <= width + gColumnInfo[CIName].width)
         {
            return IDBMP_CURSOR_OPENHAND;
         }
         
         //else see if it's in the active column
         for (int j = CIName; j < CIActive; j++) width += gColumnInfo[j].width;
         if (pt.x >= width && pt.x <= width + gColumnInfo[CIActive].width)
         {
            return IDBMP_CURSOR_HAND;
         }
      }
   }
   return Parent::getMouseCursorTag();
}

void ColumnListCtrl::onMouseDown(const SimGui::Event &event)
{
   if(!active)
   {
      Parent::onMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   
   //make sure we don't accidently choose a column
   cellToMove = -1;
   cellToToggle = -1;
   
   //if the point is not within the column header
   if (pt.y >= headerDim.y)
   {
      Parent::onMouseDown(event);
      if (selectedCell.y >= 0)
      {
         int width = 0;
         
         //see if it's in the name column
         for (int i = 0; i < CIName; i++) width += gColumnInfo[i].width;
         if (pt.x >= width && pt.x <= width + gColumnInfo[CIName].width)
         {
            for (int j = 0; j < gNumColumns; j++)
            {
               if (gColumns[j].position == selectedCell.y)
               {
                  cellToMove = j;
                  cellStartPosition = selectedCell.y;
                  break;
               }
            }
            return;
         }
         
         //else see if it's in the active column
         for (int k = CIName; k < CIActive; k++) width += gColumnInfo[k].width;
         if (pt.x >= width && pt.x <= width + gColumnInfo[CIActive].width)
         {
            for (int i = 0; i < gNumColumns; i++)
            {
               if (gColumns[i].position == selectedCell.y)
               {
                  cellToToggle = i;
                  gColumns[cellToToggle].active = !gColumns[cellToToggle].active;
                  break;
               }
            }
         }
      }
   }
}

void ColumnListCtrl::onMouseDragged(const SimGui::Event &event)
{
   Point2I pt = globalToLocalCoord(event.ptMouse);
   int whichPosition;
   
   if ((cellToMove >= 0) && (cellSize.y > 0))
   {
      if ((pt.y < 0) || (pt.y > 2 * headerDim.y + cellSize.y * size.y))
      {
         whichPosition = cellStartPosition;
      }
      else
      {
         //which slot did the mouse drag into
         whichPosition = (pt.y - headerDim.y) / cellSize.y;
         whichPosition = min(gNumColumns - 1, max(0, whichPosition));
      }
         
      //see if we need to switch with any
      int difference = whichPosition - gColumns[cellToMove].position;
      int direction = (difference < 0 ? -1 : (difference > 0 ? 1 : 0));
      
      if (difference != 0)
      {
         //store the order temporarily
         ColumnDisplayInfo *tempTable[gNumColumns];
         for (int k = 0; k < gNumColumns; k++)
         {
            tempTable[gColumns[k].position] = &gColumns[k];
         }
         
         //cycle through, and increment all required intermediate positions
         for (int i = whichPosition; i != gColumns[cellToMove].position; i -= direction)
         {
            tempTable[i]->position -= direction;
         }
         
         //now update the cellToMove
         gColumns[cellToMove].position = whichPosition;
         
         //update the whole control if a cell is changing positions
         setUpdate();
      }
   }
   Parent::onMouseDragged(event);
}

void ColumnListCtrl::onMouseUp(const SimGui::Event &event)
{
   //any change will end in a mouse up, therefore...
   writeDisplayTable();
   
   cellToMove = -1;
   cellToToggle = -1;
   Parent::onMouseUp(event);
}

void ColumnListCtrl::readDisplayTable(void)
{
   int i;
   for (i = 0; i < gNumColumns; i++)
   {
      char varBuf[80];
      sprintf(varBuf, "%s%d", columnDisplayVariables, i); 
      const char *var = Console->getVariable(varBuf);
      if (var && var[0] != '\0')
      {
         char buf[64], *temp, *temp2;
         strcpy(buf, var);
         temp = &buf[0];
         
         //set the position;
         temp2 = strchr(temp, ',');
         if (temp2)
         {
            *temp2 = '\0';
            gColumns[i].position = atoi(buf);
            temp = temp2 + 1;
         }
         
         //set the active flag
         temp2 = strchr(temp, ',');
         if (temp2)
         {
            *temp2 = '\0';
            if (! stricmp(temp, " ON")) gColumns[i].active = TRUE;
            else gColumns[i].active = FALSE;
            temp = temp2 + 1;
         }
         
         //set the width
         gColumns[i].width = atoi(temp);
      }
   }
   
   //validate the vars
   bool valid = TRUE;
   for (i = 0; i < gNumColumns; i++)
   {
      bool found = FALSE;
      for (int j = 0; j < gNumColumns; j++)
      {
         if (gColumns[j].position == i)
         {
            found = TRUE;
            break;
         }
      }
      if (! found)
      {
         valid = FALSE;
         break;
      }
   }
   if (! valid)
   {
      Console->printf("Column list invalid - resetting");
      for (int i = 0; i < gNumColumns; i++)
      {
         gColumns[i].position = i;
      }
      writeDisplayTable();
   }
}

void ColumnListCtrl::writeDisplayTable(void)
{
   for (int i = 0; i < gNumColumns; i++)
   {
      char varBuf[80];
      sprintf(varBuf, "%s%d", columnDisplayVariables, i); 
      char buf[64];
      sprintf(buf, "%d, %s, %d", gColumns[i].position,
                                 (gColumns[i].active ? "ON" : "OFF"),
                                 gColumns[i].width); 
      Console->setVariable(varBuf, buf);
   }
}

bool ColumnListCtrl::cellSelected(Point2I cell)
{
   //used to unselect all rows, but don't want the Parent::() to resize and change offsets
   if (gNumColumns <= cell.y)
      return false;
      
   return Parent::cellSelected(cell);
}

bool ColumnListCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //other misc vars
	columnsResizeable = FALSE;
	columnsSortable = FALSE;
	numColumns = CICount;
	columnInfo = gInfoPtrs;
   cellToMove = -1;
	cellToToggle = -1;
   
   //set the ptrs table
   for (int j = 0; j < CICount; j++)
   {
      gInfoPtrs[j] = &gColumnInfo[j];
   }
   
   //set the cell dimensions
   int width = 0;
   for (int i = 0; i < CICount; i++)
   {
      width += gInfoPtrs[i]->width;
   }
   cellSize.set(width, hFont->getHeight() + 6);
   
   //read in the widths from the serverDisplay variables
   readDisplayTable();
   
   return true;
}

void ColumnListCtrl::onWake()
{
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   setSize(Point2I( 1, gNumColumns));
}

void ColumnListCtrl::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool sel, bool mouseOver)
{
   //compiler warning
   sel;
   
   //get the parent coords
   Point2I parentOffset = parent->localToGlobalCoord(Point2I(0, 0));
   
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   //get the mouse position
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
/*   
   //get the bitmaps
   GFXBitmap *horz, *vert, *horzOn, *vertOn;
   horz = bma->getBitmap(BmpHorzLine);
   horzOn = bma->getBitmap(BmpHorzLineOn);
   vert = bma->getBitmap(BmpVertLine);
   vertOn = bma->getBitmap(BmpVertLineOn);
*/   
   //initialize the font
   GFXFont *font;
   if (ghosted) font = hFontNA;
   else if (cellToMove >= 0 && gColumns[cellToMove].position == cell.y) font = hFontHL;
   else if (mouseOver && cellToMove < 0) font = hFontMO;
   else font = hFont;
   
   //determine if the mouse is depressing the "Active" column
   bool activeDepressed = FALSE;
   int activeWidth = 0;
   int i;
   for (i = 0; i < CIActive; i++) activeWidth += gColumnInfo[i].width;
   
   if ((cellToToggle >= 0) && (gColumns[cellToToggle].position == cell.y) && stateDepressed &&
         (pt.x >= activeWidth) && (pt.x <= activeWidth + gColumnInfo[CIActive].width))
   {
      activeDepressed = TRUE;
   }
   
   //initialize the draw offset
   Point2I drawOffset = offset;

/* redraw   
   //first draw a left boarder
   sfc->drawBitmap2d(vert, &Point2I(parentOffset.x, drawOffset.y),
                        &Point2I(vert->getWidth(), cellSize.y));
   
   //if this is the cell we are moving, the entire cell should be hilited
   if (cellToMove >= 0 && gColumns[cellToMove].position == cell.y)
   {
      sfc->drawRect2d_f(&RectI(drawOffset.x, drawOffset.y,
                        drawOffset.x + cellSize.x, drawOffset.y + cellSize.y), SELECT_COLOR);
   }
   else if (activeDepressed)
   {
      sfc->drawRect2d_f(&RectI(offset.x + activeWidth, drawOffset.y,
                        offset.x + activeWidth + gColumnInfo[CIActive].width, drawOffset.y + cellSize.y), SELECT_COLOR);
   }
*/
   
/* replace
   //draw the cels first
   for (int j = 0; j < CICount; j++)
   {
      GFXBitmap *tempVert, *tempHorz;
      if ((j == CIActive) && activeDepressed)
      {
         tempVert = vertOn;
         tempHorz = horzOn;
      }
      else
      {
         tempVert = vert;
         tempHorz = horz;
      }
      sfc->drawBitmap2d(tempVert, &Point2I(drawOffset.x + 1 - tempVert->getWidth(), drawOffset.y),
                        &Point2I(tempVert->getWidth(), cellSize.y));
      sfc->drawBitmap2d(tempHorz, &drawOffset, &Point2I(gColumnInfo[j].width - 1, tempHorz->getHeight()));
      drawOffset.x += gColumnInfo[j].width;
   }
   
   //make sure the cell extends the full width
   int widthLeft = extent.x - (drawOffset.x - offset.x);
   if (widthLeft > 1)
   {
      sfc->drawBitmap2d(vert, &Point2I(drawOffset.x + 1 - vert->getWidth(), drawOffset.y),
                        &Point2I(vert->getWidth(), cellSize.y));
      sfc->drawBitmap2d(horz, &drawOffset, &Point2I(widthLeft - 1, horz->getHeight()));
   } 
*/
   //draw the cels first
   for (i = 0; i < numColumns; i++)
   {
      if (columnInfo[i]->active)
      {
         //sfc->drawBitmap2d(vert, &Point2I(drawOffset.x, drawOffset.y),
         //                  &Point2I(1, cellSize.y));
         //sfc->drawBitmap2d(horz, &drawOffset, &Point2I(columnInfo[i]->width - 1, horz->getHeight()));
         sfc->drawLine2d(&drawOffset, &Point2I(drawOffset.x, drawOffset.y + cellSize.y - 1), SELECT_COLOR);
         drawOffset.x += columnInfo[i]->width;
      }
   }
   
   //draw the closing line
   sfc->drawLine2d(&drawOffset, &Point2I(drawOffset.x, drawOffset.y + cellSize.y - 1), SELECT_COLOR);
   
   //make sure the cell extends the full width
   int widthLeft = extent.x - (drawOffset.x - offset.x);
   if (widthLeft > 1)
   {
      //sfc->drawBitmap2d(vert, &Point2I(drawOffset.x, drawOffset.y),
      //                  &Point2I(1, cellSize.y));
      //sfc->drawBitmap2d(horz, &drawOffset, &Point2I(widthLeft - 1, horz->getHeight()));
   }
   
   //find the cell with the correct position
   ColumnDisplayInfo *cellToRender = NULL;
   for (int k = 0; k < gNumColumns; k++)
   {
      if (gColumns[k].position == cell.y)
      {
         cellToRender = &gColumns[k];
         break;
      }
   } 
   AssertFatal(cellToRender, "Out of range finding the cellToRender.");
   
   //now draw the individual elements
   drawOffset = offset;
   
   //COLUMN NAME
   const char *name = SimTagDictionary::getString(manager, cellToRender->nameTag);
   drawInfoText(sfc, font, name, Point2I(drawOffset.x + 2, drawOffset.y - 2),
                                       Point2I(gColumnInfo[CIName].width - 4, cellSize.y), FALSE);
   drawOffset.x += gColumnInfo[CIName].width;
   
   //COLUMN DESCRIPTION
   const char *desc = SimTagDictionary::getString(manager, cellToRender->descTag);
   drawInfoText(sfc, font, desc, Point2I(drawOffset.x + 2, drawOffset.y - 2),
                                       Point2I(gColumnInfo[CIDesc].width - 4, cellSize.y), FALSE);
   drawOffset.x += gColumnInfo[CIDesc].width;
   
   //COLUMN ACTIVE?
   drawInfoText(sfc, (activeDepressed ? (GFXFont*)hFontHL : font),
                                       (cellToRender->active ? "YES" : "NO"), Point2I(drawOffset.x + 2, drawOffset.y - 2),
                                       Point2I(gColumnInfo[CIActive].width - 4, cellSize.y), FALSE);
   drawOffset.x += gColumnInfo[CIActive].width;

/*   
   //The last cell needs a closing
   drawOffset.x = offset.x + min(extent.x, parent->extent.x);
   sfc->drawBitmap2d(vert, &Point2I(drawOffset.x - vert->getWidth(), drawOffset.y),
                     &Point2I(vert->getWidth(), cellSize.y));
*/                     
   //frame the cell
   sfc->drawRect2d(&RectI(parentOffset.x, offset.y, parentOffset.x + parent->extent.x - 1, offset.y + cellSize.y), SELECT_COLOR);
}

};
