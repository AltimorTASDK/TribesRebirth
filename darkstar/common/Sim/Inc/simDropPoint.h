//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMDROPPOINT_H_
#define _SIMDROPPOINT_H_

#include "simMarker.h"

class SimDropPoint : public SimMarker
{
   private:
      typedef SimMarker Parent;
   
   protected:
		// default shape for the object
      Int32 getDefaultShape() { return IDCOMEDIT_SHP_5M_ARROW_RED; }

   public:
		bool onAdd();
		void onRemove();

		// Persistent IO
		DECLARE_PERSISTENT(SimDropPoint);
};   

#endif //_SIMDROPPOINT_H_
					 		