//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _SIMMOVEMENT_H_
#define _SIMMOVEMENT_H_

#include <sim.h>
#include <simContainer.h>
#include <bitset.h>


//--------------------------------------------------------------------------- 

// The force tolerance is this constant multiplied
// by the forces on the object for a single time slice
#define SimMovementForceRestTolerance  			2.0f

// Rotational tolerance is an absolute value
#define SimMovementRotationalRestTolerance		0.02f


//--------------------------------------------------------------------------- 
// Some default force types values

#define SimMovementGravityForce			BIT(0)
#define SimMovementBuoyancyForce			BIT(1)
#define SimMovementAppForce				8


//--------------------------------------------------------------------------- 

class SimMovementCollisionEvent: public SimEvent
{
public:
	SimObjectId objectId;
	SimObject* object;

	// Should use the getObject method to retrieve the
	// object ptr. It will make sure that the pointer is
	// still valid, that the object has not been deleted.
	SimMovementCollisionEvent() { type =  SimMovementCollisionEventType; }
	SimObject* getObject(SimManager* manager) const;
};



//--------------------------------------------------------------------------- 

class DLLAPI SimMovement:  public SimContainer
{
	typedef SimContainer Parent;
public:
	// List object images to collide against
	struct ObjectImage {
		SimObject* object;
		SimCollisionImage* image;
	};
	class ImageList : public Vector<ObjectImage> {
	public:
		void push_back(SimObject* obj,SimCollisionImage* image);
	};

	// Movement processing data
	// The times in this structure are not simTime but actually
	// elapsed time since the start of the movement.
	struct MovementInfo {
		int retryCount;
		float startTime;				// Starting time
		TMat3F* transform;			// Starting transform (last good pos)
		float collisionTime;			// Time of collision
		TMat3F* collision;			// Transform at collision time
		TMat3F* target;				// Transform trying to get to.
		SimCollisionImage* image;	// Our image
		ImageList imageList;			// The images we are testing against.

		// Collision surface information.
		SimCollisionInfoList collisionList;
		SimObjectList collidedList;
      
      // this produces different release and debug behavior... it is only
      // temporary code for the next few days for debugging
      #ifdef DEBUG
      MovementInfo() 
      {
         retryCount = 0;
         startTime = 0;
         transform = NULL;
         collisionTime = 0;
         collision = NULL;
         target = NULL;
         image = NULL;
      }
      #endif
	};

	// Displacement info
	struct MovementState {
		SimMovement* object;
		TMat3F transform;
		BitSet32 flags;
		int collisionMask;
		Vector3F lVelocity;

      #ifdef DEBUG
      MovementState() 
      {
         object = NULL;
         collisionMask = 0;
      }
      #endif
	};
	typedef Vector<MovementState> StateList;

	// List of constant forces
	struct Force {
		int type;
		bool ignoreMass;
		Point3F vector;

      #ifdef DEBUG
      Force()
      {
         type = 0;
         ignoreMass = false;
      }
      #endif
	};
	class ForceList: public Vector<Force> {
	public:
		iterator find(int type);
		const_iterator find(int type) const;
      Point3F forceSumExcept(int exceptFor) const;
	};

	// 
	struct Coefficient {
		float mass,invMass;
		float density;
		float drag;
		float friction;
		float elasticity;
	};

	// List of contact surface
	struct Contact {
		SimMovement* object;
	};
	class ContactList: public Vector<Contact> {
	public:
		bool add(SimMovement*);
		bool remove(SimMovement*);
	};

private:
	// Current position and orientation of shape
	TMat3F transform;
	TMat3F invTransform;

   // rotation matrix good till timeSlice changes or aVelocity changes:
	RMat3F dRM; 

	// Forces currently integrated during motion update
	float containerDrag;
	float currentDrag;
	float currentSpeed;
	Point3F currentDirection;
	ForceList forceList;
	float forceSpeed;
	SimTime dt;

protected:
	ContactList contactList;

private:
	//
	void setForceSpeed();
	void findFirstPoint(SimCollisionInfoList&,Point3F*);

protected:
	enum Constants {
		DefaultCollisionRetryCount = 3,
	};
	enum Flags {
		UseFriction =         BIT(0),
		UseAngulerVelocity =  BIT(1),
		UseElasticity =       BIT(2),
		UseCurrent =          BIT(3),
		UseDrag =             BIT(4),
		TrackContacts =       BIT(5),
		RotateBoundingBox =   BIT(6),
		AtRest =              BIT(7),
		AlwaysCalcAVelocity = BIT(8),
	};
	BitSet32 flags;

	// Dynamics
	float timeSlice;
	int forceMask;
	Coefficient coefficient;

	Vector3F lPosition;
	Vector3F lVelocity;
	Vector3F lAcceleration;

	Vector3F aPosition;
	Vector3F aVelocity;
	Vector3F aAcceleration;

	// keep tabs on things that need to be re-computed
	bool aPositionChanged;
	bool redoRM;

	// Container / Collision
	// The bounding box is the axis aligned bounding box
	// in object space.  As opposed to the SimContainer
	// box which is an axis aligned bounding box in world
	// space.
	Box3F boundingBox;
	int collisionMask;
 	int collisionRetryCount;
	bool contact;
   int excludedId;

