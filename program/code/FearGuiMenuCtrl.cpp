#include <g_bitmap.h>
#include <g_surfac.h>
#include <g_pal.h>
#include <g_Font.h>
#include <r_clip.h>
#include <simResource.h>
#include <console.h>
#include "simGuiSimpleText.h"
#include "FearGuiMenuCtrl.h"
#include "Darkstar.Strings.h"
#include "Fear.Strings.h"

namespace FearGui
{

bool FGMenuCtrl::onAdd(void)
{
   if (! Parent::onAdd())
      return FALSE;
      
   return true;
}

void FGMenuCtrl::onWake(void)
{
   extent.x = parent->extent.x;
}

void FGMenuCtrl::disableAll(void)
{
   for(iterator i = begin(); i != end(); i++)
   {
      FGMenuItem *item = dynamic_cast<FGMenuItem*>(*i);
      if (item)
      {
         item->onMessage(this, IDSYS_SIBLING_DISABLE);
      }
   }
}

void FGMenuCtrl::onPreRender(void)
{
   int newHeight = 0;
   for(iterator i = begin(); i != end(); i++)
   {
      FGMenuItem *item = dynamic_cast<FGMenuItem*>(*i);
      if (item)
      {
         item->position.set(0, newHeight);
         item->calculateExtent();
         newHeight += item->extent.y;
      }
   }
   if (newHeight != extent.y)
   {
      extent.y = newHeight;
      setUpdate();
      if (parent) parent->setUpdate();
   }
}

void FGMenuCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I & updateRect)
{
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(1);
      if ((! active) ||
          (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
      {
         // ??? DMM
      }
   }
   
   Point2I itemOffset = offset;   
   for(iterator i = begin(); i != end(); i++)
   {
      FGMenuItem *item = dynamic_cast<FGMenuItem*>(*i);
      if (item)
      {
         item->onRender(sfc, itemOffset, updateRect);
         itemOffset.y += item->extent.y;
      }
   }
}        

IMPLEMENT_PERSISTENT_TAG( FGMenuCtrl, FOURCC('F','G','m','z'));

//_______________________________________________________________________________________

FGMenuItem::FGMenuItem(void)
{
   mbSet = FALSE;
   mbExpanded = FALSE;
}


bool FGMenuItem::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
    
   //load the fonts
   hFont = SimResource::get(manager)->load("sf_orange214_10.pft");
   hFontHL = SimResource::get(manager)->load("sf_white_10.pft");
   hFontShadow = SimResource::get(manager)->load("sf_black_10.pft");
   hFontDisabled = SimResource::get(manager)->load("sf_grey100_10b.pft");
   hFontHeader = SimResource::get(manager)->load("sf_orange255_10.pft");
   
   //load the bitmaps
   mClosedDF = SimResource::get(manager)->load("MNU_Closed_DF.BMP");
   AssertFatal(mClosedDF.operator bool(), "Unable to load MNU_Closed_DF.BMP"); 
   mClosedDF->attribute |= BMA_TRANSPARENT;
   
   mClosedON = SimResource::get(manager)->load("MNU_Closed_ON.BMP");
   AssertFatal(mClosedON.operator bool(), "Unable to load MNU_Closed_ON.BMP"); 
   mClosedON->attribute |= BMA_TRANSPARENT;
   
   mExpandedDF = SimResource::get(manager)->load("MNU_Expand_DF.BMP");
   AssertFatal(mExpandedDF.operator bool(), "Unable to load MNU_Expand_DF.BMP"); 
   mExpandedDF->attribute |= BMA_TRANSPARENT;
   
   mExpandedON = SimResource::get(manager)->load("MNU_Expand_ON.BMP");
   AssertFatal(mExpandedON.operator bool(), "Unable to load MNU_Expand_ON.BMP"); 
   mExpandedON->attribute |= BMA_TRANSPARENT;
   
   return TRUE;
}

Int32 FGMenuItem::getMouseCursorTag(void)
{
   return IDBMP_CURSOR_HAND;
}

void FGMenuItem::calculateExtent(void)
{
   Point2I newExtent = extent;
   if (parent) newExtent.x = parent->extent.x;
   newExtent.y = mExpandedDF->getHeight();
   
   if (mbExpanded)
   {
      for (iterator i = begin(); i != end(); i++)
      {
         FGMenuItem *item = dynamic_cast<FGMenuItem*>(*i);
         if (item)
         {
            item->position.set(0, newExtent.y);
            item->calculateExtent();
            newExtent.y += item->extent.y;
         }
      }
   }
   extent = newExtent;
}

void FGMenuItem::onWake(void)
{
   extent.set(parent->extent.x, mClosedDF->getHeight());
   setState(mbSet);
   setExpanded(mbExpanded);
}


void FGMenuItem::setState(bool turnOn)
{
   if (turnOn)
   {
      //turn all pages off
      FGMenuCtrl *menu = dynamic_cast<FGMenuCtrl*>(findRootControlWithTag(IDCTG_MENU_ROOT_PAGE));
      if (menu) menu->disableAll();
      
      //turn the page on
      SimGui::Control *ctrl = NULL;
      if (mPageTag > 0) ctrl = (SimGui::Control*)findRootControlWithTag(mPageTag);
      if (ctrl) ctrl->setVisible(TRUE);
      
   }
   else
   {
      SimGui::Control *ctrl = NULL;
      if (mPageTag > 0) ctrl = (SimGui::Control*)findRootControlWithTag(mPageTag);
      if (ctrl) ctrl->setVisible(FALSE);
   }
   mbSet = turnOn;
   setUpdate();
}
   
