#include "vcprotocol.h"

namespace DNet
{

class LOOPTransport : public Transport
{

   bool getAddressFromString(const char *addr_string, LPVOID addr);
   void getStringFromAddress(const LPVOID addr, char *nameBuf);
   static LOOPTransport *loopbackTransport;

public:

   LOOPTransport(Session *ssn);
   ~LOOPTransport();

   void process();
   bool activate(int port);
   VC *connect(const char *addrString);
   void deactivate( void );
   Error send(const char *address, BYTE *data, int datalen);
};


class LOOPVC : public VC
{
   friend class LOOPTransport;

   struct PacketQueue
   {
   	BYTE *packet;
      int   packet_size;
      struct PacketQueue *next;
   };

   struct PacketQueue *start, *end, *node;
   class LOOPVC *connected;
   bool  processing;

   int deviceNo;
   BYTE packetBuffer[MaxPacketSize + MaxHeaderSize];
   class VCProtocol *protocol;

public:
   LOOPVC(const char *address, Session *ssn, Transport *trans);
   ~LOOPVC();

   void accept(BYTE *data, int datalen);
   void reject(BYTE *data, int datalen);
   void disconnect() { disconnect(NULL,0);}
   void disconnect(BYTE *data, int datalen);

   BYTE *getDataBuffer() { return packetBuffer + MaxHeaderSize; }
   BYTE *getHeaderBuffer() { return packetBuffer; }

   void receivePacket(BYTE *data, int len, DWORD recvTime);
   Error send(BYTE *data, int len, DWORD notifyKey, int packetType = DataPacket);
   bool initiateConnection(const char *remoteAddress);
   void initiateConnectionFrom(struct sockaddr *remAddr, int fromlen);
   bool windowFull();

   void queue_send_data(BYTE *data, int datalen);
};


};
