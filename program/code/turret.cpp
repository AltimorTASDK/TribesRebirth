//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifdef _BORLANDC_
#pragma warn -inl
#pragma warn -aus
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4710)
#pragma warning(disable : 4101)
#endif

#include <console.h>
#include <simResource.h>

#include "Player.h"
#include "FearPlayerPSC.h"
#include "PlayerManager.h"
#include "Projectile.h"
#include "fearGlobals.h"
#include "tsfx.h"
#include "netGhostManager.h"
#include "dataBlockManager.h"
#include "Turret.h"

#include "Fear.Strings.h"
#include "sfx.strings.h"


//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG( Turret, FearTurretPersTag );

inline float m_clamp(float v, float a, float b)
{
	return (v < a)? a: (v > b)? b: v;
}	

//----------------------------------------------------------------------------

Turret::Turret ()
{
	elevationThread = NULL;
	rotationThread = NULL;
	animThread = NULL;

	turretElevation = 0;
	turretRotation = M_PI;

	waitTime = 0;

	maxElevation = M_PI / 4;
	minElevation = -M_PI / 4;

	maxRotation = 0;
	minRotation = 0;

	hasFocus = false;
	fireCount = 0;

	sleepTime = 0;

	projectileSpeed = 40;

	primaryTarget = NULL;
	targetsTracked = 0;

	state = RETRACTED;

	data = NULL;
	whirSound = 0;
   fireSound = 0;

	cameraNode = -1;
	gunNode = -1;
   fireSequence = -1;
   powerSequence = -1;

}


Turret::~Turret ()
{
}


//----------------------------------------------------------------------------

bool Turret::initResources (GameBase::GameBaseData *in_data)
{
   if(!Parent::initResources(in_data))
      return false;

   data = dynamic_cast<TurretData *>(in_data);

	if (!isGhost())
		{
		   DataBlockManager *dbm = (DataBlockManager *) manager->findObject(DataBlockManagerId);
			if (data->projectile.type != -1)
			{
				Projectile::ProjectileData* ptr = static_cast<Projectile::ProjectileData *>
					(dbm->lookupDataBlock(data->projectile.dataType,data->projectile.type));
				projectileSpeed = ptr->getTerminalVelocity ();
			}
			else
				projectileSpeed = 50;
		}
		
	if (!rotationThread)
		{
			rotationThread = Parent::createThread (0);
			if (rotationThread)
				{
					rotationThread->setTimeScale( 1.0f );
					rotationThread->SetSequence ("turn", 0.0);

//					const TSShape &ts_shape = image.shape->getShape();
//					int node = ts_shape.findNode ("kill 15");
//					AssertFatal (node != -1, "Can't find turret base...");
//					if (node != -1)
//						{
//							ts_shape.selectDetail (15);
//							rotationThread->SetPosition (1.0);
//							rotationThread->UpdateSubscriberList ();
//							image.shape->animate ();
//							const TMat3F &mat = image.shape->getTransform(node);
//							EulerF angles;
//							mat.angles(&angles);
//
//							maxRotation = angles.z;
//							if (maxRotation > M_PI)
//								maxRotation -= (float)M_2PI;
//
//							rotationThread->SetPosition (0.0);
//							rotationThread->UpdateSubscriberList ();
//							image.shape->animate ();
//							const TMat3F &mat2 = image.shape->getTransform(node);
//							mat2.angles(&angles);
//
//							minRotation = angles.z;

							rotationThread->SetPosition (0);
							image.shape->animate ();

							minRotation = maxRotation = 0;
							turretRotation = minRotation + ((maxRotation - minRotation) / 2);
//						}
				}
		}

	if (!elevationThread)
		{
			elevationThread = Parent::createThread (0);
			if (elevationThread)
				{
					elevationThread->setTimeScale( 1.0f );
					elevationThread->SetSequence ("elevate", 0.0);
					
					const TSShape &ts_shape = image.shape->getShape();
					int node = ts_shape.findNode ("dummy muzzle");
					AssertFatal (node != -1, "No dummy muzzle");
					if (node != -1)
						{
							elevationThread->SetPosition (0);
							image.shape->setDetailLevel(0);
							image.shape->animate ();
							const TMat3F &mat = image.shape->getTransform(node);
							EulerF angles;
							mat.angles(&angles);
							
							minElevation = angles.x;
							
							elevationThread->SetPosition (1.0);
							image.shape->animate ();
							const TMat3F &mat2 = image.shape->getTransform(node);
							mat2.angles(&angles);
							
							maxElevation = angles.x;
							
							if (minElevation > M_PI)
								minElevation -= (float)M_2PI;

							AssertFatal (maxElevation > minElevation, "Turret - maxElevation must be greater than minElevation");

//							if (maxElevation < minElevation)
//								{
//									float temp = minElevation;
//									minElevation = maxElevation;
//									maxElevation = temp;
//								}

							turretElevation = 0;
							elevationThread->SetPosition ((turretElevation - minElevation) / (maxElevation - minElevation));
							image.shape->animate ();
						}
				}
		}

	if (!animThread && image.shape->getShape().lookupName ("fire") != -1 && image.shape->getShape().lookupName ("power") != -1)
	{
		animThread = Parent::createThread (0);
      fireSequence = animThread->GetSequenceIndex("fire");
      powerSequence = animThread->GetSequenceIndex("power");
   }

	setPowerThread();

	if (animThread)
		animThread->SetPosition (0.0);

	if (!isGhost())
		scriptOnAdd();

	cameraNode = image.shape->getShape().findNode("dummy eye");
   gunNode = image.shape->getShape().findNode("dummy muzzle");

	return true;
}


//----------------------------------------------------------------------------

const Vector3F &Turret::getCompassRotation (void)
{
	static Vector3F rot;

	rot.set (0, 0, turretRotation + getRot().z);

	return rot;
}

//----------------------------------------------------------------------------

bool Turret::onAdd ()
{
	if (!Parent::onAdd ())
		return false;

   if (isGhost() == false) {
      m_fireState  = Waiting;
      m_beganState = wg->currentTime;
   }

	return true;
}

void Turret::onRemove()
{
	if (!isGhost())
		scriptOnRemove();
	Parent::onRemove();
}


//----------------------------------------------------------------------------

