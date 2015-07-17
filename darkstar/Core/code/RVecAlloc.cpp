// RVecAlloc.cpp
//    memory allocator for RVectors
//    see tRVector.h for details

#include <RVecAlloc.h>

RVectorAlloc::Node * RVectorAlloc::topNode = NULL;

RVectorAlloc rvAlloc;


RVectorAlloc::Node * RVectorAlloc::getArray()
{
   Node * node;
   if (topNode)
   {
      node       = topNode;
      topNode    = topNode->next;
   }
   else
   {
      node       = (Node*) malloc(BlockSize + ArrayOffset);
      node->size = BlockSize + ArrayOffset;
   }
   node->next = reinterpret_cast<Node*>(0xACDCACDC);
   return node;
}

void RVectorAlloc::release(Node * node)
{
   AssertFatal(node->next == reinterpret_cast<Node*>(0xACDCACDC),"RVector::release: someone sat on me");
   node->next = topNode;
   topNode    = node;
}

RVectorAlloc::Node * RVectorAlloc::resize(RVectorAlloc::Node * node,int size)
{
   int blocks = size / BlockSize;
   if (size % BlockSize)
      blocks++;
   int mem_size = blocks * BlockSize + ArrayOffset;
   node = node ? (Node*) realloc((char*)node,mem_size) :
                 (Node*) malloc(mem_size);
                 
   // Instead of resizing our chunks of memory, we could check them back in and
   // create new bigger chunks, which may seem to result in less memory allocation
   // since we keep the old one around for use.  However, this would probably result
   // in more memory allocation since the smaller chunk will hang around and have to
   // be expanded once again...the goal here is to have all our chunks as big as they
   // need to be.

   if (node)
   {
      node->size          = mem_size;
      node->next = reinterpret_cast<Node*>(0xACDCACDC);
   }

   return node;
}

RVectorAlloc::~RVectorAlloc()
{
   while (topNode)
   {
      Node * node = topNode;
      topNode = topNode->next;
      free(node);
   }
}

