//----------------------------------------------------------------------------
//   
//  $Workfile:   LS_Cmplx.cpp  $
//  $Version$
//  $Revision:   1.00  $
//    
//  DESCRIPTION:
//        LandScape Complex fill and filter Methods
//        
//  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
// 
//----------------------------------------------------------------------------

#include <float.h>
#include "ls.h"      // base class defined
#include <g_timer.h>

///////////////////////////////////////////////////////////////////////////////

#define GOOD_TRIG
#include "lstrig.h"
#define SQRT2   2*0.70710678118654752440084436210484
static void fht( REAL *fz, int size );

///////////////////////////////////////////////////////////////////////////////

//#define minf(x,y)  (((x)<(y))? (x):(y))
#define maxf(x,y)  (((x)>(y))? (x):(y))
#define absf(x)    ((x)>0.0?(x):(-(x)))
//#define inf(x,a,b) (((x)>=(a)) && ((x)<=(b)))
#define modf(x,y)  (((x)+(y))%(y))

#define El(vv, xq, yq)  vv[((yq) * size) + xq]  /* elmt of array */
#define Elmod(v,x,y) v[(((y)>=0)?(y)%size:(((y)%size)+size)%size)*size \
           + (((x)>=0)?(x)%size:(((x)%size)+size)%size) ]
#define Elclip(v,x,y) v[(((y)<0?0:(((y)>=size)?(size-1):(y))) * size)\
            + ((x)<0?0:(((x)>=size)?(size-1):(x)))]

///////////////////////////////////////////////////////////////////////////////

void LandScape::wrap()
{
    if (stack.size() < 1)
    {
      sprintf( mExecString, "WRAP: nothing on stack to wrap" );
      mExecStatus = EXEC_FAILURE;
      return;
    }

    // process the edges of the matrix so they wrap nicely when tiled.
    bool   ok;
    int    size = stack[0]->size;
    float *line;
    float *lineBuffer = new float[size];
    float  hMax, delta, n0, n1;
    int    y, x;

    // "factor" is how large of a jump at the seam (in proportion to the
    // maximum jump in a scanline) is acceptable
    const float factor = 0.5;

    // do horiz wrapping, visit each scanline    
    for (y = 0; y < size; y++)
    {
        // grab a pointer to this scanline
        line = &(stack[0]->real[y*size]);

        // for scanline y, find out the min and mix instantaneous changes
        // in height
        hMax = -99999.0;
        for (x = 0; x < size - 1; x++)
        {
            delta = absf(line[x + 1] - line[x]);
            hMax  = maxf(hMax, delta);
        }
        
        // now try to wrap this scanline horizontally s.t. the instantaneous
        // changes in height are within the bounds of the rest of the scanline
        n0 = size - 1;
        n1 = 0;

        // loop until the seam is smothed, or at the worst (and very unlikely)
        // case that we extended the seam to the entire scanline
        while (n1 < size - 1)
        {
            // see if the instantaneous changes over the seam are within the
            // max and min changes over the inside (non-seam)
            ok = true;
            for (x = n0; x != n1; x = modf(x + 1, size))
            {
                delta = absf(line[modf(x + 1, size)] - line[x]);
                if (delta > factor*hMax)
                {
                    ok = false;
                    break;
                }
            }
            if (ok)
                break;

            // some changes are too large (or too small), so apply a blur
            // filter to points n0..n1, over the seam. 
            for (x = n0; x != n1; x = modf(x + 1, size))
                lineBuffer[x] = (line[modf(x - 1, size)] +
                                 line[x] +
                                 line[modf(x + 1, size)]) / 3.0;
    
            // since we didn't work in-place, copy back the new data
            for (x = n0; x != n1; x = modf(x + 1, size))
                line[x] = lineBuffer[x];

            n0--;
            n1++;
        }
    }

    // now do vert wrapping, this is almost the same as horiz. version
    // above, the difference being that since scanlines here are vertical,
    // they are addressed as line[y*size] instead of line[x] as above
    for (x = 0; x < size; x++)
    {
        line = &(stack[0]->real[x]);
        hMax = -99999.0;
        for (y = 0; y < size - 1; y++)
        {
            delta = absf(line[(y + 1)*size] - line[y*size]);
            hMax  = maxf(hMax, delta);
        }
        
        n0 = size - 1;
        n1 = 0;
        while (n1 < size - 1)
        {
            ok = true;
            for (y = n0; y != n1; y = modf(y + 1, size))
            {
                delta = absf(line[modf(y + 1, size)*size] - line[y*size]);
                if (delta > factor*hMax)
                {
                    ok = false;
                    break;
                }
            }
            if (ok)
                break;

            for (y = n0; y != n1; y = modf(y + 1, size))
                lineBuffer[y] = (line[modf(y - 1, size)*size] +
                                 line[y*size] +
                                 line[modf(y + 1, size)*size]) / 3.0;
            for (y = n0; y != n1; y = modf(y + 1, size))
                line[y*size] = lineBuffer[y];

            n0--;
            n1++;
        }
    }

    delete [] lineBuffer;
}

///////////////////////////////////////////////////////////////////////////////

static Matrix *getMatrix(Matrix *source, int x, int y, int size)
{
    int u, v, sourceIndex;
    Matrix *m = new Matrix(size);

    for (v = 0; v < size; v++)
        for (u = 0; u < size; u++)
        {
            sourceIndex = (y + v)*source->size + (u + x);
            m->real[v*size + u] = source->real[sourceIndex];
        }

    return(m);
}

static void placeMatrix(Matrix *target, Matrix *source, int x, int y)
{
    int u, v, targetIndex;

    for (v = 0; v < source->size; v++)
        for (u = 0; u < source->size; u++)
        {
            targetIndex = (y + v)*target->size + (u + x);
            target->real[targetIndex] = source->real[v*source->size + u];
        }
}

