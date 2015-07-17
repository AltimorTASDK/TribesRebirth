//--------------------------------------------------------------------------- 
// SimDebrisRecord.h
//--------------------------------------------------------------------------- 

#ifndef _SIMDEBRISRECORD_H_
#define _SIMDEBRISRECORD_H_

struct SimDebrisRecord
{
	Int32 id;                       // used to look-up debris
	Int32 type;                     // used for debris selection

	int shapeTag;                   // id in SimTagDictionary of shape filename

	int imageType;					     // these needed in case we have translucent debris
	int imageSortValue;

	// physical description
	float mass;
	float elasticity;
	float friction;
	float center[3];                // center of mass -- offset from position

	Int32 collisionMask;
	Int32 knockMask;                // things that know us around

	int   animationSequence;        // sequence to animate, -1 for no animation
	float animRestartTime; 	        // restart animation after this amount of time

	float minTimeout;
	float maxTimeout;

	float explodeOnBounce;          // probability of exploding on bounce

	float damage;                   // in units of velocity...
	float damageThreshhold;

	Int32 spawnedDebrisMask;
	float spawnedDebrisStrength;
	float spawnedDebrisRadius;

	int   spawnedExplosionID;       // explosion to make when expiring -- 0 for none

	float p;                        // probability density of this item

   int   explodeOnRest;
   int   collisionDetail;          // 0 for sphere, 1 for box, and 2 for detailed
};



#endif