#ifndef _H_COMMONPERSISTTAGS_
#define _H_COMMONPERSISTTAGS_

#include "simEvDcl.h"
#include "simPersistTags.h"

#define CommonPersTagsRange          (SimPersTagsRangeEnd + 1)

#define ESFProjPersTag               SimRangeNum(CommonPersTagsRange, 0)
#define SimMarkerPersTag             SimRangeNum(CommonPersTagsRange, 1)
#define SimDropPointPersTag          SimRangeNum(CommonPersTagsRange, 2)
#define SimSoundSourcePersTag        SimRangeNum(CommonPersTagsRange, 3)
#define SimTriggerPersTag            SimRangeNum(CommonPersTagsRange, 4)
#define SimMagnetPersTag		       SimRangeNum(CommonPersTagsRange, 5)
#define SimVolumetricPersTag         SimRangeNum(CommonPersTagsRange, 6)


#define CommonPersTagsRangeEnd       (SimPersTagsRangeEnd + 127)

#endif