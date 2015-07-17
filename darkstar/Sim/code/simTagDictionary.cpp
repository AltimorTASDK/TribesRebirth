//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <simTagDictionary.h>
#include "tagDictionary.h"

//------------------------------------------------------------------------------

// for backwards compatability
const char* SimTagDictionary::getString(SimManager *manager, int strId)
{
   manager;
   return tagDictionary.idToString(strId);
}

int SimTagDictionary::getDefine(SimManager *manager, const char *define)
{
   manager;
   const char *searchStr = stringTable.insert(define);
   return tagDictionary.defineToId(searchStr);
}   

const char* SimTagDictionary::getDefineString(SimManager *manager, int defId)
{
   manager;
   return tagDictionary.idToDefine(defId);
}   

//--------------------------------------
// prefered interface
const char* SimTagDictionary::getString(int strId)
{
   return tagDictionary.idToString(strId);
}

int SimTagDictionary::getDefine(const char *define)
{
   const char *searchStr = stringTable.insert(define);
   return tagDictionary.defineToId(searchStr);
}   

const char* SimTagDictionary::getDefineString(int defId)
{
   return tagDictionary.idToDefine(defId);
}   
