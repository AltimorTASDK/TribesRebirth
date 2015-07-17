#include "simGuiArrayCtrl.h"
#include "g_surfac.h"
#include "r_clip.h"

namespace SimGui
{

// child should call parent if cell select is ok.

bool ArrayCtrl::cellSelected(Point2I cell)
{
   selectedCell = cell;
   scrollSelectionVisible();
   return true;
}

Point2I ArrayCtrl::getSize()
{
   return size;
}

void ArrayCtrl::setSize(Point2I newSize)
{
   size = newSize;
   Point2I newExtent(newSize.x * cellSize.x + headerDim.x, newSize.y * cellSize.y + headerDim.y);

   resize(position, newExtent);
}

void ArrayCtrl::setSelectedCell(Point2I cell)
{
   cellSelected(cell);
}

Point2I ArrayCtrl::getSelectedCell()
{
   return selectedCell;
}

void ArrayCtrl::getScrollDimensions(int &cell_size, int &num_cells)
{
   cell_size = cellSize.y;
   num_cells = size.y;
}

void ArrayCtrl::scrollSelectionVisible()
{
   if(!parent)
      return;
   if(selectedCell.x == -1)
      return;

   Point2I parentExtent = parent->getExtent();


   Point2I cellPos(position.x + selectedCell.x * cellSize.x,
                   position.y + selectedCell.y * cellSize.y);

   Point2I delta(0,0);

   if(cellPos.x <= 0)
      delta.x = -cellPos.x;
   else if(cellPos.x + cellSize.x > parentExtent.x)
      delta.x = parentExtent.x - (cellPos.x + cellSize.x);

   if(cellPos.y <= 0)
      delta.y = -cellPos.y;
   else if(cellPos.y + cellSize.y > parentExtent.y)
      delta.y = parentExtent.y - (cellPos.y + cellSize.y);

   if(delta.x || delta.y)
   {
      Point2I newPosition = position;
      newPosition += delta;
      resize(newPosition, extent);
   }
}

void ArrayCtrl::onRenderColumnHeaders(GFXSurface *sfc, Point2I offset, Point2I parentOffset, Point2I headerDim)
{
   if (mbBoarder)
   {
      RectI cellR(offset.x + headerDim.x, parentOffset.y, offset.x + extent.x, parentOffset.y + headerDim.y);
      sfc->drawRect2d_f(&cellR, boarderColor);
   }
}

void ArrayCtrl::onRenderRowHeader(GFXSurface *sfc, Point2I offset, Point2I parentOffset, Point2I headerDim, Point2I cell)
{
   int color;
   RectI cellR;

   color = (cell.y % 2 ? 254 : 253);
   cellR.upperL.set(parentOffset.x, offset.y);
   cellR.lowerR.set(parentOffset.x + headerDim.x, offset.y + cellSize.y);
   sfc->drawRect2d_f(&cellR, color);
}

void ArrayCtrl::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   int color = cell.x + cell.y;
   if(selected)
      color = 255;
   else if(mouseOver)
      color = 0;
   RectI cellR(offset.x, offset.y, offset.x + cellSize.x, offset.y + cellSize.y);
   sfc->drawRect2d_f(&cellR, color);
}

void ArrayCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   //make sure we have a parent
   if (! parent) return;
   
   int i, j;
   RectI headerClip;
   RectI clipRect(updateRect.fMin, updateRect.fMax);
   clipRect.lowerR -= 1; // inset by 1 for GFX
   
   Point2I parentRect = parent->localToGlobalCoord(Point2I(0, 0));
   
   //if we have column headings
   if (headerDim.y > 0)
   {
      headerClip.upperL.x =   parentRect.x + headerDim.x;
      headerClip.upperL.y =   parentRect.y;
      headerClip.lowerR.x =   clipRect.lowerR.x;
      headerClip.lowerR.y =   parentRect.y + headerDim.y;
      
      if (rectClip(&headerClip, &clipRect))
      {
         sfc->setClipRect(&headerClip);
               
         //now render the header
         onRenderColumnHeaders(sfc, offset, parentRect, headerDim);
         
         clipRect.upperL.y = headerClip.lowerR.y;
      }
      offset.y += headerDim.y;
   }
   
   //save the original for clipping the row headers
   RectI origClipRect(clipRect.upperL, clipRect.lowerR);
   
   //if we have row headings 
   if (headerDim.x > 0)
   {
      clipRect.upperL.x = max(clipRect.upperL.x, parentRect.x + headerDim.x);
      offset.x += headerDim.x;
   }
   
   for(j = 0; j < size.y; j++)
   {
      if((j + 1) * cellSize.y + offset.y < updateRect.fMin.y)
         continue;
      if(j * cellSize.y + offset.y >= updateRect.fMax.y)
         break;
         
      //render the header
      if (headerDim.x > 0)
      {
         headerClip.upperL.x =   parentRect.x;
         headerClip.lowerR.x =   parentRect.x + headerDim.x;
         headerClip.upperL.y = offset.y + j * cellSize.y;
         headerClip.lowerR.y = offset.y + (j + 1) * cellSize.y;
         if(rectClip(&headerClip, &origClipRect))
         {
            sfc->setClipRect(&headerClip);
            
            //render the row header
            onRenderRowHeader(sfc, Point2I(0, offset.y + j * cellSize.y),
                              Point2I(parentRect.x, offset.y + j * cellSize.y),
                              headerDim, Point2I(0, j));
         }
      }
      
      //render the cells for the row
      for(i = 0; i < size.x; i++)
      {
         if((i + 1) * cellSize.x + offset.x < updateRect.fMin.x)
            continue;
         if(i * cellSize.x + offset.x >= updateRect.fMax.x)
            break;
         int cellx = offset.x + i * cellSize.x;
         int celly = offset.y + j * cellSize.y;

         RectI cellClip(cellx, celly, 
            cellx + cellSize.x - 1, celly + cellSize.y - 1);
         
         if(rectClip(&cellClip, &clipRect))
         {
            sfc->setClipRect(&cellClip);
            onRenderCell(sfc, Point2I(cellx, celly), Point2I(i, j), 
               i == selectedCell.x && j == selectedCell.y,
               i == mouseOverCell.x && j == mouseOverCell.y);
         }
      }
   }
}

