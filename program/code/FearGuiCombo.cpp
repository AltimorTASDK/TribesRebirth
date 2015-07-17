//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "console.h"
#include "FearGuiCombo.h"
#include "simGuiScrollCtrl.h"
#include "fearGuiScrollCtrl.h"
#include "simGuiArrayCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "simResource.h"

namespace SimGui {
extern Control *findControl(const char *name);
};

namespace FearGui
{

static const char *gBMPTags[FGComboBox::BMP_Count] =
{
	"_DF.BMP",	//default
	"_HI.BMP",	//hilite
	"_MD.BMP",	//mouse-over default
	"_MH.BMP",	//mouse-over hilite
	"_ON.BMP",	//depressed
	"_NA.BMP",	//not available (ghosted)
};

bool FGComboPopUp::onAdd()
{
   if(!GrandParent::onAdd())
      return false;
      
   //set the delete flag to false so the popup can open and close
   flags.set(DeleteOnLoseContent, FALSE);
   
   return TRUE;
}
 
void FGComboPopUp::setDim(Int32 w, Int32 hMin, Int32 hMax)
{
   if (sc && ac)
   {
      sc->extent.set(w, min(hMin + 2 * sc->getBorderThickness(), hMax));
      dim.set(w,min(hMin, hMax));
      sc->onWake();
      ac->extent.x = sc->getScrollContentCtrl()->extent.x - 1; //viewRect.len_x();
   }
}  

Int32 FGComboPopUp::getMouseCursorTag(void)
{
   Point2I cursorPos = root->getCursorPos();
   
   // handle state depressed
   if (sc->isDepressed())
   {
      switch (sc->getCurHitRegion())
      {
         case SimGui::ScrollCtrl::VertThumb:
         case SimGui::ScrollCtrl::HorizThumb:
            return IDBMP_CURSOR_GRAB;
         
         default:
            return IDBMP_CURSOR_HAND;
      }
   }
   
   switch (sc->findHitRegion(sc->globalToLocalCoord(cursorPos)))
   {
      case SimGui::ScrollCtrl::VertThumb:
      case SimGui::ScrollCtrl::HorizThumb:
         return IDBMP_CURSOR_OPENHAND;
         
      default:
         return IDBMP_CURSOR_HAND;
   }
}

void FGComboPopUp::setSelected(Point2I &cell)
{
   if (cell.x >= 0 && cell.y >= 0)
   {
      if (ac)
      {
         ac->setSelectedCell(cell);
      }
   }
}

Point2I FGComboPopUp::getSelected(void)
{
   if (ac)
   {
      return ac->getSelectedCell();
   }
   else return(Point2I(-1, -1));
}

void FGComboPopUp::setBoarder(void)
{
   if (sc)
   {
      sc->mbBoarder = TRUE;
      sc->boarderColor = HILITE_COLOR;
      sc->selectBoarderColor = HILITE_COLOR;
      sc->ghostBoarderColor = HILITE_COLOR;
      
      sc->mbOpaque = TRUE;
      sc->fillColor = GREEN_78;
      sc->selectFillColor = GREEN_78;
      sc->ghostFillColor = GREEN_78;
   }
}


//------------------------------------------------------------------------------

Int32 FGComboBox::getMouseCursorTag(void)
{
   return IDBMP_CURSOR_HAND;
}

void FGComboBox::setBitmaps(void)
{
   char buf[256];
   const char *bmpRoot = "POP_Arrow";
   
   //load the hi res bitmaps
   int i;
   for (i = 0; i < BMP_Count; i++)
   {
      sprintf(buf, "%s%s", bmpRoot, gBMPTags[i]);
      mBitmaps[i] = SimResource::get(manager)->load(buf);
      if (bool(mBitmaps[i])) mBitmaps[i]->attribute |= BMA_TRANSPARENT;
   }
   AssertFatal(mBitmaps[0].operator bool(), "POP_Arrow_DF.BMP was not loaded.\n");
   
   //load the lo res bitmaps
   for (i = 0; i < BMP_Count; i++)
   {
      sprintf(buf, "LR_%s%s", bmpRoot, gBMPTags[i]);
      mBitmaps[i + BMP_Count] = SimResource::get(manager)->load(buf);
      if (bool(mBitmaps[i + BMP_Count])) mBitmaps[i + BMP_Count]->attribute |= BMA_TRANSPARENT;
   }
}

bool FGComboBox::onAdd()
{
   if(!GrandParent::onAdd())
      return false;
   
   //set the bitmaps   
   setBitmaps();
   if (bool(mBitmaps[0]))
   {
      extent.y = mBitmaps[0]->getHeight() + 4;
   }

   //check the fonts
   hFont = SimResource::loadByTag(manager, IDFNT_9_STANDARD, true);
   AssertFatal(hFont.operator bool(), "Unable to load font.");
   hFontHL = SimResource::loadByTag(manager, IDFNT_9_HILITE, true);
   AssertFatal(hFontHL.operator bool(), "Unable to load font.");
   hFontDisabled = SimResource::loadByTag(manager, IDFNT_9_DISABLED, true);
   AssertFatal(hFontDisabled.operator bool(), "Unable to load font.");
   hFontShadow = SimResource::loadByTag(manager, IDFNT_9_BLACK, true);
   AssertFatal(hFontShadow.operator bool(), "Unable to load font.");
   hFontTitle = SimResource::loadByTag(manager, IDFNT_10_HILITE, true);
   AssertFatal(hFontTitle.operator bool(), "Unable to load font.");
   hFontTitleShadow = SimResource::loadByTag(manager, IDFNT_10_BLACK, true);
   AssertFatal(hFontTitleShadow.operator bool(), "Unable to load font.");
   hFontTitleDisabled = SimResource::loadByTag(manager, IDFNT_10_DISABLED, true);
   AssertFatal(hFontTitleDisabled.operator bool(), "Unable to load font.");
   
   
   //clear the combo to start
   setText("");
   
   return true;
}  
 
void FGComboBox::setSelected(Point2I &cell)
{
   if (cell.x >= 0 && cell.y >= 0)
   {
      FGComboPopUp *popUp = dynamic_cast<FGComboPopUp*>(popUpCtrl);
      if (popUp)
      {
         popUp->setSelected(cell);
         updateFromArrayCtrl();
      }
   }
}

//console member functions
void FGComboBox::addEntry(const char *buf, int id)
{
   if (popUpCtrl)
   {
      FGComboList *listCtrl = dynamic_cast<FGComboList*>(popUpCtrl->getArrayCtrl());
      if (listCtrl) listCtrl->addEntry(buf, id);
   }
   updateFromArrayCtrl();
}

void FGComboBox::deleteEntry(int id)
{
   if (popUpCtrl)
   {
      FGComboList *listCtrl = dynamic_cast<FGComboList*>(popUpCtrl->getArrayCtrl());
      if (listCtrl) listCtrl->deleteEntry(id);
   }
   updateFromArrayCtrl();
}

int FGComboBox::findEntry(const char *buf)
{
   if (popUpCtrl)
   {
      FGComboList *listCtrl = dynamic_cast<FGComboList*>(popUpCtrl->getArrayCtrl());
      if (listCtrl) return listCtrl->findEntry(buf);
   }
   return -1;
}

void FGComboBox::clear(void)
{
   if (popUpCtrl)
   {
      FGComboList *listCtrl = dynamic_cast<FGComboList*>(popUpCtrl->getArrayCtrl());
      if (listCtrl) listCtrl->clear();
   }
   updateFromArrayCtrl();
}

int FGComboBox::getSelectedEntry(void)
{
   if (popUpCtrl)
   {
      FGComboList *listCtrl = dynamic_cast<FGComboList*>(popUpCtrl->getArrayCtrl());
      if (listCtrl) return listCtrl->getSelectedEntry();
      else return -1;
   }
   else return -1;
}

void FGComboBox::setSelectedEntry(int id)
{
   if (popUpCtrl)
   {
      FGComboList *listCtrl = dynamic_cast<FGComboList*>(popUpCtrl->getArrayCtrl());
      if (listCtrl) listCtrl->setSelectedEntry(id);
   }
   updateFromArrayCtrl();
}

const char *FGComboBox::getSelectedText(void)
{
   if (popUpCtrl)
   {
      FGComboList *listCtrl = dynamic_cast<FGComboList*>(popUpCtrl->getArrayCtrl());
      if (listCtrl) return listCtrl->getSelectedText();
   }
   return NULL;
}

void FGComboBox::selectPrev(void)
{
   if (popUpCtrl)
   {
      FGComboList *listCtrl = dynamic_cast<FGComboList*>(popUpCtrl->getArrayCtrl());
      if (listCtrl) listCtrl->selectPrev();
   }
   updateFromArrayCtrl();
}

void FGComboBox::selectNext(void)
{
   if (popUpCtrl)
   {
      FGComboList *listCtrl = dynamic_cast<FGComboList*>(popUpCtrl->getArrayCtrl());
      if (listCtrl) listCtrl->selectNext();
   }
   updateFromArrayCtrl();
}

Point2I FGComboBox::getSelected(void)
{
   FGComboPopUp *popUp = dynamic_cast<FGComboPopUp*>(popUpCtrl);
   if (popUp)
   {
      return popUp->getSelected();

   }
   return Point2I(-1, -1);
}

void FGComboBox::drawText(GFXSurface *sfc, GFXFont *font, const char* text, Point2I &offset, Point2I &dest)
{
	if (!font)
		return;
   //if the string fits
   if (font->getStrWidth(text) < dest.x)
   {
      sfc->drawText_p(hFontShadow, &Point2I(offset.x - 1, offset.y + 1), text);
      sfc->drawText_p(font, &offset, text);
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
      sfc->drawText_p(hFontShadow, &Point2I(offset.x - 1, offset.y + 1), buf);
      sfc->drawText_p(font, &offset, buf);
   }
}

void FGComboBox::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   //Compiler Warning
   updateRect;
   
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   int colorTable[8] =
   {
   	BOX_INSIDE,
   	BOX_OUTSIDE,
   	BOX_LAST_PIX,
   	BOX_FRAME,
   	BOX_GHOST_INSIDE,
   	BOX_GHOST_OUTSIDE,
   	BOX_GHOST_LAST_PIX,
   	BOX_GHOST_FRAME,   
   };
   
