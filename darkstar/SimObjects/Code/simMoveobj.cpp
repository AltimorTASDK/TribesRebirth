//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 


#include <sim.h>
#pragma warn -inl
#pragma warn -aus


#include "simAction.h"
#include "simMoveobj.h"
#include "simGame.h"
#include "simTimerGrp.h"
#include "simTSViewport.h"

#define MAX_SPEED			1.0
#define MAX_ROTATION		(1/10)

#define RADIUS			0.5f
#define FOCALLENGTH	0.1f
	

//--------------------------------------------------------------------------- 

SimMoveObject::SimMoveObject()
{
	movement.pos.set(0,0,0);
	movement.vel.set(0,0,0);
	rotation.pos.set(0,0,0);
	rotation.vel.set(0,0,0);
	throttle.set(0,0,0);
	desired_throttle.set(0,0,0);

	collisionImage.radius = RADIUS;
	boundingBox.fMin.set(-RADIUS/2.0f,-RADIUS/2.0f,-RADIUS/2.0f);
	boundingBox.fMax.set(+RADIUS/2.0f,+RADIUS/2.0f,+RADIUS/2.0f);
}

SimMoveObject::~SimMoveObject()
{
}


//--------------------------------------------------------------------------- 

void SimMoveObject::update(SimTime dt)
{
	// Update position and orientation
	MoveLinear lastM = movement;
	MoveLinear lastR = rotation;

	movement.update(dt);
	rotation.update(dt);
	TMat3F mat;
	mat.set(EulerF(rotation.pos.x,rotation.pos.y,rotation.pos.z),movement.pos);

	// Collision testing
	SimCollisionInfoList cList;
	if (!setPosition(mat,&cList)) {
		movement = lastM;
		rotation = lastR;
	}

	// Use throttle to calculate new velocity
	RMat3F mrot;
	mrot.set(EulerF(rotation.pos.x,rotation.pos.y,rotation.pos.z));
	Vector3F vel = throttle;
	vel *= MAX_SPEED;
	m_mul(vel,mrot,&movement.vel);
	throttle = desired_throttle;
}


//--------------------------------------------------------------------------- 

const Point3F& SimMoveObject::getPos (void)
{
	return getTransform().p;
}

void SimMoveObject::setPos(Point3F pos)
{
	// Not a good example of how to do this.
	TMat3F mat = getTransform();
	mat.p = pos;
	SimCollisionInfoList cList;
	setPosition(mat,&cList);
}


void SimMoveObject::setActionMap(const char* file)
{
	ResourceManager *rm = SimResource::get(manager);
   actionMap = rm->load(file);
}


//--------------------------------------------------------------------------- 

bool SimMoveObject::processEvent(const SimEvent* event)
{
	switch (event->type) {
		case SimTimerEventType:
			// Update every 1/30 sec.
			update(static_cast<const SimTimerEvent*>(event)->timerInterval);
			return true;
		case SimCollisionEventType:
			//??
			break;
		case SimGainFocusEventType:
			if (bool(actionMap))
				SimActionHandler::find (manager)->push(this,actionMap);
			break;
		case SimLoseFocusEventType:
			if (bool(actionMap))
				SimActionHandler::find (manager)->pop(this,actionMap);
			break;
		case SimActionEventType: {
			const SimActionEvent* ep = 
				static_cast<const SimActionEvent*>(event);
			switch(ep->action) {
				case SetSpeed:
					desired_throttle.y = ep->fValue;
					break;
				case MoveForward:
					desired_throttle.y = ep->fValue;
					desired_throttle.x = desired_throttle.z = 0.0f;
					break;
				case MoveBackward:
					desired_throttle.y = -ep->fValue;
					desired_throttle.x = desired_throttle.z = 0.0f;
					break;
				case MoveLeft:
					desired_throttle.x = -ep->fValue;
					desired_throttle.y = desired_throttle.z = 0.0f;
					break;
				case MoveRight:
					desired_throttle.x = ep->fValue;
					desired_throttle.y = desired_throttle.z = 0.0f;
					break;
				case MoveUp:
					desired_throttle.z = ep->fValue;
					desired_throttle.y = desired_throttle.x = 0.0f;
					break;
				case MoveDown:
					desired_throttle.z = -ep->fValue;
					desired_throttle.y = desired_throttle.x = 0.0f;
					break;
				case MoveRoll:
					rotation.vel.y = ep->fValue;
					break;
				case MovePitch:
					rotation.vel.x = ep->fValue;
					break;
				case MoveYaw:
					rotation.vel.z = ep->fValue;
					break;
			}
			return true;
		}
	}
	return false;
}


//--------------------------------------------------------------------------- 