void Turret::setFireThread (void)
{
//	AssertFatal (image.shape->getShape().lookupName ("fire") != -1, "Can't find turret fire anim...");
	if (fireSequence != -1)
	{
		animThread->setTimeScale (1.0);
		animThread->SetSequence (fireSequence, 0.0);
		animThread->SetPosition (1.0);
	}

	if (animThread)
		animThread->setPriority (0);
}

bool Turret::setPowerThread (void)
{
//	AssertFatal (image.shape->getShape().lookupName ("power") != -1, "Can't find turret power anim...");
	if (powerSequence != -1)
		{
			float pos = animThread->getPosition ();
			animThread->setTimeScale (1.0);
			animThread->SetSequence (powerSequence, 0.0);
			animThread->SetPosition (pos);
			animThread->setPriority (-1);

			if (elevationThread)
				{
					turretElevation = 0;
					elevationThread->SetPosition ((turretElevation - minElevation) / (maxElevation - minElevation));
//					elevationThread->SetPosition (0.5);
				}

			return true;
		}
	else
		return false;
}

//----------------------------------------------------------------------------

void Turret::setTeam(int teamId)
{
   Parent::setTeam(teamId);
   // should nullify its target... if necessary
}

bool Turret::isControlable (void)
{
	return !getControlClient() && getState() == StaticBase::Enabled && isActive();
}

//----------------------------------------------------------------------------

bool Turret::setPrimaryTarget (Player *target)
{
	primaryTarget = target;
	
	return true;
}

//----------------------------------------------------------------------------

float Turret::elevation (float dx, float dy, float dz)
{
	float angle = -m_atan (dx, dy);
	float newX;
	
	newX = (dx * m_cos (angle)) - (dy * m_sin (angle));
	
	return m_atan (-dz, newX) - (M_PI /  2);
}


float Turret::rotation (float dx, float dy)
{
	float angle = m_atan (dx, dy) + (M_PI / 2);
	
	return angle;
}


bool Turret::inDeadZone (float desRot, float desEle)
{
	bool result = false;
	
	if (desRot > M_PI)
		desRot -= (float)M_2PI;
		
	if (maxRotation != minRotation)
		if ((desRot > 0 && desRot > maxRotation) ||
			 (desRot < 0 && desRot < minRotation))
			result = true;
			
	if (desEle > maxElevation || desEle < minElevation)
		result = true;
		
	return result;
}


void Turret::leadPosition (Point3F pos, Point3F vel, float dist, Point3F *leadPos)
{
	SimTime elapse;
	
	elapse = dist / projectileSpeed;
	
	*leadPos = pos + (vel * elapse);
}


void Turret::wrapRotation (void)
{
	while (turretRotation > M_2PI)
		turretRotation -= M_2PI;
	
	while (turretRotation < 0)
		turretRotation += M_2PI;
}


void Turret::wrapElevation (void)
{
	while (turretElevation < -M_PI)
		turretElevation += M_2PI;
		
	while (turretElevation > M_PI)
		turretElevation -= M_2PI;
}


