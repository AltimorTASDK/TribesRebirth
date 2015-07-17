//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#ifndef _SIMBASE_H_
#define _SIMBASE_H_

#include <tvector.h>
#include <tAlgorithm.h>
#include <bitset.h>

#include <persist.h>
#include <simEvDcl.h>
#include <simObjectTypes.h>
#include <simDictionary.h>

#define MANAGER_ID_SHIFT   (24)
#define MANAGER_ID_MASK    (0xffffffff << MANAGER_ID_SHIFT)

//--------------------------------------------------------------------------- 

class SimEvent;
class SimObject;
class SimGroup;
class SimManager;
class SimPreLoadManager;

typedef float SimTime;

inline void destroy(SimObject**) {}
inline void destroy(SimObject**,SimObject**) {}
inline void destroy(SimEvent**) {}
inline void destroy(SimEvent**,SimEvent**) {}

class DLLAPI SimObjectList: public VectorPtr<SimObject*>
{
   static int _USERENTRY compareObjectNumber(const void* a,const void* b);
public:
   void pushBack(SimObject*);
   void pushFront(SimObject*);
   void remove(SimObject*);
   void removeStable(SimObject*);
   void sortObjectNumber();
};

class DLLAPI SimEventQueue: public VectorPtr<SimEvent*>
{
   // The event queue owns the current time because
   // it has to be maintained in the critical section
   // along with the queue management.
   SimTime currentTime;

private:
   HANDLE hQueueEvent;
   CRITICAL_SECTION csQueue;
   void lock()    { EnterCriticalSection( &csQueue ); }
   void unlock()  { LeaveCriticalSection( &csQueue ); }
public:
   SimEventQueue();
   ~SimEventQueue();

   void signal();
   HANDLE getEventHandle() { return ( hQueueEvent );  }
   iterator insert(SimEvent* event,bool currentEvent);
   void remove(SimObject* obj);
   SimEvent* pop(SimTime);
   void setCurrentTime(SimTime);
   SimTime getCurrentTime();
   void subtractTime(SimTime);
};

inline SimTime SimEventQueue::getCurrentTime()
{
   return currentTime;
}



//------------------------------------------------------------------------------
class DLLAPI SimIdBlockManager
{
private:
   // idBlocks are numbered 0-n but represent SimObjectId's in the range 
   // block << SIM_ID_BLOCK_SHIFT >= id < (block+1) << SIM_ID_BLOCK_SHIFT

   struct Entry
   {
      SimIdBlock idBlock;
      SimObject  *obj;      
   };
   Vector<Entry> allocated;
   Vector<SimIdBlock> pool;
   SimIdBlock nextIdBlock;

   void reclaim();

public:
   SimIdBlockManager();

   SimIdBlock alloc(SimObject *obj);
   void free(SimIdBlock block);              // free a particular id block
   void free(SimObject *obj);                // free all id blocks accociated with a SimObject
   SimObject* findBlock(SimIdBlock block);   // find SimObject owning an id block
   SimObject* findId(SimObjectId id);        // find SimObject owning id 
};


inline SimObject* SimIdBlockManager::findId(SimObjectId id)
{
   return ( findBlock(id >> SIM_ID_BLOCK_SHIFT) );   
}   


//------------------------------------------------------------------------------
class DLLAPI SimIdGenerator
{
private:
   SimObjectId idBlockBase; 
   SimObjectId idBlockLimit;
   Vector<SimObjectId> pool;
   SimObjectId nextId;
   void reclaim();

public:
   SimIdGenerator();
   void reset();
   void addBlock(SimIdBlock block);

   bool inRange(SimObjectId id);
   SimObjectId alloc();
   void free(SimObjectId id);
   int  numIdsUsed();
};

inline bool SimIdGenerator::inRange(SimObjectId id)
{
   return (id >= idBlockBase && id <idBlockLimit);   
}   



inline bool isseperator(char a)
{
   return a == '\\' || a == '/';
}


//---------------------------------------------------------------------------
struct SimAddress
{
   SimObject *pObject;
   SimObjectId objectId;
   SimObjectId managerId;

