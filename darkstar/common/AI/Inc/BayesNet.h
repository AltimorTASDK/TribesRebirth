//------------------------------------------------------------------------------
//
// Description:   Base class for Bayesian Network Node object used in AI
//    
// $Workfile$     BayesNet.h
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#ifndef _BAYESNET_H_
#define _BAYESNET_H_

#include <types.h>
#include <BitSet.h>
#include <persist.h>
#include <streams.h>
#include <ml.h>

#include "BayesNode.h"

//--------------------------------------------------------------------------------

class BayesNet : public Persistent::VersionedBase
{
   int   nextNodeId;

public:
   struct   StateValue
   {
      float    *pFloat;             // ptr to float value
      float    val;                 // default float 
      StateValue(float _val=0.0f) { pFloat=&val; val=_val; }
   };
   typedef VectorPtr<StateValue*>   StateVal;
   StateVal                *statePointer; // ptr to array of state value pointers

   typedef  BayesNode::BayesList::iterator iterator;
   BayesNode::BayesList    space;
   BayesNode::BayesList    sorted;

   void  visit();

public:
   BayesNet();
   ~BayesNet();

   void  addNode( BayesNode *pBN );
   void  delNode( BayesNode *pBN );
   void  removeNode( BayesNode *pBN );
   void  clear();
   int   getNextNodeId() { return nextNodeId++; }
   BayesNode *findNode( int id );

   void  TopologicalSort();
   void  EvaluateNetwork(StateVal* sv=NULL);

   int   addState();
   void  setState( int index, float value ) { *(*statePointer)[index]->pFloat=value; }   
   float getState( int index ) { return (*(*statePointer)[index]->pFloat); }

	// Persistent functions
   DECLARE_PERSISTENT(BayesNet);
	Persistent::Base::Error read(StreamIO &, int version, int user);
	Persistent::Base::Error write(StreamIO &, int version, int user); 
	virtual void dump();
};

//------------------------------------------------------------------------

class ResourceTypeBayesNet: public ResourceType
{
  public:   
   ResourceTypeBayesNet(const char *ext = ".bay"):
   ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int)
   { 
      Persistent::Base::Error err; 
      return (void*)Persistent::Base::load(*stream,&err); 
   }
   void destruct(void *p)            { delete (BayesNet*)p; }
};   

//------------------------------------------------------------------------------

class BayesNetInstance
{
   VectorPtr<BayesNet::StateValue*> stateValues;     // ptr to array of state values
   Resource<BayesNet>   BNet;

public:
   BayesNetInstance( Resource<BayesNet> const & bNet );
   ~BayesNetInstance();

   void  evaluateNetwork();
   bool  registerStateVariable( const char *stateName, float *pfloat );
   BayesNet *getBayesNet() { return BNet; }
};

#endif   // _BAYESNET_H_
