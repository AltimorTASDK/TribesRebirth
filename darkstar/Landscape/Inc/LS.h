//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   LS.h  $
//北  $Version$
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北        Class for creating LandScape bitmaps from complex operations.
//北        
//北  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#ifndef  _LS_H_
#define  _LS_H_

#include <types.h>   // for Phoenix types
#include <tvector.h> // to manage our matrix stacks
#include <ml.h>

#include "stdio.h"
#include "m_random.h" // for random and normal numbers

// Forward declarations...
//
class GFXBitmap;


enum LS_ROT_90S
{
   CCW_0 = 0,  // no rotation
   CCW_90 = 1, // counter clockwise 90 degree rotation
   CCW_180 = 2,
   CCW_270 = 3
};


//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// opcodes 

#define sq(x) ((x)*(x))


enum LS_OPCODE
{
   LS_SEED     = 100,
   // IO
   LS_LOAD     = 200,   // ( char *_fileName )  load bitmap from file
	LS_LOADM,				// load bitmap from file with given max height
   LS_SAVE,             // ( char *_fileName )  store bitmap to file
   // stack operators
   LS_PUSH     = 300,   // push y->n, x->y
   LS_POP,              // pop x-> , y->x, n->y, return x
   LS_GET,              // return x
   LS_SWAP,             // x<->y
   LS_DUP,              // y=x
   LS_SIZE,             // scale top of stack 
   // arithmetic operators
   LS_ADD      = 400,   // ( double _n )  add _n to matrix x 
   LS_SUB,              // ( double _n )  sub _n from matrix x 
   LS_MUL,              // ( double _n )  mul matrix by _n
   LS_DIV,              // ( double _n )  div matrix by _n
   LS_EXP,              // ( double _n )  raise matrix to power of _n
   LS_NEG,              // invert min/max values in x
   LS_CLR,              // ( double _n )  set matrix to _n
   LS_DIFF,             // take differential (local slope) of x.  Also known as "embossing" in art lingo
   LS_NORMALIZE,        // ( double _min, double _max ) normalize x to _min,_max
   LS_ADDS,             // add x with stack element n, pop n, result in x, default stack y
   LS_SUBS,             // subtract x from stack element n, pop n, result in x, default stack y
   // complex operators see LS.TXT for parameters
   LS_TERRAIN  = 500,   // generates random rolling hills
   LS_PLASMA,   			// generates random rolling hills
   LS_CRATER,           // generates a crater field
   LS_PEAK,             // generates a high point on the landscape
   LS_ROT,              // rotates the landscape 90 degrees
   LS_RING,             // generates a smooth ring shape
   LS_FILLBASIN,        // averages subsamples below a minimum threshold
   LS_OVERLAY,          // average x with y keeping y constant
   LS_BLEND,            // add x with y 
   LS_FLIPX,            // flip left-to-right
   LS_FLIPY,            // flip top-to-bottom
   // filter functions
   LS_LPFILTER = 600,
   LS_HPFILTER,
   LS_BPFILTER,
   LS_BRFILTER,
   LS_FFLP,
   LS_FFHP,
   LS_FFBP,
   LS_FFBR,
   LS_FFT,
   LS_SMOOTH,           // sub-sample
   LS_SLOPE,            // averages pixels outside maximum local slope
	LS_SHAVE,
   LS_CURVE,            // averages pixels outside local curve
   LS_FLOOR,            // clip minimum values
   LS_CEIL,             // clip maximum values
   LS_FILL_N,           // fill 1/f Gaussian noise quadrants 1234 all different
   //
   LS_CLEAR,            // clears entire stack
   LS_ALPHABLEND,
   LS_TILE,
   LS_WRAP,
   LS_CLAMP,            // a filter to limit growing under Phoenix terrain system
   LS_CLIPMIN,          // clips minimum values
   LS_CLIPMAX,          // clips maximum values
	LS_MASK,
};

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

