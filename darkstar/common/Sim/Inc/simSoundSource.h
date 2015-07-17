//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMSOUNDSOURCE_H_
#define _SIMSOUNDSOURCE_H_

#include "simMarker.h"
#include <soundFX.h>

class SimSoundSource : public SimMarker
{
   private:
      typedef SimMarker Parent;   	
   
      Int32 channel;
      DWORD soundId;

   public:
      SimSoundSource();
   
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
		DECLARE_PERSISTENT(SimSoundSource);
		Error read(StreamIO &, int version, int);
		Error write(StreamIO &, int version, int);
};   

#endif //_SIMSOUNDSOURCE_H_
					 		