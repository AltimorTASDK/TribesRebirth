// memory allocator for RVectors
// see tRVector.h for details

#ifndef _RVECTORALLOC_H_
#define _RVECTORALLOC_H_

#include <malloc.h>
#include <base.h>

class RVectorAlloc
{
   public:
      struct Node
      {
         int size;
         Node * next;
         char arrayStart;
      };

      enum { BlockSize = 1024, ArrayOffset = sizeof(int)+sizeof(Node*) };

   private:
      static Node * topNode;

   public:
      Node * getArray();
      void   release(Node * node);
      Node * resize(Node*,int size);
      
      ~RVectorAlloc();
};

#endif