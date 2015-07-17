/*************************************************************************
 * RADAR.CPP
 * 
 *************************************************************************/

#ifdef __BORLANDC__
#pragma warn -inl
#pragma warn -aus
#elif defined(_MSC_VER)
#pragma warning(disable : 4101)
#pragma warning(disable : 4710)
#endif

#include <player.h>
#include <sensor.h>
#include <sensormanager.h>
#include <console.h>
#include <simresource.h>
#include <feardcl.h>
#include <datablockmanager.h>

Sensor::SensorData::SensorData() : StaticBaseData()
{
   // Default sensor is sensor (it had to be something)
   fCastLOS         = false;
   iRange           = 100;
   iDopplerVelocity = 0;
   supression = false;
   supressable = true;
	pinger = true;
}

void Sensor::SensorData::pack(BitStream *stream)
{
   Parent::pack(stream);

   stream->write(iRange);
   stream->write(iDopplerVelocity);
   stream->writeFlag(supression);
	stream->writeFlag (pinger);	// needed on client?
}

void Sensor::SensorData::unpack(BitStream *stream)
{
   Parent::unpack(stream);

   stream->read(&iRange);
   stream->read(&iDopplerVelocity);
   supression = stream->readFlag();
	pinger = stream->readFlag ();
}

Sensor::Sensor()
{
   nextSensor = NULL;
   prevSensor = NULL;
	sensorData = 0;
}

Sensor::~Sensor()
{
}

bool Sensor::onAdd()
{
   if(isGhost())
      addToSet(SensorVisibleSetId);
   return Parent::onAdd();
}

void Sensor::setTeam(int teamId)
{
   Parent::setTeam(teamId);

   if(isGhost())
      return;

   // remove us from any linked lists we might belong to.
   if(nextSensor)
      nextSensor->prevSensor = prevSensor;
   if(prevSensor)
      prevSensor->nextSensor = nextSensor;
   nextSensor = prevSensor = NULL;

   if(manager)
   {
      SensorManager *radMgr = dynamic_cast<SensorManager *>
                            (manager->findObject(SensorManagerId));

      AssertFatal(radMgr, "Sensor cannot locate sensor manager");
      radMgr->addSensor(this);
   }
}

void Sensor::onRemove()
{
   // remove us from any linked lists we might belong to.
   if(nextSensor)
      nextSensor->prevSensor = prevSensor;
   if(prevSensor)
      prevSensor->nextSensor = nextSensor;

   if (!isGhost())
      scriptOnRemove();

   Parent::onRemove();
}

int Sensor::getDatGroup()
{
   return (DataBlockManager::SensorDataType);
}

bool Sensor::initResources(GameBase::GameBaseData *gbData)
{
   if (false == Parent::initResources(gbData))
   {
      return false;
   }

   sensorData = dynamic_cast<SensorData *>(gbData);

   // Build sensitivity box
   buildSensorBox();

   // Clients are only representations of sensor ... all functionality
   // resides on the server
   if (!isGhost())
   {
      if(nextSensor)
         nextSensor->prevSensor = prevSensor;
      if(prevSensor)
         prevSensor->nextSensor = nextSensor;
      nextSensor = prevSensor = NULL;

      // Add ourselves to the sensor manager
      SensorManager *radMgr = dynamic_cast<SensorManager *>
                            (manager->findObject(SensorManagerId));

      AssertFatal(radMgr, "Sensor cannot locate sensor manager");
      radMgr->addSensor(this);
   }
   return (true);
}

void Sensor::buildSensorBox()
{
   Point3F pos;

   pos = getPos();

   // Define our sensitivity box by adding range to position
   sensorBox.fMin.set(pos.x - sensorData->iRange, 
      pos.y - sensorData->iRange, pos.z - sensorData->iRange);
   sensorBox.fMax.set(pos.x + sensorData->iRange, 
      pos.y + sensorData->iRange, pos.z + sensorData->iRange);
}

bool Sensor::detectItem(GameBase *obj)
{
	if (!isActive())
		return false;
   
   // Get distance vector, uses the center of each bounding box.
   const Box3F &box = obj->getBoundingBox();
   Point3F position = (box.fMin + box.fMax) / 2.0f;
   Point3F distance = position - getLinearPosition();

   // If they are within our range, continue ...
   if (distance.lenf() <= sensorData->iRange)
   {
      // If this is not a velocity-sensitive sensor, or the object's
      // velocity is greater than or equal to the sensor's sensitivity,
      // continue ...
      if (sensorData->iDopplerVelocity == 0 ||
          sensorData->iDopplerVelocity <= obj->getLinearVelocity().lenf())
      {
         // If this sensor requires an LOS check, continue ...
         if (sensorData->fCastLOS) {
         	SimObject* los = checkLOS(position);
            return !los || los == obj;
         }
         return (true);
      }
   }

   return (false);
}

SimObject* Sensor::checkLOS(Point3F &position)
{
   // Build query, tests from the center of our bounding
	// box to the given position.
   SimContainerQuery  query;
   query.id = getId();
   query.type = -1;
   query.mask = SimTerrainObjectType  | 
					SimInteriorObjectType  | 
					StaticObjectType       | 
					VehicleObjectType  | 
					MoveableObjectType;
   query.detail = SimContainerQuery::DefaultDetail;
   const Box3F &box = getBoundingBox();
   query.box.fMin = (box.fMin + box.fMax) * 0.5f;
   query.box.fMax = position;

	//
   SimCollisionInfo info;
   SimContainer *root = findObject(manager, SimRootContainerId, root);
	root->findLOS(query, &info);
	return info.object;
}

DWORD Sensor::packUpdate(Net::GhostManager *gm, DWORD dwMask, BitStream *stream)
{
	Parent::packUpdate(gm, dwMask, stream);

	if (stream->writeFlag(dwMask & InitialUpdate)) 
   {
      packDatFile(stream);
   }

	return (0);
}

void Sensor::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
	Parent::unpackUpdate(gm, stream);

	if (stream->readFlag()) 
   {
		AssertFatal(manager == 0,
			"Sensor::unpackUpdate: Can only update shape type on initial packet");
      unpackDatFile(stream);
	}
}

IMPLEMENT_PERSISTENT_TAGS(Sensor, FOURCC('R', 'A', 'D', 'R'), SensorPersTag);

Persistent::Base::Error Sensor::read(StreamIO &sio, int iVer, int iUsr)
{
   Parent::read(sio, iVer, iUsr);
   return ((sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

Persistent::Base::Error Sensor::write(StreamIO &sio, int iVer, int iUsr)
{
   Parent::write(sio, iVer, iUsr);
   return ((sio.getStatus() == STRM_OK) ? Ok : WriteError);
}