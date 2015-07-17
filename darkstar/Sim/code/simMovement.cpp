//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifdef __BORLANDC__
#pragma warn -inl
#pragma warn -aus
#endif

#include <ts.h>
#include <base.h>
#include <sim.h>

#include <simResource.h>
#include <simNetObject.h>

#include "simMovement.h"


//--------------------------------------------------------------------------- 

const float SurfaceShift = 0.0001f;



//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

SimMovement::SimMovement()
 : excludedId(0)
{
	//
	transform.identity();
	invTransform.identity();
	collisionMask = -1;
	collisionRetryCount = DefaultCollisionRetryCount;

	flags = UseFriction | AtRest | AlwaysCalcAVelocity;
	forceMask = 0;

	currentDrag = 0.0f;
	currentSpeed = 0.0f;
	currentDirection.set(.0f,.0f,.0f);
	containerDrag = 0.0f;

	lPosition.set(.0f,.0f,.0f);
	lVelocity.set(.0f,.0f,.0f);
	lAcceleration = lVelocity;

	aPosition.set(.0f,.0f,.0f);
	aVelocity.set(.0f,.0f,.0f);
	aAcceleration = aVelocity;
	aPositionChanged = false;
	redoRM = true;

	coefficient.drag = 0.0f;
	coefficient.elasticity = 0.0f;
	coefficient.friction = 1.0f;
	coefficient.mass = coefficient.invMass = 1.0f;
	coefficient.density = 1.0f;
	timeSlice = 1.0f/30.0f;
	dt = 1.0f;
	contact = false;

	// Default bounding box in object space
	boundingBox.fMin.set(-0.5f,-0.5f,-0.5f);
	boundingBox.fMax.set(+0.5f,+0.5f,+0.5f);
}


SimMovement::~SimMovement()
{
	clearContacts();
}


//--------------------------------------------------------------------------- 

void SimMovement::setMass(float m)
{
	float dm = m / coefficient.mass;
	// Update constant forces dependant on mass
	for (int f = 0; f < forceList.size(); ++f) {
		Force& force = forceList[f];
		if (!force.ignoreMass)
			force.vector *= dm;
	}
	coefficient.mass = m;
	coefficient.invMass = (m == 0.0f)? 0.0f : 1.0f / m;
}	


//----------------------------------------------------------------------------

bool SimMovement::addForce(int ftype, const Point3F& fvec,bool ignoreMass)
{
   ForceList::iterator itr = forceList.find(ftype);
   if (itr != forceList.end()) {
   	(*itr).ignoreMass = ignoreMass;
   	(*itr).vector = fvec;
   	if (!ignoreMass)
   		(*itr).vector *= coefficient.invMass;
   }
   else {
   	forceList.increment();
   	Force& f = forceList.last();
   	f.type = ftype;
   	f.ignoreMass = ignoreMass;
   	f.vector = fvec;
   	if (!ignoreMass)
   		f.vector *= coefficient.invMass;
   }
   //
   setForceSpeed();
   return true;
}

bool SimMovement::removeForce(int f)
{
	ForceList::iterator itr = forceList.find(f);
	if (itr != forceList.end()) {
		forceList.erase(itr);
		setForceSpeed();
		return true;
	}
	return false;
}

bool SimMovement::getForce(int ftype,Point3F* vec)
{
	ForceList::iterator itr = forceList.find(ftype);
	if (itr != forceList.end()) {
		*vec = (*itr).vector;
		return true;
	}
	return false;
}	

bool
SimMovement::isForceActive(int ftype) const
{
	ForceList::const_iterator itr = forceList.find(ftype);
	if (itr != forceList.end())
		return true;
	return false;
}

void SimMovement::setForceSpeed()
{
	// ForceSpeed is set to the length of the accumulated
	// forces. This value is mainly used as a threshold
	// for determining rest condition.
	//
	Point3F vec(0.0f,0.0f,0.0f);
	for (ForceList::iterator itr = forceList.begin();
		itr != forceList.end(); itr++)
		vec += (*itr).vector;
	forceSpeed = vec.len();
	flags.clear(AtRest);
}


//----------------------------------------------------------------------------

float SimMovement::getVolume()
{
	return coefficient.mass * 1.0f / coefficient.density;
}


//----------------------------------------------------------------------------

void SimMovement::setContainerCurrent(const Point3F& vel,float drag)
{
   // lenf() actually returns a very small NON-ZERO value for {0,0,0}...  
   if( ! vel.isZero() )
   {
      currentDrag = drag;
      currentSpeed = vel.lenf();
      currentDirection = vel;
      currentDirection *= 1.0f / currentSpeed;
   }
   else
   {
      currentDrag = currentSpeed = 0;
      currentDirection.set(0.0, 0.0, 0.0 );
   }
}


//----------------------------------------------------------------------------

void SimMovement::applyImpulse(const Vector3F& force)
{
	// Force is also in world space.
	//
	AssertFatal(coefficient.mass != 0.0f,
		"SimMovement::applyImpulse: Cannot apply impulse to mass-free object.");

	// Simply add mass adjusted velocity
	lVelocity.x += force.x * coefficient.invMass;
	lVelocity.y += force.y * coefficient.invMass;
	lVelocity.z += force.z * coefficient.invMass;
	flags.clear(AtRest);
}