   // ManagerId = 0 defaults to the local manager
   // objectId  = 0 is the default id for the local manager
   // objectId  = -1 for invalid id.

   SimAddress()               { pObject = NULL; objectId=-1; managerId=0; }
   SimAddress(SimObject* obj) { set(obj); }
   SimAddress(SimObjectId objectId, SimObjectId managerId = 0);
   void set(SimObjectId objectId, SimObjectId managerId = 0);
   void set(SimObject* obj);

   SimObject* resolve(SimManager*) const;
};

inline void SimAddress::set(SimObject* obj)
{
   // Addresses with pointers default to the current manager
   pObject = obj;
   objectId = -1;
   managerId = 0;
}

inline SimAddress::SimAddress(SimObjectId objectId, SimObjectId managerId) 
{ 
   set(objectId, managerId);  
}

inline void SimAddress::set(SimObjectId _objectId, SimObjectId _managerId)  
{ 
   pObject   = NULL; 
   objectId  = _objectId; 
   managerId = _managerId; 
}


namespace Net
{
class PacketStream;
};
class BitStream;

//---------------------------------------------------------------------------
class DLLAPI SimEvent : public Persistent::Base
{
public:
   enum Flag {
      Guaranteed =      BIT(0),
      Priority =        BIT(1),
      Remote =          BIT(2),
      Ordered =         BIT(3),
      ToGhost =         BIT(4),
      ToGhostParent =   BIT(5),
   };
   int type;
   BitSet32 flags;
   SimTime time;
   SimAddress address;
   int sourceManagerId;
   bool verifyNotServer(SimManager *manager); // this will set an error and return false
                           // if an event is unpacked on the server and this is called

   virtual void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   virtual void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);

   virtual void dump();
   virtual ~SimEvent() {}  // dummy virtual destructor is required
                           // so that subclasses can be deleted properly
};

struct SimQuery
{
   int type;
};

// Event & query delivery
#define onEvent(T) \
   case T##Type: return on##T (static_cast<const T *> (event));
#define onQuery(T) \
   case T##Type: return on##T (static_cast<T *> (query));


class Inspect;

//--------------------------------------------------------------------------- 
class DLLAPI SimObject: public Persistent::VersionedBase
{
   friend SimManager;
   friend SimGroup;
   friend SimNameDictionary;
   friend SimIdDictionary;

   // dictionary information stored on the object
   StringTableEntry objectName;
   SimObject *nextNameObject;
   SimObject *nextIdObject;

   enum {
      Deleted = BIT(0),
      Removed = BIT(1),
      DynamicId = BIT(2),
      Added = BIT(3),
      Selected = BIT(4),
      Expanded = BIT(5),
   };
   BitSet32 flags;

   SimObjectList deleteNotifyList;
   SimObjectList clearNotifyList;
   int objectNumber;                     // Nth object added to manager

public:
   struct CMDFieldEntry
   {
      StringTableEntry slotName;
      char *value;
      CMDFieldEntry *nextEntry;
   };
private:
   CMDFieldEntry *fieldList;

protected:
   SimObjectId id;
   SimGroup* group;
   SimManager* manager;
   BitSet32 type;

public:

   CMDFieldEntry *getDynamicFieldList() { return fieldList; }
   virtual void setFieldValue(StringTableEntry slotName, const char *value);
   const char *getFieldValue(StringTableEntry slotName);

   SimObject();
   virtual ~SimObject();
   virtual bool processEvent(const SimEvent*);
   virtual bool processQuery(SimQuery*);
   virtual bool processArguments(int argc, const char **argv);

   virtual void inspectRead(Inspect *inspector);
   virtual void inspectWrite(Inspect *inspector);

   virtual bool onAdd();
   virtual void onRemove();
   virtual void onGroupAdd();
   virtual void onGroupRemove();
   virtual void onNameChange(const char *name);

   virtual void onDeleteNotify(SimObject *object);
   virtual void onPreLoad(SimPreLoadManager *splm) {} 

   void deleteNotify(SimObject* obj);
   void clearNotify(SimObject* obj);
   void clearAllNotifications();