void Turret::shoot (bool playerControlled, Player* targetPlayer)
{
   if (data && data->isSustained == false) {
	   if (data && data->projectile.type == -1)
	   	{
	   		if (!isGhost())
	   			if (const char* script = scriptName("onFire"))
	   				Console->executef(2, script, scriptThis());
	   	}
	   else
	   	{
	   		float energy = getEnergy();
	   		if (waitTime <= manager->getCurrentTime() && data && energy >= data->minGunEnergy && data->projectile.type != -1)
	   			{
                  TMat3F muzzleTransform;
	   				getMuzzleTransform(0, &muzzleTransform);
	   				Projectile* bullet = createProjectile(data->projectile);

	   				if (!playerControlled && data->deflection)
	   					{
	   						static Random random;
	   						EulerF angles;
	   					   muzzleTransform.angles (&angles);
	   						angles.x += (random.getFloat() - 0.5) * M_2PI * data->deflection;
	   						angles.z += (random.getFloat() - 0.5) * M_2PI * data->deflection;
	   						muzzleTransform.set (angles, muzzleTransform.p);
	   					}
	   				else
	   					if (playerControlled)
	   						{
	   							Point3F start = muzzleTransform.p;
	   							muzzleTransform = getEyeTransform ();
	   							aimedTransform (&muzzleTransform, start);
	   							muzzleTransform.p = start;
	   						}

	   				bullet->initProjectile (muzzleTransform, Point3F (0, 0, 0), getId());

	   	         if (bullet->isTargetable() == true) {
	   	            if (targetPlayer != NULL) {
	   						if (GameBase* mo = targetPlayer->getMountObject())
	   		               bullet->setTarget(static_cast<ShapeBase*>(mo));
	   						else
	   		               bullet->setTarget(targetPlayer);
                     } else if (playerControlled) {
                        ShapeBase* pClosest   = NULL;
                        Point3F    closeHisPos;
                        float      closestVal = -2.0f;
                        SimSet::iterator itr;
                     
                        Point3F lookDir;
                        getEyeTransform().getRow(1, &lookDir);
                        lookDir.normalize();

                        SimContainerQuery collisionQuery;
                        SimCollisionInfo  info;
                        collisionQuery.id     = getId();
                        collisionQuery.type   = -1;
                        collisionQuery.mask   = Projectile::csm_collisionMask;
                        collisionQuery.detail = SimContainerQuery::DefaultDetail;
                        collisionQuery.box.fMin = getEyeTransform().p;
                        SimContainer* pRoot = (SimContainer*)manager->findObject(SimRootContainerId);

                        SimSet* pSet = dynamic_cast<SimSet*>(manager->findObject(PlayerSetId));
                        AssertFatal(pSet != NULL, "No player set?");
                        for (itr = pSet->begin(); itr != pSet->end(); itr++) {
                           Player* pPlayer = dynamic_cast<Player*>(*itr);

                           if (!pPlayer || pPlayer->getVisibleToTeam(getTeam()) == false)
                              continue;

                           collisionQuery.box.fMax = pPlayer->getBoxCenter();
                           if (pRoot->findLOS(collisionQuery, &info, SimCollisionImageQuery::High) == true) {
                              if (info.object != (SimObject*)pPlayer)
                                 continue;
                           }

                           Point3F hisPos = pPlayer->getBoxCenter();
                           hisPos -= getLinearPosition();
                           hisPos.normalize();

                           float prod = m_dot(hisPos, lookDir);
                           if (prod > 0.0f && prod > closestVal) {
                              closestVal = prod;
                              pClosest   = pPlayer;
                              closeHisPos = hisPos;
                           }
                        }

                        pSet = dynamic_cast<SimSet*>(manager->findObject(MoveableSetId));
                        AssertFatal(pSet != NULL, "No moveable set?");
                        for (itr = pSet->begin(); itr != pSet->end(); itr++) {
                           if (((*itr)->getType() & VehicleObjectType) == 0)
                              continue;

                           ShapeBase* pObject = dynamic_cast<ShapeBase*>(*itr);
                           
                           if (pObject->getVisibleToTeam(getTeam()) == false)
                              continue;

                           collisionQuery.box.fMax = pObject->getBoxCenter();
                           if (pRoot->findLOS(collisionQuery, &info, SimCollisionImageQuery::High) == true) {
                              if (info.object != (SimObject*)pObject)
                                 continue;
                           }

                           Point3F hisPos = pObject->getBoxCenter();
                           hisPos -= getLinearPosition();
                           hisPos.normalize();

                           float prod = m_dot(hisPos, lookDir);
                           if (prod > 0.0f && prod > closestVal) {
                              closestVal = prod;
                              closeHisPos = hisPos;
                              pClosest   = pObject;
                           }
                        }

                        // We need to find the current FOV, and take the percentage of
                        //  it specified in the .dat file for this turret.  Only if the
                        //  do product is greater than this, do we allow the target to
                        //  be set...
                        //
                        float myFov   = (fov / 2.0) * data->targetableFovRatio;
                        float compCos = cos(myFov);
                        if (compCos > 0.996f)   // hack for single precision math.  It's very
                           compCos = 0.996;     // hard to get more precise answers from the dot prod.

                        if (pClosest != NULL && closestVal > compCos)
                           bullet->setTarget(pClosest);
                     }
                  }

	   				if (data->maxGunEnergy)
	   					{
	   						float e;
	   						e = energy > data->maxGunEnergy ? data->maxGunEnergy : energy;

                        float pofm = e / float(data->maxGunEnergy);

	   						bullet->setEnergy (e, pofm);

	   						energy -= e;
	   						setEnergy (energy);
	   					}

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

	   				if (animThread)
	   					{
	   						setFireThread ();
	   						animThread->SetPosition (0.0);
	   					}
	   				
	   				fireCount++;
	   				setMaskBits (ShootingMask);
	   			}
	   	}
   } else {
      if (data && data->projectile.type == -1) {
         if (!isGhost())
            if (const char* script = scriptName("onFire"))
               Console->executef(2, script, scriptThis());
      }
      else {
         float energy = getEnergy();
         if (waitTime <= manager->getCurrentTime() && data && energy >= data->minGunEnergy && data->projectile.type != -1) {
            TMat3F muzzleTransform;
            getMuzzleTransform(0, &muzzleTransform);
            Projectile* bullet = createProjectile(data->projectile);

            if (!playerControlled && data->deflection) {
               static Random random;
               EulerF angles;
               muzzleTransform.angles (&angles);
               angles.x += (random.getFloat() - 0.5) * M_2PI * data->deflection;
               angles.z += (random.getFloat() - 0.5) * M_2PI * data->deflection;
               muzzleTransform.set (angles, muzzleTransform.p);
            } else if (playerControlled) {
               Point3F start = muzzleTransform.p;
               muzzleTransform = getEyeTransform ();
               aimedTransform (&muzzleTransform, start);
               muzzleTransform.p = start;
            }

            bullet->initProjectile (muzzleTransform, Point3F (0, 0, 0), getId());
            AssertFatal(bullet->isSustained() == true, "Error, must be sustained bullet");
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

            if (animThread) {
               setFireThread ();
               animThread->SetPosition (0.0);
            }
            
            fireCount++;
            setMaskBits (ShootingMask);

            m_fireState  = Firing;
            m_beganState = wg->currentTime;

            m_pProjectile = bullet;
            m_pTarget     = targetPlayer;
            
            if (m_pTarget)
               deleteNotify(m_pTarget);
         }
      }
   }
}

void
Turret::unshoot()
{
   AssertFatal(m_pProjectile != NULL, "Error, no projectile?");

   m_beganState = wg->currentTime;
   m_fireState  = Reloading;

   m_pProjectile->shutOffProjectile();
   m_pProjectile = NULL;
   
   if (m_pTarget != NULL)
      clearNotify(m_pTarget);
   m_pTarget     = NULL;
}

bool Turret::isTargetable (Player *player, float *minDist, float useRange)
{
	if (player && player->getTeam () != getTeam() && player->getVisibleToTeam (getTeam()) && !player->isDead())
		{
			if (const char* script = scriptName("verifyTarget")) {
            const char* pRet = Console->evaluatef("%s(%d);", script, player->getId());
            if (pRet[0] == 'F' || pRet[0] == 'f')
               return false;
         }

			Point3F playerPos = player->getLeadCenter();
			float dist = m_distf (getBoxCenter(), playerPos);

			if (dist < useRange)
				targetsTracked++;

			if (dist < *minDist)
				{
					TMat3F invMat;
					getNodeOffset (&invMat, "dummy muzzle", gunNode);
					invMat.inverse();
					m_mul (Point3F (playerPos.x, playerPos.y, playerPos.z), invMat, &playerPos);

					float ele = elevation (playerPos.x, playerPos.y, playerPos.z);
					float rotz = rotation (-playerPos.x, -playerPos.y);

					if (!inDeadZone (rotz, ele))
						{
					      SimContainerQuery collisionQuery;
					      collisionQuery.id = getId();
					      collisionQuery.type   = -1;
					      collisionQuery.mask   = SimTerrainObjectType | SimInteriorObjectType;
					      collisionQuery.detail = SimContainerQuery::DefaultDetail;
					      collisionQuery.box.fMin = getBoxCenter();
					      collisionQuery.box.fMax = player->getLeadCenter();

					      SimCollisionInfo collisionInfo;
							SimContainer* root = findObject(manager,SimRootContainerId,root);
					      root->findLOS(collisionQuery, &collisionInfo);
							if (!collisionInfo.object || collisionInfo.object == player)
								{
									*minDist = dist;
									return true;
								}
						}
				}
		}

	return false;
}