void LandScape::tile()
{
    if (stack.size() < 1)
    {
        sprintf( mExecString, "TILE: Nothing on the stack to tile");
        mExecStatus = EXEC_FAILURE;
        return;
    }

    //
    // breaks the NxN matrix into four tiles, each N/2xN/2, then puts
    // them back together in a 2Nx2N matrix.  The tiles are named 
    // 0, 1, 2 and 3 (corresponding to their quadrant).  So the original
    // matrix looks like:
    //
    //       10
    //       23
    //
    // and the final (result) matrix will be tiled as:
    //
    //       3232
    //       0101
    //       3232
    //       0101
    //

    Matrix *m        = stack[0];
    int     tileSize = m->size/2;
    Matrix *tile0    = getMatrix(m, tileSize, 0, tileSize); 
    Matrix *tile1    = getMatrix(m, 0, 0, tileSize);
    Matrix *tile2    = getMatrix(m, 0, tileSize, tileSize);
    Matrix *tile3    = getMatrix(m, tileSize, tileSize, tileSize);

    Matrix *target   = new Matrix(tileSize*4);

    // First row
    placeMatrix(target, tile3, 0,          0);
    placeMatrix(target, tile2, tileSize,   0);
    placeMatrix(target, tile3, 2*tileSize, 0);
    placeMatrix(target, tile2, 3*tileSize, 0);

    // Second row
    placeMatrix(target, tile0, 0,          tileSize);
    placeMatrix(target, tile1, tileSize,   tileSize);
    placeMatrix(target, tile0, 2*tileSize, tileSize);
    placeMatrix(target, tile1, 3*tileSize, tileSize);

    // Third row
    placeMatrix(target, tile3, 0,          2*tileSize);
    placeMatrix(target, tile2, tileSize,   2*tileSize);
    placeMatrix(target, tile3, 2*tileSize, 2*tileSize);
    placeMatrix(target, tile2, 3*tileSize, 2*tileSize);

    // Fourth row
    placeMatrix(target, tile0, 0,          3*tileSize);
    placeMatrix(target, tile1, tileSize,   3*tileSize);
    placeMatrix(target, tile0, 2*tileSize, 3*tileSize);
    placeMatrix(target, tile1, 3*tileSize, 3*tileSize);

    pop();
    stack.push_front(target);
}


//----------------------------------------------------------------------------

void LandScape::alphablend(int x, int y)
{
    // Performs alpha blending of items on stack.
    // Top of the stack is the alpha filter, 2nd to the top is the overlay,
    // 3rd to the top is the original
    //
    // x,y are placement coordinates on the original where overlay is going
    //

    if (stack.size() < 3)
    {
        sprintf( mExecString, "ALPHA_BLEND: not enough items on the stack (need 3)");
        mExecStatus = EXEC_FAILURE;
        return;
    }

    // assumes alpha and m1 are same size
    if (stack[0]->size != stack[1]->size)
    {
        sprintf( mExecString, "ALPHA_BLEND: alpha chan and overlay are different sizes");
        mExecStatus = EXEC_FAILURE;
        return;
    }

    Matrix *alpha = stack[0];
    Matrix *m1    = stack[1];
    Matrix *m0    = stack[2];

    float *alphaReal = alpha->real;
    float *m0Real = m0->real;
    float *m1Real = m1->real;

    int   size1 = m1->size, size0 = m0->size;
    int   x0, y0, x1, y1;
    float va, v1;
    
    for (y1 = 0; y1 < size1; y1++)
        for (x1 = 0; x1 < size1; x1++)
        {
            // x1,y1 is coordinate on m1/alpha,
            // x0,y0 is coordinate on m0
            x0 = x1 + x;
            y0 = y1 + y;

            if (x0 >= 0 && y0 >= 0 && x0 < size0 && y0 < size0)
            {
                va = alphaReal[y1*size1 + x1];
                v1 = m1Real[y1*size1 + x1];
                float & rF = m0Real[y0*size0 + x0];
//                v0 = m0Real[y0*size0 + x0];

//                m0Real[y0*size0 + x0] = va*v1 + (1.0 - va)*v0;
                rF = va*v1 + (1.0 - va)*rF;
            }
        }

   pop();   // pop alpha off the stack
   pop();   // pop overaly off the stack
}

//----------------------------------------------------------------------------

const float convWeights[] = 
{
    0.25,    0.75,    1.00,    0.75,    0.25,
    0.50,    2.00,    3.00,    2.00,    0.75,
    1.00,    3.00,    4.00,    3.00,    1.00,
    0.75,    2.00,    3.00,    2.00,    0.75,
    0.25,    0.75,    1.00,    0.75,    0.25,
};
const int convWeightsSize = sizeof(convWeights)/sizeof(float);

