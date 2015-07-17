#include <ts.h>
#include <base.h>
#include <sim.h>

#include "simTerrain.h"
#include "simInteriorShape.h"
#include <grdBlock.h>
#include "simResource.h"
#include "simPersman.h"
#include "simInterior.h"

#include <simNetObject.h>
#include <NetGhostManager.h>
#include "tsfx.h"

#include "Player.h"
#include "fearGuiHudCtrl.h"
#include "fearGuiInvList.h"
#include "simGuiTSCtrl.h"
#include "Fear.Strings.h"
#include "sfx.strings.h"
#include "PlayerManager.h"
#include "FearCam.h"
#include "SimExplosion.h"
#include "FearForceDcl.h"

#include "netEventManager.h"
#include "SoundFX.h"

#include "console.h"
#include "FearPlayerPSC.h"

#include "fearGlobals.h"
#include "vehicle.h"
#include "stringTable.h"
#include "dataBlockManager.h"
#include "sensorManager.h"
#include <esfObjectTypes.h>
#include <decalmanager.h>

#define JetFuelMin (5)
#define MaxPitch		DEGTORAD(88.0)
#define CameraRadius	0.3

#define GroundStatic 0.75
#define DEGTORAD(x)  ((x) * (M_PI / 180))

enum {
   TicksBetweenJumps = 32,
   JumpSkipContactsMax = 8,
};

const float SlideSlope = 0.707f; 	// 45 deg.
const float SlideFactor = 0.0f;
const float StickFactor = 0.8f;
const float SurfaceShift = 0.0001f;


static int ClientCollisionMask =
	   	SimInteriorObjectType | 
	   	SimTerrainObjectType | 
	   	SimPlayerObjectType | 
	   	StaticObjectType |
	   	VehicleObjectType |
	   	MoveableObjectType;

static int ClientCorpseCollisionMask = 
	   	SimInteriorObjectType | 
	   	SimTerrainObjectType | 
	   	StaticObjectType |
	   	VehicleObjectType |
	   	MoveableObjectType;

static int ServerIgnoreCollisionMask = 
			CorpseObjectType |
      	MineObjectType |
			ItemObjectType;

static int StepCollisionMask = 
	   	SimTerrainObjectType | 
	   	SimInteriorObjectType | 
	   	MoveableObjectType;

// PSC calls unroll when the server rejects an update and sends a new
// position/velocity.

void Player::readPacketData(BitStream *bstream)
{
   Point3F startPos;
   Point3F rot = getRot();
   Point3F vel;

   BYTE pingStatus = bstream->readInt(2);
   setSensorPinged(pingStatus);
   bstream->read(&rot.z);
   if(bstream->readFlag())
   {
      bstream->read(sizeof(Point3F), &startPos);
      bstream->read(sizeof(Point3F), &vel);
      setLinearVelocity(vel);
      TMat3F mat;
      mat.set(EulerF(rot.x,rot.y,rot.z), startPos);
      setTransform(mat);
   }   
   bstream->read(&viewPitch);
   bstream->read(&energy);
   contact = bstream->readFlag();
   setRot(rot);
   jetting = bstream->readFlag();
   traction = bstream->readFloat(7);
	bool newCrouch = bstream->readFlag ();
	if (newCrouch && !crouching)
		setAnimation (ANIM_CROUCH);
	else
		if (crouching && !newCrouch)
			setAnimation (ANIM_STAND);
	crouching = newCrouch;
   jumpSurfaceLastContact = bstream->readInt(4);
   interpDoneTime = 0;
}

void Player::writePacketData(BitStream *bstream)
{
   BYTE pingStatus = getSensorPinged();
   bstream->writeInt(pingStatus, 2);
   bstream->write(getRot().z);
   if(bstream->writeFlag(mount == NULL))
   {
      bstream->write(sizeof(Point3F), getLinearPosition());
      bstream->write(sizeof(Point3F), getLinearVelocity());
   }
   bstream->write(viewPitch);
   bstream->write(energy);
   bstream->writeFlag(contact);
   bstream->writeFlag(jetting);
   bstream->writeFloat(traction, 7);
	bstream->writeFlag (crouching);
   bstream->writeInt(jumpSurfaceLastContact > 15 ? 15 : jumpSurfaceLastContact, 4);
}

// the server gets multiple updates for a player movement.

void Player::serverUpdateMove(PlayerMove *moves, int moveCount)
{
   // this processes trigger / item stuff from the current move
   // and movement from the last player move - so:
   // the final move that the player makes is NOT processed until
   // the next move set from the client, BUT it is passed on to
   // all the ghosts.

   // this results in faster firing, but one extra lag step on the
   // movement... should help smooth out for lagged connects.

   // as soon as you die, no player control is allowed:
   if(dead)
      return;

   while(moveCount--)
   {
      if(updateDebt > 5)
         break;
      updateDebt++;

      if(moves->useItem != -1) {
			char buff[16];
			sprintf(buff,"%d",moves->useItem);
		   Console->executef(3, "remoteUseItem", scriptThis(), buff);
      }

      if(lastPlayerMove.trigger && !moves->trigger)
         setImageTriggerUp(0);
      else if(!lastPlayerMove.trigger && moves->trigger)
         setImageTriggerDown(0);

   	updateDamage(0.032);
      updateMove(&lastPlayerMove, true);
      updateAnimation (0.032);

      lastPlayerMove = *moves++;
   }

   if(!mount || mountPoint != 1)
      setMaskBits(OrientationMask);
   updateSkip = 0;

}

