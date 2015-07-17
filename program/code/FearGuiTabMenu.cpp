#include <g_bitmap.h>
#include <g_surfac.h>
#include <g_pal.h>
#include <g_Font.h>
#include <r_clip.h>
#include <simResource.h>

#include "FearGuiShellPal.h"
#include "FearGuiTabMenu.h"
#include "Darkstar.Strings.h"
#include "Fear.Strings.h"

namespace FearGui
{

bool FGTabMenu::onAdd(void)
{
   if (! Parent::onAdd())
      return FALSE;
      
   pbaTag = IDPBA_SHELL_TAB;
   loadBitmapArray();
   
   return true;
}

void FGTabMenu::loadBitmapArray()
{
   const char *pbaName = SimTagDictionary::getString(manager, pbaTag);
   AssertFatal(pbaName, "Invalid PBA name");
   bma = SimResource::get(manager)->load(pbaName, true);
   AssertFatal((bool)bma, "Unable to load bma");
   
   GFXBitmap *bitmap;
   for (int i = 0; i < TabBmpCount; i++)
   {
      bitmap = bma->getBitmap(i);
      AssertFatal(bitmap, "Unable to get PBA bitmap.");
      bitmap->attribute |= BMA_TRANSPARENT;
   }  
}
      
void FGTabMenu::renderTab(GFXSurface *sfc, Point2I &bmpOffset, FGTab *tab, int width, bool firstTab, bool lastTab, bool nextTabOn)
{
   int tabWidth = width;
   GFXBitmap *left, *mid, *right;
   
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   if (tab->isSet())
   {
      if (firstTab) left = bma->getBitmap(!ghosted ? TabBmpLeftMostOn : TabBmpLeftMostOnNA);
      else left = NULL;
      
      mid = bma->getBitmap(!ghosted ? TabBmpMidOn : TabBmpMidOnNA);
      
      if (lastTab) right = bma->getBitmap(!ghosted ? TabBmpRightMostOn : TabBmpRightMostOnNA);
      else right = bma->getBitmap(!ghosted ? TabBmpJoinRightOn : TabBmpJoinRightOnNA);
   }
   else
   {
      if (firstTab) left = bma->getBitmap(!ghosted ? TabBmpLeftMost : TabBmpLeftMostNA);
      else left = NULL; //the left portion would have been drawn by the prev tab
      
      mid = bma->getBitmap(!ghosted ? TabBmpMid : TabBmpMidNA);
      
      if (lastTab) right = bma->getBitmap(!ghosted ? TabBmpRightMost : TabBmpRightMostNA);
      else if (nextTabOn) right = bma->getBitmap(!ghosted ? TabBmpJoinLeftOn : TabBmpJoinLeftOnNA);
      else right = bma->getBitmap(!ghosted ? TabBmpJoinRight : TabBmpJoinRightNA);
   }
   
   //draw the left portion
   if (left)
   {
      sfc->drawBitmap2d(left, &bmpOffset);
      bmpOffset.x += left->getWidth();
      tabWidth -= left->getWidth();
   }
   
   //draw the middle
   sfc->drawBitmap2d(mid, &bmpOffset, &Point2I(tabWidth, mid->getHeight()));
   
   //draw the text
   tab->drawText(sfc, &bmpOffset, &Point2I(tabWidth, mid->getHeight()));
   bmpOffset.x += tabWidth;
   
   //draw the right
   sfc->drawBitmap2d(right, &bmpOffset);
   bmpOffset.x += right->getWidth();
}

void FGTabMenu::onRender(GFXSurface *sfc, Point2I offset, const Box2I & updateRect)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   //get the bitmaps
   GFXBitmap *topEdge     = bma->getBitmap(!ghosted ? TabBmpTopEdge : TabBmpTopEdgeNA);
   GFXBitmap *topRight    = bma->getBitmap(!ghosted ? TabBmpTopRight : TabBmpTopRightNA);
   GFXBitmap *leftEdge    = bma->getBitmap(!ghosted ? TabBmpLeftEdge : TabBmpLeftEdgeNA);
   GFXBitmap *bottomLeft  = bma->getBitmap(!ghosted ? TabBmpBottomLeft : TabBmpBottomLeftNA);
   GFXBitmap *bottomEdge  = bma->getBitmap(!ghosted ? TabBmpBottomEdge : TabBmpBottomEdgeNA);
   GFXBitmap *joinRight   = bma->getBitmap(!ghosted ? TabBmpJoinRight : TabBmpJoinRightNA);
   
   Point2I bmpOffset = offset;
   int hStretch, vStretch;
   
   //render each row of tabs
   //for (int j = 0; j < mRowCount; j++)
   //{
      FGTab *curTab = NULL;
      bool firstTab = TRUE;
      
