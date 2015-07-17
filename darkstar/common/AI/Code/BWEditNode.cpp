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

#include "BWEditNode.h"
#include "BWEditNet.h"
#include "BWNEdit.h"

//------------------------------------------------------------------------

EditNode::EditNode( EditNet *pParentNet, int id )
{
   parentNet = pParentNet;
   ChanceNode *pBN = new ChanceNode();
   char *n = (char*)avar("ChanceNode: %i",id);
   pBN->setName( n );
   pBN->setId( id );
   pParentNet->getBayesNet()->addNode( pBN );
   data = id;

   hWnd = CreateWindowEx( NULL ,"STATIC",pBN->getName(),SS_CENTER|WS_CHILD|WS_VISIBLE|WS_DLGFRAME, 20,20,100,60, parentNet->getHandle(), (HMENU)id, GetModuleHandle(NULL) ,NULL );
   SetWindowLong( hWnd, GWL_USERDATA, (LONG)this );
   HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   SendMessage( hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0) );
}

//------------------------------------------------------------------------

EditNode::EditNode( EditNet *pParentNet, BayesNode *pBN )
{
   parentNet = pParentNet;
   data = pBN->getId();
   RectI r = pBN->getRect();
   hWnd = CreateWindowEx( NULL ,"STATIC",pBN->getName(),SS_CENTER|WS_CHILD|WS_VISIBLE|WS_DLGFRAME, r.upperL.x,r.upperL.y,r.len_x(),r.len_y(), parentNet->getHandle(), (HMENU)pBN->getId(), GetModuleHandle(NULL) ,NULL );
   SetWindowLong( hWnd, GWL_USERDATA, (LONG)this );
   HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   SendMessage( hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0) );
}

//------------------------------------------------------------------------

EditNode::~EditNode()
{
   DestroyWindow( hWnd );   
}

//------------------------------------------------------------------------------

void EditNode::detach()
{
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
      (*itr)->delChild( this );

   for ( iterator itr2=children.begin(); itr2!=children.end(); itr2++ )
      (*itr2)->delParent( this );
}   

//------------------------------------------------------------------------------

EditNode *EditNode::getEditNode( HWND hw )
{
   EditNode *pwl = (EditNode*)GetWindowLong( hw, GWL_USERDATA );
   EditNode *pEN;
#ifdef _MSC_VER
   // EditNode is NOT polymorphic (at time of writing) ... MSC
   // complains and stops compilation for dynamic_cast of non-
   // polymorphic types
   if ( (pEN=static_cast <EditNode*>(pwl)) != NULL )
#else
   if ( (pEN=dynamic_cast<EditNode*>(pwl)) != NULL )
#endif
      return pEN;
   else
      return NULL;
}

//------------------------------------------------------------------------

void EditNode::render( HWND hpw, HDC hDC )
{
   SetWindowText( hWnd, getBayesNode()->getName() );
   RECT anchor;
   GetWindowRect( hWnd, &anchor );
   MapWindowPoints(NULL,hpw,(LPPOINT)&anchor,2 );
   getBayesNode()->setRect( RectI(anchor.left,anchor.top,anchor.right,anchor.bottom) );

   for ( iterator itr=children.begin(); itr!=children.end(); itr++ )
   {
      RECT  child;
      GetWindowRect( ((EditNode*)(*itr))->hWnd, &child );
      MapWindowPoints(NULL,hpw,(LPPOINT)&child,2 );

      Point2I pt1((anchor.right+anchor.left)/2, (anchor.bottom+anchor.top)/2);
      Point2I startPt( (child.right+child.left)/2, (child.bottom+child.top)/2 );
      Point2I pt2 = startPt; // save for later
      RectI cRect( anchor.left, anchor.top, anchor.right, anchor.bottom );
      rectClip( &pt1, &startPt, &cRect );
      Point2I endPt = startPt;
      cRect( child.left, child.top, child.right, child.bottom );
      rectClip( &pt2, &endPt, &cRect );

      MoveToEx( hDC, startPt.x, startPt.y , NULL );
      LineTo( hDC, endPt.x, endPt.y );

      drawArrowHead( startPt, endPt, hDC );
   }
}   

//------------------------------------------------------------------------------

void EditNode::drawArrowHead( Point2I &pt1, Point2I &pt2, HDC hDC )
{
   if ( pt2.y-pt1.y )
   {
      POINT points[4];
      points[0].x = pt2.x;
      points[0].y = pt2.y;
      points[1].x = 8.0 * m_cos( m_atan((double)pt2.x-pt1.x,(double)pt2.y-pt1.y) - 7*M_PI/8 )+pt2.x;
      points[1].y = 8.0 * m_sin( m_atan((double)pt2.x-pt1.x,(double)pt2.y-pt1.y) - 7*M_PI/8 )+pt2.y;
      points[2].x = 8.0 * m_cos( m_atan((double)pt2.x-pt1.x,(double)pt2.y-pt1.y) + 7*M_PI/8 )+pt2.x;
      points[2].y = 8.0 * m_sin( m_atan((double)pt2.x-pt1.x,(double)pt2.y-pt1.y) + 7*M_PI/8 )+pt2.y;
      points[3].x = pt2.x;
      points[3].y = pt2.y;
      Polygon( hDC, points, 4 );
   }
}   