void SimMovement::applyImpulse(const Point3F& pos, const Vector3F& force)
{
	// Pos is offset from object center in world space.
	// Force is also in world space.
	//
	AssertFatal(coefficient.mass != 0.0f,
		"SimMovement::applyImpulse: Cannot apply impulse to mass-free object.");

        float posSq = m_dot(pos,pos);
	if (!flags.test(UseAngulerVelocity) || posSq < 0.1f)
		applyImpulse(force);
	else
	{
		float lpos = 1.0f / posSq;

		// Update linear velocity
		float f1 = coefficient.invMass * m_dot(pos,force) * lpos;
		lVelocity.x += pos.x * f1;
		lVelocity.y += pos.y * f1;
		lVelocity.z += pos.z * f1;

		// Update angular velocity
		// Inertia formula assumes sphere.
		// note:  assumes pos on outside of object (worth fixing?)
		Point3F dw;
		m_cross(pos,force,&dw);
		float f2 = coefficient.invMass * 2.5f * lpos;
		aVelocity.x += dw.x * f2;
		aVelocity.y += dw.y * f2;
		aVelocity.z += dw.z * f2;
		flags.clear(AtRest);
		redoRM = true;
	}
}


//----------------------------------------------------------------------------
// Pos in world coordinates

void SimMovement::collide(const Point3F& point,const Point3F& normal)
{
	Point3F cp = point;
	cp -= lPosition;

	// Get velocity of point along line of normal
	Point3F cpVel;
	getVelocity(cp,&cpVel);
	float cpSpeed = m_dot(cpVel,normal);
	if (cpSpeed > .0f)
		return;

	// Reflect velocity according to restitution
	float vaf = coefficient.elasticity * -cpSpeed;

	// Convert resulting changes in velocity into changes
	// in linear and rotational velocities.
	Point3F nVel = normal;
	nVel *= (vaf - cpSpeed);

	// Linear velocity
	Point3F fVel = cp;
	fVel.normalize();
	fVel *= m_dot(nVel,fVel);
	lVelocity += fVel;

	// Rotational velocity
	Point3F dw,wVel = nVel;
	wVel -= fVel;
	m_cross(cp,wVel,&dw);
	float wlen = dw.len();
	if (wlen)
		dw.normalize(1.0f,wlen);
	else
		dw.set(0.0f,0.0f,0.0f);
	dw *= wVel.len() / cp.len();
	aVelocity += dw;

	//
	flags.clear(AtRest);
	redoRM = true;
}


//----------------------------------------------------------------------------

void SimMovement::setAngulerVelocity(const Vector3F& vel)
{
	aVelocity = vel;
	flags.clear(AtRest);
	redoRM = true;
}	

void SimMovement::setLinearVelocity(const Vector3F& vel)
{
	lVelocity = vel;
	flags.clear(AtRest);
}

void SimMovement::setDisplacementVelocity(Vector3F vel)
{
	// Normally used to incorporate motion produced
	// by animation.  Makes sure the current linear
	// velocity includes the passed velocity.
	float len2 = m_dot(vel,vel);
	if (len2 != 0.0f) {
		float dot = m_dot(vel,lVelocity);
		if (dot < len2) {
			float f1 = dot / len2;
			lVelocity.x += (vel.x - vel.x * f1);
			lVelocity.y += (vel.y - vel.y * f1);
			lVelocity.z += (vel.z - vel.z * f1);
			flags.clear(AtRest);
		}
	}
}


//--------------------------------------------------------------------------- 
// Pos in world space relative to the center of the object.
//
void SimMovement::getVelocity(const Point3F& pos,Vector3F* vel)
{
	// Returns the velocity of the point in world space
	m_cross(aVelocity,pos,vel);
	(*vel) += lVelocity;
}


//--------------------------------------------------------------------------- 

void SimMovement::setTimeSlice(SimTime adt)
{
	if (!IsEqual(timeSlice,adt))
		redoRM=true;
	timeSlice = adt;
}


//--------------------------------------------------------------------------- 

void SimMovement::updateLinearVelocity()
{
	// New linear velocity from forces
	for (int f = 0; f < forceList.size(); ++f) {
      if (forceList[f].type & forceMask) {
		   Force& force = forceList[f];
		   lVelocity.x += force.vector.x * timeSlice;
		   lVelocity.y += force.vector.y * timeSlice;
		   lVelocity.z += force.vector.z * timeSlice;
      }
	}
	lVelocity.x += lAcceleration.x * timeSlice;
	lVelocity.y += lAcceleration.y * timeSlice;
	lVelocity.z += lAcceleration.z * timeSlice;

	// Drag
	if (flags.test(UseDrag) && containerDrag != 0.0f) {
		float dc = coefficient.drag * containerDrag * timeSlice;
		lVelocity.x -= lVelocity.x * dc;
		lVelocity.y -= lVelocity.y * dc;
		lVelocity.z -= lVelocity.z * dc;
	}

	// Current
	if (flags.test(UseCurrent) && currentSpeed != 0.0f) {
		float dc = m_dot(lVelocity,currentDirection);
		dc = (currentSpeed - dc) * coefficient.drag * currentDrag * timeSlice;
		lVelocity.x += currentDirection.x * dc;
		lVelocity.y += currentDirection.y * dc;
		lVelocity.z += currentDirection.z * dc;
	}
}

