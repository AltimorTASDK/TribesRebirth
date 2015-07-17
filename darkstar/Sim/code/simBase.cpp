//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 
#include <base.h>

#pragma warn -aus

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma option -Jg
#endif

#include <simBase.h>
#include "stringTable.h"
#include <simev.h>
#include "SimPersman.h"
#include "inspect.h"
#include "editor.strings.h"
// for isServer() only...
#include "simGame.h"
#include "m_qsort.h"
#include "simEvDcl.h"
#include "simConsoleEvent.h"
#include "simPreLoadManager.h"

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	SimEvent 
//  Persistent Functions
//---------------------------------------------------------------------------

void SimEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   manager;
   ps;

   // all we have in default pack is id packing information

   if(flags.test(ToGhostParent))
   {
      bstream->writeFlag(true);
      bstream->writeInt(address.objectId, 10);
   }
   else 
   {
      bstream->writeFlag(false);
      if(flags.test(ToGhost))
      {
         bstream->writeFlag(true);
         bstream->writeInt(address.objectId, 10);
      }
      else
      {
         bstream->writeFlag(false);
         bstream->writeInt(address.objectId, 32);
      }
   }
}

//---------------------------------------------------------------------------

void SimEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   manager;
   ps;

   if(bstream->readFlag())
   {
      flags.set(ToGhostParent);
      address.objectId = bstream->readInt(10);
   }
   else
   {
      if(bstream->readFlag())
      {
         flags.set(ToGhost);
         address.objectId = bstream->readInt(10);
      }
      else
         address.objectId = bstream->readInt(32);
   }
}

//---------------------------------------------------------------------------

void SimEvent::dump()
{
	printf("type: %08x\n", type );
	printf("time: %08f\n", float(time));
}

//---------------------------------------------------------------------------

//--------------------------------------------------------------------------- 

SimEventQueue::SimEventQueue()
{
   //create an auto-reset event initially NOT signaled
	currentTime = 0.0f;
   hQueueEvent = CreateEvent( NULL, false, false, NULL );
   InitializeCriticalSection ( &csQueue );
}

SimEventQueue::~SimEventQueue()
{
	// Delete all pending events
	for (iterator ptr = begin(); ptr != end(); ptr++)
		delete *ptr;
   CloseHandle( hQueueEvent );
   DeleteCriticalSection( &csQueue );
}

int _USERENTRY compare(const SimEvent *a,const SimEvent *b)
{
	// For sorting in event queue
	return a->time < b->time;
}

void SimEventQueue::subtractTime(SimTime t)
{
   t;
   iterator fst = begin();
   if(size() > 1)
   {
      for(iterator itr = fst+1; itr != end(); itr++)
         (*itr)->time -= (*fst)->time;
   }
   if(size() != 0)
      (*fst)->time = 0;
   currentTime = 0;
}

void SimEventQueue::setCurrentTime(SimTime ctime)
{
	lock();
	// Don't want to be changing time while pushing
	// events on the queue.
	currentTime = ctime;
	unlock();
}

SimEventQueue::iterator SimEventQueue::insert(SimEvent* event,bool currentEvent)
{
   lock();
	// Assign the event the current time, this has to be
	// inside the lock.
	if (currentEvent)
		event->time = currentTime;

	// Find insertion point using binary search.
   int len = end() - begin();
   int half;
   iterator first = begin();
   iterator middle;

   while (len > 0) {
     half = len / 2;
     middle = first + half;
     if (event->time < (*middle)->time)
         len = half;
     else {
			first = middle + 1;
			len = len - half - 1;
     }
   }

	// Ensure stable insert
	while (first != end() && (*first)->time <= event->time)
		first++;
   VectorPtr<SimEvent*>::insert( first, event);
   unlock();
   return (first);
}

void SimEventQueue::signal()
{
   SetEvent( hQueueEvent );
}   

void SimEventQueue::remove(SimObject* obj)
{
	// Remove all pending events
   lock();
	SimObjectId id = obj->getId();
	for (iterator ptr = begin(); ptr != end(); ++ptr)
		if ((*ptr)->address.pObject) {
			if ((*ptr)->address.pObject == obj)
			{
            delete *ptr;
				erase(ptr--);
			}
		}
		else
			if ((*ptr)->address.objectId == id)
			{
            delete *ptr;
				erase(ptr--);
			}
   unlock();
}

SimEvent* SimEventQueue::pop(SimTime targetTime)
{
	lock();
	if (begin() != end() && (*begin())->time <= targetTime) 
	{
		SimEvent* ptr = *begin();
		erase(begin());

		// Current time must be updated within the lock
		AssertFatal(ptr->time >= currentTime,
			"SimEventQueue::pop: Cannot go back in time.");
		if (ptr->time > currentTime)
			currentTime = ptr->time;
		unlock();
		return ptr;
	}
	currentTime = targetTime;
	unlock();
	return 0;
}


