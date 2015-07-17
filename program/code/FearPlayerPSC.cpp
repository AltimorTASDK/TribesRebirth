#include "sim.h"
#include "FearPlayerPSC.h"
#include "player.h"
#include "feardcl.h"

#include "PlayerManager.h"
#include "fear.strings.h"
#include "netGhostManager.h"
#include "FearGlobals.h"
#include "simAction.h"
#include "console.h"
#include "SimResource.h"
#include <simguicanvas.h>
#include <simguitsctrl.h>
#include <flyingcamera.h>
#include "dataBlockManager.h"
#include "fearGuiTSCommander.h"
#include "sensorManager.h"
#include "fearGuiUnivButton.h"
#include "dlgPlay.h"
#include "observerCamera.h"
#include "sky.h"
#include "fearDcl.h"
#include "simterrain.h"
#include "flier.h"

//----------------------------------------------------------------------------
// PlayerFov and actions are in Degrees

#define MaxPitch		1.34
#define MinCamDist 0.3
#define CameraRadius	0.3

#define CAM_SPEED    15
#define CAM_MAX_DIST 4.0

const float DefaultFov = 90.f;
const float DefaultSniperFov = DefaultFov / 4;
const float MinFovDeg = 5.625f;
const float MaxFovDeg = 120.0f;
const float MaxTurnSpeed = 1.0f;
//

const float DefaultZoomSpeed = 0.2f;
const float MinZoomSpeed = 0.0f;
const float MaxZoomSpeed = 1.0f;

const char *fovPrefVariable = "pref::PlayerFov";
const char *zoomPrefVariable = "pref::PlayerZoomFov";
const char *zoomSpeedPrefVariable = "pref::PlayerZoomSpeed";

float PlayerPSC::playerFov = 0.0f;
float PlayerPSC::sniperFov = 0.0f;
float PlayerPSC::zoomSpeed = 0.0f;
float prefFov = DefaultFov;
static bool sniping = false;

inline float m_clamp(float value,float min,float max)
{
	return (value <= min)? min: (value >= max)? max: value;
}	

const char *PlayerPSC::validateFov(CMDConsole*,int id,int argc, const char *argv[])
{
	id, argc, argv;
   if (prefFov > MaxFovDeg) prefFov = MaxFovDeg;
   if (prefFov < MinFovDeg) prefFov = MinFovDeg;
   if (sniperFov < MinFovDeg) sniperFov = MinFovDeg;
   if (sniperFov > MaxFovDeg) sniperFov = MaxFovDeg;
   if (playerFov < MinFovDeg) playerFov = MinFovDeg;
   if (playerFov > MaxFovDeg) playerFov = MaxFovDeg;
   if (zoomSpeed < MinZoomSpeed) zoomSpeed = MinZoomSpeed;
   if (zoomSpeed > MaxZoomSpeed) zoomSpeed = MaxZoomSpeed;
   
   return 0;
}

PlayerMove::PlayerMove()
{
   reset();
}

void PlayerMove::reset()
{
   forwardAction = 0;
   backwardAction = 0;
   leftAction = 0;
   rightAction = 0;
   jetting = false;
   jumpAction = false;
   crouching = false;
   turnRot = 0;
   pitch = 0;
   trigger = false;
   useItem = -1;
}

PlayerPSC::PlayerPSC(bool in_isServer)
{
   int i;
	memset(shoppingList,0,sizeof(shoppingList));
	memset(buyList,0,sizeof(buyList));
	memset(clientShoppingList,0,sizeof(clientShoppingList));
	memset(clientBuyList,0,sizeof(clientBuyList));

   triggerCount = prevTriggerCount = 0;
   limitCommandBandwidth = false;   
   firstPerson = true;
   camDist = 0;

   yawSpeed = 0;
   pitchSpeed = 0;
   turnLeftSpeed = 0;
   turnRightSpeed = 0;
   lookUpSpeed = 0;
   lookDownSpeed = 0;
   lastSensorSeq = -1;
   lastSent = -8;

   controlObject = NULL;
   controlPlayer = NULL;
   isServer = in_isServer;
   id = PlayerPSCId;
   resetItems = FALSE;
   playerManager = NULL;
   lastPlayerMove = 0;
   firstMoveSeq = 0;

   playerFov = DefaultFov;
   sniperFov = DefaultSniperFov;
   targetFov = prefFov;
   fovStartTime = 0;
   fovTargetTime = 0;
   viewPitch = 0;

   fInObserverMode = false;
   fInEditMode     = false;
   curGuiMode = InitialGuiMode;

	memset(itemTypeCount,0,sizeof(itemTypeCount));

	// The console addVariable methods need to be called
	// in this order, do not change.
   if(!isServer)
   {
      cg.psc = this;
   	Console->addVariable(0,fovPrefVariable, CMDConsole::Float,&prefFov);
   	Console->addVariable(0,fovPrefVariable,validateFov);
   	//Console->addVariable(0,zoomPrefVariable, CMDConsole::Float,&sniperFov);
   	//Console->addVariable(0,zoomPrefVariable,validateFov);
   	Console->addVariable(0,zoomSpeedPrefVariable, CMDConsole::Float,&zoomSpeed);
   	Console->addVariable(0,zoomSpeedPrefVariable,validateFov);
   }
   for(i = 0; i < 8; i++)
      visibleList[i] = 0;
   for(i = 0; i < 128; i++)
      targetNames[i] = NULL;
}

PlayerPSC::~PlayerPSC()
{
   if(!isServer && cg.psc == this)
      cg.psc = 0;
}


//--------------------------------------------------------------------------- 

void PlayerPSC::getFovVars(float *pFov, float *pZoomFov, float *pTargetFov, DWORD *pStartTime, DWORD *pFovTime)
{
   if (pFov) *pFov = playerFov;
   if (pZoomFov) *pZoomFov = sniperFov;
   if (pTargetFov) *pTargetFov = targetFov;
   if (pFovTime) *pStartTime = fovStartTime;
   if (pFovTime) *pFovTime = fovTargetTime;
}

int PlayerPSC::itemCount(int item)
{
   if(item >= 0 && item < Player::MaxItemTypes)
      return itemTypeCount[item];
   return 0;
}

