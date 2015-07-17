//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <sim.h>
#include <console.h>
#include <simTagDictionary.h>
#include <simEvDcl.h>
#include "esfEvDcl.h"
#include "simLog.h"
#include "simConsolePlugin.h"
#include "esfPlugin.h"
#include "commonEditor.strings.h"
#include "simGame.h"

namespace esfObjects
{
extern int dummy_link;
};


enum CallbackID {
   ActivateObject,
   DeactivateObject,
};


//------------------------------------------------------------------------------
void ESFPlugin::init()
{
	// this is just forcing the esfObjectPersTags.cpp file to be linked in
	esfObjects::dummy_link = 0;

   console->printf("ESFPlugin");
	console->addCommand(ActivateObject,    "activateObject", this);
	console->addCommand(DeactivateObject,  "deactivateObject", this);
}

//----------------------------------------------------------------------------
const char * ESFPlugin::consoleCallback(CMDConsole* console, int id, int argc, const char* argv[])
{
   console, argc, argv;
   switch(id) 
   {
      //--------------------------------------
		case ActivateObject: {
		   if (argc != 3) 
		   {
            Console->printf("activateObject( targetId, fValue );");
		      return ("False");
		   }
         SimObject *target = manager->findObject(argv[1]);
         SimTriggerEvent event(SimTriggerEvent::Activate, atof(argv[2]));
         if (target)
            target->processEvent(&event);
		   }
			break;

      //--------------------------------------
		case DeactivateObject: {
		   if (argc != 3) 
		   {
            Console->printf("deactivateObject( targetId, fValue );");
		      return ("False");
		   }
         SimObject *target = manager->findObject(argv[1]);
         SimTriggerEvent event(SimTriggerEvent::Deactivate, atof(argv[2]));
         if (target)
            target->processEvent(&event);
		   }
         break;
    }
   return ("True");
}





