//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

#ifndef _SIMCOLLIDABLE_H_
#define _SIMCOLLIDABLE_H_

#include <sim.h>
#include <simContainer.h>
#include <simCollision.h>


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class DLLAPI SimCollideable: public SimContainer
{
protected:
	TMat3F transform;
	bool alarmCollision;
	int collisionMask;
	Box3F boundingBox;
   bool boundingBoxRotate;
   
	//
	SimCollisionImage* getImage(SimObject*);
	bool getIntersectionList(Point3F&,SimContainerList*);

protected:
	virtual void buildBoundingBox(const TMat3F& mat,Box3F* box);

public:
	SimCollideable();
	~SimCollideable();

	//
   bool setPosition(TMat3F pos,SimCollisionInfoList*,bool force = false);
   bool movePosition(TMat3F& pos,SimCollisionInfoList*);
   const TMat3F& getTransform()  { return transform; }
	// Queries:
	// Return image
};



#endif
