//------------------------------------------------------------------------------
//
// Description:   Class for a Bayesian Chance node.
//    Chance nodes have a chance probability table used in the evaluation of the network.
//    
// $Workfile$     BayesChance.h
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#ifndef _BAYESCHANCE_H_
#define _BAYESCHANCE_H_

#include <types.h>
#include <BitSet>
#include <persist.h>
#include <streams.h>
#include <ml.h>

#include "BayesNode.h"

//--------------------------------------------------------------------------------

class ChanceNode : public BayesNode
{
   typedef BayesNode Parent;
   float       *cpt;    // conditional probability table
   float       *pCPT;   // ptr into conditional prob table

   float    recursParent( int p );

public:
   ChanceNode();
   virtual ~ChanceNode();
   void     addChild( BayesNode *bn );
   void     delChild( BayesNode *bn );
   void     addParent( BayesNode *bn );
   void     delParent( BayesNode *bn );
   void     attachChild( BayesNode *bn );
   void     attachParent( BayesNode *bn );
   void     evaluate();
   bool     reset(bool notifyChildren);

   // chance node members
   void  __cdecl setCP( float val, ... );
   void     setCPVal( float val, int iState, int item );
   float    getCPVal( int iState, int item );

   DECLARE_PERSISTENT(ChanceNode);
	Persistent::Base::Error read(StreamIO &, int version, int user);
	Persistent::Base::Error write(StreamIO &, int version, int user);
	virtual void dump() { ; }
};

#endif   // _BAYESCHANCE_H_
