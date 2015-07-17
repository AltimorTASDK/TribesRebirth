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
int dummy_link = 0;
};

IMPLEMENT_PERSISTENT_TAG(SimPath, FOURCC('S', 'P', 'T','H'));
IMPLEMENT_PERSISTENT_TAGS(SimLight, FOURCC('S', 'l', 'i', 't'), SimLightPersTag );
