#include "simBase.h"
#include "simEv.h"
#include "netEventManager.h"
#include "BitStream.h"
#include "netGhostManager.h"

#define DebugChecksum FOURCC('D','b','U','g')

bool SimEvent::verifyNotServer(SimManager *mgr)
{
   if(mgr->getId() == 2048)
   {
      // uh-oh.
      Net::setLastError("Invalid packet.");
      return false;
   }
   return true;
}

namespace Net
{

void EventManager::onDeleteNotify(SimObject *object)
{
   if(object == polledSet)
      polledSet = NULL;
   Parent::onDeleteNotify(object);
}

void EventManager::onRemove()
{
   if(polledSet)
   {
      polledSet->deleteObject();
      polledSet = NULL;
   }
   Parent::onRemove();
}

EventManager::EventLink *EventManager::allocEventLink()
{
   if(freeList)
   {
      EventLink *ret = freeList;
      freeList = freeList->nextEvent;
      ret->evt = NULL;
      return ret;
   }
   else
   {
      EventLink *ret = new EventLink;
      ret->evt = NULL;
      return ret;
   }
}

void EventManager::freeEventLink(EventManager::EventLink *freed)
{
   if (freed->evt)
      delete freed->evt;
   freed->evt = NULL;
   freed->nextEvent = freeList;
   freeList = freed;
}

EventManager::EventManager()
{
   freeList = 0;
   sendQueueHead = NULL;
   sendQueueTail = NULL;
   waitSeqEvents = NULL;

   lastAckedSeq = 0;
   nextSendSeq = 0;
   nextRecvSeq = 0;
   highSentSeq = 0;
   ackMask[0] = 0;
   ackMask[1] = 0;
   ackMask[2] = 0;
   ackMask[3] = 0;


   polledSet = NULL;
}

EventManager::~EventManager()
{
   EventLink *temp;

   while(sendQueueHead)
   {
      temp = sendQueueHead->nextEvent;
      freeEventLink(sendQueueHead);
      sendQueueHead = temp;
   }
   while(waitSeqEvents)
   {
      temp = waitSeqEvents->nextEvent;
      freeEventLink(waitSeqEvents);
      waitSeqEvents = temp;
   }

   while(freeList)
   {
      temp = freeList->nextEvent;
      delete freeList;
      freeList = temp;
   }
}

void EventManager::addPolledObject(SimObject *obj)
{
   if(!polledSet)
   {
      polledSet = new SimSet(false);
      manager->addObject(polledSet);
      deleteNotify(polledSet);
   }
   polledSet->addObject(obj);
}

void EventManager::postRemoteEvent(const SimEvent *theEvent)
{
   if(owner->getStreamMode() == PacketStream::PlaybackMode)
   {
      delete theEvent;
      return;
   }

   EventLink *ev = allocEventLink();

   ev->evt = (SimEvent *) theEvent;
   if(theEvent->flags.test(SimEvent::Guaranteed))
   {
      ev->guaranteed = true;
      if(theEvent->flags.test(SimEvent::Ordered))
         ev->seqCount = nextSendSeq++;
      else
         ev->seqCount = -2;
   }
   else
   {
      ev->guaranteed = false;
   }
   ev->nextEvent = NULL;
   if(!sendQueueHead)
      sendQueueHead = sendQueueTail = ev;
   else
   {
      sendQueueTail->nextEvent = ev;
      sendQueueTail = ev;
   }
}

bool EventManager::writePacket(BitStream *bstream, DWORD &key)
{
   if(polledSet)
   {
      SimPolledUpdateEvent ev;
      ev.evManager = this;
      SimSet::iterator i;
      for(i = polledSet->begin(); i != polledSet->end(); i++)
         (*i)->processEvent(&ev);
   }

   if(!sendQueueHead)
      return false;

   EventLink *packQueueHead = NULL, *packQueueTail = NULL;
   int prevSeq = -4;

   while(sendQueueHead)
   {
		if(bstream->isFull())
			break;

      int idx = (lastAckedSeq >> 5) & 0x3;

      while(!(ackMask[idx] & (1 << (lastAckedSeq & 0x1F))) &&
         (lastAckedSeq < highSentSeq))
      {
         lastAckedSeq++;
         idx = (lastAckedSeq >> 5) & 0x3;
      }

      if(sendQueueHead->guaranteed && sendQueueHead->seqCount > lastAckedSeq + 126)
         break;

      EventLink *ev = sendQueueHead;
      sendQueueHead = sendQueueHead->nextEvent;

      // there is an event, so write a bit sayin so
      bstream->writeFlag(true);

      // write out guaranteeing / ordering data

      int startPos = bstream->getCurPos();

      if(ev->guaranteed)
      {
         bstream->writeFlag(true);
         ev->nextEvent = NULL;

         if(!packQueueHead)
            packQueueHead = packQueueTail = ev;
         else
         {
            packQueueTail->nextEvent = ev;
            packQueueTail = ev;
         }
         if(bstream->writeFlag(ev->seqCount == prevSeq + 1))
            prevSeq++;
         else
         {
            if(ev->seqCount == -2)
               bstream->writeFlag(false);
            else
            {
               bstream->writeFlag(true);
               bstream->writeInt(ev->seqCount,7);
            }
            prevSeq = ev->seqCount;
         }
         if(prevSeq != -2)
            ackMask[(prevSeq >> 5) & 0x3] |= 1 << (prevSeq & 0x1F);

         if(prevSeq > highSentSeq)
            highSentSeq = prevSeq;
      }
      else
         bstream->writeFlag(false);

      AssertFatal(ev->evt->type >= 1024 && ev->evt->type < 1152, "event type must be in 10-bit range for remote events");
      bstream->writeInt(ev->evt->type - 1024, 7);

      ev->evt->pack(manager, owner, bstream);

      PacketStream::getStats()->addBits(PacketStats::Send, bstream->getCurPos() - startPos, ev->evt->getPersistTag());
#ifdef DEBUG_NET
      // write out a checksum...
      // we'll check this on the other side for validation
      // later we can put in stuff to check sizes and such

      bstream->writeInt(ev->evt->type ^ DebugChecksum, 32);
#endif

      if(!ev->guaranteed)
         freeEventLink(ev);
   }

   bstream->writeFlag(false);

   if(packQueueHead)
      key = (DWORD) packQueueHead;
   else
      key = 0;

   return true;
}

bool EventManager::sendEvent(SimEvent *event)
{
   SimObject *obj = event->address.resolve(manager);
   if(obj && !obj->isDeleted())
      obj->processEvent(event);
   delete event;
   return getLastError()[0] == 0;
}

void EventManager::readPacket(BitStream *bstream, DWORD)
{
   int prevSeq = -4;
   while(bstream->readFlag())
   {
      int startPos = bstream->getCurPos();

      bool guaranteed = bstream->readFlag();
      int seq = -2;

      if(guaranteed)
      {
         if(bstream->readFlag())
            seq = (prevSeq + 1) & 0x7F;
         else
         {
            if(bstream->readFlag())
               seq = bstream->readInt(7);
            else
               seq = -2;
         }
         prevSeq = seq;
      }
      int classTag = bstream->readInt(7) + 1024;
      SimEvent *evt = (SimEvent *) Persistent::create(classTag);
      if(!evt)
      {
         setLastError("Invalid packet.");
         return;
      }
      evt->type = classTag;
      evt->sourceManagerId = owner->getId();

      // event unpack sets the address field for addressing
      evt->unpack(manager, owner, bstream);
      if(getLastError()[0])
         return;

      PacketStream::getStats()->addBits(PacketStats::Receive, bstream->getCurPos() - startPos, evt->getPersistTag());
#ifdef DEBUG_NET
      int checksum = bstream->readInt(32);
      AssertFatal( (checksum ^ DebugChecksum) == (unsigned int)classTag,
         avar("unpack did not match pack for event of class %s.",
            evt->getClassName()) );
#endif
      // special event cases...
      // default behavior for using the ghost manager to deliver
      // events
      evt->flags.set(SimEvent::Remote);

      if(evt->flags.test(SimEvent::ToGhost) || evt->flags.test(SimEvent::ToGhostParent))
      {
         SimObject *destObject;
         if(evt->flags.test(SimEvent::ToGhost))
            destObject = owner->getGhostManager()->resolveGhost(evt->address.objectId);
         else
            destObject = owner->getGhostManager()->resolveGhostParent(evt->address.objectId);

         // only deliver to objects that exist...
         // if the object isn't there, tag it with bogus id

         if(destObject)
            evt->address.set(destObject);
         else
            evt->address.set(-2, 0);
      }

      if(seq != -2)
      {
         if(seq != nextRecvSeq)
         {
            EventLink *ev = allocEventLink();
            ev->seqCount = seq;
            ev->evt = evt;
            ev->nextEvent = waitSeqEvents;
            waitSeqEvents = ev;
         }
         else
         {
            if(!sendEvent(evt))
               return;

            nextRecvSeq = (nextRecvSeq + 1) & 0x7F;
            EventLink **walk = &waitSeqEvents;
            while(*walk)
            {
               EventLink *ev = *walk;
               if(ev->seqCount == nextRecvSeq)
               {
                  nextRecvSeq = (nextRecvSeq + 1) & 0x7F;
                  *walk = ev->nextEvent;
                  SimEvent *evt = ev->evt;
                  ev->evt = NULL;
                  walk = &waitSeqEvents;
                  freeEventLink(ev);
                  if(!sendEvent(evt))
                     return;
               }
               else
                  walk = &(ev->nextEvent);
            }
         }
      }
      else
      {
         if(!sendEvent(evt))
            return;
      }
   }
}

void EventManager::packetDropped(DWORD key)
{
   // put these on the front of the event queue to be sent

   EventLink *evtList = (EventLink *) key;
   EventLink *walk = evtList;
   while(walk->nextEvent)
      walk = walk->nextEvent;

   walk->nextEvent = sendQueueHead;
   if(!walk->nextEvent)
      sendQueueTail = walk;

   sendQueueHead = evtList;
}

void EventManager::packetReceived(DWORD key)
{
   EventLink *evtList = (EventLink *) key;
   while(evtList)
   {
      EventLink *temp = evtList->nextEvent;
      if(evtList->guaranteed && evtList->seqCount != -2)
      {
         int idx = (evtList->seqCount >> 5) & 0x3;
         int bitMask = ~(1 << (evtList->seqCount & 0x1F));
         ackMask[idx] &= bitMask;
      }
      freeEventLink(evtList);
      evtList = temp;
   }
}

};