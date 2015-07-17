//-----------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//-----------------------------------------------------------------------------

#include <sim.h>
#include "netGhostManager.h"
#include "simev.h"
#include "console.h"
#include "simSetIterator.h"
#include "netEventManager.h"
#include "netCSDelegate.h";
//-----------------------------------------------------------------------------

#define DebugChecksum FOURCC('D','b','U','g')

namespace Net
{

IMPLEMENT_PERSISTENT_TAG(RemoteCreateEvent, RemoteCreateEventType);

RemoteCreateEvent::RemoteCreateEvent()
{
   type = RemoteCreateEventType;
   newRemoteObject = NULL;
}

RemoteCreateEvent::~RemoteCreateEvent()
{
   delete newRemoteObject;
}

void RemoteCreateEvent::pack(SimManager *manager, PacketStream *ps, BitStream *bstream)
{
   manager;

   AssertFatal(newRemoteObject, "NULL remote object in RemoteCreate post.");

   DWORD tag = newRemoteObject->getPersistTag();

   AssertFatal(tag > 0 && tag < 1024, "Bad ghost tag.");

   int posDelta = -bstream->getCurPos();
   bstream->writeInt(tag, 10);

   newRemoteObject->packUpdate(ps->getGhostManager(), 0xFFFFFFFF, bstream);

   posDelta += bstream->getCurPos();

   PacketStream::getStats()->addBits(PacketStats::Send, posDelta, 
      newRemoteObject->getPersistTag());
   PacketStream::getStats()->addBits(PacketStats::Send, -posDelta, 
      RemoteCreateEventType);

#ifdef DEBUG_NET
   // write out a checksum...
   // we'll check this on the other side for validation
   // later we can put in stuff to check sizes and such

   bstream->writeInt(tag ^ DebugChecksum, 32);
#endif
}

void RemoteCreateEvent::unpack(SimManager *manager, PacketStream *ps, BitStream *bstream)
{
   if(!verifyNotServer(manager))
      return;
   DWORD tag;

   int posDelta = -bstream->getCurPos();
   tag = (DWORD) bstream->readInt( 10);

   SimNetObject *obj = (SimNetObject *) Persistent::create(tag);
   obj->unpackUpdate(ps->getGhostManager(), bstream);
   
   posDelta += bstream->getCurPos();
   PacketStream::getStats()->addBits(PacketStats::Receive, posDelta, 
      obj->getPersistTag());
   PacketStream::getStats()->addBits(PacketStats::Receive, -posDelta, 
      RemoteCreateEventType);
#ifdef DEBUG_NET
   int checksum = bstream->readInt(32);
   AssertFatal( (checksum ^ DebugChecksum) == tag,
      avar("unpackUpdate did not match packUpdate for object of class %s.",
         obj->getClassName()) );
#endif
   manager->addObject(obj);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void GhostManager::packetDropped(DWORD key)
{
   PacketObjectRef *packRef = (PacketObjectRef *) key;

   // loop through all the packRefs in the packet

   while(packRef)
   {
      PacketObjectRef *temp = packRef->nextRef;

      DWORD orFlags = 0;
      AssertFatal(packRef->nextUpdateChain == NULL, "Out of order notify!!");

      // clear out the ref for this object, plus or together all
      // flags from updates after this

      PacketObjectRef **walk = &(packRef->ghost->updateChain);
      while(*walk != packRef)
      {
         orFlags |= (*walk)->mask;
         walk = &((*walk)->nextUpdateChain);
      }
      *walk = 0;
      
      // for any flags we haven't updated since this (dropped) packet
      // or them into the mask so they'll get updated soon

      packRef->ghost->updateMask |= (packRef->mask & ~orFlags);
      
      // if this packet was ghosting an object, set it
      // to re ghost at it's earliest convenience

      if(packRef->ghostInfoFlags & GhostInfo::Ghosting)
      {
         packRef->ghost->flags |= GhostInfo::NotYetGhosted;
         packRef->ghost->flags &= ~GhostInfo::Ghosting;
      }
      
      // otherwise, if it was being deleted,
      // set it to re-delete

      else if(packRef->ghostInfoFlags & GhostInfo::KillingGhost)
      {
         packRef->ghost->flags |= GhostInfo::KillGhost;
         packRef->ghost->flags &= ~GhostInfo::KillingGhost;
      }

      // add this ref to the free list

      packRef->nextRef = freePackRefList;
      freePackRefList = packRef;

      packRef = temp;
   }
}

void GhostManager::packetReceived(DWORD key) 
{
   PacketObjectRef *packRef = (PacketObjectRef *) key;

   // loop through all the notifies in this packet

   while(packRef)
   {
      PacketObjectRef *temp = packRef->nextRef;

      AssertFatal(packRef->nextUpdateChain == NULL, "Out of order notify!!");

      // clear this notify from the end of the object's notify
      // chain

      PacketObjectRef **walk = &(packRef->ghost->updateChain);
      while(*walk != packRef)
      {
         walk = &((*walk)->nextUpdateChain);
      }
      *walk = 0;
      
      // if this object was ghosting , it is now ghosted

      if(packRef->ghostInfoFlags & GhostInfo::Ghosting)
         packRef->ghost->flags &= ~GhostInfo::Ghosting;

      // otherwise, if it was dieing, free the ghost

      else if(packRef->ghostInfoFlags & GhostInfo::KillingGhost)
         freeGhostInfo(packRef->ghost);

      // add the ref to the free list

      packRef->nextRef = freePackRefList;
      freePackRefList = packRef;

      packRef = temp;
   }
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
static int __cdecl UQECompare(const void *a,const void *b)
{
   float ret = ((GhostManager::UpdateQueueEntry *)b)->priority - ((GhostManager::UpdateQueueEntry *)a)->priority;
   return (ret < 0) ? -1 : ((ret > 0) ? 1 : 0);
} 

int GhostManager::UpdateQueueEntry::operator<(const UpdateQueueEntry &qe) const
{ 
   return priority < qe.priority;
}

bool GhostManager::writePacket(BitStream *bstream, DWORD &key)
{
   if(curMode != GhostScopeAlwaysMode && curMode != GhostNormalMode &&
      !(curMode == InactiveMode && ghostList) )
      return false;

   // fill a packet (or two) with ghosting data

   // first step is to check all our polled ghosts:

   // 1. Scope query - find if any new objects have come into
   //    scope and if any have gone out.
   //    While the client rep is sending out the ghost always packets
   //    the scope query is not called... just ghost always objects are in the list
   // 2. call scoped objects' priority functions if the flag set is nonzero
   //    A removed ghost is assumed to have a high priority
   //    If the client rep is sending out ghost always objects
   //    then instead of the priority function, the instantiation
   //    time is used for priority. No objects are updated until
   //    all are ghosted during this phase.
   // 3. call updates based on sorted priority until the packet is
   //    full.  set flags to zero for all updated objects

   camInfo.camera = NULL;
   camInfo.pos.set(0,0,0);
   camInfo.orientation.set(0,1,0);
   camInfo.visibleDistance = 1;
   camInfo.fov = (float)(M_PI / 4.0f);
   camInfo.sinFov = 0.5;
   camInfo.cosFov = 0.5;

   GhostInfo *walk = ghostList;
   bstream->writeFlag(curMode == GhostScopeAlwaysMode);
   
   if(curMode == GhostNormalMode || curMode == InactiveMode)
   {
      if(curMode == GhostNormalMode)
      {
         while(walk)
         {
            if(!(walk->flags & GhostInfo::ScopeAlways))
               walk->flags &= ~GhostInfo::InScope;
            walk = walk->nextRef;
         }
         if(scopeObject)
            scopeObject->buildScopeAndCameraInfo(this, &camInfo);
      }
      walk = ghostList;
      while(walk)
      {
         if(!(walk->flags & GhostInfo::InScope))
         {
            walk->flags |= GhostInfo::KillGhost;
            // remove this ghostManager from the object's ghostRefs
            if(walk->obj)
            {
               Vector<GhostRef>::iterator i;
               for(i = walk->obj->ghosts.begin(); i != walk->obj->ghosts.end(); i++)
               {
                  if(i->ghostManager == this)
                  {
                     walk->obj->ghosts.erase(i);
                     break;
                  }
               }
               walk->obj = NULL;
            }
         }
         walk = walk->nextRef;
      }
   }

   static Vector<UpdateQueueEntry> qPriority;
   static Vector<UpdateQueueEntry> qDelete;

   qPriority.clear();
   qDelete.clear();

   walk = ghostList;
   
   while(walk)
   {
      // clear out any kill objects that haven't been ghosted yet
      if((walk->flags & GhostInfo::KillGhost) && (walk->flags & GhostInfo::NotYetGhosted))
      {
         GhostInfo *temp = walk->nextRef;
         freeGhostInfo(walk);
         walk = temp;
         continue;
      }
      // don't do any ghost processing on objects that are being killed
      // or in the process of ghosting
      else if(!(walk->flags & (GhostInfo::KillingGhost | GhostInfo::Ghosting)))
      {
         if(walk->flags & GhostInfo::KillGhost)
            qDelete.push_back(UpdateQueueEntry(0, walk));
         else if((walk->flags & GhostInfo::NotYetGhosted) && (curMode == GhostScopeAlwaysMode))
            qPriority.push_back(UpdateQueueEntry(-walk->obj->getNumber(), walk));
         else if(walk->updateMask && (curMode == GhostNormalMode))
         {
            float priority;
               if(scopeObject)
                  priority = walk->obj->getUpdatePriority(&camInfo, walk->updateMask, walk->updateSkipCount);
               else
                  priority = 1;
            qPriority.push_back(UpdateQueueEntry(priority, walk));
         }
      }
      walk = walk->nextRef;
   }
   PacketObjectRef *updateList = NULL;
   m_qsort((void *) &qPriority[0], qPriority.size(), sizeof(UpdateQueueEntry), UQECompare);

   Vector<UpdateQueueEntry>::iterator i = qDelete.begin();

   int packedIdSize = sendIdSize - 3; // 0-7 3 bit number
   bstream->writeInt(packedIdSize, 3);

   for(;;i++)
   {
      if(i == qDelete.end())
         i = qPriority.begin();
      if(i == qPriority.end())
         break;

      if(bstream->isFull())
      {
         (*i).obj->updateSkipCount++;
      }
      else
      {
         walk = (*i).obj;

		   if(walk->flags & (GhostInfo::KillGhost | GhostInfo::Ghosting) == (GhostInfo::KillGhost | GhostInfo::Ghosting))
		   	continue;
		   	
         int startPos = bstream->getCurPos();
         bstream->writeFlag(true);

         bstream->writeInt(walk->ghostIndex, sendIdSize);

         DWORD updateMask = walk->updateMask;
      
         PacketObjectRef *upd = freePackRefList;
         if(upd)
            freePackRefList = freePackRefList->nextRef;
         else
            upd = new PacketObjectRef;

         upd->nextRef = updateList;
         updateList = upd;
         upd->nextUpdateChain = walk->updateChain;
         walk->updateChain = upd;

         upd->ghost = walk;
         upd->ghostInfoFlags = 0;

         if(walk->flags & GhostInfo::KillGhost)
         {
            walk->flags &= ~GhostInfo::KillGhost;
            walk->flags |= GhostInfo::KillingGhost;
            upd->ghostInfoFlags = GhostInfo::KillingGhost;
            bstream->writeFlag(true); // killing ghost
         }
         else 
         {
            bstream->writeFlag(false);
            if(walk->flags & GhostInfo::NotYetGhosted)
            {
               if(curMode == GhostScopeAlwaysMode)
                  bstream->writeInt(walk->obj->getNumber(), 32);

#ifdef    DEBUG_NET
               if(!walk->obj->getClass())
               {
                  AssertFatal(0, avar("%s is not a persist declared class.", 
                     walk->obj->getClassName()));
               }
#endif
               DWORD tag = walk->obj->getGhostTag();

               AssertFatal(tag > 0 && tag < 1024, "Bad ghost tag.");

               walk->flags &= ~GhostInfo::NotYetGhosted;
               walk->flags |= GhostInfo::Ghosting;
               upd->ghostInfoFlags = GhostInfo::Ghosting;
               bstream->writeInt(tag, 10);
            }

            // update the object
            DWORD retMask = walk->obj->packUpdate(this, updateMask, bstream);

            AssertFatal((retMask & (~updateMask)) == 0, "Cannot set new bits in packUpdate return");

            walk->updateMask = retMask;
            upd->mask = updateMask & ~retMask;

            PacketStream::getStats()->addBits(PacketStats::Send, bstream->getCurPos() - startPos, walk->obj->getPersistTag());
#ifdef    DEBUG_NET
            // write out a checksum...
            // we'll check this on the other side for validation
            // later we can put in stuff to check sizes and such

            bstream->writeInt(walk->ghostIndex ^ DebugChecksum, 32);
#endif
         }
         walk->updateSkipCount = 0;
      }
   }
   // no more objects...
   bstream->writeFlag(false);

   key = (DWORD) updateList;

   if(curMode == GhostScopeAlwaysMode)
   {
      // if this is our mode,
      // loop through all the objects
      // if they all have been ghosted, (ie, flags NotYetGhosted and 
      // Ghosting are NOT set), then tag this packet as the ghost always
      // done packet
      // just a bit of advanced warning to the other side to add all its
      // objects.  (if this packet is dropped - no big deal, read will
      // figure it out when it gets a normal ghost packet)

      walk = ghostList;
      bool allGhosted = true;
      while(walk)
      {
         if(walk->flags & (GhostInfo::NotYetGhosted | GhostInfo::Ghosting))
         {
            allGhosted = false;
            break;
         }
         walk = walk->nextRef;
      }
      bstream->writeFlag(allGhosted);
      if(allGhosted)
      {
         curMode = GhostNormalMode;
         CSDelegate *delegate = (CSDelegate *) manager->findObject(SimCSDelegateId);
         if(delegate)
            delegate->onGhostAlwaysDone(getOwner());
      }
   }
   return true;
}

// object update format is:
// first bit - if there is an object to update.
// next 10 bits - id of object
// if it is a ghost being killed, the next bit is true, otherwise false
// if it is a new ghost the next 10 bits are the class and subsequent bits are update
// otherwise all the other bits are just update


void GhostManager::readPacket(BitStream *bstream, DWORD)
{
   bool ghostAlwaysMode = bstream->readFlag();

   int idSize;
   idSize = bstream->readInt( 3);
   idSize += 3;

   if(!ghostAlwaysMode && scopeAlwaysList.size())
      flushScopeAlwaysObjects();

   // while there's an object waiting...

   while(bstream->readFlag())
   {
      if(!allowGhosts)
      {
         // houston, we have a problem - a client is trying to make
         // ghosts on us - NOone tries to make ghosts on us.
         setLastError("Invalid packet.");
         return;
      }
      DWORD index;
      int startPos = bstream->getCurPos();
      index = (DWORD) bstream->readInt(idSize);

      if(bstream->readFlag()) // is this ghost being deleted?
         freeGhost(index);
      else
      {
         if(!ghostRefs[index].localGhost) // it's a new ghost... cool
         {
            DWORD tag;
            int oNumber;

            if(ghostAlwaysMode)
               oNumber = bstream->readInt(32);

            tag = (DWORD) bstream->readInt( 10);

            SimNetObject *obj = (SimNetObject *) Persistent::create(tag);
            if(!obj)
            {
               setLastError("Invalid packet.");
               return;
            }
            obj->netFlags.set(SimNetObject::IsGhost);

            // object gets initial update before adding to the manager

            obj->netIndex = index;
            ghostRefs[index].localGhost = obj;
            ghostRefs[index].localGhost->unpackUpdate(this, bstream);
            
            if(!cleanupGroup)
            {
               cleanupGroup = new SimGroup();
               manager->addObject(cleanupGroup, "GhostGroup");
            	deleteNotify(cleanupGroup);
            }
            if(ghostAlwaysMode)
            {
               scopeAlwaysList.push_back(AddListEntry(oNumber, obj));
            }
            else
            {
               if(!manager->registerObject(obj))
               {
                  if(!getLastError()[0])
                     setLastError("Invalid packet.");
                  return;
               }
               cleanupGroup->addObject(obj);
               if(obj->netFlags.test(SimNetObject::PolledGhost))
                  owner->getEventManager()->addPolledObject(obj);
            }
         }
         else
            ghostRefs[index].localGhost->unpackUpdate(this, bstream);

         PacketStream::getStats()->addBits(PacketStats::Receive, bstream->getCurPos() - startPos, ghostRefs[index].localGhost->getPersistTag());
#ifdef DEBUG_NET
         int checksum = bstream->readInt(32);
         AssertFatal( (checksum ^ DebugChecksum) == index,
            avar("unpackUpdate did not match packUpdate for object of class %s.",
               ghostRefs[index].localGhost->getClassName()) );          
#endif
         if(getLastError()[0])
            return;
      }
   }
   if(ghostAlwaysMode)
   {
      if(bstream->readFlag())
         flushScopeAlwaysObjects();
   }
}

//-----------------------------------------------------------------------------

void GhostManager::setScopeObject(SimNetObject *obj)
{
   if(obj == scopeObject)
      return;

   if(scopeObject)
      clearNotify(scopeObject);
   scopeObject = obj;
   if(scopeObject)
      deleteNotify(scopeObject);
}

void GhostManager::removeGhostRef(SimNetObject *no)
{
   Vector<Net::GhostRef>::iterator i;
   for(i = no->ghosts.begin(); i != no->ghosts.end(); i++)
   {
      if(i->ghostManager == this)
      {
         no->ghosts.erase(i);
         return;
      }
   }
}

void GhostManager::freeGhostInfo(GhostInfo *ghost)
{
   // add this id slot to the free ghost ref list

   if(ghost->prevRef)
      ghost->prevRef->nextRef = ghost->nextRef;
   else
      ghostList = ghost->nextRef;

   if(ghost->nextRef)
      ghost->nextRef->prevRef = ghost->prevRef;

   ghost->nextRef = refFreeList;
   refFreeList = ghost;

   AssertFatal(ghost->updateChain == NULL, "Ack!");
}

void GhostManager::freeGhost(DWORD ghostIndex)
{
   SimNetObject *ghost = ghostRefs[ghostIndex].localGhost;

#pragma message("MarkF:  Please check this out.  NULL ghost encountered")
   if ( !ghost )
      return;

   if(ghost->manager) {
      //ghost->deleteObject();
      manager->unregisterObject(ghost);
      delete ghost;
   }
   else
   {
      // if it's not in the manager, it's in the scopeAlwaysList
      SortableVector<AddListEntry>::iterator i;
      for (i = scopeAlwaysList.begin(); i != scopeAlwaysList.end(); i++)
      {
         if((*i).obj == ghost)
         {
            scopeAlwaysList.erase(i);
            delete ghost;
            break;
         }
      }
   }
   ghostRefs[ghostIndex].localGhost = NULL;
}

//-----------------------------------------------------------------------------

void GhostManager::onDeleteNotify(SimObject *object)
{
   if (object == cleanupGroup)
      cleanupGroup = 0;
	else if (object == scopeObject)
      scopeObject = NULL;

   Parent::onDeleteNotify(object);
}

void GhostManager::flushScopeAlwaysObjects()
{
   scopeAlwaysList.sort();

   SortableVector<AddListEntry>::iterator i;
   for(i = scopeAlwaysList.begin(); i != scopeAlwaysList.end(); i++)
   {
      if(!manager->registerObject((*i).obj))
      {
         if(!getLastError()[0])
            setLastError("Invalid packet.");
         return;
      }
      cleanupGroup->addObject((*i).obj);
      if((*i).obj->netFlags.test(SimNetObject::PolledGhost))
         owner->getEventManager()->addPolledObject((*i).obj);
   }
   scopeAlwaysList.clear();
   CSDelegate *delegate = (CSDelegate *) manager->findObject(SimCSDelegateId);
   if(delegate)
      delegate->onGhostAlwaysDone(getOwner());
}

//-----------------------------------------------------------------------------

void GhostManager::objectInScope(SimNetObject *obj)
{
	if (obj->isScopeLocal() && owner->getId() != ServerManagerId+1)
		return;
   Vector<Net::GhostRef>::iterator i;
   for(i = obj->ghosts.begin(); i != obj->ghosts.end(); i++)
   {
      if(i->ghostManager == this)
      {
         i->ghostInfo->flags |= GhostInfo::InScope;
         return;
      }
   }
   createGhost(obj);
}

static int CountBits(int i)
{
   int ret = 1;
   while(i >>= 1)
      ret++;
   return ret;
}

void GhostManager::createGhost(SimNetObject *obj)
{
   AssertWarn(refFreeList, "scoping over 1024 ghosts!");
   if (!refFreeList)
      return;
   GhostInfo *giptr = refFreeList;
   refFreeList = refFreeList->nextRef;

   giptr->flags = GhostInfo::NotYetGhosted | GhostInfo::InScope;
   
   if(obj->netFlags.test(SimNetObject::ScopeAlways))
      giptr->flags |= GhostInfo::ScopeAlways;

   giptr->obj = obj;
   giptr->updateMask = 0xFFFFFFFF;
   giptr->updateChain = NULL;
   giptr->updateSkipCount = 0;

   giptr->prevRef = NULL;
   giptr->nextRef = ghostList;

   if(ghostList)
      ghostList->prevRef = giptr;

   ghostList = giptr;

   GhostRef gr;
   gr.ghostManager = this;
   gr.ghostInfo = giptr;

   obj->ghosts.push_back(gr);

   int giSize = CountBits(giptr->ghostIndex);

   if(giSize > sendIdSize)
      sendIdSize = giSize;
}

//-----------------------------------------------------------------------------

GhostManager::GhostManager(bool allowG)
{
   id = GhostManagerId;
   ghostRefs = new GhostInfo[MaxGhostCount];
   freePackRefList = NULL;
   scopeObject = NULL;
   cleanupGroup = NULL;
   ghostList = NULL;
   curMode = InactiveMode;
   refFreeList = ghostRefs;
   allowGhosts = allowG;
   int i;
   
   for(i = 0; i < MaxGhostCount - 1; i++)
   {
      ghostRefs[i].nextRef = ghostRefs + i+1;
      ghostRefs[i].localGhost = NULL;
      ghostRefs[i].obj = NULL;
      ghostRefs[i].ghostIndex = i;
   }
   ghostRefs[INVALID_GHOST_INDEX - 1].nextRef = NULL;
   ghostList = NULL;

   sendIdSize = 3;
}

void GhostManager::displayFreeListStats()
{
   int i;
   int numGhosts = 0;
//   int numFree = 0;
   for (i = 0; i < MaxGhostCount; i++)
   {
      if (ghostRefs[i].localGhost)
         numGhosts++;      
   }

/*
   GhostInfo *walk = refFreeList;
   while (walk)
   {
      numFree++;
      walk = walk->nextRef;
   }
*/
   Console->printf("num ghosts: %d", numGhosts);

//   if (numGhosts + numFree == MaxGhostCount)
//      Console->printf("ok: num ghosts: %d, num free: %d == max ghosts: %d", numGhosts, numFree, MaxGhostCount);
//   else
//      Console->printf("YIKES: num ghosts: %d, num free: %d != max ghosts: %d", numGhosts, numFree, MaxGhostCount); 

}   

void GhostManager::activate()
{
   curMode = GhostScopeAlwaysMode;
}

void GhostManager::reset()
{
   curMode = InactiveMode;
}

GhostManager::~GhostManager()
{
   delete[] ghostRefs;
   while (freePackRefList)
   {
      PacketObjectRef *tmp = freePackRefList->nextRef; 
      delete freePackRefList;
      freePackRefList = tmp;
   }
}

//-----------------------------------------------------------------------------

bool GhostManager::onAdd()
{
   if(!Parent::onAdd())
      return false;

	if (!getId())
		manager->assignId(this);

   // iterate through the ghost always objects and InScope them...
   SimSet *ghostAlwaysSet = (SimSet *) manager->findObject(SimGhostAlwaysSetId);

   SimSet::iterator i;
   for(i = ghostAlwaysSet->begin(); i != ghostAlwaysSet->end(); i++)
   {
      AssertFatal(dynamic_cast<SimNetObject *>(*i) != NULL, "Non SimNetObject in GhostAlwaysSet");
      SimNetObject *obj = (SimNetObject *)(*i);
      if(obj->netFlags.test(SimNetObject::Ghostable))
         objectInScope(obj);
   }
   // add to the ghostManager set
   addToSet(GhostManagerSetId);
   return true;
}

//-----------------------------------------------------------------------------

void GhostManager::onRemove()
{
   // cleanup time...

   while(ghostList)
   {
      if(ghostList->obj)
         removeGhostRef(ghostList->obj);
      ghostList = ghostList->nextRef;
   }

   // delete any ghosts on my machine

   if(cleanupGroup)
   {
      SimGroup *grp = cleanupGroup;
      manager->unregisterObject(grp);
      delete grp;
      cleanupGroup = NULL;
   }
   Parent::onRemove();
}

SimNetObject *GhostManager::resolveGhost(int id)
{
   return ghostRefs[id].localGhost;
}

SimNetObject *GhostManager::resolveGhostParent(int id)
{
   return ghostRefs[id].obj;
}

int GhostManager::getGhostIndex(SimNetObject *obj)
{
   Vector<GhostRef>::iterator i;
   for(i = obj->ghosts.begin(); i != obj->ghosts.end(); i++)
   {
      if(i->ghostManager == this)
      {
         if(i->ghostInfo->flags & (GhostInfo::KillingGhost | GhostInfo::Ghosting | GhostInfo::NotYetGhosted | GhostInfo::KillGhost))
            return -1;
         return i->ghostInfo->ghostIndex;
      }
   }
   return -1;
}


static char sg_errorBuffer[1024] = { '\0' };

void
setLastError(const char* in_pErrorString)
{
   AssertFatal(in_pErrorString[0] == 0, in_pErrorString);
   AssertFatal(strlen(in_pErrorString) < 1023, "Error, string too long");
   strcpy(sg_errorBuffer, in_pErrorString);
}

const char*
getLastError()
{
   return sg_errorBuffer;
}


} // namespace Net
