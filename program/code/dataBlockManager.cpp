#include <gamebase.h>
#include <datablockmanager.h>
#include <feardcl.h>
#include <staticbase.h>
#include <player.h>
#include <item.h>
#include <console.h>
#include <consoleInternal.h>
#include <moveable.h>
#include <sensor.h>
#include <vehicle.h>
#include <flier.h>
#include <projectile.h>
#include <projBullet.h>
#include <projRocketDumb.h>
#include <turret.h>
#include <projGrenade.h>
#include <projLaser.h>
#include <projTargetLaser.h>
#include <projLightning.h>
#include <projRepair.h>
#include <sound.h>
#include <explosion.h>
#include <debris.h>
#include <mine.h>
#include <marker.h>
#include <projSeekMissile.h>
#include <trigger.h>
#include <fearglobals.h>
#include <fearguiircjoincontrol.h>
#include <netGhostManager.h>

static const char *dataNames[DataBlockManager::NumDataTypes] = {
   "Sound Profile",
   "Sound",
   "Damage Skin",
   "Armor",
   "Static Shape",
   "Item",
   "Item Image",
   "Moveable",
   "Sensor",
   "Vehicle",
   "Flier",
   "Tank",
   "Hover",
   "Projectile",
   "Bullet",
   "Grenade",
   "Rocket",
   "Laser",
   "Interior Shape",
   "Turret",
   "Explosion",
   "Marker",
   "Debris",
   "Mine",
   "Target Laser",
   "Seeking Missile",
   "Trigger",
   "Car",
   "Lightning",
   "Repair Effect",
   "IRC Channel",
};

IMPLEMENT_PERSISTENT_TAG(DataBlockEvent, DataBlockEventType);

static int CountBits(int in_val)
{
	int ret = 0;
	while(in_val)
	{
		ret++;
		in_val >>= 1;		
	}
	return ret;
}

DataBlockEvent::DataBlockEvent()
{
   flags.set(SimEvent::Ordered | SimEvent::Guaranteed);
   type = DataBlockEventType;
   data = NULL;
}

void DataBlockEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   manager;
   ps;

   bstream->writeInt(group, 6);
   bstream->writeInt(gsize, 8);
   bstream->writeInt(block, 8);
   if(data)
      data->pack(bstream);
}

void DataBlockEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   if(!verifyNotServer(manager))
      return;
   ps;

   DataBlockManager *dbm = (DataBlockManager *) manager->findObject(DataBlockManagerId);

   group = bstream->readInt(6);
   gsize = bstream->readInt(8);
   block = bstream->readInt(8);

   if(block != 255)
   {
      data = dbm->createDataBlock(group);
      data->unpack(bstream);
   }
   else
      block = -1;
   address.objectId = DataBlockManagerId;
}

DataBlockManager::DataBlockManager(bool in_isServer)
{
   isServer = in_isServer;
   curBlock = 0;
   curGroup = -1;
   numBlocksRemaining = 0;
   lastDataBlock = NULL;
   lastDataBlockName = NULL;

   if(isServer)
      sg.dbm = this;
   else
      cg.dbm = this;
   for(int i = 0; i < HashTableSize; i++)
      hashTable[i] = NULL;
}

DataBlockManager::~DataBlockManager()
{
   reset();
}

int DataBlockManager::getDataGroupBitSize(int group)
{
   AssertFatal(group >= 0 && group < NumDataTypes, "Invalid data group.");

   return CountBits(dataBlocks[group].size()+1);
}

void DataBlockManager::reset()
{
   curBlock = 0;
   numBlocksRemaining = 0;
   curGroup = -1;

   int i;
   for(i = 0; i < NumDataTypes; i++)
   {
      int j;
      for(j = 0; j < dataBlocks[i].size(); j++)
         delete dataBlocks[i][j];
		dataBlocks[i].clear();
   }
   for(i = 0; i < HashTableSize; i++)
      hashTable[i] = NULL;
}

void DataBlockManager::sendDataToClient(int clientManagerId)
{
   int i;
   for(i = 0; i < NumDataTypes; i++)
   {
      int j;
      if(dataBlocks[i].size())
      {
         for(j = 0; j < dataBlocks[i].size(); j++)
         {
            DataBlockEvent *evt;
            evt = new DataBlockEvent;
            evt->data = dataBlocks[i][j];
            evt->group = i;
            evt->gsize = dataBlocks[i].size();
            evt->block = j;
            evt->address.managerId = clientManagerId;
            manager->postCurrentEvent(evt);
         }
      }
      else
      {
         DataBlockEvent *evt;
         evt = new DataBlockEvent;
         evt->data = NULL;
         evt->group = i;
         evt->gsize = 0;
         evt->block = 255;
         evt->address.managerId = clientManagerId;
         manager->postCurrentEvent(evt);
      }
   }
}

