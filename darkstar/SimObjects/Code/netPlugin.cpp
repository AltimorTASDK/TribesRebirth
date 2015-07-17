//----------------------------------------------------------------------------


//----------------------------------------------------------------------------


#include <sim.h>

#include "simConsolePlugin.h"
#include <netPlugin.h>
#include <netCSDelegate.h>
#include <netPacketStream.h>
#include "simGame.h"
#include "m_qsort.h"

#define NET_UNKNOWN      (0)
#define NET_SUCCESS      (1)
#define NET_FAILURE      (2)
#define NET_SERVER_FULL  (3)
#define NET_BAD_PASSWORD (4)

enum CallbackID
{
   NetStats,
   
   PlayDemo,
   TimeDemo,
   Connect,
   Disconnect,
   Kick,
   RateChanged,
   LogPacketStats,
   TranslateAddress,
};

//----------------------------------------------------------------------------
void NetPlugin::init()
{
	console->printf("NetPlugin");

   console->addCommand( NetStats, "netStats", this );
   console->addCommand( LogPacketStats, "logPacketStats", this);


   console->addCommand(TranslateAddress, "DNet::TranslateAddress", this);
	console->addCommand(PlayDemo, "playDemo", this);
	console->addCommand(TimeDemo, "timeDemo", this);
   console->addCommand(Connect, "connect", this);
   console->addCommand(Disconnect, "disconnect", this);
   console->addCommand(Kick, "net::kick", this, 1);
   console->addVariable(RateChanged, "pref::PacketFrame",this);
   console->addVariable(RateChanged, "pref::PacketSize",this);
   console->addVariable(RateChanged, "pref::PacketRate",this);
}


//----------------------------------------------------------------------------
void NetPlugin::startFrame()
{
}


//----------------------------------------------------------------------------
void NetPlugin::endFrame()
{
}

struct ClassStat
{
   char className[256];
   int count;
};

static int __cdecl cstatCompare(const void *a, const void *b)
{
   return ((ClassStat *) a)->count - ((ClassStat *) b)->count;
}

//----------------------------------------------------------------------------
const char *NetPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   if ( !manager ) return ( 0 );
   Net::CSDelegate *del;

	switch ( id )
   {
      //--------------------------------------
      case LogPacketStats:
         {
            Vector<ClassStat> v;
            int type, classCt;
            for(type = 0; type < 2; type++)
            {
               v.setSize(0);
               for(classCt = 0; classCt < 2048; classCt++)
               {
                  int count = Net::PacketStream::getStats()->bitAccumulator[type][classCt];
                  Net::PacketStream::getStats()->bitAccumulator[type][classCt] = 0;

                  if(count)
                  {
                     ClassStat cs;
                     cs.count = count;
                     strcpy(cs.className, Persistent::AbstractTaggedClass::lookupClassName(classCt));
                     v.push_back(cs);
                  }
               }

               m_qsort((void *) &v[0], v.size(), sizeof(ClassStat), cstatCompare);
               Vector<ClassStat>::iterator i;
               console->printf(type ? "Receive stats: %d packets" : "Send stats: %d packets",
                     type ? Net::PacketStream::getStats()->totalRecv :
                            Net::PacketStream::getStats()->totalSent);
               for(i = v.begin(); i != v.end(); i++)
                  console->printf("%d %s", i->count, i->className);
            }
            Net::PacketStream::getStats()->totalSent = 0;
            Net::PacketStream::getStats()->totalRecv = 0;

         }
         break;
      case RateChanged:
         // gotta loop through all the managers and find 
         // all the packet streams

         int w;
         for(w = SimGame::SERVER; w < SimGame::N_WORLDS; w++)
         {
            SimManager *man = SimGame::get()->getManager((SimGame::WorldType) w);
            if(man)
            {
               SimSet *set;
               set = (SimSet *) manager->findObject(PacketStreamSetId);
               SimSet::iterator i;
               for(i = set->begin(); i != set->end(); i++)
               {
                  Net::PacketStream *stream = (Net::PacketStream *) (*i);
                  stream->checkMaxRate();
               }
            }
         }
         break;
      case Kick: {
         if(argc != 2 && argc != 3)
         {
            console->printf("kick(playerId, <reason>);");
            break;
         }
         SimManager *serverManager = SimGame::get()->getManager(SimGame::SERVER);
         if (!serverManager)
         {
            console->printf("no server to kick player from");
            break;
         }

         del = (Net::CSDelegate *) serverManager->findObject(SimCSDelegateId);
         if(argc == 2)
            del->kick(atoi(argv[1]), NULL);
         else
            del->kick(atoi(argv[1]), argv[2]);
         break;
      }

      case TranslateAddress:
         
         del = (Net::CSDelegate *) manager->findObject(SimCSDelegateId);
         if(argc != 3 || !del)
            break;
         static char ret[128];
         if(!del->translateAddress(argv[1], ret, atoi(argv[2])))
            return "";
         return ret;
      case Disconnect:
         del = (Net::CSDelegate *) manager->findObject(SimCSDelegateId);
         if(!del)
            console->printf("Disconnect: no CSDelegate.");
         else
            del->simDisconnect();
         break;
      case PlayDemo:
         del = (Net::CSDelegate *) manager->findObject(SimCSDelegateId);
         if(!del)
            console->printf("PlayDemo: no CSDelegate.");
         else
            del->playDemo(argv[1]);

         break;

      case TimeDemo:
         del = (Net::CSDelegate *) manager->findObject(SimCSDelegateId);
         if(!del)
            console->printf("PlayDemo: no CSDelegate.");
         else
            del->timeDemo(argv[1]);
         break;

      case Connect:
         del = (Net::CSDelegate *) manager->findObject(SimCSDelegateId);
         if(!del)
            console->printf("Connect: cannot connect without a CSDelegate.");
         else if(argc == 2)
            if( del->simConnect(argv[1]) )
               break;
         console->printf("Connect: <net_address>");
         break;
      //--------------------------------------
      case NetStats:
      {
         break;
      }
	}
	return 0;
}

