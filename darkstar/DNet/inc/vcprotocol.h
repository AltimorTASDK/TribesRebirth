
#ifndef _H_VCPROTOCOL
#define _H_VCPROTOCOL

#include "bitstream.h"
#include "dnet.h"

namespace DNet {

class VCProtocol {
public:
protected:
private:

	VC *myVC;
   Session *mySession;

   bool acks[32];
   DWORD timeSum;
   DWORD lastRecvTime;
   int pingSendCount;
   int timeSumCount;
   int vcRetryCount;

   UInt32 lastSeqRecvd;
   int highAckRecvd;
   int ackClearPt;
   int lastSendSeq;
   int lastPackAckCleared;
   int lastPacketSize;

   int vcConnectSequence;

   enum VCPacketType
   {
      Unused = 6,
      Ping = 7,
      Resend = 8,
      Ack = 16,
   };
   struct PacketNotifyLink
   {
      DWORD sendTime;
      bool acked; // set to false initially and set to true when this is acked
      int packetType; // either a GuaranteeType or a VCPacketType
      DWORD notifyKey;
   };

   struct udpAck
   {
      int seqNum;
      int count;
   };
   PacketNotifyLink pnotes[32];

public:
   VCProtocol(Session *my_session,VC *my_vc);
   ~VCProtocol();
   void  flushNotifies();
   bool  checkTimeout(DWORD time);
	int   buildAckHeader(BitStream *bstream, int guaranteeType);
   BYTE *prepare_data_for_send(int *sendlen, BYTE *data, int datalen,
                               DWORD notifyKey, int packetType);
	void  receive(BYTE *data, int datalen, DWORD recvTime);
   bool windowFull();
   void setRecvTime( DWORD time ) { lastRecvTime = time; }

};

};
#endif