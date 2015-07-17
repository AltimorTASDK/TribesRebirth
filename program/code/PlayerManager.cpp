#include "simBase.h"
#include "PlayerManager.h"
#include "FearDcl.h"
#include "console.h"
#include "ts.h"
#include "Player.h"
#include "netGhostManager.h"
#include "fear.strings.h"
#include "SimGuiCtrl.h"
#include "SimGuiCanvas.h"
#include "FearGuiChatDisplay.h"
#include "FearPlayerPSC.h"
#include "sfx.strings.h"
#include "fearGlobals.h"
#include <simpathmanager.h>
#include "simGuiDelegate.h"
#include "dlgPlay.h"
#include "dataBlockManager.h"
#include "sensorManager.h"
#include "stringTable.h"
#include "tsfx.h"
#include "sound.h"
#include <decalmanager.h>
#include <observerCamera.h>

//---------------------------------------------------------------

extern bool filterBadWords;

inline char
getBadChar()
{
   static Random s_random;
   const char* arrayReplace = "#@&*$@!*";
   
   int index = s_random.getInt(strlen(arrayReplace) - 1);
   return arrayReplace[index];
}

void languageFilter(char *message)
{
   const char *searchWords = Console->getVariable("BADWORDS");
   
   char currentWord[12];
   int charOffsets[12];
   
   const char *nextWord = searchWords;
   
   for(;;) {
      while(isspace(nextWord[0]))
         nextWord++;
      strncpy(currentWord, nextWord, 11);
      currentWord[11] = 0;
      char *spaceTerm = strchr(currentWord, ' ');
      if(spaceTerm)
         *spaceTerm = 0;
      int curWordLen = strlen(currentWord);
      if(curWordLen == 0)
         return;
   
      int index;
      for(index = 0; index < curWordLen; index++)
         currentWord[index] = tolower(currentWord[index]);
         
      for(index = 0; message[index] != 0; index++)
      {
         int it = index, ct;
         for(ct = 0; currentWord[ct] != 0; ct++)
         {
            while(message[it] && !isalnum(message[it]))
               it++;
            if(tolower(message[it]) == currentWord[ct])
            {
               charOffsets[ct] = it;
               it++;
            }
            else
               break;
         }
         if(!currentWord[ct])
         {
            // we found a naughty word!
            for(int i = 0; i < strlen(currentWord); i++) {
               message[charOffsets[i]] = getBadChar();
//               message[charOffsets[i]] = '*';
            }
         }
      }
      nextWord+= strlen(currentWord);
   }
}

static Sfx::Profile base2dSfxProfile;

static const char *intToStr(int d)
{
   static char buf[16];
   sprintf(buf, "%d", d);
   return buf;
}


class MissionResetEvent : public SimEvent
{
public:
   MissionResetEvent();

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);

   static void post(SimManager *manager, int destManagerId);

   DECLARE_PERSISTENT(MissionResetEvent);
};

IMPLEMENT_PERSISTENT_TAG(MissionResetEvent, MissionResetEventType);

MissionResetEvent::MissionResetEvent()
{
   type = MissionResetEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void MissionResetEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;
   bs;
}

void MissionResetEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;
   bs;
   address.objectId = PlayerManagerId;
}

void MissionResetEvent::post(SimManager *manager, int destManagerId)
{
   MissionResetEvent *evt = new MissionResetEvent;
   evt->address.managerId = destManagerId;
   manager->postCurrentEvent(evt);
}

//---------------------------------------------------------------

class PingPLEvent : public SimEvent
{
public:
   PingPLEvent();

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);

   DECLARE_PERSISTENT(PingPLEvent);
};

IMPLEMENT_PERSISTENT_TAG(PingPLEvent, PingPLEventType);


PingPLEvent::PingPLEvent()
{
   type = PingPLEventType;
   // unguaranteed
}

void PingPLEvent::pack(SimManager *, Net::PacketStream *, BitStream *bs)
{
   int numCl = 0;
   if(sg.playerManager)
      numCl = sg.playerManager->getNumClients();
   bs->writeInt(numCl, 7);
   if(numCl)
   {
      PlayerManager::ClientRep *cr = sg.playerManager->getClientList();
      while(cr)
      {
         bs->writeInt(cr->id - 2048, 7);
         bs->writeInt(cr->ping >> 2, 8);
         bs->writeFloat(cr->packetLoss, 6);
         cr = cr->nextClient;
      }
   }
}

void PingPLEvent::unpack(SimManager *manager, Net::PacketStream *, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   int count = bs->readInt(7);
   while(count--)
   {
      int id = bs->readInt(7) + 2048;
      int ping = bs->readInt(8) << 2;
      float pl = bs->readFloat(6);
      if(cg.playerManager)
      {
         PlayerManager::ClientRep *cl = cg.playerManager->findClient(id);
         if(cl)
         {
            cl->ping = ping;
            cl->packetLoss = pl;
         }
      }
   }
   address.objectId = -2;
}

//---------------------------------------------------------------
   
class DeltaScoreEvent : public SimEvent
{
public:
   int id;           // player id or team id
   int sortValue;    // sorting value of the score
   bool teamScore;   // if true, this is a team score
   char scoreString[MaxScoreString];

   DeltaScoreEvent();

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);

   DECLARE_PERSISTENT(DeltaScoreEvent);
};
IMPLEMENT_PERSISTENT_TAG(DeltaScoreEvent, DeltaScoreEventType);

DeltaScoreEvent::DeltaScoreEvent()
{
   type = DeltaScoreEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void DeltaScoreEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;
   if(bs->writeFlag(teamScore))
      bs->writeInt(id + 1, TeamIdSize + 1);
   else
      bs->writeInt(id - 2048, ClientIdSize);
   bs->writeInt(sortValue, 32);
   bs->writeString(scoreString);
}

void DeltaScoreEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;
   teamScore = bs->readFlag();
   if(teamScore)
      id = bs->readInt(TeamIdSize+1) - 1;
   else
      id = bs->readInt(ClientIdSize) + 2048;
   sortValue = bs->readInt(32);
   bs->readString(scoreString);

   address.objectId = PlayerManagerId;
}


//---------------------------------------------------------------

struct PlayerSkinEvent : public SimEvent
{
   int clientId;
   const char *skinBase;

   PlayerSkinEvent();
   DECLARE_PERSISTENT(PlayerSkinEvent);
   
   static void post(SimManager *manager, int playerId, unsigned int channel, const char *message, int destManagerId);
   
   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
};

IMPLEMENT_PERSISTENT_TAG(PlayerSkinEvent, PlayerSkinEventType);

PlayerSkinEvent::PlayerSkinEvent()
{
   type = PlayerSkinEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void PlayerSkinEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;
   
   bs->writeInt(clientId - 2048, 7);
   bs->writeString(skinBase);
}

void PlayerSkinEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

   clientId = bs->readInt(7) + 2048;
   skinBase = bs->readSTString();
   address.objectId = PlayerManagerId;
}


//---------------------------------------------------------------
struct PlayerSayEvent : public SimEvent
{
   int sender;
   unsigned int msgType;
   char message[256];

   PlayerSayEvent();
   ~PlayerSayEvent();   // have to de-allocate the string
   
   DECLARE_PERSISTENT(PlayerSayEvent);
   
   static void post(SimManager *manager, int playerId, unsigned int channel, const char *message, int destManagerId);
   
   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
};

IMPLEMENT_PERSISTENT_TAG(PlayerSayEvent, PlayerSayEventType);

PlayerSayEvent::PlayerSayEvent()
{
   type = PlayerSayEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

PlayerSayEvent::~PlayerSayEvent()
{
}

void PlayerSayEvent::post(SimManager *manager, int playerId, unsigned int msgType, const char *in_message, int destManagerId)
{
   PlayerSayEvent *event = new PlayerSayEvent();
   event->address.managerId = destManagerId;

   event->sender = playerId;
   event->msgType = msgType;
   strcpy(event->message, in_message);
   manager->postCurrentEvent(event);
}

void PlayerSayEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;
   
   bs->writeInt(sender, 12);
   bs->writeInt(msgType, 5);
   bs->writeString(message);
}

void PlayerSayEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

   sender = bs->readInt(12);
   msgType = bs->readInt(5);
   bs->readString(message);
   address.objectId = PlayerManagerId;
}

//---------------------------------------------------------------
struct PlayerSelectCmdrEvent : public SimEvent
{
   int peon;
   int cmdr;

   PlayerSelectCmdrEvent();
   
   DECLARE_PERSISTENT(PlayerSelectCmdrEvent);
   
   static void post(SimManager *manager, int peonId, int cmdrId, int destManagerId);
   
   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
};

IMPLEMENT_PERSISTENT_TAG(PlayerSelectCmdrEvent, PlayerSelectCmdrEventType);