void Player::ghostSetMove(PlayerMove *move, Point3F &newPos, Point3F &newVel, bool cont, float newRot, float newPitch, int skipCount, bool noInterp)
{
   int i;
   lastPlayerMove = *move;

   Point3F rot = getRot();
   Point3F lastPos = getTransform().p;
   float lastRot = rot.z;
   float lastPitch = viewPitch;

   setLinearVelocity(newVel);
   setPos(newPos);
   contact = cont;
   viewPitch = newPitch;
   rot.z = newRot;
   setRot(rot);

   DWORD t = 0;
   DWORD endTime = cg.predictForwardTime + 32;
   interpDoneTime = 0;

   updateSkip = 0;
   while(t < endTime)
   {
      updateMove(&lastPlayerMove, false);
      t += 32;
   }
   updateSkip = skipCount;

   if(cg.interpolateTime && !noInterp)
   {
      interpDoneTime = (cg.currentTime + cg.interpolateTime + 0x1F) & ~0x1F;
      DWORD interpTime = interpDoneTime - lastProcessTime;

      deltaPitch = viewPitch - lastPitch;
      deltaPitch /= interpTime;
      viewPitch = lastPitch;

      deltaTurn = getRot().z - lastRot;
      if(deltaTurn < -M_PI)
         deltaTurn += M_2PI;
      if(deltaTurn > M_PI)
         deltaTurn -= M_2PI;

      deltaTurn /= interpTime;
   
		Point3F rot = getRot ();
      setRot(Point3F(rot.x,rot.y,lastRot));

      if(mount)
		{
			Point3F rot = getRot();
			TMat3F mat (EulerF(rot.x, rot.y, rot.z), Point3F (0, 0, 0));
			TMat3F tmat;
			TMat3F result;
			mount->getObjectMountTransform(mountPoint, &tmat);
			m_mul (mat, tmat, &result);
			setTransform (result);
		}
      else
      {
         deltaPos = (getTransform().p - lastPos) / interpTime;
         setPos(lastPos);
      }   
   }
   else
   {
      interpDoneTime = 0;
      lastProcessTime = cg.currentTime & ~0x1F;
      deltaPos.set(0,0,0);
      deltaTurn = 0;
      deltaPitch = 0;
   }
}

void Player::setPlayerControl(bool control)
{
   hasFocus = control;
   if(hasFocus)
      cg.player = this;
}

void Player::fallToGround (float interval)
{
	float adjust = interval / 0.032;
	Point3F pos = getPos();

   // look for the terrain normal
	SimContainer *container = getContainer ();
   SimContainer *root = findObject(manager, SimRootContainerId, root);
	
	while (container != root && !(container->getType () & SimTerrainObjectType))
		container = container->getContainer();

	Point3F terrainNormal (0, 0, 0);
	float terrainHeight = -1.0e30f;
	if (container->getType () & SimTerrainObjectType)
		{
			SimTerrain *terrain = (SimTerrain *)container;
		   CollisionSurface collisionSurface;
		   if (terrain->getSurfaceInfo(Point2F(pos.x, pos.y), &collisionSurface))
				{
		      	terrainHeight = collisionSurface.position.z;
					terrainNormal = collisionSurface.normal;
				}
		}

   // look for the zed normal
   SimCollisionInfo info;
   SimContainerQuery query;
   query.id = getId();
   query.type = -1;
   query.mask = SimInteriorObjectType;
   query.box.fMax.set(pos.x, pos.y, pos.z - 1.0);
   query.box.fMin.set(pos.x, pos.y, pos.z + 0.125);

	Point3F zedNormal (0, 0, 0);
   float zedHeight = terrainHeight;
   if (root->findLOS(query, &info))
		{
	      Point3F p;
	      m_mul(info.surfaces[0].position, info.surfaces.tWorld, &p);
	      m_mul(info.surfaces[0].normal, (RMat3F)info.surfaces.tWorld, &zedNormal);
			zedNormal.normalize();
	      zedHeight = p.z;
	   }

	Point3F normal;
	float height;
	if (zedHeight > terrainHeight)
		{
			height = pos.z - zedHeight;
			normal = zedNormal;
		}
	else
		{
			height = pos.z - terrainHeight;
			normal = terrainNormal;
		}

	if (height > -0.5 && height < 0.5 && (normal.x || normal.y || normal.z))
		{
			Point3F oldRot;
			Point3F rot = getRot ();
			Point3F newRot;
			RMat3F mat;

			mat.set (EulerF (0, 0, -rot.z));

			m_mul (normal, mat, &newRot);
			rot.x = m_atan (newRot.y, newRot.z) - (M_PI /  2);
			rot.y = -(m_atan (newRot.x, newRot.z) - (M_PI /  2));
			oldRot = getRot ();

			if (oldRot.x < rot.x)
				{
					oldRot.x += 0.05 * adjust;
					if (oldRot.x > rot.x)
						oldRot.x = rot.x;
				}
			else
				if (oldRot.x > rot.x)
					{
						oldRot.x -= 0.05 * adjust;
						if (oldRot.x < rot.x)
							oldRot.x = rot.x;
					}

			if (oldRot.y < rot.y)
				{
					oldRot.y += 0.05 * adjust;
					if (oldRot.y > rot.y)
						oldRot.y = rot.y;
				}
			else
				if (oldRot.y > rot.y)
					{
						oldRot.y -= 0.05 * adjust;
						if (oldRot.y < rot.y)
							oldRot.y = rot.y;
					}

			setRot (oldRot);
		}
}

