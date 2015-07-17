
#ifndef _BSPNODE_H_
#define _BSPNODE_H_

#include "thredprimitives.h"
class CThredBrush;

#define NO_THING	(-1)

#define PARALLEL_VECTOR_ERROR (double)0.00001

// used to classify a polygon
enum {	POLYGON_IN_FRONT, POLYGON_IN_BACK, POLYGON_IN, POLYGON_SPLIT, POLYGON_COPLANAR_SAME, POLYGON_COPLANAR_REVERSE  };

// used to define what we are filtering through the tree
enum FilterType { ADD_BRUSH, SUBTRACT_BRUSH };


// A node in the tree
class CBspNode {
public:
    double NearestVertexDistance(ThredPoint* pos);
    void normalizeTexture(ThredPolygon* poly, CThredBrush* brush);//, Point2F center);
    void ShiftFaceTextureU(double offset);
    void ShiftFaceTextureV(double offset);
	CBspNode *FrontChild, *BackChild, *CoplanarChild, *NextNode;
    void RotateFaceTexture(double angle, Point2F center);
    void FlipFaceTextureH();
    void FlipFaceTextureV();

	// we are making the polygon implicit here
	// because we don't want a bullshit global list
	// anymore.
	ThredPolygon Polygon;
	double ConstantA, ConstantB, ConstantC, ConstantD;

    int perturbed;

	// define whether or not this is a "new" node.
	// set this every time we add a node to the
	// tree or create a node.
	int	mNewNode;
	// the id of the brush we are from
	int mBrushId;
	// whether we are valid to be drawn
	int mValidated;
	// the polygon number from within the brush that
	// we originated from.  We use this so that
	// when we are split all the split polys
	// reference the same original polygon
	// and so that rebuilding the tree works fine.
	CThredBrush* mBrushPtr;
	int mPolygonNumber;
	int mReverseRef;
	// back reference to the brush that created us.
	int Splitter;
    Point2I mTextureShift;

	// constructor
	CBspNode()
    {
      FrontChild = BackChild = CoplanarChild = NextNode = NULL; mBrushPtr = NULL; mNewNode = TRUE; mValidated = TRUE; Splitter = 0;
      mPolygonNumber = mReverseRef = 0; mTextureShift.x = 0; mTextureShift.y = 0;
    }
	void AssignPolygon(ThredPolygon* PolyPtr);
	void AssignReversePolygon(ThredPolygon* PolyPtr);
	int ClassifyPoint(ThredPoint* Point);
	int ClassifyPolygon(ThredPolygon* Polygon);
	ThredPolygon* SplitPolygon(ThredPolygon* SplitPolygon, ThredPolygon* NewPolygon);
	//int IntersectLinePlane(ThredPoint* Point1, ThredPoint* Point2, ThredPoint* NewPoint, Point2F* TPoint1, Point2F* TPoint2, Point2F* TexturePoint);
    int IntersectLinePlane(ThredPoint* Point1, ThredPoint* Point2, ThredPoint* NewPoint);
	void CalcPolyFromPlane(CBspNode* p, ThredPolygon& NewPoly);
	void CalcBackPolyFromPlane(CBspNode* p, ThredPolygon& NewPoly);
	void CheckFace();

private:
	void CalculatePlaneConstants();
};


#endif

