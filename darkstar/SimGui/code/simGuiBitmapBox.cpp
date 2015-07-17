
#include "simGuiBitmapBox.h"
#include "g_surfac.h"
#include "simTagDictionary.h"
#include "r_clip.h"
#include "editor.strings.h"
#include "darkstar.strings.h"
#include "simResource.h"

namespace SimGui
{

BitmapBox::BitmapBox()
{
   bmaTag = IDPBA_BOX_DEFAULT;
   extent.set(100, 100);
}

void BitmapBox::setBitmapArray(Int32 tag)
{
   bma = SimResource::loadByTag(manager, tag, true);
   if((bool)bma)
   {
      bmaTag = tag;

      upperLeft = bma->getBitmap(0);
      upperLeft->attribute |= BMA_TRANSPARENT;
      upperCenter = bma->getBitmap(1);
      upperCenter->attribute |= BMA_TRANSPARENT;
      upperRight = bma->getBitmap(2);
      upperRight->attribute |= BMA_TRANSPARENT;

      centerLeft = bma->getBitmap(3);
      centerLeft->attribute |= BMA_TRANSPARENT;
      center = bma->getBitmap(4);
      centerRight = bma->getBitmap(5);
      centerRight->attribute |= BMA_TRANSPARENT;

      lowerLeft = bma->getBitmap(6);
      lowerLeft->attribute |= BMA_TRANSPARENT;
      lowerCenter = bma->getBitmap(7);
      lowerCenter->attribute |= BMA_TRANSPARENT;
      lowerRight = bma->getBitmap(8);
      lowerRight->attribute |= BMA_TRANSPARENT;
   }
}  
 
void BitmapBox::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);

   insp->write(IDITG_PBA_TAG, true, "IDPBA_BOX*", (Int32)bmaTag);
}   

void BitmapBox::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);

   Int32 tag;
   insp->read(IDITG_PBA_TAG, NULL, NULL, tag); 
   
   setBitmapArray(tag);
}   

void BitmapBox::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   if(!(bool)bma)
      return;
   // first lay down the center tiles...
   int left = (centerLeft)->getWidth();
   int right = extent.x - (centerRight)->getWidth();
   int top = (upperCenter)->getHeight();
   int bottom = extent.y - (upperCenter)->getHeight();

   RectI areaClip(left + offset.x, top + offset.y, right + offset.x, bottom + offset.y);
   RectI curClipRect(updateRect.fMin.x, updateRect.fMin.y, 
                     updateRect.fMax.x - 1, updateRect.fMax.y - 1);

   if(rectClip(&areaClip, &curClipRect))
   {
      sfc->setClipRect(&areaClip);
      for(;top < bottom; top += (center)->getHeight())
      {
         int xc;
         for(xc = left ;xc < right; xc += (center)->getWidth())
            sfc->drawBitmap2d(center, &Point2I(xc + offset.x, top + offset.y));
      }
      sfc->setClipRect(&curClipRect);
   }
   // now draw borders

   // left border
   top = (upperLeft)->getHeight();
   bottom = extent.y - (lowerLeft)->getHeight();
   for(;top < bottom; top += (centerLeft)->getHeight())
      sfc->drawBitmap2d(centerLeft, &Point2I(offset.x, top + offset.y));

   // right border
   top = (upperRight)->getHeight();
   bottom = extent.y - (lowerRight)->getHeight();

   left = extent.x - (centerRight)->getWidth() + offset.x;
   for(;top < bottom; top += (centerRight)->getHeight())
      sfc->drawBitmap2d(centerRight, &Point2I(left, top + offset.y));

   // top border

   left = (upperLeft)->getWidth();
   right = extent.x - (upperRight)->getWidth();
   
   for(;left < right; left += (upperCenter)->getWidth())
      sfc->drawBitmap2d(upperCenter, &Point2I(left + offset.x, offset.y));

   // bottom border

   left = (lowerLeft)->getWidth();
   right = extent.x - (lowerRight)->getWidth();

   top = extent.y + offset.y - (lowerCenter)->getHeight();
   for(;left < right; left += (lowerCenter)->getWidth())
      sfc->drawBitmap2d(lowerCenter, &Point2I(left + offset.x, top));

   // corners
   sfc->drawBitmap2d(upperLeft, &offset);
   sfc->drawBitmap2d(upperRight, &Point2I(offset.x + 
         extent.x - (upperRight)->getWidth(), offset.y));
   sfc->drawBitmap2d(lowerRight, &Point2I(offset.x + 
         extent.x - (lowerRight)->getWidth(), offset.y + 
         extent.y - (lowerRight)->getHeight()));
   sfc->drawBitmap2d(lowerLeft, &Point2I(offset.x, offset.y + 
         extent.y - (lowerLeft)->getHeight()));

   Parent::onRender(sfc, offset, updateRect);
}

bool BitmapBox::onAdd()
{
   if(!Parent::onAdd())
      return false;
   setBitmapArray(bmaTag);
   return true;
}

Persistent::Base::Error BitmapBox::write( StreamIO &sio, int version, int user )
{
   sio.write(bmaTag);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error BitmapBox::read( StreamIO &sio, int version, int user)
{
   sio.read(&bmaTag);
   return Parent::read(sio, version, user);
}


};