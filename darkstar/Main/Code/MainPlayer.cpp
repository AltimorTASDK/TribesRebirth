//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 


#include <sim.h>
#pragma warn -inl
#pragma warn -aus


#include "simAction.h"
#include "mainPlayer.h"
#include "maindcl.h"
#include "main.strings.h"
#include "mainPlayerManager.h"
#include "netEventManager.h"

#define MAX_SPEED			.01
#define MAX_ROTATION		(1/10)

#define RADIUS			0.5f
#define FOCALLENGTH	0.1f
	
class MainPlayerUpdateEvent : public SimEvent
{
public:
	Point3F pos;
	Point3F rot;

   MainPlayerUpdateEvent() { type = MainPlayerUpdateEventType; }

   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);

   DECLARE_PERSISTENT(MainPlayerUpdateEvent);
};

IMPLEMENT_PERSISTENT_TAG(MainPlayerUpdateEvent, MainPlayerUpdateEventType);


void MainPlayerUpdateEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   manager;
   ps;

   bstream->write (sizeof(pos), (Int8 *) &pos);
   bstream->write (sizeof(rot), (Int8 *) &rot);
}

void MainPlayerUpdateEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   ps;

   MainPlayerManager::PlayerRep *pl = MainPlayerManager::get(manager)->findPlayer(sourceManagerId);

   if(!pl)
   {
      address.objectId = -2;
   }
   else
      address.set(pl->player);
   
   bstream->read (sizeof(pos), (Int8 *) &pos);
   bstream->read (sizeof(rot), (Int8 *) &rot);
}

//--------------------------------------------------------------------------- 
IMPLEMENT_PERSISTENT_TAG(MainPlayer, MainPlayerPersTag);
//--------------------------------------------------------------------------- 

MainPlayer::MainPlayer()
{
	movement.pos.set(500,500,500);
	movement.vel.set(0,0,0);
	rotation.pos.set(0,0,0);
	rotation.vel.set(0,0,0);
	throttle.set(0,0,0);
	desired_throttle.set(0,0,0);

	collisionMask = -1;
	collisionImage.radius = RADIUS;
	collisionImage.center.set(0.0f,0.0f,0.0f);
	boundingBox.fMin.set(-RADIUS/2.0f,-RADIUS/2.0f,-RADIUS/2.0f);
	boundingBox.fMax.set(+RADIUS/2.0f,+RADIUS/2.0f,+RADIUS/2.0f);

   image.shape = NULL;
   image.itype = SimRenderImage::Normal;
   netFlags.set(Ghostable);
}

MainPlayer::~MainPlayer()
{
   delete image.shape;
}

void MainPlayer::setGhostInfo(Net::GhostManager *gm, Int32 pid)
{
   ghostManager = gm;
   playerId = pid;
}

//--------------------------------------------------------------------------- 

void MainPlayer::update(SimTime dt)
{
	// Update position and orientation
	movement.update(dt);
	rotation.update(dt);
	TMat3F mat;
	mat.set(EulerF(rotation.pos.x,rotation.pos.y,rotation.pos.z),movement.pos);

	// Collision testing
	setPosition(mat);
	movement.pos = getTransform().p;

	// Use throttle to calculate new velocity
	RMat3F mrot;
	mrot.set(EulerF(rotation.pos.x,rotation.pos.y,rotation.pos.z));
	Vector3F vel = throttle;
	vel *= MAX_SPEED;
	m_mul(vel,mrot,&movement.vel);
	throttle = desired_throttle;
}

//--------------------------------------------------------------------------- 

const Point3F& MainPlayer::getPos (void)
{
	return getTransform().p;
}

void MainPlayer::setPos(Point3F pos)
{
	// Not a good example of how to do this.
	TMat3F mat = getTransform();
	mat.p = pos;
	setPosition(mat);
	movement.pos = pos;
}

void MainPlayer::setActionMap(const char* file)
{
	ResourceManager *rm = SimResource::get(manager);
   actionMap = rm->load(file);
}


//--------------------------------------------------------------------------- 

bool MainPlayer::processArguments(int argc, const char **argv)
{
   if (argc >= 3) {
      float f[3];
   	memset(f,0,sizeof(f));
      for (int i = 0; i < argc-1  &&  i < 3; i++)
         sscanf(argv[i], "%f", &f[i]);
   	setPos(Point3F(f[0], f[1], f[2]));
   }

   if (argc >= 4)
   	setActionMap(argv[3]);

   return true;
}

DWORD MainPlayer::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
   if(mask & InitMask)
   {
      // it's an initial update...
      // if this is the player belonging to that ghost manager,
      // set it to be a polled ghost
      stream->writeFlag(gm == ghostManager);
      stream->write(sizeof(movement.pos), (Int8 *) &movement.pos);
      stream->write(sizeof(rotation.pos), (Int8 *) &rotation.pos);
   } 
   else
   {
      if(stream->writeFlag((mask & PositionMask) && gm != ghostManager))
      {
         stream->write(sizeof(movement.pos), (Int8 *) &movement.pos);
         stream->write(sizeof(rotation.pos), (Int8 *) &rotation.pos);
      }         
   }
   // return a 0 mask...
   return 0;
}

void MainPlayer::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
   bool moved = false;
   if(!manager)
   {
      // set the polled ghost if this is me...
      if(stream->readFlag())
         netFlags.set(PolledGhost);

  		stream->read (sizeof( movement.pos ), (Int8 *)&movement.pos);
  		stream->read (sizeof( rotation.pos ), (Int8 *)&rotation.pos);
      moved = true;
   }
   else
   {
      if(stream->readFlag()) // PositionMask
      {
     		stream->read (sizeof( movement.pos ), (Int8 *)&movement.pos);
     		stream->read (sizeof( rotation.pos ), (Int8 *)&rotation.pos);
         moved = true;
      }
   }
   if(moved)
   {
   	TMat3F mat;
   	mat.set(EulerF(rotation.pos.x,rotation.pos.y,rotation.pos.z),movement.pos);
   	setTransform(mat);
   }
}