void PlayerPSC::packetDropped(DWORD key)
{
   PacketHead *ph = (PacketHead *) key;
   if(ph->resetItems)
      resetItems = true;

   // clear out any shoppingList changes, so we can recompute them
   int i;
   for(i = 0; i < ItemShoppingListWords; i++) {
      clientShoppingList[i] ^= ph->xorShoppingList[i];
      clientBuyList[i] ^= ph->xorBuyList[i];
   }
   for(i = 0; i < 8; i++)
      visibleList[i] ^= ph->xorVisibleList[i];


   InvUpdate *walk = ph->invUpdates;
   while(walk)
   {
      InvUpdate *next = walk->link;
      itemTypeCount[walk->item] -= walk->delta;
      invLL.free(walk);
      walk = next;
   }
   phLL.free(ph);
}

void PlayerPSC::packetReceived(DWORD key)
{
   PacketHead *ph = (PacketHead *) key;
   InvUpdate *walk = ph->invUpdates;
   while(walk)
   {
      InvUpdate *temp = walk->link;
      invLL.free(walk);
      walk = temp;
   }
   phLL.free(ph);
}


void PlayerMove::write(BitStream *bstream, bool full, PlayerMove *prev)
{
   bool writeInfo = true;
   if(prev)
   {
      writeInfo = !bstream->writeFlag(forwardAction == prev->forwardAction &&
                     backwardAction == prev->backwardAction &&
                     leftAction == prev->leftAction &&
                     rightAction == prev->rightAction &&
                     jetting == prev->jetting &&
                     jumpAction == prev->jumpAction &&
                     crouching == prev->crouching);
   }
   if(writeInfo)
   {
      bstream->writeInt(forwardAction * 15, 4);
      bstream->writeInt(backwardAction * 15, 4);
      bstream->writeInt(leftAction * 15, 4);
      bstream->writeInt(rightAction * 15, 4);
      bstream->writeFlag(jetting);
      bstream->writeFlag(jumpAction);
      bstream->writeFlag(crouching);
   }
   if(full)
   {
      bstream->writeFlag(trigger);
      if(bstream->writeFlag(useItem != -1))
         bstream->writeInt(useItem, 8);
      if(bstream->writeFlag(pitch != 0))
         bstream->write(pitch);
      if(bstream->writeFlag(turnRot != 0))
         bstream->write(turnRot);
   }
}

void PlayerMove::read(BitStream *bstream, bool full, PlayerMove *prev)
{
   bool readInfo = true;
   if(prev)
      readInfo = !bstream->readFlag();
   if(readInfo)
   {
      forwardAction = bstream->readInt(4) / 15.0f;
      backwardAction = bstream->readInt(4) / 15.0f;
      leftAction = bstream->readInt(4) / 15.0f;
      rightAction = bstream->readInt(4) / 15.0f;
      jetting = bstream->readFlag();
      jumpAction = bstream->readFlag();
      crouching = bstream->readFlag();
   }
   else if(prev != this)
   {
      forwardAction = prev->forwardAction;
      backwardAction = prev->backwardAction;
      leftAction = prev->leftAction;
      rightAction = prev->rightAction;
      jetting = prev->jetting;
      jumpAction = prev->jumpAction;
      crouching = prev->crouching;
   }
   if(full)
   {
      trigger = bstream->readFlag();
      if(bstream->readFlag())
         useItem = bstream->readInt(8);
      else
         useItem = -1;
      if(bstream->readFlag())
         bstream->read(&pitch);
      else
         pitch = 0;
      if(bstream->readFlag())
         bstream->read(&turnRot);
      else
         turnRot = 0;
   }
   else
   {
      trigger = false;
      useItem = -1;
      pitch = 0;
      turnRot = 0;
   }
}

PlayerMove *PlayerPSC::getClientMove(DWORD time)
{
   if(moves.size() == 30)
      return NULL;

   int movesBack = ((cg.currentTime-1) >> 5) - (time >> 5);
   if(movesBack >= moves.size())
      return NULL;
   return &moves[moves.size() - 1 - movesBack];

}


void PlayerPSC::clientCollectInput(DWORD startTime, DWORD endTime)
{
   if (firstPerson && camDist)
   {
   	camDist -= CAM_SPEED * (endTime - startTime) * 0.001;
   	if (camDist < 0)
   		camDist = 0;
   }
   else if(!firstPerson && camDist < CAM_MAX_DIST)
	{
		camDist += CAM_SPEED * (endTime - startTime) * 0.001;
		if (camDist > CAM_MAX_DIST)
			camDist = CAM_MAX_DIST;
	}

   if(owner->getStreamMode() != Net::PacketStream::PlaybackMode)
   {
      int numTicks = ((endTime-1) >> 5) - ((startTime-1) >> 5);
      int prevMovesSize = moves.size();
      if(numTicks > 0)
      {
         float pitchAdd = pitchSpeed + lookUpSpeed - lookDownSpeed;
         float yawAdd = - yawSpeed - turnRightSpeed + turnLeftSpeed;

         if(moves.size() < 30)
         {
            curMove.pitch += pitchAdd;
            curMove.turnRot += yawAdd;

            curMove.trigger = false;
            if(triggerCount & 1)
               curMove.trigger = true;
            else if(!(prevTriggerCount & 1) && prevTriggerCount != triggerCount)
               curMove.trigger = true;

            char buf[256];
            BitStream bs(buf, 256);
            curMove.write(&bs);
            bs.setCurPos(0);
            curMove.read(&bs);

            prevTriggerCount = triggerCount;
            moves.push_back(curMove);
         }
         curMove.trigger = triggerCount & 1;
         curMove.pitch = pitchAdd;
         curMove.turnRot = yawAdd;
         curMove.jumpAction = false;
         curMove.useItem = -1;

         // write and read curMove to make sure all the values get
         // clamped properly.

         while(--numTicks && moves.size() < 30)
            moves.push_back(curMove);
         curMove.pitch = 0;
         curMove.turnRot = 0;
      }

      if(owner->getStreamMode() == Net::PacketStream::RecordMode)
      {
         int i;
         int count = moves.size() - prevMovesSize;
         DWORD curtime = ((startTime - 1) & ~0x1F) + 33;
         for(i = 0; i < count; i++)
         {
            StreamIO *sio = owner->getRecorderStream(this, curtime);
            sio->write(BYTE(1));
            sio->write(sizeof(PlayerMove), &moves[prevMovesSize + i]);
            curtime += 32;
         }
      }
   }
   else
   {
      for(int i = 0; i < pmvec.size(); i++)
         moves.push_back(pmvec[i]);
      pmvec.setSize(0);
   }
}

