//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simPreLoadManager.h"
#include "simResource.h"
#include <ts_shape.h>
#include <console.h>

//------------------------------------------------------------------------------
SimPreLoadManager::SimPreLoadManager()
{   
   id = SimPreLoadManagerId;
   buckets = NULL;
   numBuckets = 0;
   itemCount = 0;

   buckets = (Node **) malloc(INIT_SIZE * sizeof(Node *));
   AssertFatal(buckets, "could not allocate mem");
 	for(int i = 0; i < INIT_SIZE; i++) {
		buckets[i] = NULL;
	}
	numBuckets = INIT_SIZE;
	itemCount = 0;
}   

//------------------------------------------------------------------------------
SimPreLoadManager::~SimPreLoadManager()
{
} 

//------------------------------------------------------------------------------
void SimPreLoadManager::onRemove()
{
   Parent::onRemove();
   // go through each node and free its resource   
   for (int i = 0; i < numBuckets; i++)
   {
      Node *walk = buckets[i];
      while (walk)
      {
         if (walk->obj)
            walk->obj->rm->unlock(walk->obj, RES_DEFAULT_TIMEOUT);
         walk->obj = NULL;
         walk = walk->next;
      }
   }
   free(buckets);
   // memory for the nodes are freed by the data chuncker
}  
 
//------------------------------------------------------------------------------
Int32 SimPreLoadManager::hashString(StringTableEntry string)
{
   AssertFatal(string, "invalid param");
   Int32 address = (Int32)string;
   address = address >> 2; // get rid of the word align zeros
   return address % numBuckets;
}   

//------------------------------------------------------------------------------
void SimPreLoadManager::insert(StringTableEntry string, ResourceObject *obj)
{
   AssertFatal(string, "invalid param");

   Node *newNode = (Node *)mempool.alloc(sizeof(Node));
   newNode->name = string;
   newNode->obj = obj;

 	Int32 index = hashString(string);
	newNode->next = buckets[index];
   buckets[index] = newNode;
   itemCount++;
   if (itemCount > 2 * numBuckets)
      resize(4 * numBuckets - 1); // hopefully a prime number
}  

//------------------------------------------------------------------------------ 
bool SimPreLoadManager::lookup(StringTableEntry string)
{
	Int32 index = hashString(string);
	Node *walk = buckets[index];
	while (walk != NULL)	
	{
      if (walk->name == string)
         return true;
		walk = walk->next;
	}
   return false;
}   

//------------------------------------------------------------------------------
void SimPreLoadManager::resize(int newSize)
{
   Node *head = NULL, *walk, *temp;
   int i;

   for (i = 0; i < numBuckets; i++)
   {
      walk = buckets[i];
      while (walk)
      {
         temp = walk->next;
         walk->next = head;
         head = walk;
         walk = temp;
      }
   }

	buckets = (Node **) realloc(buckets, newSize * sizeof(Node));
 	for(i = 0; i < newSize; i++) {
		buckets[i] = 0;
	}

	numBuckets = newSize;
   while (head) 
   {
		Node *temp = head;
		
		head = head->next;
		int index = hashString(temp->name);
		temp->next = buckets[index];
		buckets[index] = temp;
   }
}   

//------------------------------------------------------------------------------
void SimPreLoadManager::getPreLoadedList(Vector<StringTableEntry> &list)
{
   list.clear();
   
   for (int i = 0; i < numBuckets; i++)
   {
      Node *walk = buckets[i];
      while (walk)
      {
         list.push_back(walk->name);
         walk = walk->next;
      }
   }
}  

//------------------------------------------------------------------------------
void SimPreLoadManager::echoPreLoadedList()
{
   Console->printf("Preloaded %d files:", itemCount);
   
   for (int i = 0; i < numBuckets; i++)
   {
      Node *walk = buckets[i];
      while (walk)
      {
         Console->printf("  %s", walk->name);
         walk = walk->next;
      }
   }
}  
 
//------------------------------------------------------------------------------  
void SimPreLoadManager::preLoad(const char *filename, bool delay)
{
   StringTableEntry name = stringTable.insert(filename);
   if (lookup(name))
      return; // already preloaded

   ResourceObject *obj = (delay) ? NULL : SimResource::loadByName(manager, name, true);
   insert(name, obj);
}   

//------------------------------------------------------------------------------
void SimPreLoadManager::preLoadMaterialList(const char *materialListName, bool delay)
{
   StringTableEntry name = stringTable.insert(materialListName);
   if (lookup(name))
      return; // already preloaded
 
   Resource<TS::MaterialList> hMaterialList = SimResource::loadByName(manager, name, true);
   if (!(bool)hMaterialList)
      return;

   preLoad(name, delay);
   preLoadMaterialList(hMaterialList, delay);
}

//------------------------------------------------------------------------------
void SimPreLoadManager::preLoadMaterialList(const TS::MaterialList *materialList, bool delay)
{
   AssertFatal(materialList, "invalid param");
   for (int k = 0; k < materialList->getDetailsCount(); k++)
   {
      for (int i = 0; i < materialList->getMaterialsCount(); i++)
      {
         TS::Material mat = materialList->getMaterial(k, i);
         if ((mat.fParams.fFlags & TS::Material::MatFlags) == TS::Material::MatTexture && mat.fParams.fMapFile)
            preLoad(mat.fParams.fMapFile, delay);
      }
   }
}   

//------------------------------------------------------------------------------
void SimPreLoadManager::preLoadTSShape(const char *tsshapeName, bool delay)
{
   StringTableEntry name = stringTable.insert(tsshapeName);
   if (lookup(name))
      return; // already preloaded

   Resource<TS::Shape> hTSShape = SimResource::loadByName(manager, name, true);
   if (!(bool)hTSShape)
      return;

   preLoad(name, delay);
   if (hTSShape->getMaterialList())
      preLoadMaterialList(hTSShape->getMaterialList(), delay);
}   

//------------------------------------------------------------------------------
void SimPreLoadManager::preLoadDelayed()
{
   for (int i = 0; i < numBuckets; i++)
   {
      Node *walk = buckets[i];
      while (walk)
      {
         if (!walk->obj)
            walk->obj = SimResource::loadByName(manager, walk->name, true);
         walk = walk->next;
      }
   }
}   

