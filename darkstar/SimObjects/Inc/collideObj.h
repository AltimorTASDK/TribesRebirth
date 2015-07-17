//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _COLLIDEOBJ_H_
#define _COLLIDEOBJ_H_

#include <sim.h>
#include <move.h>
#include <simCollision.h>


//--------------------------------------------------------------------------- 

class CollideObject:  public SimObject
{
public:
   SimTime lastTime, nextTime;
	MoveLinear nextMovement;
   UInt8   collisionShape;
   int     collisions;
   float   radius;

   SimCollideable     collideable;
   SimCollisionEntry *collisionEntry;

	void collide(const SimCollisionInfoList &collisionList) {return;}
};

#endif
