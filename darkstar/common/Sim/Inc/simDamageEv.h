//
// simDamageEv.h
// 

#ifndef _SIMDAMAGEEV_H_
#define _SIMDAMAGEEV_H_

#include <simEv.h>
#include "esfEvDcl.h"

class SimCollisionInfo;

struct DLLAPI SimDamageEvent : public SimEvent
{
   enum {
      General, 
      Collision,     
      Fire,
      Concussion,
      Electrical,
      Thermal,
      Internal
   };

   SimCollisionInfo *collisionInfo;
   float damageValue;
   int damageType;
   int projectileId; // id of the projectile that created this damage
   Point3F mv;          // momentum vector
   Point3F hv;          // hitvector vector
   SimObjectId origin;  // id of object that fired the projectile
   int team;
   
   SimDamageEvent()
   { type = SimDamageEventType; 
     projectileId = -1;
     mv.set(0, 0, 0);
     team = 0;
     origin = 0;
   }
   DECLARE_PERSISTENT( SimDamageEvent );
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

#endif