//--------------------------------------------------------------------------- 
//------------------------------------------------------------------------------
SimIdBlockManager::SimIdBlockManager()
{
   nextIdBlock = 0;   
}   


//------------------------------------------------------------------------------
void SimIdBlockManager::reclaim()
{
   // attempt to keep the pool vector as small as possible by reclaiming 
   // pool entries back into the nextIdBlock variable
   while (!pool.empty() && (pool.last() == (nextIdBlock-1)) )
   {
      nextIdBlock--;
      pool.pop_back();
   }   
}   


//------------------------------------------------------------------------------
SimIdBlock SimIdBlockManager::alloc(SimObject *obj)
{
   Entry entry;
   entry.obj = obj;

   if (!pool.empty())   
   {  // can we get a block from the pool list?
      entry.idBlock = pool.last();
      pool.pop_back();
      reclaim();      
   }
   else
   {  // gotta alloc a new idBlock
      entry.idBlock = nextIdBlock;   
      nextIdBlock++;
   }
   allocated.push_back( entry );

   return (entry.idBlock);
}   


//------------------------------------------------------------------------------
void SimIdBlockManager::free(SimIdBlock block)
{
   Vector<Entry>::iterator i = allocated.begin();
   for (; i != allocated.end(); i++)
      if ( i->idBlock == block )
      {
         allocated.erase( i );
         pool.push_back( block );
         reclaim();
         return;
      }
   AssertFatal(0, "SimIdBlockManager::free: attempt made to free unallocated idBlock.");
}   


//------------------------------------------------------------------------------
void SimIdBlockManager::free(SimObject *obj)
{
   Vector<Entry>::iterator i = allocated.begin();
   for (; i != allocated.end(); )
   {
      if ( i->obj == obj )
      {
         pool.push_back( i->idBlock );
         allocated.erase( i );
         reclaim();
      }
      else
         i++;
   }
}   


//------------------------------------------------------------------------------
SimObject* SimIdBlockManager::findBlock(SimIdBlock block)
{
   Vector<Entry>::iterator i = allocated.begin();
   for (; i != allocated.end(); i++)
      if ( i->idBlock == block )
         return (i->obj);
   AssertFatal(0, "SimIdBlockManager::find: idBlock not allocated.");
   return (NULL);
}   


//--------------------------------------------------------------------------- 
//------------------------------------------------------------------------------
SimIdGenerator::SimIdGenerator()
{
   reset();
}   


//------------------------------------------------------------------------------
void SimIdGenerator::reset()
{
   idBlockBase = 0;
   idBlockLimit= 0;
   nextId = 0;
   pool.clear();
}   


//------------------------------------------------------------------------------
void SimIdGenerator::reclaim()
{
   // attempt to keep the pool vector as small as possible by reclaiming 
   // pool entries back into the nextIdBlock variable
   while (!pool.empty() && (pool.last() == (nextId-1)) )
   {
      nextId--;
      pool.pop_back();
   }   
}   


//------------------------------------------------------------------------------
void SimIdGenerator::addBlock(SimIdBlock block)
{
   block <<= SIM_ID_BLOCK_SHIFT;
   idBlockBase = block;
   idBlockLimit= block + (1 << SIM_ID_BLOCK_SHIFT);
   nextId      = block;
}   


//------------------------------------------------------------------------------
SimObjectId SimIdGenerator::alloc()
{
   AssertFatal(idBlockLimit != 0, "SimIdGenerator::alloc: must add a SimIdBlock first.");

   SimObjectId id;
   if (!pool.empty())   
   {  // can we get an id from the pool list?
      id = pool.last();
      pool.pop_back();
      reclaim();      
   }
   else
   {  // gotta alloc a new id
      AssertFatal(nextId < idBlockLimit, "SimIdGenerator::alloc: all id's have been allocated, add idBlock.")
      id = nextId;   
      nextId++;
   }
   return (id);
}   


//------------------------------------------------------------------------------
void SimIdGenerator::free(SimObjectId id)
{
   AssertFatal(id >= idBlockBase && id < idBlockLimit, "SimIdGenerator::alloc: invalid id, id does not belong to this IdGenerator.")
   pool.push_back( id );
   reclaim();   
}


//------------------------------------------------------------------------------
int SimIdGenerator::numIdsUsed()
{
   return (nextId-idBlockBase-pool.size());
}   



//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

void SimObjectList::pushBack(SimObject* obj)
{
	if (find(begin(),end(),obj) == end())
		push_back(obj);
}	

void SimObjectList::pushFront(SimObject* obj)
{
	if (find(begin(),end(),obj) == end())
		push_front(obj);
}	

void SimObjectList::remove(SimObject* obj)
{
	iterator ptr = find(begin(),end(),obj);
	if (ptr != end()) {
		iterator last = end() - 1;
		*ptr = *last;
		erase(last);
	}
}

