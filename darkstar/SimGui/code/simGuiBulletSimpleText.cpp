#include <simguisimpletext.h>
#include <simguibulletsimpletext.h>

#include <g_surfac.h>
#include <editor.strings.h>

namespace SimGui
{
BulletSimpleText::BulletSimpleText()
{
   iBulletType    = BULLET_TYPE_CIRCLE;
   iBulletSize    =  6;
   iBulletSpacing = 10;

   extent.x += iBulletSize + iBulletSpacing;
}

BulletSimpleText::~BulletSimpleText()
{
}

Point2I BulletSimpleText::getExtent()
{
   Point2I tot_extent = Parent::getExtent();

   tot_extent.x += iBulletSpacing + iBulletSize;

   return (tot_extent);
}

void BulletSimpleText::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   Point2I global;
   Int32   iHalfSize, iHalfExtent;

   if ((bool)hFont)
   {
      iHalfSize   = (iBulletSize             / 2);
      iHalfExtent = (hFont->fi.baseline + 6) / 2;

      // Draw the bullet, centered in Y, in place on X
      if (iBulletType == BULLET_TYPE_SQUARE)
      {
         sfc->drawRect2d_f(
            &RectI(offset.x, 
                   offset.y + (iHalfExtent - iHalfSize) + 1, 
                   offset.x + iBulletSize, 
                   offset.y + (iHalfExtent + iHalfSize) + 1), 
                   hFont->getForeColor());
      }
      else if (iBulletType == BULLET_TYPE_CIRCLE)
      {
         sfc->drawCircle2d_f(
            &RectI(offset.x, 
                   offset.y + (iHalfExtent - iHalfSize) + 1, 
                   offset.x + iBulletSize, 
                   offset.y + (iHalfExtent + iHalfSize) + 1), 
                   hFont->getForeColor());
      }

     global = localToGlobalCoord(
        Point2I(iBulletSize + iBulletSpacing, textVPosDelta));

      sfc->drawText_p(hFont, &global, text);
   }

   renderChildControls(sfc, offset, updateRect);
}

void BulletSimpleText::inspectRead(Inspect *inspect)
{
   Int32 iTmp;

   Parent::inspectRead(inspect);

   inspect->read(IDITG_BULLET_TYPE_TAG,    iTmp);
   inspect->read(IDITG_BULLET_SIZE_TAG,    iBulletSize);
   inspect->read(IDITG_BULLET_SPACING_TAG, iBulletSpacing);

   if (iTmp >= 0 && iTmp < NUM_TYPES)
   {
      iBulletType = iTmp;
   }
}

void BulletSimpleText::inspectWrite(Inspect *inspect)
{
   Parent::inspectWrite(inspect);

   inspect->write(IDITG_BULLET_TYPE_TAG,    iBulletType);
   inspect->write(IDITG_BULLET_SIZE_TAG,    iBulletSize);
   inspect->write(IDITG_BULLET_SPACING_TAG, iBulletSpacing);
}

void BulletSimpleText::setBulletSize(int iSize)
{
   iBulletSize = iSize;
}

void BulletSimpleText::setBulletSpacing(int iSpacing)
{
   iBulletSpacing = iSpacing;
}

Int32 BulletSimpleText::getBulletSize()
{
   return (iBulletSize);
}

Int32 BulletSimpleText::getBulletSpacing()
{
   return (iBulletSpacing);
}

Persistent::Base::Error BulletSimpleText::read(StreamIO &sio, int iVer, int iUsr)
{
   sio.read(&iBulletType);
   sio.read(&iBulletSize);
   sio.read(&iBulletSpacing);

   return (Parent::read(sio, iVer, iUsr));
}

Persistent::Base::Error BulletSimpleText::write(StreamIO &sio, int iVer, int iUsr)
{
   sio.write(iBulletType);
   sio.write(iBulletSize);
   sio.write(iBulletSpacing);

   return (Parent::write(sio, iVer, iUsr));
}
};