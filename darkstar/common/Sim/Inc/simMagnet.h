//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMMAGNET_H_
#define _SIMMAGNET_H_

#include "simMarker.h"
#include <soundFX.h>

class SimMagnet : public SimMarker
{
   private:
      typedef SimMarker Parent;   	
   
      float force;

   public:
      SimMagnet();
   
   protected:
      // default shape for the object
      Int32 getDefaultShape() { return IDCOMEDIT_SHP_2M_CUBE; }

		// Query Handlers
      
		// Event Handlers
      bool onSimTriggerEvent(const SimTriggerEvent *event);
      
   public:
		bool processEvent(const SimEvent *);
		void inspectRead(Inspect* inspect);
		void inspectWrite(Inspect* inspect);
		
      // Persistent IO
		DECLARE_PERSISTENT(SimMagnet);
		Error read(StreamIO &, int version, int);
		Error write(StreamIO &, int version, int);
};   

#endif //_SIMMAGNET_H_
					 		