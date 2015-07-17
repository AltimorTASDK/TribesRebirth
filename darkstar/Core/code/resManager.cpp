//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <base.h>
#include <streams.h>
#include <stdlib.h>
#include <stdio.h>
#include "ResManager.h"
#include "coreRes.h"

extern int gfxResTypes;
int forcelink = gfxResTypes;

ResourceManager* ResourceManager::sm_pManager = NULL;
static char sgCurExeDir[1024];
static int sgCurExeDirStrLen;

ResourceType *ResourceType::typeList = NULL;
ResourceType *ResourceType::find(DWORD type)
{
   for(ResourceType *walk = typeList; walk; walk = walk->nextType)
      if(type == walk->type)
         return walk;
   return NULL;
}

//------------------------------------------------------------------------------
ResourceObject::ResourceObject() 
{
   next = prev = 0;
   timeout   = GetTickCount();
   lockCount = 0;
   resource  = NULL;
}

void ResourceObject::construct(StreamIO *stream, int size)
{
   AssertFatal(type, "ResourceObject::construct: NULL resource object type.");
   AssertFatal(!resource, "ResourceObject::construct: NON-NULL resource data pointer.");
   resource = type->construct( stream, size );
}

void ResourceObject::destruct()
{
   if(!type)
      return;

   // If the resource was not loaded because of an error, the resource
   // pointer will be NULL
   if (resource) {
      type->destruct( resource );
      resource = NULL;   
   }
}

