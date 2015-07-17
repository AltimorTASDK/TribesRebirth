// IPX.cpp contains code for IPXTransport and IPXVC classes

#include "dnet.h"
#include "IPXnet.h"

#include <stdio.h>
#include <stdlib.h>

#include "logfile.h"

#define MAINPORT 6234

#define USE_API 0

extern class LogFile log_file;

#if USE_API
typedef struct _WINSOCKAPI {
    int     (APIENTRY *bind       )(SOCKET,const sockaddr *,int);
    int     (APIENTRY *closesocket)(SOCKET);
	 int     (APIENTRY *getsockopt )(SOCKET, int, int, char *, int);
    u_short (APIENTRY *htons      )(u_short);
    int     (APIENTRY *ioctlsocket)(SOCKET s, long cmd, u_long *argp);
    u_short (APIENTRY *ntohs      )(u_short);
    int     (APIENTRY *recvfrom   )(SOCKET,char *,int,int,sockaddr *,int *);
    int     (APIENTRY *sendto     )(SOCKET,const char *,int,int,const sockaddr *,int);
    int     (APIENTRY *setsockopt )(SOCKET,int,int,const char *,int);
    SOCKET  (APIENTRY *socket     )(int,int,int);
    int     (APIENTRY *WSAStartup )(WORD,LPWSADATA);
    int     (APIENTRY *WSACleanup )();
    int     (APIENTRY *WSAGetLastError)();

} WINSOCKAPI, *WINSOCKAPIPTR;

static WINSOCKAPIPTR       ipx_winsockapi    = NULL;
#endif

