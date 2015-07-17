#include <sim.h>

#pragma warn -inl
#pragma warn -aus

#include "simAction.h"
#include "editCamera.h"
#include "fear.strings.h"
#include "simResource.h"
#include "console.h"
#include "sky.h"
#include "fearDcl.h"
#include "simterrain.h"
#include "loopnet.h"
#include "netPacketStream.h"
#include <fearglobals.h>

#define MAX_PITCH       1.3962      // from observerCamera
#define MIN_INTERVAL    0.033
#define RADIUS          0.5f
#define FOCALLENGT      0.1f

//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT(EditCamera);

EditCamera::EditCamera()
{
	lastUpdate = 0;
	movement.pos.set(0,0,0);
	movement.vel.set(0,0,0);
	rotation.pos.set(-.65,0,1.81);
	rotation.vel.set(0,0,0);
	throttle.set(0,0,0);
   
	collisionMask = -1;
	collisionImage.radius = RADIUS;
	collisionImage.center.set(0.0f,0.0f,0.0f);
	boundingBox.fMin.set(-RADIUS/2.0f,-RADIUS/2.0f,-RADIUS/2.0f);
	boundingBox.fMax.set(+RADIUS/2.0f,+RADIUS/2.0f,+RADIUS/2.0f);
}

//--------------------------------------------------------------------------- 

EditCamera::~EditCamera()
{
}

//--------------------------------------------------------------------------- 

void EditCamera::update(SimTime dt)
{
	if( dt < MIN_INTERVAL )
		dt = MIN_INTERVAL;
      
	// Update position and orientation
	movement.update(dt);
	rotation.update(dt);
	TMat3F mat;
	mat.set( EulerF( rotation.pos.x, rotation.pos.y, rotation.pos.z ), movement.pos );

	// 
	setPosition( mat );
	movement.pos = getTransform().p;

	// Use throttle to calculate new velocity
	RMat3F mrot;
	mrot.set( EulerF( rotation.pos.x, rotation.pos.y, rotation.pos.z ) );
	Vector3F vel = throttle;
	m_mul( vel, mrot, &movement.vel );
	lastUpdate = manager->getCurrentTime();
}

//--------------------------------------------------------------------------- 

const Point3F &EditCamera::getPos( void )
{
	return( getTransform().p );
}

//--------------------------------------------------------------------------- 

void EditCamera::setPos(Point3F pos)
{
	// Not a good example of how to do this.
	TMat3F mat = getTransform();
	mat.p = pos;
	setPosition(mat);
	movement.pos = pos;
}

//--------------------------------------------------------------------------- 

void EditCamera::setActionMap(const char* file)
{
	ResourceManager *rm = SimResource::get(manager);
   actionMap = rm->load(file);
}

//--------------------------------------------------------------------------- 

bool EditCamera::processArguments(int argc, const char **argv)
{
   if (argc >= 1)
   	setActionMap(argv[0]);

   if (argc >= 2) {
      float f[3];
   	memset(f,0,sizeof(f));
      for (int i = 0; i < argc-1  &&  i < 3; i++)
         sscanf(argv[i+1], "%f", &f[i]);
   	setPos(Point3F(f[0], f[1], f[2]));
   }
   return( true );
}

//--------------------------------------------------------------------------- 

bool EditCamera::onAdd()
{
   // Check to make sure that the manager is connected to the server
   //  via loopback transport...
   //
   if (cg.packetStream == NULL)
      return false;
   if (cg.packetStream->getVC() != NULL) {
      if (dynamic_cast<DNet::LOOPTransport*>(cg.packetStream->getVC()->getTransport()) == NULL)
         return false;
   } else {
      if (cg.packetStream->getStreamMode() != Net::PacketStream::PlaybackMode)
         return false;
   }

   deleteNotify(cg.packetStream);
   watchPacketStream = cg.packetStream;

   if( Parent::onAdd() == false ) 
      return( false );

	lastUpdate = manager->getCurrentTime();
   
	SimContainer* root = findObject(manager,SimRootContainerId,root);
	root->addObject(this);
   
   return( true );
}

//--------------------------------------------------------------------------- 

void EditCamera::resetSystemKeys()
{
   // clear the modifiers
   flags |= ~( Mod1 | Mod2 | Mod3 );   
   CMDConsole::getLocked()->setBoolVariable( "$ME::Mod1", false );
   CMDConsole::getLocked()->setBoolVariable( "$ME::Mod2", false );
   CMDConsole::getLocked()->setBoolVariable( "$ME::Mod3", false );
}

//--------------------------------------------------------------------------- 