void Player::stepDelta(float amt)
{
   // step the dude's deltas in a forward or backward manner:

   Point3F rot = getRot();

   rot.z += deltaTurn * amt;
   if(rot.z < 0)
      rot.z += M_2PI;
   if(rot.z > M_2PI)
      rot.z -= M_2PI;

   setRot(rot);
   viewPitch += deltaPitch * amt;

   if(mount)
   {
      // don't step the position, just the rotation
      TMat3F mat(EulerF(0,0,rot.z), Point3F(0,0,0));
      TMat3F tmat, result;
      mount->getObjectMountTransform(mountPoint, &tmat);
      m_mul(mat, tmat, &result);
      setTransform(result);
   }
   else
   {
      // step the position:
      TMat3F transform = getTransform();
      ((RMat3F *) &transform)->set(EulerF(rot.x,rot.y,rot.z));
      transform.p += deltaPos * amt;
      setTransform(transform);
   }
}

void Player::clientProcess(DWORD curTime)
{
   DecalManager *dmgr =
      dynamic_cast<DecalManager *>(manager->findObject(DecalManagerId));
   AssertFatal(dmgr != NULL, "Must have decal manager");

	if (dead)
		fallToGround (0.001 * (curTime - lastAnimateTime));

   if(interpDoneTime)
   {
      DWORD endInterpTime = min(curTime, interpDoneTime);

      if(lastProcessTime < endInterpTime)
      {
         stepDelta(endInterpTime - lastProcessTime);
         lastProcessTime = endInterpTime;
      }
      else
         interpDoneTime = 0;
   }
   while(lastProcessTime < curTime)
   {
      DWORD endTick = (lastProcessTime + 32) & ~0x1F;
      int step = lastProcessTime & 0x1F;

      if(endTick >= curTime)
         endTick = curTime;
      
      int endStep = ((endTick - 1) & 0x1F) + 1;

      if(step)
         stepDelta((endStep - step) / 32.0f);
      else
      {
         // step the dude forward
         PlayerMove *pm;
         if(hasFocus)
         {
            pm = cg.psc->getClientMove(lastProcessTime);
            if(pm)
               lastPlayerMove = *pm;
         }
         else
            pm = &lastPlayerMove;
         if(pm)
            updateMove(pm, false);
         else
         {
            deltaPos.set(0,0,0);
            deltaTurn = 0;
            deltaPitch = 0;
         }
         if(endStep != 32)
            stepDelta((endStep - 32) / 32.0f);
      }
      lastProcessTime = endTick;
   }
   float animTimeDelta = 0.001 * (curTime - lastAnimateTime);
   lastAnimateTime = curTime;

	if (!mount && !jetting && contact && (getLinearVelocity().x || getLinearVelocity().y))
		bounce += 0.174f * 30 * animTimeDelta;

   updateAnimation (animTimeDelta);
	for (int i = 0; i < MaxItemImages; i++)
		updateImageAnimation(i,animTimeDelta);

   if (surfaceType != -1 && !mount && lastContactCount < 8)
	{
	   int trigger[1];
		if (image.shape->findTriggerFrames (trigger, 1, true)) {
		   switch (trigger[0]) 
         {
				// Right foot fall
		      case 1:
					TSFX::PlayAt(data->rFootSounds[surfaceType], getTransform (), Point3F(0.0f, 0.0f, 0.0f));
               dmgr->addFootprint(getTransform(), DecalManager::DECALTYPE(getFootPrint(true)), 0.5f);
					break;
				// Left foot fall
		      case 2:
					TSFX::PlayAt(data->lFootSounds[surfaceType], getTransform (), Point3F(0.0f, 0.0f, 0.0f));
               dmgr->addFootprint(getTransform(), DecalManager::DECALTYPE(getFootPrint(false)), -0.5f);
					break;
		   } 
		}
	}
	// check for client-side jet thread and sound
   if(jetting && !mount) {
      if(!jetSound)
   		jetSound = TSFX::PlayAt(data->jetSound, getTransform(), Point3F(0, 0, 0));
      else {
			Sfx::Manager *man = Sfx::Manager::find (manager);
			if(man) {
   			man->selectHandle (jetSound);
   			man->setTransform (getTransform(), getLinearVelocity());
         }
      }
   }
   else {
      if(jetSound) {
			Sfx::Manager::Stop(manager, jetSound);
			jetSound = 0;
      }
   }
   
   lastAnimateTime = curTime;
}

int
Player::getFootPrint(bool isRight)
{
   return data->footPrints[isRight ? 1 : 0];
}

void Player::serverProcess(DWORD curTime)
{
	if (updateDebt < -25 || dead || (!aiControlled  && !getControlClient()) )
   {
      PlayerMove emptyMove;
   
   	updateDamage(0.032);
		updateMove (&emptyMove, true);
      updateAnimation (0.032);
      if(!mount || mountPoint != 1)
         setMaskBits(OrientationMask);
      
      lastPlayerMove = emptyMove;
      updateSkip = 0;
   }
   else
   {
      updateSkip++;
      updateDebt--;
   }
      
	updateTimers();
   curTime;

   if (jetting)
      m_lastJetTime = curTime;

	bool newFreeLook = prefFreeLook;
	if (!newFreeLook && lastFreeLook)
	{
		viewPitch = 0;
		setMaskBits (OrientationMask);
	}

	lastFreeLook = newFreeLook;

   if(aiControlled)
      setPDA(false);
   else
   {
      PlayerManager::ClientRep *cr = sg.playerManager->findClient(getControlClient());
      if(!cr)
         setPDA(true);
      else
         setPDA(cr->curGuiMode != 1);
   }
   // check if the player is in the world:

   Point3F p = getTransform().p;
   bool insideWorld = (p.x >= sg.missionCenterPos.x && p.x <= (sg.missionCenterPos.x + sg.missionCenterExt.x) &&
                       p.y >= sg.missionCenterPos.y && p.y <= (sg.missionCenterPos.y + sg.missionCenterExt.y));

   if(inWorld && !insideWorld)
      Console->evaluatef("Player::leaveMissionArea(%d);", getId());
   else if(!inWorld && insideWorld)
      Console->evaluatef("Player::enterMissionArea(%d);", getId());

   inWorld = insideWorld;

}

