//----------------------------------------------------------------------------

// Console script evaluation service

//----------------------------------------------------------------------------

#include "simConsoleEvent.h"
#include "simGame.h"

//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(SimConsoleEvent, SimConsoleEventType);
IMPLEMENT_PERSISTENT(SimConsoleScheduler);

void SimConsoleEvent::postFocused(const char *buf, bool in_echo, int privLevel)
{
   SimConsoleEvent *evt = new SimConsoleEvent;
   evt->address.objectId = 0;
   strcpy(evt->argBuffer, buf);
   evt->privilegeLevel = privLevel;
   evt->argc = -1;
   evt->echo = in_echo;

   strcpy(evt->argBuffer, buf);
   evt->privilegeLevel = privLevel;

   SimManager *manager = SimGame::get()->getManager();
   if(manager)
      manager->postCurrentEvent(evt);
   else
      delete evt;
}

void SimConsoleEvent::post(SimManager *manager, SimObjectId managerId, int argc, const char **argv)
{
   SimConsoleEvent *evt = new SimConsoleEvent;
   evt->address.managerId = managerId;
   evt->flags |= SimEvent::Guaranteed | SimEvent::Ordered;
   int len = 0;
   if(argc > MaxArgs)
      return;
   int i;
   for(i = 0; i < argc;i ++)
   {
      int slen = strlen(argv[i]) + 1;
      if(len + slen  > MaxBufferSize)
         return;
      strcpy(evt->argBuffer + len, argv[i]);
      evt->argv[i] = evt->argBuffer + len;
      len += slen;
   }
   evt->argc = argc;
   manager->postCurrentEvent(evt);
}

void SimConsoleEvent::schedule(SimManager *manager, const char *evalString, float timeDelta)
{
   SimConsoleEvent *evt = new SimConsoleEvent;
   evt->address.objectId = SimConsoleSchedulerId;
   strcpy(evt->argBuffer, evalString);
   evt->time = manager->getCurrentTime() + timeDelta;
   evt->argc = -1;
   evt->privilegeLevel = 0;
   evt->echo = false;

   manager->postEvent(evt, false);
}

void SimConsoleEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   manager;
   ps;

   bstream->writeInt(argc, ArgCountBits);
   int i;
   for(i = 0; i < argc; i++)
      bstream->writeString(argv[i]);
}

void SimConsoleEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   ps;
   manager;
   
   argc = bstream->readInt(ArgCountBits);
   int i;
   int len = 0;
   strcpy(argBuffer, "remote");
   bstream->readString(argBuffer + 6);
   len += strlen(argBuffer) + 1;
   argv[0] = argBuffer;
   argv[1] = argBuffer + len;
   sprintf(argBuffer + len, "%d", sourceManagerId);
   len += strlen(argBuffer + len) + 1;

   argc--;
   for(i = 0; i < argc; i++)
   {
      argv[i + 2] = argBuffer + len;
      bstream->readString(argBuffer + len);
      len += strlen(argBuffer + len) + 1;
      if(len >= MaxBufferSize)
         break;
   }
   argc = i + 2;

   char buf[1024];
   sprintf(buf, "%s(", argv[0]);
   for(i = 1; i < argc; i++)
   {
      strcat(buf, argv[i]);
      if(i != argc - 1)
         strcat(buf, ", ");
   }
   strcat(buf, ")");

   Console->dbprintf(2, "RCMD: %d => %s", manager->getId(), buf);


   // ok, send this right to the console (c/o the manager)
   address.objectId = 0;
   privilegeLevel = 0; // change this later
   echo = false;
}

SimConsoleScheduler::SimConsoleScheduler()
{
   id = SimConsoleSchedulerId;
}
