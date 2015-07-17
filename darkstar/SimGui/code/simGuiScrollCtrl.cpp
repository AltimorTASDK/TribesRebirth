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
#include "simGuiArrayCtrl.h"
#include "simGuiScrollCtrl.h"
#include "editor.strings.h"
#include "g_bitmap.h"
#include "r_clip.h"
#include "simResource.h"

namespace SimGui
{

void ScrollContentCtrl::childResized(Control *child)
{
   ((ScrollCtrl *) parent)->computeSizes();

   Parent::childResized(child);
}

void ScrollContentCtrl::removeObject(SimObject *object)
{
   Parent::removeObject(object);
   ScrollCtrl *sctrl = NULL;
   if(parent)
      sctrl = dynamic_cast<ScrollCtrl *>(parent);
   if(sctrl)
      sctrl->computeSizes();
}

void ScrollContentCtrl::setParentDisabled(bool disabled)
{
   ScrollCtrl *pCtrl = dynamic_cast<ScrollCtrl*>(parent);
   if (pCtrl) pCtrl->disabled = disabled;
}

void ScrollContentCtrl::setParentBoarder(bool on_off, BYTE color, BYTE selColor, BYTE disColor)
{
   if (parent)
   {
      parent->mbBoarder = on_off;
      parent->boarderColor = color;
      parent->selectBoarderColor = selColor;
      parent->ghostBoarderColor = disColor;
   }
}
   
void ScrollContentCtrl::setParentOpaque(bool on_off, BYTE color, BYTE selColor, BYTE disColor)
{
   if (parent)
   {
      parent->mbOpaque = on_off;
      parent->fillColor = color;
      parent->selectFillColor = selColor;
      parent->ghostFillColor = disColor;
   }
}

ScrollCtrl::ScrollCtrl()
{
   extent.set(200,200);
   borderThickness = 1;
   scrollBarThickness = 16;
   scrollBarArrowBtnLength = 16;
   stateDepressed = false;
   curHitRegion = None;
   disabled = FALSE;
   
   line_V = 0.1f;
   line_H = 0.1f;
   page_V = 0.2f;
   page_H = 0.2f;
   
   willFirstRespond = true;
   thumbAnchorPos = 0.0f;
   useConstantHeightThumb = false;

   forceVScrollBar = ScrollBarAlwaysOn;
   forceHScrollBar = ScrollBarAlwaysOn;

   vBarThumbPos = 0;
   hBarThumbPos = 0;

   pbaTag = IDPBA_SCROLL_DEFAULT;
   contentCtrl = NULL;
}

void ScrollCtrl::setContentBoarder(bool on_off, BYTE color, BYTE selColor, BYTE disColor)
{
   if (contentCtrl)
   {
      contentCtrl->mbBoarder = on_off;
      contentCtrl->boarderColor = color;
      contentCtrl->selectBoarderColor = selColor;
      contentCtrl->ghostBoarderColor = disColor;
   }
}
   
void ScrollCtrl::setContentOpaque(bool on_off, BYTE color, BYTE selColor, BYTE disColor)
{
   if (contentCtrl)
   {
      contentCtrl->mbOpaque = on_off;
      contentCtrl->fillColor = color;
      contentCtrl->selectFillColor = selColor;
      contentCtrl->ghostFillColor = disColor;
   }
}

void ScrollCtrl::resize(const Point2I &newPos, const Point2I &newExt)
{
   Parent::resize(newPos, newExt);
   computeSizes();
}

void ScrollCtrl::loadBitmapArray()
{
   const char *pbaName = SimTagDictionary::getString(manager, pbaTag);
   if(pbaName)
   {
      bma = SimResource::get(manager)->load(pbaName, true);
      if(bool(bma))
      {
         baseThumbSize = bma->getBitmap(BmpVerticalThumbTopCap)->getHeight() +
                        bma->getBitmap(BmpVerticalThumbBottomCap)->getHeight();
         scrollBarThickness = bma->getBitmap(BmpVerticalPage)->getWidth();
         scrollBarArrowBtnLength = bma->getBitmap(BmpUpArrow)->getHeight();
      }
   }
   computeSizes();
}

bool ScrollCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   if(!size())
   {
      ScrollContentCtrl *ctrl = new ScrollContentCtrl();
      ctrl->setSizingOptions(Control::horizResizeWidth, Control::vertResizeHeight);
      manager->addObject(ctrl);
      Control::addObject(ctrl);
   }
   contentCtrl = (ScrollContentCtrl *) front();

