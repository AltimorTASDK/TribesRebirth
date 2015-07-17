#include <simRenderGrp.h>
#include <simLightGrp.h>
#include <ts_RenderItem.h>
#include <ts_shapeInst.h>
#include <persist.h>

#include <simNetObject.h>
#include "vehicle.h"
#include "sfx.strings.h"
#include "esfObjectTypes.h"
#include "debris.h"
#include "tsfx.h"
#include "partDebris.h"

#include "grdFile.h"
#include "grdCollision.h"
#include "itrInstance.h"
#include <m_coll.h>

//#include "FearExplosion.h"
#include "console.h"
#include "dataBlockManager.h"
#include "Projectile.h"

//-----------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(Vehicle, VehiclePersTag);

//-----------------------------------------------------------------------------------

float Vehicle::prefShapeDetail = 1.0f;
float Vehicle::prefVehicleShadowSize = 40;

// Objects for which we invoke the script collision callback
static int ScriptCollisionMask = 
			SimPlayerObjectType | 
			VehicleObjectType |
      	StaticObjectType |
			MineObjectType |
			ItemObjectType;

//-----------------------------------------------------------------------------------

Vehicle::Vehicle()
{
   netFlags.set( Ghostable );
   type = VehicleObjectType;
	
	gunCount = 0;
	
	desiredSpeed = speed = 0;
	fireCount = 0;
	waitTime = 0;

   interpDoneTime = 0;
   updateDebt = 0;
   updateSkip = 0;
	lastPitch = 0;
	lastPos.set (0, 0, 0);
	lastRot.set (0, 0, 0);

	velocityVector.set (0, 0, 0);
	trackVelocity = 0;
	fallVelocity = 0;
	onGround = true;

	forceMask = -1;
	SimMovement::flags.set( UseCurrent | UseDrag | UseFriction);
	
	data = NULL;
	damageLevel = 0;

	hasFocus = false;

	idleSound = 0;
	throttleSound = 0;
	soundState = NO_SOUND;

	maxPassengers = 0;
	passengerCount = 0;

	eyeNode = -1;
	gunNode = -1;
	pilotNode = -1;
	exitNode = -1;
	muzzleNodes = NULL;
	passengerNodes = NULL;
}

bool Vehicle::onAdd()
{
   if(!Parent::onAdd())
      return FALSE;
		
	// Switch to moveable update set
	removeFromSet(PlayerSetId);
	addToSet(MoveableSetId);
	
	if(isGhost())
		addToSet (SimCameraMountSetId);

   return true;
}

void Vehicle::onRemove()
{
   if(idleSound)
      Sfx::Manager::Stop(manager, idleSound);

   if(throttleSound)
      Sfx::Manager::Stop(manager, throttleSound);

	if (muzzleNodes)
		{
			delete [] muzzleNodes;
			muzzleNodes = NULL;
		}

	if (passengerNodes)
		{
			delete [] passengerNodes;
			passengerNodes = NULL;
		}

	Parent::onRemove();
}

void Vehicle::clearMount (void)
{
	if (const char* script = scriptName("jump"))
		Console->evaluatef("%s(%s,\"%g %g %g\");", script, scriptThis(), 0, 0, 0);
}

bool Vehicle::initResources(GameBase::GameBaseData *in_data)
{
   if(!Parent::initResources(in_data))
      return false;

   data = dynamic_cast<VehicleData *>(in_data);
	
	setMass(data->mass);
	setDrag(data->drag);
	setDensity(data->density);
	
   if(isGhost())
	   collisionMask = SimInteriorObjectType | /*SimTerrainObjectType |*/
      					 SimPlayerObjectType | MoveableObjectType |
							 VehicleObjectType | StaticObjectType;
   else
      collisionMask = SimInteriorObjectType | /*SimTerrainObjectType |*/ 
      					 MineObjectType | SimPlayerObjectType |
							 TriggerObjectType | MoveableObjectType |
							 VehicleObjectType | StaticObjectType;

	CollisionDamageMask = MineObjectType | SimPlayerObjectType | 
							 	 MoveableObjectType | VehicleObjectType |
								 StaticObjectType;

   if(image.shape)
		{
			initGuns();
			initPassengers();

			if (isGhost())
				{
//					int node;
//					node = image.shape->getShape().findNode("dummy eye");
//					AssertWarn (node != -1, avar ("Eye point not found in vehicle %s", data->fileName));
					AssertWarn (image.shape->getShape().findNode("dummy eye") != -1,
									avar ("Eye point not found in vehicle %s", data->fileName));
				}
		}

	if (isGhost())
		idleSound = TSFX::PlayAt (data->idleSound, getTransform(), Point3F(0, 0, 0));

   exitNode = image.shape->getShape().findNode("dummy exit");
   pilotNode = image.shape->getNodeAtCurrentDetail("dummy pilot");
	gunNode = image.shape->getShape().findNode("dummy muzzle");
	eyeNode = image.shape->getShape().findNode("dummy eye");

	return true;
}

