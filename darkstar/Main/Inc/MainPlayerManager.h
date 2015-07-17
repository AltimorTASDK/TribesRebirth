#ifndef _H_MAINPLAYERMANAGER_
#define _H_MAINPLAYERMANAGER_

#include "MainDcl.h"
#define MaxPlayerNameLen 64
class SimNetObject;

class MainPlayerManager : public SimObject
{
   bool isServer;
public:
   struct PlayerRep
   {
      int id;
      char name[MaxPlayerNameLen];
      SimNetObject *player;
   };
   Vector<PlayerRep> playerList;

   MainPlayerManager(bool onServer);
   void playerAdded(int playerId, const char *playerName);
   void playerDropped(int playerId);
   void reset();
   void dropInGame(int playerId);

   bool processQuery(SimQuery *q);
   bool processEvent(const SimEvent *event);

   PlayerRep *findPlayer(int id);

   static MainPlayerManager *get(SimManager *manager);
};

inline MainPlayerManager *MainPlayerManager::get(SimManager *manager)
{
   return (MainPlayerManager *) manager->findObject(MainPlayerManagerId);
}

#endif
