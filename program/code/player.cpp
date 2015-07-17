//--------------------------------------------------------------------------- 

#ifdef _BORLANDC_
#pragma warn -inl
#pragma warn -aus
#endif

#ifdef _MSVC_VER
#pragma warning(disable : 4710) // do not warn for non-expanded inline fns
#pragma warning(disable : 4101) // do not warn for unused identifiers
#endif

#include <ts.h>
#include <base.h>
#include <sim.h>
#include <g_surfac.h>
#include <d_caps.h>

#include "interiorShape.h"
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
//#include "fearPlayer.h"
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
#include "observerCamera.h"
#include "simsetiterator.h"
#include <esfObjectTypes.h>

//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG( Player, PlayerPersTag );

//----------------------------------------------------------------------------
#define DEGTORAD(x)  ((x) * (M_PI / 180))
#define MIN_DETECT_SIZE 0.0287f

#define JetFuelMin (5)

#define CorpseTimeoutMsg   100
#define CorpseTimeoutValue 15

#define CorpseTriggerTimeout 4
#define CorpseTriggerMsg     200
#define MaxPitch		DEGTORAD(88.0)


//--------------------------------------------------------------------------- 


float Player::prefShapeDetail = 1.0f;
float Player::prefPlayerShadowSize = 50;
bool Player::prefFreeLook = true;
extern bool gAnimateTransitions;
const DWORD Player::csm_respawnEffectTime = 1500;

//----------------------------------------------------------------------------

static int ClientCollisionMask =
	   	SimInteriorObjectType | 
	   	SimTerrainObjectType | 
	   	SimPlayerObjectType | 
	   	StaticObjectType |
	   	VehicleObjectType |
	   	MoveableObjectType;

static int ServerCollisionMask = 
			ClientCollisionMask |
         TriggerObjectType |
			CorpseObjectType |
      	MineObjectType |
			ItemObjectType;

static int ClientCorpseCollisionMask = 
	   	SimInteriorObjectType | 
	   	SimTerrainObjectType | 
	   	StaticObjectType |
	   	VehicleObjectType |
	   	MoveableObjectType;

static int ServerCorpseCollisionMask = 
			ClientCorpseCollisionMask |
	   	SimPlayerObjectType;

static int ScriptCollisionMask = 
			SimPlayerObjectType | 
			VehicleObjectType |
      	StaticObjectType |
			CorpseObjectType |
			MineObjectType |
			ItemObjectType;


//----------------------------------------------------------------------------

const char* PlayerIdVariable = "PlayerId";
TMat3F PlayerViewOverride;


//----------------------------------------------------------------------------

inline float m_clamp(float value,float min,float max)
{
	return (value <= min)? min: (value >= max)? max: value;
}	


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

Player::PlayerData::PlayerData()
{
   groundForce = 20;
   groundTraction = 1.0;
   maxJetForwardVelocity = 20;
   maxJetSideForceFactor = 1;
   minJetEnergy = 1;
   jetEnergyDrain = 1;
   jumpSurfaceMinDot = 0.2;
   jumpImpulse = 100;

	minDamageSpeed = 13;
	damageScale = 0.009;

   boxNormalHeadPercentage  = 0.6666f;
   boxNormalTorsoPercentage = 0.3333f;
   boxCrouchHeadPercentage  = 0.6666f;
   boxCrouchTorsoPercentage = 0.3333f;

   boxHeadLeftPercentage    = 0.0f;
   boxHeadRightPercentage   = 1.0f;
   boxHeadFrontPercentage   = 1.0f;
   boxHeadBackPercentage    = 0.0f;
}


//--------------------------------------------------------------------------- 

void Player::setInitInfo(const char *in_datFileName, Point3F pos, Point3F rot)
{
   setRot(rot);
   setPos(pos);
   setDatFileName(in_datFileName);
}

Player::Player()
{
   playerTransition.fTransform.identity();
   transitionTime = 0;

	type = SimPlayerObjectType;
   currentAnimation = -1;
	newAnimTime = 0;
	data = 0;

   jumpSurfaceNormal.set(0,0,1);
   jumpSurfaceLastContact = 100;

   detectMinSize = MIN_DETECT_SIZE;
   detectHazeFactor = 1.0f;
   m_lastJetTime = 0;
   gunPullIn = 0;
   blownUp = false;
   lastContactCount = 0;
   updateSkip = 0;	
   updateDebt = 0;
	fov = M_PI/2.0f;
	viewPitch = 0;
	bounce = 0;
	traction = 0;

   inWorld = true;
   pickNewAnimation = true;
   mount = NULL;
   aiControlled = false;

   old_fov = 0.0f;
   cos_fov = 0.0f;
   tan_fov = 0.0f;
   
	forceMask = -1;
	setMass(9.0f);
	setDrag(1.0f);
	setDensity(1.2f);
	SimMovement::flags.set( UseCurrent | UseDrag | UseFriction);
   netFlags.set( Ghostable );

	jetting = false;
	jetSound = 0;
	
	flameImage.itype = SimRenderImage::Translucent;
	flameImage.shape = NULL;
	flameImage.castShadow = false;
	flameThread = NULL;

	myThread = NULL;
	damageThread = NULL;
   serverAnimation = 0;
   dead = false;
   jetting = false;
   crouching = false;
	pdaing = false;

   falling = false;
   interpDoneTime = 0;
   hasFocus = false;
	
   damageFlash = 0;

	surfaceType = -1;

   nextSup = NULL;
   prevSup = NULL;
	supressionDist = 0;

	// Constant, should be the same for all players.
	PlayerViewOverride.set(EulerF(0,0,M_PI/2),Point3F(0,0,0));

   m_pLensFlare       = new Planet::LensFlare;
}


//--------------------------------------------------------------------------- 

