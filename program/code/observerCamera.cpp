#include <observercamera.h>
#include <netghostmanager.h>
#include <feardcl.h>
#include <fearglobals.h>
#include <fearplayerpsc.h>
#include <playerManager.h>

#define MaxPitch 1.3962

ObserverCamera::ObserverCamera(int in_clientId)
: rotation(0,0,0), position(0,0,0)
{
   inWorld = true;
   clientId = in_clientId;
   curOrbitDist = minOrbitDist = maxOrbitDist = 0;
   mode = FlyMode;
   orbitObject = NULL;
   damageObject = NULL;
}

ObserverCamera::~ObserverCamera()
{
}

bool ObserverCamera::onAdd()
{
   if(!Parent::onAdd())
      return false;
   return true;
}

void ObserverCamera::setDamageObject(GameBase *object)
{
   if(object == orbitObject)
      object = NULL;

   if(damageObject)
      clearNotify(damageObject);
   damageObject = object;
   if(damageObject)
      deleteNotify(damageObject);
}

bool ObserverCamera::isEyesView(GameBase *gb)
{
   return gb == orbitObject && curOrbitDist == -1;
}
 
float ObserverCamera::getDamageLevel()
{
   if(orbitObject)
      return orbitObject->getDamageLevel();
   return 0;
}

float ObserverCamera::getEnergyLevel()
{
   if(orbitObject)
      return orbitObject->getEnergyLevel();
   return 0;
}

float ObserverCamera::getDamageFlash()
{
   if(damageObject)
      return damageObject->getDamageFlash();
   else if(orbitObject)
      return orbitObject->getDamageFlash();
   return 0;
}

bool ObserverCamera::getCommandStatus(CommandStatus* status)
{
	// Returns max damage level
	status->damageLevel = 2;
	status->active = false;
	return true;
}

void ObserverCamera::setPlayerControl(bool ctrl)
{
   if(ctrl)
   {
      lastProcessTime = cg.currentTime;
      addToSet(ObserverSetId);
   }
   else
      removeFromSet(ObserverSetId);
}

void ObserverCamera::onDeleteNotify(SimObject *object)
{
   if(object == damageObject)
      damageObject = NULL;
   if(object == orbitObject)
   {
      orbitObject = NULL;
      mode = OrbitPointMode;
      if(!isGhost())
         Console->evaluatef("Observer::orbitObjectDeleted(%d);",clientId);
   }
   Parent::onDeleteNotify(object);
}

void ObserverCamera::setOrbitMode(GameBase *obj, Point3F &pos, Point3F &rot, float minDist, float maxDist, float curDist)
{
   setDamageObject(NULL);
   if(orbitObject)
      clearNotify(orbitObject);
   orbitObject = obj;
   if(orbitObject)
   {
      deleteNotify(orbitObject);
      position = orbitObject->getBoxCenter();
      mode = OrbitObjectMode;
   }
   else
   {
      mode = OrbitPointMode;
      position = pos;
   }
   rotation.set(rot.x, rot.y, rot.z);
   minOrbitDist = minDist;
   maxOrbitDist = maxDist;
   curOrbitDist = curDist;
}

void ObserverCamera::setFlyMode(bool canMove, bool canRotate, Point3F &pos, Point3F &rot)
{
   setDamageObject(NULL);
   if(orbitObject)
   {
      clearNotify(orbitObject);
      orbitObject = NULL;
   }
   position = pos;
   rotation.set(rot.x, rot.y, rot.z);
   setTransform(TMat3F(rotation, position));
   if(canMove && canRotate)
      mode = FlyMode;
   else if(canRotate)
      mode = FreeRotateMode;
   else
      mode = StationaryMode;
}

