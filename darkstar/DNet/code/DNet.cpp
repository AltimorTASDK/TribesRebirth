// dnet.cpp - contains code for Session, Transport and VC base classes

#include <windows.h>

#include "dnet.h"
#include "udpnet.h"
#include "loopnet.h"
#include "ipxnet.h"
//#include "tapinet.h"
//#include "nullnet.h"
#include "logfile.h"
#include "Console.h"
#include <stdio.h>

LogFile log_file(true,false,1,"DNET TEST");

namespace DNet {

extern bool gLogToConsole;
extern int gPacketLoss;

/*---------------------------------------------------------------------------*/
/* Session                                                                   */
/*---------------------------------------------------------------------------*/

Session::Session(HINSTANCE hInst):hInstance(hInst)
{
	Console->addVariable(0, "DNet::ShowStats", CMDConsole::Bool, &gLogToConsole);
	Console->addVariable(0, "DNet::PacketLoss", CMDConsole::Int, &gPacketLoss);


	num_transports = 0;

   Transport *newTrans;

   // UDP is always available, if not through internet, then local or network
   newTrans = (Transport *) new UDPTransport(this);
   transportList.push_back(newTrans);
   num_transports++;

   newTrans = (Transport *) new IPXTransport(this);
   transportList.push_back(newTrans);
   num_transports++;

	// LOOPBACK is always available
   newTrans = (Transport *) new LOOPTransport(this);
   transportList.push_back(newTrans);
   num_transports++;

   curConnectSequence = GetTickCount();

}

Session::~Session()
{
   // delete the transport list
   for (int i = 0; i < transportList.size(); i++)
      delete transportList[i];
}

DWORD Session::getConnectSequence()
{
   return curConnectSequence++;
}

bool Session::translateAddress(const char *src, char *dest, int timeOut)
{
   for(int i = 0; i < transportList.size(); i++)
      if(transportList[i]->translateAddress(src, dest, timeOut))
         return true;
   return false;
}

/*-------------------------------------------------------------------------*/
/* Activation is usually done on the server side.  The server activates a  */
/* transport in anticipation of a client trying to connect to it.  We      */
/* search through the list of available transports and activate the one    */
/* that matches transportName.  "port" is used only for UDP for now, which */
/* has to bind to a port for activation.                                   */
/*-------------------------------------------------------------------------*/
Transport * Session::activate(const char *transportName, int port)
{
   Vector<Transport *>::iterator transport;

//log_file.log("Session::activate %s:%ld",transportName, (long)port);
   for(transport = transportList.begin(); transport != transportList.end(); transport++)
   {
      if( !strcmp(transportName,(*transport)->getName()))
      {
      	if((*transport)->activate( port ))
	         return (*transport);
         else
         	return NULL;
      }
   }
   return NULL;
}

/*--------------------------------------------------------------------------*/
/* The connect call us usually made by the client.  The vcAddress string is */
/* in the format transportName:address[:port] such as "MODEM1:123-4565" or  */
/* "IP:123.45.67.89:2222".  The LOOPBACK and COMn transports don't require  */
/* an address extension.  connect sends the complete address to each        */
/* transport until one accepts the string and connects with it.             */
/*--------------------------------------------------------------------------*/
VC *Session::connect(const char *vcAddress, BYTE *data, int datalen)
{
   Vector<Transport *>::iterator transport;

   for(transport = transportList.begin(); transport != transportList.end(); transport++)
   {
		VC *vc = (*transport)->connect(vcAddress);
   	if(vc)
	   {
   	   vc->send(data, datalen, 0, RequestConnect);
      	return vc;
	   }
   }
   return NULL;
}


void Session::process()
{
   Vector<Transport *>::iterator transport;
   for(transport = transportList.begin(); transport != transportList.end(); transport++)
      (*transport)->process();
}

Error Session::send(const char *addrString, BYTE *data, int datalen)
{
   Vector<Transport *>::iterator i;

   for(i = transportList.begin(); i != transportList.end(); i++)
   {
      if((*i)->send(addrString, data, datalen) == NoError)
         return NoError;
   }

   return UnsupportedAddressType;
}

void Session::disconnect( VC *vc )
{
	if( vc )
		vc->disconnect();
}

void Session::disconnect( void )
{
   Vector<Transport *>::iterator i;
   for(i = transportList.begin(); i != transportList.end(); i++)
      (*i)->disconnect();
}

void Session::deactivate( Transport *transport )
{
	if( transport )
		transport->deactivate();
}

void Session::deactivate( void )
{
   Vector<Transport *>::iterator i;
   for(i = transportList.begin(); i != transportList.end(); i++)
      (*i)->deactivate();
}

const char * Session::getTransportName( int index)
{
   Vector<Transport *>::iterator transport;
   int count=0;

   for(transport = transportList.begin(); transport != transportList.end(); transport++)
   {
      if( count == index )
      	return (*transport)->getName();
	   count++;
   }
   return NULL;
}

int Session::numConnections( void )
{
   Vector<Transport *>::iterator transport;
   Vector<VC *>::iterator vc;
   int count=0;

   for(transport = transportList.begin(); transport != transportList.end(); transport++)
   {
	   for(vc = (*transport)->VCList.begin(); vc != (*transport)->VCList.end(); vc++)
   	   count++;
   }
   return count;
}

const char *Session::getConnectionName( int index )
{
   Vector<Transport *>::iterator transport;
   Vector<VC *>::iterator vc;
   int count=1;

   for(transport = transportList.begin(); transport != transportList.end(); transport++)
   {
	   for(vc = (*transport)->VCList.begin(); vc != (*transport)->VCList.end(); vc++)
      {
         if( count == index )
         	return (*vc)->getAddressString( );
   	   count++;
      }
   }
   return NULL;
}

/*---------------------------------------------------------------------------*/
/* Transport                                                                 */
/*---------------------------------------------------------------------------*/

Transport::~Transport()
{
   // kill all the VCs in the VCList;
   Vector<VC *>::iterator i;

   for(i = VCList.begin(); i != VCList.end(); i++)
      delete (*i);
}

bool Transport::translateAddress(const char *src, char *dest, int timeout)
{
   if(!strnicmp(src, transportName, strlen(transportName)))
   {
      if(src[strlen(transportName)] != ':')
         return false;
      strcpy(dest, src);
      return true;
   }
   return false;
}

#if 0
VC *Transport::resolveAddress(const char *taString)
{
   Vector<VC *>::iterator i;


   for(i = VCList.begin(); i != VCList.end(); i++)
   {
      if(!stricmp(taString, (*i)->getName()))
         return *i;
   }
   return NULL;
}
#endif

void Transport::deleteVC(VC *vc)
{
   Vector<VC *>::iterator i;
   for(i = VCList.begin(); i != VCList.end(); i ++)
   {
      if(*i == vc)
      {
         VCList.erase(i);
         delete vc;
         break;
      }
   }
}

void Transport::disconnect( void )
{
   // kill all the VCs in the VCList;
   Vector<VC *>::iterator i;

   for(i = VCList.begin(); i != VCList.end(); i++)
      (*i)->disconnect();
}

void Transport::disconnect( VC *vc )
{
	vc->disconnect();
}

VC *Transport::getVCfromAddress(const char *addrString)
{
	Vector<VC *>::iterator i;

	for(i = VCList.begin(); i != VCList.end(); i++)
   {
      if( !strcmp((*i)->getAddressString(), addrString))
      	return (*i);
   }

   return NULL;
}



/*---------------------------------------------------------------------------*/
/* VC                                                                        */
/*---------------------------------------------------------------------------*/

VC::VC(Session *session, Transport *trans)
{
   mySession = session;
   myTransport = trans;
   curState = Unbound;
   avgRTT = 0;
   userData = 0;
}

// note that all vc's on a transport will be deleted before that transport
// and all transports will be destroyed before the session.

VC::~VC()
{
}


};