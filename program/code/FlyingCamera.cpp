#include <simaction.h>
#include <flyingcamera.h>
#include <fear.strings.h>
#include <simresource.h>
#include <gamebase.h>
#include <simevdcl.h>
#include <simtsviewport.h>
#include <player.h>
#include <PlayerManager.h>
#include <fearglobals.h>
#include <simguitsctrl.h>
#include <simguisimpletext.h>
#include "fearDcl.h"
#include "sky.h"
#include "simterrain.h"
#include "loopnet.h"
#include "netPacketStream.h"

#define MAX_ATTACHED_DISTANCE    20.0f
#define MIN_ATTACHED_DISTANCE     2.0f
#define MAX_ATTACHED_ROTATION     M_PI
#define MIN_ATTACHED_ROTATION     0.0f
#define MAX_ATTACHED_SPEED        1.0f

#define DEFAULT_RADIUS            1.500f
#define DEFAULT_FOCAL_LENGTH      0.100f

// Default globals
static const float g_rDefaultFOV = 90.0f * M_PI / 360.0f;

inline float m_clamp(float value, float min, float max)
{
	return ((value <= min) ? min : (value >= max) ? max : value);
}	

FlyingCamera::FlyingCamera()
{
   int rHalfRadius;

   type             = 0;
   fActivated       = false;
   szGUIFileName[0] = 0;
   szGUIObjName [0] = 0;
   szSAEName[0]     = 0;
   objFollow        = NULL;

   throttle.set(0.0f, 0.0f, 0.0f);
   rotation.set(0.0f, 0.0f, 0.0f);
   position.set(0.0f, 0.0f, 0.0f);

   collisionImage.radius = DEFAULT_RADIUS;
   collisionImage.center.set(0.0f, 0.0f, 0.0f);

   rHalfRadius = DEFAULT_RADIUS / 2.0f;

   boundingBox.fMin.set(-rHalfRadius, -rHalfRadius, -rHalfRadius);
   boundingBox.fMax.set( rHalfRadius,  rHalfRadius,  rHalfRadius);
   
   // set the default max speeds
   maxLinearSpeed.set( 50.0f, 50.0f, 50.0f );
   maxAngularSpeed.set( M_PI, M_PI, M_PI );
}

FlyingCamera::~FlyingCamera()
{
}

bool
FlyingCamera::onAdd()
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

   SimContainer *root;

   if (Parent::onAdd() == false)
   {
      return (false);
   }

   root = findObject(manager, SimRootContainerId, root);
   root->addObject(this);

   return (true);
}

bool FlyingCamera::processArguments(int argc, const char **argv)
{
   if (argc >= 1)
   {
      int   i;
      char *pStr;

      strcpy(szGUIFileName, argv[0]);

      // Skip past path delimiters
      pStr = strrchr(szGUIFileName, '\\');
      pStr = pStr ? pStr + 1 : szGUIFileName;

      // Copy letters up to the period
      for (i = 0; *pStr != '\0' && *pStr != '.'; pStr ++)
      {
         szGUIObjName[i ++] = tolower(*pStr);
      }

      // Skip the period
      if (*pStr == '.')
      {
         // Uppercase letter after the period
         szGUIObjName[i ++] = toupper(*(++ pStr));

         // And copy the rest
         while (*pStr != '\0')
            szGUIObjName[i ++] = tolower(*(++ pStr));
      }

      if (argc == 2)
      {
         strcpy(szSAEName, argv[1]);
      }
      return (true);
   }
   else
   {
      return (false);
   }
}

void FlyingCamera::activate(bool fFreeFly)
{
   SimCameraMountSet *observables;
   static bool fAdded = false;

   // Can't do this in onAdd, because adding ourselves to
   // the camera set crashes the game (???)
   if (fAdded == false)
   {
      addToSet(SimCameraSetId);
      addToSet(SimFrameEndNotifySetId);
      
   }

   reset();

   iCurrentObserved = 0;

   if (fFreeFly)
   {
      setFreeFlyMode();
   }
   else
   {
      // Get the observable group and bind to the first observable object
      observables = dynamic_cast<SimCameraMountSet *>
                                (manager->findObject(SimCameraMountSetId));

      // If we can't attach to the first observable object, we're freeflying
      if (NULL  == observables  || observables->empty() || 
          false == attachToObject((*observables)[iCurrentObserved]))
      {
         setFreeFlyMode();
      }
   }
}

