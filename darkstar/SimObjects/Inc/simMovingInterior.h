//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _SIMMOVINGITR_H_
#define _SIMMOVINGITR_H_

#include <sim.h>
#include "simMovement.h"
#include "simInterior.h"


//--------------------------------------------------------------------------- 

class SimMovingInterior: public SimInterior
{
public:
   typedef SimInterior Parent;

   //
   enum State {
      Start,
      End,
      Forward,
      Backward,
      Stop,
   };

   struct Record
   {
      enum Flags {
         ForwardOnCollision =    0x1,
         AutoBackward =          0x2,
         AutoForward =           0x4,
      };

      BitSet32 flags;            // Flags enum
      float    timeScale;        // Time scale modifier
      float    collisionDelay;   // Delay after contact
      float    forwardDelay;     // Used with autoForward
      float    backwardDelay;    // Used with autoBackward

      int startSoundId;
      int stopSoundId;
      int runningSoundId;

      int collisionMask;
      int collisionDamage;
   };

private:
   //
   Record record;
   Int32 objectMask;          // Collision mask

   // State data
   State state;
   float time;
   int wayPoint;
   Point3F lPosition;
   Point3F lVector;
   Point3F aPosition;
   Point3F aVector;

   //
   bool setMoveState(State newState);
   void updateMoveState(SimTime time);
   void updatePosition();
   bool buildMovementVector(int index);

protected:
   bool onSimMovementCollisionEvent(const SimMovementCollisionEvent* event);
   bool onSimMessageEvent(const SimMessageEvent* event);
   bool onSimTimerEvent(const SimTimerEvent* event);
   bool onSimTriggerEvent(const SimTriggerEvent* event);

public:

   SimMovingInterior();
   ~SimMovingInterior();

   bool processEvent(const SimEvent*);
	bool onAdd();
	void inspectRead(Inspect *inspector);
	void inspectWrite(Inspect *inspector);

   DECLARE_PERSISTENT(SimMovingInterior);
   Persistent::Base::Error read(StreamIO &, int version, int user);
   Persistent::Base::Error write(StreamIO &, int version, int user); 
};


#endif
