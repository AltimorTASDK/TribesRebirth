//------------------------------------------------------------------------------
//
// simMapCanvas.h
//
//------------------------------------------------------------------------------

#ifndef _SIMMAPCANVAS_H_
#define _SIMMAPCANVAS_H_

//------------------------------------------------------------------------------

class SimMapCanvas;

class SimMapWindow : public GWCanvas
{
 public:
   SimMapCanvas *mapCanvas;
	void onMouseMove(int x, int y, UINT keyFlags);
	void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void onLButtonUp(int x, int y, UINT keyFlags);
	void onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void onRButtonUp(int x, int y, UINT flags);
};

//------------------------------------------------------------------------------

class SimMapCanvas : public SimCanvas
{
 private:
   typedef SimCanvas Parent;

 protected:
   typedef enum { Idle, LDown, RDown } MouseState;

   Point2F        worldSize;
   Vector<RectF>  worldRect;
   Point2F        worldViewportSize;
   GFXBitmap     *imageBuffer;
   int            refreshCt;
   MouseState     mouseState;
   RectI          mouseRect;

   virtual void mapOverlay();
   virtual void init();
      
 public:
   SimMapCanvas();
   virtual ~SimMapCanvas();
	virtual bool processEvent(const SimEvent *);
	virtual bool processQuery(SimQuery *);
   virtual void render();
	virtual void onMouseMove(int x, int y, UINT keyFlags);
	virtual void onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	virtual void onLButtonUp(int x, int y, UINT keyFlags);
	virtual void onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags);
	virtual void onRButtonUp(int x, int y, UINT flags);

	void setMapWindow(SimMapWindow *window)
	{ 
// SimCanvas no longer has a setCanvas() member function
//      Parent::setCanvas((GWCanvas *)window);
//      window->mapCanvas = this;
   }
   Point2I screenPosition(const Point2F p);
   Point2I screenPosition(const float x, const float y) { 
      return screenPosition(Point2F(x, y)); 
   }
   Point2F worldPosition(Point2I p);
};

//------------------------------------------------------------------------------

inline void SimMapWindow::onMouseMove(int x, int y, UINT keyFlags)
{
   mapCanvas->onMouseMove(x, y, keyFlags);
}

inline void SimMapWindow::onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   mapCanvas->onLButtonDown(fDoubleClick, x, y, keyFlags);
}

inline void SimMapWindow::onLButtonUp(int x, int y, UINT keyFlags)
{
   mapCanvas->onLButtonUp(x, y, keyFlags);
}

inline void SimMapWindow::onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   mapCanvas->onRButtonDown(fDoubleClick, x, y, keyFlags);
}

inline void SimMapWindow::onRButtonUp(int x, int y, UINT flags)
{
   mapCanvas->onRButtonUp(x, y, flags);
}

#endif 
