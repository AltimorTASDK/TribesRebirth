#include <simRenderGrp.h>
#include <simLightGrp.h>
#include <ts_RenderItem.h>
#include <ts_shapeInst.h>
#include <persist.h>

#include "simTerrain.h"
#include "simInteriorShape.h"
#include <simNetObject.h>
#include "esfObjectTypes.h"
#include "FearDcl.h"
#include "FearForceDcl.h"
#include "player.h"
#include "Flier.h"
#include "dataBlockManager.h"
#include "Projectile.h"

//-----------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(Flier, FlierPersTag);

//-----------------------------------------------------------------------------------

bool Flier::flipYAxis = false;

Flier::Flier()
{
	tiltThread = NULL;
	flameThread = NULL;

	landed = false;
	lift = 0;
	lateralThrust = 0;
	data = NULL;
	velocityVector.z = 0;

	braking = false;
	thrusting = false;

	bounce = 0;
}

bool Flier::initResources(GameBase::GameBaseData *in_data)
{
   if(!Parent::initResources(in_data))
      return false;
	
   data = dynamic_cast<FlierData *>(in_data);

	if (isGhost() && ((TSShapeInstance *)(image.shape))->getShape().lookupName("jet") != -1)
	{
	   tiltThread = Parent::createThread(0);
	   if (tiltThread)
		   tiltThread->SetSequence("jet");
		tiltThread->SetPosition (0.5);
	}
	else
	   tiltThread = NULL;

	if (isGhost() && ((TSShapeInstance *)(image.shape))->getShape().lookupName("idle") != -1)
	{
	   flameThread = Parent::createThread(0);
	   if (flameThread)
		   flameThread->SetSequence("idle");
		flameThread->SetPosition (0);
	}
	else
	   flameThread = NULL;

   if(flameThread)
   {
      thrustSequence = flameThread->GetSequenceIndex("thrust");
      idleSequence = flameThread->GetSequenceIndex("idle");
   }

   if(isGhost())
	   collisionMask = SimInteriorObjectType | SimTerrainObjectType |
      					MineObjectType | SimPlayerObjectType | 
							MoveableObjectType |
							VehicleObjectType | StaticObjectType;
   else
      collisionMask = SimInteriorObjectType | SimTerrainObjectType |
      					MineObjectType | SimPlayerObjectType | 
							TriggerObjectType | MoveableObjectType |
							VehicleObjectType | StaticObjectType;

	// hack for now... need bbox to terrain collision... hint, hint.
	collisionImage.sphere.radius /= 4;
	setMaxAlt ();
	
	return true;
}

//-----------------------------------------------------------------------------------

void Flier::clientProcess (DWORD curTime)
{
	float interval = 0.001 * (curTime - lastProcessTime);
	float adjust = interval / 0.032;

	if (!hasFocus)
		{
			thrusting = false;
			braking = false;
		}
	else
		if (thrusting || braking)
			landed = false;

	if (tiltThread)
		{
			float animPos = tiltThread->getPosition();
			float desiredPos;

			if (braking && speed)
				desiredPos = 1.0;
			else
				desiredPos = 1 - (0.5 + ((desiredSpeed / data->maxSpeed) / 2));

			if (animPos < desiredPos)
				{
					animPos += interval;
					if (animPos > desiredPos)
						animPos = desiredPos;
				}
			else
				if (animPos > desiredPos)
					{
						animPos -= interval;
						if (animPos < desiredPos)
							animPos = desiredPos;
					}

			if (animPos > 1.0)
				animPos = 1.0;

			if (animPos < 0)
				animPos = 0;

			tiltThread->SetPosition (animPos);
		}

	if (flameThread)
		{
			int index;
			if (thrusting)
				index = thrustSequence;
			else
				index = idleSequence;

			if (flameThread->getSequence().getIndex(image.shape->getShape()) != index)
				{
				   flameThread->SetSequence(index);
//					flameThread->SetPosition (0);
				}

			flameThread->AdvanceTime (interval);
		}

	Parent::clientProcess (curTime);

	if(image.shape)
		image.shape->animate();

	if (landed)
		{
			bounce += adjust * 0.1;
			Point3F pos = getPos();
			pos.z = minAlt + m_sin (bounce) / 16;
			setPos (pos);
		}
	else
		bounce = 0;
}

void Flier::serverProcess (DWORD curTime)
{
	Parent::serverProcess (curTime);

	if (hasFocus && lift > 0 && soundState != RUN_SOUND)
		{
			soundState = RUN_SOUND;
			setMaskBits (SoundMask);
		}
}