void SimMovement::updateAngulerVelocity()
{
}

void SimMovement::updatePosition(const Point3F& lVel,TMat3F* mat)
{
	// New linear position into matrix
	float hdt = timeSlice * 0.5f;
	mat->p = lPosition;
	mat->p.x += (lVel.x + lVelocity.x) * hdt;
	mat->p.y += (lVel.y + lVelocity.y) * hdt;
	mat->p.z += (lVel.z + lVelocity.z) * hdt;
	mat->flags |= TMat3F::Matrix_HasTranslation;
}

void SimMovement::updateRotation(TMat3F* mat)
{
	// New anguler position into matrix
	if (flags.test(UseAngulerVelocity)) {
		if (redoRM || flags.test(AlwaysCalcAVelocity)) {
			dRM.set( EulerF(aVelocity.x * timeSlice,aVelocity.y * timeSlice,aVelocity.z * timeSlice) );
			redoRM=false;
		}
		m_mul((RMat3F&)transform,dRM,(RMat3F*)mat);
	}
	else
		*(RMat3F*)mat = (RMat3F&)transform;
	mat->flags |= TMat3F::Matrix_HasRotation|TMat3F::Matrix_HasTranslation; // doh! RMat mult.
	                                                                        // clears TMat flags
}	

void SimMovement::updateMovement(TMat3F* mat)
{
	Point3F svel = lVelocity;
	updateLinearVelocity();
	//updateAngulerVelocity(); Doesn't do anything yet
	updatePosition(svel,mat);
	updateRotation(mat);
}


//--------------------------------------------------------------------------- 

void SimMovement::setTransform(const TMat3F& mat)
{
	// New position and rotation
	transform = invTransform = mat;
	invTransform.inverse();
	lPosition = mat.p;
   aPositionChanged = true;

	// Update data needed by container
	Box3F box;
	buildContainerBox(transform,&box);
	setBoundingBox(box);
}


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

bool SimMovement::setRotation(const RMat3F& mat,bool /*force*/)
{
	TMat3F tmat;
	(RMat3F&)tmat = mat;
	tmat.p = lPosition;
	tmat.flags |= TMat3F::Matrix_HasTranslation;
	return setPosition(tmat);
}


//--------------------------------------------------------------------------- 

bool SimMovement::testPosition(TMat3F mat,MovementInfo* info)
{
	if (!collisionMask)
		return true;

	if (!info->imageList.size())
		getImageList(mat.p,&info->imageList);
	if (!info->image)
		if ((info->image = getSelfImage()) == 0)
			return true;
	info->image->transform = mat;

	info->collisionList.clear();
	if (testImage(info->image,info))
		return true;
	return false;
}


//--------------------------------------------------------------------------- 

bool SimMovement::setPosition(TMat3F mat,bool force)
{
	contact = false;
	flags.clear(AtRest);

	// If no collisions, then just force the position.
	if (!collisionMask || force) {
		setTransform(mat);
		return true;
	}

	// Collision testing
	MovementInfo info;
	info.target = &mat;
	info.transform = &transform;
	info.image = getSelfImage();
	if (!info.image)
		return false;
	info.collision = &info.image->transform;
	info.image->transform = mat;
	getImageList(mat.p,&info.imageList);

	for (int i = 0; i < collisionRetryCount; i++) {
		info.collisionList.clear();
		if (testImage(info.image,&info)) {
			setTransform(mat);
			notifyCollision(info);
			testRestCondition();
			return true;
		}

		// Process collision info
		info.startTime = 0.0f;
		info.collisionTime = 1.0f;
		info.retryCount = i;
		if (!processCollision(&info)) {
			// Didn't collide with anything, force the position
			setTransform(mat);
			notifyCollision(info);
			testRestCondition();
			return true;
		}
	}

	flags.set(AtRest);
	lVelocity.set(0.0f,0.0f,0.0f);
	aVelocity.set(0.0f,0.0f,0.0f);
	redoRM = true;
	notifyCollision(info);
	return false;
}


//----------------------------------------------------------------------------

bool SimMovement::stepPosition(TMat3F target,float minDist,float maxDist)
{
	contact = false;
	flags.clear(AtRest);

	MovementInfo info;
	info.target = &target;
	info.transform = &transform;
	info.image = getSelfImage();
	info.collision = &info.image->transform;
	info.startTime = 0.0f;
	getImageList(target.p,&info.imageList);

	float dist = maxDist;
	for (info.retryCount = 0; info.retryCount < collisionRetryCount;
			info.retryCount++) {
		//
		info.image->transform = *info.target;
		info.image->transform.p = info.transform->p;
		info.image->transform.flags |= TMat3F::Matrix_HasTranslation|TMat3F::Matrix_HasRotation;

		Point3F stepVector = info.target->p;
		stepVector -= info.transform->p;
		float stepLength = stepVector.len();

		// Step along the vector
		bool collided = false;
		int stepCount = int(ceil(stepLength / dist));
		if (stepCount) {
			stepVector *= 1.0f / float(stepCount);
			for (int s = 0; s < stepCount; s++) {
				info.image->transform.p += stepVector;

				info.collisionList.clear();
				if (!testImage(info.image,&info)) {
					// Process collision
					float dt = (timeSlice - info.startTime) / float(stepCount);
					info.collisionTime = info.startTime + dt * float(s+1);
					info.startTime = info.startTime + dt * float(s);
					if (processCollision(&info)) {
						collided = true;
						dist = minDist;
						break;
					}
				}

				info.transform->p = info.image->transform.p;
				info.transform->flags |= TMat3F::Matrix_HasTranslation|TMat3F::Matrix_HasRotation;
			}
		}
		//
		if (!collided) {
			setTransform(info.image->transform);
			notifyCollision(info);
			testRestCondition();
			return true;
		}
	}

	flags.set(AtRest);
	setTransform(transform);
	lVelocity.set(0.0f,0.0f,0.0f);
	aVelocity.set(0.0f,0.0f,0.0f);
	redoRM = true;
	notifyCollision(info);
	return false;
}