Player::~Player()
{
#ifdef DEBUG
   if(sg.manager && !isGhost())
   {
      // find any observer cameras that have me as a target and assert if there is one
      SimSetIterator i(sg.manager);
      while(SimObject *obj = *i)
      {
         ObserverCamera *obs = dynamic_cast<ObserverCamera *>(obj);
         AssertFatal(!obs || obs->getOrbitObject() != this, "Observer camera not delete notified.");
         ++i;
      }
   }
#endif
   if (flameImage.shape)
   {
      delete flameImage.shape;
      flameImage.shape = NULL;
   }

   delete m_pLensFlare;
   m_pLensFlare = NULL;
}

//---------------------------------------------------------------------------

void Player::onRemove()
{
   disableSupressionField();

   if (cg.player == this)
      cg.player = NULL;
   if(jetSound)
      Sfx::Manager::Stop(manager, jetSound);

	for (int i = 0; i < MaxItemImages; i++)
		resetImageSlot(i);

	Parent::onRemove();
}

bool Player::onAdd()
{
   if(!Parent::onAdd())
      return false;

	addToSet (PlayerSetId);

   if(isGhost())
   {
      addToSet(SensorVisibleSetId);
		collisionMask = ClientCollisionMask;

		addToSet (SimLightSetId);
      addToSet (SimCameraMountSetId);

	   lastAnimateTime = cg.currentTime;


      m_respawnStartTime = cg.currentTime;
      m_respawnEndTime   = cg.currentTime + csm_respawnEffectTime;
   }
   else
   {
      collisionMask = ServerCollisionMask;
   }
	// Make we set our starting position in the container database.
	setPos(getPos());

	lastFreeLook = prefFreeLook;

   return true;
}

void Player::disableSupressionField()
{
	if(nextSup)
	   nextSup->prevSup = prevSup;
	if(prevSup)
	   prevSup->nextSup = nextSup;
}	

void Player::setSupressionField(float distance)
{
   if(!isGhost())
   {
      disableSupressionField();
		if(distance > 0) {
	      SensorManager *rm = (SensorManager *) manager->findObject(SensorManagerId);
	      if(rm)
	         rm->addPlayerSupressor(this);
		}
      supressionDist = distance;
   }
}

// This table is indexed by the Player::MountPoint enum
static char* NodeMountName[] =
{
  "dummy hand",      // "dummyalways_slot0",		//   PrimaryMount         
  "dummy unused",    // "dummyalways_slot2",		//   SecondaryMount       
  "dummy midback",   // "dummyalways_slot1",		//   BackpackMount        
  "dummy lowback",   // "dummyalways_slot4",		//   JetExhaust           
};

bool Player::initResources(GameBase::GameBaseData *in_data)
{
   data = dynamic_cast<PlayerData *>(in_data);

   if(!Parent::initResources(in_data))
      return false;

	updateImageMass();
	setDrag(data->drag);
	setDensity(data->density);

	int rnode = image.shape->getShape().findNode("dummyalways root");
	const TMat3F& rmat = image.shape->getTransform(rnode);
	image.shape->insertOverride("lowerback",0,&PlayerViewOverride);
	image.shape->insertOverride("lowerback",1,const_cast<TMat3F*>(&rmat));
	image.shape->setOverride(2);

	// 
	myThread = createThread (0);
   viewThread = Parent::createThread(0);
   viewThread->setPriority(-1);

   looksSequence = viewThread->GetSequenceIndex("looks");
   crouchLooksSequence = viewThread->GetSequenceIndex("crouch looks");

   if(crouchLooksSequence == -1)
      crouchLooksSequence = looksSequence;

   AssertFatal(looksSequence != -1, "DOH!");
   viewThread->SetSequence( looksSequence, 0.0 );

   damageThread = 0;
   int i;
   for (i = 0; i < Player::NUM_ANIMS; i ++)
	{
		animIndex[i] = myThread->GetSequenceIndex((char *) data->animData[i].name);
      if(animIndex[i] == -1)
         animIndex[i] = 0;
   }

	Point3F pos;
	clearAnimTransform ();
   for (int j = 0; j < Player::NUM_ANIMS; j ++)
	{
      if((j >= ANIM_MOVE_FIRST && j <= ANIM_MOVE_LAST) ||
         (j >= ANIM_CROUCH_MOVE_FIRST && j <= ANIM_CROUCH_MOVE_LAST))
      {
   		myThread->setTimeScale(1.0);
   		myThread->SetSequence(animIndex[j]);
   		myThread->AdvanceTime(1.0);
   		myThread->UpdateSubscriberList();
			image.shape->animateRoot();
   		pos = getAnimTransform();
         offsetList[j].dist = pos.len();
         offsetList[j].dir = pos;
      	if(offsetList[j].dist >= 0.1)
         {
      	   if (data->animData[j].direction <= 0)
               offsetList[j].dir *= -1 / offsetList[j].dist;
            else
               offsetList[j].dir *= 1 / offsetList[j].dist;
         }
	   	clearAnimTransform();
         offsetList[j].hasOffset = true;
      }
      else
         offsetList[j].hasOffset = false;

		myThread->SetSequence(0);
   }
   if(currentAnimation != -1)
   {
      // it's a death animation: set it to the end
      myThread->SetSequence(animIndex[currentAnimation]);
   	if (data->animData[currentAnimation].direction > 0)
   		myThread->SetPosition (0.99f);
   	else
   		myThread->SetPosition (0);
      pickNewAnimation = false;
   }

   delete flameImage.shape;
   flameImage.shape = 0;
	flameThread = 0;

	if (isGhost()) {
		char name[256];
		strcpy(name, data->flameShapeName);
		strcat(name, ".dts");

		ResourceManager *rm = SimResource::get(manager);
		Resource<TSShape> shape = rm->load(name, true);
	   if(bool(shape))
	   {
	      flameImage.shape = new TSShapeInstance(shape, *rm);
         flameThread = flameImage.shape->CreateThread();
         flameThread->setTimeScale(1.0f);
         flameThread->SetSequence("activation");
	   }
	}
	
	// Container & Collision
	boundingBox.fMin.x = -data->boxWidth;
	boundingBox.fMin.y = -data->boxDepth;
	boundingBox.fMin.z = 0;
	boundingBox.fMax.x = data->boxWidth;
	boundingBox.fMax.y = data->boxDepth;
	boundingBox.fMax.z = data->boxNormalHeight;
	collisionImage.bbox.fMin = boundingBox.fMin;
	collisionImage.bbox.fMax = boundingBox.fMax;
	collisionImage.crouchBox.fMin = boundingBox.fMin;
	collisionImage.crouchBox.fMax = boundingBox.fMax;
	setBoundingBox ();
	collisionImage.sphere.radius = (collisionImage.bbox.fMax.z - collisionImage.bbox.fMin.z) / 2;
	collisionImage.sphere.center = Point3F(0.0f,0.0f, collisionImage.sphere.radius);
	collisionImage.shapeInst = image.shape;
	collisionImage.collisionDetail = 0;
	chaseNode = image.shape->getShape().findNode ("dummyalways chasecam");
	eyeNode = image.shape->getNodeAtCurrentDetail("dummy eye");

	// Resolve all the mount node indexes.
   for(i = 0; i < MaxMountPoints; i++) {
		image.shape->setDetailLevel(0);
      mountNode[i] = image.shape->getNodeAtCurrentDetail(NodeMountName[i]);

		// HACK Alert!
		// Lower detail nodes are patch to reference the same
		// transform as the highest detail.
		TS::ShapeInstance::NodeInstance *node = image.shape->getNode(mountNode[i]);
		for (int d = 1; d < image.shape->getShape().fDetails.size(); d++) {
			image.shape->setDetailLevel(d);
      	int dn = image.shape->getNodeAtCurrentDetail(NodeMountName[i]);
			if (dn != -1)
				image.shape->getNode(dn)->fpTransform = node->fpTransform;
		}
   }
   image.shape->setDetailLevel(0);

	// Update initial state of images transfered from the server.
	for (i = 0; i < MaxItemImages; i++) {
		ItemImageEntry& itemImage = itemImageList[i];
		if (itemImage.state == ItemImageEntry::Fire)
			setImageState(i,ItemImageEntry::Fire);
	}

   return true;
}

