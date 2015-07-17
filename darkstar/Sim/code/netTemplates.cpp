//----------------------------------------------------------------------------

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)

#pragma option -Jg

#include "netGhostManager.h"

template class SortableVector<Net::GhostManager::UpdateQueueEntry>;
template class SortableVector<Net::GhostManager::AddListEntry>;

#endif

