#include "simGuiEditCtrl.h"
#include "g_surfac.h"
#include "inspectWindow.h"
#include <stdlib.h>

namespace SimGui
{

Point2I gGridSnap(0,0);

enum { GUI_BLACK = 0, GUI_WHITE = 255 };
enum { NUT_SIZE = 3 };

void EditControl::setCurrentAddSet(Control *ctrl)
{
   if (ctrl != currentAddSet)
   {
      selectedControls.clear();
      currentAddSet = ctrl;
   }
}

void EditControl::setSelection(Control *ctrl, bool inclusive)
{
   //sanity check
   if (! ctrl) return;
   
   // otherwise, we hit a new control...
   Control *newAddSet = (Control *) ctrl->getParent();
   
   //see if we should clear the old selection set
   if (newAddSet != currentAddSet || (! inclusive))
   {
      // new selection
      selectedControls.clear();
   }
   
   //make sure we have an add set
   if (! newAddSet)
   {
      newAddSet = root->getContentControl();
   }
   
   //set the selection
   currentAddSet = newAddSet;
   selectedControls.push_back(ctrl);
}

void EditControl::addNewControl(Control *ctrl)
{
   if(!currentAddSet)
      currentAddSet = root->getContentControl();
   currentAddSet->addObject(ctrl);
   selectedControls.clear();
   selectedControls.push_back(ctrl);
}

void EditControl::inspectControl(Control *ctrl)
{
   InspectWindow *gi= dynamic_cast<InspectWindow *>(manager->findObject("guiInspector"));
   if(gi)
   {
      gi->setObjToInspect(ctrl, ctrl ? ctrl->getClassName() : NULL);
   }
}

void EditControl::drawNut(GFXSurface *sfc, Point2I &nut, bool multisel)
{
   RectI r(nut.x - NUT_SIZE, nut.y - NUT_SIZE, nut.x + NUT_SIZE, nut.y + NUT_SIZE);
   sfc->drawRect2d(&r, multisel ? GUI_BLACK : GUI_WHITE);
   r.upperL += 1;
   r.lowerR -= 1;
   sfc->drawRect2d_f(&r, multisel ? GUI_WHITE : GUI_BLACK);
}

static inline bool inNut(const Point2I &pt, int x, int y)
{
   int dx = pt.x - x;
   int dy = pt.y - y;
   return dx <= NUT_SIZE && dx >= -NUT_SIZE && dy <= NUT_SIZE && dy >= -NUT_SIZE;
}

int EditControl::getSizingHitKnobs(const Point2I &pt, const RectI &box)
{
   int lx = box.upperL.x, rx = box.lowerR.x;
   int cx = (lx + rx) >> 1;
   int ty = box.upperL.y, by = box.lowerR.y;
   int cy = (ty + by) >> 1;

   if(inNut(pt, lx, ty))
      return sizingLeft | sizingTop;
   if(inNut(pt, cx, ty))
      return sizingTop;
   if(inNut(pt, rx, ty))
      return sizingRight | sizingTop;
   if(inNut(pt, lx, by))
      return sizingLeft | sizingBottom;
   if(inNut(pt, cx, by))
      return sizingBottom;
   if(inNut(pt, rx, by))
      return sizingRight | sizingBottom;
   if(inNut(pt, lx, cy))
      return sizingLeft;
   if(inNut(pt, rx, cy))
      return sizingRight;
   return 0;
}

void EditControl::drawNuts(GFXSurface *sfc, RectI &box, bool multisel)
{
   int lx = box.upperL.x, rx = box.lowerR.x;
   int cx = (lx + rx) >> 1;
   int ty = box.upperL.y, by = box.lowerR.y;
   int cy = (ty + by) >> 1;
   drawNut(sfc, Point2I(lx, ty), multisel);
   drawNut(sfc, Point2I(lx, cy), multisel);
   drawNut(sfc, Point2I(lx, by), multisel);
   drawNut(sfc, Point2I(rx, ty), multisel);
   drawNut(sfc, Point2I(rx, cy), multisel);
   drawNut(sfc, Point2I(rx, by), multisel);
   drawNut(sfc, Point2I(cx, ty), multisel);
   drawNut(sfc, Point2I(cx, by), multisel);
}

void EditControl::getDragRect(Box2I &box)
{
   if(lastMousePos.x < selectionAnchor.x)
   {
      box.fMin.x = lastMousePos.x;
      box.fMax.x = selectionAnchor.x;
   }
   else
   {
      box.fMax.x = lastMousePos.x;
      box.fMin.x = selectionAnchor.x;
   }
   if(lastMousePos.y < selectionAnchor.y)
   {
      box.fMin.y = lastMousePos.y;
      box.fMax.y = selectionAnchor.y;
   }
   else
   {
      box.fMax.y = lastMousePos.y;
      box.fMin.y = selectionAnchor.y;
   }
}

void EditControl::onPreRender()
{
   setUpdate();
}

void EditControl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   Point2I ctOffset;
   Point2I cext;