	//
	void setTransform(const TMat3F& mat);
	void notifyTrigger(SimObject*);
	void notifyCollision(const MovementInfo& info);
	void testRestCondition();
	bool displaceObjects(const TMat3F&);
	bool displaceContacts(StateList& stateList,const Vector3F& dVec);
	bool displaceCollisions(StateList& stateList,const Vector3F& dVec);
	bool displaceObject(SimMovement*,const Vector3F&);

	virtual void onDisplacedObject(SimMovement* object,const Point3F& delta);
	virtual void onDisplacement(SimMovement* displacer,const Point3F& delta);

	virtual bool processCollision(MovementInfo* info);
	virtual bool processBlocker(SimObject*);
	virtual bool processSurface(Vector3F normal,const CollisionSurface& surface,BitSet32 flags);
	virtual void buildContainerBox(const TMat3F& mat,Box3F* box);
	virtual void buildCollisionBox(const TMat3F& mat,Box3F* box);

	virtual SimCollisionImage* getSelfImage();
	SimCollisionImage* getImage(SimObject* object);
	virtual bool getImageList(const Point3F&,ImageList*);
	bool getImageList(const SimContainerQuery&,ImageList*);
	bool testImage(SimCollisionImage*,MovementInfo*);
	bool testImage(SimCollisionTubeImage*,MovementInfo*);
	bool testImage(SimCollisionLineImage*,MovementInfo*);

	void addContact(SimMovement*);
	void removeContact(SimMovement*);

public:
	SimMovement ();
	~SimMovement ();

	// Constant and Dynamic Forces, Currents & Drag
	void setMass(float m);
	void setWeight(float w);
   float getMass();
	float getVolume();
	void setElasticity(float b);
	float getElasticity();
	void setDrag(float b);
	void setDensity(float b);
	void setFriction(float b);
	float getDensity();
   Point3F forceSumExcept(int exceptFor) const { return forceList.forceSumExcept(exceptFor); }
	virtual bool addForce(int ftype,const Point3F& vector,bool ignoreMass = false);
	bool removeForce(int ftype);
	bool getForce(int ftype,Point3F* vec);
   bool isForceActive(int ftype) const;
	void applyImpulse(const Vector3F& force);
	void applyImpulse(const Point3F& pos,const Vector3F& force);
	void collide(const Point3F& point,const Point3F& normal);
	void clearContacts();
	void clearRestFlag();

	void setContainerDrag(float drag);
	void setContainerCurrent(const Point3F& vel,float drag);

	// Kinematics
	// Update movement calculates changes based on
	// the timeSlice value
	void setTimeSlice(SimTime dt);
	SimTime getTimeSlice();
	void setAngulerVelocity(const Vector3F& vel);
	void setLinearVelocity(const Vector3F& vel);
	virtual void setDisplacementVelocity(Vector3F vel);

	void updateLinearVelocity();
	void updateAngulerVelocity();
	void updatePosition(const Point3F& startVel,TMat3F* mat);
	void updateRotation(TMat3F* mat);
	void updateMovement(TMat3F* mat);

	// Movement and Collision testing
	bool setRotation(const RMat3F& mat,bool force = false);
	bool setPosition(TMat3F mat,bool force = false);
	bool testPosition(TMat3F mat,MovementInfo* info);
	bool stepPosition(TMat3F mat,float minDist,float maxDist);
	bool moveSphere(TMat3F mat,float radius);
	bool movePoint(TMat3F mat);
	bool isAtRest();
	bool didContact();

	// Default scoping provided by movement class
   void buildScopeAndCameraInfo(Net::GhostManager *cr, CameraInfo *camInfo);

	//
	const Vector3F& getLinearVelocity() const { return lVelocity; }
	const Vector3F& getAngulerVelocity() const { return aVelocity; }
	const Vector3F& getLinearPosition() const { return lPosition; }
	const Vector3F& getAngulerPosition();
	void getVelocity(const Point3F& pos, Vector3F* vel);

   const TMat3F& getTransform() const    { return transform; }
   const TMat3F& getInvTransform() const { return invTransform; }
   const RMat3F& getRotation() const     { return (RMat3F&)transform; }
   const RMat3F& getInvRotation() const  { return (RMat3F&)invTransform; }
};

typedef SimMovement::MovementInfo SimMovementInfo;

inline SimTime SimMovement::getTimeSlice()
{
	return timeSlice;
}

inline void SimMovement::setWeight(float w)
{
	// Standard kilogram force weight.
	setMass(w / 9.8f);
}

inline float SimMovement::getMass()
{
	return coefficient.mass;
}

inline void SimMovement::setContainerDrag(float drag)
{
	containerDrag = drag;
}

inline void SimMovement::setDrag(float b)
{
	coefficient.drag = b;
}

inline void SimMovement::setDensity(float b)
{
	coefficient.density = b;
}

inline float SimMovement::getDensity()
{
	return coefficient.density;
}

inline void SimMovement::setElasticity(float b)
{
	coefficient.elasticity = b;
}

inline void SimMovement::setFriction(float b)
{
	coefficient.friction = b;
}

inline float SimMovement::getElasticity()
{
	return coefficient.elasticity;
}

inline bool SimMovement::isAtRest()
{
	return flags.test(AtRest);
}

inline bool SimMovement::didContact()
{
	return contact;
}

inline const Vector3F& SimMovement::getAngulerPosition()
{
   if (aPositionChanged)
   {
	   transform.angles(reinterpret_cast<EulerF*>(&aPosition));
      aPositionChanged = false;
   }
   return aPosition;
}

inline void SimMovement::clearRestFlag()
{
	flags.clear(AtRest);
}

#endif