namespace DNet  {

IPXTransport::IPXTransport(Session *ssn) : Transport(ssn)
{
//log_file.log("IPXTransport::IPXTransport");

  	strcpy(transportName,"IPX");
   transportSocket = 0;
   curPort = -1;

#if USE_API
	// LOAD THE WINSOCK LIBRARY
	ipx_winsocklib = LoadLibrary(TEXT("wsock32.dll"));
   if (!ipx_winsocklib)
   {
		log_file.log("   IPXTransport:: unable to load wsock32.dll");
		return;
   }

	// ALLOCATE AN API STRUCTURE
   if (!ipx_winsockapi)
   {
      ipx_winsockapi = new WINSOCKAPI;
   	if (!ipx_winsockapi)
      {
			log_file.log("   IPXTransport:: unable to create winsock api");
      	return;
      }
	}

	// BIND TO THE INDIVIDUAL FUNCTIONS
   BOOL success = 1;
#define  BIND(a,b)  *(void **)&ipx_winsockapi->##a                               \
                      = GetProcAddress(ipx_winsocklib,(LPCSTR)MAKELONG((b),0)); \
                    if (!ipx_winsockapi->##a)                                    \
                      success = 0;
   BIND(bind       ,2);
   BIND(closesocket,3);
   BIND(getsockopt ,7);
   BIND(htons      ,9);
   BIND(ioctlsocket,12);
   BIND(ntohs      ,15);
   BIND(recvfrom   ,17);
   BIND(sendto     ,20);
   BIND(setsockopt ,21);
   BIND(socket     ,23);
   BIND(WSAStartup ,115);
   BIND(WSACleanup ,116);
   BIND(WSAGetLastError,111);
#undef  BIND

	if(!success)
   {
   	log_file.log("   IPXTransport:: failed to bind all functions!");
      return;
   }
#endif

   WSADATA stWSAData;
#if USE_API
   if(ipx_winsockapi->WSAStartup(0x0101, &stWSAData))
   {
   	log_file.log("   IPXTransport:: WSAStartup fails %ld",ipx_winsockapi->WSAGetLastError());
#else
   if(WSAStartup(0x0101, &stWSAData))
   {
   	log_file.log("   IPXTransport:: WSAStartup fails %ld",WSAGetLastError());
#endif
      return;
   }
//   log_file.log("   IPX::IPX WSAStartup with max sockets %ld",(long)stWSAData.iMaxSockets);

}

IPXTransport::~IPXTransport()
{
//log_file.log("IPXTransport::~IPXTransport");
   // close our socket
   if(transportSocket)
#if USE_API
      ipx_winsockapi->closesocket(transportSocket);
#else
      closesocket(transportSocket);
#endif

#if USE_API
	if( ipx_winsockapi )
   {
	   ipx_winsockapi->WSACleanup();
   	delete ipx_winsockapi;
   }

   if (ipx_winsocklib)
		FreeLibrary(ipx_winsocklib);
#endif
}

bool IPXTransport::activate(int port)
{
//log_file.log("IPX::activate %ld",(long)port);
   if(port == curPort || port == 0 && curPort != -1 )
   {
		log_file.log("   IPX::activate already active on port %ld",(long)curPort);
      return transportSocket != 0;
   }

   if(transportSocket)
#if USE_API
      ipx_winsockapi->closesocket(transportSocket);
#else
      closesocket(transportSocket);
#endif

   transportSocket = 0;
   curPort = -1;

#if USE_API
   SOCKET s = ipx_winsockapi->socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
#else
   SOCKET s = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
#endif

   if(s == INVALID_SOCKET)
   {
#if USE_API
		log_file.log("   IPX:activate unable to init socket %ld",ipx_winsockapi->WSAGetLastError());
#else
		log_file.log("   IPX:activate unable to init socket %ld",WSAGetLastError());
#endif
      return false;
   }

	SOCKADDR_IPX addr_ipx;

	memset((char *)&addr_ipx, 0, sizeof(SOCKADDR_IPX));
	addr_ipx.sa_family = AF_IPX;
#if USE_API
	addr_ipx.sa_socket = ipx_winsockapi->htons(port);
#else
	addr_ipx.sa_socket = htons(port);
#endif

   DWORD noblock = true;
   int error = 0;

#if USE_API
	if( ipx_winsockapi->bind(s, (const struct sockaddr *)&addr_ipx, sizeof(SOCKADDR_IPX)) )
   {
   	log_file.log("   IPXTrans::activate - unable to bind socket %lx",(long)ipx_winsockapi->WSAGetLastError());
	}
#else
	if( bind(s, (const struct sockaddr *)&addr_ipx, sizeof(SOCKADDR_IPX)) )
   {
   	log_file.log("   IPXTrans::activate - unable to bind socket %lx",(long)WSAGetLastError());
	}
#endif


#if 1
	IPX_ADDRESS_DATA iad;
	int optlen;

   iad.adapternum = 0;
   optlen = sizeof(iad);
   error = getsockopt(s, NSPROTO_IPX, IPX_ADDRESS, (char *)&iad, &optlen);

   if( error == SOCKET_ERROR )
   {
   	log_file.log("   getsockopt(IPX_ADDRESS) error %lx",
#if USE_API
			(long)ipx_winsockapi->WSAGetLastError());
#else
			(long)WSAGetLastError());
#endif
   }
   else
   {
   	log_file.log("   IPX::activate ad(%lx)max(%lx)spd(%lx)wan(%lx)stat(%lx)ne(%lx)ne(%lx)ne(%lx)ne(%lx)no(%lx)no(%lx)no(%lx)no(%lx)no(%lx)no(%lx)",
         (long)iad.adapternum,(long)iad.maxpkt,(long)iad.linkspeed,(long)iad.wan,(long)iad.status,
         (long)iad.netnum[0],(long)iad.netnum[1],(long)iad.netnum[2],(long)iad.netnum[3],
         (long)iad.nodenum[0],(long)iad.nodenum[1],(long)iad.nodenum[2],(long)iad.nodenum[3],(long)iad.nodenum[4],(long)iad.nodenum[5]);
	}
#endif

   if(!error)
#if USE_API
      error = ipx_winsockapi->ioctlsocket(s, FIONBIO, &noblock);
#else
      error = ioctlsocket(s, FIONBIO, &noblock);
#endif

   // set send and receive buffers to a reasonable amount for a server
#if USE_API
   int bufferSize = 32768;
   if(!error)
      error = ipx_winsockapi->setsockopt (s, SOL_SOCKET, SO_RCVBUF, (char *) &bufferSize, sizeof(bufferSize));
   if(!error)
      error = ipx_winsockapi->setsockopt (s, SOL_SOCKET, SO_SNDBUF, (char *) &bufferSize, sizeof(bufferSize));
   BOOL broadcast = true;
   if(!error)
      error = ipx_winsockapi->setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(BOOL));
#else
   int bufferSize = 32768;
   if(!error)
      error = setsockopt (s, SOL_SOCKET, SO_RCVBUF, (char *) &bufferSize, sizeof(bufferSize));
   if(!error)
      error = setsockopt (s, SOL_SOCKET, SO_SNDBUF, (char *) &bufferSize, sizeof(bufferSize));
   BOOL broadcast = true;
   if(!error)
      error = setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(BOOL));
#endif