   sfc->setClipRect(&RectI(0,0,sfc->getWidth()-1, sfc->getHeight()-1));

   if(currentAddSet)
   {
      // draw a white frame inset around the current add set.
      cext = currentAddSet->getExtent();
      ctOffset = currentAddSet->localToGlobalCoord(Point2I(0,0));
      RectI box(ctOffset.x + 1,ctOffset.y + 1,
            ctOffset.x + cext.x - 2,
            ctOffset.y + cext.y - 2);
      sfc->drawRect2d(&box, GUI_WHITE);
      box.upperL -= 1;
      box.lowerR += 1;
      sfc->drawRect2d(&box, GUI_BLACK);
   }
   Vector<Control *>::iterator i;
   bool multisel = selectedControls.size() > 1;
   for(i = selectedControls.begin(); i != selectedControls.end(); i++)
   {
      Control *ctrl = (*i);
      cext = ctrl->getExtent();
      ctOffset = ctrl->localToGlobalCoord(Point2I(0,0));
      RectI box(ctOffset.x,ctOffset.y,
            ctOffset.x + cext.x,
            ctOffset.y + cext.y);
      drawNuts(sfc, box, multisel);
   }
   if(mouseDownMode == DragSelecting)
   {
      Box2I b;
      getDragRect(b);
      RectI r(b.fMin, b.fMax);
      sfc->drawRect2d(&r, GUI_WHITE);
   }
   renderChildControls(sfc, offset, updateRect);
}

bool EditControl::selectionContains(Control *ctrl)
{
   Vector<Control *>::iterator i;
   for(i = selectedControls.begin(); i != selectedControls.end(); i++)
      if(ctrl == *i) return true;
   return false;
}

void EditControl::onRightMouseDown(const Event &event)
{
   Control *ctrl = NULL, *topCtrl;
   topCtrl = root->getTopDialog();
   if (topCtrl) ctrl = topCtrl->findHitControl(event.ptMouse);
   if (! ctrl) ctrl = root->getContentControl()->findHitControl(event.ptMouse);
   if(ctrl != currentAddSet)
   {
      selectedControls.clear();
      currentAddSet = ctrl;
   }
}

void EditControl::onMouseDown(const Event &event)
{
   Point2I ctOffset;
   Point2I cext;
   Control *ctrl;

   lastMousePos = event.ptMouse;
   // first see if we hit a sizing knob on the currently selected control...

   if(selectedControls.size() == 1)
   {
      ctrl = selectedControls.first();
      cext = ctrl->getExtent();
      ctOffset = ctrl->localToGlobalCoord(Point2I(0,0));
      RectI box(ctOffset.x,ctOffset.y,
            ctOffset.x + cext.x,
            ctOffset.y + cext.y);

      if((sizingMode = getSizingHitKnobs(event.ptMouse, box)) != 0)
      {
         mouseDownMode = SizingSelection;
         return;
      }
   }
   ctrl = root->getContentControl()->findHitControl(event.ptMouse);
   if(selectionContains(ctrl))
   {
      if(event.modifier & SI_SHIFT)
      {
         mouseDownMode = Selecting;
         // gotta remove this control from the current selected set
         Vector<Control *>::iterator i;
         for(i = selectedControls.begin(); i != selectedControls.end(); i++)
         {
            if(*i == ctrl)
            {
               selectedControls.erase(i);
               break;
            }
         }
      }
      else
         mouseDownMode = MovingSelection;
   }
   else if(ctrl == currentAddSet)
   {
      // start dragging a rectangle
      if(!(event.modifier & SI_SHIFT)) // if the shift is not down, nuke prior selection
         selectedControls.clear();
      selectionAnchor = event.ptMouse;
      mouseDownMode = DragSelecting;
   }
   else
   {
      // otherwise, we hit a new control...
      mouseDownMode = Selecting;
      Control *newAddSet = (Control *) ctrl->getParent();
      if(newAddSet != currentAddSet || !(event.modifier & SI_SHIFT))
      {
         // new selection
         selectedControls.clear();
         if(!newAddSet)
         {
            currentAddSet = root->getContentControl();
            return;
         }
         mouseDownMode = MovingSelection;
         currentAddSet = newAddSet;
         selectedControls.push_back(ctrl);
      }
      else if(newAddSet == currentAddSet && (event.modifier & SI_SHIFT))
      {
         selectedControls.push_back(ctrl);
      }
   }
}

