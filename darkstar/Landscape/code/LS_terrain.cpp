//----------------------------------------------------------------------------
//   
//  $Workfile:   LS_cmplx.cpp  $
//  $Version$
//  $Revision:   1.00  $
//    
//  DESCRIPTION:
//        LandScape complex or compund methods
//        
//  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
// 
//----------------------------------------------------------------------------

#include "LS.h"
#include <m_trig.h>

//----------------------------------------------------------------------------

void LandScape::terrain( int size, float _h )
{
    Matrix *real = new Matrix(size);
    stack.push_front(real);
    Matrix *imag = new Matrix(size);
    stack.push_front(imag);

    fillNormal(_h);
    fft(1);              // perform forward fft

    stack.pop_front();   // pop imaginaries back off the stack
    delete imag;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//--------------------------------------------------------------------------

struct Plasma {
	float* data;
	int width;
	float deviation;
	Random* rand;
	//
	float& pixel(int x,int y);
	float avrgColor(float c1,float c2);
	float avrgX(int x1, int ax, int x2,int y);
	float avrgY(int x, int y1, int ay, int y2);
	void recurse(int minX,int minY,int maxX,int maxY);
};


//--------------------------------------------------------------------------

inline float& Plasma::pixel(int x,int y)
{
	return data[y * width + x];
}

inline float Plasma::avrgColor(float c1,float c2)
{
	float offset = deviation * (rand->getFloat() - 0.5f);
	return (c1 + c2) * 0.5 + offset;
}

inline float Plasma::avrgX(int x1, int ax, int x2,int y)
{
	float &p = pixel(ax,y);
	return p = (p != 0.0f)? p: avrgColor(pixel(x1,y),pixel(x2,y));
}

inline float Plasma::avrgY(int x, int y1, int ay, int y2)
{
	float &p = pixel(x,ay);
	return p = (p != 0.0f)? p: avrgColor(pixel(x,y1),pixel(x,y2));
}

void Plasma::recurse(int minX,int minY,int maxX,int maxY)
{
	int avgX = (minX + maxX) >> 1;
	int avgY = (minY + maxY) >> 1;

	float ac = 0.0f;
	ac += avrgX(minX,avgX,maxX,minY);
	ac += avrgX(minX,avgX,maxX,maxY);
	ac += avrgY(minX,minY,avgY,maxY);
	ac += avrgY(maxX,minY,avgY,maxY);
	pixel(avgX,avgY) = ac * 0.25;

	if ((maxX - minX) <= 1 || (maxY - minY) <= 1)
		return;

	recurse(minX,minY,avgX,avgY);
	recurse(avgX,minY,maxX,avgY);
	recurse(avgX,avgY,maxX,maxY);
	recurse(minX,avgY,avgX,maxY);
}

//----------------------------------------------------------------------------

void LandScape::plasma( int size, float deviation )
{
	Matrix *real = new Matrix(size);
	stack.push_front(real);
	clr(0.0f);

	Plasma pp;
   pp.width = stack[0]->size;
   pp.data = stack[0]->real;
	pp.deviation = deviation;
	pp.rand = &rand;

	int minX = 0, maxX = stack[0]->size - 1;
	int minY = 0, maxY = stack[0]->size - 1;

	// data setup
	pp.pixel(minX,minY) = pp.avrgColor(-deviation,deviation);
	pp.pixel(maxX,minY) = pp.avrgColor(-deviation,deviation);
	pp.pixel(maxX,maxY) = pp.avrgColor(-deviation,deviation);
	pp.pixel(minX,maxY) = pp.avrgColor(-deviation,deviation);
	pp.recurse(minX,minY,maxX,maxY);
}

