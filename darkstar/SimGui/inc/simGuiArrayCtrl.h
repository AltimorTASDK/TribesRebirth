
#ifndef _H_SIMGUIARRAYCTRL
#define _H_SIMGUIARRAYCTRL

#include "simGuiActiveCtrl.h"

namespace SimGui
{

class ArrayCtrl : public ActiveCtrl
{
   typedef ActiveCtrl Parent;

protected:
	Point2I headerDim;
   Point2I size;
   Point2I cellSize;
   Point2I selectedCell;
   Point2I mouseOverCell;

   virtual bool cellSelected(Point2I cell);
public:
   Point2I getSize();
   virtual void setSize(Point2I size);
	void setHeaderDim(const Point2I &dim) { headerDim = dim; }
   
   void setSelectedCell(Point2I cell);
   Point2I getSelectedCell();

	void getScrollDimensions(int &cell_size, int &num_cells);
   void scrollSelectionVisible();
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

	virtual void onRenderColumnHeaders(GFXSurface *sfc, Point2I offset, Point2I parentOffset, Point2I headerDim);
	virtual void onRenderRowHeader(GFXSurface *sfc, Point2I offset, Point2I parentOffset, Point2I headerDim, Point2I cell);
   virtual void onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool selected, bool mouseOver);

   void onMouseDown(const Event &event);
   void onMouseMove(const Event &event);
   void onMouseEnter(const Event &event);
   void onMouseLeave(const Event &event);
   void onKeyDown(const Event &event);
   void onKeyRepeat(const Event &event);
   void onWake();

   ArrayCtrl();

   bool becomeFirstResponder();

   DECLARE_PERSISTENT(ArrayCtrl);
};

};

#endif