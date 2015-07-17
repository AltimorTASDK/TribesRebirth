#include "dnet.h"
#include "tapi.h"
#include "tapinet.h"
#include "stdio.h"
#include "stdlib.h"
#include "logfile.h"

extern class LogFile log_file;


namespace DNet
{



TAPITransport::TAPITransport(Session *ssn, int dev_id, HINSTANCE hInst) : Transport(ssn)
{

	mySession = ssn;
	deviceId  = dev_id;
   hInstance = hInst;
   callbackId = GetTickCount();
   sprintf(transportName,"MODEM%ld",(long)deviceId+1);


	// flags to prevent re-entrancy problems.
   g_bConnected         = false;
	g_bStoppingCall      = false;
	g_bCalling           = false;
   g_bReplyReceived     = false;
   g_bCallStateReceived = false;

	// Data needed per call.
	g_hCall = NULL;
	g_hLine = NULL;
   g_hCommFile = NULL;
	g_dwDeviceID = MAXDWORD;
	g_dwAPIVersion  = 0;
	g_dwCallState   = 0;
   g_dwRequestedID = 0;
   g_lAsyncReply   = 0;
	g_szDisplayableAddress[0] = 0;
	g_szDialableAddress[0] = 0;
}

TAPITransport::~TAPITransport()
{
	// make sure we delete VCs
   while (VCList.size())
      deleteVC(*VCList.begin());
   deactivate();
   SetMyThis(NULL, deviceId, 0);
}

bool TAPITransport::activate(int port)
{
   // initialize TAPI and open line, port is ignored
	if( !InitializeTAPI())
   	return false;

	SetMyThis(this, deviceId, callbackId);  // pointer to class for callback function
	if( !OpenTAPILine((DWORD)deviceId) )
   	return false;

   return true;
}

void TAPITransport::deactivate()
{
  	ShutdownTAPI();
}

VC *TAPITransport::connect(const char *addrString)
{
   char address[128];

   if(getAddressFromString(addrString, (LPVOID)address))
   {
	   if ( g_hLine == NULL ) // hasn't yet been initialized
   	{
	   	if( !activate(0) )
   	   	return NULL;
	   }

		if(DialCall(address))
	   {
      	char addr_str[128];
      	getStringFromAddress((LPVOID)address, addr_str);
			TAPIVC *vc = new TAPIVC(addr_str, mySession, this);
   		VCList.push_back(vc);
	      return vc;
   	}
   }
   return NULL;
}

Error TAPITransport::send(const char *addrString, BYTE *data, int datalen)
{
	char address[128];

	if(getAddressFromString(addrString, (LPVOID)address))
   {
	   if(!VCList.empty())
   	{
	   	TAPIVC *tapivc = (TAPIVC *)(*(VCList.begin()));

	  		return tapivc->comm_port->WriteCommString(data, datalen);
   	}
   }
   return SendError;
}

// this routine checks to see if the transport name
// is correct and if so, returns the address portion(phone number) in 'addr'
// address is in format MODEMn:phone_number (ex. "MODEM1:123-4567")
bool TAPITransport::getAddressFromString(const char *str, LPVOID addr)
{
	char transport_str[128];

   strcpy(transport_str,str);
   char *address_str = strchr(transport_str, ':');
   if(!address_str)
   	return false;

   *address_str++ = 0;

	// transport name must match, transport name "MODEM" is treated as "MODEM1"
   if(!strcmp(transportName, transport_str) ||
      (deviceId == 0 && !strcmp("MODEM",transport_str)))
   {
   	strcpy((char *)addr,address_str);
   	return true;
   }
   else
   	return false;
}

void TAPITransport::getStringFromAddress(const LPVOID addr, char *str)
{
	sprintf(str,"%s:%s",transportName, (char *)addr);
}

void TAPITransport::process()
{
   bool data_read;
   bool comm_uninitiated = true;
   bool vc_uninitiated = true;
   TAPIVC *tapivc = NULL;

	// process the comm port
   if( !VCList.empty() )
   {
   	vc_uninitiated = false;
//	   Vector<VC *>::iterator vc = VCList.begin();
//   	TAPIVC *tapivc = (TAPIVC *)(*vc);
   	tapivc = (TAPIVC *)(*(VCList.begin()));

	   if (tapivc->comm_port->GetComm() ) // there is an active connection, check for data on commline
   	{
      	comm_uninitiated = false;
	   	data_read = tapivc->comm_port->Process(tapivc);

			if( data_read ) // data will be in the recvBuffer
			{
   		}

			// check for timeouts
   		DWORD time = GetTickCount();
	   	if( tapivc->protocol->checkTimeout(time))
			{
				log_file.log("***connection timed out***");
			   tapivc->protocol->flushNotifies();
      		mySession->onConnection(tapivc, Session::TimedOut, NULL, 0);
	     		deleteVC(tapivc);
		   }
   	}
   }

   if( comm_uninitiated && g_hCommFile )
   {
   	// The lineCallBack function will set the h_gCommFile handle
      // when a connection is establish.  We must then create the VC
      // and get the comm port going

//log_file.log("** Initing comm %lx",g_hCommFile);
		if(vc_uninitiated) // vc on dialing end already inited
      {
      	char addr_str[128], addr[]="123-4567"; // have to use some bogus address since we don't know the calling num
         getStringFromAddress((LPVOID)addr, addr_str);
	      tapivc = new TAPIVC(addr_str, mySession, this);
  			VCList.push_back(tapivc);
      }
      tapivc->comm_port->SetComm( g_hCommFile );
      tapivc->comm_port->StartComm();
   	tapivc->protocol->setRecvTime(GetTickCount());

      if( !vc_uninitiated ) // this vc was on dialing end, it will initiate comm
      {
      	Error error = tapivc->send(tapivc->wait_data, tapivc->wait_data_size, 0, RequestConnect);
         if( tapivc->wait_data )
         {
         	delete tapivc->wait_data;
            tapivc->wait_data_size = 0;
         }

	      if( error != NoError )
   	   {
				log_file.log(LOG_ERROR,"vc->send error %lx, deleting vc",(long)error);
      		delete tapivc;
	      }
      }
   }
}

TAPIVC::TAPIVC(const char *addr_string,Session *ssn,
               Transport *trans) : VC(ssn, trans)
{
	mySession   = ssn;
   myTransport = trans;

   strcpy(addressString, addr_string);

   comm_port = new Comm( FALSE, receive_data_from_comm_port ); // must be initialized first
   protocol  = new VCProtocol(mySession,this);
   wait_data = NULL;
   wait_data_size = 0;

   DataStartTime = DataRecvTotal = DataPacketsTotal = 0;
}

bool TAPIVC::windowFull()
{
	return protocol->windowFull();
}

TAPIVC::~TAPIVC()
{
   delete protocol;
   delete comm_port;

	DWORD secs = (GetTickCount()-DataStartTime) / 1000;
	log_file.log("**********************************************");
   log_file.log("TAPIVC statistics");
   log_file.log("   TOTAL TIME %ld secs",secs);
   log_file.log("   TOTAL PACKETS %ld",(long)DataPacketsTotal);
   log_file.log("   TOTAL DATA %ld bytes",(long)DataRecvTotal);
   log_file.log("   DATA RECV RATE %ld bytes/sec",(long)DataRecvTotal/secs);

   ((TAPITransport *)myTransport)->HangupCall();
}


Error TAPIVC::send(BYTE *data, int datalen, DWORD notifyKey, int packetType)
{
   BYTE *send_data;
   int send_len;
   BYTE *dataBuffer = getDataBuffer();

   if(!comm_port->GetComm()) // return if comm is not active yet
   {
   	// since connect returns before and actual connection, we must save the
      // data and send when the modem connects
		if( packetType == RequestConnect )
      {
         if( wait_data )
         	log_file.log(LOG_ERROR,"TAPIVC::send ERROR: multiple wait data");
         else
         {
         	wait_data = new BYTE[datalen];
            memcpy(wait_data, data, datalen);
            wait_data_size = datalen;
         }
      	return NoError;
      }
      else
	   	return SendError;
   }

   AssertFatal(datalen < MaxPacketSize, "Packet overflow.");
   AssertFatal(datalen == 0 || data, "Bad packet.");

//   log_file.log("VC send with type %lx, state %lx",(long)packetType,(long)getState());

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


void TAPIVC::receivePacket(BYTE *data, int datalen, DWORD recvTime)
{
	protocol->receive(data, datalen, recvTime);
}

void TAPIVC::accept(BYTE *data, int datalen)
{
	log_file.log("***accepting connection");
   send(data, datalen, 0, AcceptConnect);
}

void TAPIVC::reject(BYTE *data, int datalen)
{
	log_file.log("***rejecting connection");
   send(data, datalen, 0, RejectConnect);
   deleteVC();
}

void TAPIVC::disconnect(BYTE *data, int datalen)
{
	log_file.log("***disconnect connection");
   send(data, datalen, 0, Disconnect);
   deleteVC(); // hangs up and shuts down comm port
}

void TAPIVC::receive_data_from_comm_port( BYTE *recvBuf, int recvLen, void *scope)
{
	TAPIVC *myThis = (TAPIVC *)scope;
   DWORD time = GetTickCount();
	BitStream bstream(recvBuf, MaxHeaderSize);

	if( !myThis->DataStartTime )
   	myThis->DataStartTime = GetTickCount();
   myThis->DataRecvTotal += recvLen;
   myThis->DataPacketsTotal++;

   if(!myThis)
   	return;
//   log_file.log("*receive_data_from_comm_port: %lx bytes, state %lx",recvLen,myThis->getState());
   // is it a game info packet?
   if(!bstream.readFlag())
   {
//		log_file.log("   sending data to session receive");
      myThis->mySession->onReceive(NULL, myThis->addressString, recvBuf, recvLen);
   }
   else
   {
      myThis->receivePacket(recvBuf, recvLen, time);
      if(myThis->getState() == VC::Unbound)
      {
			log_file.log(LOG_ERROR,"   Unbound VC in receive_data_from_comm_port");
         myThis->deleteVC();
      }
   }
}

};
