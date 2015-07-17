//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "g_surfac.h"
#include "g_bitmap.h"
#include "simResource.h"
#include "fear.strings.h"
#include "fearGuiShellPal.h"
#include "FearGuiScrollCtrl.h"

namespace FearGui
{

// the function of the scroll content control class
// is to notify the parent class that children have resized.
// basically it just calls it's parent (enclosing) control's
// childResized method which turns around and computes new sizes
// for the scroll bars

static const Int32 gCtrlVersion = 0;

static const char *gBitmapNames[FearGuiScrollCtrl::BMP_Count] =
{
   "POP_TopLeft_DF.BMP",
   "POP_TopEdge_DF.BMP",
   "POP_TopRight_DF.BMP",
   "POP_RightEdge_DF.BMP",
   "POP_BottomRight_DF.BMP",
   "POP_BottomEdge_DF.BMP",
   "POP_BottomLeft_DF.BMP",
   "POP_LeftEdge_DF.BMP",
};

bool FearGuiScrollCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   //load the bitmaps
   for (int i = 0; i < BMP_Count; i++)
   {
      mBitmaps[i] = SimResource::get(manager)->load(gBitmapNames[i]);
      if (bool(mBitmaps[i])) mBitmaps[i]->attribute |= BMA_TRANSPARENT;
      else AssertFatal(0, "Unable to load bitmap.");
   }
   
   return true;
}

Int32 FearGuiScrollCtrl::getMouseCursorTag(void)
{
   if (! root) return 0;
   Point2I cursorPos = root->getCursorPos();
   
   // handle state depressed
   if (isDepressed())
   {
      switch (getCurHitRegion())
      {
         case SimGui::ScrollCtrl::VertThumb:
         case SimGui::ScrollCtrl::HorizThumb:
            return IDBMP_CURSOR_GRAB;
         
         default:
            return IDBMP_CURSOR_HAND;
      }
   }
   
   switch (findHitRegion(globalToLocalCoord(cursorPos)))
   {
      case SimGui::ScrollCtrl::VertThumb:
      case SimGui::ScrollCtrl::HorizThumb:
         return IDBMP_CURSOR_OPENHAND;
         
      default:
         return IDBMP_CURSOR_HAND;
   }
}

void FearGuiScrollCtrl::drawVScrollBar(GFXSurface *sfc, const Point2I &offset)
{
   bool prevState = vBarEnabled;
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(root->findDialogNumber(this) + 1);
   if (topDialog && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      ghosted = TRUE;
   }
   if (ghosted) vBarEnabled = FALSE;
   Parent::drawVScrollBar(sfc, offset);
   vBarEnabled = prevState;
}

void FearGuiScrollCtrl::drawHScrollBar(GFXSurface *sfc, const Point2I &offset)
{
   bool prevState = hBarEnabled;
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(root->findDialogNumber(this) + 1);
   if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      ghosted = TRUE;
   }
   if (ghosted) hBarEnabled = FALSE;
   Parent::drawHScrollBar(sfc, offset);
   hBarEnabled = prevState;
}