//--------------------------------------------------------------------------- 

bool SimMovement::moveSphere(TMat3F mat,float radius)
{
	contact = false;
	flags.clear(AtRest);

	// If no collisions, then just force the position.
	if (!collisionMask) {
		setTransform(mat);
		return true;
	}

	// Get the image to test.
	SimCollisionTubeImage tube;
	tube.start = transform.p;
	tube.radius = radius;
	tube.transform.identity();

	// Collision testing
	MovementInfo info;
	info.transform = &transform;
	info.collision = &transform;
	info.image = &tube;
	info.target = &mat;

	for (int i = 0; i < collisionRetryCount; i++) {
		info.collisionList.clear();
		tube.end = mat.p;
		if (testImage(&tube,&info)) {
			setTransform(mat);
			notifyCollision(info);
			testRestCondition();
			return true;
		}

		// Move to the collision point
		findFirstPoint(info.collisionList,&transform.p);

		// Process collision info
		info.retryCount = i;
		info.startTime = 0.0f;
		info.collisionTime = 1.0f;
		if (!processCollision(&info)) {
			// Didn't collide with anything, force the position
			mat.p = transform.p;
			mat.flags |= TMat3F::Matrix_HasTranslation;
         setTransform(mat);
			notifyCollision(info);
			testRestCondition();
			return true;
		}
	}

	flags.set(AtRest);
	setTransform(transform);
	lVelocity.set(0.0f,0.0f,0.0f);
	aVelocity.set(0.0f,0.0f,0.0f);
	redoRM = true;
	notifyCollision(info);
	return false;
}


//--------------------------------------------------------------------------- 

bool SimMovement::movePoint(TMat3F mat)
{
	contact = false;
	flags.clear(AtRest);

	// If no collisions, then just force the position.
	if (!collisionMask) {
		setTransform(mat);
		return true;
	}

	// Get the image to test.
	SimCollisionLineImage line;
	line.start = transform.p;
	line.transform.identity();

	// Collision testing
	MovementInfo info;
	info.transform = &transform;
	info.collision = &transform;
	info.image = &line;
	info.target = &mat;
   
	for (int i = 0; i < collisionRetryCount; i++) {
		info.collisionList.clear();
		line.end = mat.p;
		if (testImage(&line,&info)) {
			setTransform(mat);
			notifyCollision(info);
			testRestCondition();
			return true;
		}

		// Move to the collision point
		findFirstPoint(info.collisionList,&transform.p);

		// Process collision info
		info.retryCount = i;
		info.startTime = 0.0f;
		info.collisionTime = 1.0f;
		if (!processCollision(&info)) {
			// Didn't collide with anything, force the position
			mat.p = transform.p;
			mat.flags |= TMat3F::Matrix_HasTranslation;
         setTransform(mat);
			notifyCollision(info);
			testRestCondition();
			return true;
		}
	}

	flags.set(AtRest);
	setTransform(transform);
	lVelocity.set(0.0f,0.0f,0.0f);
	aVelocity.set(0.0f,0.0f,0.0f);
	redoRM = true;
	notifyCollision(info);
	return false;
}


//----------------------------------------------------------------------------

void SimMovement::testRestCondition()
{
	// Attempt to detect rest condition.
	// Basically: no acc, linear or rotational, no rotational
	// velocity. The linear velocity is compared against the
	// current force affecting the object.  If the linear velocity
	// after contact is less than the current forces applied in
	// this last timeslice, we are a rest.
	if (contact && !flags.test(AtRest)) {
		float vtol = forceSpeed * timeSlice * SimMovementForceRestTolerance;
		float atol = SimMovementRotationalRestTolerance;
		if (lVelocity.x > -vtol && lVelocity.x < vtol && 
				lVelocity.y > -vtol && lVelocity.y < vtol && 
				lVelocity.z > -vtol && lVelocity.z < vtol && 
				aVelocity.x > -atol && aVelocity.x < atol && 
				aVelocity.y > -atol && aVelocity.y < atol && 
				aVelocity.z > -atol && aVelocity.z < atol && 
				lAcceleration.x + lAcceleration.y + lAcceleration.z == 0.0f &&
				aAcceleration.x + aAcceleration.y + aAcceleration.z == 0.0f) {
			//lVelocity.set(0,0,0);
			//aVelocity.set(0,0,0);
			flags.set(AtRest);
		}
	}
}

//----------------------------------------------------------------------------
// Object displacement
//----------------------------------------------------------------------------

