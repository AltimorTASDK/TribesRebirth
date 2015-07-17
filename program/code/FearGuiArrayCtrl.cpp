#include "console.h"
#include "FearGuiArrayCtrl.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "soundFX.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "m_qsort.h"
#include "g_font.h"
#include "simResource.h"
#include "simGuiScrollCtrl.h"

namespace SimGui {
extern Control *findControl(const char *name);
};

namespace FearGui
{

static Sfx::Profile base2dSfxProfile;

void FGArrayCtrl::loadBitmapArray(void)
{
   const char *pbaName = SimTagDictionary::getString(manager, pbaTag);
   AssertFatal(pbaName, "Invalid PBA name");
   bma = SimResource::get(manager)->load(pbaName, true);
   AssertFatal((bool)bma, "Unable to load bma");
   
   GFXBitmap *bitmap;
   for (int i = 0; i < BmpCount; i++)
   {
      bitmap = bma->getBitmap(i);
      bitmap->attribute |= BMA_TRANSPARENT;
   }  
}

Int32 FGArrayCtrl::getMouseCursorTag(void)
{
   if (! root) return 0;
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
   if (columnsSortable || columnsResizeable)
   {
      //if the point is within the column header
      if (pt.y < headerDim.y)
      {
         //see if the point is on a column boundary
         int widthSoFar = 0;
         for (int i = 0; i < numColumns; i++)
         {
            if (columnInfo[i]->active)
            {
               widthSoFar += columnInfo[i]->width;
               if (columnsResizeable && (pt.x >= widthSoFar - 2) && (pt.x <= widthSoFar + 2))
               {
                  return IDBMP_CURSOR_HADJUST;
               }
               else if (columnsSortable && (pt.x < widthSoFar))
               {
                  return IDBMP_CURSOR_HAND;
               }
            }
         }
      }
   }
   return Parent::getMouseCursorTag();
}

void FGArrayCtrl::onMouseDown(const SimGui::Event &event)
{
   //find the soundfx manager
   Sfx::Manager *sfx = Sfx::Manager::find(manager);
   
   if(!active)
   {
      Parent::onMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   
   //make sure we don't accidently choose a column
   columnToResize = -1;
   
   //if the point is within the column header
   if (pt.y + position.y >= 0 && pt.y + position.y < headerDim.y)
   {
      stateDepressed = TRUE;
      if (columnsResizeable || columnsSortable)
      {
         //play the sound
         if (sfx) sfx->play(stringTable.insert("Button5.wav"), 0, &base2dSfxProfile);
         
         //see if the point is on a column boundary
         int widthSoFar = 0;
         for (int i = 0; i < numColumns; i++)
         {
            if (columnInfo[i]->active)
            {
               widthSoFar += columnInfo[i]->width;
               if (columnsResizeable && (pt.x >= widthSoFar - 2) && (pt.x <= widthSoFar + 2))
               {
                  columnToResize = i;
                  dragPoint = pt;
                  startPoint.x = widthSoFar - columnInfo[i]->width;
                  startExtent = extent;
                  break;
               }
               else if (columnsSortable && (pt.x < widthSoFar))
               {
                  if (columnToSort == i) sortAscending = !sortAscending;
                  else sortAscending = TRUE;
                  columnToSort = i;
                  
                  //cause the cells to be resorted in the PreRender() member
                  refresh = TRUE;
                  break;
               }
            }
         }
      }
   }
   else
   {
      //play the sound
      if (sfx) sfx->play(stringTable.insert("Button4.wav"), 0, &base2dSfxProfile);
      Parent::onMouseDown(event);
   }
}

void FGArrayCtrl::onRightMouseDown(const SimGui::Event &event)
{
   //find the soundfx manager
   Sfx::Manager *sfx = Sfx::Manager::find(manager);
   
   if(!active)
   {
      Parent::onRightMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   
   //make sure we don't accidently choose a column
   columnToResize = -1;
   
   //if the point is within the column header
   if (pt.y + position.y >= 0 && pt.y + position.y < headerDim.y)
   {
      stateDepressed = TRUE;
      if (columnsDualSortable)
      {
         //play the sound
         if (sfx) sfx->play(stringTable.insert("Button5.wav"), 0, &base2dSfxProfile);
         
         //see if the point is on a column boundary
         int widthSoFar = 0;
         for (int i = 0; i < numColumns; i++)
         {
            if (columnInfo[i]->active)
            {
               widthSoFar += columnInfo[i]->width;
               if (columnsResizeable && (pt.x >= widthSoFar - 2) && (pt.x <= widthSoFar + 2))
               {
                  break;
               }
               else if (pt.x < widthSoFar)
               {
                  if (i == columnToSort)
                     break;

                  if (columnToSortSecond == i)
                     sortAscendingSecond = !sortAscendingSecond;
                  else
                     sortAscendingSecond = TRUE;
                  columnToSortSecond = i;
                  
                  //cause the cells to be resorted in the PreRender() member
                  refresh = TRUE;
                  break;
               }
            }
         }
      }
   }
   else
   {
      //play the sound
      if (sfx) sfx->play(stringTable.insert("Button4.wav"), 0, &base2dSfxProfile);
      Parent::onRightMouseDown(event);
   }
}

void FGArrayCtrl::onMouseDragged(const SimGui::Event &event)
{
   if (columnToResize >= 0)
   {
      Point2I pt = globalToLocalCoord(event.ptMouse);
      if (pt.x + position.x > parent->extent.x)
      {
         long width = 0;
         for (int i = 0; i < numColumns; i++)
         {
            if (columnInfo[i]->active)
            {
               width += columnInfo[i]->width;
            }
         }
         cellSize.set(max(width, parent->extent.x), cellSize.y);
         resize(position, Point2I(cellSize.x, extent.y));
         columnToResize = -1;
      }
      else
      {
         if (pt.x > startPoint.x + 10)
         {
            int diff = pt.x - dragPoint.x;
            columnInfo[columnToResize]->width += diff;
            long width = 0;
            for (int i = 0; i < numColumns; i++)
            {
               if (columnInfo[i]->active)
               {
                  width += columnInfo[i]->width;
               }
            }
            cellSize.set(max(startExtent.x, max(width, parent->extent.x)), cellSize.y);
            resize(position, Point2I(cellSize.x, extent.y));
            dragPoint = pt;
         }
      }
      //update the whole control if a column is changing
      setUpdate();
   }
   Parent::onMouseDragged(event);
}

void FGArrayCtrl::onMouseUp(const SimGui::Event &event)
{
   if (columnToResize >= 0)
   {
      long width = 0;
      for (int i = 0; i < numColumns; i++)
      {
         if (columnInfo[i]->active)
         {
            width += columnInfo[i]->width;
         }
      }
      cellSize.set(max(width, parent->extent.x), cellSize.y);
      resize(position, Point2I(cellSize.x, extent.y));
      columnToResize = -1;
   }
   Parent::onMouseUp(event);
}

//console member functions
bool FGArrayCtrl::mbCconsoleFunctionsAdded = FALSE;

static const char *FGArrayAddEntry(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 4)
   {
      Console->printf("%s(control, string, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGArrayCtrl *ac = NULL;
   if (ctrl) ac = dynamic_cast<FGArrayCtrl *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   ac->addEntry(argv[2], atoi(argv[3]));
   return "TRUE";
}

static const char *FGArrayDeleteEntry(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGArrayCtrl *ac = NULL;
   if (ctrl) ac = dynamic_cast<FGArrayCtrl *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   ac->deleteEntry(atoi(argv[2]));
   return "TRUE";
}

static const char *FGArrayFindEntry(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control, string);", argv[0]);
      return "-1";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGArrayCtrl *ac = NULL;
   if (ctrl) ac = dynamic_cast<FGArrayCtrl *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "-1";
   }
   static char buf[20];
   sprintf(buf, "%d",  ac->findEntry(argv[2]));
   return buf;
}

static const char *FGArrayClear(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGArrayCtrl *ac = NULL;
   if (ctrl) ac = dynamic_cast<FGArrayCtrl *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   ac->clear();
   return "true";
}

static const char *FGArraySetSelected(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGArrayCtrl *ac = NULL;
   if (ctrl) ac = dynamic_cast<FGArrayCtrl *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   ac->setSelectedEntry(atoi(argv[2]));
   return "true";
}

static const char *FGArrayGetSelected(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "0";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGArrayCtrl *ac = NULL;
   if (ctrl) ac = dynamic_cast<FGArrayCtrl *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "0";
   }
   static char buf[20];
   sprintf(buf, "%d", ac->getSelectedEntry());
   return buf;
}

static const char *FGArrayGetSelectedText(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGArrayCtrl *ac = NULL;
   if (ctrl) ac = dynamic_cast<FGArrayCtrl *>(ctrl);

   if (! ac)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "";
   }
   return ac->getSelectedText();
}