PlayerSelectCmdrEvent::PlayerSelectCmdrEvent()
{
   type = PlayerSelectCmdrEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void PlayerSelectCmdrEvent::post(SimManager *manager, int peonId, int cmdrId, int destManagerId)
{
   PlayerSelectCmdrEvent *event = new PlayerSelectCmdrEvent();
   event->address.managerId = destManagerId;

   event->peon = peonId;   
   event->cmdr = cmdrId;   
   manager->postCurrentEvent(event);
}

void PlayerSelectCmdrEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;
   
   bs->writeInt(peon - 2048, 7);
   if(bs->writeFlag(cmdr != 0))
      bs->writeInt(cmdr - 2048, 7);
}

void PlayerSelectCmdrEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

   peon = bs->readInt(7) + 2048;
   if(bs->readFlag())
      cmdr = bs->readInt(7) + 2048;
   else
      cmdr = 0;
   address.objectId = PlayerManagerId;
}

//---------------------------------------------------------------
struct TeamObjectiveEvent : public SimEvent
{
   int objNum;
   char text[MAX_MSG_LENGTH + 1];

   TeamObjectiveEvent();
   
   DECLARE_PERSISTENT(TeamObjectiveEvent);
   
   static void post(SimManager *manager, int objNum, const char *text, int destManagerId);
   
   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
};

IMPLEMENT_PERSISTENT_TAG(TeamObjectiveEvent, TeamObjectiveEventType);

TeamObjectiveEvent::TeamObjectiveEvent()
{
   type = TeamObjectiveEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void TeamObjectiveEvent::post(SimManager *manager, int objNum, const char *text, int destManagerId)
{
   TeamObjectiveEvent *event = new TeamObjectiveEvent();
   event->address.managerId = destManagerId;

   event->objNum = objNum;
   strncpy(event->text, text, MAX_MSG_LENGTH);
   event->text[MAX_MSG_LENGTH] = '\0';   
   manager->postCurrentEvent(event);
}

void TeamObjectiveEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;
   
   bs->write(objNum);
   bs->writeString(text);
}

void TeamObjectiveEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

   bs->read(&objNum);
   bs->readString(text);
   address.objectId = PlayerManagerId;
}

//---------------------------------------------------------------
class PlayerCommandEvent : public SimEvent
{
public:
   int commander;
   int target;
   int command;
   int commandTarget;
   Point2I wayPoint;
   int status;
   bool statusOnly;
   bool hasMessage;
   const char *message;

   PlayerCommandEvent();
   DECLARE_PERSISTENT(PlayerCommandEvent);
   
   static void post(SimManager *manager, int destPlayerId, int commanderId, int targetId, int command, const char *message, int ctarget, Point2I &wayPoint, int status, bool statusOnly);
   
   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
};

IMPLEMENT_PERSISTENT_TAG(PlayerCommandEvent, PlayerCommandEventType);

PlayerCommandEvent::PlayerCommandEvent()
{
   type = PlayerCommandEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void PlayerCommandEvent::post(SimManager *manager, int destPlayerId, int commanderId, int targetId, int commandId, const char *message, int ctarget, Point2I &wayPoint, int status, bool statusOnly)
{
   PlayerCommandEvent *event = new PlayerCommandEvent();
   event->address.managerId = destPlayerId;

   event->commander = commanderId;
   event->target = targetId;
   event->command = commandId;
   event->commandTarget = ctarget;   
   event->wayPoint = wayPoint;
   event->status = status;
   event->statusOnly = statusOnly;
   event->message = NULL;
   event->hasMessage = bool(message);
   if(event->hasMessage)
   {
      PlayerManager::ClientRep *cr = sg.playerManager->findClient(destPlayerId);

      if(cr->lastCMDMessage != message)
      {
         event->message = message;
         cr->lastCMDMessage = message;
      }
   }
   manager->postCurrentEvent(event);
}

void PlayerCommandEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;
   
   bs->writeInt(target - 2048, 10);
   bs->writeInt(status, 2);

   if(!bs->writeFlag(statusOnly))
   {
      if(bs->writeFlag(command != -1))
      {
         bs->writeInt(commander - 2048, 10);
         bs->writeInt(command, 6);
         if(bs->writeFlag(commandTarget != -1))
            bs->writeInt(commandTarget, 8);
         bs->writeInt(wayPoint.x, 10);
         bs->writeInt(wayPoint.y, 10);
      }
   }
   if(bs->writeFlag(hasMessage))
   {
      if(bs->writeFlag(message))
         bs->writeString(message);
   }
}

void PlayerCommandEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

   target = bs->readInt(10) + 2048;
   status = bs->readInt(2);

   if(!(statusOnly = bs->readFlag()))
   {
      if(bs->readFlag())
      {
         commander = bs->readInt(10) + 2048;
         command = bs->readInt(6);
         commandTarget = -1;
         if(bs->readFlag())
            commandTarget = bs->readInt(8);
         wayPoint.x = bs->readInt(10);
         wayPoint.y = bs->readInt(10);
      }
      else
      {
         command = -1;
         commander = -1;
      }
   }
   hasMessage = bs->readFlag();

   message = NULL;
   if(hasMessage)
   {
      if(bs->readFlag())
         message = bs->readSTString(true);
   }
   address.objectId = PlayerManagerId;
}

//---------------------------------------------------------------

class PlayerAddEvent : public SimEvent
{
public:
   PlayerManager::ClientRep pr;
   PlayerAddEvent();

   static void post(SimManager *manager, PlayerManager::ClientRep *pr, int destManagerId);

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);

   DECLARE_PERSISTENT(PlayerAddEvent);
};

IMPLEMENT_PERSISTENT_TAG(PlayerAddEvent, PlayerAddEventType);

PlayerAddEvent::PlayerAddEvent()
{
   type = PlayerAddEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void PlayerAddEvent::post(SimManager *manager, PlayerManager::ClientRep *pr, int destManagerId)
{
   PlayerAddEvent *ev = new PlayerAddEvent();

   ev->pr = *pr;

   ev->address.objectId = PlayerManagerId;
   ev->address.managerId = destManagerId;

   manager->postCurrentEvent(ev);
}

void PlayerAddEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

   bs->write(pr.id);
   bs->writeFlag(pr.isBaseRepType);
   bs->writeString(pr.name);
   bs->writeString(pr.skinBase);
   bs->writeString(pr.voiceBase);
   bs->writeInt(pr.gender, 1);
   bs->write(pr.commanderId);
   if(bs->writeFlag(pr.team != -1))
      bs->writeInt(pr.team, TeamIdSize);
}

void PlayerAddEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

   char buf[255];

   bs->read(&pr.id);
   pr.isBaseRepType = bs->readFlag();
   bs->readString(buf);
   if(filterBadWords)
      languageFilter(buf);
   pr.name = stringTable.insert(buf, true);
   pr.skinBase = bs->readSTString();
   pr.voiceBase = bs->readSTString();
   pr.gender = bs->readInt(1);
   bs->read(&pr.commanderId);
   if(bs->readFlag())
      pr.team = bs->readInt(TeamIdSize);
   else
      pr.team = -1;

   pr.controlObject = NULL;
   pr.ownedObject = NULL;
   pr.position.set(0.0f, 0.0f, 0.0f);
   pr.selected = false;
   pr.curCommand = -1;
   pr.scoreString[0] = 0;
   pr.scoreSortValue = 0;
   pr.observerCamera = NULL;
   pr.commandStatus = PlayerManager::CMD_NONE;
   pr.commander = NULL;
   pr.firstPeon = NULL;

   pr.expanded = true;
   pr.ping = 0;
   pr.nextPeon = NULL;
   pr.chatSfxHandle = NULL;
   for (int i = 0; i < NUM_MUTED_DWORDS; i++)
   {
      pr.mutedPlayerList[i] = 0;
   }
   pr.mutedMe = FALSE;
  
   address.objectId = PlayerManagerId;
}

//---------------------------------------------------------------

class TeamAddEvent : public SimEvent
{
public:
   PlayerManager::TeamRep teamRep;
   TeamAddEvent();

   static void post(SimManager *manager, PlayerManager::TeamRep *rep, int destManagerId);

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);

   DECLARE_PERSISTENT(TeamAddEvent);
};

IMPLEMENT_PERSISTENT_TAG(TeamAddEvent, TeamAddEventType);

TeamAddEvent::TeamAddEvent()
{
   type = TeamAddEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void TeamAddEvent::post(SimManager *manager, PlayerManager::TeamRep *tr, int destManagerId)
{
   TeamAddEvent *ev = new TeamAddEvent();

   ev->teamRep = *tr;

   ev->address.objectId = PlayerManagerId;
   ev->address.managerId = destManagerId;

   manager->postCurrentEvent(ev);
}

void TeamAddEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

	bs->write (teamRep.energy);
   bs->write(teamRep.id);
   bs->writeString(teamRep.name);
   bs->writeString(teamRep.skinBase);
}

void TeamAddEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

	bs->read (&teamRep.energy);
   bs->read(&teamRep.id);
   char buf[255];
   bs->readString(buf);
   if(filterBadWords)
      languageFilter(buf);
   teamRep.name = stringTable.insert(buf, true);
   teamRep.skinBase = bs->readSTString();
   teamRep.firstCommander = NULL;
   teamRep.scoreString[0] = 0;
   teamRep.scoreSortValue = 0;
   for (int j = 0; j < MAX_NUM_OBJECTIVES; j++)
   {
      teamRep.objectives[j].seqNum = 0;
      teamRep.objectives[j].text[0] = 0;
   }
   address.objectId = PlayerManagerId;
}

//---------------------------------------------------------------

class VoiceEvent : public SimEvent
{
public:
   int sourceId;
   char soundFileName[32];
   VoiceEvent();

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);

   DECLARE_PERSISTENT(VoiceEvent);
};

IMPLEMENT_PERSISTENT_TAG(VoiceEvent, VoiceEventType);

VoiceEvent::VoiceEvent()
{
   type = VoiceEventType;
   flags.set(SimEvent::Guaranteed);
}

void VoiceEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

   bs->writeInt(sourceId - 2048, ClientIdSize);
   bs->writeString(soundFileName);
}

void VoiceEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

	sourceId = bs->readInt(ClientIdSize) + 2048;
   bs->readString(soundFileName);
   address.objectId = PlayerManagerId;
}

IMPLEMENT_PERSISTENT_TAG(SoundEvent, SoundEventType);

SoundEvent::SoundEvent()
{
   type = SoundEventType;
   flags.set(SimEvent::Guaranteed);
}

void SoundEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

   bs->writeInt(ghostIndex, 10);
   bs->writeInt(channel, 2);
   wg->dbm->writeBlockId(bs, DataBlockManager::SoundDataType, id);
}

void SoundEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

   ghostIndex = bs->readInt(10);
   channel = bs->readInt(2);
   id = wg->dbm->readBlockId(bs, DataBlockManager::SoundDataType);

   address.set(ps->getGhostManager()->resolveGhost(ghostIndex));
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
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void PlayerRemoveEvent::post(SimManager *manager, int removeId, int destManagerId)
{
   PlayerRemoveEvent *ev = new PlayerRemoveEvent();

   ev->pid = removeId;
   ev->address.managerId = destManagerId;

   manager->postCurrentEvent(ev);
}

void PlayerRemoveEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

   bs->writeInt(pid-2048, 7);
}

void PlayerRemoveEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

   pid = bs->readInt(7) + 2048;
   address.objectId = PlayerManagerId;
}


//---------------------------------------------------------------

class PlayerTeamChangeEvent : public SimEvent
{
public:
   int playerId;
   int teamId;

   PlayerTeamChangeEvent();

   static void post(SimManager *manager, int playerId, int teamId, int managerId);

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs);

   DECLARE_PERSISTENT(PlayerTeamChangeEvent);
};

IMPLEMENT_PERSISTENT_TAG(PlayerTeamChangeEvent, PlayerTeamChangeEventType);

PlayerTeamChangeEvent::PlayerTeamChangeEvent()
{
   type = PlayerTeamChangeEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void PlayerTeamChangeEvent::post(SimManager *manager, int in_playerId, int in_teamId, int destManagerId)
{
   PlayerTeamChangeEvent *ev = new PlayerTeamChangeEvent();

   ev->playerId = in_playerId;
   ev->teamId = in_teamId;

   ev->address.managerId = destManagerId;
   manager->postCurrentEvent(ev);
}

void PlayerTeamChangeEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager;
   ps;

   bs->writeInt(playerId - 2048, 7);
   bs->writeInt(teamId + 1, 4);
}

void PlayerTeamChangeEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager;
   ps;

   playerId = bs->readInt(7) + 2048;
   teamId = bs->readInt(4) - 1;
   address.objectId = PlayerManagerId;
}


//---------------------------------------------------------------
// BaseRep support functions.  Some SingleLinkList functions since we now have 
//    another list being managed.  


template <class T>
static void pushFrontSLL( T * * listPtr, T * elemPtr )
{
   elemPtr->nextClient = * listPtr;
   * listPtr = elemPtr;
}

// Unhook the element from the given list.  Put it back in the free list if 
//  one is supplied.  Return value indicates if element was found and axed. 
template <class T>
static bool unhookFromSLL( T * * listPtr, T * elemPtr, T * * freeList = 0 )
{
   T  * cl;
   while((cl = *listPtr) != NULL) 
   {
      if( cl == elemPtr )
      {
         * listPtr = cl->nextClient;
         if( freeList )
            pushFrontSLL( freeList, cl );
         return true;
      }
      listPtr = &(cl->nextClient);
   }
   return false;
}

void PlayerManager::BaseRep::setGenericDefaults(StringTableEntry playerName, 
         StringTableEntry voice, StringTableEntry skin, bool male, AIObj * otherControl )

{
   isBaseRepType = ((ai=otherControl)!=0);
   nextClient = NULL; 

   curGuiMode = PlayerPSC::InitialGuiMode;
   scoreString[0] = 0;
   ping = 0;
   listenChannels = 0;
   team = -1;
   lastScoreSequence = 0;
   scoreSequence = 0;
   lastCommandSequence = 0;
   lastObjectiveSequence = 0;
   ownedObject = NULL;
   controlObject = NULL;
   
   //
   position.set(0.0f, 0.0f, 0.0f);
   curCommandSequence = 0;
   curCommand = -1;
   commandStatus = CMD_NONE;
   commander = NULL;
   commanderId = -1;
   firstPeon = NULL;
   sendScores = false;
   expanded = TRUE;
   lastCMDMessage = NULL;
   
   name = playerName;
   skinBase = skin;
   voiceBase = voice;
   gender = male ? Male : Female;
}


//---------------------------------------------------------------

PlayerManager::PlayerManager(bool onServer)
{
   teamScoreHeading[0] = 0;
   clientScoreHeading[0] = 0;

   base2dSfxProfile.flags = 0;
   base2dSfxProfile.baseVolume = 0;
   base2dSfxProfile.coneInsideAngle = 90;
   base2dSfxProfile.coneOutsideAngle= 180; 
   base2dSfxProfile.coneVector.set(0.0f, -1.0f, 0.0f);
   base2dSfxProfile.coneOutsideVolume = 0.0f;  
   base2dSfxProfile.minDistance = 30.0f;     
   base2dSfxProfile.maxDistance = 300.0f;     

   numClients = 0;
   numBaseReps = 0;
   curLoginId = 0;
   numTeams = -1;
   reset();
   lastCMDMessage = NULL;

   isServer = onServer;
   id = PlayerManagerId;
   if(isServer)
      sg.playerManager = this;
   else
      cg.playerManager = this;
   curCommandSequence = 0;
   curObjectiveSequence = 0;
}

PlayerManager::~PlayerManager()
{
   if(sg.playerManager == this)
      sg.playerManager = NULL;
   if(cg.playerManager == this)
      cg.playerManager = NULL;                  
}

void PlayerManager::setTeam(int playerId, int teamId)
{
   if(!isServer)
      return;
   if(teamId < -1 || teamId >= numTeams)
      return;
   BaseRep *playerRep = findBaseRep(playerId);
   TeamRep *team = findTeam(teamId);

	if (playerRep->team != teamId)
	{
	   playerRep->team = teamId;
      if(team)
         playerRep->skinBase = team->skinBase;
      
      playerRep->lastCommandSequence = 0;
      playerRep->lastObjectiveSequence = 0;
      if( playerRep->isClientRep() )
         clientGuiModeChanged(playerId, playerRep->curGuiMode, playerRep->curGuiMode);

      for(ClientRep *walk = clientLink; walk; walk = walk->nextClient)
      {
         PlayerTeamChangeEvent::post(manager, playerRep->id, teamId, walk->id);
         if (walk->commanderId == playerRep->id)
            walk->commanderId = playerRep->commanderId;
      }
      if(playerRep->ownedObject)
         playerRep->ownedObject->setTeam(teamId);
      playerRep->commanderId = -1;
   }
}

