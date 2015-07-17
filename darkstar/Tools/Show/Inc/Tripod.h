//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _tripod_H_
#define _tripod_H_

#include <simResource.h>
#include <simAction.h>
#include <simRenderGrp.h>
#include <ts_shapeInst.h>
#include "mover.h"


#define COREAPI
class myGuy;

//--------------------------------------------------------------------------- 

extern char *actionTable[];

class tripod: public SimObject
{
   typedef SimObject ObjParent;

protected:
//   SimActionEventMap eventMap;
   moverHelper mover;
   bool angleRelative; // angle relative to actor or absolute
   myGuy *actor;
   void update(SimTime time);
   void setEventMap();
   void init();
   float objectRadius;
   float Hoffcenter;
   float Voffcenter;
   Point3F offset;
   
   Resource<SimActionMap> eventMap;
   

public:
   tripod();
   ~tripod();

   void setPos(Point3F &np) { mover.setPos(np); }
   void setRot(EulerF &nr) { mover.setRot(nr); }
   const Point3F & getPos() { return mover.getPos(); }
   const EulerF & getRot() { return mover.getRot(); }
   const EulerF & getOrbitRot() { return mover.getOrbitRot(); }
   void setOrbitRot(const EulerF &nor) { mover.setOrbitRot(nor); }
   void setOffset(const Point3F &newo) { offset=newo; }
   void setHoffset(float newo) { Hoffcenter=newo; }
   void setVoffset(float newo) { Voffcenter=newo; }
   float getHoffset() { return Hoffcenter; }
   float getVoffset() { return Voffcenter; }
   void setActor(myGuy *na) { actor = na; }
   void setDistance(float nd) { mover.setOrbitDist(nd); }
   float getDistance() { return mover.getOrbitDist(); }
   void setSphere(float nrad) { objectRadius=nrad; }
   float getSphere() { return objectRadius; }
   bool hasActor() { return actor!=0; }
   myGuy *getActor() { return actor; }
   void setRelative(bool nval) { angleRelative=nval; }
   bool getRelative() { return angleRelative; }
    bool onAdd();

   bool COREAPI processEvent(const SimEvent*);
   bool COREAPI processQuery(SimQuery*);
};
#endif
