#include <simRenderGrp.h>
#include <simLightGrp.h>
#include <ts_RenderItem.h>
#include <ts_shapeInst.h>
#include <persist.h>

#include <simNetObject.h>
#include <simTerrain.h>
#include "FearForceDcl.h"
#include "Hover.h"
//#include "fearProjectile.h"
#include "dataBlockManager.h"

//-----------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(Hover, HoverPersTag);

//-----------------------------------------------------------------------------------

Hover::Hover()
{
	bounce = 0;
	bounceDir = 0.01;
	velocity.set (0, 0, 0);
}

bool Hover::initResources(GameBase::GameBaseData *in_data)
{
   if(!Parent::initResources(in_data))
      return false;

	data = dynamic_cast<HoverData *>(in_data);

	return true;
}

//-----------------------------------------------------------------------------------


void Hover::clientProcess (DWORD curTime)
{
	if (isGhost() && cg.psc)
		{
			if (cg.psc->getControlObject () == this)
				pilot = (Player *) 1;
			else
				pilot = NULL;
		}

//   if(calcFinalPos)
//   {
//      DWORD t = lastProcessTime;
//      DWORD endTime = curTime & ~0x1F + cg.predictForwardTime;
//
//      while(t < endTime)
//      {
//         updateMove (&lastPlayerMove, 0.032);
//         t += 32;
//      }
//      calcFinalPos = false;
//      interpDoneTime = curTime + cg.interpolateTime;
//
//      savedVelocity = getLinearPosition() - savedPosition;
//      lastProcessTime = savedLastTime;
//      savedVelocity *= 1000.0f / float(interpDoneTime - lastProcessTime);
//
//      TMat3F tm = getTransform();
//      Point3F temp = tm.p;
//      tm.p = savedPosition;
//      setTransform(tm);
//      savedPosition = temp;
//   }

   while (lastProcessTime < curTime)
	   {
	      DWORD endTick = (lastProcessTime + 32) & ~0x1F;
			if(endTick > curTime)
	         endTick = curTime;

	      PlayerMove *pm;

	      if (pilot)
		      {
		         pm = cg.psc->getClientMove(lastProcessTime);
		         if(pm)
						{
							lastPlayerMove = *pm;
							updateMove (pm, 0.001 * (endTick - lastProcessTime));
						}
		      }
			else
				updateMove (NULL, 0.001 * (endTick - lastProcessTime));

	      lastProcessTime = endTick;
		}
}

void Hover::serverProcess (DWORD)
{
	if (!pilot)
		updateMove (NULL, 0.032);
	else
		updateSkip++;
}

void Hover::setDesiredZ (float /* adjust */)
{
	Point3F pos = getPos();
	
//	bounce += bounceDir * adjust;
//	if (bounce > 0.15 || bounce < -0.15)
//		bounceDir = -bounceDir;

   // look for the terrain z
	SimContainer *container = getContainer ();
   SimContainer *root = findObject(manager, SimRootContainerId, root);

	while (container != root && !(container->getType () & SimTerrainObjectType))
		container = container->getContainer();

	float terrainHeight = -1.0e30f;
	if (container->getType () & SimTerrainObjectType)
		{
			SimTerrain *terrain = (SimTerrain *)container;
		   CollisionSurface collisionSurface;
		   if (terrain->getSurfaceInfo(Point2F(pos.x, pos.y), &collisionSurface))
	      	terrainHeight = collisionSurface.position.z;
		}

   // look for the zed z
   SimCollisionInfo info;
   SimContainerQuery query;
   query.id = -1;
   query.type = -1;
   query.mask = SimInteriorObjectType;
   query.box.fMax.set(pos.x, pos.y, pos.z - 5.0);
   query.box.fMin.set(pos.x, pos.y, pos.z + 5.0);
   float zedHeight = terrainHeight;
   if (root->findLOS(query, &info))
		{
	      Point3F p;
	      m_mul(info.surfaces[0].position, info.surfaces.tWorld, &p);
	      zedHeight = p.z;
	   }
	
	float newZ = zedHeight > terrainHeight ? zedHeight : terrainHeight;
	
	newZ += data->hoverHeight + bounce;

	pos.z = newZ;
	Point3F rot = getRot();
	rot.x = rot.y = 0;
	setRot (rot);
	setPos (pos);
}

