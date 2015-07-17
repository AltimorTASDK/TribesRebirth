#include <player.h>
#include <feardcl.h>
#include <sensormanager.h>
#include <fearglobals.h>
#include <PlayerManager.h>
#include <dataBlockManager.h>
#include <FearPlayerPSC.h>

IMPLEMENT_PERSISTENT_TAG(TargetNameEvent, TargetNameEventType);

bool gSensorNetworkEnabled = true;

TargetNameEvent::TargetNameEvent()
{
   type = TargetNameEventType;
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
}

void TargetNameEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   manager; ps;
   bs->writeInt(targId, 7);
   bs->writeString(name);
}

void TargetNameEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bs)
{
   if(!verifyNotServer(manager))
      return;
   manager; ps;
   targId = bs->readInt(7);
   name = bs->readSTString(true);
   address.objectId = cg.psc->getId();
}

SensorManager::SensorManager()
{
   targetCount = 0;
   id = SensorManagerId;
   lastPinged = -1;
   lastPlayer = -1;
   int i;
   for(i = 0; i < MaxTargets; i++)
   {
      targets[i] = NULL;
      targetNames[i] = NULL;
   }
   playerSupressorListHead = new Player;
}

SensorManager::~SensorManager()
{
   delete playerSupressorListHead;
}

void SensorManager::addPlayerSupressor(Player *pl)
{
   pl->nextSup = playerSupressorListHead->nextSup;
   pl->prevSup = playerSupressorListHead;
   if(pl->nextSup)
      pl->nextSup->prevSup = pl;
   pl->prevSup->nextSup = pl;
}

bool SensorManager::onAdd()
{
   if (!Parent::onAdd())
      return (false);

   addToSet(SimTimerSetId);
   return (true);
}

void SensorManager::addSensor(Sensor *pSensor)
{
   Sensor *insWalk;
   if(pSensor->isSupressionSensor())
      insWalk = &supressionListHead;
   else
      insWalk = &sensorListHeads[pSensor->getTeam()+1];

   // order of list is:
   // non-suppress pinger, non-supress non-pinger, suppress pinger, supress non-pinger
   // if you're supressed it can stop before supress pinger
   // if you've been detected and pinged it can stop
   // or if you've been detected but not pinged it can stop at supress non-pinger

   if(!pSensor->isSupressableSensor() && pSensor->isPinger())
   {
      // insert at the beginning
   }
   else if(!pSensor->isSupressableSensor() && !pSensor->isPinger())
   {
      while(insWalk->nextSensor && !insWalk->nextSensor->isSupressableSensor() && insWalk->nextSensor->isPinger())
         insWalk = insWalk->nextSensor;
   }
   else if(pSensor->isSupressableSensor() && pSensor->isPinger())
   {
      while(insWalk->nextSensor && !insWalk->nextSensor->isSupressableSensor())
         insWalk = insWalk->nextSensor;
   }
   else
   {
      while(insWalk->nextSensor && !insWalk->nextSensor->isSupressableSensor())
         insWalk = insWalk->nextSensor;
      while(insWalk->nextSensor && insWalk->nextSensor->isPinger())
         insWalk = insWalk->nextSensor;
   }
   pSensor->nextSensor = insWalk->nextSensor;
   pSensor->prevSensor = insWalk;

	if (insWalk->nextSensor)
	   insWalk->nextSensor->prevSensor = pSensor;
   insWalk->nextSensor = pSensor;
}


void SensorManager::removeTarget(int targId)
{
   targetCount--;
   targets[targId] = NULL;
   changeTargetName(targId, NULL);
}

void SensorManager::changeTargetName(int key, const char *name)
{
   if(name && !name[0])
      name = NULL;

   if(name != targetNames[key])
   {
      targetNames[key] = name;
      PlayerManager::ClientRep *i;
      for (i = sg.playerManager->getClientList(); i; i = i->nextClient)
      {
         TargetNameEvent *evt = new TargetNameEvent;
         evt->address.managerId = i->id;
         evt->targId = key;
         evt->name = targetNames[key];
         manager->postCurrentEvent(evt);
      }
   }
}

void SensorManager::newClient(int managerId)
{
   int i;
   for(i = 0; i < MaxTargets; i++)
   {
      if(targetNames[i] != NULL)
      {
         TargetNameEvent *evt = new TargetNameEvent;
         evt->address.managerId = managerId;
         evt->targId = i;
         evt->name = targetNames[i];
         manager->postCurrentEvent(evt);
      }
   }
}

