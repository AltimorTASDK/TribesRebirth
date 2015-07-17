#include "version.h"
#include "console.h"
#include "netCSDelegate.h"
#include "netPacketStream.h"
#include "netGhostManager.h"
#include "PlayerManager.h"
#include "FearDcl.h"
#include "console.h"
#include "fearCSDelegate.h"
#include "fear.strings.h"
#include "FearPlayerPSC.h"
#include "FearGlobals.h"
#include "dataBlockManager.h"
#include "fearGuiServerList.h"
#include "dlgJoinGame.h"
#include <stringTable.h>
#include <simpathmanager.h>
#include <sensormanager.h>
#include <g_timer.h>
#include <decalmanager.h>
#include "smokePuff.h"
#include <banList.h>
#include "fearGuiChatDisplay.h"

IMPLEMENT_PERSISTENT(FearCSDelegate);

const BYTE gPingVerifyByte = 0xf0;

namespace SimGui {
extern Control *findControl(const char *name);
};

//----------------------------------------------------------------------------

const char *gameString = "Tribes";

static int __cdecl playerScoreCompare(const void *a,const void *b)
{
   PlayerManager::ClientRep **entry_A = (PlayerManager::ClientRep **)(a);
   PlayerManager::ClientRep **entry_B = (PlayerManager::ClientRep **)(b);
   return ((*entry_B)->scoreSortValue - (*entry_A)->scoreSortValue);
} 

static int __cdecl teamScoreCompare(const void *a,const void *b)
{
   PlayerManager::TeamRep **entry_A = (PlayerManager::TeamRep **)(a);
   PlayerManager::TeamRep **entry_B = (PlayerManager::TeamRep **)(b);
   return ((*entry_B)->scoreSortValue - (*entry_A)->scoreSortValue);
} 

//----------------------------------------------------------------------------

FearCSDelegate::~FearCSDelegate()
{
   if(sg.csDelegate == this)
      sg.csDelegate = NULL;
   if(cg.csDelegate == this)
      cg.csDelegate = NULL;
   if(isServer)
       sg.csDelegate = 0;
    else
       cg.csDelegate = 0;
       
   for (int i = 0; i < mServerList.size(); i++)
   {
      destroy(&mServerList[i]);
   }       
}

void FearCSDelegate::onGameInfoPacket(const char *transportAddress, BYTE *data, int datalen)
{
   if(gBanList.isBanned(transportAddress))
   {
      Console->dbprintf(2, "SimNET: ignoring GI from banned address %s", transportAddress);
      return;
   }
   if(data[0] == GameSpyPing || data[0] == GameSpyQuery1 && datalen == 3)
   {
      char buf[1500];
      // generic query struct:
      Vector<PlayerManager::ClientRep *> players;
      for(PlayerManager::ClientRep *j = playerManager->getClientList(); j; j = j->nextClient)
         players.push_back(j);

      MemRWStream strm(1400, buf);
      if(data[0] == GameSpyPing)
         strm.write(BYTE(GameSpyPingResponse));
      else
         strm.write(BYTE(GameSpyQuery1Response));
      strm.write(BYTE(data[1]));
      strm.write(BYTE(data[2]));
      strm.write(BYTE(GameSpyQuery1));
      strm.writeString(gameString);
      strm.writeString(FearVersion);
      strm.writeString(Console->getVariable("Server::Hostname"), 32);
      strm.write(BYTE(Console->getBoolVariable("Dedicated")));
      const char *pass = Console->getVariable("Server::Password");
      strm.write(BYTE(pass[0] != 0));
      strm.write(BYTE(players.size()));
      strm.write(BYTE(Console->getIntVariable("Server::MaxPlayers")));

      if(data[0] == GameSpyQuery1)
      {
         strm.write((UInt16) Console->getIntVariable("CPU::estimatedSpeed"));
         strm.writeString(Console->getVariable("modList"), 32);
         strm.writeString(Console->getVariable("Game::MissionType"), 8);
         strm.writeString(Console->getVariable("MissionName"), 16);
         strm.writeString(Console->getVariable("Server::Info"));
         strm.write(BYTE(playerManager->getNumTeams()));
         strm.writeString(playerManager->teamScoreHeading, 64);
         strm.writeString(playerManager->clientScoreHeading, 64);
         int i;
         for(i = 0; i < playerManager->getNumTeams(); i++)
         {
            PlayerManager::TeamRep *tm = playerManager->findTeam(i);
            strm.writeString(tm->name, 32);
            strm.writeString(tm->scoreString, 32);
         }
         for(i = 0; i < players.size(); i++)
         {
            strm.write(BYTE(players[i]->ping >> 2));
            strm.write(BYTE(players[i]->packetLoss * 100));
            strm.write(BYTE(players[i]->team));
            strm.writeString(players[i]->name, 18);
            strm.writeString(players[i]->scoreString, 32);
         }
      }
      send(transportAddress, buf, strm.getPosition());
      return;
   }
   Parent::onGameInfoPacket(transportAddress, data, datalen);
}

