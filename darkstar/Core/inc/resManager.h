//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _RESMANAGER_H_
#define _RESMANAGER_H_

#include <types.h>
#include <stringTable.h>

#define RES_DEFAULT_TIMEOUT (5*60*1000)   //5-minutes

class StreamIO;
class DLLAPI ResourceType;
class DLLAPI ResourceManager;
class FindMatch;

//------------------------------------------------------------------------------

// ok, here's the deal.
// you can lock files in a volume.
// those files must be unlocked before the volume is closed
// a volume can not have locked memory when the volume is closed (assert)
// a volume CAN have loaded resources when the volume is closed
// this becomes a detached (ie Added) resource
// when a volume is opened, all its entries are added to the 
// resource dictionary.
// if an entry name conflicts with an Added resource,
// the Added resource becomes a VolumeBlock resource
// same with the search path
// if a resource's volume is closed out from under it
// the resource becomes an Added resource
// purged resources that are Added are deleted

struct VolumeEntry
{
   VolumeEntry *nextVolume; // volumes are stored as a linked list.

   StringTableEntry filePath;
   
   StringTableEntry path;
   StringTableEntry name;

   int volRefCount; // number of times volume has been added to the res manager
   int lockCount; // number of outstanding memory locked resources
   HANDLE hFile;
   HANDLE hFileMap;
   BYTE *baseAddr;
   int fileSize;
};



class DLLAPI ResourceObject
{
   friend class ResDictionary;
   friend class ResourceManager;

   ResourceObject *prev, *next; // timeout list
   ResourceObject *nextEntry; // objects are inserted by id or name
   ResourceObject *nextResource; // in linked list of all resources
   ResourceObject *prevResource;
public:
   enum
   {
      VolumeBlock = 1,
      File = 2,
      Added = 4,
      RLECompressed = 8,
      LZHCompressed = 16,
      MarkFlag = 32, // for marking all the file resources in the search path
   };
   void *resource;
   void *lockedData;
   int flags;
   VolumeEntry *volume;  // for grabbing memory mapped files
                         // if someone tries to 'lock' a non mem-mapped
                         // resource, it is just loaded into memory.
                         // you can't both load a resource and
                         // lock the same file.

   StringTableEntry path;     // resource path
   StringTableEntry name;     // resource name

   StringTableEntry filePath; // path/name of file or volume if in volume
   StringTableEntry fileName;

   int fileOffset;       // offset on disk in fileName file of resource
   int fileSize;         // size on disk of resource block

   ResourceType *type;
   ResourceManager *rm;
   int lockCount;
   int timeout;

   ResourceObject();
   ~ResourceObject() { unlink(); }

   void construct(StreamIO *vol, int size);
   void destruct();

   ResourceObject* getNext() const { return next; }
   void unlink();
   void linkAfter(ResourceObject* res);
};   


inline void ResourceObject::unlink()
{
   if (next)
      next->prev = prev;
   if (prev)
      prev->next = next;
   next = prev = 0;
}

inline void ResourceObject::linkAfter(ResourceObject* res)
{
   unlink();
   prev = res;
   if ((next = res->next) != 0)
      next->prev = this;
   res->next = this;
}


//------------------------------------------------------------------------------
template <class T>
class Resource
{
private:
   ResourceObject *obj;
   // ***WARNING***
   // Using a faster lock that bypasses the resource manger.
   // void _lock() { if (obj) obj->rm->lockResource( obj ); }
   void _lock();
   void _unlock();

public:
   // If assigned a ResourceObject, it's assumed to already have
   // been locked, lock count is incremented only for copies or
   // assignment from another Resource.
   Resource() : obj(NULL) { ; }
   Resource(ResourceObject *p) : obj(p) { ; }
   Resource(const Resource &res) : obj(res.obj) { _lock(); }
   ~Resource() { unlock(); }
   
   const char *getFileName() const { return (obj ? obj->name : NULL); }

   Resource& operator= (ResourceObject *p) { _unlock(); obj = p; return *this; }
   Resource& operator= (const Resource &r) { _unlock(); obj = r.obj; _lock(); return *this; }

   operator bool() const { return ((obj != NULL) && (obj->resource != NULL)); }
   T* operator->()   { return (T*)obj->resource; }
   T& operator*()    { return *((T*)obj->resource); }
   operator T*()     { return (obj) ? (T*)obj->resource : (T*)NULL; }
   T const * operator->() const  { return (T const *)obj->resource; }
   T const & operator*() const   { return *((T const *)obj->resource); }
   operator T const *() const    { return (obj) ? (T const *)obj->resource :  (T const *)NULL; }
   void unlock(int ms=0);
   void purge();
};