   int colorOffset = (ghosted ? 4 : 0);
   Point2I tl = offset;
   Point2I br(offset.x + extent.x - 1, offset.y + extent.y - 1);
   
   //offset to match the pulldown menu
   tl.x += 3;
   br.x -= 3;
   
   //top edge
   sfc->drawLine2d(&Point2I(tl.x + 1, tl.y),
                   &Point2I(br.x - 1, tl.y),
                   colorTable[1 + colorOffset]);
   sfc->drawLine2d(&Point2I(tl.x, tl.y + 1),
                   &Point2I(br.x, tl.y + 1),
                   colorTable[0 + colorOffset]);
   sfc->drawPoint2d(&Point2I(tl.x, tl.y + 1), colorTable[2 + colorOffset]);
   sfc->drawPoint2d(&Point2I(br.x, tl.y + 1), colorTable[2 + colorOffset]);
   
   //bottom edge
   sfc->drawLine2d(&Point2I(tl.x, br.y - 1),
                   &Point2I(br.x, br.y - 1),
                   colorTable[0 + colorOffset]);
   sfc->drawLine2d(&Point2I(tl.x + 1, br.y),
                   &Point2I(br.x - 1, br.y),
                   colorTable[1 + colorOffset]);
   sfc->drawPoint2d(&Point2I(tl.x, br.y - 1), colorTable[2 + colorOffset]);
   sfc->drawPoint2d(&Point2I(br.x, br.y - 1), colorTable[2 + colorOffset]);
                   
