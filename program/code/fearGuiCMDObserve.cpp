#include <observercamera.h>
#include <simguitsctrl.h>
#include <fearguicmdobserve.h>
#include <fearglobals.h>
#include <player.h>
#include <playermanager.h>
#include <console.h>
#include <fearguishellpal.h>
#include <g_surfac.h>
#include <fearplayerpsc.h>
#include <fear.strings.h>

#define INFO_FRAME_TIMEOUT    0.03f
#define INFO_FRAME_INCREMENT  4
#define INFO_SCREEN_SPACING   8
#define BAR_WIDTH             1

namespace FearGui
{
CMDObserve::CMDObserve()
: ts_control(NULL), fObject(false), iObservedID(0)
{
   cg.cmdObserve = this;
}

CMDObserve::~CMDObserve()
{
}

void CMDObserve::onWake()
{
   ts_control = static_cast<SimGui::TSControl *>
                           (findControlWithTag(IDCTG_COMMAND_TSOBSERVE));

   if (ts_control)
   {
      ctl_offset = ts_control->getPosition();
      ctl_extent = ts_control->getExtent();
   }
   setVisible(false);

   iObservedID = 0;
}

bool CMDObserve::observePlayer(int iPlayerID)
{
   char szID[10];

   if (iPlayerID == iObservedID)
   {
      return (false);
   }
   if(!isVisible())
      return false;

   sprintf(szID, "%d", iPlayerID);

   snowinfo.iCur  =  4;
   fTVSnowDone    = false;
   iEnergyBarTick = 0;
   iHealthBarTick = 0;
   fInfoBarsGrown = false;

   // Call console
   Console->executef(4, "remoteEval", "2048", "SCOM", szID);
   fObject     = true;
   iObservedID = iPlayerID;

   return (fObject);
}

void CMDObserve::activate(bool fActivate)
{
   char szID [ 20];
   char szBuf[256];

   if (fActivate == isVisible())
   {
      return;
   }

   if (fActivate)
   {
      if(!cg.playerManager)
         return;

      int sel = -1;
      PlayerManager::BaseRep *br = cg.playerManager->getBaseRepList();
      while(br)
      {
         if(br->selected)
         {
            sel = br->id;
            break;
         }
         br = br->nextClient;   
      }
      if(sel == -1)
         return;
      if (ts_control)
      {
         char buf[20];
         sprintf(buf, "%d", sel);
         Console->executef(4, "remoteEval", "2048", "SCOM", buf);
         fObject     = true;
         iObservedID = sel;

         eff_offset.x = ctl_offset.x + (ctl_extent.x / 2);
         eff_offset.y = ctl_offset.y + (ctl_extent.y / 2);
         eff_extent.x = 0;
         eff_extent.y = 2;

         snowinfo.iMin  =  4;
         snowinfo.iMax  = 80;
         snowinfo.iCur  =  4;
         snowinfo.iInc  =  6;
         snowinfo.rSPF  = random.getFloat(0.025f, 0.035f);

         iEnergyBarTick =  0;
         iHealthBarTick =  0;
         fUpdateInfo    = false;
         fTVStartUpDone = false;
         fTVSnowDone    = false;
         fTVRollDone    = false;
         fInfoBarsGrown = false;

         ts_control->setObject((SimObject *)cg.psc);
         ts_control->resize(eff_offset, eff_extent);
      }
      setVisible(true);
   }
   else
   {
      Console->executef(4, "remoteEval", "2048", "SCOM", "-1");
      setVisible(false);
   }
}

void CMDObserve::doTVStartUp()
{
   if (eff_offset.x > ctl_offset.x || eff_extent.x < ctl_extent.x)
   {
      eff_offset.x = (eff_offset.x -  8 < ctl_offset.x) ? ctl_offset.x : eff_offset.x -  8;
      eff_extent.x = (eff_extent.x + 16 > ctl_extent.x) ? ctl_extent.x : eff_extent.x + 16;
   }
   else if (eff_offset.y > ctl_offset.y || eff_extent.y < ctl_extent.y)
   {
      eff_offset.y = (eff_offset.y - 10 < ctl_offset.y) ? ctl_offset.y : eff_offset.y - 10;
      eff_extent.y = (eff_extent.y + 20 > ctl_extent.y) ? ctl_extent.y : eff_extent.y + 20;
   }
   else
   {
      fTVStartUpDone = true;
      snow_timer.reset();
   }

   ts_control->resize(eff_offset, eff_extent);
}

void CMDObserve::doTVSnow(GFXSurface *sfc, Point2I offset)
{
   for (int x = 0; x < eff_extent.x; x ++)
   {
      for (int y = 0; y < eff_extent.y; y ++)
      {
         if (random.getInt(0, snowinfo.iCur) == (snowinfo.iCur / 2))
         {
            sfc->drawPoint2d(&Point2I(offset.x + eff_offset.x + x, offset.y + eff_offset.y + y), WHITE);
         }
      }
   }
}

void CMDObserve::doTVSnowFadeOut(GFXSurface *sfc, Point2I offset)
{
   if (fObject && fTVStartUpDone && snow_timer.getElapsed() > snowinfo.rSPF)
   {
      snow_timer.reset();
      snowinfo.rSPF  = random.getFloat(0.025f, 0.035f);
      snowinfo.iCur += snowinfo.iInc;

      if (snowinfo.iCur >= snowinfo.iMax)
      {
         fTVSnowDone = true;
      }
   }
   
   doTVSnow(sfc, offset);
}

void CMDObserve::growInfoBars(int iHealth, int iEnergy)
{
   if (info_timer.getElapsed() > INFO_FRAME_TIMEOUT)
   {
      info_timer.reset();

      iEnergyBarTick += INFO_FRAME_INCREMENT;
      iHealthBarTick += INFO_FRAME_INCREMENT;

      if (iHealthBarTick > iHealth && iEnergyBarTick > iEnergy)
      {
         fInfoBarsGrown = true;
      }
      
      if (iHealthBarTick > iHealth)
      {
         iHealthBarTick = iHealth;
      }

      if (iEnergyBarTick > iEnergy)
      {
         iEnergyBarTick = iEnergy;
      }
   }
}

void CMDObserve::onPreRender()
{
   setUpdate();
}

void CMDObserve::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   int   iHealth, iEnergy;
   float rHealth, rEnergy;