template<class T> 
inline void Resource<T>::unlock( int ms )
{
   if (obj) {
      obj->rm->unlock( obj, ms );
      obj=NULL;
   }
}

template<class T> 
inline void Resource<T>::purge()
{
   if (obj) { 
      obj->rm->unlock( obj, 0);
      if (obj->lockCount == 0)
         obj->rm->purge(obj); 
      obj = NULL;
   }
}
template <class T>
inline void Resource<T>::_lock()
{
   if (obj)
      obj->lockCount++;
}

template <class T>
inline void Resource<T>::_unlock()
{
   if (obj)
      obj->rm->unlock( obj );
}

#define INVALID_ID ((DWORD)(~0))

//----------------------------------------------------------------------------
// Map of Names and Object IDs to objects
// Provides fast lookup for name->object, id->object and
// for fast removal of an object given object*
//
class DLLAPI ResDictionary
{
   enum { DefaultTableSize = 1029 };

   ResourceObject **hashTable;
   int entryCount;
   int hashTableSize;
   DataChunker memPool;
   int hash(StringTableEntry path, StringTableEntry name);
   int hash(ResourceObject *obj) { return hash(obj->path, obj->name); }
public:
   ResDictionary();
   ~ResDictionary();

   void insert(ResourceObject *obj, StringTableEntry path, StringTableEntry file);
   ResourceObject* find(StringTableEntry path, StringTableEntry file);
   ResourceObject* find(StringTableEntry path, StringTableEntry file, StringTableEntry filePath, StringTableEntry fileName);
   void pushBehind(ResourceObject *obj, int mask);
   void remove(ResourceObject *obj);
};


//------------------------------------------------------------------------------
class DLLAPI ResourceManager
{
private:
   char writeablePath[1024];
   VolumeEntry *volumeList;
   ResourceObject timeoutList;
   ResourceObject resourceList;

   ResDictionary dictionary;

   ResourceObject *createResource(StringTableEntry path, StringTableEntry file, StringTableEntry filePath, StringTableEntry fileName);
   void freeResource(ResourceObject *resObject);
   void searchPath(const char *pathStart, char buf[]);
   bool outsideSearchEnable;
public:
   ResourceManager();
   ~ResourceManager();

   void setSearchPath(const char *path);
   bool addVolume(const char *volumeName); // must exist in the search path
   void removeVolume(const char *volumeName); // must have been added previously

   int getSize(const char* filename);
   const char* getFullPath(const char* filename, char *path);
   const char* getPathOf(const char* filename);
   void* lock(const char *fileName);   

   ResourceObject* load(const char *fileName, bool block = true);
   ResourceObject* find(const char *fileName);

   void unlock(ResourceObject*, int msDelay=0);
   bool add(DWORD resTypeId, const char* name, void *resource, bool extraLock = false);

   ResourceObject* findMatch(const char *expression, const char **fn, ResourceObject *start = NULL);

   void purge( bool force=false );
   void purge( ResourceObject *obj );
   
   int  findMatches( FindMatch *pFM );
   bool findFile( const char *name );
   
   bool getCrc(const char * fileName, UInt32 & crcVal, const UInt32 crcInitialVal = 0xffffffff );
   void setOutsideSearch(bool enable);
   void setWriteablePath(const char *path);
   bool isValidWriteFileName(const char *fn);

   static ResourceManager* sm_pManager;
};   


//------------------------------------------------------------------------------
class DLLAPI ResourceType
{
private:
   DWORD type;
   int timeoutDelay;
   ResourceType *nextType;

   static ResourceType *typeList;
public:
   ResourceType(DWORD type);
   DWORD getType() const;
   int   getTimeoutDelay() const;

   static DWORD typeof( const char *name );
   static ResourceType *find(DWORD type);

   // User must provide construct/destruct functionallity
   virtual void* construct(StreamIO *stream, int size) = 0;
   virtual void  destruct(void *) = 0;
};   

inline ResourceType::ResourceType(DWORD _type)
{
   type = _type;
   timeoutDelay = RES_DEFAULT_TIMEOUT;
   nextType = typeList;
   typeList = this;
}

inline int ResourceType::getTimeoutDelay() const { return timeoutDelay; }
inline DWORD ResourceType::getType()       const { return type; }

// simple crc - may need to move somewhere else someday
namespace CRC
{
   // will generate the table on the first call
   static UInt32 calculate(void * buffer, int len, UInt32 crcVal = 0xffffffff);
};

#endif //_RESMANAGER_H_
