//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <SimTagDictionary.h>
#include <simPreLoadManager.h>
#include "simExplosionTable.h"
#include "simResource.h"
#include "console.h"
#include "coreRes.h"

//------------------------------------------------------------------------------
// SimExplosionTable
//------------------------------------------------------------------------------

SimExplosionTable::SimExplosionTable()
{
   id = SimDefaultExplosionTableId;
}

SimExplosionTable::~SimExplosionTable()
{
}

bool SimExplosionTable::processEvent(const SimEvent *)
{
   return ( false );
}

bool SimExplosionTable::processQuery(SimQuery *)
{
   return ( false );
}

SimExplosionTable* SimExplosionTable::find(SimManager* io_pManager)
{
   AssertFatal(io_pManager != NULL, "No manager to find table!");

   return (SimExplosionTable*)io_pManager->findObject(SimDefaultExplosionTableId);
}

const SimExplosionRecord* SimExplosionTable::lookup(Int32 id, SimManager* io_pManager)
{
	SimExplosionTable * et = SimExplosionTable::find(io_pManager);
	AssertFatal(et,"SimExplosionTable::lookup: no explosion table loaded");
	return et->lookup(id);
}

const SimExplosionRecord* SimExplosionTable::lookup(Int32 id)
{
	for (int i=0; i<table.size(); i++)
		if (table[i].entry.id==id)
			return &table[i].entry;
	return 0;
}

const SimExplosionRecord* SimExplosionTable::get(int index, SimManager* io_pManager)
{
	SimExplosionTable * et = SimExplosionTable::find(io_pManager);
	AssertFatal(et,"SimExplosionTable::get: no explosion table loaded");
	return et->get(index);
}

const SimExplosionRecord* SimExplosionTable::get(int index)
{
	AssertFatal(index<table.size(),"SimExplosionTable::get:  explosion index out of range.");
	return &table[index].entry;
}

void SimExplosionTable::add(const SimExplosionRecord & item, SimManager* io_pManager)
{
	SimExplosionTable * et = SimExplosionTable::find(io_pManager);
	if (!et)
		et = new SimExplosionTable();
	et->add(item);
}

void SimExplosionTable::add(const SimExplosionRecord & item)
{
	table.increment();
	Entry & newEntry = table.last();
	newEntry.entry = item;
	table.sort();
}

void SimExplosionTable::getShortList(float megatons, Int32 typeMask, ShortList & sl, SimManager* io_pManager)
{
	SimExplosionTable * et = SimExplosionTable::find(io_pManager);
	AssertFatal(et,"SimExplosionTable::getShortList: no explosion table loaded");
	et->getShortList(megatons,typeMask,sl);
}

void SimExplosionTable::getShortList(float megatons, Int32 typeMask, ShortList & sl)
{
	sl.clear();

	int i;
	for (i=0;i<table.size();i++)
	{
		SimExplosionRecord & rec = table[i].entry;
		if (rec.megatons>megatons)
			break;
		if (rec.type&typeMask)
		{
			sl.increment();
			ShortListEntry & sle = sl.last();
			sle.megatons = rec.megatons;
			sle.index = i;
		}
	}
}

bool SimExplosionTable::load(int tableTagID,SimManager * mgr)
{
	const char* tableName = SimTagDictionary::getString(mgr,tableTagID);
	return load(tableName,mgr);
}

bool SimExplosionTable::load(const char * fileName,SimManager *mgr)
{
	SimExplosionTable * et = SimExplosionTable::find(mgr);
	if (!et)
	{
		et = new SimExplosionTable();
		mgr->addObject(et);
	}
		
	return et->load(fileName);
}

bool SimExplosionTable::load(const char * fileName)
{
	Resource<RawData> rawData = SimResource::get(manager)->load(fileName);

	if (!(bool)rawData)
		return false;

   SimExplosionRecord *data = (SimExplosionRecord *) (*rawData).data;
	int sz = (*rawData).size / sizeof(SimExplosionRecord);
	SimExplosionRecord *end  = data + sz;

	for (;data!=end; data++)
	{
		table.increment();
		table.last().entry = *data;
	}

	table.sort();

	return true;
}

bool SimExplosionTable::processArguments(int          io_argc,
                                         const char** in_argv)
{
   if(io_argc != 1) {
      Console->printf("SimExplosionTable: <explosionTableName>.dat");
      return false;
   }

   if(load(in_argv[0]) == false) {
      Console->printf("SimExplosionTable: unable to load table %s", in_argv[0]);
      return false;
   }

   return true;
}

void SimExplosionTable::onPreLoad(SimPreLoadManager *splm)
{
   const char *name;

   for (int i =0; i < table.size(); i++)
   {
      SimExplosionRecord &entry = table[i].entry;
      if (entry.shapeTag)
      {
         name = SimTagDictionary::getString(entry.shapeTag);
         if (name) splm->preLoadTSShape(name);
      }
      if (entry.translucentShapeTag)
      {
         name = SimTagDictionary::getString(entry.translucentShapeTag);
         if (name) splm->preLoadTSShape(name);
      }
   }
}   