   loadBitmapArray();

   return true;
}

void ScrollCtrl::onWake()
{
   Parent::onWake();
   computeSizes();
}

void ScrollCtrl::editResized()
{
   computeSizes();
}

bool ScrollCtrl::calcChildExtents(Point2I *pos, Point2I *ext)
{
   SimGroup::iterator i;

   // loop through the children of this scroll view...
   if(!contentCtrl->size())
      return false;
   else
   {
      Control *ctrl = (Control *) (contentCtrl->front());
      *ext = ctrl->extent;
      *pos = ctrl->position;

      for(i = contentCtrl->begin() + 1; i != contentCtrl->end();i++)
      {
         ctrl = (Control *) (*i);
         if(ctrl->position.x < pos->x)
            pos->x = ctrl->position.x;
         if(ctrl->position.y < pos->y)
            pos->y = ctrl->position.y;
         if(ctrl->position.x + ctrl->extent.x > pos->x + ext->x)
            ext->x = ctrl->position.x + ctrl->extent.x - pos->x;
         if(ctrl->position.y + ctrl->extent.y > pos->y + ext->y)
            ext->y = ctrl->position.y + ctrl->extent.y - pos->y;
      }
   }
   return true;
}

SimObject *ScrollCtrl::addObject(SimObject *object)
{
   if(contentCtrl)
   {
      SimObject *ret = contentCtrl->addObject(object);
      computeSizes();
      return ret;
   }
   else
   {
      AssertFatal(dynamic_cast<ScrollContentCtrl *>(object) != NULL,"Yikes!!");
      return Parent::addObject(object);
   }
}

