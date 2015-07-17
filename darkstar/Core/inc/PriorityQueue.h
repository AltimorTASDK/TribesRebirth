//
//    PriorityQueue.h
//  
//    Implements a priority queue of pointers to user-defined Items.  The two
//    motivations for creating this queue type in addition to the BinaryHeap
//    are that 1) the binary heap moves objects around which could be slow
//    for large objects, and 2) we need a way for object keys to be changed
//    efficiently even though we don't know where in the heap the object is.
//    Djikstra's algorithm, for example, has this requirement.  Dijkstra() was 
//    our original motivation for this implementation, which requires the consumer 
//    to use or derive from a PriorityQueue<KeyType>::Item, which maintains a back 
//    pointer, and which encapsulates changes to the key (changeKey()).  See
//    aiGraph.h & aiDijkstra.cpp for this example.  
// 
//    There are two main caveats of use:
//    a) The queue requires that the items it refers to exist and stay in 
//       one place.  Assuring that the items get properly destructed should
//       avoid most problems.  Alternatively, if the Item is extracted (or
//       the queue is flushed), then an item destructor doesn't need to 
//       happen.  Care should be taken if installing Items that are in a 
//       Vector<OfYourItemType> since a realloc() can occur when new items
//       are added to the vector and then the queue can point at wrong memory.  
//    b) An Item can only be installed in one Queue.  Installing in multiple
//       queues can be done by having multiple Item members in an object.  See
//       the AI::Role item type (aiObj.h) which derives from two Item types
//       to maintain itself in two queues (a schedule Q and an importance Q).  
//
#ifndef _PRIORITYQUEUE_H_
#define _PRIORITYQUEUE_H_

#include <tBinaryHeap.h>

template <class KeyType,bool isMinQ=false>  class PriorityQueue
{
   public:
      class Item;
      friend Item;
      
   private:
      // We keep a binary heap of these which refer to the actual items the user
      // wants queued.  The user items (which must derive from or use Item, below) 
      // stay in one place, while the ItemRefs shuffle around.  Items keep a back 
      // pointer to their ItemRef, which is modified only when movements happen.
      struct ItemRef
      {
         Item * itemPtr;
         ItemRef()                        { itemPtr = 0; }
         ItemRef(Item & item)             { itemPtr = & item; }
         operator<(const ItemRef & ip)    
         { 
            return isMinQ ^ (itemPtr->key < ip.itemPtr->key); 
         }
      };
      
   public:
      // We are managing a list of items which derive from this base class.  
      // We control access to the key since we need to alter the Queue when
      // it changes.  
      class Item
      {
         private:
            friend            ItemRef;
            friend            PriorityQueue;
            ItemRef           * backPtr;
            PriorityQueue     * queue;
            KeyType           key;
            
         public:
            Item(KeyType k1)     {  backPtr = 0; queue = 0; key = k1;   }
            Item()               {  backPtr = 0; queue = 0;             }
            ~Item()              {  if(backPtr) remove();               }
            bool isInQ(void) { return (backPtr != NULL); }
            KeyType getKey() { return key; }
            void remove();
            void add( PriorityQueue & q );
            void insert( PriorityQueue & q );
            bool changeKey( KeyType newKey );
            virtual int userProc( int x=0, void * v=0 ); //might come in handy....
      };
      
   private:
      void fixAllBackPtrs(void);
      void fixPercolated(void);
      void fixBackPointers(void);

   protected:
      BinaryHeap<ItemRef>     Q;
      
   public:
      PriorityQueue() {}
      ~PriorityQueue();
      void MakeQueue(void);
      void add(Item & item);
      void insert(Item & item);
      void remove(Item & item);
      Item * extractHead(void);
      Item * examineHead(void);
      int  size() { return Q.heapSize(); }
};

// On deletion we must go modify all objects referred to by the itemRefs - just
// remove their back pointers so the items will destruct properly.  
template <class KeyType,bool isMinQ>
PriorityQueue<KeyType,isMinQ>::~PriorityQueue()
{
   int   i = Q.arraySize();
   while( i-- ){
      Item  * itemPtr = Q[i].itemPtr;
      itemPtr->backPtr = 0;
      itemPtr->queue = 0;
   }
}

