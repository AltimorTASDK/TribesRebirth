// UDPnet.h - contains UDPTransport and UDPVC classes

#include "dnet.h"
#include "vcprotocol.h"
#include "winsock.h"
#include "wsipx.h"
#include "wsnwlink.h"
#include "bitstream.h"

namespace DNet
{

class IPXTransport : public Transport
{
   int curPort;
   SOCKET transportSocket;
   HINSTANCE ipx_winsocklib;

   bool getAddressFromString(const char *addrString, LPVOID addr);
   void getStringFromAddress(const LPVOID addr, char *addrString);

public:
   SOCKET getSocket() { return transportSocket; }

   IPXTransport(Session *ssn);
   ~IPXTransport();

   bool translateAddress(const char *src, char *dest, int timeout);
   bool activate(int port);
   VC *connect(const char *addrString);
   void deactivate( void );
   Error send(const char *address, BYTE *data, int datalen);
   void process();
   Session *getSession() { return mySession; }
   bool getTransportAddress(const char *src, char *transportAddress);
};


class IPXVC : public VC
{
   friend class IPXTransport;

   BYTE packetBuffer[MaxPacketSize + MaxHeaderSize];

   SOCKADDR_IPX remoteAddress;

   class VCProtocol *protocol;
public:
   IPXVC(PSOCKADDR_IPX addr, const char *stringName,
      Session *ssn, Transport *trans);
   ~IPXVC();

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