void EditControl::onMouseUp(const Event &event)
{
   lastMousePos = event.ptMouse;
   if(mouseDownMode == DragSelecting)
   {
      Box2I b;
      getDragRect(b);
      Control::iterator i;
      for(i = currentAddSet->begin(); i != currentAddSet->end(); i++)
      {
         Control *ctrl = dynamic_cast<Control *>(*i);
         Point2I lowerR = ctrl->extent;
         Point2I upperL(0,0);
         upperL = ctrl->localToGlobalCoord(upperL);
         lowerR += upperL;
         if(b.contains(upperL) && b.contains(lowerR) && !selectionContains(ctrl))
            selectedControls.push_back(ctrl);
      }
   }
   if(selectedControls.size() == 1)
      inspectControl(*selectedControls.begin());
   else if(selectedControls.size() == 0)
      inspectControl(currentAddSet);
   else
      inspectControl(NULL);
   mouseDownMode = Selecting;
}

void EditControl::onMouseDragged(const Event &event)
{
   Point2I ptMouse = event.ptMouse;

   if(mouseDownMode == SizingSelection)
   {
      if(gGridSnap.x)
         ptMouse.x -= ptMouse.x % gGridSnap.x;
      if(gGridSnap.y)
         ptMouse.y -= ptMouse.y % gGridSnap.y;

      Control *ctrl = selectedControls.first();
      Point2I ctrlPoint = currentAddSet->globalToLocalCoord(ptMouse);
      Point2I minExtent = ctrl->getMinExtent();
      int newExtent;

      if(sizingMode & sizingLeft)
      {
         newExtent = ctrl->extent.x + ctrl->position.x - ctrlPoint.x;
         if(newExtent < minExtent.x)
            newExtent = minExtent.x;
         ctrl->position.x += ctrl->extent.x - newExtent;
         ctrl->extent.x = newExtent;
      }
      else if(sizingMode & sizingRight)
      {
         newExtent = ctrlPoint.x - ctrl->position.x;
         if(newExtent < minExtent.x)
            ctrl->extent.x = minExtent.x;
         else
            ctrl->extent.x = newExtent;
      }
      if(sizingMode & sizingTop)
      {
         newExtent = ctrl->extent.y + ctrl->position.y - ctrlPoint.y;
         if(newExtent < minExtent.y)
            newExtent = minExtent.y;
         ctrl->position.y += ctrl->extent.y - newExtent;
         ctrl->extent.y = newExtent;
      }
      else if(sizingMode & sizingBottom)
      {
         newExtent = ctrlPoint.y - ctrl->position.y;
         if(newExtent < minExtent.y)
            ctrl->extent.y = minExtent.y;
         else
            ctrl->extent.y = newExtent;
      }
      ctrl->editResized();
      currentAddSet->childResized(ctrl);
   } 
   else if(mouseDownMode == MovingSelection && selectedControls.size())
   {
      Vector<Control *>::iterator i = selectedControls.begin();
      Point2I minPos = (*i)->position;
      for(; i != selectedControls.end(); i++)
      {
         if((*i)->position.x < minPos.x)
            minPos.x = (*i)->position.x;
         if((*i)->position.y < minPos.y)
            minPos.y = (*i)->position.y;
      }
      Point2I delta = ptMouse - lastMousePos;
      delta += minPos; // find new minPos;

      if(gGridSnap.x)
         delta.x -= delta.x % gGridSnap.x;
      if(gGridSnap.y)
         delta.y -= delta.y % gGridSnap.y;

      delta -= minPos;
      moveSelection(delta);
      lastMousePos += delta;
   }
   else
      lastMousePos = ptMouse;
}