void Flier::setMaxAlt (void)
{
	SimContainer *container = getContainer ();
   SimContainer *root = findObject(manager, SimRootContainerId, root);
	
	while (container != root && !(container->getType () & SimTerrainObjectType))
		container = container->getContainer();
	
	if (container->getType () & SimTerrainObjectType)
		{
			SimTerrain *terrain = (SimTerrain *)container;
			GridRange<float> range = terrain->getGridFile()->getHeightRange();
			
			maxAlt = range.fMax + data->maxAlt + (range.fMax - range.fMin / 2);
		}
}

void Flier::setMinAlt (void)
{
	Point3F pos = getPos();
	
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
   query.mask = SimInteriorObjectType | StaticObjectType | VehicleObjectType;
   query.box.fMax.set(pos.x, pos.y, pos.z - 5.0);
   query.box.fMin.set(pos.x, pos.y, pos.z);
   float zedHeight = terrainHeight;
   if (root->findLOS(query, &info))
		{
			SimObject *object = info.object;

			if (!(object && object->getType() == VehicleObjectType && !(((Vehicle *)object)->getLeadVelocity().lenf())))
				{
			      Point3F p;
			      m_mul(info.surfaces[0].position, info.surfaces.tWorld, &p);
			      zedHeight = p.z;
				}
	   }
	
	float newZ = zedHeight > terrainHeight ? zedHeight : terrainHeight;

	if (!hasFocus || speed < data->maxSpeed / 3)
		if (damageLevel < data->maxDamage)
			newZ += 2.0;

//	pos.z -= bounce;

	if (pos.z <= newZ && !speed)
		{
			landed = true;
			if (!hasFocus)
				{
					Point3F rot = getRot();
					rot.x = rot.y = 0;
					setRot (rot);
				}
				
			pos.z = newZ;
			setPos (pos);
			if (!isGhost())
				setMaskBits (OrientationMask);
		}
	else
		if (pos.z - bounce > newZ)
			landed = false;

	minAlt = newZ;
}

const Point3F &Flier::getLeadVelocity (void)
{
	static Point3F temp;
	Point3F rot = getRot();
	
	RMat3F rmat (EulerF (rot.x, rot.y, rot.z));
	
	m_mul (Point3F (0, speed, lift), rmat, &temp);
	
	return temp;
}

void Flier::dynamics (float adjust)
{
	setMinAlt ();

	Point3F rot = getRot();

	if (!landed && speed)
		{
			lift -= data->lift * adjust;

			lift -= lift * (speed / data->maxSpeed) * adjust;

			if (lift > data->maxVertical)
				lift = data->maxVertical;
		}
	else
		{
			lift -= data->lift * adjust;
			if (landed && lift < 0)
				lift = 0;
		}

	float alt = getPos().z - minAlt;
	float maxLift;
	float minLift = data->maxVertical * -1.75;
	if (alt < 4)
		maxLift = minLift * (alt / 4);
	else
		maxLift = minLift;

	if (lift < maxLift)
		lift = maxLift;

	if (getPos().z > maxAlt && lift)
		lift = 0;

	RMat3F rmat (EulerF (rot.x, rot.y, rot.z));
	Point3F temp;

	m_mul (Point3F (lateralThrust, speed, lift), rmat, &velocityVector);
//	velocityVector.z += lift;

	if (damageLevel < data->maxDamage)
		setLinearVelocity (velocityVector);
}

