

// ConstructiveBsp.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "THRED.h"
#include "ConstructiveBsp.h"
#include "3dmworld.h"
#include "BspManagerDialog.h"
#include "ThredBrush.h"
#include "ThredParser.h"
#include "itrgeometry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//===============================================================
// Calculate the plane constants of the bsp node so
// that we can use all of these other great functions.
//===============================================================
void CBspNode::CalculatePlaneConstants()
{
#if 0
	int MaxVertex;
	ThredPoint Vector1, Vector2;
	double Length, XUnit, YUnit, ZUnit;

	double VectorLengthOne, VectorLengthTwo, CosAngle, DotProduct;
	int NextVertex, PrevVertex, VectorOriginVertex;

	// Create two vectors in the polygon plane
	MaxVertex = Polygon.NumberOfPoints - 1;

	// Go through all of the vertices of the polygon looking
	// for a vert that will give us a good cross product.
	for(VectorOriginVertex = 0; VectorOriginVertex < Polygon.NumberOfPoints; VectorOriginVertex++) {

		// Get the next vertex and previous vertex from the vectororiginvertex
		NextVertex = (VectorOriginVertex+1) < Polygon.NumberOfPoints ? (VectorOriginVertex+1) : 0;
		PrevVertex = (VectorOriginVertex-1) >= 0 ? (VectorOriginVertex-1) : MaxVertex;

		// Get the vectors
		Vector1.X = Polygon.Points[NextVertex].X - Polygon.Points[VectorOriginVertex].X;
		Vector1.Y = Polygon.Points[NextVertex].Y - Polygon.Points[VectorOriginVertex].Y;
		Vector1.Z = Polygon.Points[NextVertex].Z - Polygon.Points[VectorOriginVertex].Z;

		Vector2.X = Polygon.Points[PrevVertex].X - Polygon.Points[VectorOriginVertex].X;
		Vector2.Y = Polygon.Points[PrevVertex].Y - Polygon.Points[VectorOriginVertex].Y;
		Vector2.Z = Polygon.Points[PrevVertex].Z - Polygon.Points[VectorOriginVertex].Z;

		// Check if the vectors are co-linear
		DotProduct = (Vector1.X*Vector2.X) + (Vector1.Y*Vector2.Y) + (Vector1.Z*Vector2.Z);

		// Get the lengths of the vectors
		VectorLengthOne = sqrt((double)((Vector1.X*Vector1.X) + (Vector1.Y*Vector1.Y) + (Vector1.Z*Vector1.Z)));
		VectorLengthTwo = sqrt((double)((Vector2.X*Vector2.X) + (Vector2.Y*Vector2.Y) + (Vector2.Z*Vector2.Z)));

		// Get the cos of the angle between them
		CosAngle = DotProduct / (VectorLengthOne*VectorLengthTwo);

		// We are safe if these are not co-linear
		if(fabs(1.0 - fabs(CosAngle)) > PARALLEL_VECTOR_ERROR)
			break;
	}


	// Check if the polygon is FUCKED up bad.
	_ASSERTE(VectorOriginVertex <= Polygon.NumberOfPoints);

	// Get the cartesian constants for the plane
	XUnit = (Vector1.Y * Vector2.Z) - (Vector2.Y * Vector1.Z);
	YUnit = -((Vector1.X * Vector2.Z) - (Vector2.X * Vector1.Z));
	ZUnit = (Vector1.X * Vector2.Y) - (Vector2.X * Vector1.Y);

	Length = sqrt((double)(XUnit*XUnit + YUnit*YUnit + ZUnit*ZUnit) );

	XUnit = XUnit / Length;
	YUnit = YUnit / Length;
	ZUnit = ZUnit / Length;

	// Save constants
	ConstantA = XUnit;
	ConstantB = YUnit;
	ConstantC = ZUnit;
#endif

    if (Polygon.NumberOfPoints > 2)
    {
      // Calculate the cross product of the vertices to get the normal
      Point3F vect1, vect2, normal;

      vect1.x = Polygon.Points[0].X - Polygon.Points[Polygon.NumberOfPoints-1].X;
      vect1.y = Polygon.Points[0].Y - Polygon.Points[Polygon.NumberOfPoints-1].Y;
      vect1.z = Polygon.Points[0].Z - Polygon.Points[Polygon.NumberOfPoints-1].Z;

      vect2.x = Polygon.Points[1].X - Polygon.Points[0].X;
      vect2.y = Polygon.Points[1].Y - Polygon.Points[0].Y;
      vect2.z = Polygon.Points[1].Z - Polygon.Points[0].Z;

      m_cross(vect1, vect2, &normal);
      normal.normalize();

      ConstantA = normal.x;
      ConstantB = normal.y;
      ConstantC = normal.z;

	  // calculate the D constant by subbing a point into the equation
	  // ax + by + cz + d = 0
	  // d = -ax -by -cz
	  ConstantD =
	   - ConstantA * Polygon.Points[0].X
	   - ConstantB * Polygon.Points[0].Y
	   - ConstantC * Polygon.Points[0].Z;
    }
}