bool FearCSDelegate::processArguments(int argc, const char **argv)
{
   if( Parent::processArguments(argc, argv) )
   {
      if (isServer)
      {
         manager->addObject(new SensorManager, SensorManagerId);
      }
      else
      {
         manager->addObject(new DecalManager, DecalManagerId);
         manager->addObject(new SmokeManager, SmokeManagerId);
      }

      manager->addObject(new SimSet, SensorVisibleSetId);

      manager->addObject(new DataBlockManager(isServer), DataBlockManagerId);
      manager->addObject(new SimPathManager, SimPathManagerId);
      playerManager = new PlayerManager(isServer);
      manager->addObject(playerManager, PlayerManagerId);
      if(isServer)
      {
         sg.csDelegate = this;
         playerManager->sendPingPackets();
      }
      else
         cg.csDelegate = this;
      return true;
   }
   return false;
}

bool FearCSDelegate::simConnect(const char *transportAddress)
{
   if(isServer)
      return false;                    
      
   BYTE buffer[DNet::MaxPacketSize];

   BitStream bstream(buffer, DNet::MaxPacketSize);
   //first write the packet version
   bstream.write(sizeof(UInt16), (void*)&mPacketVersion);
   
   bstream.writeString(gameString);
   char buf[32];
   strcpy(buf, FearVersion);
   char *ptr = strchr(buf, '.');
   ptr++;
   *ptr++ = '0';
   *ptr = 0;
   
   bstream.writeString(buf);

   bstream.writeString(Console->getVariable("Server::JoinPassword"));
   bstream.writeString(Console->getVariable("PCFG::Name"));
   bstream.writeString(Console->getVariable("PCFG::Gender"));
   bstream.writeString(Console->getVariable("PCFG::Voice"));
   bstream.writeString(Console->getVariable("PCFG::SkinBase"));

   vcConnection = connect(transportAddress, buffer, bstream.getBytePtr() - buffer);
   return (vcConnection != NULL);

}

void FearCSDelegate::onConnectionRequested(DNet::VC *vc, BYTE *data, int datalen)
{
   if(gBanList.isBanned(vc->getAddressString()))
   {
      Console->dbprintf(2, "SimNET: ignoring connect from banned address %s", vc->getAddressString());
      vc->deleteVC();
      return;
   }

   const char *rejectString;
   float vers;
   BitStream bstream(data, datalen);
   
   if(!isServer)
   {
      rejectString = "Cannot connect to a client.";
      goto reject;
   }

   //first read the packet version
   UInt16 packetVersion;
   bstream.read(sizeof(UInt16), &packetVersion);
   
   if (packetVersion != mPacketVersion)
   {
      rejectString = "Wrong packet version.";
      goto reject;
   }

   char buf[256];
   bstream.readString(buf);
   if(strcmp(buf, gameString))
   {
      rejectString = "Wrong game type.";
      goto reject;
   }

   bstream.readString(buf);
   int len;
   len = strchr(FearVersion, '.') - FearVersion - 1;

   if(strncmp(buf, FearVersion, len))
   {
      rejectString = "Wrong game version.";
      goto reject;
   }
      
   bstream.readString(buf);
   const char *password;
   password = Console->getVariable("Server::Password");
   if (password[0] && strcmp(buf, password))
   {
      rejectString = "Wrong password.";
      goto reject;
   }
   
   int maxClients;
   maxClients = Console->getIntVariable("Server::MaxPlayers", 32);
   if(maxClients > MaxClients)
      maxClients = MaxClients;
   if (playerManager->getNumClients() >= maxClients)
   {
      rejectString = "Server is full.";
reject:
      vc->reject((unsigned char *)rejectString, strlen(rejectString)+1);
      return;
   }
   else
   {
      //get the player name

      bstream.readString(buf);
      buf[16] = 0;
      //loop through and remove any bad chars
      for (int i = 0; i < 16; i++)
      {
         if (buf[i] == '\0') break;
         else if (((BYTE)buf[i] < 32) || ((BYTE)buf[i] > 127) || (buf[i] == '\\') || (buf[i] == '\"') || (buf[i] == '~'))
         {
            buf[i] = '*';
         }
      }
      
      const char *name = stringTable.insert(buf, true);
      const char *tname = name;
      //check to see how many people already have that name
      int count = 1;
   
      for(;;)
      {
         PlayerManager::ClientRep *cl;
         for(cl = playerManager->getClientList(); cl; cl = cl->nextClient)
            if(tname == cl->name)
               break;
         if(!cl)
            break;
         sprintf(buf, "%s.%d", name, count++);
         tname = stringTable.insert(buf, true);
      }
      const char *gender = bstream.readSTString();
      const char *voice = bstream.readSTString();

      int id = playerManager->getFreeId();

      Net::PacketStream *pstream = new Net::PacketStream(vc, Net::PacketStream::ServerMode, NULL);
      manager->addObject(pstream, id);

      BitStream bs(vc->getDataBuffer(), 1500);
      bs.writeInt(id, 32);
      bs.writeInt(manager->getId(), 32);

      vc->accept(vc->getDataBuffer(), (bs.getCurPos() + 7) >> 3);
      addStreamClients(pstream);
      vc->userData = (DWORD) pstream;

      playerManager->clientAdded(tname, voice, stringTable.insert("base"), !stricmp(gender, "male"));
   }
}

