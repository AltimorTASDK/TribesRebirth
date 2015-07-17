//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMEXPLOSIONTABLE_H_
#define _SIMEXPLOSIONTABLE_H_

//Includes
#include <tSorted.h>
#include <simBase.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#include "simExplosionRecord.h"

class SimExplosionTable : public SimObject
{
public:
	struct Entry
	{
		SimExplosionRecord entry;
		int operator<(const Entry & e) const { return e.entry.megatons > entry.megatons; }
	};

	struct ShortListEntry
	{
		int index;
		Int32 megatons;
	};
	typedef Vector<ShortListEntry> ShortList;

private:
	SortableVector<Entry> table;

	bool load(const char * fileName);

	const SimExplosionRecord* lookup(Int32 id);
	const SimExplosionRecord* get(int index);
	void  add(const SimExplosionRecord & item);
	void  getShortList(float megatons, Int32 typeMask, ShortList & sl);

public:
	SimExplosionTable();
	~SimExplosionTable();

	static bool load(int tableTagID,SimManager * mgr);
	static bool load(const char * fileName,SimManager * mgr);

	static const SimExplosionRecord* lookup(Int32 id, SimManager * mgr);
	static const SimExplosionRecord* get(int id, SimManager * mgr);
	static void  add(const SimExplosionRecord & item, SimManager *);
	static void  getShortList(float megatons, Int32 typeMask, ShortList & sl, SimManager *);
	static bool  loaded(SimManager * mgr) { return find(mgr); }
	static SimExplosionTable* find(SimManager*);

   void onPreLoad(SimPreLoadManager *splm);

	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
   
   bool processArguments(int, const char**);

   DECLARE_PERSISTENT(SimExplosionTable);
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SIMEXPLOSIONTABLE_H_