void Player::setBoundingBox (void)
{
	if (mount && currentAnimation == ANIM_FLIER)
		collisionImage.crouchBox.fMax.z = data->boxNormalHeight / 2;
	else
		if (!crouching)
			collisionImage.crouchBox.fMax.z = data->boxNormalHeight;
		else
			collisionImage.crouchBox.fMax.z = data->boxCrouchHeight;
}



// client to server player update is:

// multiple deltas of:
// position, z, x rotation
// 1 bit jetting
// 1 bit crouching

// 1 bit turning left
// 1 bit turning right
// fire sequence (4 bit)
// wave sequence (3 bit)
// action in forward / side (-1 to 1 for each)

// player determines animation from:
// if dead anim = ANIM_DIE
// else if oneshot anim (hit, waving, landing, standing) play that
// else if jetting anim = ANIM_JET
// else if falling anim = ANIM_FALL
// else if crouching anim = ANIM_CROUCH
// else if forward action || side action
// {
//    if abs(forward) > abs(side)
//       anim = forward action > 0 ? ANIM_RUN : ANIM_RUNBACK
//    else
//       anim = side action > 0 ? ANIM_SIDER : ANIM_SIDEL
// }
// else
//    anim = ANIM_IDLE

// in the player update, it must decide whether or not to set
// falling to true.  If the player is falling and lands,
// depending on the velocity, it will either clear the falling flag
// or clear the fall flag and setAnimation to the landing animation

int Player::getAnimation()
{
   return currentAnimation;
}

void Player::serverPlayAnim(int anim)
{
	if (anim == serverAnimation || (serverAnimation != -1 && data->animData[serverAnimation].priority >= data->animData[anim].priority))
		return;

   if(!dead)
      setMaskBits(AnimationMask);
   serverAnimation = anim;
   setAnimation(anim);
}

void Player::setAnimation(int anim)
{
   bool curCrouching = (currentAnimation >= ANIM_CROUCH_MOVE_FIRST &&
               currentAnimation <= ANIM_CROUCH_MOVE_LAST) || currentAnimation == ANIM_CROUCH_IDLE || currentAnimation == ANIM_CROUCH;

   if(anim == ANIM_CROUCH && curCrouching)
      return;
   if(anim == ANIM_STAND && !curCrouching)
      return;

	if (!myThread || !animIndex || anim < 0 || anim > NUM_ANIMS)
		return;
   
   if(isGhost() && data->animData[anim].soundTag != -1)
      TSFX::PlayAt(data->animData[anim].soundTag, getTransform(), Point3F(0, 0, 0));

   myThread->SetSequence(animIndex[anim]);
   currentAnimation = anim;

	if (data->animData[currentAnimation].direction > 0)
		myThread->SetPosition (0);
	else
		myThread->SetPosition (0.99f);

   pickNewAnimation = false;
}

int Player::pickAnimation()
{
	if (newAnimTime > wg->currentTime)
		return currentAnimation;

	newAnimTime = wg->currentTime + 65;

   // death forces the player animation to an appropriate one
   if(dead)
      return currentAnimation;
	if (mount)
		{
			if (mountPoint == 1)
				return mount->getMountPose();
			else
				return ANIM_APC_RIDE;
		}
   if(jetting || (lastPlayerMove.jetting && !didContact()))
      return ANIM_JET;
   if(falling)
      return ANIM_FALL;
	if (!crouching && pdaing || (!getControlClient() && !mount && !aiControlled))
		return ANIM_PDA;

   // not one of the instant picks... so lets see which one it is
   // rotate the velocity vector into object space:

   Point3F relVel;
   m_mul( getLinearVelocity(), getInvRotation(), &relVel);

   float curMax = 0.35f;
   // check for moves:
   if(crouching)
   {
      int animPick = ANIM_CROUCH_IDLE;
      for(int i = ANIM_CROUCH_MOVE_FIRST; i <= ANIM_CROUCH_MOVE_LAST; i++)
      {
         if(offsetList[i].hasOffset)
         {
            float vel = m_dot(relVel, offsetList[i].dir);
            if(vel > curMax)
            {
               curMax = vel;
               animPick = i;
            }
         }
      }
      return animPick;
   }
   else
   {
      int animPick = ANIM_IDLE;
		for(int i = ANIM_MOVE_FIRST; i <= ANIM_MOVE_LAST; i++)
		{
		   if(offsetList[i].hasOffset)
		   {
		      float vel = m_dot(relVel, offsetList[i].dir);
		      if(vel > curMax)
		      {
		         curMax = vel;
		         animPick = i;
		      }
		   }
		}
      return animPick;
   }
}