#define CAM_SPEED    7
#define CAM_MAX_DIST 5.0
//-----------------------------------------------------------------------------------

bool Vehicle::mountRender (void)
{
	return data->visibleDriver;
}

bool Vehicle::canMount (GameBase *)
{
	return !hasFocus;
}

bool Vehicle::canRide (GameBase *)
{
	return passengerCount < maxPassengers;
}

//-----------------------------------------------------------------------------------

void Vehicle::switchSoundState (int newState)
{
	if (!data || soundState == newState)
		return;

	if (throttleSound)
		Sfx::Manager::Stop(manager, throttleSound);

	switch (newState)
		{
			case RUN_SOUND:
				throttleSound = TSFX::PlayAt (data->moveSound, getTransform(), Point3F(0, 0, 0));
				break;

//			case IDLE_SOUND:
//				engineSound = TSFX::PlayAt (data->idleSound, getTransform(), Point3F(0, 0, 0));
//				break;
		};

	soundState = newState;
}

//-----------------------------------------------------------------------------------

void Vehicle::clientProcess (DWORD curTime)
{
	hasFocus = getControlClient();

//	if (!hasFocus)
//		{
//			lastPlayerMove.trigger = 0;
//			lastPlayerMove.jumpAction = 0;
//			lastPlayerMove.forwardAction = 0;
//			lastPlayerMove.backwardAction = 0;
//			lastPlayerMove.jetting = 0;
////			Console->printf ("----->Move cleared in Vehicle::clientProcess");
//		}

	updateTimers();

   if(interpDoneTime)
   {
      DWORD endInterpTime = min(curTime, interpDoneTime);

      if(lastProcessTime < endInterpTime)
      {
			float interval = endInterpTime - lastProcessTime;
			Point3F rot = getRot();
			rot.x += lastRot.x * interval;
			rot.y += lastRot.y * interval;
			rot.z += lastRot.z * interval;
			setRot (rot);
			Point3F pos = getPos();
			pos.x += lastPos.x * interval;
			pos.y += lastPos.y * interval;
			pos.z += lastPos.z * interval;
			setPos (pos);
         lastProcessTime = endInterpTime;
      }
      else
         interpDoneTime = 0;
   }

   while(lastProcessTime < curTime)
   {
      DWORD endTick = (lastProcessTime + 32) & ~0x1F;
		if(endTick > curTime)
         endTick = curTime;
		PlayerMove *pm;

		if (cg.psc->getControlObject() == this)
	      {
	         pm = cg.psc->getClientMove(lastProcessTime);
	         if(pm)
					{
						lastPlayerMove = *pm;
						updateMove (pm, 0.001 * ((((endTick - 1) & 0x1F) + 1) - (lastProcessTime & 0x1F)));
					}
	      }
		else
			{
//				float endTime = ((endTick - 1) & 0x1F) + 1;
//				float begTime = lastProcessTime & 0x1F;
//				float interval = 0.001 * (endTime - begTime);
//				updateMove (&lastPlayerMove, interval);

//		      int step = lastProcessTime & 0x1F;
//		      int endStep = ((endTick - 1) & 0x1F) + 1;
//
//		      if(step)
//					{
//						float interval = (endStep - step) / 32.0f;
//						Point3F rot = getRot();
//						rot.x += lastRot.x * interval;
//						rot.y += lastRot.y * interval;
//						rot.z += lastRot.z * interval;
//						setRot (rot);
//						Point3F pos = getPos();
//						pos.x += lastPos.x * interval;
//						pos.y += lastPos.y * interval;
//						pos.z += lastPos.z * interval;
//						setPos (pos);
//					}
//		      else
//		      {
		         // step the dude...er, "flier"... forward
		         pm = &lastPlayerMove;
		         if(pm)
						updateMove (pm, 0.001 * ((((endTick - 1) & 0x1F) + 1) - (lastProcessTime & 0x1F)));
		         else
		         {
		            lastPos.set(0,0,0);
		            lastRot.set(0,0,0);
		         }
//		         if(endStep != 32)
//						{
//							float interval = (endStep - 32) / 32.0f;
//							Point3F rot = getRot();
//							rot.x += lastRot.x * interval;
//							rot.y += lastRot.y * interval;
//							rot.z += lastRot.z * interval;
//							setRot (rot);
//							Point3F pos = getPos();
//							pos.x += lastPos.x * interval;
//							pos.y += lastPos.y * interval;
//							pos.z += lastPos.z * interval;
//							setPos (pos);
//						}
//		      }
			}

      lastProcessTime = endTick;
   }

	Parent::clientProcess(curTime);

	Sfx::Manager *man = Sfx::Manager::find (manager);
	if (throttleSound && man)
		{
	   	man->selectHandle (throttleSound);
	      man->setTransform (getTransform(), getLinearVelocity());
	   }

	if (idleSound && man)
		{
	   	man->selectHandle (idleSound);
	      man->setTransform (getTransform(), getLinearVelocity());
	   }
}