bool MainPlayer::onAdd()
{
   if(!Parent::onAdd())
      return false;


   if(isPolledGhost() || 
      (manager == SimGame::get()->getWorld(SimGame::CLIENT)->getManager() && !isGhost()))
   {
      setActionMap("move.sae");
      if(!bool(actionMap))
         return false;
		SimActionHandler::find (manager)->push(this,actionMap);
	   addToSet(SimTimerSetId);
	   addToSet(SimCameraSetId);
   }

   // don't load the shape on the server side...
   if(isGhost())
   {
      ResourceManager *rm = SimResource::get(manager);
      Resource<TSShape> shape = rm->load("flyer.dts", true);
      if(!bool(shape))
         return false;
      image.shape = new TSShapeInstance(shape, *rm);
   }

	SimContainer* root = findObject(manager,SimRootContainerId,root);
	root->addObject(this);
	return true;
}

bool MainPlayer::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
      // player to server update
      case MainPlayerUpdateEventType:
      {
         MainPlayerUpdateEvent *evt = (MainPlayerUpdateEvent *) event;
         movement.pos = evt->pos;
         rotation.pos = evt->rot;
      	TMat3F mat;
      	mat.set(EulerF(rotation.pos.x,rotation.pos.y,rotation.pos.z),movement.pos);
         setMaskBits(PositionMask);
         return true;
      }
      // polled ghost player update event (create a MainPlayerUpdate to server)
      case SimPolledUpdateEventType:
      {
         MainPlayerUpdateEvent *mpu = new MainPlayerUpdateEvent;
         mpu->pos = movement.pos;
         mpu->rot = rotation.pos;
         ((SimPolledUpdateEvent *) event)->evManager->postRemoteEvent(mpu);
         return true;
      }
		case SimTimerEventType:
			// Update every 1/30 sec.
			update(static_cast<const SimTimerEvent*>(event)->timerInterval);
			return true;
		case SimCollisionEventType:
			//??
			break;
		case SimGainFocusEventType:
			if (bool(actionMap))
				SimActionHandler::find (manager)->push(this,actionMap);
			break;
		case SimLoseFocusEventType:
			if (bool(actionMap))
				SimActionHandler::find (manager)->pop(this,actionMap);
			break;
		case SimActionEventType:
		{
			const SimActionEvent* ep = 
				static_cast<const SimActionEvent*>(event);
			switch(ep->action) {
				case IDACTION_SETSPEED:
					desired_throttle.y = ep->fValue;
					break;
				case IDACTION_MOVEFORWARD:
					desired_throttle.y = ep->fValue;
					desired_throttle.x = desired_throttle.z = 0.0f;
					break;
				case IDACTION_MOVEBACK:
					desired_throttle.y = -ep->fValue;
					desired_throttle.x = desired_throttle.z = 0.0f;
					break;
				case IDACTION_MOVELEFT:
					desired_throttle.x = -ep->fValue;
					desired_throttle.y = desired_throttle.z = 0.0f;
					break;
				case IDACTION_MOVERIGHT:
					desired_throttle.x = ep->fValue;
					desired_throttle.y = desired_throttle.z = 0.0f;
					break;
				case IDACTION_MOVEUP:
					desired_throttle.z = ep->fValue;
					desired_throttle.y = desired_throttle.x = 0.0f;
					break;
				case IDACTION_MOVEDOWN:
					desired_throttle.z = -ep->fValue;
					desired_throttle.y = desired_throttle.x = 0.0f;
					break;
				case IDACTION_ROLL:
					rotation.vel.y = ep->fValue;
					break;
            case IDACTION_MPITCH:
               rotation.pos.x += ep->fValue;
               break;
            case IDACTION_MYAW:
               rotation.pos.z += ep->fValue;
               break;
				case IDACTION_PITCH:
					rotation.vel.x = ep->fValue;
					break;
				case IDACTION_YAW:
					rotation.vel.z = ep->fValue;
					break;
			}
			return true;
		}
	}
	return false;
}


//--------------------------------------------------------------------------- 

bool MainPlayer::processQuery(SimQuery* query)
{
	switch (query->type)
	{
		case SimRenderQueryImageType:
		{
         if(!image.shape)
            return false;

//         image.castShadow = false;
		   SimRenderQueryImage *qp = (SimRenderQueryImage *) query;
         image.transform = getTransform();
         qp->count = 1;
         qp->image[0] = &image;
			return true;
		}
		case SimCameraQueryType:
		{
			// We want this object to be attachable
			SimCameraQuery* qp = static_cast<SimCameraQuery*>(query);
			qp->cameraInfo.fov = 0.6f;
			qp->cameraInfo.nearPlane = FOCALLENGTH;
			qp->cameraInfo.farPlane = 100000;
         qp->cameraInfo.tmat = getTransform();
			break;
		}
		case SimCollisionImageQueryType:
		{
			SimCollisionImageQuery* qp = static_cast<SimCollisionImageQuery*>(query);
         qp->count = 1;
			qp->image[0] = &collisionImage;
			collisionImage.transform = getTransform();
			break;
		}
      case SimObjectTransformQueryType:
      {
			SimObjectTransformQuery *q = static_cast<SimObjectTransformQuery *>(query);
			q->tmat = getTransform();
			break;
      }
		default:
         return false;
	}
	return true;
}