//============================================================
// We want to assign this polygon to the node and then
// calculate the plane constants
//============================================================
void CBspNode::AssignPolygon(ThredPolygon* PolyPtr)
{
    Point2I shift_val;

//    shift_val.x = Polygon.mTextureShift.x;
//    shift_val.y = Polygon.mTextureShift.y;

	// make the poly
	Polygon = *PolyPtr;

//    Polygon.mTextureShift.x = shift_val.x;
//    Polygon.mTextureShift.y = shift_val.y;

	// make up a nice new colour
	Polygon.Colour = (rand()%250)+4;
	CalculatePlaneConstants();

    // Set texture handle to invalid...
//    Polygon.texture_handle = 0;

    if (mBrushPtr)
    {
      //normalizeTexture(&Polygon, mBrushPtr);
    }
#ifdef _DEBUG
	CheckFace();
#endif
}


//===============================================================
// This function classifies what side of the bsp node
// this particular point is on
//===============================================================
int CBspNode::ClassifyPoint(ThredPoint* Point)
{
   // get the distance precision from the app itself
   double DistancePrecision = theApp.m_planeDistancePrecision;
	double SideCheck;

	SideCheck = ConstantA * Point->X + ConstantB * Point->Y 
		+ ConstantC * Point->Z + ConstantD;

	if( SideCheck > DistancePrecision )
		return( POLYGON_IN_FRONT );

	if( SideCheck < -DistancePrecision )
		return( POLYGON_IN_BACK );

	return( POLYGON_IN );

}