void FGArrayCtrl::addEntry(const char *buf, int id)
{
   buf;
   id;
}

void FGArrayCtrl::deleteEntry(int id)
{
   id;
}


int FGArrayCtrl::findEntry(const char *buf)
{
   buf;
   return -1;
}

void FGArrayCtrl::clear(void)
{
}

int FGArrayCtrl::getSelectedEntry(void)
{
   return -1;
}

void FGArrayCtrl::setSelectedEntry(int id)
{
   id;
}

const char *FGArrayCtrl::getSelectedText(void)
{
   return NULL;
}

bool FGArrayCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (! mbCconsoleFunctionsAdded)
   {
      mbCconsoleFunctionsAdded = TRUE;
      Console->addCommand(0, "FGArray::addEntry", FGArrayAddEntry);
      Console->addCommand(0, "FGArray::deleteEntry", FGArrayDeleteEntry);
      Console->addCommand(0, "FGArray::findEntry", FGArrayFindEntry);
      Console->addCommand(0, "FGArray::clear", FGArrayClear);
      Console->addCommand(0, "FGArray::setSelected", FGArraySetSelected);
      Console->addCommand(0, "FGArray::getSelected", FGArrayGetSelected);
      Console->addCommand(0, "FGArray::getSelectedText", FGArrayGetSelectedText);
   }
   
   //initialize the 2d sound profile
   base2dSfxProfile.flags = 0;
   base2dSfxProfile.baseVolume = 0;
   base2dSfxProfile.coneInsideAngle = 90;
   base2dSfxProfile.coneOutsideAngle= 180; 
   base2dSfxProfile.coneVector.set(0.0f, -1.0f, 0.0f);
   base2dSfxProfile.coneOutsideVolume = 0.0f;  
   base2dSfxProfile.minDistance = 30.0f;     
   base2dSfxProfile.maxDistance = 300.0f;     
   
   //load the fonts
   headerFont = SimResource::loadByTag(manager, IDFNT_8_HILITE, true);
   headerFontHL = SimResource::loadByTag(manager, IDFNT_8_HILITE, true);
   headerFontNA = SimResource::loadByTag(manager, IDFNT_8_DISABLED, true);
   headerFontShadow = SimResource::loadByTag(manager, IDFNT_8_BLACK, true);
   
   hFont = SimResource::get(manager)->load("sf_orange_7.pft");
   AssertFatal(hFont.operator bool(), "Unable to load hFont.");
   hFontHL = SimResource::get(manager)->load("sf_white_7.pft");
   AssertFatal(hFontHL.operator bool(), "Unable to load hFontHL.");
   hFontNA = SimResource::get(manager)->load("sf_grey_7.pft");
   AssertFatal(hFontNA.operator bool(), "Unable to load hFontNA.");
   hFontMO = SimResource::get(manager)->load("sf_yellow_7.pft");
   AssertFatal(hFontMO.operator bool(), "Unable to load hFontMO.");
   
   //set the pba tag
   pbaTag = IDPBA_SHELL_COLUMNS;
   loadBitmapArray();
   
   //other misc vars
   refresh = TRUE;
   columnsSortable = TRUE;
   columnsDualSortable = FALSE;
   columnToSort = -1;
   columnToSortSecond = -1;
   columnsResizeable = TRUE;
   columnToResize = -1;
   numColumns = 0;
   columnInfo = NULL;
   
   //set the cell dimensions
   cellSize.set(20, hFont->getHeight() + 6);
   
   //set the parent matrixcontrol colors
   SimGui::ScrollContentCtrl *pCtrl = dynamic_cast<SimGui::ScrollContentCtrl*>(parent);
   if (pCtrl)
   {
      pCtrl->setParentBoarder(TRUE, HILITE_COLOR, HILITE_COLOR, BOX_GHOST_INSIDE);
   }
   
   return true;
}

