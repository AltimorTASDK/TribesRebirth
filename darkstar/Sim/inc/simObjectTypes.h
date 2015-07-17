//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _SIMOBJECTTYPES_H_
#define _SIMOBJECTTYPES_H_

// The type field is a bit set.

// Objects are assigned the default type in the SimObject constructor.
// The default is 1 because some systems use a mask against the
// object type field, if it's zero, nothing ever matches.

#define SimDefaultObjectType     (1 << 0)
#define SimTerrainObjectType     (1 << 1)
#define SimInteriorObjectType    (1 << 2)
#define SimCameraObjectType      (1 << 3)
#define SimMissionObjectType     (1 << 4)
#define SimShapeObjectType       (1 << 5)
#define SimContainerObjectType   (1 << 6)
#define SimPlayerObjectType      (1 << 7)
#define SimProjectileObjectType  (1 << 8)
#define SimVehicleObjectType     (1 << 9)

// the bit-shifts 12+ are reserved for application specific types

// NOTE: If you add or modify these types, you must make a corresponding
//  modification in the esfObjectTypes.h and the MissionPlugin.cpp file.  
//  There are some console commands to register these types and their 
//  descriptions with the Mission Editor.  You will also have to 
//  register your application specific types in your RegisterObjects.cs 
//  script.

#endif
