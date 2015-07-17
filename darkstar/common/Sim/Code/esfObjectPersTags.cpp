#include <link.h>
#include <ts.h>
#include "simTrigger.h"
#include "simVolumetric.h"
#include "simMarker.h"
#include "simDropPoint.h"
#include "simSoundSource.h"
#include "simMagnet.h"
#include "commonPersistTags.h"
#include "simInteriorShape.h"
#include "simShapeGroupRep.h"
#include "simShapeGroup.h"

namespace esfObjects
{
int dummy_link2 = 0;
};

IMPLEMENT_PERSISTENT_TAGS(SimTrigger,           FOURCC('t','r','g','r'), SimTriggerPersTag);
IMPLEMENT_PERSISTENT_TAGS(SimMarker,            FOURCC('m','a','r','k'), SimMarkerPersTag);
IMPLEMENT_PERSISTENT_TAGS(SimSoundSource,       FOURCC('s','n','d','s'), SimSoundSourcePersTag);
IMPLEMENT_PERSISTENT_TAGS(SimDropPoint,         FOURCC('D','P','N','T'), SimDropPointPersTag);
IMPLEMENT_PERSISTENT_TAG(SimMagnet,        SimMagnetPersTag);

IMPLEMENT_PERSISTENT_TAG(SimInteriorShape, SimInteriorShapePersTag);
IMPLEMENT_PERSISTENT_TAG(SimShapeGroupRep, SimShapeGroupRepPersTag);
IMPLEMENT_PERSISTENT_TAG(SimShapeGroup,    FOURCC('S', 's', 'g', 'p'));

// This isn't a pers tag, but we're forcing the link...
//
ForceLink(SimTSShapeLINK);

// No place for templates in this lib as of yet
//
#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma option -Jg

template SimShapeList::iterator find(SimShapeList::iterator,
                                     SimShapeList::iterator,
                                     SimShapeList::const_reference);

#endif


