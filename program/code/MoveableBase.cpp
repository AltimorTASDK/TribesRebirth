#include <feardcl.h>
#include <soundfx.h>
#include <fearsfx.h>
#include <console.h>
#include <consoleinternal.h>
#include <simpathquery.h>
#include <sfx.strings.h>
#include <fear.strings.h>
#include <editor.strings.h>
#include <simresource.h>
#include <moveablebase.h>

MoveableBase::MoveableBaseData::MoveableBaseData() : StaticBaseData()
{
   iStartSoundTag   = 0;
   iStopSoundTag    = 0;
   iRunSoundTag     = 0;
   rForwardDelay    = 0.0f;
   rBackwardDelay   = 0.0f;
   iLinearVelocity  = 0;
   rCrushDamage     = 0.0f;
   fInvincible      = false;
}

void MoveableBase::MoveableBaseData::pack(BitStream *stream)
{
   Parent::pack(stream);

   stream->write(iStartSoundTag);
   stream->write(iStopSoundTag);
   stream->write(iRunSoundTag);
   stream->write(rForwardDelay);
   stream->write(rBackwardDelay);
   stream->write(iLinearVelocity);
   stream->write(rCrushDamage);
   stream->write(fInvincible);
}

void MoveableBase::MoveableBaseData::unpack(BitStream *stream)
{
   Parent::unpack(stream);

   stream->read(&iStartSoundTag);
   stream->read(&iStopSoundTag);
   stream->read(&iRunSoundTag);
   stream->read(&rForwardDelay);
   stream->read(&rBackwardDelay);
   stream->read(&iLinearVelocity);
   stream->read(&rCrushDamage);
   stream->read(&fInvincible);
}

MoveableBase::MoveableBase()
{
   fEditing        = false;
   type            = MoveableBaseObjectType;
   state           = STATE_WAYPOINT;
   spm             = NULL;
   iPathID         = -1;
   fPathUpdated    = false;
   iWaypoint       = 0;
   iLastWaypoint   = 0;
   iTargetWaypoint = 0;
   rTravelTime     = 0.0f;
   fInTimerSet     = false;
   mbData          = NULL;

   ldistance.set(0.0f, 0.0f, 0.0f);
   adistance.set(0.0f, 0.0f, 0.0f);

   position.set(0.0f, 0.0f, 0.0f);
   rotation.set(0.0f, 0.0f, 0.0f);

   lvelocity.set(0.0f, 0.0f, 0.0f);
   avelocity.set(0.0f, 0.0f, 0.0f);
}

MoveableBase::~MoveableBase()
{
}

bool MoveableBase::onAdd()
{
   if (false == Parent::onAdd())
   {
      return (false);
   }

   if (mbData)
   {
      // Find the path manager
      spm = dynamic_cast<SimPathManager *>
         (manager->findObject(SimPathManagerId));

      AssertFatal(spm, "Pathed object could not locate path manager");

      // If we aren't in stop state, add to timer bin
      if (state == STATE_FORWARD || state == STATE_BACKWARD)
      {
         fInTimerSet = true;
         addToSet(SimTimerSetId);
      }

      return (true);
   }
   return (false);
}

void MoveableBase::updateServerMoveState(float rElapsedTime)
{
   const char *lpcszState;

   if (state == STATE_FORWARD || state == STATE_BACKWARD)
   {
      // If we've used up our travel time
      if ((rTime += rElapsedTime) >= rTravelTime)
      {
         // We've hit a waypoint
         setMaskBits(NETMASK_WAYPOINT);

         // We are at the next (or previous) waypoint
         iWaypoint += (state == STATE_FORWARD) ? 1 : -1;

         // Update local position
         position += ldistance;
         rotation += adistance;

         // Reset distance vectors
         ldistance.set(0.0f, 0.0f, 0.0f);
         adistance.set(0.0f, 0.0f, 0.0f);

         // See if we're at our target
         if (iWaypoint != iTargetWaypoint)
         {
            // Go to the next waypoint on the way to our target
            buildMovementVector(iWaypoint, state);
         }
         // If we're at our target, we are done moving
         else
         {
            if (fInTimerSet)
            {
               fInTimerSet = false;
               removeFromSet(SimTimerSetId);
            }

            state = STATE_WAYPOINT;
         }

         // Execute waypoint script function
         onWaypoint();

         // If at first or last, execute their script functions
         if (iTargetWaypoint == iLastWaypoint)
         {
            onLast();
         }
         else if (iTargetWaypoint == 0)
         {
            onFirst();
         }

         // Reset time to go to proper position
         rTime = 0.0f;
      }
      else
      {
         setMaskBits(NETMASK_UPDATE);
      }

      updateMovement();
   }
}