void Player::updateMove(PlayerMove *curMove, bool server)
{
	server;

   for (int i = 0; i < MaxItemImages; i++)
   	updateImageState(i,0.032);

   forwardAxisMovement = curMove->forwardAction * data->maxForwardSpeed - 
                         curMove->backwardAction * data->maxBackwardSpeed;
   sideAxisMovement = (curMove->rightAction - curMove->leftAction) * data->maxSideSpeed;
   float movelen = m_sqrt(forwardAxisMovement * forwardAxisMovement +
         sideAxisMovement * sideAxisMovement);
   float maxmovelen;

   if(forwardAxisMovement > 0)
      maxmovelen = max(data->maxForwardSpeed, data->maxSideSpeed);
   else
      maxmovelen = max(data->maxBackwardSpeed, data->maxSideSpeed);

   if(movelen > maxmovelen)
   {
      float fact = maxmovelen / movelen;
      forwardAxisMovement *= fact;
      sideAxisMovement *= fact;
   }
   if(crouching)
   {
      forwardAxisMovement *= .5;
      sideAxisMovement *= .5;
   }
   
   if(!mount && (curMove->jumpAction && !dead && getAnimation() != ANIM_LAND && jumpSurfaceLastContact < JumpSkipContactsMax && (!isGhost() || hasFocus)))
   {
      jumpSurfaceLastContact = JumpSkipContactsMax;
      if(getAnimation() != ANIM_JUMPRUN)
         setAnimation(ANIM_JUMPRUN);
      Point3F ffdv, frtv;
      getTransform().getRow(0, &frtv);
      getTransform().getRow(1, &ffdv);
      
      frtv *= (curMove->rightAction - curMove->leftAction);
      ffdv *= (curMove->forwardAction - curMove->backwardAction);

      frtv += ffdv;
      float len = frtv.len(), dot = 0;
      if(len >= 0.01)
      {
         frtv.normalize();
         dot = m_dot(frtv, jumpSurfaceNormal);
      }
      if(dot <= 0)
         applyImpulse(Point3F(0,0,jumpSurfaceNormal.z * data->jumpImpulse));
      else
      {
   	   frtv *= dot * data->jumpImpulse;
         frtv.z = jumpSurfaceNormal.z * data->jumpImpulse;
   	   applyImpulse(frtv);
      }
   }

   if(jetting)
   {
      if(energy <= data->minJetEnergy || !curMove->jetting)
         jetting = false;
   }
   else
      if(energy >= JetFuelMin && curMove->jetting && !mount)
         jetting = true;

   //if(getAnimation() == ANIM_LAND)
   //   jetting = false;

   if(jetting && !dead)
   {
      // build the force vector for the jet:
      energy -= data->jetEnergyDrain;

      Point3F frtv, ffdv, fudv;
      getTransform().getRow(0, &frtv);
      getTransform().getRow(1, &ffdv);
      getTransform().getRow(2, &fudv);

      float len = 0;
      if(jumpSurfaceLastContact > 8)
      {      
         frtv *= (curMove->rightAction - curMove->leftAction);
         ffdv *= (curMove->forwardAction - curMove->backwardAction);
         frtv += ffdv;
         len = frtv.len();
      }
      if(len > 0.01)
      {
         frtv *= 1 / len;

         float dot = m_dot(lVelocity, frtv);
         float pct;
         if(dot > data->maxJetForwardVelocity)
            pct = 0;
         else if(dot < 0)
            pct = 1;
         else
            pct = 1 - (dot / data->maxJetForwardVelocity);

         if(pct > data->maxJetSideForceFactor)
            pct = data->maxJetSideForceFactor;

         frtv *= pct * data->jetForce;
         fudv *= (1 - pct) * data->jetForce;
         frtv += fudv;
         addForce(TribesJetForce, frtv );
      }
      else
         addForce(TribesJetForce, Point3F(0,0,data->jetForce));
   }
   else
   {
      jetting = false;
      addForce(TribesJetForce, Point3F(0,0,0));
   }
   Point3F rot = getRot();

   rot.z += curMove->turnRot;
   deltaTurn = curMove->turnRot;

   if(rot.z < 0)
      rot.z += M_2PI;
   else if(rot.z > M_2PI)
      rot.z -= M_2PI;

   setRot(rot);

   float oldPitch = viewPitch;
   viewPitch += curMove->pitch;
   if(viewPitch > MaxPitch)
      viewPitch = MaxPitch;
   else if(viewPitch < -MaxPitch)
      viewPitch = -MaxPitch;

   deltaPitch = viewPitch - oldPitch;

   if(!dead)
      updateEnergy(0.032);

	if (!isGhost())
		crouching = data->canCrouch && curMove->crouching && !jetting && lastContactCount < 32;
   falling = getLinearVelocity().z <= -10.0;

   setTimeSlice(0.032);

   if(mount)
   {
      lVelocity = mount->getLinearVelocity();
      TMat3F mat(EulerF(0,0,getRot().z), Point3F(0,0,0));
      TMat3F tmat, result;
      mount->getObjectMountTransform(mountPoint, &tmat);
      m_mul(mat, tmat, &result);
      setTransform(result);
      deltaPos.set(0,0,0);
      contact = false;

		if (!isGhost() && curMove->jumpAction) {
         Point3F upv;
         getTransform().getRow(2, &upv);
			setMaskBits (OrientationMask);

			if (const char* script = mount->scriptName("passengerJump"))
				Console->evaluatef("%s(%s,%s,\"%g %g %g\");", script, mount->scriptThis(), scriptThis(), upv.x,upv.y,upv.z);
      }
   }
   else
   {
      if(isGhost())
      {
         if(hasFocus)
            updateSkip = 0;
         else
            updateSkip++;
      }
      TMat3F tmat;
      // Desired velocity

      Point3F relVel(sideAxisMovement, forwardAxisMovement, 0);
      Point3F vel;
      m_mul(relVel, getRotation(), &vel);
   		
      // Linear velocity & position
      Point3F sVel = lVelocity;
      updateLinearVelocity();

      if (didContact()) {
   	   // Apply acceleration to our desired velocity
   	   Point3F lVel = lVelocity;
   	   Point3F dVel = vel - lVel;
   	   dVel.z = 0;

   	   float len = dVel.len();
         float adjTraction = traction * data->groundTraction;
         if(adjTraction > 1.0)
            adjTraction = 1.0;
   	   float maxAcc = data->groundForce / getMass() * adjTraction * timeSlice;
   	   if (len > maxAcc) {
   	      dVel *= (maxAcc / len);
   		   flags &= ~UseFriction;
   	   }
   	   else
   		   flags |= UseFriction;
   	   lVel += dVel;
   	   //
   	   setLinearVelocity(lVel);
      }
      else {
   	   if (sVel.len() < 0.1)
   		   flags |= UseFriction;
   	   else
   		   flags &= ~UseFriction;
      }
      clearContacts();
      updatePosition(sVel,&tmat);
      const Point3F &rotation = getRot();

      // rotation into the tmat:
      ((RMat3F *) &tmat)->set(EulerF(rotation.x, rotation.y, rotation.z));
      
		// Test for collision
      float strac = traction * 0.6;
      if(strac < .1)
         strac = 0;
		traction = 0;
      Point3F oldPos = getTransform().p;

      if(!Parent::stepPosition(tmat, 0.075f, 0.3f))
         setRotation((RMat3F &) tmat, true);
      if(traction < strac)
         traction = strac;
   	if (traction > 1.0f)
   		traction = 1.0f;

      deltaPos = getTransform().p - oldPos;
   }
   calcImagePullIn();
   // keep track of number of ticks that we've been airborn
   if(didContact())
      lastContactCount = 0;
   else
      lastContactCount++;
   jumpSurfaceLastContact++;

   if((damageFlash > 0 && !dead) || (damageFlash > .4 && dead))
      damageFlash -= 0.007f;
   if(damageFlash < 0)
      damageFlash = 0;

	setBoundingBox();
}

