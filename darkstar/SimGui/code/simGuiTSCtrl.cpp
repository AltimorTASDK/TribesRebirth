//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma option -Jg
#endif

#include <sim.h>
#include <ml.h>
#include "g_surfac.h"
#include "g_pal.h"
#include "simGuiTSCtrl.h"
#include "simRenderGrp.h"
#include "simLightGrp.h"
#include "console.h"
#include "ts_PointArray.h"
#include "simCamera.h"
#include "simContainer.h"
#include "sim3DEv.h"
#include "inspect.h"
#include "editor.strings.h"
#include "gwDeviceManager.h"
#include "gfxMetrics.h"

static TSPointArray  DefaultPointArray;

namespace SimGui
{
//----------------------------------------------------------------------------

TSControl::TSControl()
{
   perspectiveCamera = new TSPerspectiveCamera(
	   // The constructor viewport and world Rects overwritten later,
   	// doesn't really matter what they are.
	   RectI(Point2I(0,0),Point2I(640-1,480-1)),
   	RectF(Point2F(-320.0f,240.0f),Point2F(320.0f,-240.0f)),
	   256.0f, 1.0E8f);
   
   orthographicCamera = new TSOrthographicCamera(
	   RectI(Point2I(0,0),Point2I(640-1,480-1)),
   	RectF(Point2F(-320.0f,240.0f),Point2F(320.0f,-240.0f)),
	   256.0f, 1.0E8f);

   camera = perspectiveCamera;
   
	unlockSurface = false;
	object = 0;
	//canvas = 0;
	fov = 0.6f;
	nearPlane = 1.0f;
	farPlane = 1.0E7f;
   produce3DMouseEvents = true;
   hasRenderedOnce = false;
}


//------------------------------------------------------------------------------
TSControl::~TSControl()
{
   delete perspectiveCamera;
   delete orthographicCamera;
}   

//----------------------------------------------------------------------------

bool TSControl::setupCamera(const Box2I &visibleBox, const Box2I &boundsBox)
{
	// Query attached object for camera info
 	
 	SimCameraQuery query;
   query.cameraInfo.projectionType = SimPerspectiveProjection;
   query.cameraInfo.alphaBlend = 0;
	if (object->processQuery(&query)) 
	{
      if (query.cameraInfo.projectionType == SimPerspectiveProjection)
         camera = perspectiveCamera;
      else
         camera = orthographicCamera;                  
		nearPlane   = query.cameraInfo.nearPlane;
		farPlane    = query.cameraInfo.farPlane;
		fov         = query.cameraInfo.fov;
      worldBounds = query.cameraInfo.worldBounds;
      camPos = query.cameraInfo.tmat.p;
		query.cameraInfo.tmat.inverse();
		camera->setTWC(query.cameraInfo.tmat);
	}
	else {
      setObject(0);
      return false;
	}

   alphaColor = query.cameraInfo.alphaColor;
   alphaBlend = query.cameraInfo.alphaBlend;

   Point2I size(boundsBox.len_x(), boundsBox.len_y());
  	RectF rect;

   if (camera == perspectiveCamera)
   {
   	// Make sure the camera is synced with the canvas size
   	// Update the tscamera
   	float wwidth  = nearPlane * tan(fov);
   	float wheight = float(size.y) / float(size.x) * wwidth;

      int left = visibleBox.fMin.x - boundsBox.fMin.x;
      int right = visibleBox.fMax.x - boundsBox.fMin.x;
      int top = visibleBox.fMin.y - boundsBox.fMin.y;
      int bottom = visibleBox.fMax.y - boundsBox.fMin.y;

      float hscale = wwidth * 2 / float(boundsBox.len_x());
      float vscale = wheight * 2 / float(boundsBox.len_y());

      rect.upperL.set(-wwidth + hscale * left, wheight - vscale * top);
      rect.lowerR.set(-wwidth + hscale * right, wheight - vscale * bottom);
   	camera->setWorldViewport(rect);
   }
   else
   {
      int left = visibleBox.fMin.x - boundsBox.fMin.x;
      int right = visibleBox.fMax.x - boundsBox.fMin.x;
      int top = visibleBox.fMin.y - boundsBox.fMin.y;
      int bottom = visibleBox.fMax.y - boundsBox.fMin.y;

      float hscale = worldBounds.len_x() / float(boundsBox.len_x());
      float vscale = worldBounds.len_y() / float(boundsBox.len_y());

      rect.upperL.set(worldBounds.upperL.x + left * hscale, worldBounds.upperL.y + top * vscale);
      rect.lowerR.set(worldBounds.upperL.x + right * hscale, worldBounds.upperL.y + bottom * vscale);

   	camera->setWorldViewport(rect);
   }
	camera->setScreenViewport(RectI(visibleBox.fMin,visibleBox.fMax));
	camera->setNearDist(nearPlane);
	camera->setFarDist(farPlane);
   return true;
}

void TSControl::dispatch3DEvent(Sim3DMouseEvent &me, const Event &evt)
{
   if(!camera || !hasRenderedOnce)
      return;
   SimObject *inputSet = manager->findObject( SimInputConsumerSetId );
   if (!inputSet)
      return;

   me.modifier = evt.modifier;
   me.sender = this;
   Point2I curPt = globalToLocalCoord(evt.ptMouse);
   Point4F vs(evt.ptMouse.x, evt.ptMouse.y, 0, 1.0f);
   Point4F ss;
   
   camera->transformProject2Inverse(vs, &ss);
   m_mul(ss, camTSW, &me.direction);
   me.startPt = camPos;
   me.direction -= me.startPt;
   me.direction.normalize();

   inputSet->processEvent(&me);
}

//----------------------------------------------------------------------------
void TSControl::onPreRender()
{
   setUpdate();
}

void TSControl::onRender(GFXSurface*  sfc,
                         Point2I      offset,
                         const Box2I& updateRect)
{
	if (!object)
		pickObject();

   Point2I upperL = localToGlobalCoord(Point2I(0,0));
   Point2I lowerR = extent;
   lowerR += upperL;

   Box2I bounds;
   bounds.fMin = upperL;
   bounds.fMax = lowerR;

   if(!object || !setupCamera( updateRect, bounds ))
   {
      sfc->drawRect2d_f (&RectI(upperL, lowerR), 0);
      return;
   }

   hasRenderedOnce = true;
   if(produce3DMouseEvents)
   {
      camera->lock();
      m_mul(camera->getTWC(), camera->getTCS(), &camTSW);
      camTSW.inverse();
      camera->unlock();
   }

   // Get the scene lighting information
   TSSceneLighting *sceneLights;
   SimLightSet *simLightSet = (SimLightSet *)(manager->findObject(SimLightSetId));
   sceneLights = simLightSet->getSceneLighting(camera->getTCW().p);
      
	// Setup the render context
	renderContext.setCamera(camera);
	renderContext.setPointArray(&DefaultPointArray);
   DefaultPointArray.setOrtho(camera == orthographicCamera);
	renderContext.setSurface(sfc);
	renderContext.setLights(sceneLights);

	renderContext.lock();

   sfc->setZMode(true); //sfc->setZMode(camera->getCameraType() == TS::PerspectiveCameraType);
   sfc->setSurfaceAlphaBlend(&alphaColor, alphaBlend);
	// Render through the container render context
	SimContainerRenderContext cContext;
	cContext.lock(this,&renderContext);

	// Render items in the RenderSet first
   //
   cContext.notRenderingContainers = true;
   
   SimRenderSet* SRgrp = ::findObject(manager,SimRenderSetId,(SimRenderSet*)0);
	if (SRgrp != 0)
		for (SimRenderSet::iterator ptr = SRgrp->begin(); ptr != SRgrp->end(); ptr++)
			cContext.render(*ptr);

   cContext.notRenderingContainers = false;

	// Render items in the containers
	SimContainer* root = ::findObject(manager,SimRootContainerId,(SimContainer*)0);
	if (root)
		root->render(cContext);

	// Unlock
	cContext.unlock();
	renderContext.unlock();
   
   Parent::onRender(sfc, offset, updateRect);
}


//----------------------------------------------------------------------------
// Pick an object to attach too from the camera set
//
void TSControl::pickObject()
{
   SimObject* obj;
   SimCameraSet* set;
	if ( (obj = manager->findObject(SimCameraSetId)) != NULL )
		if ( (set = dynamic_cast<SimCameraSet*>(obj)) != NULL )
			if (!set->empty()) {
				if (object) {
					SimCameraSet::iterator ptr = 
						set->find(set->begin(),set->end(),object);
					if (ptr != set->end() && ++ptr != set->end()) {
						setObject(*ptr);
						return;
					}
				}
				setObject(set->front());
				return;
			}
	setObject(0);
}	

//----------------------------------------------------------------------------

void TSControl::onDeleteNotify(SimObject *obj)
{
   if(object == obj)
      object = 0;
   Parent::onDeleteNotify(obj);
}

bool TSControl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   char buf[20];
   sprintf(buf, "TS_%d", tag);
   manager->assignName(this, buf);
	if (!manager->findObject(SimCameraSetId))
		manager->addObject(new SimCameraSet());

