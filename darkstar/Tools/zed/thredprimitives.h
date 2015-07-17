////////////////////////////////////////////////////////////////////
// THRED - The ThreeD Editor                                      //
// Copyright (c) 1996 											  //
// Jonathan Mavor												  //
// All Rights Reserved											  //
////////////////////////////////////////////////////////////////////


#ifndef _THREDPRIMITIVES_H_
#define _THREDPRIMITIVES_H_

#include <math.h>
#include "GlobalEnvironment.h"
#include "m_point.h"
#include "g_surfac.h"


// some bit settings we can use
#define BIT_0	(0x0001)
#define BIT_1	(0x0002)
#define BIT_2	(0x0004)
#define BIT_3	(0x0008)
#define BIT_4	(0x0010)
#define BIT_5	(0x0020)
#define BIT_6	(0x0040)
#define BIT_7	(0x0080)
#define BIT_8	(0x0100)
#define BIT_9	(0x0200)
#define BIT_10	(0x0400)
#define BIT_11	(0x0800)
#define BIT_12	(0x1000)
#define BIT_13	(0x2000)
#define BIT_14	(0x4000)
#define BIT_15	(0x8000)


typedef enum
{
  XY_SHEAR = 0,
  XZ_SHEAR,
  YX_SHEAR,
  YZ_SHEAR,
  ZX_SHEAR,
  ZY_SHEAR
} ShearType;




// a rounder
#define RoundFloat(x)  floor(x + 0.5 )


// Some math constants
#define M_PI_VALUE		3.14159265358979323846
#define M_TOTAL_DEGREES	8192

// the minimum z we can see.
#define MIN_Z_VISIBLE	(double)16.0


// some polygon constants
#define MAX_POLYGON_VERTS 32

// This structure has room for a regular point
// as well as room for the point to be transformed
struct ThredPoint {
	double X, Y, Z;
	ThredPoint() { ; }
	ThredPoint(double x, double y, double z) { X=x; Y=y; Z=z; }
	ThredPoint operator+(ThredPoint& Other) { return ThredPoint(X + Other.X, Y + Other.Y, Z + Other.Z); }
	ThredPoint operator-(ThredPoint& Other) { return ThredPoint(X - Other.X, Y - Other.Y, Z - Other.Z); }
	ThredPoint operator/(ThredPoint& Other) { return ThredPoint(X / Other.X, Y / Other.Y, Z / Other.Z); }
   void VectorSubtract(ThredPoint& a, ThredPoint& b)
   {
      X = a.X - b.X;
      Y = a.Y - b.Y;
      Z = a.Z - b.Z;
   }
   void VectorAdd(ThredPoint& a, ThredPoint& b)
   {
      X = a.X + b.X;
      Y = a.Y + b.Y;
      Z = a.Z + b.Z;
   }
   void MakeUnitVector(ThredPoint& Origin, ThredPoint &Offset) 
   {
      // calculate deltas
      *this = Offset - Origin;

      // get length
      double Length = sqrt(X*X +Y*Y +Z*Z );

      // divide
      X /= Length;
      Y /= Length;
      Z /= Length;
   }
   void Normalize()
   {
      double Length = sqrt(X*X +Y*Y +Z*Z );
      X /= Length;
      Y /= Length;
      Z /= Length;
   }
   double Length()
   {
      return( sqrt( X*X + Y*Y + Z*Z ) );
   }
   void Snap( const double val )
   {
      ( X < 0.f ) ? ( X -= val / 2 ) : ( X += val / 2 );
      ( Y < 0.f ) ? ( Y -= val / 2 ) : ( Y += val / 2 );
      ( Z < 0.f ) ? ( Z -= val / 2 ) : ( Z += val / 2 );
      X -= fmod( X, val );
      Y -= fmod( Y, val );
      Z -= fmod( Z, val );
   }
};


struct ThredPolygon
{
    ThredPolygon() {lm = NULL; volumestate = 0; mTextureScaleShift = 0; mApplyAmbient = true; };

	int Colour;
	int material_index;
	int NumberOfPoints;

    // DPW - Added for lighting
    //Point3F Normal;

	ThredPoint Points[MAX_POLYGON_VERTS];
    
    // DPW - On set of texture UVs per vertex
    Point2I           mTextureSize;
    Point2F           mTextureUV[MAX_POLYGON_VERTS];
    Point2I           mTextureOffset;
    GFXTextureHandle  texture_handle;
    GFXLightMap*      lm;
    UInt32            volumestate;
    UInt8             mTextureScaleShift;
    bool              mApplyAmbient;
};

// Standard bounding box
class ThredBox {
public:
	ThredPoint mSmallest;
	ThredPoint mLargest;
    ThredPoint mCenter;
    ThredPoint mSize;

    // Top, Front & Side Polygon used to render box in ortho view
    ThredPolygon  TopPolygon;
    ThredPolygon  FrontPolygon;
    ThredPolygon  SidePolygon;

