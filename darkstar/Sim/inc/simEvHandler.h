//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMEVHANDLER_H_
#define _SIMEVHANDLER_H_


#include <evBase.h>
#include "simBase.h"
#include "simEv.h"


//------------------------------------------------------------------------------
// SimEventHandler Object

class SimEventHandler: public SimObject
{
private:
	EventHandler *evh;

public:
	SimEventHandler();
	~SimEventHandler();
	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
	
	EventHandler* get();
	static EventHandler* get(SimManager *manager);
	static SimEventHandler* find(SimManager *manager);
};	


//--------------------------------------INLINEs

inline SimEventHandler::SimEventHandler()
{
	id = SimEventHandlerId;
   evh = new EventHandler;
}

inline SimEventHandler::~SimEventHandler()
{
   delete evh;
}

inline bool SimEventHandler::processEvent(const SimEvent *e)
{
   return ( false );
}

inline bool SimEventHandler::processQuery(SimQuery *q)
{
   return ( false );
}

inline EventHandler* SimEventHandler::get()
{
	return ( evh );	
}

inline EventHandler* SimEventHandler::get(SimManager *manager)
{
   SimEventHandler *obj = ( static_cast<SimEventHandler*>
   	(manager->findObject(SimEventHandlerId)) );
	if ( obj ) return ( obj->evh );
	else return ( NULL ); 
}

inline SimEventHandler* SimEventHandler::find(SimManager *manager)
{
   return ( static_cast<SimEventHandler*>
   	(manager->findObject(SimEventHandlerId)) );
}



#endif //_EVOBJECT_H_
