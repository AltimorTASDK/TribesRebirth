//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "netGhostManager.h"
#include "simObjectTypes.h"
#include "simShapeGroupRep.h"
#include "simShape.h"
#include "simShapeGroup.h"
#include "inspect.h"
#include "commonEditor.strings.h"

int SimShapeGroupRep::sm_fileVersion = 1;

//------------------------------------------------------------------------------
//--------------------------------------
// C'tors/D'tors
//--------------------------------------
//
SimShapeGroupRep::SimShapeGroupRep()
 : m_isInTimerSet(false),
   m_globallyConserveDamage(false)
{
   netFlags.set(ScopeAlways);
   netFlags.set(Ghostable);
}

SimShapeGroupRep::~SimShapeGroupRep()
{
   m_objectList.clear();
   m_unresolvedGhostIds.clear();
   m_isInTimerSet = false;
}

void
SimShapeGroupRep::damageForward(const SimShape* in_pForwarder,
                                const double    in_damage)
{
   for (SimShapeList::iterator itr = m_objectList.begin();
        itr != m_objectList.end(); itr++) {
      if (*itr != in_pForwarder)
         (*itr)->acceptForwardedDamage(in_damage);
   }
}

//------------------------------------------------------------------------------
//--------------------------------------
// Event/Query handling
//--------------------------------------
bool SimShapeGroupRep::processEvent(const SimEvent* event)
{
   switch (event->type) {
      onEvent(SimTimerEvent);
   }
	return Parent::processEvent(event);
}

bool SimShapeGroupRep::onSimTimerEvent(const SimTimerEvent*)
{
   AssertFatal(isGhost() == true, "This should never be run on the server...");

   processUnresolvedGhosts();
   return true;
}

bool SimShapeGroupRep::onAdd()
{
	if (!Parent::onAdd())
		return false;

   // Nothing to do but get onto the timer list to update any unresolved ghosts
   //  if we are a ghost ourself
   //
   if (isGhost() == true)
      processUnresolvedGhosts();
	return true;
}

void
SimShapeGroupRep::inspectRead(Inspect *sd)
{
   Parent::inspectRead(sd);
   
   sd->read(IDITG_SSP_GLOBALLYCONSERVEDAMAGE, m_globallyConserveDamage);
}

void
SimShapeGroupRep::inspectWrite(Inspect *sd)
{
   Parent::inspectWrite(sd);
   
   sd->write(IDITG_SSP_GLOBALLYCONSERVEDAMAGE, m_globallyConserveDamage);
}

//------------------------------------------------------------------------------
//--------------------------------------
// Object list management
//--------------------------------------

//--------------------------------------
// Client side only
//--------------------------------------
//
void
SimShapeGroupRep::onDeleteNotify(SimObject* io_pDelete)
{
   AssertFatal(isGhost() == true, "This should never be run on the server...");

   SimShape* pShape = static_cast<SimShape*>(io_pDelete);
   
   for (SimShapeList::iterator itr = m_objectList.begin();
        itr != m_objectList.end();
        itr++) {
      if (pShape == *itr) {
         (*itr)->clearDamageParent();
         m_objectList.erase(itr);
         break;
      }
   }
}

void
SimShapeGroupRep::processUnresolvedGhosts()
{
   AssertFatal(isGhost() == true, "This should never be run on the server...");
   AssertFatal(manager != NULL, "Called prematurely");

   if (m_unresolvedGhostIds.size() == 0) {
      // Nothing to do, other than make sure we aren't in the timer group
      //
      removeFromTimerSet();
   } else {
      Net::GhostManager* gm = Net::GhostManager::get(manager);
      AssertFatal(gm != NULL, "Couldn't find a GhostManager...");
      
      for (Vector<int>::iterator itr = m_unresolvedGhostIds.end() - 1;
           itr >= m_unresolvedGhostIds.begin();
           itr--) {
         int id = *itr;
         AssertFatal(id != -1, "This should never happen");

         SimObject* pObject = gm->resolveGhost(id);
         SimShape*  pShape  = static_cast<SimShape*>(pObject);
         
         if (pObject == NULL) {
            continue;
         }
         
         AssertFatal((pObject->getType() & SimShapeObjectType) != 0,
                     "Ghost id refers to a non-shape object!");
         m_objectList.push_back(pShape);
         manager->deleteNotify(pShape);
         pShape->setDamageParent(this);
         m_unresolvedGhostIds.erase(itr);
      }
      
      if (m_unresolvedGhostIds.size() != 0) {
         addToTimerSet();
      }
   }
}

void SimShapeGroupRep::addToTimerSet()
{
   AssertFatal(isGhost() == true, "This should never be run on the server...");
   AssertFatal(manager != NULL,   "Oy!  No manager");

   if (m_isInTimerSet == true)
      return;
   
   SimSet* pSimTimerSet = dynamic_cast<SimSet*>(manager->findObject(SimTimerSetId));
   AssertFatal(pSimTimerSet != NULL, "No timer set?");
   
   pSimTimerSet->addObject(this);
   m_isInTimerSet = true;
}