float PlayerPSC::getDamageLevel (void)
{
	if (controlObject)
		return controlObject->getDamageLevel ();
	else
		return 0;
}

float PlayerPSC::getEnergyLevel (void)
{
	if (controlObject)
		return controlObject->getEnergyLevel ();
	else
		return 0;
}

bool PlayerPSC::onSimTimerEvent(const SimTimerEvent *)
{
   return true;
}

PlayerMove *PlayerPSC::getCurrentMove()
{
   if(moves.size() && moves.size() < 30)
      return &moves.last();
   return NULL;
}

const char *PlayerPSC::getTargetName(int sensorKey)
{
   if(sensorKey < 0 || sensorKey > 127 || !targetNames[sensorKey])
      return "";
   return targetNames[sensorKey];
      
}

bool PlayerPSC::writePacket(BitStream *bstream, DWORD &key)
{
   if(isServer)
   {
      PlayerManager::ClientRep *me = sg.playerManager->findClient(getOwner()->getId());

      bstream->writeInt(curGuiMode, 3);
      bstream->writeFlag(me->sendScores);
      bstream->writeInt(lastPlayerMove-1, 32);
      
      GameBase *fo = controlObject;
      if(!bstream->writeFlag(fo != NULL))
         return true;

      // update my commander side waypoint:
      bool wayposChanged = false;

      MissionCenterPos *cpos = (MissionCenterPos *) manager->findObject(MissionCenterPosId);

      if(me)
      {
         if(me->curCommand != -1 && me->curCommandTarget != -1)
         {
            int t = me->curCommandTarget;
            GameBase *obj = NULL;
            if(t < 128)
            {
               PlayerManager::ClientRep *cl = sg.playerManager->findBaseRep(2048 + t);
               if(cl && cl->ownedObject && cl->ownedObject->getVisibleToTeam(me->team))
                  obj = cl->ownedObject;
            }
            else
            {
               SensorManager *rm = (SensorManager *) manager->findObject(SensorManagerId);
               obj = rm->targets[t - 128];
               if(obj && !obj->getVisibleToTeam(me->team))
                  obj = NULL;
            }
            if(obj)
            {
               Point3F pos = obj->getLinearPosition();
               int x = 1023 * (pos.x - cpos->pos.x) / cpos->extent.x;
               int y = 1023 * (pos.y - cpos->pos.y) / cpos->extent.y;
               if(!((x | y) & ~1023))
               {
                  if(me->wayPoint.x != x || me->wayPoint.y != y)
                  {
                     me->wayPoint.x = x;
                     me->wayPoint.y = y;
                     wayposChanged = true;
                  }
               }
            }
         }
      }
      getOwner()->getGhostManager()->setScopeObject(fo);

      if(controlObject)
      {
         controlPlayer = dynamic_cast<Player *>(controlObject);
         if(!controlPlayer)
         {
            ObserverCamera *oc = dynamic_cast<ObserverCamera *>(controlObject);
            if(oc && oc->getOrbitObject())
               controlPlayer = dynamic_cast<Player *>(oc->getOrbitObject());
         }
      }

      if(controlObject)
         bstream->writeFloat(controlObject->getDamageFlash(),8);
      else
         bstream->writeFloat(0.0, 8);
      int focusedObjectId = fo->getGhostedIndex(owner->getGhostManager());

      bstream->writeInt(focusedObjectId, 10);
      if(focusedObjectId != INVALID_GHOST_INDEX)
         fo->writePacketData(bstream);

      int i;
      PacketHead *ph = phLL.alloc();
      for(i = 0; i < ItemShoppingListWords; i++)
      {
         ph->xorShoppingList[i] = 0;
         ph->xorBuyList[i] = 0;
      }
      for(i = 0; i < 8; i++)
         ph->xorVisibleList[i] = 0;

      ph->resetItems = resetItems;
      ph->invUpdates = NULL;

      if(curGuiMode == InventoryGuiMode)
      {
         for(i = 0; i < ItemShoppingListWords; i++)
         {
            ph->xorShoppingList[i] = shoppingList[i] ^ clientShoppingList[i];
            ph->xorBuyList[i] = buyList[i] ^ clientBuyList[i];
            clientShoppingList[i] = shoppingList[i];
            clientBuyList[i] = buyList[i];

            if(ph->xorShoppingList[i] || ph->xorBuyList[i])
            {
               bstream->writeFlag(true);
               bstream->writeInt(i, 4);

   	         if(bstream->writeFlag(ph->xorShoppingList[i] != 0))
   	            bstream->writeInt(ph->xorShoppingList[i], 32);
   	         if(bstream->writeFlag(ph->xorBuyList[i] != 0))
   	            bstream->writeInt(ph->xorBuyList[i], 32);
            }
         }
         bstream->writeFlag(false);
      }
      
      if(bstream->writeFlag(resetItems))
      {
         for(i = 0; i < Player::MaxItemTypes; i++)
            itemTypeCount[i] = 0;
         resetItems = false;
      }

      if(controlPlayer)
      {
         for(i = 0; i < Player::MaxItemTypes; i++)
            if(itemTypeCount[i] != controlPlayer->getItemCount(i)) {
               InvUpdate *iu = invLL.alloc();
               iu->delta = controlPlayer->getItemCount(i) - itemTypeCount[i];
               iu->item = i;
               iu->link = ph->invUpdates;
               ph->invUpdates = iu;
               itemTypeCount[i] += iu->delta;

               bstream->writeFlag(true);
               bstream->writeInt(iu->item,Player::ItemTypeBits);
               bstream->writeSignedInt(iu->delta,10);
            }
      }
      bstream->writeFlag(false);

      key = DWORD(ph);
      SensorManager *rm = (SensorManager *) manager->findObject(SensorManagerId);

      if(curGuiMode == CommandGuiMode)
      {
         int startBlock = (lastSent + 8) & 0xFF;
         lastSent = startBlock;

         bstream->writeInt(startBlock, 8);
         int startPos = bstream->getCurPos();
         for(;;)
         {
            bstream->writeFlag(true);
            if(lastSent < 128)
            {
               // it's a player/owner block:
               for(int i = 0; i < 8; i++)
               {
                  PlayerManager::ClientRep *cl = playerManager->findBaseRep(lastSent + 2048 + i);
                  if(cl && cl->ownedObject && (cl->ownedObject->getVisibleToTeam(me->team) || cl == me))
                  {
                     Point3F pos = cl->ownedObject->getLinearPosition();
                     int x = 1023 * (pos.x - cpos->pos.x + cpos->extent.x/2) / cpos->extent.x;
                     int y = 1023 * (pos.y - cpos->pos.y + cpos->extent.y/2) / cpos->extent.y;
                     bool oob = (x | y) & ~2047;
                     if(bstream->writeFlag(!oob))
                     {
                        bstream->writeInt(x, 11);
                        bstream->writeInt(y, 11);
                        bstream->writeInt(cl->ownedObject->getTeam(), 3);

                        //write the damage level for the players here
								GameBase::CommandStatus status;
								cl->ownedObject->getCommandStatus(&status);
                        bstream->writeInt(status.damageLevel,2);
                        
                        //write the commandStatus of active status
                        int cmdStatus = cl->commandStatus + 1;
                        bstream->writeInt(cmdStatus, 3);
                        
                        int dg = cl->ownedObject->getDatGroup();
                        bstream->writeInt(dg,6);
                        bstream->writeInt(cl->ownedObject->getDatFileId(), wg->dbm->getDataGroupBitSize(dg));
                     }                        
                  }
                  else
                     bstream->writeFlag(false);
               }
            }
            else
            {
               for(int i = 0; i < 8;i++)
               {
                  GameBase *obj = rm->targets[lastSent - 128 + i];
                  if(obj && obj->getVisibleToTeam(me->team))
                  {
                     Point3F pos = obj->getLinearPosition();
                     int x = 1023 * (pos.x - cpos->pos.x + cpos->extent.x/2) / cpos->extent.x;
                     int y = 1023 * (pos.y - cpos->pos.y + cpos->extent.y/2) / cpos->extent.y;
                     bool oob = (x | y) & ~2047;
                     if(bstream->writeFlag(!oob))
                     {
                        bstream->writeInt(x, 11);
                        bstream->writeInt(y, 11);
                        bstream->writeInt(obj->getTeam(), 3);
                        
                        //for now, just write in no damage
								GameBase::CommandStatus status;
								obj->getCommandStatus(&status);
                        bstream->writeInt(status.damageLevel,2);

								bstream->writeFlag(obj->isControlable());
                        bstream->writeFlag(status.active);
                        
                        int dg = obj->getDatGroup();
                        bstream->writeInt(dg, 6);
                        bstream->writeInt(obj->getDatFileId(), wg->dbm->getDataGroupBitSize(dg));
                     }                        
                  }
                  else
                     bstream->writeFlag(false);
               }
            }
            lastSent = (lastSent + 8) & 0xFF;
            // only send 50 bytes of commander info if we're limited
            if(limitCommandBandwidth && bstream->getCurPos() > startPos + 400)
               break;
            if(lastSent == startBlock || bstream->isFull())
               break;
         }
         bstream->writeFlag(false);
         int i;
         for(i = 0; i < 8; i++)
            ph->xorVisibleList[i] = 0;
      }
      else if(curGuiMode == PlayGuiMode)
      {
         if(bstream->writeFlag(wayposChanged)) // bad if packets are lost
         {
            bstream->writeInt(me->wayPoint.x, 10);
            bstream->writeInt(me->wayPoint.y, 10);
         }
         int i;
         for(i = 0; i < 8; i++)
            ph->xorVisibleList[i] = 0;

         for(PlayerManager::BaseRep *walk = sg.playerManager->getBaseRepList(); walk; walk = walk->nextClient)
            if(walk && walk->ownedObject && walk->ownedObject->getVisibleToTeam(me->team))
               ph->xorVisibleList[(walk->id - 2048) >> 5] |= (1 << (walk->id & 0x1F));

         for(i = 0; i < 128; i++) {
            GameBase *obj = rm->targets[i];
            if(obj && obj->getVisibleToTeam(me->team))
               ph->xorVisibleList[4 + (i >> 5)] |= 1 << (i & 0x1F);
         }
         for(i = 0; i < 8; i++)
         {
            ph->xorVisibleList[i] ^= visibleList[i];
            visibleList[i] ^= ph->xorVisibleList[i];

            if(ph->xorVisibleList[i])
            {
               bstream->writeFlag(true);
               bstream->writeInt(i, 3);
               bstream->writeInt(ph->xorVisibleList[i], 32);
            }
         }
         bstream->writeFlag(false);
      }
      return true;
   }
   else
   {
      bstream->writeFloat(playerFov / 135.0, 8);
      bstream->writeInt(firstMoveSeq, 32);
      Vector<PlayerMove>::iterator prev = NULL;
      int j;

      for(j = 0; j < moves.size(); j++)
      {
         bstream->writeFlag(true);
         moves[j].write(bstream, true, prev);
         prev = &moves[j];
      }
      bstream->writeFlag(false);
      return true;
   }
}

