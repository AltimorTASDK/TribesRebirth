#include "vcprotocol.h"
#include "logfile.h"
#include "Console.h"

extern class LogFile log_file;

namespace DNet
{

bool gLogToConsole = false;

VCProtocol::VCProtocol(Session *session,VC *vc)
{
	mySession = session;
	myVC = vc;
   myVC->setPacketLoss(0);
   myVC->setAverageRTT(100);

   int i;
   for(i = 0; i < 32; i++)
      pnotes[i].packetType = Unused;

   lastPackAckCleared = 0;

   lastSeqRecvd = 0;
   highAckRecvd = 0;
   ackClearPt = 0;
   lastSendSeq = 0;

   timeSum = 0;
   timeSumCount = 0;
   vcRetryCount = 15;

   lastRecvTime = GetTickCount();
//   userData = 0;
   pingSendCount = 0;

   vcConnectSequence = mySession->getConnectSequence();
}

VCProtocol::~VCProtocol()
{
   flushNotifies();
}

int VCProtocol::buildAckHeader(BitStream *bstream, int guaranteeType)
{
   udpAck packetAcks[16];
   int i;
   int last = (lastSeqRecvd+1) & 0x1F;
   int start = (last+1) & 0x1F;
   int numAcks = 0;

   // build up the ack info
   for(i = start; i != last; i = (i + 1) & 0x1F)
   {
      if(acks[i])
      {
         packetAcks[numAcks].seqNum = i;
         packetAcks[numAcks].count = 0;
         for(;i != last && acks[i]; i = (i + 1) & 0x1F)
            packetAcks[numAcks].count++;
         while(packetAcks[numAcks].count > 7)
         {
            packetAcks[numAcks+1].seqNum =
               (packetAcks[numAcks].seqNum + 7) & 0x1F;
            packetAcks[numAcks+1].count =
               packetAcks[numAcks].count - 7;
            packetAcks[numAcks].count = 7;
            numAcks++;
         }
         numAcks++;
      }
      if(i == last)
         break;
   }
   for(i = 0; i < numAcks; i++)
   {
      bstream->writeInt(packetAcks[i].count, 3);
      bstream->writeInt(packetAcks[i].seqNum, 5);
   }
   bstream->writeInt(0, 3);
   bstream->writeInt(guaranteeType, 5);
   return numAcks + 1;
}

BYTE * VCProtocol::prepare_data_for_send(int *newlen, BYTE *data, int datalen,
                                         DWORD notifyKey, int packetType)
{
   if(gLogToConsole)
      Console->printf("Sent packet %d, %d.", lastSendSeq, datalen);


   BYTE *packetBuffer = myVC->getHeaderBuffer();
   lastPacketSize = datalen;

   BYTE *sendPtr;
   int sendLen;

   BitStream pstream(packetBuffer, MaxPacketSize);

   int size = 2; // header 2 bytes

   memset(packetBuffer, 0, MaxHeaderSize);
   if(packetType == RequestConnect && myVC->getState() == VC::Unbound)
      myVC->setState( VC::RequestingConnection );
   else if(packetType == AcceptConnect && myVC->getState() == VC::WaitingForAcceptReject)
      myVC->setState( VC::AcceptingConnection);

   AssertFatal(!(packetType == DataPacket && myVC->getState() != VC::Connected), "DOH!");

   if(packetType & Resend)
   {
      // resend the last packet we sent...
      // but tack on any extra acks we may have since then.
      int seq = lastSendSeq & 0x1F;

      pnotes[seq].packetType |= Resend;

      pstream.writeFlag(true);
      pstream.writeInt(vcConnectSequence, 1);
      pstream.writeInt(lastSendSeq, 9);
      pstream.writeInt(highAckRecvd, 5);

      size += buildAckHeader(&pstream, pnotes[seq].packetType | packetType);
      switch(pnotes[seq].packetType & ~Resend)
      {
         case Disconnect:
         case RequestConnect:
         case AcceptConnect:
         case RejectConnect:
            pstream.writeInt(vcConnectSequence, 32);
            size += 4;
            break;
      }
      // that's it for the header
      sendPtr = data - size;
      sendLen = datalen + size;

      memmove(sendPtr, packetBuffer, size);
   }
   else if(packetType == Ping || packetType == Ack)
   {
      pstream.writeFlag(true);
      pstream.writeInt(vcConnectSequence, 1);
      pstream.writeInt(lastSendSeq, 9);
      pstream.writeInt(highAckRecvd, 5);
      size += buildAckHeader(&pstream, packetType);

      sendPtr = packetBuffer;
      sendLen = size;
   }
   else
   {
      // we are not resending the last data packet...
      // find out which pnote we're using...

      // insta-ack facility... fake a resend on accept connection
      int ackHeaderType = packetType;
      if(packetType == AcceptConnect)
         ackHeaderType |= Resend;

      int seq = (lastSendSeq + 1) & 0x1F;
      if(seq != lastPackAckCleared)
      //if(pnotes[seq].packetType == Unused)
      {
         pstream.writeFlag(true);
         pstream.writeInt(vcConnectSequence, 1);
         pstream.writeInt(++lastSendSeq, 9);
         pstream.writeInt(highAckRecvd, 5);
         size += buildAckHeader(&pstream, ackHeaderType);

         pnotes[seq].sendTime = GetTickCount();

         pnotes[seq].packetType = packetType;
         pnotes[seq].acked = false;
         pnotes[seq].notifyKey = notifyKey;

         switch(packetType & ~Resend)
         {
            case Disconnect:
            case RequestConnect:
            case AcceptConnect:
            case RejectConnect:
               pstream.writeInt(vcConnectSequence, 32);
               size += 4;
               break;
         }
         sendPtr = data - size;
         sendLen = datalen + size;
         memmove(sendPtr, packetBuffer, size);
      }
      else
      {
         return NULL; //WindowFull;
      }
   }

	*newlen = sendLen;
   return sendPtr;
}


void VCProtocol::receive(BYTE *data, int datalen, DWORD recvTime)
{
   udpAck packetAcks[16];
   BYTE *recvData;
   int recvDataLen;

   BitStream pstream(data, MaxHeaderSize + MaxPacketSize);

   pstream.readFlag(); // get rid of the game info packet bit

   int connectSeqBit = pstream.readInt(1);
   UInt32 seqNum = pstream.readInt(9);
   UInt32 highAck = pstream.readInt(5);

   // the high bit of pack[1] determines if it was a game info packet or not

   UInt32 ackCount = 0;
   while((packetAcks[ackCount].count = pstream.readInt(3)) != 0)
   {
      packetAcks[ackCount].seqNum = pstream.readInt(5);
      ackCount++;
   }
   int pType = pstream.readInt(5);
   bool ackBack = ((pType == Ping) || (pType & Resend)) && !(pType & Ack);
   
   pType &= ~(Resend | Ack); // mask off the resend/Ack until later.
   if(pType == AcceptConnect)
      ackBack = true;

   int connectSequence;

   if(pType == RequestConnect)
   {
      connectSequence = pstream.readInt(32);
      recvData = pstream.getBytePtr();
      recvDataLen = datalen - (recvData - data);

      if(myVC->getState() != VC::Unbound && connectSequence != vcConnectSequence)
      {
         flushNotifies();
         mySession->onConnection(myVC, Session::TimedOut, NULL, 0);
         myVC->deleteVC();
         return;
      }
      else if(myVC->getState() == VC::Unbound)
         vcConnectSequence = connectSequence;
   }

   // verify packet ordering and acking and stuff
   // first check sequence bit

   if(connectSeqBit != (vcConnectSequence & 1))
   {
		log_file.log(LOG_ERROR,"VCProtocol::packet ordering incorrect, returning...");
      return;
   }

   // check if the 9-bit sequence is within 31 packets of
   // the last received sequence number.

   UInt32 rangeMin = lastSeqRecvd;
   UInt32 rangeMax = rangeMin + 30;

   seqNum |= (rangeMin & 0xFFFFFE00);

   if(!(seqNum >= rangeMin && seqNum <= rangeMax))
   {
      seqNum = (seqNum & 0x1FF) | (rangeMax & 0xFFFFFE00);
      // if this packet is out of order - discard it.
      if(!(seqNum >= rangeMin && seqNum <= rangeMax))
      {
			log_file.log(LOG_ERROR,"VCProtocol::bad seqNum, returning...");
         return;
      }
   }

   if(gLogToConsole)
   {
      if(seqNum != lastSeqRecvd + 1)
         Console->printf("Dropped %d packets", seqNum - lastSeqRecvd - 1);
      Console->printf("Recvd packet %d", datalen);
   }

   // neg ack all the packets between lastSeqRecvd and seqNum;

   UInt32 i;

   for(i = lastSeqRecvd+1; i < seqNum; i++)
   {
      acks[i & 0x1F] = false;
   }
   if((pType & ~(Resend | Ack)) != Ping) // only ack if the sent packet wasn't a ping
      acks[seqNum & 0x1F] = true;

   // clear out all the acks we've already gotten.
   for(i = (seqNum+1) & 0x1F; i != ((highAck + 1) & 0x1F); i = (i + 1) & 0x1F)
      acks[i] = false;

   // check the window to see if we need to force an ack-back
   int ct = (seqNum & 0x1F) - highAck;
   if(ct < 0)
      ct += 32;

   // if we have 12 or more packets to ack, ping em back to the other side.
   // this prevents the pipe from being stalled by a process that is only
   // receiving data (prevents timeout retrys)

   if(ct > 11)
      ackBack = true;

   // do all the notifies...

   if(ackCount)
   {
      UInt32 seq;
      UInt32 firstAck = packetAcks[0].seqNum;
      for(i = 0; i < ackCount; i++)
      {
         seq = packetAcks[i].seqNum;
         int count = packetAcks[i].count;
         while(count)
         {
            if(pnotes[seq].packetType != Unused)
               pnotes[seq].acked = true;
            count--;
            seq = (seq + 1) & 0x1F;
         }
      }
      lastPackAckCleared = firstAck;

      i = (lastSendSeq + 1) & 0x1F;
      do
      {
         if(pnotes[i].packetType != Unused)
         {
            if(pnotes[i].acked)
            {
               myVC->setPacketLoss((myVC->getPacketLoss() * 99) / 100.0);
               if(pnotes[i].notifyKey)
                  mySession->onNotify(myVC, pnotes[i].notifyKey, true);

               if((pnotes[i].packetType & ~Resend) == AcceptConnect 
                     && myVC->getState() == VC::AcceptingConnection)
               {
                  myVC->setState(VC::Connected);
                  mySession->onConnection(myVC, Session::Connected, NULL, 0);
               }

               pnotes[i].packetType = Unused;

               myVC->setAverageRTT((recvTime - pnotes[i].sendTime + myVC->getAverageRTT() * 31) >> 5);
               /*timeSum += recvTime - pnotes[i].sendTime;
               timeSumCount++;
               if(timeSumCount == 32)
               {
                  myVC->setAverageRTT(timeSum >> 5);
                  timeSum = 0;
                  timeSumCount = 0;
               }*/
               if(gLogToConsole)
                  Console->printf("Sent packet recieved. %g", myVC->getPacketLoss());
            }
            else
            {
               myVC->setPacketLoss((myVC->getPacketLoss() * 99 + 1) / 100.0);
               if(pnotes[i].notifyKey)
                  mySession->onNotify(myVC, pnotes[i].notifyKey, false);

               pnotes[i].packetType = Unused;
               if(gLogToConsole)
                  Console->printf("Sent packet dropped. %g", myVC->getPacketLoss());
            }
         }
         i = (i + 1) & 0x1F;
      } while(i != seq);
      highAckRecvd = (seq - 1) & 0x1F;
   }

   // okioki
   // let's see about checking what we got...

   // first things first...
   // ackback any pings or resends
   lastRecvTime = recvTime;
   pingSendCount = 0;

   bool processMore = (lastSeqRecvd != seqNum);
   lastSeqRecvd = seqNum;

   if(ackBack)
   {
      // ack with either the last packet we sent
      // or if it's been acked already, a simple ping packet will do.

      int idx = lastSendSeq & 0x1F;

      if(pnotes[idx].packetType != Unused)
         myVC->send(myVC->getDataBuffer(), lastPacketSize, 0, Ack | Resend);
      else
         myVC->send(NULL, 0, 0, Ack);
   }

   if(!processMore)
      return;

   if(pType == RequestConnect && myVC->getState() == VC::Unbound)
   {
      myVC->setState( VC::WaitingForAcceptReject);
      mySession->onConnection(myVC, Session::ConnectionRequested, recvData, recvDataLen);
      return;
   }
   if((pType == Disconnect && myVC->getState() == VC::Connected) ||
      ((pType == AcceptConnect || pType == RejectConnect) &&
        myVC->getState() == VC::RequestingConnection))
   {
      int connectSequence = pstream.readInt(32);

      recvData = pstream.getBytePtr();
      recvDataLen = datalen - (recvData - data);
      if(connectSequence != vcConnectSequence)
      {
         // this is a packet for an old connection
         // ignore it... it's possible that one of these made it past the
         // single bit test.
			log_file.log(LOG_ERROR,"VCProtocol::OLD packet, returning...");
         return;
      }
      if(pType == Disconnect || pType == RejectConnect)
      {
         flushNotifies();
         mySession->onConnection(myVC, pType == Disconnect ?
            Session::Disconnected : Session::ConnectionRejected,
            recvData, recvDataLen);
         myVC->deleteVC();
         return;
      }
      else
      {
         mySession->onConnection(myVC, Session::ConnectionAccepted,
            recvData, recvDataLen);
         myVC->setState(VC::Connected);
      }
   }
   else if(pType == DataPacket && myVC->getState() == VC::Connected)
   {
      recvData = pstream.getBytePtr();
      recvDataLen = datalen - (recvData - data);
      mySession->onReceive(myVC, myVC->getAddressString(), recvData, recvDataLen);
   }
   else
   {
      AssertWarn(0, avar("Bad Packet! - ptype: %d  curState: %d", pType, myVC->getState()));
   }
}

void VCProtocol::flushNotifies()
{
   // loop through all outstanding notifies and deal with em.
   int i = (lastSendSeq + 1) & 0x1F;

   while(i != (lastSendSeq & 0x1F))
   {
      if(pnotes[i].packetType != Unused)
      {
         if(pnotes[i].notifyKey)
            mySession->onNotify(myVC, pnotes[i].notifyKey, false);
         pnotes[i].packetType = Unused;
      }
      i = (i + 1) & 0x1F;
   }
   myVC->setState(VC::Unbound);
}

bool VCProtocol::checkTimeout(DWORD time)
{
   //return false;
   if(time > (lastRecvTime + ((pingSendCount + 1) * 3000)))
   {
      // time to send a new ping
      int idx = lastSendSeq & 0x1F;

      if(pnotes[idx].packetType != Unused)
         myVC->send(myVC->getDataBuffer(), lastPacketSize, 0, Resend);
      else
         myVC->send(NULL, 0, 0, Ping);

      pingSendCount++;

      return pingSendCount > vcRetryCount;
   }
   return false;
}

bool VCProtocol::windowFull()
{
   int seq = (lastSendSeq + 1) & 0x1F;

   return seq == lastPackAckCleared;
}

};