void PlayerManager::buildCommandTrees()
{
   BaseRep *walk;
   for(int i = 0; i < numTeams + 1; i++)
      teamList[i].firstCommander = NULL;

   for(walk = baseRepLink; walk; walk = walk->nextClient)
   {
      walk->commander = NULL;
      walk->firstPeon = NULL;
      walk->nextPeon = NULL;
   }
   for(walk = baseRepLink; walk; walk = walk->nextClient)
   {
      if(walk->commanderId)
         walk->commander = findClient(walk->commanderId);
      if(walk->commander)
      {
         walk->nextPeon = walk->commander->firstPeon;
         walk->commander->firstPeon = static_cast<ClientRep *>(walk);
      }
      else
      {
         walk->commanderId = -1;
         walk->nextPeon = teamList[walk->team+1].firstCommander;
         teamList[walk->team+1].firstCommander = static_cast<ClientRep *>(walk);
      }
   }
}

void PlayerManager::teamAdded(int teamId)
{
   if(numTeams == MaxTeams)
      return;

   char teamNameVar[] = "$Server::teamNameX";
   char teamSkinVar[] = "$Server::teamSkinX";

   teamNameVar[strlen(teamNameVar) - 1] = '0' + numTeams;
   teamSkinVar[strlen(teamSkinVar) - 1] = '0' + numTeams;

   const char *teamName = Console->getVariable(teamNameVar);
   const char *skinBase = Console->getVariable(teamSkinVar);

   printf(FearGui::FearGuiChatDisplay::MSG_System, "Server: Team \"%s\" added - id = %d", teamName, teamId);

   TeamRep *t = teamList + numTeams + 1;
   numTeams++;

   t->scoreString[0] = 0;
   t->id = teamId;
   t->firstCommander = 0;

   if(!teamName[0])
      t->name = stringTable.insert("unnamed", true);
   else
      t->name = stringTable.insert(teamName, true);
   if(!skinBase[0])
      t->skinBase = stringTable.insert("base");
   else
      t->skinBase = stringTable.insert(skinBase);
   
   for (int j = 0; j < MAX_NUM_OBJECTIVES; j++)
   {
      t->objectives[j].seqNum = 1;
      t->objectives[j].text[0] = 0;
   }

   for(ClientRep *i = clientLink; i; i = i->nextClient)
      TeamAddEvent::post(manager, t, i->id);
}

TeamGroup *PlayerManager::getTeamGroup (int teamId)
{
	SimGroup *teamsGroup = dynamic_cast<SimGroup *>(manager->findObject (FearMissionTeams));
   if(!teamsGroup)
      return NULL;
   char name[6] = "teamX";
   name[4] = '0' + teamId;

	return dynamic_cast<TeamGroup *>
	   (teamsGroup->findObject (name));
}


void PlayerManager::ClientRep::clean(void)
{
   nextClient = NULL;
   chatSfxHandle = NULL;
   ghostManager = NULL;
   eventManager = NULL;
   packetStream = NULL;
   playerPSC = NULL;
}

void PlayerManager::clientAdded(const char *playerName, const char *voice, const char *skin,
                              bool male, AIObj * ai )
{
   skin;
   int playerId = clientFreeList->id;
   printf(FearGui::FearGuiChatDisplay::MSG_System, "Server: Player \"%s\" added - id = %d", playerName, playerId);

   ClientRep *p = clientFreeList;
   clientFreeList = clientFreeList->nextClient;

   p->clean();   
   p->setGenericDefaults( playerName, voice, stringTable.insert("base"), male, ai );
   p->loginId = curLoginId++;

   bool  isRegularClient = p->isClientRep();
   if( isRegularClient )
   {
      numClients++;
      p->packetStream = (Net::PacketStream *) manager->findObject(playerId);
      p->ghostManager = p->packetStream->getGhostManager();
      p->eventManager = p->packetStream->getEventManager();
      p->playerPSC = (PlayerPSC *) p->packetStream->findObject("PlayerPSC");
      p->observerCamera = new ObserverCamera(p->id);
      manager->addObject(p->observerCamera);

      pushFrontSLL( & clientLink, p );
      
      int i;
      for(i = 0; i <= numTeams; i++)
         TeamAddEvent::post(manager, teamList + i, clientLink->id);
   }
   pushFrontSLL( & baseRepLink, (BaseRep *)p );
   numBaseReps++;
   
   for (int i = 0; i < NUM_MUTED_DWORDS; i++)
      p->mutedPlayerList[i] = 0;

   p->mutedMe = FALSE;

   if( isRegularClient )
   {
      for(p = p->nextClient; p; p = p->nextClient)
      {
         // tell all managers out there to add this client:
         PlayerAddEvent::post(manager, clientLink, p->id);
         // inform new client of existing clients:
         PlayerAddEvent::post(manager, p, clientLink->id);
      }
      PlayerAddEvent::post(manager, clientLink, clientLink->id);
      
      if( numBaseReps > numClients )
      {
         // If there are any existing AIs (BaseReps which are not ClientReps), then 
         //    inform this client (specified by clientLink->id) of them.  
         BaseRep  * br = baseRepLink;
         while( br )
         {
            if( ! br->isClientRep() )
               PlayerAddEvent::post(manager, static_cast<ClientRep *>(br), clientLink->id);
            br = br->nextClient;
         }
      }
   }
   else
   {
      // BaseRep: tell all managers out there to add this rep.  
      ClientRep   * newRep = static_cast<ClientRep *>( baseRepLink );
      for(p = clientLink; p; p = p->nextClient )
         PlayerAddEvent::post(manager, newRep, p->id);
   }
   
   if( isRegularClient )
   {
      Console->evaluatef("Server::onClientConnect(%d);", playerId);
      sg.dbm->sendDataToClient(clientLink->id);
      clientLink->dataFinished = false;
      SensorManager *radMgr = dynamic_cast<SensorManager *>
                     (manager->findObject(SensorManagerId));
      radMgr->newClient(clientLink->id);
         
      DeltaScoreEvent *event = new DeltaScoreEvent;
      event->id = -1;
      event->teamScore = true;
      event->sortValue = 0;
      strcpy(event->scoreString, teamScoreHeading);
      event->address.managerId = playerId;
      manager->postCurrentEvent(event);

      event = new DeltaScoreEvent;
      event->id = 2048;
      event->teamScore = false;
      event->sortValue = 0;
      strcpy(event->scoreString, clientScoreHeading);
      event->address.managerId = playerId;
      manager->postCurrentEvent(event);
   }
}

void PlayerManager::sendPingPackets()
{
   ClientRep * cl;
   for(cl = getClientList(); cl; cl = cl->nextClient)
   {
      cl->ping = cl->packetStream->getVC()->getAverageRTT();
      if(cl->ping > 999)
         cl->ping = 999;
      cl->packetLoss = cl->packetStream->getVC()->getPacketLoss();
   }
   cl = getClientList();
   while(cl)
   {
      if(cl->sendScores)
      {
         PingPLEvent *event = new PingPLEvent;
         event->address.managerId = cl->id;
         manager->postCurrentEvent(event);
      }
      cl = cl->nextClient;
   }
   PingPLEvent *event = new PingPLEvent;
   event->time = manager->getCurrentTime() + 1.5;
   manager->postEvent(this, event);
}

void PlayerManager::setDisplayScores(int clientId, bool display)
{
   ClientRep *cl = findClient(clientId);
   if(!cl || !cl->dataFinished)
      return;
   cl->sendScores = display;
   if(display && cl->curGuiMode != PlayerPSC::VictoryGuiMode)
   {
      ClientRep *i;
      for(i = clientLink; i; i = i->nextClient)
      {
         if(i->scoreSequence > cl->lastScoreSequence)
         {
            DeltaScoreEvent *event = new DeltaScoreEvent;
            event->id = i->id;
            event->teamScore = false;
            event->sortValue = i->scoreSortValue;
            strcpy(event->scoreString, i->scoreString);
            event->address.managerId = clientId;
            manager->postCurrentEvent(event);
         }
      }
      int t;
      for(t = 1; t <= numTeams; t++)
      {
         if(teamList[t].scoreSequence > cl->lastScoreSequence)
         {
            DeltaScoreEvent *event = new DeltaScoreEvent;
            event->id = t - 1;
            event->teamScore = true;
            event->sortValue = teamList[t].scoreSortValue;
            strcpy(event->scoreString, teamList[t].scoreString);
            event->address.managerId = clientId;
            manager->postCurrentEvent(event);
         }
      }
      cl->lastScoreSequence = curScoreSequence;
   }
}

void PlayerManager::setClientSkin(int clientId, const char *in_skin)
{
   int l = strlen(in_skin);
   if(l < 1 || l > 19)
      return;
   int i;
   for(i = 0; i < l; i++)
   {
      char c = in_skin[i];
      if(!( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c>= 'a' && c <= 'z') ))
         return;
   }
   BaseRep *br = findClient(clientId);
   if(!br)
      return;
   const char *skin = stringTable.insert(in_skin);
   if(br->skinBase == skin)
      return;
   br->skinBase = skin;
   ClientRep *cl = getClientList();
   while(cl)
   {
      PlayerSkinEvent *event = new PlayerSkinEvent;
      event->address.managerId = cl->id;
      event->skinBase = skin;
      event->clientId = clientId;
      manager->postCurrentEvent(event);
      cl = cl->nextClient;
   }
}