void PlayerPSC::readPacket(BitStream *bstream, DWORD currentTime)
{
   if(isServer)
   {
      float fov;
      fov = bstream->readFloat(8) * M_PI * 135.0 / 180.0f;
      if(controlObject)
         controlObject->setFov(fov);
      int move = bstream->readInt(32);
      PlayerMove pm, *pmp = NULL;

      moves.setSize(0);
      int skipCount = lastPlayerMove - move;
      if(skipCount < 0)
      {
         Net::setLastError("Invalid packet.");
         return;
      }
      AssertFatal(skipCount >= 0, "Doh1!");

      while(bstream->readFlag())
      {
         pm.read(bstream, true, pmp);
         pmp = &pm;
         if(skipCount)
            skipCount--;
         else
         {
            moves.push_back(pm);
            lastPlayerMove++;
         }
      }
      if(controlObject && moves.size())
         controlObject->serverUpdateMove(&moves[0], moves.size());
   }
   else
   {
      // get the position and velocity at the last server
      // accepted move.
      int newMode = bstream->readInt(3);
      bool scoresVisible = bstream->readFlag();

      if(curGuiMode == InitialGuiMode && newMode != InitialGuiMode)
         Console->executef(1, "loadPlayGui");

      if(curGuiMode != newMode)
      {
         cg.playDelegate->setCurrentGuiMode(newMode);
         curGuiMode = newMode;
      }
      if(cg.playDelegate)
         cg.playDelegate->setScoresVisible(scoresVisible);
      
      Int32 lastMove = bstream->readInt(32);
      Int32 popMoves = lastMove - firstMoveSeq;

      for(int i = 0; i < pmvec.size(); i++)
         moves.push_back(pmvec[i]);
      pmvec.setSize(0);

      while(popMoves-- > 0)
      {
         firstMoveSeq++;
         moves.pop_front();
      }
      if(!bstream->readFlag())
         return;

      Point3F startPos, startVel, rot;

      damageFlash = bstream->readFloat(8);
      int ghostIndex = bstream->readInt(10);

      // if we have that object, read it out
      if(ghostIndex != 1023)
      {
         GameBase *ghost = (GameBase *) owner->getGhostManager()->resolveGhost(ghostIndex);
         if(controlObject != ghost)
         {
            if(controlObject)
            {
               clearNotify(controlObject);
               controlObject->setPlayerControl(false);
            }
            controlObject = ghost;
            deleteNotify(controlObject);
            controlObject->setPlayerControl(true);
            controlPlayer = dynamic_cast<Player *>(controlObject);
//				firstPerson = true;
//				camDist = 0;
         }
         controlObject->readPacketData(bstream);
         DWORD ct = (currentTime-1) & ~0x1F;

         controlObject->setLastProcessTime(ct - ((moves.size() - 1) << 5));
      }
      else
      {
         if(controlObject)
         {
            clearNotify(controlObject);
            controlObject->setPlayerControl(false);
         }
         controlObject = NULL;
         controlPlayer = NULL;
      }

      if(curGuiMode == InventoryGuiMode)
      {
         // read in shoppingList changes
         
         while(bstream->readFlag())
         {
            int index = bstream->readInt(4);
   			if (bstream->readFlag()) {
   	         DWORD xor = bstream->readInt(32);
   	         shoppingList[index] ^= xor;
   			}
   			if (bstream->readFlag()) {
   	         DWORD xor = bstream->readInt(32);
   	         buyList[index] ^= xor;
   			}
         }
      }      
      // read in inventory changes
      // was it reset?
      if(bstream->readFlag())
      {
         int i;
         for(i = 0; i < Player::MaxItemTypes; i++)
            itemTypeCount[i] = 0;
      }

      while(bstream->readFlag())
      {
			// The client side player's inventory is not updated.
         int index = bstream->readInt(Player::ItemTypeBits);
         itemTypeCount[index] += bstream->readSignedInt (10);
      }

      // read in sensor info (commander stuff)
      if(curGuiMode == CommandGuiMode)
      {
         PlayerManager::ClientRep *me = cg.playerManager->findClient(manager->getId());

         int startBlock = bstream->readInt(8);
         while(bstream->readFlag())
         {
            for(int i = 0; i < 8; i++)
            {
               FearGui::TSCommander::SensorTargetInfo *info = cg.commandTS->targs + startBlock;
               if(bstream->readFlag())
               {
                  info->pos.x = bstream->readInt(11) - 512;
                  info->pos.y = bstream->readInt(11) - 512;
                  info->team = bstream->readInt(3);
                  info->damageStatus = bstream->readInt(2);
                  if(startBlock < 128)
                  {
							info->inUse = false;
	                  info->activeStatus = bstream->readInt(3);
                     info->datGroup = bstream->readInt(6);
                     info->datId = bstream->readInt(wg->dbm->getDataGroupBitSize(info->datGroup));
                     info->playerId = startBlock + 2048;
                  }
                  else
                  {
							info->inUse = bstream->readFlag();
	                  info->activeStatus = bstream->readFlag();
                     info->datGroup = bstream->readInt(6);
                     info->datId = bstream->readInt(wg->dbm->getDataGroupBitSize(info->datGroup));
                     info->name = targetNames[startBlock - 128];
                  }
                  if(i == me->curCommandTarget)
                  {
                     cg.wayPoint = cg.missionCenterPos;
                     cg.wayPoint.x += (info->pos.x * cg.missionCenterExt.x) / 1024.0f;
                     cg.wayPoint.y += (info->pos.y * cg.missionCenterExt.y) / 1024.0f;
                  }
               }
               else
                  info->datId = -1;
               startBlock = (startBlock + 1) & 0xFF;
            }
         }
      }
      else if(curGuiMode == PlayGuiMode)
      {
         // read in waypos changes:
         if(bstream->readFlag())
         {
            int wpx = bstream->readInt(10);
            int wpy = bstream->readInt(10);
            cg.wayPoint = cg.missionCenterPos;
            cg.wayPoint.x += (wpx * cg.missionCenterExt.x) / 1024.0f;
            cg.wayPoint.y += (wpy * cg.missionCenterExt.y) / 1024.0f;
         }
         // read in sensor info (hud stuff)
         while(bstream->readFlag())
         {
            int idx = bstream->readInt(3);
            DWORD xor = bstream->readInt(32);
            visibleList[idx] ^= xor;
         }
      }
   }
}