   if(!error)
   {
      transportSocket = s;
      curPort = port;
//		log_file.log("   IPX activation successful");
      return true;
   }
   else
   {
#if USE_API
      ipx_winsockapi->closesocket(s);
#else
      closesocket(s);
#endif
//      log_file.log("   IPX::activate failed");
      return false;
   }
}

void IPXTransport::deactivate()
{
}

void IPXTransport::getStringFromAddress(const LPVOID addr, char *nameBuf)
{
	const PSOCKADDR_IPX paddr_ipx = (PSOCKADDR_IPX)addr;
//log_file.log("IPXTransport::getStringFromAddress");

	if( *((unsigned long *)(paddr_ipx->sa_nodenum)) == INADDR_BROADCAST )
#if USE_API
      sprintf(nameBuf, "IPX:broadcast:%d", ipx_winsockapi->ntohs(paddr_ipx->sa_socket));
#else
      sprintf(nameBuf, "IPX:broadcast:%d", ntohs(paddr_ipx->sa_socket));
#endif
   else
      sprintf(nameBuf, "IPX:%.2X%.2X%.2X%.2X:%.2X%.2X%.2X%.2X%.2X%.2X:%d",
      	(BYTE)paddr_ipx->sa_netnum[0],(BYTE)paddr_ipx->sa_netnum[1],
         (BYTE)paddr_ipx->sa_netnum[2],(BYTE)paddr_ipx->sa_netnum[3],
         (BYTE)paddr_ipx->sa_nodenum[0],(BYTE)paddr_ipx->sa_nodenum[1],
         (BYTE)paddr_ipx->sa_nodenum[2],(BYTE)paddr_ipx->sa_nodenum[3],
         (BYTE)paddr_ipx->sa_nodenum[4],(BYTE)paddr_ipx->sa_nodenum[5],
#if USE_API
         ipx_winsockapi->ntohs(paddr_ipx->sa_socket));
#else
         ntohs(paddr_ipx->sa_socket));
#endif

//	log_file.log("   IPX::getStringFromAddress %s",nameBuf);
}

bool IPXTransport::translateAddress(const char *src, char *dest, int timeout)
{
   timeout;
   SOCKADDR_IPX addr_ipx;
   if(!getAddressFromString(src, (LPVOID) &addr_ipx))
      return false;
   getStringFromAddress((LPVOID) &addr_ipx, dest);
   return true;
}

/* ---------------------------------------------------------------------------*/
/* IPX address takes the form of IPX:netnum:nodenum:portnum.  The netnum is   */
/* 4 bytes, nodenum is 6 bytes and portnum is integer.                        */
/* Example valid address = IPX:00000000:00600896c336:17000                    */
/* For broadcast the address = IPX:broadcast:                                 */
/*----------------------------------------------------------------------------*/