//------------------------------------------------------------------------------

void EditNode::displayNode( NodeEditor *pNE )
{
   // delete all headers
   for ( int i=0; i<pNE->parentHeaderList.size(); i++ )
      DestroyWindow( pNE->parentHeaderList[i] );
   pNE->parentHeaderList.clear();

   for ( int ii=0; ii<pNE->descList.size(); ii++ )
      DestroyWindow( pNE->descList[ii] );
   pNE->descList.clear();

   for ( int j=0; j<pNE->stateHeaderList.size(); j++ )
      DestroyWindow( pNE->stateHeaderList[j] );
   pNE->stateHeaderList.clear();

   for ( int k=0; k<parents.size(); k++ )
   {
      addParentHeader( pNE, k );
   }

   for ( int m=0; m<getBayesNode()->numStates(); m++ )
   {
      addStateHeader( pNE, m );
//      updateState( pNE, i );
   }
}   

//------------------------------------------------------------------------------

void EditNode::addParentHeader( NodeEditor *pNE, int index )
{
   HWND hNE = pNE->getHandle();
   RECT  r,rtb;
   GetClientRect( pNE->hToolBar, &r );
   HWND hHeader = CreateWindowEx(
               WS_EX_WINDOWEDGE,
               WC_HEADER,
               "",
               HDS_HORZ|DS_MODALFRAME|WS_CHILD,
               0,r.bottom,r.right,r.bottom,
               hNE,
               NULL,
               GWMain::hInstance,
               NULL );

   pNE->parentHeaderList.push_back( hHeader );

   HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   SendMessage( hHeader, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0) );

   HD_LAYOUT hdl;
   WINDOWPOS wp;
   GetClientRect( hNE, &r );
   hdl.prc = &r;
   hdl.pwpos = &wp;

   SendMessage( hHeader, HDM_LAYOUT, 0, (LPARAM)&hdl );
   GetClientRect( pNE->hToolBar, &rtb );
   wp.x += LEFT_OFFSET + STATE_COL_WIDTH + TABLE_COL_WIDTH;
   wp.y = TOP_OFFSET + rtb.bottom + index*wp.cy;
   wp.cx = TABLE_COL_WIDTH;
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
      wp.cx *= ((EditNode *)(*itr))->getBayesNode()->numStates();

   SetWindowPos( hHeader, wp.hwndInsertAfter, wp.x, wp.y, 
                 wp.cx, wp.cy, wp.flags | SWP_SHOWWINDOW); 

   HWND hDesc = CreateWindow( "STATIC",parents[index]->getBayesNode()->getName(),
                              WS_CHILD|WS_VISIBLE|SS_LEFT,
                              LEFT_OFFSET, wp.y, STATE_COL_WIDTH + TABLE_COL_WIDTH, wp.cy,
                              pNE->getHandle(),
                              NULL,
                              GWMain::hInstance,
                              NULL );

   SendMessage( hDesc, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0) );
   pNE->descList.push_back( hDesc );
   HD_ITEM hdi;
   hdi.mask = HDI_TEXT|HDI_FORMAT|HDI_WIDTH;
   hdi.fmt  = HDF_CENTER | HDF_STRING;

   int numStates = parents[index]->getBayesNode()->numStates();
   int numItems = 1;
   for ( int i=0; i<index; i++ )
      numItems *= parents[i]->getBayesNode()->numStates();

   numItems *= numStates;
   if ( numItems )
   {
      hdi.cxy = wp.cx/numItems;
      for ( int i=0; i<numItems; i++ )
      {
         hdi.pszText = parents[index]->getBayesNode()->getStateName(i%numStates);
         hdi.cchTextMax = lstrlen( hdi.pszText );
         SendMessage( hHeader, HDM_INSERTITEM, i, (LPARAM)&hdi );
      }
   }
}   

//------------------------------------------------------------------------------