void PlayerPSC::setControlObject(GameBase *obj)
{
   if(controlObject)
      clearNotify(controlObject);
   controlObject = obj;
   controlPlayer = NULL;
   if(controlObject)
   {
      deleteNotify(controlObject);
      controlPlayer = dynamic_cast<Player *>(controlObject);
   }
}

void PlayerPSC::onDeleteNotify(SimObject *obj)
{
   if(obj == controlObject)
   {
      controlObject = NULL;
      controlPlayer = NULL;
   }
   Parent::onDeleteNotify(obj);
}

bool PlayerPSC::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if(!isServer)
   {
   	addToSet(SimCameraSetId);
      addToSet(SimTimerSetId);

      eventMap = SimResource::loadByName(manager, "player.sae", true);
      if(bool(eventMap))
   	   SimActionHandler::find (manager)->push (this, eventMap);
   }
   playerManager = (PlayerManager *) manager->findObject(PlayerManagerId);

   if(!getId())
      manager->assignId(this);
   return true;
}

bool PlayerPSC::processEvent(const SimEvent *event)
{
   switch(event->type)
   {
      onEvent(SimActionEvent);
      onEvent(SimTimerEvent);
      onEvent(SimGainFocusEvent);
      onEvent(SimLoseFocusEvent);
      case TargetNameEventType:
         TargetNameEvent *tne;
         tne = (TargetNameEvent *) event;
         targetNames[tne->targId] = tne->name;
         return true;
      default:
         return Parent::processEvent(event);
   }
}

