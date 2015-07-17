//----------------------------------------------------------------------------

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)

#pragma option -Jg

#include <simDebris.h>
#include <simExplosion.h>

template class SortableVector<SimDebrisTable::Entry>;
template class SortableVector<SimExplosionTable::Entry>;

#endif