void Player::updateAnimation(SimTime t)
{
   viewThread->SetSequence(crouching ? crouchLooksSequence : looksSequence);
	if (viewThread) {
		float pos = 0.5 + (-viewPitch / MaxPitch) * 0.5;
      if(viewThread->getPosition() != pos)
   		viewThread->SetPosition(pos);
	}
   if(!myThread)
      return;

   float curPos = myThread->getPosition();
   int newAnimation; //  = -1;
   bool done = (data->animData[currentAnimation].direction > 0 ?
         curPos >= 0.99 : curPos <= 0.01 );
	bool interruptable = done || myThread->getSequence().fCyclic;


   if(!isGhost())
   {
      if (dead)
      {
         if(!done)
            myThread->AdvanceTime(t);
      }
      else
      {
         if (done)
         {
		      serverAnimation = -1;
            currentAnimation = -1;
         }
         if(serverAnimation != -1)
            myThread->AdvanceTime(t);
         else
         {
            int newAnimation = ANIM_IDLE;
            if(crouching && !jetting && !falling && !mount && !dead)
               newAnimation = ANIM_CROUCH_IDLE;
            if(newAnimation != currentAnimation)
            {
               currentAnimation = newAnimation;
               myThread->SetSequence(animIndex[newAnimation]);
               myThread->SetPosition(newAnimation == ANIM_CROUCH_IDLE ?
                  1 : 0);
            }
         }
      }
		int flags = data->animData[currentAnimation].viewFlags;
		if (flags & AnimData::FirstPerson) {
			image.shape->setOverride(2);
			viewThread->setPriority(-1);
		}
		else {
			image.shape->setOverride(0);
			viewThread->setPriority(5000);
		}
		clearAnimTransform ();
      image.shape->animate();
      return;
   }
   float time = t;
   if(transitionTime != 0)
   {
      if(transitionTime > time)
      {
         myThread->AdvanceTime(time);
         transitionTime -= time;
         time = 0;
      }
      else
      {
         myThread->AdvanceTime(transitionTime);
         time -= transitionTime;
         transitionTime = 0;
      }
   }
   
   if(time != 0)
   {
      if(!pickNewAnimation) // check for current animation done
         pickNewAnimation = interruptable && !dead;

      newAnimation = pickAnimation();
	   if (data->animData[currentAnimation].priority < data->animData[newAnimation].priority)
		   pickNewAnimation = true;

	   bool hasPriority = currentAnimation != -1 && data->animData[currentAnimation].priority > data->animData[newAnimation].priority;
	   bool holdPose = data->animData[currentAnimation].viewFlags & AnimData::HoldPose;
	   bool priorityOverride =  hasPriority && (!interruptable || holdPose);
	
      if(!pickNewAnimation || newAnimation == currentAnimation || priorityOverride)
      {
         if(offsetList[currentAnimation].hasOffset)
         {
            // rotate velocity into player's coordinate system:
            Point3F relVel;
            m_mul( getLinearVelocity(), getInvRotation(), &relVel);
            float lspeed = m_dot(relVel, offsetList[currentAnimation].dir);

            // anim travels offsetList.dist in 1 second
            // we want to slow/speed it by (lspeed / animspeed) * time;

            // only play if we were on the ground recently
        
            if(lastContactCount < 8)
            {
               float speedRatio = lspeed / offsetList[currentAnimation].dist;
               if(speedRatio < 0.66)
                  speedRatio = 0.66f;
               else if(speedRatio > 1.5)
                  speedRatio = 1.5;

           
               myThread->AdvanceTime( float(time) * speedRatio * 
                  data->animData[currentAnimation].direction);
            }
            else
            {
               if(gAnimateTransitions)
               {
                  float curPos = myThread->getPosition();
                  if(curPos > 0.1)
                  {
                     playerTransition.fStartSequence = animIndex[currentAnimation];
                     playerTransition.fEndSequence = animIndex[currentAnimation];
                     playerTransition.fStartPosition = curPos;
                     playerTransition.fEndPosition = 0;
                     playerTransition.fDuration = 0.045;
                     transitionTime = 0.04;
                     if(!myThread->SetTransition(&playerTransition))
                     {
                        myThread->SetSequence(animIndex[currentAnimation]);
                        myThread->SetPosition(0);
                     }
                     else
                        myThread->AdvanceTime(0.005);
                  }
               }
               else
                  myThread->SetPosition(0);
            }
         }
         else
            myThread->AdvanceTime( float(time) * 
                     data->animData[currentAnimation].direction);

         if(pickNewAnimation && done && !myThread->getSequence().fCyclic)
			   myThread->SetPosition (data->animData[currentAnimation].direction > 0 ? 1.0 : 0.0);
      }
      else
      {
         // got a new animation
         // make a transition:
   	   float endpos;
   	   if (data->animData[newAnimation].direction > 0 && newAnimation != ANIM_CROUCH_IDLE)
            endpos = 0.01;
         else
            endpos = 0.99;

         if(gAnimateTransitions)
         {
            playerTransition.fStartSequence = animIndex[currentAnimation];
            playerTransition.fEndSequence = animIndex[newAnimation];
            playerTransition.fStartPosition = myThread->getPosition() + 0.01;
            playerTransition.fEndPosition = endpos;
            playerTransition.fDuration = 0.12;
            transitionTime = 0.13;
      
            currentAnimation = newAnimation;
            if(!myThread->SetTransition(&playerTransition))
            {
               currentAnimation = newAnimation;
               myThread->SetSequence(animIndex[currentAnimation]);
               myThread->SetPosition(endpos);
            }
            else
               myThread->AdvanceTime(0.01);
            // FIXME! no crouch root animation
         }
         else
         {
            currentAnimation = newAnimation;
            myThread->SetSequence(animIndex[currentAnimation]);
            myThread->SetPosition(endpos);
         }
      }
   }
   myThread->UpdateSubscriberList ();

   // update the jet flame animation here
   if(flameThread)
   {
      if(currentAnimation == ANIM_JET && jetting)
   	   flameThread->AdvanceTime(t);
      else
         flameThread->SetPosition(0.0);
   }

	updateShieldThread (t);
   if(image.shape) {
		if (viewThread) {
			int flags = data->animData[currentAnimation].viewFlags;
			if (isFirstPersonView()) {
				if (flags & AnimData::FirstPerson) {
					image.shape->setOverride(2);
					viewThread->setPriority(-1);
				}
				else {
					image.shape->setOverride(0);
					viewThread->setPriority(5000);
				}
			}
			else {
				if (cg.psc->getControlObject() == this) {
					// Chase cam view
					if (flags & AnimData::ChaseCam) {
						image.shape->setOverride(1);
						viewThread->setPriority(-1);
					}
					else {
						image.shape->setOverride(0);
						viewThread->setPriority(5000);
					}
				}
				else {
					if (flags & AnimData::ThirdPerson) {
						image.shape->setOverride(1);
						viewThread->setPriority(-1);
					}
					else {
						image.shape->setOverride(0);
						viewThread->setPriority(5000);
					}
				}
			}
		}
		if (cg.player == this) {
			clearAnimTransform ();
	      image.shape->animate();
		}
   }
}