   void deleteObject();
   int getNumber() const { return objectNumber; }
   SimObjectId getId() const { return id; }
   BitSet32 getType() const  { return type; }
   const char* getName() const { return objectName; };

   void setId(SimObjectId id);
   void assignName(const char* name);
   SimGroup* getGroup() const { return group; }
   SimManager* getManager() const { return manager; }
   bool isProperlyAdded() const { return flags.test(Added); }
   bool isDeleted() const { return flags.test(Deleted); }
   bool isRemoved() const { return flags.test(Deleted | Removed); }
   bool isDynamicId() const { return flags.test(DynamicId); }
   bool isLocked();
   void setLocked( bool b );
   
   bool addToSet(SimObjectId);
   bool addToSet(const char *);
   bool removeFromSet(SimObjectId);
   bool removeFromSet(const char *);

   bool isSelected() const { return flags.test(Selected); }
   bool isExpanded() const { return flags.test(Expanded); }
   void setSelected(bool sel) { flags.set(Selected, sel); }
   void setExpanded(bool exp) { flags.set(Expanded, exp); }

   virtual void postEvent(SimEvent* event,bool currentEvent = false);
   virtual void postCurrentEvent(SimEvent* event);
   // Persistent functions

   Persistent::Base::Error read(StreamIO &, int version, int user);
   Persistent::Base::Error write(StreamIO &, int version, int user); 
   virtual void dump();
};


//--------------------------------------------------------------------------- 
class DLLAPI SimSet: public SimObject
{
   typedef SimObject Parent;
protected:
   SimObjectList objectList;
   bool ownObjects;
   bool stable;
   
public:
   typedef SimObjectList::iterator iterator;
   typedef SimObjectList::value_type value;
   SimObject* front() { return objectList.front(); }
   bool       empty() { return objectList.empty();   }
   int        size()  { return objectList.size(); }
   iterator   begin() { return objectList.begin(); }
   iterator   end()   { return objectList.end(); }
   value operator[] (int index) { return objectList[index]; }

   iterator find( iterator first, iterator last, SimObject *obj)
      { return ::find(first, last, obj); }

   bool reOrder( SimObject *obj, SimObject *target=0 );

   SimSet(bool own = false);
   ~SimSet();
   virtual bool processEvent(const SimEvent*);
   virtual bool processArguments(int argc, const char **argv);

   virtual void inspectRead(Inspect *inspector);
   virtual void inspectWrite(Inspect *inspector);

   virtual bool onAdd();
   virtual void onRemove();
   virtual void onPreLoad(SimPreLoadManager *splm);

   virtual void onDeleteNotify(SimObject *object);

   virtual SimObject* addObject(SimObject*);
   virtual void removeObject(SimObject*);

   void setStable(bool _stable) { stable = _stable; }
   void setOwnObjects(bool own) { ownObjects = own; }
   DECLARE_PERSISTENT(SimSet);
   virtual Persistent::Base::Error read(StreamIO &, int version, int user);
   virtual Persistent::Base::Error write(StreamIO &, int version, int user);
};



//--------------------------------------------------------------------------- 
class DLLAPI SimGroup: public SimSet
{
private:
   friend SimManager;
   typedef SimSet Parent;
   SimNameDictionary dictionary;

public:
   SimGroup();
   ~SimGroup();

   SimObject* addObject(SimObject*);
   SimObject* addObject(SimObject*, SimObjectId);
   SimObject* addObject(SimObject*, const char *name);
   void removeObject(SimObject*);
   void onRemove();

   virtual void assignName(SimObject*,const char* name);
   virtual SimObject* findObject(SimObjectId);
   virtual SimObject* findObject(const char* name);

   bool processArguments(int argc, const char **argv);

   DECLARE_PERSISTENT(SimGroup);
   virtual Persistent::Base::Error read(StreamIO &, int version, int user);
   virtual Persistent::Base::Error write(StreamIO &, int version, int user);
};


inline SimObject* SimGroup::addObject(SimObject *obj, SimObjectId id)
{
   // AddObject will assign it whatever id it already has.
   // This should normally be done only with reserved id's.
   obj->id = id;
   addObject( obj );
   return obj;
}

