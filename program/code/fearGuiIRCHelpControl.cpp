#include <fearguiirchelpcontrol.h>
#include <g_surfac.h>
#include <fear.strings.h>
#include <g_bitmap.h>
#include <simresource.h>
#include <fearguishellpal.h>

namespace FearGui
{
FGIRCHelpControl::FGIRCHelpControl()
: lpszHelpText(NULL), rsFont(NULL)
{
   format.setFont(0, IDFNT_9_STANDARD);
}

FGIRCHelpControl::~FGIRCHelpControl()
{
}

void FGIRCHelpControl::setHelpText(const char *lpszText, float, bool)
{
   Point2I     pos = root->getCursorPos();
   GFXBitmap  *bmp = root->getCursorBmp();
   GFXSurface *sfc = root->getSurface();

   if (lpszHelpText)
   {
      delete [] (char*)lpszHelpText;
      lpszHelpText = NULL;
   }

   if (lpszText)
   {
      lpszHelpText = strnew(lpszText);

      if (!bool(rsFont))
      {
         rsFont = SimResource::loadByTag(root->getManager(), IDFNT_9_STANDARD, true);
         AssertFatal(rsFont.operator bool(), "FGIRCHelpControl: cannot load font");
      }

      int iWidth  = min(rsFont->getStrWidth(lpszHelpText) + 8, (Int32)200);
      format.formatControlString(lpszHelpText, iWidth - 5);
      int iHeight = format.getHeight() + 1;
      format.resize(Point2I(0, 0), Point2I(iWidth, iHeight));

      if (bmp)
      {
         pos.y += bmp->getHeight();
      }

      if (sfc)
      {               
         if (pos.x + extent.x > (sfc->getWidth() - 3))
         {
            pos.x -= (pos.x + extent.x - (sfc->getWidth() - 3));
         }
         if (pos.y + extent.y > (sfc->getHeight() - 3))
         {
            pos.y -= (pos.y + extent.y - (sfc->getHeight() - 3));
         }
      }
      position.set(pos.x, pos.y);
      extent.set(iWidth, iHeight);
   }

   setUpdate();
}

void FGIRCHelpControl::render(GFXSurface *sfc)
{
   if (lpszHelpText)
   {
      // Opaque
      sfc->drawRect2d_f(&RectI(position, 
         Point2I(position.x + extent.x - 1, 
         position.y + extent.y - 1)), GREEN_40);

      // Border
      sfc->drawRect2d(&RectI(position, 
         Point2I(position.x + extent.x - 1, 
         position.y + extent.y - 1)), GREEN_78);

      // Text
      position.x += 4;
      position.y -= 2;
      format.onRender(sfc, position, Box2I(&position, &extent));
   }
}

};