bool IPXTransport::getAddressFromString(const  char *addrString, LPVOID addr )
{
	PSOCKADDR_IPX paddr_ipx = (PSOCKADDR_IPX)addr;
   char net_str[256], *node_str, *socket_str;
   int  i, index, temp;
//log_file.log("IPXTransport::getAddressFromString %s",addrString);

   if(addrString == NULL)
   	return false;
   if(strnicmp(addrString, "IPX:", 4))
      return false;
   if(strlen(addrString) > 255)
      return false;

	memset((char *)paddr_ipx, 0, sizeof(SOCKADDR_IPX));
   paddr_ipx->sa_family = AF_IPX;
   strcpy(net_str, addrString + 4);

   // get the node number; this will be null if broadcast message
   node_str = strchr(net_str, ':');
   if( !node_str )
   	return false;
   *node_str++ = 0;

   // check at this point if this is a broadcast address
   if(!stricmp(net_str, "broadcast"))
   {
		for (i = 0; i < 6; ++i)
      	paddr_ipx->sa_nodenum[i] = (BYTE)0xFF;
//		for (i = 0; i < 4; ++i)
//      	paddr_ipx->sa_netnum[i] = (BYTE)0xFF;
#if USE_API
		paddr_ipx->sa_socket = ipx_winsockapi->htons(atoi(node_str));
#else
		paddr_ipx->sa_socket = htons(atoi(node_str));
#endif
      return true;
   }

	// get the socket number
   socket_str = strchr(node_str, ':');
   if( !socket_str )
   	return false;
   *socket_str++ = 0;

   // do some basic checks
   if( strlen(net_str) != 8 || strlen(node_str) != 12 )
   	return false;

//log_file.log("   ipx:%s:%s:%s",net_str, node_str, socket_str);
   // get the net number; should always be 4 bytes
   for( i = 3, index = 6; i >= 0; i--, index -= 2 )
	{
   	sscanf(&net_str[index],"%X",&temp);
      paddr_ipx->sa_netnum[i] = (char)temp;
      net_str[index] = 0;
   }

   // get the node_number; should always be 6 bytes
   for( i = 5, index = 10; i >= 0; i--, index -= 2 )
   {
      sscanf(&node_str[index],"%X",&temp);
      paddr_ipx->sa_nodenum[i] = (char)temp;
      node_str[index] = 0;
   }

   // finally, get the socket
#if USE_API
	paddr_ipx->sa_socket = ipx_winsockapi->htons(atoi(socket_str));
#else
	paddr_ipx->sa_socket = htons(atoi(socket_str));
#endif

   return true;
}

VC *IPXTransport::connect(const char *addrString)
{
   SOCKADDR_IPX addr_ipx;
	IPXVC *ret;

   // check for a connection already
   if( ret = (IPXVC *)getVCfromAddress(addrString))
   	return ret;

//log_file.log("IPXTransport::connect %s",addrString);
   if(!getAddressFromString(addrString, (LPVOID)&addr_ipx))
      return NULL;

   if( !activate(0))
   	return NULL;

	if( *((unsigned long *)(addr_ipx.sa_nodenum)) == INADDR_BROADCAST )
      return NULL;

   char astr[255];
   getStringFromAddress((LPVOID)&addr_ipx, astr);

   ret = new IPXVC(&addr_ipx, astr, mySession, this);
   if( ret )
	   VCList.push_back(ret);
//log_file.log("   IPXTransport::connect succesful");
   return ret;
}

void IPXTransport::process()
{
   // when the IPX transport gets a new packet it will:
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
      SOCKADDR_IPX addr_ipx;
      int addrlen = sizeof(addr_ipx);
      SOCKET oldSocket = transportSocket;

#if USE_API
      int len = ipx_winsockapi->recvfrom(oldSocket, (char*)recvBuf, MaxHeaderSize + MaxPacketSize,
            0, (sockaddr *) &addr_ipx, &addrlen);
#else
      int len = recvfrom(oldSocket, (char*)recvBuf, MaxHeaderSize + MaxPacketSize,
            0, (sockaddr *) &addr_ipx, &addrlen);
#endif

		if( len <= 0 )
      {
         break;
      }

      // ok, we got a packet...
//      log_file.log("IPX::process packet received %lx bytes",(long)len);
//      for(int i=0; i < len; i++)
//      	log_file.log("   %lx",(long)recvBuf[i]);

      BitStream bstream(recvBuf, MaxHeaderSize);

      getStringFromAddress((LPVOID)&addr_ipx, addrBuf);

      // is it a game info packet?
      if(!bstream.readFlag())
         mySession->onReceive(NULL, addrBuf, recvBuf, len);
      else
      {
         IPXVC *vc;

         if( !(vc=(IPXVC *)getVCfromAddress(addrBuf)) )
            vc = (IPXVC *) connect(addrBuf);

         if( vc )
	         vc->receivePacket(recvBuf, len, time);
      }
   }
   Vector<VC *>::iterator vc;
   for(vc = VCList.begin(); vc != VCList.end();)
   {
      if(((IPXVC *) (*vc))->getState() == VC::Unbound)
         deleteVC((IPXVC*)vc);
      
      if(((IPXVC *) (*vc))->protocol->checkTimeout(time))
      {
         ((IPXVC *) (*vc))->protocol->flushNotifies();
         mySession->onConnection(*vc, Session::TimedOut, NULL, 0);
         deleteVC(*vc);
      }
      else
         vc++;
   }
}

