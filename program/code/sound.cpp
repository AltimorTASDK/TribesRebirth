#include "sound.h"
#include "stringTable.h"
#include "fearGlobals.h"
#include "dataBlockManager.h"
#include "tsfx.h"
#include "netGhostManager.h"
#include "feardcl.h"
SoundProfileData::SoundProfileData()
{
   profile.flags = 0;
   profile.baseVolume = 0;
   profile.coneInsideAngle = 90;
   profile.coneOutsideAngle= 180; 
   profile.coneVector.set(0.0f, -1.0f, 0.0f);
   profile.coneOutsideVolume = 0.0f;  
   profile.minDistance = 30.0f;     
   profile.maxDistance = 300.0f;     
}

bool SoundProfileData::preload(ResourceManager *, bool, char errorBuffer[256])
{
   errorBuffer;
   dist = profile.minDistance + (profile.maxDistance - profile.minDistance) * 0.65;
   return true;
}

void SoundProfileData::pack(BitStream *bstream)
{
   SoundProfileData test;
   bstream->writeInt(profile.flags, 6);
   bstream->writeFloat(profile.baseVolume, 10);
   if(bstream->writeFlag(test.profile.coneInsideAngle != profile.coneInsideAngle))
      bstream->writeFloat(profile.coneInsideAngle / 360.0f, 10);
   if(bstream->writeFlag(test.profile.coneOutsideAngle != profile.coneOutsideAngle))
      bstream->writeFloat(profile.coneOutsideAngle / 360.0f, 10);
   if(bstream->writeFlag(profile.coneVector.x != test.profile.coneVector.x || profile.coneVector.y != test.profile.coneVector.y || test.profile.coneVector.z != profile.coneVector.z))
      bstream->writeNormalVector(&profile.coneVector, 10);
   if(bstream->writeFlag(test.profile.coneOutsideVolume != profile.coneOutsideVolume))
      bstream->write(profile.coneOutsideVolume);
   if(bstream->writeFlag(test.profile.minDistance != profile.minDistance))
      bstream->write(profile.minDistance);
   if(bstream->writeFlag(test.profile.maxDistance != profile.maxDistance))
      bstream->write(profile.maxDistance);
}

void SoundProfileData::unpack(BitStream *bstream)
{
   profile.flags = bstream->readInt(6);
   profile.baseVolume = bstream->readFloat(10);
   if(bstream->readFlag())
      profile.coneInsideAngle = bstream->readFloat(10) * 360.0f;
   if(bstream->readFlag())
      profile.coneOutsideAngle = bstream->readFloat(10) * 360.0f;
   if(bstream->readFlag())
      bstream->readNormalVector(&profile.coneVector, 10);
   if(bstream->readFlag())
      bstream->read(&profile.coneOutsideVolume);
   if(bstream->readFlag())
      bstream->read(&profile.minDistance);
   if(bstream->readFlag())
      bstream->read(&profile.maxDistance);
}

SoundData::SoundData()
{
   wavFileName = stringTable.insert("");
   priority = 0;
   profileIndex = -1;
   pro = NULL;
}

bool SoundData::preload(ResourceManager *rm, bool server, char errorBuffer[256])
{
   errorBuffer;

   if(server)
      return true;
   else
   {
      int accum = 0;
      BYTE *buffer = (BYTE *) rm->lock(wavFileName);
      if(buffer)
      {
         // loop through buffer and touch the bytes.
         ResourceObject *ro = rm->find(wavFileName);
         if(ro)
         {
            int len = ro->fileSize;
            int i;
            for(i = 0; i < len; i++)
               accum += buffer[i];
         }
      }
      return bool((accum & 0) +1); // just in case the optimizer is REALLY smart.
   }
}

void SoundData::pack(BitStream *bstream)
{
   bstream->writeString(wavFileName);
   bstream->writeFloat(priority, 6);
   wg->dbm->writeBlockId(bstream, DataBlockManager::SoundProfileDataType, profileIndex);
}

void SoundData::unpack(BitStream *bstream)
{
   wavFileName = bstream->readSTString(wavFileName);
   priority = bstream->readFloat(6);
   profileIndex = wg->dbm->readBlockId(bstream, DataBlockManager::SoundProfileDataType);
}

SFX_HANDLE TSFX::PlayAt(DWORD id, const TMat3F &tmat, const Vector3F &vel)
{
   SoundData *snd = (SoundData *) wg->dbm->lookupDataBlock(id, DataBlockManager::SoundDataType);
   Sfx::Manager *smgr = Sfx::Manager::find(cg.manager);
   if(snd && smgr)
   {
      if(!snd->pro)
         snd->pro = (SoundProfileData *) wg->dbm->lookupDataBlock(snd->profileIndex, DataBlockManager::SoundProfileDataType);
      return smgr->playAt(snd->wavFileName, snd->priority, &(snd->pro->profile), tmat, vel);
   }
   else
      return SFX_INVALID_HANDLE;
}

