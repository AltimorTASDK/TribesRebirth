//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMDICTIONARY_H_
#define _SIMDICTIONARY_H_
#include <types.h>

class SimObject;

//----------------------------------------------------------------------------
// Map of Names to simObjects
// Provides fast lookup for name->object and
// for fast removal of an object given object*
//
class DLLAPI SimNameDictionary
{
   enum
   {
      DefaultTableSize = 29,
   };

   SimObject **hashTable;  // hash the pointers of the names...
   int hashTableSize;
   int hashEntryCount;
public:
	void insert(SimObject* obj);
	void remove(SimObject* obj);
	SimObject* find(StringTableEntry name);

	SimNameDictionary();
	~SimNameDictionary();
};

//----------------------------------------------------------------------------
// Map of ID's to simObjects
// Provides fast lookup for ID->object and
// for fast removal of an object given object*
//
class DLLAPI SimIdDictionary
{
   enum
   {
      DefaultTableSize = 4096,
      TableBitMask = 4095,
   };
   SimObject *table[DefaultTableSize];
public:
	void insert(SimObject* obj);
	void remove(SimObject* obj);
	SimObject* find(int id);

	SimIdDictionary();
	~SimIdDictionary();
};

#endif //_SIMDICTIONARY_H_
