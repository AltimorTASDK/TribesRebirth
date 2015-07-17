#ifndef _H_NETEVENTMANAGER
#define _H_NETEVENTMANAGER

#include "netPacketStream.h"

namespace Net
{


class EventManager : public PacketStreamClient
{
   typedef PacketStreamClient Parent;

   struct EventLink
   {
      bool guaranteed;
      int seqCount;  // seqCount is -1 for nonsequenced events
      SimEvent *evt;
      bool useGhostManager;
      EventLink *nextEvent;
   };

   EventLink *freeList;
   EventLink *sendQueueHead;
   EventLink *sendQueueTail;
   EventLink *waitSeqEvents;
   int nextSendSeq;
   int nextRecvSeq;
   int lastAckedSeq;
   int highSentSeq;
   DWORD ackMask[4];


   EventManager::EventLink *allocEventLink();
   void freeEventLink(EventManager::EventLink *freed);

   SimSet *polledSet;
public:
   EventManager();
   ~EventManager();

   void packetDropped(DWORD key);
   void packetReceived(DWORD key);
   bool sendEvent(SimEvent *event);
   bool writePacket(BitStream *bstream, DWORD &key);
   void readPacket(BitStream *bstream, DWORD time);
   
   void addPolledObject(SimObject *obj);
   void postRemoteEvent(const SimEvent *theEvent);

   void onRemove();
   void onDeleteNotify(SimObject *object);
};

};

class SimPolledUpdateEvent : public SimEvent
{
public:
   Net::EventManager *evManager;
   SimPolledUpdateEvent() { type = SimPolledUpdateEventType; }
};


#endif