void PlayerPSC::processRecorderStream(StreamIO *sio, DWORD /*time*/)
{
   BYTE flag;

   sio->read(&flag);
   if(flag == 0) // it was an action:
   {
      int action;
      float fValue;
      sio->read(&fValue);
      sio->read(&action);
      processAction(action, fValue, SimActionTypeDigital);
   }
   else
   {
      // it was a move
      PlayerMove pm;
      sio->read(sizeof(PlayerMove), &pm);
      moves.push_back(pm);
   }
}

void PlayerPSC::notifyPlaybackOver()
{
   // a demo finished playing back,
   // notify the game via a console call.
   cg.packetStream = 0;
   Console->executef(3, "schedule", "onPlaybackFinished();", "0");
}
DWORD gPlayerZoomChangeTime = 0;

bool PlayerPSC::processAction(int action, float eventValue, int iDevice)
{
   // We will only get a toggle action here in playback mode, which
   // means we are playing back a toggle of the game cam
   if (action == IDACTION_OBSERVER_TOGGLE)
   {
      toggleObserverCamera();
      return (true);
   }
   // If we're in game observer mode, pass the recorded or live action
   // to the game observer cam
   else if (fInObserverMode == true)
   {
      FlyingCamera *objCamera = dynamic_cast<FlyingCamera *>
                                 (manager->findObject("observerCamera"));

      AssertFatal(objCamera, "Could not find observer camera");

      return (objCamera->handleAction(action, eventValue, iDevice));
   }
   // Handle the action locally
   else
   {
	   switch(action)
	   {
			case IDACTION_CENTERVIEW:
				if (controlPlayer)
					{
						if (controlPlayer->mousePitch())
			            curMove.pitch -= controlPlayer->getViewPitch ();
						controlPlayer->setViewPitch (0);
					}
						
				break;
		   case IDACTION_VIEW:
            firstPerson = !firstPerson;
				if (!firstPerson)
				{
					sniping = false;
					targetFov = DefaultFov;
				}
            else
               targetFov = prefFov;
               
			   break;
		   case IDACTION_INC_SNIPER_FOV:
            if (eventValue > 0)
            {
               switch (int(sniperFov))
               {
                  case 45:    sniperFov = 22.5f;   break;
                  case 22:    sniperFov = 11.25f;  break;
                  case 11:    sniperFov = 5.625f;  break;
                  case  5:    sniperFov = 45.0f;   break;
                  default:    sniperFov = 45.0f;   break;
               }
            }
            else
            {
               switch (int(sniperFov))
               {
                  case 45:       sniperFov = 5.625f;  break;
                  case 22:       sniperFov = 45.0f;   break;
                  case 11:       sniperFov = 22.5f;   break;
                  case 5:        sniperFov = 11.25f;  break;
                  default:       sniperFov = 45.0f;   break;
               }
            }
            if(sniping)
            {
               fovStartTime = cg.currentTime;
               fovTargetTime = fovStartTime + zoomSpeed * 1000;
               targetFov = sniperFov;
               startFov = playerFov;
            }
            else
            {
               if (fovTargetTime < cg.currentTime)
                  fovStartTime = cg.currentTime;
               gPlayerZoomChangeTime = cg.currentTime;
            }
            break;
		   case IDACTION_SNIPER_FOV:
            if (eventValue)
            {
               if (!isFirstPerson() || !controlObject || !controlObject->cameraZoomAndCrosshairsEnabled())
                  break;
               targetFov = sniperFov;
               sniping = true;
            }
            else
            {
               if(isFirstPerson())
                  targetFov = prefFov;
               else
                  targetFov = DefaultFov;
               sniping = false;
            }
            startFov = playerFov;
            fovStartTime = cg.currentTime;
            fovTargetTime = fovStartTime + zoomSpeed * 1000;
            break;
	   }
	   return (true);
   }
}

