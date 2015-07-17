#include <simRenderGrp.h>
#include <simLightGrp.h>
#include <ts_RenderItem.h>
#include <ts_shapeInst.h>
#include <persist.h>

#include <simNetObject.h>
#include <simTerrain.h>
#include "Tank.h"
//#include "fearProjectile.h"
#include "dataBlockManager.h"

//-----------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(Tank, TankPersTag);

//-----------------------------------------------------------------------------------

Tank::Tank()
{
	track1Thread = NULL;
	gunThread = NULL;
}

bool Tank::initResources(GameBase::GameBaseData *in_data)
{
   if(!Parent::initResources(in_data))
      return false;

	if (isGhost() && ((TSShapeInstance *)(image.shape))->getShape().lookupName("track") != -1)
	{
	   track1Thread = Parent::createThread(0);
	   if (track1Thread)
		   track1Thread->SetSequence("track");
		track1Thread->SetPosition (0);
	}
	else
	   track1Thread = NULL;

	if (((TSShapeInstance *)(image.shape))->getShape().lookupName("elevate") != -1)
	{
	   gunThread = Parent::createThread(0);
	   if (gunThread)
		   gunThread->SetSequence("elevate");
	}
	else
	   gunThread = NULL;

	data = dynamic_cast<TankData *>(in_data);

//	collisionImage.sphere.radius /= 4;

	return true;
}

//-----------------------------------------------------------------------------------


void Tank::clientProcess (DWORD curTime)
{
	if (track1Thread)
		{
			track1Thread->setTimeScale( 2.0 * (trackVelocity / data->maxSpeed) );
			track1Thread->AdvanceTime (0.001 * (curTime - lastProcessTime));
		}

	Parent::clientProcess (curTime);

	if(image.shape)
		image.shape->animate();
}

void Tank::serverProcess (DWORD curTime)
{
	Parent::serverProcess (curTime);

	if(image.shape)
		image.shape->animate();
}

//----------------------------------------------------------------------------------

#define TRACTION 1.0
void Tank::groundMechanics (float adjust)
{
	TMat3F trans = getTransform();
	Point3F forward;
	Point3F right;
	Point3F up;
	trans.getRow (1, &forward);
	trans.getRow (0, &right);
	trans.getRow (2, &up);
	float yVel = m_dot (forward, velocityVector);
	float xVel = m_dot (right, velocityVector);
	float zVel = 0; // m_dot (up, velocityVector);
	float groundTraction = TRACTION * adjust;

	fallVelocity = 0;

	// no throttle -> grind to a halt
	if (!speed)
		{
			if (yVel < 0)
				yVel -= yVel < -groundTraction * 2 ? -groundTraction * 2 : yVel;
			if (yVel > 0)
				yVel -= yVel > groundTraction * 2 ? groundTraction * 2 : yVel;
		}
	// otherwise cap speed and accel.
	else
		yVel -= (yVel > 0 ? (yVel * yVel) / data->maxSpeed : (yVel * yVel) / data->minSpeed) * adjust;

	// the slippery slope...
	Point3F rot = getRot();
	float grade = rot.x > data->maxGrade ? data->maxGrade : rot.x;
	float maxGradeSquared = data->maxGrade * data->maxGrade;
	if (grade > data->maxGrade / 2)
		yVel -= ((grade * grade) / maxGradeSquared) * data->maxSpeed * adjust;
	else
		if (grade < -data->maxGrade / 2)
			yVel -= ((grade * grade) / maxGradeSquared) * data->minSpeed * adjust;

	yVel += speed * adjust;

	if (!desiredSpeed)
		{
			if (yVel < 0 && speed > 0)
				speed = 0;
			else
				if (yVel > 0 && speed < 0)
					speed = 0;
		}

	if (yVel > data->maxSpeed)
		yVel = data->maxSpeed;
	if (yVel < data->minSpeed)
		yVel = data->minSpeed;

	// lateral traction
	if (xVel < 0)
		xVel -= xVel < -groundTraction ? -groundTraction : xVel;
	if (xVel > 0)
		xVel -= xVel > groundTraction ? groundTraction : xVel;

	if (zVel < 0)
		zVel -= zVel < -groundTraction / 2 ? -groundTraction / 2 : zVel;
	if (zVel > 0)
		zVel -= zVel > groundTraction / 2 ? groundTraction / 2 : zVel;

	m_mul (Point3F(xVel, yVel, zVel), (RMat3F)trans, &velocityVector);
	setLinearVelocity (velocityVector);

	trackVelocity = yVel;
}

