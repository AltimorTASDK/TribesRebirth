//------------------------------------------------------------------------------
//
// Description:   Base class for Bayesian Network Node object used in AI
//    
// $Workfile$     Bayes.h
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#ifndef _BAYESNODE_H_
#define _BAYESNODE_H_

#include <types.h>
#include <BitSet.h>
#include <persist.h>
#include <streams.h>
#include <ml.h>

//#include "BayesNet.h"

class BayesNet;

//--------------------------------------------------------------------------------

class BayesNode : public Persistent::VersionedBase
{
public:
   typedef     VectorPtr<BayesNode*> BayesList;
   typedef     BayesList::iterator iterator;
   enum BayesFlags
   {
      VISITED     = (1<<0),
   };
   BitSet32    flags;

protected:
   struct   State
   {
      char  *name;
      int   valueIndex;
      enum StateFlags
      {
         READ_ONLY   = (1<<0),
      };
      BitSet32 flags;

      State( const char *_name, int index ) { name = strnew(_name), valueIndex=index; flags.clear(); }
      ~State() { delete [] name; }
   };
   typedef  VectorPtr<State*> StateList;
   StateList   state;

   char        *name;
   int         id;
   RectI       pos;

   BayesList   parents;
   BayesList   children;

   BayesNet    *parentNet;


public:
   BayesNode();
   virtual ~BayesNode();

   virtual void      addChild( BayesNode *bn )=0;
   virtual void      delChild( BayesNode *bn )=0;
   virtual void      addParent( BayesNode *bn )=0;
   virtual void      delParent( BayesNode *bn )=0;
   virtual void      attachChild( BayesNode *bn )=0;
   virtual void      attachParent( BayesNode *bn )=0;
   virtual void      evaluate()=0;
   virtual bool      reset(bool notifyChildren)=0;

   bool  isChild( BayesNode *pBN );
   bool  isParent( BayesNode *pBN );
   bool  resolveParents();

   void  setName( const char *_name );
   void  setId( int _id ) { id = _id; }
   void  setParentNet( BayesNet *pBN ) { parentNet = pBN; }
   char  *getName() { return (name); }
   void  setRect( RectI &r ) { pos=r; }
   RectI &getRect() { return (pos); }
   int   getId() { return ( id ); }

   void  addState( char *name );
   void  delState( int index );
   void  setStateName( int index, const char *name );
   char  *getStateName( int index ) { return ( state[ index ]->name ); }
   int   numStates() { return ( state.size() ); }
   int   getstatePointer( int index ) { return ( state[ index ]->valueIndex ); }

   int   numParents() { return ( parents.size() ); }
   int   numChildren() { return ( children.size() ); }
   int   getChildId( int index ) { return ( children[index]->id ); }
   void  visit( BayesList *pSorted );

   void  setState( int index, float val );
   float getState( int index );

	// Persistent functions
	Persistent::Base::Error read(StreamIO &, int version, int user);
	Persistent::Base::Error write(StreamIO &, int version, int user); 
	virtual void dump();
};

#endif // _BAYESNODE_H_