void ScrollCtrl::computeSizes()
{
   Point2I pos(borderThickness, borderThickness);
   Point2I ext(extent.x - (borderThickness << 1), extent.y - (borderThickness << 1));
   Point2I cpos;
   Point2I cext;
   hBarEnabled = false;
   vBarEnabled = false;
   hasVScrollBar = (forceVScrollBar == ScrollBarAlwaysOn);
   hasHScrollBar = (forceHScrollBar == ScrollBarAlwaysOn);

   setUpdate();

   if(calcChildExtents(&cpos, &cext))
   {
      if(hasVScrollBar)
         ext.x -= scrollBarThickness;
      if(hasHScrollBar)
         ext.y -= scrollBarThickness;

      if(cext.x > ext.x && (forceHScrollBar == ScrollBarDynamic))
      {
         hasHScrollBar = true;
         ext.y -= scrollBarThickness;
      }
      if(cext.y > ext.y && (forceVScrollBar == ScrollBarDynamic))
      {
         hasVScrollBar = true;
         ext.x -= scrollBarThickness;
         // doh! ext.x changed, so check hscrollbar again
         if(cext.x > ext.x && !hasHScrollBar && (forceHScrollBar == ScrollBarDynamic))
         {
            hasHScrollBar = true;
            ext.y -= scrollBarThickness;
         }
      }
      contentCtrl->position = pos;
      contentCtrl->extent = ext;

      // see if the child controls need to be repositioned (null space in control)
      Point2I delta(0,0);

      if(cpos.x > 0)
         delta.x = -cpos.x;
      else if(ext.x > cpos.x + cext.x)
      {
         Int32 diff = ext.x - (cpos.x + cext.x);
         delta.x = min(-cpos.x, diff);
      }
      if(cpos.y > 0)
         delta.y = -cpos.y;
      else if(ext.y > cpos.y + cext.y)
      {
         Int32 diff = ext.y - (cpos.y + cext.y);
         delta.y = min(-cpos.y, diff);
      }
      // apply the deltas to the children...
      if(delta.x || delta.y)
      {
         SimGroup::iterator i;
         for(i = contentCtrl->begin(); i != contentCtrl->end();i++)
         {
            Control *ctrl = (Control *) (*i);
            ctrl->position += delta;
         }
         cpos += delta;
      }
      if(cext.x < ext.x)
         cext.x = ext.x;
      if(cext.y < ext.y)
         cext.y = ext.y;

      // enable needed scroll bars
      if(cext.x > ext.x)
         hBarEnabled = true;
      if(cext.y > ext.y)
         vBarEnabled = true;

   }
   // build all the rectangles and such...
   calcScrollRects();
   line_V = 0.1f;
   line_H = 0.1f;
   page_V = 0.2f;
   page_H = 0.2f;

   SimGroup::iterator i;
   i = contentCtrl->begin();
   if (i != contentCtrl->end())
   {
      Control *ctrl = (Control *) (*i);
      RectI clientR(0,0,contentCtrl->extent.x, contentCtrl->extent.y);
      if (ctrl->extent.y)
      {
         line_V = ctrl->scroll_Line_V(clientR) / (float)ctrl->extent.y;
         page_V = ctrl->scroll_Page_V(clientR) / (float)ctrl->extent.y;
      }
      if (ctrl->extent.x)
      {
         line_H = ctrl->scroll_Line_H(clientR) / (float)ctrl->extent.x; 
         page_H = ctrl->scroll_Page_H(clientR) / (float)ctrl->extent.x; 
      }
   }

   calcThumbs(cpos, cext);
}

void ScrollCtrl::calcScrollRects(void)
{
   if(hasHScrollBar)
   {
      leftArrowRect(borderThickness, extent.y - borderThickness - scrollBarThickness,
         borderThickness + scrollBarArrowBtnLength, extent.y - borderThickness);
      rightArrowRect(extent.x - borderThickness - (hasVScrollBar ? scrollBarThickness : 0) - scrollBarArrowBtnLength, extent.y - borderThickness - scrollBarThickness, 
         extent.x - borderThickness - (hasVScrollBar ? scrollBarThickness : 0), extent.y - borderThickness);
      hTrackRect(leftArrowRect.lowerR.x, leftArrowRect.upperL.y,
                     rightArrowRect.upperL.x, rightArrowRect.lowerR.y);
   }
   if(hasVScrollBar)
   {
      upArrowRect(extent.x - borderThickness - scrollBarThickness, borderThickness,
         extent.x - borderThickness, borderThickness + scrollBarArrowBtnLength);
      downArrowRect(extent.x - borderThickness - scrollBarThickness, extent.y - borderThickness - (hasHScrollBar ? scrollBarThickness : 0) - scrollBarArrowBtnLength,
         extent.x - borderThickness, extent.y - borderThickness - (hasHScrollBar ? scrollBarThickness : 0));
      vTrackRect(upArrowRect.upperL.x, upArrowRect.lowerR.y,
                     downArrowRect.lowerR.x, downArrowRect.upperL.y);
   }
}