void FearCSDelegate::addStreamClients(Net::PacketStream *ps)
{
   Parent::addStreamClients(ps);

   PlayerPSC *psc = new PlayerPSC(isServer);

   manager->registerObject(psc);
   ps->addObject(psc);
   ps->assignName(psc, "PlayerPSC");

   Net::GhostManager *gm = new Net::GhostManager(!isServer);
   manager->registerObject(gm);
   ps->addObject(gm);
   ps->assignName(gm, "GhostManager");
}

bool FearCSDelegate::playDemo(const char *fileName)
{
   if(cg.packetStream)
   {
      manager->unregisterObject(cg.packetStream);
      delete cg.packetStream;
   }
   // force the delete list to clear - HACK
   manager->advanceToTime(manager->getTargetTime());
   cg.dbm->reset();

   if(!Parent::playDemo(fileName))
      return false;

   SimObject *obj = manager->findObject(2048);
   if(obj)
      cg.packetStream = dynamic_cast<Net::PacketStream *>(obj);
   playerManager->reset();
   
   //clear out the previous chat display
   FearGui::FearGuiChatDisplay *chatDisplay = NULL;   
   SimGui::Control *ctrl = SimGui::findControl("chatDisplayHud");
   if (ctrl) chatDisplay = dynamic_cast<FearGui::FearGuiChatDisplay*>(ctrl);
   if (chatDisplay) chatDisplay->clear();
   Console->setBoolVariable("$playingDemo", true);
   CMDConsole::getLocked()->evaluate("onConnection(Accepted);", false);
   return true;
}

void FearCSDelegate::cancelDuringConnection(void)
{
   //vcConnection is set to the return value of connect(),
   //but I don't know if it has to be deleted here.
   if (vcConnection)
   {
      vcConnection->disconnect();
      vcConnection = NULL;
   }
}

void FearCSDelegate::onConnectionAccepted(DNet::VC *vc, BYTE *data, int datalen)
{
   if(cg.packetStream)
   {
      manager->unregisterObject(cg.packetStream);
      delete cg.packetStream;
   }
   // force the delete list to clear - HACK
   manager->advanceToTime(manager->getTargetTime());
   cg.dbm->reset();

   // build the packet stream and stuff...
   BitStream bs(data, datalen);
   UInt32 manId = bs.readInt(32);
   UInt32 remoteId = bs.readInt(32);

   SimGroupObjectIdEvent event1( manId );
   manager->processEvent( &event1 );
   
   CMDConsole *con = CMDConsole::getLocked();
   const char *demoFileName = con->getVariable("recorderFileName");
   Net::PacketStream *pstream;

   if(demoFileName[0])
      pstream = new Net::PacketStream(vc, Net::PacketStream::RecordMode, demoFileName);
   else
      pstream = new Net::PacketStream(vc, Net::PacketStream::NormalMode, NULL);

   manager->addObject(pstream, remoteId);

   addStreamClients(pstream);
   vc->userData = (DWORD) pstream;
   cg.packetStream = pstream;

   // packet stream is set up...
   
   //clear out the previous chat display
   FearGui::FearGuiChatDisplay *chatDisplay = NULL;   
   SimGui::Control *ctrl = SimGui::findControl("chatDisplayHud");
   if (ctrl) chatDisplay = dynamic_cast<FearGui::FearGuiChatDisplay*>(ctrl);
   if (chatDisplay) chatDisplay->clear();
   
   // reset the playerManager
   playerManager->reset();
   
   Console->setBoolVariable("$playingDemo", false);
   CMDConsole::getLocked()->evaluate("onConnection(Accepted);", false);
   vcConnection = NULL;
}

