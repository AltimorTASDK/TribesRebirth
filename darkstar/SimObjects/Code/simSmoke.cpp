//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <SimSmoke.h>
#include <SimTagDictionary.h>
#include <g_surfac.h>
#include <d_caps.h>
#include <soundFX.h>
#include "simTimerGrp.h"
#include "simResource.h"
#include "simGame.h"
#include "simExplosion.h"

//--------------------------------------------------------------------------- 

Random smokeRand;

SimSmoke::SimSmoke( int _shapeTag, int _tshapeTag, float _smokeOut)
{
   type |= SimExplosion::defaultFxObjectType;
   
	shapeTag = _shapeTag;
	translucentShapeTag = _tshapeTag;
	smokeOut = smokeDuration = _smokeOut;

   pos.set(0, 0, 0);
	vel.set(0, 0, 0);

	renderedYet = false;
	faceCam = false;
	followCam = false;
}

void SimSmoke::setPosition(const Point3F & _pos)
{ 
	pos = _pos;
}

void SimSmoke::setVelocity(const Point3F & _vel)
{
	vel = _vel;
}

void SimSmoke::setDuration(float _time)
{ 
	smokeOut = smokeDuration = _time;
	if (manager)
		smokeOut += manager->getCurrentTime();
}

//--------------------------------------------------------------------------- 

bool SimSmoke::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
		onEvent(SimTimerEvent);
   }
   return false;
}

//------------------------------------------------------------------------------
bool SimSmoke::onAdd()
{
	if (!Parent::onAdd())
		return false;
	// don't know shape size yet, fake it for now
	bbox.fMin.set(-1,-1,-1);
	bbox.fMax.set(1,1,1);
	if (!faceCam && !followCam)
		image.transform.identity();
	image.transform.p = pos;
	image.transform.flags |= TMat3F::Matrix_HasTranslation; // will keep this flag setting
	updateBBox(image.transform.p);

   // calculate the rotation
   image.setAxisSpin(smokeRand.getFloat((float)M_2PI));

	// add to timer set
	addToSet(SimTimerSetId);

	// add to container db so we get render query
	SimContainer *root = NULL;
   
   root = findObject(manager, SimRootContainerId,root);
	root->addObject(this);

	// when to quit?
	prevAnimTime = manager->getCurrentTime(); // measure smoke duration from here
	smokeOut += prevAnimTime;
	return true;
}

bool SimSmoke::onSimTimerEvent(const SimTimerEvent *event)
{
	if ((float)manager->getCurrentTime()>smokeOut)
	{
		deleteObject();
		return true;
	}

	// update position of smoke (all is in world-space coords)
	Point3F dv = vel;
	dv *= (float) (event->timerInterval);
	image.transform.p += dv;
	updateBBox(image.transform.p);

	return true;
}

//------------------------------------------------------------------------------
void SimSmoke::onRemove()
{
	// remove from whatever container we're in
	if (getContainer())
		getContainer()->removeObject(this);

   if (image.translucentShape)
		delete image.translucentShape;
   if (image.nonTranslucentShape)
		delete image.nonTranslucentShape;

	Parent::onRemove();
}

//------------------------------------------------------------------------------

bool SimSmoke::processQuery(SimQuery *query)
{
   switch (query->type)
   {
      case SimRenderQueryImageType: 
      {
			if ((float)manager->getCurrentTime()>smokeOut)
				return false;

     		SimRenderQueryImage *qp = static_cast<SimRenderQueryImage *>(query);
			TSRenderContext & rc = *qp->renderContext;
			GFXSurface *srf = rc.getSurface();

			// shall we draw translucent?
			bool translucent = (srf->getCaps()&GFX_DEVCAP_SUPPORTS_CONST_ALPHA || 
			                    SimExplosion::SoftwareTranslucency ||
				                 shapeTag==-1)
				                && translucentShapeTag!=-1;

			if (!renderedYet || translucent!=image.drawTranslucent)
			{
				// need to switch modes
				if (translucent && !image.translucentShape)
					// need to load translucent shape
					getShape(translucentShapeTag,
					         &image.translucentShape,
					         &image.translucentThread);
				else if (!translucent && !image.nonTranslucentShape)
					// need to load non-translucent shape
					getShape(shapeTag,
					         &image.nonTranslucentShape,
					         &image.nonTranslucentThread);

				image.setTranslucent(translucent);
			}

			// find axis from camera to object if first render
			if ( (!renderedYet && faceCam) || followCam || SimGame::isRecording)
				image.faceCamera(rc);

		   // advance the animation
		   float now = manager->getCurrentTime();
		   image.thread->AdvanceTime(now-prevAnimTime);
			prevAnimTime = now;

			// get haze value from container
			SimContainer * cnt = getContainer();
			if (cnt)
				image.hazeValue = cnt->getHazeValue(rc, image.transform.p);

         image.setSortValue(rc.getCamera());
     		qp->count = 1;
     		qp->image[0] = &image;

			renderedYet = true;

         return true;
      }
   }
   return false;
}

//--------------------------------------------------------------------------- 

void SimSmoke::getShape(int shapeTag,
                                TSShapeInstance **ppShape,
                                TSShapeInstance::Thread **ppThread)
{
   // Read shape, initialize thread
   ResourceManager *rm = SimResource::get(manager);
	const char* shapeName = SimTagDictionary::getString(manager,shapeTag);
   hShape = rm->load(shapeName);
   AssertFatal((bool)hShape,"Unable to load smoke shape");
   
   (*ppShape) = new TS::ShapeInstance(hShape, *rm);
   AssertFatal((bool)(*ppShape),"Could not create smoke shape instance");

   (*ppThread) = (*ppShape)->CreateThread(); // pos 0, seq 0 by default
	AssertFatal(*ppThread,"Could not create smoke animation thread");
	(*ppThread)->setTimeScale((*ppThread)->getSequence().fDuration/smokeDuration);
	(*ppShape)->animateRoot(); // never have to do this again

	// set-up shape space bounding box -- real size
	bbox.fMin  = (*ppShape)->getShape().fCenter;
	bbox.fMin += (*ppShape)->fRootDeltaTransform.p; // could be trouble if root has rot.
	bbox.fMax  = bbox.fMin;
	bbox.fMin -= (*ppShape)->getShape().fRadius;
	bbox.fMax += (*ppShape)->getShape().fRadius;
}

//------------------------------------------------------------------------------

void SimSmoke::updateBBox(const Point3F & p)
{
	Box3F box;
	box = bbox;
	box.fMin += p;
	box.fMax += p;
	setBoundingBox(box);
}