bool SimMovement::displaceObjects(const TMat3F& mat)
{
	// Calculate delta displacement
	Point3F dVec = transform.p;
	dVec -= mat.p;

	StateList stateList;
	if (displaceContacts(stateList,dVec)) {
		// Need to fix this later, tg.
		int stype[20];
		for (int i = 0; i < contactList.size(); i++) {
			stype[i] = contactList[i].object->type;
			contactList[i].object->type = 0;
		}
		
		bool ok = displaceCollisions(stateList,dVec);

		for (int j = 0; j < contactList.size(); j++)
			contactList[j].object->type = stype[j];

		if (ok) {
			// Notify the objects that they were displaced
			for (StateList::iterator itr = stateList.begin();
					itr != stateList.end(); itr++) {
				MovementState& state = (*itr);
				Point3F delta = state.object->getTransform().p - state.transform.p;
				onDisplacedObject(state.object,delta);
				state.object->onDisplacement(this,delta);
				state.object->lVelocity = state.lVelocity;
			}
			return true;
		}
	}

	// If we couldn't move all the objects, then unwind
	// the changes we've made.
	for (StateList::iterator itr = stateList.begin();
			itr != stateList.end(); itr++) {
		MovementState& state = (*itr);
		state.object->setPosition(state.transform,true);
		state.object->lVelocity = state.lVelocity;
		state.object->collisionMask = state.collisionMask;
		state.object->flags = state.flags;
	}
	return false;
}


//----------------------------------------------------------------------------

bool SimMovement::displaceContacts(StateList& stateList,const Vector3F& dVec)
{
	// Get our own collision image
	SimCollisionImage* image = getSelfImage();
	if (!image)
		return true;
	image->transform = transform;

	//
	int originalType = type;
	type = 0;
	SimCollisionInfo info;
	ContactList::iterator itr;
	for (itr = contactList.begin(); itr != contactList.end(); itr++) {
		// Don't bother with triggers or non-movement objects
		SimMovement* obj = dynamic_cast<SimMovement*>((*itr).object);
		if (!obj || !obj->type)
			continue;

		// Save it's original state before moving it.
		stateList.increment();
		MovementState& state = stateList.last();
		state.object = obj;
		state.transform = obj->transform;
		state.lVelocity = obj->lVelocity;
		state.collisionMask = obj->collisionMask;
		state.flags = obj->flags;

		//
		if (!displaceObject(obj,dVec)) {
			// The object has failed to move the distance we want,
			// we need to check to see if it's actually blocking us.
			info.surfaces.clear();
			SimCollisionImage* objImage = getImage(obj);
			if (objImage && 
					SimCollisionImage::test(image,objImage,&info.surfaces) ||
					!obj->displaceObjects(obj->transform)) {
				type = originalType;
				if (processBlocker(obj))
					return false;
			}
		}
	}
	type = originalType;
	return true;
}


//----------------------------------------------------------------------------

bool SimMovement::displaceCollisions(StateList& stateList,const Vector3F& dVec)
{
	// Get our own collision image
	SimCollisionImage* image = getSelfImage();
	if (!image)
		return true;
	image->transform = transform;

	// Get list of all the object images to test against.
	// Don't bother with static objects.
	int originalMask = collisionMask;
	int originalType = type;
	collisionMask &= ~(SimTerrainObjectType | SimInteriorObjectType |
		SimContainerObjectType);
	ImageList iList;
	getImageList(transform.p,&iList);
	type = 0;

	// Get object intersections
	SimCollisionInfo info;
	ImageList::iterator itr;
	for (itr = iList.begin(); itr != iList.end(); itr++) {
		SimCollisionImage* objImage = (*itr).image;

		// Don't bother non-movement objects
		SimMovement* obj = dynamic_cast<SimMovement*>((*itr).object);
		if (!obj)
			continue;

		// Test for object intersection
		info.surfaces.clear();
		if (SimCollisionImage::test(image,objImage,&info.surfaces)) {

			// Save it's original state before moving it.
			stateList.increment();
			MovementState& state = stateList.last();
			state.object = obj;
			state.transform = obj->transform;
			state.lVelocity = obj->lVelocity;
			state.collisionMask = obj->collisionMask;
			state.flags = obj->flags;

			// Have the object displace any moveable objects it's
			// intersecting.
			if (!displaceObject(obj,dVec) ||
					!obj->displaceObjects(obj->transform)) {
				collisionMask = originalMask;
				type = originalType;
				if (processBlocker(obj))
					return false;
			}
#if 0
			// Let's check to see if we are still colliding against it.
			objImage = getImage(obj);
			info.surfaces.clear();
			if (objImage && SimCollisionImage::test(image,objImage,&info.surfaces)) {
				collisionMask = originalMask;
				type = originalType;
				if (processBlocker(obj))
					return false;
			}
#endif
		}
	}
	collisionMask = originalMask;
	type = originalType;
	return true;
}	


//----------------------------------------------------------------------------