//===============================================================
// This function will tell me which side of the node
// the polygon is on
//===============================================================
int CBspNode::ClassifyPolygon(ThredPolygon* Polygon)
{
	int PointInFront, PointInBack, PointIn, VertStatus;

	PointInFront = PointInBack = PointIn = FALSE;

	// Go through all the vertices in the polygon, checking which
	// side of the parent plane the polygon falls on
	for(int VertIndex = 0;VertIndex < Polygon->NumberOfPoints;VertIndex++) {

		VertStatus = ClassifyPoint(&Polygon->Points[VertIndex]);

		// See if the point is in front(or in) or back
		if(VertStatus == POLYGON_IN_FRONT)
			PointInFront = TRUE;
		if(VertStatus == POLYGON_IN_BACK)
			PointInBack = TRUE;
		if(VertStatus == POLYGON_IN)
			PointIn = TRUE;
	}

	// Check if the poly is completely in front of the parent poly
	if(PointInFront == TRUE && PointInBack == FALSE) 
   {
		return POLYGON_IN_FRONT;
	}
	// Check if the poly is completely in back of the parent poly
	else if(PointInBack == TRUE && PointInFront == FALSE) 
   {
		return POLYGON_IN_BACK;
	}
	// We need a split'n
	else {
		// Check if we have a coplanar
		if(PointInFront == FALSE && PointInBack == FALSE && PointIn == TRUE ) {
			double DotProduct;

			// Add to front only if the coplanar planes point in
			// same direction, else add to back

			// we need to make a fake node here to get the dot product.
			// this node will just be discarded
			CBspNode PolyNode;
			PolyNode.AssignPolygon(Polygon );

			// Get the dot product between the two plane unit normals
			// Here, the dot product = cosine of angle between planes
			DotProduct = (ConstantA * PolyNode.ConstantA)
			+ (ConstantB * PolyNode.ConstantB)
			+ (ConstantC * PolyNode.ConstantC);

			// If the cosine is 1, the planes face the same direction,
			// so add to front.  (Use > 0 check just in case)
			if(DotProduct > 0) {
				return POLYGON_COPLANAR_SAME;
//				return 	POLYGON_COPLANAR;
			}
			else 
         {
//				return POLYGON_IN_BACK;
				return POLYGON_COPLANAR_REVERSE;
			}
		}
		// Else we have a split
		else {
			return POLYGON_SPLIT;
		}
	}
}


