//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <sim.h>
#include <ml.h>
#include "g_surfac.h"
#include "simTSViewport.h"
#include "simRenderGrp.h"
#include "simLightGrp.h"
#include "console.h"
#include "ts_PointArray.h"
#include "simCamera.h"
#include "simContainer.h"
#include "simAction.h"

static TSPointArray  DefaultPointArray;

//----------------------------------------------------------------------------

SimTSViewport::SimTSViewport()
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
	clearColor = 2;
	object = 0;
	//canvas = 0;
	fov = 0.6f;
	nearPlane = 1.0f;
	farPlane = 1.0E7f;
}


//------------------------------------------------------------------------------
SimTSViewport::~SimTSViewport()
{
   delete perspectiveCamera;
   delete orthographicCamera;
}   

//----------------------------------------------------------------------------

void SimTSViewport::setupCamera(Point2I &size)
{
	// Query attached object for camera info
	if (!object)
		pickObject();
	if (object) 
	{
 	   SimCameraQueryInfo query;
      query.cameraInfo.projectionType = SimPerspectiveProjection;
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
			query.cameraInfo.tmat.inverse();
			camera->setTWC(query.cameraInfo.tmat);
		}
		else
			setObject(0);
	}
   else {
      TMat3F m;
      m.identity();
      camera->setTWC(m);
   }

   if (camera == perspectiveCamera)
   {
   	// Make sure the camera is synced with the canvas size
   	// Update the tscamera
   	float wwidth  = nearPlane * tan(fov);
   	float wheight = float(size.y) / float(size.x) * wwidth;
   	RectF rect;
   	rect.upperL.set(-wwidth,wheight);
   	rect.lowerR.set(wwidth,-wheight);
   	camera->setWorldViewport(rect);
   }
   else
   	camera->setWorldViewport(worldBounds);

	camera->setScreenViewport(RectI(Point2I(0,0),Point2I(size.x,size.y)));
	camera->setNearDist(nearPlane);
	camera->setFarDist(farPlane);
}


//----------------------------------------------------------------------------

void SimTSViewport::render( SimCanvas *sCanvas )
{
	if (!(sCanvas && sCanvas->getCanvas()) ) return;

   GWCanvas *pCanvas = sCanvas->getCanvas();
   setupCamera( pCanvas->getClientSize() );

   // Get the scene lighting information
   TSSceneLighting *sceneLights;
   SimLightSet *simLightSet = 
      dynamic_cast<SimLightSet *>(manager->findObject(SimLightSetId));
   if (simLightSet)
      sceneLights = simLightSet->getSceneLighting(camera->getTCW().p);
   else
      // if no light set exists, create an empty scene lighting
      sceneLights = new TSSceneLighting;
      
	// Setup the render context
	renderContext.setCamera(camera);
	renderContext.setPointArray(&DefaultPointArray);
	renderContext.setSurface(pCanvas->getSurface());
	renderContext.setLights(sceneLights);

	// Lock
	renderContext.lock();

   // Clear the background before render
	if (clearColor >= 0)
	{
		pCanvas->getSurface()->draw3DBegin();
		pCanvas->getSurface()->clear(clearColor);
		pCanvas->getSurface()->draw3DEnd();
	}
	
	// Render through the container render context
	SimContainerRenderContext cContext;
	cContext.lock(sCanvas,&renderContext);

	// Render items in the RenderSet first
   SimRenderSet* SRgrp = findObject(manager,SimRenderSetId,(SimRenderSet*)0);
	if (SRgrp != 0)
		for (SimRenderSet::iterator ptr = SRgrp->begin(); ptr != SRgrp->end(); ptr++)
			cContext.render(*ptr);

	// Render items in the containers
	SimContainer* root = findObject(manager,SimRootContainerId,(SimContainer*)0);
	if (root)
		root->render(cContext);

	cContext.unlock();

	// Unlock
	renderContext.unlock();
   
   if (!simLightSet)
      delete sceneLights;
}


//----------------------------------------------------------------------------
// Pick an object to attach too from the camera set
//
void SimTSViewport::pickObject()
{
   SimObject* obj;
   SimCameraSet* grp;
	if ( (obj = manager->findObject(SimCameraSetId)) != NULL )
		if ( (grp = dynamic_cast<SimCameraSet*>(obj)) != NULL )
			if (!grp->empty()) {
				if (object) {
					SimCameraSet::iterator ptr = 
						grp->find(grp->begin(),grp->end(),object);
					if (ptr != grp->end() && ++ptr != grp->end()) {
						setObject(*ptr);
						return;
					}
				}
				setObject(grp->front());
				return;
			}
	setObject(0);
}	

//----------------------------------------------------------------------------

bool SimTSViewport::processArguments(int argc, const char **argv)
{
   CMDConsole *console = CMDConsole::getLocked();

	if (argc > 0) 
	{
		SimCanvas* canvas = 0;
      SimObject* obj;
		if ( (obj = manager->findObject(argv[0])) != NULL )
			canvas = dynamic_cast<SimCanvas*>(obj);
		if (canvas) 
		{
			canvas->addObject(this);
			if(argc > 1)
				setClearColor(atoi(argv[1]));
         return true;
		}
		else
			console->printf("SimTSViewport: Could not find canvas");
	}
	else
		console->printf("SimTSViewport: "
			"canvasName [backgroundColor]");
	return false;
}

//----------------------------------------------------------------------------

bool SimTSViewport::processEvent(const SimEvent* event)
{
	switch (event->type) {
		case SimDeleteNotifyEventType:
			if (object == ((SimDeleteNotifyEvent*)event)->object)
				// Object pointer no longer valid
				object = 0;
			break;
		case SimAddNotifyEventType:
			// Automatically create camera set
			if (!manager->findObject(SimCameraSetId))
				manager->addObject(new SimCameraSet());
			break;
		case SimRemoveNotifyEventType:
			setObject(0);
			break;
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
			break;
		}
	}
	// Give the parent class a chance to process events
	return SimViewport::processEvent(event);
}	

bool SimTSViewport::processQuery(SimQuery* query)
{
	return SimViewport::processQuery(query);
}

inline void SimTSViewport::setObject(SimObject* newobj)
{
   if ( newobj == object ) return;

   SimCameraMountEvent me;
	if (object) {
		clearNotify(object);
      SimCameraUnmountEvent ume;
      object->processEvent(&ume);
      me.prevCamera = object;
   }
	if (newobj && !newobj->isDeleted()) {
		deleteNotify(newobj);
		object = newobj;
      object->processEvent(&me);
	}
	else
		object = 0;
}