void EditControl::moveSelection(Point2I &delta)
{
   Vector<Control *>::iterator i;

   for(i = selectedControls.begin(); i != selectedControls.end(); i++)
   {
      (*i)->position += delta;
      (*i)->editResized();
      currentAddSet->childResized(*i);
   }
}

void EditControl::justifySelection(Justification j)
{
   Int32 minX, maxX;
   Int32 minY, maxY;
   Int32 extentX, extentY;

   if(selectedControls.size() < 2)
      return;

   Vector<Control *>::iterator i = selectedControls.begin();
   minX = (*i)->position.x;
   maxX = minX + (*i)->extent.x;
   minY = (*i)->position.y;
   maxY = minY + (*i)->extent.y;
   extentX = (*i)->extent.x;
   extentY = (*i)->extent.y;
   i++;
   for(;i != selectedControls.end(); i++)
   {
      minX = min(minX, (*i)->position.x);
      maxX = max(maxX, (*i)->position.x + (*i)->extent.x);
      minY = min(minY, (*i)->position.y);
      maxY = max(maxY, (*i)->position.y + (*i)->extent.y);
      extentX += (*i)->extent.x;
      extentY += (*i)->extent.y;
   }
   Int32 deltaX = maxX - minX;
   Int32 deltaY = maxY - minY;
   switch(j)
   {
      case JUSTIFY_LEFT:
         for(i = selectedControls.begin(); i != selectedControls.end(); i++)
            (*i)->position.x = minX;
         break;
      case JUSTIFY_TOP:
         for(i = selectedControls.begin(); i != selectedControls.end(); i++)
            (*i)->position.y = minY;
         break;
      case JUSTIFY_RIGHT:
         for(i = selectedControls.begin(); i != selectedControls.end(); i++)
            (*i)->position.x = maxX - (*i)->extent.x;
         break;
      case JUSTIFY_BOTTOM:
         for(i = selectedControls.begin(); i != selectedControls.end(); i++)
            (*i)->position.y = maxY - (*i)->extent.y;
         break;
      case JUSTIFY_CENTER:
         for(i = selectedControls.begin(); i != selectedControls.end(); i++)
            (*i)->position.x = minX + ((deltaX - (*i)->extent.x) >> 1);
         break;
      case SPACING_VERTICAL:
         {
            Vector<Control *> sortedList;
            Vector<Control *>::iterator k;
            for(i = selectedControls.begin(); i != selectedControls.end(); i++)
            {
               for(k = sortedList.begin(); k != sortedList.end(); k++)
               {
                  if((*i)->position.y < (*k)->position.y)
                     break;
               }
               sortedList.insert(k, *i);
            }
            Int32 space = (deltaY - extentY) / (selectedControls.size() - 1);
            Int32 curY = minY;
            for(k = sortedList.begin(); k != sortedList.end(); k++)
            {
               (*k)->position.y = curY;
               curY += (*k)->extent.y + space;
            }
         }
         break;
      case SPACING_HORIZONTAL:
         {
            Vector<Control *> sortedList;
            Vector<Control *>::iterator k;
            for(i = selectedControls.begin(); i != selectedControls.end(); i++)
            {
               for(k = sortedList.begin(); k != sortedList.end(); k++)
               {
                  if((*i)->position.x < (*k)->position.x)
                     break;
               }
               sortedList.insert(k, *i);
            }
            Int32 space = (deltaX - extentX) / (selectedControls.size() - 1);
            Int32 curX = minX;
            for(k = sortedList.begin(); k != sortedList.end(); k++)
            {
               (*k)->position.x = curX;
               curX += (*k)->extent.x + space;
            }
         }
         break;
   }
   for(i = selectedControls.begin(); i != selectedControls.end(); i++)
   {
      (*i)->editResized();
      currentAddSet->childResized(*i);
   }
}

