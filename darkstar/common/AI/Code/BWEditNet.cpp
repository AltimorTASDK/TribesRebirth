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

#include <ML.h>
#include <R_Clip.h>

#include "BWNEdit.h"
#include "BWEditNode.h"
#include "BWEditNet.h"

//------------------------------------------------------------------------------

EditNet::EditNet(HWND hWndParent)
{
   pBNet = NULL;
   hWnd = hWndParent;
}   

//------------------------------------------------------------------------------

EditNet::~EditNet()
{
   for ( iterator itr=nodeList.begin(); itr!=nodeList.end(); itr++ )
      (*itr)->detach();

   for ( iterator itr2=nodeList.begin(); itr2!=nodeList.end(); itr2++ )
      delete (*itr2);
}   

//------------------------------------------------------------------------------

void EditNet::newNode( )
{
   EditNode *en = new EditNode( this, pBNet->getNextNodeId() );
   addNode( en );
}   

//------------------------------------------------------------------------------

void EditNet::addNode( EditNode*en )
{
   nodeList.push_back( en );
}

//------------------------------------------------------------------------------

void EditNet::delNode( EditNode*en )
{
   removeNode( en );
   pBNet->delNode( en->getBayesNode() );
   delete en;
}

//------------------------------------------------------------------------------

void EditNet::removeNode( EditNode*en )
{
   for ( iterator itr=nodeList.begin(); itr!=nodeList.end(); itr++ )
      if ( *itr == en )
      {
         (*itr)->detach();
         nodeList.erase( itr );
         break;
      }
}

//------------------------------------------------------------------------------

void EditNet::clear()
{
   for ( iterator itr=nodeList.begin(); itr!=nodeList.end(); itr++ )
      delete (*itr);
   nodeList.clear();   
}

//------------------------------------------------------------------------------

EditNet *EditNet::load( const char *finame, HWND hWndParent )
{
   EditNet *en = new EditNet(hWndParent);
   Persistent::Base::Error err;
   BayesNet *pBNet = (BayesNet*)Persistent::Base::fileLoad(finame, &err );
   if ( pBNet && err==Persistent::Base::Ok )
      if ( en->setBayesNet( pBNet ) )
         return en;
      else
         delete pBNet;

   delete en;
   return NULL;
}   

//------------------------------------------------------------------------------

bool EditNet::save( const char *finame )
{
   Persistent::Base::Error err;
   pBNet->TopologicalSort();     // sort network
   err = pBNet->fileStore( finame );
   return ( err!=Persistent::Base::Ok );
}   

//------------------------------------------------------------------------------

void EditNet::render( HDC hDC )
{
   for ( iterator itr=nodeList.begin(); itr!=nodeList.end(); itr++ )
      (*itr)->render( hWnd, hDC );
}   

//------------------------------------------------------------------------

void EditNet::evaluate()
{
   pBNet->TopologicalSort();
   pBNet->EvaluateNetwork(NULL);
}   

//------------------------------------------------------------------------

BayesNode *EditNet::findNode( int id )
{
   return ( pBNet->findNode( id ) );
}

//------------------------------------------------------------------------

EditNode *EditNet::findEditNode( int id )
{
   for ( iterator itr=nodeList.begin(); itr!=nodeList.end(); itr++ )
   {
      if ( (*itr)->getBayesNode()->getId() == id )
         return (*itr);
   }
   return NULL;
}

//------------------------------------------------------------------------

bool EditNet::setBayesNet( BayesNet *pBN )
{
   pBNet = pBN;
   for ( int i=0; i<pBNet->statePointer->size(); i++ )
      stateValues.push_back( new BayesNet::StateValue(pBNet->getState(i)) );

   pBNet->statePointer = &stateValues;
   
   // create edit nodes for all bayes nodes
   for ( BayesNode::iterator itr=pBNet->space.begin(); itr!=pBNet->space.end(); itr++ )
   {
      EditNode *en = new EditNode( this, *itr );
      addNode( en );
   }

   // now link all edit nodes to match bayes network
   for ( iterator itr2=nodeList.begin(); itr2!=nodeList.end(); itr2++ )
      (*itr2)->attachChildren();

   return true;
}   