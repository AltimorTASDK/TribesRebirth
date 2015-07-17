#include "g_bitmap.h"
#include "g_pal.h"
#include "g_surfac.h"
#include "editor.strings.h"
#include "fear.strings.h"
#include "fearGuiShellPal.h"
#include "g_barray.h"
#include "simResource.h"
#include "console.h"
#include "FearGuiShellBorder.h"
#include "inspect.h"
#include "editor.strings.h"
#include "darkstar.strings.h"

namespace FearGui
{

void FGShellBorder::onPreRender(void)
{
   if (! parent) return;
   position = parent->position;
   extent = parent->extent;
}

void FGShellBorder::onRender(GFXSurface* sfc, Point2I offset, const Box2I& updateRect)
{
   //first, draw the opaque color
   
   //draw the background grid in first (centered)
   if(bool(bkgndBitmap))
   {
      bkgndCtrl.position = Point2I(0,0);
      bkgndCtrl.extent = extent;
      bkgndCtrl.onRender(sfc, offset, updateRect);
   }
   else
      sfc->drawRect2d_f(&RectI(&offset, &Point2I(offset.x + extent.x - 1, offset.y + extent.y - 1)), fillColor);
   
   //draw the children after the background grid
   renderChildControls(sfc, offset, updateRect);
   
   //reset the clip rect
   RectI clipRect(updateRect.fMin, updateRect.fMax);
   sfc->setClipRect(&clipRect);
   
   //make sure the window is at least 512x384 before drawing the border
   if (extent.x < 512 || extent.y < 384) return;
   
   GFXBitmap *bmp;
   int stretch;
   Point2I bmpOffset = offset;
   
   //make space for the 3 tab buttons
   bmpOffset.x = 224;
   
   //topEdge 
   bmp = bma->getBitmap(BMP_TopEdge);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   //StretchTop
   stretch = extent.x - bmpOffset.x -
                        bma->getBitmap(BMP_TopEdge2)->getWidth() -
                        44 - //space for the cancel button
                        bma->getBitmap(BMP_TopRight)->getWidth();
   bmp = bma->getBitmap(BMP_StretchTop);
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(stretch, bmp->getHeight()));
   bmpOffset.x += stretch;
   
   //TopEdge2
   bmp = bma->getBitmap(BMP_TopEdge2);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   //space for the cancel button
   bmpOffset.x += 44;
   
   //TopRight
   bmp = bma->getBitmap(BMP_TopRight);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.y += bmp->getHeight();
   
   //TopRight2
   bmp = bma->getBitmap(BMP_TopRight2);
   bmpOffset.x = extent.x - bmp->getWidth();
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.y += bmp->getHeight();
   
   //StretchRight
   stretch = extent.y - bmpOffset.y -
                        bma->getBitmap(BMP_BottomRight)->getHeight();
   bmp = bma->getBitmap(BMP_StretchRight);
   bmpOffset.x = extent.x - bmp->getWidth();
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(bmp->getWidth(), stretch));
   bmpOffset.y += stretch;
   
   //BottomRight
   bmp = bma->getBitmap(BMP_BottomRight);
   bmpOffset.x = extent.x - bmp->getWidth();
   sfc->drawBitmap2d(bmp, &bmpOffset);
   
   //TopLeft
   bmpOffset = offset;
   bmp = bma->getBitmap(BMP_TopLeft);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.y += bmp->getHeight();
   
   //TopLeft2
   bmp = bma->getBitmap(BMP_TopLeft2);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.y += bmp->getHeight();
   
   //StretchLeft
   stretch = extent.y - bmpOffset.y -
                        bma->getBitmap(BMP_BottomLeft)->getHeight();
   bmp = bma->getBitmap(BMP_StretchLeft);
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(bmp->getWidth(), stretch));
   bmpOffset.y += stretch;
   
   //BottomLeft
   bmp = bma->getBitmap(BMP_BottomLeft);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   //BottomLeft2
   bmp = bma->getBitmap(BMP_BottomLeft2);
   bmpOffset.y = extent.y - bmp->getHeight();
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   //StretchBottom
   stretch = extent.x - bmpOffset.x -
                        bma->getBitmap(BMP_BottomRight2)->getWidth() -
                        bma->getBitmap(BMP_BottomRight)->getWidth();
   bmp = bma->getBitmap(BMP_StretchBottom);
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(stretch, bmp->getHeight()));
   bmpOffset.x += stretch;
   
   //BottomRight2
   bmp = bma->getBitmap(BMP_BottomRight2);
   bmpOffset.y = extent.y - bmp->getHeight();
   sfc->drawBitmap2d(bmp, &bmpOffset);
}

void FGShellBorder::inspectRead(Inspect *insp)
{
   Inspect_Str str;
   Parent::inspectRead(insp);
   insp->read(IDITG_BMP_TAG, str);
   bitmapName = stringTable.insert(str);
   bkgndBitmap = SimResource::loadByName(manager, bitmapName, false);
   if (bool(bkgndBitmap)) bkgndCtrl.setBitmap(bkgndBitmap);
   else bitmapName= NULL;
}

void FGShellBorder::inspectWrite(Inspect *insp)
{
   Inspect_Str str;
   if(bitmapName)
      strcpy(str, bitmapName);
   else
      str[0] = 0;
   Parent::inspectWrite(insp);
   insp->write(IDITG_BMP_TAG, str);
}

bool FGShellBorder::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //load the bitmaps
   pbaTag = IDPBA_SHELL_BORDER;
   loadBitmapArray();
   //load the grid background
   if(bitmapName) bkgndBitmap = SimResource::loadByName(manager, bitmapName, false);
   if (bool(bkgndBitmap)) bkgndCtrl.setBitmap(bkgndBitmap);
   else bitmapName= NULL;
   return true;
}

Persistent::Base::Error FGShellBorder::read( StreamIO &sio, int version, int user)
{
   //first read in the version number
   BYTE versionByte;
   sio.read(&versionByte);
   if(versionByte == 0xFF)
      bitmapName = sio.readSTString();
   else
   {
      bitmapName = NULL;
      sio.setPosition(sio.getPosition() - 1);
   }
   return Parent::read(sio, version, user);
}

Persistent::Base::Error FGShellBorder::write( StreamIO &sio, int version, int user )
{
   Inspect_Str temp;

   sio.write(BYTE(0xFF));
   sio.writeString(bitmapName);

   return Parent::write(sio, version, user);
}

FGShellBorder::FGShellBorder()
{
   bitmapName = NULL;
}

void FGShellBorder::loadBitmapArray(void)
{
   const char *pbaName = SimTagDictionary::getString(manager, pbaTag);
   AssertFatal(pbaName, "Invalid PBA name");
   
   bma = SimResource::get(manager)->load(pbaName, true);
   AssertFatal((bool)bma, "Unable to load bma");
   
   for (int i = 0; i < BMP_Count; i++)
   {
      GFXBitmap *bmp = bma->getBitmap(i);
      bmp->attribute |= BMA_TRANSPARENT;
   }
}

IMPLEMENT_PERSISTENT_TAG( FearGui::FGShellBorder,   FOURCC('F','G','s','b') );
IMPLEMENT_PERSISTENT_TAG( FearGui::MMShellBorder,   FOURCC('M','M','s','b') );

};