bool FlyingCamera::onSimFrameEndNotifyEvent(const SimFrameEndNotifyEvent *)
{
   float x = rotation.x;
   float d = rDistance;

   // get the move info
   float speed = atof( CMDConsole::getLocked()->getVariable( "MoveSpeed" ) );
   float rot = atof( CMDConsole::getLocked()->getVariable( "PosRotation" ) ) * M_PI * 5;
   
   maxLinearSpeed.set( speed, speed, speed );
   maxAngularSpeed.set( rot, rot, rot );
   
   // Sample inputs and get new throttles
   throttle.x  = input.linear.x.getInput()  * maxLinearSpeed.x;
   throttle.y  = input.linear.y.getInput()  * maxLinearSpeed.y;
   throttle.z  = input.linear.z.getInput()  * maxLinearSpeed.z;
  
   rotation.x += input.angular.x.getInput() * maxAngularSpeed.x;
   rotation.y += input.angular.y.getInput() * maxAngularSpeed.y;
   rotation.z += input.angular.z.getInput() * maxAngularSpeed.z;

   // If we're following we need to keep track of our relative position
   // and rotation
   if (objFollow)
   {
      if (throttle.y)
      {
         rDistance += throttle.y < 0 ? MAX_ATTACHED_SPEED : -MAX_ATTACHED_SPEED;
    
         if (rDistance >= MAX_ATTACHED_DISTANCE || rDistance < MIN_ATTACHED_DISTANCE)
         {
            rDistance = d;
         }
      }

      if (rotation.x >= MAX_ATTACHED_ROTATION || rotation.x <= MIN_ATTACHED_ROTATION)
      {
         rotation.x = x;
      }
   }

   // We only set our own position if we're not following
   else
   {
      Point3F tmp;

      m_mul(throttle, RMat3F(EulerF(rotation.x, 
         rotation.y, rotation.z)), &tmp);

      position += tmp;

      setPosition(TMat3F(EulerF(rotation.x, 
         rotation.y, rotation.z), position), true);
   }
   return (true);
}

bool FlyingCamera::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{
   EulerF angles;

   // Get angles
   event->tmat.angles(&angles);

   rotation.set(angles.x, angles.y, angles.z);
   position = event->tmat.p;

   // Set our position
   setPosition(event->tmat);
   return (true);
}

bool FlyingCamera::handleAction(int iAction, float rValue, int iDevice)
{
   switch (iAction)
   {
      case IDACTION_MOVEFORWARD :
         input.linear.y.sample( rValue, iDevice);
         break;

      case IDACTION_MOVEBACK :
         input.linear.y.sample(-rValue, iDevice);
         break;

      case IDACTION_MOVELEFT :
         input.linear.x.sample(-rValue, iDevice);
         break;

      case IDACTION_MOVERIGHT :
         input.linear.x.sample( rValue, iDevice);
         break;

      case IDACTION_MOVEUP :
         input.linear.z.sample( rValue, iDevice);
         break;

      case IDACTION_MOVEDOWN :
         input.linear.z.sample(-rValue, iDevice);
         break;

      case IDACTION_PITCH :
         input.angular.x.sample( rValue, iDevice);
         break;

      case IDACTION_YAW :
         input.angular.z.sample( rValue, iDevice);
         break;
   
      case IDACTION_OBSERVER_NEXT :
         nextObservable(true);
         break;

      case IDACTION_OBSERVER_PREV :
         nextObservable(false);
         break;

      default :
         return (false);
   }

   return (true);
}

void FlyingCamera::pushActionMap()
{
   Resource<SimActionMap> rsActionMap;

   if (szSAEName[0] != 0)
   {
      rsActionMap = SimResource::get(manager)->load(szSAEName);

      AssertFatal(rsActionMap, "No action map for edit cam");

      SimActionHandler::find(manager)->push(this, rsActionMap);
   }
}

void FlyingCamera::popActionMap()
{
   Resource<SimActionMap> rsActionMap;

   if (szSAEName[0] != 0)
   {
      rsActionMap = SimResource::get(manager)->load(szSAEName);

      AssertFatal(rsActionMap, "No action map for edit cam");

      SimActionHandler::find(manager)->pop(this, rsActionMap);
   }
}

bool FlyingCamera::onSimGainFocusEvent(const SimGainFocusEvent *)
{
   pushActionMap();

   if (fActivated == false)
   {
      activate(true);
      fActivated = true;
   }

   return (true);
}

