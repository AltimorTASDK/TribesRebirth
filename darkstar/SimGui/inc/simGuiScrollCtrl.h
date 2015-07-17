//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUISCROLLCTRL_H_
#define _SIMGUISCROLLCTRL_H_

#include "simGuiActiveCtrl.h"
#include "g_barray.h"
#include "darkstar.strings.h"

namespace SimGui
{

// the function of the scroll content control class
// is to notify the parent class that children have resized.
// basically it just calls it's parent (enclosing) control's
// childResized method which turns around and computes new sizes
// for the scroll bars

class ScrollContentCtrl : public Control
{
private:
   typedef Control Parent;
protected:
   void childResized(Control *child);
   void removeObject(SimObject *object);

public:
	void setParentDisabled(bool disabled);
	void setParentBoarder(bool on_off, BYTE color, BYTE selColor, BYTE disColor);
	void setParentOpaque(bool on_off, BYTE color, BYTE selColor, BYTE disColor);

   DECLARE_PERSISTENT(ScrollContentCtrl);
};

class ScrollCtrl : public Control
{
private:
   typedef Control Parent;

protected:
   // the scroll control uses a bitmap array to draw all its
   // stuff... these are the bitmaps it needs:
   Resource<GFXBitmapArray> bma;

   enum BitmapIndices
   {
      BmpUpArrowDisabled = 0,
      BmpUpArrowSelected,
      BmpUpArrow,
      BmpDownArrowDisabled,
      BmpDownArrowSelected,
      BmpDownArrow,
      BmpLeftArrowDisabled,
      BmpLeftArrowSelected,
      BmpLeftArrow,
      BmpRightArrowDisabled,
      BmpRightArrowSelected,
      BmpRightArrow,
      BmpHorizontalThumb,
      BmpHorizontalThumbLeftCap,
      BmpHorizontalThumbRightCap,
      BmpVerticalThumb,
      BmpVerticalThumbBottomCap,
      BmpVerticalThumbTopCap,
      BmpHorizontalPageDisabled,
      BmpHorizontalPageSelected,
      BmpHorizontalPage,
      BmpVerticalPageDisabled,
      BmpVerticalPageSelected,
      BmpVerticalPage,
      BmpResizeBox,
      BmpHorizontalThumbSelected,
      BmpHorizontalThumbLeftCapSelected,
      BmpHorizontalThumbRightCapSelected,
      BmpVerticalThumbSelected,
      BmpVerticalThumbBottomCapSelected,
      BmpVerticalThumbTopCapSelected,      
   };

   ScrollContentCtrl *contentCtrl;  // always have a pointer to the content control

   Int32 borderThickness;           // this gets set per class in the constructor

   // note - it is implicit in the scroll view that the buttons all have the same
   // arrow length and that horizontal and vertical scroll bars have the
   // same thickness

   Int32 scrollBarThickness;        // determined by the width of the vertical page bmp
   Int32 scrollBarArrowBtnLength;   // determined by the height of the up arrow

   bool hBarEnabled;
   bool vBarEnabled;
   bool hasHScrollBar;
   bool hasVScrollBar;

   float hBarThumbPos;
   float hBarThumbWidth;
   float vBarThumbPos;
   float vBarThumbWidth;

   Int32 hThumbSize;
   Int32 hThumbPos;
   
   Int32 vThumbSize;
   Int32 vThumbPos;

   Int32 baseThumbSize;

   RectI upArrowRect;
   RectI downArrowRect;
   RectI leftArrowRect;
   RectI rightArrowRect;
   RectI vTrackRect;
   RectI hTrackRect;

   float line_V;     // percentage to scroll line Vertically
   float line_H;
   float page_V;     // percentage to scroll page Vertically
   float page_H;


   //--------------------------------------
   // for determing hit area   
public:		//called by the ComboPopUp class
   enum Region 
   { 
      UpArrow, 
      DownArrow,
      LeftArrow,
      RightArrow, 
      UpPage, 
      DownPage, 
      LeftPage,
      RightPage,
      VertThumb,
      HorizThumb,
      None,
   };
   Region findHitRegion(const Point2I &);

protected:
   Bool stateDepressed;
   Region curHitRegion;
   void scrollByRegion(Region reg);

   virtual bool calcChildExtents(Point2I *pos, Point2I *ext);
	virtual void calcScrollRects(void);
   void calcThumbs(Point2I cpos, Point2I cext);
   
   //--------------------------------------
   
   Point2I curMousePos;

   //--------------------------------------
   // for mouse dragging the thumb
   float thumbAnchorPos;
   int thumbDelta;
   //--------------------------------------

public:
	bool disabled;

	void setContentBoarder(bool on_off, BYTE color, BYTE selColor, BYTE disColor);
	void setContentOpaque(bool on_off, BYTE color, BYTE selColor, BYTE disColor);

   void scrollTo(float x, float y);
   void computeSizes();

   enum {
      ScrollBarAlwaysOn = 0,
      ScrollBarAlwaysOff = 1,
      ScrollBarDynamic = 2,
   };
   // you can change the bitmap array dynamically.
   void loadBitmapArray();

   Int32 forceHScrollBar;
   Int32 forceVScrollBar;
   Int32 pbaTag;

   bool useConstantHeightThumb;
   Bool willFirstRespond;     // for automatically handling arrow keys

   ScrollCtrl();
   
   ScrollContentCtrl *getScrollContentCtrl() {return contentCtrl;}

   SimObject *addObject(SimObject *object);
   void resize(const Point2I &newPos, const Point2I &newExt);
	Int32 getBorderThickness(void) { return borderThickness; }

   bool wantsTabListMembership();
   bool becomeFirstResponder();
   bool loseFirstResponder();

   bool isDepressed(void) { return stateDepressed; }
   Region getCurHitRegion(void) { return curHitRegion; }

   void onKeyDown(const Event &event);
   void onKeyRepeat(const Event &event);
   void onMouseDown(const Event &event);
   void onMouseRepeat(const Event &event);
   void onMouseUp(const Event &event);
   void onMouseDragged(const Event &event);
   void onWake();
   void editResized();

   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   virtual void drawBorder(GFXSurface *sfc, const Point2I &offset);
   virtual void drawVScrollBar(GFXSurface *sfc, const Point2I &offset);
   virtual void drawHScrollBar(GFXSurface *sfc, const Point2I &offset);
   virtual void drawScrollCorner(GFXSurface *sfc, const Point2I &offset);

   void inspectRead(Inspect *);
   void inspectWrite(Inspect *);

   bool onAdd();

   DECLARE_PERSISTENT(ScrollCtrl);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};   
};

#endif //_SIMGUISCROLLVIEW_H_
