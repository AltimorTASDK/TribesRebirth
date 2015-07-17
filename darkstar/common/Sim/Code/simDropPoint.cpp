//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "SimDropPoint.h"
#include "simGame.h"

//-------------------------------------------------------------------------- 

bool SimDropPoint::onAdd()
{
	if (!Parent::onAdd())
		return false;
	if (manager == SimGame::get()->getManager(SimGame::SERVER))
   {
      // add the dropPoint to a set so it can be queried on scenario startup
   	// create the set, if necessary
   	SimSet *dpg = static_cast<SimSet *>(manager->findObject(SimDropPointSetId));
   	if(!dpg)
   	{
   		dpg = new SimSet();
   		dpg->setId(SimDropPointSetId);
   		manager->addObject(dpg);
   	}
   	dpg->addObject(this);
   }
	return true;
}

//-------------------------------------------------------------------------- 

void SimDropPoint::onRemove()
{
	Parent::onRemove();   
}

