//----------------------------------------------------------------------------

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)

#pragma option -Jg

#include "mpmState.h"
#include "mpmPseudoTrace.h"
#include "mpmPopTable.h"
#include <map>

template PseudoTrace& operator<<(PseudoTrace&, const char*);
template PseudoTrace& operator<<(PseudoTrace&, char*);
template PseudoTrace& operator<<(PseudoTrace&, unsigned int);
template PseudoTrace& operator<<(PseudoTrace&, int);
template PseudoTrace& operator<<(PseudoTrace&, unsigned long);
template PseudoTrace& operator<<(PseudoTrace&, long);
template PseudoTrace& operator<<(PseudoTrace&, float);
template PseudoTrace& operator<<(PseudoTrace&, char);
template PseudoTrace& operator<<(PseudoTrace&, unsigned char);

template class std::map<PALETTEENTRY, float, PEntryLess>;

#endif