//------------------------------------------------------------------------------
DWORD ResourceType::typeof( const char *name )
{
   DWORD type = INVALID_ID;

   char *d = (char*)&type;
   const char *s = strrchr( name, '.' );
   if(s++) 
   {
      int l = 0;
      while(*s && l < 4)
      {
         *d++ = (char)tolower(*s++);
         l++;
      }
   }
   return (type);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

ResourceManager::ResourceManager()
{
   writeablePath[0] = 0;
   outsideSearchEnable = true;
   volumeList = NULL;
   resourceList.nextResource = NULL;
   resourceList.next = resourceList.prev = NULL;
   timeoutList.nextResource = NULL;
   timeoutList.next = timeoutList.prev = NULL;
   sgCurExeDir[0] = '\0';
   GetCurrentDirectory(1024, sgCurExeDir);
   sgCurExeDirStrLen = strlen(sgCurExeDir);
}

//------------------------------------------------------------------------------
ResourceManager::~ResourceManager()
{
   purge(true);
   // volume list should be gone.

   for(ResourceObject *walk = resourceList.nextResource; walk; walk = walk->nextResource)
      walk->destruct();
   
   while(resourceList.nextResource)
      freeResource(resourceList.nextResource);
}

//------------------------------------------------------------------------------
void ResourceManager::setOutsideSearch(bool enable)
{
   outsideSearchEnable = enable;
}

bool ResourceManager::isValidWriteFileName(const char *fn)
{
   if(!writeablePath[0])
      return true;

   // first find the path to this file
   static char buf[1024];
   strncpy(buf, fn, 1023);
   for(int i = 0; buf[i]; ++i)
      if(buf[i] == '/')
         buf[i] = '\\';
   buf[1023] = 0;
   
   char *divider = strrchr(buf, '\\');
   char *path;
   if(!divider)
   {
      path = ".";
   }
   else
   {
      path = buf;
      *divider = 0;
      
      // if the path isn't relative, make it so
      if (!strnicmp(path, sgCurExeDir, sgCurExeDirStrLen))
      {
         path = &path[sgCurExeDirStrLen];
         if (*path == '\\')
            path++;
         if (*path == '\0')
            path = ".";   
      }
   }
 
   // now loop through the writeable path.
   char *start = writeablePath;
   int pathLen = strlen(path);
   while(start && *start)
   {
      if (!strnicmp(start, path, pathLen))
      {
         if (start[pathLen] == ';' || start[pathLen] == '\0')
         {
            // because path must match one of the 
            // specified paths exactly
            return true;
         }
      }
      start = strchr(start, ';');
      if (start)
         start++;
   }
   return false;
}

void ResourceManager::setWriteablePath(const char *path)
{
   strcpy(writeablePath, path);
}

//------------------------------------------------------------------------------
static const char *buildPath(StringTableEntry path, StringTableEntry file)
{
   static char buf[1024];
   if(path)
      sprintf(buf, "%s\\%s", path, file);
   else
      strcpy(buf, file);
   return buf;
}

//------------------------------------------------------------------------------
static void getPaths(const char *fullPath, StringTableEntry &path, StringTableEntry &fileName)
{
   static char buf[1024];
   char *ptr = (char *) strrchr(fullPath, '\\');
   if(!ptr)
   {
      path = NULL;
      fileName = stringTable.insert(fullPath);
   }
   else
   {
      int len = ptr - fullPath;
      strncpy(buf, fullPath, len);
      buf[len] = 0;
      fileName = stringTable.insert(ptr + 1);
      path = stringTable.insert(buf);
   }
}

//------------------------------------------------------------------------------
bool ResourceManager::addVolume(const char *volumeName)
{
   StringTableEntry path, fileName;
   getPaths(volumeName, path, fileName);
   for(VolumeEntry *walk = volumeList; walk;walk = walk->nextVolume)
   {
      if(walk->name == fileName && walk->path == path)
      {
         walk->volRefCount++;
         return true;
      }
   }
   ResourceObject *volObject = dictionary.find(path, fileName);
   if(!volObject)
      return false; // not in current search path
   if(volObject->type || volObject->flags != ResourceObject::File)
      return false; // a typed resource is not a volume

   VolumeEntry *newVol = new VolumeEntry;
   newVol->nextVolume = volumeList;
   volumeList = newVol;

   newVol->volRefCount = 1;
   newVol->fileSize = volObject->fileSize;
   newVol->name = fileName;
   newVol->path = path;
   newVol->filePath = volObject->filePath;
   newVol->hFile = INVALID_HANDLE_VALUE;
   newVol->baseAddr = NULL;

   // now open the volume and add all its resources to the dictionary
   VolumeRStream vrs;
   vrs.openVolume(buildPath(newVol->filePath, newVol->name));

   VolumeStream::VolumeItemVector::iterator i;
   for(i = vrs.volumeItems.begin(); i != vrs.volumeItems.end(); i++)
   {
      if(!i->string)
         continue;
      StringTableEntry rpath, rfile;
      getPaths(i->string, rpath, rfile);

      ResourceObject *ro = createResource(rpath, rfile, newVol->filePath, newVol->name);
      // just force in these flags: any existing resource object
      // will be mapped newly into this volume
      // which is ok.

      if(i->compressType == STRM_COMPRESS_RLE)
         ro->flags = ResourceObject::VolumeBlock | ResourceObject::RLECompressed;
      else if(i->compressType == STRM_COMPRESS_LZH)
         ro->flags = ResourceObject::VolumeBlock | ResourceObject::LZHCompressed;
      else
         ro->flags = ResourceObject::VolumeBlock;

      ro->volume = newVol;
      ro->fileSize = i->size;
      ro->fileOffset = sizeof(BlockStream::Header) + i->blockOffset;
      dictionary.pushBehind(ro, ResourceObject::File);
   }
   vrs.closeVolume();
   return true;
}

//------------------------------------------------------------------------------
void ResourceManager::removeVolume(const char *volumeName)
{
   StringTableEntry vpath, vfile;
   getPaths(volumeName, vpath, vfile);

   VolumeEntry **walk;
   for(walk = &volumeList; *walk;walk = &((*walk)->nextVolume))
   {
      if((*walk)->name == vfile && (*walk)->path == vpath)
         break;
   }
   VolumeEntry *temp = *walk;
   if(!temp)
      return;
   if(!--temp->volRefCount) {
      if(temp->baseAddr)
      {
         UnmapViewOfFile(temp->baseAddr);
         CloseHandle(temp->hFileMap);
         CloseHandle(temp->hFile);
      }
      *walk = temp->nextVolume;

      // loop through all the resources, removing ones that
      // were owned by this volume and detaching ones that are loaded

      ResourceObject *rwalk = resourceList.nextResource;
      ResourceObject *rtemp;

      while(rwalk != NULL) {
         if(rwalk->volume == temp) {
            if(rwalk->resource) {
               rwalk->volume = NULL;
               rwalk->flags = ResourceObject::Added;
               rwalk = rwalk->nextResource;
               dictionary.pushBehind(rwalk, ResourceObject::File | ResourceObject::VolumeBlock);
            }
            else {
               rtemp = rwalk;
               rwalk = rwalk->nextResource;
               rtemp->unlink();
               dictionary.remove(rtemp);
               freeResource(rtemp);
            }
         }
         else {
            rwalk = rwalk->nextResource;
         }
      }
      delete temp;
   }
}

//------------------------------------------------------------------------------
void ResourceManager::searchPath(const char* basePath, char curPath[])
{
   char buf[1024];
   if(curPath[0])
      sprintf(buf, "%s\\%s\\*", basePath, curPath);
   else
      sprintf(buf, "%s\\*", basePath);
   
   int curPathLen = strlen(curPath);
   WIN32_FIND_DATA findData;

   HANDLE hFind = FindFirstFile( buf, &findData );
   while(hFind != INVALID_HANDLE_VALUE)
   {
      if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         // recursively search directories.
         if(findData.cFileName[0] != '.')
         {
            if(curPath[0])
               strcat(curPath, "\\");
            strcat(curPath, findData.cFileName);
            searchPath(basePath, curPath);
            curPath[curPathLen] = 0;
         }
      }
      else
      {
         // it's a file!
         StringTableEntry filePath, virtPath, fileName;

         if(curPath[0])
         {
            sprintf(buf, "%s\\%s", basePath, curPath);
            filePath = stringTable.insert(buf);
            virtPath = stringTable.insert(curPath);
         }
         else
         {
            filePath = stringTable.insert(basePath);
            virtPath = NULL;
         }
         fileName = stringTable.insert(findData.cFileName);
         ResourceObject *ro = createResource(virtPath, fileName, filePath, fileName);
         dictionary.pushBehind(ro, ResourceObject::File);

         ro->flags = ResourceObject::File;
         ro->fileOffset = 0;
         ro->fileSize = findData.nFileSizeLow;
         ro->volume = NULL;
      }
      if(!FindNextFile(hFind, &findData))
      {
         FindClose(hFind);
         hFind = INVALID_HANDLE_VALUE;
      }
   }
}

