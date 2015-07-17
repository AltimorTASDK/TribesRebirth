//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _SIMMOVEOBJ_H_
#define _SIMMOVEOBJ_H_

#include <sim.h>
#include <move.h>
#include "simContainer.h"
#include "simCollideable.h"


//--------------------------------------------------------------------------- 

class SimMoveObject: public SimCollideable
{
	typedef SimCollideable Parent;

	Point3F throttle;
	Point3F desired_throttle;
	SimCollisionSphereImage collisionImage;
	Resource<SimActionMap> actionMap;

	void update(SimTime time);

public:
	MoveLinear movement;
	MoveLinear rotation;

	const Point3F& getPos(void);
	void setPos(Point3F pos);
	void setActionMap(const char* file);

	SimMoveObject();
	~SimMoveObject();
	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
	bool onAdd();
};


#endif
