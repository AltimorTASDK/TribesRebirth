#include "netCSDelegate.h"
#include "netPacketStream.h"
#include "netGhostManager.h"
#include "MainPlayerManager.h"
#include "MainDcl.h"
#include "console.h"
#include "MainCSDelegate.h"

IMPLEMENT_PERSISTENT(MainCSDelegate);

const char *mainVersion = "Main V. 0.6";

bool MainCSDelegate::processArguments(int argc, const char **argv)
{
   if( Parent::processArguments(argc, argv) )
   {
      playerManager = new MainPlayerManager(isServer);
      manager->addObject(playerManager, MainPlayerManagerId);
      return true;
   }
   return false;
}

bool MainCSDelegate::simConnect(const char *transportAddress)
{
   if(isServer)
      return false;

   CMDConsole *con = CMDConsole::getLocked();
   const char *playerName = con->getVariable("Name");

   BYTE packet[DNet::MaxPacketSize];

   BitStream bstream(packet, DNet::MaxPacketSize);
   bstream.writeString(mainVersion);
   bstream.writeString(playerName);
   int datalen = bstream.getBytePtr() - packet;

   return (connect(transportAddress, packet, datalen) != NULL);
}

void MainCSDelegate::onConnectionRequested(DNet::VC *vc, BYTE *data, int datalen)
{
   char *rejectString = NULL;
   if(isServer)
   {
      char buf[256];

      BitStream bstream(data, datalen);
      bstream.readString(buf);

      if(strcmp(buf, mainVersion))
         rejectString = "Wrong game or version.";
      else
      {
         acceptConnection(vc);
         // add the player / name stuff
         
         bstream.readString(buf);
         Net::PacketStream *pstream = (Net::PacketStream *) vc->userData;
         playerManager->playerAdded(pstream->getId(), buf);
      }
   }
   else
      rejectString = "Cannot connect to a client.";
   if(rejectString)
      vc->reject((unsigned char *)rejectString, strlen(rejectString) + 1);
}

void MainCSDelegate::addStreamClients(Net::PacketStream *ps)
{
   Parent::addStreamClients(ps);

   Net::GhostManager *gm = new Net::GhostManager;
   manager->addObject(gm);
   ps->addObject(gm);
   ps->assignName(gm, "GhostManager");

   gm->activate();
}

void MainCSDelegate::onConnectionAccepted(DNet::VC *vc, BYTE *data, int datalen)
{
   // build the packet stream and stuff...
   Parent::onConnectionAccepted(vc, data, datalen);

   // packet stream is set up...
   // reset the playerManager

   playerManager->reset();
}

bool MainCSDelegate::processEvent(const SimEvent *evt)
{
   switch(evt->type)
   {
      case NetGhostAlwaysDoneEventType:
         // the ghost manager is done ghosting...
         // time to create the player object.

         if(isServer)
         {
            int id = ((NetGhostAlwaysDoneEvent *) evt)->ghostManager->getOwner()->getId();
            playerManager->dropInGame(id);
         }
         return true;
      default:
         return Parent::processEvent(evt);
   }
}

void MainCSDelegate::simDisconnect()
{
   Parent::simDisconnect();
   playerManager->reset();
}

void MainCSDelegate::onConnectionDropped(DNet::VC *vc, BYTE *data, int datalen, bool timeout)
{
   Net::PacketStream *pstream = (Net::PacketStream *) vc->userData;
   // get rid of the player object...

   if(isServer)
   {
      AssertFatal(pstream, "Ack Null pstream - how'd that get there?");
      playerManager->playerDropped(pstream->getId());
   }
   else
   {
      // renumber the manager on the client

      SimGroupObjectIdEvent event1( 0 );
      manager->processEvent( &event1 );

      playerManager->reset();      
#ifdef NO_BUILD   
      if(timeout && !pstream)
      {
         // con dump that connect failed.
         CMDConsole *con = CMDConsole::getLocked();
         con->printf("Connect request to %s timed out.\n", vc->getTransportAddress());
      }
#endif
   }
   Parent::onConnectionDropped(vc, data, datalen, timeout);
}

void MainCSDelegate::onGameInfoPacket(const char *transportAddress, BYTE *data, int datalen)
{
   transportAddress, data, datalen;
#ifdef NO_BUILD   
   CMDConsole *console = CMDConsole::getLocked();

   // game info packets:
   // 00 00 <version>: game info request
   // 00 01 <version>: server info response
   // 00 02 <version>: server list response
   // 00 03 <version>: fear query
   // 00 04 <version>: fear query response

   if(data[0] != 0)
      return;
   if(isServer)
   {
      // if it's a server info request...
      if(data[1] == GameServerQuery)
      {
         console->printf("Got server info request from %s.", transportAddress);

         const char *serverName = console->getVariable("Server::Hostname");

         BYTE reply[DNet::MaxPacketSize];

         BitStream bstream(reply, DNet::MaxPacketSize);

         bstream.write(BYTE(0));
         bstream.write(BYTE(GameServerResponse));
         bstream.write(BYTE(data[2])); // write out the sequence
         bstream.write(BYTE(0));

         bstream.writeString(mainVersion);
         bstream.writeString(serverName);

         bstream.write(playerManager->playerList.size());
         int replySize = bstream.getBytePtr() - reply;
         send(transportAddress, reply, replySize);
      }
      else
         console->printf("Unknown GameInfo request from %s.", transportAddress);
   }
   else
   {
      // if it's a server info response...
      if(data[1] == MasterServerList)
         Parent::onGameInfoPacket(transportAddress, data, datalen);
      else if(data[1] == GameServerResponse && datalen > 2)
      {
         // validate that it's a fear server
         char buf[256];

         BitStream bstream(data + 4, datalen - 4);
         bstream.readString(buf);
         if(!strcmp(buf, mainVersion))
            Parent::onGameInfoPacket(transportAddress, data, datalen);
      }
   }
#endif
}