void ScrollCtrl::calcThumbs(Point2I cpos, Point2I cext)
{
   Point2I ext = contentCtrl->extent;
   if(hBarEnabled)
   {
      hBarThumbPos = -cpos.x / float(cext.x - ext.x);
      hBarThumbWidth = ext.x / float(cext.x);

      if(useConstantHeightThumb)
         hThumbSize = baseThumbSize;
      else
         hThumbSize = max(baseThumbSize, Int32(hBarThumbWidth * hTrackRect.len_x()));
      hThumbPos = hTrackRect.upperL.x + hBarThumbPos * (hTrackRect.len_x() - hThumbSize);
   }
   if(vBarEnabled)
   {
      vBarThumbPos = -cpos.y / float(cext.y - ext.y);
      vBarThumbWidth = ext.y / float(cext.y);

      if(useConstantHeightThumb)
         vThumbSize = baseThumbSize;
      else
         vThumbSize = max(baseThumbSize, Int32(vBarThumbWidth * vTrackRect.len_y()));
      vThumbPos = vTrackRect.upperL.y + vBarThumbPos * (vTrackRect.len_y() - vThumbSize);
   }
}

void ScrollCtrl::scrollTo(float x, float y)
{
   setUpdate();
   if(x < 0)
      x = 0;
   else if(x > 1)
      x = 1;

   if(y < 0)
      y = 0;
   else if(y > 1)
      y = 1;

   if(x == hBarThumbPos && y == vBarThumbPos)
      return;

   Point2I cpos, cext;

   if(calcChildExtents(&cpos, &cext))
   {
      Point2I ext = contentCtrl->extent;
      Point2I npos;
      if(ext.x < cext.x)
         npos.x = -x * (cext.x - ext.x);
      else
         npos.x = 0;
      if(ext.y < cext.y)
         npos.y = -y * (cext.y - ext.y);
      else
         npos.y = 0;

      Point2I delta(npos.x - cpos.x, npos.y - cpos.y);
      // get rid of bad (roundoff) deltas
      if(x == hBarThumbPos)
         delta.x = 0;
      if(y == vBarThumbPos)
         delta.y = 0;
      SimGroup::iterator i;
      
      for(i = contentCtrl->begin(); i != contentCtrl->end();i++)
      {
         Control *ctrl = (Control *) (*i);
         ctrl->position += delta;
      }
      cpos += delta;
      calcThumbs(cpos, cext);
   }
   if (contentCtrl) contentCtrl->setUpdate();
}

ScrollCtrl::Region ScrollCtrl::findHitRegion(const Point2I &pt)
{
   if (vBarEnabled && hasVScrollBar)
   {
      if(rectClip(&pt, &upArrowRect))
         return UpArrow;
      else if(rectClip(&pt, &downArrowRect))
         return DownArrow;
      else if(rectClip(&pt, &vTrackRect))
      {
         if(pt.y < vThumbPos)
            return UpPage;
         else if(pt.y < vThumbPos + vThumbSize)
            return VertThumb;
         else
            return DownPage;
      }
   }
   if (hBarEnabled && hasHScrollBar)
   {
      if(rectClip(&pt, &leftArrowRect))
         return LeftArrow;
      else if(rectClip(&pt, &rightArrowRect))
         return RightArrow;
      else if(rectClip(&pt, &hTrackRect))
      {
         if(pt.x < hThumbPos)
            return LeftPage;
         else if(pt.x < hThumbPos + hThumbSize)
            return HorizThumb;
         else
            return RightPage;
      }
   }
   return None;
}

void ScrollCtrl::onMouseRepeat(const Event &event)
{
   event;

   if (!stateDepressed)
      return;

   scrollByRegion(curHitRegion);
 
   Region hit = findHitRegion(curMousePos);
   if ((curHitRegion != VertThumb) && (curHitRegion != HorizThumb))
      stateDepressed = (hit == curHitRegion);
}   

bool ScrollCtrl::wantsTabListMembership()
{
   return true;
}   

bool ScrollCtrl::loseFirstResponder()
{
   setUpdate();
   return true;
}   

bool ScrollCtrl::becomeFirstResponder()
{
   setUpdate();
   return willFirstRespond;
}

