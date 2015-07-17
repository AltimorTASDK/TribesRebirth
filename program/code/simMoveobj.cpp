//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 


#include <sim.h>
#pragma warn -inl
#pragma warn -aus


#include "simAction.h"
#include "simMoveobj.h"
#include "fear.strings.h"
#include "simResource.h"
#include "simterrain.h"
#include "FearDcl.h"
#include "sky.h"

#define MAX_SPEED			1.0
#define MAX_ROTATION		(1/10)

#define RADIUS			0.5f
#define FOCALLENGTH	0.1f
	

//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT(SimMoveObject);

SimMoveObject::SimMoveObject()
{
	lastUpdate = 0;
	movement.pos.set(0,0,0);
	movement.vel.set(0,0,0);
	rotation.pos.set(-.65,0,1.81);
	rotation.vel.set(0,0,0);
	throttle.set(0,0,0);
	desired_throttle.set(0,0,0);

	collisionMask = -1;
	collisionImage.radius = RADIUS;
	collisionImage.center.set(0.0f,0.0f,0.0f);
	boundingBox.fMin.set(-RADIUS/2.0f,-RADIUS/2.0f,-RADIUS/2.0f);
	boundingBox.fMax.set(+RADIUS/2.0f,+RADIUS/2.0f,+RADIUS/2.0f);
}

SimMoveObject::~SimMoveObject()
{
}


//--------------------------------------------------------------------------- 

void SimMoveObject::update(SimTime dt)
{
	if (dt < 0.033f)
		dt = 0.033f;
	// Update position and orientation
	movement.update(dt);
	rotation.update(dt);
	TMat3F mat;
	mat.set(EulerF(rotation.pos.x,rotation.pos.y,rotation.pos.z),movement.pos);

	// Collision testing
	setPosition(mat);
	movement.pos = getTransform().p;

	// Use throttle to calculate new velocity
	RMat3F mrot;
	mrot.set(EulerF(rotation.pos.x,rotation.pos.y,rotation.pos.z));
	Vector3F vel = throttle;
	vel *= MAX_SPEED;
	m_mul(vel,mrot,&movement.vel);
	throttle = desired_throttle;
	lastUpdate = manager->getCurrentTime();
}


//--------------------------------------------------------------------------- 

const Point3F &SimMoveObject::getPos (void)
{
	return getTransform().p;
}

void SimMoveObject::setPos(Point3F pos)
{
	// Not a good example of how to do this.
	TMat3F mat = getTransform();
	mat.p = pos;
	setPosition(mat);
	movement.pos = pos;
}


void SimMoveObject::setActionMap(const char* file)
{
	ResourceManager *rm = SimResource::get(manager);
   actionMap = rm->load(file);
}


//--------------------------------------------------------------------------- 

bool SimMoveObject::processArguments(int argc, const char **argv)
{
   if (argc >= 1)
   	setActionMap(argv[0]);

   if (argc >= 2) {
      float f[3];
   	memset(f,0,sizeof(f));
      for (int i = 0; i < argc-1  &&  i < 3; i++)
         sscanf(argv[i+1], "%f", &f[i]);
   	setPos(Point3F(f[0], f[1], f[2]));
   }
   return true;
}

bool SimMoveObject::onAdd()
{
   if (Parent::onAdd() == false) 
      return false;

	lastUpdate = manager->getCurrentTime();
	addToSet(SimCameraSetId);
	addToSet(SimFrameEndNotifySetId);
   
	SimContainer* root = findObject(manager,SimRootContainerId,root);
	root->addObject(this);
   
   return true;
}

