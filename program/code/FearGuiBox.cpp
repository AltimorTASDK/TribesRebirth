#include "FearGuiBox.h"
#include "g_bitmap.h"
#include "g_pal.h"
#include "g_surfac.h"
#include "editor.strings.h"
#include "fear.strings.h"
#include "fearGuiShellPal.h"
#include "g_barray.h"
#include "simResource.h"

namespace FearGui
{

void FearGuiBox::onRender(GFXSurface* sfc, Point2I offset, const Box2I& updateRect)
{
   bool ghosted = mGhosted;
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(1);
      if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
      {
         ghosted = TRUE;
      }
   }
   
   if (mbOpaque)
   {
      sfc->drawRect2d_f(&RectI(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1), fillColor);
   }
   
   int colorTable[8] =
   {
   	BOX_INSIDE,
   	BOX_OUTSIDE,
   	BOX_LAST_PIX,
   	BOX_FRAME,
   	BOX_GHOST_INSIDE,
   	BOX_GHOST_OUTSIDE,
   	BOX_GHOST_LAST_PIX,
   	BOX_GHOST_FRAME,   
   };
   
   int colorOffset = (ghosted ? 4 : 0);
   Point2I tl = offset;
   Point2I br(offset.x + extent.x - 1, offset.y + extent.y - 1);
   int spc = 4;
   
   //top edge
   sfc->drawLine2d(&Point2I(tl.x + spc, tl.y),
                   &Point2I(br.x - spc, tl.y),
                   colorTable[1 + colorOffset]);
   sfc->drawLine2d(&Point2I(tl.x + spc, tl.y + 1),
                   &Point2I(br.x - spc, tl.y + 1),
                   colorTable[0 + colorOffset]);
   sfc->drawPoint2d(&Point2I(tl.x + spc - 1, tl.y + 1), colorTable[2 + colorOffset]);
   sfc->drawPoint2d(&Point2I(br.x - spc + 1, tl.y + 1), colorTable[2 + colorOffset]);
   
   //bottom edge
   sfc->drawLine2d(&Point2I(tl.x + spc, br.y - 1),
                   &Point2I(br.x - spc, br.y - 1),
                   colorTable[0 + colorOffset]);
   sfc->drawLine2d(&Point2I(tl.x + spc, br.y),
                   &Point2I(br.x - spc, br.y),
                   colorTable[1 + colorOffset]);
   sfc->drawPoint2d(&Point2I(tl.x + spc - 1, br.y - 1), colorTable[2 + colorOffset]);
   sfc->drawPoint2d(&Point2I(br.x - spc + 1, br.y - 1), colorTable[2 + colorOffset]);
                   
   //left edge                
   sfc->drawLine2d(&Point2I(tl.x, tl.y + spc),
                   &Point2I(tl.x, br.y - spc),
                   colorTable[1 + colorOffset]);
   sfc->drawLine2d(&Point2I(tl.x + 1, tl.y + spc),
                   &Point2I(tl.x + 1, br.y - spc),
                   colorTable[0 + colorOffset]);
   sfc->drawPoint2d(&Point2I(tl.x + 1, tl.y + spc - 1), colorTable[2 + colorOffset]);
   sfc->drawPoint2d(&Point2I(tl.x + 1, br.y - spc + 1), colorTable[2 + colorOffset]);
                   
   //right edge                
   sfc->drawLine2d(&Point2I(br.x, tl.y + spc),
                   &Point2I(br.x, br.y - spc),
                   colorTable[1 + colorOffset]);
   sfc->drawLine2d(&Point2I(br.x - 1, tl.y + spc),
                   &Point2I(br.x - 1, br.y - spc),
                   colorTable[0 + colorOffset]);
   sfc->drawPoint2d(&Point2I(br.x - 1, tl.y + spc - 1), colorTable[2 + colorOffset]);
   sfc->drawPoint2d(&Point2I(br.x - 1, br.y - spc + 1), colorTable[2 + colorOffset]);
   
   //inside box
   tl += spc - 1;
   br -= spc - 1;
   sfc->drawRect2d(&RectI(tl.x, tl.y, br.x, br.y), colorTable[3 + colorOffset]);
   
   //draw any chil'en
   renderChildControls(sfc, offset, updateRect);
}

bool FearGuiBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //set the ghosted flag
   mGhosted = FALSE;
      
   return true;
}

IMPLEMENT_PERSISTENT_TAG( FearGui::FearGuiBox,   FOURCC('F','G','b','x') );

};