//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _ESFOBJECTTYPES_H_
#define _ESFOBJECTTYPES_H_

#define SimMarkerObjectType	(1 << 10)
#define SimTriggerObjectType  (1 << 11)

// these object types extend the set defined in simObjectTypes.h
// NOTE:  If you add or modify these types, you will also have to make
//  a modification to the RegisterObjects.cs script file.  Make sure
//  that all types are registered with the MissionEditor using the 
//  MissionRegType console command.

#endif
