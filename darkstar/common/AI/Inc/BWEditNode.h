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

#ifndef _BWEDITNODE_H_
#define _BWEDITNODE_H_

#include <BayesNode.h>
#include <BayesNet.h>
#include <BayesChance.h>
#include <GW.h>

class NodeEditor;
class EditNet;
//------------------------------------------------------------------------------

class EditNode
{
   EditNet  *parentNet;

public:
   typedef  VectorPtr<EditNode*> EditNodeList;
   typedef  EditNodeList::iterator iterator;

   int   data;
   HWND  hWnd;

   enum BayesFlags
   {
      SELECTED    = (1<<0),
   };
   BitSet32    flags;

   EditNodeList   parents;
   EditNodeList   children;

public:
   EditNode( EditNet *pParentNet, int id );
   EditNode( EditNet *pParentNet, BayesNode *pBN );
   ~EditNode();

   void  addChild( EditNode *pEN );
   void  delChild( EditNode *bn );
   void  addParent( EditNode *bn );
   void  delParent( EditNode *bn );
   void  reset(bool notifyChildren);
   void  attachChildren();
   void  attachChild( int id );
   void  attachParent( EditNode *en );
   void  detach();
   bool  isChild( EditNode *pEN );
   bool  isParent( EditNode *pEN );


   void render( HWND hpw, HDC hDC );
   void drawArrowHead( Point2I &pt1, Point2I &pt2, HDC hDC );
   void edit( GWWindowClass *gwc, GWWindow *pParent );
   void displayNode( NodeEditor *pNE );
   void addParentHeader( NodeEditor *pNE, int index );
   void addStateHeader( NodeEditor *pNE, int index );

   static EditNode *getEditNode( HWND hw );
   BayesNode   *getBayesNode();
};

#endif   // _BWEDITNODE_H_