void Vehicle::serverProcess (DWORD)
{
	hasFocus = getControlClient();

	if (hasFocus)
		{
			if (desiredSpeed && soundState != RUN_SOUND)
				{
					soundState = RUN_SOUND;
					setMaskBits (SoundMask);
				}
			else
				if (!desiredSpeed && soundState != IDLE_SOUND)
					{
						soundState = IDLE_SOUND;
						setMaskBits (SoundMask);
					}
		}
	else
		if (soundState != NO_SOUND)
			{
				soundState = NO_SOUND;
				setMaskBits (SoundMask);
			}

   updateDamage(0.032);
	updateTimers();
	if (!hasFocus || updateDebt < -25)
		{
	      PlayerMove emptyMove;
	   
			updateMove (&emptyMove, 0.032);
	      
//	      lastPlayerMove = emptyMove;
	      updateSkip = 0;
		}
	else
		{
			updateSkip++;
			updateDebt--;
		}
}

void Vehicle::onCollisionNotify(GameBase* object)
{
	if (!isGhost() && object->getType() & ScriptCollisionMask)
		scriptOnCollision(object);
}

bool Vehicle::onSimTimerEvent (const SimTimerEvent*)
{
   return true;
}

bool Vehicle::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{
	if (!isGhost())
		setMaskBits(OrientationMask);
		
   return Parent::onSimObjectTransformEvent(event);
}

void Vehicle::setDamageLevel (float level)
{
//	if (!isGhost())
//		Parent::setDamageLevel (level);
//	if (damageLevel < data->maxDamage)
//	{
		Parent::setDamageLevel(level);
		setMaskBits (DamageMask);
		if (damageLevel >= data->maxDamage)
		{
			if (!isGhost())
				{
					Point3F mom;
//					Point3F rot = getRot();
//					RMat3F rmat (EulerF (rot.x, rot.y, rot.z));

//					m_mul (Point3F (0, speed, 5), rmat, &mom);
					mom = getLinearVelocity ();

//					if (const char* script = scriptName("onDestroyed"))
//						Console->evaluatef("%s(%s,\"%g %g %g\");", script, scriptThis(), mom.x,mom.y,mom.z);

					speed = desiredSpeed = 0;
					setMaskBits (StatusMask);

					soundState = NO_SOUND;
					setMaskBits (SoundMask);
				}
		}
//	}
}

bool Vehicle::processEvent(const SimEvent *event)
{
   switch(event->type)
   {
      onEvent(SimTimerEvent);
      onEvent(SimObjectTransformEvent);
      default:
         return Parent::processEvent(event);
   }
}

//-----------------------------------------------------------------------------------

bool Vehicle::onSimCollisionImageQuery (SimCollisionImageQuery *query)
{
//	if (damageLevel < data->maxDamage)
//		{
			query->count = 1;
			query->image[0] = &collisionImage;
			collisionImage.transform = getTransform();
			return true;
//		}
//	else
//		return false;
}

bool Vehicle::onSimRenderQueryImage (SimRenderQueryImage* query)
{
   if (isAtRest() == false && m_lastSunUpdated + 250 < wg->currentTime) {
      float sunAttenuation;
      ColorF positionalColor;
      bool overrideSun = getPositionalColor(positionalColor, sunAttenuation);
      updateSunOverride(overrideSun, sunAttenuation, positionalColor);
      m_lastSunUpdated = wg->currentTime;
   }

   image.shape->setDetailScale( prefShapeDetail );
   return Parent::processQuery(query);
}

void Vehicle::scriptOnCollision(GameBase* obj)
{
	if (obj && (obj->getType() & SimPlayerObjectType || obj->getType() & ItemObjectType))
		Parent::scriptOnCollision (obj);
}

bool Vehicle::processQuery(SimQuery *query)
{
   switch(query->type)
   {
      onQuery(SimCollisionImageQuery);
		onQuery(SimRenderQueryImage);
      default:
         return Parent::processQuery(query);
   }
}

//-----------------------------------------------------------------------------------


