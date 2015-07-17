#include "dnet.h"
#include "stdio.h"
#include "signal.h"

DNet::Session *sess;
DNet::UDPVC *vc = NULL;

void onReceive(DNet::Session *, DNet::VC *conn, const char *transportAddress, DNet::Packet *pack)
{
   printf("got packet: %d\n", pack->appDataPtr[0]);
   sess->freePacket(pack);
}

void onConnection(DNet::Session *, DNet::VC *conn, DNet::Session::ConnectionNotifyType ct, DNet::Packet *pack)
{
   switch(ct)
   {
      case DNet::Session::Connected:
         printf("Connected.\n");
         break;
      case DNet::Session::ConnectionRequested:
         printf("Connect requested\n");
         sess->acceptConnection(conn, NULL);
         vc = (DNet::UDPVC *) conn;
         break;
      case DNet::Session::Disconnected:
         printf("Client %s dropped\n", conn->getTransportAddress());
         vc = NULL;
         break;
      case DNet::Session::TimedOut:
         printf("Client %s timed out\n", conn->getTransportAddress());
         vc = NULL;
         break;
   }
}

void onNotify(DNet::Session *, DNet::VC *, DWORD notifyKey, bool received)
{
   printf("Notify %d, %d\n", notifyKey, received);
}

void _USERENTRY _EXPFUNC cleanup(int sig)
{
   printf("Cleaning up...\n");
   delete sess;
   exit(0);
}

void main()
{
   sess = new Session(25025);
   
   signal(SIGINT, cleanup);
   signal(SIGBREAK, cleanup);

   sess->onReceive = onReceive;
   sess->onNotify = onNotify;
   sess->onConnection = onConnection;

   for(;;)
   {
      static int i = 1;
      sess->process();
      if(vc && vc->getCurrentState() == DNet::VC::Connected && !vc->windowFull())
      {
         if(!vc->windowFull())
         {
            Packet *p = sess->newPacket();
            p->appDataPtr[0] = i & 0xFF;
            p->appDataSize = 1;
            printf("Send: %d\n", i);
            vc->send(p, DNet::Packet::NotGuaranteed, i);
            i++;
         }
         else
         {
            printf("hmmm\n");
         }
      }
      //Sleep(20);
   }
}