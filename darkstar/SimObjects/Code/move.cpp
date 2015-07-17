//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 


#include <sim.h>
#include "move.h"

void MoveLinear::update(SimTime dt)
{
	pos.x += dt * vel.x;
	pos.y += dt * vel.y;
	pos.z += dt * vel.z;
}


void MoveAcc::update(SimTime dt)
{
	Vector3F accdt = acc;
	accdt *= dt;

	Vector3F avrg = vel;
	vel.x = accelerate(vel.x,velTarget.x,accdt.x);
	vel.y = accelerate(vel.y,velTarget.y,accdt.y);
	vel.z = accelerate(vel.z,velTarget.z,accdt.z);

	avrg += vel;
	avrg *= .5 * dt;
	pos += avrg;
}

// Positive acceleration towards target
//
int MoveAcc::accelerate(float current, float target, float accel )
{
	if (target < current) {
		if ((current -= accel) < target)
			current = target;
	}
	else
		if (target > current)
			if ((current += accel) > target)
				current = target;
	return current;
}