void SimObjectList::removeStable(SimObject* obj)
{
	iterator ptr = find(begin(),end(),obj);
	if (ptr != end())
		erase(ptr);
}

int _USERENTRY SimObjectList::compareObjectNumber(const void* a,const void* b)
{
	return (*reinterpret_cast<const SimObject* const*>(a))->getNumber() -
		(*reinterpret_cast<const SimObject* const*>(b))->getNumber();
}

void SimObjectList::sortObjectNumber()
{
 	m_qsort(address(),size(),sizeof(value_type),compareObjectNumber);
}	


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

SimObject::SimObject()
{
	id = 0;
	type = SimDefaultObjectType;
	group = 0;
	manager = 0;
	objectNumber = 0;
   objectName = NULL;
   nextNameObject = (SimObject*)-1;
   nextIdObject = NULL;

   fieldList = NULL;
}

void SimObject::setFieldValue(StringTableEntry slotName, const char *value)
{
   // empty field
   if(!*value)
   {
      CMDFieldEntry **walk = &fieldList;
      CMDFieldEntry *temp;
      while((temp = *walk) != NULL)
      {
         if(temp->slotName == slotName)
         {
            delete[] temp->value;
            *walk = temp->nextEntry;
            delete temp;
            return;
         }
         walk = &(temp->nextEntry);
      }
      return;
   }
   CMDFieldEntry *walk = fieldList;
   
   while(walk)
   {
      if(walk->slotName == slotName)
      {
         delete[] walk->value;
         walk->value = new char[strlen(value) + 1];
         strcpy(walk->value, value);
         return;
      }
      walk = walk->nextEntry;
   }
   walk = new CMDFieldEntry;
   walk->value = new char[strlen(value) + 1];
   strcpy(walk->value, value);
   walk->nextEntry = fieldList;
   walk->slotName = slotName;
   fieldList = walk;
}

const char *SimObject::getFieldValue(StringTableEntry slotName)
{
   CMDFieldEntry *walk = fieldList;
   
   while(walk)
   {
      if(walk->slotName == slotName)
         return walk->value;
      walk = walk->nextEntry;
   }
   return NULL;
}

SimObject::~SimObject()
{
   CMDFieldEntry *temp;
   while(fieldList)
   {
      temp = fieldList->nextEntry;
      delete[] fieldList->value;
      delete fieldList;
      fieldList = temp;
   }

	AssertFatal(nextNameObject == (SimObject*)-1,avar(
      "SimObject::~SimObject:  Not removed from dictionary: name %s, id %i",
      objectName,id));
	AssertFatal(manager == 0,"SimObject::~SimObject: "
		"Object still registered with a manager");
   AssertFatal(flags.test(Added) == 0, "SimObject::object "
   	"missing call to SimObject::onRemove");
}

//--------------------------------------------------------------------------- 

bool SimObject::isLocked()
{
   const char * val = getFieldValue( stringTable.insert( "locked", false ) );

   return( val ? CMDConsole::getBool(val) : false );   
}

void SimObject::setLocked( bool b = true )
{
   setFieldValue( stringTable.insert( "locked", false ), b ? "true" : "false" );
}

//--------------------------------------------------------------------------- 

bool SimObject::onAdd()
{
   flags.set(Added);
   // onAdd() should return FALSE if there was an error
   return true;
}

void SimObject::onRemove()
{
   flags.clear(Added);
}

void SimObject::onGroupAdd()
{
}

void SimObject::onGroupRemove()
{
}

void SimObject::onDeleteNotify(SimObject *obj)
{
   obj;
}

void SimObject::onNameChange(const char *name)
{
   name;
}   

void SimObject::inspectRead(Inspect *insp)
{
   char newName[256];
   insp->read(IDITG_SIMOBJECT_NAME, (Inspect::IString) newName);
   if(objectName && !stricmp(newName, objectName))
      return;
   assignName(newName);
}

void SimObject::inspectWrite(Inspect *insp)
{
   insp->write(IDITG_SIMOBJECT_NAME, (Inspect::IString) (objectName ? objectName : ""));
}

bool SimObject::processEvent(const SimEvent *event)
{
   if(event->type == SimConsoleEventType)
   {
      SimConsoleEvent *evt = (SimConsoleEvent *) event;
      if(evt->argc == -1)
      {
         Console->dbprintf(2, "SEVT: %d => %s", getId(), evt->argBuffer);
         Console->evaluate(evt->argBuffer, evt->echo, NULL, evt->privilegeLevel);
      }
      else
         Console->execute(evt->argc, evt->argv);
      return true;
   }
   return false;
}

bool SimObject::processQuery(SimQuery *query)
{
   query;
   return false;
}