bool SimMoveObject::onAdd()
{
	if (!Parent::onAdd())
		return false;

	addToSet(SimCameraSetId);
	addToSet(SimTimerSetId);

	SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);
	if (root)
		root->addObject(this);
	return true;
}


//--------------------------------------------------------------------------- 

bool SimMoveObject::processQuery(SimQuery* query)
{
	switch (query->type) {
		case SimRenderQueryImageType: {
			return false;
		}
		case SimCameraQueryType: {
			// We want this object to be attachable
			SimCameraQuery* qp = static_cast<SimCameraQuery*>(query);
			qp->cameraInfo.fov = 0.6f;
			qp->cameraInfo.nearPlane = FOCALLENGTH;
			qp->cameraInfo.farPlane = 1.0E7f;
         qp->cameraInfo.tmat = getTransform();
			break;
		}
		case SimCollisionImageQueryType: {
			SimCollisionImageQuery* qp = static_cast<SimCollisionImageQuery*>(query);
         qp->count = 1;
			qp->image[0] = &collisionImage;
			collisionImage.transform = getTransform();
			break;
		}
      case SimObjectTransformQueryType: {
			SimObjectTransformQuery *q = static_cast<SimObjectTransformQuery *>(query);
			q->tmat = getTransform();
			break;
      }
		default:
         return false;
	}
	return true;
}


//--------------------------------------------------------------------------- 
// Old collision stuff
#if 0
void SimMoveObject::collide(const SimCollisionInfoList &collisionList)
{
   // Go back to the last valid position
	float deltaZ = nextMovement.pos.z - movement.pos.z;
   nextMovement.pos = movement.pos;

	// Build velocity in object space
	RMat3F mrot;
	mrot.set(EulerF(rotation.pos.x,rotation.pos.y,rotation.pos.z));
	mrot.inverse();
	Point3F velocity;
	m_mul(nextMovement.vel,mrot,&velocity);

#if 1
#if 1
	Point3F vec(0.0f,0.0f,0.0f);
	for (int i = 0; i < collisionList.size(); i++) {
		const SimCollisionInfo& info = collisionList[i];
		float dot = m_dot(velocity,info.normal);
		if (dot < 0.0f)
			vec += info.normal;
	}
	vec.normalize();
	float dot = m_dot(velocity,vec);
	if (dot < 0.0f) {
		vec *= -dot;
		velocity += vec;
	}

#else
	// Find the plane we hit first
	float bestValue = 1.0E20f;
	const SimCollisionInfo* bestCollision = 0;
	float bestDot = 0.0f;
	for (int i = 0; i < collisionList.size(); i++) {
		const SimCollisionInfo& info = collisionList[i];
		float dot = m_dot(velocity,info.normal);
		if (dot < -0.0001f) {
			if (info.distance < bestValue) {
				bestDot = dot;
				bestValue = info.distance;
				bestCollision = &info;
			}
		}
	}

	// Subtract out collision plane we hit first.
	if (bestCollision) {
		Point3F vecx = bestCollision->normal;
		vecx *= -bestDot;
		velocity += vecx;
	}
#endif

	// Matrix TOW
	mrot.inverse();

#else
	// 
	Point3F step(.0f,.0f,.0f);
	Point3F vertical(.0f,.0f,1.0f);
	for (int i = 0; i < collisionList.size(); i++) {
		Point3F vec1 = collisionList[i].normal;

	   // Subtract off along each collision vector.
		float dot = m_dot(velocity,vec1);
		if (dot <= 0.0f) {
			vec1 *= -dot;
#if 0
			// Make sure change in velocity doesn't push us
			// through any previous normals.
			for (int b = i - 1; b >= 0; b--) {
				Point3F vec2 = collisionList[b].normal;
				float dot = m_dot(vec1,vec2);
				if (dot < 0.0f) {
					vec2 *= -dot;
					vec1 += vec2;
				}
			}
#endif
		}

#if 0
		// Offset for steps?
		if (m_dot(vertical,collisionList[i].normal) > 0.7) {
			if (collisionList[i].distance < RADIUS) {
				// Should really step by the distance from the start
				// position and get rid of the deltaZ hack.
				step.z += (RADIUS + 0.0001f) - collisionList[i].distance;
			}
		}
#endif

		velocity += vec1;
	}

	// Matrix TOW
	mrot.inverse();

	// Step back into world space
	if (step.z) {
		step.z += deltaZ;
		Point3F wstep;
		m_mul(step,mrot,&wstep);
		nextMovement.pos += wstep;
	}
#endif

	// Try advancing to new position along adjusted velocity.
	m_mul(velocity,mrot,&nextMovement.vel);
	nextMovement.update(nextMovementTime - manager->getCurrentTime());
}
#endif