void ObserverCamera::readPacketData(BitStream *bstream)
{
   if(orbitObject)
   {
      clearNotify(orbitObject);
      orbitObject = NULL;
   }
   bstream->read(sizeof(EulerF), &rotation);
   mode = bstream->readInt(3);
   int ghostIndex;

   switch(mode)
   {
      case StationaryMode:
      case FreeRotateMode:
      case FlyMode:
         bstream->read(sizeof(Point3F), &position);
         setTransform(TMat3F(rotation, position));
         break;
      case OrbitPointMode:
         bstream->read(sizeof(Point3F), &position);
         bstream->read(&minOrbitDist);
         bstream->read(&maxOrbitDist);
         bstream->read(&curOrbitDist);
         break;
      case OrbitObjectMode:
         ghostIndex = bstream->readInt(10);
         bstream->read(&minOrbitDist);
         bstream->read(&maxOrbitDist);
         bstream->read(&curOrbitDist);
         Net::PacketStream *sstream;
         sstream = (Net::PacketStream *) manager->findObject(2048);
         if(sstream)
         {
            Net::GhostManager *gm = sstream->getGhostManager();
            if(gm)
               orbitObject = (GameBase *) gm->resolveGhost(ghostIndex);
         }
         if(!orbitObject)
            mode = OrbitPointMode;
         else
            deleteNotify(orbitObject);
         break;
   }
}

void ObserverCamera::writePacketData(BitStream *bstream)
{
   bstream->write(sizeof(EulerF), &rotation);
   int outMode = mode;
   int ghostIndex = INVALID_GHOST_INDEX;
   if(outMode == OrbitObjectMode)
   {
      PlayerManager::ClientRep *cl = sg.playerManager->findClient(clientId);
      if(cl && cl->ghostManager)
         ghostIndex = orbitObject->getGhostedIndex(cl->ghostManager);
      if(ghostIndex == INVALID_GHOST_INDEX)
         outMode = OrbitPointMode;
   }
   bstream->writeInt(outMode, 3);
   switch(outMode)
   {
      case StationaryMode:
      case FreeRotateMode:
      case FlyMode:
         bstream->write(sizeof(Point3F), &position);
         break;
      case OrbitPointMode:
         bstream->write(sizeof(Point3F), &position);
         bstream->write(minOrbitDist);
         bstream->write(maxOrbitDist);
         bstream->write(curOrbitDist);
         break;
      case OrbitObjectMode:
         bstream->writeInt(ghostIndex, 10);
         bstream->write(minOrbitDist);
         bstream->write(maxOrbitDist);
         bstream->write(curOrbitDist);
         break;
   }
}

void ObserverCamera::serverUpdateMove(PlayerMove *rgpm, int iCount)
{
   while (iCount --)
   {
      if(rgpm->trigger && !pmLast.trigger)
         Console->evaluatef("Observer::triggerDown(%d);", clientId);
      else if(!rgpm->trigger && pmLast.trigger)
         Console->evaluatef("Observer::triggerUp(%d);", clientId);
      if(rgpm->jumpAction)
         Console->evaluatef("Observer::jump(%d);", clientId);

      update(0, 32, &pmLast);
      pmLast = *(rgpm ++);
   }
   bool insideWorld = (position.x >= sg.missionCenterPos.x && position.x <= (sg.missionCenterPos.x + sg.missionCenterExt.x) &&
                       position.y >= sg.missionCenterPos.y && position.y <= (sg.missionCenterPos.y + sg.missionCenterExt.y));

   if(inWorld && !insideWorld)
      Console->evaluatef("Observer::leaveMissionArea(%d);", getId());
   else if(!inWorld && insideWorld)
      Console->evaluatef("Observer::enterMissionArea(%d);", getId());
   inWorld = insideWorld;
}

