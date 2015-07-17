#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)

#pragma option -Jg

#include "ml.h"
//#include "grdbase.h"
//#include "grdbuff.h"
//#include "grdcam.h"
//#include "grdrend.h"

template const int &min(const int &, const int &);
template const int &max(const int &, const int &);
template const long &min(const long &, const long &);
template const long &max(const long &, const long &);
template const float &min(const float &, const float &);
template const float &max(const float &, const float &);

#endif