Player *Turret::chooseTarget (void)
{
	if (data->projectile.type == -1)
		return NULL;

	GameBase *closePlayer = NULL;

	float useRange = data->gunRange == -1 ? data->iRange : data->gunRange;
	float minDist = useRange;

	if (isTargetable (primaryTarget, &minDist, useRange))
		return primaryTarget;

	//PlayerManager::ClientRep *client = sg.playerManager->getClientList();
	PlayerManager::BaseRep *client = sg.playerManager->getBaseRepList();

	while (client)
		{
			if(client->ownedObject)
         {
			   if (Player *player = dynamic_cast<Player *>(client->ownedObject))
            {
				   if (isTargetable (player, &minDist, useRange))
				   	closePlayer = client->ownedObject;
            }
         }
			client = client->nextClient;
		}

	return closePlayer ? dynamic_cast<Player *>(closePlayer) : NULL;
}


void Turret::trackAndFire (Player *closePlayer, float interval)
{
   if (data->isSustained == false) {
	   Vector3F rot = getAngulerPosition();
	   int aimed = 0;
	   float old_rot = turretRotation;
	   float increment = data->speed * interval;
	   float des_z;
	   Point3F playerPos;

	   float dist = m_distf (getBoxCenter(), closePlayer->getLeadCenter());
	   leadPosition (closePlayer->getLeadCenter(), closePlayer->getLeadVelocity(), dist, &playerPos);

	   TMat3F invMat;
	   getNodeOffset (&invMat, "dummy muzzle", gunNode);
	   invMat.inverse();
	   m_mul (Point3F (playerPos.x, playerPos.y, playerPos.z), invMat, &playerPos);

	   des_z = rotation (-playerPos.x, -playerPos.y);
	   	
	   while (des_z < 0)
	   	des_z += (float)M_2PI;
	   	
	   while (des_z > M_2PI)
	   	des_z -= (float)M_2PI;
	   	
	   float diff = des_z - turretRotation;
	
	   if (diff > M_PI || diff < -M_PI)
	   	increment = -increment;
	   	
	   if (diff < increment && diff > -increment)
	   	{
	   		turretRotation = des_z;
	   		aimed += 1;
	   	}
	   else
	   	if (diff < 0)
	   		turretRotation -= increment;
	   	else
	   		turretRotation += increment;
	   		
	   wrapRotation ();
	   		
	   if (turretRotation != old_rot)
	   	setMaskBits (TRotationMask);
	
	   float old_elevation = turretElevation;
	   float des_y;
	
	   increment = data->speed * interval;
	   des_y = elevation (playerPos.x, playerPos.y, playerPos.z);
	   diff = des_y - turretElevation;
	
	   if (diff > M_PI || diff < -M_PI)
	   	increment = -increment;
	
	   if (diff < increment && diff > -increment)
	   	{
	   		turretElevation = des_y;
	   		aimed += 1;
	   	}
	   else
	   	if (diff < 0)
	   		turretElevation -= increment;
	   	else
	   		turretElevation += increment;
	   		
	   wrapElevation ();
	   		
	   if (old_elevation != turretElevation)
	   	setMaskBits (ElevationMask);
	
	   if (closePlayer && aimed >= 2 && waitTime <= manager->getCurrentTime())
	   	shoot (false, closePlayer);
   } else {
      Vector3F rot = getAngulerPosition();
      int aimed = 0;
      float old_rot = turretRotation;
      float increment = data->speed * interval;
      float des_z;
      Point3F playerPos;

      float dist = m_distf (getBoxCenter(), closePlayer->getLeadCenter());
      leadPosition (closePlayer->getLeadCenter(), closePlayer->getLeadVelocity(), dist, &playerPos);

      TMat3F invMat;
      getNodeOffset (&invMat, "dummy muzzle", gunNode);
      invMat.inverse();
      m_mul (Point3F (playerPos.x, playerPos.y, playerPos.z), invMat, &playerPos);

      des_z = rotation (-playerPos.x, -playerPos.y);
         
      while (des_z < 0)
         des_z += (float)M_2PI;
         
      while (des_z > M_2PI)
         des_z -= (float)M_2PI;
         
      float diff = des_z - turretRotation;
   
      if (diff > M_PI || diff < -M_PI)
         increment = -increment;
         
      if (diff < increment && diff > -increment)
         {
            turretRotation = des_z;
            aimed += 1;
         }
      else
         if (diff < 0)
            turretRotation -= increment;
         else
            turretRotation += increment;
            
      wrapRotation ();
            
      if (turretRotation != old_rot)
         setMaskBits (TRotationMask);
   
      float old_elevation = turretElevation;
      float des_y;
   
      increment = data->speed * interval;
      des_y = elevation (playerPos.x, playerPos.y, playerPos.z);
      diff = des_y - turretElevation;
   
      if (diff > M_PI || diff < -M_PI)
         increment = -increment;
   
      if (diff < increment && diff > -increment)
         {
            turretElevation = des_y;
            aimed += 1;
         }
      else
         if (diff < 0)
            turretElevation -= increment;
         else
            turretElevation += increment;
            
      wrapElevation ();
            
      if (old_elevation != turretElevation)
         setMaskBits (ElevationMask);
   
      if (closePlayer && aimed >= 2 && waitTime <= manager->getCurrentTime()) {
         shoot (false, closePlayer);
      }
   }
}


void Turret::extend (float interval)
{
	if (!isGhost() && state != EXTENDED && state != EXTENDING)
		setMaskBits (StateMask);

	if (!animThread || !setPowerThread())
		state = EXTENDED;
	else
		{
			if (state == RETRACTED)
				animThread->SetPosition (0.0);

			if (animThread->getPosition () == 0.0 && data && data->activationSound != -1)
				{
			      if(isGhost())
			         TSFX::PlayAt(data->activationSound, getTransform(), Point3F(0, 0, 0));

					if (whirSound)
						{
							Sfx::Manager::Stop (manager, whirSound);
							whirSound = 0;
						}
				}

			if (animThread->getPosition () >= 0.99)
				{
					setFireThread ();
					if (!isGhost())
						{
							state = EXTENDED;
							setMaskBits (StateMask);
						}
				}
			else
				{
					state = EXTENDING;
					animThread->AdvanceTime (interval);
				}
		}
}