	ThredBox() { ClearBox(); }
void ClearBox() { mSmallest.X = mSmallest.Y = mSmallest.Z = 5555555;
				  mLargest.X = mLargest.Y = mLargest.Z = -5555555;
                  mCenter.X = mCenter.Y = mCenter.Z = 0;
                  mSize.X = mSize.Y = mSize.Z = 0;
                }
// This function checks if there is a "special" point in
// our bounding box
double GetVolume()
{
	double Volume;
	Volume = ((mLargest.X - mSmallest.X)/1000) * ((mLargest.Y - mSmallest.Y)/1000) * ((mLargest.Z - mSmallest.Z )/1000);
	return Volume;
}

void AddPoint(ThredPoint Point) 
	{ 
		if(Point.X < mSmallest.X )
			mSmallest.X = Point.X;
		if(Point.Y < mSmallest.Y )
			mSmallest.Y = Point.Y;
		if(Point.Z < mSmallest.Z )
			mSmallest.Z = Point.Z;
		if(Point.X > mLargest.X )
			mLargest.X = Point.X;
		if(Point.Y > mLargest.Y )
			mLargest.Y = Point.Y;
		if(Point.Z > mLargest.Z )
			mLargest.Z = Point.Z;

        mSize.X = (mLargest.X - mSmallest.X);
        mSize.Y = (mLargest.Y - mSmallest.Y);
        mSize.Z = (mLargest.Z - mSmallest.Z);

//        mCenter.X = (mSmallest.X + mLargest.X) / 2.0;
//        mCenter.Y = (mSmallest.Y + mLargest.Y) / 2.0;
//        mCenter.Z = (mSmallest.Z + mLargest.Z) / 2.0;
	}
int CheckIntersection(ThredBox OtherBox )
	{
        double  dist;

        dist = OtherBox.mSmallest.X - mLargest.X;
        if (dist <= 0.01)
        {
          dist = OtherBox.mLargest.X - mSmallest.X;
          if (dist >= -0.01)
          {
            dist = OtherBox.mSmallest.Y - mLargest.Y;
            if (dist <= 0.01)
            {
              dist = OtherBox.mLargest.Y - mSmallest.Y;
              if (dist >= -0.01)
              {
                dist = OtherBox.mSmallest.Z - mLargest.Z;
                if (dist <= 0.01)
                {
                  dist = OtherBox.mLargest.Z - mSmallest.Z;
                  if (dist >= -0.01)
                  {
                    return 1;
                  }
                }
              }
            }
          }
        }

#if 0
		if(OtherBox.mSmallest.X <= mLargest.X )
			if(OtherBox.mLargest.X >= mSmallest.X ) 
				if(OtherBox.mSmallest.Y <= mLargest.Y )
					if(OtherBox.mLargest.Y >= mSmallest.Y )
						if(OtherBox.mSmallest.Z <= mLargest.Z )
							if(OtherBox.mLargest.Z >= mSmallest.Z )
								return 1;
#endif
		return 0;
	}
// This checks if the otherbox is COMPLETELY inside
// this box.
int CheckSpecialIntersection(ThredBox OtherBox )
	{
		if(mSmallest.X == 0 ) {
			if(OtherBox.mSmallest.Y >= mSmallest.Y )
				if(OtherBox.mLargest.Y <= mLargest.Y )
					if(OtherBox.mSmallest.Z >= mSmallest.Z )
						if(OtherBox.mLargest.Z <= mLargest.Z )
							return 1;
		}
		else if(mSmallest.Y == 0 ) {
			if(OtherBox.mSmallest.X >= mSmallest.X )
				if(OtherBox.mLargest.X <= mLargest.X )
					if(OtherBox.mSmallest.Z >= mSmallest.Z )
						if(OtherBox.mLargest.Z <= mLargest.Z )
							return 1;
		}
		else {
			if(OtherBox.mSmallest.Y >= mSmallest.Y )
				if(OtherBox.mLargest.Y <= mLargest.Y )
					if(OtherBox.mSmallest.X >= mSmallest.X )
						if(OtherBox.mLargest.X <= mLargest.X )
							return 1;
		}
		return 0;
	}
// This function checks if there is a "special" point in
// our bounding box
int CheckPointIntersection(ThredPoint Point)
{
		if(Point.X == 0 ) {
			if(mSmallest.Y <= Point.Y )
				if(mLargest.Y >= Point.Y )
					if(mSmallest.Z <= Point.Z )
						if(mLargest.Z >= Point.Z )
							return 1;
		}
		else if(Point.Y == 0 ) {
			if(mSmallest.X <= Point.X )
				if(mLargest.X >= Point.X )
					if(mSmallest.Z <= Point.Z )
						if(mLargest.Z >= Point.Z )
							return 1;
		}
		else {
			if(mSmallest.Y <= Point.Y )
				if(mLargest.Y >= Point.Y )
					if(mSmallest.X <= Point.X )
						if(mLargest.X >= Point.X )
							return 1;
		}
		return 0;
}

};

#endif