//------------------------------------------------------------------------------
void ResourceManager::setSearchPath(const char *path)
{
   char buf[2048];
   char buf2[2048];
   strcpy(buf, path);
   char *walk = buf;
   char *temp;

   for(ResourceObject *pwalk = resourceList.nextResource; pwalk; pwalk = pwalk->nextResource)
   {
      // detach all the files.
      if(pwalk->flags & ResourceObject::File)
      {
         pwalk->flags = ResourceObject::Added;
         dictionary.pushBehind(pwalk, ResourceObject::File | ResourceObject::VolumeBlock);
      }
   }

   do {
      temp = strchr(walk, ';');
      if(temp)
         *temp++ = 0;
      buf2[0] = 0;
      // scan for: backslash at start, '.' and ':'
      // '.' is valid as the first character only.
      // walk must also have at least one character.

      if(*walk && (outsideSearchEnable || (walk[0] != '\\' && !strchr(walk+1, '.') && !strchr(walk, ':'))))
         searchPath(walk, buf2);
      walk = temp;
   } while(temp);

   ResourceObject *rwalk = resourceList.nextResource, *rtemp;

   // unlink all the added baddies that aren't loaded.
   while(rwalk != NULL)
   {
      if((rwalk->flags & ResourceObject::Added) && !rwalk->resource)
      {
         rwalk->unlink();
         dictionary.remove(rwalk);
         rtemp = rwalk->nextResource;
         freeResource(rwalk);
         rwalk = rtemp;
      }
      else
         rwalk = rwalk->nextResource;
   }
}

//------------------------------------------------------------------------------

int ResourceManager::getSize(const char *fileName)
{
   ResourceObject *ro = find(fileName);
   if(!ro)
      return 0;
   else
      return ro->fileSize;
}

//------------------------------------------------------------------------------
const char* ResourceManager::getFullPath(const char* fileName, char *path)
{
   AssertFatal(fileName, "ResourceManager::getFullPath: fileName is NULL");
   AssertFatal(path, "ResourceManager::getFullPath: path is NULL");
   ResourceObject *obj = find(fileName);
   if(!obj)
      strcpy(path, fileName);
   else
      sprintf(path, "%s\\%s", obj->filePath, fileName);
   return path;
}   

//------------------------------------------------------------------------------
const char* ResourceManager::getPathOf(const char* fileName)
{
   AssertFatal(fileName, "ResourceManager::getFullPath: fileName is NULL");
   ResourceObject *obj = find(fileName);
   if(!obj)
      return NULL;
   else
      return obj->filePath;
}   

//------------------------------------------------------------------------------

