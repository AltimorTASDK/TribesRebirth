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

#include "BayesNet.h"
#include "BayesChance.h"

IMPLEMENT_PERSISTENT_TAG(ChanceNode, FOURCC('B','N','O','D'));
IMPLEMENT_PERSISTENT_TAG(BayesNet,   FOURCC('B','N','E','T'));
static ResourceTypeBayesNet 			_resbay(".ai");

//------------------------------------------------------------------------

BayesNet::BayesNet()
{
   nextNodeId = 10001;
   statePointer = new VectorPtr<StateValue*>;
}   

//------------------------------------------------------------------------

BayesNet::~BayesNet()
{
   clear();
   while (statePointer->size())
   {
      delete statePointer->last();
      statePointer->pop_back();
   }
   delete statePointer;
}   

//------------------------------------------------------------------------

BayesNode *BayesNet::findNode( int id )
{
   for ( iterator itr=space.begin(); itr!=space.end(); itr++ )
      if ( (*itr)->getId() == id )
         return ( *itr );
   return NULL;
}

//------------------------------------------------------------------------

Persistent::Base::Error BayesNet::write(StreamIO &sio, int version, int user)
{
   version, user;
   bool err = !sio.write( nextNodeId );
   err |= !sio.write( statePointer->size() );
   for ( int i=0; i<statePointer->size(); i++ )
      err |= !sio.write( getState(i) );

   if ( !err ) err = !sio.write( sorted.size() );
   for ( iterator itr=sorted.begin(); !err && itr!=sorted.end(); itr++ )
      err = (*itr)->store( sio );
   return ((!err)? Persistent::Base::Ok : Persistent::Base::WriteError);
}

//------------------------------------------------------------------------

Persistent::Base::Error BayesNet::read(StreamIO &sio, int, int)
{
   bool err = !sio.read( &nextNodeId );
   int numNodes = 0;
   int numSVals;
   if ( !err && sio.read( &numSVals ) )
      for ( int i=0; i<numSVals; i++ )
      {
         float val;
         err = !sio.read(&val);
         statePointer->push_back( new StateValue(val) );
      }

   if ( !err )
      err = !sio.read( &numNodes );

   Error error=Ok;
   for ( int i=0; !err && !error && i<numNodes; i++ )
   {
      Persistent::Base *base = Persistent::Base::load( sio, &error );
      BayesNode *pBN = dynamic_cast<BayesNode *>(base);
      if ( pBN && (error == Ok) )
      {
         addNode( pBN );
         pBN->resolveParents();  
      }
   }
   TopologicalSort();
   return ((!err)? Persistent::Base::Ok : Persistent::Base::ReadError);
}
 
//------------------------------------------------------------------------

void BayesNet::dump()
{
   for ( iterator itr=sorted.begin(); itr!=sorted.end(); itr++ )
      (*itr)->dump();   
}

//------------------------------------------------------------------------

void BayesNet::clear()
{
   sorted.clear();
   for ( iterator itr=space.begin(); itr!=space.end(); itr++ )
      delete (*itr);
   space.clear();   
}   

//------------------------------------------------------------------------

void BayesNet::addNode( BayesNode *pBN )
{
   pBN->setParentNet( this );
   space.push_back(pBN);
}

//------------------------------------------------------------------------

void BayesNet::delNode( BayesNode *pBN )
{
   removeNode( pBN );
   delete pBN;
}   

//------------------------------------------------------------------------

void BayesNet::removeNode( BayesNode *pBN )
{
   for ( iterator itr=space.begin(); itr!=space.end(); itr++ )
      if ( *itr == pBN )
      {
         space.erase( itr );
         break;
      }
}   

//------------------------------------------------------------------------

void BayesNet::EvaluateNetwork( StateVal* sv )
{
   if ( !sv )
      sv = statePointer;
   StateVal *oldSV = statePointer;
   statePointer = sv;
   for ( iterator itr=sorted.begin(); itr!=sorted.end(); itr++ )
      (*itr)->evaluate();
   statePointer = oldSV;
}   

//------------------------------------------------------------------------

void BayesNet::TopologicalSort()
{
   sorted.clear();
   for ( iterator itr=space.begin(); itr!=space.end(); itr++ )
      (*itr)->flags.set(BayesNode::VISITED,false);

   for ( iterator itr2=space.begin(); itr2!=space.end(); itr2++ )
      if ( !( *itr2 )->flags.test(BayesNode::VISITED) )
         (*itr2)->visit( &sorted );
}

//------------------------------------------------------------------------

int BayesNet::addState()
{
   statePointer->push_back( new StateValue() );
   return ( statePointer->size()-1 );
}   

//########################################################################

BayesNetInstance::BayesNetInstance( Resource<BayesNet> const &bNet )
{
   BNet = bNet;
   for ( BayesNet::StateVal::iterator itr=BNet->statePointer->begin();
         itr!=BNet->statePointer->end(); itr++ )
      stateValues.push_back(new BayesNet::StateValue((*itr)->val));
}   

//------------------------------------------------------------------------

BayesNetInstance::~BayesNetInstance()
{
   while (stateValues.size())
   {
      delete stateValues.last();
      stateValues.pop_back();
   }
}   

//------------------------------------------------------------------------

void BayesNetInstance::evaluateNetwork()
{
   BNet->EvaluateNetwork(&stateValues);
}   

//------------------------------------------------------------------------

bool BayesNetInstance::registerStateVariable( const char *stateName, float *pFloat )
{
   for (BayesNet::iterator itr=BNet->space.begin(); itr!=BNet->space.end(); itr++ )
      for ( int i=0; i<(*itr)->numStates(); i++ )
         if ( !strcmpi( stateName, (*itr)->getStateName(i) ) )
         {
            stateValues[(*itr)->getstatePointer(i)]->pFloat=pFloat;
            return (true);
         }

   AssertWarn( 0, avar("BayesNetInstance::registerStateVariable: Unable to locate node state: %s",stateName) );
   return false;
}   