//===============================================================
// This function will split a polygon.  The new polygon
// will be on the back side.  The old polygon is modified
// to be on the frontside
//===============================================================
ThredPolygon* CBspNode::SplitPolygon(ThredPolygon* SplitPolygon, ThredPolygon* NewPolygon)
{
	//bspPolygonData *ParentPtr, *SplitPtr, FrontHalf, BackHalf;
	int FrontVertexCount, BackVertexCount;
	int LastSideCheck, SideCheck;
	int SplitCount = 0;
	ThredPolygon FrontHalf, BackHalf;
	ThredPoint  NewPoint;
    Point2F     TexturePoint;


	FrontVertexCount = BackVertexCount = 0;
	LastSideCheck = POLYGON_IN;

	// Loop through all the split polygon's vertices and create the back
	// and front polygons, creating new vertices if we need them
	for(int VertIndex = 0;VertIndex < SplitPolygon->NumberOfPoints;VertIndex++)
    {
		// Check which side the point is on
		SideCheck = ClassifyPoint(&SplitPolygon->Points[VertIndex]);
			
		// If the point is in front
		if(SideCheck == POLYGON_IN_FRONT)
        {
			// If the previous point was not behind, add as is
			if(LastSideCheck == POLYGON_IN_FRONT || LastSideCheck == POLYGON_IN)
            {
				FrontHalf.Points[FrontVertexCount] = SplitPolygon->Points[VertIndex];

				FrontVertexCount++;
			}
			// If not, we need a split
			else
            {
				SplitCount++;

				// get the new point
				IntersectLinePlane(&SplitPolygon->Points[VertIndex-1], 
					&SplitPolygon->Points[VertIndex], &NewPoint);//,

				// assign the verts
				FrontHalf.Points[FrontVertexCount] = NewPoint;

				FrontVertexCount++;

                // Assign poly points
				FrontHalf.Points[FrontVertexCount] = SplitPolygon->Points[VertIndex];

				FrontVertexCount++;

                // Assign poly points
				BackHalf.Points[BackVertexCount] = NewPoint;

				BackVertexCount++;
			}
		}

		// If the point is behind
		else if(SideCheck == POLYGON_IN_BACK) {
			// If the previous point was not in front, add as is
			if(LastSideCheck == POLYGON_IN_BACK || LastSideCheck == POLYGON_IN)
            {
				BackHalf.Points[BackVertexCount] = SplitPolygon->Points[VertIndex];
				BackVertexCount++;
			}
			// If not, we need a split
			else
            {
				SplitCount++;

				// get the new point
				IntersectLinePlane(&SplitPolygon->Points[VertIndex-1], 
					&SplitPolygon->Points[VertIndex], &NewPoint);//,

				// assign the new verts
				BackHalf.Points[BackVertexCount] = NewPoint;

				BackVertexCount++;

                // Assign poly points
				BackHalf.Points[BackVertexCount] = SplitPolygon->Points[VertIndex];

				BackVertexCount++;

                // Assign poly points
				FrontHalf.Points[FrontVertexCount] = NewPoint;

				FrontVertexCount++;
			}
		}

		// If the point is in the plane
		else
        {
			SplitCount++;

			// Add it to front side
			FrontHalf.Points[FrontVertexCount] = SplitPolygon->Points[VertIndex];

			FrontVertexCount++;

			// Add it to back side
			BackHalf.Points[BackVertexCount] = SplitPolygon->Points[VertIndex];

			BackVertexCount++;
		}

		// The new side check is now old
		LastSideCheck = SideCheck;
	}

	// Here we want to check if the last and first vertices are on opposite
	// sides in which case the split is required on both sides.

	// Check which side the point is on
	SideCheck = ClassifyPoint(&SplitPolygon->Points[0] );

	if((LastSideCheck == POLYGON_IN_BACK && SideCheck == POLYGON_IN_FRONT)
	|| (LastSideCheck == POLYGON_IN_FRONT && SideCheck == POLYGON_IN_BACK))
    {
		SplitCount++;

		// get the new point
		IntersectLinePlane(&SplitPolygon->Points[SplitPolygon->NumberOfPoints - 1], 
			&SplitPolygon->Points[0], &NewPoint);//,

        // Assign the point
		FrontHalf.Points[FrontVertexCount] = NewPoint;

		FrontVertexCount++;

        // Assign the point
		BackHalf.Points[BackVertexCount] = NewPoint;

		BackVertexCount++;
	}

	// if this isn't 2 we had a convex polygon.  That is VERY bad...
	_ASSERTE(SplitCount == 2 );

	// assign the number of points
	FrontHalf.NumberOfPoints = FrontVertexCount;
	BackHalf.NumberOfPoints = BackVertexCount;

	// The new front split polygon will now replace the old split polygon
	for(VertIndex = 0;VertIndex < FrontHalf.NumberOfPoints;VertIndex++)
    {
		SplitPolygon->Points[VertIndex] = FrontHalf.Points[VertIndex];
	}
	SplitPolygon->NumberOfPoints = FrontHalf.NumberOfPoints;

	// assign to the newpolgyon
	*NewPolygon = BackHalf;

    // Renormalize the texture coordinates now...
    normalizeTexture(NewPolygon, mBrushPtr);//, Point2F(0.0, 0.0));
    normalizeTexture(SplitPolygon, mBrushPtr);//, Point2F(0.0, 0.0));
    
    NewPolygon->volumestate = SplitPolygon->volumestate;

   // copy the texture shift scale
   NewPolygon->mTextureScaleShift = SplitPolygon->mTextureScaleShift;
   
   // copy the ambient flag
   NewPolygon->mApplyAmbient = SplitPolygon->mApplyAmbient;
   
	// return the back polygon.  We already have the front one
	return NewPolygon;
}