bool FearCSDelegate::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
      
   //get the major version number
   char buf[256];
   strcpy(buf, FearVersion);
   char *tempPtr = strchr(buf, '.');
   AssertFatal(tempPtr, "Fear version is not of the form x.y");
   *tempPtr = '\0';
   mPacketVersion = atoi(buf);
      
   // instantiate the scoreboard.
   return true;
}


void FearCSDelegate::onGhostAlwaysDone(Net::PacketStream *pstream)
{
   if(isServer)
   {
      playerManager->ghostAlwaysDone(pstream->getId());
      Console->evaluatef("onServerGhostAlwaysDone(%d);", pstream->getId());
   }
   else
      Console->executef(1, "onClientGhostAlwaysDone");
}

void FearCSDelegate::onConnectionError(Net::PacketStream *pstream, const char *string)
{
   if(isServer)
   {
      playerManager->clientDropped(pstream->getId());
      pstream->disconnect();
      pstream->deleteObject();
   }
   else
   {
      cg.packetStream = NULL;
      // renumber the manager on the client

      SimGroupObjectIdEvent event1( 0 );
      manager->processEvent( &event1 );

      playerManager->reset();      
   
      char idbuf1[24], idbuf2[24];
      sprintf(idbuf1, "%d", pstream->getId());
      sprintf(idbuf2, "%d", manager->getId());
      Console->executef(4, "onConnectionError", idbuf1, idbuf2, string);
      pstream->disconnect();
      pstream->deleteObject();
   }
}   

void FearCSDelegate::simDisconnect()
{
   Parent::simDisconnect();
   playerManager->reset();
   cg.packetStream = NULL;
   vcConnection = NULL;
}

void FearCSDelegate::onConnectionRejected(DNet::VC *vc, BYTE *data, int datalen)
{
   if(!cg.packetStream)
   {
      CMDConsole *con = CMDConsole::getLocked();

      con->setVariable("errorString", (const char *)data);
      con->evaluate("onConnection(Rejected);", false);
      Parent::onConnectionRejected(vc, data, datalen);
   }
   vcConnection = NULL;
}

void FearCSDelegate::onConnectionDropped(DNet::VC *vc, BYTE *data, int datalen, bool timeout)
{
   Net::PacketStream *pstream = (Net::PacketStream *) vc->userData;
   // get rid of the player object...

   if(!pstream)
      return;

   if(isServer)
   {
      AssertWarn(pstream, "Ack Null pstream - how'd that get there?");
      if(pstream)
         playerManager->clientDropped(pstream->getId());
   }
   else
   {
      if(cg.packetStream == pstream)
      {
         cg.packetStream = NULL;
         // renumber the manager on the client

         SimGroupObjectIdEvent event1( 0 );
         manager->processEvent( &event1 );

         playerManager->reset();      
         CMDConsole *con = CMDConsole::getLocked();

         if(timeout)
            con->evaluate("onConnection(TimedOut);", false);
         else
         {
            if(datalen)
               con->setVariable("errorString", (const char *)data);
            else
               con->setVariable("errorString", "");
            con->evaluate("onConnection(Dropped);", false);
         }
      }
   }
   if(pstream)
   {
      pstream->getVC()->userData = 0;
      pstream->connectionDied();
      manager->unregisterObject(pstream);
      delete pstream;
   }
   
   vcConnection = NULL;
}

void FearCSDelegate::kick(int playerId, const char *reason)
{
   if(isServer)
   {
      SimObject *obj = manager->findObject(playerId);
      Net::PacketStream *pstream;
      // if it's a player
      if((pstream = dynamic_cast<Net::PacketStream *>(obj)) != NULL)
      {
         playerManager->clientDropped(playerId);
         
         if(!reason)
            reason = "You were kicked from the game.";

         pstream->getVC()->userData = 0;
         pstream->getVC()->disconnect((BYTE *) reason, strlen(reason) + 1);
         pstream->connectionDied();
         pstream->deleteObject();
      }
   }
}

