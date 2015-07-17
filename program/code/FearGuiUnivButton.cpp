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
#include "SimGuiSimpleText.h"
#include "darkstar.strings.h"
#include "fear.strings.h"
#include "simResource.h"
#include "fearGlobals.h"
#include "fearPlayerPSC.h"
#include "FearGuiUnivButton.h"

namespace FearGui
{

static Sfx::Profile base2dSfxProfile;

const int gButtonVersion = 3;

static const char *gBMPTags[FGUniversalButton::BMP_Count] =
{
	"_DF.BMP",	//default
	"_HI.BMP",	//hilite
	"_MD.BMP",	//mouse-over default
	"_MH.BMP",	//mouse-over hilite
	"_ON.BMP",	//depressed
	"_NA.BMP",	//not available (ghosted)
	"_BD.BMP",	//default (background)
	"_BO.BMP",	//pressed (background)
};

inline FGUniversalButton::FGUniversalButton()
{
   active = true;
   mbSwitch = false;
   mbSet = false;
   mbFlashing = FALSE;
   mFlashTime = -1;
   
	mBitmapTag = -1;
	mRadioTag = -1;
   
   mBitmapRootName[0] = '\0';
   
	mbLowRes = FALSE;
   
	mHiResPosition.set(0, 0);
	mLoResPosition.set(0, 0);
   
   mbMirrorConsoleVar = FALSE;
   
   currentVersion = gButtonVersion;
}

Int32 FGUniversalButton::getMouseCursorTag(void)
{
   return (active ? IDBMP_CURSOR_HAND : 0);
}

void FGUniversalButton::setBitmaps(void)
{
   char buf[256];
   const char *bmpRoot;
   if (currentVersion < 2)
   {
      bmpRoot = SimTagDictionary::getString(manager, mBitmapTag);
   }
   else
   {
      bmpRoot = &mBitmapRootName[0];
   }
   if (! bmpRoot) return;
   
   //load the hi res bitmaps
   int i;
   for (i = 0; i < BMP_Count; i++)
   {
      sprintf(buf, "%s%s", bmpRoot, gBMPTags[i]);
      mBitmaps[i] = SimResource::get(manager)->load(buf);
      if (bool(mBitmaps[i])) mBitmaps[i]->attribute |= BMA_TRANSPARENT;
   }
   
   //load the lo res bitmaps
   for (i = 0; i < BMP_Count; i++)
   {
      sprintf(buf, "LR_%s%s", bmpRoot, gBMPTags[i]);
      mBitmaps[i + BMP_Count] = SimResource::get(manager)->load(buf);
      if (bool(mBitmaps[i + BMP_Count])) mBitmaps[i + BMP_Count]->attribute |= BMA_TRANSPARENT;
   }
   
   mRadioCheckBMP = SimResource::get(manager)->load("RAD_CheckX_ON.BMP");
   if (bool(mRadioCheckBMP)) mRadioCheckBMP->attribute |= BMA_TRANSPARENT; 
   mRadioCheckGhostBMP = SimResource::get(manager)->load("RAD_CheckX_NA.BMP");
   if (bool(mRadioCheckGhostBMP)) mRadioCheckGhostBMP->attribute |= BMA_TRANSPARENT; 
}

bool FGUniversalButton::onAdd(void)
{
   if(!Parent::onAdd())
      return false;

   //initialize the 2d sound profile
   base2dSfxProfile.flags = 0;
   base2dSfxProfile.baseVolume = 0;
   base2dSfxProfile.coneInsideAngle = 90;
   base2dSfxProfile.coneOutsideAngle= 180; 
   base2dSfxProfile.coneVector.set(0.0f, -1.0f, 0.0f);
   base2dSfxProfile.coneOutsideVolume = 0.0f;  
   base2dSfxProfile.minDistance = 30.0f;     
   base2dSfxProfile.maxDistance = 300.0f;     
   
   setBitmaps();
   mbLowRes = FALSE;
   
   //mirror the console variable
   if (mbSwitch && consoleVariable[0])
   {
      const char *conVar = getVariable();
      if (! stricmp(conVar, "TRUE"))
      {
         mbSet = TRUE;
      }
      else mbSet = FALSE;
      setVariable(mbSet ? "TRUE" : "FALSE");
   }
   
   return true;
}

void FGUniversalButton::onWake(void)
{
   mHiResPosition = position;
}

void FGUniversalButton::setLowRes(bool lores)
{
   mbLowRes = lores;
   if (lores)
   { 
      position = mLoResPosition;
      if (bool(mBitmaps[BMP_Count]))
      {
         extent.x = mBitmaps[BMP_Count]->getWidth();
         extent.y = mBitmaps[BMP_Count]->getHeight();
      }
   }
   else
   {
      position = mHiResPosition;
      if (bool(mBitmaps[0]))
      {
         extent.x = mBitmaps[0]->getWidth();
         extent.y = mBitmaps[0]->getHeight();
      }
   }
}

/*
bool FGUniversalButton::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
		onEvent (SimActionEvent);
      
      default:
         return Parent::processEvent(event);
	}
}
*/

DWORD FGUniversalButton::onMessage(SimObject *sender, DWORD msg)
{
   //whatever the message, refresh
   setUpdate();
   
   if (msg == IDSYS_SIBLING_DISABLE)
   {
      FGUniversalButton *ctrl = dynamic_cast<FGUniversalButton*>(sender);
      if (ctrl && (ctrl->getRadioTag() == getRadioTag()))
      {
         mbSet = false;
         
         //find the gui to set
         Int32 msg = getMessage();
         if (msg > 0)
         {
            SimGui::Control *ctrl = findRootControlWithTag(msg);
            if ((! ctrl) && root)
            {
               ctrl = root->getContentControl()->findControlWithTag(msg);
            }
            if (ctrl) ctrl->setVisible(mbSet);
         }
      }
   }
   
   return Parent::onMessage(sender, msg);
}   

void FGUniversalButton::onMouseDown(const SimGui::Event &event)
{
   if (active)
   {
      //find the soundfx manager
      Sfx::Manager *sfx = Sfx::Manager::find(manager);
      if (mbSwitch)
      {
         if ((! mbSet) && sfx) sfx->play(stringTable.insert("Button2.wav"), 0, &base2dSfxProfile);
      }
      else
      {
         if (sfx) sfx->play(stringTable.insert("Button1.wav"), 0, &base2dSfxProfile);
      }
   }
   Parent::onMouseDown(event);
}

void FGUniversalButton::onAction()
{
   if (! active) return;
   
   //find the soundfx manager
   Sfx::Manager *sfx = Sfx::Manager::find(manager);
            
   //only if the tag is set, does the button behave like a radio button
   if (getRadioTag())
   {
      mbSet = true;
      messageSiblings(IDSYS_SIBLING_DISABLE);
      
      //find the gui to set
      SimGui::Control *ctrl = findRootControlWithTag(getMessage());
      if ((! ctrl) && root)
      {
         ctrl = root->getContentControl()->findControlWithTag(getMessage());
      }
      if (ctrl) ctrl->setVisible(mbSet);
   }
   else if (mbSwitch)
   {
      mbSet = !mbSet;
      if (mbSet)
      {
         setVariable("TRUE");
      
      }
      else
      {
         setVariable("FALSE");
         if (sfx) sfx->play(stringTable.insert("Button1.wav"), 0, &base2dSfxProfile);
      }
   }
   else
   {
      mbSet = FALSE;
   }
   
   //store the old gui for referrence
   SimGui::Canvas *cvs = root;

   SimGui::Control *oldGui = cvs->getContentControl();
   
   if (mbSwitch)
   {
      if (mbSet && consoleCommand[0]) Console->evaluate(consoleCommand, false);
      else if ((! mbSet) && altConsoleCommand[0]) Console->evaluate(altConsoleCommand, false);
   }
   else if (consoleCommand[0]) Console->evaluate(consoleCommand, false);

   if(cvs->getContentControl() != oldGui) // the gui changed!
      return;
      
   if (message)
   {
      //make sure the root content control hasn't changed
#ifdef _MSC_VER
      // microsoft compiler airo
      unsigned long (SimGui::Control::* fp)(SimObject *, unsigned long) = 
         SimGui::Control::onMessage;
      (this->*fp)(this, message);
#else
      SimGui::Control::onMessage(this, message);
#endif
   }
}   

const char *FGUniversalButton::getScriptValue()
{
   if (getRadioTag() || mbSwitch)
   {
      return (mbSet ? "TRUE" : "FALSE");
   }
   else return "FALSE";
}

void FGUniversalButton::setScriptValue(const char *value)
{
   if (getRadioTag() || mbSwitch)
   {
      if (! stricmp(value, "TRUE"))
      {
         mbSet = TRUE;
      }
      else
      {
         mbSet = FALSE;
      }
   }
}

void FGUniversalButton::onPreRender(void)
{
   if (mbMirrorConsoleVar && mbSwitch)
   {
      const char *myVar = getVariable();
      if ((! myVar) || (! stricmp(myVar, "FALSE"))) mbSet = FALSE;
      else mbSet = TRUE;
   }
   
   if (mbFlashing) setUpdate();
}

void FGUniversalButton::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   bool background = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   
   if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      background = TRUE;
   }
   
   //find the bitmap
   GFXBitmap *bmp;
   
   if (background)
   {
      if (mbSet)
      {
         bmp = mBitmaps[BMP_BackgroundOn];
         if (! bmp) bmp = mBitmaps[BMP_BackgroundOff];
      }
      else bmp = mBitmaps[BMP_BackgroundOff];
      if (! bmp) bmp = mBitmaps[BMP_Ghosted];
   }
   else if (! active)
   {
      bmp = mBitmaps[BMP_Ghosted];
      if (! bmp) bmp = mBitmaps[BMP_BackgroundOff];
   }
   //else if (getRadioTag() && mbSwitch && mbSet && (! stateDepressed)) bmp = mBitmaps[BMP_Standard];
   else if (getRadioTag() && mbSwitch && mbSet && (! stateDepressed) && (currentVersion >= 2)) bmp = mBitmaps[BMP_Standard];
   else if (mbSet || stateDepressed) bmp = mBitmaps[BMP_Pressed];
   else
   {
      if (mbFlashing)
      {
         SimTime curTime = GetTickCount();
         if (mFlashTime <= 0) mFlashTime = curTime;
         SimTime timeElapsed = curTime - mFlashTime;
         bmp = ((int(timeElapsed) >> 9) % 2 ? mBitmaps[BMP_Standard] : mBitmaps[BMP_Pressed]);
      }
      else
      {
         bmp = mBitmaps[BMP_Standard];
      }
   }
   if (! bmp) bmp = mBitmaps[BMP_Standard];
   
   //draw in the button
   if (bmp) sfc->drawBitmap2d(bmp, &offset);
   
   //if its a radio switch
   if (getRadioTag() && mbSwitch && mbSet && bool(mRadioCheckBMP) && (currentVersion >= 2))
   {
      bmp = NULL;
      if (background || (! active)) bmp = mRadioCheckGhostBMP;
      if (! bmp) bmp = mRadioCheckBMP;
      if (bmp) sfc->drawBitmap2d(bmp, &Point2I(offset.x + 1, offset.y + 0));
   }
}

