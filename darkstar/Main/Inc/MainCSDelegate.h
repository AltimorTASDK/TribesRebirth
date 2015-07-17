#ifndef _H_MAINCSDELEGATE
#define _H_MAINCSDELEGATE

#include "netCSDelegate.h"

class DLLAPI MainCSDelegate : public Net::CSDelegate
{
   typedef Net::CSDelegate Parent;
   MainPlayerManager *playerManager;
public:
   bool processArguments(int argc, const char **argv);
   void addStreamClients(Net::PacketStream *ps);

   void simDisconnect();
   bool simConnect(const char *transportAddress);

   void onConnectionRequested(DNet::VC *vc, BYTE *data, int datalen);
   void onConnectionAccepted(DNet::VC *vc, BYTE *data, int datalen);
   void onConnectionDropped(DNet::VC *vc, BYTE *data, int datalen, bool timeout);
   void onGameInfoPacket(const char *transportAddress, BYTE *data, int datalen);

   bool processEvent(const SimEvent *evt);
   void queryServer(const char *transportAddress);

   DECLARE_PERSISTENT(MainCSDelegate);
};

#endif