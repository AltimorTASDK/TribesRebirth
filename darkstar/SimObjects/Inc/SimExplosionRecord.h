//--------------------------------------------------------------------------- 
// SimExplosionRecord.h
//--------------------------------------------------------------------------- 

#ifndef _SIMEXPLOSIONRECORD_H_
#define _SIMEXPLOSIONRECORD_H_

#define EXP_nosound (-1)

struct SimExplosionRecord
{
	Int32 id;                       // used to look-up explosion
	Int32 type;                     // used for explosion selection

	int shapeTag;                   // id in SimTagDictionary of shape filename
	int translucentShapeTag;        // id in SimTagDictionary of translucent version of shape
	int soundID;                    // id used to play sound on detonation

	float timeScale;                // must be positive

	int faceCam;                    // should the shape face the camera? (1=yes,0=no)
   int spin;                       // rotate explosion? (1=yes 0=no)

	// physical description
	float megatons;

   int   hasLight;                 // has a dynamic light?
   float r0, g0, b0;               // dynamic light color attached to this explosion
   float r1, g1, b1;               // dynamic light color attached to this explosion
   float r2, g2, b2;               // dynamic light color attached to this explosion
   float radFactor0;
   float radFactor1;
   float radFactor2;
   float range;                    // dynamic light range attached to this explosion
   float t0;                       // time (0..1) at which the light is full brightness
   float t1;                       // time (0..1) at which the light begins to fall off
   int   preventShiftPos;
};



#endif