//===============================================================
// This function is used to calculate where a line intersects
// a plane so that we can make a new vertex out of it when
// splitting polygons across a plane
//===============================================================
//int CBspNode::IntersectLinePlane(ThredPoint* Point1, ThredPoint* Point2, ThredPoint* NewPoint, Point2F* TPoint1, Point2F* TPoint2, Point2F* TexturePoint)
int CBspNode::IntersectLinePlane(ThredPoint* Point1, ThredPoint* Point2, ThredPoint* NewPoint)
{
	double  VectorX, VectorY, VectorZ;
	double  Length;
	double  t;

	// Create a unit vector from point 1 to point 2
	VectorX = Point2->X - Point1->X;
	VectorY = Point2->Y - Point1->Y;
	VectorZ = Point2->Z - Point1->Z;

	Length = sqrt(VectorX*VectorX + VectorY*VectorY + VectorZ*VectorZ);

	VectorX /= Length;
	VectorY /= Length;
	VectorZ /= Length;

	// Calculate parametric intersection point
	t = -(ConstantA * Point1->X + ConstantB * Point1->Y + ConstantC * Point1->Z + ConstantD)
		/ (ConstantA * VectorX + ConstantB * VectorY + ConstantC * VectorZ);

	// Calculate real intersection point
	VectorX = Point1->X + (VectorX * t);
	VectorY = Point1->Y + (VectorY * t);
	VectorZ = Point1->Z + (VectorZ * t);

	NewPoint->X = VectorX;
	NewPoint->Y = VectorY;
	NewPoint->Z = VectorZ;

	return 0;
}


//============================================================
// We want to assign this polygon to the node and then
// calculate the plane constants
//============================================================
void CBspNode::AssignReversePolygon(ThredPolygon* PolyPtr)
{
	// make the poly
	Polygon = *PolyPtr;

	// go through and copy over the verts in reverse order
	Polygon.NumberOfPoints = PolyPtr->NumberOfPoints;

	// assign the polygon in the reverse.. so that the plane is inverted
	int CurTo, CurFrom = PolyPtr->NumberOfPoints-1;
	for(CurTo = 0; CurTo < PolyPtr->NumberOfPoints; CurTo++, CurFrom--) {
		Polygon.Points[CurTo] = PolyPtr->Points[CurFrom];
	}

	CalculatePlaneConstants();
}


//===============================================================
// This function will create a NEW node that has the same
// plane constants as this node but that intersects
// the edges of the bounding box passed.
//===============================================================
#define	BOGUS_RANGE	10000
#define VEC_EPSILON	0.001
void CBspNode::CalcPolyFromPlane(CBspNode* p, ThredPolygon& NewPoly)
{
	// by default we use the 
	int	x;
	double	max, v;
	
	// invert constantb
	p->ConstantA = -p->ConstantA;
	p->ConstantB = -p->ConstantB;
	p->ConstantC = -p->ConstantC;

	// find the major axis

	max = -BOGUS_RANGE;
	x = -1;
	// Here we are looking for the axis that has
	// the largest plane constant so we can
	// make it as the dominant axis
	v = fabs(p->ConstantA );
	if(v > max ) {
		x = 0;
		max = v;
	}
	v = fabs(p->ConstantB );
	if(v > max ) {
		x = 1;
		max = v;
	}
	v = fabs(p->ConstantC );
	if(v > max ) {
		x = 2;
		max = v;
	}
	_ASSERTE (x != -1);

	ThredPoint vup;
	vup.X = 0;
	vup.Y = 0;
	vup.Z = 0;
	switch (x)
	{
	case 0:
	case 1:
		vup.Z = 1;
		break;		
	case 2:
		vup.X = 1;
		break;		
	}

	// calc dot product between vup and normal of plane
	v = vup.X*p->ConstantA + vup.Y*p->ConstantB + vup.Z*p->ConstantC;

	// MA the vup
//	VectorMA (vup, -v, p->normal, vup);
	vup.X = vup.X - v * p->ConstantA;
	vup.Y = vup.Y - v * p->ConstantB;
	vup.Z = vup.Z - v * p->ConstantC;
//	vup.X = vup.X + v * p->ConstantA;
//	vup.Y = vup.Y + v * p->ConstantB;
//	vup.Z = vup.Z + v * p->ConstantC;

	// normalize man
	vup.Normalize();
		
	// scale
	ThredPoint org;
	org.X = p->ConstantA * p->ConstantD;
	org.Y = p->ConstantB * p->ConstantD;
	org.Z = p->ConstantC * p->ConstantD;

	// get the cross product
	ThredPoint wright;
	wright.X = vup.Y*p->ConstantC - vup.Z * p->ConstantB;
	wright.Y = (vup.Z*p->ConstantA - vup.X * p->ConstantC);
	wright.Z = vup.X*p->ConstantB - vup.Y * p->ConstantA;

	// scale vup
	vup.X *= 8192;
	vup.Y *= 8192;
	vup.Z *= 8192;
	// scale wright
	wright.X *= 8192;
	wright.Y *= 8192;
	wright.Z *= 8192;


	// project a really big	axis aligned box onto the plane
	NewPoly.NumberOfPoints = 4;
	NewPoly.Points[0].VectorSubtract(org, wright );
	NewPoly.Points[0].VectorAdd(NewPoly.Points[0], vup);

	NewPoly.Points[1].VectorAdd(org, wright);
	NewPoly.Points[1].VectorAdd(NewPoly.Points[1], vup);

	NewPoly.Points[2].VectorAdd(org, wright);
	NewPoly.Points[2].VectorSubtract(NewPoly.Points[2], vup);

	NewPoly.Points[3].VectorSubtract(org, wright);
	NewPoly.Points[3].VectorSubtract(NewPoly.Points[3], vup);

	// invert constantb
	p->ConstantA = -p->ConstantA;
	p->ConstantB = -p->ConstantB;
	p->ConstantC = -p->ConstantC;

}