//-------------------------------------------------------------------------------------

//    This method is used by the AI jetting code to find out what its choices are for 
// applying a jet force to alter its velocity.  A simplification is used where they only 
// jet in straight lines - in the xy direction of their velocity (lVelocity).  This means 
// there are at most four possible forces that result.  
//    This code is wedded to the above code that builds the jet force vector, so if that
// changes, this should too.  
//    The (up to) four return values are total forces that will result for the four possible
// input configurations.  Gravity and other forces are included.  The return value gives
// how many choices there are.
//    0  ->    no jetting
//    1  ->    jetting straight up
//    2  ->    jetting in the specified lateral direction
//    3  ->    jetting opposite to the specified direction
// The specified direction (if not null) will generally be in the same direction
//    as the linear velocity.  
int Player::possibleJetForces( Point3F forces[4], const Point3F * unitLateralDir )
{
   int   numForceChoices = 0;

   forces[ numForceChoices++ ].set(0,0,0);
   forces[ numForceChoices++ ].set(0,0,data->jetForce);
   
   if( jumpSurfaceLastContact > 8 && unitLateralDir != NULL )
   {      
      Point3F  latVel = * unitLateralDir;
      float    dot = m_dot( lVelocity, latVel ), pct;
         
      while( numForceChoices < 4 )                    // compute the other two choices
      {
         if(dot > data->maxJetForwardVelocity)
            pct = 0;
         else if(dot <= 0)
            pct = 1;
         else
            pct = 1 - (dot / data->maxJetForwardVelocity);

         if(pct > data->maxJetSideForceFactor)
            pct = data->maxJetSideForceFactor;

         latVel *= pct * data->jetForce;
         latVel.z += (1 - pct) * data->jetForce;      // assuming fudv above is {0,0,1}
         
         forces[ numForceChoices++ ] = latVel;
         
         latVel = -(* unitLateralDir);                // second iteration, use opposite
         dot = -dot;
      }
   }

   // account for other forces in system, and the time slice.  
   Point3F  otherForce = forceSumExcept( TribesJetForce );
   for( int i = 0; i < numForceChoices; i++ )         
      forces[i] = (forces[i] + otherForce) * timeSlice;
      
   return numForceChoices;
}

//----------------------------------------------------------------------------

void Player::onDisplacement(SimMovement* displacer,const Point3F& delta)
{
	// Beeing displaced by an elevator, or something.
	displacer,delta;
	if (isGhost()) {
		// Saved transform is only valid on the clients.
	}
   else
      setMaskBits(NoInterpMask | OrientationMask);
}


//----------------------------------------------------------------------------