void PlayerManager::setTeamScore(int teamId, const char *scoreString, int sortValue)
{
   if(!isServer)
      return;

   if(teamId < -1 || teamId > numTeams)
      return;
   teamId++; // account for -1 team
   if(sortValue == teamList[teamId].scoreSortValue && !strncmp(teamList[teamId].scoreString, scoreString, MaxScoreString-1))
      return;

   strncpy(teamList[teamId].scoreString, scoreString, MaxScoreString-1);
   teamList[teamId].scoreString[MaxScoreString-1] = 0;
   teamList[teamId].scoreSortValue = sortValue;
   teamList[teamId].scoreSequence = ++curScoreSequence;
   
   ClientRep *i;
   for(i = clientLink; i; i = i->nextClient) {
      if(i->sendScores && i->curGuiMode != PlayerPSC::VictoryGuiMode) {
         i->lastScoreSequence = curScoreSequence;
         DeltaScoreEvent *event = new DeltaScoreEvent;
         event->id = teamId - 1;
         event->teamScore = true;
         event->sortValue = sortValue;
         strcpy(event->scoreString, teamList[teamId].scoreString);
         event->address.managerId = i->id;
         manager->postCurrentEvent(event);
      }
   }
}

void PlayerManager::setClientScoreHeading(const char *heading)
{
   strncpy(clientScoreHeading, heading, MaxScoreString-1);
   clientScoreHeading[MaxScoreString-1] = 0;

   for(ClientRep *i = clientLink; i; i = i->nextClient)
   {
      DeltaScoreEvent *event = new DeltaScoreEvent;
      event->id = 2048;
      event->teamScore = false;
      event->sortValue = 0;
      strcpy(event->scoreString, clientScoreHeading);
      event->address.managerId = i->id;
      manager->postCurrentEvent(event);
   }   
}

void PlayerManager::setTeamScoreHeading(const char *heading)
{
   strncpy(teamScoreHeading, heading, MaxScoreString-1);
   teamScoreHeading[MaxScoreString-1] = 0;

   for(ClientRep *i = clientLink; i; i = i->nextClient)
   {
      DeltaScoreEvent *event = new DeltaScoreEvent;
      event->id = -1;
      event->teamScore = true;
      event->sortValue = 0;
      strcpy(event->scoreString, teamScoreHeading);
      event->address.managerId = i->id;
      manager->postCurrentEvent(event);
   }   
}

void PlayerManager::setClientScore(int clientId, const char *scoreString, int sortValue)
{
   if(!isServer)
      return;
   
   ClientRep *cl = findClient(clientId);
   if(!cl)
      return;

   if(sortValue == cl->scoreSortValue && !strncmp(cl->scoreString, scoreString, MaxScoreString-1))
      return;
   strncpy(cl->scoreString, scoreString, MaxScoreString-1);
   cl->scoreString[MaxScoreString-1] = 0;
   cl->scoreSortValue = sortValue;
   cl->scoreSequence = ++curScoreSequence;

   ClientRep *i;
   for(i = clientLink; i; i = i->nextClient)
   {
      if(i->sendScores && i->curGuiMode != PlayerPSC::VictoryGuiMode) {
         i->lastScoreSequence = curScoreSequence;
         DeltaScoreEvent *event = new DeltaScoreEvent;
         event->id = clientId;
         event->teamScore = false;
         event->sortValue = sortValue;
         strcpy(event->scoreString, cl->scoreString);
         event->address.managerId = i->id;
         manager->postCurrentEvent(event);
      }
   }
}

void PlayerManager::removeClient(ClientRep *cr)
{
   ClientRep *cl;    // **clw, 
   
   //clear the mute flag from the server
   cl = clientLink;
   int playerNum = cr->id - 2048;
   while (cl)
   {
      cl->mutedPlayerList[playerNum / 32] &= ~(1 << (playerNum % 32));
      cl = cl->nextClient;
   }

   cr->name = NULL;
   if( cr->isClientRep() ){
      unhookFromSLL( & clientLink, cr, & clientFreeList ); 
      numClients--;
   }
   else{
      // Need to put AI objects back into free list.  
      pushFrontSLL( & clientFreeList, cr );
   }
   unhookFromSLL( & baseRepLink, (BaseRep *)cr ); 
   numBaseReps--;
   
   for(ClientRep *walk = clientLink; walk; walk = walk->nextClient)
   {
      if (walk->commanderId == cr->id)
         walk->commanderId = cr->commanderId;
   }
}


void PlayerManager::clientDropped(int playerId)
{
   ClientRep *playerRep = findBaseRep(playerId);
   
   if( playerRep->isClientRep() )
   {
      Console->evaluatef("Server::onClientDisconnect(%d);", playerId);

      // if the function Client::leaveGame() unsets the object as
      // owned by this client rep, it will not be deleted here:

      if(playerRep->ownedObject)
         playerRep->ownedObject->deleteObject();
      if( playerRep->isClientRep() )
         printf(FearGui::FearGuiChatDisplay::MSG_System, "Server: Player \"%s\" dropped - id = %d", 
                        playerRep->name, playerRep->id);
      manager->unregisterObject(playerRep->observerCamera);
      delete playerRep->observerCamera;
   }
   
   removeClient(playerRep);
   
   for(ClientRep *walk = clientLink; walk; walk = walk->nextClient)
      PlayerRemoveEvent::post(manager, playerId, walk->id);
}

void PlayerManager::clientGuiModeChanged(int clientId, int newGuiMode, int oldGuiMode)
{
   oldGuiMode;

   ClientRep *cmdr = findClient(clientId);
   if(!cmdr->dataFinished)
      return;

   if(newGuiMode == PlayerPSC::CommandGuiMode)
   {
      for(ClientRep *i = clientLink; i; i = i->nextClient)
         if((i != cmdr) && (i->team == cmdr->team) && ((cmdr->lastCommandSequence < i->curCommandSequence) || (i->curCommandTarget != -1)))
            PlayerCommandEvent::post(manager, clientId, i->curCommander, i->id, i->curCommand, NULL, i->curCommandTarget, i->wayPoint, i->commandStatus, false);
      cmdr->lastCommandSequence = curCommandSequence;
   }

   // update objectives on entry to victory mode and objective mode,
   // but don't update while in victory mode (in setTeamObjective below)

   if(cmdr->curGuiMode != PlayerPSC::VictoryGuiMode && cmdr->curGuiMode != PlayerPSC::ObjectiveGuiMode &&
      (newGuiMode == PlayerPSC::VictoryGuiMode || newGuiMode == PlayerPSC::ObjectiveGuiMode))
   {
      //update the team objectives
      TeamRep *cmdrTeam = findTeam(cmdr->team);
      if (! cmdrTeam) return;
      
      for (int j = 0; j < MAX_NUM_OBJECTIVES; j++)
         if (cmdr->lastObjectiveSequence < cmdrTeam->objectives[j].seqNum)
            TeamObjectiveEvent::post(manager, j, cmdrTeam->objectives[j].text, clientId);

      cmdr->lastObjectiveSequence = curObjectiveSequence;
   }
   // scores are frozen in VictoryGuiMode
   if(newGuiMode == PlayerPSC::VictoryGuiMode && !cmdr->sendScores)
   {
      setDisplayScores(clientId, true);
      setDisplayScores(clientId, false);
   }
   else if(newGuiMode != PlayerPSC::VictoryGuiMode && cmdr->curGuiMode == PlayerPSC::VictoryGuiMode && cmdr->sendScores)
   {
      cmdr->curGuiMode = newGuiMode;
      setDisplayScores(clientId, true);
   }
   cmdr->curGuiMode = newGuiMode;
}

void PlayerManager::clearTeamObjectives(int teamId)
{
   if (teamId >= -1 && teamId < numTeams)
   {
      teamId++;
      curObjectiveSequence++;
      for (int i = 0; i < MAX_NUM_OBJECTIVES; i++)
      {
         if (! strcmp(teamList[teamId].objectives[i].text, "")) continue;
         
         teamList[teamId].objectives[i].seqNum = curObjectiveSequence;
         strcpy(teamList[teamId].objectives[i].text, "");
         
         //now check everyone on the team, and update anyone who is currently in command mode
         for(ClientRep *j = clientLink; j; j = j->nextClient)
         {
            if ((j->team == teamId - 1) && j->curGuiMode == PlayerPSC::ObjectiveGuiMode)
            {
               TeamObjectiveEvent::post(manager, i, "", j->id);
               j->lastObjectiveSequence = curObjectiveSequence;
            }
         } 
      }
   }
}