bool SimMoveObject::processEvent(const SimEvent* event)
{
	switch (event->type) {
		case SimTimerEventType:
			// Update every 1/30 sec.
			update(static_cast<const SimTimerEvent*>(event)->timerInterval);
			return true;
		case SimFrameEndNotifyEventType:
			update(manager->getCurrentTime() - lastUpdate);
			break;
		case SimCollisionEventType:
			//??
			return true;
		case SimGainFocusEventType:
			if (bool(actionMap))
				SimActionHandler::find (manager)->push(this,actionMap);
			return true;
		case SimLoseFocusEventType:
			if (bool(actionMap))
				SimActionHandler::find (manager)->pop(this,actionMap);
			return true;
		case SimObjectTransformEventType: {
			const SimObjectTransformEvent* ev = 
				static_cast<const SimObjectTransformEvent*>(event);
			setPos(ev->tmat.p);
         return true;
		}
		case SimActionEventType: {
			const SimActionEvent* ep = 
				static_cast<const SimActionEvent*>(event);
			switch(ep->action) {
				case IDACTION_SETSPEED:
					desired_throttle.y = ep->fValue;
					break;
				case IDACTION_MOVEFORWARD:
					desired_throttle.y = ep->fValue;
					desired_throttle.x = desired_throttle.z = 0.0f;
					break;
				case IDACTION_MOVEBACK:
					desired_throttle.y = -ep->fValue;
					desired_throttle.x = desired_throttle.z = 0.0f;
					break;
				case IDACTION_MOVELEFT:
					desired_throttle.x = -ep->fValue;
					desired_throttle.y = desired_throttle.z = 0.0f;
					break;
				case IDACTION_MOVERIGHT:
					desired_throttle.x = ep->fValue;
					desired_throttle.y = desired_throttle.z = 0.0f;
					break;
				case IDACTION_MOVEUP:
					desired_throttle.z = ep->fValue;
					desired_throttle.y = desired_throttle.x = 0.0f;
					break;
				case IDACTION_MOVEDOWN:
					desired_throttle.z = -ep->fValue;
					desired_throttle.y = desired_throttle.x = 0.0f;
					break;
				case IDACTION_ROLL:
					rotation.vel.y = ep->fValue;
					break;
				case IDACTION_PITCH:
					rotation.vel.x = ep->fValue;
					break;
				case IDACTION_YAW:
					rotation.vel.z = ep->fValue;
					break;
			}
			return true;
		}
	}
	return Parent::processEvent(event);
}


//--------------------------------------------------------------------------- 

bool SimMoveObject::processQuery(SimQuery* query)
{
	switch (query->type) {
		case SimCameraQueryType: {
			// We want this object to be attachable
			SimCameraQuery* qp = static_cast<SimCameraQuery*>(query);
			qp->cameraInfo.fov = 90 * M_PI / 360.0f;
			qp->cameraInfo.nearPlane = 0.1f;
			qp->cameraInfo.farPlane = getFarPlane();
         qp->cameraInfo.tmat = getTransform();
	      return true;
		}
		case SimCollisionImageQueryType: {
			SimCollisionImageQuery* qp = static_cast<SimCollisionImageQuery*>(query);
         qp->count = 1;
			qp->image[0] = &collisionImage;
			collisionImage.transform = getTransform();
	      return true;
		}
      case SimObjectTransformQueryType: {
			SimObjectTransformQuery *q = static_cast<SimObjectTransformQuery *>(query);
			q->tmat = getTransform();
	      return true;
      }
		default:
			return Parent::processQuery(query);
	}
}


#ifndef DEFAULT_FAR_PLANE
#define DEFAULT_FAR_PLANE 100000.0f;
#endif

float SimMoveObject::getFarPlane()
{
   if (manager == NULL)
      return DEFAULT_FAR_PLANE;

   Sky* pSky = (Sky*)manager->findObject(TribesSkyId);
   if (pSky != NULL) {
      // Use the sky distance, plus a tad.   
      return pSky->getDistance() * 1.1f;      
   }

   SimTerrain* pTerrain = (SimTerrain*)manager->findObject(SimTerrainId);
   if (pTerrain != NULL) {
      // Use the terrain visible distance, plus a tad
      return pTerrain->getVisibleDistance() * 1.1f;      
   }

   // else, well, crap.
   return DEFAULT_FAR_PLANE;
}