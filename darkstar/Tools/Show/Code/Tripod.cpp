//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#include <ts.h>
#include <base.h>
#include <sim.h>

#include "simGame.h"
#include "simEv.h"

#include "tripod.h"
#include "myGuy.h"


#include "simPersman.h"

//--------------------------------------------------------------------------- 

void tripod::init()
{
	mover.setOrbitDist(0);
	Hoffcenter=0;
	Voffcenter=0;
	actor = 0;
	angleRelative = false;
}

tripod::tripod()
{
	init();
}

tripod::~tripod()
{
}

void tripod::setEventMap()
{
   // eventMap = SimResource::loadByName(manager, "showMap.sae", true);
   // if(bool(eventMap))
   //    SimActionHandler::find (manager)->push (this, eventMap);

/*
   AnalogEvent joy;
   SimActionEvent action;

   eventMap.clear();

   int flags = SimActionFlagValue|SimActionFlagCenter|
   		SimActionFlagDeadZone|SimActionFlagSquare;
   joy.attributes = EVENT_LOCAL;
   joy.deviceNo= 0;
   action.fValue = 0.10f;

   joy.message   = AnalogEvent::MoveX;
   action.action = MoveRoll;
   eventMap.mapEvent( &joy, &action, flags, false );

   joy.message   = AnalogEvent::MoveY;
   action.action = MovePitch;
   eventMap.mapEvent( &joy, &action, flags, false );

   joy.message   = AnalogEvent::MoveR;
   action.action = MoveYaw;
   eventMap.mapEvent( &joy, &action, flags, false );

   SimActionHandler::find( manager )->push( this, &eventMap );
*/
}

void tripod::update(SimTime t)
{
	// This function is only called by the when
	// we get events from the timer group which is
	// at a fixed interval.
	mover.update(t);
}

bool tripod::onAdd()
{
	if (!ObjParent::onAdd())
		return false;

	// Post by sending an event
   addToSet(SimTimerSetId);
   SimSet *grp;

   grp = (SimSet*)manager->findObject( SimCameraSetId );
  
   if ( grp )
      grp->addObject( this );

   setEventMap();      
      
    return true;
}

bool tripod::processEvent(const SimEvent* event)
{
	switch (event->type) 
   {
		case SimTimerEventType:
			// Update every 1/30 sec. from timer group
			update(static_cast<const SimTimerEvent *>(event)->timerInterval);
			break;
		case SimActionEventType:
      {
			const SimActionEvent* ep = 
				static_cast<const SimActionEvent*>(event);

			if (actor) mover.setMode(moveOrbital);
			else mover.setMode(moveSemiRelative);

			switch(ep->action)
         {
				case ViewRoll:
				case ViewPitch:
				case ViewYaw:
            case MoveUp:
            case MoveDown:
            case MoveForward:
            case MoveBackward:
            case MoveRight:
            case MoveLeft:
					mover.processAction(ep->action,ep->fValue);
               break;
			}
			break;
		}
		default:
			return false;
	}
	return true;
}

bool tripod::processQuery( SimQuery *q )
{
	switch (q->type){
		case SimCameraQueryType:
      {
			// We want this object to be attachable
			SimCameraQuery * qp = static_cast<SimCameraQuery *>(q);
			qp->cameraInfo.fov = 0.6f;
			qp->cameraInfo.nearPlane = 1.0f;
			qp->cameraInfo.farPlane = 1.0E7f;

			if (actor)
			{
				// ask for actor's position here and use it to fill out tmat
				const Point3F & pos = *actor->getPos();
				const EulerF & rot = *actor->getRot();

				TMat3F transMat;
				EulerF absoluteCamAngle;
				absoluteCamAngle=mover.getOrbitRot();

				if (angleRelative)
					// just do yaw, at least for now
					absoluteCamAngle.z += rot.z;

				// get point to orbit around (use center of object usually)
				Point3F focusPoint;
				m_mul(offset,TMat3F(rot,pos),&focusPoint);
							
				transMat.set(absoluteCamAngle,focusPoint);
				if (mover.getOrbitDist()<objectRadius)
				  mover.setOrbitDist(objectRadius);

				Point3F newPos;
				m_mul(Point3F(0.0f,-mover.getOrbitDist(),0.0f),transMat,&newPos);
				transMat.set(absoluteCamAngle,newPos);
				// apply the offcenter variables to move left/right up/down from object center
				m_mul(Point3F(objectRadius*Hoffcenter,0,objectRadius*Voffcenter),transMat,&newPos);
				mover.setPos(newPos);
				mover.setRot(absoluteCamAngle);
			}
         qp->cameraInfo.tmat.set( mover.getRot(), mover.getPos() );
         break;
		}
		default:
			return false;
	}
	return true;
}


