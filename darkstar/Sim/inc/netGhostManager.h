//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#ifndef _NETGHOSTMANAGER_H_
#define _NETGHOSTMANAGER_H_

#include "simbase.h"
#include "simNetObject.h"
#include "m_sphere.h"
#include "m_box.h"
#include "bitset.h"
#include "netPacketStream.h"
#include "simEv.h"
#include "tsorted.h"

//-----------------------------------------------------------------------------

namespace Net { class GhostManager; };

class NetGhostAlwaysDoneEvent : public SimEvent
{
public:
   Net::GhostManager *ghostManager;
   NetGhostAlwaysDoneEvent(Net::GhostManager *in_gm) 
      { type = NetGhostAlwaysDoneEventType; ghostManager = in_gm; }
};

namespace Net
{

class RemoteCreateEvent : public SimEvent
{
public:
   SimNetObject *newRemoteObject;
   
   DECLARE_PERSISTENT(RemoteCreateEvent);

   RemoteCreateEvent();
   ~RemoteCreateEvent();

   void pack(SimManager *manager, PacketStream *ps, BitStream *bs);
   void unpack(SimManager *manager, PacketStream *ps, BitStream *bs);
};


struct GhostInfo;
struct GhostRef;

class DLLAPI GhostManager : public PacketStreamClient
{
   typedef PacketStreamClient Parent;
public:
   enum { MaxGhostCount = 1024 };

   struct PacketObjectRef
   {
      DWORD mask;
      DWORD ghostInfoFlags;
      GhostInfo *ghost;
      PacketObjectRef *nextRef;
      PacketObjectRef *nextUpdateChain;
   };

   class UpdateQueueEntry
   {
   public:
      float priority;
      GhostInfo *obj;

      UpdateQueueEntry(float in_priority, GhostInfo *in_obj) 
         { priority = in_priority; obj = in_obj; }
	   int operator<(const UpdateQueueEntry &qe) const;
   };

   class AddListEntry
   {
   public:
      int objectNumber;
      SimNetObject *obj;
      AddListEntry(int on, SimNetObject *in_obj)
         { obj = in_obj; objectNumber = on; }
      inline int operator<(const AddListEntry &ale) const
         { return objectNumber < ale.objectNumber; }
   };

   enum GhostManagerMode
   {
      InactiveMode,
      GhostScopeAlwaysMode,
      DoneGhostScopeAlwaysMode,
      GhostNormalMode,
   };

private:

   SortableVector<UpdateQueueEntry> priorityQueue;
   SortableVector<AddListEntry> scopeAlwaysList;

   GhostInfo *ghostRefs;
   GhostInfo *ghostList;
   GhostInfo *refFreeList;
   SimNetObject *scopeObject;
   SimNetObject::CameraInfo camInfo;
   PacketObjectRef *freePackRefList;

   int sendIdSize; // in bits
   bool allowGhosts;

   GhostManagerMode curMode;

   SimGroup *cleanupGroup;

	// Event handlers
	bool onAdd();
	void onRemove();
	void onDeleteNotify(SimObject *del);
   void createGhost(SimNetObject *obj);
   void removeGhostRef(SimNetObject *no);

   void freePacketObjectRef(PacketObjectRef *);
   void flushScopeAlwaysObjects();
public:
   GhostManager(bool allowG = true);
   ~GhostManager();
   void displayFreeListStats();
	static GhostManager* get(SimManager*);
   void setScopeObject(SimNetObject *obj);
   SimNetObject * getScopeObject() { return scopeObject; }

   GhostManagerMode getCurrentMode() { return curMode; }

	// SimSet virtuals
   void packetReceived(DWORD key);
   void GhostManager::packetDropped(DWORD key);
   void freeGhostInfo(GhostInfo *ghost);
   void freeGhost(DWORD ghostIndex);
   bool writePacket(BitStream *bstream, DWORD &key);
   void readPacket(BitStream *bstream, DWORD time);
   void objectInScope(SimNetObject *obj);

   SimNetObject *resolveGhost(int id);
   SimNetObject *resolveGhostParent(int id);
   int getGhostIndex(SimNetObject *obj);

   void reset();
   void activate();
};

struct GhostInfo
{
   SimNetObject *localGhost; // local ghost for remote object
	DWORD updateMask;       // 32 bits of object information - object owns these
	SimNetObject *obj;	   // the 'real' object.
   GhostManager::PacketObjectRef *updateChain;
	DWORD flags;			   // static, fluff, etc.
   GhostInfo *nextRef;
   GhostInfo *prevRef;
   UInt16 ghostIndex;
   UInt16 updateSkipCount;

	enum Flags 
	{
		Valid = BIT(0),
		InScope = BIT(1),
      ScopeAlways = BIT(2),
      NotYetGhosted = BIT(3),
      Ghosting = BIT(4),
      KillGhost = BIT(5),
      KillingGhost = BIT(6),
      LastGhostAlwaysPacket = BIT(7),
	};
};

//-----------------------------------------------------------------------------

inline GhostManager* GhostManager::get(SimManager* manager)
{
	return static_cast<GhostManager*>(manager->findObject(GhostManagerId));
}

void        setLastError(const char* in_pErrorString);
const char* getLastError();

} // namespace Net

#endif