bool SimObject::processArguments(int argc, const char **argv)
{
   argv;
   return argc == 0;
}

void SimObject::deleteObject()
{
	AssertFatal(manager != 0,
		"SimObject::deleteObject: Object not registered with manager");
	manager->deleteObject(this);
}

void SimObject::assignName(const char* name)
{
	if (group)
		group->assignName(this,name);
	else
	   objectName = stringTable.insert(name);
}


//--------------------------------------------------------------------------- 

void SimObject::deleteNotify(SimObject* obj)
{
	AssertFatal(!obj->isDeleted(),
		"SimManager::deleteNotify: Object is being deleted");
	obj->deleteNotifyList.pushBack(this);
	clearNotifyList.pushBack(obj);
}

void SimObject::clearNotify(SimObject* obj)
{
	obj->deleteNotifyList.remove(this);
	clearNotifyList.remove(obj);
}

void SimObject::clearAllNotifications()
{
	for (SimObjectList::iterator itr = clearNotifyList.begin();
			itr != clearNotifyList.end(); itr++)
		(*itr)->deleteNotifyList.remove(this);
	clearNotifyList.setSize(0);
}


//----------------------------------------------------------------------------

Persistent::Base::Error SimObject::read(StreamIO &, int, int)
{
	AssertFatal(0,"SimObject::read: No IO provided for object");
	return Persistent::Base::ReadError;
}

Persistent::Base::Error SimObject::write(StreamIO &, int, int)
{
	AssertFatal(0,"SimObject::write: No IO provided for object");
	return Persistent::Base::WriteError;
}

void SimObject::dump()
{
	AssertWarn(0,"SimObject::dump: No dump provided for object");
}


//--------------------------------------------------------------------------- 

bool SimObject::addToSet(SimObjectId spid)
{
	SimObject* ptr = manager->findObject(spid);
	if (ptr) {
		SimSet* sp = dynamic_cast<SimSet*>(ptr);
		AssertFatal(sp != 0,
			"SimObject::addToSet: "
			"ObjectId does not refer to a set object");
		sp->addObject(this);
		return true;
	}
	return false;
}

bool SimObject::addToSet(const char *ObjectName)
{
	SimObject* ptr = manager->findObject(ObjectName);
	if (ptr) {
		SimSet* sp = dynamic_cast<SimSet*>(ptr);
		AssertFatal(sp != 0,
			"SimObject::addToSet: "
			"ObjectName does not refer to a set object");
		sp->addObject(this);
		return true;
	}
	return false;
}

bool SimObject::removeFromSet(SimObjectId sid)
{
	SimObject* ptr = manager->findObject(sid);
	if (ptr) {
		SimSet* sp = dynamic_cast<SimSet*>(ptr);
		AssertFatal(sp != 0,
			"SimManager::removeFromSet: "
			"ObjectId does not refer to a set object");
		sp->removeObject(this);
		return true;
	}
	return false;
}


bool SimObject::removeFromSet(const char *ObjectName)
{
	SimObject* ptr = manager->findObject(ObjectName);
	if (ptr) {
		SimSet* sp = dynamic_cast<SimSet*>(ptr);
		AssertFatal(sp != 0,
			"SimManager::removeFromSet: "
			"ObjectName does not refer to a set object");
		sp->removeObject(this);
		return true;
	}
	return false;
}


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

SimSet::SimSet(bool own)
{
	ownObjects = own;
   stable = false;
}

SimSet::~SimSet()
{
	if (ownObjects) {
		// XXX Move this later into Group Class
		// If we have any objects at this point, they should 
		// already have been removed from the manager, so we
		// can just delete them directly.
		objectList.sortObjectNumber();
		while (!objectList.empty()) {
			delete objectList.last();
			objectList.decrement();
		}
	}
}

SimObject* SimSet::addObject(SimObject* obj)
{
	objectList.pushBack(obj);
	deleteNotify(obj);
   return ( obj );
}

void SimSet::removeObject(SimObject* obj)
{
   if (stable)
   	objectList.removeStable(obj);
   else      
   	objectList.remove(obj);
	clearNotify(obj);
}

bool SimSet::reOrder( SimObject *obj, SimObject *target )
{
   iterator itrS, itrD;
   if ( (itrS = find(begin(),end(),obj)) == end() )
      return false;  // object must be in list
   if ( obj == target )
      return true;   // don't reorder same object but don't indicate error
   if ( !target )    // if no target, then put to back of list
   {
      if ( itrS != (end()-1) )      // don't move if already last object
      {
         objectList.erase(itrS);    // remove object from its current location
         objectList.push_back(obj); // push it to the back of the list
      }
   }
   else              // if target, insert object in front of target
   {
      if ( (itrD = find(begin(),end(),target)) == end() )
         return false;              // target must be in list
      objectList.erase(itrS);
      objectList.insert(itrD,obj);
   }
   return true;
}   

