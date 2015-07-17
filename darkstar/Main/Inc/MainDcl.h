#ifndef _MAINDCL_H_
#define _MAINDCL_H_

#include "simEvDcl.h"
#include "simPersistTags.h"

#define MainPlayerUpdateEventType               SimRangeNum(AppTypeRange, 1)
#define PlayerAddEventType                      SimRangeNum(AppTypeRange, 9)
#define PlayerRemoveEventType                   SimRangeNum(AppTypeRange, 10)

#define MainPlayerManagerId                     SimRangeNum(AppIdRange, 1)

#define MainPersTagsRange	SimPersTagsRangeEnd
#define MainPlayerPersTag	SimRangeNum(MainPersTagsRange, 0)

#endif