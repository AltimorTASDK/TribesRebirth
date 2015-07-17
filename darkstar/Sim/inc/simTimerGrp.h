//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _SIMTIMERGRP_H_
#define _SIMTIMERGRP_H_

#include <simbase.h>
#include <simev.h>


//--------------------------------------------------------------------------- 

class SimTimerSet: public SimSet
{
	typedef SimSet Parent;

  protected:
   SimTime timerInterval;

  public:
   bool onAdd();
	SimTimerSet(SimTime interval, SimObjectId id);
	bool processEvent(const SimEvent*);
};


#endif