class LocSoundEvent : public SimEvent
{
   int soundId;
   Point3F vec;
   float vecLen;
   float proMaxDist;
public:
   LocSoundEvent();
   static void Post(int, int, Point3F, float, float);
   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   DECLARE_PERSISTENT(LocSoundEvent);
};

IMPLEMENT_PERSISTENT_TAG(LocSoundEvent, LocSoundEventType);

void LocSoundEvent::Post(int  managerId, int soundId, Point3F vec, float vecLen, float proMaxDist)
{
   LocSoundEvent *evt = new LocSoundEvent;
   evt->address.managerId = managerId;
   evt->soundId = soundId;
   evt->vec = vec;
   evt->vecLen = vecLen;
   evt->proMaxDist = proMaxDist;
   sg.manager->postCurrentEvent(evt);
}

LocSoundEvent::LocSoundEvent()
{
   type = LocSoundEventType;
   // not guaranteed.
}

void LocSoundEvent::pack(SimManager* /*manager*/, Net::PacketStream* /*ps*/, BitStream *bstream)
{
   wg->dbm->writeBlockId(bstream, DataBlockManager::SoundDataType, soundId);
   if(bstream->writeFlag(vecLen > 0.05))
   {
      vec *= 1 / vecLen;
      bstream->writeNormalVector(&vec, 8);
      bstream->writeFloat(vecLen / proMaxDist, 9);
   }
}

void LocSoundEvent::unpack(SimManager *manager, Net::PacketStream* /*ps*/, BitStream *bstream)
{
   if(!verifyNotServer(manager))
      return;
   Sfx::Manager *smgr = Sfx::Manager::find(manager);

   int id = wg->dbm->readBlockId(bstream, DataBlockManager::SoundDataType);
   SoundData *snd = (SoundData *) wg->dbm->lookupDataBlock(id, DataBlockManager::SoundDataType);

   if(!snd->pro)
      snd->pro = (SoundProfileData *) wg->dbm->lookupDataBlock(snd->profileIndex, DataBlockManager::SoundProfileDataType);

   Point3F vec;
   if(!bstream->readFlag())
		vec.set(0,0,0);
	else
   {
      bstream->readNormalVector(&vec, 8);
      float pct = bstream->readFloat(9);
      vec *= (pct * snd->pro->profile.maxDistance);
   }

	if (smgr)
      smgr->playRelative(snd->wavFileName, snd->priority, &(snd->pro->profile), vec);

   address.objectId = -2;
}

void TSFX::serverPlaySound(DWORD id, Point3F pos)
{
   if(!sg.manager)
      return;
   SoundData *snd = (SoundData *) wg->dbm->lookupDataBlock(id, DataBlockManager::SoundDataType);
   if(snd->profileIndex == -1)
      return;

   SoundProfileData *pro = (SoundProfileData *) sg.dbm->lookupDataBlock(snd->profileIndex, DataBlockManager::SoundProfileDataType);

   SimSet *set = (SimSet *) sg.manager->findObject(GhostManagerSetId);
   SimSet::iterator i;
   for(i = set->begin(); i != set->end(); i++)
   {
      Net::GhostManager *gm = (Net::GhostManager *) (*i);
      GameBase *scopeObject = (GameBase *) gm->getScopeObject();
      if(!scopeObject)
         continue;
      Point3F vec = scopeObject->getLinearPosition() - pos;
      float vecLen = vec.len();
      if(vecLen < pro->dist)
         LocSoundEvent::Post(gm->getOwner()->getId(), id, vec, vecLen, pro->profile.maxDistance);
   }
}

void TSFX::serverPlaySound(DWORD id, GameBase *object, int channel)
{
   if(!sg.manager)
      return;

   SoundData *snd = (SoundData *) wg->dbm->lookupDataBlock(id, DataBlockManager::SoundDataType);
   if(snd->profileIndex == -1)
      return;

   SoundProfileData *pro = (SoundProfileData *) sg.dbm->lookupDataBlock(snd->profileIndex, DataBlockManager::SoundProfileDataType);
   Vector<Net::GhostRef>::iterator i;
   for(i = object->ghosts.begin(); i != object->ghosts.end(); i++)
   {
      if(i->ghostInfo->flags & (Net::GhostInfo::KillingGhost | Net::GhostInfo::Ghosting | Net::GhostInfo::NotYetGhosted | Net::GhostInfo::KillGhost))
         continue;
      GameBase *scopeObject = (GameBase *) i->ghostManager->getScopeObject();
      if(!scopeObject)
         continue;

      Point3F vec = scopeObject->getLinearPosition() - object->getLinearPosition();
      float vecLen = vec.len();
      if(vecLen < pro->dist)
      {
         SoundEvent *event = new SoundEvent;
         event->id = id;
         event->ghostIndex = i->ghostInfo->ghostIndex;
         event->channel = channel;
         event->address.managerId = i->ghostManager->getOwner()->getId();
         sg.manager->postCurrentEvent(event);
      }
   }
}
