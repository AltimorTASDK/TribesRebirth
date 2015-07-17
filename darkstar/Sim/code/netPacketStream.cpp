#include "simBase.h"
#include "netPacketStream.h"
#include "simEv.h"
#include "netGhostManager.h"
#include "netEventManager.h"
#include "console.h"
#include "simGame.h"
#include "netcsdelegate.h"
namespace Net
{

PacketStats::PacketStats()
{
   int i;
   for(i = 0; i < 2048; i++)
      bitAccumulator[0][i] = bitAccumulator[1][i] = 0;
   totalSent = 0;
   totalRecv = 0;
}

PacketStats PacketStream::pstats;

bool PacketStream::addToTimerSet = true;
DWORD PacketStream::currentTime = 0;
DWORD gPacketSendTime = 32;

PacketStream::PacketStream(DNet::VC *vc, Mode mode, const char *recordFileName)
{
   if(recordFileName)
      strcpy(fileName, recordFileName);

   connection = vc;
   ghostManager = NULL;
   eventManager = NULL;
   lastUpdate = 1;

   freeNotifyList = NULL;
   freeHeadList = NULL;
   noteList = NULL;
   streamMode = mode;
   needsReply = false;

   packetSeq = 1;
   lastPacketRecvd = 0;
}

PacketStream::~PacketStream()
{
   if (noteList)
   {
      PacketNotify *walk = noteList->noteList;
      while(walk)
      {
         PacketNotify *tmp = walk->nextNotify;
         delete walk;
         walk = tmp;
      }
      delete noteList;
   }

   while(freeNotifyList)
   {
      PacketNotify *tmp = freeNotifyList->nextNotify;
      delete freeNotifyList;
      freeNotifyList = tmp;
   }
   while(freeHeadList)
   {
      PacketNotifyHead *tmp = freeHeadList->nextPacket;
      delete freeHeadList;
      freeHeadList = tmp;
   }
}

void PacketStream::connectionDied()
{
   connection = NULL;
}

SimObject *PacketStream::addObject(SimObject *obj)
{
   if(!ghostManager)
      ghostManager = dynamic_cast<GhostManager *>(obj);
   if(!eventManager)
      eventManager = dynamic_cast<EventManager *>(obj);
   AssertFatal(dynamic_cast<PacketStreamClient *>(obj) != NULL,
      "Tried to add a non-client to a packet stream.");
   ((PacketStreamClient *) obj)->owner = this;

   return Parent::addObject(obj);
}

PacketStream::PacketNotifyHead *PacketStream::allocNotifyHead()
{
   if(freeHeadList)
   {
      PacketNotifyHead *ret = freeHeadList;
      freeHeadList = freeHeadList->nextPacket;
      return ret;
   }
   else
      return new PacketNotifyHead;
}

void PacketStream::freeNotifyHead(PacketNotifyHead *head)
{
   head->nextPacket = freeHeadList;
   freeHeadList = head;
}

PacketStream::PacketNotify *PacketStream::allocNotify()
{
   if(freeNotifyList)
   {
      PacketNotify *ret = freeNotifyList;
      freeNotifyList = freeNotifyList->nextNotify;
      return ret;
   }
   else
      return new PacketNotify;
}

void PacketStream::freeNotify(PacketNotify *note)
{
   note->nextNotify = freeNotifyList;
   freeNotifyList = note;
}

void PacketStream::ClearNotifies(PacketStreamClient *client)
{
   // go through all the clients and just get rid of the notify
   PacketNotifyHead *walk = noteList;
   while(walk)
   {
      PacketNotify **nwalk = &walk->noteList;
      while(*nwalk)
      {
         PacketNotify *temp = *nwalk;
         if(temp->client == client)
         {
            *nwalk = temp->nextNotify;
            freeNotify(temp);
         }
         else
            nwalk = &(temp->nextNotify);
      }
      walk = walk->nextPacket;
   }
}

void PacketStream::handleNotify(DWORD notifyKey, bool recvd)
{
   PacketNotifyHead *noteHead = (PacketNotifyHead *) notifyKey;

   if(noteHead->rateChanged && !recvd)
      curRate.changed = true;
   if(noteHead->maxRateChanged && !recvd)
      maxRate.changed = true;

   if(recvd)
      lastPacketRecvd = noteHead->seq;
   PacketNotify *note = noteHead->noteList;
   while(note)
   {
      if(recvd)
         note->client->packetReceived(note->notifyKey);
      else
         note->client->packetDropped(note->notifyKey);

      PacketNotify *temp = note;
      note = note->nextNotify;
      freeNotify(temp);
   }

   PacketNotifyHead **walk = &noteList;
   while(*walk)
   {
      if(noteHead == *walk)
      {
         *walk = noteHead->nextPacket;
         break;
      }
      else
         walk = &((*walk)->nextPacket);
   }
   freeNotifyHead(noteHead);
}

void PacketStream::handlePacket(BYTE *data, int len)
{
   BitStream bstream(data, len);
   if(streamMode == RecordMode)
   {
      if(!addToTimerSet)
         demoWriteFile.write(currentTime - timeOffset);
      demoWriteFile.write(BYTE(0xFF));
      demoWriteFile.write(UInt16(len));
      demoWriteFile.write(len, data);
   }

   PacketStream::getStats()->totalRecv++;

   if(streamMode == ReplyMode) // force the update on the next one
      needsReply = true;

   if(bstream.readFlag())
   {
      curRate.updateDelay = bstream.readInt(10);
      curRate.packetSize = bstream.readInt(10);
   }
   if(bstream.readFlag())
   {
      DWORD omaxDelay = bstream.readInt(10);
      int omaxSize = bstream.readInt(10);
      if(omaxDelay < maxRate.updateDelay)
         omaxDelay = maxRate.updateDelay;
      if(omaxSize > maxRate.packetSize)
         omaxSize = maxRate.packetSize;
      if(omaxDelay != curRate.updateDelay || omaxSize != curRate.packetSize)
      {
         curRate.updateDelay = omaxDelay;
         curRate.packetSize = omaxSize;
         curRate.changed = true;
      }
   }
   iterator i;

   for(i = begin(); i != end(); i++)
   {
      PacketStreamClient *client = (PacketStreamClient *) (*i);
      setLastError("");
      if(bstream.readFlag())
         client->readPacket(&bstream, currentTime);
      const char *error = getLastError();
      if(!error[0] && !bstream.isValid())
      {
         setLastError("Invalid packet.");
         error = getLastError();
      }
      if(error[0])
      {
         CSDelegate *delegate = (CSDelegate *) manager->findObject(SimCSDelegateId);
         if(delegate)
            delegate->onConnectionError(this, error);
         return;
      }
   }
}

void PacketStream::checkMaxRate()
{
   CMDConsole *con = CMDConsole::getLocked();

   DWORD pst = con->getIntVariable("pref::PacketFrame", 32);
   if(pst < 32)
      pst = 32;
   if(pst > 128)
      pst = 128;

   gPacketSendTime = pst;

   DWORD rate = con->getIntVariable("pref::PacketRate", 10);
   int size = con->getIntVariable("pref::PacketSize", 200);

   if(rate > 30)
      rate = 30;
   else if(rate < 1)
      rate = 1;
   if(size > 450)
      size = 450;
   else if(size < 100)
      size = 100;

   rate = 1000 / rate;
   if(maxRate.updateDelay != rate || maxRate.packetSize != size)
   {
      maxRate.updateDelay = rate;
      maxRate.packetSize = size;
      maxRate.changed = true;
   }
}

void PacketStream::checkPacketSend()
{
   DWORD curTime;
   curTime = GetTickCount();

   DWORD modUpdDelay = curRate.updateDelay;

   if(!connection) // never send packets
      return;
   if(connection->getState() != DNet::VC::Connected || connection->windowFull())
      return;

   if(streamMode == ReplyMode) // time and a half if we're in reply mode
      modUpdDelay += (modUpdDelay >> 1);

   if(currentTime != 0 && streamMode != ServerMode)
   {
      // we're in tribes land now
      if(gPacketSendTime < 32)
         gPacketSendTime = 32;
      if(currentTime < lastUpdate + gPacketSendTime)
         return;
      DWORD maxSeq = 1000 / gPacketSendTime;
      if(maxSeq < packetSeq - lastPacketRecvd)
         return;
   }
   else
   {
      DWORD maxSeq = 1000 / modUpdDelay;
      if(maxSeq < packetSeq - lastPacketRecvd)
         return;
      if(!needsReply && (curTime - lastUpdate < modUpdDelay))
         return;
   }    
   needsReply = false;

   iterator i;
   bool nUsed = false;

   BitStream nStream(connection->getDataBuffer(), curRate.packetSize, 1500);

   PacketNotifyHead *nHead = allocNotifyHead();

   nHead->seq = packetSeq++;
   nHead->rateChanged = curRate.changed;
   nHead->maxRateChanged = maxRate.changed;
   nHead->noteList = NULL;

   nStream.writeFlag(curRate.changed);

   if(curRate.changed)
   {
      nStream.writeInt(curRate.updateDelay, 10);
      nStream.writeInt(curRate.packetSize, 10);
      curRate.changed = 0;
      nUsed = true;
   }
   nStream.writeFlag(maxRate.changed);
   if(maxRate.changed)
   {
      nStream.writeInt(maxRate.updateDelay, 10);
      nStream.writeInt(maxRate.packetSize, 10);
      maxRate.changed = 0;
      nUsed = true;
   }
   for(i = begin(); i != end(); i++)
   {
      AssertFatal(dynamic_cast<PacketStreamClient *>(*i), "Invalid packet stream client.");
      PacketStreamClient *cl = (PacketStreamClient *)(*i);

      DWORD key = 0;

      int cp = nStream.getCurPos();
      nStream.writeFlag(true);
      if(!cl->writePacket(&nStream, key))
         nStream.setBit(cp, false);
      else
      {
         nUsed = true;
         if(key)
         {
            PacketNotify *note = allocNotify();
            note->notifyKey = key;
            note->client = cl;
            note->nextNotify = nHead->noteList;
            nHead->noteList = note;
         }
      }
   }
   if(nUsed)
   {
      PacketStream::getStats()->totalSent++;
      connection->send(connection->getDataBuffer(), 
         (nStream.getCurPos() + 7) >> 3, (DWORD) nHead);

      nHead->nextPacket = noteList;
      noteList = nHead;
   }
   else
      freeNotifyHead(nHead);
   if(nUsed)
   {
      if(currentTime)
         lastUpdate = currentTime;
      else
         lastUpdate = curTime;
   }
}

void PacketStream::disconnect()
{
   if(connection)
   {
      connection->userData = 0;
      connection->disconnect(NULL, 0);
      connection = NULL;
   }
}

void PacketStream::endRecording()
{
   if(streamMode == RecordMode) {
      // before write the footer, let the packet stream clients
      // finish their thoughts
      SimGame::isRecording = false;
      for (iterator i = begin(); i != end(); i++)
         static_cast<PacketStreamClient *>(*i)->notifyRecordOver();
      if(addToTimerSet)
      {
         demoWriteFile.write(BYTE(0xFF));
         demoWriteFile.write(UInt16(0xFFFF));
      }
      else
         demoWriteFile.write(DWORD(0xFFFFFFFF));
      demoWriteFile.close();
      streamMode = NormalMode;
   }
}

void PacketStream::processRecorder(DWORD time)
{
   if(streamMode == PlaybackMode)
   {
      for(;;)
      {
         if (demoReadFile.getStatus() != STRM_OK) {
            // unexpected end of stream
            endPlayback();
            return;
         }
         if(nextEventTime == 0xFFFFFFFF)
         {
            endPlayback();
            return;
         }
         else if(time - timeOffset >= nextEventTime)
         {
            BYTE destIdx;
            BYTE buf[1500];
      
            demoReadFile.read(&destIdx);

            if (demoReadFile.getStatus() != STRM_OK) {
               // unexpected end of stream
               endPlayback();
               return;
            }
            if(destIdx == 0xFF)
            {
               UInt16 len;
               demoReadFile.read(&len);
               demoReadFile.read(len, buf);
               currentTime = timeOffset + nextEventTime;
               handlePacket(buf, len);
            }
            else
            {
               PacketStreamClient *cl = (PacketStreamClient *) objectList[destIdx];
               cl->processRecorderStream(&demoReadFile, nextEventTime + timeOffset);
            }
            nextEventTime = 0xFFFFFFFF;
            demoReadFile.read(&nextEventTime);
         }
         else
            return;
      }
   }  
}

void PacketStream::onRemove()
{
   endRecording();
   if(connection)
      connection->disconnect(NULL, 0);
   Parent::onRemove();
}

void PacketStream::onDeleteNotify(SimObject *obj)
{
   ClearNotifies((PacketStreamClient *) obj);
   Parent::onDeleteNotify(obj);
}

StreamIO *PacketStream::getRecorderStream(PacketStreamClient *cl, DWORD time)
{
   iterator i;
   int count = 0;

   for(i = begin(); i != end(); i++, count++)
   {
      if(*i == cl)
         break;
   }
   if(!addToTimerSet)
      demoWriteFile.write(time - timeOffset);

   demoWriteFile.write(BYTE(count));
   return &demoWriteFile;
}

bool PacketStream::onSimTimerEvent(const SimTimerEvent *)
{
   if(streamMode == RecordMode)
      demoWriteFile.write(BYTE(0xFE));
   else if(streamMode == PlaybackMode)
   {
      BYTE destIdx;
      BYTE buf[1500];
      
      while(!isDeleted())
      {
         if (demoReadFile.getStatus() != STRM_OK) {
            // unexpected end of stream
            endPlayback();
            return true;
         }
            
         demoReadFile.read(&destIdx);

         if (demoReadFile.getStatus() != STRM_OK) {
            // unexpected end of stream
            endPlayback();
            return true;
         }
 
         if(destIdx == 0xFE)
            break;
         if(destIdx == 0xFF)
         {
            UInt16 len;
            demoReadFile.read(&len);
            if(len == 0xFFFF)
            {
               endPlayback();
               return true;
            }
            else
            {
               // it's a packet for me, so I'll just read it in and handle it.
               demoReadFile.read(len, buf);
               handlePacket(buf, len);
            }
         }
         else
         {
            PacketStreamClient *cl = (PacketStreamClient *) objectList[destIdx];
            cl->processRecorderStream(&demoReadFile, 0xFFFFFFFF);
         }
      }      
   }
   return true;
}

void PacketStream::endPlayback()
{
   // this function allows other objects to end the playback of the recording
   // prematurely
   if (getStreamMode() == PlaybackMode) {
      SimGame::isPlayback = false;
      // clients need to be able to distinguish betw their deletion due to
      // generic end of sim as opposed to playback finishing
      for (iterator i = begin(); i != end(); i++)
         static_cast<PacketStreamClient *>(*i)->notifyPlaybackOver();
      deleteObject();
   }      
}

DWORD PacketStream::getAverageRTT()
{
   if(connection)
      return connection->getAverageRTT();
   else
      return 50;
}

bool PacketStream::onAdd()
{
   addToSet(PacketStreamSetId);
   curRate.updateDelay = 100;
   curRate.packetSize = 200;
   curRate.changed = false;
   maxRate.updateDelay = 100;
   maxRate.packetSize = 200;
   maxRate.changed = false;

   checkMaxRate();

   if(streamMode == RecordMode)
   {
      SimGame::isRecording = true;
      if(addToTimerSet)
         addToSet(SimTimerSetId);
      demoWriteFile.open(fileName);
      demoWriteFile.write(manager->getId());
      demoWriteFile.write(id);
      timeOffset = currentTime & ~0x1F;
   }
   else if(streamMode == PlaybackMode)
   {
      SimGame::isPlayback = true;
      if(addToTimerSet)
         addToSet(SimTimerSetId);
      demoReadFile.open(fileName);
      // playback should halt gracefully in onSimTimerEvent() if the file
      // wasn't opened successfully
      if (demoReadFile.getStatus() == STRM_OK) {
         SimObjectId managerId, myId;
         demoReadFile.read(&managerId);
         demoReadFile.read(&myId);
         setId(myId);
         SimGroupObjectIdEvent event1( managerId );
         manager->processEvent( &event1 );
         if(!addToTimerSet)
         {
            timeOffset = (currentTime & ~0x1F);
            nextEventTime = 0xFFFFFFFF;
            demoReadFile.read(&nextEventTime);
         }
      }      
   }
   return Parent::onAdd();
}

bool PacketStream::processEvent(const SimEvent *event)
{
   if(event->address.managerId == getId())
   {
      // it's an event addressed to an object on the manager
      // across the wire.
      if(eventManager)
         eventManager->postRemoteEvent(event);
      else
         delete event;

      return true;
   }
   switch(event->type)
   {
      case SimMessageEventType:
         switch(((SimMessageEvent *) event)->message)
         {
            case 0:
               checkPacketSend();
               return true;
         }
         break;
      onEvent(SimTimerEvent);
   }
   return Parent::processEvent(event);
}

bool PacketStreamClient::processQuery(SimQuery *query)
{
   query;
   return false;
}

bool PacketStreamClient::processEvent(const SimEvent *evt)
{
   evt;
   return false;
}

void PacketStreamClient::processRecorderStream(StreamIO *, DWORD /*time*/)
{
}

};