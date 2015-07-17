//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <simDictionary.h>
#include <simBase.h>

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
extern int HashPointer(StringTableEntry e);

SimNameDictionary::SimNameDictionary()
{
   hashTable = new SimObject *[DefaultTableSize];
   hashTableSize = DefaultTableSize;
   hashEntryCount = 0;
   int i;
   for(i = 0; i < hashTableSize; i++)
      hashTable[i] = NULL;
}

SimNameDictionary::~SimNameDictionary()
{
   delete[] hashTable;
}

void SimNameDictionary::insert(SimObject* obj)
{
   if(!obj->objectName)
      return;

   int idx = HashPointer(obj->objectName) % hashTableSize;
   obj->nextNameObject = hashTable[idx];
   hashTable[idx] = obj;
   hashEntryCount++;
   if(hashEntryCount > hashTableSize)
   {
      // resize the hash table
      int i;
      SimObject *head = NULL, *walk, *temp;
   	for(i = 0; i < hashTableSize; i++) {
   		walk = hashTable[i];
         while(walk)
         {
            temp = walk->nextNameObject;
            walk->nextNameObject = head;
            head = walk;
            walk = temp;
         }
   	}
      delete[] hashTable;
      hashTableSize = hashTableSize * 2 + 1;
      hashTable = new SimObject *[hashTableSize];
      
      for(i = 0; i < hashTableSize;i++)
         hashTable[i] = NULL;
      while(head)
      {
         temp = head->nextNameObject;
         idx = HashPointer(head->objectName) % hashTableSize;
         head->nextNameObject = hashTable[idx];
         hashTable[idx] = head;
         head = temp;
      }
   }
}

SimObject* SimNameDictionary::find(StringTableEntry name)
{
   // NULL is a valid lookup - it will always return NULL

   int idx = HashPointer(name) % hashTableSize;
   SimObject *walk = hashTable[idx];
   while(walk)
   {
      if(walk->objectName == name)
         return walk;
      walk = walk->nextNameObject;
   }
   return NULL;
}

void SimNameDictionary::remove(SimObject* obj)
{
   if(!obj->objectName)
      return;

   SimObject **walk = &hashTable[HashPointer(obj->objectName) % hashTableSize];
   while(*walk)
   {
      if(*walk == obj)
      {
         *walk = obj->nextNameObject;
			obj->nextNameObject = (SimObject*)-1;
         hashEntryCount--;
         return;
      }
      walk = &((*walk)->nextNameObject);
   }
}	

//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

SimIdDictionary::SimIdDictionary()
{
   for(int i = 0; i < DefaultTableSize; i++)
      table[i] = NULL;
}

SimIdDictionary::~SimIdDictionary()
{
}

void SimIdDictionary::insert(SimObject* obj)
{
   int idx = obj->id & TableBitMask;
   obj->nextIdObject = table[idx];
   table[idx] = obj;
}

SimObject* SimIdDictionary::find(int id)
{
   int idx = id & TableBitMask;
   SimObject *walk = table[idx];
   while(walk)
   {
      if(walk->id == DWORD(id))
         return walk;
      walk = walk->nextIdObject;
   }
   return NULL;
}

void SimIdDictionary::remove(SimObject* obj)
{
   SimObject **walk = &table[obj->id & TableBitMask];
   while(*walk && *walk != obj)
      walk = &((*walk)->nextIdObject);
   if(*walk)
      *walk = obj->nextIdObject;
}

//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