void FearGuiScrollCtrl::drawBorder(GFXSurface *sfc, const Point2I &offset)
{
   bool ghosted = disabled;
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(root->findDialogNumber(this) + 1);
      if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
      {
         ghosted = TRUE;
      }
   }
   
   if (borderThickness > 1)
   {
      if (mbOpaque)
      {
         sfc->drawRect2d_f(&RectI(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1), fillColor);
      }
      
      int colorTable[8] =
      {
      	BOX_INSIDE,
      	BOX_OUTSIDE,
      	BOX_LAST_PIX,
      	GREEN_78,
      	BOX_GHOST_INSIDE,
      	BOX_GHOST_OUTSIDE,
      	BOX_GHOST_LAST_PIX,
      	BOX_GHOST_INSIDE,   
      };
      
      int colorOffset = (ghosted ? 4 : 0);
      Point2I tl = offset;
      Point2I br(offset.x + extent.x - 1, offset.y + extent.y - 1);
      int spc = 4;
      
      //top edge
      sfc->drawLine2d(&Point2I(tl.x + spc, tl.y),
                      &Point2I(br.x - spc, tl.y),
                      colorTable[0 + colorOffset]);
      sfc->drawLine2d(&Point2I(tl.x + spc, tl.y + 1),
                      &Point2I(br.x - spc, tl.y + 1),
                      colorTable[1 + colorOffset]);
      sfc->drawLine2d(&Point2I(tl.x + spc, tl.y + 2),
                      &Point2I(br.x - spc, tl.y + 2),
                      BLACK);
      sfc->drawPoint2d(&Point2I(tl.x + spc - 1, tl.y + 1), colorTable[0 + colorOffset]);
      sfc->drawPoint2d(&Point2I(br.x - spc + 1, tl.y + 1), colorTable[0 + colorOffset]);
      
      //bottom edge
      sfc->drawLine2d(&Point2I(tl.x + spc, br.y),
                      &Point2I(br.x - spc, br.y),
                      colorTable[0 + colorOffset]);
      sfc->drawLine2d(&Point2I(tl.x + spc, br.y - 1),
                      &Point2I(br.x - spc, br.y - 1),
                      colorTable[1 + colorOffset]);
      sfc->drawLine2d(&Point2I(tl.x + spc, br.y - 2),
                      &Point2I(br.x - spc, br.y - 2),
                      BLACK);
      sfc->drawPoint2d(&Point2I(tl.x + spc - 1, br.y - 1), colorTable[0 + colorOffset]);
      sfc->drawPoint2d(&Point2I(br.x - spc + 1, br.y - 1), colorTable[0 + colorOffset]);
                      
      //left edge                
      sfc->drawLine2d(&Point2I(tl.x, tl.y + spc),
                      &Point2I(tl.x, br.y - spc),
                      colorTable[0 + colorOffset]);
      sfc->drawLine2d(&Point2I(tl.x + 1, tl.y + spc),
                      &Point2I(tl.x + 1, br.y - spc),
                      colorTable[1 + colorOffset]);
      sfc->drawLine2d(&Point2I(tl.x + 2, tl.y + spc),
                      &Point2I(tl.x + 2, br.y - spc),
                      BLACK);
      sfc->drawPoint2d(&Point2I(tl.x + 1, tl.y + spc - 1), colorTable[0 + colorOffset]);
      sfc->drawPoint2d(&Point2I(tl.x + 1, br.y - spc + 1), colorTable[0 + colorOffset]);
                      
      //right edge                
      sfc->drawLine2d(&Point2I(br.x, tl.y + spc),
                      &Point2I(br.x, br.y - spc),
                      colorTable[0 + colorOffset]);
      sfc->drawLine2d(&Point2I(br.x - 1, tl.y + spc),
                      &Point2I(br.x - 1, br.y - spc),
                      colorTable[1 + colorOffset]);
      sfc->drawLine2d(&Point2I(br.x - 2, tl.y + spc),
                      &Point2I(br.x - 2, br.y - spc),
                      BLACK);
      sfc->drawPoint2d(&Point2I(br.x - 1, tl.y + spc - 1), colorTable[0 + colorOffset]);
      sfc->drawPoint2d(&Point2I(br.x - 1, br.y - spc + 1), colorTable[0 + colorOffset]);
      
      //inside box
      tl += spc - 1;
      br -= spc - 1;
      sfc->drawRect2d(&RectI(tl.x, tl.y, br.x, br.y), colorTable[2 + colorOffset]);
      tl += 1;
      br -= 1;
      sfc->drawRect2d(&RectI(tl.x, tl.y, br.x, br.y), colorTable[3 + colorOffset]);
      tl += 1;
      br -= 1;
      sfc->drawRect2d(&RectI(tl.x, tl.y, br.x, br.y), BLACK);
   }
   else
   {
      bool prevDisabled = disabled;
      disabled = ghosted;
      Parent::drawBorder(sfc, offset);
      disabled = prevDisabled;
   }
}   

void FearGuiScrollCtrl::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   
   insp->write(IDITG_BORDER_WIDTH, borderThickness);
}

void FearGuiScrollCtrl::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read(IDITG_BORDER_WIDTH, borderThickness);
}   

Persistent::Base::Error FearGuiScrollCtrl::write( StreamIO &sio, int version, int user )
{
   sio.write(gCtrlVersion);
   sio.write(borderThickness);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FearGuiScrollCtrl::read( StreamIO &sio, int version, int user)
{
   Int32 currentVersion;
   sio.read(&currentVersion);
   sio.read(&borderThickness);
   
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG(FearGuiScrollCtrl, FOURCC('F','G','s','l'));

};
