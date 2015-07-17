//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gwMsgMap.h"


const gwMsgMapEntry* 
GWFindMsgMapEntry(const gwMsgMapEntry* in_pEntries,
                  const UInt32         in_messageId,
                  const UInt32         in_code,
                  const UInt32         in_id)
{
   for (int i = 0; in_pEntries[i].paramSig != GWParamSig_End; i++) {
      const gwMsgMapEntry* pEntry = &in_pEntries[i];
      
      if (pEntry->messageId == in_messageId                    &&
          pEntry->code      == in_code                         &&
          ((in_id >= pEntry->id) && (in_id <= pEntry->lastId))) {
         return pEntry;
      }
   }

   return NULL;
}
