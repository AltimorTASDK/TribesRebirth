#ifndef _H_DNET
#define _H_DNET

#include "tvector.h"


namespace DNet
{

enum Error {
   NoError, WindowFull, QueuedPacket, UnsupportedAddressType, SendError, TransportFailure
};

class VC;
class Transport;

enum PacketType
{
   DataPacket,
   Disconnect = 2,
   RequestConnect,
   AcceptConnect,
   RejectConnect,
};

enum
{
   MaxPacketSize = 1500, MaxHeaderSize = 24,
};

enum
{
	MaxTransportName = 9, MaxVCAddress = 128,
};

class Session
{
protected:
   Vector<Transport *> transportList;
public:
   enum ConnectionNotifyType
   {
      ConnectionRequested,
      ConnectionAccepted,
      ConnectionRejected,
      Connected,
      TimedOut,
      Disconnected,
   };

   virtual void onReceive(VC *connection, const char *transportAddress, BYTE *data, int datalen) = 0;
   virtual void onNotify(VC *connection, DWORD notifyKey, bool received) = 0;
   virtual void onConnection(VC *connection, ConnectionNotifyType con, BYTE *data, int datalen) = 0;

   DWORD curConnectSequence;
   int num_transports;
   HINSTANCE hInstance;
public:
   Session(HINSTANCE hInst);
   ~Session();

   // translate a transport address
	bool translateAddress(const char *src, char *dest, int timeOut = 0);

   Transport    *activate(const char *transportName, int port);
   VC           *connect(const char *vcAddress, BYTE *data, int datalen);
   virtual Error send(const char *address, BYTE *data, int datalen);

   void process();
   int numTransports( void ) { return num_transports; }
   const char * getTransportName( int num);
   int numConnections( void );
   const char * getConnectionName( int connectionNum );
   void GetTAPIDevices(DWORD num_ids, DWORD device_ids[]);

   DWORD getConnectSequence();


   void disconnect( VC *vc );  // disconnect a specific VC
   void disconnect( void );    // disconnect all VCs
   void deactivate( Transport *transport ); // deactivate a transport
   void deactivate( void );    // deactivate all transports
};

class Transport
{
   friend class Session;
protected:
   Vector<VC *> VCList;
   Session *mySession;
   char transportName[MaxTransportName]; // "IP", "MODEMn", "COMn", etc

   virtual bool getAddressFromString(const char *addr_string, LPVOID addr) = 0;
   virtual void getStringFromAddress(const LPVOID addr, char *nameBuf) = 0;

public:

   virtual bool translateAddress(const char *src, char *dest, int timeout);

   Transport(Session *ssn) { mySession = ssn; }
   virtual ~Transport();

   const Vector<VC *> &getVCList() { return VCList; }
//   VC *resolveAddress(const char *taString);
   VC *getVCfromAddress(const char *addrString);

   void deleteVC(VC *vcdel);
   void disconnect( void );  // disconnect all VCs in transport (some may have multiple VCs)
   void disconnect( VC *vc );

   virtual void process() = 0;
   virtual bool activate(int port) = 0;
   virtual VC *connect(const char *addrString) = 0;
   virtual void deactivate( void ) = 0;
   virtual Error send(const char *address, BYTE *data, int datalen) = 0;
   const char *getName( void ) { return transportName; };
};

class VC
{
public:
   enum VCState { Unbound, WaitingForAcceptReject, RequestingConnection, Connected, RejectingConnection, AcceptingConnection };
protected:
   Session *mySession;
   Transport *myTransport;
   VCState curState;
   DWORD avgRTT;
   float packetLoss;
   char addressString[MaxVCAddress];
public:
   DWORD userData;

   VC(Session *session, Transport *trans);
   virtual ~VC();

   virtual BYTE *getDataBuffer() = 0;
   virtual BYTE *getHeaderBuffer() = 0;
   virtual Error send(BYTE *data, int len, DWORD notifyKey, int packetType = DataPacket ) = 0;
   virtual bool windowFull() = 0;

   virtual void accept(BYTE *data, int datalen) = 0;
   virtual void reject(BYTE *data, int datalen) = 0;
   virtual void disconnect( void ) = 0;
   virtual void disconnect(BYTE *data, int datalen) = 0;

   VCState getState() { return curState; }
   void    setState(VCState state) { curState = state; }
   float   getPacketLoss() { return packetLoss; }
   void    setPacketLoss(float pl) { packetLoss = pl; }
   DWORD   getAverageRTT() { return avgRTT; }
   void    setAverageRTT( DWORD rtt ) { avgRTT = rtt; }

   const char *getAddressString( void ) { return addressString; };
   Session *getSession() { return mySession; }
   Transport *getTransport() { return myTransport; }
   void deleteVC();
};

inline void VC::deleteVC()
{
   myTransport->deleteVC(this);
}


};

#endif