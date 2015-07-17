//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <g_surfac.h>
#include <g_bitmap.h>
#include <g_mem.h>
#include <r_clip.h>
#include <string.h>
#include <m_box.h>
#include <simGuiTestCtrl.h>
#include <simTagDictionary.h>
#include <inspect.h>
#include <gwDeviceManager.h>

#include "rerenderCtrl.h"
#include "main.strings.h"

IMPLEMENT_PERSISTENT_TAG( RerenderCtrl,  FOURCC('s','g','r','c') );

namespace {

const Int32 sg_memSurfaceDim = 64;

};


void COREAPI RerenderCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I& /*updateRect*/)
{
   if (pMemSurface == NULL)
      return;
   pMemSurface->setPalette(sfc->getPalette());
      
   RectI drawRect(offset.x, offset.y, offset.x + extent.x, offset.y + extent.y);
   
   sfc->lock();
   sfc->draw3DBegin();
   sfc->draw3DEnd();

   pMemSurface->lock();
   pMemSurface->draw3DBegin();

   pMemSurface->setAlphaSource(GFX_ALPHA_NONE);
   pMemSurface->setShadeSource(GFX_SHADE_NONE);
   pMemSurface->setHazeSource(GFX_HAZE_NONE);
   pMemSurface->setFillMode(GFX_FILL_CONSTANT);
   pMemSurface->setFillColor(Int32(0));
   pMemSurface->setZTest(GFX_ZWRITE);
   
   // emit fill poly...
   Point3F fillPoints[4];
   fillPoints[0].set(0,                0,                0.0f);
   fillPoints[1].set(sg_memSurfaceDim, 0,                0.0f);
   fillPoints[2].set(sg_memSurfaceDim, sg_memSurfaceDim, 0.0f);
   fillPoints[3].set(0,                sg_memSurfaceDim, 0.0f);
   
   pMemSurface->addVertex(&fillPoints[0]);
   pMemSurface->addVertex(&fillPoints[1]);
   pMemSurface->addVertex(&fillPoints[2]);
   pMemSurface->addVertex(&fillPoints[3]);
   pMemSurface->emitPoly();
   
   pMemSurface->setFillColor(1);
   pMemSurface->setZTest(GFX_ZTEST_AND_WRITE);
   
   fillPoints[0].z    =
      fillPoints[1].z =
      fillPoints[2].z = 0.5f;
   
   pMemSurface->addVertex(&fillPoints[0]);
   pMemSurface->addVertex(&fillPoints[1]);
   pMemSurface->addVertex(&fillPoints[2]);
   pMemSurface->emitPoly();
   
   pMemSurface->draw3DEnd();
   pMemSurface->unlock();


   sfc->drawBitmap2d(pBitmap, &drawRect.upperL, &drawRect.lowerR);
   sfc->unlock();
}

bool COREAPI RerenderCtrl::onSimAddNotifyEvent(const SimAddNotifyEvent *event)
{
   pBitmap     = GFXBitmap::create(sg_memSurfaceDim, sg_memSurfaceDim);
   pBitmap->attribute |= BMA_TRANSPARENT;
   pMemSurface = static_cast<GFXMemSurface*>(deviceManager.getGFXDeviceManager()->createMemSurface(pBitmap));

   return Parent::onSimAddNotifyEvent(event);
}

bool COREAPI RerenderCtrl::onSimRemoveNotifyEvent(const SimRemoveNotifyEvent *event)
{
   deviceManager.getGFXDeviceManager()->freeMemSurface(pMemSurface);
   pMemSurface = NULL;
   
   delete pBitmap;
   pBitmap = NULL;

   return Parent::onSimRemoveNotifyEvent(event);
}