void EditNode::addStateHeader( NodeEditor *pNE, int index )
{
   HWND hNE = pNE->getHandle();
   RECT  r,rtb;
   GetClientRect( pNE->hToolBar, &r );
   HWND hHeader = CreateWindowEx(
               WS_EX_WINDOWEDGE,
               WC_HEADER,
               "state",
               HDS_HORZ|HDS_BUTTONS|DS_MODALFRAME|WS_CHILD,
               LEFT_OFFSET,r.bottom,r.right,r.bottom,
               hNE,
               (HMENU)(IDC_STATE+index),
               GWMain::hInstance,
               NULL );

   pNE->stateHeaderList.push_back( hHeader );

   HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
   SendMessage( hHeader, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(true,0) );

   HD_LAYOUT hdl;
   WINDOWPOS wp;
   GetClientRect( hNE, &r );
   hdl.prc = &r;
   hdl.pwpos = &wp;

   SendMessage( hHeader, HDM_LAYOUT, 0, (LPARAM)&hdl );
   GetClientRect( pNE->hToolBar, &rtb );
   wp.x = LEFT_OFFSET;
   wp.y = TOP_OFFSET + rtb.bottom + (pNE->parentHeaderList.size())*wp.cy + (pNE->stateHeaderList.size()-1)*wp.cy;
   int numStates = parents.size()? 1:0;
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
      numStates *= (*itr)->getBayesNode()->numStates();

   wp.cx = (TABLE_COL_WIDTH * numStates) + STATE_COL_WIDTH + TABLE_COL_WIDTH;

   SetWindowPos( hHeader, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy, wp.flags | SWP_SHOWWINDOW); 

   HD_ITEM hdi;
   hdi.mask = HDI_TEXT|HDI_FORMAT|HDI_WIDTH;
   hdi.cxy  = STATE_COL_WIDTH;
   hdi.fmt  = HDF_CENTER | HDF_STRING;

   hdi.pszText = getBayesNode()->getStateName(index);
   hdi.cchTextMax = lstrlen( hdi.pszText );
   SendMessage( hHeader, HDM_INSERTITEM, 0, (LPARAM)&hdi );

   hdi.cxy  = TABLE_COL_WIDTH;
   hdi.pszText = (char*)avar( "%3.4f",getBayesNode()->getState(index) );
   hdi.cchTextMax = lstrlen( hdi.pszText );
   hdi.fmt  = HDF_RIGHT | HDF_STRING;
   SendMessage( hHeader, HDM_INSERTITEM, 1, (LPARAM)&hdi );

   for ( int i=0; i<numStates; i++ )
   {
      hdi.pszText = (char*)avar( "%3.4f",((ChanceNode*)getBayesNode())->getCPVal(index,i) );
      hdi.cchTextMax = lstrlen( hdi.pszText );
      hdi.fmt  = HDF_RIGHT | HDF_STRING;
      SendMessage( hHeader, HDM_INSERTITEM, 2+i, (LPARAM)&hdi );
   }
}   

//------------------------------------------------------------------------------

bool  EditNode::isChild( EditNode *pEN )
{
   BayesNode *pBN1 = getBayesNode();
   BayesNode *pBN2 = pEN->getBayesNode();

   if ( pBN1 && pBN2 )
      return ( pBN1->isChild( pBN2 ) );
   else
      return false;
}   

//------------------------------------------------------------------------------

bool  EditNode::isParent( EditNode *pEN )
{
   BayesNode *pBN1 = getBayesNode();
   BayesNode *pBN2 = pEN->getBayesNode();

   if ( pBN1 && pBN2 )
      return ( pBN1->isParent( pBN2 ) );
   else
      return false;
}   

//------------------------------------------------------------------------------

BayesNode *EditNode::getBayesNode()
{
   BayesNode *pBN = parentNet->findNode(data);
   AssertWarn( pBN, avar("EditNode::getBayesNode:  Unable to locate node %i",data) );
   return ( pBN );
}   

//------------------------------------------------------------------------------

void EditNode::addChild( EditNode *en )
{
   children.push_back( en );
   en->addParent( this );
   getBayesNode()->addChild( en->getBayesNode() );
}   

void EditNode::delChild( EditNode *en )
{
   for ( iterator itr=children.begin(); itr!=children.end(); itr++ )
      if ( *itr == en )
      {
         children.erase( itr );
         break;
      }

   getBayesNode()->delChild( en->getBayesNode() );
}   

void EditNode::addParent( EditNode *en )
{
   parents.push_back( en );
   getBayesNode()->addParent( en->getBayesNode() );
}   

void EditNode::delParent( EditNode *en )
{
   for ( iterator itr=parents.begin(); itr!=parents.end(); itr++ )
      if ( *itr == en )
      {
         parents.erase( itr );
         reset(false);
         break;
      }

   getBayesNode()->delParent( en->getBayesNode() );
}   

void EditNode::reset( bool notifyChildren )
{
   getBayesNode()->reset(notifyChildren);
}   


void EditNode::attachChildren()
{
   for ( int i=0; i<getBayesNode()->numChildren(); i++ )
   {
      attachChild( getBayesNode()->getChildId(i) );
   }
}   

void EditNode::attachChild( int id )
{
   EditNode *en = parentNet->findEditNode( id );
   if ( en )
      children.push_back(en);
   en->attachParent( this );   
}   

void EditNode::attachParent( EditNode *en )
{
   parents.push_back( en );
}   