void CBspNode::CalcBackPolyFromPlane(CBspNode* p, ThredPolygon& NewPoly)
{
	CBspNode Node;


	// flip the plane constants
	Node.AssignPolygon(&NewPoly);
	Node.ConstantA  = 0 - p->ConstantA;
	Node.ConstantB  = 0 - p->ConstantB;
	Node.ConstantC  = 0 - p->ConstantC;
	Node.ConstantD  = 0 - p->ConstantD;

	// calc the plane
	CalcPolyFromPlane(&Node, NewPoly);
}


// This function will go through the faces of a
// polygon that is assign and see if all of the
// points are on the plane.
void CBspNode::CheckFace()
{
#if 0
	// go through each point and classify it
	for(int Cur = 0; Cur < Polygon.NumberOfPoints; Cur++ ) {

		if(ClassifyPoint(&Polygon.Points[Cur] ) != POLYGON_IN ) {
			TRACE0("Nonplanar polygon!!!\n");
			_ASSERT(0);
		}
	}
#endif
}


void CBspNode::RotateFaceTexture(double angle, Point2F center)
{
  double  rotation;
  double  tmp_x, tmp_y;

  // angle is in degrees, so convert to radians...
  rotation = (angle / 180.0)*M_PI_VALUE;

  for (int i = 0; i < Polygon.NumberOfPoints; i++)
  {
    tmp_x = Polygon.mTextureUV[i].x - center.x;
    tmp_y = Polygon.mTextureUV[i].y - center.y;

    Polygon.mTextureUV[i].x = center.x + 
        ((tmp_x * cos(rotation)) +
         (tmp_y * sin(rotation)));

    Polygon.mTextureUV[i].y = center.y + 
        ((tmp_x * -sin(rotation)) +
         (tmp_y * cos(rotation)));
  }
}


void CBspNode::ShiftFaceTextureU(double offset)
{
  double  shift = 0.0;

  // Back off the last change....
  Polygon.mTextureOffset.x = (Polygon.mTextureOffset.x - mTextureShift.x);
  if (Polygon.mTextureOffset.x < 0)
    Polygon.mTextureOffset.x += 256;

  mTextureShift.x = int(offset);

  Polygon.mTextureOffset.x = (Polygon.mTextureOffset.x + int(offset)) % 256;
}


