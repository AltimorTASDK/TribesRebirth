//------------------------------------------------------------------------------
//
// Description:   Base class for Bayesian Network Node object used in AI
//    
// $Workfile$     Bayes.cpp
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#include "BayesChance.h"

//------------------------------------------------------------------------

ChanceNode::ChanceNode()
  : BayesNode()
{
   cpt = NULL;   
}   

//------------------------------------------------------------------------

ChanceNode::~ChanceNode()
{
   delete [] cpt;

//   for ( StateList::iterator itr=state.begin(); itr!=state.end(); itr++ )
//      delete (*itr);

   for ( iterator itr2=parents.begin(); itr2!=parents.end(); itr2++ )
      (*itr2)->delChild( this );

   for ( iterator itr3=children.begin(); itr3!=children.end(); itr3++ )
      (*itr3)->delParent( this );
}   

//------------------------------------------------------------------------

void ChanceNode::addChild( BayesNode *bn )
{
   children.push_back( bn );
   reset(false);
}

//------------------------------------------------------------------------

void ChanceNode::attachChild( BayesNode *bn )
{
   children.push_back( bn );
}

//------------------------------------------------------------------------

void ChanceNode::attachParent( BayesNode *bn )
{
   parents.push_back( bn );
}

//------------------------------------------------------------------------

void ChanceNode::delChild( BayesNode *bn )
{
   for ( iterator itr=children.begin(); itr!=children.end(); itr++ )
      if ( *itr == bn )
      {
         children.erase( itr );
         break;
      }
}

//------------------------------------------------------------------------

void ChanceNode::addParent( BayesNode *bn )
{
   parents.push_back( bn );
   reset(false);
}

//------------------------------------------------------------------------

void ChanceNode::delParent( BayesNode *bn )
{
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
      if ( *itr == bn )
      {
         parents.erase( itr );
         reset(false);
         break;
      }
}

//------------------------------------------------------------------------

void __cdecl ChanceNode::setCP( float val, ... )
{
   AssertFatal( parents.size(), avar("ChanceNode::setCP: Node has no CP table use setState for %s",name) );

   va_list  va;
   va_start( va, val );
   int index = va_arg( va,int );
   AssertFatal( index<state.size(), avar("ChanceNode::setCP:  Index state out of range for %s",name) );
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
   {
      index *= ((ChanceNode*)(*itr))->numStates();
      int param = va_arg( va,int );
      AssertFatal( param<((ChanceNode*)(*itr))->numStates(), avar("ChanceNode::setCP:  Index state out of range for %s:%s",name,((ChanceNode*)*itr)->name) );
      index += param;
   }
   va_end( va );

   cpt[index] = val;
}   

//------------------------------------------------------------------------

void ChanceNode::setCPVal( float val, int iState, int item )
{
   int numStates = parents.size()? 1:0;
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
      numStates *= ((ChanceNode *)(*itr))->numStates();

   cpt[ iState*numStates + item ] = val;
}   

//------------------------------------------------------------------------

float ChanceNode::getCPVal( int iState, int item )
{
   int numStates = parents.size()? 1:0;
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
      numStates *= ((ChanceNode *)(*itr))->numStates();

   return cpt[ iState*numStates + item ];
}   

//------------------------------------------------------------------------

void ChanceNode::evaluate()
{
   pCPT = cpt;             // init ptr into the cpt      
   if ( parents.size() )   // is this node dependent?
      for ( int s=0; s<numStates(); s++ )
         if (!state[s]->flags.test(State::READ_ONLY) )
            setState( s, recursParent(0) );
         else
            recursParent(0);
}   

//------------------------------------------------------------------------

float ChanceNode::recursParent( int p )
{
   float ttl = 0;
   for ( int s=0; s<((ChanceNode*)parents[p])->numStates(); s++ )
      if ( p<parents.size()-1 )
         ttl += recursParent( p+1 ) * ((ChanceNode*)parents[p])->getState(s);
      else
         ttl += *pCPT++ *  ((ChanceNode*)parents[p])->getState(s);
   return ttl;
}   

//------------------------------------------------------------------------

bool ChanceNode::reset(bool notifyChildren)
{
   bool status = true;
   delete [] cpt;
   int nodes = state.size();  // number of states this node
   
   // for each parent node
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
      nodes *= ((ChanceNode*)*itr)->numStates();

   cpt = new float[ nodes ];  // allocate a new CP table

   if ( cpt )
   {
      for ( int i=0; i<nodes; i++ )
         cpt[i] = 1.0;

      if ( notifyChildren )
         for ( iterator itr=children.begin(); itr!=children.end(); itr++ )
            status &= (*itr)->reset(false);
   }
   else
      status = false;

   return status;
}   

//------------------------------------------------------------------------

Persistent::Base::Error ChanceNode::read(StreamIO &sio, int version, int user)
{
   Parent::read(sio,version,user);

   int   nodes;
   sio.read( &nodes );
   cpt = new float[ nodes ];  // allocate a new CP table
   sio.read( nodes*sizeof(float), cpt );

   // code here to read in conditional probability table
   return Persistent::Base::Ok;
}

//------------------------------------------------------------------------

Persistent::Base::Error ChanceNode::write(StreamIO &sio, int version, int user)
{
   Parent::write(sio,version,user);

   int nodes = state.size();  // number of states this node
   
   // for each parent node
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
      nodes *= ((ChanceNode*)*itr)->numStates();

   sio.write(nodes);
   sio.write( nodes*sizeof(float), cpt );

   // code here to write out conditional probability table
   return Persistent::Base::Ok;
} 