struct   Matrix
{
    int    size;
    float *real;   

    Matrix(int s)
    { 
        size = s;
        real = new float[size*size];   
    }
    ~Matrix() 
    { 
        delete [] real; 
    }
};

class ResourceManager;

class LandScape
{
   typedef  VectorPtr<Matrix*> MATRIX;
   MATRIX   stack;

   Random   rand;

	ResourceManager *rm; // for loading bitmaps
   
   // return codes for execV functions
   enum {
      EXEC_FAILURE  =  0,
      EXEC_SUCCESS  =  1,
   };
   int   mExecStatus;
   char  mExecString[256];
   
public:
   LandScape( Int32 );
   ~LandScape();

   DWORD __cdecl exec( LS_OPCODE, ... );    // issue a command to the landscape
   DWORD execV( LS_OPCODE, va_list );
   void  parseScript( char *_buff, bool clearStack = true );

   // HouseKeeping
   void  setSeed( DWORD _seed ) { rand.setSeed( _seed ); }  
   unsigned int getSeed() { return(rand.getSeed()); }  

   // IO
   bool  load( char *_fileName, float maxH=1.0f );  // load bitmap
   bool  save( char *_fileName );						 // save bitmap
	void	setRM( ResourceManager * pRM );

   // special function for Grid system, fills existing grid block
   void  setGrdHeightMap( float *_pGrdHeightMap, Box2I& _box );
   int   getSize();

   // stack operators
   void  clear();
   void  pop();
   GFXBitmap *get(int norm);
   void  pushBM( GFXBitmap *_pBM=NULL, float maxH=1.0f );
   void  push(int size);
   void  swap();
   void  dup();
   void  size(int size);

   // arithmetic operators
   void  add( float _n );
   void  sub( float _n );
   void  mul( float _n );
   void  div( float _n );
   void  exp( float _n );
   void  neg();
   void  clr( float _n );
   void  diff();
   void  normalize( float _min, float _max );
   void  addStack( int _n=1 );
   void  subStack( int _n=1 );

   // complex operators see LS.TXT for parameters
   void  terrain( int size, float _h );
   void  plasma( int size, float _deviation );
   void  crater();
   void  peak();
   void  rot();
   void  ring();
   void  fillbasin();
   void  overlay( int _n, float _thresh );
   void  blend( int _n, int _dist );
   void  flipX();
   void  flipY();

   // fill functions
   void  fillNormal( float _height );
   void  fillNormalHalf( float _height );

   // base functions
   void  lpfilter();
   void  hpfilter();
   void  bpfilter();
   void  brfilter();
   void  fflp();
   void  ffhp();
   void  ffbp();
   void  ffbr();
   void  fft( int _sign );
   void  smooth(float,float);
   void  slope( float _thresh, int _iter ); // limits slope difference
	void  shaveArea( float epsilon, float exponent, float avgWeight );
   void  curve( float _thresh, int _iter ); // averages the slope
   void  floor( float flr_start, float flr_depth, float scanDepth);
   void  ceil( float ceil_start, float ceil_height, float scanHeight );
   void  alphablend(int x, int y);
   void  tile();
   void  wrap();
   void  mask(int _level, float _deviation);
   void  clamp(int _scale, float _deviation);
   void  clamp(Point2I &, int);
   void  clamp(Point2I &, LS_ROT_90S, Vector<Point2I> *, int);
   void  flatten(Point2I &, LS_ROT_90S, Vector<Point2I> *, Flt32);
   void  flatten(Point2I &, LS_ROT_90S, Vector<Point2I> *);
   void  clipMin(float minVal);
   void  clipMax(float maxVal);
};

float normalProbability( float _mean, float _sdev, float _val );
float normalProbability( float _min, float _max, float _mean, float _dev, float _val );


inline int LandScape::getSize()
{
   if (stack.size())
      return (stack[0]->size);
   return (0);
}   


#endif   // _LS_H_