   Point2I bmpOffset(offset.x + 10, offset.y + 4);
   
   if (mBitmapRootName[0])
   {
      sfc->drawText_p(hFontTitleShadow, &Point2I(bmpOffset.x, offset.y + 1), mBitmapRootName);
      sfc->drawText_p((! ghosted ? hFontTitle : hFontTitleDisabled), &Point2I(bmpOffset.x + 1, offset.y), mBitmapRootName);
      bmpOffset.x += hFontTitle->getStrWidth(mBitmapRootName) + 6;
   }
   
   GFXBitmap *bmp;
   if (ghosted) bmp = mBitmaps[BMP_Ghosted];
   else if (stateDepressed) bmp = mBitmaps[BMP_Pressed];
   else if (stateOver) bmp = mBitmaps[BMP_MouseOverStandard]; 
   else bmp = mBitmaps[BMP_Standard];
   if (! bmp) bmp = mBitmaps[BMP_Standard];
   
   //draw the down arrow bitmap
   sfc->drawBitmap2d(bmp, &Point2I(offset.x + extent.x - 7 - bmp->getWidth(), offset.y + 2));
   
   //write the current text
   if (text)
   {
      int textWidth = (offset.x + extent.x - 11 - bmp->getWidth() - 3) - bmpOffset.x;
      drawText(sfc, (!ghosted ? hFont : hFontDisabled), text,
                     Point2I(bmpOffset.x, offset.y + 1), Point2I(textWidth, extent.y));
   }
} 