bool PlayerPSC::onSimActionEvent (const SimActionEvent *event)
{
   int action = event->action;
   float eventValue = event->fValue;

   // move actions are just put into curMove...
   // other actions are recorded, etc.

   switch(action)
   {
	   case IDACTION_MOVEBACK:
         curMove.backwardAction = m_clamp(eventValue, 0.0, 1.0);
         break;
	   case IDACTION_MOVEFORWARD:
         curMove.forwardAction = m_clamp(eventValue, 0.0, 1.0);
		   break;
	   case IDACTION_YAW:
         curMove.turnRot += M_PI * m_clamp(eventValue * playerFov / DefaultFov, -1.0, 1.0);
		   break;
	   case IDACTION_PITCH:
			{
				GameBase *fo = getControlObject();
				if (fo && fo->mousePitch())
				{
					float pitchAdd = M_PI * m_clamp(eventValue * playerFov / DefaultFov, -1.0, 1.0);
					if (fo->getType() == VehicleObjectType && Flier::flipYAxis)
	            	curMove.pitch -= pitchAdd;
					else
	            	curMove.pitch += pitchAdd;
				}
			}
         break;
      case IDACTION_RUN:
         if(eventValue < -0.01)
         {
            curMove.backwardAction = 1;
            curMove.forwardAction = 0;
         }
         else if(eventValue > 0.01)
         {
            curMove.forwardAction = 1;
            curMove.backwardAction = 0;
         }
         else
         {
            curMove.forwardAction = 0;
            curMove.backwardAction = 0;
         }
         break;
      case IDACTION_STRAFE:
         if(eventValue < -0.01)
         {
            curMove.leftAction = 0;
            curMove.rightAction = 1;
         }
         else if(eventValue > 0.01)
         {
            curMove.leftAction = 1;
            curMove.rightAction = 0;
         }
         else
         {
            curMove.leftAction = 0;
            curMove.rightAction = 0;
         }
         break;
	   case IDACTION_MOVELEFT:
         curMove.leftAction = m_clamp(eventValue, 0.0, 1.0);
         break;
	   case IDACTION_MOVERIGHT:
         curMove.rightAction = m_clamp(eventValue, 0.0, 1.0);
		   break;
	   case IDACTION_TURNLEFT:
         turnLeftSpeed = m_clamp(eventValue, 0.0, MaxTurnSpeed);
         break;
      case IDACTION_TURNRIGHT:
	      turnRightSpeed = m_clamp(eventValue, 0.0, MaxTurnSpeed);
         break;
      case IDACTION_LOOKUP:                                
         lookUpSpeed = m_clamp(eventValue, 0.0, MaxTurnSpeed);
         break;
      case IDACTION_LOOKDOWN:
         lookDownSpeed = m_clamp(eventValue, 0.0, MaxTurnSpeed);
         break;
      case IDACTION_PITCHSPEED:
			{
				GameBase *fo = getControlObject();
				if (fo)
				{
					if (fo->getType() == VehicleObjectType && Flier::flipYAxis)
			         pitchSpeed = -m_clamp(eventValue, -MaxTurnSpeed, MaxTurnSpeed);
					else
			         pitchSpeed = m_clamp(eventValue, -MaxTurnSpeed, MaxTurnSpeed);
				}
			}
         break;
      case IDACTION_YAWSPEED:
         yawSpeed = m_clamp(eventValue, -MaxTurnSpeed, MaxTurnSpeed);
         break;
	   case IDACTION_CROUCH:
         curMove.crouching = true;
		   break;
	   case IDACTION_TOGGLE_CROUCH:
         curMove.crouching = !curMove.crouching;
		   break;
      case IDACTION_MOVEUP:
         curMove.jumpAction = true;
         break;
	   case IDACTION_STAND:
         curMove.crouching = false;
		   break;
      case IDACTION_JET:
         curMove.jetting = (eventValue != 0);
         break;
	   case IDACTION_FIRE1:
         if(!(triggerCount & 1))
            triggerCount++;
         else
            triggerCount += 2;
         break;
      case IDACTION_BREAK1:
         if(triggerCount & 1)
            triggerCount++;
         else
            triggerCount += 2;
		   break;
      case IDACTION_USEITEM:
         curMove.useItem = int(eventValue);
         break;
      default:
         // If we're recording, save our actions to disk
         if (owner->getStreamMode() == Net::PacketStream::RecordMode)
         {
            StreamIO *sio = owner->getRecorderStream(this, cg.currentTime);
            sio->write(BYTE(0));
            sio->write(event->fValue);
            sio->write(event->action);
         }

         // If the user is attempting to pop up a camera
         if (event->action == IDACTION_OBSERVER_TOGGLE)
         {
            // If we're in playback mode, pop up the recorder observer cam
            if (owner->getStreamMode() == Net::PacketStream::PlaybackMode)
            {
               toggleEditCamera();
            }

            // If we're in game mode, pop up the game observer cam
            else
            {
               toggleObserverCamera();
            }

            return (true);
         }
         // If we are in game mode, we handle all actions
         else if (owner->getStreamMode() != Net::PacketStream::PlaybackMode)
         {
            return (processAction(event->action, event->fValue, event->device));
         }
   }
   return (false);
}

         
bool PlayerPSC::processQuery(SimQuery *query)
{
	switch (query->type)
	{
		case SimCameraQueryType:
		{
         if(!controlObject)
            return false;

			SimCameraQuery* qp   = static_cast<SimCameraQuery*>(query);
         float fov;

         //set the query FOV
         DWORD curTime = cg.currentTime;
         if (fovTargetTime > curTime)
         {
            playerFov = startFov + ((curTime - fovStartTime) / float(fovTargetTime - fovStartTime)) *
                                    (targetFov - startFov);
         }
         else
         {
            if(!firstPerson)
               playerFov = DefaultFov;
            else if(sniping)
               playerFov = sniperFov;
            else
               playerFov = prefFov;
            targetFov = playerFov;
         }
         controlObject->getCameraTransform(camDist, &qp->cameraInfo.tmat);

         fov = playerFov * M_PI / 180.0f;
			// We want this object to be attachable
			qp->cameraInfo.fov       = fov / 2.0f;

         if (isFirstPerson() && dynamic_cast<ObserverCamera*>(controlObject) == NULL)
			   qp->cameraInfo.nearPlane = 0.2;
         else
			   qp->cameraInfo.nearPlane = DEFAULT_NEAR_PLANE;
			qp->cameraInfo.farPlane  = getFarPlane();

//			SimContainer *container = controlObject->getContainer();
//			if(!container)
//			{
//				SimContainer *root = findObject(manager, SimRootContainerId, root);
//				container = root->findOpenContainer(controlObject);
//			}
//
//         if (container)
//         {
//            container->getAlphaBlend( qp->cameraInfo.tmat, 
//                             &qp->cameraInfo.alphaColor,
//                             &qp->cameraInfo.alphaBlend);
//         }
//			else
//			{
//				qp->cameraInfo.alphaBlend = 0;
//				qp->cameraInfo.alphaColor.red = 0;
//				qp->cameraInfo.alphaColor.green = 0;
//				qp->cameraInfo.alphaColor.blue = 0;
//				
//			}
//
//         if(damageFlash)
//         {
//            if(damageFlash > .76)
//               damageFlash = .76;
//            float curAlpha = qp->cameraInfo.alphaBlend;
//            float scale = curAlpha / (curAlpha + damageFlash);
//            qp->cameraInfo.alphaBlend = max(damageFlash, curAlpha);
//            qp->cameraInfo.alphaColor.red = damageFlash / (curAlpha + damageFlash) + qp->cameraInfo.alphaColor.red * scale;
//            qp->cameraInfo.alphaColor.green = qp->cameraInfo.alphaColor.green * scale;
//            qp->cameraInfo.alphaColor.blue = qp->cameraInfo.alphaColor.blue * scale;
//         }

         if (damageFlash != 0.0f) {
            if (damageFlash < 0.0f)
               damageFlash = 0.0f;
            else if (damageFlash > 0.76f)
               damageFlash = 0.76;

            qp->cameraInfo.alphaBlend       = damageFlash;
            qp->cameraInfo.alphaColor.red   = 1.0f;
            qp->cameraInfo.alphaColor.green = 0.0f;
            qp->cameraInfo.alphaColor.blue  = 0.0f;
         } else {
            qp->cameraInfo.alphaBlend       = 0;
            qp->cameraInfo.alphaColor.set(0, 0, 0);
         }

			return true;
		}
	}
	return Parent::processQuery(query);
}