void Turret::retract (float interval)
{
	if (!isGhost() && state != RETRACTED && state != RETRACTING)
		setMaskBits (StateMask);
		
	if (!animThread || !setPowerThread ())
		state = RETRACTED;
	else
		{
			if (state == EXTENDED)
				animThread->SetPosition (1.0);
			state = RETRACTING;

			if (turretRotation == 0)
				{
					if (!isGhost())
						setMaskBits (TRotationMask);

					if (animThread->getPosition () == 1.0 && data && data->deactivateSound != -1)
						{
					      if(isGhost())
					         TSFX::PlayAt(data->deactivateSound, getTransform(), Point3F(0, 0, 0));

							if (whirSound)
								{
									Sfx::Manager::Stop (manager, whirSound);
									whirSound = 0;
								}
						}

					animThread->AdvanceTime (-interval);
					if (animThread->getPosition () <= 0.0)
						{
//							setFireThread ();
							state = RETRACTED;
							if (!isGhost())
								setMaskBits (StateMask);
						}
				}
			else
				{
					if (isGhost())
						{
							if (whirSound)
								{
									Sfx::Manager *man = Sfx::Manager::find (manager);
									man->selectHandle(whirSound);

									if (man->isDone ())
										whirSound = TSFX::PlayAt(data->whirSound, getTransform(), Point3F(0, 0, 0));
//										man->play ();
								}
							else
								if (data && data->whirSound != -1)
									whirSound = TSFX::PlayAt(data->whirSound, getTransform(), Point3F(0, 0, 0));
						}
					else
						setMaskBits (TRotationMask);

					float tr = turretRotation;

					while (tr > M_PI)
						tr -= M_2PI;

					if (tr > 0)
						{
							turretRotation -= data->speed * interval;
							tr -= data->speed * interval;
							if (tr < 0)
								turretRotation = 0;
						}
					else
						{
							turretRotation += data->speed * interval;
							tr += data->speed * interval;
							if (tr > 0)
								turretRotation = 0;
						}
				}
		}
}


bool Turret::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   bool retVal = Parent::processQuery(query);

   if (query->count != 0 && query->image[0]->itype != SimRenderImage::Translucent) {
      query->image[0]->itype = SimRenderImage::Translucent;
      ((ShadowRenderImage*)query->image[0])->setSortValue(query->renderContext->getCamera()); // does nothing if not translucent
   }

   return retVal;
}

bool Turret::processQuery(SimQuery* query)
{
	switch (query->type)
		{
         onQuery(SimRenderQueryImage);

			default:
				return Parent::processQuery (query);
		}
}

bool Turret::getWeaponAmmoType(ProjectileDataType& out_rPrimary,
                          ProjectileDataType& out_rSecondary)
{
   if (data == NULL)
      return false;

   out_rPrimary = data->projectile;
   out_rSecondary.type = -1;

   return true;
}

//---------------------------------------------------------------------------

bool Turret::detectItem(GameBase *object)
{
	Point3F forward,objectVec = object->getBoxCenter() - getBoxCenter();
	objectVec.normalize();
	getEyeTransform().getRow(1,&forward);
	float dot = m_dot (forward, objectVec);
	if (dot >= data->FOV)
		return Parent::detectItem (object);
	return false;
}

//---------------------------------------------------------------------------

void Turret::serverProcess (DWORD time)
{
	Parent::serverProcess(time);

   if (data->isSustained == false) {
	   if (!getControlClient() && getState () == StaticBase::Enabled && isActive())
	   	{
	   		targetsTracked = 0;
	   		Player *closePlayer = chooseTarget ();

	   		if (targetsTracked)
	   			sleepTime = manager->getCurrentTime() + 3.0;

	   		if (closePlayer)
	   			{
	   				if (state == EXTENDED)
	   					trackAndFire (closePlayer, 0.032);
	   				else
	   					extend (0.032);
	   			}
	   		else
	   			if (!targetsTracked && manager->getCurrentTime() > sleepTime)
	   				{
	   					if (state != RETRACTED)
	   						{
	   							retract (0.032);
	   						}
	   				}
	   	}
	
	   if (!getControlClient())
	   	updateMove (NULL, 0.032);
	   else
	   	updateSkip++;
   } else {
      switch (m_fireState) {
        case Waiting: {
         serverProcessWaiting(time);
         }
         break;

        case Firing: {
         serverProcessFiring(time);
         }
         break;
         
        case Reloading: {
         serverProcessReloading(time);
         }
         break;
         
        default:
         AssertFatal(0, "invalid state");
      }

      if (m_fireState == Firing) {
         float e = getEnergy();
         e -= data->energyRate * 0.032;
         if(e < 0.0) {
            unshoot();
            e = 0.0;
         }
         setEnergy(e);
      }
   }
}

void Turret::clientProcess (DWORD curTime)
{
	Parent::clientProcess(curTime);

   if (getControlClient() == int(manager->getId()))
   {
	   while (lastProcessTime < curTime)
		   {
		      DWORD endTick = (lastProcessTime + 32) & ~0x1F;
				if(endTick > curTime)
		         endTick = curTime;

		      if (PlayerMove *pm = cg.psc->getClientMove(lastProcessTime))
					updateMove (pm, 0.001 * (endTick - lastProcessTime));

		      lastProcessTime = endTick;
			}
   }
	else
	{
		updateMove (NULL, 0.001 * (curTime - lastProcessTime));
      lastProcessTime = curTime;
	}
}

//---------------------------------------------------------------------------
bool Turret::cameraZoomAndCrosshairsEnabled()
{
   return true;
}

void Turret::getCameraTransform(float camDist, TMat3F *transform)
{
   if(!image.shape)
      return;
   image.shape->animate();
	int node = cameraNode;
   if(node != -1)
   {
      const TMat3F &nodeTrans = image.shape->getTransform(node);
      TMat3F mat1(EulerF(turretElevation, 0, turretRotation), nodeTrans.p);
      m_mul(mat1, getTransform(), transform);
	   validateEyePoint (transform, camDist * 1.3);
   }
   else
      *transform = getTransform();
}

bool Turret::getMuzzleTransform(int, TMat3F *transform)
{
   if(!image.shape)
      return false;
   image.shape->animate();
   int node = gunNode;
   if(node != -1)
   {
      const TMat3F &nodeTrans = image.shape->getTransform(node);
      TMat3F mat1(EulerF(turretElevation, 0, turretRotation), nodeTrans.p);
      m_mul(mat1, getTransform(), transform);
   }
   else
      *transform = getTransform();
   return true;
}