namespace Net
{
struct Addr
{
   char address[256];
};
extern Vector<Addr> timedoutList;
};

static int numMasters = 0;
static int curMaster = 0;

void FearCSDelegate::rebuildServerList(void)
{
   if(isServer)
      return;
      
   //clear the list
   int i;
   for (i = 0; i < mServerList.size(); i++)
   {
      destroy(&mServerList[i]);
   }
   mServerList.clear();

   Console->printf("Querying servers in address book...");
   Net::timedoutList.clear();
   
   //rebuild the timedout list
   if (cg.gameServerList)
   {
      cg.gameServerList->rebuildNeverPingList();
   }

   char transportAddress[255];
   int port = Console->getIntVariable("Server::Port");
   sprintf(transportAddress, "IP:BROADCAST:%d", port);
   pushPingInfoRequest(transportAddress);
   sprintf(transportAddress, "IPX:BROADCAST:%d", port);
   pushPingInfoRequest(transportAddress);

   // now do the first master:

   if(Console->getBoolVariable("pref::lanOnly", false))
      return;

   Console->executef(1, "checkMasterTranslation");

   numMasters = Console->getIntVariable("Server::numMasters");
   curMaster = 0;
   if(!numMasters)
      return;

   const char *addr = Console->getVariable("Server::XLMasterN0");
   if(addr[0])
      pushPingInfoRequest(addr);
}

void FearCSDelegate::getPingInfo(PingInfo &pingInfo)
{
   // sample routine, should be overridden in sub class
   pingInfo.gameType = gPingVerifyByte;
   pingInfo.playerCount = playerManager->getNumClients();
   pingInfo.maxPlayers = 32;
   
   const char *serverName = Console->getVariable("Server::Hostname");
   //the first 2 bytes of the name are going to be the version number
   
   UInt16 *namePtr = (UInt16*)&pingInfo.name[0];
   *namePtr = mPacketVersion;
   
   strncpy(&pingInfo.name[2], serverName, 29);
   pingInfo.name[31] = '\0';
}