void Player::serverWave(int anim)
{ 
   if(anim < 0 || anim > ANIM_PLAYER_LAST - ANIM_PLAYER_FIRST)
      return;

   if(!dead)
      serverPlayAnim(ANIM_PLAYER_FIRST + anim);
}

void Player::setDamageLevel(float level)
{
   if(isGhost())
      return;

	if (!dead) {
		Parent::setDamageLevel(level);
		if (damageLevel == data->maxDamage)
			kill();
	}
   else
   {
      Parent::setDamageLevel(level);
      if(damageLevel < data->maxDamage)
      {
         // unkill the player
         dead = false;
         setMaskBits(DamageMask);
         type = SimPlayerObjectType;
         collisionMask = ServerCollisionMask;
      }
   }
}

void Player::setDamageFlash(float df)
{
	damageFlash = df;
	if (damageFlash > 1)
		damageFlash = 1;
	else
		if (damageFlash < 0)
			damageFlash = 0;
}

void Player::kill ()
{
   if (isGhost() || dead)
      return;

	dead = true;
	damageLevel = data->maxDamage;
	setMaskBits (DamageMask);

	disableSupressionField();
	dieTime = manager->getCurrentTime();
	  
	if (const char* script = scriptName("onKilled"))
		Console->executef(2,script,scriptThis());

	type = CorpseObjectType;
	collisionMask = ServerCorpseCollisionMask;
	collisionImage.trigger = true;
	SimMessageEvent::post(this, manager->getCurrentTime() + CorpseTriggerTimeout, CorpseTriggerMsg);
	jetting = false;
}


//--------------------------------------------------------------------------- 

void Player::getThrowVector(Point3F* pos,Point3F* vec)
{
	RMat3F view,mat;
	// Add a small upward component to the pitch to
	// throw the item up a little.
	float offset = 0.5 * cos(viewPitch);
	view.set(EulerF(viewPitch + offset,0.0f,0.0f));
	m_mul(view,(RMat3F&)getTransform(),&mat);

	Point3F vv(0,1,0);
	m_mul(vv,mat,vec);
	*pos = getBoxCenter();
}


//---------------------------------------------------------------------------

float Player::coverage (Point3F eye)
{
	float total = 0;
	Point3F foot = getLinearPosition () + collisionImage.sphere.center / 2;
	Point3F head = foot + collisionImage.sphere.center;
	Point3F temp;
	Point3F lshoulder;
	Point3F rshoulder;
	temp = getRot ();
	RMat3F rot (EulerF(temp.x, temp.y, temp.z));
	
	temp.set (collisionImage.bbox.fMin.x / 2, 0, 0);
	m_mul (temp, rot, &lshoulder);
	lshoulder += getLinearPosition ();
	lshoulder.z += collisionImage.sphere.center.z;
	lshoulder.z += collisionImage.sphere.center.z / 4;
	
	temp.set (collisionImage.bbox.fMax.x / 2, 0, 0);
	m_mul (temp, rot, &rshoulder);
	rshoulder += getLinearPosition ();
	rshoulder.z += collisionImage.sphere.center.z;
	rshoulder.z += collisionImage.sphere.center.z / 4;
	
	SimContainerQuery cq;
	cq.id = getId();
	cq.type = -1;
	cq.mask = SimTerrainObjectType | SimInteriorObjectType | SimPlayerObjectType | StaticObjectType;

	cq.box.fMin = eye;
	cq.box.fMax = foot;
	SimCollisionInfo info;
	SimContainer* root = findObject(manager,SimRootContainerId,(SimContainer*)0);
	bool obstructed = root->findLOS (cq, &info);
	if (!obstructed)
		total += 0.25;

	cq.box.fMax = head;
	obstructed = root->findLOS (cq, &info);
	if (!obstructed)
		total += 0.25;

	cq.box.fMax = lshoulder;
	obstructed = root->findLOS (cq, &info);
	if (!obstructed)
		total += 0.25;

	cq.box.fMax = rshoulder;
	obstructed = root->findLOS (cq, &info);
	if (!obstructed)
		total += 0.25;

	return total;
}

//---------------------------------------------------------------------------

bool Player::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{
	if (!isGhost())
		setMaskBits(OrientationMask);
	return Parent::processEvent (event);
}

//---------------------------------------------------------------------------