//bool Vehicle::processCollision(SimMovementInfo *info)
//{
//	float damage = data->ramDamage * ((trackVelocity < 0 ? -trackVelocity : trackVelocity) / data->maxSpeed);
//
//	for (int i = 0; i < info->collisionList.size(); i++)
//		{
//			if (damage && info->collisionList[i].object->getType() & CollisionDamageMask)
//				{
//					ShapeBase* pShape = dynamic_cast<ShapeBase*>(info->collisionList[i].object);
//					if (pShape)
//						{
////							info->collidedList.push_back(info->collisionList[i].object);
//				         info->collisionList.erase(i);
//
//  						   pShape->applyDamage(data->ramDamageType, damage, getPos(), getLinearVelocity(), Point3F(0,0,0), getOwnerId());
//  						   applyDamage(data->ramDamageType, damage / 2, getPos(), getLinearVelocity(), Point3F(0,0,0), getOwnerId());
//  							desiredSpeed = 0;
//							speed = 0;
//							trackVelocity = 0;
//  							setMaskBits (StatusMask);
//  							continue;
//  						}
//  				}
//
//			if (info->collisionList[i].object->getType() & SimInteriorObjectType)
//				{
//					CollisionSurfaceList& sList = info->collisionList[i].surfaces;
//
//					Point3F normal (0, 0, 0);
//					TMat3F trans = getTransform ();
//					Point3F forward;
//					trans.getRow (1, &forward);
//					if (trackVelocity < 0)
//						forward = -forward;
//					forward.normalize();
//					for (int j = 0; j < sList.size(); j++)
//						{
//							Point3F tNorm;
//							m_mul(sList[j].normal, static_cast<RMat3F&>(sList.tWorld), &tNorm);
//							tNorm.normalize();
//							if (m_dot (forward, tNorm) < 0.1)
//								{
//									normal += tNorm;
//									if (normal.x || normal.y || normal.z)
//										normal.normalize();
//								}
//						}
//
////					float dot = m_dot (forward, normal);
////					if (normal.z > 0.25 && ((!normal.x && !normal.y && !normal.z) || dot > -0.75))
//					if (normal.z > 0.25)
//						{
//				         info->collisionList.erase(i);
//							continue;
//						}
//					else
//						if (damage && trackVelocity > data->maxSpeed / 2)
//							{
////								Console->printf("Damage type 2 : trackVelocity = %f, damage = %f", trackVelocity, damage);
//	  						   applyDamage(data->ramDamageType, damage / 2, getPos(), getLinearVelocity(), Point3F(0,0,0), getOwnerId());
//								speed = desiredSpeed = 0;
//								trackVelocity = 0;
//							}
//				}
//		}
//
//	return Parent::processCollision (info);
//}


//-----------------------------------------------------------------------------------

void Vehicle::setPlayerControl(bool ctrl)
{
   // called only on the client side, when
   // the player takes control of this object - indicates we
   // should get input from the global psc.

   hasFocus = ctrl;
}

void Vehicle::initGuns (void)
{
	TSShapeInstance *si = image.shape;
	char muzzleStr[20];
	int node;
	gunCount = 0;

	sprintf (muzzleStr, "dummy muzzle%d", gunCount + 1);
	node = si->getShape().findNode (muzzleStr);

	while (si && node != -1)
		{
			gunCount++;
			sprintf (muzzleStr, "dummy muzzle%d", gunCount + 1);
			node = si->getShape().findNode (muzzleStr);
		}

	if (gunCount > 0)
		{
			muzzleNodes = new int[gunCount];
			for (int i = 0; i < gunCount; i++)
				{
					sprintf (muzzleStr, "dummy muzzle%d", i + 1);
					muzzleNodes[i] = si->getShape().findNode (muzzleStr);
				}
		}
}

void Vehicle::initPassengers (void)
{
	TSShapeInstance *si = image.shape;
	char muzzleStr[25];
	int node;
	maxPassengers = 0;

	sprintf (muzzleStr, "dummy passenger%d", maxPassengers + 1);
	node = si->getShape().findNode (muzzleStr);

	while (si && node != -1)
		{
			maxPassengers++;
			sprintf (muzzleStr, "dummy passenger%d", maxPassengers + 1);
			node = si->getShape().findNode (muzzleStr);
		}

	if (maxPassengers > 0)
		{
			passengerNodes = new int[maxPassengers];
			for (int i = 0; i < maxPassengers; i++)
				{
					sprintf (muzzleStr, "dummy passenger%d", i + 1);
					passengerNodes[i] = si->getShape().findNode (muzzleStr);
				}
		}
}

void Vehicle::changePassengerCount (int change)
{
	passengerCount += change;
	if (passengerCount < 0)
		passengerCount = 0;
}

bool Vehicle::cameraZoomAndCrosshairsEnabled()
{
   return true;
}

void Vehicle::getObjectMountTransform(int mountPoint, TMat3F *transform)
{
	int node;

	switch (mountPoint)
		{
			case 0:
			   node = exitNode;
				break;
			case 1:
			   node = pilotNode;
				break;
			default:
				{
//			      static char muzzleStr[] = "dummy passengerX";
//			      muzzleStr[strlen(muzzleStr) - 1] = '0' + mountPoint - 1;
//			      node = image.shape->getShape().findNode(muzzleStr);
					node = passengerNodes [mountPoint - 2];
				}
				break;
		}

	if (node != -1)
	{
		TMat3F nodeTrans = image.shape->getTransform (node);
      m_mul(nodeTrans, getTransform(), transform);
	}
	else
		*transform = getTransform();

}

