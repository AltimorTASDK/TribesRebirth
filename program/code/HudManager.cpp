#include "HudManager.h"

#include <g_bitmap.h>
#include <g_surfac.h>
#include <simResource.h>

namespace FearGui
{

#define HUD_SPACING 4
#define HUDS_PER_ROW 4

void HudManager::addHud(HudCtrl *hud)
{
   Vector<HudCtrl *>::iterator i;
   
   for(i = huds.begin(); i != huds.end(); i++)
   {
      if(*i == hud)
         return;
   }

   i = huds.begin();
   if (i == huds.end())
   {
       GFXBitmap *bmp = hud->getOnBitmap();
       if(bmp)
       {
          extent.x = bmp->getWidth();
          extent.y = bmp->getHeight();
          mMaxExtent = extent;
       }
   }
   else
   {
        int hudWidth = (*i)->getOnBitmap()->getWidth() + HUD_SPACING;
        int hudHeight = (*i)->getOnBitmap()->getHeight() + HUD_SPACING;
        int hudCount = 0;
        for(i = huds.begin(); i != huds.end(); i++)
        {
            hudCount ++;
        }
        hudCount++;
        
        extent.x = hudWidth * min(hudCount, HUDS_PER_ROW);
        extent.y = hudHeight * ((hudCount / HUDS_PER_ROW) + (hudCount % HUDS_PER_ROW ? 1 : 0));

        //store the maximum extent so far
        mMaxExtent = extent;
        
        //make sure the we're still onscreen
        snap();
   }
   huds.push_back(hud);
   deleteNotify(hud);
}

void HudManager::onDeleteNotify(SimObject* io_pDelete)
{
   Vector<HudCtrl *>::iterator i;
   HudCtrl* obj = (HudCtrl *)io_pDelete;

   i = huds.begin();
   if (i == huds.end())
   {
       extent.x = extent.y = 0;
       mMaxExtent = extent;
   }
   else
   {        
      int hudWidth = (*i)->getOnBitmap()->getWidth() + HUD_SPACING;
      int hudHeight = (*i)->getOnBitmap()->getHeight() + HUD_SPACING;
      
      for(i = huds.begin(); i != huds.end(); i++)
      {
         if(*i == obj)
         {
            huds.erase(i);
            int hudCount = 0;
            for(i = huds.begin(); i != huds.end(); i++)
            {
               hudCount++;
            }
            
            extent.x = hudWidth * min(hudCount, HUDS_PER_ROW);
            extent.y = hudHeight * ((hudCount / HUDS_PER_ROW) + (hudCount % HUDS_PER_ROW ? 1 : 0));
            
            //store the maximum extent so far
            mMaxExtent = extent;
           
            Parent::onDeleteNotify(io_pDelete);
            return;
         }      
      }
   }
         
   Parent::onDeleteNotify(io_pDelete);
}

bool HudManager::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
   
   mOnBitmap      = SimResource::get(manager)->load("i_clock_on.bmp");
   mPressedBitmap = SimResource::get(manager)->load("i_clock_on.bmp");
   mOffBitmap     = SimResource::get(manager)->load("i_clock_off.bmp");
   extent.x   = mOnBitmap->getWidth();
   extent.y   = mOnBitmap->getHeight();
   mMaxExtent = extent;
   
   return true;
}


void HudManager::onMouseDown(const SimGui::Event &event)
{
   Point2I localPt = globalToLocalCoord(event.ptMouse);
   dragHit = false;

   Vector<HudCtrl *>::iterator i;
   
   //make sure we have at least one hud
   i = huds.begin();
   if (i == huds.end()) return;
   
   int hudWidth = (*i)->getOnBitmap()->getWidth() + HUD_SPACING;
   int hudHeight = (*i)->getOnBitmap()->getHeight() + HUD_SPACING;
   
   //find the index of the hud clicked on
   int index = (HUDS_PER_ROW * (localPt.y / hudHeight)) + (localPt.x / hudWidth);
   int j = 0;
   
   for(i = huds.begin(); (i != huds.end()) && (j < index) ; i++)
   {
      j++;
   }
   
   //see if we clicked on empty space
   if (i == huds.end()) return;
   
   //see if we clicked on the first hud - the hud manager's icon
   if(i == huds.begin())
   {
      //toggle the minimized state
      bMinimize = !bMinimize;

      //resize the hud
	  Point2I newExtent = extent;
	  if (bMinimize)
	     newExtent.x = mOnBitmap->getWidth();
	  else
	     newExtent.x = mMaxExtent.x;
	  resize(position, newExtent);

      Parent::onMouseDown(event);
   }
   
   //else toggle the hud icon we did click on
   else if (! bMinimize)
   {
      (*i)->toggleActive();
      (*i)->setPressed(TRUE);
   }
   mouseLock();
}

void HudManager::onMouseDragged(const SimGui::Event &event)
{
   //do nothing
   event;
}

void HudManager::onMouseUp(const SimGui::Event &event)
{
   event;
   
   //unpress all buttons
   Vector<HudCtrl *>::iterator i;
   for(i = huds.begin(); i != huds.end(); i++)
      (*i)->setPressed(FALSE);
   mouseUnlock();
}

void HudManager::onRightMouseDown(const SimGui::Event &event)
{
   //right clicking anywhere on the hud manager should drag the whole bar
   dragHit = true;
   Parent::onRightMouseDown(event);
   mouseLock();
}

void HudManager::onRightMouseDragged(const SimGui::Event &event)
{
   if(dragHit)
      Parent::onRightMouseDragged(event);
}

void HudManager::onRightMouseUp(const SimGui::Event &event)
{
   if(dragHit)
      Parent::onRightMouseUp(event);
   dragHit = false;
   mouseUnlock();
}

HudManager::HudManager()
{
   bMinimize = false;
   huds.push_back(this);
}

HudManager::~HudManager()
{
}

void HudManager::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   updateRect;
   Vector<HudCtrl *>::iterator i;
   i = huds.begin();
   if (i == huds.end()) return;
   
   int hudWidth = (*i)->getOnBitmap()->getWidth() + HUD_SPACING;
   int hudHeight = (*i)->getOnBitmap()->getHeight() + HUD_SPACING;
   
   //make sure we have a valid palette;   
   FearGui::InitializeHudColors(sfc->getPalette());

   int hudCount = 0;
   Point2I hudOffset = offset;
   
   for(i = huds.begin(); i != huds.end(); i++)
   {
      //choose the appropriate bitmap
      GFXBitmap *bmp;
      if ((*i)->isPressed())
         bmp = (*i)->getPressedBitmap();
      else if ((*i)->isActive())
	     bmp = (*i)->getOnBitmap();
	  else
         bmp = (*i)->getOffBitmap();

	  //make sure we found a valid bitmap
	  AssertWarn(bmp, "bitmap missing");
      if (! bmp) continue;

      //make sure the bitmap is transparent
      bmp->attribute |= BMA_TRANSPARENT;
	  sfc->drawBitmap2d(bmp, &hudOffset);
      
      hudCount++;
      if (! (hudCount % HUDS_PER_ROW))
      {
         hudOffset.x = offset.x;
         hudOffset.y += hudHeight;
      }
      else
      {
         hudOffset.x += hudWidth;
      }

      //if the hud manager is minimized, only draw the first hud icon
      if (bMinimize)
	     break;
   }
}

IMPLEMENT_PERSISTENT(HudManager);

};