#ifndef _H_SIMGUIEDITCTRL
#define _H_SIMGUIEDITCTRL

#include "simGuiCtrl.h"
#include "tvector.h"

namespace SimGui
{

class EditControl : public Control
{
   Vector<Control *> selectedControls;
   Control *currentAddSet;
   Point2I lastMousePos;
   Point2I selectionAnchor;

   enum mouseModes { Selecting, MovingSelection, SizingSelection, DragSelecting };
   enum sizingModes { sizingLeft = 1, sizingRight = 2, sizingTop = 4, sizingBottom = 8 };

   mouseModes mouseDownMode;
   int sizingMode;

public:
   EditControl() { currentAddSet = NULL; }
   int  getSizingHitKnobs(const Point2I &pt, const RectI &box);
   void inspectControl(Control *ctrl);
   void getDragRect(Box2I &b);
   void drawNut(GFXSurface *sfc, Point2I &nut, bool);
   void drawNuts(GFXSurface *sfc, RectI &box, bool);
   void onPreRender();
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   void addNewControl(Control *ctrl);
   bool selectionContains(Control *ctrl);
	void setCurrentAddSet(Control *ctrl);
	void setSelection(Control *ctrl, bool inclusive = FALSE);

   void onKeyDown(const Event &event);
   void onMouseDown(const Event &event);
   void onMouseUp(const Event &event);
   void onMouseDragged(const Event &event);
   void onRightMouseDown(const Event &event);

   enum Justification { JUSTIFY_LEFT, JUSTIFY_CENTER, JUSTIFY_RIGHT, JUSTIFY_TOP, JUSTIFY_BOTTOM, SPACING_VERTICAL, SPACING_HORIZONTAL };
   void justifySelection( Justification j);
   void moveSelection(Point2I &delta);
   void saveSelection(const char *filename);
   void loadSelection(const char *filename);
   void deleteSelection(void);
   void selectAll(void);
   void bringToFront();
   void sendToBack();

   DECLARE_PERSISTENT(EditControl);
};

};

#endif
