#include "FearGuiCmdView.h"
#include "g_bitmap.h"
#include "g_pal.h"
#include "g_surfac.h"
#include "editor.strings.h"
#include "fear.strings.h"
#include "fearGuiShellPal.h"
#include "g_barray.h"
#include "simResource.h"
#include "console.h"
#include "fearPlayerPSC.h"
#include "fearGlobals.h"

namespace FearGui
{

void FGCommandView::onPreRender()
{
   SimCameraQuery query;
   if(cg.psc)
   {
      cg.psc->processQuery(&query);
      alphaBlend = query.cameraInfo.alphaBlend;
      alphaColor = query.cameraInfo.alphaColor;
      if(alphaBlend != 0 || prevAlphaBlend != 0)
         setUpdate();
   }
   else
   {
      alphaBlend = 0;
      alphaColor.set(0, 0, 0);
      if(prevAlphaBlend)
         setUpdate();
   }
   prevAlphaBlend = alphaBlend;
}

void FGCommandView::onRender(GFXSurface* sfc, Point2I offset, const Box2I& updateRect)
{
   sfc->setSurfaceAlphaBlend(&alphaColor, alphaBlend);

   mLowRes = false;
   Resource<GFXBitmapArray> resBma = bma;

   // paint the background
   sfc->drawRect2d_f(&RectI(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1), 0);
   
   //draw the children first
   renderChildControls(sfc, offset, updateRect);
   
   //reset the clip rect
   RectI clipRect(updateRect.fMin, updateRect.fMax);
   sfc->setClipRect(&clipRect);
   
   GFXBitmap *bmp;
   int stretch;
   Point2I bmpOffset = offset;
   
   //draw all the bitmaps starting with the top left counter clockwise
   bmp = resBma->getBitmap(BMP_TopLeft);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.y += bmp->getHeight();
   
   bmp = resBma->getBitmap(BMP_TopLeft2);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.y += bmp->getHeight();
   
   stretch = extent.y - (bmpOffset.y - offset.y) - resBma->getBitmap(BMP_BottomLeft)->getHeight();
   bmp = resBma->getBitmap(BMP_StretchLeft);
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(bmp->getWidth(), stretch));
   bmpOffset.y += stretch;
   
   bmp = resBma->getBitmap(BMP_BottomLeft);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   bmp = resBma->getBitmap(BMP_BottomLeft2);
   bmpOffset.y = offset.y + extent.y - bmp->getHeight();
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   bmp = resBma->getBitmap(BMP_BottomEdge);
   bmpOffset.y = offset.y + extent.y - bmp->getHeight();
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   stretch = extent.x - (bmpOffset.x - offset.x) - resBma->getBitmap(BMP_BottomRight2)->getWidth() -
                                                   resBma->getBitmap(BMP_BottomRight)->getWidth();
   bmp = resBma->getBitmap(BMP_StretchBottom);
   bmpOffset.y = offset.y + extent.y - bmp->getHeight();
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(stretch, bmp->getHeight()));
   bmpOffset.x += stretch;
   
   bmp = resBma->getBitmap(BMP_BottomRight2);
   bmpOffset.y = offset.y + extent.y - bmp->getHeight();
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   bmp = resBma->getBitmap(BMP_BottomRight);
   bmpOffset.y = offset.y + extent.y - bmp->getHeight();
   sfc->drawBitmap2d(bmp, &bmpOffset);
   
   //now draw from top left to bottom right clockwise
   bmpOffset.set(offset.x + 224, offset.y);
   bmp = resBma->getBitmap(BMP_TopEdge1);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   stretch = extent.x - (bmpOffset.x - offset.x) - resBma->getBitmap(BMP_TopEdge2)->getWidth() -
                                                   44 - //space for the cancel button
                                                   resBma->getBitmap(BMP_TopRight)->getWidth();
   bmp = resBma->getBitmap(BMP_StretchTop);
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(stretch, bmp->getHeight()));
   bmpOffset.x += stretch;
   
   bmp = resBma->getBitmap(BMP_TopEdge2);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   //space for the cancel button
   bmpOffset.x += 44;
   
   
   bmp = resBma->getBitmap(BMP_TopRight);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.y += bmp->getHeight();
   
   stretch = extent.y - (bmpOffset.y - offset.y) - resBma->getBitmap(BMP_RightEdge)->getHeight() -
                                                   resBma->getBitmap(BMP_BottomRight)->getHeight();
   bmp = resBma->getBitmap(BMP_StretchRight);
   bmpOffset.x = offset.x + extent.x - bmp->getWidth();
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(bmp->getWidth(), stretch));
   bmpOffset.y += stretch;
   
   bmp = resBma->getBitmap(BMP_RightEdge);
   bmpOffset.x = offset.x + extent.x - bmp->getWidth();
   sfc->drawBitmap2d(bmp, &bmpOffset);
}

void FGCommandView::onWake(void)
{
   position.set(0, 0);
   prevAlphaBlend = alphaBlend = 0;
   
	horizSizing = horizResizeWidth;
	vertSizing = vertResizeHeight;
}

bool FGCommandView::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //load the bitmaps
   pbaTag = IDPBA_COMMAND_VIEW;
   lr_pbaTag = IDPBA_LR_COMMAND_VIEW;
   loadBitmapArray();
   
   mLowRes = FALSE;
   
   return true;
}

void FGCommandView::loadBitmapArray()
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
/*   
   pbaName = SimTagDictionary::getString(manager, lr_pbaTag);
   AssertFatal(pbaName, "Invalid PBA name");
   lr_bma = SimResource::get(manager)->load(pbaName, true);
   AssertFatal((bool)bma, "Unable to load bma");
   for (int i = 0; i < BmpCount; i++)
   {
      GFXBitmap *bmp = lr_bma->getBitmap(i);
      bmp->attribute |= BMA_TRANSPARENT;
   }
*/
}

IMPLEMENT_PERSISTENT_TAG( FearGui::FGCommandView,   FOURCC('F','G','c','v') );

};