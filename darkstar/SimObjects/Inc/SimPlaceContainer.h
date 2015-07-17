//
// SimPlaceContainer -- This is a container that is meant to be placed at
//                      strategic locations to ease the strain on the overall
//                      container database.  An example of a strategic location
//                      would be surrounding a group of closely spaced buildings.
//                      The gain of this would be that when checking for collisions,
//                      the buildings could be rejected in one check when not in the
//                      group.  Additionally, a SimPlaceContainer can have a mask
//                      associated with it, to keep out all but certain kinds of objects.
//                      For example, one could have a global projectile container
//                      that would then only have to be querried when looking for
//                      projectiles.
//

#ifndef _SIMPLACECONTAINER_H_
#define _SIMPLACECONTAINER_H_

#include <simContainer.h>
#include <simCoordDB.h>
#include <simTerrain.h>

class SimPlaceContainer : public SimContainer
{
   typedef SimContainer Parent;

   SimTerrain * terrain;
   
   void findTerrain();
   
   bool onAdd();
   void onDeleteNotify(SimObject *);
   
   void enterContainer(SimContainer*);
   void leaveContainer(SimContainer*);
   
   public:
      SimPlaceContainer(const Box3F & box, Int32 mask = -1, bool ghost = false);
      SimPlaceContainer(Int32 mask = -1, bool ghost = false);
      
      bool render(SimContainerRenderContext &);
      float getHazeValue(const Point3F &, const Point3F &);
      float getHazeValue(TSRenderContext & rc, const Point3F & pos);
};


#endif // _SIMPLACECONTAINER_H_


