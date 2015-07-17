//----------------------------------------------------------------------------
// Fear/Tribes Version
//----------------------------------------------------------------------------

#include "version.h"


//the version number must be the major version, '.', the minor version
#ifdef DEBUG

//the debug major version must be an odd number
const char *FearVersion = "1.3.12d";
#else

//the release major version must be an even number
const char *FearVersion = "1.3.12";
#endif

const char *BuildTimeStamp = __TIME__;
const char *BuildDateStamp = __DATE__;