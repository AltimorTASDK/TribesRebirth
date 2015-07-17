// udp.cpp contains code for UDPTransport and UDPVC classes

#include "dnet.h"
#include "udpnet.h"

#include <stdio.h>
#include <stdlib.h>

#include "logfile.h"
#include "m_random.h"
extern class LogFile log_file;

namespace DNet  {

int gPacketLoss = 0;
static Random plRandom;

UDPTransport::UDPTransport(Session *ssn) : Transport(ssn)
{
   WSADATA stWSAData;
   WSAStartup(0x0101, &stWSAData);

   log_file.log("WSAStartup with max sockets %ld",(long)stWSAData.iMaxSockets);

  	strcpy(transportName,"IP");
   transportSocket = 0;
   curPort = -1;
}

UDPTransport::~UDPTransport()
{
   // close our socket
   if(transportSocket)
      closesocket(transportSocket);
   WSACleanup();
}

bool UDPTransport::activate(int port)
{
//log_file.log("Activating UDP port %ld",(long)port);
   if(port == curPort || port == 0  && curPort != -1)
   {
//		log_file.log("   IP::activate already active on port %ld",(long)curPort);
      return transportSocket != 0;
   }

   if(transportSocket)
      closesocket(transportSocket);

   transportSocket = 0;
   curPort = -1;

   SOCKET s = socket(PF_INET, SOCK_DGRAM, 0);

   if(s == INVALID_SOCKET)
      return false;

	SOCKADDR_IN addr_in;

	memset((char *)&addr_in, 0, sizeof(SOCKADDR_IN));
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = INADDR_ANY;
	addr_in.sin_port = htons(port);

   DWORD noblock = true;
   int error = 0;

	if(!error)
	   error = bind(s, (PSOCKADDR)&addr_in, sizeof(SOCKADDR_IN));

   if(!error)
      error = ioctlsocket(s, FIONBIO, &noblock);

   // set send and receive buffers to a reasonable amount for a server
   int bufferSize = 32768;
   if(!error)
      error = setsockopt (s, SOL_SOCKET, SO_RCVBUF, (char *) &bufferSize, sizeof(bufferSize));
   if(!error)
      error = setsockopt (s, SOL_SOCKET, SO_SNDBUF, (char *) &bufferSize, sizeof(bufferSize));
   BOOL broadcast = true;
   if(!error)
      error = setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(BOOL));

   if(!error)
   {
      transportSocket = s;
      curPort = port;
		log_file.log("   activation successful");
      return true;
   }
   else
   {
      closesocket(s);
		log_file.log("   activation failed");
      return false;
   }
}

void UDPTransport::deactivate()
{
}

void UDPTransport::getStringFromAddress(const LPVOID addr, char *nameBuf)
{
   STgetStringFromAddress(addr, nameBuf);
}

void UDPTransport::STgetStringFromAddress(const LPVOID addr, char *nameBuf)
{
	const PSOCKADDR_IN addr_in = (PSOCKADDR_IN)addr;

   if(addr_in->sin_addr.s_addr == htonl(INADDR_BROADCAST))
      sprintf(nameBuf, "IP:Broadcast:%d", ntohs(addr_in->sin_port));
   else
      sprintf(nameBuf, "IP:%d.%d.%d.%d:%d", addr_in->sin_addr.s_net,
         addr_in->sin_addr.s_host, addr_in->sin_addr.s_lh,
         addr_in->sin_addr.s_impno, ntohs(addr_in->sin_port));
}


static DWORD doneTime;

static int __stdcall blockingHook()
{
   if(doneTime < GetTickCount())
      WSACancelBlockingCall();
   return false;
}

bool UDPTransport::translateAddress(const char *src, char *dest, int timeout)
{
   SOCKADDR_IN addr_in;
   bool ret;
   if(timeout)
   {
      doneTime = GetTickCount() + DWORD(timeout);
      WSASetBlockingHook(blockingHook);
   }
   if(!getAddressFromString(src, &addr_in))
      ret = false;
   else
   {
      getStringFromAddress(&addr_in, dest);
      ret = true;
   }
   if(timeout)
      WSAUnhookBlockingHook();
   return ret;
}