bool Player::onSimRenderQueryImage (SimRenderQueryImage* query)
{
	// Update our drawShape flag.
   // If we're first person and mounted on something:
   bool drawShape = true;
	if (query->viewportObject) {
		// Check to see if we are mounted by the current camera
		SimGui::TSControl* vp = dynamic_cast<SimGui::TSControl*>
			(query->viewportObject);
      GameBase *co = cg.psc->getControlObject();
		if (vp && vp->getObject() == cg.psc) {
         if(cg.psc->isFirstPerson()) {
//            if(mount && (cg.psc->getControlObject() == mount || !mount->mountRender()))
            if(mount && co == mount && mountPoint == 1)
               return false;
            if(cg.psc->getControlObject() == this)
               drawShape = false;
         }
      }
      if(co) {
         ObserverCamera *oc = dynamic_cast<ObserverCamera *>(co);
         if(oc && oc->isEyesView(this))
            drawShape = false;
      }
   }

	// Some mounted vehicles don't render the player
	if (mount && !mount->mountRender() && mountPoint == 1)
		return false;
	
	// Get the image from the base class, this also checks to 
	// see if the player is within the viewcone.
	if (!Parent::onSimRenderQueryImage(query))
		return false;
   image.drawShape = drawShape;
	if (mount)
		image.castShadow = false;

	// Animate the player
	clearAnimTransform();
	image.shape->animate();

	// Player fading
   float fadeFactor = 1.0f;
   if (m_fading != NotFading) {
      float factor = float(wg->currentTime - m_fadeStarted) / float(csm_fadeLength);
      // Bail if we're _really faded...
      if (factor > 1.0)
         factor = 1.0;
      fadeFactor = 1.0 - factor;
   }

	// Shadow fading and swing down
	image.alphaLevel = 0.8f * fadeFactor;
	if (image.castShadow) {
	   SimContainer* pContainer = getContainer();
	   while (pContainer != NULL) {
	      if ((pContainer->getType() & SimInteriorObjectType) != 0)
	         break;
	      else
	         pContainer = pContainer->getContainer();
	   }
	   if (pContainer != NULL) {
	      InteriorShape* pItr = dynamic_cast<InteriorShape*>(pContainer);
	      AssertFatal(pItr != NULL, "Not an interiorshape?  Shouldn't happen");

	      if (pItr->getInstance()->isLinked() == true) {
	         image.alphaLevel = 0.0f;
	      } else {
	         // find the closest bbox face...
	         const Box3F& itrBox = pItr->getBoundingBox();
	         Point3F myCenter    = getBoxCenter();
	   
	         ITRGeometry* pGeom = pItr->getInstance()->getHighestGeometry();
	         AssertFatal(pGeom != NULL, "This doesn't seem possible");
		      int leafIndex;
		      if ((leafIndex = pGeom->externalLeaf(myCenter)) == 0) {
		      	leafIndex = pGeom->findLeaf(myCenter);
	         }
	         ITRGeometry::BSPLeafWrap leafWrap(pGeom, leafIndex);
		      int outsideBits = pGeom->getOutsideBits(leafWrap);
	         if ((outsideBits & ITRGeometry::OutsideMask) == 0) {
	            image.alphaLevel = 0.0f;
	         } else {
	            float minDist = itrBox.fMax.x - myCenter.x;
	            AssertFatal(minDist > 0.0f, "Um, that's not possible if I'm contained");

	            if ((itrBox.fMax.y - myCenter.y) < minDist)
	               minDist = itrBox.fMax.y - myCenter.y;
	            if ((itrBox.fMax.z - myCenter.z) < minDist)
	               minDist = itrBox.fMax.z - myCenter.z;
	            if ((myCenter.x - itrBox.fMin.x) < minDist)
	               minDist = myCenter.x - itrBox.fMin.x;
	            if ((myCenter.y - itrBox.fMin.y) < minDist)
	               minDist = myCenter.y - itrBox.fMin.y;
	            if ((myCenter.z - itrBox.fMin.z) < minDist)
	               minDist = myCenter.z - itrBox.fMin.z;
	            if (minDist < 0.0) {
	               AssertWarn(0, "That shouldn't happen, minDist < 0");
	               minDist = 0.0f;
	            }

	            if (minDist < 3.0f) {
	               image.alphaLevel = 0.8f * fadeFactor;
	               image.swingDown  = 0.0f;
	            } else if (minDist > 15.0f) {
	               image.alphaLevel = 0.3 * fadeFactor;
	               image.swingDown  = 1.0f;
	            } else {
	               image.alphaLevel = 0.3f + 0.5f * (1.0f - ((minDist - 3.0f) / 12.0f)) * fadeFactor;
	               image.swingDown  = (minDist - 3.0f) / 12.0f;
	            }
	         }
	      }
	   }
	}

	// Shade based on surface
   float sunAttenuation;
   ColorF positionalColor;
   bool overrideSun = getPositionalColor(positionalColor, sunAttenuation);
   updateSunOverride(overrideSun, sunAttenuation, positionalColor);
   m_lastSunUpdated = wg->currentTime;

	// Mounted items
   int i;
	for (i = 0; i < MaxItemImages; i++)
		if (SimRenderImage* pImage = getImage(i,query->renderContext)) {
			query->image[query->count++] = pImage;
         
         FearRenderImage* pFRI  = dynamic_cast<FearRenderImage*>(pImage);
         if (pFRI != NULL) {
            pFRI->shape->setAlphaAlways(image.shape->getAlwaysAlpha(),
                                        image.shape->getAlwaysAlphaValue());
            if (image.shape->getAlwaysAlpha()) {
               pFRI->sortValue = image.sortValue + 0.01;
               pFRI->itype = SimRenderImage::Translucent;
            }
         }
      }

	// Jet Image
	if (image.drawShape) {
      if (jetting && flameImage.shape) {
         getMountTransform(JetExhaust,&flameImage.transform);
   		// Return the image
         flameImage.sortValue = image.sortValue + 0.01;
   		query->image[query->count++] = &flameImage;
      }
      
      // Set detail scale for player shapes.  Higher for others, here
      //    take halfway to full - need separate value?  
      image.shape->setDetailScale( hasFocus ? prefShapeDetail  :
                              ((prefShapeDetail + 1.0) / 2.0)   );
	}

	// Haze everything
	for(i = 1; i < query->count; i++)
      query->image[i]->hazeValue = query->image[0]->hazeValue;

	return true;
}