void FGComboBox::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_BMP_ROOT_TAG, mBitmapRootName);
}

void FGComboBox::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read(IDITG_BMP_ROOT_TAG, mBitmapRootName);
}

static const int gComboBoxVersion = 0;   

Persistent::Base::Error FGComboBox::write( StreamIO &sio, int version, int user )
{
   sio.write(gComboBoxVersion);
   
   BYTE len = strlen(mBitmapRootName);
   sio.write(len);
   if (len > 0)
   {
      sio.write(len, mBitmapRootName);
   }
      
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FGComboBox::read( StreamIO &sio, int version, int user)
{
   int currentVersion;
   sio.read(&currentVersion);
   BYTE len;
   sio.read(&len);
   if (len > 0)
   {
      sio.read(len, mBitmapRootName);
   }
   mBitmapRootName[len] = 0;
   
   return Parent::read(sio, version, user);
}

//------------------------------------------------------------------------------

bool FGComboList::mbCconsoleFunctionsAdded = FALSE;

static const char *FGComboAddEntry(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 4)
   {
      Console->printf("%s(control, string, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGComboBox *cbList = NULL;
   if (ctrl) cbList = dynamic_cast<FGComboBox *>(ctrl);

   if (! cbList)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   cbList->addEntry(argv[2], atoi(argv[3]));
   return "TRUE";
}

static const char *FGComboDeleteEntry(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGComboBox *cbList = NULL;
   if (ctrl) cbList = dynamic_cast<FGComboBox *>(ctrl);

   if (! cbList)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   cbList->deleteEntry(atoi(argv[2]));
   return "TRUE";
}

static const char *FGComboFindEntry(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control, string);", argv[0]);
      return "-1";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGComboBox *cbList = NULL;
   if (ctrl) cbList = dynamic_cast<FGComboBox *>(ctrl);

   if (! cbList)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "-1";
   }
   static char buf[20];
   sprintf(buf, "%d",  cbList->findEntry(argv[2]));
   return buf;
}

static const char *FGComboClear(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGComboBox *cbList = NULL;
   if (ctrl) cbList = dynamic_cast<FGComboBox *>(ctrl);

   if (! cbList)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   cbList->clear();
   return "true";
}

static const char *FGComboSetSelected(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGComboBox *cbList = NULL;
   if (ctrl) cbList = dynamic_cast<FGComboBox *>(ctrl);

   if (! cbList)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   cbList->setSelectedEntry(atoi(argv[2]));
   return "true";
}

static const char *FGComboGetSelected(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "0";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGComboBox *cbList = NULL;
   if (ctrl) cbList = dynamic_cast<FGComboBox *>(ctrl);

   if (! cbList)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "0";
   }
   static char buf[20];
   sprintf(buf, "%d", cbList->getSelectedEntry());
   return buf;
}

static const char *FGComboGetSelectedText(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGComboBox *cbList = NULL;
   if (ctrl) cbList = dynamic_cast<FGComboBox *>(ctrl);

   if (! cbList)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "";
   }
   return cbList->getSelectedText();
}

