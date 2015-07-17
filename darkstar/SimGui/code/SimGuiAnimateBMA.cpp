//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <simResource.h>
#include <string.h>
#include <g_surfac.h>
#include <g_barray.h>
#include <g_bitmap.h>
#include "SimGuiAnimateBMA.h"
#include "editor.strings.h"
#include "darkstar.strings.h"

namespace SimGui
{
                             
//------------------------------------------------------------------------------
AnimateBMA::AnimateBMA()
{
   lastFrameTime;
   timePerFrame = 0.3f;
   frame = 0;
   bmaTag = 0;
   sleepBetweenLoop = 0;
   looping = true;
   animating = true;
}

//------------------------------------------------------------------------------
void AnimateBMA::doNotLoop()
{
   looping = false;
   animating = false;   
}  
 
//------------------------------------------------------------------------------
void AnimateBMA::startAnim()
{
   animating = true;   
   frame = 0;
}  
 
//------------------------------------------------------------------------------
bool AnimateBMA::onAdd()
{
   if (!Parent::onAdd()) return (false);

   lastFrameTime = manager->getCurrentTime();
   loadBma(bmaTag);
   if (!looping)
      doNotLoop();

   return (true);
}  

//------------------------------------------------------------------------------
void AnimateBMA::loadBma(Int32 _bmaTag)
{
   bmaTag = _bmaTag;
   hBma = SimResource::loadByTag(manager, bmaTag, true);
   if ((bool)hBma)
   {
      for (int i = 0; i < hBma->numBitmaps; i++)
         hBma->getBitmap(i)->attribute |= BMA_TRANSPARENT;
      extent.set(hBma->getBitmap(0)->width, hBma->getBitmap(0)->height);
   }
}

//------------------------------------------------------------------------------
void AnimateBMA::onPreRender()
{
   if (!isVisible() || !(bool)hBma || (!looping && !animating))
      return;

   SimTime deltaTime = manager->getCurrentTime();
   deltaTime -= lastFrameTime;
   if (deltaTime > timePerFrame)
   {
      frame++;
      
      if (frame >= hBma->getCount() + sleepBetweenLoop)
      {
         frame = 0;
         if (!looping)
         {
            animating = false;
            setVisible(false);
         }
      }

      lastFrameTime = manager->getCurrentTime();
      setUpdate();
   }
}  

//------------------------------------------------------------------------------
void AnimateBMA::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   if (!(bool)hBma)
      return;

   if (frame < hBma->getCount())
      sfc->drawBitmap2d(hBma->getBitmap(frame), &offset);
   else
      sfc->drawBitmap2d(hBma->getBitmap(hBma->getCount() - 1), &offset);
}

//------------------------------------------------------------------------------
void AnimateBMA::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);

   insp->read(IDITG_PBA_TAG, NULL, NULL   , (Int32)bmaTag);
   insp->read(IDITG_BMA_FRAME_TIME        , timePerFrame);
   insp->read(IDITG_BMA_CONTINUOUS_PLAY   , looping);
   insp->read(IDITG_BMA_LOOP_WAIT_TIME    , sleepBetweenLoop);

   loadBma(bmaTag);  
}   

//------------------------------------------------------------------------------
void AnimateBMA::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);

   insp->write(IDITG_PBA_TAG, true, "IDPBA*" , (Int32)bmaTag);
   insp->write(IDITG_BMA_FRAME_TIME			   , timePerFrame);
   insp->write(IDITG_BMA_CONTINUOUS_PLAY     , looping);
   insp->write(IDITG_BMA_LOOP_WAIT_TIME	   , sleepBetweenLoop);
} 

//------------------------------------------------------------------------------
Persistent::Base::Error AnimateBMA::write( StreamIO &sio, int version, int user )
{
   Int32 dummy;
   
   //write the bma array tag
   sio.write(bmaTag);
   
   //write the time Per frame
   sio.write(timePerFrame);
   
   //write the looping flag
   dummy = !looping;
   sio.write(dummy);
   
   //write the time between loops
   sio.write(sleepBetweenLoop);
   
   //space
   sio.write(dummy);
   sio.write(dummy);
   sio.write(dummy);
   sio.write(dummy);
   sio.write(dummy);
   sio.write(dummy);
   sio.write(dummy);
   sio.write(dummy);
   
   return Parent::write(sio, version, user);
}

//------------------------------------------------------------------------------
Persistent::Base::Error AnimateBMA::read( StreamIO &sio, int version, int user)
{
   Int32 dummy;
   
   //read the bma array tag
   sio.read(&bmaTag);
   
   //read the time Per frame
   sio.read(&timePerFrame);
   
   //read the looping flag
   sio.read(&dummy);
   looping = !dummy;
   
   //read the time between loops
   sio.read(&sleepBetweenLoop);
   
   sio.read(&dummy);
   sio.read(&dummy);
   sio.read(&dummy);
   sio.read(&dummy);
   sio.read(&dummy);
   sio.read(&dummy);
   sio.read(&dummy);
   sio.read(&dummy);

   return Parent::read(sio, version, user);
}

};
