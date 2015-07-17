#ifndef _H_NETPACKETSTREAM_
#define _H_NETPACKETSTREAM_

#include "dnet.h"
#include "simBase.h"
#include "filstrm.h"
#include "simtimergrp.h"

class BitStream;
struct SimAddNotifyEvent;

namespace Net {

// PacketStream clients register keys for every packet they send out
// when the packet is either delivered or not delivered, the client is
// notified with the key that the client registered for that packet.

class EventManager;
class GhostManager;
class PacketStreamClient;

class PacketStats
{
public:
   enum { Send = 0, Receive = 1 };
   int bitAccumulator[2][2048];
   int totalSent;
   int totalRecv;

   PacketStats();

   void addBits(int mode, int bitcount, int type)
   {
      bitAccumulator[mode][type] += bitcount;
   }
};

class PacketStream : public SimGroup
{
   typedef SimGroup Parent;

   static PacketStats pstats;

   DNet::VC *connection;
   DWORD lastUpdate;
   char fileName[256];
   FileWStream demoWriteFile;
   FileRStream demoReadFile;
   float demoStartTime;
   DWORD timeOffset;
   DWORD nextEventTime;
   DWORD packetSeq;
   DWORD lastPacketRecvd;

   struct NetRate
   {
      DWORD updateDelay;
      int packetSize;
      bool changed;
   };

   NetRate curRate;
   NetRate maxRate;

   struct PacketNotify
   {
      PacketStreamClient *client;
      DWORD notifyKey;
      PacketNotify *nextNotify;
   };
   struct PacketNotifyHead
   {
      PacketNotify *noteList;
      bool rateChanged;
      bool maxRateChanged;
      DWORD seq;
      PacketNotifyHead *nextPacket;
   };

   PacketNotify *freeNotifyList;
   PacketNotifyHead *freeHeadList;

   PacketNotifyHead *noteList;

   GhostManager *ghostManager;
   EventManager *eventManager;

   PacketNotifyHead *allocNotifyHead();
   void freeNotifyHead(PacketNotifyHead *);

   PacketNotify *allocNotify();
   void freeNotify(PacketNotify *);

   void ClearNotifies(PacketStreamClient *client);
public:
   static bool addToTimerSet;
   static DWORD currentTime;

   enum Mode
   {
      NormalMode,  // standard client mode
      ReplyMode,   // standard server mode (reply to client packets)
      RecordMode,  // client+recording
      PlaybackMode,// client only playback recording
      ServerMode,  // special tribes mode
   };
private:
   Mode streamMode;
   bool needsReply;
public:
   StreamIO *getRecorderStream(PacketStreamClient *cl, DWORD time = 0xFFFFFFFF);
   void stepReplay();

   static PacketStats *getStats() { return &pstats; }

   GhostManager *getGhostManager() { return ghostManager; }
   EventManager *getEventManager() { return eventManager; }
   DNet::VC *getVC() { return connection; }

   PacketStream(DNet::VC *vc, Mode mode, const char *recordFileName);
   ~PacketStream();
	
	SimObject* addObject(SimObject*);

   void connectionDied();
   void disconnect();

   void processRecorder(DWORD time);
   void checkMaxRate();
   DWORD getAverageRTT();
   DWORD getUpdateDelay() {return curRate.updateDelay;}
   void checkPacketSend();
   void sendRemoteEvent(const SimEvent *event, bool deliverThroughGhostManager=false);
   void handlePacket(BYTE *data, int datalen);
   void handleNotify(DWORD notifyKey, bool recvd);

   bool onSimTimerEvent(const SimTimerEvent *event);
   bool onAdd();
   void onRemove();
   void onDeleteNotify(SimObject *object);
   void endPlayback();
   void endRecording();
   
   bool processEvent(const SimEvent *event);
   Mode getStreamMode() { return streamMode; }
   const char *getRecordingFileName() { return &(fileName[0]); }
};

class PacketStreamClient : public SimObject
{
   friend class PacketStream;

   typedef SimObject Parent;
   
protected:
   PacketStream *owner;
public:
   PacketStream *getOwner() { return owner; }

   virtual void packetDropped(DWORD key) = 0;
   virtual void packetReceived(DWORD key) = 0;
   virtual bool writePacket(BitStream *bstream, DWORD &key) = 0;
   virtual void readPacket(BitStream *bstream, DWORD time) = 0;
   virtual void processRecorderStream(StreamIO *sio, DWORD time);
   virtual void notifyPlaybackOver() {};
   virtual void notifyRecordOver() {};
   bool processQuery(SimQuery *query);
   bool processEvent(const SimEvent *evt);
};

};
#endif