int SensorManager::addTarget(GameBase *targ, const char *name)
{
   int i;
   // players are handled separately
   if(targ->getDatGroup() == DataBlockManager::PlayerDataType)
      return -1;
      
   for(i = 0; i < MaxTargets; i++) {
      if(targets[i] == NULL) {
         targets[i] = targ;
         changeTargetName(i, name);
         targetCount++;
         return i;
      }
   }
   return -1;
}

bool SensorManager::onSimTimerEvent(const SimTimerEvent *)
{
   // always test one gamebase per timer tick

   if(targetCount)
   {
      for(lastPinged = (lastPinged + 1) & MaxTargetsMask;;lastPinged = (lastPinged + 1) & MaxTargetsMask)
      {
         if(targets[lastPinged])
         {
            testObject(targets[lastPinged]);
            break;
         }
      }
   }
   int pCount = sg.playerManager->getNumBaseReps();
   if(pCount)
   {
      // test NumClients / 16 every tick -> gives us approx 1/2 sec refresh on players
      int testCount = pCount >> 4;
      if(!testCount)
         testCount = 1;

      while(testCount--)
      {
         for(lastPlayer = (lastPlayer + 1) & 0x7F;;lastPlayer = (lastPlayer + 1) & 0x7F)
         {
            PlayerManager::ClientRep *cl = sg.playerManager->findBaseRep(2048 + lastPlayer);
            if(cl)
            {
               if(cl->ownedObject)
                  testObject(cl->ownedObject);
               break;
            }
         }
      }
   }
   return true;
}

void SensorManager::testObject(GameBase *obj)
{
   PlayerManager *mgr = sg.playerManager;
   Sensor *walk;

   //intialize the value
   obj->setSensorPinged(GameBase::PingStatusClear);

   int iTeam;
   if(!gSensorNetworkEnabled)
   {
      for(iTeam = 0; iTeam < mgr->getNumTeams(); iTeam++)
         obj->setVisibleToTeam(iTeam, false);
      return;
   }

   // Cycle through all teams
   int visTeam = obj->getTeam();
   for(iTeam = 0; iTeam < mgr->getNumTeams(); iTeam++)
      obj->setVisibleToTeam(iTeam, iTeam == visTeam);

   // check for supression
   bool supressed = false;
   for(walk = supressionListHead.nextSensor; walk;walk = walk->nextSensor)
      if(walk->detectItem(obj))
      {
         //if we're in a supressions field, set the value
         obj->setSensorPinged(GameBase::PingStatusSupressed);
         supressed = true;
         break;
      }


   Player *plwalk;
   for(plwalk = playerSupressorListHead->nextSup; plwalk; plwalk = plwalk->nextSup)
      if((obj->getLinearPosition() - plwalk->getLinearPosition()).lenf() < plwalk->getSupressionField())
      {
         //if we're in a supressions field, set the value
         obj->setSensorPinged(GameBase::PingStatusSupressed);
         supressed = true;
         break;
      }

   for (iTeam = 0; iTeam < mgr->getNumTeams(); iTeam ++)
   {
      if (iTeam == obj->getTeam())
         continue;
      else
      {
         // Cycle through all sensors for this team
         bool vis = false;

         for (walk = sensorListHeads[iTeam+1].nextSensor; walk; walk = walk->nextSensor)
         {
            if(supressed && walk->isSupressableSensor())
               break;

            // See if we can see the object
            if (walk->detectItem(obj))
            {
               vis = true;
               // Set its vis bit
               if(walk->isPinger())
               {
                  // If at least one ping sensor can see it, we're done with this team
                  obj->setSensorPinged(GameBase::PingStatusOn);
                  break;
               }
            }
         }
         obj->setVisibleToTeam(iTeam, vis);
      }
   }

   // Now cycle through all player image enhancers
   PlayerManager::BaseRep * rep;
   for (rep = mgr->getBaseRepList(); rep; rep = rep->nextClient)
      if (!obj->getVisibleToTeam(rep->team) && rep->ownedObject)
      {
         Player *player = dynamic_cast<Player *>(rep->ownedObject);
         if(player && player->detectItem(obj))
            obj->setVisibleToTeam(rep->team, true);
      }

}

bool SensorManager::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimTimerEvent);
   }
   return (false);
}