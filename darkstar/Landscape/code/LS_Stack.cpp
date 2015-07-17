//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   LS_Stack.cpp  $
//北  $Version$
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北        LandScape Stack Methods
//北        
//北  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#include <g_bitmap.h>  // because we POP GFXBitmap's
#include "ls.h"        // base class defined

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// clears entire stack

void  LandScape::clear()
{
   while( stack.size() )
   {
      delete stack[0];
      stack.erase( stack.begin() );
   }
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// removes matrix from stack.x and returns resulting GFXBitmap

void LandScape::pop()
{
   if ( !stack.size() )
      return;

   delete stack[0];
   stack.pop_front();
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// returns resulting GFXBitmap but leaves stack.x unchanged

GFXBitmap *LandScape::get(int norm)
{
   if (!stack.size())
   {
      sprintf( mExecString, "GET: statck empty" );
      mExecStatus = EXEC_FAILURE;
      return NULL;
   }

   int size = stack[0]->size;
   if (norm)      // normalize to use palette entries 10-245
   {
      dup();
      normalize( 0.04f, 0.96f );
   }

   GFXBitmap *pBM = GFXBitmap::create(size, size);

   if (!pBM)
   {
      sprintf( mExecString, "GET: failed to create bitmap" );
      mExecStatus = EXEC_FAILURE;
      return NULL;
   }

   for (int y = 0; y < size; y++ )
      for (int x = 0; x < size; x++ )
         pBM->pBits[y*pBM->stride + x] = stack[0]->real[y*size + x]*255.0;  // normalize

   if (norm)
      pop();

   return pBM;
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void  LandScape::push(int size)
{
    Matrix *m = new Matrix(size);
    stack.push_front(m);
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// converts GFXBitmap into a matrix and pushes onto stack.x
// heights are between 0 and maxH -- if maxH<0, then heights between 0 and 1
void  LandScape::pushBM( GFXBitmap *_pBM, float maxH )
{
   int     size = _pBM->width;         // better be SQUARE!
   Matrix *pMTX = new Matrix(size);

   for (int y = 0; y < size; y++ )
      for (int x = 0; x < size; x++ )
         pMTX->real[y*size + x] = maxH * _pBM->pBits[y*_pBM->stride + x] / 255.0;  // normalize

   pMTX->size = size;
   stack.push_front( pMTX );  // push onto stack
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// swaps stack.x with stack.y

void  LandScape::swap()
{
   if ( stack.size() < 2 )
   {
      sprintf( mExecString, "SWAP: stack size too small" );
      mExecStatus = EXEC_FAILURE;
      return;
   }

   Matrix *pMTX = stack[0];
   stack[0] = stack[1];
   stack[1] = pMTX;
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void  LandScape::dup()
{
   if ( !stack.size() )
   {
      sprintf( mExecString, "DUP: stack empty" );
      mExecStatus = EXEC_FAILURE;
      return;
   }

   int size = stack[0]->size;
   Matrix *pMTX = new Matrix(size);
   memcpy(pMTX->real, stack[0]->real, sq(size)*sizeof(float) );
   stack.push_front( pMTX );
}

//------------------------------------------------------------------------------
void LandScape::size(int sz)
{
   if ( !stack.size() )
   {
      sprintf( mExecString, "SIZE: stack is empty" );
      mExecStatus = EXEC_FAILURE;
      return;
   }

   Matrix *pOld = stack[0];

   if (sz > pOld->size)
   {
      Matrix *pNew = new Matrix(sz);

      int scale = sz/pOld->size;
      int scalex, scaley;
      int x,xo,y,yo;      // coords in old matrix

      for (y=0, yo=0, scaley=0; y < sz; y++, scaley++)
      {
         if (scaley == scale)
         {
            yo++; scaley = 0;
         }
         for (x=0, xo=0, scalex=0; x < sz; x++, scalex++)
         {
            if (scalex == scale)
            {
               xo++; scalex = 0;
            }
            pNew->real[y*sz+x] = pOld->real[yo*pOld->size+xo];
         }
      }

      delete stack[0];
      stack.erase( stack.begin() );
      stack.push_front( pNew );
   }
}   