bool SimMovement::displaceObject(SimMovement* obj,const Vector3F& dVec)
{
	// Test the object against the static environment.
	Vector3F oVec = dVec;
	int originalMask = obj->collisionMask;
	int originalType = obj->type;
	obj->flags.clear(UseFriction | UseElasticity);
	obj->collisionMask &= SimTerrainObjectType | SimInteriorObjectType;
	TMat3F objMat = obj->transform;

	// Going to try this a few times.
	for (int retry = 0; retry < 3; retry++) {
		obj->setDisplacementVelocity(lVelocity);
		objMat.p = obj->transform.p;
		objMat.p += oVec;

		if (!obj->setPosition(objMat)) {
			obj->collisionMask = originalMask;
			obj->type = originalType;
			return false;
		}

		// See if it moved far enough
		Vector3F rvec = obj->transform.p;
		rvec -= objMat.p;
		rvec += oVec;
		float len = oVec.len();
		float dot = m_dot(rvec,oVec);
		if (dot >= len * len) {
			obj->collisionMask = originalMask;
			obj->type = originalType;
			return true;
		}

		// Try again with what's missing
		rvec = oVec;
		rvec *= dot / (len * len);
		oVec -= rvec;
	}
	obj->collisionMask = originalMask;
	obj->type = originalType;
	return false;
}

bool SimMovement::processBlocker(SimObject*)
{
	// We don't do anything to blockers by default
	return true;
}	

void SimMovement::onDisplacedObject(SimMovement* object,const Point3F& delta)
{
	// Object that was displaced and the amount.
	object, delta;
}	

void SimMovement::onDisplacement(SimMovement* displacer,const Point3F& delta)
{
	// Object that displaced us and the amount.
	displacer, delta;
}


//----------------------------------------------------------------------------
// Object Contacts
//----------------------------------------------------------------------------

void SimMovement::addContact(SimMovement* obj)
{
	if (contactList.add(obj))
		obj->contactList.add(this);
}

void SimMovement::removeContact(SimMovement* obj)
{
	if (contactList.remove(obj))
		obj->contactList.remove(this);
}

void SimMovement::clearContacts()
{
	for (int i = 0; i < contactList.size(); i++)
		contactList[i].object->contactList.remove(this);
	contactList.clear();
}



//----------------------------------------------------------------------------
// Collision
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

SimCollisionImage* SimMovement::getImage(SimObject* object)
{
	SimCollisionImageQuery query;
	//
	// Query position
	// Query detail level
	//
	if (object->processQuery(&query))
		return query.image[0];
	return 0;
}


//----------------------------------------------------------------------------

SimCollisionImage* SimMovement::getSelfImage()
{
	SimCollisionImageQuery query;
	//
	// Query position
	// Query detail level
	//
	if (this->processQuery(&query))
		return query.image[0];
	return 0;
}


//----------------------------------------------------------------------------

bool SimMovement::getImageList(const Point3F& pos,ImageList* list)
{
	// Build bounding box of new position in world space
	SimContainerQuery query;
	query.id = (excludedId != 0) ? excludedId : getId();
	query.type = getType();
	query.mask = collisionMask;
	query.detail = SimContainerQuery::DefaultDetail;
	TMat3F mat = transform;
	mat.p = pos;
	mat.flags |= TMat3F::Matrix_HasTranslation;
	buildCollisionBox(mat,&query.box);
	return getImageList(query,list);
}


//----------------------------------------------------------------------------

bool SimMovement::getImageList(const SimContainerQuery& cquery,ImageList* list)
{
	// Query for all the objects that intersect query
	SimContainerList iList;
	SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);
	root->findIntersections(cquery,&iList);

	// Query objects for their images
	SimCollisionImageQuery iquery;
	for (SimContainerList::iterator itr = iList.begin();
			itr != iList.end(); itr++) {
		if ((*itr)->processQuery(&iquery))
			for (int i = 0; i < iquery.count; i++)
				list->push_back(*itr,iquery.image[i]);
	}
	return !list->empty();
}


//----------------------------------------------------------------------------
// Build a bounding box for the base SimContainer class
// This box is world axis aligned.
//
void SimMovement::buildContainerBox(const TMat3F& mat,Box3F* box)
{
	// Transform our local bounding box into a bounding box
	// axis aligned in world space.
   if (flags.test(RotateBoundingBox))
      m_mul(boundingBox, mat, box);
   else {
      // if an object's bound box is defined by a radius then it doesn't
      // change under rotation, hence we simply translate it
      *box = boundingBox;
      box->fMin += mat.p;
      box->fMax += mat.p;
   }
}


//--------------------------------------------------------------------------- 
// Build a bounding box that will be used to query the container
// database for object to collide against.  This box must encompass
// the motion of the object during the current time slice.
//
void SimMovement::buildCollisionBox(const TMat3F& mat,Box3F* box)
{
	buildContainerBox(mat,box);
	// Expand the box by the current velocity
	Box3F nb;
	Point3F svel = lVelocity * timeSlice;
	nb.fMin = box->fMin + svel;
	nb.fMax = box->fMax + svel;
	box->fMin.setMin(nb.fMin);
	box->fMax.setMax(nb.fMax);
}


//--------------------------------------------------------------------------- 

bool SimMovement::testImage(SimCollisionImage* image,MovementInfo* minfo)
{
	// Test our image against all the images in iList
	minfo->collisionList.increment();
	ImageList& iList = minfo->imageList;
	for (ImageList::iterator itr = iList.begin(); itr != iList.end(); itr++) {
	   SimCollisionInfo &info = minfo->collisionList.last();
		info.surfaces.clear();
		if (SimCollisionImage::test(image,(*itr).image,&info.surfaces)) {
			if ((*itr).image->trigger) {
				notifyTrigger((*itr).object);
				// Pull this image out of the list, we only need to
				// trigger once.
				*itr = iList.last();
				iList.decrement();
				itr--;
			}
			else {
				info.time = 1.0f;
				info.object = (*itr).object;
				info.image = (*itr).image;
            minfo->collisionList.increment();
			}
		}
	}
   minfo->collisionList.decrement();
	return minfo->collisionList.empty();
}