   return true;
}

void TSControl::onRemove()
{
   setObject(0);
   Parent::onRemove();
}

bool TSControl::processEvent(const SimEvent* event)
{
	switch (event->type) {
		case SimActionEventType: {
			const SimActionEvent* ev = static_cast<const SimActionEvent*>(event);
			switch (ev->action) {
				case ActionNext:
					pickObject();
					break;
				case ActionAttach: {
					SimObject* obj = manager->findObject(int(ev->fValue));
					if (obj)
						setObject(obj);
					break;
				}
			}
         return true;
		}
      default:
      	return Parent::processEvent(event);
	}
}	

void TSControl::setObject(SimObject* newobj, SimObject *data)
{
   if (newobj && newobj == object) {
      SimCameraRemountEvent rme;
      rme.data = data;
      object->processEvent(&rme);
      return;
   }

   SimCameraMountEvent me;
	if (object) {
      me.prevCamera = object;
		clearNotify(object);
      SimCameraUnmountEvent ume;
      object->processEvent(&ume);
   }
   me.data = data;   // object of interest, path object etc...
	if (newobj && !newobj->isDeleted()) {
		deleteNotify(newobj);
		object = newobj;
      object->processEvent(&me);
	}
	else
		object = 0;
}

void TSControl::onMouseDown(const Event &evt)
{
   if(!produce3DMouseEvents)
      return;
   Sim3DMouseEvent mouseEvent;
   mouseEvent.meType = Sim3DMouseEvent::MouseDown;
   dispatch3DEvent(mouseEvent, evt);
}