void ScrollCtrl::onKeyDown(const Event &event)
{
   if (willFirstRespond)
   {
      switch(event.diKeyCode)
      {
         case DIK_RIGHT:
            scrollByRegion(RightArrow);
            return;
         case DIK_LEFT:
            scrollByRegion(LeftArrow);
            return;
         case DIK_DOWN:
            scrollByRegion(DownArrow);
            return;
         case DIK_UP:
            scrollByRegion(UpArrow);
            return;
         case DIK_PGUP:
            scrollByRegion(UpPage);
            return;
         case DIK_PGDN:
            scrollByRegion(DownPage);
            return;
      }
   }
   Parent::onKeyDown(event);
}   

void ScrollCtrl::onKeyRepeat(const Event &event)
{
   if(root->getFirstResponder() == this)
      onKeyDown(event);
}

void ScrollCtrl::onMouseDown(const Event &event)
{
   root->mouseLock(this); 
   root->makeFirstResponder(this);

   setUpdate();
   
   curMousePos = globalToLocalCoord(event.ptMouse);
   curHitRegion = findHitRegion(curMousePos);
   stateDepressed = true;

   scrollByRegion(curHitRegion);

   if(curHitRegion == VertThumb)
   {
      thumbAnchorPos = vBarThumbPos;
      thumbDelta = curMousePos.y - vThumbPos;
   }
   else if(curHitRegion == HorizThumb)
   {
      thumbAnchorPos = hBarThumbPos;
      thumbDelta = curMousePos.x - hThumbPos;
   }
}

void ScrollCtrl::onMouseUp(const Event &)
{
   if(root->getMouseLockedControl() == this)
      root->mouseUnlock();

   setUpdate();

   curHitRegion = None;
   stateDepressed = false;
}

void ScrollCtrl::onMouseDragged(const Event &event)
{
   curMousePos = globalToLocalCoord(event.ptMouse);
   setUpdate();

   if ( (curHitRegion != VertThumb) && (curHitRegion != HorizThumb) )
   {
      Region hit = findHitRegion(curMousePos);
      if (hit != curHitRegion)
         stateDepressed = false;
      else
         stateDepressed = true;   
      return;
   }

   // ok... if the mouse is 'near' the scroll bar, scroll with it
   // otherwise, snap back to the previous position.

   if (curHitRegion == VertThumb)
   {
      if(curMousePos.x >= vTrackRect.upperL.x - scrollBarThickness &&
         curMousePos.x <= vTrackRect.lowerR.x + scrollBarThickness &&
         curMousePos.y >= vTrackRect.upperL.y - scrollBarThickness &&
         curMousePos.y <= vTrackRect.lowerR.y + scrollBarThickness)
      {
         scrollTo(hBarThumbPos, 
            (curMousePos.y - thumbDelta - vTrackRect.upperL.y) /
            float(vTrackRect.len_y() - vThumbSize));
      }
      else
         scrollTo(hBarThumbPos, thumbAnchorPos);
   }
   else if (curHitRegion == HorizThumb)
   {
      if(curMousePos.x >= hTrackRect.upperL.x - scrollBarThickness &&
         curMousePos.x <= hTrackRect.lowerR.x + scrollBarThickness &&
         curMousePos.y >= hTrackRect.upperL.y - scrollBarThickness &&
         curMousePos.y <= hTrackRect.lowerR.y + scrollBarThickness)
      {
         scrollTo((curMousePos.x - thumbDelta - hTrackRect.upperL.x) /
            float(hTrackRect.len_x() - hThumbSize),
            vBarThumbPos);
      }
      else
         scrollTo(thumbAnchorPos, vBarThumbPos);
   }
}  

