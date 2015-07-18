//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   LS_Math.cpp  $
//北  $Version$
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北        LandScape Arithmetic Methods
//北        
//北  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#include "ls.h"      // base class defined

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void  LandScape::add( float _n )
{
    if (stack.size() < 1)
    {
         sprintf( mExecString, "ADD: nothing on stack to add" );
         mExecStatus = EXEC_FAILURE;
        return;
    }
    int length = sq(stack[0]->size);

    for (int i = 0; i < length; i++)
        stack[0]->real[i] += _n;   
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void  LandScape::sub( float _n )
{
    if (stack.size() < 1)
    {
         sprintf( mExecString, "SUB: nothing on stack to subtract" );
         mExecStatus = EXEC_FAILURE;
        return;
    }
    int length = sq(stack[0]->size);

    for (int i = 0; i < length; i++ )
        stack[0]->real[i] -= _n;   
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void  LandScape::mul( float _n )
{
    if (stack.size() < 1)
    {
         sprintf( mExecString, "MUL: nothing on stack to multiply" );
         mExecStatus = EXEC_FAILURE;
        return;
    }
    int length = sq(stack[0]->size);

    for (int i = 0; i < length; i++ )
        stack[0]->real[i] *= _n;   
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void  LandScape::div( float _n )
{
    if (stack.size() < 1)
    {
         sprintf( mExecString, "DIV: nothing on stack to divide" );
         mExecStatus = EXEC_FAILURE;
        return;
    }
    int length = sq(stack[0]->size);

    for (int i = 0; i < length; i++ )
        stack[0]->real[i] /= _n;   
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void  LandScape::exp( float _n )
{
    if (stack.size() < 1)
    {
         sprintf( mExecString, "EXP: nothing on stack to exp" );
         mExecStatus = EXEC_FAILURE;
        return;
    }
    int length = sq(stack[0]->size);

    for (int i = 0; i < length; i++ )
        stack[0]->real[i] = pow(stack[0]->real[i], _n);   
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void  LandScape::neg()
{
    if (stack.size() < 1)
    {
         sprintf( mExecString, "NEG: nothing on stack to neg" );
         mExecStatus = EXEC_FAILURE;
        return;
    }
    int length = sq(stack[0]->size);
    for (int i = 0; i < length; i++ )
        stack[0]->real[i] = -stack[0]->real[i];   
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void  LandScape::clr( float _n )
{
    if (stack.size() < 1)
    {
         sprintf( mExecString, "NEG: nothing on stack to clear" );
         mExecStatus = EXEC_FAILURE;
        return;
    }
    int length = sq(stack[0]->size);

    memset(stack[0]->real, _n, length*sizeof(float));
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#include <float.h>  // to get float min/max limits

// Already defined in math.h
//inline float fmin( float x, float y ) { return ( x<y? x:y ); }
//inline float fmax( float x, float y ) { return ( x>y? x:y ); }

void  LandScape::normalize( float _min, float _max )
{
   int i;
   if (stack.size() < 1)
   {
      sprintf( mExecString, "NORMALIZE: nothing on stack to normalize" );
      mExecStatus = EXEC_FAILURE;
      return;
   }

	float * realPtr = stack[0]->real;

   int   length =  sq(stack[0]->size);
   float min    =  FLT_MAX;
   float max    = -FLT_MAX;

   for ( i = 0; i < length; i++)
   {
      min = fmin( min, realPtr[i] );
      max = fmax( max, realPtr[i] );
   }

   if ( !(max-min) )
      return;

   float ratio = ( _max - _min ) / ( max - min );

   for ( i = 0; i < length; i++ )
      realPtr[i] = ratio*(realPtr[i] - min) + _min;
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void LandScape::addStack( int _n ) 
{
    if ( stack.size() < _n+1 )
    {
        sprintf( mExecString, "ADD_STACK: stack too small for operation" );
        mExecStatus = EXEC_FAILURE;
        return;
    }

    if (stack[0]->size != stack[_n]->size)
    {
        sprintf( mExecString, "ADD_STACK: can't add different sized matrices");
        mExecStatus = EXEC_FAILURE;
        return;
    }

    int size = stack[0]->size;

    for ( int y=0; y < size; y++ )
        for ( int x=0; x < size; x++ )
            stack[0]->real[y*size + x] += stack[_n]->real[y*size + x];

    delete stack[_n];
    stack.erase( stack.begin()+_n );
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

void LandScape::subStack( int _n ) 
{
   if ( stack.size() < _n+1 )
   {
      sprintf( mExecString, "SUB_STACK: stack too small for operation" );
      mExecStatus = EXEC_FAILURE;
      return;
   }
 
    if (stack[0]->size != stack[_n]->size)
    {
        sprintf( mExecString, "SUB_STACK: can't subtract different sized matrices");
        mExecStatus = EXEC_FAILURE;
        return;
    }

    int size = stack[0]->size;

    for ( int y=0; y < size; y++ )
        for ( int x=0; x < size; x++ )
         stack[0]->real[y*size + x] = stack[_n]->real[y*size + x] - stack[0]->real[y*size + x];

   delete stack[_n];
   stack.erase( stack.begin()+_n );
}


//---------------------------------------------------------------------------------
// Calculates the probability that a normal randomly chosen number
// will fall between _val and _mean given a standard deviation of _sdev.
// Remember, max probability is 0.5 because half is on one side of _mean.
//
// it uses the formula
// probability = 1/2 - r(at + bt^2 + ct^3) + err(x)
// where a = 0.4361836
// where b = -0.1201676
// where c = 0.937298
// where r = ( exp(-x*x/2) )/sqrt(2PI)
// where t = 1/( 1+0.3326*x)
// err(x) is the residual error, this routine is accurate to 10^-5

float normalProbability( float _mean, float _sdev, float _val )
{
   _val = (_val-_mean)/_sdev;
   if ( _val<0.0 )
      _val = -_val;
   float t = 1/( 1 + 0.3326*_val );
   float r = ( exp( -_val*_val*0.5f ) ) * ( 0.398942280 );
   return( 0.5 - r*t*( 0.4361836 - 0.1201676*t + 0.937298*t*t ) );
}

//---------------------------------------------------------------------------------
// Calculates the probability that a normal randomly chosen number
// will fall between _val and _mean.  Values are normalized to
// fit within the _min and _max values from the mean.  Note that in
// this function, _dev is a normalized standard deviation and must be
// between 0.0 and 1.0
// Returns values between 0.0 to 0.5

float normalProbability( float _min, float _max, float _mean, float _dev, float _val )
{
   float x;
   
   // first normalize value -1.0 -mean- 1.0
   if ( _val == _mean )
      return( 0.0 );
   else if ( _val < _min )
      return( 0.5 );
   else if ( _val > _max )
      return( 0.5 );
   else if ( _val <_mean )
      x = (_val-_mean)/(_mean-_min);
   else if ( _val > _mean )
      x = (_val-_mean)/(_max-_mean);
   else
      return (0.5);

    return( normalProbability( 0.0, _dev, x ) );
}   