void SimShapeGroupRep::removeFromTimerSet()
{
   AssertFatal(isGhost() == true, "This should never be run on the server...");
   AssertFatal(manager != NULL,   "Oy!  No manager");

   if (m_isInTimerSet == false)
      return;
      
   SimSet* pSimTimerSet = dynamic_cast<SimSet*>(manager->findObject(SimTimerSetId));
   AssertFatal(pSimTimerSet != NULL, "No timer set?");
   
   pSimTimerSet->removeObject(this);
   m_isInTimerSet = false;
}

//--------------------------------------
// Both sides
//--------------------------------------
//
void SimShapeGroupRep::removeAll()
{
   for (SimShapeList::iterator itr = m_objectList.begin();
        itr != m_objectList.end(); itr++) {
      (*itr)->clearDamageParent();
   }

   m_objectList.clear();
   m_unresolvedGhostIds.clear();
   setMaskBits(ObjectsChangedMask);
}


//--------------------------------------
// Server side only
//--------------------------------------
//
void SimShapeGroupRep::initFromCurr()
{
   AssertFatal(isGhost() == false, "This should never be run on the client...");

   SimShapeGroup* pCurrGroup = dynamic_cast<SimShapeGroup*>(getGroup());
   AssertFatal(pCurrGroup != NULL, "Not initing in a shape group!");
   
   for (SimSet::iterator itr = pCurrGroup->begin();
        itr != pCurrGroup->end(); itr++) {
      _addObject(*itr);
   }
}

void SimShapeGroupRep::_addObject(SimObject* io_pAdd)
{
   AssertFatal(isGhost() == false, "This should never be run on the client...");

   if ((io_pAdd->getType() & SimShapeObjectType) == 0) {
      // We don't have to worry about adding ourselves, since we aren't a
      //  ShapeObject
      // We don't track these...
      return;
   }
   
   SimShape* pShape = static_cast<SimShape*>(io_pAdd);
   if (find(m_objectList.begin(), m_objectList.end(), pShape) == m_objectList.end()) {
      m_objectList.push_back(pShape);
      pShape->setDamageParent(this);
      setMaskBits(ObjectsChangedMask);
   }
}

void SimShapeGroupRep::_removeObject(SimObject* io_pRemove)
{
   AssertFatal(isGhost() == false, "This should never be run on the client...");
   if ((io_pRemove->getType() & SimShapeObjectType) == 0) {
      // We don't track these...
      return;
   }

   SimShapeList::iterator itr = find(m_objectList.begin(), m_objectList.end(),
                                     static_cast<SimShape*>(io_pRemove));
   if (itr != m_objectList.end()) {
      (*itr)->clearDamageParent();
      m_objectList.erase(itr);
      setMaskBits(ObjectsChangedMask);
   }
}


//------------------------------------------------------------------------------
//--------------------------------------
// Persistent Functionality
//--------------------------------------
//
DWORD SimShapeGroupRep::packUpdate(Net::GhostManager* gm,
                             DWORD              mask,
                             BitStream*         stream)
{
   if (mask & InitialUpdate) {
      // We don't care for now...
      //
   }

   if (mask & ObjectsChangedMask) {
      // Make sure that we wait for the ghosts to be sent and confirmed...
      //
      int i;
      for (i = 0; i < m_objectList.size(); i++) {
         SimShape* pShape = m_objectList[i];
         int ghostId = gm->getGhostIndex(pShape);
         if (ghostId == -1) {
            stream->writeFlag(false);
            return ObjectsChangedMask;
         }
      }

      stream->writeFlag(true);
      stream->write(m_objectList.size());
      for (i = 0; i < m_objectList.size(); i++) {
         SimShape* pShape = m_objectList[i];
         
         int ghostId = gm->getGhostIndex(pShape);
         stream->write(ghostId);
      }
   } else {
      stream->writeFlag(false);
   }
   
   return 0;
}

void
SimShapeGroupRep::unpackUpdate(Net::GhostManager* /*gm*/,
                               BitStream*         stream)
{
   AssertFatal(isGhost() == true, "Error, server GroupRep received unpack update?");
   
   if (stream->readFlag() == true) {
      // Clear out our objectList, a new one is coming in...
      removeAll();
      
      if (m_isInTimerSet == true)
         removeFromTimerSet();

      int numObjects;
      stream->read(&numObjects);
      for (int i = 0; i < numObjects; i++) {
         int        ghostId;
         
         stream->read(&ghostId);
         m_unresolvedGhostIds.push_back(ghostId);
      }

      if (manager && numObjects != 0) {
         processUnresolvedGhosts();
      }
   }
}

Persistent::Base::Error
SimShapeGroupRep::read(StreamIO& sio,
                       int       version,
                       int       user)
{
   AssertFatal(version == sm_fileVersion, "Wrong version: SimShapeGroupRep");

   if (Parent::read(sio, version, user) != Ok)
      return ReadError;

   sio.read(&m_globallyConserveDamage);

   return ((sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

Persistent::Base::Error
SimShapeGroupRep::write(StreamIO& sio,
                        int       version,
                        int       user)
{
   if (Parent::write(sio, version, user) != Ok)
      return WriteError;

   sio.write(m_globallyConserveDamage);

   return ((sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

int
SimShapeGroupRep::version()
{
   return sm_fileVersion;
}

