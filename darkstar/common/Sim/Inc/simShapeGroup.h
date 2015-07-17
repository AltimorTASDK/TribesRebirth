//------------------------------------------------------------------------------
// Description: A very simple extension of a simGroup.  Simply keeps track of
//               a SimShapeGroupRep that is added to it.  Only allows [01] of these
//               to belong to the group at any one time.
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMSHAPEGROUP_H_
#define _SIMSHAPEGROUP_H_

//Includes
#include "simBase.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class SimShape;
class SimShapeGroupRep;

class SimShapeGroup : public SimGroup
{
   typedef SimGroup Parent;

   SimShapeGroupRep* m_pGroupRep;

  public:
   SimShapeGroup();

	SimObject* addObject(SimObject*);
	void       removeObject(SimObject*);
   
   SimShapeGroupRep* returnGroupRep() const;
   
   // We need this object to be persistent, but it is not necessary to override
   //  read/write...
   DECLARE_PERSISTENT(SimShapeGroup);
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SIMSHAPEGROUP_H_
