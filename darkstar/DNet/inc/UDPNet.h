// UDPnet.h - contains UDPTransport and UDPVC classes

#include "dnet.h"
#include "vcprotocol.h"
#include "winsock.h"
#include "bitstream.h"

namespace DNet
{

class UDPTransport : public Transport
{
   int curPort;
   SOCKET transportSocket;
   bool getAddressFromString(const char *addrString, LPVOID addr);
   void getStringFromAddress(const LPVOID addr, char *addrString);

public:
   static bool STgetAddressFromString(const char *addrString, LPVOID addr);
   static void STgetStringFromAddress(const LPVOID addr, char *addrString);
   SOCKET getSocket() { return transportSocket; }

   UDPTransport(Session *ssn);
   ~UDPTransport();

   bool activate(int port);
   bool translateAddress(const char *src, char *dest, int timeout);
   VC *connect(const char *addrString);
   void deactivate( void );
   Error send(const char *address, BYTE *data, int datalen);
   void process();
   Session *getSession() { return mySession; }
   bool getTransportAddress(const char *src, char *transportAddress);
};


class UDPVC : public VC
{
   friend class UDPTransport;

   BYTE packetBuffer[MaxPacketSize + MaxHeaderSize];

   struct sockaddr_in remoteAddress;

   class VCProtocol *protocol;
public:
   UDPVC(struct sockaddr_in *addr, const char *stringName,
      Session *ssn, Transport *trans);
   ~UDPVC();

   void accept(BYTE *data, int datalen);
   void reject(BYTE *data, int datalen);
   void disconnect() { disconnect(NULL,0);}
   void disconnect(BYTE *data, int datalen);

   BYTE *getDataBuffer();
   BYTE *getHeaderBuffer() { return packetBuffer; }

   void receivePacket(BYTE *data, int len, DWORD recvTime);
   Error send(BYTE *data, int len, DWORD notifyKey, int packetType = DataPacket);
   bool initiateConnection(const char *remoteAddress);
   void initiateConnectionFrom(struct sockaddr *remAddr, int fromlen);
   bool windowFull();
};




};