//--------------------------------------------------------------------------- 

bool SimMovement::testImage(SimCollisionTubeImage* tube,MovementInfo* minfo)
{
	// Build bounding box of tube position in world space
	SimContainerQuery query;
	query.id = (excludedId != 0) ? excludedId : getId();
	query.type = getType();
	query.mask = collisionMask;
	query.detail = SimContainerQuery::DefaultDetail;
	query.box.fMin = tube->start;
	query.box.fMax = tube->end;
	query.box.fMin.setMin(tube->end);
	query.box.fMax.setMax(tube->start);

	query.box.fMin.x -= tube->radius;
	query.box.fMin.y -= tube->radius;
	query.box.fMin.z -= tube->radius;
	query.box.fMax.x += tube->radius;
	query.box.fMax.y += tube->radius;
	query.box.fMax.z += tube->radius;

	SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);
	SimContainerList iList;
   
   minfo->collisionList.increment();
	if (root->findIntersections(query,&iList)) {
		// Query & test each object's collision image
		for (SimContainerList::iterator itr = iList.begin();
				itr != iList.end(); itr++) {
			SimCollisionImage* image2 = getImage(*itr);
			if (image2) {
				// Move alarms into a seperate list?
            SimCollisionInfo &info = minfo->collisionList.last();
				info.surfaces.clear();
				if (SimCollisionImage::test(tube,image2,&info.surfaces)) {
					if (image2->trigger)
						notifyTrigger(*itr);
					else {
						info.time = 1.0f;
						info.object = *itr;
						info.image = image2;
						// minfo->collisionList.push_back(info);
                  minfo->collisionList.increment();
					}
				}
			}
		}
	}
   minfo->collisionList.decrement();
	return minfo->collisionList.empty();
}

//--------------------------------------------------------------------------- 

bool SimMovement::testImage(SimCollisionLineImage* line,MovementInfo* minfo)
{
	// Build bounding box of line position in world space
	SimContainerQuery query;
	query.id = (excludedId != 0) ? excludedId : getId();
	query.type = getType();
	query.mask = collisionMask;
	query.detail = SimContainerQuery::DefaultDetail;
	query.box.fMin = line->start;
	query.box.fMax = line->end;
	query.box.fMin.setMin(line->end);
	query.box.fMax.setMax(line->start);

	SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);
	SimContainerList iList;
   minfo->collisionList.increment();
	if (root->findIntersections(query,&iList)) {
		// Query & test each object's collision image
		for (SimContainerList::iterator itr = iList.begin();
				itr != iList.end(); itr++) {
			SimCollisionImage* image2 = getImage(*itr);
			if (image2) {
				// Move alarms into a seperate list?
		      SimCollisionInfo & info = minfo->collisionList.last();
				info.surfaces.clear();
				if (SimCollisionImage::test(line,image2,&info.surfaces)) {
					if (image2->trigger)
						notifyTrigger(*itr);
					else {
						info.time = 1.0f;
						info.object = *itr;
						info.image = image2;
                  minfo->collisionList.increment();
						// minfo->collisionList.push_back(info);
					}
				}
			}
		}
	}
   minfo->collisionList.decrement();
	return minfo->collisionList.empty();
}


//--------------------------------------------------------------------------- 

void SimMovement::findFirstPoint(SimCollisionInfoList& cList,Point3F* cpoint)
{
	if (cList.size() == 1)
		m_mul(cList[0].surfaces[0].position,cList[0].surfaces.tWorld,cpoint);
	else {
		// Keep the best surface, closest point to the start of the line.
		// With the tube collision image each object should only have
		// a single collision surface.
		Point3F point;
		float bestTime = 1.0e30f;
		CollisionSurface* surface = 0;
		for (SimCollisionInfoList::iterator itr = cList.begin();
				itr != cList.end(); itr++ ) {
			CollisionSurfaceList& surfaces = (*itr).surfaces;
			if (surfaces[0].time < bestTime) {
				bestTime = surfaces[0].time;
				surface = &surfaces[0];
			}
		}
		m_mul(surface->position,cList[0].surfaces.tWorld,cpoint);
	}
}


//--------------------------------------------------------------------------- 