static const char *FGComboSelectPrev(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGComboBox *cbList = NULL;
   if (ctrl) cbList = dynamic_cast<FGComboBox *>(ctrl);

   if (! cbList)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "";
   }
   cbList->selectPrev();
   return "TRUE";
}

static const char *FGComboSelectNext(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGComboBox *cbList = NULL;
   if (ctrl) cbList = dynamic_cast<FGComboBox *>(ctrl);

   if (! cbList)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "";
   }
   cbList->selectNext();
   return "TRUE";
}

//console member functions
void FGComboList::addEntry(const char *buf, int id)
{
   buf;
   id;
}

void FGComboList::deleteEntry(int id)
{
   id;
}


int FGComboList::findEntry(const char *buf)
{
   buf;
   return -1;
}

void FGComboList::clear(void)
{
}

int FGComboList::getSelectedEntry(void)
{
   return -1;
}

void FGComboList::setSelectedEntry(int id)
{
   id;
}

const char *FGComboList::getSelectedText(void)
{
   return NULL;
}

void FGComboList::selectPrev(void)
{
}

void FGComboList::selectNext(void)
{
}

bool FGComboList::onAdd()
{
   if (! mbCconsoleFunctionsAdded)
   {
      mbCconsoleFunctionsAdded = TRUE;
      Console->addCommand(0, "FGCombo::addEntry", FGComboAddEntry);
      Console->addCommand(0, "FGCombo::deleteEntry", FGComboDeleteEntry);
      Console->addCommand(0, "FGCombo::findEntry", FGComboFindEntry);
      Console->addCommand(0, "FGCombo::clear", FGComboClear);
      Console->addCommand(0, "FGCombo::setSelected", FGComboSetSelected);
      Console->addCommand(0, "FGCombo::getSelected", FGComboGetSelected);
      Console->addCommand(0, "FGCombo::getSelectedText", FGComboGetSelectedText);
      Console->addCommand(0, "FGCombo::selectPrev", FGComboSelectPrev);
      Console->addCommand(0, "FGCombo::selectNext", FGComboSelectNext);
   }
   
   if(!Parent::onAdd())
      return false;
      
   hFont = SimResource::loadByTag(manager, IDFNT_9_STANDARD, true);
   AssertFatal(hFont.operator bool(), "Unable to load font.");
   hFontHL = SimResource::loadByTag(manager, IDFNT_9_HILITE, true);
   AssertFatal(hFontHL.operator bool(), "Unable to load font.");
   hFontSel = SimResource::loadByTag(manager, IDFNT_9_SELECTED, true);
   AssertFatal(hFontSel.operator bool(), "Unable to load font.");
   hFontShadow = SimResource::loadByTag(manager, IDFNT_9_BLACK, true);
   AssertFatal(hFontShadow.operator bool(), "Unable to load font.");
   
   cellSize.set(extent.x, hFont->getHeight() + 2);
   return true;
}

void FGComboList::onPreRender()
{
}

void FGComboList::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   mouseOver;
   char buf[8];
   //Point2I clipMax = root->getContentControl()->extent;
   //clipMax.x -= 1;
   //clipMax.y -= 1;
   
   GFXFont *font;
   int color;
   if (mouseOver)
   {
      font = hFontHL;
      color = HILITE_COLOR;
   }
   else if (selected)
   {
      font = hFontSel;
      color = GREEN_78;
   }
   else
   {
      font = hFont;
      color = GREEN_78;
   }
   
   //RectI cellRect(offset.x, offset.y, min(clipMax.x, offset.x + extent.x),
   //                min(clipMax.y, offset.y + cellSize.y));
   RectI cellRect(offset.x, offset.y, offset.x + extent.x - 1, offset.y + cellSize.y - 1);
   
   //draw the background
   sfc->drawRect2d_f(&cellRect, color);
   
   //draw the text
   const char *cellText = getCellText(NULL, cell, Point2I(0, 0), Point2I(0, 0));
   if (cellText)
   {
      sfc->drawText_p(hFontShadow, &Point2I(offset.x + 15, offset.y - 1), cellText);
      sfc->drawText_p(font, &Point2I(offset.x + 16, offset.y - 2), cellText);
   }
}

bool FGComboList::becomeFirstResponder()
{
   return FALSE;
}

};