bool FlyingCamera::onSimLoseFocusEvent(const SimLoseFocusEvent *)
{
   popActionMap();
   return (true);
}

bool FlyingCamera::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      case SimActionEventType :
      {
         const SimActionEvent *action = 
                               dynamic_cast<const SimActionEvent *>(event);
         
         if (false == handleAction(action->action, action->fValue, action->device))
         {
            return (Parent::processEvent(event));
         }
         
         return (true);
      }
      onEvent(SimGainFocusEvent);
      onEvent(SimLoseFocusEvent);
      onEvent(SimFrameEndNotifyEvent);
      onEvent(SimObjectTransformEvent);

      default :
         return (false);
   }
}

bool FlyingCamera::onSimCameraQuery(SimCameraQuery *query)
{
   SimObjectTransformQuery tquery;

   query->cameraInfo.fov       = g_rDefaultFOV;
   query->cameraInfo.nearPlane = DEFAULT_NEAR_PLANE;
   query->cameraInfo.farPlane  = getFarPlane();

   if (objFollow && objFollow->processQuery(&tquery))
   {
      Point3F  objPos = tquery.tmat.p;
      Vector3F x, y, z;
      RMat3F   rmat(EulerF(rotation.x - M_PI / 2, rotation.y, -rotation.z));

      tquery.tmat.p   += m_mul(Vector3F(0.0f, rDistance, 0.0f), rmat, &y);
      tquery.tmat.p.z += 2.0f;

      y.neg();
      y.normalize();
      m_cross(y, Vector3F(0.0f, 0.0f, 1.0f), &x);
      x.normalize();
      m_cross(x, y, &z);

      tquery.tmat.setRow(0, x);
      tquery.tmat.setRow(1, y);
      tquery.tmat.setRow(2, z);

      // Set our position
      findLOSPosition(tquery.tmat, objPos);
   }

   query->cameraInfo.tmat = getTransform();
   return (true);
}

bool FlyingCamera::onSimCollisionImageQuery(SimCollisionImageQuery *query)
{
   query->count = 1;
   query->image[0] = &collisionImage;

   collisionImage.transform = getTransform();
   return (true);
}

bool FlyingCamera::onSimObjectTransformQuery(SimObjectTransformQuery *query)
{
   query->tmat = getTransform();
   return (true);
}

bool FlyingCamera::onSimImageTransformQuery(SimImageTransformQuery *query)
{
   query->transform = getTransform();
   return (true);
}

bool FlyingCamera::processQuery(SimQuery *query)
{
   switch (query->type)
   {
      onQuery(SimCameraQuery);
      onQuery(SimImageTransformQuery);
      onQuery(SimCollisionImageQuery);
      onQuery(SimObjectTransformQuery);

      default :
         return (false);
   }
}

bool FlyingCamera::attachToObject(SimObject *obj)
{
   SimGui::TSControl            *gui;
   SimGui::SimpleText           *txtModCtrl, *txtObjCtrl;
   Player                       *player;
   SimObjectTransformQuery       query;
   PlayerManager::ClientRep *rep;

   // Can this object tell us its position?
   if (obj->processQuery(&query))
   {
      objFollow = obj;
      rDistance = MAX_ATTACHED_DISTANCE;

      updateGUIInfo();

      position.set(0.0f, 0.0f, 0.0f);
      rotation.set(MAX_ATTACHED_ROTATION / 2, 0.0f, 0.0f);
            
      // Set our position and rotation equal to that of the player
      setPosition(query.tmat);
      return (true);
   }

   return (false);
}

void FlyingCamera::reset()
{
   // Ditch old object
   objFollow = NULL;

   // Clear everything out
   throttle.set(0.0f, 0.0f, 0.0f);
}

void FlyingCamera::setFreeFlyMode()
{
   SimGui::TSControl  *gui;
   SimGui::SimpleText *txtModCtrl, *txtObjCtrl;
   EulerF              angles;
   TMat3F              tmat = getTransform();

   // If we didn't get an object, we're free-flying
   iCurrentObserved = -1;

   updateGUIInfo();

   // Change our local information from relative to absolute
   position = tmat.p;
   tmat.angles(&angles);
   rotation.set(angles.x, angles.y, angles.z);
}