void ScrollCtrl::scrollByRegion(Region reg)
{
   if (vBarEnabled)
   {
      bool alreadyScrolled = FALSE;
      SimGui::ArrayCtrl *ac = NULL;
      if (contentCtrl && contentCtrl->begin())
      {
         Control *tempCtrl = (Control *)(contentCtrl->front());
         ac = dynamic_cast<SimGui::ArrayCtrl*>(tempCtrl);
      }
      if (ac)
      {
         int cellSize, numCells;
         ac->getScrollDimensions(cellSize, numCells);
         if (cellSize > 0 && numCells > 0)
         {
            int pageSize = int(contentCtrl->extent.y / cellSize) * (cellSize - 1) - 1;
            switch(reg)
            {
               case UpPage:
                  ac->position.y = min(long(0), max(contentCtrl->extent.y - ac->extent.y, ac->position.y + pageSize));
                  break;
               case DownPage:
                  ac->position.y = min(long(0), max(contentCtrl->extent.y - ac->extent.y, ac->position.y - pageSize));
                  break;
               case UpArrow:
                  ac->position.y = min(long(0), max(contentCtrl->extent.y - ac->extent.y, ac->position.y + cellSize));
                  break;
               case DownArrow:
                  ac->position.y = min(long(0), max(contentCtrl->extent.y - ac->extent.y, ac->position.y - cellSize));
                  break;
            }
            calcThumbs(ac->position, ac->extent);
            alreadyScrolled = TRUE;
         }
         else
         {
            page_V = 0.1f;
            line_V = 0.1f;
         }
      }
      else
      {
         page_V = 0.1f;
         line_V = 0.1f;
      }
   
      if (! alreadyScrolled)
      {
         switch(reg)
         {
            case UpPage:
               scrollTo(hBarThumbPos, vBarThumbPos - page_V);
               break;
            case DownPage:
               scrollTo(hBarThumbPos, vBarThumbPos + page_V);
               break;
            case UpArrow:
               scrollTo(hBarThumbPos, vBarThumbPos - line_V);
               break;
            case DownArrow:
               scrollTo(hBarThumbPos, vBarThumbPos + line_V);
               break;
         }
      }
   }

   if (hBarEnabled)
   {
      switch(reg)
      {
         case LeftPage:
            scrollTo(hBarThumbPos - page_H, vBarThumbPos);
            break;
         case RightPage:
            scrollTo(hBarThumbPos + page_H, vBarThumbPos);
            break;
         case LeftArrow:
            scrollTo(hBarThumbPos - line_H, vBarThumbPos);
            break;
         case RightArrow:
            scrollTo(hBarThumbPos + line_H, vBarThumbPos);
            break;
      }
   }
   if (contentCtrl) contentCtrl->setUpdate();
}   

void ScrollCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   // draw border
   drawBorder(sfc, offset);
   
   // draw scroll bars
   if (hasVScrollBar)
      drawVScrollBar(sfc, offset);
   if (hasHScrollBar)
      drawHScrollBar(sfc, offset);
   if (hasVScrollBar && hasHScrollBar)
      drawScrollCorner(sfc, offset);
   // draw content and its children
   renderChildControls(sfc, offset, updateRect);
}

void ScrollCtrl::drawBorder(GFXSurface *sfc, const Point2I &offset)
{
   RectI r(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1);
   
   if (mbOpaque)
   {
      sfc->drawRect2d_f(&r, (!disabled ? fillColor : ghostFillColor));
   }

   if (mbBoarder)
   {
      //if (root->getFirstResponder() == this)
      //   sfc->drawRect2d(&r, 0);
      //else   
         sfc->drawRect2d(&r, (!disabled ? boarderColor : ghostBoarderColor));
   }
}   