bool Player::processCollision(SimMovementInfo* info)
{
	// We want to collision notification, but we don't
	// want to physically collide with em.
	for (int i = 0; i < info->collisionList.size(); i++) {
		SimCollisionInfo& sinfo = info->collisionList[i];
		if (sinfo.object == mount) {
			info->collisionList.erase (i);
			i--;
		}
		if (sinfo.object->getType().test(ServerIgnoreCollisionMask)) {
			info->collidedList.push_back(sinfo.object);
			info->collisionList.erase (i);
			i--;
		}
	}

	// Let's get this all into a form we can use better.
	Vector<SurfaceInfo> surfaceList;
	surfaceList.reserve(10);
	Vector3F vertical;
	if (getForce(SimMovementGravityForce,&vertical)) {
		vertical.normalize();
		vertical.neg();
	}
	else
		vertical.set(0,0,1);

	for (int j = 0; j < info->collisionList.size(); j++) {
		CollisionSurfaceList& sList = info->collisionList[j].surfaces;
		// List any objects as collided (might not of actually hit
		// it at this point).
		info->collidedList.push_back(info->collisionList[j].object);
		for (int i = 0; i < sList.size(); i++) {
			surfaceList.increment();
			SurfaceInfo& ss = surfaceList.last();
			ss.surface = &sList[i];
			ss.object = info->collisionList[j].object;
			ss.stepHeight = info->collisionList[j].surfaces.stepVector.z;
			m_mul(ss.surface->normal,static_cast<RMat3F&>(sList.tWorld),&ss.normal);
			ss.verticalDot = m_dot(ss.normal,vertical);
		}
	}

	// A few things we do on the first try only
	if (info->retryCount == 0)
		pickSurfaceType(surfaceList);

	// Step over surfaces
	if (!mount && surfaceList.size() > 1 && stepSurface(info,surfaceList))
		return true;

	// Do the velocity stuff on all the surfaces
	bool hit = false;
	for (int k = 0; k < surfaceList.size(); k++)
		if (collideSurface(k,surfaceList))
			hit = true;

	// Set next position to try, ignore aVelocity
	if (hit) {
		info->target->p = lVelocity;
		info->target->p *= timeSlice - info->startTime;
		info->target->p += info->transform->p;
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------

bool Player::stepSurface(SimMovementInfo* info,SurfaceInfoList& surfaceList)
{
	// There must be at least two planes for the current
	// stepping algorithm to work.
	int stepIndex = -1;
	float stepDistance = 0;
	for (int j = 0; j < surfaceList.size(); j++) {
		// Look first for a plane that may be blocking our
		// line of travel.
		if (surfaceList[j].object->getType().test(StepCollisionMask) &&
				surfaceList[j].verticalDot <= 0.707 /*45 deg*/) {
			// Now we look for a perpendicular plane to step over
			for (int k = 0; k < surfaceList.size(); k++) {
				if (surfaceList[k].object->getType().test(StepCollisionMask) &&
						surfaceList[k].verticalDot >= 0.707 /*45 deg*/) {
					float dist = surfaceList[k].stepHeight;
					if (dist < 0) {
						// Not everyone provides a stepHeight
						CollisionSurface& surface = *surfaceList[k].surface;
						dist = collisionImage.sphere.radius - surface.distance;
					}
					if (dist > stepDistance)
						stepDistance = dist;
					if (stepIndex < 0 || surfaceList[k].verticalDot >= 
								surfaceList[stepIndex].verticalDot)
						stepIndex = k;
				}
			}
			break;
		}
	}
	if (stepIndex < 0)
		return false;

	// Found one, make sure it's not too high
	float maxStep = collisionImage.sphere.radius * 0.6f;
	if (stepDistance > maxStep)
		return false;

	// If we've picked a plane to step over, let's go ahead and try it.
	MovementInfo sinfo;
	sinfo.image = info->image;
	sinfo.imageList = info->imageList;
	TMat3F step = *info->collision;
	step.p.z += stepDistance + .001f;

	if (testPosition(step,&sinfo)) {
		// Go ahead and do velocity stuff for the
		// face that we step over.
		collideSurface(stepIndex,surfaceList);

		// New transform and target
		*info->transform = step;
		*info->target = step;
		Point3F vec = getLinearVelocity();
		vec *= timeSlice - info->collisionTime;
		info->target->p += vec;
		return true;
	}

	return false;
}


//----------------------------------------------------------------------------

void Player::pickSurfaceType(SurfaceInfoList& surfaceList)
{
	// Pick the flatest one for our surface type.
	TS::Material* material = 0;
	float flatest = 0.0;
	for (int i = 0; i < surfaceList.size(); i++) {
		SurfaceInfo& si = surfaceList[i];
		if (si.verticalDot > flatest && si.surface->material) {
			material = reinterpret_cast<TS::Material*>(si.surface->material);
			flatest = si.verticalDot;
		}
	}

	if (material)
		surfaceType = material->fParams.fType;
}	


//----------------------------------------------------------------------------

bool Player::collideSurface(int index,SurfaceInfoList& surfaceList)
{
	contact = true;
	SurfaceInfo surface = surfaceList[index];
	if (surface.surface->distance < 0.0f)
		return false;
	float dot = m_dot(lVelocity,surface.normal);
	if (dot < 0.0f) {
      // Take damage from the surface if it's on the server
      
      // see if we can jump off this surface:
      // jump surface dot is always z component of surface normal

      if(surface.normal.z > data->jumpSurfaceMinDot && 
         (surface.normal.z > jumpSurfaceNormal.z || jumpSurfaceLastContact > 1))
      {
         jumpSurfaceNormal = surface.normal;
         jumpSurfaceLastContact = 0;
      }
      
      if(!isGhost()) {
         float impactSpeed = -dot;
         if(impactSpeed > data->minDamageSpeed) {
            impactSpeed -= data->minDamageSpeed;
            float damage = data->damageScale * impactSpeed;
				Point3F vec(0,0,0);
				applyDamage(0,damage,vec,vec,vec,getOwnerClient());
         }
      }

      if (dot < -data->minDamageSpeed && !dead)
         if(getAnimation() != ANIM_LAND)
            setAnimation(ANIM_LAND);

		// Subtract out velocity into surface velocity
		Point3F dVel;
		float f = -dot + SurfaceShift;
		dVel.x = surface.normal.x * f;
		dVel.y = surface.normal.y * f;
		dVel.z = surface.normal.z * f;
		lVelocity += dVel;

		// Make sure the change doesn't push us into previous face.
#if 0
		bool cc;
		do {
		cc = false;
		for (int i = 0; i <= index; i++) {
			SurfaceInfo& si = surfaceList[i];
			if (si.surface->distance >= 0.0f) {
				float dot = m_dot(lVelocity,si.normal);
				if (dot < 0.0f) {
//					dot += SurfaceShift;
					lVelocity.x -= si.normal.x * dot;
					lVelocity.y -= si.normal.y * dot;
					lVelocity.z -= si.normal.z * dot;
					cc = true;
				}
			}
		}
		} while (cc);
#endif

		// Traction
		if (surface.verticalDot > 0.0f) {
			float t = surface.verticalDot;
			if (surface.surface->material)
				t *= ((TSMaterial*)surface.surface->material)->fParams.fFriction;
			traction += t;
			if (flags & UseFriction) {
				if (lVelocity.len() < t * GroundStatic)
					lVelocity.set(0,0,0);
			}

			if (surface.verticalDot > 0.985f) {
				if (surface.object->getType() & MoveableObjectType)
					addContact(dynamic_cast<SimMovement*>(surface.object));
			}
		}
		return true;
	}
	return false;
}

DWORD Player::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
   bool myClient = (int(gm->getOwner()->getId()) == getControlClient());
   DWORD retMask = 0;
   Parent::packUpdate(gm, mask, stream);

   if(stream->writeFlag(mask & BaseStateMask)){
      packDatFile(stream);
      stream->writeFlag(aiControlled);
   }

   if(mount && (mask & MountMask)) {
      int gi = mount->getGhostedIndex(gm);
      if(stream->writeFlag(gi != INVALID_GHOST_INDEX))
      {
         stream->writeFlag(true);
         stream->writeInt(gi, 10);
         stream->writeInt(mountPoint, 5);
      }
      else
         retMask |= MountMask;
   } else if(!mount && (mask & MountMask)) {
      stream->writeFlag(true);
      stream->writeFlag(false);
   } else {
      stream->writeFlag(false);
   }
   if(stream->writeFlag( mask & DamageMask ))
   {
      stream->writeFlag(dead);
      if(!dead)
         stream->writeInt ((damageLevel/data->maxDamage) * 63, 6);
      else
      {
         if(!stream->writeFlag(blownUp))
            stream->writeInt(currentAnimation, 6);
      }
   }
   bool writePos = mask == 0xffffffff ||
      ( (mask & OrientationMask) && !myClient );
   if(stream->writeFlag( writePos ))
   {
      stream->writeSignedFloat(viewPitch / MaxPitch, 5);
      stream->write(sizeof(Point3F), getLinearPosition());
      
      float len = lVelocity.len();
      if(stream->writeFlag(len > .02))
      {
         Point3F v = lVelocity;
         v *= 1/len;
         float outLen = len * 512.0;
         if(outLen > 131071)
            outLen = 131071;
         stream->writeInt(outLen, 17);
         stream->writeNormalVector(&v, 10);
      }            
      stream->writeFlag(didContact());
      stream->writeInt(getRot().z * 511 / M_2PI, 9);

      // send the last move
      if(mask != 0xffffffff)
      {
         lastPlayerMove.write(stream, false);
         stream->writeInt(energy * 127 / data->maxEnergy, 7);
         stream->writeInt(min(updateSkip, 15), 4);
         stream->writeFlag(mask & NoInterpMask);
      }
   }      
   if(mask == 0xffffffff)
      stream->writeFlag(false);
   else
   {
      if(stream->writeFlag((mask & AnimationMask) && serverAnimation))
         stream->writeInt(serverAnimation, 6);
   }

	if (stream->writeFlag ((myClient || mask == 0xffffffff) && (mask & InfoMask)))
		stream->write (rechargeRate);

	stream->writeFlag (pdaing);
	stream->writeFlag (crouching);

	packItemImages(stream,mask);
   return retMask;
}

void Player::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
	Point3F pos;
	Point3F rot;

   Parent::unpackUpdate(gm, stream);

   // first goes initial update stuff
   if(stream->readFlag())
   {
      int oldDatFileId = datFileId;
      unpackDatFile(stream);
      if(datFileId != oldDatFileId && manager)
      {
         loadDatFile();
         reSkin(true);
      }
      aiControlled = stream->readFlag();
   }
   if(stream->readFlag())
   {
      // mount mask changed
      if(stream->readFlag())
      {
         int ghostIndex = stream->readInt(10);
         mountPoint = stream->readInt(5);
			if (mountPoint == 1)
				setRot (Point3F (0, 0, 0));
         GameBase *newMount = (GameBase *) gm->resolveGhost(ghostIndex);
         AssertFatal(newMount != NULL, "ACK!");
			
         if(mount && mount != newMount)
            clearNotify(mount);
         mount = newMount;
         deleteNotify(newMount);
         lastPlayerMove.reset();
      }
      else
         mount = NULL;
   }
   if(stream->readFlag())
   {
      if(stream->readFlag())
      {
         blownUp = stream->readFlag();
         int anim = -1;
         if(!blownUp)
            anim = stream->readInt(6);
         if(!dead)
         {
            dead = true;
            damageLevel = 1.0;
            if(manager)
            {
               if(blownUp)
                  blowUp();
               else
                  setAnimation(anim);
            }
            else
               currentAnimation = anim;
            type = CorpseObjectType;
            collisionMask = ClientCorpseCollisionMask;
         }
      }
      else
      {
			damageLevel = stream->readInt(6) / 63.0f;
         dead = false;
         type = SimPlayerObjectType;
         collisionMask = ClientCollisionMask;
      }
      reSkin();
   }

   if(stream->readFlag())
   {
      float pitch = stream->readSignedFloat(5) * MaxPitch;
      stream->read (sizeof( pos ), &pos);

      Point3F vel;
      if(stream->readFlag())
      {
         float len = stream->readInt(17) / (512.0f);
         stream->readNormalVector(&vel, 10);
         vel *= len;
      }
      else
         vel.set(0,0,0);

      bool cont = stream->readFlag();

		int rz = stream->readInt (9);
		float rot = rz * M_2PI / 511.0f;

      // if this isn't the initial update, it'll have a move too.
      if(manager)
      {
         PlayerMove move;
         
         move.read(stream, false);
         energy = stream->readInt(7) * data->maxEnergy / 127.0f;
         updateSkip = stream->readInt(4);
         bool noInterp = stream->readFlag();
         ghostSetMove(&move, pos, vel, cont, rot, pitch, updateSkip, noInterp);
         if(cont)
            lastContactCount = 0;
         else
            lastContactCount = 8;
      }
      else
      {
         // if this  is a new ghost, don't interpolate it.
         lastProcessTime = cg.currentTime & ~0x1F;
         interpDoneTime = 0;
         setPos(pos);
         setLinearVelocity(vel);
         Point3F oldRot = getRot();
         oldRot.z = rot;
         setRot(oldRot);
         viewPitch = pitch;
         contact = cont;
         deltaPos.set(0,0,0);
         deltaPitch = 0;
         deltaTurn = 0;
      }
   }

   if(stream->readFlag())
      setAnimation(stream->readInt(6));

	if (stream->readFlag ())
		stream->read (&rechargeRate);

	pdaing = stream->readFlag ();
	bool newCrouch = stream->readFlag ();
	if (newCrouch && !crouching)
		setAnimation (ANIM_CROUCH);
	else
		if (crouching && !newCrouch)
			setAnimation (ANIM_STAND);
	crouching = newCrouch;

	unpackItemImages(stream);
}