int FearCSDelegate::getGameInfo(GameInfo &gameInfo, int maxDataSize, int /*dataLeftToRead*/)
{
   //use a bitstream to load the info...
   BitStream bstream(&gameInfo.data[0], maxDataSize);
   
   //write the packet version
   bstream.write(sizeof(UInt16), (void*)&mPacketVersion);

   //the game string, and game version number
   bstream.writeString(gameString);
   bstream.writeString(FearVersion);
   
   //server name
   const char *str = Console->getVariable("Server::Hostname");
   if (str && str[0]) bstream.writeString(str);
   else bstream.writeString("N/A");

   //number of players
   bstream.write(BYTE(playerManager->getNumClients()));
   
   //max players
   BYTE maxPlayers = 0;
   const char *maxPlyrsStr = Console->getVariable("Server::MaxPlayers");
   if (maxPlyrsStr && maxPlyrsStr[0])
   {
      const char *temp = maxPlyrsStr;
      while (*temp)
      {
         maxPlayers = (10 * maxPlayers) + (*temp - '0');
         temp++;
      }
   }
   bstream.write(maxPlayers);
   
   //mission name
   str = Console->getVariable("MissionName");
   if (str && str[0]) bstream.writeString(str);
   else bstream.writeString("N/A");
   
   //dedicated?
   const char *dedicated = Console->getVariable("Dedicated");
   if (! stricmp(dedicated, "TRUE")) bstream.write(BYTE(1));
   else bstream.write(BYTE(0));
   
   //password?
   const char *password = Console->getVariable("Server::Password");
   if (password && password[0]) bstream.write(BYTE(1));
   else bstream.write(BYTE(0));
   
   //write the processor speed
   int cpuSpeed;
   const char *cpuSpeedStr = Console->getVariable("CPU::estimatedSpeed");
   if ((! cpuSpeedStr) || (! cpuSpeedStr[0])) cpuSpeed = -1;
   else cpuSpeed = atoi(cpuSpeedStr);
   bstream.write(cpuSpeed);
   bstream.writeString(Console->getVariable("$modList"));
   bstream.writeString(Console->getVariable("$Game::MissionType"));
   bstream.writeString(Console->getVariable("$Server::Info"));
      
   //calculate the size left
   BYTE *lenPtr = bstream.getBytePtr();
   int lengthAvail = maxDataSize - (lenPtr - &gameInfo.data[0]);
   
   //here we calculate all the strings....
   Vector<PlayerManager::TeamRep *> teams;
   Vector<PlayerManager::ClientRep *> players;
   bool drawTeamScores;
   
   int newSize;
   if(playerManager->teamScoreHeading[0])
   {
      drawTeamScores = true;
      newSize = playerManager->getNumTeams() + playerManager->getNumClients() + 2;
   }
   else
   {
      drawTeamScores = false;
      newSize = playerManager->getNumClients() + 1;
   }
      
   //clear the vectors
   teams.clear();
   players.clear();
   
   if(drawTeamScores)
   {
      int i;
      for(i = 0; i < playerManager->getNumTeams(); i++)
         teams.push_back(playerManager->findTeam(i));

      //sort the teams by score
      m_qsort((void *)&teams[0], teams.size(), sizeof(PlayerManager::TeamRep *), teamScoreCompare);
   }   
   for(PlayerManager::ClientRep *j = playerManager->getClientList(); j; j = j->nextClient)
      players.push_back(j);

   //sort the players by score
   m_qsort((void *)&players[0], players.size(), sizeof(PlayerManager::ClientRep *), playerScoreCompare);
   
   //first push the tab stop strings
   bstream.writeString(playerManager->teamScoreHeading);
   bstream.writeString(playerManager->clientScoreHeading);
   
   //now create the bufs
   for (int i = 0; i < newSize; i++)
   {
      char buf[1024];
      buf[0] = '\0';

      if ((i == 0) && drawTeamScores)
      {
         // team heading
         buf[0] = '0';
         buf[1] = '1';
         strcpy(&buf[2], playerManager->teamScoreHeading);
      }
      else if ((i < teams.size()+1 ) && drawTeamScores)
      {
         buf[0] = '0';
         buf[1] = '0';
         const char *s = teams[i - 1]->scoreString;
         char *bufptr = &buf[2];

         while(*s)
         {
            if(*s == '%' && *(s+1) == 't')
            {
               strcpy(bufptr, teams[i-1]->name);
               bufptr += strlen(bufptr);
               s += 2;
            }
            else
               *bufptr++ = *s++;
         }
         *bufptr = 0;
      }
      else if ((drawTeamScores && (i == teams.size() + 1)) || (!drawTeamScores && (i == 0)))
      {
         buf[0] = '1';
         buf[1] = '1';
         strcpy(&buf[2], playerManager->clientScoreHeading);
      }
      else
      {
         buf[0] = '1';
         buf[1] = '0';
         int pidx = i - teams.size() - (drawTeamScores ? 2 : 1);
         const char *s = players[pidx]->scoreString;
         char *bufptr = &buf[2];

         while(*s)
         {
            if(*s == '%' && *(s+1) == 'n')
            {
               strcpy(bufptr, players[pidx]->name);
               bufptr += strlen(bufptr);
               s += 2;
            }
            else if(*s == '%' && *(s+1) == 't')
            {
               int team = players[pidx]->team;
               if(team != -1)
               {
                  strcpy(bufptr, playerManager->findTeam(team)->name);
                  bufptr += strlen(bufptr);
               }
               s += 2;
            }
            else if(*s == '%' && *(s+1) == 'p')
            {
               sprintf(bufptr, "%d", players[pidx]->ping);
               bufptr += strlen(bufptr);
               s += 2;
            }
            else if(*s == '%' && *(s+1) == 'l')
            {
               sprintf(bufptr, "%d", int(players[pidx]->packetLoss * 100));
               bufptr += strlen(bufptr);
               s += 2;
            }
            else
               *bufptr++ = *s++;
         }
         *bufptr = 0;
      }
      
      //see if the buf will fit
      int totalLength = strlen(buf); 
      
      if (totalLength <= lengthAvail)
      {
         //if this is a header. preceed the header with an empty string
         if (buf[1] == '1')
         {
            bstream.writeString("00");
            lengthAvail -= strlen("00");
         }
         //write out the buf
         bstream.writeString(buf);
         lengthAvail -= totalLength;
         
      }
      else
      {
         break;
      }
   }
   
   //fill in the gameInfo length
   lenPtr = bstream.getBytePtr();
   gameInfo.dataSize = lenPtr - &gameInfo.data[0]; 
   
   return 0;
}

static void strncpyt(char *dst, const char *src, int n)
{
   while(n--)
   {
      char c = *dst++ = *src++;
      if(!c)
         return;
   }
   *dst = 0;
}