void LandScape::smooth(float epsilon, float exponent)
{
    if (stack.size() < 1)
    {
        sprintf( mExecString, "SMOOTH: Not enough items on the stack to blend");
        mExecStatus = EXEC_FAILURE;
        return;
    }
    int     x, y, u, v, xc, yc, i, cDelta;
    float   targetVal, delta, sign;

    Matrix *sourceM = stack[0];
    float *source   = sourceM->real;
    int size        = sourceM->size;

    Matrix *targetM = new Matrix(size);
    float *target   = targetM->real;

    // compute 1/epsilon just once
    float  invEpsilon;
    if (epsilon!=0.0)
      invEpsilon = 1.0f/epsilon;

    // figure the side dimension of the convolution filter
    int convolutionSize = sqrt((double)convWeightsSize);  
    
    // figure the sampling range, for every elt (x,y), sampling will
    // occur in the rectangle whose corners are (x-c,y-c) and (x+c,y+c)
    // where c = cDelta
    cDelta = (convolutionSize - 1)/2;

    // find the of the wieghts in the filter
    float totalWeight = 0;
    for (i = 0; i < convWeightsSize; i++)
        totalWeight += convWeights[i];

    float invTotalWeight = 1.0/totalWeight;

    // Visit every elt of the target matrix, compute the value of each
    // elt of the target as the weighted average of some elts of source
    for (y = 0; y < size; y++)
        for (x = 0; x < size; x++)
        {
            // initialize this elt of the target matrix as zero
            i = y*size + x;
            targetVal = 0.0;
            
            // sample convolutionSize^2 elts of the source matrix
            const float * conWptr = convWeights;
            int vUpper = y + cDelta;
            int uUpper = x + cDelta;
            int vLower = y - cDelta;
            int uLower = x - cDelta;
            if (vUpper<size && uUpper<size && vLower>=0 && uLower>=0)
            {
               // not near edges of grid: we can be a little more efficient...
               float *fPtr = source + vLower*size + uLower;
               int rowEndToRowStart = size - (vUpper-vLower) - 1;
               for (v = vLower; v <= vUpper; v++)
               {
//                     AssertFatal(fPtr==source + v*size + uLower,"oops: error in pointer arithmatic");
                   for (u = uLower; u <= uUpper; u++)
                       targetVal += *conWptr++ * *fPtr++;
                   fPtr += rowEndToRowStart;
               }
            }
            else
            { // a cosmetic bracket :)
               // near edges: need modulo arithmatic so we wrap correctly
               for (v = vLower; v <= vUpper; v++)
                   for (u = uLower; u <= uUpper; u++)
                   {
                       // figure the coordinates of u,v for sampling, wrapping
                       // around incase u,v are neg, or >= size
                       xc = (u + size)%size;
                       yc = (v + size)%size;
    
                       targetVal += *conWptr * source[yc*size + xc];
                       conWptr++;
                   }
            }

            // amortize the resulting value of this target elt over the
            // total weight of the filter
            targetVal *= invTotalWeight;

            // get unsigned delta and sign separately
            delta = targetVal - source[i];
            sign  = delta >= 0.0 ? 1.0 : -1.0;
            delta *= sign;

            if (delta >= epsilon)
                // fine, the difference is above the threshhold, apply the
                // smoothed intensity
                target[i] = targetVal - sign*epsilon;
            else
            {
               // the difference is below the threshold, curve it with
               // a polynomial function

               // normalize delta to 0..1
               delta *= invEpsilon;

               // apply the polynomial function
               delta = pow(delta, exponent);

               // normalize back to 0..epsilon
               delta *= epsilon;

               // apply the difference to the original intensity
               target[i] = source[i] + (delta*sign);
            }
        }
    
   pop();
   stack.push_front(targetM);    
}

//----------------------------------------------------------------------------

// All regions in the current matrix which is below flr_start
// will be rescaled so that it will lie between the elevations
// (flr_start) and (flr_start - flr_depth)  
//
// More specifically, the elevations between (flr_start) and 
// (flr_start - scanDepth) is normalized between (flr_start) 
// and (flr_start - flr_depth).  Any region that is deeper then 
// scanDepth is clamped at flr_depth.
//
// Constraints:
//  flr_depth >= 0
//  scanDepth >= 0

void LandScape::floor( float flr_start, float flr_depth, float scanDepth )
{

    if (stack.size() < 1)
    {
        sprintf( mExecString, "FLOOR: Nothing on the stack to floor");
        mExecStatus = EXEC_FAILURE;
        return;
    }

    AssertWarn(flr_depth >= 0, "LandScape::floor: flr_depth < 0");
    AssertWarn(scanDepth >= 0, "LandScape::floor: scanDepth < 0");     

    int length = sq(stack[0]->size);
    int i;
    
    float * realPtr = stack[0]->real;

    if ((flr_depth == 0) || (scanDepth == 0)) 
    {
        // special case so we don't divide by zero
        for ( i=0; i<length; i++ )
             if (realPtr[i] < flr_start)
                  realPtr[i] = flr_start;
    }
    else 
    {
        float scale = (flr_depth) / (scanDepth);
        float scanDepthElev = flr_start - scanDepth;
        float flrDepthElev = flr_start - flr_depth;

        for ( i=0; i<length; i++ )
        {
            if (realPtr[i] < scanDepthElev)
                realPtr[i] = flrDepthElev;
            else if (realPtr[i] < flr_start)
                realPtr[i] = flr_start - (flr_start - realPtr[i]) * scale;
        }
    }
}

//----------------------------------------------------------------------------

// All regions in the current matrix which is above ceil_start
// will be rescaled so that it will lie between the elevations
// (ceil_start) and (ceil_start + ceil_height)  
//
// More specifically, the elevations between (ceil_start) and 
// (ceil_start + scanHeight) is normalized between (ceil_start) 
// and (ceil_start + ceil_height).  Any region that is higher then 
// scanDepth is clamped at ceil_height.
//
// Constraints:
//  ceil_depth >= 0
//  scanDepth >= 0

void LandScape::ceil( float ceil_start, float ceil_height, float scanHeight )
{
    if (stack.size() < 1)
    {
        sprintf( mExecString, "CEIL: Nothing on the stack to ceil");
        mExecStatus = EXEC_FAILURE;
        return;
    }

    AssertWarn(ceil_height >= 0, "LandScape::ceil: ceil_height < 0");
    AssertWarn(scanHeight >= 0, "LandScape::ceil: scanHeight < 0");     
    
    int length = sq(stack[0]->size);
    int i;

    float *realPtr = stack[0]->real;

    if ((ceil_height == 0) || (scanHeight == 0))
    {
        // special case so we don't divide by zero
        for ( i=0; i<length; i++)
             if (realPtr[i] > ceil_start)
                  realPtr[i] = ceil_start;
    }
    else 
    {
        float scale = ceil_height / scanHeight;
        float scanHeightElev = ceil_start + scanHeight;
        float ceilHeightElev = ceil_start + ceil_height;

        for ( i=0; i<length; i++)
        {
            if (realPtr[i] > scanHeightElev)
                realPtr[i] = ceilHeightElev;
            else if (realPtr[i] > ceil_start)
                realPtr[i] = ceil_start + (realPtr[i] - ceil_start) * scale;
        }
    }
}

//----------------------------------------------------------------------------