void FGArrayCtrl::onWake()
{
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   setSize(Point2I( 1, 0));
}

void FGArrayCtrl::drawInfoText(GFXSurface *sfc, GFXFont *font, const char* text, Point2I &offset, Point2I &dest, bool shadow, bool centered)
{
   //if the string fits
   if (font->getStrWidth(text) < dest.x)
   {
      if (shadow && bool(headerFontShadow))
      {
         sfc->drawText_p(headerFontShadow, &Point2I(centered ? offset.x - 1 + (dest.x - font->getStrWidth(text)) / 2 : offset.x - 1,
                      offset.y + (dest.y - font->getHeight()) / 2), text);
      }
      sfc->drawText_p(font, &Point2I(centered ? offset.x + (dest.x - font->getStrWidth(text)) / 2 : offset.x,
                      offset.y - 1 + (dest.y - font->getHeight()) / 2), text);
   }
   
   //else create a string that will 
   else
   {
      char buf[260], *temp;
      char etcBuf[4] = "...";
      int etcWidth = font->getStrWidth(etcBuf);
      
      //make sure we can at least hold the etc
      if (dest.x < etcWidth) return;
      
      //copy the string into a temp buffer
      strncpy(buf, text, 255);
      buf[255] = '\0';
      int stringlen = strlen(buf);
      temp = &buf[stringlen];
      
      //search for how many chars can be displayed
      while (stringlen && (font->getStrWidth(buf) > dest.x - etcWidth))
      {
         stringlen--;
         temp--;
         *temp = '\0';
      }
      
      //now copy the etc onto the end of the string, and draw the text
      strcpy(temp, etcBuf);
      if (shadow && bool(headerFontShadow))
      {
         sfc->drawText_p(headerFontShadow, &Point2I(offset.x - 1, offset.y + 1 + (dest.y - font->getHeight()) / 2), buf);
      }
      sfc->drawText_p(font, &Point2I(offset.x, offset.y + (dest.y - font->getHeight()) / 2), buf);
   }
}