inline SimObject* SimGroup::addObject(SimObject *obj, const char *name)
{
   addObject( obj );
   assignName(obj, name);   
   return obj;
}   


//--------------------------------------------------------------------------- 
class DLLAPI SimManager: public SimGroup
{
private:
   typedef SimGroup Parent;
   SimTime targetTime;

   SimEventQueue eventQueue;
   SimObjectList deleteList;
   SimIdDictionary dictionary;

   CRITICAL_SECTION cs_managerLock;

   int serverType;
   void clearDeleteList();
   int advanceQueue();

public:
   SimIdBlockManager idManager;
   SimIdGenerator idGenerator;

   SimManager();
   ~SimManager();

   void lock();
   void unlock();

   using SimGroup::addObject;
   SimObject* addObject(SimObject*);
   bool registerObject(SimObject*);
   void unregisterObject(SimObject*);
   void deleteObject(SimObject*);
   SimObjectId assignId(SimObject*,SimObjectId = 0);
   void assignName(SimObject*,const char* name);

   //using SimGroup::findObject; // XXX ?? humm
   SimObject* findObject(SimObjectId);
   SimObject* findObject(const char* name);

   int advanceToTime(SimTime time);
   void resetTime();
   SimTime getCurrentTime();
   SimTime getTargetTime();
   HANDLE getQueueEventHandle();
   void   signalQueue();

   bool processEvent(const SimEvent*);
   void postEvent(SimEvent*,bool currentEvent = false);
   void postEvent(const char* name,SimEvent*,bool currentEvent = false);
   void postEvent(SimObject*,SimEvent*);
   void postEvent(SimObjectId,SimEvent*);

   void postCurrentEvent(SimEvent*);
   void postCurrentEvent(SimObject*,SimEvent*);
   void postCurrentEvent(SimObjectId,SimEvent*);

   bool isServer();
   void preLoad();
};


inline SimObject* SimManager::findObject(const char* name)
{
   return Parent::findObject( name );
}   

inline void SimManager::postEvent(SimObject* obj,SimEvent* event)
{
   event->address.set(obj);
   postEvent(event,false);
}

inline void SimManager::postEvent(SimObjectId id,SimEvent* event)
{
   event->address.set(id);
   postEvent(event,false);
}

inline void SimManager::postCurrentEvent(SimEvent* event)
{
   // Current time now set in the eventQueue.
   postEvent(event,true);
}

inline void SimManager::postCurrentEvent(SimObject* obj,SimEvent* event)
{
   // Current time now set in the eventQueue.
   event->address.set(obj);
   postEvent(event,true);
}

inline void SimManager::postCurrentEvent(SimObjectId id,SimEvent* event)
{
   // Current time now set in the eventQueue.
   event->address.set(id);
   postEvent(event,true);
}

inline SimTime SimManager::getCurrentTime()
{
   return eventQueue.getCurrentTime();
}

inline SimTime SimManager::getTargetTime()
{
   return targetTime;
}

inline HANDLE SimManager::getQueueEventHandle() 
{ 
   return eventQueue.getEventHandle();  
}

inline void SimManager::signalQueue()
{
   eventQueue.signal();
}   




//--------------------------------------------------------------------------- 

template<class T> DLLAPI
T* findObject(SimManager* manager,SimObjectId id,T*)
{
   SimObject* obj = manager->findObject(id);
   return (obj)? dynamic_cast<T*>(obj): 0;
}


//--------------------------------------------------------------------------- 

inline void SimObject::postEvent(SimEvent* event, bool currentEvent)
{
   currentEvent;
   manager->postEvent(this,event);
}

inline void SimObject::postCurrentEvent(SimEvent* event)
{
   manager->postCurrentEvent(this,event);
}

inline void SimObject::setId(SimObjectId newId)
{
   if (manager)
      manager->assignId(this,newId);
   else
      id = newId;
}   

inline SimObject* SimAddress::resolve(SimManager* _manager) const
{
   return pObject? pObject: _manager->findObject(objectId);
}   


#endif