void FearCSDelegate::gotGameInfo(const GameInfo &gameInfo)
{
   // read the data using a Bitstream class
   BitStream bstream((char*)(&gameInfo.data[0]), gameInfo.dataSize);
   
   //read and check the version
   UInt16 packetVersion;
   bstream.read(sizeof(UInt16), &packetVersion);
   if (packetVersion != mPacketVersion)
   {
      Console->printf("Different version:  %s", gameInfo.address);
      serverPingTimeout(gameInfo.address);
      return;
   }
   
   //read the game string and check
   char buf[256];
   bstream.readString(buf);
   if (strcmp(buf, gameString))
   {
      serverPingTimeout(gameInfo.address);
      return;
   }
   
   //if the packet version and game string are correct, add the server to the list
   //first see if the server is already in the list
   ServerInfo *newServer = NULL;
   for (int i = 0; i < mServerList.size(); i++)
   {
      if (! strcmp(mServerList[i].transportAddress, gameInfo.address))
      {
         newServer = &mServerList[i];
         //if the game info has a new key, it's a response from a new query,
         //otherwise, don't clear the player list - add the new players to the end
         if (newServer->key != gameInfo.key) newServer->mInfoList.clear();
         break;
      }
   }
   
   //if the server was not already in the list, add a new server entry
   if (! newServer)
   {
      mServerList.increment();
      new(&mServerList.last()) ServerInfo;
      newServer = &mServerList.last();
   }
   
   //set the key
   newServer->key = gameInfo.key;
   
   //copy the game string
   strncpyt(newServer->game, buf, 31);
   
   //get the address and ping directly from the GameInfo.
   strncpyt(newServer->transportAddress, gameInfo.address, 31);
   newServer->pingTime = gameInfo.ping;
   
   //read the game version
   bstream.readString(buf);
   strncpyt(newServer->version, buf, 31);
   newServer->correctVersion = TRUE;   //would have aborted earlier otherwise
   
   //get the game name
   bstream.readString(buf);
   strncpyt(newServer->name, buf, 63);
   
   //get the number of players
   bstream.read(&newServer->numPlayers);
      
   //get the maximum number of players
   bstream.read(&newServer->maxPlayers);
   
   //get the mission name
   bstream.readString(buf);
   strncpyt(newServer->missionName, buf, 31);
   
   //dedicated?
   BYTE temp;
   bstream.read(&temp);
   newServer->dedicated = temp;
   
   //password?
   bstream.read(&temp);
   newServer->password = temp;
   
   //cpuSpeed
   bstream.read(&newServer->cpuSpeed);
   bstream.readString(buf);
   strncpyt(newServer->modName, buf, 31);
   bstream.readString(buf);
   strncpyt(newServer->missionType, buf, 31);
   bstream.readString(newServer->hostInfo);
   
   //now read the tab stop strings
   bstream.readString(&newServer->teamScoreHeading[0]);
   bstream.readString(&newServer->clientScoreHeading[0]);
   
   //calculate the size left to read
   BYTE *lenPtr = bstream.getBytePtr();
   int lengthToRead = gameInfo.dataSize - (lenPtr - &gameInfo.data[0]);
   
   while (lengthToRead > 0)
   {
      Info newInfo;
      bstream.readString(&newInfo.buf[0]);
      
      //push the info
      newServer->mInfoList.push_back(newInfo);
      
      //update the lengthToRead
      lenPtr = bstream.getBytePtr();
      lengthToRead = gameInfo.dataSize - (lenPtr - &gameInfo.data[0]);
   }
   
   //signal the server list of incoming
   if (cg.gameServerList)
   {
      cg.gameServerList->gotGameInfo(newServer);
   }
   SimGui::JoinGameDelegate *dlgt = (SimGui::JoinGameDelegate *) manager->findObject("SimGui::JoinGameDelegate");
   if (dlgt)
   {
      if (gameList.size() > 0) dlgt->updatePingLists(gameList.size(), mServerList.size(), FALSE);
      else dlgt->queryFinished();
      dlgt->verifyServer();
   }
}

void FearCSDelegate::removeServer(const char *address)
{
   if ((! address) || (! address[0])) return;
   
   //remove it from the serverList if it exists...
   for (int i = 0; i < mServerList.size(); i++)
   {
      if (! stricmp(address, mServerList[i].transportAddress))
      {
         destroy(&mServerList[i]);
         mServerList.erase(i);
         break;
      }
   }
}