const TMat3F &Turret::getEyeTransform (void)
{
   static TMat3F ret;
	TMat3F temp;

	int node = cameraNode;
	if (node != -1) {
		const TMat3F& nmat = image.shape->getTransform(node);
		temp.set(EulerF(turretElevation, 0, turretRotation), nmat.p);
	}
	else
		temp.identity();
	m_mul(temp, getTransform(), &ret);
   return ret;
}


void Turret::getNodeOffset (TMat3F *trans, char *nodeName, int Node)
{
	TSShapeInstance *si = image.shape;

	if(si)
	   {
	      int node;

			if (Node == -1)
				node = si->getShape().findNode(nodeName);
			else
				node = Node;

			if (node != -1)
				{
					TMat3F nodeTrans;
					nodeTrans.identity();
					nodeTrans.p = si->getTransform (node).p;
					m_mul (nodeTrans, getTransform(), trans);
				}
	   }
}

void Turret::updateMove (PlayerMove *move, float interval)
{
   if (data->isSustained == false) {
	   if (getControlClient() && state != EXTENDED)
	   	{
	   		if (!isGhost())
	   			extend (interval);
	   	}

		if (state != EXTENDED)
			move = NULL;

	   float oTR = turretRotation;
	   float oTE = turretElevation;

	   if (move && (!animThread || animThread->getPriority() != -1 ||
	   		animThread->getPosition() >= 1.0))
	   	{
#if 0
	   		float maxSpeed = data->speed * data->speedModifier * interval;
	   		float moveFrac = interval * 32;
#else
	   		float maxSpeed = data->speed * data->speedModifier;
	   		float moveFrac = interval * 16;
#endif
	   		float pitch = m_clamp(move->pitch,-maxSpeed,maxSpeed);
	   		float turn = m_clamp(move->turnRot,-maxSpeed,maxSpeed);
	   		turretElevation += pitch * moveFrac;
	   		turretRotation += turn * moveFrac;

	   		wrapElevation ();
	   		if (maxElevation != minElevation)
	   			{
	   				if (turretElevation > maxElevation)
	   					turretElevation = maxElevation;
	   			
	   				if (turretElevation < minElevation)
	   					turretElevation = minElevation;
	   			}
	
	   		wrapRotation ();
	   		if (maxRotation != minRotation)
	   			{
	   				if (turretRotation > maxRotation)
	   					turretRotation = maxRotation;
	   			
	   				if (turretRotation < minRotation)
	   					turretRotation = minRotation;
	   			}
	   	
	   		if (move->trigger)
	   			{
	   				if (!isGhost ())
	   					shoot (true);
	   			}
	   	
	   		if (move->jumpAction && !isGhost())
            {
               const char *fn = scriptName("jump");
               if(fn)
                  Console->executef(2, fn, scriptThis());
            }
	   	}

	   if (elevationThread && maxElevation != minElevation)
	   	elevationThread->SetPosition ((turretElevation - minElevation) / (maxElevation - minElevation));
	   	
	   if (rotationThread)
	   	{
	   		if (!isEqual (maxRotation, minRotation))
	   			rotationThread->SetPosition ((turretRotation - minRotation) / (maxRotation - minRotation));
	   		else
	   			rotationThread->SetPosition (turretRotation / M_2PI);
	   	}

	   // this is for the firing anim...
	   if (animThread && state == EXTENDED)
	   	animThread->AdvanceTime (interval);

	   // this is for the power anim...
	   if (animThread && isGhost())
	   	{
	   		if (state == EXTENDING)
	   			extend (interval);
	   		else
	   			if (state == RETRACTING)
	   				retract (interval);
	   	}

	   if (!isGhost ())
	   	{
	   		// Need to animate on the server to get the
	   		// node transforms.
	   		image.shape->animate ();

	   		if (oTE != turretElevation)
	   			setMaskBits (ElevationMask);
	   		if (oTR != turretRotation)
	   			setMaskBits (TRotationMask);
	   	}

	} else {
	   if (getControlClient() && state != EXTENDED) {
	      if (!isGhost())
	         extend (interval);
	   }

		if (state != EXTENDED)
			move = NULL;

	   float oTR = turretRotation;
	   float oTE = turretElevation;

	   if (move && (!animThread || animThread->getPriority() != -1 ||
	   		animThread->getPosition() >= 1.0))
	   {
	   	float maxSpeed = data->speed * interval;
	   	float pitch = m_clamp(move->pitch,-maxSpeed,maxSpeed);
	   	float turn = m_clamp(move->turnRot,-maxSpeed,maxSpeed);
	   	float moveFrac = interval * 32;
	   	turretElevation += pitch * moveFrac;
	   	turretRotation += turn * moveFrac;

	   	wrapElevation ();
	   	if (maxElevation != minElevation) {
	   		if (turretElevation > maxElevation)
	   			turretElevation = maxElevation;
	   	
	   		if (turretElevation < minElevation)
	   			turretElevation = minElevation;
	   	}
	
	   	wrapRotation ();
	   	if (maxRotation != minRotation) {
	   		if (turretRotation > maxRotation)
	   			turretRotation = maxRotation;
	   	
	   		if (turretRotation < minRotation)
	   			turretRotation = minRotation;
	   	}
	
	   	if (move->trigger && m_fireState == Waiting) {
	   		if (!isGhost ())
	   			shoot(true, NULL);
	   	} else if (!move->trigger && m_fireState == Firing) {
	   		if (!isGhost())
	   			unshoot();
         }
	
	   	if (move->jumpAction && !isGhost()) {
            if (m_fireState == Firing)
               unshoot();

            const char *fn = scriptName("jump");
            if(fn)
               Console->executef(2, fn, scriptThis());
         }
	   }

	   if (elevationThread && maxElevation != minElevation)
	   	elevationThread->SetPosition ((turretElevation - minElevation) / (maxElevation - minElevation));
	   	
	   if (rotationThread) {
	   	if (!isEqual (maxRotation, minRotation))
	   		rotationThread->SetPosition ((turretRotation - minRotation) / (maxRotation - minRotation));
	   	else
	   		rotationThread->SetPosition (turretRotation / M_2PI);
	   }

	   // this is for the firing anim...
	   if (animThread && state == EXTENDED)
	   	animThread->AdvanceTime (interval);

	   // this is for the power anim...
	   if (animThread && isGhost()) {
	   	if (state == EXTENDING)
	   		extend (interval);
	   	else if (state == RETRACTING)
	   	   retract (interval);
	   }

	   if (!isGhost ()) {
	   	// Need to animate on the server to get the
	   	// node transforms.
	   	image.shape->animate ();

	   	if (oTE != turretElevation)
	   		setMaskBits (ElevationMask);
	   	if (oTR != turretRotation)
	   		setMaskBits (TRotationMask);
	   }
   }
}


