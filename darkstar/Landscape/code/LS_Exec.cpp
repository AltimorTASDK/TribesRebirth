//----------------------------------------------------------------------------
//   
//  $Workfile:   LS.cpp  $
//  $Version$
//  $Revision:   1.00  $
//    
//  DESCRIPTION:
//        Class for creating LandScape bitmaps from complex operations.
//        
//  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
// 
//----------------------------------------------------------------------------

#include <stdio.h>
#include "LS.h"

//----------------------------------------------------------------------------

LandScape::LandScape(Int32 size )
{
   Matrix *pMTX = new Matrix(size);   // allocate a new matrix
   stack.push_back( pMTX );
   clr(0);
	rm = 0;
}   

void	LandScape::setRM( ResourceManager *pRM )
{
	rm = pRM;
}

//----------------------------------------------------------------------------

LandScape::~LandScape()
{
   for( int i=0; i<stack.size(); i++ )
      delete stack[i];

   stack.clear();
}

//----------------------------------------------------------------------------

DWORD __cdecl LandScape::exec( LS_OPCODE _opcode, ... )
{
   DWORD result;

   va_list  va;
   va_start( va, _opcode );

   result = execV( _opcode, va );

   va_end( va );

   return result;
};

DWORD LandScape::execV( LS_OPCODE _opcode, va_list _va )
{
   // reset the return status - individual functions will alter this if need be
   mExecStatus = EXEC_SUCCESS;
   mExecString[0] = '\0';
      
   switch( _opcode )
   {
      case LS_SEED: setSeed( va_arg(_va,DWORD) ); break;
      case LS_LOAD: return load( va_arg(_va,char*) );

      case LS_LOADM:
   		{
   			char * tmpCh = va_arg(_va,char*);
   			double tmpD = va_arg(_va,double);
   			return load(tmpCh, float(tmpD));
   		}
      case LS_SAVE: return save( va_arg(_va,char*) );
      case LS_CLEAR: clear(); break;
      case LS_PUSH: push( va_arg(_va,int)); break;
      case LS_POP:  pop(); break;
      case LS_GET:  return (DWORD)get( va_arg(_va,int) );
      case LS_SWAP: swap(); break;
      case LS_DUP:  dup(); break;
      case LS_ADD:  add( float(va_arg(_va,double)) ); break;
      case LS_SUB:  sub( float(va_arg(_va,double)) ); break;
      case LS_MUL:  mul( float(va_arg(_va,double)) ); break;
      case LS_DIV:  div( float(va_arg(_va,double)) ); break;
      case LS_EXP:  exp( float(va_arg(_va,double)) ); break;
      case LS_NEG:  neg(); break;
      case LS_CLR:  clr( float(va_arg(_va,double)) ); break;
      case LS_DIFF: diff(); break;

      case LS_NORMALIZE:
         {
            double i = va_arg(_va,double);
            double j = va_arg(_va,double);
            normalize( float(i), float(j) ); 
         }
         break;

      case LS_ADDS: addStack(); break;
      case LS_SUBS: subStack(); break;

      case LS_FLOOR:
         {
            double i = va_arg(_va,double);
            double j = va_arg(_va,double);
            double k = va_arg(_va,double);
            floor( float(i), float(j), float(k) );   
         }
         break;

      case LS_CEIL:
         {
            double i = va_arg(_va,double);
            double j = va_arg(_va,double);
            double k = va_arg(_va,double);
            ceil( i, j, k );   
         }
         break;

      case LS_CLIPMIN:  clipMin( float(va_arg(_va,double)) ); break;
      case LS_CLIPMAX:  clipMax( float(va_arg(_va,double)) ); break;
      case LS_ROT:   rot();   break;
      case LS_FLIPX: flipX(); break;
      case LS_FLIPY: flipY(); break;

      case LS_TERRAIN:
         {
         int    i = va_arg(_va,int);
         double j = va_arg(_va,double);
         if (i > 256)
         {
            terrain( 256, float(j) );
            size(i);
         }
         else
            terrain( i, float(j) );
         }
         break;

      case LS_PLASMA:
         {
         int    i = va_arg(_va,int);
         double j = va_arg(_va,double);
         if (i > 256)
         {
            plasma( 256, float(j) );
            size(i);
         }
         else
            plasma( i, float(j) );
         }
         break;

      case LS_SIZE: size( va_arg(_va,int) );  break;

      case LS_SLOPE:
         {
         double i = va_arg(_va,double);
         int    j = va_arg(_va,int);
         slope( i, float(j) );
         }
         break;

      case LS_SHAVE:
         {
         double i = va_arg(_va,double);
         double j = va_arg(_va,double);
         double k = va_arg(_va,double);
         shaveArea( float(i), float(j), float(k) );
         }
         break;
 
      case LS_CURVE:
         {
         double i = va_arg(_va,double);
         int    j = va_arg(_va,int);
         curve( float(i), j );
         }
         break;

      case LS_FFT:
         if (stack.size() > 1)
             fft( va_arg(_va,int) ); break;

      case LS_FILL_N:
        if (stack.size() > 1)
             fillNormal( float(va_arg(_va,double)) ); break;

      case LS_OVERLAY:
         {
         int    i = va_arg(_va,int);
         double j = va_arg(_va,double);
         overlay( i, float(j) );
         }
         break;

      case LS_ALPHABLEND:
         {
            int x = va_arg(_va, int);
            int y = va_arg(_va, int);
            alphablend(x, y);
         }
        break;

      case LS_BLEND:
         {
            int   i = va_arg(_va,int);
            int   j = va_arg(_va,int);
            blend( i, j );
         }

      case LS_SMOOTH:
         {
            double i = va_arg(_va,double);
            double j = va_arg(_va,double);
            smooth( float(i), float(j) ); 
         }
         break;

      case LS_TILE:  tile(); break;
      case LS_WRAP:  wrap(); break;

      case LS_CLAMP:
         {
         int    i = va_arg(_va,int);
         double j = va_arg(_va,double);
         clamp(i, float(j));
         }
         break;

      case LS_MASK:
         {
         int    i = va_arg(_va,int);
         double j = va_arg(_va,double);
         mask(i, float(j));
         }
         break;

      case LS_CRATER:
      case LS_PEAK:
      case LS_RING:
      case LS_FILLBASIN:
      case LS_LPFILTER:
      case LS_HPFILTER:
      case LS_BPFILTER:
      case LS_BRFILTER:
      case LS_FFLP:
      case LS_FFHP:
      case LS_FFBP:
      case LS_FFBR:
      default:
         break;
   }
   return NULL;
}
