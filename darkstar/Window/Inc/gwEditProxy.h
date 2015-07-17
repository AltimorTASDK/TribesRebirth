//------------------------------------------------------------------------------
// Description: VERY bare bones wrapper for a single line edit control.  Since
//               all a basic wrapper does is call Set/GetWindowText for the
//               control, and this is implemented in GWControlProxy, this is
//               an empty class for now.  Later, some of the more fancy stuff
//               may come in handy...
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GWEDITPROXY_H_
#define _GWEDITPROXY_H_

//Includes
#include "gwControlProxy.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GWEditProxy : public GWControlProxy {
  public:
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GWEDITPROXY_H_
