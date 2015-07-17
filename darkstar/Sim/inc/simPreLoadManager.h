//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMPRELOADMANAGER_H_
#define _SIMPRELOADMANAGER_H_

#include <types.h>
#include <stringTable.h>
#include <resManager.h>
#include "simBase.h"
#include <ts_material.h>

class SimPreLoadManager : public SimObject
{
private:
   typedef SimObject Parent;

protected:
   enum Constants
   {
      INIT_SIZE = 29,
   };
   
   struct Node
   {
      StringTableEntry name;
      ResourceObject *obj;
      Node *next;
   };

   Node **buckets;
   int numBuckets;
   int itemCount;

   DataChunker mempool;

   Int32 hashString(StringTableEntry string);
   void insert(StringTableEntry string, ResourceObject *obj);
   bool lookup(StringTableEntry string);
   void resize(int newSize);

   void onRemove();

public:
   SimPreLoadManager();
   ~SimPreLoadManager();

   static SimPreLoadManager *get(SimManager *manager);

   void preLoad(const char *filename, bool delay = false);                     
   void preLoadMaterialList(const char *materialListName, bool delay = false);
   void preLoadMaterialList(const TS::MaterialList *materialList, bool delay = false);
   void preLoadTSShape(const char *tsshapeName, bool delay = false);
   
   void preLoadDelayed();

   void getPreLoadedList(Vector<StringTableEntry> &list);
   void echoPreLoadedList();
};

//--------------------------------------------------------------------------- 
inline SimPreLoadManager *SimPreLoadManager::get(SimManager *manager)
{
   return dynamic_cast<SimPreLoadManager *>(manager->findObject(SimPreLoadManagerId));
}


#endif //_SIMPRELOADMANAGER_H_
