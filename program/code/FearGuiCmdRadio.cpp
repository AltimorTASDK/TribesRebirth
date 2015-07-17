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
#include "g_bitmap.h"
#include "SimGuiSimpleText.h"
#include "darkstar.strings.h"
#include "fear.strings.h"
#include "simResource.h"
#include "fearGlobals.h"
#include "fearPlayerPSC.h"
#include "FearGuiCmdRadio.h"

namespace FearGui
{

inline FGCommandRadio::FGCommandRadio()
{
	mCmdTag = -1;
	mRadioTag = -1;
   
	bmpOnTag = 0;
	bmpOffTag = 0;

	loResBmpOnTag = 0;
	loResBmpOffTag = 0;

	mLowRes = FALSE;
   
	hiResPosition.set(0, 0);
	hiResExtent.set(100, 50);
	loResPosition.set(0, 0);
	loResExtent.set(100, 50);
}

bool FGCommandRadio::onAdd(void)
{
   if(!Parent::onAdd())
      return false;

   setBitmaps();
   
   return true;
}

void FGCommandRadio::onWake(void)
{
   hiResPosition = position;
   hiResExtent = extent;
}

void FGCommandRadio::setLowRes(bool lores)
{
   if (lores)
   { 
      mLowRes = TRUE;
      position = loResPosition;
      if (bool(mLoResOnBitmap))
      {
         extent.x = mLoResOnBitmap->getWidth();
         extent.y = mLoResOnBitmap->getHeight();
      }
   }
   else
   {
      mLowRes = FALSE;
      position = hiResPosition;
      if (bool(mOnBitmap))
      {
         extent.x = mOnBitmap->getWidth();
         extent.y = mOnBitmap->getHeight();
      }
   }
}

DWORD FGCommandRadio::onMessage(SimObject *sender, DWORD msg)
{
   //whatever the message, refresh
   setUpdate();
   
   if (msg == IDSYS_SIBLING_DISABLE)
   {
      FGCommandRadio *ctrl = dynamic_cast<FGCommandRadio*>(sender);
      if (ctrl && (ctrl->getRadioTag() == getRadioTag()))
      {
         set = false;
         
         //find the gui to set
         Int32 msg = getMessage();
         if (msg > 0)
         {
            SimGui::Control *ctrl = findRootControlWithTag(msg);
            if ((! ctrl) && root)
            {
               ctrl = root->getContentControl()->findControlWithTag(msg);
            }
            if (ctrl) ctrl->setVisible(set);
         }
      }
   }
   
   return GrandParent::onMessage(sender, msg);
}   

void FGCommandRadio::onAction()
{
   if (cg.psc && cg.psc->inVictoryMode()) return;
   
   //only if the tag is set, does the button behave like a radio button
   if (getRadioTag())
   {
      set = true;
      messageSiblings(IDSYS_SIBLING_DISABLE);
      //messageSiblings(getMessage());
      
      //find the gui to set
      SimGui::Control *ctrl = findRootControlWithTag(getMessage());
      if ((! ctrl) && root)
      {
         ctrl = root->getContentControl()->findControlWithTag(getMessage());
      }
      if (ctrl) ctrl->setVisible(set);
         
      //set the command field
      SimGui::SimpleText *textCtrl = (SimGui::SimpleText*)(root->getContentControl()->findControlWithTag(IDCTG_CURRENT_COMMAND));
      const char *cmdText = SimTagDictionary::getString(manager, mCmdTag);
      if (textCtrl && cmdText && cmdText[0])
      {
         textCtrl->setText(cmdText);
      }
   }
   else set = !set;
   
   if (set && consoleCommand[0])
      CMDConsole::getLocked()->evaluate(consoleCommand, false);
   else if ((! set) && altConsoleCommand[0])   
      CMDConsole::getLocked()->evaluate(altConsoleCommand, false);
      
   if(message)
      SimGui::Control::onMessage(this, message);
}   

void FGCommandRadio::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   GFXBitmap *bmp;
   if (!mLowRes) bmp = (set || stateDepressed ? mOnBitmap : mOffBitmap);
   else bmp = (set || stateDepressed ? mLoResOnBitmap : mLoResOffBitmap);
   