      for(iterator i = begin(); i != end(); i++)
      {
         FGTab *tab = dynamic_cast<FGTab*>(*i);
         if (tab)
         {
            //see if this is there is a current tab to render
            if (curTab)
            {
               int tabWidth = curTab->extent.x;
               if (! firstTab) {
                  tabWidth -= joinRight->getWidth();
                  curTab->position.x = bmpOffset.x - offset.x - joinRight->getWidth();
               }
               else
               {
                  curTab->position.x = bmpOffset.x - offset.x;
               }
               curTab->position.y = bmpOffset.y - offset.y;
               renderTab(sfc, bmpOffset, curTab, tabWidth, firstTab, FALSE, tab->isSet());
               firstTab = FALSE;
            }
            
            //set the curTab
            curTab = tab;
         }
      }
      
      //draw the last tab in the row
      if (curTab)
      {
         int tabWidth = curTab->extent.x;
         if (! firstTab) 
         {
            tabWidth -= joinRight->getWidth();
            curTab->position.x = bmpOffset.x - offset.x - joinRight->getWidth();
         }
         else
         {
            curTab->position.x = bmpOffset.x - offset.x;
         }
         curTab->position.y = bmpOffset.y - offset.y;
         renderTab(sfc, bmpOffset, curTab, tabWidth, firstTab, TRUE, FALSE);
      }
   //}
   
   //now finish drawing the tab row
   int stretch = extent.x - (bmpOffset.x - offset.x) - topRight->getWidth();
   sfc->drawBitmap2d(topEdge, &bmpOffset, &Point2I(stretch, topEdge->getHeight()));
   bmpOffset.x += stretch;
   sfc->drawBitmap2d(topRight, &bmpOffset);
   bmpOffset.y += topEdge->getHeight();
   
   //sides
   bmpOffset.x = offset.x;
   vStretch = extent.y - topEdge->getHeight() - bottomLeft->getWidth();
   sfc->drawBitmap2d(leftEdge, &bmpOffset, &Point2I(leftEdge->getWidth(), vStretch));
   bmpOffset.x = offset.x + extent.x - leftEdge->getWidth();
   sfc->drawBitmap2d(leftEdge, &bmpOffset, &Point2I(leftEdge->getWidth(), vStretch), GFX_FLIP_X);
   //bottom
   bmpOffset.x = offset.x;
   bmpOffset.y = offset.y + extent.y - bottomLeft->getHeight();
   hStretch = extent.x - 2 * bottomLeft->getWidth();
   sfc->drawBitmap2d(bottomLeft, &bmpOffset);
   bmpOffset.x += bottomLeft->getWidth();
   bmpOffset.y = offset.y + extent.y - bottomEdge->getHeight();
   sfc->drawBitmap2d(bottomEdge, &bmpOffset, &Point2I(hStretch, bottomEdge->getHeight()));
   bmpOffset.x += hStretch;
   bmpOffset.y = offset.y + extent.y - bottomLeft->getHeight();
   sfc->drawBitmap2d(bottomLeft, &bmpOffset, GFX_FLIP_X);
   
   //render the chil'en
   renderChildControls(sfc, offset, updateRect);
}


Int32 FGTabMenu::getMouseCursorTag(void)
{
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
   //if the point is within the tabs header
   GFXBitmap *topEdge     = bma->getBitmap(TabBmpTopEdge);
   if (pt.y < topEdge->getHeight())
   {
      //loop through and find which tab was hit
      for(iterator i = begin(); i != end(); i++)
      {
         FGTab *tab = dynamic_cast<FGTab*>(*i);
         if (tab)
         {
            if (pt.x >= tab->position.x && pt.x <= tab->position.x + tab->extent.x)
            {
               return IDBMP_CURSOR_HAND;
            }
         }
      }
   }
   return Parent::getMouseCursorTag();
}

void FGTabMenu::onMouseDown(const SimGui::Event &event)
{
   GFXBitmap *topEdge     = bma->getBitmap(TabBmpTopEdge);
   
   if(!active)
   {
      Parent::onMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   
   //if the point is within the tabs header
   if (pt.y < topEdge->getHeight())
   {
      //loop through and find which tab was hit
      for(iterator i = begin(); i != end(); i++)
      {
         FGTab *tab = dynamic_cast<FGTab*>(*i);
         if (tab)
         {
            if (pt.x >= tab->position.x && pt.x <= tab->position.x + tab->extent.x)
            {
               //turn the tab on
               tab->setState(TRUE);
               
               setUpdate();
               break;
            }
         }
      }
   }
   
   Parent::onMouseDown(event);
}

void FGTabMenu::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   
   insp->write( IDITG_TAB_ROW_COUNT,       mRowCount );
}

