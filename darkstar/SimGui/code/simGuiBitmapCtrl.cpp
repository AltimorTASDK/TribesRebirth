#include <g_surfac.h>
#include <string.h>
#include <m_box.h>
#include "simGuiTestCtrl.h"
#include "simGuiBitmapCtrl.h"
#include "simTagDictionary.h"
#include "inspect.h"
#include "editor.strings.h"
#include "darkstar.strings.h"
#include "simResource.h"

namespace SimGui
{

BitmapCtrl::BitmapCtrl()
{
   cBmp = NULL;
   bmpTag = IDBMP_BITMAP_DEFAULT;
   transparent = false;
   flushTextureCache = false;
   extent(64, 64);
}

void BitmapCtrl::setBitmap()
{
   delete cBmp;
   cBmp = NULL;
   setUpdate();
   
   // if no bmp to load, then bail
   if(bmpTag == 0)
      return;

   hBmp = SimResource::loadByTag(manager, bmpTag, true);

   AssertWarn((bool)hBmp, avar("SimGui::BitmapCtrl: failed to load bitmap %d", bmpTag));
   if( (bool)hBmp )
   {
       if ( (hBmp->width > 256) || (hBmp->height > 256) )
       {
          cBmp = new ChunkedBitmap(hBmp);
          flushTextureCache = true;
          hBmp.unlock();
       }
       setTransparent(transparent);
   }
}   

void BitmapCtrl::setBitmap(const char *name)
{
   delete cBmp;
   cBmp = NULL;
   setUpdate();
   
   // ignore NULL loads
   if(!name)
      return;

   hBmp = SimResource::loadByName(manager, name, true);

   AssertWarn((bool)hBmp, avar("SimGui::BitmapCtrl: failed to load bitmap %s", name));
   if( (bool)hBmp )
   {
       if ( (hBmp->width > 256) || (hBmp->height > 256) )
       {
          cBmp = new ChunkedBitmap(hBmp);
          flushTextureCache = true;
          hBmp.unlock();
       }
       setTransparent(transparent);
   }
}  

void BitmapCtrl::setBitmap(Resource<GFXBitmap> bmp)
{
   delete cBmp;
   cBmp = NULL;
   setUpdate();

   bmpTag = 0;
   hBmp = bmp;

   AssertWarn((bool)hBmp, avar("SimGui::BitmapCtrl: failed to load bitmap %d", bmpTag));
   if( (bool)hBmp )
   {
       if ( (hBmp->width > 256) || (hBmp->height > 256) )
       {
          cBmp = new ChunkedBitmap(hBmp);
          flushTextureCache = true;
          hBmp.unlock();
       }
       setTransparent(transparent);
   }
}  

void BitmapCtrl::setTransparent(bool tf)
{
   transparent = tf;

   if ((Bool)hBmp)
   {
      if (tf)
         hBmp->attribute |= BMA_TRANSPARENT;
      else
         hBmp->attribute &= ~BMA_TRANSPARENT;
   }

   if (cBmp)
      cBmp->setTransparent(tf);
}   

void BitmapCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   Box2I r(offset.x, offset.y, offset.x + extent.x, offset.y + extent.y);
   if (flushTextureCache)
   {
      sfc->flushTextureCache();
      flushTextureCache = false;
   }

   if ((bool)hBmp)
      sfc->drawBitmap2d(hBmp, &r.fMin, &extent, GFX_FLIP_NONE);
   else
      if (cBmp)
         cBmp->drawBitmap(sfc, r);

   renderChildControls(sfc, offset, updateRect);
}

void BitmapCtrl::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_BMP_TAG, true, "IDBMP_*", (Int32)bmpTag);
   insp->write(IDITG_BMP_TRANSPARENT, transparent);
}   

bool BitmapCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   setBitmap();
   return true;
}

BitmapCtrl::~BitmapCtrl()
{
   delete cBmp;
}

void BitmapCtrl::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   insp->read(IDITG_BMP_TAG, NULL, NULL, bmpTag);
   insp->read(IDITG_BMP_TRANSPARENT, transparent);

   setBitmap();
}   

Persistent::Base::Error BitmapCtrl::write( StreamIO &sio, int version, int user )
{
   Inspect_Str temp;

   sio.write(bmpTag);
   // write out space for a string... compatibility with old controls
   sio.write(Inspect::MAX_STRING_LEN + 1, temp);
   sio.write(transparent);

   return Parent::write(sio, version, user);
}

Persistent::Base::Error BitmapCtrl::read( StreamIO &sio, int version, int user)
{
   Inspect_Str temp;

   sio.read(&bmpTag);
   sio.read(Inspect::MAX_STRING_LEN + 1, temp);
   sio.read(&transparent);

   return Parent::read(sio, version, user);
}

}; //namespace GUI