//------------------------------------------------------------------------------
//
// Description:   Editable Bayesian Network Node object used in BayWatch
//    
// $Workfile$     BWNode.h
// $Revision$
// $Author  $     Louie McCrady
// $Modtime $
//               (c) Copyright 1997, 1998, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#ifndef _BWEDITNET_H_
#define _BWEDITNET_H_

#include <BayesNode.h>
#include <BayesNet.h>
#include <BayesChance.h>
#include <GW.h>

#include "BWNEdit.h"
#include "BWEditNode.h"
#include "BWEditNet.h"

//------------------------------------------------------------------------------

class EditNet
{
public:
   typedef VectorPtr<EditNode*> EditNodeList;
   typedef EditNodeList::iterator iterator;

private:
   HWND           hWnd;                // handle to parent window
   BayesNet       *pBNet;              // handle to bayesNet associated with this editNet
   EditNodeList   nodeList;            // list of EditNodes associated with this editNet

   VectorPtr <BayesNet::StateValue*> stateValues;  // vector of state values

public:
   EditNet( HWND hWndParent );
   ~EditNet();

   static EditNet *load( const char *finame, HWND hWndParent );
   bool save( const char *finame );

   void newNode();
   void addNode( EditNode *pEN );
   void delNode( EditNode *pEN );
   void removeNode( EditNode *pEN );
   void clear();
   bool setBayesNet( BayesNet *pBN );
   BayesNet *getBayesNet() { return pBNet; }
   HWND getHandle() { return hWnd; }
   void render( HDC hDC );
   BayesNode *findNode( int id );
   EditNode  *findEditNode( int id );
   void evaluate();
};

#endif   // _BWEDITNET_H_
