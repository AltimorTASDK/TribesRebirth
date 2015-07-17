//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _ITRCOLLISION_H_
#define _ITRCOLLISION_H_

#include <ml.h>
#include <tvector.h>
#include <tBitVector.h>
#include <m_collision.h>
#include <ts_material.h>
#include "itrgeometry.h"
#include "polyBSPClip.h"

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

class ITRCollision
{
public:
	struct PlaneEntry {
		int planeIndex;
		float distance;
	};
	typedef Vector<PlaneEntry> PlaneStack;

private:
	//
	int tubePlane;
	float tubeTime;
	Point3F tubeStart,tubeVector;
	Point3F center;
	float radius,radius2x;
	Point3F vec[3];

   BitVector *foundSurfaces;
	PlaneStack planeStack;
	Vector<Point3F> pointStack;

#if 1
	class ITRPolyBSPClip: public PolyBSPClip {
	public:
		ITRCollision* cptr;
		void collide(const Node* node,const Stack& poly);
	};
	friend ITRPolyBSPClip;
	static ITRPolyBSPClip polyBSP;
	Point3F stepVector;
	float stepHeight;
	bool hitSolidNode;
#endif

	//
	void collideSphere(int node);
	void collideLeaf(int node);
	bool collideLine(int node,int plane,const Point3F& start,const Point3F& end);
   void collideTube(int node,int plane,float start,float end);
	bool collide(const ITRGeometry::Surface& surface, int surfaceIndex);
	bool collide(const ITRGeometry::Surface& surface,const Point3F& normal,
		const SphereF& sphere);
	void findSurface(const ITRGeometry::BSPLeafWrap& leaf,const Point3F& pos,
		CollisionSurface* info);
	void collideBoxLeaf(int nodeIndex);

#if 0
	struct Poly {
		enum Constants {
			MaxVertex = 20,
		};
		int vertexCount;
		Point3F* vertex[MaxVertex];
	};

	void collideBox(int node,Poly& poly);
	bool split(const TPlaneF& plane,const Poly& poly,Poly* front,Poly* back);
	bool intersect(const TPlaneF& plane,const Point3F& start, const Point3F& end, Point3F* ip);
#endif
#if 0
	void collideBox(int node);
#endif
#if 1
	ITRPolyBSPClip::Node* collideBox(int node);
	int pickSurface(int nodeIndex,int planeIndex);
#endif
public:
	//
	const ITRGeometry* geometry;	// Interior to collide against
	TMat3F* transform;				// Transform into geometry space
	TSMaterialList* materialList;

	// Collisions are returned in collisionList
	CollisionSurfaceList* collisionList;

	//
	ITRCollision(const ITRGeometry* = 0, TMat3F* tmat = 0, TSMaterialList* mat = 0);
	~ITRCollision();
	bool collide(const Point3F& start,const Point3F& end);
	bool collide(const Point3F& center,float radius);
	bool collide(const Box3F& box);
   bool collide(const float rad, const Point3F& start,const Point3F& end);
};


#endif