void PlayerManager::setTeamObjective(int teamId, int objNum, const char *text)
{
   if(!isServer)
      return;
      
   if (objNum >= 0 && objNum < MAX_NUM_OBJECTIVES && teamId >= -1 && teamId < numTeams)
   {
      teamId++;
      if(!strncmp(teamList[teamId].objectives[objNum].text, text, MAX_MSG_LENGTH))
         return;
      
      curObjectiveSequence++;
      teamList[teamId].objectives[objNum].seqNum = curObjectiveSequence;
      strncpy(teamList[teamId].objectives[objNum].text, text, MAX_MSG_LENGTH);
      teamList[teamId].objectives[objNum].text[MAX_MSG_LENGTH] = '\0';
   
      //now check everyone on the team, and update anyone who is currently in command mode
      for(ClientRep *j = clientLink; j; j = j->nextClient)
      {
         if ((j->team == teamId - 1) && j->curGuiMode == PlayerPSC::ObjectiveGuiMode)
         {
            TeamObjectiveEvent::post(manager, objNum, text, j->id);
            j->lastObjectiveSequence = curObjectiveSequence;
         }
      } 
   }
}

void PlayerManager::issueCommand(int cmdrId, int targId, int commandIcon, const char *commandString, Point2I &wayPoint, int objectIndex)
{
   if (! isServer) return;
   
   if(objectIndex < -1 || objectIndex > 255)
      return;
   
   ClientRep *cmdr = findClient(cmdrId);
   ClientRep *targ = findBaseRep(targId);

   curCommandSequence++;
   
   if(cmdrId && ((!cmdr || !targ) || (cmdr->team != targ->team)))
      return;

   if((cmdrId == targId) || !cmdrId || isInCommandChain(targId, cmdrId) ||
      (targ->commanderId == -1 && (targ->curCommand == -1 || targ->curCommander != targId)))
   {
      targ->curCommandSequence = curCommandSequence;
      targ->curCommander = cmdrId;
      targ->curCommand = commandIcon;
      targ->curCommandString = commandString;
      targ->commandStatus = (cmdrId  && cmdrId != targId ? CMD_RECEIVED : CMD_ACKNOWLEDGED);
      targ->wayPoint = wayPoint;
      targ->curCommandTarget = objectIndex;
      
      if( !targ->isClientRep() )
      {
         // Send off AI script callback.  But they can't do anything with a targetId 
         // command, so exit if this isn't a simple waypoint command.  
         if( objectIndex != -1 )
            return;
      
         const char * argv[14];
         char  commandBuff[314];
         char * curBuf = commandBuff;
         char * * cpp = const_cast<char * *>(argv);

         *cpp++ = "AI::onCommand";
         curBuf += sprintf( *cpp++ = curBuf, "%s",    targ->name ) + 1;
         curBuf += sprintf( *cpp++ = curBuf, "%d",    targ->curCommander ) + 1;
         curBuf += sprintf( *cpp++ = curBuf, "%d",    targ->curCommand ) + 1;
         curBuf += sprintf( *cpp++ = curBuf, "%d %d", targ->wayPoint.x,targ->wayPoint.y ) + 1;
         curBuf += sprintf( *cpp++ = curBuf, "%d",    targ->curCommandTarget ) + 1;
         curBuf += sprintf( *cpp++ = curBuf, "%s",    targ->curCommandString ) + 1;
         curBuf += sprintf( *cpp++ = curBuf, "%d",    targ->commandStatus ) + 1;
         sprintf( *cpp = curBuf, "%d", targ->curCommandSequence );
         
         Console->execute(9, argv);
      }

      for(ClientRep *i = clientLink; i; i = i->nextClient)
      {
         if(i->curGuiMode == PlayerPSC::CommandGuiMode)
            i->lastCommandSequence = curCommandSequence;
         
         if (i->team == cmdr->team && 
               (i->curGuiMode == PlayerPSC::CommandGuiMode || i == targ))
         {
            const char *msg = NULL;
            if(i->id == targId || i->id == cmdrId)
               msg = targ->curCommandString;

            PlayerCommandEvent::post(manager, i->id, cmdrId, targId, targ->curCommand, 
                  msg, targ->curCommandTarget, wayPoint, targ->commandStatus, false);
         }
      }
   }
}

bool PlayerManager::isInCommandChain(int peonId, int cmdrId)
{
   ClientRep *peon = findBaseRep(peonId);

   if(!peon)
      return false;

   for(ClientRep *walk = peon; walk; walk = findClient(walk->commanderId))
      if(walk->commanderId == cmdrId)
         return true;

   return false;
}

void PlayerManager::setCommander(ClientRep *peon, ClientRep *cmdr)
{
   if(!cmdr)
      peon->commanderId = -1;
   else
   {
      ClientRep *walk;
      for(walk = cmdr; walk; walk = findClient(walk->commanderId))
      {
         if(walk->commanderId == peon->id)
            break;
      }
      if(!walk) // no wacky link in the command tree
         peon->commanderId = cmdr->id;
   }
}

void PlayerManager::selectCommander(int peonId, int cmdrId)
{
   if(!isServer)
      return;
      
   //make sure the command trees are up to date
   buildCommandTrees();
      
   if(peonId == cmdrId)
      return;      
   ClientRep *peon = findBaseRep(peonId);
   ClientRep *cmdr = (cmdrId >= 0 ? findClient(cmdrId) : NULL);

   if(!peon)
      return;
   if(cmdr && cmdr->team != peon->team)
      return;
   if(peon == cmdr || peon->commander == cmdr)
      return;

   int prevCmdr = peon->commanderId;
   setCommander(peon, cmdr);

   if(peon->commanderId != prevCmdr)
   {
      //now signal all the players
      for(ClientRep *i = clientLink; i; i = i->nextClient)
         PlayerSelectCmdrEvent::post(manager, peonId, cmdrId, i->id);
   }
}

bool PlayerManager::commandStatusReport(int peonId, int status, const char *message)
{
   if(!isServer)
      return false;
      
   ClientRep *peon = findClient(peonId);

   if(!peon)
      return false;

   curCommandSequence++;

   //check the status
   if (peon->commandStatus == CMD_RECEIVED || (peon->commandStatus == CMD_ACKNOWLEDGED && !status))
   {
      peon->curCommandSequence = curCommandSequence;
      if (status)
         peon->commandStatus = CMD_ACKNOWLEDGED;
      else
         peon->commandStatus = CMD_NONE;

      //set the server side
      int oldCommander = peon->curCommander;
      if (!status)
      {
         peon->curCommand = -1;
         peon->curCommander = -1;
      }
      
      //now signal all the players on the team
      for(ClientRep *i = clientLink; i; i = i->nextClient)
      {
         if(i->curGuiMode == PlayerPSC::CommandGuiMode)
            i->lastCommandSequence = curCommandSequence;
         if (i->team == peon->team && 
               (i->curGuiMode == PlayerPSC::CommandGuiMode || i == peon))
         {
            const char *msg = NULL;
            if(i == peon || i->id == oldCommander)
               msg = message;

            PlayerCommandEvent::post(manager, i->id, -1, peonId, -1, msg, -1, Point2I(0,0), peon->commandStatus, true);
         }
      }
      return true;
   }
   return false;
}

void PlayerManager::messageClient(int playerId, int srcClientId, int messageType, const char *message)
{
   ClientRep *p = findClient(playerId);
   if(!p)
      return;
   if(srcClientId == -1)
      srcClientId = 0;
      
   PlayerSayEvent::post(manager, srcClientId, messageType, message, playerId);
}

void PlayerManager::mute(int playerId, int mutePlayerId, bool on_off)
{
playerId;
mutePlayerId;
on_off;
}

static void playSoundMessage(PlayerManager::ClientRep *rep, SimManager *manager, char *snd)
{
   //see if voices have been turned off
   if (! strcmp(Console->getVariable("pref::playVoices"), ""))
   {
      Console->setVariable("pref::playVoices", "TRUE");
   }
   if (! Console->getBoolVariable("pref::playVoices")) return;
   
   // scan for sound string
   while(*snd)
   {
      if(*snd == '~' && *(snd+1) == 'w')
      {
         *snd = 0;
         snd += 2;
         break;
      }
      snd++;
   }
   if(*snd) {
      Sfx::Manager *sfx = Sfx::Manager::find(manager);
      if(sfx) {
         if(!rep)
            sfx->play(stringTable.insert(snd), 0, &base2dSfxProfile);
         else
         {
            char sndBuf[128];

            sprintf(sndBuf, "%s.w%s.wav", rep->voiceBase, snd);
            if(!cg.resManager->findFile(sndBuf))
            {   
               sprintf(sndBuf, "w%s.wav", snd);
               if(!cg.resManager->findFile(sndBuf))
                  return;
            }
            if(rep->chatSfxHandle)
            {
               if(sfx->selectHandle(rep->chatSfxHandle))
               {
                  sfx->stop();
                  sfx->release();
               }
            }
            rep->chatSfxHandle = sfx->play(stringTable.insert(sndBuf), 0, &base2dSfxProfile);
         }
      }
   }
}