template <class KeyType,bool isMinQ> 
void PriorityQueue<KeyType,isMinQ>::fixAllBackPtrs(void)
{
   register int i = Q.heapSize();
   while( i-- )
      Q[i].itemPtr->backPtr = &Q[i];
}
template <class KeyType,bool isMinQ> 
inline void PriorityQueue<KeyType,isMinQ>::fixPercolated(void)
{
   register int i = Q.movedCount;
   while( i-- ){
      register ItemRef  * refPtr = & Q [ Q.movedList[i] ];
      refPtr->itemPtr->backPtr = refPtr;
   }
}
// Called after any operation that might have moved things around.  The heap
// provides a couple of members which are set after operations.  The arrayMoved
// flag is set whenever the Vector's data moves (and note that it doesn't get
// UNset - it's there for classes that know what they're doing, plus the 
// consuming class needs to decide when to deal with it).  
template <class KeyType,bool isMinQ> 
inline void PriorityQueue<KeyType,isMinQ>::fixBackPointers(void)
{
   if( Q.arrayMoved )
      fixAllBackPtrs(), Q.arrayMoved = false;
   else if( Q.movedCount > 0 )
      fixPercolated();
}

// The add() method, at least for now, can only be used for FIRST additions on an empty
// array, and then the heap must be built.  After that insert() must be used (which can 
// also be used from the beginning). It is slightly more efficient to add() many elements
// and then make the queue (build the heap). But insert() is reasonable (log time)
template <class KeyType,bool isMinQ> 
void PriorityQueue<KeyType,isMinQ>::add(Item & item)
{
   ItemRef  itemRef(item);
   item.queue = this;
   Q.add( itemRef );
}
template <class KeyType,bool isMinQ> 
void PriorityQueue<KeyType,isMinQ>::MakeQueue(void)
{ 
   Q.buildHeap();
   fixBackPointers();
}

template <class KeyType,bool isMinQ> 
void PriorityQueue<KeyType,isMinQ>::insert(Item & item)
{
   ItemRef  itemRef(item);
   item.queue = this;
   Q.insert( itemRef );
   fixBackPointers();
}

template <class KeyType,bool isMinQ> 
void PriorityQueue<KeyType,isMinQ>::remove(Item & item)
{
   ItemRef * itemRefPtr = item.backPtr;
   if( itemRefPtr )
   {
      item.backPtr = 0;
      item.queue = 0;
      Q.remove( itemRefPtr );
      fixBackPointers();
   }
}

template <class KeyType,bool isMinQ> 
void PriorityQueue<KeyType,isMinQ>::Item::remove()
{
   if( queue )
      queue->remove( * this );
}

template <class KeyType,bool isMinQ>
void PriorityQueue<KeyType,isMinQ>::Item::add( PriorityQueue & q )
{
   q.add( * this );
}

template <class KeyType,bool isMinQ>
void PriorityQueue<KeyType,isMinQ>::Item::insert( PriorityQueue & q )
{
   q.insert( * this );
}

// Change the key of the item and re-order it in the queue.  Return true
// if the POSITION in the queue changed, otherwise false.  It can 
// also be called for elements that aren't in a queue currently (returns
// false).  
template <class KeyType,bool isMinQ>
bool PriorityQueue<KeyType,isMinQ>::Item::changeKey( KeyType newKey )
{
   bool positionChanged = false;
   if( key != newKey ){
      key = newKey;
      if( backPtr ){
         queue->Q.change( backPtr );
         if( queue->Q.movedCount > 0 )
            positionChanged = true;
         queue->fixBackPointers();
      }
   }
   return positionChanged;
}

//The user might want a callback, such as to handle whatever needs to be done with
//the item at the head of the list.  
template <class KeyType,bool isMinQ>
int PriorityQueue<KeyType,isMinQ>::Item::userProc( int x, void * v )
{
   return v ? x : 0;
}

template <class KeyType,bool isMinQ>
PriorityQueue<KeyType,isMinQ>::Item * 
PriorityQueue<KeyType,isMinQ>::extractHead(void)
{
   Item * itemPtr = NULL;
   if( Q.heapSize() ){
      itemPtr = Q.extractMax().itemPtr;
      itemPtr->backPtr = 0;
      itemPtr->queue = 0;
      fixBackPointers();
   }
   return itemPtr;
}

template <class KeyType,bool isMinQ>
PriorityQueue<KeyType,isMinQ>::Item * 
PriorityQueue<KeyType,isMinQ>::examineHead(void)
{
   return( Q.heapSize() ? Q.returnMax().itemPtr : NULL );
}

#endif

