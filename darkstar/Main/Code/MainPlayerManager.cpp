#include "simBase.h"
#include "MainPlayerManager.h"
#include "console.h"
#include "ts.h"
#include "netGhostManager.h"
#include "mainPlayer.h"

//---------------------------------------------------------------

class PlayerAddEvent : public SimEvent
{
public:
   int pid;
   char name[MaxPlayerNameLen];

   PlayerAddEvent();

   static void post(SimManager *manager, MainPlayerManager::PlayerRep *pr, int destManagerId);

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);

   DECLARE_PERSISTENT(PlayerAddEvent);
};

IMPLEMENT_PERSISTENT_TAG(PlayerAddEvent, PlayerAddEventType);

PlayerAddEvent::PlayerAddEvent()
{
   type = PlayerAddEventType;
   flags.set(SimEvent::Ordered);
}

void PlayerAddEvent::post(SimManager *manager, MainPlayerManager::PlayerRep *pr, int destManagerId)
{
   PlayerAddEvent *ev = new PlayerAddEvent();

   ev->pid = pr->id;
   strcpy(ev->name, pr->name);

   ev->address.objectId = MainPlayerManagerId;
   ev->address.managerId = destManagerId;

   manager->postCurrentEvent(ev);
}

void PlayerAddEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

   bs->write(pid);
   bs->writeString(name);
}

void PlayerAddEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

   bs->read(&pid);
   bs->readString(name);

   address.objectId = MainPlayerManagerId;
}

//---------------------------------------------------------------

class PlayerRemoveEvent : public SimEvent
{
public:
   int pid;

   PlayerRemoveEvent();

   static void post(SimManager *manager, int removeId, int destManagerId);

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);

   DECLARE_PERSISTENT(PlayerRemoveEvent);
};

IMPLEMENT_PERSISTENT_TAG(PlayerRemoveEvent, PlayerRemoveEventType);

PlayerRemoveEvent::PlayerRemoveEvent()
{
   type = PlayerRemoveEventType;
   flags.set(SimEvent::Ordered);
}

void PlayerRemoveEvent::post(SimManager *manager, int removeId, int destManagerId)
{
   PlayerRemoveEvent *ev = new PlayerRemoveEvent();

   ev->pid = removeId;
   ev->address.objectId = MainPlayerManagerId;
   ev->address.managerId = destManagerId;

   manager->postCurrentEvent(ev);
}

void PlayerRemoveEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

   bs->write(pid);
}

void PlayerRemoveEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

   bs->read(&pid);
   address.objectId = MainPlayerManagerId;
}


//---------------------------------------------------------------
//---------------------------------------------------------------

MainPlayerManager::MainPlayerManager(bool onServer)
{
   isServer = onServer;
   id = MainPlayerManagerId;
}

void MainPlayerManager::playerAdded(int playerId, const char *playerName)
{
   CMDConsole *con = CMDConsole::getLocked();
   con->printf("Server: Player \"%s\" added - id = %d", playerName, playerId);

   PlayerRep p;
   p.id = playerId;
   p.player = NULL;

   strncpy(p.name, playerName, MaxPlayerNameLen-1);
   p.name[MaxPlayerNameLen-1] = 0;

   Vector<PlayerRep>::iterator i;
   for(i = playerList.begin(); i != playerList.end(); i++)
   {
      PlayerAddEvent::post(manager, &p, (*i).id);
      PlayerAddEvent::post(manager, i, playerId);
   }
   playerList.push_back(p);
   PlayerAddEvent::post(manager, &p, playerId);
}

void MainPlayerManager::playerDropped(int playerId)
{
   Vector<PlayerRep>::iterator i;
   for(i = playerList.begin(); i != playerList.end();)
   {
      if((*i).id == playerId)
      {
         CMDConsole *con = CMDConsole::getLocked();
         con->printf("Server: Player \"%s\" dropped - id = %d", (*i).name, (*i).id);

         i->player->deleteObject();
         playerList.erase(i);
      }
      else
      {
         PlayerRemoveEvent::post(manager, playerId, (*i).id);
         i++;
      }
   }
}

bool MainPlayerManager::processQuery(SimQuery *)
{
   return false;
}

bool MainPlayerManager::processEvent(const SimEvent *event)
{
   CMDConsole *con = CMDConsole::getLocked();
   switch (event->type)
   {
      case PlayerAddEventType:
         PlayerRep pr;
         PlayerAddEvent *fpae;

         if(isServer)
            return false;

         fpae = (PlayerAddEvent *) event;

         pr.id = fpae->pid;
         strcpy(pr.name, fpae->name);
         pr.player = NULL;
         playerList.push_back(pr);
         con->printf("%s joined the game.", pr.name);
         break;

      case PlayerRemoveEventType:
         int id;
         if(isServer)
            return false;
         id = ((PlayerRemoveEvent *) event)->pid;
         Vector<PlayerRep>::iterator i;
         for(i = playerList.begin(); i != playerList.end(); i++)
            if((*i).id == id)
            {
               con->printf("%s dropped.", (*i).name);
               playerList.erase(i);
               break;
            }
         break;
   }
   return false;
}

void MainPlayerManager::reset()
{
   playerList.setSize(0);
}

void MainPlayerManager::dropInGame(int playerId)
{
	MainPlayer *player = new MainPlayer();
   Net::PacketStream *pStream = (Net::PacketStream *) manager->findObject(playerId);   
   Net::GhostManager *gm = (Net::GhostManager *) pStream->getGhostManager();

   player->setGhostInfo(gm, playerId);
   PlayerRep *pr = findPlayer(playerId);

   pr->player = player;

   manager->addObject(player);
   gm->setScopeObject(player);
}

MainPlayerManager::PlayerRep *MainPlayerManager::findPlayer(int id)
{
   Vector<PlayerRep>::iterator i;
   for(i = playerList.begin(); i != playerList.end(); i++)
      if(i->id == id)
         return i;
   return NULL;
}