void Player::PlayerData::pack(BitStream *stream)
{
   Parent::pack(stream);
   
   stream->writeString(flameShapeName);

   int i;
   for(i = 0; i < NUM_ANIMS; i++)
   {
      stream->writeString(animData[i].name);
      if(stream->writeFlag(animData[i].soundTag != -1))
         sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType, animData[i].soundTag);
      stream->writeFlag(animData[i].direction > 0);
      stream->writeInt(animData[i].viewFlags,4);
		stream->writeInt (animData[i].priority, 4);
   }

   stream->write(minDamageSpeed);
   stream->write(jetEnergyDrain);
   stream->writeFloat(maxForwardSpeed / 64.0, 10);
   stream->writeFloat(maxBackwardSpeed / 64.0, 10);
   stream->writeFloat(maxSideSpeed / 64.0, 10);
   stream->writeFlag(canCrouch);

   stream->writeFloat(maxJetSideForceFactor, 8);
   stream->write(groundForce);
   stream->write(groundTraction);
   stream->write(maxJetForwardVelocity);
   stream->writeFloat(jumpSurfaceMinDot, 8);
   stream->write(jumpImpulse);
   stream->write(mass);
   stream->write(drag);
   stream->write(density);
   stream->write(jetForce);
   sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType, jetSound);
   for(i = 0; i < NUM_FOOTSOUNDS; i++)
   {
      sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType, rFootSounds[i]);
      sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType, lFootSounds[i]);
   }
   stream->write(sizeof(int) * 2, footPrints);

	stream->write (boxWidth);
	stream->write (boxDepth);
	stream->write (boxNormalHeight);
	stream->write (boxCrouchHeight);
}