void SimSet::onDeleteNotify(SimObject *object)
{
   removeObject(object);
   Parent::onDeleteNotify(object);
}

bool SimSet::onAdd()
{
   Vector<SimObject*>deletionList;
   
   for (SimObjectList::iterator ptr = objectList.begin(); ptr != objectList.end(); ptr++) {
	   // If an object has been pre-loaded by a SimSet
	   // it may already belong to the manager.
      if (!(*ptr)->getManager()) {
         if (manager->registerObject(*ptr) == false) {
            Console->printf("Failed to registerObject of type: %s (%s)",
                            (*ptr)->getClassName(),
                            (*ptr)->getName() ? (*ptr)->getName() :
                                                "<Unnamed>");
                                         
            // add this object for deletion.. which rearranges the objectlist
            deletionList.push_back(*ptr);
         }
      }
   }
   
   for(Vector<SimObject*>::iterator itr = deletionList.begin(); itr != deletionList.end(); itr++)
      manager->deleteObject(*itr);
      
   return Parent::onAdd();
}

void SimSet::onRemove()
{
	objectList.sortObjectNumber();
	if (objectList.size())
	{
		// This backwards iterator loop doesn't work if the
		// list is empty, check the size first.
		for (SimObjectList::iterator ptr = objectList.end() - 1;
				ptr >= objectList.begin(); ptr--)
	   {
			clearNotify(*ptr);
			if (ownObjects)
				manager->unregisterObject(*ptr);
		}
   }
	Parent::onRemove();
}

void SimSet::onPreLoad(SimPreLoadManager *splm)
{
   SimObjectList tmp = objectList;
	// Post the event to all members of the set
   for (iterator ptr = tmp.begin(); ptr != tmp.end(); ptr++)
	   (*ptr)->onPreLoad(splm);
}   

void SimSet::inspectRead(Inspect *inspector)
{
   Parent::inspectRead(inspector);
   inspector->read(IDITG_OWN_OBJECTS, ownObjects);
}

void SimSet::inspectWrite(Inspect *inspector)
{
   Parent::inspectWrite(inspector);
   inspector->write(IDITG_OWN_OBJECTS, ownObjects);
}

bool SimSet::processEvent(const SimEvent* event)
{
   // let SimObject process the console event
   if(Parent::processEvent(event))
      return true;

   SimObjectList tmp = objectList;
	// Post the event to all members of the set
   for (iterator ptr = tmp.begin(); ptr != tmp.end(); ptr++)
	   (*ptr)->processEvent(event);

   return false;
}

bool SimSet::processArguments(int argc, const char **argv)
{
   if(argc == 1)
      ownObjects = (bool) atoi(argv[0]);
   return true;
}


//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(SimSet, FOURCC('S','I','M','S'));

Persistent::Base::Error SimSet::write(StreamIO &sio, int, int user)
{
   if (!sio.write((int)objectList.size()))
      return Persistent::Base::WriteError;

	// Write objects
   for (int i = 0; i < objectList.size(); i++)
      if (objectList[i]->store(sio, user) != Ok)
         return Persistent::Base::WriteError;
   return Persistent::Base::Ok;
}

Persistent::Base::Error SimSet::read(StreamIO &sio, int, int)
{
   int objectCt = 0;

   if (!sio.read(&objectCt))
      return Persistent::Base::ReadError;

   Persistent::Base *base;
   Persistent::Base::Error error;

	// Read in the objects;
   for (int i = 0; i < objectCt; i++) {
      base = Persistent::Base::load(sio, &error);
      SimObject *object = dynamic_cast<SimObject *>(base);
      if (object && (error == Persistent::Base::Ok))
         // child objects will get added to the manager when this set
         // get's it's add notify 
         addObject(object);
      else if (base)
         // loaded something, not a sim object though
         delete base;
   }

   return Persistent::Base::Ok;
}


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

SimGroup::SimGroup(): SimSet(true)
{
}

SimGroup::~SimGroup()
{
	for (iterator itr = begin(); itr != end(); itr++)
		dictionary.remove(*itr);
}


//--------------------------------------------------------------------------- 

SimObject* SimGroup::addObject(SimObject* obj)
{
	if (obj->group != this) {
		if (obj->group)
			obj->group->removeObject(obj);
      dictionary.insert(obj);
		obj->group = this;
      objectList.push_back(obj); // force it into the object list
                                 // doesn't get a delete notify
		obj->onGroupAdd();
		return obj;
	}
	return this;
}

void SimGroup::removeObject(SimObject* obj)
{
	if (obj->group == this) {
		obj->onGroupRemove();
		dictionary.remove(obj);
      if(stable)
         objectList.removeStable(obj);
      else
         objectList.remove(obj);
		obj->group = 0;
	}
}

//--------------------------------------------------------------------------- 

