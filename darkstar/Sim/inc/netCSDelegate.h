#ifndef _H_NETCSDELEGATE
#define _H_NETCSDELEGATE

#include "dnet.h"
#include "simBase.h"

namespace Net
{

class PacketStream;

class DLLAPI CSDelegate : public SimObject, public DNet::Session
{
   private:
      typedef SimObject Parent;

   public:
      enum { Unknown_Ping = 0xffff };

   protected:
      // THIS SHADOWS the packet struct in the master server,
      // DO NOT CHANGE THIS STRUCT w/o also changing in master server
      struct Packet { 
         enum { 
            Version = 0x10,
            BaseSize = 4*sizeof(BYTE) + 2*sizeof(WORD),  // must change GameInfo::data size below
            DataSize = DNet::MaxPacketSize - BaseSize,   // must change GameInfo::data size below
         };
      	BYTE version;      
         BYTE type;             // message type
         BYTE packet_num;       // packet number out of total; starts at 1
         BYTE packet_tot;       // total packets of info
         WORD key;              // for verification and id purposes
         WORD id;               // master server id (read from config file)
         UInt8 data[DataSize];  // total size = 2048 (from mstrsvr.h)
         Packet(BYTE _type, WORD _key, BYTE num = 0);
      };

      struct PingInfo {
         UInt8 gameType;
         UInt8 playerCount;
         UInt8 maxPlayers;
         int ping;           
         char name[32];
         char address[256];
         PingInfo() { ping = Unknown_Ping; }
      };
      
      struct GameInfo {
         int ping;   // valid only if carried from PingEntry to GameEntry
         WORD key;   // for determining if the response contains additional information
         char address[256];
         int dataSize;
         UInt8 data[DNet::MaxPacketSize - (4*sizeof(BYTE) + 2*sizeof(WORD))]; // ms fix - must be same as Packet::DataSize above
            //UInt8 data[Packet::DataSize];   
         GameInfo() { address[0] = '\0'; dataSize = 0; ping = Unknown_Ping; }
      };

      struct PingEntry {
         enum { General, Enumerated };
         WORD key;
         int type;                 // gen or enum
         int no;                   // for mstr srvr, packet no of n (e.g. 7 of 9)
         int tries;                // no of attampts to ping, so far
         UInt32 time;              // time when this entry was sent
         char address[256];
         PingEntry(const char *_address, int _no = 0);
      };
      
      struct GameEntry {
         int ping;  // valid only if carried from PingEntry
         int tries;
         UInt32 time;
         char address[256];
         GameEntry(const char *_address, int _ping = Unknown_Ping);
      };         

      struct {
         int maxConcurrentPings;
         int pingTimeoutTime;
         int pingRetryCount;
         int maxConcurrentRequests;
         int requestTimeoutTime;
         int requestRetryCount;
      } prefs;

      struct {
         int pings;
         int requests;
      } open;

      bool isServer;
      bool sendingHeartbeat;
      SimIdGenerator managerIdGenerator;
      WORD queryKey;
      UInt32 lastReceivedTick;
         
      Vector<PingEntry> pingList;
      Vector<GameEntry> gameList;

      PingEntry *findPingEntry(const char *address, int no);
      GameEntry *findGameEntry(const char *address);
      void processLists();
      bool onAdd();

   public:
 
      enum GameInfoPacketType
      {
         PingInfoQuery          = 0x03,    // aka GAMESPY_QUERY in mstrsvr.h
         PingInfoResponse       = 0x04,    //  "  GAMESVR_REPLY  "  "
         MasterServerHeartbeat  = 0x05,    //  "  HEARTBT_MESSAGE 
         MasterServerList       = 0x06,    //  "  MSTRSVR_REPLY
   		GameInfoQuery          = 0x07,    //  "  GAMESVR_VERIFY 
   		GameInfoResponse       = 0x08,
      };

      CSDelegate();
      ~CSDelegate();
   
      void onReceive(DNet::VC *connection, const char *transportAddress, BYTE *data, int datalen);
      void onNotify(DNet::VC *connection, DWORD notifyKey, bool received);
      void onConnection(DNet::VC *connection, ConnectionNotifyType con, BYTE *data, int datalen);
      virtual void onGhostAlwaysDone(PacketStream *pstream);

      void freeServerList();
      int getQueryKey() { return queryKey; }

      bool processEvent(const SimEvent *evt);
      bool processArguments(int argc, const char **argv);
      bool processQuery(SimQuery *query);
      DNet::Transport *activate(const char *transportName, int port);

      void sendHeartbeat();

      void acceptConnection(DNet::VC *vc);
      virtual void onConnectionAccepted(DNet::VC *vc, BYTE *data, int datalen);
      virtual void onConnectionError(PacketStream *stream, const char *errorString);
        // subclasses must not modify the pack for onConnectionAccepted

      virtual void addStreamClients(PacketStream *ps);
      
      virtual bool playDemo(const char *fileName);
      virtual bool timeDemo(const char *fileName);
      virtual void simDisconnect();
      virtual bool simConnect(const char *transportAddress);
      virtual void onConnectionRequested(DNet::VC *vc, BYTE *data, int datalen);

      virtual void onConnectionRejected(DNet::VC *vc, BYTE *data, int datalen);
      virtual void onConnectionDropped(DNet::VC *vc, BYTE *data, int datalen, bool timeout);

      virtual void onGameInfoPacket(const char *transportAddress, BYTE *data, int datalen);
      virtual void kick(int playerId, const char *reason);

      void pushPingInfoRequest(const char *address, int no = 0);
      void pushGameInfoRequest(const char *address, int ping = 0);
		void clearPingRequestList(void);
		void clearGameRequestList(void);
      const char *getConnectionString(SimObjectId packetStreamId);
      UInt32 getLastReceivedTick() { return lastReceivedTick; }
      
      virtual void getPingInfo(PingInfo &pingInfo);
      virtual int getGameInfo(GameInfo &gameInfo, int maxDataSize, int dataLeftToRead);
      virtual void gotPingInfo(const PingInfo &pingInfo);
      virtual void gotGameInfo(const GameInfo &gameInfo);
      virtual void gotMasterServer(const char *address, const char *name, const char *motd, bool firstPacket);
      virtual void serverPingTimeout(const char *address);

      DECLARE_PERSISTENT(CSDelegate);
};

//------------------------------------------------------------------------------

inline CSDelegate::GameEntry::GameEntry(const char *_address, int _ping)
{
   ping = _ping;
   tries = 0;
   time = 0;
   strcpy(address, _address);
}

inline CSDelegate::PingEntry::PingEntry(const char *_address, int _no)
{
   if (_no > 0) {
      // we're already expecting the packet from the master server
      type = Enumerated;
      tries = 1;  
   }
   else {
      type = General;
      tries = 0;
   }      
   no = _no;
   time = 0;
   strcpy(address, _address);
}

//------------------------------------------------------------------------------

inline CSDelegate::Packet::Packet(BYTE _type, WORD _key, BYTE num)
{
   version = Version;
   type = _type;
   packet_num = (num == 0) ? 0xff : num;  // 0xff means request all from mstr svr
   packet_tot = 0;
   key = _key;
   id = 0;
   data[0] = '\0';
}   

//------------------------------------------------------------------------------

inline bool CSDelegate::processQuery(SimQuery *query)
{
   query;
   return false;
}

//------------------------------------------------------------------------------

inline void CSDelegate::kick(int playerId, const char *reason)
{
   playerId;
   reason;
}

};

#endif

      

