//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMTAGDICTIONARY_H_
#define _SIMTAGDICTIONARY_H_

#include <sim.h>

//------------------------------------------------------------------------------
typedef Int32 SimTag;

class DLLAPI SimTagDictionary: public SimObject
{
public:
   // for backwards compatability
   static const char* getString(SimManager *manager, int strId);
   static int         getDefine(SimManager *manager, const char *define);
   static const char* getDefineString(SimManager *manager, int defId);

   // prefered interface
   static const char* getString(int strId);
   static int         getDefine(const char *define);
   static const char* getDefineString(int defId);
};

//------------------------------------------------------------------------------


#endif   // _SIMTAGDICTIONARY_H_