void LandScape::rot()
{
    if (stack.size() < 1)
    {
      sprintf( mExecString, "ROT: Nothing on the stack to rot");
      mExecStatus = EXEC_FAILURE;
        return;
    }
    if ( !stack.size() )
        return;

    int     size   = stack[0]->size;
    Matrix *pMTX   = new Matrix(size);

    for (int row = 0; row < size; row++ )
      for ( int col = 0; col < size; col++ )
         pMTX->real[(col + 1)*size - row - 1] = stack[0]->real[row*size + col];
//   delete [] stack[0];
//   stack[0] = pMTX;
    
    Matrix *old = stack.front();
    stack.pop_front();
    delete old;
    stack.push_front(pMTX);
}

//----------------------------------------------------------------------------

void LandScape::curve( float _thresh, int _iter )
{
    if (stack.size() < 1)
    {
        sprintf( mExecString, "CURVE: Nothing on the stack to curve");
        mExecStatus = EXEC_FAILURE;
        return;
    }

    // we'll use this instead
    float thresh2 = _thresh*_thresh;

    int   size = stack[0]->size;
    float bitSrc, bitDst, deltaX, deltaY;
    bool  changed=TRUE;  

    dup();

    while ( changed && _iter-- )
    {
        changed = FALSE;   // TRUE if something changes
        swap();
        float * topReal = stack[0]->real;
        float * nextReal = stack[1]->real;
        float * topXY = topReal;
        float * nextXY = nextReal;
        for ( int iy = 0; iy < size; iy++ )
        {
            for ( int ix=0; ix < size; ix++ )
            {
                float    bitX,bitY,bitX1,bitY1;   /* values of HF elements on either side of this one */
                if (ix==0 || ix==size-1 || iy==0 || iy==size-1)
                {
                   bitDst = *topXY;
                   bitX   = Elmod(topReal,ix-1,iy);
                   bitY   = Elmod(topReal,ix,iy-1);
                    bitX1  = Elmod(topReal,ix+1,iy); 
                    bitY1  = Elmod(topReal,ix,iy+1);
                   deltaX = bitDst - (bitX+bitX1)*0.5f;
                   deltaY = bitDst - (bitY+bitY1)*0.5f;
//                   bitSrc = sqrt(deltaX*deltaX + deltaY*deltaY);
                   bitSrc = deltaX*deltaX + deltaY*deltaY;
                    if ( bitSrc > thresh2 )
                     {
                      bitSrc = (bitX + bitX1 + bitY + bitY1)*0.25f;
                       changed = true;
                    }
                   else
                      bitSrc = bitDst;
                    *nextXY = bitSrc;
                }
                else
                {
                    // not near edge, so ew can avoid mod arithmetic
                   bitDst = *topXY;
                   bitX   = *(topXY-1);
                   bitY   = *(topXY-size);
                    bitX1  = *(topXY+1);
                    bitY1  = *(topXY+size);
                   deltaX = bitDst - (bitX+bitX1)*0.5f;
                   deltaY = bitDst - (bitY+bitY1)*0.5f;
//                   bitSrc = sqrt(deltaX*deltaX + deltaY*deltaY);
                   bitSrc = deltaX*deltaX + deltaY*deltaY;
                    if ( bitSrc > thresh2 )
                     {
                      bitSrc = (bitX + bitX1 + bitY + bitY1)*0.25f;
                       changed = true;
                    }
                   else
                      bitSrc = bitDst;
                    *nextXY = bitSrc;
                }
                topXY++; nextXY++;
            }
        }
    }
    pop();
}

//----------------------------------------------------------------------------

void LandScape::slope( float _thresh, int _iter )
{
    if (stack.size() < 2)
    {
        sprintf( mExecString, "SLOPE: Not enough items on the stack to slop (need 2)");
        mExecStatus = EXEC_FAILURE;
        return;
    }

    float thresh2 = _thresh * _thresh;

    int   size = stack[0]->size;
    float bitSrc, bitDst;
    bool  changed = TRUE;

    dup();

    while ( changed && _iter-- )
    {
        changed = FALSE;   // TRUE if something changes
        swap();
        float * topReal = stack[0]->real;
        float * nextReal = stack[1]->real;
        float * topXY = topReal;
        float * nextXY = nextReal;
        for ( int iy=0; iy < size; iy++ )
        {
            for ( int ix=0; ix < size; ix++ )
            {
                float   bitX,bitY,bitX1,bitY1; // values of HF elements on either side of this one
                float deltaX,deltaY;
                if (ix==0 || ix==size-1 || iy==0 || iy==size-1)
                {
                   bitDst = *topXY;
                   bitX   = Elmod(topReal,ix-1,iy);
                   bitY   = Elmod(topReal,ix,iy-1);
                   deltaX = bitDst-bitX;
                   deltaY = bitDst-bitY;
//                   bitSrc = sqrt( deltaX*deltaX + deltaY*deltaY );
                   bitSrc = deltaX*deltaX + deltaY*deltaY;
                   if (bitSrc > thresh2)
                   {  /* yes, slope exceeds threshold */
                       bitX1 = Elmod(topReal,ix+1,iy);
                       bitY1 = Elmod(topReal,ix,iy+1);
                       bitSrc  = (bitX + bitX1 + bitY + bitY1) * 0.25f;
                       changed = true;
                   }
                   else
                       bitSrc = bitDst;
                   *nextXY = bitSrc;
                }
                else
                {
                   // not near edges so we can be a little more efficient
                   bitDst = *topXY;
                   bitX   = *(topXY-1);
                   bitY   = *(topXY-size);
                   deltaX = bitDst-bitX;
                   deltaY = bitDst-bitY;
//                   bitSrc = sqrt( deltaX*deltaX + deltaY*deltaY );
                   bitSrc = deltaX*deltaX + deltaY*deltaY;
                   if (bitSrc > thresh2)
                   {  /* yes, slope exceeds threshold */
                       bitX1 = *(topXY+1);
                       bitY1 = *(topXY+size);
                       bitSrc  = (bitX + bitX1 + bitY + bitY1) * 0.25f;
                       changed = true;
                   }
                   else
                       bitSrc = bitDst;
                   *nextXY = bitSrc;
                }
                topXY++; nextXY++;
            }
        }
    }
    pop();
}