void FGMenuItem::setExpanded(bool value)
{
   mbExpanded = value;
   for (iterator i = begin(); i != end(); i++)
   {
      FGMenuItem *item = dynamic_cast<FGMenuItem*>(*i);
      if (item)
      {
         item->active = value;
         item->setUpdate();
      }
   }
   setUpdate();
}

DWORD FGMenuItem::onMessage(SimObject *sender, DWORD msg)
{
   if (msg == IDSYS_SIBLING_DISABLE)
   {
      mbSet = false;
      SimGui::Control *ctrl = NULL;
      if (mPageTag > 0) ctrl = (SimGui::Control*)findRootControlWithTag(mPageTag);
      if (ctrl) ctrl->setVisible(FALSE);
      
      //also turn off all of it's children
      for(iterator i = begin(); i != end(); i++)
      {
         FGMenuItem *item = dynamic_cast<FGMenuItem*>(*i);
         if (item) item->onMessage(sender, IDSYS_SIBLING_DISABLE);
      }
   }
   
   setUpdate();
   
   //return Parent::onMessage(sender, msg);
   return 0;
}   

void FGMenuItem::onAction(void)
{
   if (size() > 0)
   {
      //setExpanded(! mbExpanded);
   }
   else
   {
      setState(TRUE);
   }
   setUpdate();
}
   
void FGMenuItem::drawText(GFXSurface *sfc, Point2I *offset, Point2I *rect)
{
   bool ghosted = FALSE;
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(1);
      if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
      {
         ghosted = TRUE;
      }
   }
   
   GFXFont *fnt = (ghosted ? hFontDisabled : (size() > 0 ? hFontHeader : (isSet() ? hFontHL : hFont)));
   if (bool(hFontShadow))
   {
      sfc->drawText_p(hFontShadow, &Point2I(offset->x - 1, offset->y - 1 + (rect->y - fnt->getHeight()) / 2),
                              text);
   }
   sfc->drawText_p(fnt, &Point2I(offset->x, offset->y - 2 + (rect->y - fnt->getHeight()) / 2),
                              text);
}

void FGMenuItem::onRender(GFXSurface *sfc, Point2I offset, const Box2I & updateRect)
{
   Point2I itemOffset = offset;
   int indent = mExpandedDF->getWidth();
   
   /*
   if (size() > 0)
   {
      //see if any of its children are selected
      bool childOn = FALSE;
      for(iterator i = begin(); i != end(); i++)
      {
         FGMenuItem *item = dynamic_cast<FGMenuItem*>(*i);
         if (item)
         {
            if (item->isSet())
            {
               childOn = TRUE;
               break;
            }
         }
      }
      if (! mbExpanded)
      {
         sfc->drawBitmap2d(childOn ? mClosedON : mClosedDF, &itemOffset);
      }
      else
      {
         sfc->drawBitmap2d(childOn ? mExpandedON : mExpandedDF, &itemOffset);
      }
   }
   */
   
   //draw the label
   drawText(sfc, &Point2I(itemOffset.x + indent, itemOffset.y),
                  &Point2I(extent.x, mExpandedDF->getHeight()));
   itemOffset.y += mExpandedDF->getHeight();
   
   if (size() > 0 && mbExpanded)
   {
      itemOffset.x += indent;
      for(iterator i = begin(); i != end(); i++)
      {
         FGMenuItem *item = dynamic_cast<FGMenuItem*>(*i);
         if (item)
         {
            item->onRender(sfc, itemOffset, updateRect);
            itemOffset.y += item->extent.y;
         }
      }
   }
}

void FGMenuItem::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   
   insp->write( IDITG_TAB_PAGE_CTRL,   true, "*", mPageTag);
   insp->write( IDITG_TAB_SET,         mbSet );
   insp->write( IDITG_OPT_EXPANDED,    mbExpanded );
}

void FGMenuItem::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   insp->read( IDITG_TAB_PAGE_CTRL,    NULL, NULL, mPageTag);
   insp->read( IDITG_TAB_SET,          mbSet );
   insp->read( IDITG_OPT_EXPANDED,     mbExpanded );
   
   //make sure the fonts are constant
   hFont = SimResource::get(manager)->load("sf_orange214_10.pft");
   hFontHL = SimResource::get(manager)->load("sf_white_10.pft");
   hFontShadow = SimResource::get(manager)->load("sf_black_10.pft");
   hFontDisabled = SimResource::get(manager)->load("sf_grey100_10b.pft");
   hFontHeader = SimResource::get(manager)->load("sf_orange255_10.pft");
   
   setState(mbSet);
   setExpanded(mbExpanded);
}

Persistent::Base::Error FGMenuItem::write( StreamIO &sio, int a, int b)
{
   sio.write(mPageTag);
   sio.write(mbSet);
   sio.write(mbExpanded);
   
   return Parent::write(sio, a, b);
}

Persistent::Base::Error FGMenuItem::read( StreamIO &sio, int a, int b)
{
   sio.read(&mPageTag);
   sio.read(&mbSet);
   sio.read(&mbExpanded);
   
   return Parent::read(sio, a, b);
}

IMPLEMENT_PERSISTENT_TAG( FGMenuItem, FOURCC('F','G','m','i'));

};