void PlayerManager::serverPlayVoice(int clientId, const char *voiceString)
{
   ClientRep *rep = findClient(clientId);
   if(!rep)
      return;

   if(!rep->ownedObject)
      return;

   int bid;
   SoundProfileData *dat = (SoundProfileData *) sg.dbm->lookupDataBlock(0, DataBlockManager::SoundProfileDataType);
   if(!dat)
      return;
   Vector<Net::GhostRef>::iterator i;
   for(ClientRep *cl = clientLink; cl; cl = cl->nextClient)
   {
      GameBase *fo = cl->playerPSC->getControlObject();
      if(!fo)
         continue;
      float dist = (fo->getLinearPosition() - rep->ownedObject->getLinearPosition()).len();
      if(dist < dat->profile.maxDistance)
      {
         VoiceEvent *evt = new VoiceEvent;
         evt->sourceId = clientId;
         strncpy(evt->soundFileName, voiceString, MaxVoiceFileLen);
         evt->soundFileName[MaxVoiceFileLen] = 0;
         evt->address.managerId = cl->id;
         manager->postCurrentEvent(evt);
      }
   }
}

bool PlayerManager::processEvent(const SimEvent *event)
{
   ClientRep *rep, *cl, **clw, *rep2;

   switch (event->type)
   {
      case PingPLEventType:
      {
         if(!isServer)
            return false;
         sendPingPackets();
         break;
      }
      case VoiceEventType:
      {
         if(isServer)
            return false;
            
         //see if voices have been turned off
         if (! strcmp(Console->getVariable("pref::playVoices"), ""))
         {
            Console->setVariable("pref::playVoices", "TRUE");
         }
         if (! Console->getBoolVariable("pref::playVoices")) break;
         
         VoiceEvent *evt = (VoiceEvent *) event;
         char sndBuf[128];
         ClientRep *rep = findClient(evt->sourceId);
         if(!rep || !rep->ownedObject)
            break;

         sprintf(sndBuf, "%s.w%s.wav", rep->voiceBase, evt->soundFileName);
         if(!cg.resManager->findFile(sndBuf))
         {
            sprintf(sndBuf, "w%s.wav", evt->soundFileName);
            if(!cg.resManager->findFile(sndBuf))
               return true;
         }
         SoundProfileData *dat = (SoundProfileData *) cg.dbm->lookupDataBlock(0, DataBlockManager::SoundProfileDataType);
         if(!dat)
            break;

         Sfx::Manager *sfx = Sfx::Manager::find(manager);
         if(!rep)
            break;
         if(rep->chatSfxHandle)
         {
            if(sfx->selectHandle(rep->chatSfxHandle))
            {
               sfx->stop();
               sfx->release();
            }
         }
         rep->chatSfxHandle = sfx->playAt(stringTable.insert(sndBuf), 0, &dat->profile, rep->ownedObject->getTransform(), rep->ownedObject->getLinearVelocity());
         break;
      }
      case PlayerTeamChangeEventType:
         if(isServer)
            return false;

         PlayerTeamChangeEvent *tce;
         tce = (PlayerTeamChangeEvent *) event;
         rep = findBaseRep(tce->playerId);
         if(rep)
         {
            TeamRep *team = findTeam(tce->teamId);
            rep->team = tce->teamId;

            for(ClientRep *walk = clientLink; walk; walk = walk->nextClient)
            {
               if(walk->commanderId == rep->id)
                  walk->commanderId = rep->commanderId;
            }
            rep->commanderId = -1;
            Console->evaluatef("onClientChangeTeam(%d,%d);", rep->id, rep->team);
            if(rep->team == -1)
               printf(FearGui::FearGuiChatDisplay::MSG_System, "%s is no longer on a team.", rep->name);
            else
            {
               if (! team) break;
               rep->skinBase = team->skinBase;
               if(rep->ownedObject)
               {
                  ShapeBase *sb = dynamic_cast<ShapeBase *>(rep->ownedObject);
                  sb->reSkin(true);
               }
               if(getNumTeams() > 1)
                  printf(FearGui::FearGuiChatDisplay::MSG_System, "%s joined team %s.", rep->name, team->name);
               else
                  printf(FearGui::FearGuiChatDisplay::MSG_System, "%s joined the fray.", rep->name);
            }
         }
         break;
         
      case TeamObjectiveEventType:
         if (! isServer)
         {
            TeamObjectiveEvent *evt = (TeamObjectiveEvent *) event;
            ClientRep *me = findClient(manager->getId());
            strcpy(teamList[me->team+1].objectives[evt->objNum].text, evt->text);

            //find the mission objectives page
            SimGui::Control *cmdGui;
            cmdGui = (SimGui::Control*)(manager->findObject("CmdObjectivesGui"));
            if (cmdGui)
            {
               FearGui::FGHCommandObjectivePage *objCtrl;
               objCtrl = dynamic_cast<FearGui::FGHCommandObjectivePage*>(cmdGui->findControlWithTag(IDCTG_CMD_OBJECTIVES));
               if (objCtrl) objCtrl->newObjective(evt->objNum, evt->text);
            }
         }
         break;
      case PlayerSkinEventType:
         if(!isServer)
         {
            PlayerSkinEvent *evt = (PlayerSkinEvent *) event;

            BaseRep *br = findBaseRep(evt->clientId);
            br->skinBase = evt->skinBase;
            if(br->ownedObject)
            {
               ShapeBase *sb = dynamic_cast<ShapeBase *>(br->ownedObject);
               sb->reSkin(true);
            }
         }
         break;
      case PlayerCommandEventType:
         if(!isServer)
         {
            PlayerCommandEvent *evt = (PlayerCommandEvent *) event;
            //ClientRep *pr = findClient(evt->target);
            ClientRep *pr = findBaseRep(evt->target);
            char buf[256];
            buf[0] = 0;

            if(evt->hasMessage)
            {
               if(evt->message)
                  lastCMDMessage = evt->message;
               strcpy(buf, lastCMDMessage);
            }
            if(pr)
            {
               if(evt->statusOnly)
               {
                  Int32 commander = pr->curCommander;
                  if(evt->status == CMD_NONE)
                  {
                     pr->curCommand = -1;
                     pr->curCommander = -1;
                     if (UInt32(evt->target) == manager->getId()) 
                     {
                        cg.curCommand = -1;
                     }
                        
                  }
                  pr->commandStatus = evt->status;

                  if (evt->hasMessage && ((UInt32(commander) == manager->getId()) ||
                      (UInt32(evt->target) == manager->getId())))
                  {
                     playSoundMessage(pr, manager, buf);
                     printf(FearGui::FearGuiChatDisplay::MSG_TeamChat, "%s: %s", pr->name, buf);
                  }
               }
               else
               {
                  ClientRep *cmdr = findClient(evt->commander);
                  if(!cmdr)
                     break;

                  pr->curCommand = evt->command;
                  pr->curCommander = evt->commander;
                  pr->curCommandTarget = evt->commandTarget;
                  pr->commandStatus = evt->status;
                  pr->wayPoint = evt->wayPoint;
                  
                  if (UInt32(evt->target) == manager->getId()) 
                  {
                     cg.curCommand = evt->command;
                     cg.wayPoint = cg.missionCenterPos;
                     cg.wayPoint.x += (evt->wayPoint.x * cg.missionCenterExt.x) / 1024.0f;
                     cg.wayPoint.y += (evt->wayPoint.y * cg.missionCenterExt.y) / 1024.0f;
                     
                     if(evt->hasMessage)
                     {
                        playSoundMessage(cmdr, manager, buf);
                        printf(FearGui::FearGuiChatDisplay::MSG_Command, "%s: %s", cmdr->name, buf);
                     }
                  }
                  else if(UInt32(evt->commander) == manager->getId() && evt->hasMessage)
                  {
                     playSoundMessage(cmdr, manager, buf);
                     printf(FearGui::FearGuiChatDisplay::MSG_TeamChat, "Cmd. to %s: %s", pr->name, buf);
                  }
               }
            }
         }
         break;
         
      case PlayerSelectCmdrEventType:
      {
         if (isServer)
            return FALSE;
            
         //make sure the command trees are up to date
         buildCommandTrees();
            
         PlayerSelectCmdrEvent *scEvent;
         scEvent = (PlayerSelectCmdrEvent*)event;
         
         //search for the peon and the commander
         int playerId = manager->getId();
         ClientRep *peon = findBaseRep(scEvent->peon);
         ClientRep *cmdr = scEvent->cmdr ? findClient(scEvent->cmdr) : NULL;

         setCommander(peon, cmdr);
         
         if (cmdr)
         {
            if (playerId == peon->id)
               printf(FearGui::FearGuiChatDisplay::MSG_TeamChat, "You have been placed under the command of %s.", cmdr->name);
            else if (playerId == cmdr->id)
               printf(FearGui::FearGuiChatDisplay::MSG_TeamChat, "You are now in command of %s.", peon->name);
         }
         else if(playerId == peon->id)
            printf(FearGui::FearGuiChatDisplay::MSG_TeamChat, "You now have no commander.", peon->name);
         break;
      } 
         
      case PlayerSayEventType:
         PlayerSayEvent *sayEvent;
         if (isServer)
            return FALSE;
            
         sayEvent = (PlayerSayEvent*)event;
         
         //find the sender's name
         rep = findClient(sayEvent->sender);
         const char *res;
         
         if(filterBadWords)
            languageFilter(sayEvent->message);
         
         res = Console->executef(3, "onClientMessage", rep ? intToStr(rep->id) : "0", sayEvent->message);
         if(CMDConsole::getBool(res))
         {
            playSoundMessage(rep, manager, sayEvent->message);
            if (rep) {
               printf(sayEvent->msgType, "%s: %s", rep->name, sayEvent->message);
            }
            else printf(sayEvent->msgType,"%s", sayEvent->message);
         }
         break;
         
      case DeltaScoreEventType:
         if(isServer)
            return false;
         
         DeltaScoreEvent *fpse;
         fpse = (DeltaScoreEvent *) event;
         if(fpse->teamScore)
         {
            if(fpse->id == -1)
               strcpy(teamScoreHeading, fpse->scoreString);
            else
            {
               TeamRep *team = findTeam(fpse->id);
               if(team)
               {
                  strcpy(team->scoreString, fpse->scoreString);
                  team->scoreSortValue = fpse->sortValue;
               }
            }
         }
         else
         {
            if(fpse->id == 2048)
               strcpy(clientScoreHeading, fpse->scoreString);
            else
            {
               rep = findClient(fpse->id);
               if(rep)
               {
                  strcpy(rep->scoreString, fpse->scoreString);
                  rep->scoreSortValue = fpse->sortValue;
               }
            }
         }
         break;
      case TeamAddEventType:
      {
         if(isServer)
            return false;
         teamList[++numTeams] = ((TeamAddEvent *) event)->teamRep;
         Console->executef(3, "onTeamAdd", intToStr(numTeams), teamList[numTeams].name);

         // As good a place for this as any ...
         DecalManager *dmgr = 
            dynamic_cast<DecalManager *>(manager->findObject(DecalManagerId));

         if (dmgr)
         {
            dmgr->reset();
         }
         break;
      }
      case PlayerAddEventType:
         if(isServer)
            return false;
            
         PlayerAddEvent * pae; //=(PlayerAddEvent*)event;  <<===BCC chokes (???)
         pae = (PlayerAddEvent *)event;

         rep = clientList + (pae->pr.id - 2048);
         *rep = pae->pr;
         if( rep->isClientRep() ){
            pushFrontSLL( & clientLink, rep );
            numClients++;
         }
         pushFrontSLL( & baseRepLink, (BaseRep *) rep );
         numBaseReps++;
         if( rep->isClientRep() )
         {
            Console->executef(2, "onClientJoin", intToStr(rep->id));
            printf(FearGui::FearGuiChatDisplay::MSG_System, 
                        "%s connected to the game.", rep->name);
         }
         break;

      case PlayerRemoveEventType:
         if(isServer)
            return false;
         cl = findBaseRep(((PlayerRemoveEvent *) event)->pid);
         if( cl->isClientRep() )
         {
            Console->executef(2, "onClientDrop", intToStr(cl->id));
            printf(FearGui::FearGuiChatDisplay::MSG_System, "%s dropped.", cl->name);
         }
         removeClient(cl);
         break;
      case MissionResetEventType:
      {
         if(isServer)
            return false;

         SimPathManager *spm = dynamic_cast<SimPathManager *>
                               (manager->findObject(SimPathManagerId));

         if (spm)
            spm->fullReset();

         numTeams = -1;
         BaseRep *cl;
         cl = baseRepLink;
         cg.curCommand = -1;
         while(cl)
         {
            cl->curCommand = -1;
            cl->commandStatus = CMD_NONE;
            //cl->scoreString[0] = 0;
            cl = cl->nextClient;
         }
      }
      break;
      default:
         return false;
   }
   return true;
}