   // Draw background
   sfc->drawRect2d_f(&RectI(offset, 
      Point2I(offset.x + extent.x - 1, offset.y + extent.y - 1)), BLACK);

   // Get health level
   rHealth = 1.0f - cg.psc->getDamageLevel();
   iHealth = int(rHealth * (float)ctl_extent.y);
   if (rHealth > 0.0f)
   {
      iHealth = max(iHealth, 1);
   }

   // Get energy level
   rEnergy = 1.0f - cg.psc->getEnergyLevel();
   iEnergy = int(rEnergy * (float)ctl_extent.y);
   if (rEnergy > 0.0f)
   {
      iEnergy = max(iEnergy, 1);
   }

   if (ts_control)
   {
      // TV start effect requires re-sizing - so do that before drawing
      if (fTVStartUpDone == false)
      {
         doTVStartUp();
      }

      // Health bars and TS control should only be drawn if we are
      // observing an object
      if (fObject)
      {
         if (fTVStartUpDone)
         {
            if (fInfoBarsGrown == false)
            {
               growInfoBars(iHealth, iEnergy);
            }
            // If all effects have been done, we're just keeping track of
            // the observed guy's health and energy
            else
            {
               iHealthBarTick = iHealth;
               iEnergyBarTick = iEnergy;
            }

            // Draw health HUD
            if (iHealthBarTick)
            {
               Int32 iColor;

               if (iHealth >= 57)
               {
                  iColor = GREEN;
               }
               else if (iHealth >= 28)
               {
                  iColor = YELLOW;
               }
               else
               {
                  iColor = RED;
               }

               sfc->drawRect2d_f(&RectI(
                  offset.x + ctl_offset.x + ctl_extent.x + 4, 
                  offset.y + ctl_offset.y + ctl_extent.y - iHealthBarTick, 
                  offset.x + ctl_offset.x + ctl_extent.x + 5, 
                  offset.y + ctl_offset.y + ctl_extent.y - 1), iColor);
            }

            // Draw energy HUD
            if (iEnergyBarTick)
            {
               sfc->drawRect2d_f(&RectI(
                  offset.x + ctl_offset.x + ctl_extent.x + 8,
                  offset.y + ctl_offset.y + ctl_extent.y - iEnergyBarTick, 
                  offset.x + ctl_offset.x + ctl_extent.x + 9, 
                  offset.y + ctl_offset.y + ctl_extent.y - 1), LIGHT_BLUE);
            }
         }
         // Draw TS control
         if (fObject)
         {
            renderChildControls(sfc, offset, updateRect);
         }
      }

      // Do snow effect (drawn on top of the TS)
      if (fTVSnowDone == false)
      {
         doTVSnowFadeOut(sfc, Point2I(offset.x, offset.y));
      }
   }
}

IMPLEMENT_PERSISTENT_TAG(CMDObserve, FOURCC('F','G','o','s'));
};
