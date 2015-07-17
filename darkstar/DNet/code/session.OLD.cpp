#include "dnet.h"

namespace DNet
{

Session::Session()
{
   Transport *newTrans = (Transport *) new UDPTransport(this);
   transportList.push_back(newTrans);
   curConnectSequence = GetTickCount();
}

Session::~Session()
{
   // delete the transport list
   delete transportList[0];
}

DWORD Session::getConnectSequence()
{
   return curConnectSequence++;
}

bool Session::getTransportAddress(const char *src, char *dest)
{
   Vector<Transport *>::iterator i;

   for(i = transportList.begin(); i != transportList.end(); i++)
      if((*i)->getTransportAddress(src, dest))
         return true;
   return false;
}

VC *Session::connect(const char *transportAddress, BYTE *data, int datalen)
{
   Vector<Transport *>::iterator i;

   for(i = transportList.begin(); i != transportList.end(); i++)
   {
      VC *ret = (*i)->newVC(transportAddress);
      if(ret)
      {
         ret->send(data, datalen, 0, RequestConnect);
         return ret;
      }
   }
   return NULL;
}

bool Session::setPort(int port)
{
   Vector<Transport *>::iterator i;
   for(i = transportList.begin(); i != transportList.end(); i++)
   {
      if(!(*i)->setPort(port))
         return false;
   }
   return true;
}

void Session::process()
{
   Vector<Transport *>::iterator i;
   for(i = transportList.begin(); i != transportList.end(); i++)
      (*i)->process();
}

Error Session::send(const char *addrString, BYTE *data, int datalen)
{
   Vector<Transport *>::iterator i;
   for(i = transportList.begin(); i != transportList.end(); i++)
      if((*i)->send(addrString, data, datalen) == NoError)
         return NoError;
   return UnsupportedAddressType;
}

};