//----------------------------------------------------------------------------
//
// shaveArea   -- flattens areas in next to top terrain corresponding to
//                  non-zero locations in top terrain.  Leaves terrain on stack
//                  in the order they were when shaveArea called.
//                  epsilon and exponent act as in smooth command.  avgWeight
//                  indicates how much weight to give the average height (of 2nd
//                  terrain over all non-zero locations of top terrain).  Value of
//                  0 means to ignore average height and just smooth.  Value of 1
//                  means to ignore neighboring heights and regress all heights
//                  toward the average.  Values between 0 and 1 are acceptable.

const float convWeights2[] = 
{
    0.50,    1.00,    1.00,    1.00,    0.50,
    1.00,    2.00,    2.00,    2.00,    1.00,
    1.00,    2.00,    1.00,    2.00,    1.00,
    1.00,    2.00,    2.00,    2.00,    1.00,
    0.50,    1.00,    1.00,    1.00,    0.50,
};
const int convWeightsSize2 = sizeof(convWeights2)/sizeof(float);

void LandScape::shaveArea( float epsilon, float exponent, float avgWeight)
{
    if (stack.size() < 2)
    {
        sprintf( mExecString, "SHAVE_AREA: Stack must contain shave filter and target terrain.");
        mExecStatus = EXEC_FAILURE;
        return;
    }

   swap();
   int size       = stack[0]->size;
   float *source  = stack[0]->real;
   float *shaver  = stack[1]->real;

   int avgCnt = 0;
   float avg = 0.0f;
   int ix,iy;

   for (iy=0; iy<size; iy++)
   {
      for (ix=0; ix<size; source++, shaver++, ix++)
      {
//         AssertFatal( stack[0]->real + iy*size + ix == source, "oops");
//         AssertFatal( stack[1]->real + iy*size + ix == shaver, "oops");
         if (*shaver==0.0f)
            continue;
         avg += *source;
         avgCnt++;
      }
   }
   if (avgCnt!=0)
      avg /= (float) avgCnt;

   int     x, y, u, v, xc, yc, i, cDelta;
   float   targetVal, delta, sign;

   // compute 1/epsilon just once
   float  invEpsilon;
   if (epsilon!=0.0)
      invEpsilon = 1.0f/epsilon;

   // figure the side dimension of the convolution filter
   int convolutionSize = sqrt((double)convWeightsSize2);  
    
   // figure the sampling range, for every elt (x,y), sampling will
   // occur in the rectangle whose corners are (x-c,y-c) and (x+c,y+c)
   // where c = cDelta
   cDelta = (convolutionSize - 1)/2;

   // find the of the wieghts in the filter
   float totalWeight = 0;
   for (i = 0; i < convWeightsSize2; i++)
       totalWeight += convWeights2[i];

   float invTotalWeight = 1.0/totalWeight;

   Matrix *targetM = new Matrix(size);

   source  = stack[0]->real;
   shaver  = stack[1]->real;
   float *target   = targetM->real;

   // Visit every elt of the target matrix, compute the value of each
   // elt of the target as the weighted average of some elts of source
   for (y = 0; y < size; y++)
       for (x = 0; x < size; x++)
       {
         i = y*size + x;
         if (*shaver++==0.0f)
         {
            target[i]=source[i];
            continue;
         }

         // initialize this elt of the target matrix as zero
         targetVal = 0.0;
           
         // sample convolutionSize^2 elts of the source matrix
         const float * conWptr = convWeights;
         int vUpper = y + cDelta;
         int uUpper = x + cDelta;
         int vLower = y - cDelta;
         int uLower = x - cDelta;
         if (vUpper<size && uUpper<size && vLower>=0 && uLower>=0)
         {
            // not near edges of grid: we can be a little more efficient...
            float *fPtr = source + vLower*size + uLower;
            int rowEndToRowStart = size - (vUpper-vLower) - 1;
            for (v = vLower; v <= vUpper; v++)
            {
//                  AssertFatal(fPtr==source + v*size + uLower,"oops: error in pointer arithmatic");
                  for (u = uLower; u <= uUpper; u++)
                      targetVal += *conWptr++ * *fPtr++;
                fPtr += rowEndToRowStart;
            }
         }
         else
         { // a cosmetic bracket :)
            // near edges: need modulo arithmatic so we wrap correctly
              for (v = vLower; v <= vUpper; v++)
                  for (u = uLower; u <= uUpper; u++)
                  {
                    // figure the coordinates of u,v for sampling, wrapping
                      // around incase u,v are neg, or >= size
                      xc = (u + size)%size;
                      yc = (v + size)%size;
          
                     targetVal += *conWptr * source[yc*size + xc];
                    conWptr++;
                  }
         }

         // amortize the resulting value of this target elt over the
         // total weight of the filter
         targetVal *= invTotalWeight;
         targetVal = (1-avgWeight) * targetVal + avgWeight*avg;

         // get unsigned delta and its sign separately
         delta = targetVal - source[i];
         sign  = delta >= 0.0 ? 1.0 : -1.0;
         delta *= sign;

         if (delta >= epsilon)
            // fine, the difference is above the threshhold, apply the
            // smoothed intensity
            target[i] = targetVal - sign*epsilon;
         else
         {
            // the difference is below the threshold, curve it

            // normalize delta to 0..1
            delta *= invEpsilon;

            // apply the polynomial function
            delta = pow(delta, exponent);

            // normalize back to 0..epsilon
            delta *= epsilon;

            // apply the difference to the original intensity
            target[i] = source[i] + (delta*sign);
         }
       }
    
   pop();
   stack.push_front(targetM);
   swap();
}


//------------------------------------------------------------------------------
// NAME 
//    static void tileRotate(Point2I &origin, Point2I &tile, LS_ROT_90 rotation)
//    
// DESCRIPTION 
//    Static helper function for LandScape::flatten() and 
//
//    Rotates a tile by the specified angle
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
static void tileRotate(Point2I &tile, LS_ROT_90S rotation)
{

    Int32 tmp;

    switch (rotation)
    {
        case CCW_0:
            return;

        case CCW_90:
            tmp = tile.x;
            tile.x = -tile.y;
            tile.y = tmp;
            return;

        case CCW_180:
            tile.x = -tile.x;
            tile.y = -tile.y;
            return;

        case CCW_270:
            tmp = tile.x;
            tile.x = tile.y;
            tile.y = -tmp;
            return;
    }
}