#define MaxPrintfLength 256

static char printfDest[1024];
static char consoleOut[2048];

extern void expandEscape(char *dest, const char *src);

void __cdecl PlayerManager::printf(int msgType, const char* fmt,...)
{
   va_list argptr;

   va_start(argptr, fmt);
   int cnt = vsprintf(printfDest, fmt, argptr);
	AssertFatal(cnt < 1024,
			"PlayerManager::printf: Overflowed format buffer");
   va_end(argptr);
   
   SimGui::Control *playGui;
   playGui = (SimGui::Control*)(manager->findObject("playGui"));
   if (playGui)
   {
      FearGui::FearGuiChatDisplay *chatCtrl;
      chatCtrl = (FearGui::FearGuiChatDisplay*)(playGui->findControlWithTag(IDCTG_HUD_CHAT_DISPLAY));
      if (chatCtrl)
         chatCtrl->newMessage(printfDest, msgType);
   }
   CMDConsole *con = CMDConsole::getLocked();
   expandEscape(consoleOut, printfDest);

   con->printf("MSG: \"%s\"", consoleOut);
}

void PlayerManager::missionReset()
{
   // ClientRep *cl = clientLink;
   BaseRep *cl = baseRepLink;
   while(cl)
   {
      if( cl->isClientRep() )
         MissionResetEvent::post(manager, cl->id);
      cl->curCommandSequence = 0;
      cl->lastCommandSequence = 0;
      cl->scoreSequence = 1;
      cl->lastScoreSequence = 0;
      cl->scoreString[0] = 0;
      cl->curCommand = -1;
      cl->commandStatus = CMD_NONE;
      cl->lastObjectiveSequence = 0;
      cl = cl->nextClient;
   }
   curCommandSequence = 0;
   numTeams = -1;

   SimPathManager *spm = dynamic_cast<SimPathManager *>
                         (manager->findObject(SimPathManagerId));
   if (spm)
      spm->fullReset();
   teamAdded(-1);
}

void PlayerManager::reset()
{
   int i;
   for(i = 0; i < MaxClients; i++)
   {
      clientList[i].nextClient = clientList + i + 1;
      clientList[i].id = i + 2048;
      clientList[i].playerPSC = NULL;
      clientList[i].name = NULL;
   }
   curScoreSequence = 0;
   clientLink = NULL;
   baseRepLink = NULL;
   clientFreeList = clientList + 1;
   clientList[i-1].nextClient = NULL;
   numTeams = -1;
   numClients = 0;
   numBaseReps = 0;
}

void PlayerManager::ghostAlwaysDone(int playerId)
{
   SimPathManager *spm = dynamic_cast<SimPathManager *>
                         (manager->findObject(SimPathManagerId));
   if (spm)
      spm->updateClient((Net::PacketStream *) manager->findObject(playerId));
}

PlayerManager::TeamRep *PlayerManager::findTeam(int id)
{
   if(id < -1 || id >= numTeams)
      return NULL;
   return teamList + id + 1;
}

PlayerManager::TeamRep *PlayerManager::findTeam(const char *name)
{
   int i;
   for(i = 1; i <= numTeams; i++)
      if(!strcmp(teamList[i].name, name))
         return teamList + i;
   return NULL;
}

PlayerManager::ClientRep *PlayerManager::findClient(int id)
{
   int index = id - 2048;
   
   if( (index >= 0 && index <= 127) && clientList[index].name )
   {
      ClientRep * cl = clientList + index;
      if( cl->isClientRep() )
         return cl;
   }
   return NULL;
}

PlayerManager::ClientRep * PlayerManager::findBaseRep(int id)
{
   if( !((id-=2048) & ~127)  && clientList[id].name )
      return clientList + id;
   return NULL;
}

const char *PlayerManager::getPlayerSkinBase(int clientId)
{
   ClientRep *cl = findBaseRep(clientId);
   if(!cl)
      return "base";
   if(cl->skinBase && cl->skinBase[0])
      return cl->skinBase;
   TeamRep *clTeam = findTeam(cl->team);
   if (clTeam) return clTeam->skinBase;
   else return "base";
}

const char *PlayerManager::getTeamSkinBase(int teamId)
{
   TeamRep *team = findTeam(teamId);
   if(!team)
      return "base";
   else
      return team->skinBase;
}