Error IPXTransport::send(const char *addrString, BYTE *data, int datalen)
{
	SOCKADDR_IPX addr_ipx;
//log_file.log("IPX::send to %s",addrString);
	if(!getAddressFromString(addrString, (LPVOID)&addr_ipx))
      return UnsupportedAddressType;

   if( !activate(0))  // the transport hasn't been activated
   	return TransportFailure;

//log_file.log("   IPX:sending...");
#if USE_API
   ipx_winsockapi->sendto (getSocket(), (char*)data, datalen,
         0, (struct sockaddr *) &addr_ipx, sizeof(SOCKADDR_IPX) );
#else
   sendto (getSocket(), (char*)data, datalen,
         0, (struct sockaddr *) &addr_ipx, sizeof(SOCKADDR_IPX) );
#endif
   return NoError;
}

IPXVC::IPXVC(PSOCKADDR_IPX paddr_ipx, const char *stringName,
      Session *ssn, Transport *trans) : VC(ssn, trans)
{
   memcpy(&remoteAddress, paddr_ipx, sizeof(SOCKADDR_IPX));
   strcpy(addressString, stringName);

   protocol  = new VCProtocol(mySession,this);
}


BYTE *IPXVC::getDataBuffer()
{
   return packetBuffer + MaxHeaderSize;
}

Error IPXVC::send(BYTE *data, int datalen, DWORD notifyKey, int packetType)
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

//	log_file.log("sending data %lx, len %lx, type %lx",(long)send_data,(long)send_len,(long)packetType);
//   for(int i=0; i < send_len; i++)
//   	log_file.log("   %lx",(long)send_data[i]);
   if( send_data )
   {
#if USE_API
	   error=ipx_winsockapi->sendto (((IPXTransport *) myTransport)->getSocket(), send_data, send_len,
               0, (struct sockaddr *) &remoteAddress, sizeof(SOCKADDR_IPX) );
#else
	   error=sendto (((IPXTransport *) myTransport)->getSocket(), (const char *)send_data, send_len,
               0, (struct sockaddr *) &remoteAddress, sizeof(SOCKADDR_IPX) );
#endif
      if(error == SOCKET_ERROR)
      {
#if USE_API
   		log_file.log("   send error %lx",(long)ipx_winsockapi->WSAGetLastError());
#else
   		log_file.log("   send error %lx",(long)WSAGetLastError());
#endif
	     	return SendError;
      }
      else
	   	return NoError;
   }
   else
   	return SendError;
}


void IPXVC::receivePacket(BYTE *data, int datalen, DWORD recvTime)
{
	protocol->receive(data, datalen, recvTime);
}

IPXVC::~IPXVC()
{
   delete protocol;
}

bool IPXVC::windowFull()
{
	return protocol->windowFull();
}

void IPXVC::accept(BYTE *data, int datalen)
{
   send(data, datalen, 0, AcceptConnect);
}

void IPXVC::reject(BYTE *data, int datalen)
{
   send(data, datalen, 0, RejectConnect);
   deleteVC();
}

void IPXVC::disconnect(BYTE *data, int datalen)
{
   send(data, datalen, 0, Disconnect);
   deleteVC();
}

};