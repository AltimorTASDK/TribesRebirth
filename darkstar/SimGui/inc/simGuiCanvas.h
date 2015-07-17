#ifndef _H_SIMGUICANVAS
#define _H_SIMGUICANVAS

#include "simCanvas.h"
#include "simGuiBase.h"
#include <g_barray.h>

#define WINKEYBOARD
#define DEFAULT_FRAME_TIME 0.5f

namespace GWMouseEvent
{
   enum { 
      MouseMove, LButtonDown, LButtonUp, 
      RButtonDown, RButtonUp
   };
};

namespace SimGui
{

class DLLAPI Delegate;
class Control;
class HelpCtrl;

struct Cursor 
{
   Int32                    simTag;
   Resource<GFXBitmap>      hBmp;
   Resource<GFXBitmapArray> rsBMA;
   Point2I                  hotspot;
   float                    rFrameTime;
   int                      iFrame;
};

class DLLAPI Canvas : public SimCanvas, public Responder
{
private:
   typedef SimCanvas Parent;

protected:
   bool cursorON;
   bool cursorOverApp;
   bool cursorVisible;
   bool forwardMouseMove;
   int  skipNextDelta;
   bool useWindowsMouseEvents;

#ifdef WINKEYBOARD
   bool useWindowsKeyboardEvents;
   BYTE lastAscii;
   BYTE lastDIKCode;
#endif

   RectI oldUpdateRects[2];
   RectI curUpdateRect;

   float rLastFrameTime;
   bool hasFocus;
   Point2F cursorPt;
   Point2I lastCursorPt;
   bool lastCursorON;
   RectI canvasUpdateRegion;

   Cursor defaultCursor;   // default cursor used
   Cursor controlCursor;   // can be set by other gui controls to override the default cursor
   Cursor *lastCursor;

   float pixelsPerMickey;
   float limit(float val, float m);
   bool  insideLimit(float val, float m);

   Delegate *delegate;
   SimCanvas *canvas;
   Responder *firstResponder;
   Control *editControl;
   Control *mouseCapturedControl;
   Control *mouseControl; // the control the mouse was last seen in unless some other on captured it
	bool mouseControlClicked;

	//used by the auto-help
	DWORD		prevMouseTime;		//this determines how long the mouse has been in the same control
	HelpCtrl	*helpCtrl;

   Point2F ptMouse;
   bool mouseButtonDown;
   bool mouseRightButtonDown;
   Vector<Control *> tabList;
   Vector<Control *> contentControlStack;
   Control *topContentControl;
   DWORD nextKeyTime;
   DWORD nextMouseTime;

   Event lastKeyEvent;
   Event lastMouseEvent;

	BYTE lastMouseDownCount;
	DWORD lastMouseDownTime;
	bool	leftMouseLast;

   DWORD initialMouseDelay;
   DWORD initialKeyDelay;
   DWORD repeatMouseDelay;
   DWORD repeatKeyDelay;

   bool willCaptureAllInputs;

   void setCursor(Cursor &cur, Int32 simTag);

	virtual void onSysCommand(UINT cmd, int x, int y);
   

public:
   static bool canvasCursorTrapped;

public:
   Canvas();
   ~Canvas();

   void preRender();
   void render();
   void paint();

   bool onAdd();
   bool processEvent(const SimEvent *event);
   bool processQuery(SimQuery *query);
   void showCursor( bool show ){ cursorVisible = show; };
   void forwardMouse( bool forward ){ forwardMouseMove = forward; };
   
   void processMouseEvent(int x, int y, int type);

   void flip();
   void addUpdateRegion(Point2I pos, Point2I ext);
   void resetUpdateRegions();
   void buildUpdateUnion(RectI *updateUnion);