bool UDPTransport::getAddressFromString(const  char *addrString, LPVOID addr)
{
   return STgetAddressFromString(addrString, addr);
}

bool UDPTransport::STgetAddressFromString(const  char *addrString, LPVOID addr)
{
	PSOCKADDR_IN addr_in = (PSOCKADDR_IN)addr;
   char remoteAddr[256];

   if(addrString == NULL)
   	return false;

   if(!strnicmp(addrString, "IP:", 3))
      addrString += 3;
   else if(!strnicmp(addrString, "IPX:", 4) || !strnicmp(addrString, "LOOPBACK:", 9))
      return false;

   if(strlen(addrString) > 255)
      return false;

   strcpy(remoteAddr, addrString);
   char *portString = strchr(remoteAddr, ':');

   if(!portString)
      return false;

   *portString++ = 0;
	struct hostent *hp;

   if(!stricmp(remoteAddr, "broadcast"))
   {
      addr_in->sin_addr.s_addr = htonl(INADDR_BROADCAST);
   }
   else
   {
      addr_in->sin_addr.s_addr = inet_addr(remoteAddr);
      if(addr_in->sin_addr.s_addr == INADDR_NONE)
      {
         if((hp = gethostbyname(remoteAddr)) == NULL)
            return false;
   	   else
   		   memcpy(&addr_in->sin_addr.s_addr, hp->h_addr, sizeof(IN_ADDR));
      }
   }
  	addr_in->sin_port = htons(atoi(portString));
   addr_in->sin_family = AF_INET;
   return true;
}

VC *UDPTransport::connect(const char *addrString)
{
   SOCKADDR_IN addr;
   UDPVC *ret;

   // check for a connection already
   if( (ret = (UDPVC *)getVCfromAddress(addrString)) != NULL)
   	return ret;

   log_file.log("Connecting to %s",addrString);
   if(!getAddressFromString(addrString, (LPVOID)&addr))
      return NULL;

   if( !activate(0))
   	return NULL;

   if(addr.sin_addr.s_addr == htonl(INADDR_BROADCAST))
      return NULL;

   char astr[255];
   getStringFromAddress((LPVOID)&addr, astr);

   ret = new UDPVC(&addr, astr, mySession, this);
   VCList.push_back(ret);

   log_file.log("   Connect succesful");
   return ret;
}


void UDPTransport::process()
{
   // when the UDP transport gets a new packet it will:
   // determine if it's a game info packet (second byte high
   // bit set).  If so pass it up through the session as such

   // if it's a game packet, the transport will determine
   // which VC it came from and dispatch to that VC.
   // if there is no VC for that remote address the thread
   // creates one and passes up the join request.

   // first poll the socket to see if there is any data ready...

   DWORD time = GetTickCount();
   char addrBuf[64];
   BYTE recvBuf[MaxHeaderSize + MaxPacketSize];

   for(;;)
   {
      SOCKADDR_IN addr_in;
      int addrlen = sizeof(SOCKADDR_IN);
      SOCKET oldSocket = transportSocket;

      int len = recvfrom(oldSocket, (char*)recvBuf, MaxHeaderSize + MaxPacketSize,
            0, (PSOCKADDR) &addr_in, &addrlen);

		if( len <= 0 )
      {
         break;
      }
      if(gPacketLoss)
      {
         if(plRandom.getInt(0, 100) < gPacketLoss)
            continue;
      }

      // ok, we got a packet...
      // check for a loopback packet (bad person)
      if(addr_in.sin_addr.s_net == 127 &&
            addr_in.sin_addr.s_host == 0 &&
            addr_in.sin_addr.s_lh == 0 &&
            addr_in.sin_addr.s_impno == 1 &&
            ntohs(addr_in.sin_port) == curPort)
         continue;


      BitStream bstream(recvBuf, MaxHeaderSize);

      getStringFromAddress((LPVOID)&addr_in, addrBuf);

      // is it a game info packet?
      if(!bstream.readFlag())
         mySession->onReceive(NULL, addrBuf, recvBuf, len);
      else
      {
         UDPVC *vc;

         if( (vc=(UDPVC *)getVCfromAddress(addrBuf)) == NULL )
            vc = (UDPVC *) connect(addrBuf);
         if(vc)
            vc->receivePacket(recvBuf, len, time);
      }
   }
   Vector<VC *>::iterator vc;
   for(vc = VCList.begin(); vc != VCList.end();)
   {
      if(((UDPVC *) (*vc))->getState() == VC::Unbound)
         deleteVC((UDPVC *) *vc);
      else if(((UDPVC *) (*vc))->protocol->checkTimeout(time))
      {
         ((UDPVC *) (*vc))->protocol->flushNotifies();
         mySession->onConnection(*vc, Session::TimedOut, NULL, 0);
         deleteVC(*vc);
      }
      else
         vc++;
   }
}

