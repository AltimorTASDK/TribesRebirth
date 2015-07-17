#ifndef _H_SIMPERSISTTAGS_
#define _H_SIMPERSISTTAGS_

#include "simEvDcl.h"

#define SimPersTagsRange            (1)
#define SimVolumePersTag            SimRangeNum(SimPersTagsRange, 0)
#define SimTerrainPersTag           SimRangeNum(SimPersTagsRange, 1)
#define SimPalettePersTag           SimRangeNum(SimPersTagsRange, 2)
#define SimInteriorPersTag          SimRangeNum(SimPersTagsRange, 3)
#define SimSkyPersTag               SimRangeNum(SimPersTagsRange, 4)
#define SimLightPersTag             SimRangeNum(SimPersTagsRange, 5)
#define SimPlanetPersTag            SimRangeNum(SimPersTagsRange, 6)
#define SimStarFieldPersTag         SimRangeNum(SimPersTagsRange, 7)
#define SimExplosionPersTag         SimRangeNum(SimPersTagsRange, 8)
#define SimExplosionCloudPersTag    SimRangeNum(SimPersTagsRange, 9) 
#define SimFirePersTag              SimRangeNum(SimPersTagsRange, 10)
#define SimDebrisPersTag            SimRangeNum(SimPersTagsRange, 11)
#define SimDebrisCloudPersTag       SimRangeNum(SimPersTagsRange, 12)
#define SimInteriorGroupingPersTag  SimRangeNum(SimPersTagsRange, 13)
#define SimMovingInteriorPersTag    SimRangeNum(SimPersTagsRange, 14)
#define SimInteriorShapePersTag     SimRangeNum(SimPersTagsRange, 15)
#define SimShapeGroupRepPersTag     SimRangeNum(SimPersTagsRange, 16)

#define SimPersTagsRangeEnd   (127)

#endif