void Tank::airMechanics (float adjust)
{
	TMat3F trans = getTransform();
	Point3F forward;
	Point3F right;
	Point3F up;
	trans.getRow (1, &forward);
	trans.getRow (0, &right);
	trans.getRow (2, &up);
	float yVel = m_dot (forward, velocityVector);
	float xVel = m_dot (right, velocityVector);
	float zVel = m_dot (up, velocityVector);

	fallVelocity = zVel;

	Point3F gravity;
	getForce (SimMovementGravityForce, &gravity);
	xVel += gravity.x * adjust * 0.15;
	yVel += gravity.y * adjust * 0.15;
	zVel += gravity.z * adjust * 0.15;

	m_mul (Point3F(xVel, yVel, zVel), (RMat3F)trans, &velocityVector);
	setLinearVelocity (velocityVector);
}

float Tank::getSuspensionHeight (Point3F pos)
{
   // look for the terrain normal
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

   // look for the zed normal
   SimCollisionInfo info;
   SimContainerQuery query;
   query.id = -1;
   query.type = -1;
   query.mask = SimInteriorObjectType;
   query.box.fMax.set(pos.x, pos.y, pos.z - 1.0);
   query.box.fMin.set(pos.x, pos.y, pos.z + 0.5);
   float zedHeight = terrainHeight;
   if (root->findLOS(query, &info))
		{
	      Point3F p;
	      m_mul(info.surfaces[0].position, info.surfaces.tWorld, &p);
			if (p.z <= pos.z + 0.5)
		      zedHeight = p.z;
	   }

	return zedHeight > terrainHeight ? zedHeight : terrainHeight;
}

void Tank::setDesiredRot (float adjust)
{
	float zHeights[4];
	Point3F worldCorners[4];

	Point3F rot = getRot();
	Point3F pos = getPos();
	RMat3F rmat;
	rmat.set (EulerF (rot.x, rot.y, rot.z));
	for (int i = 0; i < 4; i++)
		{
			m_mul (data->suspension[i], rmat, &worldCorners[i]);
			worldCorners[i] += pos;
			zHeights[i] = getSuspensionHeight (worldCorners[i]);
		}

	Point3F forward;
	Point3F right;
	Point3F normal;
	float highestZ;

	if (zHeights[3] > zHeights[2])
		{
			forward = worldCorners[0] - worldCorners[3];
			forward.z = zHeights[0] - zHeights[3];
			highestZ = worldCorners[3].z - zHeights[3];
		}
	else
		{
			forward = worldCorners[1] - worldCorners[2];
			forward.z = zHeights[1] - zHeights[2];
			highestZ = worldCorners[2].z - zHeights[2];
		}

	bool oldOnGround = onGround;
	onGround = false;
	for (int i = 0; i < 4; i++)
		onGround = onGround || worldCorners[i].z <= zHeights[i];

	if (!isGhost() && !oldOnGround && onGround && fallVelocity < -8.0)
		{
			float vel = (-fallVelocity - 8.0);
			if (vel > data->maxSpeed)
				vel = data->maxSpeed;
			float damage = data->ramDamage * (vel / data->maxSpeed);
			applyDamage(data->ramDamageType, damage, getPos(), Point3F(0, -vel, 0), Point3F(0,0,0), getOwnerId());
//			Console->printf("Damage Type 3 : fallVelocity = %f, vel = %f, damage = %f", fallVelocity, vel, damage);
		}

	right = worldCorners[2] - worldCorners[3];
	right.z = zHeights[2] - zHeights[3];

	forward.normalize();
	right.normalize();
	m_cross (right, forward, &normal);
	normal.normalize();

	if (onGround && (normal.x || normal.y || normal.z))
		{
			Point3F newRot;
			RMat3F mat;

			mat.set (EulerF (0, 0, -rot.z));

			m_mul (normal, mat, &newRot);
			desiredRot.x = m_atan (newRot.y, newRot.z) - (M_PI /  2);
			desiredRot.y = -(m_atan (newRot.x, newRot.z) - (M_PI /  2));
		}
	else
		if (!onGround && fallVelocity < -2.0)
			{
				if (trackVelocity > 0)
					desiredRot.x = rot.x - (0.25 * adjust);
				else
					if (trackVelocity < 0)
						desiredRot.x = rot.x + (0.25 * adjust);

				if (desiredRot.x - rot.x > M_PI / 9)
					desiredRot.x = (M_PI / 9) + rot.x;
			}

//	if (trackVelocity && desiredRot.x < rot.x)
//		{
//			float diffx = (rot.x - desiredRot.x);
//			desiredRot.x -= diffx * ((1 - trackVelocity) / data->maxSpeed);
//		}

	if (desiredRot.x < -M_PI / 3)
		desiredRot.x = -M_PI / 3;
	if (desiredRot.x > M_PI / 3)
		desiredRot.x = M_PI / 3;
	if (desiredRot.y < -M_PI / 3)
		desiredRot.y = -M_PI / 3;
	if (desiredRot.y > M_PI / 3)
		desiredRot.y = M_PI / 3;

	if (highestZ < 0)
		pos.z -= highestZ;

	setPos (pos);
}

