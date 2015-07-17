#ifndef _FXRENDERIMAGE_H_
#define _FXRENDERIMAGE_H_

#include <sim.h>
#include <ts.h>
#include "simRenderGrp.h"

//--------------------------------------------------------------
// FxRenderImage - render image intended for special effects.
//                 Provides support for facing camera, back-
//                 facing one sided shapes, and swaping 
//                 translucent/non-translucent versions of a
//                 shape depending on, say, current render 
//                 context.

// Note on swapping translucent and non-translucent shapes:
// It is assumed that before swapping to one or the other, the
// appropriate shape (see below) has been loaded and the thread
// if needed has been created.  This includes the first time
// one calls "setTranslucent".  The setTranslucent method will
// take care of making sure that the new thread is set to the
// same position as the previously set thread.  If one uses
// the "setTranslucent" method, one should not touch "shape"
// and "thread".  However, if you want to bypass the whole
// translucency issue and just use some shape, you can use
// "shape" and "thread" as long as you never call the
// "setTranslucent" method.

// You can use the "faceCamera" method just once and the shape
// will stay in that position.  Or, you can call it each frame
// and the shape will follow the camera.  If you set a rotation
// axis, the shape will rotate on that axis as it tries to face
// the camera.  This axis will be the shapes z-axis.

class DLLAPI fxRenderImage: public SimRenderImage
{
	Point3F rotationAxis; // if set, faceCamera/direction rotates on this
	                      // axis...this will be z-axis of shape
	bool useRotationAxis;

   float axisSpin;
   bool  useAxisSpin;

	Point3F camAxis;
	float camDist;

	void faceDirection(Point3F & direction, Point3F & _rotationAxis);

public:
	TMat3F transform;

	TSShapeInstance *shape;
	TSShapeInstance::Thread *thread;

	bool drawTranslucent;
	TSShapeInstance *translucentShape;
	TSShapeInstance *nonTranslucentShape;

	TSShapeInstance::Thread *translucentThread;
	TSShapeInstance::Thread *nonTranslucentThread;

	void findCameraAxis(TSRenderContext &rc);
	void faceCamera(TSRenderContext &rc, bool _findCamAxis = true);
	void faceAwayCamera(TSRenderContext &rc, bool _findCamAxis = true);
	void faceDirection(Point3F & direction);
	bool testBackfacing(const Point3F & front);
	void render(TSRenderContext &rc);
	void setRotationAxis(const Point3F & rotationAxis);
   void setAxisSpin(const float in_spin);
   void clearAxisSpin();
	void clearRotationAxis();
	
	const Point3F & getCameraAxis() { return camAxis; }
	float getCameraDistance() { return camDist; }

   float findCameraDistance(TSCamera *);
	void setSortValue(TSCamera *);

	void setTranslucent(bool nv);

	fxRenderImage()
	{
		drawTranslucent = false;
		useRotationAxis = false;
		useAxisSpin = false;
		shape = translucentShape = nonTranslucentShape = 0;
		thread = translucentThread = nonTranslucentThread = 0;
	}
};

#endif

