void Hover::dynamics (float adjust)
{
//	if (velocity.x > 0)
//		{
//			velocity.x -= adjust * 0.35;
//			if (velocity.x < 0)
//				velocity.x = 0;
//		}
//	else
//		if (velocity.x < 0)
//			{
//				velocity.x += adjust * 0.35;
//				if (velocity.x > 0)
//					velocity.x = 0;
//			}
//
//	if (velocity.y > 0)
//		{
//			velocity.y -= adjust * 0.35;
//			if (velocity.y < 0)
//				velocity.y = 0;
//		}
//	else
//		if (velocity.y < 0)
//			{
//				velocity.y += adjust * 0.35;
//				if (velocity.y > 0)
//					velocity.y = 0;
//			}

	Point3F rot = getRot();
	RMat3F rmat(EulerF (rot.x, rot.y, rot.z));
	Point3F temp;
	m_mul (Point3F (0, speed * adjust, 0), rmat, &temp);
	velocity += temp;
	
//	if (velocity.x < data->maxSpeed && velocity.x > -data->maxSpeed)
//		velocity.x += temp.x;
//		
//	if (velocity.y < data->maxSpeed && velocity.y > -data->maxSpeed)
//		velocity.y += temp.y;
		
	velocity.z = 0;
		
	setLinearVelocity (velocity);
	onGround = false;
}

bool Hover::updateMove(PlayerMove *move, float interval)
{
	if (!Parent::updateMove (move, interval))
		return false;
		
	float adjust = interval / 0.032;
	float turn = 0;
	float oldDS = desiredSpeed;
	float oldS = speed;
	
	if (move)
		{
			pitch += move->pitch * 0.25 * adjust;
			turn = move->turnRot * 0.25 * adjust;
	
//			if (move->triggerCount != triggerCount)
//				{
//					triggerCount = move->triggerCount;
//				}

			if (move->jumpAction)
				{
					dismount ();
//					playerLoseControl ();
					return false;
				}
		
			if (move->jetting || move->forwardAction)
				desiredSpeed = data->maxSpeed;
			else
				if (move->backwardAction)
					desiredSpeed = -data->maxSpeed;
				else
					desiredSpeed = 0;
		}
	else
		if (!pilot)
			if (desiredSpeed)
				desiredSpeed = 0;
		
//	if(lastProcessTime == interpDoneTime)
//		{
//			TMat3F tm = getTransform();
//			tm.p = savedPosition;
//			setTransform(tm);
//		}

	if (desiredSpeed < data->minSpeed)	// -7
		desiredSpeed = data->minSpeed;
		
	if (desiredSpeed > data->maxSpeed)	// 10
		desiredSpeed = data->maxSpeed;
	
	setDesiredZ (adjust);
	Point3F oldPos = getPos();
	Point3F rot = getRot();
	
	if (speed > desiredSpeed)
		{
			speed -= 0.25 * adjust;

			if (speed < desiredSpeed)
				speed = desiredSpeed;
		}
		
	if (speed < desiredSpeed)
		{
			speed += 0.3 * adjust;
			
			if (speed > desiredSpeed)
				speed = desiredSpeed;
		}
	
	rot.z += turn / 2;
	
//	if(lastProcessTime < interpDoneTime)
//		{
//			TMat3F tmat;
//		   updatePosition(savedVelocity, &tmat);
//			
//			// Rotation into the tmat
//			const Point3F &rotation = getRot();
//			((RMat3F *) &tmat)->set(EulerF(rotation.x, rotation.y, rotation.z));
//			
//			setTransform (tmat);
//		}
//	else
		{
			setRot (rot);
			dynamics (adjust);
			updateMovement ();
		}
	
	if (!isGhost())
		{
			if (oldPos != getPos() || turn)
				setMaskBits (OrientationMask);

			if (oldDS != desiredSpeed || oldS != speed)
				setMaskBits (StatusMask);
		}
		
	return true;
}


bool Hover::processCollision(SimMovementInfo *info)
{
	for (int i = 0; i < info->collisionList.size(); i++)
		{
			if (lastPilot && info->collisionList[i].object == lastPilot)
		      {
					info->collidedList.push_back(info->collisionList[i].object);
		         info->collisionList.erase(i);
		      }
		}

	return ShapeBase::processCollision (info);
}


//-----------------------------------------------------------------------------------

Persistent::Base::Error Hover::read(StreamIO &sio, int, int)
{
	Parent::read (sio, 0, 0);
	
   DWORD vers;
   sio.read(&vers);

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


Persistent::Base::Error Hover::write(StreamIO &sio, int, int)
{
   Parent::write(sio, 0, 0);
	
   sio.write(DWORD(0));
	
	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


//-----------------------------------------------------------------------------------

int Hover::getDatGroup()
{
   return DataBlockManager::HoverDataType;
}

//----------------------------------------------------------------------------

void Hover::HoverData::pack(BitStream *stream)
{
   Vehicle::VehicleData::pack(stream);
	
   stream->write(hoverHeight);
}

void Hover::HoverData::unpack(BitStream *stream)
{
   Vehicle::VehicleData::unpack(stream);
	
   stream->read(&hoverHeight);
}