void CBspNode::ShiftFaceTextureV(double offset)
{
  double  shift = 0.0;

  // Back off the last change...
  Polygon.mTextureOffset.y = (Polygon.mTextureOffset.y - mTextureShift.y);
  if (Polygon.mTextureOffset.y < 0)
    Polygon.mTextureOffset.y += 256;

  mTextureShift.y = int(offset);

  Polygon.mTextureOffset.y = (Polygon.mTextureOffset.y + int(offset)) % 256;
}

void CBspNode::FlipFaceTextureH()
{
  for (int i = 0; i < Polygon.NumberOfPoints; i++)
  {
    //Polygon.mTextureUV[i].y = (double)Polygon.mTextureSize.y - Polygon.mTextureUV[i].y;
    Polygon.mTextureUV[i].y = 256.0 - Polygon.mTextureUV[i].y;
  }
}

void CBspNode::FlipFaceTextureV()
{
  for (int i = 0; i < Polygon.NumberOfPoints; i++)
  {
    //Polygon.mTextureUV[i].x = (double)Polygon.mTextureSize.x - Polygon.mTextureUV[i].x;
    Polygon.mTextureUV[i].x = 256.0 - Polygon.mTextureUV[i].x;
  }
}

#define BOX_YZ            0
#define BOX_XY            1
#define BOX_XZ            2
#define MAX_TEXTURE_COOR  256
#define MAX_TEXTURE_MASK  (MAX_TEXTURE_COOR-1)
void CBspNode::normalizeTexture(ThredPolygon* poly, CThredBrush* brush)//, Point2F center)
{
      int     j;
      int     mapping;

      // Calculate Normal of this polygon
      Point3F normal;
      Point3F vect1, vect2;
      Point2F tmin(+1.0E20f, +1.0E20f);
      Point2F tmax(-1.0E20F, -1.0E20F);
      int     numpoints = poly->NumberOfPoints;
      Point2F center(0.0, 0.0);
      Point3F worldcenter(0.0, 0.0, 0.0);
#if 1
      for (j = 0; j < numpoints; j++)
      {
        // Clamp them vertex points to within 0.001
        poly->Points[j].X = (floor((poly->Points[j].X * 1000.0) + 0.5)) / 1000.0;
        poly->Points[j].Y = (floor((poly->Points[j].Y * 1000.0) + 0.5)) / 1000.0;
        poly->Points[j].Z = (floor((poly->Points[j].Z * 1000.0) + 0.5)) / 1000.0;
      }
#endif
      if (brush)
      {
        worldcenter = brush->GetBrushFaceCenter(mPolygonNumber);
      }

      vect1.x = poly->Points[1].X - poly->Points[0].X;
      vect1.y = poly->Points[1].Z - poly->Points[0].Z;
      vect1.z = poly->Points[1].Y - poly->Points[0].Y;

      vect2.x = poly->Points[numpoints-1].X - poly->Points[0].X;
      vect2.y = poly->Points[numpoints-1].Z - poly->Points[0].Z;
      vect2.z = poly->Points[numpoints-1].Y - poly->Points[0].Y;

      m_cross(vect1, vect2, &normal);
      normal.normalize();

      // Select the proper box face
      if (fabs(normal.x) > fabs(normal.y))
        mapping = ((fabs(normal.x) > fabs(normal.z)) ? BOX_YZ : BOX_XY);
      else
        mapping = ((fabs(normal.y) > fabs(normal.z)) ? BOX_XZ : BOX_XY);

      // Map the vertices to the box face
      for (j = 0; j < numpoints; j++)
      {
        switch (mapping)
        {
          // DPW - note that the y values should be (-) for the new 3space stuff
        case BOX_YZ:
          poly->mTextureUV[j].x = poly->Points[j].Z;   // y in 3space
          poly->mTextureUV[j].y = -poly->Points[j].Y;  // -z in 3space
          center.x = worldcenter.z;
          center.y = -worldcenter.y;
          break;
        case BOX_XY:
          poly->mTextureUV[j].x = poly->Points[j].X;   // x in 3space
          poly->mTextureUV[j].y = -poly->Points[j].Z;  // -y in 3space
          center.x = worldcenter.x;
          center.y = -worldcenter.z;
          break;
        case BOX_XZ:
          poly->mTextureUV[j].x = poly->Points[j].X;   // x in 3space
          poly->mTextureUV[j].y = -poly->Points[j].Y;  // -z in 3space
          center.x = worldcenter.x;
          center.y = -worldcenter.y;
          break;
        } // end switch
      }

      if (brush)
      {
        RotateFaceTexture(brush->mTextureID[mPolygonNumber].TextureRotate, center);

        if (brush->mTextureID[mPolygonNumber].TextureFlipH)
        {
          FlipFaceTextureH();
        }

        if (brush->mTextureID[mPolygonNumber].TextureFlipV)
        {
          FlipFaceTextureV();
        }
      }

      float TextureScale;
      int bits = ITRGeometry::Surface::Constants::textureScaleBits;
      
      int signMask = 1 << ( bits - 1 );
      int valueMask = signMask - 1;
      
      if( poly->mTextureScaleShift & signMask )
      {                                  
         TextureScale = theApp.m_texturescale;
         if( poly->mTextureScaleShift & valueMask )
            TextureScale /= ( 1 << ( poly->mTextureScaleShift & valueMask ) );
      }
      else
      {
         TextureScale = theApp.m_texturescale;
         if( poly->mTextureScaleShift & valueMask )
            TextureScale *= ( 1 << ( poly->mTextureScaleShift & valueMask ) );
      }
      
      for (j = 0; j < numpoints; j++)
      {
        poly->mTextureUV[j].x *= TextureScale; //theApp.m_texturescale;
        poly->mTextureUV[j].y *= TextureScale; //theApp.m_texturescale;

        if (poly->mTextureUV[j].x < tmin.x)
          tmin.x = poly->mTextureUV[j].x;

        if (poly->mTextureUV[j].y < tmin.y)
          tmin.y = poly->mTextureUV[j].y;

        if (poly->mTextureUV[j].x > tmax.x)
          tmax.x = poly->mTextureUV[j].x;

        if (poly->mTextureUV[j].y > tmax.y)
          tmax.y = poly->mTextureUV[j].y;
      } // end loop thru vertices

      // Min/Max must be whole numbers
      tmin.x = floor(tmin.x); tmin.y = floor(tmin.y);
      tmax.x = ceil(tmax.x);  tmax.y = ceil(tmax.y);

      poly->mTextureSize.x = int(tmax.x - tmin.x);
      poly->mTextureSize.y = int(tmax.y - tmin.y);

      if ((poly->mTextureSize.x < 1) || (poly->mTextureSize.y < 1))
      {
        return;
      }

      // Calculate texture offsets
      poly->mTextureOffset.x = int(tmin.x) & MAX_TEXTURE_MASK;
      poly->mTextureOffset.y = int(tmin.y) & MAX_TEXTURE_MASK;

      // Translate coordinates so they fit on the texture
      for (j = 0; j < numpoints; j++)
      {
        poly->mTextureUV[j].x -= tmin.x;
        poly->mTextureUV[j].y -= tmin.y;
      }
}

double CBspNode::NearestVertexDistance(ThredPoint* pos)
{
  int     i;
  double  retval = 99999999.0;

  for (i = 0; i < Polygon.NumberOfPoints; i++)
  {
    double sqrx = (pos->X - Polygon.Points[i].X) * (pos->X - Polygon.Points[i].X);
    double sqry = (pos->Y - Polygon.Points[i].Y) * (pos->Y - Polygon.Points[i].Y);
    double sqrz = (pos->Z - Polygon.Points[i].Z) * (pos->Z - Polygon.Points[i].Z);
    double distance = sqrt(sqrx + sqry + sqrz);

    if (distance < retval)
    {
      retval = distance;
    }
  }

  return retval;
}