void FlyingCamera::nextObservable(bool fForward)
{
   SimCameraMountSet *observables;

   reset();

   // Have to re-get the group every time, b/c of new joins
   observables = dynamic_cast<SimCameraMountSet *>
                             (manager->findObject(SimCameraMountSetId));

   if (observables && !observables->empty())
   {
      if (fForward)
      {
         // Try to go to next observable object
         for (iCurrentObserved ++; 
              iCurrentObserved < observables->size(); iCurrentObserved ++)
         {
            if (attachToObject((*observables)[iCurrentObserved]))
            {
               return;
            }
         }
      }
      else
      {
         // If we're in free-fly mode, go to end of list of observables
         if (iCurrentObserved == -1)
         {
            iCurrentObserved = observables->size();
         }

         // Try to go to next observable object
         for (iCurrentObserved --; 
              iCurrentObserved >= 0; iCurrentObserved --)
         {
            if (attachToObject((*observables)[iCurrentObserved]))
            {
               return;
            }
         }
      }

      // If we couldn't find an attachable object, go to freefly mode
      setFreeFlyMode();
   }
}

void FlyingCamera::findLOSPosition(TMat3F &tmat, const Point3F &objPos)
{
   Point3F            collisionPos;
   SimContainer      *root;
   SimCollisionInfo   info;
   SimContainerQuery  query;

   query.id       = -1;
   query.type     = -1;
   query.mask     = ItemObjectType     | 
   					  SimInteriorObjectType |
                    SimTerrainObjectType |
                    MoveableObjectType | 
                    VehicleObjectType;

   query.box.fMin = objPos; // position of followed object
   query.box.fMax = tmat.p; // our position

   root         = findObject(manager, SimRootContainerId, root);
   collisionPos = tmat.p;

   // Find a line-of-sight from our position to the object we are following
   if (root->findLOS(query, &info)) 
   {
      // If LOS collides with something, find the point at which it hits
      m_mul(info.surfaces[0].position, info.surfaces.tWorld, &collisionPos);
   }

   // Move to where the collision occurred
   tmat.p = collisionPos;

   setPosition(tmat, true);
   position = tmat.p;
}

void FlyingCamera::updateGUIInfo()
{
   GameBase                       *player;
   PlayerManager::ClientRep *rep;
   SimGui::TSControl            *gui;
   SimGui::SimpleText           *txtModCtrl, *txtObjCtrl;

   gui = dynamic_cast<SimGui::TSControl *>
                     (manager->findObject(szGUIObjName));
   if (gui)
   {
      txtModCtrl = dynamic_cast<SimGui::SimpleText *>
                  (gui->findControlWithTag(IDCTG_OBSERVER_CURMOD));

      if (txtModCtrl)
      {
         txtModCtrl->setText("OBSERVER MODE");
      }

      txtObjCtrl = dynamic_cast<SimGui::SimpleText *>
                  (gui->findControlWithTag(IDCTG_OBSERVER_CUROBJ));

      if (txtObjCtrl)
      {
         if (objFollow)
         {
            player = dynamic_cast<GameBase *>(objFollow);

            if (player &&
               (rep = cg.playerManager->findClient(player->getOwnerClient())) != NULL &&
                strlen(rep->name))
            {
               txtObjCtrl->setText(rep->name);
            }
            else
            {
               txtObjCtrl->setText("Follow Mode");
            }
         }
         else
         {
            txtObjCtrl->setText("Free Camera");
         }
      }
   }
}

void FlyingCamera::attachToGUI()
{
   SimGui::TSControl *control;
   SimGui::Canvas    *canvas;
   SimActionEvent    *action;
   
   canvas  = dynamic_cast<SimGui::Canvas *>(manager->findObject("MainWindow"));
   AssertFatal(canvas, "Could not find main window");
   canvas->handleGUIMessage(szGUIFileName);

   control = dynamic_cast<SimGui::TSControl *>(manager->findObject(szGUIObjName));
   AssertFatal(control, "Could not find TSControl for camera");
   
   action  = new SimActionEvent;

   action->fValue = getId();
   action->action = ActionAttach;

   control->postCurrentEvent(action);

   updateGUIInfo();
}

float FlyingCamera::getFarPlane()
{
   if (manager == NULL)
      return DEFAULT_FAR_PLANE;

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
   return DEFAULT_FAR_PLANE;
}

void
FlyingCamera::onDeleteNotify(SimObject* io_pDelete)
{
   if (io_pDelete == watchPacketStream)
      deleteObject();

   Parent::onDeleteNotify(io_pDelete);
}


IMPLEMENT_PERSISTENT(FlyingCamera);