void* ResourceManager::lock(const char *fileName)
{
   ResourceObject *obj = find(fileName);
   if(!obj)
      return NULL;
   if(!obj->lockedData)
   {
      if(obj->flags & ResourceObject::VolumeBlock)
      {
         VolumeEntry *v = obj->volume;
         if(!v->baseAddr)
         {
            v->hFile = CreateFile(buildPath(v->filePath, v->name),  GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if(v->hFile == INVALID_HANDLE_VALUE)
               return NULL;
            v->hFileMap = CreateFileMapping(v->hFile, NULL, PAGE_READONLY, 0, GetFileSize(v->hFile, NULL), NULL);
            if(v->hFileMap == INVALID_HANDLE_VALUE)
            {
               CloseHandle(v->hFile);
               v->hFile = INVALID_HANDLE_VALUE;
            }
            v->baseAddr = (BYTE *) MapViewOfFile(v->hFileMap, FILE_MAP_READ, 0,0,0);
         }
         obj->lockedData = (void *) (v->baseAddr + obj->fileOffset);
      }
      else if(obj->flags & ResourceObject::File)
      {
         FileRStream sio(buildPath(obj->filePath, obj->fileName));
         obj->fileSize = sio.getSize();
         BYTE *ret = new BYTE[obj->fileSize];
         sio.read(sio.getSize(), ret);
         obj->lockedData = (void *) ret;
      }
   }
   return obj->lockedData;
}

//------------------------------------------------------------------------------
void ResourceManager::unlock(ResourceObject *obj, int msDelay)
{
   if (!obj) return;
   AssertFatal(obj->lockCount > 0, "ResourceManager::unlock: lock count is zero.");
   //set the timeout to the max requested 
   if (--obj->lockCount == 0)
   {
      msDelay = msDelay ? msDelay : obj->type->getTimeoutDelay();
      obj->timeout = max(obj->timeout, (int)GetTickCount()+msDelay );   
      obj->linkAfter(&timeoutList);
   }
}

//------------------------------------------------------------------------------
// gets the crc of the file, ignores the stream type
bool ResourceManager::getCrc(const char * fileName, UInt32 & crcVal, const UInt32 crcInitialVal )
{
   ResourceObject * obj = find(fileName);
   if(!obj) 
      return(false);

   // check if in a volume
   if(obj->flags & (ResourceObject::VolumeBlock | ResourceObject::File))
   {
      FileRStream sio(buildPath(obj->filePath, obj->fileName));

      // read in the info         
      sio.setPosition(obj->fileOffset);
      BYTE * data = new BYTE[obj->fileSize];
      sio.read(obj->fileSize, data);
      
      // get the crc value
      crcVal = CRC::calculate(data, obj->fileSize, crcInitialVal);
      
      delete [] data;
         
      return(true);
   }
   
   return(false);
}

//------------------------------------------------------------------------------

ResourceObject* ResourceManager::load(const char *fileName, bool block)
{
   block;
   ResourceObject *obj = find(fileName);
   if(!obj)
      return NULL;
   
   obj->lockCount++;
   obj->unlink(); // remove from timeout list
   if(!obj->resource)
   {
      if(obj->flags & (ResourceObject::File | ResourceObject::VolumeBlock))
      {
         FileRStream sio(buildPath(obj->filePath, obj->fileName));
         if(obj->flags & ResourceObject::File)
            obj->fileSize = sio.getSize();

         sio.setPosition(obj->fileOffset);
         if(obj->flags & ResourceObject::RLECompressed)
         {
            RLERStream strm(sio);
            obj->construct( &strm, obj->fileSize );
         }
         else if(obj->flags & ResourceObject::LZHCompressed)
         {
            LZHRStream strm(sio);
            obj->construct( &strm, obj->fileSize );
         }
         else
            obj->construct( &sio, obj->fileSize );
         return obj;
      }
      else
      {
         obj->lockCount--;
         return NULL;
      }
   }
   return obj;
}


//------------------------------------------------------------------------------
ResourceObject* ResourceManager::find(const char *fileName)
{
   if(!fileName)
      return NULL;
   StringTableEntry path, file;
   getPaths(fileName, path, file);
   return dictionary.find(path, file);
}


//------------------------------------------------------------------------------
// Add resource constructed outside the manager

bool ResourceManager::add(DWORD resType, const char* name, void *resource,
   bool extraLock)
{
   resType;
   StringTableEntry path, file;
   getPaths(name, path, file);

   ResourceObject* obj = dictionary.find(path, file);
   if (obj && obj->resource)
      // Resource already exists?
      return false;

   if (!obj)
   {
      obj = createResource(path, file, NULL, NULL);
      if(!obj->type)
      {
         freeResource(obj);
         return false;
      }
   }
   dictionary.pushBehind(obj, ResourceObject::File | ResourceObject::VolumeBlock);
   obj->resource = resource;
   obj->lockCount = extraLock ? 2 : 1;
   unlock(obj);
   return true;
}

//------------------------------------------------------------------------------

void ResourceManager::purge( bool force )
{
   int time = GetTickCount();
   bool found;
   do {
      ResourceObject *obj = timeoutList.getNext();
      found = false;
      while(obj)
      {
         ResourceObject *temp = obj;
         obj = obj->next;
         if(force || temp->timeout < time)
         {
            temp->unlink();
            temp->destruct();
            found = true;
            if(temp->flags & ResourceObject::Added)
               freeResource(temp);
         }
      }
   } while(found);
}

//------------------------------------------------------------------------------

void ResourceManager::purge( ResourceObject *obj )
{
   AssertFatal(obj->lockCount == 0, "ResourceManager::purge: handle lock count is not ZERO.")
   obj->unlink();
   obj->destruct();   
}   

//------------------------------------------------------------------------------
ResourceObject* ResourceManager::findMatch(const char *expression, const char ** fn, ResourceObject *start)
{
   if(!start)
      start = resourceList.nextResource;
   else
      start = start->nextResource;
   while(start)
   {
      const char *fname = buildPath(start->path, start->name);
      if(FindMatch::isMatch(expression, fname, false))
      {
         *fn = fname;
         return start;
      }
      start = start->nextResource;
   }
   return NULL;
}

int ResourceManager::findMatches( FindMatch *pFM )
{
   static char buffer[16384];
   int bufl = 0;
   ResourceObject *walk;
   for(walk = resourceList.nextResource; walk && !pFM->isFull(); walk = walk->nextResource)
   {
      const char *fpath = buildPath(walk->path, walk->name);
      if(bufl + strlen(fpath) >= 16380)
         return pFM->numMatches();
      strcpy(buffer + bufl, fpath);
      if(pFM->findMatch(buffer + bufl))
         bufl += strlen(fpath) + 1;
   }
   return ( pFM->numMatches() );
}

//------------------------------------------------------------------------------
bool ResourceManager::findFile(const char *name)
{
   return (bool) find(name);
}

//------------------------------------------------------------------------------
ResourceObject *ResourceManager::createResource(StringTableEntry path, StringTableEntry file, StringTableEntry filePath, StringTableEntry fileName)
{
   ResourceObject *newRO = dictionary.find(path, file, filePath, fileName);
   if(newRO)
      return newRO;
      
   newRO = new ResourceObject;
   newRO->path = path;
   newRO->name = file;
   newRO->type = ResourceType::find(ResourceType::typeof(newRO->name));
   newRO->lockCount = 0;
   newRO->resource = NULL;
   newRO->lockedData = NULL;
   newRO->flags = ResourceObject::Added;
   newRO->next = newRO->prev = NULL;
   newRO->nextResource = resourceList.nextResource;
   resourceList.nextResource = newRO;
   newRO->prevResource = &resourceList;
   if(newRO->nextResource)
      newRO->nextResource->prevResource = newRO;
   dictionary.insert(newRO, path, file);
   newRO->fileSize = newRO->fileOffset = 0;
   newRO->filePath = filePath;
   newRO->fileName = fileName;
   newRO->timeout = GetTickCount();
   newRO->rm = this;
   newRO->volume = NULL;

   return newRO;
}

//------------------------------------------------------------------------------
void ResourceManager::freeResource(ResourceObject *ro)
{
   ro->destruct();
   ro->unlink();
   if((ro->flags & ResourceObject::File) && ro->lockedData)
      delete[] ro->lockedData;
   if(ro->prevResource)
      ro->prevResource->nextResource = ro->nextResource;
   if(ro->nextResource)
      ro->nextResource->prevResource = ro->prevResource;
   dictionary.remove(ro);
   delete ro;
}


static ResourceTypeRawData       _resDat(".dat");
static ResourceTypeStaticRawData _resSDt(".sdt");

//------------------------------------------------------------------------------
// simple crc function - generates lookup table on first call
UInt32 CRC::calculate(void * buffer, int len, UInt32 crcVal )
{
   static bool    tableValid = false;
   static UInt32  table[256];
   
   // check if need to generate the crc table
   if(!tableValid)
   {
      UInt32 val;
   
      for(int i = 0; i < 256; i++)
      {
         val = i;
         for(int j = 0; j < 8; j++)
         {
            if(val & 0x01)
               val = 0xedb88320 ^ (val >> 1);
            else
               val = val >> 1;
         }
         table[i] = val;
      }
      
      tableValid = true;
   }
   
   // now calculate the crc
   char * buf = (char*)buffer;
   for(int i = 0; i < len; i++)
      crcVal = table[(crcVal ^ buf[i]) & 0xff] ^ (crcVal >> 8);
   return(crcVal);
}