//------------------------------------------------------------------------------
// NAME 
//    void LandScape::flatten(Point2I &origin, Vector<Point2I> *tileList)
//    
// DESCRIPTION 
//    All vertices on the height field specified by the
//    tileList is flattened to an average height.  
//
//    The points in the tileList are specified relative to the origin and
//    rotation.
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------

void LandScape::flatten(Point2I &origin, LS_ROT_90S rot, Vector<Point2I> *tileList)
{
    AssertFatal(tileList, "LandScape::flatten: null parameter"); 
    if (stack.size() < 1)
    {  
        AssertWarn(0, "Nothing on the stack to flatten");
        return;
    }

    Point2I tile;
    Int32 size;
    Vector<Point2I>::iterator iter;

    size = stack[0]->size;
    float *realPtr = stack[0]->real;

    // find footprint to average over -- all squares adjacent to
    // a tile square, excluding tile squares themselves
    float avg=0.0f;
    for (iter = tileList->begin(); iter != tileList->end(); iter++)
    {
       tile = *iter;
       tileRotate(tile, rot);
       tile += origin;
       avg += realPtr[tile.x+tile.y*size];
    }
    if (tileList->size()>0)
      avg /= tileList->size();
    flatten(origin,rot,tileList,avg);
}

void LandScape::flatten(Point2I &origin, LS_ROT_90S rot, Vector<Point2I> *tileList, Flt32 flatHeight)
{
    AssertFatal(tileList, "LandScape::flatten: null parameter"); 
    if (stack.size() < 1)
    {  
        AssertWarn(0, "Nothing on the stack to flatten");
        return;
    }

    Point2I tile;
    Int32 size;
    Vector<Point2I>::iterator iter;

    size = stack[0]->size;
    float *realPtr = stack[0]->real;


    // flatten the tiles to the specified height
    for (iter = tileList->begin(); iter != tileList->end(); iter++)
    {
        tile = *iter;
        tileRotate(tile, rot);
        tile += origin;

        // now, flatten the four vertices of the tile
        static int x_offset[] = { 0, 1, 1, 0 };
        static int y_offset[] = { 0, 0, 1, 1 };
        int x, y;
        if (tile.x<size-1 && tile.y<size-1)
           for (int i = 0; i < 4; i++) 
           {
               x = tile.x + x_offset[i];
               y = tile.y + y_offset[i];

               realPtr[y * size + x] = flatHeight; 
           }
        else
           for (int i = 0; i < 4; i++) 
           {
               x = (tile.x + x_offset[i] ) % size;
               y = (tile.y + y_offset[i] ) % size;

               realPtr[y * size + x] = flatHeight; 
           }
    }
}   

//----------------------------------------------------------------------------
// this filter goes through the detail levels found in a Phoenix
// terrain system and limits the deviation in heights between
// detail levels.  _deviation is a percentage of variation from
// the respective details scale value

void LandScape::clamp( int _detail, float _deviation )
{
   if (stack.size() < 1)
   {
      sprintf( mExecString, "CLAMP: Nothing on the stack to clamp");
      mExecStatus = EXEC_FAILURE;
      return;
   }
    
   int      width = stack[0]->size;
   float    *hfBase = stack[0]->real;
   int      wmask = width-1;

   while (--_detail >= 0)
   {
      Point2I offset(1 << _detail, width << _detail);
      float range = float(1 << _detail) * _deviation;
      int size = (width>>_detail)+1;

      Point2I pos;
      for (pos.y = 0; pos.y < size; pos.y++)
         for (pos.x = 0; pos.x < size; pos.x++)
         {
            int index = (((pos.y << _detail)&wmask) * (width)) + ((pos.x << _detail)&wmask);
            // Determin points to average, if at all.
            Point2I ho(0,0);
            switch ( ((pos.x & 1) << 1) | (pos.y & 1))
            {
               case 1: // Vertical
                  ho.set(0,1);
                  break;
               case 2: // Horizontal
                  ho.set(1,0);
                  break;
               case 3: // Diagonal
                  ho.y = 1;
                  if ((pos.x ^ pos.y) & 2)
                     ho.x = -1;
                  else
                     ho.x = 1;
                  break;
            }

            if (ho.x || ho.y)
            {
               int i1 = ((( (pos.y+ho.y) << _detail)&wmask) * (width)) + (( (pos.x+ho.x) << _detail)&wmask);
               int i2 = ((( (pos.y-ho.y) << _detail)&wmask) * (width)) + (( (pos.x-ho.x) << _detail)&wmask);

               float target = (hfBase[i1] + hfBase[i2]) * 0.5f;
               // Clamp the height
               float & here = hfBase[index];
               if (here > target + range)
                  here = target + range;
               else if (here < target - range)
                  here = target - range;
            }
         }
   }
}