void EditControl::deleteSelection(void)
{
   Vector<Control *>::iterator i;
   for(i = selectedControls.begin(); i != selectedControls.end(); i++)
   {
      (*i)->deleteObject();
   }
   selectedControls.clear();
}

void EditControl::loadSelection(const char *filename)
{
   Int32 controlCount;
   FileRStream sio;
   if(!sio.open(filename))
      return;
   sio.read(&controlCount);
   int i;
   selectedControls.clear();
   for(i = 0; i < controlCount; i++)
   {
      Persistent::Base::Error err;
      Control *ctrl = (Control *) Persistent::Base::load(sio, &err);
      manager->addObject(ctrl);
      currentAddSet->addObject(ctrl);
      selectedControls.push_back(ctrl);
   }
   sio.close();
}

void EditControl::saveSelection(const char *filename)
{
   FileRWStream sio;
   if(!sio.open(filename))
      return;
   Int32 count = selectedControls.size();
   sio.write(count);
   Vector<Control *>::iterator i;
   for(i = selectedControls.begin(); i != selectedControls.end(); i++)
      (*i)->store(sio);

   sio.close();
}

void EditControl::selectAll(void)
{
   Control::iterator i;
   if(!currentAddSet)
      return;
   selectedControls.clear();
   for(i = currentAddSet->begin(); i != currentAddSet->end(); i++)
   {
      Control *ctrl = dynamic_cast<Control *>(*i);
      selectedControls.push_back(ctrl);
   }
}

void EditControl::bringToFront(void)
{
   if(selectedControls.size() != 1)
      return;
   Control *ct = *(selectedControls.begin());
   currentAddSet->removeObject(ct);
   currentAddSet->addObject(ct);
}

void EditControl::sendToBack(void)
{
   if(selectedControls.size() != 1)
      return;
   Control *ctrl = *(selectedControls.begin());
   Vector<Control *> saveList;
   Control::iterator i;
   for(i = currentAddSet->begin(); i != currentAddSet->end();)
   {
      Control *c = dynamic_cast<Control *>(*i);
      if(c != ctrl)
      {
         saveList.push_back(c);
         currentAddSet->removeObject(c);
      }
      else
         i++;
   }
   Vector<Control *>::iterator k;
   for(k = saveList.begin(); k != saveList.end(); k++)
      currentAddSet->addObject(*k);
}

void EditControl::onKeyDown(const Event &event)
{
   if(event.modifier & SI_CTRL)
   {
      switch(event.diKeyCode)
      {
         case DIK_A:
            selectAll();
            break;
         case DIK_C:
            saveSelection("temp\\clipboard.gui");
            break;
         case DIK_X:
            saveSelection("temp\\clipboard.gui");
            deleteSelection();
            break;
         case DIK_V:
            loadSelection("temp\\clipboard.gui");
            break;
      }
   }
   else
   {
      int delta = (event.modifier & SI_SHIFT) ? 10 : 1;

      switch(event.diKeyCode)
      {
         case DIK_RIGHT:
            moveSelection(Point2I(delta, 0));
            break;
         case DIK_LEFT:
            moveSelection(Point2I(-delta, 0));
            break;
         case DIK_UP:
            moveSelection(Point2I(0, -delta));
            break;
         case DIK_DOWN:
            moveSelection(Point2I(0, delta));
            break;
         case DIK_BACK:
         case DIK_DELETE:
            deleteSelection();
            break;
         case DIK_F1:
            justifySelection(JUSTIFY_LEFT);
            break;
         case DIK_F2:
            justifySelection(JUSTIFY_CENTER);
            break;
         case DIK_F3:
            justifySelection(JUSTIFY_RIGHT);
            break;
         case DIK_F4:
            justifySelection(JUSTIFY_TOP);
            break;
         case DIK_F5:
            justifySelection(JUSTIFY_BOTTOM);
            break;
         case DIK_F6:
            justifySelection(SPACING_VERTICAL);
            break;
         case DIK_F7:
            justifySelection(SPACING_HORIZONTAL);
            break;
         case DIK_F9:
            bringToFront();
            break;
         case DIK_F10:
            sendToBack();
            break;
      }
   }
}

};