bool Vehicle::getMuzzleTransform(int slot, TMat3F* transform)
{
   slot; // guns are guns
   int node;
   if(!gunCount)
      node = gunNode;
   else
   {
//      static char muzzleStr[] = "dummy muzzleX";
//      muzzleStr[strlen(muzzleStr) - 1] = '0' + (fireCount % gunCount) + 1;
//      node = image.shape->getShape().findNode(muzzleStr);
		node = muzzleNodes [fireCount % gunCount];
   }
   if(node != -1)
   {
		TMat3F nodeTrans = image.shape->getTransform (node);
      m_mul(nodeTrans, getTransform(), transform);
      return true;
   }   
	else
   {
		*transform = getTransform();
      return false;
   }
}

void Vehicle::getCameraTransform(float camDist, TMat3F *transform)
{
   int node = eyeNode;
	if (node != -1) {
		TMat3F nodeTrans = image.shape->getTransform (node);
		m_mul(nodeTrans, getTransform(), transform);

		validateEyePoint (transform, camDist * 2);
		if (camDist) {
   		EulerF angles;
   		Point3F p;

   		p = transform->p;
   		transform->angles(&angles);
			angles.x /= 1.25;
			angles.y /= 2;
   		transform->set (angles, p);
		}
	}
	else
		*transform = getTransform();

}

const TMat3F &Vehicle::getEyeTransform()
{
   static TMat3F ret;
	TMat3F temp;

   int node = eyeNode;
	if (node != -1)
		temp = image.shape->getTransform(node);
	else
		temp.identity();

	m_mul(temp, getTransform(), &ret);
   return ret;
}

void Vehicle::ghostSetMove(PlayerMove *move, Point3F *newPos, Point3F *newRot, float newSpeed, float newLift, int skipCount)
{
//	float oldLift = lift;
//	float oldSpeed = speed;
//   lastPlayerMove = *move;
//
//   updateSkip = skipCount;
//
//	// new values are actually from back in time.
//	lift = newLift;
//   speed = newSpeed;
//	lastRot = getRot();
//	lastPos = getPos();
//	setRot(*newRot);
//   setPos(*newPos);
//
//   DWORD t = 0;    
//   DWORD endTime = cg.predictForwardTime;
//   interpDoneTime = 0;
//
//	// predict forward from reset position to current time
//   while(t < endTime)
//   {
//	   updateMove (&lastPlayerMove, 0.032);
//      t += 32;
//   }
//
//	// setup offsets to add in during interpolation.
//   if(cg.interpolateTime)
//   {
//		Point3F tempRot = getRot();
//
//      tempRot.x -= lastRot.x;
//      if(tempRot.x < -M_PI)
//         tempRot.x += M_2PI;
//      if(tempRot.x > M_PI)
//         tempRot.x -= M_2PI;
//      tempRot.x /= cg.interpolateTime;
//
//      tempRot.y -= lastRot.y;
//      if(tempRot.y < -M_PI)
//         tempRot.y += M_2PI;
//      if(tempRot.y > M_PI)
//         tempRot.y -= M_2PI;
//      tempRot.y /= cg.interpolateTime;
//
//      tempRot.z -= lastRot.z;
//      if(tempRot.z < -M_PI)
//         tempRot.z += M_2PI;
//      if(tempRot.z > M_PI)
//         tempRot.z -= M_2PI;
//      tempRot.z /= cg.interpolateTime;
//
//		savedRot = getRot();
//		setRot(lastRot);
//      lastRot = tempRot;
//
//		Point3F tempPos = getPos();
//		tempPos.x -= lastPos.x;
//		tempPos.x /= cg.interpolateTime;
//		tempPos.y -= lastPos.y;
//		tempPos.y /= cg.interpolateTime;
//		tempPos.z -= lastPos.z;
//		tempPos.z /= cg.interpolateTime;
//		
//		savedPos = getPos();
//		setPos (lastPos);
//		lastPos = tempPos;
//
//      lastProcessTime = cg.currentTime & ~0x1F;
//      interpDoneTime = lastProcessTime + cg.interpolateTime;
//
//		speed = oldSpeed;
//		lift = oldLift;
//   }
//   else
//   {
//		interpDoneTime = 0;
//      lastProcessTime = cg.currentTime & ~0x1F;
//      lastPitch = 0;
//      lastRot.set (0, 0, 0);
//		lastPos.set (0, 0, 0);
//   }
   lastPlayerMove = *move;

	float oldSpeed = speed;
	float oldLift = lift;
	speed = newSpeed;
	lift = newLift;

   lastRot = getRot();
   lastPos = getTransform().p;

   setPos(*newPos);
   setRot(*newRot);

   DWORD t = 0;
   DWORD endTime = cg.predictForwardTime + 32;
   interpDoneTime = 0;

   updateSkip = 0;
   while(t < endTime)
   {
      updateMove(&lastPlayerMove, 0.032);
      t += 32;
   }
   updateSkip = skipCount;

   if(cg.interpolateTime)
   {
		lastProcessTime = cg.currentTime & ~0x1F;
      interpDoneTime = (cg.currentTime + cg.interpolateTime + 0x1F) & ~0x1F;
      DWORD interpTime = interpDoneTime - lastProcessTime;

		Point3F tempPos = lastPos;
		savedPos = getTransform().p;
		lastPos = (getTransform().p - lastPos) / interpTime;
		setPos (tempPos);

		Point3F tempRot = lastRot;
		
		lastRot = getRot() - lastRot;

      if(lastRot.x < -M_PI)
         lastRot.x += M_2PI;
      if(lastRot.x > M_PI)
         lastRot.x -= M_2PI;
      lastRot.x /= interpTime;

      if(lastRot.y < -M_PI)
         lastRot.y += M_2PI;
      if(lastRot.y > M_PI)
         lastRot.y -= M_2PI;
      lastRot.y /= interpTime;

      if(lastRot.z < -M_PI)
         lastRot.z += M_2PI;
      if(lastRot.z > M_PI)
         lastRot.z -= M_2PI;
      lastRot.z /= interpTime;

		savedRot = getRot();
		setRot (tempRot);
   }
   else
   {
      interpDoneTime = 0;
      lastProcessTime = cg.currentTime & ~0x1F;
      lastPos.set(0,0,0);
      lastRot.set(0,0,0);
   }

	speed = oldSpeed;
	lift = oldLift;
}