void FGArrayCtrl::onPreRender()
{
   if (refresh) refresh = FALSE;
}

void FGArrayCtrl::onRenderColumnHeaders(GFXSurface *sfc, Point2I offset, Point2I parentOffset, Point2I headerDim)
{
   bool ghosted = FALSE;
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(1);
      if ((! active) ||
          (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
      {
         ghosted = TRUE;
      }
   }

/*
   GFXBitmap *left = bma->getBitmap(!ghosted ? BmpLeft : BmpLeftNA);
   GFXBitmap *mid = bma->getBitmap(!ghosted ? BmpMid : BmpMidNA);
   GFXBitmap *right = bma->getBitmap(!ghosted ? BmpRight : BmpRightNA);
*/
   GFXBitmap *divide = bma->getBitmap(!ghosted ? BmpDivide : BmpDivideNA);
   GFXBitmap *sort = NULL;

   int sortWidth = 0;
   
   if (columnsSortable)
   {
      if (sortAscending) sort = bma->getBitmap(!ghosted ? BmpSortUp : BmpSortUpNA);
      else sort = bma->getBitmap(!ghosted ? BmpSortDown : BmpSortDownNA);
      
      sortWidth = sort->getWidth() + divide->getWidth() + 2;
   }
   
   Point2I bmpOffset;
   bmpOffset.x = parentOffset.x + headerDim.x;
   bmpOffset.y = parentOffset.y;
   sfc->drawRect2d_f(&RectI(parentOffset.x, parentOffset.y, parentOffset.x + parent->extent.x - 1,
                           parentOffset.y + headerDim.y - 1), (ghosted ? BLACK : STANDARD_COLOR));
   
/*
   //draw the middle header piece
   int hStretch = parent->extent.x - headerDim.x - left->getWidth() - right->getWidth();
   bmpOffset.x = parentOffset.x + headerDim.x + left->getWidth();
   bmpOffset.y = parentOffset.y;
   sfc->drawBitmap2d(mid, &bmpOffset, &Point2I(hStretch, mid->getHeight()));
*/
   
   //draw the header divides
   bmpOffset.x = offset.x + headerDim.x;
   
   for (int i = 0; i < numColumns; i++)
   {
      if (columnInfo[i]->active)
      {
         bmpOffset.x += columnInfo[i]->width;
         
         //draw the divider
         sfc->drawLine2d(&bmpOffset, &Point2I(bmpOffset.x, bmpOffset.y + headerDim.y), (ghosted ? BOX_GHOST_LAST_PIX : HILITE_COLOR));
         //sfc->drawBitmap2d(divide, &Point2I(bmpOffset.x - divide->getWidth(), bmpOffset.y));
      }
   }
   
/*
   //draw the end caps
   bmpOffset.x = parentOffset.x + headerDim.x;
   sfc->drawBitmap2d(left, &bmpOffset);
   bmpOffset.x += left->getWidth() + hStretch;
   sfc->drawBitmap2d(right, &bmpOffset);
*/   
   //draw the header text
   bmpOffset.x = offset.x + headerDim.x + 1;
   
   for (int j = 0; j < numColumns; j++)
   {
      if (columnInfo[j]->active)
      {
         //draw the text
         if (columnInfo[j]->headerTag >= 0)
         {
            const char *header = SimTagDictionary::getString(manager, columnInfo[j]->headerTag);
            int headerTextWidth = columnInfo[j]->width - 2 - (j == columnToSort ? sortWidth : 2);
            
            drawInfoText(sfc, (ghosted ? headerFontNA : (j == columnToSort ? headerFontHL : headerFont)), header,
                                             Point2I(bmpOffset.x + 2, bmpOffset.y),
                                             Point2I(headerTextWidth, headerDim.y));
                                             
            if ((j == columnToSort) && (columnInfo[j]->width > sortWidth))
            {
               //draw the sort icon
               sfc->drawBitmap2d(sort, &Point2I(bmpOffset.x + columnInfo[j]->width - sortWidth,
                                          bmpOffset.y + headerDim.y / 2));
            }
         }
         
         bmpOffset.x += columnInfo[j]->width;
      }
   }
   
   //LAST_PIX the header
   sfc->drawRect2d(&RectI(parentOffset.x, parentOffset.y, parentOffset.x + parent->extent.x - 1, parentOffset.y + headerDim.y - 1), (ghosted ? BOX_GHOST_LAST_PIX : HILITE_COLOR));
}

void FGArrayCtrl::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool sel, bool mouseOver)
{
   //get the parent coords
   Point2I parentOffset = parent->localToGlobalCoord(Point2I(0, 0));
   
   bool ghosted = FALSE;
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(1);
      if ((! active) || 
          (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
      {
         ghosted = TRUE;
      }
   }
   
   //get the bitmaps
/*   
   GFXBitmap *horz, *vert;
   if (sel) horz = bma->getBitmap(!ghosted ? BmpHorzLineOn : BmpHorzLineOnNA);
   else horz = bma->getBitmap(!ghosted ? BmpHorzLine : BmpHorzLineNA);
   if (sel) vert = bma->getBitmap(!ghosted ? BmpVertLineOn : BmpVertLineOnNA);
   else vert = bma->getBitmap(!ghosted ? BmpVertLine : BmpVertLineNA);
*/
   
   //initialize the font
   GFXFont *font;
   if (ghosted || (! cellSelectable(cell))) font = hFontNA;
   else if (sel) font = hFontHL;
   else if (mouseOver) font = hFontMO;
   else font = hFont;
   
   //initialize the draw offset
   Point2I drawOffset = offset;
   
   //first draw a left boarder
   //sfc->drawBitmap2d(vert, &Point2I(parentOffset.x, drawOffset.y),
   //                     &Point2I(1, cellSize.y));
   
   
   //if selected, the entire row should be hilited
/*   
   if (sel)
   {
      sfc->drawRect2d_f(&RectI(drawOffset.x, drawOffset.y,
                        drawOffset.x + cellSize.x, drawOffset.y + cellSize.y),
                        (!ghosted ? SELECT_COLOR : MEDIUM_GREY));
   }
*/
   
   //draw the cels first
   for (int i = 0; i < numColumns; i++)
   {
      if (columnInfo[i]->active)
      {
         //sfc->drawBitmap2d(vert, &Point2I(drawOffset.x, drawOffset.y),
         //                  &Point2I(1, cellSize.y));
         //sfc->drawBitmap2d(horz, &drawOffset, &Point2I(columnInfo[i]->width - 1, horz->getHeight()));
         sfc->drawLine2d(&drawOffset, &Point2I(drawOffset.x, drawOffset.y + cellSize.y - 1), (ghosted ? BOX_GHOST_LAST_PIX : HILITE_COLOR));
         drawOffset.x += columnInfo[i]->width;
      }
   }
   //draw the closing line
   sfc->drawLine2d(&drawOffset, &Point2I(drawOffset.x, drawOffset.y + cellSize.y - 1), (ghosted ? BOX_GHOST_LAST_PIX : HILITE_COLOR));
   
   //make sure the cell extends the full width
   int widthLeft = extent.x - (drawOffset.x - offset.x);
   if (widthLeft > 1)
   {
      //sfc->drawBitmap2d(vert, &Point2I(drawOffset.x, drawOffset.y),
      //                  &Point2I(1, cellSize.y));
      //sfc->drawBitmap2d(horz, &drawOffset, &Point2I(widthLeft - 1, horz->getHeight()));
   }
   
   //now draw the individual elements
   drawOffset = offset;
   
   for (int j = 0; j < numColumns; j++)
   {
      if (columnInfo[j]->active)
      {
         char *entryText = getCellText(sfc, Point2I(columnInfo[j]->origPosition, cell.y),
                                          drawOffset, Point2I(columnInfo[j]->width, cellSize.y)); 
         if (entryText)
         {
            drawInfoText(sfc, font, entryText, Point2I(drawOffset.x + 2, drawOffset.y - 2),
                                               Point2I(columnInfo[j]->width - 4, cellSize.y), FALSE);
         }
         drawOffset.x += columnInfo[j]->width;
      }
   }
   
   //frame the cell
   sfc->drawRect2d(&RectI(parentOffset.x, offset.y, parentOffset.x + parent->extent.x - 1, offset.y + cellSize.y), (ghosted ? BOX_GHOST_LAST_PIX : HILITE_COLOR));
}

void FGArrayCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   Parent::onRender(sfc, offset, updateRect);
   
   bool ghosted = FALSE;
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(1);
      if ((! active) ||
          (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
      {
         ghosted = TRUE;
      }
   }
   
   //keep the parent matrix control up to date
   SimGui::ScrollContentCtrl *pCtrl = dynamic_cast<SimGui::ScrollContentCtrl*>(parent);
   if (pCtrl)
   {
      pCtrl->setParentDisabled(ghosted);
   }
   
   //now draw in the final closing line
   if (size.y > 0)
   {
      //GFXBitmap *horz = bma->getBitmap(!ghosted ? BmpHorzLine : BmpHorzLineNA);
      //sfc->drawBitmap2d(horz, &Point2I(offset.x, offset.y + extent.y - 1),
      //                  &Point2I(extent.x - 1, horz->getHeight()));
      sfc->drawLine2d(&Point2I(offset.x, offset.y + extent.y - 1),
                        &Point2I(offset.x + extent.x - 1, offset.y + extent.y - 1),
                        (ghosted ? BOX_GHOST_LAST_PIX : HILITE_COLOR));
   }
}

};
