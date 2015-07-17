//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMTRIGGER_H_
#define _SIMTRIGGER_H_


#include <simtrigger.h>
#include <simcollideable.h>
#include <simmovement.h>
#include <boxrender.h>
#include <inspect.h>


class SimTrigger: public SimCollideable
{
private :
   typedef SimCollideable Parent;

   enum State {
      ACTIVE,
      INACTIVE,
      DISABLED
   };

   enum NETMASKBITS
   {
      NETMASK_MOVED = 0x01,
      NETMASK_SIZED = 0x02,
   };

   struct Pair
   {
      SimObject *object;
      SimTime    expire;   
      SimTime    contact;   
      char       stringId[8];
   };

   typedef Vector<Pair> ContactList;
   ContactList contactList;

   float                rate;
   bool                 isEnabled;
   bool                 isSphere;
   BitSet32             collisionMask;
   BoxRenderImage       image;
   SimCollisionBoxImage    boxCollisionImage;
   SimCollisionSphereImage sphereCollisionImage;

   const char *className;  // a 'class' of triggers (optional)
   char stringId[8];       // my string Id

   const char* callScript(int argc, char **fn, ...);
   bool resolve;
   char *onEnterFn;
   char *onContactFn;
   char *onLeaveFn;
   char *onAddFn;
   char *onEnabledFn;
   char *onDisabledFn;

   void syncBoxes();
   void fnLookup(const char *fn, const char *name, char **result);
   void fnResolve();

protected :

   // Query Handlers
   bool onSimObjectTransformQuery(SimObjectTransformQuery *);
   bool onSimCollisionImageQuery(SimCollisionImageQuery  *);
   bool onSimRenderQueryImage(SimRenderQueryImage     *);
   bool onSimImageTransformQuery(SimImageTransformQuery  *);

      // Event Handlers
   bool onSimEditEvent(const SimEditEvent *);
   bool onSimObjectTransformEvent(const SimObjectTransformEvent   *);
   bool onSimMovementCollisionEvent(const SimMovementCollisionEvent *);
   bool onSimTimerEvent(const SimTimerEvent *event);
   void onDeleteNotify(SimObject *object);
   bool onAdd();
   void onGroupAdd();

   void setFieldValue(StringTableEntry slotName, const char *value);

public:
   SimTrigger();
   ~SimTrigger();

   bool processEvent(const SimEvent *);
   bool processQuery(SimQuery *);

   bool enable(bool tf);

   void inspectRead (Inspect *);
   void inspectWrite(Inspect *);

   DWORD packUpdate  (Net::GhostManager *, DWORD, BitStream *);
   void  unpackUpdate(Net::GhostManager *, BitStream *);

   DECLARE_PERSISTENT(SimTrigger);
   static void initPersistFields();
   Persistent::Base::Error read (StreamIO &, int, int);
   Persistent::Base::Error write(StreamIO &, int, int);
};

#endif //_SIMTRIGGER_H_
