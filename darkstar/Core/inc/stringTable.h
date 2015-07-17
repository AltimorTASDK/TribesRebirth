#ifndef _H_STRINGTABLE
#define _H_STRINGTABLE


class DataChunker
{
public:
   enum {
      ChunkSize = 16376,
   };
private:
   struct DataBlock
   {
      DataBlock *next;
      BYTE *data;
      int curIndex;
      DataBlock(int size);
      ~DataBlock();
   };
   DataBlock *curBlock;
   int chunkSize;
public:
   void *alloc(int size);
   void freeBlocks();

   DataChunker(int size=ChunkSize);
   ~DataChunker();
};

class StringTable
{
   struct Node
   {
      char *val;
      Node *next;
   };

   Node **buckets;
   int numBuckets;
   int itemCount;
   DataChunker mempool;
public:
   StringTableEntry insert(const char *string, bool caseSens = false);
   StringTableEntry lookup(const char *string, bool caseSens = false);
   void resize(int newSize);
   StringTable();
   ~StringTable();
};

extern StringTable stringTable;
extern unsigned int HashString(const char *string);

#endif