bool MoveableBase::updateMovement()
{
   TMat3F  mat;
   Point3F pos, oldPos;
   Point3F rot, oldRot;

   pos.x = position.x + (lvelocity.x * rTime);
   pos.y = position.y + (lvelocity.y * rTime);
   pos.z = position.z + (lvelocity.z * rTime);

   rot.x = rotation.x + (avelocity.x * rTime);
   rot.y = rotation.y + (avelocity.y * rTime);
   rot.z = rotation.z + (avelocity.z * rTime);

   mat    = getTransform();
   oldRot = getRot();
   oldPos = getPos();

   // Have to move before trying to displace, or displaceObjects
   // will think that we haven't moved far enough
   setPos(pos);
   setRot(rot);

	if (!isGhost())
   {
	   if (false == displaceObjects(mat))
	   {
	      setPos(oldPos);
	      setRot(oldRot);

	      return (false);
	   }
   }

   return (true);
}

void MoveableBase::getPathID()
{
   SimPathQuery  query;
   SimGroup     &group = *getGroup();

   for (SimGroup::iterator iter = group.begin(); iter < group.end();  iter ++)
   {
      if ((*iter)->processQuery(&query))
      {
         iPathID       = query.iPathID;
         iLastWaypoint = query.iNumWaypoints - 1;
      }
   }
}

bool MoveableBase::buildMovementVector(Int8 iCurWaypoint, Int8 state)
{
   float                     rDistance;
   Int8                      iCount;
   SimPathManager::Waypoint *pwp = NULL;

   // Get next waypoint from path manager
   if (state == STATE_BACKWARD)
   {
      iCurWaypoint --;
   }

   iCount = spm->getWaypoint(iPathID, iCurWaypoint, &pwp);

   if (pwp && iCurWaypoint < iCount - 1)
   {
      if (state == STATE_FORWARD)
      {
         ldistance = pwp->lVector;
         adistance = pwp->aVector;
      }
      else
      {
         ldistance.x = -(pwp->lVector.x);
         ldistance.y = -(pwp->lVector.y);
         ldistance.z = -(pwp->lVector.z);
         adistance.x = -(pwp->aVector.x);
         adistance.y = -(pwp->aVector.y);
         adistance.z = -(pwp->aVector.z);
      }

      // Determine linear travel time via non-vector linear velocity 
      // given in dynamic data
      rTravelTime = rDistance = ldistance.lenf();

      if (mbData->iLinearVelocity)
      {
         rTravelTime /= mbData->iLinearVelocity;
      }

      // Determine linear velocity components
      lvelocity.x = ldistance.x / rTravelTime;
      lvelocity.y = ldistance.y / rTravelTime;
      lvelocity.z = ldistance.z / rTravelTime;

      // At this point, angular velocity is controlled by
      // linear velocity (much simpler that way).  So determine
      // the angular velocity components via the non-vector
      // linear velocity
      avelocity.x = adistance.x / rTravelTime;
      avelocity.y = adistance.y / rTravelTime;
      avelocity.z = adistance.z / rTravelTime;

      return (true);
   }

   return (false);
}

bool MoveableBase::setMoveState(STATE stNew)
{
   switch (stNew)
   {
      case STATE_STOP     :
         if (fInTimerSet == true)
         {
            fInTimerSet = false;
            removeFromSet(SimTimerSetId);
         }
         state = STATE_STOP;
         break;

      case STATE_FORWARD  :
         if (state == STATE_STOP || state == STATE_WAYPOINT)
         {
            // Try to build movement vector from this waypoint
            // to the next
            if (buildMovementVector(iWaypoint, STATE_FORWARD))
            {
               if (fInTimerSet == false)
               {
                  fInTimerSet = true;
                  addToSet(SimTimerSetId);
               }

               rTime = 0.0f;
               state = STATE_FORWARD;

               setMaskBits(NETMASK_SOUND | NETMASK_WAYPOINT);
            }
         }
         break;

      case STATE_BACKWARD :
         if (state == STATE_STOP || state == STATE_WAYPOINT)
         {
            // Try to build movement vector from previous
            // waypoint to this one
            if (buildMovementVector(iWaypoint, STATE_BACKWARD))
            {
               if (fInTimerSet == false)
               {
                  fInTimerSet = true;
                  addToSet(SimTimerSetId);
               }

               rTime = 0.0f;
               state = STATE_BACKWARD;

               setMaskBits(NETMASK_SOUND | NETMASK_WAYPOINT);
            }
         }
         break;
   }
   return (true);
}