bool Player::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
      case SimMessageEventType: {
			SimMessageEvent* ev = (SimMessageEvent*)event;
         if(ev->message == CorpseTimeoutMsg)
            deleteObject();
			else
	         if(ev->message == CorpseTriggerMsg) {
					for (int i = 0; i < MaxItemImages; i++)
						setImageTriggerUp(i);
	         }
         return true;
      }
		onEvent (SimObjectTransformEvent);
		
		default:
			return Parent::processEvent (event);
	}
}

void Player::onCollisionNotify(GameBase* object)
{
	if (!isGhost() && object->getType() & ScriptCollisionMask)
		scriptOnCollision(object);
}

bool Player::onSimLightQuery ( SimLightQuery * query )
{
	query->count = 0;
   query->ambientIntensity.set();
	for (int i = 0; i < MaxItemImages; i++)
		if (TSLight* light = getImageLight(i))
			query->light[query->count++] = light;
	return true;
}

bool Player::onSimCollisionImageQuery (SimCollisionImageQuery *query)
{
	query->count = 1;
	query->image[0] = &collisionImage;
	setBoundingBox();
	collisionImage.transform = getTransform();
	return true;
}

bool Player::getWeaponAmmoType(ProjectileDataType& out_rPrimary,
                          ProjectileDataType& out_rSecondary)
{
	ItemImageEntry& itemImage = itemImageList[0];
   if (itemImage.imageId == -1)
      return false;
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);

   if (imageData.projectile.type != -1)
      out_rPrimary = imageData.projectile;
   out_rSecondary.type = -1;

   return true;
}

bool Player::processQuery(SimQuery* query)
{
	switch (query->type)
	{
		onQuery (SimRenderQueryImage);
		onQuery (SimLightQuery);
		onQuery (SimCollisionImageQuery);
		
		default:
			return Parent::processQuery (query);
	}
}

bool Player::isFirstPersonView()
{
   if(!isGhost())
      return true;

   GameBase *co = cg.psc->getControlObject();   
   if(co)
   {
      ObserverCamera *oc = dynamic_cast<ObserverCamera *>(co);
      if(oc && oc->isEyesView(this))
         return true;
   }
   return (cg.psc->isFirstPerson() && co == this);
}

bool Player::cameraZoomAndCrosshairsEnabled()
{
   return true;
}

void Player::getCameraTransform(float camDist, TMat3F *transform)
{
#define MinCamDist 0.01
   if(camDist < MinCamDist) {
		*transform = getEyeTransform();
		transform->p.z += m_sin (bounce * 2) / 120;
   }
	else {
		int node = chaseNode;
		TMat3F nmat = image.shape->getTransform(node);
		nmat.set(EulerF(viewPitch, 0, 0), nmat.p);
		nmat.p.x = nmat.p.y = 0;
      m_mul(nmat, getTransform(), transform);
		transform->p.z += m_sin (bounce * 2) / 120;

//		Point3F dir;
//		transform->getRow(1, &dir);
//		float oldDist = dir.lenf();
		
		validateEyePoint (transform, camDist);
//		if (newDist > oldDist)
//			cg.psc->setCamDist(newDist);
   }
}

const TMat3F &Player::getEyeTransform()
{
   static TMat3F ret;
	TMat3F temp;

	int node = eyeNode;
	if (node != -1) {
		const TMat3F& nmat = image.shape->getTransform(node);
		temp.set(EulerF(viewPitch, 0, 0), nmat.p);
	}
	else
		temp.identity();

	m_mul(temp, getTransform(), &ret);
   return ret;
}

//---------------------------------------------------------------------------

const Point3F &Player::getLeadVelocity (void)
{
	if (mount)
		return mount->getLeadVelocity ();
	else
		return Parent::getLeadVelocity ();
}

Point3F Player::getLeadCenter (void)
{
	if (mount)
		return mount->getLinearPosition ();
	else
		return getBoxCenter ();
}

bool Player::getVisibleToTeam (int team)
{
	if (mount)
		return mount->getVisibleToTeam (team);
	else
		return Parent::getVisibleToTeam (team);
}

//---------------------------------------------------------------------------

void Player::setMountObject (GameBase *object, int in_mountPoint)
{
   if(mount)
      clearNotify(mount);

   setMaskBits(MountMask);
   mount = object;
   mountPoint = in_mountPoint;
   if(mount)
		{
	      deleteNotify(mount);

		   TMat3F tmat;
			mount->getObjectMountTransform(mountPoint, &tmat);
			if (!mountPoint)
				{
					Point3F rot = getRot ();
					tmat.set (EulerF (rot.x, rot.y, rot.z), tmat.p);
				}
		   setTransform(tmat);
			EulerF angles;
			tmat.angles(&angles);
			if (mountPoint < 1)
				setRot (Point3F (0, 0, angles.z));
			else
				if (mountPoint == 1)
					setRot (Point3F (0, 0, 0));
				else
					{
						Point3F rot = getRot ();
						rot.z -= angles.z;
						setRot (rot);
					}

         setImageTriggerUp(0);

			setLinearVelocity (Point3F (0, 0, 0));
		}
}

void Player::setPDA (bool tf)
{
	pdaing = tf;
}

void Player::updateSelected (int index)
{
	setMaskBits (VisItemMask << index);
}

//---------------------------------------------------------------------------

void Player::onDeleteNotify(SimObject *object)
{
   if(object == mount)
   {
      mount = NULL;
      setMaskBits(MountMask);
   }
   Parent::onDeleteNotify(object);
}

int Player::getDatGroup()
{
   return DataBlockManager::PlayerDataType;
}

