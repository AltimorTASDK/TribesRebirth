/*
*  tBinaryHeap.h
*
*  Basic binary heap for use in implementing priority queues.  The binary 
*  heap can be used directly as a priority queue on any item that defines 
*  operator <, and the heap keeps the maximum element at the start of the 
*  arrray.  
* 
*  A more full featured PriorityQueue class is defined elsewhere (on top of
*  this one) that queues pointers to Items.  See PriorityQueue.h.  
* 
*  Most of this comes straight out of the standard text of "Introduction to 
*  Algorithms" by Cormen, Leiserson, & Rivest, which is what section 
*  numbers below refer to.  
*
*/
#ifndef _TBINARYHEAP_H_
#define _TBINARYHEAP_H_

#include <tVector.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif
#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)
#pragma warn -inl
#endif

#define  HeapParent(i)     ( ( (i) - 1 ) >> 1 )
#define  HeapLeft(i)       ( ( (i) << 1 ) + 1 )
#define  HeapRight(i)      ( ( (i) + 1 ) << 1 )

// Determines maximum amount that can be contained (2^HeapMaxDepth-1). 
#define  HeapMaxDepth   22

template <class T> 
class BinaryHeap 
{
   private:
      //the invariant is that heapSz tells how many elements obey the 
      //"heap property", whereas A.size() gives how many are in the list.
      //the most efficient way to get a heap started is to add() all 
      //the elements, and then call buildHeap(). 
      Vector<T>      A;
      int            heapSz;

   private:
      //utility routines.  heapify() is the main workhorse routine which 
      //percolates down if A[i] doesn't meet heap property (but children do).  
      //findSlotUp() percolates UP.  these two are the only ones which move
      //things arround (and hence fill the movedList array).  
      int  heapify(int i);
      void findSlotUp(const T & item, int i);
      
      // bracket that goes within add and remove methods to set movedCount 
      // to -1 if the vector array shifts.  
      T * oldAddr;
      startArrayCheck(void)   { oldAddr=A.address(); }
      finishArrayCheck(void)  { if(oldAddr != A.address()) arrayMoved = true; }
   
   public:
      BinaryHeap() : A(100) {   
         movedCount = heapSz = 0; 
         arrayMoved = false;
         for( int i = 0; i < HeapMaxDepth; i++ )
            movedList[i] = -1;
      }
      
      // add() only post-pends, insert preserves heap property   
      void add( const T & elem );
      void insert(const T & item);
      void remove(int i);
      void remove(const T * item)         { remove( item - A.address() );  }
      void change(int i);
      void change(const T * item)         { change( item - A.address() );  }
      void change(int i, const T & Val)   { A[i] = Val;  change(i);        }
      void buildHeap(void);
      
      // extract removes front element, return leaves it there.  
      T extractMax(void);
      const T & returnMax(void);
      int arraySize(void)                 { return A.size();   }
      int heapSize(void)                  { return heapSz;     }
      const T & operator[](int i) const   { return A[i];       }
      T & operator[](int i)               { return A[i];       }

      // once a heap is built, operations which result in data shifting around
      // will fill in the movedList.  arrayMoved will indicate a shift of the 
      // vector array (namely on additions, where a realloc() can happen).  
      int   movedList[HeapMaxDepth];
      int   movedCount;
      bool  arrayMoved;
};


#if 0
// Assuming that the left and right children are valid heaps, put this element
// in the right place.  See section 7.2.  
template<class T> inline void BinaryHeap<T>::heapify(int i)
{
   int   left  = HeapLeft(i);
   int   right = HeapRight(i);

   // find the largest for all three at this juncture:
   int   largest =  (left < heapSz &&  A[i] < A[left]) ? left : i;
   if( right < heapSz &&  A[largest] < A[right] )
      largest = right;

   // if largest is down one child branch, then pull it up (xchg) and heapify on 
   // down to insure that branch maintains property.  
   if( largest != i ){
      T  temp = A[i];
      A[i] = A[largest];
      A[largest] = temp;
      heapify( largest );
   }
}
#else
// Iterative version of heapify() - It fills the movedList and is still a bit
// faster (~ 25% - 40%) than the recursive version.  The moveCount is returned
// and is used below to know that A[i] didn't get put into the movedList, and
// so may need to be if it was just inserted there.  We don't want heapify() to
// put i in the move list always since change() doesn't require it, so we leave
// that up to our (local) caller.  
template<class T> inline int BinaryHeap<T>::heapify(int i)
{
   T     save = A[i], * largestPtr;
   int   left, right, largest, * moved = movedList;
   
   do{
      largest = i;
      if( (left = HeapLeft(i)) < heapSz ){
         if( save < A[left] )
            largestPtr = & A[largest=left];
         else
            largestPtr = & save;
         if( (right = HeapRight(i)) < heapSz && *largestPtr < A[right] )
            largest = right;
      }
      if( largest != i ){
         A[* moved++ = i] = A[largest];
         i = largest;
      }
      else
         break;
   }while(1);
   
   if( moved != movedList ){
      A[* moved++ = i] = save;
      return( movedCount = moved - movedList );
   }
   else
      return( movedCount = 0 );
}
#endif

