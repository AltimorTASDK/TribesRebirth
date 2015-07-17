#include "FearGuiArrayCtrl.h"
#include "FearCSDelegate.h"
#include "PlayerManager.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "g_font.h"
#include "simResource.h"
#include <m_qsort.h>
#include "fearGuiServerInfo.h"

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG(ServerInfoCtrl, FOURCC('F','G','s','i'));

bool ServerInfoCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   mServer = NULL;
   cellSize.set(640, hFont->getHeight() + 2);
   return true;
}

void ServerInfoCtrl::onWake(void)
{
   setSize(Point2I(1, 0));
   if (parent) cellSize.set(parent->extent.x, hFont->getHeight() + 2);
}

void ServerInfoCtrl::setServerInfo(FearCSDelegate::ServerInfo *info)
{
   mServer = info;
   
   int stop = 1;
   teamTabStops[0] = 0;
   playerTabStops[0] = 0;

   BYTE *s = (BYTE *) mServer->teamScoreHeading;
   while(*s && (stop < MaxColumns))
   {
      if(*s == '\t' && *(s+1))
      {
         teamTabStops[stop++] = *(s+1);
         s+=2;
      }
      else
         s++;
   }
   teamTabStops[stop] = 640;
   
   s = (BYTE *) mServer->clientScoreHeading;
   stop = 1;
   while(*s && (stop < MaxColumns))
   {
      if(*s == '\t' && *(s+1))
      {
         playerTabStops[stop++] = *(s+1);
         s+=2;
      }
      else
         s++;
   }
   playerTabStops[stop] = 640;
   
   //set the size
   setSize(Point2I(1, mServer->mInfoList.size()));
}

void ServerInfoCtrl::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool, bool)
{
   //make sure we have a server
   if (! mServer) return;
   
   char *s = &(mServer->mInfoList[cell.y].buf[0]);
   
   //find out which tab stops array to use
   int *tabStops;
   if (*s == '0') tabStops = &teamTabStops[0];
   else tabStops = &playerTabStops[0];
   s++;
   
   //find out whether we skip the count after the tab or not
   bool tabSkipOne;
   if (*s == '0') tabSkipOne = FALSE;
   else tabSkipOne = TRUE; 
   s++;
   
   int stop = 0;
   while (1)
   {
      char buft[256];
      int i = 0;
      while(*s && *s != '\t')
         buft[i++] = *s++;
      buft[i] = 0;
      if(i)
      {
         RectI cr = *sfc->getClipRect();
         sfc->setClipRect(
            &RectI(max(offset.x + tabStops[stop], cr.upperL.x),
                   max(offset.y, cr.upperL.y),
                   min(offset.x + tabStops[stop+1] - 2, cr.lowerR.x),
                   min(offset.y + 2 * hFont->getHeight(), cr.lowerR.y) ) );

         sfc->drawText_p((tabSkipOne ? hFont : hFontHL), &Point2I(offset.x + tabStops[stop] + 4, offset.y - 2), buft);
         sfc->setClipRect(&cr);
      }
      if(*s == '\t' && *(s+1) && tabSkipOne)
         s += 2;
      else if(*s == '\t')
         s++;

      if(!*s)
         break;
      stop++;
   }
}

void ServerInfoCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   Grandparent::onRender(sfc, offset, updateRect);
}

};