bool MoveableBase::move(MOVEMENT move, Int8 iNewWaypoint)
{
   if (iPathID == -1)
   {
      getPathID();
      fPathUpdated = (iPathID != -1);
   }

   // Can't respond to a command if we're already in motion
   if (iPathID != -1 && isPowered() && state == STATE_WAYPOINT)
   {
      switch (move)
      {
         case FORWARD :
            if (iWaypoint < iLastWaypoint)
            {
               iTargetWaypoint = iLastWaypoint;

               if (mbData->rForwardDelay == 0.0f)
               {
                  setMoveState(STATE_FORWARD);
               }
               else
               {
                  SimMessageEvent::post(this,
                     (float)manager->getCurrentTime() + mbData->rForwardDelay,
                     STATE_FORWARD);
               }
               return (true);
            }
            return (false);

         case BACKWARD :
            if (iWaypoint > 0)
            {
               iTargetWaypoint = 0;

               if (mbData->rForwardDelay == 0.0f)
               {
                  setMoveState(STATE_BACKWARD);
               }
               else
               {
                  SimMessageEvent::post(this,
                     (float)manager->getCurrentTime() + mbData->rForwardDelay,
                     STATE_BACKWARD);
               }
               return (true);
            }
            return (false);
      
         case WAYPOINT :
            if (iNewWaypoint != iWaypoint && iNewWaypoint <= iLastWaypoint &&
                iNewWaypoint >= 0)
            {
               iTargetWaypoint = iNewWaypoint;

               return (setMoveState(iNewWaypoint > 
                  iWaypoint ? STATE_FORWARD : STATE_BACKWARD));
            }
            return (false);
      
         case STOP :
            return (setMoveState(STATE_STOP));
      }
   }

   return (false);
}

bool MoveableBase::onSimTimerEvent(const SimTimerEvent *event)
{
   if (fInTimerSet && isPowered())
   {
      if (iPathID == -1)
      {
         getPathID();
         fPathUpdated = (iPathID != -1);
      }
      else
      {
         updateServerMoveState(event->timerInterval);
      }
   }

   return (true);
}

bool MoveableBase::onSimMessageEvent(const SimMessageEvent *event)
{
   setMoveState((STATE)event->message);
   return (true);
}

bool MoveableBase::onSimTriggerEvent(const SimTriggerEvent *)
{
   // Can't respond to a command if we're already in motion
   if (iPathID != -1 && isPowered() && state == STATE_WAYPOINT)
   {
      if (!mbData->rForwardDelay)
      {
         setMoveState(STATE_FORWARD);
      }
      else
      {
         SimMessageEvent::post(this,
            (float)manager->getCurrentTime() + mbData->rForwardDelay,
            STATE_FORWARD);
      }
   }
   return (false);
}

bool MoveableBase::onSimEditEvent(const SimEditEvent *event)
{
   fEditing = event->state;
   return (true);
}

bool MoveableBase::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimTimerEvent);
      onEvent(SimMessageEvent);
      onEvent(SimTriggerEvent);
      onEvent(SimEditEvent);
   }

   return Parent::processEvent(event);
}

void MoveableBase::onRemove()
{
   if (!isGhost())
   {
      scriptOnRemove();
   }

   Parent::onRemove();
}

bool MoveableBase::initResources(GameBase::GameBaseData *gbData)
{
   if (false == Parent::initResources(gbData))
   {
      return (false);
   }

   mbData = dynamic_cast<MoveableBaseData *>(gbData);
   return (true);
}

DWORD MoveableBase::packUpdate(Net::GhostManager *gm, DWORD dwMask, BitStream *stream)
{
   Parent::packUpdate(gm, dwMask, stream);

   if (!isGhost())
   {
      if (dwMask & NETMASK_INIT)
      {
         stream->write(sizeof(Vector3F), (Int8 *)&(getPos()));
         stream->write(sizeof(Vector3F), (Int8 *)&(getRot()));
         stream->write(iWaypoint);

         packDatFile(stream);

         position = getPos();
         rotation = getRot();
      }
      else
      {
         // If the path has changed
         if (fPathUpdated)
         {
            stream->writeFlag(true);
            stream->write(sizeof(Int8), (void *)&iPathID);
            stream->write(sizeof(Int8), (void *)&iLastWaypoint);
            fPathUpdated = false;
         }
         else
         {
            stream->writeFlag(false);
         }

         // Edit mode ...
         if (dwMask & NETMASK_EDIT)
         {
            stream->writeFlag(true);
            stream->write(sizeof(Vector3F), (Int8 *)&(getPos()));
            stream->write(sizeof(Vector3F), (Int8 *)&(getRot()));
         }
         else
         {
            stream->writeFlag(false);
         }

         // If we hit a waypoint ...
         if (dwMask & NETMASK_WAYPOINT)
         {
            stream->writeFlag(true);
            stream->write((UInt8)((state << 4) | (iWaypoint & 0x0F)));
         }
         else
         {
            stream->writeFlag(false);
         }

         // If the moveable has moved, and is powered ...
         if (dwMask & NETMASK_UPDATE)
         {
            stream->writeFlag(true);
            stream->writeFloat(rTime / rTravelTime, 10);
         }
         else
         {
            stream->writeFlag(false);
         }

         stream->writeFlag(dwMask & NETMASK_SOUND ? true : false);
      }
   }

   return (0);
}