void FGUniversalButton::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   
   insp->write(IDITG_IS_A_SWITCH, mbSwitch);
   insp->write(IDSTR_RADIO_SET, true, "IDCTG_*", mRadioTag);
   insp->write(IDITG_BMP_ROOT_TAG, mBitmapRootName);
   insp->write(IDITG_BMP_ROOT_TAG, false, IDBMP_BEG_SWITCH, IDBMP_END_SWITCH, mBitmapTag);
   insp->write(IDITG_LR_POSITION, mLoResPosition);
   insp->write(IDSTR_AI_ACTIVE, mbSet);
   insp->write(IDITG_MIRROR_CONS_VAR, mbMirrorConsoleVar);
}

void FGUniversalButton::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read(IDITG_IS_A_SWITCH, mbSwitch);
   insp->read(IDSTR_RADIO_SET, NULL, NULL, mRadioTag);
   insp->read(IDITG_BMP_ROOT_TAG, mBitmapRootName);
   insp->read(IDITG_BMP_ROOT_TAG, NULL, NULL, NULL, mBitmapTag);
   insp->read(IDITG_LR_POSITION, mLoResPosition);
   insp->read(IDSTR_AI_ACTIVE, mbSet);
   insp->read(IDITG_MIRROR_CONS_VAR, mbMirrorConsoleVar);
   
   setBitmaps();
   
   //set the extent
   int idx;
   for (idx = 0; idx < BMP_Count; idx++)
   {
      if (bool(mBitmaps[idx])) break;
   }
   if (idx < BMP_Count)
   {
      extent.x = mBitmaps[idx]->getWidth();
      extent.y = mBitmaps[idx]->getHeight();
   }
}   