void FGTabMenu::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   insp->read( IDITG_TAB_ROW_COUNT,    mRowCount );
}

Persistent::Base::Error FGTabMenu::write( StreamIO &sio, int a, int b)
{
   sio.write(mRowCount);
   
   return Parent::write(sio, a, b);
}

Persistent::Base::Error FGTabMenu::read( StreamIO &sio, int a, int b)
{
   sio.read(&mRowCount);
   
   return Parent::read(sio, a, b);
}

IMPLEMENT_PERSISTENT_TAG( FGTabMenu, FOURCC('F','G','t','m'));

//_______________________________________________________________________________________

FGTab::FGTab(void)
{
   extent.set(96, 32);
   set = FALSE;
   mRowNumber = 0;
}

Int32 FGTab::getMouseCursorTag(void)
{
   return IDBMP_CURSOR_HAND;
}

void FGTab::drawText(GFXSurface *sfc, Point2I *offset, Point2I *rect)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      ghosted = TRUE;
   }
   
   GFXFont *fnt = (ghosted ? hFontDisabled : (isSet() ? hFontHL : hFont));
   if (bool(hFontShadow))
   {
      sfc->drawText_p(hFontShadow, &Point2I(offset->x - 1 + (rect->x - fnt->getStrWidth(text)) / 2, 
                              offset->y - 1 + (rect->y - fnt->getHeight()) / 2),
                              text);
   }
   sfc->drawText_p(fnt, &Point2I(offset->x + (rect->x - fnt->getStrWidth(text)) / 2, 
                              offset->y - 2 + (rect->y - fnt->getHeight()) / 2),
                              text);
}
   
void FGTab::onWake(void)
{
   //check the fonts
   if (! (bool)hFont) hFont = SimResource::loadByTag(manager, IDFNT_FONT_DEFAULT, true);
   if (! (bool)hFontHL) hFontHL = SimResource::loadByTag(manager, IDFNT_FONT_DEFAULT, true);
   if (! (bool)hFontDisabled) hFontDisabled = SimResource::loadByTag(manager, IDFNT_FONT_DEFAULT, true);
   
   switch (fontNameTag)
   {
      case IDFNT_9_STANDARD:
         hFontShadow = SimResource::loadByTag(manager, IDFNT_9_BLACK, true);
         break;
         
      case IDFNT_10_STANDARD:
         hFontShadow = SimResource::loadByTag(manager, IDFNT_10_BLACK, true);
         break;
      
      default:
         hFontShadow = NULL;
         break;
   }
   
   setState(set);
}


void FGTab::setState(bool turnOn)
{
   set = turnOn;
   if (set)
   {
      //turn the page on
      SimGui::Control *ctrl = NULL;
      if (mPageTag > 0) ctrl = (SimGui::Control*)parent->findControlWithTag(mPageTag);
      if (ctrl) ctrl->setVisible(TRUE);
      
      //turn off all the other tabs
      messageSiblings(IDSYS_SIBLING_DISABLE);
   }
   else
   {
      SimGui::Control *ctrl = NULL;
      if (mPageTag > 0) ctrl = (SimGui::Control*)parent->findControlWithTag(mPageTag);
      if (ctrl) ctrl->setVisible(FALSE);
   }
}
   
DWORD FGTab::onMessage(SimObject *sender, DWORD msg)
{
   if (msg == IDSYS_SIBLING_DISABLE)
   {
      set = false;
      SimGui::Control *ctrl = NULL;
      if (mPageTag > 0) ctrl = (SimGui::Control*)parent->findControlWithTag(mPageTag);
      if (ctrl) ctrl->setVisible(FALSE);
   }
   
   return Parent::onMessage(sender, msg);
}   

void FGTab::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   
   insp->write( IDITG_TAB_PAGE_CTRL,   true, "*", mPageTag);
   insp->write( IDITG_TAB_SET,         set );
}

void FGTab::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   insp->read( IDITG_TAB_PAGE_CTRL,    NULL, NULL, mPageTag);
   insp->read( IDITG_TAB_SET,          set );
}

Persistent::Base::Error FGTab::write( StreamIO &sio, int a, int b)
{
   sio.write(mPageTag);
   Int32 output = set;
   sio.write(output);
   
   return Parent::write(sio, a, b);
}

Persistent::Base::Error FGTab::read( StreamIO &sio, int a, int b)
{
   sio.read(&mPageTag);
   Int32 input;
   sio.read(&input);
   set = input;
   
   return Parent::read(sio, a, b);
}

IMPLEMENT_PERSISTENT_TAG( FGTab, FOURCC('F','G','t','b'));

};