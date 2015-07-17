#ifndef _H_SIMGUITSCTRL
#define _H_SIMGUITSCTRL

#include <simTSViewport.h>
#include "simGuiCtrl.h"
#include "sim3DEv.h"
#include "inspect.h"
#include "ts.h"

class SimRenderImage;

namespace SimGui
{

class TSControl : public Control
{
private:
   typedef Control Parent;
protected:
//	typedef TSPerspectiveCamera Camera;
   TMat3F               camTSW;
   TSCamera             *camera;
   TSPerspectiveCamera  *perspectiveCamera;
   TSOrthographicCamera *orthographicCamera;

	SimObject* object;		// Object attached to
	float nearPlane;
	float farPlane;
	float fov;					// Field of view
   RectF worldBounds;
   Point3F camPos;
   ColorF alphaColor;
   float alphaBlend;

   bool produce3DMouseEvents;
   bool hasRenderedOnce;   
	bool unlockSurface;
	TSRenderContext renderContext;
	//
	void pickObject();
   virtual bool setupCamera(const Box2I &visibleBox, const Box2I &boundsBox);
   SortableVectorPtr<SimRenderImage *> translucentRenderImageList;
   Vector<SimRenderImage *> overlayRenderImageList;

   void dispatch3DEvent(Sim3DMouseEvent &me, const Event &evt);

public:
   DECLARE_PERSISTENT( TSControl );

	TSControl();
	~TSControl();
	void setObject(SimObject* object, SimObject *data=NULL);
	SimObject* getObject();
	void setFOV(float ff);
	float getFOV(void);
   TSCamera* getCamera();
   TSRenderContext * getRenderContext();

	virtual bool processEvent(const SimEvent*);
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   void onPreRender();

   void onMouseDown(const Event& evt);
   void onMouseUp(const Event& evt);
   void onMouseDragged(const Event& evt);
   void onMouseMove(const Event& evt);
   void onRightMouseDown(const Event& evt);
   void onRightMouseUp(const Event& evt);
   void onRightMouseDragged(const Event& evt);

   void inspectRead(Inspect *);
   void inspectWrite(Inspect *);

   bool onAdd();
   void onRemove();
   void onDeleteNotify(SimObject *obj);

   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );

   void lockDevice(GFXDevice*);
   void unlockDevice(GFXDevice*);
};

inline void TSControl::setFOV(float ff)
{
	fov = ff;
}

inline float TSControl::getFOV(void)
{
	return fov;
}

inline SimObject* TSControl::getObject()
{
   return ( object );
}

inline TSCamera* TSControl::getCamera()
{
   return camera;
}

inline TSRenderContext* TSControl::getRenderContext()
{
   return &renderContext;
}

};

#endif