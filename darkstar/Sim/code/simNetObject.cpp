//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

#include "simNetObject.h"
#include "inspect.h"
#include "editor.strings.h"
#include "netGhostManager.h"
#include "simSetIterator.h"

//----------------------------------------------------------------------------

SimNetObject::SimNetObject()
{
	// netFlags will clear itself to 0
   owner = NULL;
	netIndex = DWORD(-1);
}


//-----------------------------------------------------------------------------

void SimNetObject::setMaskBits(DWORD orMask)
{
   if(isDeleted())
      return;

   Vector<Net::GhostRef>::iterator i;
   for(i = ghosts.begin(); i != ghosts.end(); i++)
      i->ghostInfo->updateMask |= orMask;
}

bool SimNetObject::onAdd()
{
   if(netFlags.test(ScopeAlways) && netFlags.test(Ghostable) && !netFlags.test(IsGhost))
   {
      // if it's a ghost always object, add it to the ghost always set
      // for ClientReps created later.

      addToSet(SimGhostAlwaysSetId);

      // add it to all ClientReps that already exist.

      SimSet *ghostManagerSet = (SimSet *) manager->findObject(GhostManagerSetId);
      SimSet::iterator i;
      for(i = ghostManagerSet->begin(); i != ghostManagerSet->end(); i++)
      {
         ((Net::GhostManager *) (*i))->objectInScope(this);
      }
   }
   return Parent::onAdd();
}

int SimNetObject::getGhostedIndex(Net::GhostManager *gm)
{
   Vector<Net::GhostRef>::iterator i;
   for(i = ghosts.begin(); i != ghosts.end(); i++)
   {
      if(i->ghostManager == gm)
      {
         if(i->ghostInfo->flags & (Net::GhostInfo::NotYetGhosted | Net::GhostInfo::Ghosting | Net::GhostInfo::KillingGhost))
            return INVALID_GHOST_INDEX;
         return i->ghostInfo->ghostIndex;
      }
   }
   return INVALID_GHOST_INDEX;            
}

void SimNetObject::onRemove()
{
   Vector<Net::GhostRef>::iterator i;
   
   for(i = ghosts.begin(); i != ghosts.end(); i++)
   {
      // set this object up to be deleted by its ghost manager...
      i->ghostInfo->flags |= Net::GhostInfo::KillGhost;
      i->ghostInfo->obj = NULL;
   }

   Parent::onRemove();
}

void SimNetObject::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);

   bool ghost;
   insp->read(IDITG_WILL_GHOST, ghost);

   if(ghost) 
      netFlags.set(Ghostable);
   else 
      netFlags.clear(Ghostable);
}

void SimNetObject::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_WILL_GHOST, bool(netFlags.test(Ghostable)));
}
//-----------------------------------------------------------------------------

float SimNetObject::getUpdatePriority(CameraInfo *focusObject, DWORD updateMask, int updateSkips)
{
   focusObject;
   updateMask;
   updateSkips;

   return 0;
}

DWORD SimNetObject::getGhostTag()
{
   return getPersistTag();
}

DWORD SimNetObject::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
   gm;
   mask;
   stream;
   return 0;
}

void SimNetObject::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
   gm;
   stream;
}

void SimNetObject::buildScopeAndCameraInfo(Net::GhostManager *cr, CameraInfo* /*camInfo*/)
{
   // default behavior -
   // ghost everything that is ghostable
   
   for ( SimSetIterator obj(manager); *obj; ++obj)
   {
		SimNetObject* nobj = dynamic_cast<SimNetObject*>(*obj);
		if (nobj)
		{
			// Some objects don't ever want to be ghosted
			if (!nobj->netFlags.test(SimNetObject::Ghostable))
				continue;
         if (!nobj->netFlags.test(SimNetObject::ScopeAlways))
         {
            // it's in scope...
            cr->objectInScope(nobj);
         }
      }
   }
}

//-----------------------------------------------------------------------------

void SimNetObject::initPersistFields()
{
   Parent::initPersistFields();
   addField("netFlags", TypeInt, Offset(netFlags, SimNetObject));
}

//-----------------------------------------------------------------------------

Persistent::Base::Error SimNetObject::read(StreamIO &s,int,int)
{
   s.read( &id );

   SimAddress tempAddress;

   // This is probably temporary, when switching to the new networking
   // system we like to set the ScopeAlways flag in the contructor of
   // the object.  Older objects that are persist-read tend to clobber
   // this flag and therefore don't get ghosted.  We or the flags from
   // the stream with the existing flags to get around this problem.
   BitSet32 nf;
   s.read( sizeof( netFlags ), (void*)&nf);
   netFlags = netFlags.mask() | nf.mask();

   // set the parent's locked flag according to this' locked flag state
   Parent::setLocked( netFlags.test( SimNetObject::Locked ) );
   
   s.read( sizeof( tempAddress ), (void*)&tempAddress );
	return (s.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error SimNetObject::write(StreamIO &s,int,int)
{
   SimAddress tempAddress;

	s.write(isDynamicId()? 0: id);
   
   // check for the locked flag on the parent...
   netFlags.set( SimNetObject::Locked, Parent::isLocked() );
   
   s.write( sizeof( netFlags ), (void*)&netFlags );
   s.write( sizeof( tempAddress ), (void*)&tempAddress);
	return (s.getStatus() == STRM_OK)? Ok: WriteError;
}