void SimGroup::onRemove()
{
	objectList.sortObjectNumber();
	if (objectList.size())
	{
		// This backwards iterator loop doesn't work if the
		// list is empty, check the size first.
		for (SimObjectList::iterator ptr = objectList.end() - 1;
				ptr >= objectList.begin(); ptr--)
	   {
			(*ptr)->group = NULL;
			manager->unregisterObject(*ptr);
			(*ptr)->group = this;
		}
   }
	SimObject::onRemove();
}

//--------------------------------------------------------------------------- 

void SimGroup::assignName(SimObject* obj,const char* name)
{
	AssertFatal(obj->group == this,"SimGroup:assignName: Object not in group");
   if(obj->objectName)
      dictionary.remove(obj);
   if(*name == 0)
      obj->objectName = NULL;
   else
   {
	   obj->objectName = stringTable.insert(name);
		dictionary.insert(obj);
      obj->onNameChange(obj->objectName);
	}
}

//--------------------------------------------------------------------------- 

inline bool isGroupSeperator(char c)
{
	return c == '\\' || c == '/';
}

SimObject* SimGroup::findObject(SimObjectId id)
{
	// Manager has id->object dictionary
	return manager->findObject(id);
}

SimObject* SimGroup::findObject(const char* name)
{
   char c = *name;
   SimGroup *grp;
   SimSet *set = NULL;
   SimObject *obj;
   if(c >= '0' && c <= '9')
   {
      // it's an id group
      const char *temp = name + 1;
      for(;;)
      {
         c = *temp++;
         if(!c)
            return manager->findObject(atoi(name));
         else if(isGroupSeperator(c))
         {
            obj = manager->findObject(atoi(name));
            if(!obj)
               return NULL;
            grp = dynamic_cast<SimGroup *>(obj);
            if(!grp)
               return NULL;
            name = temp;
            break;
         }
      }
   }
   else
   {
      while(isspace(*name))
         name++;
	   grp = isGroupSeperator(*name) ? manager: this;
   }
   char buffer[256];
   char *walk = buffer;

   for(;;)
   {
      char c = *name++;
      if(isGroupSeperator(c))
      {
         obj = NULL;
         *walk = 0;
         StringTableEntry name = stringTable.lookup(buffer);
         if(!name)
            return NULL;
         if(grp)
            obj = grp->dictionary.find(name);
         else if(set)
         {
            SimSet::iterator i;
            for(i = set->begin(); i != set->end(); i++)
            {
               if((*i)->getName() == name)
               {
                  obj = *i;
                  break;
               }
            }
         }
         if(!obj)
            return NULL;
         grp = dynamic_cast<SimGroup *>(obj);
         if(!grp)
         {
            set = dynamic_cast<SimSet *>(obj);
            if(!set)
               return NULL;
         }
         walk = buffer;
      }
      else if(c == 0)
      {
         *walk = 0;
         StringTableEntry name = stringTable.lookup(buffer);
         if(grp)
            return grp->dictionary.find(name);
         else if(set)
         {
            SimSet::iterator i;
            for(i = set->begin(); i != set->end(); i++)
               if((*i)->getName() == name)
                  return *i;
         }
         return NULL;
      }
      else
         *walk++ = c;
   }
}

//--------------------------------------------------------------------------- 

bool SimGroup::processArguments(int, const char **)
{
   return true;
}


//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(SimGroup, FOURCC('S','I','M','G'));

void write_string(StreamIO& sio, const char* s)
{
	UInt8 len = s? strlen(s): 0;
	sio.write(len);
	if (len)
		sio.write(len,s);
}

char* read_string(StreamIO& sio)
{
	UInt8 len;
	sio.read(&len);
	if (len) {
		char* s = new char[len + 1];
		sio.read(len,s);
		s[len] = '\0';
		return s;
	}
	return 0;
}

Persistent::Base::Error SimGroup::write(StreamIO &sio, int ver, int user)
{
	// Base class write out the objects.
	Parent::write(sio,ver,user);

	// Write out names
   for (int i = 0; i < objectList.size(); i++)
		write_string(sio,objectList[i]->objectName);

   return (sio.getStatus() == STRM_OK)? Persistent::Base::Ok:
		Persistent::Base::WriteError;
}

Persistent::Base::Error SimGroup::read(StreamIO &sio, int ver, int user)
{
	// Base class will read in all the objects
	Parent::read(sio,ver,user);

	// Read in all the names
	char* s;
   for (int i = 0; i < objectList.size(); i++)
   {
   	if ((s = read_string(sio)) != 0)
   	{
			assignName(objectList[i],s);
         delete[] s;
      }
   }

   return (sio.getStatus() == STRM_OK)? Persistent::Base::Ok:
		Persistent::Base::ReadError;
}


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

