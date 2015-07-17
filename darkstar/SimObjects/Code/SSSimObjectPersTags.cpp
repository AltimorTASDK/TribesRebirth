#include <ts.h>
#include "simLight.h"
#include "simSky.h"
#include "simStarField.h"
#include "simPlanet.h"
#include "simMovingInterior.h"
#include "simPath.h"
#include "simInteriorGrouping.h"
#include "simPersistTags.h"
#include "simExplosion.h"
#include "simDebris.h"

namespace SimObjects
{
int dummy_link2 = 0;
};

IMPLEMENT_PERSISTENT(SimExplosionTable);
IMPLEMENT_PERSISTENT_TAGS(SimSky, FOURCC('S','K','Y','T'), SimSkyPersTag);
IMPLEMENT_PERSISTENT_TAGS(SimPlanet, FOURCC('S','K','P','L'), SimPlanetPersTag);
IMPLEMENT_PERSISTENT_TAGS(SimStarField, FOURCC('S','K','S','F'), SimStarFieldPersTag);
IMPLEMENT_PERSISTENT_TAGS(SimMovingInterior, FOURCC('S','M','I','T'), SimMovingInteriorPersTag);
IMPLEMENT_PERSISTENT_TAG(SimInteriorGrouping, SimInteriorGroupingPersTag);
IMPLEMENT_PERSISTENT(SimDebrisTable);

