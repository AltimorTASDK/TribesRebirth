//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include "g_surfac.h"
#include "g_bitmap.h"
#include "g_font.h"
#include "simGame.h"
#include "simResource.h"
#include "fear.strings.h"
#include "fearHudCmdObj.h"
#include "fearGlobals.h"
#include "playerManager.h"

namespace FearGui
{

FGHCommandObjectivePage::FGHCommandObjectivePage(void)
{
   for (int i = 0; i < MAX_NUM_OBJECTIVES; i++)
   {
      objectives[i] = NULL;
   }
   extent.set(100, 100);
}

FGHCommandObjectivePage::~FGHCommandObjectivePage(void)
{
   for (int i = 0; i < MAX_NUM_OBJECTIVES; i++)
   {
      if (objectives[i]) delete objectives[i];
   }
}

void FGHCommandObjectivePage::newObjective(int index, const char *text)
{
   if (objectives[index]) delete objectives[index];
   if(!text)
   {
      objectives[index] = NULL;
      return;
   }
   
   objectives[index] = new SimGui::TextFormat();
   
   SimGui::TextFormat *newObj = objectives[index];
   newObj->setFont(0, "sf_white_10b.pft");
   newObj->setFont(1, "sf_green_10b.pft");
   newObj->setFont(2, "sf_yellow_10b.pft");
   newObj->setFont(3, "sf_orange214_10.pft");
   newObj->setFont(4, "sf_red_10b.pft");
   newObj->setFont(5, "sf_green_12.pft");
   newObj->setFont(6, "sf_yellow_12.pft");
   newObj->setFont(7, "sf_orange_12.pft");
   newObj->setFont(8, "sf_green_14.pft");
   newObj->setFont(9, "sf_yellow_14.pft");
   
   newObj->formatControlString(text, extent.x);
   
   //reset the height
   int height = 0;
   for (int i = 0; i < MAX_NUM_OBJECTIVES; i++)
   {
      if (objectives[i]) height += objectives[i]->getHeight();
   }
   Point2I newExt = extent;
   newExt.y = height + 4;
   resize(position, newExt);
   setUpdate();
}

bool FGHCommandObjectivePage::onAdd(void)
{
   return Parent::onAdd();
}

void FGHCommandObjectivePage::onWake(void)
{
   if (parent) extent.x = parent->extent.x;
   PlayerManager::ClientRep *me = cg.playerManager->findClient(manager->getId());
   if(me)
   {
      PlayerManager::TeamRep *team = cg.playerManager->findTeam(me->team);
      if(team)
         for(int i = 0; i < MAX_NUM_OBJECTIVES; i++)
            newObjective(i, team->objectives[i].text);
   }         
}

void FGHCommandObjectivePage::onPreRender()
{
   if (parent)
   {
      if(parent->extent.x != extent.x)
      {
         int height = 0;
         for (int i = 0; i < MAX_NUM_OBJECTIVES; i++)
         {
            if (objectives[i])
            {
               objectives[i]->formatControlString(NULL, parent->extent.x, TRUE);
               height += objectives[i]->getHeight();
            }
         }
         Point2I newExt(parent->extent.x, height + 4);
         if(newExt.x != extent.x || newExt.y != extent.y)
            resize(position, newExt);
      }
   }
}

void FGHCommandObjectivePage::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   Point2I curOffset = offset;
   
   for (int i = 0; i < MAX_NUM_OBJECTIVES; i++)
   {
      if (objectives[i])
      {
         objectives[i]->onRender(sfc, curOffset, updateRect);
         curOffset.y += objectives[i]->getHeight();
      }
   }
}
      
IMPLEMENT_PERSISTENT_TAG( FGHCommandObjectivePage, FOURCC('F','G','c','o') );

};