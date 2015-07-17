#include "dnet.h"
#include "loopnet.h"
#include "stdio.h"
#include "stdlib.h"
#include "logfile.h"

extern class LogFile log_file;

namespace DNet
{

LOOPTransport *LOOPTransport::loopbackTransport = 0;

LOOPTransport::LOOPTransport(Session *ssn) : Transport(ssn)
{
	strcpy(transportName,"LOOPBACK");
}

LOOPTransport::~LOOPTransport()
{
	// make sure we delete VCs
   while (VCList.size()) 
      deleteVC(*VCList.begin());
   deactivate();
}

bool LOOPTransport::activate(int port)
{
	LOOPTransport::loopbackTransport = this;

   return true;
/*
   LOOPVC *vc = new LOOPVC(transportName, mySession, this);
   if( vc )
   {
   	VCList.push_back(vc);
   	return true;
   }
	return false;
*/
}

void LOOPTransport::deactivate()
{
	LOOPTransport::loopbackTransport = NULL;
}

VC *LOOPTransport::connect(const char *addrString)
{
	char address[128];

	if(!getAddressFromString(addrString, (LPVOID)address))
   	return NULL;

   if( !LOOPTransport::loopbackTransport )
   {
   	log_file.log("Unable to connect loopback, no activated transport");
   	return NULL;
   }


   LOOPVC *client_vc = new LOOPVC(addrString, mySession, this);
   LOOPVC *server_vc = new LOOPVC(addrString, LOOPTransport::loopbackTransport->mySession,
                                              LOOPTransport::loopbackTransport);
   if( client_vc && server_vc)
   {
   	VCList.push_back(client_vc);
   	LOOPTransport::loopbackTransport->VCList.push_back(server_vc);
      client_vc->connected = server_vc;
      server_vc->connected = client_vc;
   }

   return client_vc;
}

Error LOOPTransport::send(const char *addrString, BYTE *data, int datalen)
{
	char address[128];

	if(getAddressFromString(addrString, (LPVOID)address))
   {
	   if(!VCList.empty())
   	{
	   	LOOPVC *vc = (LOOPVC *)(*(VCList.begin()));
	      if( vc->connected )
   	   {
	   	   vc->queue_send_data(data, datalen);
	   		return NoError;
	      }
   	}
   }
   return SendError;
}

bool LOOPTransport::getAddressFromString(const char *src, LPVOID addr)
{
   if(!strncmp(src, transportName, strlen(transportName)))
   {
		strcpy((char *)addr, src);
   	return true;
   }
   else
   	return false;
}

void LOOPTransport::getStringFromAddress(const LPVOID addr, char *str)
{
	strcpy(str, (char *)addr);
}


void LOOPTransport::process()
{
   DWORD time = GetTickCount();
   LOOPVC *vc;

	// process the loopback queue
   if( !VCList.empty() )
   {
   	vc = (LOOPVC *)(*(VCList.begin()));

		vc->processing = true;  // prevents packets added to queue while processing
	   while (vc->start ) // there is a packet in the queue
   	{
      	BitStream bstream(vc->start->packet, MaxHeaderSize);
	      if(!bstream.readFlag())
   	      mySession->onReceive(NULL, NULL, vc->start->packet, vc->start->packet_size);
      	else
         	vc->receivePacket(vc->start->packet, vc->start->packet_size, time);

         if ( VCList.empty() )
            return;

			vc->node = vc->start;
         vc->start = vc->start->next;
         if( vc->start == NULL ) vc->end = NULL;
         delete [] vc->node->packet;
         delete vc->node;
		}
      vc->processing = false;

			// check for timeouts
   	DWORD time = GetTickCount();
	   if( vc->getState() == VC::Connected && vc->protocol->checkTimeout(time))
		{
			log_file.log("LOOPVC::***connection timed out***");
		   vc->protocol->flushNotifies();
      	mySession->onConnection(vc, Session::TimedOut, NULL, 0);
	   	deleteVC(vc);
		}
   }
}


LOOPVC::LOOPVC(const char *address, Session *ssn, Transport *trans) : VC(ssn, trans)
{
	mySession   = ssn;
   myTransport = trans;

   start = end = NULL;
   connected = NULL;
   processing = false;

   if(address)
		strcpy(addressString, address);

   protocol  = new VCProtocol(mySession,this);
}

bool LOOPVC::windowFull()
{
	return protocol->windowFull();
}

LOOPVC::~LOOPVC()
{
   // clean up the packets that never got delivered
   if(connected)
      connected->connected = NULL;
   while (start)
   {
      node = start;
      start = start->next;
      delete[] node->packet;
      delete node;   
   }
    
   delete protocol;
}


Error LOOPVC::send(BYTE *data, int datalen, DWORD notifyKey, int packetType)
{
   BYTE *send_data;
   int send_len;
   BYTE *dataBuffer = getDataBuffer();

   if(!connected) // return if for some reason not connected to loopback vc
   	return SendError;

   AssertFatal(datalen < MaxPacketSize, "Packet overflow.");
   AssertFatal(datalen == 0 || data, "Bad packet.");

   log_file.log("LOOPVC::send type %lx, state %lx",(long)packetType,(long)getState());

   if(!data)
      data = dataBuffer;
   else if(data != dataBuffer)
   {
      memcpy(dataBuffer, data, datalen);
      data = dataBuffer;
   }


   send_data = protocol->prepare_data_for_send(&send_len, data, datalen,
                                               notifyKey, packetType);

   if( send_data )
   {
   	queue_send_data(send_data, send_len);
      return NoError;
   }
   else
   	return SendError;
}


void LOOPVC::receivePacket(BYTE *data, int datalen, DWORD recvTime)
{
	protocol->receive(data, datalen, recvTime);
}

void LOOPVC::accept(BYTE *data, int datalen)
{
	log_file.log("LOOPVC::accept");
   send(data, datalen, 0, AcceptConnect);
}

void LOOPVC::reject(BYTE *data, int datalen)
{
	log_file.log("LOOPVC::reject");
   send(data, datalen, 0, RejectConnect);
   deleteVC();
}

void LOOPVC::disconnect(BYTE *data, int datalen)
{
	log_file.log("LOOPVC::disconnect");
   send(data, datalen, 0, Disconnect);
   deleteVC();
}

void LOOPVC::queue_send_data(BYTE *data, int datalen)
{
   if(!connected)
      return;

   // create queue element and copy in packet
	node = new struct PacketQueue;
   node->packet = new BYTE[MaxPacketSize];
   memcpy(node->packet, data, datalen);
   node->packet_size = datalen;
   node->next = NULL;

   // queue up element in the vc we are connected to
   if( connected->end == NULL )
   {
   	connected->start = connected->end = node;
   }
   else
   {
     	connected->end->next = node;
      connected->end = node;
   }
}

};