//------------------------------------------------------------------------------
// NAME 
//    void LandScape::clamp( Point2I &tile, int _detail )
//    
// DESCRIPTION 
//  this filter goes through the detail levels found in a Phoenix
//  terrain system and limits the deviation in heights between
//  detail levels around the specified tile.  
//
// ARGUMENTS
//  _detail is the maximum detail level to affect
//  
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void LandScape::clamp( Point2I &tile, int _detail )
{
   static int x_offset[] = {0, 1, 1, 0};
   static int y_offset[] = {0, 0, 1, 1};
   static Flt32 deviation_allowed[] = { 0.0f, 3.0f, 12.0f, 30.0f, 60.0f, 130.0f, 300.0f, 700.0f };

   if (stack.size() < 1)
   {  
      AssertWarn(0, "Nothing on the stack to clamp");
      return;
   }
    
   int width       = stack[0]->size;
   float *hfBase     = stack[0]->real;
   Flt32 flat_height = hfBase[tile.y * width + tile.x];

   if (_detail < 1)
      return;

   // the deviation_allowed[] table has only 8 entries;
   if (_detail > 8)
      _detail = 8;

   int curDetail;
   Point2I detail_tile;

   for (curDetail = 1; curDetail <= _detail; curDetail++)
   {
      detail_tile.x = (tile.x & (0xffffffff << curDetail) );
      detail_tile.y = (tile.y & (0xffffffff << curDetail) );

      for (int i = 0; i < 4; i++)
      {
         int vertice = (detail_tile.y + (y_offset[i] << curDetail)) * width 
                          + (detail_tile.x + (x_offset[i] << curDetail));
         
         if (hfBase[vertice] < flat_height - deviation_allowed[curDetail])
         {
            hfBase[vertice] = flat_height - deviation_allowed[curDetail];   
         }
         else
         {
            if (hfBase[vertice] > flat_height + deviation_allowed[curDetail])
               hfBase[vertice] = flat_height + deviation_allowed[curDetail];
         }
      }
   }
}

//------------------------------------------------------------------------------
// NAME 
//    void LandScape::clamp( Point2I &, LS_ROT_90S, Vector<Point2I> *, int, float )
//    
// DESCRIPTION 
//  this filter goes through the detail levels found in a Phoenix
//  terrain system and limits the deviation in heights between
//  detail levels around the specified tiles.  
//
// ARGUMENTS
//  _detail is the maximum detail level to affect
//  
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void LandScape::clamp( Point2I &origin, LS_ROT_90S rot, Vector<Point2I> *tileList, int _detail )
{
    AssertFatal(tileList, "LandScape::clamp: null parameter"); 
    if (stack.size() < 1)
    {  
        AssertWarn(0, "Nothing on the stack to clamp");
        return;
    }

    Point2I tile;
    Int32 size;
    Vector<Point2I>::iterator iter;

    size = stack[0]->size;

    // smooth the tileices to the specified height
    for (iter = tileList->begin(); iter != tileList->end(); iter++)
    {
        tile = *iter;
        tileRotate(tile, rot);
        tile += origin;

        // wrap it around if off the HF
        tile.x = (tile.x + size) % size;
        tile.y = (tile.y + size) % size;

        clamp(tile,_detail);
    }
}


//----------------------------------------------------------------------------
void LandScape::mask( int _detail, float _scale )
{
   if (stack.size() < 1)
   {  
      sprintf( mExecString, "MASK: Nothing on the stack to mask");
      mExecStatus = EXEC_FAILURE;
      return;
   }

	if (_detail > 0) {
	   int width = stack[0]->size;
	   float *base = stack[0]->real;
		int dsize = 1 << _detail;
	   int dmask = dsize - 1;
		float dscale = (1.0f / dsize) * 2.0f;

	   Point2I pos;
	   for (pos.y = 0; pos.y < width; pos.y++)
	      for (pos.x = 0; pos.x < width; pos.x++)
	      {
				// Point is 0,0 on detail level points
				// 1,1 at the midpoint of the detail square.
				float x = (pos.x & dmask) * dscale - 1.0f;
				x = (x >= 0.0f)? 1.0f - x: 1.0f + x;
				float y = (pos.y & dmask) * dscale - 1.0f;
				y = (y >= 0.0f)? 1.0f - y: 1.0f + y;

				//
				base[(pos.y * width) + pos.x] *= x * y * _scale;
			}
	}
}

//----------------------------------------------------------------------------

void LandScape::diff()
{
    if (stack.size() < 1)
    {
        sprintf( mExecString, "DIFF: nothing on stack to diff" );
        mExecStatus = EXEC_FAILURE;
        return;
    }
    int   size = stack[0]->size;
    float bitDst;

    dup();

    for ( int y = 0; y < size; y++ )
        for ( int x=0; x < size; x++ )
        {
            bitDst = El(stack[0]->real,x,y);
            El(stack[0]->real,x,y) = sqrt(pow(bitDst-Elmod(stack[0]->real,x-1,y),2) + pow(bitDst-Elmod(stack[0]->real,x,y-1),2));
        }

   pop();
}

//----------------------------------------------------------------------------

void LandScape::fft( int _sign )
{
   if (stack.size() < 2)
   {  
      sprintf( mExecString, "FFT: Nothing on the stack to fft" );
      mExecStatus = EXEC_FAILURE;
      return;
   }
   
   int msize = sq(stack[0]->size);
   int half  = msize>>1;
   float *rPi = stack[0]->real;
   float *rPj = &stack[0]->real[msize-1];
   float *iPi = stack[1]->real;
   float *iPj = &stack[1]->real[msize-1];

   if ( _sign > 0 )
   {
      for ( int i=1,k=half; i<k; i++,rPi++,iPi++,rPj--,iPj-- ) 
      {
         double a,b,c,d;
         double q,r,s,t;
         a = *rPi; b = *rPj;  q=a+b; r=a-b;
         c = *iPi; d = *iPj;  s=c+d; t=c-d;
         *rPi = (q+t)*.5; *rPj = (q-t)*.5;
         *iPi = (s-r)*.5; *iPj = (s+r)*.5;
      }
      fht(stack[0]->real,msize);
      fht(stack[1]->real,msize);
   }
   else
   {
      fht(stack[0]->real,msize);
      fht(stack[1]->real,msize);
      for ( int i=1,k=half; i<k; i++,rPi++,iPi++,rPj--,iPj-- ) 
      {
         double a,b,c,d;
         double q,r,s,t;
         a = *rPi; b = *rPj;  q=a+b; r=a-b;
         c = *iPi; d = *iPj;  s=c+d; t=c-d;
         *iPi = (s+r)*0.5;  *iPj = (s-r)*0.5;
         *rPi = (q-t)*0.5;  *rPj = (q+t)*0.5;
      }
   }
}

//----------------------------------------------------------------------------

