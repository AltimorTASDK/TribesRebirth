#include "vcprotocol.h"
#include "commcode.h"

namespace DNet
{

class NULLTransport : public Transport
{
   DWORD deviceId;
//	HCALL transportCall;

   bool getAddressFromString(const char *addrString, LPVOID addr);
   void getStringFromAddress(const LPVOID addr, char *addrString);

public:
/*   HCALL *getCall() { return transportCall; }*/

   NULLTransport(Session *ssn, int deviceId);
   ~NULLTransport();

   bool activate(int port);
   VC *connect(const char *addrString);
   void deactivate( void );
   Error send(const char *address, BYTE *data, int datalen);
   void process();
   Session *getSession() { return mySession; }
   void SetDeviceId(DWORD id) { deviceId = id; };
};


class NULLVC : public VC
{
   friend class NULLTransport;

   BYTE packetBuffer[MaxPacketSize + MaxHeaderSize];

   int deviceNo;

   class Comm *comm_port;
   class VCProtocol *protocol;

public:
   NULLVC(const char *address, Session *ssn, Transport *trans);
   ~NULLVC();

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

   static void  receive_data_from_comm_port( BYTE *, int, void *myThis );
};


};