GameBase::GameBaseData *DataBlockManager::createDataBlock(int blockType)
{
   switch(blockType)
   {
		case MineDataType:
			return new Mine::MineData;
		case TurretDataType:
			return new Turret::TurretData;
//		case TankDataType:
//			return new Tank::TankData;
		case FlierDataType:
			return new Flier::FlierData;
		case VehicleDataType:
			return new Vehicle::VehicleData;
      case PlayerDataType:
         return new Player::PlayerData;
      case StaticShapeDataType:
         return new StaticShape::StaticShapeData;
      case ItemDataType:
         return new Item::ItemData;
      case ItemImageDataType:
         return new Player::ItemImageData;
      case MoveableDataType:
         return new Moveable::MoveableData;
      case SensorDataType:
         return new Sensor::SensorData;

      case ProjectileDataType:
         return new Projectile::ProjectileData;
		case BulletDataType:
         return new Bullet::BulletData;
		case GrenadeDataType:
         return new Grenade::GrenadeData;
		case LaserDataType:
         return new LaserProjectile::LaserData;
		case TargetLaserDataType:
         return new TargetLaser::TargetLaserData;
      case RocketDataType:
         return new RocketDumb::RocketDumbData;
      case SeekingMissileDataType:
         return new SeekingMissile::SeekingMissileData;
      case LightningDataType:
         return new Lightning::LightningData;
      case RepairEffectDataType:
         return new RepairEffect::RepairEffectData;

      case SoundProfileDataType:
         return new SoundProfileData;
      case SoundDataType:
         return new SoundData;
      case ExplosionDataType:
         return new Explosion::ExplosionData;
      case DebrisDataType:
         return new Debris::DebrisData;
		case MarkerDataType:
         return new Marker::MarkerData;
      case DamageSkinDataType:
         return new ShapeBase::DamageSkinData;
      case TriggerDataType:
			return new Trigger::TriggerData;
      case IRCChannelType:
         return new FearGui::FGIRCJoinControl::Channel;
//      case CarDataType:
//			return new Car::CarData;
   }
   return NULL;
}

bool DataBlockManager::serverPreload()
{
   bool ret = true;
   char errorBuffer[256];

   for(int i = 0; i < NumDataTypes; i++)
   {
      for(int j = 0; j < dataBlocks[i].size(); j++)
      {
         if(!dataBlocks[i][j]->preload(sg.resManager, true, errorBuffer))
         {
            Console->printf("%s", errorBuffer);
            ret = false;
         }
      }
   }
   return ret;
}

bool DataBlockManager::processEvent(const SimEvent *event)
{
   char errorBuffer[256];

   if(event->type != DataBlockEventType)
      return Parent::processEvent(event);
   DataBlockEvent *evt = (DataBlockEvent *) event;
   if(dataBlocks[evt->group].size() == 0)
   {
      dataBlocks[evt->group].setSize(evt->gsize);
      for(int i = 0; i < evt->gsize; i++)
         dataBlocks[evt->group][i] = NULL;
   }
   if(evt->block != -1)
   {
      dataBlocks[evt->group][evt->block] = evt->data;
      if(!evt->data->preload(cg.resManager, false, errorBuffer))
      {
         Net::setLastError(errorBuffer);  // this should disconnect us
         return true;
      }
      evt->data->dbmIndex = evt->block;
      evt->data->dbmDatFileName = NULL;
      char pctBuffer[24];
      float pct = (evt->group * evt->gsize + evt->block) 
                  / float(NumDataTypes * evt->gsize);
      sprintf(pctBuffer, "%g", pct);
      Console->executef(3, "dataGotBlock", dataNames[evt->group], pctBuffer);
   }
   if(evt->block == (evt->gsize - 1) && (evt->group == NumDataTypes - 1))
      Console->executef(1, "dataFinished");
   return true;
}

GameBase::GameBaseData *DataBlockManager::lookupDataBlock(int blockId, int groupId)
{
	if (blockId >= 0 && blockId < dataBlocks[groupId].size())
		return dataBlocks[groupId][blockId];
   return 0;
}

const char* DataBlockManager::lookupBlockName(int blockId, int groupId)
{
	if (blockId >= 0 && blockId < dataBlocks[groupId].size())
   	return dataBlocks[groupId][blockId]->dbmDatFileName;
	return 0;
}

GameBase::GameBaseData *DataBlockManager::lookupDataBlock(const char *name, int groupId, int &blockId)
{
   for(int i = 0; i < dataBlocks[groupId].size(); i++)
   {
      if (stricmp(dataBlocks[groupId][i]->dbmDatFileName, name) == 0) {
         blockId = i;
         return dataBlocks[groupId][i];
      }
   }
	blockId = -1;
   return NULL;
}

GameBase::GameBaseData *DataBlockManager::lookupDataBlock(const char *name)
{
   int hashIndex = ( ((DWORD)name) >> 2) & HashTableMask;
   for(GameBase::GameBaseData *walk = hashTable[hashIndex]; walk; walk = walk->dbmNextHash)
      if(walk->dbmDatFileName == name)
         return walk;
   return NULL;
}

void DataBlockManager::createBlock(const char *name, int type, void *classPtr)
{
   GameBase::GameBaseData *dat = createDataBlock(type);
   dat->pluginClass = classPtr;
   dat->dbmDatFileName = stringTable.insert(name);
   dat->dbmIndex = dataBlocks[type].size();

   dataBlocks[type].push_back(dat);
   int hashIndex = ( ((DWORD)dat->dbmDatFileName) >> 2) & HashTableMask;

   dat->dbmNextHash = hashTable[hashIndex];
   hashTable[hashIndex] = dat;
}