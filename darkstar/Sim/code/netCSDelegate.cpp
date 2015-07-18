#include "netCSDelegate.h"
#include "netPacketStream.h"
#include "netGhostManager.h"
#include "netEventManager.h"
#include "console.h"
#include "link.h"

DeclareLink(netCSDelegate)

#define Heartbeat_Frequency (120)   // how often to send heartbeat, in seconds

IMPLEMENT_PERSISTENT(Net::CSDelegate);

namespace Net
{

struct Addr
{
   char address[256];
};

Vector<Addr> timedoutList;

//------------------------------------------------------------------------------

void CSDelegate::onConnection(DNet::VC *virtualConnection, DNet::Session::ConnectionNotifyType con, BYTE *data, int datalen)
{
   switch(con)
   {
      case DNet::Session::ConnectionRequested:
         onConnectionRequested(virtualConnection, data, datalen);
         break;
      case DNet::Session::ConnectionAccepted:
         onConnectionAccepted(virtualConnection, data, datalen);
         break;
      case DNet::Session::ConnectionRejected:
         onConnectionRejected(virtualConnection, data, datalen);
         break;
      case DNet::Session::TimedOut:
         onConnectionDropped(virtualConnection, data, datalen, true);
         break;
      case DNet::Session::Disconnected:
         onConnectionDropped(virtualConnection, data, datalen, false);
         break;
   }
}


//------------------------------------------------------------------------------

const char *CSDelegate::getConnectionString(SimObjectId packetStreamId)
{
   PacketStream *pStream = static_cast<PacketStream *>(manager->findObject(packetStreamId));
   if (pStream != NULL)
      for (int i = 0; i < transportList.size(); i++) {
         const Vector<DNet::VC *> &vcList = transportList[i]->getVCList();
         for (int j = 0; j < vcList.size(); j++)
            if ((PacketStream *)vcList[j]->userData == pStream)
               return vcList[j]->getAddressString();
      }
   return NULL;
}

//------------------------------------------------------------------------------

void CSDelegate::onReceive(DNet::VC *conn, const char *transportAddress, BYTE *data, int datalen)
{
   lastReceivedTick = GetTickCount();
   if(conn)
   {
      PacketStream *pStream = (PacketStream *) conn->userData;
      if(pStream)
         pStream->handlePacket(data, datalen);
   }
   else
      onGameInfoPacket(transportAddress, data, datalen);
}

//------------------------------------------------------------------------------

void CSDelegate::onNotify(DNet::VC *conn, DWORD notifyKey, bool recvd)
{
   PacketStream *pStream = (PacketStream *) conn->userData;
   if(pStream)
      pStream->handleNotify(notifyKey, recvd);
}

//------------------------------------------------------------------------------

bool CSDelegate::onAdd()
{
   prefs.maxConcurrentPings    = Console->getIntVariable("pref::maxConcurrentPings", 20);
   prefs.pingTimeoutTime       = Console->getIntVariable("pref::pingTimeoutTime", 1500);
   prefs.pingRetryCount        = Console->getIntVariable("pref::pingRetryCount", 2);
   prefs.maxConcurrentRequests = Console->getIntVariable("pref::maxConcurrentRequests", 10);
   prefs.requestTimeoutTime    = Console->getIntVariable("pref::requestTimeoutTime", 2000);
   prefs.requestRetryCount     = Console->getIntVariable("pref::requestRetryCount", 2);
   return Parent::onAdd();
}   

//------------------------------------------------------------------------------

CSDelegate::CSDelegate() : DNet::Session(GetModuleHandle(NULL))
{
   id = SimCSDelegateId;
   managerIdGenerator.addBlock(ManagerIdRange);
   queryKey = 0;
   memset(&prefs, 0, sizeof(prefs));
   open.pings = 0;
   open.requests = 0;
   lastReceivedTick = 0;
   sendingHeartbeat = false;      
}

//------------------------------------------------------------------------------

CSDelegate::~CSDelegate()
{
}

//------------------------------------------------------------------------------

void CSDelegate::onGhostAlwaysDone(PacketStream *pstream)
{
   pstream;
}

void CSDelegate::onConnectionError(PacketStream *stream, const char* /*errorString*/)
{
   // MUST at least disconnect/delete the stream...
   // if not, bad things will happen (error will leave ghost/net managers in bad state)
   stream->disconnect();
   stream->deleteObject();
}

//------------------------------------------------------------------------------

bool CSDelegate::processEvent(const SimEvent *evt)
{
   switch(evt->type)
   {
      case SimMessageEventType:
         int message;
         message = ((SimMessageEvent *)evt)->message;
         switch(message)
         {
            case 0:
               process();
               break;
            case 1:
               processLists();
               break;
            case 2:
               sendHeartbeat();
               break;
         }
         return true;
      default:
         return Parent::processEvent(evt);
   }
}

//------------------------------------------------------------------------------

bool CSDelegate::processArguments(int argc, const char **argv)
{
   CMDConsole *console = CMDConsole::getLocked();
   if(argc < 1)
      return false;

   // first argument is server flag
   isServer = (argv[0][0] == 't' || argv[0][0] == 'T' || argv[0][0] == '1');
   
   // the rest of the arguments are pairs of transport types and port #'s
   // transport names are "LOOP", "IPX", "IP", "COMn", and "TAPI"

   bool usingIP = false;   
   bool activated = false;
   for (int i = 1; i + 1 < argc; i += 2) {
      // attempt to activate the requested transport
      if(activate(argv[i], atoi(argv[i + 1])))
         activated = true;
      if (strcmp(argv[i], "IP") == 0 && activated)
         usingIP = true;

      console->dbprintf(2, "SimNET: Session (%s): transport %s on port %d: %s", isServer ? "server" : "client",
         argv[i], atoi(argv[i + 1]), activated ? "Ok" : "ERROR");
   }
   if (!activated)
      return (false);
    
   if(isServer && manager->getId() == 0)
   {
      SimGroupObjectIdEvent event1( managerIdGenerator.alloc() );
      manager->processEvent( &event1 );
   }
   if (usingIP && isServer && !console->getBoolVariable("SinglePlayer", false))
      sendHeartbeat();
   return true;
}

//------------------------------------------------------------------------------

DNet::Transport *CSDelegate::activate(const char *transportName, int port)
{
   
	// Altimor: not anymore
/*#ifdef _MSC_VER
   // microsoft has error in compiler here - need to call through ptr to member function
   DNet::Transport * (DNet::Session::*fp)(const char *, int) = DNet::Session::activate;
   DNet::Transport * transport = (this->*fp)(transportName, port);
#else*/
   DNet::Transport *transport = DNet::Session::activate(transportName, port);
//#endif
   return transport;
}

//------------------------------------------------------------------------------

bool CSDelegate::simConnect(const char *vcAddress)
{
   return CSDelegate::connect(vcAddress, NULL, 0) != NULL;
}

//------------------------------------------------------------------------------

void CSDelegate::onConnectionRequested(DNet::VC *vc, BYTE *data, int datalen)
{
   data;
   datalen;

   // some default behavior here...
   if(isServer)
      acceptConnection(vc);
   else
      vc->reject(NULL, 0);
}

//------------------------------------------------------------------------------

void CSDelegate::addStreamClients(PacketStream *ps)
{
   EventManager *em = new EventManager;
   manager->addObject(em);
   ps->addObject(em);
}

//------------------------------------------------------------------------------

void CSDelegate::simDisconnect()
{
   // see if we have a packet stream corresponding to a server

   PacketStream *pstream = (PacketStream *) manager->findObject(ServerManagerId);
   if(pstream)
   {
      pstream->disconnect();
      pstream->deleteObject();
   }
   else
   {
      CMDConsole *con = CMDConsole::getLocked();
      con->printf("disconnect: not connected.");
   }
}

//------------------------------------------------------------------------------

void CSDelegate::acceptConnection(DNet::VC *vc)
{
   PacketStream *pstream = new PacketStream(vc, PacketStream::ReplyMode, NULL);

   int id = managerIdGenerator.alloc();

   manager->addObject(pstream, id);
   
   BitStream bs(vc->getDataBuffer(), 100);
   bs.writeInt(id, 32);
   bs.writeInt(manager->getId(), 32);

   vc->accept(vc->getDataBuffer(), 8);

   addStreamClients(pstream);
   vc->userData = (DWORD) pstream;
}

//------------------------------------------------------------------------------

bool CSDelegate::playDemo(const char *fileName)
{
   PacketStream *pstream = new PacketStream(NULL, PacketStream::PlaybackMode, fileName);
   manager->addObject(pstream);
   addStreamClients(pstream);
   return true;
}

bool CSDelegate::timeDemo(const char *fileName)
{
   PacketStream *pstream = new PacketStream(NULL, PacketStream::PlaybackMode, fileName);
   manager->addObject(pstream);
   addStreamClients(pstream);
   return true;
}

//------------------------------------------------------------------------------

void CSDelegate::onConnectionAccepted(DNet::VC *vc, BYTE *data, int datalen)
{
   BitStream bs(data, datalen);
   UInt32 manId = bs.readInt(32);
   UInt32 remoteId = bs.readInt(32);

   // construct a new packet stream and stuff...

   SimGroupObjectIdEvent event1( manId );
   manager->processEvent( &event1 );
   
   CMDConsole *con = CMDConsole::getLocked();
   const char *demoFileName = con->getVariable("recorderFileName");
   PacketStream *pstream;

   if(demoFileName[0])
      pstream = new PacketStream(vc, PacketStream::RecordMode, demoFileName);
   else
      pstream = new PacketStream(vc, PacketStream::NormalMode, NULL);

   manager->addObject(pstream, remoteId);

   addStreamClients(pstream);
   vc->userData = (DWORD) pstream;
}

//------------------------------------------------------------------------------

void CSDelegate::onConnectionRejected(DNet::VC *vc, BYTE *data, int datalen)
{
   vc;
   datalen;

   CMDConsole *con = CMDConsole::getLocked();
   con->dbprintf(2, "SimNET: Connection Rejected: %s", data);
}

//------------------------------------------------------------------------------

void CSDelegate::onConnectionDropped(DNet::VC *vc, BYTE *data, int datalen, bool timeout)
{
   data; datalen; timeout;

   Net::PacketStream *pstream = (Net::PacketStream *) vc->userData;
   if(pstream)
   {
      if(isServer)
         managerIdGenerator.free(pstream->getId());

      pstream->connectionDied();
      pstream->deleteObject();
   }
}

//------------------------------------------------------------------------------

static void translateTransportAddress(const char *src, char *dest)
{
   if (src[0] == 0x6) {
   BYTE *data = (BYTE *)src;
      sprintf(dest, "IP:%d.%d.%d.%d:%d", (int)data[1], (int)data[2], (int)data[3],
         (int)data[4], (int)(((int)(data[6]) << 8) | (int)data[5]));
   }
   else
      strcpy(dest, src);
}

//------------------------------------------------------------------------------

CSDelegate::PingEntry *CSDelegate::findPingEntry(const char *address, int no)
{
   for (int i = 0; i < pingList.size(); i++)
      if (strcmp(pingList[i].address, address) == 0 && no == pingList[i].no)
         return &(pingList[i]);
   return NULL;
}

//------------------------------------------------------------------------------

CSDelegate::GameEntry *CSDelegate::findGameEntry(const char *address)
{
   for (int i = 0; i < gameList.size(); i++)
      if (strcmp(gameList[i].address, address) == 0)
         return &(gameList[i]);
   return NULL;
}

//------------------------------------------------------------------------------

static bool isBroadcastAddress(const char *address)
{
   if (strnicmp(address, "IP:", 3) == 0) {
      if (strnicmp(&(address[3]), "broadcast", 9) == 0)
         return true;
   }
   else 
      if (strnicmp(address, "IPX:", 4) == 0)
         if (strnicmp(&(address[4]), "broadcast", 9) == 0)
            return true;
   return false;
}

//------------------------------------------------------------------------------

void CSDelegate::processLists()
{
   unsigned int time = GetTickCount();

   // process the ping list
   int i = 0;
   int seenPings = 0;
   while (i < pingList.size()) {
      PingEntry &entry = pingList[i];
      bool sendPacket = false;
      if (entry.tries == 0) {
         if (open.pings < prefs.maxConcurrentPings) {
            open.pings++;                                   // ping for 1st time
            sendPacket = true;
            seenPings++;
         }
      }
      else { // we've tried to send this packet before, has it timed out?
         if (entry.time + prefs.pingTimeoutTime < time) {   // this ping timed out
            if (entry.tries > prefs.pingRetryCount) {       // no more retrys
               // The user may push another ping request in their serverPingTimeout
               // handler, and pushPingInfoRequest calls processList, resulting in
               // a possible nasty ordering problem.  So delete this entry before 
               // calling serverPingTimeout, to avoid multiple timeout reports for 
               // the same packet
               char szAddress[256];
               strcpy(szAddress, entry.address);
               Addr a;
               strcpy(a.address, entry.address); // add it to the timeout list
               timedoutList.push_back(a);
               pingList.erase(&entry);  
               open.pings--;
               Console->dbprintf(2, "SimNET: Ping to server %s timed out, no %s retries", szAddress, prefs.pingRetryCount > 0 ? "more" : "");
               serverPingTimeout(szAddress);
               continue;
            }
            else {                                          // retry ping
               Console->dbprintf(2, "SimNET: Ping to server %s timed out, retrying", entry.address);
               sendPacket = true;
               seenPings++;
            }
         } else {
            // Has not timed out...
            seenPings++;
         }
      }

      if (sendPacket) {
         // send the ping it out the door
         entry.key = queryKey++;
         send(entry.address, (UInt8 *)&Packet(PingInfoQuery, entry.key, entry.no), Packet::BaseSize);
         entry.time = GetTickCount();
         entry.tries++;
         if (isBroadcastAddress(entry.address)) {
            // don't expect broadcasts to come back, they're a one time show
            Console->dbprintf(2, "SimNET: Broadcasting %s", entry.address);
            pingList.erase(&entry);
            open.pings--;
            seenPings--;
            continue;
         }
         Console->dbprintf(2, "SimNET: Pinging server %s (try %d).", entry.address, entry.tries);
      }
      i++;
      if (seenPings == prefs.maxConcurrentPings) {
         // We won't be sending any more pings, no further need to process...
         break;
      }
   }

   // if we're done doing pings, we can start requesting info from servers
   if (open.pings == 0) {
      // process the server list
      int i = 0;
      int seenQueries = 0;
      while (i < gameList.size()) {
         GameEntry &entry = gameList[i];
         bool sendPacket = false;
         if (entry.tries == 0) {
            if (open.requests < prefs.maxConcurrentRequests) {
               open.requests++;           // request info for the 1st time
               sendPacket = true;
               seenQueries++;
            }
         }
         else { // we've tried to send this packet before, has it timed out?
            if (entry.time + prefs.requestTimeoutTime < time) {    // info timed out
               if (entry.tries > prefs.requestRetryCount) {        // no more retrys
                  Console->dbprintf(2, "SimNET: Info request to server %s timed out, no %s retries", entry.address, prefs.requestRetryCount > 0 ? "more" : "");
                  serverPingTimeout(entry.address);
                  gameList.erase(&entry);
                  open.requests--;
                  continue;
               }
               else {                                              // retry request
                  Console->dbprintf(2, "SimNET: Info request to server %s timed out, retrying", entry.address);
                  sendPacket = true;
                  seenQueries++;
               }
            } else {
               // Has not timed out...
               seenQueries++;
            }
         }
         
         if (sendPacket) {            
            // send the query it out the door
            send(entry.address, (UInt8 *)&Packet(GameInfoQuery, queryKey++), Packet::BaseSize);
            entry.time = GetTickCount();
            entry.tries++;
            Console->dbprintf(2, "SimNET: Querying server %s (try %d).", entry.address, entry.tries);
         }
         i++;
         if (seenQueries == prefs.maxConcurrentRequests) {
            // We won't be sending any more queries, no further need to process...
            break;
         }
      }
   }
   
   if (open.pings > 0 || open.requests > 0)
      SimMessageEvent::post(this, manager->getCurrentTime() + .03, 1);
}
   
//------------------------------------------------------------------------------

void CSDelegate::sendHeartbeat()
{
   //first check the pref, see if the host is public
   if (! strcmp(Console->getVariable("Server::HostPublicGame"), ""))
      Console->setVariable("Server::HostPublicGame", "TRUE");
   
   // send another heartbeat in a couple minutes:
   SimMessageEvent::post(this, manager->getCurrentTime() + Heartbeat_Frequency, 2);
   sendingHeartbeat = true;
   
   //...unless of course we're not hosting a public game...
   if (!Console->getBoolVariable("Server::HostPublicGame"))
      return;
   
   Console->executef(1, "checkMasterTranslation");
   int i = 1;
   for(;;)
   {
      char varName[32];
      sprintf(varName, "inet::Master%d", i);
      const char *address = Console->getVariable(varName);

      if(address[0] == 0)
         break;

      Packet packet(MasterServerHeartbeat, 0);
      send(address, (UInt8 *)&packet, Packet::BaseSize);
      Console->dbprintf(2, "SimNET: Sending heartbeat to master %s.", address);
      i++;
   }
}

//------------------------------------------------------------------------------

void CSDelegate::onGameInfoPacket(const char *transportAddress, BYTE *data, int datalen)
{
   Packet *packet = reinterpret_cast<Packet *>(data);

   // check if we got the game info response:
   if (packet->version != Packet::Version)
      return;

   switch (packet->type)
   {
      case PingInfoQuery: {
         // someone is asking this server to acknowledge its existence
         if (!isServer)
            return;
         PingInfo pingInfo;
         getPingInfo(pingInfo);
         Packet outPacket(PingInfoResponse, packet->key);
         outPacket.packet_tot = pingInfo.gameType;
         outPacket.id = (pingInfo.playerCount << 8) | pingInfo.maxPlayers;
         pingInfo.name[31] = '\0';
         memcpy(outPacket.data, pingInfo.name, 32);
         send(transportAddress, (UInt8 *)&outPacket, Packet::BaseSize + 32);
         Console->dbprintf(3, "SimNET: Got ping request from %s", transportAddress);
         break;
      }

      case PingInfoResponse: {
         if (isServer)
            return;
         // got a ping back from a game server
         if(!pingList.size())
            break; // if the broadcast has timed out, or anything's been cancelled... progress no farther
         PingEntry *pingEntry = findPingEntry(transportAddress, 0);
         if (pingEntry == NULL) {
            // An anonymous ping response, probably someone responding to a broadcast,
            // we could capture their data but we won't know their ping, so start over
            // if it was already timed out, ignore it.
            for (int i = 0; i < timedoutList.size(); i++)
               if (strcmp(timedoutList[i].address, transportAddress) == 0)
                  return;

            pushPingInfoRequest(transportAddress);
            
            break;
         }
            
         if (pingEntry->key != packet->key)
            // this must be from a retry that turned out unecessary
            return;
            
         // extract game info
         PingInfo pingInfo;
         pingInfo.gameType = packet->packet_tot;
         pingInfo.playerCount = (packet->id >> 8)&0xff;
         pingInfo.maxPlayers = packet->id&0xff;
         pingInfo.ping = GetTickCount() - pingEntry->time;
         packet->data[31] = '\0';
         memcpy(pingInfo.name, packet->data, 32);
         strcpy(pingInfo.address, transportAddress);
         // remove from ping list
         pingList.erase(pingEntry);
         // push this server onto the timedout list, so
         // we don't end up re querying it.
         Addr a;
         strcpy(a.address, transportAddress);
         timedoutList.push_back(a);

         // dispatch game info
         gotPingInfo(pingInfo);
         open.pings--;
         Console->dbprintf(2, "SimNET: Got ping from %s (%s, pc:%d, mp:%d)", transportAddress,
            pingInfo.name, pingInfo.playerCount, pingInfo.maxPlayers);
         break;
      }      

      case GameInfoQuery: {
         // someone is asking for info on this server, get the info, mash it into
         // a packet and ship it out
         if (!isServer)
            return;  
         Console->dbprintf(3, "SimNET: Got server info request from %s.", transportAddress);
         GameInfo gameInfo;
         int dataLeftToRead = -1;
         do
         {
            dataLeftToRead = getGameInfo(gameInfo, Net::CSDelegate::Packet::DataSize, dataLeftToRead);
            Packet response(GameInfoResponse, packet->key);
            if (gameInfo.dataSize > 0)
               memcpy(response.data, gameInfo.data, gameInfo.dataSize);
            send(transportAddress, (UInt8 *)&response, gameInfo.dataSize + Net::CSDelegate::Packet::BaseSize);
         } while (dataLeftToRead > 0);
         break;
      }
         
      case GameInfoResponse: {
         if (isServer)
            return;
         GameEntry *entry = findGameEntry(transportAddress);
         GameInfo gameInfo;
         if (entry != NULL) {
            // we received a response from a game info query we sent earlier
            gameInfo.ping = entry->ping;
            gameList.erase(entry);
            open.requests--;
         }
         strcpy(gameInfo.address, transportAddress);
         gameInfo.key = packet->key;
         gameInfo.dataSize = datalen - Net::CSDelegate::Packet::BaseSize;
         if (gameInfo.dataSize > 0)
         {
            memcpy(gameInfo.data, packet->data, gameInfo.dataSize);
         }
         gotGameInfo(gameInfo);
         Console->dbprintf(2, "SimNET: Got server data from %s", transportAddress);
         break;
      }

      case MasterServerList: {
         if (isServer)
            return;
         // look for general entry for this server
         PingEntry *entry = findPingEntry(transportAddress, 0);
         if (entry) {
            if (packet->key != entry->key)
               return;
            // remove general entry, add entries for all the packets we expect
            // from this master server (except this packet)
            pingList.erase(entry);
            open.pings--;
            for (int i = 1; i <= packet->packet_tot; i++) 
               if (i != packet->packet_num) {
                  // we are already expecting these from the master server, so
                  // they are already open
                  open.pings++;
                  PingEntry p(transportAddress, i);
                  p.key = packet->key;
                  p.time = GetTickCount();
                  p.tries = 1;
                  pingList.push_back(p);
               }
         }
         else {
            // look for this specific ping entry, remove it
            PingEntry *entry = findPingEntry(transportAddress, packet->packet_num);
            if (entry == NULL || packet->key != entry->key)
               // this must be from a retry that turned out unecessary
               return;
            pingList.erase(entry);
            open.pings--;
         }            

         int byte = 0;
         int nameLen = 0;
         char *name = "(no-name)";
         int motdLen = 0;
         char *motd = "(no-motd)";
         if (packet->packet_num == 1) {
            // capture master server name
            nameLen = packet->data[byte++];
            if (nameLen > 0) {
               name = new char [nameLen + 1];
               memcpy(name, &(packet->data[byte]), nameLen);
               name[nameLen] = '\0';
            }
            byte += nameLen;
            // capture message of the day
            motdLen = packet->data[byte++];
            if (motdLen > 0) {
               motd = new char [motdLen + 1];
               memcpy(motd, &(packet->data[byte]), motdLen);
               motd[motdLen] = '\0';
            }
            byte += motdLen;
         }
        
         int numServers = int(packet->data[byte++]) << 8;
         numServers +=  int(packet->data[byte++]);
         Console->dbprintf(3, "SimNET: MS %s, packet %d of %d, %d servers",
            transportAddress, packet->packet_num, packet->packet_tot, numServers);

         // now add ping entries for the data we just received               
         data = &(packet->data[byte]);
         char taBuf[256];
         while(numServers--)
         {
            translateTransportAddress((char*)data, taBuf);
            data += *data + 1;
            bool noPing = FALSE;
            for (int i = 0; i < timedoutList.size(); i++)
            {
               if (strcmp(timedoutList[i].address, taBuf) == 0)
               {
                  noPing = TRUE;
                  break;
               }
            }
            if (! noPing) pushPingInfoRequest(taBuf);
            Console->dbprintf(3, "SimNET: Got server %s from master.", taBuf);
         }
         
         gotMasterServer(transportAddress, name, motd, (packet->packet_num == 1));
         if (nameLen > 0) delete [] name;
         if (motdLen > 0) delete [] motd;
         
         break;
      }
   }
}

//------------------------------------------------------------------------------

void CSDelegate::pushPingInfoRequest(const char *address, int no)
{
   char newAddress[256];

   if(!translateAddress(address, newAddress))
      return;

   // if the entry already exists
   PingEntry *entry = findPingEntry(newAddress, no);
   if (entry) return;
   
   //otherwise, push the request, and process it
   pingList.push_back(PingEntry(newAddress, no));   
   processLists();
}

//------------------------------------------------------------------------------

void CSDelegate::pushGameInfoRequest(const char *address, int ping)
{
   gameList.push_back(GameEntry(address, ping));
   processLists();
}

//------------------------------------------------------------------------------

void CSDelegate::clearPingRequestList(void)
{
   pingList.clear();
   open.pings = 0;
}

//------------------------------------------------------------------------------

void CSDelegate::clearGameRequestList(void)
{
   gameList.clear();
   open.requests = 0;
}

//------------------------------------------------------------------------------

void CSDelegate::getPingInfo(PingInfo &pingInfo)
{
   // sample routine, should be overridden in sub class
   pingInfo.gameType = 0xfe;
   pingInfo.playerCount = 5;
   pingInfo.maxPlayers = 10;
   strcpy(pingInfo.name, "The Central Scrutinizer");
}

//------------------------------------------------------------------------------

void CSDelegate::gotPingInfo(const PingInfo &pingInfo)
{
   // default behavior for receiving a ping back, just turn around and
   // ask for the game info
   pushGameInfoRequest(pingInfo.address, pingInfo.ping);
}

//------------------------------------------------------------------------------

int CSDelegate::getGameInfo(GameInfo &gameInfo, int maxDataSize, int dataLeftToRead)
{
   // app sub class should fill in this data
   maxDataSize;
   dataLeftToRead;
   gameInfo.dataSize = 0; 
   return 0;
}

//------------------------------------------------------------------------------

void CSDelegate::gotGameInfo(const GameInfo &gameInfo)
{
   // app sub class should fill in this data..
   gameInfo;
}

void CSDelegate::serverPingTimeout(const char *address)
{
   address;
}

//------------------------------------------------------------------------------

void CSDelegate::gotMasterServer(const char *address, const char *name, const char *motd, bool firstPacket)
{
   address;
   if (firstPacket)
      Console->dbprintf(2, "SimNET: master server %s says \"%s\"", name, motd);
}

};