void fht( REAL *fz, int size )
{
   int k;
   REAL *fi,*fn,*gi;
   TRIG_VARS;

   for ( int k1=1,k2=0; k1<size; k1++ )
   {
      REAL a;
      for ( int k=size>>1; (!((k2^=k)&k)); k>>=1 );
      if (k1>k2)
      {
         a=fz[k1];
         fz[k1]=fz[k2];
         fz[k2]=a;
      }
   }
   for ( k=0; (1<<k)<size; k++ );
   k &= 1;
   if ( k==0 )
   {
      for ( fi=fz, fn=fz+size; fi<fn; fi+=4 )
      {
         REAL f0,f1,f2,f3;
         f1     = fi[0]-fi[1];
         f0     = fi[0]+fi[1];
         f3     = fi[2]-fi[3];
         f2     = fi[2]+fi[3];
         fi[2]  = (f0-f2);   
         fi[0]  = (f0+f2);
         fi[3]  = (f1-f3);   
         fi[1]  = (f1+f3);
      }
   }
   else
   {
      for ( fi=fz, fn=fz+size, gi=fi+1; fi<fn; fi+=8, gi+=8 )
      {
         REAL s1,c1,s2,c2,s3,c3,s4,c4,g0,f0,f1,g1,f2,g2,f3,g3;
         c1     = fi[0] - gi[0];
         s1     = fi[0] + gi[0];
         c2     = fi[2] - gi[2];
         s2     = fi[2] + gi[2];
         c3     = fi[4] - gi[4];
         s3     = fi[4] + gi[4];
         c4     = fi[6] - gi[6];
         s4     = fi[6] + gi[6];
         f1     = (s1 - s2);   
         f0     = (s1 + s2);
         g1     = (c1 - c2);   
         g0     = (c1 + c2);
         f3     = (s3 - s4);   
         f2     = (s3 + s4);
         g3     = SQRT2*c4;      
         g2     = SQRT2*c3;
         fi[4]  = f0 - f2;
         fi[0]  = f0 + f2;
         fi[6]  = f1 - f3;
         fi[2]  = f1 + f3;
         gi[4]  = g0 - g2;
         gi[0]  = g0 + g2;
         gi[6]  = g1 - g3;
         gi[2]  = g1 + g3;
      }
   }
   if (size<16) return;

   int k4;
   do
   {
      REAL  s1,c1;
      int   k1,k2,k3,kx;
      k  += 2;
      k1  = 1  << k;
      k2  = k1 << 1;
      k3  = k2 + k1;
      k4  = k2 << 1;
      kx  = k1 >> 1;
      fi  = fz;
      gi  = fi + kx;
      fn  = fz + size;
      do
      {
         REAL g0,f0,f1,g1,f2,g2,f3,g3;
         f1      = fi[0]  - fi[k1];
         f0      = fi[0]  + fi[k1];
         f3      = fi[k2] - fi[k3];
         f2      = fi[k2] + fi[k3];
         fi[k2]  = f0     - f2;
         fi[0]   = f0     + f2;
         fi[k3]  = f1     - f3;
         fi[k1]  = f1     + f3;
         g1      = gi[0]  - gi[k1];
         g0      = gi[0]  + gi[k1];
         g3      = SQRT2  * gi[k3];
         g2      = SQRT2  * gi[k2];
         gi[k2]  = g0     - g2;
         gi[0]   = g0     + g2;
         gi[k3]  = g1     - g3;
         gi[k1]  = g1     + g3;
         gi     += k4;
         fi     += k4;
      } while (fi<fn);
      TRIG_INIT(k,c1,s1);
      for ( int i=1; i<kx; i++ )
      {
         REAL c2,s2;
         TRIG_NEXT(k,c1,s1);
         c2 = c1*c1 - s1*s1;
         s2 = 2*(c1*s1);
         fn = fz + size;
         fi = fz +i;
         gi = fz +k1-i;
         do
         {
            REAL a,b,g0,f0,f1,g1,f2,g2,f3,g3;
            b       = s2*fi[k1] - c2*gi[k1];
            a       = c2*fi[k1] + s2*gi[k1];
            f1      = fi[0]     - a;
            f0      = fi[0]     + a;
            g1      = gi[0]     - b;
            g0      = gi[0]     + b;
            b       = s2*fi[k3] - c2*gi[k3];
            a       = c2*fi[k3] + s2*gi[k3];
            f3      = fi[k2]    - a;
            f2      = fi[k2]    + a;
            g3      = gi[k2]    - b;
            g2      = gi[k2]    + b;
            b       = s1*f2     - c1*g3;
            a       = c1*f2     + s1*g3;
            fi[k2]  = f0        - a;
            fi[0]   = f0        + a;
            gi[k3]  = g1        - b;
            gi[k1]  = g1        + b;
            b       = c1*g2     - s1*f3;
            a       = s1*g2     + c1*f3;
            gi[k2]  = g0        - a;
            gi[0]   = g0        + a;
            fi[k3]  = f1        - b;
            fi[k1]  = f1        + b;
            gi     += k4;
            fi     += k4;
         } while (fi<fn);
      }
      TRIG_RESET(k,c1,s1);
   } while (k4<size);
}

//----------------------------------------------------------------------------

void LandScape::clipMin( float minHeight )
{
   if (stack[0]->size < 1)
   {
      sprintf( mExecString, "CLIP_MIN: Nothing on the stack");
      mExecStatus = EXEC_FAILURE;
      return;
   }

   int length = sq(stack[0]->size);
   for ( float *fPtr = stack[0]->real; fPtr<stack[0]->real+length; fPtr++ )
      if ( *fPtr < minHeight )
         *fPtr = minHeight;
}   

//----------------------------------------------------------------------------

void LandScape::clipMax( float maxHeight )
{
   if (stack[0]->size < 1)
   {
      sprintf( mExecString, "CLIP_MAX: Nothing on the stack");
      mExecStatus = EXEC_FAILURE;
      return;
   }

   int length = sq(stack[0]->size);
   for ( float *fPtr = stack[0]->real; fPtr<stack[0]->real+length; fPtr++ )
      if ( *fPtr > maxHeight )
         *fPtr = maxHeight;
}   