void Turret::serverUpdateMove(PlayerMove *moves, int moveCount)
{
   while(moveCount--)
		{
		   updateMove (&lastPlayerMove, 0.032);

	      updateSkip--;
			if(updateSkip < 1)
				updateSkip = 1;
				
	      lastPlayerMove = *moves++;
		}


   if(updateSkip > 3)
      updateSkip = 1;
}


//----------------------------------------------------------------------------

void Turret::buildScopeAndCameraInfo(Net::GhostManager *cr, CameraInfo *camInfo)
{
   Parent::buildScopeAndCameraInfo(cr,camInfo);
}	


//----------------------------------------------------------------------------

void Turret::onDamageStateChange(DamageState oldState)
{
	Parent::onDamageStateChange(oldState);
	if (!isGhost())
		setMaskBits (StateMask);
}

//----------------------------------------------------------------------------

void Turret::readPacketData(BitStream *bstream)
{
	bstream->read(&turretRotation);
	bstream->read(&turretElevation);
	int fc;
	fc = bstream->readInt (2);
	if (fc != fireCount)
		{
         if (data->isSustained == false)
	         TSFX::PlayAt(data->fireSound, getTransform(), Point3F(0, 0, 0));

			if (animThread)
				{
					setFireThread ();
					animThread->SetPosition (0.0);
				}
		}

	fireCount = fc;

   if (bstream->readFlag()) {
      if (fireSound == 0)
         fireSound = TSFX::PlayAt(data->fireSound, getTransform(), Point3F(0, 0, 0));
   } else if (fireSound != 0) {
		Sfx::Manager::Stop (manager, fireSound);
      fireSound = 0;
   }

	int newState;
	newState = bstream->readInt (4);
	
	if (animThread)
		{
			if (newState == RETRACTING && state == EXTENDED)
				{
					setPowerThread ();
					animThread->SetPosition (1.0);
				}
			else
				if (newState == EXTENDING && state == RETRACTED)
					{
						setPowerThread ();
						animThread->SetPosition (0.0);
					}
		}

	state = newState;
	
	bstream->read (&energy);
}

void Turret::writePacketData(BitStream *bstream)
{
	bstream->write(turretRotation);
	bstream->write(turretElevation);
	bstream->writeInt (fireCount, 2);
   bstream->writeFlag(m_fireState == Firing);
	bstream->writeInt (state, 4);
	bstream->write (energy);
}


//--------------------------------------------------------------------------- 

void Turret::inspectRead(Inspect *i)
{
	Parent::inspectRead (i);
}


void Turret::inspectWrite(Inspect *i)
{
	Parent::inspectWrite (i);
}


//--------------------------------------------------------------------------- 

DWORD Turret::packUpdate(Net::GhostManager *g, DWORD mask, BitStream *stream)
{
   bool myClient = getControlClient() == int(g->getOwner()->getId());
	if (myClient && !(mask & (InitialUpdate | BaseStateMask)))
		{
			stream->writeFlag (true);
			return 0;
		}
	else
		stream->writeFlag (false);

	Parent::packUpdate (g, mask, stream);

	stream->writeFlag (mask & TRotationMask);
	if (mask & TRotationMask)
		stream->writeFloat (turretRotation / M_2PI, 8);

	stream->writeFlag (mask & ElevationMask);
	if (mask & ElevationMask)
		stream->writeFloat ((turretElevation - minElevation) / (maxElevation - minElevation), 8);

	if (stream->writeFlag (mask & ShootingMask))
		stream->writeInt (fireCount, 2);

//	stream->writeFlag ((mask & ExtendMask) && mask != 0xffffffff);
	if (stream->writeFlag (mask & StateMask))
		stream->writeInt (state, 4);

   stream->writeFlag(m_fireState == Firing);

   return 0;
}

void Turret::unpackUpdate(Net::GhostManager *g, BitStream *stream)
{
	if (stream->readFlag ())
		{
			return;
		}

	Parent::unpackUpdate (g, stream);

	float oTR = turretRotation;
	float oTE = turretElevation;

	if (stream->readFlag())
		turretRotation = stream->readFloat (8) * M_2PI;

	if (stream->readFlag())
		turretElevation = (stream->readFloat (8) * (maxElevation - minElevation)) + minElevation;

	if (oTR != turretRotation || oTE != turretElevation)
		{
			if (whirSound)
				{
					Sfx::Manager *man = Sfx::Manager::find (manager);
					man->selectHandle(whirSound);

					if (man->isDone ())
						whirSound = TSFX::PlayAt(data->whirSound, getTransform(), Point3F(0, 0, 0));
//						man->play ();
				}
			else
				if (data && data->whirSound != -1)
					whirSound = TSFX::PlayAt(data->whirSound, getTransform(), Point3F(0, 0, 0));
		}
	else
		if (whirSound && manager)
			{
				Sfx::Manager::Stop (manager, whirSound);
				whirSound = 0;
			}

	if (stream->readFlag())
		{
			int fc;
			fc = stream->readInt (2);
			if (manager && fc != fireCount)
            if (data && data->isSustained == false)
		         TSFX::PlayAt(data->fireSound, getTransform(), Point3F(0, 0, 0));

			fireCount = fc;

			if (animThread)
				{
					setFireThread ();
					animThread->SetPosition (0.0);
				}
		}

	if (stream->readFlag ())
		{
			int newState;
			newState = stream->readInt (4);

			if (animThread)
				{
					setPowerThread ();

					if (newState == RETRACTING && state == EXTENDED)
						animThread->SetPosition (1.0);
					else
						if (newState == EXTENDING && state == RETRACTED)
							animThread->SetPosition (0.0);
				}

			if (!(state == RETRACTING && newState == RETRACTED))
				state = newState;
		}

   if (stream->readFlag()) {
      if (data && fireSound == 0)
         fireSound = TSFX::PlayAt(data->fireSound, getTransform(), Point3F(0, 0, 0));
   } else if (fireSound != 0) {
		Sfx::Manager::Stop (manager, fireSound);
      fireSound = 0;
   }
}


//--------------------------------------------------------------------------- 

