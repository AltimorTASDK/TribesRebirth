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
#include "darkstar.strings.h"
#include "fear.strings.h"
#include "editor.strings.h"
#include "FearGuiSwitch.h"
#include "simResource.h"

namespace FearGui
{

bool FearGuiSwitch::becomeFirstResponder()
{
   return false;
}

bool FearGuiSwitch::onAdd(void)
{
   if(!Parent::onAdd())
      return false;

   setBitmaps();
   
   return true;
}
   
void FearGuiSwitch::onMouseDown(const SimGui::Event &event)
{
   set = ! set;
   onAction();
   Parent::onMouseDown(event);
}

void FearGuiSwitch::onAction(void)
{
   if (targetTag > 0)
   {
      
      //find the gui to set
      SimGui::Control *ctrl = findRootControlWithTag(targetTag);
      if ((! ctrl) && root)
      {
         ctrl = root->getContentControl()->findControlWithTag(targetTag);
      }
      if (ctrl) ctrl->setVisible(set);
   }
   Parent::onAction();

}

void FearGuiSwitch::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   GFXBitmap *bmp = (set ? mOnBitmap : mOffBitmap);
   if (! bmp) return;
   
   //draw in the button
   sfc->drawBitmap2d(bmp, &offset);

   renderChildControls(sfc, offset, updateRect);
}

void FearGuiSwitch::setBitmaps(void)
{
   if (bmpOnTag > 0)
   {
      mOnBitmap = SimResource::loadByTag(manager, bmpOnTag, true);
   }
   if (bmpOffTag > 0)
   {
      mOffBitmap = SimResource::loadByTag(manager, bmpOffTag, true);
   }
   if (! (bool)mOffBitmap)
   {
      mOffBitmap = mOnBitmap;
   }
   //if (transparent)
   //{
      if ((bool)mOnBitmap) mOnBitmap->attribute |= BMA_TRANSPARENT;
      if ((bool)mOffBitmap) mOffBitmap->attribute |= BMA_TRANSPARENT;
   //}
   
   if ((bool)mOnBitmap)
   {
      extent.x = mOnBitmap->getWidth();
      extent.y = mOnBitmap->getHeight();
   }
}   

void FearGuiSwitch::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_TARGET_CONTROL_TAG, true, "*", targetTag);
   insp->write(IDITG_BMP_ROOT_TAG, false, IDBMP_BEG_SWITCH, IDBMP_END_SWITCH, bmpOnTag);
   insp->write(IDITG_OFF_BMP_TAG, false, IDBMP_BEG_SWITCH, IDBMP_END_SWITCH, bmpOffTag);
   insp->write(IDITG_BMP_TRANSPARENT, transparent);
}

void FearGuiSwitch::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   insp->read( IDITG_TARGET_CONTROL_TAG, NULL, NULL, targetTag);
   insp->read(IDITG_BMP_ROOT_TAG, NULL, NULL, NULL, bmpOnTag);
   insp->read(IDITG_OFF_BMP_TAG, NULL, NULL, NULL, bmpOffTag);
   insp->read(IDITG_BMP_TRANSPARENT, transparent);

   setBitmaps();
}   


Persistent::Base::Error FearGuiSwitch::write( StreamIO &sio, int version, int user )
{
   sio.write(bmpOnTag);
   sio.write(bmpOffTag);
   sio.write(targetTag);
   sio.write(set);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FearGuiSwitch::read( StreamIO &sio, int version, int user)
{
   sio.read(&bmpOnTag);
   sio.read(&bmpOffTag);
   sio.read(&targetTag);
   sio.read(&set);
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG(FearGuiSwitch, FOURCC('F','G','s','w'));

};