void Player::PlayerData::unpack(BitStream *stream)
{
   Parent::unpack(stream);
   char buf[256];
   stream->readString(buf);
   flameShapeName = stringTable.insert(buf);
   int i;
   for(i = 0; i < NUM_ANIMS; i++)
   {
      stream->readString(buf);
      animData[i].name = stringTable.insert(buf);
      if(stream->readFlag())
         animData[i].soundTag = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
      else
         animData[i].soundTag = -1;
      if(stream->readFlag())
         animData[i].direction = 1;
      else
         animData[i].direction = -1;
		animData[i].viewFlags = (Player::AnimData::ViewFlags)(stream->readInt(4));
		animData[i].priority = stream->readInt (4);
   }   
   stream->read(&minDamageSpeed);
   stream->read(&jetEnergyDrain);
   maxForwardSpeed = stream->readFloat(10) * 64;
   maxBackwardSpeed = stream->readFloat(10) * 64;
   maxSideSpeed = stream->readFloat(10) * 64;
   canCrouch = stream->readFlag();

   maxJetSideForceFactor = stream->readFloat(8);
   stream->read(&groundForce);
   stream->read(&groundTraction);
   stream->read(&maxJetForwardVelocity);
   jumpSurfaceMinDot = stream->readFloat(8);
   stream->read(&jumpImpulse);
   stream->read(&mass);
   stream->read(&drag);
   stream->read(&density);
   stream->read(&jetForce);
   jetSound = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
   for(i = 0; i < NUM_FOOTSOUNDS; i++)
   {
      rFootSounds[i] = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
      lFootSounds[i] = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
   }
   stream->read (sizeof(int) * 2, footPrints);
	stream->read (&boxWidth);
	stream->read (&boxDepth);
	stream->read (&boxNormalHeight);
	stream->read (&boxCrouchHeight);
}