bool SimMovement::processCollision(MovementInfo* info)
{
	bool hit = false;
	BitSet32 tflags = flags;
	if (info->retryCount)
		tflags.clear(UseFriction | TrackContacts);

	// Process all the surfaces in the info list.
	Point3F normal;
	for (int j = 0; j < info->collisionList.size(); j++) {
		CollisionSurfaceList& surfaceList = info->collisionList[j].surfaces;

		// List the object as collided, at this point we
		// basically have hit it.  We could also wait to see
		// if any of the surfaces are "hit", but that cause
		// some other problems. tg
		info->collidedList.push_back(info->collisionList[j].object);

		// Process all the surfaces.
		for (int i = 0; i < surfaceList.size(); i++) {
			CollisionSurface& surface = surfaceList[i];
			if (surface.distance >= 0.0f) {
				m_mul(surface.normal,static_cast<RMat3F&>(surfaceList.tWorld),&normal);
				if (processSurface(normal,surface,tflags))
					hit = true;
			}
		}
	}

	if (hit) {
		// Set next position to try, ignore aVelocity
		info->target->p = lVelocity;
		info->target->p *= timeSlice - info->startTime;
		info->target->p += info->transform->p;
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------

bool SimMovement::processSurface(Vector3F normal,const CollisionSurface& surface,BitSet32 flags)
{
	// Normal is surface.normal in world space
	float dot = m_dot(normal,lVelocity);
	if (dot < 0.0f) {
		contact = true;

		// Remove velocity along surface normal
		normal *= -dot + SurfaceShift;
		lVelocity += normal;

		// Remove velocity parallel to surface
		if (flags.test(UseFriction)) {
			Point3F fv = lVelocity;
			float f = coefficient.friction * -dot;
			if (surface.material)
				f *= ((TSMaterial*)surface.material)->getFriction();
			float len = fv.len();
			if (len > f)
				fv *= f / len;
			lVelocity -= fv;
		}

		// Put back velocity along surface normal
		// No surface material defaults elasticity to 0
		if (flags.test(UseElasticity)) {
			float ee = coefficient.elasticity;
			if (surface.material)
				ee *= ((TSMaterial*)surface.material)->getElasticity();
			normal *= ee;
			lVelocity += normal;
		}

		// Track surface contacts for traction
		if (flags.test(TrackContacts)) {
		}
		return true;
	}
	return false;
}


//--------------------------------------------------------------------------- 

void SimMovement::notifyTrigger(SimObject* object)
{
	// Only the trigger object gets notified, as far
	// as were concerned, we didn't hit anything.
	SimMovementCollisionEvent* event = new SimMovementCollisionEvent;
	event->objectId = this->getId();
	event->object = this;
	manager->postCurrentEvent(object,event);
}	

void SimMovement::notifyCollision(const MovementInfo& info)
{
	// Notify all objects involved in the incident.
	for (int j = 0; j < info.collidedList.size(); j++) {
		SimObject* object = info.collidedList[j];
		SimMovementCollisionEvent* event;

		// Notify the other guy.
		event = new SimMovementCollisionEvent;
		event->objectId = this->getId();
		event->object = this;
		manager->postCurrentEvent(object,event);

		// Notify ourselves.
		event = new SimMovementCollisionEvent;
		event->objectId = object->getId();
		event->object = object;
		manager->postCurrentEvent(this,event);
	}
}


//----------------------------------------------------------------------------
// Default Scoping
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void SimMovement::buildScopeAndCameraInfo(Net::GhostManager *cr, CameraInfo *camInfo)
{
	SimContainer* root = NULL;

   root = findObject(manager,SimRootContainerId,root);
	if (root) {
		camInfo->camera = this;
		camInfo->pos = getLinearPosition();

		// Orientation is set to the object's Y axis 
		// vector in world space.
		getInvTransform().getRow(1,&camInfo->orientation);

		// Sin of 1/2 total FOV: default to 90deg.
		camInfo->fov = m_sin(M_PI / 4.0f);

		// Should get this from somewhere, normally a derived
		// scoping object should override this method and provide
		// something a little more accurate.
		camInfo->visibleDistance = 1500.0f; // 1.0E30f;

		//
		SimContainerScopeContext scopeContext;
		scopeContext.lock(cr,camInfo);
		root->scope(scopeContext);
		scopeContext.unlock();
	}
}	


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

SimMovement::ForceList::iterator SimMovement::ForceList::find(int type)
{
	for (iterator itr = begin(); itr != end(); itr++)
		if ((*itr).type == type)
			return itr;
	return end();
}	

SimMovement::ForceList::const_iterator SimMovement::ForceList::find(int type) const
{
	for (const_iterator itr = begin(); itr != end(); itr++)
		if ((*itr).type == type)
			return itr;
	return end();
}	

// What forces, with one (possible) exception, are at work on the object?
// Ai jetting uses this to figure out what jet force to apply. 
Point3F SimMovement::ForceList::forceSumExcept(int exceptFor) const
{
   Point3F  sum(0,0,0);
	for( const_iterator itr = begin(); itr != end(); itr++ )
		if( itr->type != exceptFor )
			sum += itr->vector;
	return sum;
}


//----------------------------------------------------------------------------

bool SimMovement::ContactList::add(SimMovement* obj)
{
	for (int i = 0; i < size(); i++)
		if ((*this)[i].object == obj)
			return false;
	increment();
	last().object = obj;
	return true;
}

bool SimMovement::ContactList::remove(SimMovement* obj)
{
	for (int i = 0; i < size(); i++)
		if ((*this)[i].object == obj) {
			(*this)[i] = (*this)[size() - 1];
			decrement();
			return true;
		}
	return false;
}


//----------------------------------------------------------------------------

void SimMovement::ImageList::push_back(SimObject* obj,SimCollisionImage* image)
{
	increment();
	ObjectImage& ll = last();
	ll.object = obj;
	ll.image = image;
}


//----------------------------------------------------------------------------

SimObject* SimMovementCollisionEvent::getObject(SimManager* manager) const
{
	SimObject* obj = manager->findObject(objectId);
	return (obj == object)? obj: 0;
}