   // for specifying and getting characteristics of the default cursor
   void setCursor(Resource<GFXBitmapArray>, Point2I hotspot = Point2I(0, 0), float rFrameTime = DEFAULT_FRAME_TIME);
   void setCursor(Resource<GFXBitmap> hBmp, Point2I hotspot = Point2I(0,0));
   void setCursor(Int32 cursorTag)        { setCursor(defaultCursor, cursorTag); }
   Resource<GFXBitmap> getCursorBmp()     { return defaultCursor.hBmp; }

   // general cursor commands
   bool isCursorON() {return cursorON; }
   void setCursorON(bool onOff);
   void setCursorPos(const Point2I &pt)   { cursorPt.x = float(pt.x); cursorPt.y = float(pt.y); }
   Point2I getCursorPos()                 { return Point2I(cursorPt.x, cursorPt.y); }

	HelpCtrl *getHelpCtrl(void) { return helpCtrl; }
	void setHelpCtrl(HelpCtrl *ctrl);

   virtual void  onMouseMove(const Event &event);
   virtual void  onMouseMove(int x, int y, UINT keyFlags);
   virtual UINT  onNCHitTest(int x, int y);
   virtual void  onEnterMenuLoop( bool trackPopupMenu );
   virtual void  onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
   virtual void  onLButtonUp(int x, int y, UINT keyFlags);
   virtual void  onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
   virtual void  onRButtonUp(int x, int y, UINT flags);


   void onActivateApp(BOOL, DWORD);

   void setUseWindowsMouseEvents(bool use);
#ifdef WINKEYBOARD
   void setUseWindowsKeyboardEvents(bool use);
   virtual void  onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags);
#endif

public:
   bool onSimInputEvent(const SimInputEvent *event);

   void render (SimCanvas *canvas);
   void mouseLock(Control *lockingControl);
   Control* getMouseControl()       { return mouseControl; }
   Control* getMouseLockedControl() { return mouseCapturedControl; }
   void mouseUnlock();

   void setContentControl(Control *control, Delegate *newDlgt = NULL, bool firstTime = TRUE, const char *dlgtName = NULL);
   Control* getContentControl();

   void makeFirstResponder(Control *control);
   Responder* getFirstResponder() { return firstResponder; }

   void setDelegate(Delegate *del);
   Delegate* getDelegate();

	void surfaceChanged();

   void rootMouseDown(const Event &event);
   void rootMouseUp(const Event &event);
   void rootMouseMove(const Event &event);
   void rootMouseDragged(const Event &event);

   void rootRightMouseDown(const Event &event);
   void rootRightMouseUp(const Event &event);
   void rootRightMouseDragged(const Event &event);

   void resetMouse();

   void onKeyDown(const Event &event);

   void editMode(bool mode);

   void tabNext();
   void tabPrev();

   bool inEditMode() {return editControl != NULL; }
   void invalidateControl(Control *ctrl);

   void addTabs(Control *ctrl);
   void rebuildTabList(void);

   DWORD onMessage(SimObject *sender, DWORD msg); // root handles IDGUI and IDCMD messages
   void handleGUIMessage(const char *guiName);

   void pushDialogControl(Control *ctrl, Delegate *dlgDelegate = NULL);
   void popDialogControl();
   Control * getTopDialog();
   int getNumDialogs();
	Control* getDialogNumber(int stackNumber);
	int findDialogNumber(Control *ctrl);
   void handleDLGClose(Int32 message);
   void handleDLGMessage(const char *gui, Int32 message);

   void captureAllInputs(bool tf) {willCaptureAllInputs = tf;}

   // From GWCanvas...
   void lock();
   void unlock();

   DECLARE_PERSISTENT(Canvas);
};

inline int Canvas::getNumDialogs()
{
   return ((contentControlStack.size() <= 1) ? 0 : contentControlStack.size() - 1);
}

inline float Canvas::limit(float val, float m)
{
   if (val > m)
      return (m);
   if (val < 0.0f)
      return (0.0f);
   return (val);
}   

inline bool Canvas::insideLimit(float val, float m)
{
   if (val > m)
      return false;
   if (val < 0.0f)
      return false;
   return true;
}   

};

#endif