void Vehicle::serverUpdateMove(PlayerMove *moves, int moveCount)
{
   while(moveCount--)
   {
      if(updateDebt > 5)
         break;
      updateDebt++;

      updateMove(&lastPlayerMove, 0.032);

      lastPlayerMove = *moves++;
   }

   updateSkip = 0;
}


bool Vehicle::updateMove(PlayerMove *, float interval)
{
//	if (!isGhost() && damageLevel >= data->maxDamage)
//		return false;

	updateEnergy(interval);
	updateShieldThread(interval);
	updateScriptThreads(interval);

	return true;
}

bool Vehicle::processBlocker(SimObject* obj)
{
	if (GameBase* go = dynamic_cast<GameBase*>(obj))
		{
			if (go->getType() == CorpseObjectType) {
				// Corpses don't block
				go->clearRestFlag();
				return false;
			}
			TMat3F mat = getTransform();
			Point3F upVec;
			mat.getRow (1, &upVec);
         go->applyImpulse(upVec);
			ShapeBase* so = dynamic_cast<ShapeBase*>(go);
			if (so)
				{
					// Ignore mounted players.
					if (so->getType().test(SimPlayerObjectType) &&	static_cast<Player*>(so)->getMountObject() == this)
						return false;

					Point3F diffVel = getLinearVelocity() - so->getLinearVelocity();
					float damageSpeed = diffVel.lenf();
					if (damageSpeed > data->maxSpeed)
						damageSpeed = data->maxSpeed;

					if (damageSpeed < data->maxSpeed / 4)
						damageSpeed = 0;

					float damage = damageSpeed ? data->ramDamage * (damageSpeed / data->maxSpeed) : 0;

				   so->applyDamage(data->ramDamageType, damage, getPos(), getLinearVelocity(), Point3F(0,0,0), getId());
				}
			return true;
		}
	return false;
}

//-----------------------------------------------------------------------------------

void Vehicle::shoot (void)
{
	if (data && data->projectile.type == -1)
		{
			if (!isGhost())
				if (const char* script = scriptName("onFire"))
					Console->executef(2, script, scriptThis());
		}
	else
		if (data && waitTime <= manager->getCurrentTime())
			{
				TMat3F muzzleTransform;
				getMuzzleTransform(0, &muzzleTransform);

//				Point3F start = muzzleTransform.p;
//				muzzleTransform = getEyeTransform ();
//				aimedTransform (&muzzleTransform, start);
//				muzzleTransform.p = start;

				muzzleTransform.p += getLinearVelocity() * 0.096;

				Projectile* bullet = createProjectile(data->projectile);
				bullet->initProjectile (muzzleTransform, getLinearVelocity(), getId());

            SimGroup *grp = NULL;
            if(SimObject *obj = manager->findObject("MissionCleanup"))
               grp = dynamic_cast<SimGroup*>(obj);
            if(!manager->registerObject(bullet))
               delete bullet;
            else
            {
               if(grp)
                  grp->addObject(bullet);
               else
                  manager->addObject(bullet);
            }

				waitTime = manager->getCurrentTime() + data->reloadDelay;

				fireCount++;
				setMaskBits (ShootingMask);
			}
}

//----------------------------------------------------------------------------

