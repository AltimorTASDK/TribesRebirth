//--------------------------------------------------------------------------- 
// SimFireRecord.h
//--------------------------------------------------------------------------- 

#ifndef _SIMFIRERECORD_H_
#define _SIMFIRERECORD_H_

#define nosound (-1)

struct SimFireRecord
{
	Int32 id;                       // used to look-up fire

	int shapeTag;                   // id in SimTagDictionary of shape filename
	int shapeTagT;                  // id in SimTagDictionary of translucent version of shape
	int soundID;                    // id used to play sound on detonation

	int smokeTag;                   // id in tag dictionary of smoke filename
	int smokeTagT;                  // tag of translucent version of smoke
	float ISI;                      // inter-smoke-interval -- delay between smoke puffs
	float smokeDuration;            // duration of each smoke puff
	float smokeXoffset;
	float smokeYoffset;
	float smokeZoffset;
   
   float lightRange;               // dynamic light range (0 means no light)
   float r,g,b;                    // light color

	int faceCam;                    // should the shape face the camera? (0=no,1=yes,2=follow)
};


#endif