void ObserverCamera::buildScopeAndCameraInfo(Net::GhostManager *gm, CameraInfo *camInfo)
{
   SimContainerScopeContext  context;
	SimContainer             *root = findObject(manager,SimRootContainerId,root);

   if (root) 
   {
      camInfo->camera = this;
      camInfo->pos   = getLinearPosition();

      getEyeTransform().getRow(1, &camInfo->orientation);

      gm->objectInScope(this);
      if(orbitObject)
         gm->objectInScope(orbitObject);

      // Kludge, need to get the FOV from elsewhere
		camInfo->fov    = 45.0f;
		camInfo->sinFov = m_sin(45.0f * 0.5f);
		camInfo->cosFov = m_cos(45.0f * 0.5f);

      if (getContainer() != NULL)
      {
         camInfo->visibleDistance = getContainer()->getScopeVisibleDist();
      }
      else
      {
		   camInfo->visibleDistance = 1500.0f;
      }

		context.lock(gm, camInfo);
		root->scope(context);
		context.unlock();
	}
}

void ObserverCamera::clientProcess(DWORD curTime)
{
   while(lastProcessTime < curTime)
   {
      DWORD endTick = (lastProcessTime + 32) & ~0x1F;
      if(endTick > curTime)
         endTick = curTime;
      
      PlayerMove *pm;
      pm = cg.psc->getClientMove(lastProcessTime);
      if(pm)
         update(lastProcessTime & 0x1F, ((endTick - 1) & 0x1F) + 1, pm);
      lastProcessTime = endTick;
   }
}

void ObserverCamera::update(DWORD, DWORD tickEnd, PlayerMove *pm)
{
   bool endMove = (tickEnd == 32);
   float dt = tickEnd * 0.001;
   float endFrac = tickEnd / 32.0f;
   float orbitDist = curOrbitDist;

   EulerF rot = rotation;

   TMat3F tmat(rotation, position);
   Point3F vel;

   float rMoveFore = (pm->forwardAction - pm->backwardAction) * (pm->jetting ? 30 : 10);
   float rMoveSide = (pm->rightAction   - pm->leftAction)     * (pm->jetting ? 30 : 10);

   switch(mode)
   {
      case FlyMode:
         m_mul(Point3F(rMoveSide, rMoveFore, 0.0f), (RMat3F &)tmat, &vel);
         vel *= dt;
         tmat.p += vel;
         break;

      case OrbitObjectMode:
      case OrbitPointMode:
         orbitDist -= rMoveFore * dt;
         if(orbitDist < minOrbitDist)
            orbitDist = minOrbitDist;
         else if(orbitDist > maxOrbitDist)
            orbitDist = maxOrbitDist;
   }
   if(mode != StationaryMode)
   {
      rot.z += pm->turnRot * endFrac;
      rot.x += pm->pitch * endFrac;

      if(rot.z < 0)
         rot.z += M_2PI;
      else if(rot.z > M_2PI)
         rot.z -= M_2PI;
      if(rot.x > MaxPitch)
         rot.x = MaxPitch;
      else if(rot.x < -MaxPitch)
         rot.x = -MaxPitch;
   }
   if(mode == OrbitObjectMode || mode == OrbitPointMode)
   {
      if(mode == OrbitObjectMode && curOrbitDist < 0)
      {
         if(curOrbitDist == -1)
            setTransform(orbitObject->getEyeTransform());
         else
         {
            TMat3F transform;
            orbitObject->getCameraTransform(-curOrbitDist, &transform);
            setTransform(transform);
         }
      }
      else
      {
         RMat3F mat(rot);
         if(mode == OrbitObjectMode)
            position = orbitObject->getBoxCenter();
         TMat3F transform(rot, position);
         validateEyePoint(&transform, orbitDist);
         setTransform(transform);
      }
   }
   else
      setTransform(TMat3F(rot, tmat.p));

   if(endMove)
   {
      rotation = rot;
      if(mode != OrbitObjectMode && mode != OrbitPointMode)
         position = tmat.p;
      curOrbitDist = orbitDist;
   }
}

const TMat3F &ObserverCamera::getEyeTransform()
{
   // Eye transform is the normal transform, b/c camera
   // follows its view
   return (getTransform());
}

IMPLEMENT_PERSISTENT_TAG(ObserverCamera, ObserverCamPersTag);