bool Flier::updateMove(PlayerMove *move, float interval)
{
	if (!Parent::updateMove (move, interval))
		return false;

	float adjust = interval / 0.032;
	float turn = 0;
	float pitchChange = 0;
	float oldDS = desiredSpeed;
	float oldS = speed;
	float yRot = getRot().y;
	yRot -= (M_PI / 32) * (lateralThrust / (data->maxVertical / 2));
	braking = false;
	thrusting = false;
	
	if (move)
		{
			float speedScale = 1 - (speed / data->maxSpeed);
			if (speedScale < 0.33)
				speedScale = 0.33;

			pitchChange = (-move->pitch * speedScale) * 0.25 * adjust;
			turn = (move->turnRot  * speedScale) * 0.25 * adjust;
	
			if (move->trigger)
				{
					if (!isGhost ())
						shoot ();
				}

			if (move->jumpAction && !isGhost())
				{
					Point3F mom;
					Point3F rot = getRot();
					RMat3F rmat (EulerF (rot.x, rot.y, rot.z));

					m_mul (Point3F (0, (speed / 2) - 40, 40), rmat, &mom);

					if (const char* script = scriptName("jump"))
						Console->evaluatef("%s(%s,\"%g %g %g\");", script, scriptThis(), mom.x,mom.y,mom.z);

					lastPlayerMove.trigger = 0;
					lastPlayerMove.jumpAction = 0;
					lastPlayerMove.forwardAction = 0;
					lastPlayerMove.backwardAction = 0;
					lastPlayerMove.jetting = 0;
//					Console->printf ("----->Move cleared in Flier::updateMove");
					setMaskBits (OrientationMask);
				}

			if (move->forwardAction)
				{
					desiredSpeed = data->maxSpeed;
					thrusting = true;
				}
			else
				{
					desiredSpeed = 0;
//					if (hasFocus)
//						Console->printf ("---->Flier NOT thrusting");
				}

			if (move->backwardAction)
				{
					braking = true;
					thrusting = true;
					desiredSpeed = data->minSpeed;
					if (data->minSpeed)
						landed = false;
				}

			if (move->jetting)
				{
					lift += 1.25 * adjust;
					if (lift > data->maxVertical)
						lift = data->maxVertical;
					landed = false;
					thrusting = true;
				}

			if (!landed)
				{
					float maxLateral = data->maxVertical / 2;
					if (move->rightAction)
						{
							lateralThrust += 0.25 * adjust;
							if (lateralThrust > maxLateral)
								lateralThrust = maxLateral;
						}
					else
						if (move->leftAction)
							{
								lateralThrust -= 0.25 * adjust;
								if (lateralThrust < -maxLateral)
									lateralThrust = -maxLateral;
							}
				}
		}

		if (lateralThrust && (landed || (!move || !(move->leftAction || move->rightAction))))
			{
				lateralThrust -= (lateralThrust > 0 ? 0.25 : -0.25) * adjust;
				if (lateralThrust < 0.25 && lateralThrust > -0.25)
					lateralThrust = 0;
			}

	if (!hasFocus)
		{
			desiredSpeed = 0;
			desiredRot.x = 0;
			desiredRot.y = 0;
		}
		
	if (desiredSpeed < data->minSpeed)
		desiredSpeed = data->minSpeed;

	if (desiredSpeed > data->maxSpeed)
		desiredSpeed = data->maxSpeed;

	Point3F oldPos = getPos();
	Point3F rot = getRot();
	rot.y = yRot;
	Point3F oldRot = rot;

	float maxSpeed = desiredSpeed;

	if (speed > maxSpeed)
		{
			speed -= data->accel * adjust * 0.5;

			// put on the brakes when the pilot bails...
			if (!hasFocus || braking)
				speed -= data->accel * adjust;

			if (speed < maxSpeed)
				speed = maxSpeed;
		}
		
	if (speed < maxSpeed)
		{
			speed += data->accel * adjust;
			
			if (speed > maxSpeed)
				speed = maxSpeed;
		}
	
	if (oldPos.z > maxAlt && pitchChange < 0)
		pitchChange = 0;

	if (!landed || speed)
		{
			rot.x -= pitchChange;
			rot.y -= turn / 2;
		}
	
	if (data)
		{
			if (rot.y < -data->maxBank)
				{
					float amount = rot.y - -data->maxBank;
					rot.y -= (amount / 2) * adjust;
				}

			if (rot.y > data->maxBank)
				{
					float amount = rot.y - data->maxBank;
					rot.y -= (amount / 2) * adjust;
				}

			if (rot.x < -data->maxPitch)
				{
					float amount = rot.x - -data->maxPitch;
					rot.x -= (amount / 2) * adjust;
				}

			if (rot.x > data->maxPitch)
				{
					float amount = rot.x - data->maxPitch;
					rot.x -= (amount / 2) * adjust;
				}
		}

	rot.z -= (rot.y / 15) * adjust;
	rot.y += (M_PI / 32) * (lateralThrust / (data->maxVertical / 2));

	if (hasFocus)
		desiredRot = rot;
//	else
//		lift = 0;

	if (landed && !speed)
		desiredRot.x = desiredRot.y = 0;

	if (oldPos.z > maxAlt)
		desiredRot.x = 0;
	
	if (rot.x < desiredRot.x)
		{
			rot.x += 0.0125 * adjust;
			if (rot.x > desiredRot.x)
				rot.x = desiredRot.x;
		}
	else
		if (rot.x > desiredRot.x)
			{
				rot.x -= 0.0125 * adjust;
				if (rot.x < desiredRot.x)
					rot.x = desiredRot.x;
			}

	if (rot.y < desiredRot.y)
		{
			rot.y += 0.0125 * adjust;
			if (rot.y > desiredRot.y)
				rot.y = desiredRot.y;
		}
	else
		if (rot.y > desiredRot.y)
			{
				rot.y -= 0.0125 * adjust;
				if (rot.y < desiredRot.y)
					rot.y = desiredRot.y;
			}
	
	setRot (rot);
	dynamics (adjust);
	TMat3F mat = getTransform ();
	updateMovement (interval);

	int savedMask = collisionMask;

	collisionMask = SimPlayerObjectType;
	if (!displaceObjects(mat))
		{
			speed = 0;
			desiredSpeed = 0;
			lift = 0;
		}

	collisionMask = savedMask;

	if (!isGhost())
		{
			if (oldPos != getPos() || oldRot != getRot())
				setMaskBits (OrientationMask);

			if (oldDS != desiredSpeed || oldS != speed)
				setMaskBits (StatusMask);
		}

	return true;
}