void ArrayCtrl::onMouseDown(const Event &event)
{
   if(!active)
   {
      Parent::onMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   pt.x -= headerDim.x; pt.y -= headerDim.y;
   Point2I cell((pt.x < 0 ? -1 : pt.x / cellSize.x), (pt.y < 0 ? -1 : pt.y / cellSize.y));
   if(cell.x >= 0 && cell.x < size.x && cell.y >= 0 && cell.y < size.y)
      cellSelected(cell);
   Parent::onMouseDown(event);
}

void ArrayCtrl::onMouseEnter(const Event &event)
{
   Point2I pt = globalToLocalCoord(event.ptMouse);
   pt.x -= headerDim.x; pt.y -= headerDim.y;
   Point2I cell((pt.x < 0 ? -1 : pt.x / cellSize.x), (pt.y < 0 ? -1 : pt.y / cellSize.y));
   if(cell.x >= 0 && cell.x < size.x && cell.y >= 0 && cell.y < size.y)
   {
      mouseOverCell = cell;
      setUpdateRegion( Point2I(cell.x * cellSize.x + headerDim.x,
                              cell.y * cellSize.y + headerDim.y), cellSize );
   }
   
   //set the state over flag
   stateOver = ((mouseOverCell.x != -1) && (mouseOverCell.y != -1));
}

void ArrayCtrl::onMouseLeave(const Event& /*event*/)
{
   setUpdateRegion( Point2I(mouseOverCell.x * cellSize.x + headerDim.x,
                           mouseOverCell.y * cellSize.y + headerDim.y), cellSize );
   mouseOverCell.set(-1,-1);
   
   //set the state over flag
   stateOver = FALSE;
}

void ArrayCtrl::onMouseMove(const Event &event)
{
   Point2I pt = globalToLocalCoord(event.ptMouse);
   pt.x -= headerDim.x; pt.y -= headerDim.y;
   Point2I cell((pt.x < 0 ? -1 : pt.x / cellSize.x), (pt.y < 0 ? -1 : pt.y / cellSize.y));
   if(cell.x != mouseOverCell.x || cell.y != mouseOverCell.y)
   {
      if(mouseOverCell.x != -1)
      {
         setUpdateRegion( Point2I(mouseOverCell.x * cellSize.x + headerDim.x,
                           mouseOverCell.y * cellSize.y + headerDim.y), cellSize );
      }
      
      if(cell.x >= 0 && cell.x < size.x && cell.y >= 0 && cell.y < size.y)
      {
         setUpdateRegion( Point2I(cell.x * cellSize.x + headerDim.x,
                           cell.y * cellSize.y + headerDim.y), cellSize );
         mouseOverCell = cell;
      }
      else
         mouseOverCell.set(-1,-1);
         
      //set the state over flag
      stateOver = ((mouseOverCell.x != -1) && (mouseOverCell.y != -1));
   }
}

void ArrayCtrl::onKeyDown(const Event &event)
{
   if(!active)
   {
      Parent::onKeyDown(event);
      return;
   }

   Point2I delta(0,0);

   switch(event.diKeyCode)
   {
      case DIK_LEFT:
         delta.set(-1, 0);
         break;
      case DIK_RIGHT:
         delta.set(1, 0);
         break;
      case DIK_UP:
         delta.set(0, -1);
         break;
      case DIK_DOWN:
         delta.set(0, 1);
         break;
      default:
         Parent::onKeyDown(event);
         return;
   }
   if(size.x < 1 || size.y < 1)
      return;

   if(selectedCell.x == -1 || selectedCell.y == -1)
   {
      cellSelected(Point2I(0,0));
      return;
   }
   Point2I cell = selectedCell;
   cell += delta;

   while(cell.x >= 0 && cell.x < size.x && cell.y >= 0 && cell.y < size.y)
   {
      if(cellSelected(cell))
      {
         onAction();
         break;
      }
      cell += delta;
   }
}

void ArrayCtrl::onKeyRepeat(const Event &event)
{
   if(root->getFirstResponder() == this)
      onKeyDown(event);
}

void ArrayCtrl::onWake()
{
   setSize(size);
}

ArrayCtrl::ArrayCtrl()
{
   cellSize.set(80, 30);
   size = Point2I(5, 30);
   selectedCell.set(-1, -1);
   mouseOverCell.set(-1, -1);
   headerDim.set(0, 0);
   active = true;
}

bool ArrayCtrl::becomeFirstResponder()
{
   return (active && size.x && size.y);
}

};