//----------------------------------------------------------------------------

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)

#pragma option -Jg

#include "simContainer.h"
#include "simInput.h"

template class SortableVectorPtr<SimRenderImage*>;
template class SortableVector<SimContainer*>;
template SimContainer* findObject(SimManager* manager,SimObjectId id,SimContainer*);
template SimInputManager* findObject(SimManager* manager,SimObjectId id,SimInputManager*);
template SimContainerList::iterator find(SimContainerList::iterator,SimContainerList::iterator,SimContainerList::const_reference);
template const unsigned long &min(const unsigned long &, const unsigned long &);

#endif


