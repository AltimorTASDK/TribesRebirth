#include "dnet.h"
#include "nullnet.h"
#include "stdio.h"
#include "stdlib.h"
#include "logfile.h"

extern class LogFile log_file;

namespace DNet
{



NULLTransport::NULLTransport(Session *ssn, int dev_id) : Transport(ssn)
{
	this->deviceId = dev_id;
   sprintf(transportName,"COM%ld",(long)deviceId);
}

NULLTransport::~NULLTransport()
{
	// make sure we delete VCs
   while (VCList.size())
      deleteVC(*VCList.begin());
   deactivate();
}

bool NULLTransport::activate(int port)
{
	HANDLE hCom;

   if( !VCList.empty())
   	return true;

   NULLVC *vc = new NULLVC(transportName, mySession, this);
   if(vc)
   {
      hCom = vc->comm_port->OpenComm(transportName);
      if( hCom )
      {
	      vc->comm_port->SetComm( hCom );
   	   if(!vc->comm_port->StartComm())
         {
         	delete vc;
            return false;
         }
      	VCList.push_back(vc);
	      return true;
		}
      else
      	delete vc;
   }
	// need to open and activate the comm port so we can listen
   return false;
}

void NULLTransport::deactivate()
{
}

VC *NULLTransport::connect(const char *addrString)
{
	NULLVC *vc = NULL;
   char address[128];

   if(getAddressFromString(addrString,(LPVOID)address))
   {
		if(activate(0))
   	   vc = (NULLVC*)(*(VCList.begin()));
   }

   return vc;
}

Error NULLTransport::send(const char *addrString, BYTE *data, int datalen)
{
   char address[128];

   if(getAddressFromString(addrString,(LPVOID)address) && !VCList.empty())
   {
	   NULLVC *nullvc = (NULLVC *)(*(VCList.begin()));

  		return nullvc->comm_port->WriteCommString(data, datalen);
   }
   return SendError;
}

// with NULL transport there is no address translation from addressString
// as with some of the other transports (IP, IPX)
bool NULLTransport::getAddressFromString(const char *str, LPVOID addr)
{

	// get transport name token from src string (src string should be "COM1:", etc)
   if(!strncmp(transportName, str, strlen(transportName)) &&
       str[strlen(transportName)] == ':')
   {
      strcpy((char *)addr, str);
   	return true;
   }
   else
   	return false;
}

void NULLTransport::getStringFromAddress(const LPVOID addr, char *str)
{
   strcpy(str, (char *)addr);
}


void NULLTransport::process()
{
   NULLVC *nullvc;

	// process the comm port
   if( !VCList.empty() )
   {
   	nullvc = (NULLVC *)(*(VCList.begin()));

	   if (nullvc->comm_port->GetComm() ) // there is an active connection, check for data on commline
   	{
	   	if(nullvc->comm_port->Process((void *)nullvc))
         {

				// check for timeouts
   			DWORD time = GetTickCount();
	   		if( nullvc->getState() == VC::Connected && nullvc->protocol->checkTimeout(time))
				{
					log_file.log("NULLVC::***connection timed out***");
				   nullvc->protocol->flushNotifies();
      			mySession->onConnection(nullvc, Session::TimedOut, NULL, 0);
	     			deleteVC(nullvc);
			   }
         }
         else
         {
         	// a false return from process indicates a comm failure
            deleteVC(nullvc);
         }
   	}
   }
}


NULLVC::NULLVC(const char *address, Session *ssn, Transport *trans) : VC(ssn, trans)
{
	mySession   = ssn;
   myTransport = trans;

   if(address)
   {
		strcpy(addressString, address);
      addressString[strlen(address)] = ':';
      addressString[strlen(address)+1] = '\0';
   }

   comm_port = new Comm( TRUE, receive_data_from_comm_port  ); // must be initialized first
   protocol  = new VCProtocol(mySession,this);
}

bool NULLVC::windowFull()
{
	return protocol->windowFull();
}

NULLVC::~NULLVC()
{
   delete protocol;
   comm_port->StopComm();
   delete comm_port;
}


Error NULLVC::send(BYTE *data, int datalen, DWORD notifyKey, int packetType)
{
   BYTE *send_data;
   int send_len;
   BYTE *dataBuffer = getDataBuffer();

   if(!comm_port->GetComm()) // return if comm is not active yet
   	return SendError;

   AssertFatal(datalen < MaxPacketSize, "Packet overflow.");
   AssertFatal(datalen == 0 || data, "Bad packet.");

   log_file.log("NULLVC::send type %lx, state %lx",(long)packetType,(long)getState());

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
   	return comm_port->WriteCommString(send_data, send_len);
   else
   	return SendError;
}


void NULLVC::receivePacket(BYTE *data, int datalen, DWORD recvTime)
{
	protocol->receive(data, datalen, recvTime);
}

void NULLVC::accept(BYTE *data, int datalen)
{
	log_file.log("NULLVC::accept");
   send(data, datalen, 0, AcceptConnect);
}

void NULLVC::reject(BYTE *data, int datalen)
{
	log_file.log("NULLVC::reject");
   send(data, datalen, 0, RejectConnect);
   deleteVC();
}

void NULLVC::disconnect(BYTE *data, int datalen)
{
	log_file.log("NULLVC::disconnect");
   send(data, datalen, 0, Disconnect);
   deleteVC();
}

void  NULLVC::receive_data_from_comm_port( BYTE *recvBuf, int recvLen, void *scope)
{
	NULLVC *myThis = (NULLVC *)scope;
   DWORD time = GetTickCount();
	BitStream bstream(recvBuf, MaxHeaderSize);

   if(!myThis)
   	return;

	log_file.log("NULLVC::receive_data_from_comm_port: %lx bytes, state %lx",recvLen,myThis->getState());
   // is it a game info packet?
   if(!bstream.readFlag())
   {
      myThis->mySession->onReceive(NULL, myThis->addressString, recvBuf, recvLen);
	}
   else
   {
      myThis->receivePacket(recvBuf, recvLen, time);
      if(myThis->getState() == VC::Unbound)
      {
			log_file.log("NULLVC::Unbound VC in receive_data_from_comm_port");
         myThis->deleteVC();
      }
   }
}

};