void Vehicle::readPacketData(BitStream *bstream)
{
   BYTE pingStatus = bstream->readInt(2);
   setSensorPinged(pingStatus);

	Point3F rot;
	Point3F pos;
	bstream->read (sizeof (Point3F), &rot);
	setRot (rot);
	bstream->read (sizeof (Point3F), &desiredRot);
	bstream->read (sizeof (Point3F), &pos);
	setPos (pos);
	setTransform (TMat3F(EulerF(rot.x, rot.y, rot.z), pos));
//	bstream->read (sizeof (Point3F), &velocityVector);
	float speedFrac = bstream->readFloat (16);
	speed = (speedFrac * (data->maxSpeed - data->minSpeed)) + data->minSpeed;
	speedFrac = bstream->readFloat (16);
	desiredSpeed = (speedFrac * (data->maxSpeed - data->minSpeed)) + data->minSpeed;
	energy = bstream->readFloat (16) * data->maxEnergy;

	float dl;
	dl = bstream->readInt(6) / 63.0f;
//	bstream->read (&dl);
	if (dl > damageLevel && data && data->damageSound != -1)
      TSFX::PlayAt(data->damageSound, getTransform(), Point3F(0, 0, 0));
	damageLevel = dl;

	int fc;
	fc = bstream->readInt (2);
	if (fc != fireCount && data && data->fireSound != -1)
      TSFX::PlayAt(data->fireSound, getTransform(), Point3F(0, 0, 0));
	
	fireCount = fc;

	int newState = bstream->readInt (2);
	switchSoundState (newState);
}

void Vehicle::writePacketData(BitStream *bstream)
{
   BYTE pingStatus = getSensorPinged();
   bstream->writeInt(pingStatus, 2);

	bstream->write(sizeof(Point3F), getRot());
	bstream->write(sizeof(Point3F), desiredRot);
	bstream->write(sizeof(Point3F), getPos());
//	bstream->write(sizeof(Point3F), velocityVector);
	bstream->writeFloat (((speed - data->minSpeed) / (data->maxSpeed - data->minSpeed)), 16);
	bstream->writeFloat (((desiredSpeed - data->minSpeed) / (data->maxSpeed - data->minSpeed)), 16);
	bstream->writeFloat (energy / data->maxEnergy, 16);
	bstream->writeInt ((damageLevel/data->maxDamage) * 63, 6);
//	bstream->write (damageLevel);
	bstream->writeInt (fireCount, 2);
	bstream->writeInt (soundState, 2);
}

//-----------------------------------------------------------------------------------

int Vehicle::getDatGroup()
{
   return DataBlockManager::VehicleDataType;
}

//-----------------------------------------------------------------------------------

DWORD Vehicle::packUpdate(Net::GhostManager *g, DWORD mask, BitStream *stream)
{
   bool myClient = getControlClient() == int(g->getOwner()->getId());

	if(myClient && !(mask & (InitialUpdate | BaseStateMask)))
	{
		stream->writeFlag (true);
		return 0;
	}
	else
		stream->writeFlag (false);

	DWORD oldMask = mask;
	mask &= ~(PositionMask | RotationMask);
	Parent::packUpdate (g, mask, stream);
	mask = oldMask;
		
	if (mask == 0xffffffff)
      packDatFile(stream);

   if(stream->writeFlag(mask & OrientationMask))
	{
	   stream->write(sizeof(Point3F), getRot ());
	   stream->write(sizeof(Point3F), getPos ());
//		stream->write (sizeof (Point3F), desiredRot);
//		stream->write (sizeof (Point3F), getLinearVelocity());
		
//		if(stream->writeFlag(mask != 0xffffffff && !myClient && hasFocus))
		if(stream->writeFlag(mask != 0xffffffff && !myClient))
		{
			if (!hasFocus)
				{
					lastPlayerMove.trigger = 0;
					lastPlayerMove.jumpAction = 0;
					lastPlayerMove.forwardAction = 0;
					lastPlayerMove.backwardAction = 0;
					lastPlayerMove.jetting = 0;
//					Console->printf ("----->Move cleared in Vehicle::packUpdate");
				}

	      lastPlayerMove.write(stream, false);
			stream->writeFloat (((speed - data->minSpeed) / (data->maxSpeed - data->minSpeed)), 10);
			stream->write (lift);
	      stream->writeInt(min(updateSkip, 15), 4);
		}
	}
		
	if (stream->writeFlag (mask & StatusMask))
	{
		if (data)
		{
			stream->writeFloat (((speed - data->minSpeed) / (data->maxSpeed - data->minSpeed)), 10);
			stream->writeFloat (((desiredSpeed - data->minSpeed) / (data->maxSpeed - data->minSpeed)), 10);
		}
		else
		{
			stream->writeFloat (0, 10);
			stream->writeFloat (0, 10);
		}
	}
		
	if (stream->writeFlag (mask & ShootingMask))
		stream->writeInt (fireCount, 2);
		
	if (stream->writeFlag (mask & DamageMask))
		stream->writeInt ((damageLevel/data->maxDamage) * 63, 6);
//		stream->write (damageLevel);

	if (stream->writeFlag (mask & SoundMask))
		stream->writeInt (soundState, 2);

	return 0;
}

