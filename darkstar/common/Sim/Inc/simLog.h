//--------------------------------------------------------------------------- 
//
// simLog.h
//
//--------------------------------------------------------------------------- 

#ifndef _SIMLOG_H_
#define _SIMLOG_H_

#include <sim.h>
#include <gwLog.h>

//--------------------------------------------------------------------------- 

class SimLog : public SimObject, public GWLog
{
	public:

		SimLog(LPCSTR name) : GWLog(name) {}
		~SimLog() {GWLog::destroyWindow();}

		bool processEvent(const SimEvent*) {return false;}
   	bool processQuery(SimQuery*) {return false;} 
  
  	  	void onClose() {manager->deleteObject(this);}
};

#endif
