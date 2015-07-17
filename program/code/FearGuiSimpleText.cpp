#include <g_surfac.h>
#include <string.h>
#include <m_box.h>
#include "simResource.h"
#include "fearGuiSimpleText.h"
#include "simTagDictionary.h"
#include "fear.strings.h"

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG( FGSimpleText,   FOURCC('F','G','s','t') );

bool FGSimpleText::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   //set the fonts
   setFont(IDFNT_10_HILITE);
   setFontHL(IDFNT_10_SELECTED);
   setFontDisabled(IDFNT_10_DISABLED);
   hFontShadow = SimResource::loadByTag(manager, IDFNT_10_BLACK, true);
   
   //low res
   lrFont = SimResource::get(manager)->load("sf_yellow_6.pft");
   AssertFatal(lrFont.operator bool(), "Unable to load sf_yellow_6.pft");
   lrFont->fi.flags |= FONT_LOWERCAPS;
   
   lrFontHL = SimResource::get(manager)->load("sf_white_6.pft");
   AssertFatal(lrFontHL.operator bool(), "Unable to load sf_white_6.pft");
   lrFontHL->fi.flags |= FONT_LOWERCAPS;
   
   lrFontDisabled = SimResource::get(manager)->load("sf_grey_6.pft");
   AssertFatal(lrFontDisabled.operator bool(), "Unable to load sf_grey_6.pft");
   lrFontDisabled->fi.flags |= FONT_LOWERCAPS;
   
   lrFontShadow = SimResource::get(manager)->load("sf_black_6.pft");
   AssertFatal(lrFontShadow.operator bool(), "Unable to load sf_black_6.pft");
   lrFontShadow->fi.flags |= FONT_LOWERCAPS;
   
   lrFontDisabled = SimResource::get(manager)->load("sf_grey_6.pft");
   AssertFatal(lrFontDisabled.operator bool(), "Unable to load sf_grey_6.pft");
   lrFontDisabled->fi.flags |= FONT_LOWERCAPS;
   
   mbLowRes = FALSE;
   
   extent.y = hFont->getHeight() + 4; 
   return TRUE;
}

void FGSimpleText::setText(const char *txt)
{
   //ensure the fonts are set
   setFont(IDFNT_10_HILITE);
   setFontHL(IDFNT_10_SELECTED);
   setFontDisabled(IDFNT_10_DISABLED);
   hFontShadow = SimResource::loadByTag(manager, IDFNT_10_BLACK, true);
   AssertFatal(hFontShadow.operator bool(), "Unable to load sf_black_10.pft");
   
   //low res
   lrFont = SimResource::get(manager)->load("sf_yellow_6.pft");
   AssertFatal(lrFont.operator bool(), "Unable to load sf_yellow_6.pft");
   lrFont->fi.flags |= FONT_LOWERCAPS;
   
   lrFontHL = SimResource::get(manager)->load("sf_white_6.pft");
   AssertFatal(lrFontHL.operator bool(), "Unable to load sf_white_6.pft");
   lrFontHL->fi.flags |= FONT_LOWERCAPS;
   
   lrFontDisabled = SimResource::get(manager)->load("sf_grey_6.pft");
   AssertFatal(lrFontDisabled.operator bool(), "Unable to load sf_grey_6.pft");
   lrFontDisabled->fi.flags |= FONT_LOWERCAPS;
   
   lrFontShadow = SimResource::get(manager)->load("sf_black_6.pft");
   AssertFatal(lrFontShadow.operator bool(), "Unable to load sf_black_6.pft");
   lrFontShadow->fi.flags |= FONT_LOWERCAPS;
   
   lrFontDisabled = SimResource::get(manager)->load("sf_grey_6.pft");
   AssertFatal(lrFontDisabled.operator bool(), "Unable to load sf_grey_6.pft");
   lrFontDisabled->fi.flags |= FONT_LOWERCAPS;
   
   mbLowRes = FALSE;
   
   extent.y = hFont->getHeight() + 4; 
   
   Parent::setText(txt);
   if (bool(hFont)) extent.x = hFont->getStrWidth(text) + 2;
}

void FGSimpleText::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   Point2I canvasSize(sfc->getWidth(), sfc->getHeight());
   
   //check for lowres
   if (canvasSize.x < 512 || canvasSize.y < 384)
   {
      if (! mbLowRes)
      {
         mbLowRes = TRUE;
         extent.y = lrFont->getHeight() + 2; 
         return;
      }
   }
   else
   {
      if (mbLowRes)
      {
         mbLowRes = FALSE;
         extent.y = hFont->getHeight() + 4; 
         return;
      }
   }
   
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      ghosted = TRUE;
   }
   
   GFXFont *font = (! mbLowRes ? hFontShadow : lrFontShadow);
   sfc->drawText_p(font, &Point2I(offset.x, offset.y + 1), text);
   if (ghosted) font = (! mbLowRes ? hFontDisabled : lrFontDisabled);
   else if (mbLowRes) font = (active ? lrFontHL : lrFont);
   else font = (active ? hFontHL : hFont);
   sfc->drawText_p(font, &Point2I(offset.x + 1, offset.y), text);
}

}; //namespace GUI