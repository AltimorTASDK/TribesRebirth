//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

#ifndef _GRDHEIGHTS_H_
#define _GRDHEIGHTS_H_

// This file Should be included after normal grid stuff.
//#include "gridBlock.h"

//---------------------------------------------------------------------------

struct GridHeight
{
   enum SplitType {
      Split45,
      Split135,
   };
   enum FaceID {
      FaceA,            // Face containing edge vertex[0] -> vertex[1]
      FaceB,            // Face containing edge vertex[2] -> vertex[3]
   };

	float width;						// Square width
	SplitType split;
	GridBlock::Height* heights;	// Set to corner height
	int dx,dy;							// HeightMap stepping

   //
   FaceID getFace45(const Point2F& off);
   FaceID getFace135(const Point2F& off);

   void getNormal(const Point2F& pos,Vector3F* normal);
   void getNormal45a(Vector3F* v1);
   void getNormal45b(Vector3F* v2);
   void getNormal135a(Vector3F* v1);
   void getNormal135b(Vector3F* v2);

   float getHeight(const Point2F& off);
   float getHeight45a(const Point2F& off);
   float getHeight45b(const Point2F& off);
   float getHeight135a(const Point2F& off);
   float getHeight135b(const Point2F& off);

   bool  intersect(const LineSeg3F& line,Point3F* ip);
};

inline GridHeight::FaceID GridHeight::getFace45(const Point2F& off)
{
   return (off.x > off.y)? FaceB: FaceA;
}

inline GridHeight::FaceID  GridHeight::getFace135(const Point2F& off)
{
   return (width - off.x > off.y)? FaceA: FaceB;
}

#endif
