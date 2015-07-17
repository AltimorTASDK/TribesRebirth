#include <simguitextwrap.h>
#include <simguibullettextwrap.h>

#include <g_surfac.h>
#include <editor.strings.h>

namespace SimGui
{
BulletTextWrap::BulletTextWrap()
{
   iBulletType    = BULLET_TYPE_CIRCLE;
   iBulletSize    =  6;
   iBulletSpacing = 10;

   inset.x += iBulletSize + iBulletSpacing;
}

BulletTextWrap::~BulletTextWrap()
{
}

Point2I BulletTextWrap::getExtent()
{
   Point2I tot_extent = Parent::getExtent();

   tot_extent.x += iBulletSpacing + iBulletSize;

   return (tot_extent);
}

void BulletTextWrap::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   Point2I  drawPt(offset);
   Int32    iFontHeight, iTextWidth;
   Int32    iHalfSize, iHalfExtent;
   char     szBuf[256];
   GFXFont *gfxFont = (enabled) ? hFont : hFontDisabled;

   // Make sure we have all that we need
   if (!gfxFont || !text || !startLineOffset.size())
   {
      return;
   }

   // Get some info
   iFontHeight = gfxFont->getHeight();
   iHalfSize   = (iBulletSize               / 2);
   iHalfExtent = (gfxFont->fi.baseline + 6) / 2;

   // Get a draw point for our bullet
   drawPt.y += inset.y;
   drawPt.y -= (iFontHeight - gfxFont->fi.baseline + textVPosDelta);
   drawPt.x += (inset.x - iBulletSize);

   // Draw the bullet, centered in Y, in place on X
   if (iBulletType == BULLET_TYPE_SQUARE)
   {
      sfc->drawRect2d_f(
         &RectI(drawPt.x, 
                drawPt.y + (iHalfExtent - iHalfSize) + 1, 
                drawPt.x + iBulletSize, 
                drawPt.y + (iHalfExtent + iHalfSize) + 1), 
                gfxFont->getForeColor());
   }
   else if (iBulletType == BULLET_TYPE_CIRCLE)
   {
      sfc->drawCircle2d_f(
         &RectI(drawPt.x, 
                drawPt.y + (iHalfExtent - iHalfSize) + 1, 
                drawPt.x + iBulletSize, 
                drawPt.y + (iHalfExtent + iHalfSize) + 1), 
                gfxFont->getForeColor());
   }

   // Skip past the bullet, and add some spacing
   offset.x += iBulletSize + iBulletSpacing;

   // Wrap the text
   for (int i = 0; i < startLineOffset.size(); i ++)
   {
      drawPt.x = offset.x + inset.x;

      strncpy(szBuf, &text[startLineOffset[i]], lineLen[i]);
      szBuf[lineLen[i]] = '\0';
   
      sfc->drawText_p(gfxFont, &drawPt, szBuf);
      drawPt.y += (iFontHeight + lineSpacing);
   }
}

void BulletTextWrap::inspectRead(Inspect *inspect)
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

void BulletTextWrap::inspectWrite(Inspect *inspect)
{
   Parent::inspectWrite(inspect);

   inspect->write(IDITG_BULLET_TYPE_TAG,    iBulletType);
   inspect->write(IDITG_BULLET_SIZE_TAG,    iBulletSize);
   inspect->write(IDITG_BULLET_SPACING_TAG, iBulletSpacing);
}

void BulletTextWrap::setBulletSize(int iSize)
{
   iBulletSize = iSize;
}

void BulletTextWrap::setBulletSpacing(int iSpacing)
{
   iBulletSpacing = iSpacing;
}

Int32 BulletTextWrap::getBulletSize()
{
   return (iBulletSize);
}

Int32 BulletTextWrap::getBulletSpacing()
{
   return (iBulletSpacing);
}

Persistent::Base::Error BulletTextWrap::read(StreamIO &sio, int iVer, int iUsr)
{
   sio.read(&iBulletType);
   sio.read(&iBulletSize);
   sio.read(&iBulletSpacing);

   return (Parent::read(sio, iVer, iUsr));
}

Persistent::Base::Error BulletTextWrap::write(StreamIO &sio, int iVer, int iUsr)
{
   sio.write(iBulletType);
   sio.write(iBulletSize);
   sio.write(iBulletSpacing);

   return (Parent::write(sio, iVer, iUsr));
}
};