//-----------------------------------------------------------------------------------


bool Flier::processCollision(SimMovementInfo *info)
{
	Point3F normal (0, 0, 0);
	bool surfaceCollide = false;

	// Strip out mounted players first.
   int i;
	for (i = 0; i < info->collisionList.size(); i++)
		{
			SimCollisionInfo& sinfo = info->collisionList[i];
			if ((sinfo.object->getType().test(SimPlayerObjectType) &&	static_cast<Player*>(sinfo.object)->getMountObject() == this) ||
				  sinfo.object->getType().test(MineObjectType))
				{
					info->collisionList.erase (i);
					i--;
				}
		}

	// Normal collision process
	for (i = 0; i < info->collisionList.size(); i++)
		{
			if (info->collisionList[i].object->getType() & CollisionDamageMask)
				{
					ShapeBase* pShape = dynamic_cast<ShapeBase*>(info->collisionList[i].object);
					if (pShape)
						{
							Point3F diffVel = getLinearVelocity() - pShape->getLinearVelocity();
							float damageSpeed = diffVel.lenf();
							if (damageSpeed > data->maxSpeed)
								damageSpeed = data->maxSpeed;

							if (!pShape->getType().test (SimPlayerObjectType))
								{
									speed = 0;
									desiredSpeed = 0;
									lift = 0;
								}

							if (damageSpeed < data->maxSpeed / 4)
								damageSpeed = 0;
			            float damage = damageSpeed ? data->ramDamage * (damageSpeed / data->maxSpeed) : 0;
							if (damageLevel < data->maxDamage)
							   pShape->applyDamage(data->ramDamageType, damage, getPos(), getLinearVelocity(), Point3F(0,0,0), getId());
						   applyDamage(-1, damage / 2, getPos(), getLinearVelocity(), Point3F(0,0,0), getId());
							continue;
						}
				}

			if (info->collisionList[i].object->getType() & SimTerrainObjectType ||
				 info->collisionList[i].object->getType() & SimInteriorObjectType)
				{
					surfaceCollide = true;

					CollisionSurfaceList& sList = info->collisionList[i].surfaces;

					Vector3F vertical;
					Point3F rot = getRot();
					RMat3F rmat (EulerF(rot.x, rot.y, rot.z));
					m_mul (Point3F (0, 0, 1), rmat, &vertical);
					for (int j = 0; j < sList.size(); j++)
						{
							Point3F tNorm;
							m_mul(sList[j].normal, static_cast<RMat3F&>(sList.tWorld), &tNorm);
							normal += tNorm;
						}
				}
		}

	Point3F lvel = lVelocity;
	float damageSpeed = lvel.lenf();
	if (damageSpeed > data->maxSpeed)
		damageSpeed = data->maxSpeed;

	if (surfaceCollide && (normal.x || normal.y || normal.z) && (lvel.x || lvel.y || lvel.z))
		{
			normal.normalize();
			lvel.normalize();
			float dot = m_dot (lvel, normal);
			if (dot < 0.0f)
				{
					if (damageSpeed < data->maxSpeed / 3)
						damageSpeed = 0;

		         float damage = data->maxDamage * data->groundDamageScale * (damageSpeed / data->maxSpeed);

					// only reflect for relatively shallow angles...
					// and only if moving forward!
					if (dot > -0.25 && speed > data->maxSpeed / 4)
						{
							float scalar = -dot;
							Point3F reflectPoint = normal * scalar * 2;
							Point3F endPoint = lvel + reflectPoint;
							Point3F oR = getRot();

							setLinearVelocity (endPoint);

							Point3F up,r0,r2;
							Point3F vec = endPoint;
							vec.normalize();
							vec.x == 0 && vec.y == 0 ? up.set (1, 0, 0) : up.set (0, 0, 1);
							m_cross(vec, up, &r0);
							r0.normalize();
							m_cross(r0, vec, &r2);
							r2.normalize();

							TMat3F trans;
							trans.set (EulerF (0, 0, 0), getPos() + (endPoint * 0.032));
							trans.setRow (0, r0);
							trans.setRow (1, vec);
							trans.setRow (2, r2);
							trans.flags |= TMat3F::Matrix_HasRotation;
							EulerF ang;
							trans.angles (&ang);
							desiredRot.x = ang.x;
							desiredRot.y = ang.y;
							desiredRot.z = ang.z;
							desiredRot.x = oR.x + (((desiredRot.x - oR.x) / 1.5));

							setRot (desiredRot);

							speed *= 0.8;
							desiredSpeed *= 0.8;
						}
					else
						{
							speed = 0;
							desiredSpeed = 0;
							lift = 0;
//							damage *= 1.75;
						}

					setMaskBits (StatusMask);

					if (damage)
						{
							if (dot < -1)
								dot = -1;
							dot -= 0.125;

							dot = -dot;
							damage *= dot;

							applyDamage(-1,damage,getPos(), lvel, lvel, getId());
							setMaskBits (DamageMask);
						}
				}
		}

	if (isGhost())
		setMaskBits (OrientationMask);

	return Parent::processCollision (info);
}