SimManager::SimManager()
{
   InitializeCriticalSection( &cs_managerLock );

   // do NOT change the order of this code without updating
   // the object SimIdDictionary cache and SimRecorder
   // AND netCSDelegate (check simEvDecl.h)

   // SimIdDictionary   0...1023
   // SimRecorder    1024...2047
   // netCSDelegate  2048...3071

   idManager.alloc(this);                          // pre-alloc reserved id block
   idManager.alloc(this);                          // pre-alloc block for recorder
   idManager.alloc(this);                          // pre-alloc block for manager ids
   idGenerator.addBlock( idManager.alloc(this) );  // alloc second block for general use id's
   idGenerator.addBlock( idManager.alloc(this) );  // alloc second block for general use id's
   idGenerator.addBlock( idManager.alloc(this) );  // alloc second block for general use id's
   idGenerator.addBlock( idManager.alloc(this) );  // alloc second block for general use id's
   idGenerator.addBlock( idManager.alloc(this) );  // alloc second block for general use id's
   idGenerator.addBlock( idManager.alloc(this) );  // alloc second block for general use id's

	targetTime = 0;
   eventQueue.setCurrentTime(targetTime);
   serverType = -1;
	manager = this;

	//
	SimSet *set = new SimSet();
	set->id = SimLookupSetId;
	addObject (set);
}

SimManager::~SimManager()
{
	// We want to destroy the objects in the reverse order
	// in which they were added. (Deleting from the end of queue
	// is faster as well)
	objectList.sortObjectNumber();
	while (!objectList.empty()) {
		SimObject *obj = objectList.last();
      obj->flags.set(Deleted);
		unregisterObject(obj);
		delete obj;
	}

   manager = 0;
   DeleteCriticalSection( &cs_managerLock );
}


//--------------------------------------------------------------------------- 

bool SimManager::isServer()
{
	return this == SimGame::get()->getManager(SimGame::SERVER);
}

//------------------------------------------------------------------------------

void SimManager::lock()
{
   EnterCriticalSection( &cs_managerLock );
}

void SimManager::unlock()
{
   LeaveCriticalSection( &cs_managerLock );
}


//--------------------------------------------------------------------------- 

bool SimManager::registerObject(SimObject* obj)
{
	AssertFatal(manager,"SimGroup::addObject: "
			"Group is not registered with a manager");

	// For the manager, objectNumber is the total number of
	// objects added so far.  Would probably not be good if this
	// number wraps.
	obj->objectNumber = objectNumber++;
	obj->flags.clear(Deleted | Removed | DynamicId);
	obj->manager = manager;
	assignId(obj,obj->id);

	// Notify object
   bool ret = obj->onAdd();

   if(!ret)
   {
		dictionary.remove(obj);
	   obj->manager = 0;
	   if (obj->id && obj->flags.test(DynamicId)) {
	      idGenerator.free( obj->id );
	      obj->flags.clear(DynamicId);
	      obj->id = 0;
	   }
   }
   AssertFatal(!ret || obj->isProperlyAdded(), "Object did not call SimObject::onAdd()");
   return ret;
}

void SimManager::unregisterObject(SimObject* obj)
{
   obj->flags.set(Removed);
   
	// Notify object first
	obj->onRemove();

	// Clear out any pending notifications before
	// we call our own, just in case they delete
	// something that we have referenced.
	obj->clearAllNotifications();

	// Notify all objects that are waiting for delete
	// messages
   if(obj->getGroup())
      obj->getGroup()->removeObject(obj);

	if (!obj->deleteNotifyList.empty()) {
		SimObjectList& nlist = obj->deleteNotifyList;

      while (nlist.size())
      {
         SimObject *nobj = nlist.last();
         nlist.pop_back();
			nobj->clearNotifyList.remove(obj);
			nobj->onDeleteNotify(obj);
      }
	}

	dictionary.remove(obj);
   eventQueue.remove(obj);

	//
	obj->manager = 0;
	if (obj->id && obj->flags.test(DynamicId)) {
	   idGenerator.free( obj->id );
	   obj->flags.clear(DynamicId);
	   obj->id = 0;
	}
}


//----------------------------------------------------------------------------

SimObject* SimManager::addObject(SimObject* obj)
{
	if (!obj->getManager()) {
      if (registerObject(obj) == false) {
         Console->printf("Failed to registerObject of type: %s (%s)",
                         obj->getClassName(),
                         obj->getName() ? obj->getName() :
                                          "<Unnamed>");
         deleteObject(obj);
         return NULL;
      }

		if (obj->group)
			return obj;
	}
   return Parent::addObject(obj);
}

void SimManager::deleteObject(SimObject* obj)
{
	AssertFatal(!obj->isDeleted(),"SimManger::deleteObject: "
		"Object has already been deleted");
	AssertFatal(!obj->isRemoved(),"SimManger::deleteObject: "
		"Object in the process of being removed");
	obj->flags.set(Deleted);

	// Remove from the dictionaries right away.
	if (obj->group)
		obj->group->dictionary.remove(obj);
	dictionary.remove(obj);
	//deleteList.pushBack(obj);
   unregisterObject(obj);
   delete obj;
}


