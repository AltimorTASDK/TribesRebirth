//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <g_surfac.h>
#include <inspect.h>
#include "simGuiMatrixCtrl.h"
#include "simGuiArrayCtrl.h"
#include "editor.strings.h"
#include "g_bitmap.h"
#include "r_clip.h"

namespace SimGui
{

MatrixCtrl::MatrixCtrl()
{
   headerDim.set(0,0);
   borderThickness = 0;
}

void MatrixCtrl::calcScrollRects(void)
{
   if(hasHScrollBar)
   {
      leftArrowRect(borderThickness + headerDim.x, extent.y - borderThickness - scrollBarThickness,
         borderThickness + headerDim.x + scrollBarArrowBtnLength, extent.y - borderThickness);
      rightArrowRect(extent.x - borderThickness - (hasVScrollBar ? scrollBarThickness : 0) - scrollBarArrowBtnLength, extent.y - borderThickness - scrollBarThickness, 
         extent.x - borderThickness - (hasVScrollBar ? scrollBarThickness : 0), extent.y - borderThickness);
      hTrackRect(leftArrowRect.lowerR.x, leftArrowRect.upperL.y,
                     rightArrowRect.upperL.x, rightArrowRect.lowerR.y);
   }
   if(hasVScrollBar)
   {
      upArrowRect(extent.x - borderThickness - scrollBarThickness, borderThickness + headerDim.y,
         extent.x - borderThickness, borderThickness + headerDim.y + scrollBarArrowBtnLength);
      downArrowRect(extent.x - borderThickness - scrollBarThickness, extent.y - borderThickness - (hasHScrollBar ? scrollBarThickness : 0) - scrollBarArrowBtnLength,
         extent.x - borderThickness, extent.y - borderThickness - (hasHScrollBar ? scrollBarThickness : 0));
      vTrackRect(upArrowRect.upperL.x, upArrowRect.lowerR.y,
                     downArrowRect.lowerR.x, downArrowRect.upperL.y);
   }
   
   //if the child is an arrayCtrl, make sure it knows the header dimensions
   if(contentCtrl->size() > 0)
   {
   
      //set the header dimensions if required
      ArrayCtrl *mctrl = dynamic_cast<ArrayCtrl*>(contentCtrl->front());
      if (mctrl) {
         mctrl->setHeaderDim(headerDim);
      }
   }
   
}

void MatrixCtrl::drawBorder(GFXSurface *sfc, const Point2I &offset)
{
   RectI r(offset.x + headerDim.x, offset.y + headerDim.y, offset.x + extent.x - 1, offset.y + extent.y - 1);
   
   if (mbOpaque)
   {
      sfc->drawRect2d_f(&r, (! disabled ? fillColor : ghostFillColor));
   }

   if (mbBoarder)
   {
      sfc->drawRect2d(&r, (! disabled ? boarderColor : ghostBoarderColor));
   }
}   

void MatrixCtrl::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_HEADER_SIZES, headerDim);
}   
                
void MatrixCtrl::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read(IDITG_HEADER_SIZES, headerDim);
   computeSizes();
}   

Persistent::Base::Error MatrixCtrl::write( StreamIO &sio, int version, int user )
{
   headerDim.write(sio);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error MatrixCtrl::read( StreamIO &sio, int version, int user)
{
   headerDim.read(sio);
   return Parent::read(sio, version, user);
}

};