void ScrollCtrl::drawVScrollBar(GFXSurface *sfc, const Point2I &offset)
{
   Point2I pos = offset;
   pos += upArrowRect.upperL;

   GFXBitmap *bitmap;
   bitmap = bma->getBitmap ( vBarEnabled ?
      ((curHitRegion == UpArrow && stateDepressed) ? BmpUpArrowSelected : BmpUpArrow) :
      BmpUpArrowDisabled);

   sfc->drawBitmap2d( bitmap, &pos);

   pos.y += scrollBarArrowBtnLength;
   int end;
   if(vBarEnabled)
      end = vThumbPos + offset.y;
   else
      end = downArrowRect.upperL.y + offset.y;

   bitmap = bma->getBitmap( vBarEnabled ?
      ((curHitRegion == UpPage && stateDepressed) ? BmpVerticalPageSelected : BmpVerticalPage) :
      BmpVerticalPageDisabled);
      
   if (end > pos.y)
   {
      sfc->drawBitmap2d(bitmap, &pos, &Point2I(bitmap->getWidth(), end - pos.y));
   }
   
   pos.y = end;
   if(vBarEnabled)
   {
      bool thumbSelected = (curHitRegion == VertThumb && stateDepressed && bma->getCount() > BmpVerticalThumbSelected);
      GFXBitmap *ttop = bma->getBitmap( (thumbSelected) ? BmpVerticalThumbTopCapSelected : BmpVerticalThumbTopCap );
      GFXBitmap *tmid = bma->getBitmap( (thumbSelected) ? BmpVerticalThumbSelected : BmpVerticalThumb);
      GFXBitmap *tbot = bma->getBitmap( (thumbSelected) ? BmpVerticalThumbBottomCapSelected : BmpVerticalThumbBottomCap);

      // draw the thumb
      sfc->drawBitmap2d(ttop, &pos);
      pos.y += ttop->getHeight();
      end = vThumbPos + vThumbSize - tbot->getHeight() + offset.y;

      if (end > pos.y)
      {
         sfc->drawBitmap2d(tmid, &pos, &Point2I(tmid->getWidth(), end - pos.y));
      }
      pos.y = end;
      sfc->drawBitmap2d(tbot, &pos);
      pos.y += tbot->getHeight();
      end = vTrackRect.lowerR.y + offset.y;

      bitmap = bma->getBitmap( (curHitRegion == DownPage && stateDepressed) ?
         BmpVerticalPageSelected : BmpVerticalPage);
      if (end > pos.y)
      {
         sfc->drawBitmap2d(bitmap, &pos, &Point2I(bitmap->getWidth(), end - pos.y));
      }
      pos.y = end;
   }
   bitmap = bma->getBitmap ( vBarEnabled ?
      ((curHitRegion == DownArrow && stateDepressed ) ? 
         BmpDownArrowSelected : BmpDownArrow) : BmpDownArrowDisabled);

   sfc->drawBitmap2d(bitmap, &pos);
} 

void ScrollCtrl::drawHScrollBar(GFXSurface *sfc, const Point2I &offset)
{
   Point2I pos = offset;
   pos += leftArrowRect.upperL;

   GFXBitmap *bitmap;
   bitmap = bma->getBitmap ( hBarEnabled ?
      ((curHitRegion == LeftArrow && stateDepressed) ? BmpLeftArrowSelected : BmpLeftArrow) :
      BmpLeftArrowDisabled);

   sfc->drawBitmap2d( bitmap, &pos);

   pos.x += scrollBarArrowBtnLength;
   int end;
   if(hBarEnabled)
      end = hThumbPos + offset.x;
   else
      end = rightArrowRect.upperL.x + offset.x;

   bitmap = bma->getBitmap( hBarEnabled ?
      ((curHitRegion == LeftPage && stateDepressed) ? BmpHorizontalPageSelected : BmpHorizontalPage) :
      BmpHorizontalPageDisabled);

   if (end > pos.x)
   {
      sfc->drawBitmap2d(bitmap, &pos, &Point2I(end - pos.x, bitmap->getHeight()));
   }
   
   pos.x = end;
   if(hBarEnabled)
   {
      bool thumbSelected = (curHitRegion == HorizThumb && stateDepressed && bma->getCount() > BmpHorizontalThumbSelected);
      GFXBitmap *ttop = bma->getBitmap( (thumbSelected) ? BmpHorizontalThumbLeftCapSelected : BmpHorizontalThumbLeftCap );
      GFXBitmap *tmid = bma->getBitmap( (thumbSelected) ? BmpHorizontalThumbSelected : BmpHorizontalThumb);
      GFXBitmap *tbot = bma->getBitmap( (thumbSelected) ? BmpHorizontalThumbRightCapSelected : BmpHorizontalThumbRightCap);

      // draw the thumb
      sfc->drawBitmap2d(ttop, &pos);
      pos.x += ttop->getWidth();
      end = hThumbPos + hThumbSize - tbot->getWidth() + offset.x;
      if (end > pos.x)
      {
         sfc->drawBitmap2d(tmid, &pos, &Point2I(end - pos.x, tmid->getHeight()));
      }
      pos.x = end;
      sfc->drawBitmap2d(tbot, &pos);
      pos.x += tbot->getWidth();
      end = hTrackRect.lowerR.x + offset.x;

      bitmap = bma->getBitmap( (curHitRegion == RightPage && stateDepressed) ?
         BmpHorizontalPageSelected : BmpHorizontalPage);

      if (end > pos.x)
      {
         sfc->drawBitmap2d(bitmap, &pos, &Point2I(end - pos.x, bitmap->getHeight()));
      }
      pos.x = end;
   }
   bitmap = bma->getBitmap ( hBarEnabled ?
      ((curHitRegion == RightArrow && stateDepressed) ? BmpRightArrowSelected : BmpRightArrow) :
      BmpRightArrowDisabled);

   sfc->drawBitmap2d(bitmap, &pos);
}   