void Vehicle::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
	if (stream->readFlag ())
   	return;

   Parent::unpackUpdate(gm, stream);
	
	if (!manager)
		unpackDatFile (stream);
		
   TMat3F tmat;
   if(stream->readFlag())
	{
		Point3F rot;
		Point3F pos;
//		Point3F vel;
	
	   stream->read (sizeof(Point3F), &rot);
		stream->read (sizeof(Point3F), &pos);
//		stream->read (sizeof (Point3F), &desiredRot);
//		stream->read (sizeof (Point3F), &vel);
//		setLinearVelocity (vel);

		if(stream->readFlag())
		{
			PlayerMove move;
			float newSpeed;
			float newLift;

			move.read(stream, false);
			float speedFrac = stream->readFloat (10);
			newSpeed = (speedFrac * (data->maxSpeed - data->minSpeed)) + data->minSpeed;
//			stream->read (&newSpeed);
			stream->read (&newLift);
			updateSkip = stream->readInt(4);
			ghostSetMove(&move, &pos, &rot, newSpeed, newLift, updateSkip);
		}
		else
		{
			lastPos.set (0, 0, 0);
			lastRot.set (0, 0, 0);
			setRot (rot);
			setPos (pos);
		}
	}

	if (stream->readFlag ())
	{
		if (data)
		{
			float speedFrac = stream->readFloat (10);
			speed = (speedFrac * (data->maxSpeed - data->minSpeed)) + data->minSpeed;
			speedFrac = stream->readFloat (10);
			desiredSpeed = (speedFrac * (data->maxSpeed - data->minSpeed)) + data->minSpeed;
		}
		else
		{
			stream->readFloat (10);
			stream->readFloat (10);
		}
	}

	if (stream->readFlag())
	{
		int fc;
		fc = stream->readInt (2);
		if (manager && fc != fireCount && data && data->fireSound != -1)
	      TSFX::PlayAt(data->fireSound, getTransform(), Point3F(0, 0, 0));

		fireCount = fc;
	}

	if (stream->readFlag())
	{
		float dl;

		dl = stream->readInt(6) / 63.0f;
//		stream->read (&dl);

		if (dl > damageLevel && data && data->damageSound != -1)
	      TSFX::PlayAt(data->damageSound, getTransform(), Point3F(0, 0, 0));

//		if (data && dl >= data->maxDamage && damageLevel < data->maxDamage)
//			blowUp ();

		damageLevel = dl;
	}

	if (stream->readFlag ())
		{
			int newState = stream->readInt (2);
			switchSoundState (newState);
		}
}

Persistent::Base::Error Vehicle::read(StreamIO &sio, int, int)
{
	Parent::read (sio, 0, 0);
	
   DWORD vers;
   sio.read(&vers);
	
   TMat3F tmat;
   sio.read(sizeof(TMat3F), &tmat);
	
   setTransform(tmat);
	setPos (tmat.p);
	savedPos = getPos ();

	EulerF angles;
	tmat.angles (&angles);
	setRot (Point3F (angles.x, angles.y, angles.z));
	desiredRot = getRot();

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


Persistent::Base::Error Vehicle::write(StreamIO &sio, int, int)
{
   Parent::write(sio, 0, 0);
   sio.write(DWORD(0));
   sio.write(sizeof(TMat3F), &getTransform());
	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}

//----------------------------------------------------------------------------
Vehicle::VehicleData::VehicleData ()
{
	fireSound = -1;
	damageSound = -1;
	mountSound = -1;
	dismountSound = -1;
	idleSound = -1;
	moveSound = -1;
	projectile.type = -1;
	projectile.dataType = -1;
}

void Vehicle::VehicleData::pack(BitStream *stream)
{
   Parent::pack(stream);
	
   stream->write(mass);
   stream->write(drag);
   stream->write(density);
   stream->write(maxSpeed);
   stream->write(minSpeed);
	stream->write (visibleDriver);
	stream->write (driverPose);
	sg.dbm->writeBlockId (stream, DataBlockManager::SoundDataType, fireSound);
	sg.dbm->writeBlockId (stream, DataBlockManager::SoundDataType, damageSound);
	sg.dbm->writeBlockId (stream, DataBlockManager::SoundDataType, mountSound);
	sg.dbm->writeBlockId (stream, DataBlockManager::SoundDataType, dismountSound);
	sg.dbm->writeBlockId (stream, DataBlockManager::SoundDataType, idleSound);
	sg.dbm->writeBlockId (stream, DataBlockManager::SoundDataType, moveSound);

	for (int i = 0; i < 4; i++)
		stream->write (sizeof (Point3F), &suspension[i]);
}

void Vehicle::VehicleData::unpack(BitStream *stream)
{
   Parent::unpack(stream);
	
   stream->read(&mass);
   stream->read(&drag);
   stream->read(&density);
   stream->read(&maxSpeed);
   stream->read(&minSpeed);
	stream->read (&visibleDriver);
	stream->read (&driverPose);
	fireSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	damageSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	mountSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	dismountSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	idleSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	moveSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);

	for (int i = 0; i < 4; i++)
		stream->read (sizeof (Point3F), &suspension[i]);
}