bool Player::detectItem(GameBase *obj)
{
   // See if our fov has changed ... saves a couple of trig ops
   if(!fov)
      return false;

   if (old_fov != fov) {
      cos_fov = m_cos(fov / 2);
      tan_fov =   tan(fov / 2);
      old_fov = fov;
   }

	// Cast from either ourselves or the object we are controlling.

   PlayerManager::ClientRep *me = sg.playerManager->findClient(getOwnerClient());
   if(!me)
      return false;
	GameBase* src = me->controlObject;
   if(!src)
      return false;

   // Get the object's position in player space
   TMat3F mat, inv;
	mat = src->getEyeTransform();
   inv = mat; inv.inverse();

   const Box3F &box = obj->getBoundingBox();
   Vector3F distance, objPos = (box.fMin + box.fMax) * 0.5f;
   m_mul(objPos, inv, &distance);

   // Get the angle between the object and the Y axis, see if that
   // angle is less than our FOV angle -- if so, the object is
   // within our FOV
   float rDistance = distance.len();
   if(rDistance < 1)
      return true;

   float cos_theta = distance.y / rDistance;

   if (cos_theta >= cos_fov)
   {
      // Second check: see if the object's projected size is
      // large enough for us to see (converting projected size
      // into percentage-of-screen-size)
      float size = max(box.fMax.z - box.fMin.z, box.fMax.x - box.fMin.x);
      float p_size = size / (rDistance * tan_fov);

      // Multiply the projected size of the object by its haze factor
      // to see if we can really "see" it -- in essence, this means
      // that the hazier an object gets, the bigger it will need to 
      // be for you to see it (pretty sensible)

      if ((p_size * (1.0f - detectHazeFactor * obj->getHaze(mat.p))) > detectMinSize)
      {
         /*
         // Now check its velocity ... this should be thresholded
         // to some range-of-motion ...
         #define MIN_DETECT_VELOCITY 1.0f // should be made a member var
         if (obj->getLinearVelocity().lenf() >= MIN_DETECT_VELOCITY)
         {
         */
            // Build query
            SimContainerQuery query;
				query.id				= src->getId();
            query.type        = -1;
            query.mask        = SimTerrainObjectType   | 
                                SimInteriorObjectType  | 
                                StaticObjectType       | 
                                VehicleObjectType  | 
                                MoveableObjectType;
            query.detail      = SimContainerQuery::DefaultDetail;
            query.box.fMin    = mat.p;        // Shoot from the eye
            query.box.fMax    = objPos;

            // See if we can see the object
            SimCollisionInfo info;
            SimContainer *root = findObject(manager, SimRootContainerId, root);
            root->findLOS(query, &info);
				return !info.object || info.object == obj;
         /*
         }
         */
      }
   }
   return (false);
}

const TMat3F& Player::getLOSTransform()
{
	static TMat3F mat;
	mat = getEyeTransform ();
	return mat;
}

void Player::applyDamage(int type,float value,const Point3F pos,
	const Point3F& vec,const Point3F& mom,int objectId)
{
   const char* script;
	if (!isGhost())
		if ((script = scriptName("onDamage")) != NULL) {
         const char* pVert;
         const char* pQuad;
         getDamageLocation(pos, pVert, pQuad);

			Console->evaluatef("%s(%s,%d,%g,\"%g %g %g\",\"%g %g %g\",\"%g %g %g\",%s,%s,%d);",
				script,scriptThis(),type,value,
				pos.x,pos.y,pos.z,
				vec.x,vec.y,vec.z,
				mom.x,mom.y,mom.z,
            pVert, pQuad,
				objectId);
      }
}

void
Player::getDamageLocation(const Point3F& in_rPos,
                          const char*&   out_rpVert,
                          const char*&   out_rpQuad)
{
   const TMat3F& rTrans = getInvTransform();
   Point3F newPoint;
   m_mul(in_rPos, rTrans, &newPoint);

   float zHeight = (crouching == true) ? data->boxCrouchHeight : data->boxNormalHeight;
   float zTorso  = (crouching == true) ? data->boxCrouchTorsoPercentage : data->boxNormalTorsoPercentage;
   float zHead   = (crouching == true) ? data->boxCrouchHeadPercentage : data->boxNormalHeadPercentage;
   zTorso *= zHeight;
   zHead  *= zHeight;

   if (newPoint.z <= zTorso)
      out_rpVert = "legs";
   else if (newPoint.z <= zHead)
      out_rpVert = "torso";
   else
      out_rpVert = "head";

   if (strcmp(out_rpVert, "head") != 0) {
      if (newPoint.y >= 0.0f) {
         if (newPoint.x <= 0.0f) {
            out_rpQuad = "front_left";
         } else {
            out_rpQuad = "front_right";
         }
      } else {
         if (newPoint.x <= 0.0f) {
            out_rpQuad = "back_left";
         } else {
            out_rpQuad = "back_right";
         }
      }
   } else {
      // Have to do the head noc-tant calculation
      //
      float backToFront = getBoundingBox().len_x();
      float leftToRight = getBoundingBox().len_y();

      float backPoint  = backToFront * (data->boxHeadBackPercentage  - 0.5);
      float frontPoint = backToFront * (data->boxHeadFrontPercentage - 0.5);
      float leftPoint  = leftToRight * (data->boxHeadLeftPercentage  - 0.5);
      float rightPoint = leftToRight * (data->boxHeadRightPercentage - 0.5);

      int index = 0;
      if (newPoint.y < backPoint)
         index += 0;
      else if (newPoint.y <= frontPoint)
         index += 3;
      else
         index += 6;

      if (newPoint.x < leftPoint)
         index += 0;
      else if (newPoint.x <= rightPoint)
         index += 1;
      else
         index += 2;
      
      switch (index) {
        case 0: out_rpQuad = "left_back";   break;
        case 1: out_rpQuad = "middle_back"; break;
        case 2: out_rpQuad = "right_back"; break;
        case 3: out_rpQuad = "left_middle";   break;
        case 4: out_rpQuad = "middle_middle"; break;
        case 5: out_rpQuad = "right_middle"; break;
        case 6: out_rpQuad = "left_front";   break;
        case 7: out_rpQuad = "middle_front"; break;
        case 8: out_rpQuad = "right_front"; break;

        default:
         AssertFatal(0, "Bad non-tant index");
      };
   }
}

