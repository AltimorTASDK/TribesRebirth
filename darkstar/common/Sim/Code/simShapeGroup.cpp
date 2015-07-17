//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simShapeGroup.h"
#include "simShapeGroupRep.h"
#include "simShape.h"


SimShapeGroup::SimShapeGroup()
 : m_pGroupRep(NULL)
{
   //
}

SimObject* 
SimShapeGroup::addObject(SimObject* in_pAdd)
{
   // Don't ask.  This is very scewy, but if done differently, can really screw
   //  things up.
   //
   SimObject* pRet = Parent::addObject(in_pAdd);

   SimShapeGroupRep* pGroupRep = dynamic_cast<SimShapeGroupRep*>(pRet);
   if (pGroupRep != NULL) {
      AssertFatal(m_pGroupRep == NULL,
                  "Error, more than one group rep not allowed");

      m_pGroupRep = pGroupRep;
      m_pGroupRep->initFromCurr();
      return pRet;
   }

   if (m_pGroupRep != NULL &&
       pRet        != static_cast<SimGroup*>(this)) {
      m_pGroupRep->_addObject(in_pAdd);
   }

   return pRet;
}

void 
SimShapeGroup::removeObject(SimObject* in_pRemove)
{
   // If the object we are removing is our representational object, remove from
   //  it all managed objects...
   //
   if (static_cast<SimObject*>(m_pGroupRep) == in_pRemove) {
      m_pGroupRep->removeAll();
      m_pGroupRep = NULL;
   }

   if (m_pGroupRep != NULL) {
      m_pGroupRep->_removeObject(in_pRemove);
   }

   Parent::removeObject(in_pRemove);
}

SimShapeGroupRep*
SimShapeGroup::returnGroupRep() const
{
   return m_pGroupRep;
}