   //draw in the button
   if (bmp) sfc->drawBitmap2d(bmp, &offset);
}

void FGCommandRadio::setBitmaps(void)
{
   if (bmpOnTag > 0)  mOnBitmap = SimResource::loadByTag(manager, bmpOnTag, true);
   if (bmpOffTag > 0) mOffBitmap = SimResource::loadByTag(manager, bmpOffTag, true);
   if (! (bool)mOffBitmap) mOffBitmap = mOnBitmap;
   
   if ((bool)mOnBitmap) mOnBitmap->attribute |= BMA_TRANSPARENT;
   if ((bool)mOffBitmap) mOffBitmap->attribute |= BMA_TRANSPARENT;
   
   if (loResBmpOnTag > 0)  mLoResOnBitmap = SimResource::loadByTag(manager, loResBmpOnTag, true);
   if (loResBmpOffTag > 0) mLoResOffBitmap = SimResource::loadByTag(manager, loResBmpOffTag, true);
   if (! (bool)mLoResOffBitmap) mLoResOffBitmap = mLoResOnBitmap;
   
   if ((bool)mLoResOnBitmap) mLoResOnBitmap->attribute |= BMA_TRANSPARENT;
   if ((bool)mLoResOffBitmap) mLoResOffBitmap->attribute |= BMA_TRANSPARENT;
   
   if ((bool)mOnBitmap)
   {
      extent.x = mOnBitmap->getWidth();
      extent.y = mOnBitmap->getHeight();
   }
}   

void FGCommandRadio::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDSTR_CURRENT_COMMAND, true, "IDCMDR_*", mCmdTag);
   insp->write(IDSTR_RADIO_SET, true, "IDCTG_*", mRadioTag);
   insp->write(IDITG_BMP_ROOT_TAG, false, IDBMP_BEG_SWITCH, IDBMP_END_SWITCH, bmpOnTag);
   insp->write(IDITG_OFF_BMP_TAG, false, IDBMP_BEG_SWITCH, IDBMP_END_SWITCH, bmpOffTag);
   
   insp->write( IDITG_LR_POSITION,      loResPosition );
   insp->write( IDITG_LR_EXTENT,        loResExtent );
   insp->write(IDITG_LR_ON_BMP_TAG, false, IDBMP_BEG_SWITCH, IDBMP_END_SWITCH, loResBmpOnTag);
   insp->write(IDITG_LR_OFF_BMP_TAG, false, IDBMP_BEG_SWITCH, IDBMP_END_SWITCH, loResBmpOffTag);
}

void FGCommandRadio::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read(IDSTR_CURRENT_COMMAND, NULL, NULL, mCmdTag);
   insp->read(IDSTR_RADIO_SET, NULL, NULL, mRadioTag);
   insp->read(IDITG_BMP_ROOT_TAG, NULL, NULL, NULL, bmpOnTag);
   insp->read(IDITG_OFF_BMP_TAG, NULL, NULL, NULL, bmpOffTag);

   insp->read(IDITG_LR_POSITION,      loResPosition );
   insp->read(IDITG_LR_EXTENT,        loResExtent );
   insp->read(IDITG_LR_ON_BMP_TAG, NULL, NULL, NULL, loResBmpOnTag);
   insp->read(IDITG_LR_OFF_BMP_TAG, NULL, NULL, NULL, loResBmpOffTag);
   
   setBitmaps();
}   

Persistent::Base::Error FGCommandRadio::write( StreamIO &sio, int version, int user )
{
   sio.write(mCmdTag);
   sio.write(mRadioTag);
   sio.write(bmpOnTag);
   sio.write(bmpOffTag);
   
   sio.write(loResBmpOnTag);
   sio.write(loResBmpOffTag);
   loResPosition.write(sio);
   loResExtent.write(sio);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FGCommandRadio::read( StreamIO &sio, int version, int user)
{
   sio.read(&mCmdTag);
   sio.read(&mRadioTag);
   sio.read(&bmpOnTag);
   sio.read(&bmpOffTag);
   
   sio.read(&loResBmpOnTag);
   sio.read(&loResBmpOffTag);
   loResPosition.read(sio);
   loResExtent.read(sio);
   
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG(FGCommandRadio, FOURCC('F','G','c','r'));

};