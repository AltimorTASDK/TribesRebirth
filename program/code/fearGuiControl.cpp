#include <g_surfac.h>
#include <simResource.h>
#include <fear.strings.h>
#include <fearGuiControl.h>

namespace FearGui
{

void FGControl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      ghosted = TRUE;
   }
   
   //if opaque, fill the update rect with the fill color
   if (mbOpaque)
   {
      sfc->drawRect2d_f(&RectI(offset, Point2I(offset.x + extent.x - 1, offset.y + extent.y - 1)),
                        (! ghosted ? fillColor : ghostFillColor));
   }
   
   //if there's a boarder, draw the boarder
   if (mbBoarder)
   {
      sfc->drawRect2d(&RectI(offset, Point2I(offset.x + extent.x - 1, offset.y + extent.y - 1)),
                        (! ghosted ? boarderColor : ghostBoarderColor));
   }
   
   renderChildControls(sfc, offset, updateRect);
}

IMPLEMENT_PERSISTENT_TAG(FearGui::FGControl, FOURCC('F','G','k','t'));

};