Persistent::Base::Error Turret::read(StreamIO &sio, int, int)
{
	Parent::read (sio, 0, 0);
	
	DWORD vers;
	sio.read(&vers);
	
	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


Persistent::Base::Error Turret::write(StreamIO &sio, int, int)
{
	Parent::write (sio, 0, 0);
	
	// version number
	sio.write(DWORD(0));
	
	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}


//-----------------------------------------------------------------------------------

int Turret::getDatGroup()
{
   return DataBlockManager::TurretDataType;
}

//-----------------------------------------------------------------------------------

//----------------------------------------------------------------------------

Turret::TurretData::TurretData() : SensorData()
{
	gunRange = -1;
	speed = 1.0;
	minGunEnergy = maxGunEnergy = 0;
	FOV = -1.0;

	fireSound = -1;
	activationSound = -1;
	deactivateSound = -1;
	whirSound = -1;

	deflection = 0.0;
   targetableFovRatio = 0.5;

   isSustained = false;
   reloadDelay = 0.0;
}

void Turret::TurretData::pack(BitStream *stream)
{
   Parent::pack(stream);

   stream->write (sizeof (projectile), &projectile);
   stream->write(gunRange);
   stream->write(speed);
   stream->write(speedModifier);
	stream->write (FOV);	// needed on client?

	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType, fireSound);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType, activationSound);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType, deactivateSound);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType, whirSound);

// these fields aren't used on the client anyway...
//	stream->write(reloadDelay);

   stream->writeFlag(isSustained);
}

void Turret::TurretData::unpack(BitStream *stream)
{
   Parent::unpack(stream);

   stream->read (sizeof (projectile), &projectile);
   stream->read(&gunRange);
   stream->read(&speed);
   stream->read(&speedModifier);
	stream->read (&FOV);

	fireSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	activationSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	deactivateSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	whirSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);

//	stream->read(&reloadDelay);
   isSustained = stream->readFlag();
}

void
Turret::onDeleteNotify(SimObject* io_pDelete)
{
   if (io_pDelete == (SimObject*)m_pTarget)
      m_pTarget = NULL;

   Parent::onDeleteNotify(io_pDelete);
}

void
Turret::serverProcessWaiting(DWORD in_currTime)
{
   Parent::serverProcess(in_currTime);

   if (!getControlClient() && getState () == StaticBase::Enabled && isActive()) {
      targetsTracked = 0;
      Player *closePlayer = chooseTarget ();

      if (targetsTracked)
         sleepTime = manager->getCurrentTime() + 3.0;

      if (closePlayer) {
         if (state == EXTENDED) 
            trackAndFire(closePlayer, 0.032);
         else
            extend (0.032);

      } else if (!targetsTracked && manager->getCurrentTime() > sleepTime) {
         if (state != RETRACTED)
            retract (0.032);
      }
   }
   
   if (!getControlClient())
      updateMove (NULL, 0.032);
   else
      updateSkip++;
}

void
Turret::serverProcessReloading(DWORD in_currTime)
{
   if (in_currTime >= m_beganState + DWORD(data->reloadDelay * 1000.0f)) {
      m_beganState = in_currTime;
      m_fireState  = Waiting;
   }
}

void
Turret::serverProcessFiring(DWORD in_currTime)
{
   if (!getControlClient() && getState () == StaticBase::Enabled && isActive()) {
      AssertFatal(m_pProjectile != NULL, "Must have projectile");

      if (m_pTarget == NULL) {
         // Lost our target, or player mount just ended...
         unshoot();
         return;
      }

      if (in_currTime >= m_beganState + data->firingTime) {
         // If the firing time runs out, we switch to reloading...
         unshoot();
         return;
      }
   
      float useRange = data->gunRange == -1 ? data->iRange : data->gunRange;
      float minDist  = useRange;
      if (isTargetable(m_pTarget, &minDist, useRange) == false) {
         unshoot();
         return;
      }

      // Guess we're still good, track the player...
      float interval = 0.032;
	   Vector3F rot = getAngulerPosition();
	   int aimed = 0;
	   float old_rot = turretRotation;
	   float increment = data->speed * interval;
	   float des_z;
	   Point3F playerPos;

	   float dist = m_distf (getBoxCenter(), m_pTarget->getLeadCenter());
	   leadPosition (m_pTarget->getLeadCenter(), m_pTarget->getLeadVelocity(), dist, &playerPos);

	   TMat3F invMat;
	   getNodeOffset (&invMat, "dummy muzzle", gunNode);
	   invMat.inverse();
	   m_mul (Point3F (playerPos.x, playerPos.y, playerPos.z), invMat, &playerPos);

	   des_z = rotation (-playerPos.x, -playerPos.y);
	   	
	   while (des_z < 0)
	   	des_z += (float)M_2PI;
	   	
	   while (des_z > M_2PI)
	   	des_z -= (float)M_2PI;
	   	
	   float diff = des_z - turretRotation;
	
	   if (diff > M_PI || diff < -M_PI)
	   	increment = -increment;
	   	
	   if (diff < increment && diff > -increment)
	   	{
	   		turretRotation = des_z;
	   		aimed += 1;
	   	}
	   else
	   	if (diff < 0)
	   		turretRotation -= increment;
	   	else
	   		turretRotation += increment;
	   		
	   wrapRotation ();
	   		
	   if (turretRotation != old_rot)
	   	setMaskBits (TRotationMask);
	
	   float old_elevation = turretElevation;
	   float des_y;
	
	   increment = data->speed * interval;
	   des_y = elevation (playerPos.x, playerPos.y, playerPos.z);
	   diff = des_y - turretElevation;
	
	   if (diff > M_PI || diff < -M_PI)
	   	increment = -increment;
	
	   if (diff < increment && diff > -increment)
	   	{
	   		turretElevation = des_y;
	   		aimed += 1;
	   	}
	   else
	   	if (diff < 0)
	   		turretElevation -= increment;
	   	else
	   		turretElevation += increment;
	   		
	   wrapElevation ();
	   		
	   if (old_elevation != turretElevation)
	   	setMaskBits (ElevationMask);
   }

   if (m_pProjectile)
      m_pProjectile->updateImageTransform(getEyeTransform());
}

void
Turret::onPowerStateChange(GameBase* generator)
{
   if (isGhost() == false && isPowered() == false && m_fireState == Firing) {
      unshoot();
   }

   Parent::onPowerStateChange(generator);
}
