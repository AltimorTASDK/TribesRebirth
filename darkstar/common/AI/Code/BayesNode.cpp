//------------------------------------------------------------------------------
//
// Description:   Base class for Bayesian Network Node object used in AI
//    
// $Workfile$     BayesNode.cpp
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998 Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#include "BayesNode.h"
#include "BayesNet.h"

//##############################################################################
//------------------------------------------------------------------------

BayesNode::BayesNode()
{
   name = NULL;
   flags.clear();
   parentNet = NULL;
}   

//------------------------------------------------------------------------

BayesNode::~BayesNode()
{
   for ( StateList::iterator itr=state.begin(); itr!=state.end(); itr++ )
      delete (*itr);
   state.clear();
   delete [] name;
}   

//------------------------------------------------------------------------

void BayesNode::setName( const char *_name )
{
   delete [] name;
   name = strnew( _name );
}   

//------------------------------------------------------------------------

void BayesNode::setStateName( int index, const char *_name )
{
   if ( index < state.size() )
   {
      delete [] state[index]->name;
      state[index]->name = strnew(_name);
   }
}   

//------------------------------------------------------------------------

bool BayesNode::isChild( BayesNode *pBN )
{
   for ( iterator itr=children.begin(); itr!=children.end(); itr++ )
   {
      if ( *itr == pBN )
         return true;
   }
   return false;
}   

//------------------------------------------------------------------------

bool BayesNode::isParent( BayesNode *pBN )
{
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
   {
      if ( *itr == pBN )
         return true;
   }
   return false;
}   

//------------------------------------------------------------------------

void BayesNode::addState( char *name )
{
   state.push_back( new State(name, parentNet->addState()) );
   reset(true);
}

//------------------------------------------------------------------------

void BayesNode::delState( int index )
{
   if ( state.size() > index )
   {
      StateList::iterator itr=state.begin();
      for ( int i=0; i<index; i++, itr++ );
      delete *itr;
      state.erase( itr );
      reset(true);
   }
}

//------------------------------------------------------------------------

// depth first visit recursion routine
void BayesNode::visit( BayesList *pSorted )
{
   flags.set(VISITED,true);
   for ( iterator itr=children.begin(); itr!=children.end(); itr++ )
      if ( !( *itr )->flags.test(VISITED) )
         (*itr)->visit(pSorted);
   pSorted->push_front(this);
}   

//------------------------------------------------------------------------
//------------------------------------------------------------------------

Persistent::Base::Error BayesNode::write(StreamIO &sio, int version, int user)
{
   version,user;

   bool err = false;

   err |= !sio.write( id );
   err |= !sio.write( sizeof(RectI),&pos );
   err |= !sio.write( strlen(name)+1 );
   err |= !sio.write( strlen(name)+1, name );

   err |= !sio.write( parents.size() );
   for ( int i=0; !err && i<parents.size(); i++ )
   {
      err |= !sio.write( parents[i]->getId() );
   }

   err |= !sio.write( state.size() );
   for ( int ii=0; !err && ii<state.size(); ii++ )
   {
      err |= !sio.write( strlen(state[ii]->name)+1 );
      err |= !sio.write( strlen(state[ii]->name)+1,  state[ii]->name );
      err |= !sio.write( state[ii]->valueIndex );
      err |= !sio.write( state[ii]->flags );
   }
   return ((!err)? Persistent::Base::Ok : Persistent::Base::WriteError);
}
 
//------------------------------------------------------------------------

Persistent::Base::Error BayesNode::read(StreamIO &sio, int version, int user)
{
   version,user;
   bool  err = false;
   int num;

   err |= !sio.read(&id);
   err |= !sio.read( sizeof(RectI),&pos );
   err |= !sio.read( &num );
   name = new char[ num ];
   err |= !sio.read( num, name );

   err |= !sio.read( &num );
   for ( int i=num; !err && i; i-- )
   {
      int parentId;
      err |= !sio.read( &parentId );
      parents.push_back( (BayesNode*)parentId );
   }

   err |= !sio.read( &num );
   for ( int ii=num; !err && ii; ii-- )
   {
      char  buff[256];
      int   index;
      int   len;
      err |= !sio.read( &len );
      err |= !sio.read( len, buff );
      err |= !sio.read( &index );
      State *ps = new State( buff, index );
      err |= !sio.read( sizeof(BitMask32), &ps->flags );
      state.push_back( ps );
   }
   return ((!err)? Persistent::Base::Ok : Persistent::Base::ReadError);
}

//------------------------------------------------------------------------

void BayesNode::dump()
{
}

//------------------------------------------------------------------------

bool  BayesNode::resolveParents()
{
   for ( BayesList::iterator itr=parents.begin(); itr!=parents.end(); itr++ )
   {
      BayesNode *pBN = parentNet->findNode( (int)(*itr) );
      if ( pBN )
      {
         (*itr) = pBN;    // substitute id of parent with ptr to parent
         pBN->attachChild( this );
      }
      else
      {
         // slight hack here to fixup and allow loading of nodes with broken links.
         AssertWarn( pBN, avar("BayesNode::resolveParents:  Parent not found child:%i, parent%i",id,(int)(*itr)));
         BayesList::iterator nitr = itr-1;
         parents.erase(itr);
         itr = nitr;
//         if ( itr==parents.end() )
//            return true;
      }
   }
   return true;
}   

//------------------------------------------------------------------------

void  BayesNode::setState( int index, float val )
{
   parentNet->setState(state[ index ]->valueIndex, val);
}

//------------------------------------------------------------------------

float BayesNode::getState( int index )
{
   return ( parentNet->getState(state[ index ]->valueIndex) );
}