// We want to place the given item at i or above, if it's less than its 
// parent, then we need to pull things down and continue looking up.  Use
// carefully, see note below.  
template<class T> inline void BinaryHeap<T>::findSlotUp(const T & item, int i)
{
   int   parentIdx;
   movedCount = 0;
   while( i && A[parentIdx = HeapParent(i)] < item ){
      A[movedList[movedCount++]=i] = A[ parentIdx ];
      i = parentIdx;
   }
   A[movedList[movedCount++]=i] = item;
}

// Build heap from scratch.  See section 7.3.  
template<class T> inline void BinaryHeap<T>::buildHeap(void)
{
   int i = ((heapSz = arraySize()) >> 1);
   while( --i >= 0 )
      heapify(i);
   arrayMoved = true;
   movedCount = 0;
}

// Take the max (top) element off the list and replace it with end of array, then
// heapify to restore the "heap property".  
template<class T> inline T BinaryHeap<T>::extractMax(void)
{
   AssertFatal( heapSz > 0, "binHeap: nothing to extract" );
   startArrayCheck();
   
   T  maxVal = A[0];
   if( --heapSz > 0 ){
      A[0] = A[heapSz];
      if( ! heapify(0) )
         movedList[movedCount++] = 0;
   }
   else
      movedCount = 0;
   A.decrement();
   finishArrayCheck();
   return maxVal;
}

template<class T> inline const T & BinaryHeap<T>::returnMax(void)
{
   AssertFatal( heapSz > 0, "binHeap: nothing to return" );
   return A[0];
}

// Add element at the end and insure heap property going up the tree.  
template<class T> inline void BinaryHeap<T>::insert(const T & item)
{
   AssertFatal( A.size() == heapSz, "binHeap: insert needs heap property" );
   startArrayCheck();
   A.increment();
   findSlotUp( item, heapSz++ );
   finishArrayCheck();
}

// Add element at the end and insure heap property going up the tree.  
template<class T> inline void BinaryHeap<T>::add(const T & elem)
{
   startArrayCheck();
   A.push_back(elem);
   finishArrayCheck();
}

// Delete element referenced by i.  We replace the slot from the end of the 
// list and then re-establish the heap property.  This may mean either 
// adjusting going UP towards root, or down if needed (using heapify()).  
template<class T> inline void BinaryHeap<T>::remove(int i)
{
   AssertFatal( A.size() == heapSz, "binHeap: remove needs heap property" );
   AssertFatal( i < A.size() && i >= 0, "binHeap: bad index to remove" );

   startArrayCheck();
   if( i < --heapSz )
   {
      if( i && A[HeapParent(i)] < A[heapSz] )
         findSlotUp( A[heapSz], i );
      else{
         A[i] = A[heapSz];
         if( ! heapify( i ) )
            movedList[movedCount++] = i;
      }
   }
   A.decrement();
   finishArrayCheck();
}

// The key for the given element has changed and it needs to be percolated
// to preserve the heap property.  
template<class T> inline void BinaryHeap<T>::change(int i)
{
   startArrayCheck();
   if( i && A[HeapParent(i)] < A[i] )
   {
      //note findSlotUp() needs this COPY of the item at i, the following
      //code would not work!:  findSlotUp(A[i],i);  
      T  item = A[i];
      findSlotUp( item, i );
   }
   else
      heapify( i );
   finishArrayCheck();
}


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#endif