void TSControl::onMouseUp(const Event &evt)
{
   if(!produce3DMouseEvents)
      return;
   Sim3DMouseEvent mouseEvent;
   mouseEvent.meType = Sim3DMouseEvent::MouseUp;
   dispatch3DEvent(mouseEvent, evt);
}

void TSControl::onMouseMove(const Event &evt)
{
   if(!produce3DMouseEvents)
      return;
   Sim3DMouseEvent mouseEvent;
   mouseEvent.meType = Sim3DMouseEvent::MouseMove;
   dispatch3DEvent(mouseEvent, evt);
}

void TSControl::onMouseDragged(const Event &evt)
{
   if(!produce3DMouseEvents)
      return;
   Sim3DMouseEvent mouseEvent;
   mouseEvent.meType = Sim3DMouseEvent::MouseDragged;
   dispatch3DEvent(mouseEvent, evt);
}

void TSControl::onRightMouseDown(const Event &evt)
{
   if(!produce3DMouseEvents)
      return;
   Sim3DMouseEvent mouseEvent;
   mouseEvent.meType = Sim3DMouseEvent::RightMouseDown;
   dispatch3DEvent(mouseEvent, evt);
}

void TSControl::onRightMouseUp(const Event &evt)
{
   if(!produce3DMouseEvents)
      return;
   Sim3DMouseEvent mouseEvent;
   mouseEvent.meType = Sim3DMouseEvent::RightMouseUp;
   dispatch3DEvent(mouseEvent, evt);
}

void TSControl::onRightMouseDragged(const Event &evt)
{
   if(!produce3DMouseEvents)
      return;
   Sim3DMouseEvent mouseEvent;
   mouseEvent.meType = Sim3DMouseEvent::RightMouseDragged;
   dispatch3DEvent(mouseEvent, evt);
}

void TSControl::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);

   insp->write(IDITG_PRODUCE_3D_EVENTS, produce3DMouseEvents);
}   

void TSControl::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);

   insp->read(IDITG_PRODUCE_3D_EVENTS, produce3DMouseEvents);
}   

Persistent::Base::Error TSControl::write( StreamIO &sio, int version, int user )
{
   sio.write(produce3DMouseEvents);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error TSControl::read( StreamIO &sio, int version, int user)
{
   sio.read(&produce3DMouseEvents);

   return Parent::read(sio, version, user);
}

void
TSControl::lockDevice(GFXDevice* io_pDev)
{
	// Lock, determine if we are the content control.  If we are, check the lowRes3D
   //  pref, and lock the render context in double mode if so...
   bool doubleLock = false;
   GFXPrefInfo* gp = deviceManager.getPrefs();
   AssertFatal(gp, "No prefs?");
   
   if (gp->useLowRes3D == true && io_pDev->getSurface() && 
       io_pDev->getSurface()->supportsDoubleLock() == true)
      doubleLock = true;

   io_pDev->lock(doubleLock ? GFX_DOUBLELOCK : GFX_NORMALLOCK);
}

void
TSControl::unlockDevice(GFXDevice* io_pDev)
{
   io_pDev->unlock();
}
 
};
