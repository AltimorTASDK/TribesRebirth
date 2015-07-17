//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

#ifndef _MOVE_H_
#define _MOVE_H_

#include <ml.h>
#include <simBase.h>

struct MoveLinear
{
	Point3F pos;
	Vector3F vel;

	void update(SimTime dt);
};

struct MoveAcc: public MoveLinear
{
	Vector3F velTarget;
	Vector3F acc;

	static int accelerate(float current,float target,float acc);
	void update(SimTime dt);
};


#endif
