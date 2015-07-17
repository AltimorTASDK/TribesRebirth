//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _SIMCONSOLE_H_
#define _SIMCONSOLE_H_

#include <sim.h>


//----------------------------------------------------------------------------

class SimConsoleEvent: public SimEvent
{
public:
	SimConsoleEvent() { type = SimConsoleEventType; }
   Int32 argc;
   bool echo;
   int privilegeLevel;
   enum {
      MaxArgs = 31,
      ArgCountBits = 5,
      MaxBufferSize = 512
   };

   const char *argv[MaxArgs + 1];
   char argBuffer[MaxBufferSize + 256];

   static void post(SimManager *, SimObjectId managerId, int argc, const char **argv);
   static void postFocused(const char *str, bool echo, int privLevel);

   static void schedule(SimManager *, const char *evalString, float timeDelta);

   virtual void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   virtual void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);

   DECLARE_PERSISTENT( SimConsoleEvent );
};

class SimConsoleScheduler : public SimObject
{
   typedef SimObject Parent;
public:
   SimConsoleScheduler();
   DECLARE_PERSISTENT( SimConsoleScheduler );
};

#endif