bool PlayerPSC::onSimGainFocusEvent (const SimGainFocusEvent *)
{
	if (bool(eventMap) && owner->getStreamMode() != Net::PacketStream::PlaybackMode)
	{
		SimActionHandler::find (manager)->push (this, eventMap);
		return true;
	}
	return false;
}


bool PlayerPSC::onSimLoseFocusEvent (const SimLoseFocusEvent *)
{
	if (bool(eventMap))
	{
		SimActionHandler::find (manager)->pop (this, eventMap);
		return true;
	}
	return false;
}

void PlayerPSC::toggleObserverCamera()
{
   FlyingCamera  *objCamera;

   // Find the camera
   objCamera = dynamic_cast<FlyingCamera *>
                           (manager->findObject("observerCamera"));

   AssertFatal(objCamera, "Observer camera not found");

   // Keep track of state
   fInObserverMode = !fInObserverMode;

   if (fInObserverMode)
   {
      // Toggle camera's action map
      objCamera->pushActionMap();

      // Only attach to our GUI if the edit camera isn't up
      if (fInEditMode == false)
         objCamera->attachToGUI();

      // Activate the camera
      objCamera->activate(false);
   }
   else
   {
      // Toggle camera's action map
      objCamera->popActionMap();

      // Only change GUI if we're not in edit mode
      if (fInEditMode == false)
      {
         Console->evaluate("switchToPlay();", false);
      }
   }
}

void PlayerPSC::toggleEditCamera()
{
   FlyingCamera *objCamera;

   // Find the camera
   objCamera = dynamic_cast<FlyingCamera *>
                           (manager->findObject("editCamera"));

   AssertFatal(objCamera, "Edit camera not found");

   fInEditMode = !fInEditMode;

   if (fInEditMode)
   {
      // Push the camera's action map
      objCamera->pushActionMap();

      // Edit camera has precedence, so always show its GUI
      objCamera->attachToGUI();

      // And activate the camera
      objCamera->activate(false);
   }
   else
   {
      // Pop the camera's action map
      objCamera->popActionMap();

      // If in observer mode, go to that GUI
      if (fInObserverMode == true)
      {
         objCamera = dynamic_cast<FlyingCamera *>
                                 (manager->findObject("observerCamera"));
         objCamera->attachToGUI();
      }
   }
}


void PlayerPSC::useItem(int typeId)
{
	curMove.useItem = typeId;
}


//----------------------------------------------------------------------------

void PlayerPSC::setItemShoppingList(int type)
{
	if (type == -1)
		memset(shoppingList,-1,sizeof(shoppingList));
	else {
		Item::ItemData* data = static_cast<Item::ItemData*>
			(wg->dbm->lookupDataBlock(type,DataBlockManager::ItemDataType));
		if (data->showInventory)
			shoppingList[type >> 5] |= 1 << (type & 0x1f);
	}
}

void PlayerPSC::clearItemShoppingList(int type)
{
	clearItemBuyList(type);
	if (type == -1)
		memset(shoppingList,0,sizeof(shoppingList));
	else
		shoppingList[type >> 5] &= ~(1 << (type & 0x1f));
}

bool PlayerPSC::isItemShoppingOn(int type)
{
	if (type >= 0 && type < Player::MaxItemTypes)
		return shoppingList[type >> 5] & (1 << (type & 0x1f));
   else
   {
      for (int i = 0; i < ItemShoppingListWords; i++)
      {
         if (shoppingList[i]) return TRUE;
      }
   }
	return false;
}

void PlayerPSC::setItemBuyList(int type)
{
	setItemShoppingList(type);
	if (type == -1)
		memset(buyList,-1,sizeof(buyList));
	else {
		Item::ItemData* data = static_cast<Item::ItemData*>
			(wg->dbm->lookupDataBlock(type,DataBlockManager::ItemDataType));
		if (data->showInventory)
			buyList[type >> 5] |= 1 << (type & 0x1f);
	}
}

void PlayerPSC::clearItemBuyList(int type)
{
	if (type == -1)
		memset(buyList,0,sizeof(buyList));
	else
		buyList[type >> 5] &= ~(1 << (type & 0x1f));
}

bool PlayerPSC::isItemBuyOn(int type)
{
	if (type >= 0 && type < Player::MaxItemTypes)
		return buyList[type >> 5] & (1 << (type & 0x1f));
   else
   {
      for (int i = 0; i < ItemShoppingListWords; i++)
      {
         if (buyList[i]) return TRUE;
      }
   }
	return false;
}

float
PlayerPSC::getFarPlane()
{
   if (cg.manager == NULL)
      return DEFAULT_FAR_PLANE;

   Sky* pSky = (Sky*)cg.manager->findObject(TribesSkyId);
   if (pSky != NULL) {
      // Use the sky distance, plus a tad.   
      return pSky->getDistance() * 1.1f;      
   }

   SimTerrain* pTerrain = (SimTerrain*)cg.manager->findObject(SimTerrainId);
   if (pTerrain != NULL) {
      // Use the terrain visible distance, plus a tad
      return pTerrain->getVisibleDistance() * 1.1f;      
   }

   // else, well, crap.
   return DEFAULT_FAR_PLANE;
}

