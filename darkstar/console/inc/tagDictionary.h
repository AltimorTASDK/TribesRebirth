//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _TAGDICTIONARY_H_
#define _TAGDICTIONARY_H_

#include "stringTable.h"
#include "tVector.h"

class StreamIO;

class TagDictionary
{
   struct TagEntry
   {
      int id;
      StringTableEntry define;
      StringTableEntry string;
      TagEntry *chain; // for linear traversal
      TagEntry *defineHashLink;
      TagEntry *idHashLink;
   };

   TagEntry **defineHashBuckets;
   TagEntry **idHashBuckets;

   TagEntry *entryChain;
   DataChunker mempool;
   int numBuckets;
   int numEntries;

   bool match(const char* pattern, const char* str);
   void sortIdVector(Vector<int>& out_v);
public:
   TagDictionary();
   ~TagDictionary();
   
	//IO functions
	//
   int writeHeader(StreamIO &);

   // String/Define retrieval and search functions...
   //

   bool addEntry(int value, StringTableEntry define, StringTableEntry string);
   
   StringTableEntry defineToString(StringTableEntry tag);
   StringTableEntry idToString(int tag);
   StringTableEntry idToDefine(int tag);
   int defineToId(StringTableEntry tag);

   // get IDs such that minID < IDs < maxID 
   void findIDs( Vector<int> &v, const int minID, const int maxID );
	void findStrings( Vector<int> &v, const char *pattern);
	void findDefines( Vector<int> &v, const char *pattern);
};

extern TagDictionary tagDictionary;

#endif //_TAGDICTIONARY_H_