//--------------------------------------------------------------------------- 

SimObjectId SimManager::assignId(SimObject* obj,SimObjectId id)
{
   // get this object out of the id dictionary if it's in it
   dictionary.remove(obj);

	// Free current Id.
	if (obj->id && obj->flags.test(DynamicId)) 
	{
	   idGenerator.free( obj->id );
	   obj->flags.clear(DynamicId);
		obj->id = 0;
	}
	// Assign new one.
	if (id)
		obj->id = id;
	else {
		obj->flags.set(SimObject::DynamicId);
		obj->id = idGenerator.alloc();
	}
	dictionary.insert(obj);
   return (obj->id);
}

void SimManager::assignName(SimObject* obj,const char* name)
{
	if (obj->group == this)
		Parent::assignName(obj,name);
	else
		obj->assignName(name);
}


//--------------------------------------------------------------------------- 

SimObject* SimManager::findObject(SimObjectId id)
{
   if (id == 0)
      return (this);
	return dictionary.find(id);
}


//--------------------------------------------------------------------------- 
void SimManager::resetTime()
{
   eventQueue.subtractTime(0);
   targetTime = 0;
}

int SimManager::advanceToTime(SimTime time)
{
   lock();
   AssertFatal(time >= targetTime, "SimManager::advanceToTime: cannot advance to time in the past.");

   if (time == targetTime) {
      // Early out 
		clearDeleteList();
      return (0);
   }
	targetTime = time;
	
	int count = 0;
	SimEvent* event;

	// EventQueue::pop will get the next event and advance 
	// currentTime to the time of the event.
	for (; (event = eventQueue.pop(targetTime)) != 0; ++count) {
		// Resolve address & deliver event
		SimObject* obj = event->address.resolve(this);
		if (obj) {
			if (!obj->isDeleted())
				obj->processEvent(event);
		}
		else {
			// A non-guarenteed remote event may slip through for an
			// object being sent by guarenteed delivery.
	      AssertFatal(event->flags.test(SimEvent::Remote),
	      	"SimManager::postEvent: Invalid destination object Id");
		}
		clearDeleteList();
		delete event;
	}

	if (!count)
		clearDeleteList();

   unlock();
	return count;
}

//------------------------------------------------------------------------------
void SimManager::postEvent(SimEvent* event,bool currentEvent)
{
	AssertFatal(currentEvent || event->time >= getCurrentTime(),
		"SimManager::postEvent: Cannot go back in time.");
	AssertFatal(event->type >= SimTypeRange && 
		event->type < SimTypeRange + SIM_RESERVED_ID_LIMIT,
		"SimManager::postEvent: Id out of range, garbage?");

	// A managerId of 0 defaults to the current manager.
	if (!event->address.managerId || event->address.managerId == id)
	   eventQueue.insert(event,currentEvent);
	else
	{
		if (currentEvent)
		{
			// Give a time, this value might not actually be correct,
			// it's normally assigned by the queue. It's only done
			// here so the router objects have something.
			event->time = getCurrentTime();
		}
		SimObject *dispatcher = findObject(event->address.managerId);
      
      // if there's a direct router object, hand this off to that
      // object... (most likely it's a packet stream)

      if(dispatcher)
         dispatcher->processEvent(event);
      else // send this puppy to la-la land
   		delete event;
	}
}

void SimManager::postEvent(const char* name,SimEvent* event,bool currentEvent)
{
	SimObject* obj = findObject(name);
	AssertFatal(obj != 0,"SimManager::postEvent: Invalid destination object name");
	if (obj) {
		event->address.set(obj);
		postEvent(event,currentEvent);
	}
}


//--------------------------------------------------------------------------- 

bool SimManager::processEvent(const SimEvent *event)
{
	switch (event->type) 
	{
		case SimGroupObjectIdEventType:
		   id = ((SimGroupObjectIdEvent*)event)->id;
         return true;
		default:
		   return Parent::processEvent( event);
   }
}


//--------------------------------------------------------------------------- 

void SimManager::clearDeleteList()
{
	// Actually deletes the objects in the reverse order
	// that they were deleted with deleteObject().
	while (!deleteList.empty()) {
		SimObject* object = deleteList.last();
		deleteList.decrement();

		unregisterObject(object);

		delete object;
	}
}

//------------------------------------------------------------------------------
void SimManager::preLoad()
{
   // make sure there's a preloadManager before we start
   // cycling through all the objects in the manager
   SimPreLoadManager *splm = SimPreLoadManager::get(this);
   if (!splm)
      return;

   onPreLoad(splm);
}   