//----------------------------------------------------------------------------

void Flier::readPacketData(BitStream *bstream)
{
	Point3F pos = getPos();
	Parent::readPacketData (bstream);
	bstream->read (&lift);
	bstream->read (&lateralThrust);
	landed = bstream->readFlag ();
	if (landed)
		setPos (pos);
}

void Flier::writePacketData(BitStream *bstream)
{
	Parent::writePacketData (bstream);
	bstream->write (lift);
	bstream->write (lateralThrust);
	bstream->writeFlag (landed);
}

//-----------------------------------------------------------------------------------

DWORD Flier::packUpdate(Net::GhostManager *g, DWORD mask, BitStream *stream)
{
	Parent::packUpdate (g, mask, stream);

//	stream->writeFlag (landed);

	return 0;
}

void Flier::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
   Parent::unpackUpdate(gm, stream);

//	landed = stream->readFlag();
}

//-----------------------------------------------------------------------------------

Persistent::Base::Error Flier::read(StreamIO &sio, int, int)
{
	Parent::read (sio, 0, 0);
	
   DWORD vers;
   sio.read(&vers);

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


Persistent::Base::Error Flier::write(StreamIO &sio, int, int)
{
   Parent::write(sio, 0, 0);
	
   sio.write(DWORD(0));
	
	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}

//-----------------------------------------------------------------------------------

int Flier::getDatGroup()
{
   return DataBlockManager::FlierDataType;
}

//----------------------------------------------------------------------------

void Flier::FlierData::pack(BitStream *stream)
{
   Parent::pack(stream);
	
   stream->write(maxBank);
   stream->write(maxPitch);
	stream->write (lift);
   stream->write(maxAlt);
	stream->write (maxVertical);
	stream->write (accel);
	stream->write (groundDamageScale);
}

void Flier::FlierData::unpack(BitStream *stream)
{
   Parent::unpack(stream);

   stream->read(&maxBank);
   stream->read(&maxPitch);
	stream->read (&lift);
   stream->read(&maxAlt);
	stream->read (&maxVertical);
	stream->read (&accel);
	stream->read (&groundDamageScale);
}

bool
Flier::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);

     default:
      return Parent::processQuery(query);
   }
}

bool
Flier::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if (Parent::onSimRenderQueryImage(query) == true) {
      image.itype = SimRenderImage::Translucent;

      TS::Camera* pCamera = query->renderContext->getCamera();
   
      const TMat3F& rTWC = pCamera->getTWC();
      float cd = rTWC.p.y + image.transform.p.x * rTWC.m[0][1] +
                            image.transform.p.y * rTWC.m[1][1] +
                            image.transform.p.z * rTWC.m[2][1];
      float nd = pCamera->getNearDist();
      image.sortValue = (cd <= nd) ? 0.99f : nd / cd;
      return true;
   }
   return false;
}