void ScrollCtrl::drawScrollCorner(GFXSurface *sfc, const Point2I &offset)
{
   Point2I pos = offset;
   pos.x += rightArrowRect.lowerR.x;
   pos.y += rightArrowRect.upperL.y;
   sfc->drawBitmap2d(bma->getBitmap(BmpResizeBox), &pos);
}   

void ScrollCtrl::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_PBA_TAG, true, "IDPBA_SCROLL*", pbaTag);
   insp->write(IDITG_HANDLE_ARROW_KEYS, (bool)willFirstRespond);
   insp->write(IDITG_CONST_THUMB, (bool)useConstantHeightThumb);
   insp->writeSimTagPairList(IDITG_HORIZ_BAR, forceHScrollBar, 3, 
      IDITG_SBAR_ALWAYS_ON, ScrollBarAlwaysOn,
      IDITG_SBAR_ALWAYS_OFF, ScrollBarAlwaysOff,
      IDITG_SBAR_DYNAMIC, ScrollBarDynamic);
   insp->writeSimTagPairList(IDITG_VERT_BAR, forceVScrollBar, 3,
      IDITG_SBAR_ALWAYS_ON, ScrollBarAlwaysOn,
      IDITG_SBAR_ALWAYS_OFF, ScrollBarAlwaysOff,
      IDITG_SBAR_DYNAMIC, ScrollBarDynamic);
}   
                
void ScrollCtrl::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   insp->read(IDITG_PBA_TAG, NULL, NULL, pbaTag); 
   insp->read(IDITG_HANDLE_ARROW_KEYS, (bool)willFirstRespond);
   insp->read(IDITG_CONST_THUMB, (bool)useConstantHeightThumb);
   insp->readSimTagPairList(IDITG_HORIZ_BAR, forceHScrollBar);
   insp->readSimTagPairList(IDITG_VERT_BAR, forceVScrollBar);
   loadBitmapArray();
}   

Persistent::Base::Error ScrollCtrl::write( StreamIO &sio, int version, int user )
{
   sio.write(pbaTag);
   sio.write(willFirstRespond);
   sio.write(forceHScrollBar);
   sio.write(forceVScrollBar);
   sio.write(useConstantHeightThumb);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error ScrollCtrl::read( StreamIO &sio, int version, int user)
{
   sio.read(&pbaTag);
   sio.read(&willFirstRespond);
   sio.read(&forceHScrollBar);
   sio.read(&forceVScrollBar);
   sio.read(&useConstantHeightThumb);

   return Parent::read(sio, version, user);
}

};
