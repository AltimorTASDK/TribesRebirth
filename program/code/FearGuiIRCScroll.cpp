//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <string.h>
#include <g_surfac.h>
#include <simTagDictionary.h>
#include "simBase.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "console.h"
#include "SimGuiSimpleText.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include "fear.strings.h"
#include "simResource.h"
#include "fearGlobals.h"
#include "FearGuiIRCChanCtrl.h"
#include "FearGuiIRCScroll.h"

namespace FearGui
{

static const char *gBMPTags[FGIRCScrollChannel::BMP_Count] =
{
	"_DF.BMP",	//default
	"_ON.BMP",	//depressed
	"_MW.BMP",	//message waiting
	"_MP.BMP",	//message waiting pressed
};

inline FGIRCScrollChannel::FGIRCScrollChannel()
{
	mbScrollLeft = TRUE;
}

Int32 FGIRCScrollChannel::getMouseCursorTag(void)
{
   FGIRCChannelCtrl *ircChans = dynamic_cast<FGIRCChannelCtrl*>(findRootControlWithTag(IDCTG_IRC_ROOM_LIST));
   if (! ircChans) return 0;
   
   if (mbScrollLeft && ircChans->canScrollLeft())
   {
      return IDBMP_CURSOR_HAND;
   }
   else if ((! mbScrollLeft ) && ircChans->canScrollRight())
   {
      return IDBMP_CURSOR_HAND;
   }
   return 0;
}

void FGIRCScrollChannel::setBitmaps(void)
{
   char buf[256];
   
   //load the hi res bitmaps
   for (int i = 0; i < BMP_Count; i++)
   {
      sprintf(buf, "IRC_ScrollLeft%s", gBMPTags[i]);
      mBitmaps[i] = SimResource::get(manager)->load(buf);
      if (bool(mBitmaps[i])) mBitmaps[i]->attribute |= BMA_TRANSPARENT;
   }
   
   //set the extent
   if (bool(mBitmaps[0]))
   {
      extent.x = mBitmaps[0]->getWidth();
      extent.y = mBitmaps[0]->getHeight();
   }
}

bool FGIRCScrollChannel::onAdd(void)
{
   if(!Parent::onAdd())
      return false;

   setBitmaps();
   
   return true;
}

void FGIRCScrollChannel::onAction()
{
   if (! root) return;
   
   FGIRCChannelCtrl *ircChans = dynamic_cast<FGIRCChannelCtrl*>(findRootControlWithTag(IDCTG_IRC_ROOM_LIST));
   if (ircChans)
   {
      if (mbScrollLeft && ircChans->canScrollLeft())
      {
         ircChans->scrollLeft();
      }
      else if ((! mbScrollLeft) && ircChans->canScrollRight())
      {
         ircChans->scrollRight();
      }
   }
}   

void FGIRCScrollChannel::onPreRender(void)
{
   setUpdate();
}

void FGIRCScrollChannel::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   //find the bitmap
   GFXBitmap *bmp = NULL;
   
   if (! root) return;
   
   FGIRCChannelCtrl *ircChans = dynamic_cast<FGIRCChannelCtrl*>(findRootControlWithTag(IDCTG_IRC_ROOM_LIST));
   if (! ircChans) return;
   
   if (mbScrollLeft)
   {
      if (ircChans->canScrollLeft())
      {
         if (ircChans->msgWaitingLeft())
         {
            if (stateDepressed) bmp = mBitmaps[BMP_MsgWaitingPressed];
            if (! bmp) bmp = mBitmaps[BMP_MsgWaiting];
         }
         else
         {
            if (stateDepressed) bmp = mBitmaps[BMP_Pressed];
            else bmp = mBitmaps[BMP_Default];
         }
      }
   }
   else
   {
      if (ircChans->canScrollRight())
      {
         if (ircChans->msgWaitingRight())
         {
            Console->printf("msgs waiting right");
            if (stateDepressed) bmp = mBitmaps[BMP_MsgWaitingPressed];
            if (! bmp) bmp = mBitmaps[BMP_MsgWaiting];
         }
         else
         {
            if (stateDepressed) bmp = mBitmaps[BMP_Pressed];
            else bmp = mBitmaps[BMP_Default];
         }
      }
   }
   
   //draw in the button
   if (mbScrollLeft)
   {
      if (bmp) sfc->drawBitmap2d(bmp, &offset);
   }
   else
   {
      if (bmp) sfc->drawBitmap2d(bmp, &offset, GFX_FLIP_X);
   }
}

void FGIRCScrollChannel::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   
   insp->write(IDITG_ROTATION, mbScrollLeft);
}

void FGIRCScrollChannel::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read(IDITG_ROTATION, mbScrollLeft);
}   

Persistent::Base::Error FGIRCScrollChannel::write( StreamIO &sio, int version, int user )
{
   sio.write(mbScrollLeft);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FGIRCScrollChannel::read( StreamIO &sio, int version, int user)
{
   
   sio.read(&mbScrollLeft);
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG(FGIRCScrollChannel, FOURCC('F','G','i','k'));

};