//----------------------------------------------------------------------------------

bool Tank::updateMove(PlayerMove *move, float interval)
{
	if (!Parent::updateMove (move, interval))
		return false;

	if (gunThread)
		gunThread->SetPosition (pitch);

	float adjust = interval / 0.032;

//	if (isGhost())
//		Console->printf("adjust = %f", adjust);

	float turn = 0;
	float oldDS = desiredSpeed;
	float oldS = speed;
	
	setDesiredRot (adjust);

	if (move)
		{
			pitch += move->pitch * 0.25 * adjust;
			if (pitch > 1.0)
				pitch = 1.0;
			if (pitch < 0)
				pitch = 0;
			turn = move->turnRot * 0.25 * adjust;
	
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

					m_mul (Point3F (0, (speed / 2) - 10, 40), rmat, &mom);

					if (const char* script = scriptName("jump"))
						Console->evaluatef("%s(%s,\"%g %g %g\");", script, scriptThis(), mom.x,mom.y,mom.z);
				}

			if (move->jetting || move->forwardAction)
				desiredSpeed = data->maxSpeed;
			else
				if (move->backwardAction)
					desiredSpeed = data->minSpeed;
				else
					desiredSpeed = 0;
		}

	if (!hasFocus)
		desiredSpeed = 0;
		
	if (desiredSpeed < data->minSpeed)	// -7
		desiredSpeed = data->minSpeed;
		
	if (desiredSpeed > data->maxSpeed)	// 10
		desiredSpeed = data->maxSpeed;
	
	Point3F oldPos = getPos();
	Point3F rot = getRot();
	
	float maxSpeed;
	
	if (onGround)
		maxSpeed = desiredSpeed;
	else
		maxSpeed = speed;
		
	if (speed > maxSpeed)
		{
			speed -= 0.25 * adjust;

			if (speed < maxSpeed)
				speed = maxSpeed;
		}
		
	if (speed < maxSpeed)
		{
			speed += 0.2 * adjust;
			
			if (speed > maxSpeed)
				speed = maxSpeed;
		}

	rot.z += turn / 2;

	if (rot.x < desiredRot.x)
		{
			if (desiredRot.x - rot.x > M_PI / 8)
				{
					rot.x = desiredRot.x;
//					Console->printf("Snap!");
					if (!isGhost() && trackVelocity > data->maxSpeed / 4)
						{
							float damage = data->ramDamage * 0.25 * (abs(trackVelocity) / data->maxSpeed);
							applyDamage(data->ramDamageType, damage, getPos(), getLinearVelocity(), Point3F(0,0,0), getOwnerId());
//							Console->printf("Damage type 1 : trackVelocity = %f, damage = %f", trackVelocity, damage);
						}
				}
			else
				{
					rot.x += 0.07 * adjust;
					if (rot.x > desiredRot.x)
						rot.x = desiredRot.x;
				}
		}
	else
		if (rot.x > desiredRot.x)
			{
				rot.x -= 0.07 * adjust;
				if (rot.x < desiredRot.x)
					rot.x = desiredRot.x;
			}

	if (onGround)
		{
			if (rot.y < desiredRot.y)
				{
					rot.y += 0.05 * adjust;
					if (rot.y > desiredRot.y)
						rot.y = desiredRot.y;
				}
			else
				if (rot.y > desiredRot.y)
					{
						rot.y -= 0.05 * adjust;
						if (rot.y < desiredRot.y)
							rot.y = desiredRot.y;
					}
		}
	
		{
//			if (isGhost())
//				Console->printf("speed = %f, desiredSpeed = %f, onGround = %d", speed, desiredSpeed, onGround);
			setRot (rot);
			if (onGround)
				groundMechanics (adjust);
			else
				airMechanics (adjust);
			updateMovement (interval);
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


//-----------------------------------------------------------------------------------


Persistent::Base::Error Tank::read(StreamIO &sio, int, int)
{
	Parent::read (sio, 0, 0);
	
   DWORD vers;
   sio.read(&vers);

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


Persistent::Base::Error Tank::write(StreamIO &sio, int, int)
{
   Parent::write(sio, 0, 0);
	
   sio.write(DWORD(0));
	
	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


//-----------------------------------------------------------------------------------

int Tank::getDatGroup()
{
   return DataBlockManager::TankDataType;
}

//----------------------------------------------------------------------------

void Tank::TankData::pack(BitStream *stream)
{
   Vehicle::VehicleData::pack(stream);
	
   stream->write(maxGrade);
}

void Tank::TankData::unpack(BitStream *stream)
{
   Vehicle::VehicleData::unpack(stream);
	
   stream->read(&maxGrade);
}