Persistent::Base::Error FGUniversalButton::write( StreamIO &sio, int version, int user )
{
   sio.write(gButtonVersion);
   sio.write(mRadioTag);
   
   BYTE len = strlen(mBitmapRootName);
   sio.write(len);
   if(len)
      sio.write(len, mBitmapRootName);
      
   mLoResPosition.write(sio);
   
   sio.write(mbSet);
   sio.write(mbSwitch);
   
   sio.write(mbMirrorConsoleVar);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FGUniversalButton::read( StreamIO &sio, int version, int user)
{
   sio.read(&currentVersion);
   sio.read(&mRadioTag);
   
   if (currentVersion >= 2)
   {
      BYTE len;
      sio.read(&len);
      if(len)
      {
         sio.read(len, mBitmapRootName);
      }
      mBitmapRootName[len] = 0;
   }
   else
   {
      sio.read(&mBitmapTag);
   }
   
   mLoResPosition.read(sio);
   
   sio.read(&mbSet);
   
   if (currentVersion >= 1) sio.read(&mbSwitch);
   else mbSwitch = TRUE;
   
   if (currentVersion >= 3) sio.read(&mbMirrorConsoleVar);
   else mbMirrorConsoleVar = FALSE;
   
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG(FGUniversalButton, FOURCC('F','G','u','b'));

};