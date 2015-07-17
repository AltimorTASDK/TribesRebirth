#include <simPlaceContainer.h>
#include <SimMovement.h>

SimPlaceContainer::SimPlaceContainer(Int32 mask, bool ghost)
{
   containerMask = mask;
   installDatabase(new SimXYDB());
   terrain = NULL;

   if (ghost)
   {
      netFlags.set(SimNetObject::ScopeAlways);
      netFlags.set(SimNetObject::Ghostable);
   }
}

SimPlaceContainer::SimPlaceContainer(const Box3F & box, Int32 mask, bool ghost)
{
   containerMask = mask;
   setBoundingBox(box);
   installDatabase(new SimXYDB());
   terrain = NULL;
   
   if (ghost)
   {
      netFlags.set(SimNetObject::ScopeAlways);
      netFlags.set(SimNetObject::Ghostable);
   }
}

void SimPlaceContainer::findTerrain()
{
   // look up terrain if we don't already have it -- assumes we have manager
   if (!terrain)
   {
      terrain = static_cast<SimTerrain*>(manager->findObject(SimTerrainId));
      if (terrain)
         deleteNotify(terrain);
   }
}

bool SimPlaceContainer::onAdd()
{
	if (!Parent::onAdd())
		return false;

	SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);
   AssertFatal(root,"must create root container first");
	root->addObject(this);
	return true;
}

void SimPlaceContainer::onDeleteNotify(SimObject * obj)
{
   if (obj==terrain)
      terrain=NULL;
   Parent::onDeleteNotify(obj);
}

bool SimPlaceContainer::render(SimContainerRenderContext & rc)
{
   findTerrain();
   
   // set far distance every render just in case it changes...
   if (terrain)
      static_cast<SimXYDB*>(getDatabase())->setFarDist(terrain->getVisibleDistance());
   else
      static_cast<SimXYDB*>(getDatabase())->setFarDist(10E10f);

   return Parent::render(rc);
}

float SimPlaceContainer::getHazeValue(const Point3F & camPos, const Point3F & objPos)
{
   findTerrain();
   
   if (terrain)
      return terrain->getHazeValue(camPos,objPos);
   if (getContainer())
      return getContainer()->getHazeValue(camPos,objPos);
   return 0.0f;
}

float SimPlaceContainer::getHazeValue(TSRenderContext & rc, const Point3F & objPos)
{
   findTerrain();
   
   if (terrain)
      return terrain->getHazeValue(rc,objPos);
   if (getContainer())
      return getContainer()->getHazeValue(rc,objPos);
   return 0.0f;
}

//----------------------------------------------------------------------------
// Management of SimMovement objects comming in and out of the container.
//----------------------------------------------------------------------------

void SimPlaceContainer::enterContainer(SimContainer* obj)
{
   findTerrain();
   
	Parent::enterContainer(obj);
   if (!terrain)
      return;
	if (SimMovement* mobj = dynamic_cast<SimMovement*>(obj)) {
		mobj->addForce(SimMovementGravityForce,terrain->getContainerProperties().gravity,true);
		mobj->setContainerDrag(terrain->getContainerProperties().drag);
		mobj->setContainerCurrent(Point3F(.0f,.0f,.0f),.0f);
	}
}

void SimPlaceContainer::leaveContainer(SimContainer* obj)
{
	if (SimMovement* mobj = dynamic_cast<SimMovement*>(obj))
		mobj->removeForce(SimMovementGravityForce);
	Parent::leaveContainer(obj);
}


