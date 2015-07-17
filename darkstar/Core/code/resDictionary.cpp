//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <core.h>
#include "ResManager.h"
#include <stdlib.h>
#include "talgorithm.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

ResDictionary::ResDictionary()
{
   entryCount = 0;
   hashTableSize = 1023; //DefaultTableSize;
   hashTable = new ResourceObject *[hashTableSize];
   int i;
   for(i = 0; i < hashTableSize; i++)
      hashTable[i] = NULL;
}

ResDictionary::~ResDictionary()
{
   // we assume the resources are purged before we destroy
   // the dictionary

   delete[] hashTable;
}

int ResDictionary::hash(StringTableEntry path, StringTableEntry file)
{
   return ((DWORD(path) >> 2) + (DWORD(file) >> 2) ) % hashTableSize;
}

void ResDictionary::insert(ResourceObject *obj, StringTableEntry path, StringTableEntry file)
{
   obj->name = file;
   obj->path = path;
   obj->timeout = GetTickCount();

   int idx = hash(path, file);
   obj->nextEntry = hashTable[idx];
   hashTable[idx] = obj;
   entryCount++;
   
   if(entryCount > hashTableSize) {
      ResourceObject *head = NULL, *temp, *walk;
      for(idx = 0; idx < hashTableSize;idx++) {
         walk = hashTable[idx];
         while(walk)
         {
            temp = walk->nextEntry;
            walk->nextEntry = head;
            head = walk;
            walk = temp;
         }
      }
      delete[] hashTable;
      hashTableSize = 2 * hashTableSize - 1;
      hashTable = new ResourceObject *[hashTableSize];
      for(idx = 0; idx < hashTableSize; idx++)
         hashTable[idx] = NULL;
      walk = head;
      while(walk) 
      {
         temp = walk->nextEntry;
         idx = hash(walk);
         walk->nextEntry = hashTable[idx];
         hashTable[idx] = walk;
         walk = temp;
      }
   }  
}

ResourceObject* ResDictionary::find(StringTableEntry path, StringTableEntry name)
{
   for(ResourceObject *walk = hashTable[hash(path, name)]; walk; walk = walk->nextEntry)
      if(walk->name == name && walk->path == path)
         return walk;
   return NULL;
}

ResourceObject* ResDictionary::find(StringTableEntry path, StringTableEntry name, StringTableEntry filePath, StringTableEntry fileName)
{
   for(ResourceObject *walk = hashTable[hash(path, name)]; walk; walk = walk->nextEntry)
      if(walk->name == name && walk->path == path && walk->fileName == fileName && walk->filePath == filePath)
         return walk;
   return NULL;
}

void ResDictionary::pushBehind(ResourceObject *resObj, int flagMask)
{
   remove(resObj);
   entryCount++;
   ResourceObject **walk = &hashTable[hash(resObj)];
   for(; *walk; walk = &(*walk)->nextEntry)
   {
      if(!((*walk)->flags & flagMask))
      {
         resObj->nextEntry = *walk;
         *walk = resObj;
         return;
      }
   }
   resObj->nextEntry = NULL;
   *walk = resObj;
}

void ResDictionary::remove(ResourceObject *resObj)
{
   for(ResourceObject **walk = &hashTable[hash(resObj)]; *walk; walk = &(*walk)->nextEntry)
   {
      if(*walk == resObj)
      {
         entryCount--;
         *walk = resObj->nextEntry;
         return;
      }
   }
}