Error UDPTransport::send(const char *addrString, BYTE *data, int datalen)
{
	SOCKADDR_IN addr_in;

	if(!getAddressFromString(addrString, (LPVOID)&addr_in))
      return UnsupportedAddressType;

//   log_file.log("UDP::send to %s",addrString);

   if( !activate(0))  // the transport hasn't been activated
   	return TransportFailure;

   if(gPacketLoss)
   {
      if(plRandom.getInt(0, 100) < gPacketLoss)
         return NoError;
   }
   sendto (getSocket(), (char*)data, datalen, 0,
          (PSOCKADDR) &addr_in, sizeof(SOCKADDR_IN) );
   return NoError;
}

UDPVC::UDPVC( PSOCKADDR_IN addr_in, const char *addr_string,
      Session *ssn, Transport *trans) : VC(ssn, trans)
{
   memcpy(&remoteAddress, addr_in, sizeof(SOCKADDR_IN));
   strcpy(addressString, addr_string);

   protocol  = new VCProtocol(mySession,this);
}


BYTE *UDPVC::getDataBuffer()
{
   return packetBuffer + MaxHeaderSize;
}

Error UDPVC::send(BYTE *data, int datalen, DWORD notifyKey, int packetType)
{
   BYTE *send_data;
   int send_len,error;
   BYTE *dataBuffer = getDataBuffer();

   AssertFatal(datalen < MaxPacketSize, "Packet overflow.");
   AssertFatal(datalen == 0 || data, "Bad packet.");

   if(!data)
      data = dataBuffer;
   else if(data != dataBuffer)
   {
      memcpy(dataBuffer, data, datalen);
      data = dataBuffer;
   }


	send_data = protocol->prepare_data_for_send(&send_len, data, datalen,
                                               notifyKey, packetType);

	log_file.log("sending data %lx, len %lx, type %lx",(long)send_data,(long)send_len,(long)packetType);
   if( send_data )
   {
      if(gPacketLoss)
      {
         if(plRandom.getInt(0, 100) < gPacketLoss)
            return NoError;
      }
	   error=sendto (((UDPTransport *) myTransport)->getSocket(), (const char *)send_data, send_len,
               0, (PSOCKADDR) &remoteAddress, sizeof(SOCKADDR_IN) );
      if(error == SOCKET_ERROR)
      {
   		log_file.log("   send error %lx",(long)WSAGetLastError());
	     	return SendError;
      }
      else
	   	return NoError;
   }
   else
   	return SendError;
}


void UDPVC::receivePacket(BYTE *data, int datalen, DWORD recvTime)
{
	protocol->receive(data, datalen, recvTime);
}

UDPVC::~UDPVC()
{
   delete protocol;
}

bool UDPVC::windowFull()
{
	return protocol->windowFull();
}

void UDPVC::accept(BYTE *data, int datalen)
{
   send(data, datalen, 0, AcceptConnect);
}

void UDPVC::reject(BYTE *data, int datalen)
{
   send(data, datalen, 0, RejectConnect);
   deleteVC();
}

void UDPVC::disconnect(BYTE *data, int datalen)
{
   send(data, datalen, 0, Disconnect);
   deleteVC();
}

};