bool EditCamera::processEvent(const SimEvent* event)
{
	switch( event->type )
   {
		case SimTimerEventType:
      {
			// Update every 1/30 sec.
			update( static_cast< const SimTimerEvent* >( event)->timerInterval );
			return( true );
      }
		case SimFrameEndNotifyEventType:
      {
			update(manager->getCurrentTime() - lastUpdate);
			break;
      }
		case SimGainFocusEventType:
      {
			if( bool( actionMap ) )
         {
				SimActionHandler::find( manager )->push(this,actionMap);
	         addToSet(SimCameraSetId);
	         addToSet(SimFrameEndNotifySetId);
         }
   
         resetSystemKeys();
			return( true );
      }
		case SimLoseFocusEventType:
      {
			if( bool( actionMap ) )
         {
				SimActionHandler::find (manager)->pop(this,actionMap);
	         removeFromSet(SimCameraSetId);
	         removeFromSet(SimFrameEndNotifySetId);
         }
         // clear the modifiers
         resetSystemKeys();
			return( true );
      }
		case SimObjectTransformEventType: 
      {
			const SimObjectTransformEvent* ev = 
				static_cast<const SimObjectTransformEvent*>(event);
			setPos( ev->tmat.p );
         EulerF rot;
         ev->tmat.angles( &rot);
      	rotation.pos.set( rot.x, rot.y, rot.z );
         
         return( true );
		}
		case SimActionEventType: 
      {
			const SimActionEvent * ep = 
				static_cast<const SimActionEvent*>(event);
            
         // grab the move values
         float moveSpeed = CMDConsole::getLocked()->getFloatVariable( "$ME::CameraMoveSpeed", 2.f );
         float rotateSpeed = CMDConsole::getLocked()->getFloatVariable( "$ME::CameraRotateSpeed", 0.2f );
         
			switch( ep->action )
         {
				case IDACTION_MOVEFORWARD:
					throttle.y = ep->fValue * moveSpeed;
					break;
				case IDACTION_MOVEBACK:
					throttle.y = -ep->fValue * moveSpeed;
					break;
				case IDACTION_MOVELEFT:
					throttle.x = -ep->fValue * moveSpeed;
					break;
				case IDACTION_MOVERIGHT:
					throttle.x = ep->fValue * moveSpeed;
					break;
				case IDACTION_MOVEUP:
					throttle.z = ep->fValue * moveSpeed;
					break;
				case IDACTION_MOVEDOWN:
					throttle.z = -ep->fValue * moveSpeed;
					break;
				case IDACTION_PITCH:
            {
               Point3F & rot = rotation.pos;
               rot.x += ep->fValue;

               // make sure camera does not flip
               if( rot.x < -MAX_PITCH )
                  rot.x = -MAX_PITCH;
               if( rot.x > MAX_PITCH )
                  rot.x = MAX_PITCH;
               break;
            }
				case IDACTION_YAW:
               rotation.pos.z += ep->fValue;
					break;
               
            case IDACTION_ME_MOD1: // control
               CMDConsole::getLocked()->setBoolVariable( "$ME::Mod1", ( ep->fValue == 1.f ) );
               flags.set( Mod1, ep->fValue == 1.f );
               break;

            case IDACTION_ME_MOD2: // shift
               CMDConsole::getLocked()->setBoolVariable( "$ME::Mod2", ( ep->fValue == 1.f ) );
               flags.set( Mod2, ep->fValue == 1.f );
               break;

            case IDACTION_ME_MOD3: // alt
               CMDConsole::getLocked()->setBoolVariable( "$ME::Mod3", ( ep->fValue == 1.f ) );
               flags.set( Mod3, ep->fValue == 1.f );
               break;
			}
			return( true );
		}
	}
	return( Parent::processEvent(event) );
}

//--------------------------------------------------------------------------- 

bool EditCamera::processQuery(SimQuery* query)
{
	switch (query->type) 
   {
		case SimCameraQueryType: 
      {
			// We want this object to be attachable
			SimCameraQuery* qp = static_cast<SimCameraQuery*>(query);
			qp->cameraInfo.fov = 90 * M_PI / 360.0f;
			qp->cameraInfo.nearPlane = 0.1f;
			qp->cameraInfo.farPlane = getFarPlane();
         qp->cameraInfo.tmat = getTransform();
	      return( true );
		}
      case SimObjectTransformQueryType: 
      {
			SimObjectTransformQuery *q = static_cast<SimObjectTransformQuery *>(query);
			q->tmat = getTransform();
	      return( true );
      }
		case SimCollisionImageQueryType: 
      {
			SimCollisionImageQuery * qp = static_cast<SimCollisionImageQuery*>(query);
         qp->count = 1;
			qp->image[0] = &collisionImage;
			collisionImage.transform = getTransform();
	      return( true );
		}
		default:
			return( Parent::processQuery(query) );
	}
}

float
EditCamera::getFarPlane()
{
   if (manager == NULL)
      return 100000.0f;

   Sky* pSky = (Sky*)manager->findObject(TribesSkyId);
   if (pSky != NULL) {
      // Use the sky distance, plus a tad.   
      return pSky->getDistance() * 1.1f;      
   }

   SimTerrain* pTerrain = (SimTerrain*)manager->findObject(SimTerrainId);
   if (pTerrain != NULL) {
      // Use the terrain visible distance, plus a tad
      return pTerrain->getVisibleDistance() * 1.1f;      
   }

   // else, well, crap.
   return 100000.0f;
}

void
EditCamera::onDeleteNotify(SimObject* io_pDelete)
{
   if (io_pDelete == watchPacketStream)
      deleteObject();

   Parent::onDeleteNotify(io_pDelete);
}