void MoveableBase::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
   int  iTag;
   char szName[256];

   Parent::unpackUpdate(gm, stream);

   if (isGhost())
   {
      // NETMASK_INIT
      if (!manager)
      {
         stream->read(sizeof(Vector3F), (Int8 *)&position);
         stream->read(sizeof(Vector3F), (Int8 *)&rotation);
         stream->read(&iWaypoint);

         unpackDatFile(stream);
 
         setPos(position);
         setRot(rotation);
      }
      else
      {
         // Path ID
         if (stream->readFlag())
         {
            stream->read(sizeof(Int8), (void *)&iPathID);
            stream->read(sizeof(Int8), (void *)&iLastWaypoint);
         }

         // NETMASK_EDIT
         if (stream->readFlag())
         {
            stream->read(sizeof(Vector3F), (Int8 *)&position);
            stream->read(sizeof(Vector3F), (Int8 *)&rotation);
 
            setPos(position);
            setRot(rotation);
         }

         // NETMASK_WAYPOINT
         if (stream->readFlag())
         {
            Int8 iNewWaypoint, stNew, diff;

            stream->read(&stNew);

            iNewWaypoint = (stNew       & 0x0F);
            stNew        = (stNew >> 4) & 0x0F;

            diff = iNewWaypoint - iWaypoint;

            // If the server is only one waypoint ahead of us, then
            // we just need to finish our current journey
            if (diff == 1 || diff == -1)
            {
               position += ldistance;
               rotation += adistance;
            }
            // Otherwise we have to catch up, so get the location
            // of the waypoint on which the server rests, and
            // move ourselves to that location
            else
            {
               SimPathManager::Waypoint *pwp = NULL;
            
               spm->getWaypoint(iPathID, iNewWaypoint, &pwp);

               position   = pwp->position;
               rotation.x = pwp->rotation.x;
               rotation.y = pwp->rotation.y;
               rotation.z = pwp->rotation.z;
            }

            // Sync with the server
            rTime     = 0.0f;
            state     = stNew;
            iWaypoint = iNewWaypoint;

            // And build next movement vector
            buildMovementVector(iWaypoint, state);
         }

         // NETMASK_UPDATE
         if (stream->readFlag())
         {
            rTime = rTravelTime * stream->readFloat(10);
            updateMovement();
         }

         // Sounds
         if (stream->readFlag())
         {
            // Need to change code when new sounds are available
            /*
            // Play 3D action sound
            Sfx::Manager::PlayAt(manager, 
               iTag, getTransform(), Point3F(0.0f, 0.0f, 0.0f));
            */
         }
      }
   }
}

Persistent::Base::Error MoveableBase::read(StreamIO &sio, int iVer, int iUsr)
{
   Parent::read(sio, iVer, iUsr);

   sio.read(&state);
   sio.read(&rTime);
   sio.read(&iWaypoint);

   if (state == STATE_FORWARD || state == STATE_BACKWARD)
   {
      sio.read(sizeof(Vector3F), &position);
      sio.read(sizeof(Vector3F), &rotation);
      sio.read(sizeof(Vector3F), &ldistance);
      sio.read(sizeof(Vector3F), &adistance);
   }

   return ((sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

Persistent::Base::Error MoveableBase::write(StreamIO &sio, int iVer, int iUsr)
{
   Parent::write(sio, iVer, iUsr);

   sio.write(state);
   sio.write(rTime);
   sio.write(iWaypoint);

   if (state == STATE_FORWARD || state == STATE_BACKWARD)
   {
      sio.write(sizeof(Vector3F), &position);
      sio.write(sizeof(Vector3F), &rotation);
      sio.write(sizeof(Vector3F), &ldistance);
      sio.write(sizeof(Vector3F), &adistance);
   }

   return ((sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

void MoveableBase::onFirst()
{
   if (const char *lpcszScript = scriptName("onFirst"))
   {
      Console->executef(3, lpcszScript, scriptThis(), getId());
   }
}

void MoveableBase::onLast()
{
   if (const char *lpcszScript = scriptName("onLast"))
   {
      Console->executef(3, lpcszScript, scriptThis(), getId());
   }
}

void MoveableBase::onWaypoint()
{
   if (const char *lpcszScript = scriptName("onWaypoint"))
   {
      Console->executef(4, lpcszScript, scriptThis(), getId(), iWaypoint);
   }
}