void FearCSDelegate::gotMasterServer(const char *address, const char *name, const char *motd, bool firstPacket)
{
   SimGui::JoinGameDelegate *dlgt = (SimGui::JoinGameDelegate *) manager->findObject("SimGui::JoinGameDelegate");
   if (firstPacket)
   {
      if ((! name) || (! name[0])) return;
      if ((! motd) || (! motd[0])) return;
      
      //store the server name
      Console->setVariable("pref::MSName", name);
      
      //store the server MOTD  (message of the day)
      Console->setVariable("pref::MSMOTD", motd);
      
      if (dlgt)
      {
         dlgt->verifyMasterMOTD();
      }
      
      address;
      Console->printf("master server %s says \"%s\"", name, motd);
   }
   
   //let the delegate know how many servers left to ping, and how many it already has
   if (dlgt)
   {
      dlgt->updatePingLists(pingList.size(), gameList.size(), TRUE);
   }
   
   if (cg.gameServerList)
   {
      for (int i = 0; i < pingList.size(); i++)
      {
         cg.gameServerList->updatePingList(pingList[i].address);
      }
   }
}

void FearCSDelegate::gotPingInfo(const PingInfo &pingInfo)
{
   if (pingInfo.gameType != gPingVerifyByte)
   {
      serverPingTimeout(pingInfo.address);
   }
   else
   {
      //get the ping version, and name
      UInt16 localVersion;
      UInt16 *namePtr = (UInt16*)&pingInfo.name[0];
      localVersion = *namePtr;
      char localServerName[32];
      strncpy(localServerName, &pingInfo.name[2], 31);
      localServerName[31] = '\0';
      
      if (cg.gameServerList) cg.gameServerList->gotPingInfo(pingInfo.address, localServerName, localVersion, pingInfo.ping);
      if (localVersion == mPacketVersion)
      {
         pushGameInfoRequest(pingInfo.address, pingInfo.ping);
      }
   }
   
   //let the delegate know how many servers left to ping, and how many it already has
   SimGui::JoinGameDelegate *dlgt = (SimGui::JoinGameDelegate *) manager->findObject("SimGui::JoinGameDelegate");
   if (dlgt)
   {
      dlgt->updatePingLists(pingList.size(), gameList.size(), TRUE);
   }
}
   
void FearCSDelegate::serverPingTimeout(const char *address)
{
   if (cg.gameServerList) cg.gameServerList->serverTimeout(address);
   
   //remove it from the serverList if it exists...
   for (int i = 0; i < mServerList.size(); i++)
   {
      if (! stricmp(address, mServerList[i].transportAddress))
      {
         destroy(&mServerList[i]);
         mServerList.erase(i);
         break;
      }
   }
   
   //let the delegate know how many servers left to ping, and how many it already has
   SimGui::JoinGameDelegate *dlgt = (SimGui::JoinGameDelegate *) manager->findObject("SimGui::JoinGameDelegate");
   if (dlgt)
   {
      //see if what timed out was a master server
      if(curMaster < numMasters)
      {
         char varName[32];
         sprintf(varName, "Server::XLMasterN%d", curMaster);
         const char *XLAddress = Console->getVariable(varName);
         if (! stricmp(address, XLAddress))
         {
            curMaster++;
            bool found = false;
            while(curMaster < numMasters)
            {
               sprintf(varName, "Server::XLMasterN%d", curMaster);
               const char *addr = Console->getVariable(varName);
               if(addr[0])
               {
                  pushPingInfoRequest(addr);
                  found = true;
                  break;
               }
               curMaster++;
            }
            if(!found)
               dlgt->masterTimeout(address);
         }
      }
      if (pingList.size() > 0)
      {
         dlgt->updatePingLists(pingList.size(), gameList.size(), TRUE);
      }
      else
      {
         if (gameList.size() > 1) dlgt->updatePingLists(gameList.size() - 1, mServerList.size(), FALSE);
         else dlgt->queryFinished();
      }
   }
}
            
//this flag is actually only valid in the gameServerList's version            
bool FearCSDelegate::isNeverPing(const char *address)
{
   if (cg.gameServerList)
   {
      return cg.gameServerList->isNeverPing(address);
   }
   else return FALSE;
}

void FearCSDelegate::setNeverPing(const char *address)
{
   if ((! address) || (! address[0])) return;
   
   Net::Addr a;
   strcpy(a.address, address); // add it to the timeout list
   Net::timedoutList.push_back(a);
}


