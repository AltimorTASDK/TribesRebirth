//----------------------------------------------------------------------------

// Geometry collision.
// ITRCollision::transform to transform the test objects into
// geometry space.

//----------------------------------------------------------------------------

#include <itrbit.h>
#include <itrgeometry.h>
#include <itrcollision.h>
#include <tplane.h>


//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

ITRCollision::ITRCollision(const ITRGeometry* buf,TMat3F* tmat,TSMaterialList* mat)
{
	geometry = buf;
	materialList = mat;
	transform = tmat;
   foundSurfaces = NULL;    // spheres and boxes will use 
}

ITRCollision::~ITRCollision()
{
}

inline float m_clamp(float a,float min,float max)
{
	return (a < min)? min: (a > max)? max: a;
}


//----------------------------------------------------------------------------
// Collide a tube with the geometry.
//
bool ITRCollision::collide(const float rad, const Point3F& sp,const Point3F& ep)
{
	collisionList->clear();
	planeStack.clear();
	radius = rad;
	radius2x = rad * 2.0f;

	if (transform) {
		// Transform into geometry space
		m_mul(sp,*transform,&tubeStart);
		m_mul(ep,*transform,&tubeVector);
		tubeVector -= tubeStart;
	}
	else {
		tubeVector = ep;
		tubeVector -= sp;
		tubeStart = sp;
	}

	tubeTime = 2.0f;
	collideTube(0,-1,0.0f,1.0f);

	if (tubeTime <= 1.0f) {
#if 0
		// Shift away from the plane
		if ((tubeTime -= .01f) < .0f)
			tubeTime = .0f;
#endif
		collisionList->increment();
		CollisionSurface& info = collisionList->last();
		if (tubePlane == -1) {
			// Start point in the void?
			if (tubeVector.lenf() == 0.0f)
				info.normal = Point3F(.0f,.0f,1.0f);
			else {
				info.normal = tubeVector;
				info.normal.normalize();
			}
		}
		else {
			const TPlaneF& plane = geometry->planeList[tubePlane];
			info.normal = plane;
			if (plane.distance(tubeStart) < 0.0f)
				info.normal.neg();
#if 0
			// Shift away from the plane
			float dot = m_dot(info.normal,tubeVector);
			if (dot < 0.0f) {
				float dt = .01f / dot;
				if ((tubeTime += dt) < .0f)
					tubeTime = .0f;
			}
#endif
		}
		info.time = tubeTime;
		info.position = tubeVector;
		info.position *= tubeTime;
		info.position += tubeStart;

		info.distance = radius;
		info.surface = 0;
		info.part = 0;
		info.material = 0;
		return true;
	}
	return false;
}

void ITRCollision::collideTube(int nodeIndex, int planeIndex,
	float start, float end)
{
	if (nodeIndex >= 0) {
		const ITRGeometry::BSPNode& node = geometry->nodeList[nodeIndex];
		const TPlaneF& plane = geometry->planeList[node.planeIndex];
		
		float distA = plane.distance(tubeStart);
		float distB = m_dot(plane,tubeVector);
		float distC = distA + distB;

		TPlaneF::Side startSide = (distA >= radius)? TPlaneF::Inside: 
			(distA <= -radius)? TPlaneF::Outside: TPlaneF::OnPlane;
		TPlaneF::Side endSide = (distC >= radius)? TPlaneF::Inside: 
			(distC <= -radius)? TPlaneF::Outside: TPlaneF::OnPlane;

		switch(startSide * 3 + endSide) {
			case -4:		// S-, E-
				collideTube(node.back,planeIndex,start,end);
				break;
			case -3: {	// S-, E.
				float t1 = m_clamp((distA + radius) / -distB,start,end);
				collideTube(node.back,planeIndex,start,end);
				collideTube(node.front,node.planeIndex,t1,end);
				break;
			}
			case -1:		// S., E-
				collideTube(node.front,node.planeIndex,start,start);
				collideTube(node.back,planeIndex,start,end);
				break;
			case 4:		// S+, E+
				collideTube(node.front,planeIndex,start,end);
				break;
			case 3: {	// S+, E.
				float t1 = m_clamp((distA - radius) / -distB,start,end);
				collideTube(node.front,planeIndex,start,end);
				collideTube(node.back,node.planeIndex,t1,end);
				break;
			}
			case 1:		// S., E+
				collideTube(node.back,node.planeIndex,start,start);
				collideTube(node.front,planeIndex,start,end);
				break;
			case 0: {	// S., E.
				collideTube(node.back,node.planeIndex,start,end);
				collideTube(node.front,node.planeIndex,start,end);
				break;
			}
			case -2: {	// S-, E+
				float t1 = m_clamp((distA + radius) / -distB,start,end);
				float t2 = m_clamp((distA - radius2x) / -distB,start,end);
				collideTube(node.back,planeIndex,start,t2);
				collideTube(node.front,node.planeIndex,t1,end);
				break;
			}
			case 2: {	// S+, E-
				float t1 = m_clamp((distA - radius) / -distB,start,end);
				float t2 = m_clamp((distA + radius2x) / -distB,start,end);
				collideTube(node.front,planeIndex,start,t2);
				collideTube(node.back,node.planeIndex,t1,end);
				break;
			}
		}
	}
	else {
      ITRGeometry::BSPLeafWrap leafWrap(const_cast<ITRGeometry*>(geometry),
                                        -(nodeIndex+1));
		if (leafWrap.isSolid()) {
			// Save best start time.
			if (start < tubeTime) {
				tubePlane = planeIndex;
				tubeTime = start;
			}
		}
	}
}


//----------------------------------------------------------------------------
// Collide a line with the geometry.
// Sets ITRCollision::collisionPoint to the collision point
// closest to the start point.
//
bool ITRCollision::collide(const Point3F& start,const Point3F& end)
{
	collisionList->clear();
	planeStack.clear();
	if (transform) {
		LineSeg3F line;
		// Transform into geometry space
		m_mul(start,*transform,&line.start);
		m_mul(end,*transform,&line.end);
		if (collideLine(0,-1,line.start,line.end))
			return true;
		return false;
	}
	return collideLine(0,-1,start,end);
}

bool ITRCollision::collideLine(int nodeIndex,int planeIndex,
	const Point3F& start,const Point3F& end)
{
	if (nodeIndex >= 0) {
		const ITRGeometry::BSPNode& node = geometry->nodeList[nodeIndex];
		const TPlaneF& plane = geometry->planeList[node.planeIndex];
		TPlaneF::Side startSide = plane.whichSide(start);
		TPlaneF::Side endSide   = plane.whichSide(end);
		switch(startSide * 3 + endSide) {
			case -4:		// S-, E-
			case -3:		// S-, E.
			case -1:		// S., E-
				return collideLine(node.back,planeIndex,start,end);
			case 4:		// S+, E+
			case 3:		// S+, E.
			case 1:		// S., E+
				return collideLine(node.front,planeIndex,start,end);
			case 0: {	// S., E.
				// Line lies on the plane
				if (node.back >= 0)
					if (collideLine(node.back,planeIndex,start,end))
						return true;
				if (node.front >= 0)
					if (collideLine(node.front,planeIndex,start,end))
						return true;
				return false;
			}
			case -2: {	// S-, E+
				Point3F ip;
				plane.intersect(LineSeg3F(start,end),&ip);
				if (collideLine(node.back,planeIndex,start,ip))
					return true;
				return collideLine(node.front,node.planeIndex,ip,end);
			}
			case 2: {	// S+, E-
				Point3F ip;
				plane.intersect(LineSeg3F(start,end),&ip);
				if (collideLine(node.front,planeIndex,start,ip))
					return true;
				return collideLine(node.back,node.planeIndex,ip,end);
			}
		}
	}

   ITRGeometry::BSPLeafWrap leafWrap(const_cast<ITRGeometry*>(geometry),
                                     -(nodeIndex+1));
	if (leafWrap.isSolid()) {
		collisionList->increment();
		CollisionSurface& info = collisionList->last();
		if (planeIndex == -1) {
			// Start point in the void
			info.material = 0;
			info.normal = end;
			info.normal -= start;
			float len = info.normal.len();
			if (len)
				info.normal *= 1.0f / len;
			else
				info.normal.z = 1.0f;
		}
		else {
			const TPlaneF &plane = geometry->planeList[planeIndex];
			info.normal = plane;
			info.material = 0;
			if (plane.whichSide(end) != TPlaneF::Outside)
				info.normal.neg();
			if (materialList) {
				info.surface = planeIndex;
				findSurface(leafWrap,start,&info);
			}
		}
		info.position = start;
      info.distance = 0.0f;
		info.surface = planeIndex;
		info.part = -(nodeIndex+1); // leaf index
		return true;
	}
	return false;
}	

//----------------------------------------------------------------------------
// Collide a sphere with the geometry.
// Appends all the surface that intersect the sphere to 
// ITRCollision::collisionList.
//
bool ITRCollision::collide(const Point3F& aCenter,float aRadius)
{
	if (transform)
		// Transform into geometry space
		m_mul(aCenter,*transform,&center);
	else
		center = aCenter;
	radius = aRadius;
	collisionList->clear();
   int   num_surfaces = geometry->surfaceList.size();
   foundSurfaces = new BitVector ( num_surfaces );
   foundSurfaces->zero();
	collideSphere(0);
   
   delete foundSurfaces;
   foundSurfaces = NULL;
   
	if (collisionList->size() && transform) 
		return true;
	
	return collisionList->size() != 0;
}

void ITRCollision::collideSphere(int nodeIndex)
{
	const ITRGeometry::BSPNode& node = geometry->nodeList[nodeIndex];
	const TPlaneF& plane = geometry->planeList[node.planeIndex];
	float distance = m_dot(plane,center) + plane.d;

	bool pop = false;
	if (distance < radius && distance > -radius) {
		// Sphere intersects the plane
		planeStack.increment();
		PlaneEntry& pe = planeStack.last();
		pe.planeIndex = node.planeIndex;
		pe.distance = distance;
		pop = true;
	}
	if (distance > -radius) {
		if (node.front >= 0)
			collideSphere(node.front);
		else
			collideLeaf(node.front);
	}
	if (distance < radius) {
		if (node.back >= 0)
			collideSphere(node.back);
		else
			collideLeaf(node.back);
	}
	if (pop)
		planeStack.decrement();
}


#if 0
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Collide a Box with the geometry.
// Appends all the surface that intersect the box to
// ITRCollision::collisionList.
//
static int PolyVertexIndex[6][4] =
{
	{ 0,1,2,3 },
	{ 4,5,6,7 },
	{ 3,7,6,2 },
	{ 0,4,7,3 },
	{ 1,5,4,0 },
	{ 2,6,5,1 },
};

bool ITRCollision::collide(const Box3F& box)
{
	// Box is assumed to be axis aligned in the source space.
	Point3F xvec,yvec,zvec,min;
	if (transform) {
		// Transform into geometry space
		transform->getRow(0,&xvec);
		xvec *= box.len_x();
		transform->getRow(1,&yvec);
		yvec *= box.len_y();
		transform->getRow(2,&zvec);
		zvec *= box.len_z();
		m_mul(box.fMin,*transform,&min);
	}
	else {
		xvec.set( box.len_x(), .0f, .0f);
		yvec.set( .0f, box.len_y(), .0f);
		zvec.set( .0f, .0f, box.len_z());
		min = box.fMin;
	}

	// Initial eight vertices
	pointStack.clear();
	pointStack.push_back(min);
	pointStack.push_back(min + yvec);
	pointStack.push_back(min + xvec + yvec);
	pointStack.push_back(min + xvec);
	pointStack.push_back(pointStack[0] + zvec);
	pointStack.push_back(pointStack[1] + zvec);
	pointStack.push_back(pointStack[2] + zvec);
	pointStack.push_back(pointStack[3] + zvec);

	// Bounding sphere
	center = min + (xvec + yvec + zvec) * 0.5f;
	radius = m_distf(box.fMin,box.fMax) * 0.5f;

	// Process each of the six faces.
	Poly poly;
	collisionList->clear();
	for (int i = 0; i < 6; i++) {
		poly.vertexCount = 4;
		for (int v = 0; v < 4; v++)
			poly.vertex[v] = &pointStack[PolyVertexIndex[i][v]];
		collideBox(0,poly);
	}

	if (collisionList->size() && transform) 
		return true;
	return collisionList->size() != 0;
}

void ITRCollision::collideBox(int nodeIndex,Poly& poly)
{
	const ITRGeometry::BSPNode& node = geometry->nodeList[nodeIndex];
	const TPlaneF& plane = geometry->planeList[node.planeIndex];

	float distance = m_dot(plane,center) + plane.d;
	float absDist  = fabs(distance);

	// Go ahead and try to split the poly.
	Poly fpoly,bpoly;
	int popSize = pointStack.size();
	if (absDist < radius && split(plane,poly,&fpoly,&bpoly)) {
		planeStack.increment();
		PlaneEntry& pe = planeStack.last();
		pe.planeIndex = node.planeIndex;
		pe.distance = distance;

		if (fpoly.vertexCount) {
			if (node.front >= 0)
				collideBox(node.front,fpoly);
			else
				collideBoxLeaf(node.front);
		}
		if (bpoly.vertexCount) {
			if (node.back >= 0)
				collideBox(node.back,bpoly);
			else
				collideBoxLeaf(node.back);
		}

		planeStack.decrement();
	}
	else {
		// Poly must be on one side or the other.
		if (distance >= 0) {
			if (node.front >= 0)
				collideBox(node.front,poly);
			else
				collideBoxLeaf(node.front);
		}
		else {
			if (node.back >= 0)
				collideBox(node.back,poly);
			else
				collideBoxLeaf(node.back);
		}
	}
	pointStack.setSize(popSize);
}

// Test all the collision planes on the stack with the planes
// bounding this node.
//
void ITRCollision::collideBoxLeaf(int nodeIndex)
{
   ITRGeometry::BSPLeafWrap leafWrap(const_cast<ITRGeometry*>(geometry),
                                     -(nodeIndex+1));
	if (leafWrap.isSolid() && planeStack.size()) {
		UInt8* pbegin = const_cast<UInt8*>(&geometry->bitList[leafWrap.getSurfaceIndex()]);
		// Early out test
		if (!*pbegin && !leafWrap.getSurfaceCount())
			return;
		UInt8* pend = pbegin + leafWrap.getSurfaceCount();

		// See if any intersected planes bound this leaf.
		// Test against all the surfaces in the leaf
		for (ITRCompressedBitVector::iterator itr(pbegin, pend); ++itr;) {
			int sIndex = *itr;
			const ITRGeometry::Surface& surface = geometry->surfaceList[sIndex];
			for (int i = 0; i < planeStack.size(); i++) {
				PlaneEntry& entry = planeStack[i];

				if (surface.planeIndex == entry.planeIndex) {
					// Add surface to the collision list.
					collisionList->increment();
					CollisionSurface& info = collisionList->last();
					info.part = 0;
					info.surface = sIndex;

					info.distance = entry.distance;
					info.normal = geometry->planeList[surface.planeIndex];
					if (!surface.planeFront) {
						info.normal.neg();
						info.distance = -info.distance;
					}

					if (materialList && surface.material != ITRGeometry::Surface::NullMaterial)
						info.material = int(&(*materialList)[surface.material]);
					else
						info.material = 0;
					info.position = center;
					break;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------

bool ITRCollision::split(const TPlaneF& plane, const Poly& poly, Poly* front, Poly* back )
{
	Point3F *start,*end;
	Int32	ci,csign,psign,bcount,fcount;
	Int32	back_degen,front_degen;

	// Make sure the given poly is not degenerate.
	if (poly.vertexCount < 3) {
		front->vertexCount = 0;
	 	back->vertexCount = 0;
		return 0;
	}

	// Assume both back & front are degenerate polygons.
	back_degen = front_degen = 1;

	// Perform the splitting.  The polygon is closed
	// by starting with the last point.
	start = poly.vertex[poly.vertexCount - 1];
	psign = plane.whichSide(*start);

	Point3F **fcp = front->vertex;
	Point3F **bcp = back->vertex;
	Point3F *const*pcp = poly.vertex;
	
	bcount = fcount = 0;

	for (ci = poly.vertexCount; ci > 0; --ci)
	{
		// This assumes that whichSide() continues to return
		// -1 for outside, 0 on the plane and 1 for inside.
		end = *pcp++;
		csign = plane.whichSide(*end);

		switch(psign * 3 + csign)
		{
			case -4:		// P-, C-
			case -1:		// P., C-
				bcount++; *bcp++ = end;
				back_degen = 0;
				break;

			case 4:		// P+, C+
			case 1:		// P., C+
				fcount++; *fcp++ = end;
				front_degen = 0;
				break;

			case -3:		// P-, C.
			case 0:		// P., C.
			case 3:		// P+, C.
				bcount++; *bcp++ = end;
				fcount++; *fcp++ = end;
				break;

			case -2:		// P-, C+
				pointStack.increment();
				if (intersect(plane,*start,*end,&pointStack.last())) {
					bcount++; *bcp++ = &pointStack.last();
					fcount++; *fcp++ = &pointStack.last();
					fcount++; *fcp++ = end;
					front_degen = 0;
				}
				break;

			case 2:		// P+, C-
				pointStack.increment();
				if (intersect(plane,*start,*end,&pointStack.last())) {
					bcount++; *bcp++ = &pointStack.last();
					fcount++; *fcp++ = &pointStack.last();
					bcount++; *bcp++ = end;
					back_degen = 0;
				}
				break;
		}

		psign = csign;
		start = end;
	}

	// Check for degeneracy.
	// This check should be a little more complete.
	if (fcount < 3) {
		front_degen = 1;
		fcount = 0;
	}

	if (bcount < 3) {
		back_degen = 1;
		bcount = 0;
	}

	// If both front and back are degeneratem then it's not split.
	if (front_degen && back_degen)
		return false;

	front->vertexCount = fcount;
	back->vertexCount = bcount;
	return fcount && bcount;
}

bool ITRCollision::intersect(const TPlaneF& plane,const Point3F& start, const Point3F& end, Point3F* ip)
{
	Point3F	dt = end - start;

	// If den == 0 then the line and plane area parallel.
	float den;
	if ((den = plane.x * dt.x + plane.y * dt.y + plane.z * dt.z) == 0)
		return 0;
	float t = -(plane.x * start.x + plane.y * start.y + plane.z * start.z + plane.d);

	// Make sure intersection point is inbetween the line
	// endpoints.
	if (den > 0) {
		if (t < 0 || t > den)
			return 0;
	}
	else
		if (t > 0 || t < den)
			return 0;
	
	ip->x = start.x + m_muldiv(dt.x,t,den);
	ip->y = start.y + m_muldiv(dt.y,t,den);
	ip->z = start.z + m_muldiv(dt.z,t,den);
	return 1;
}
#endif

#if 0
//----------------------------------------------------------------------------
// Collide a Box with the geometry.
// Appends all the surface that intersect the box to
// ITRCollision::collisionList.
//
bool ITRCollision::collide(const Box3F& box)
{
	// Box is assumed to be axis aligned in the source space.
	if (transform) {
		// Transform into geometry space
		transform->getRow(0,&vec[0]);
		vec[0] *= box.len_x() * 0.5f;
		transform->getRow(1,&vec[1]);
		vec[1] *= box.len_y() * 0.5f;
		transform->getRow(2,&vec[2]);
		vec[2] *= box.len_z() * 0.5f;

		Point3F tmp;
		tmp.x = (box.fMin.x + box.fMax.x) * 0.5f;
		tmp.y = (box.fMin.y + box.fMax.y) * 0.5f;
		tmp.z = (box.fMin.z + box.fMax.z) * 0.5f;

		m_mul(tmp,*transform,&center);
	}
	else {
		vec[0].set( box.len_x() * 0.5f, .0f, .0f);
		vec[1].set( .0f, box.len_y() * 0.5f, .0f);
		vec[2].set( .0f, .0f, box.len_z() * 0.5f);
		center.x = (box.fMin.x + box.fMax.x) * 0.5f;
		center.y = (box.fMin.y + box.fMax.y) * 0.5f;
		center.z = (box.fMin.z + box.fMax.z) * 0.5f;
	}

	radius = m_distf(box.fMin,box.fMax) * 0.5f;
	collisionList->clear();
	collideBox(0);
	if (collisionList->size() && transform) 
		return true;

	return collisionList->size() != 0;
}

void ITRCollision::collideBox(int nodeIndex)
{
	const ITRGeometry::BSPNode& node = geometry->nodeList[nodeIndex];
	const TPlaneF& plane = geometry->planeList[node.planeIndex];

	float distance = m_dot(plane,center) + plane.d;
	float absDist  = fabs(distance);
	bool hitPlane = false;

	if (absDist < radius)
	{
		float f1 = fabs( m_dot(plane,vec[0]) );
		float f2 = fabs( m_dot(plane,vec[1]) );
		float f3 = fabs( m_dot(plane,vec[2]) );
		if (f1+f2+f3 > absDist )
		{
			// Box intersects the plane
			planeStack.increment();
			PlaneEntry& pe = planeStack.last();
			pe.planeIndex = node.planeIndex;
			pe.distance = distance;
			hitPlane = true;
		}
	}

	if ( hitPlane || distance >= .0f) {
		if (node.front >= 0)
			collideBox(node.front);
		else
			collideLeaf(node.front);
	}
	if ( hitPlane || distance < .0f) {
		if (node.back >= 0)
			collideBox(node.back);
		else
			collideLeaf(node.back);
	}
	if ( hitPlane )
		planeStack.decrement();
}
#endif

#if 1
//----------------------------------------------------------------------------
// Collide a Box with the geometry.
// Appends all the surface that intersect the box to
// ITRCollision::collisionList.
//
ITRCollision::ITRPolyBSPClip ITRCollision::polyBSP;

bool ITRCollision::collide(const Box3F& box)
{
	AssertFatal(transform != 0,"ITRCollision::collide: No transform provided");

	Point3F tmp;
	tmp.x = (box.fMin.x + box.fMax.x) * 0.5f;
	tmp.y = (box.fMin.y + box.fMax.y) * 0.5f;
	tmp.z = (box.fMin.z + box.fMax.z) * 0.5f;
	m_mul(tmp,*transform,&center);
	radius = m_distf(box.fMin,box.fMax) * 0.5f;

	m_mul(Point3F(0,0,1.0f),(RMat3F&)*transform,&stepVector);
	stepHeight = 0;
	hitSolidNode = false;

	collisionList->contained = false;
	collisionList->stepVector.set(0,0,0);
	collisionList->clear();
	polyBSP.nodeList.clear();
	polyBSP.nodeList.reserve(400);
	polyBSP.cptr = this;
	if ((polyBSP.rootNode = collideBox(0)) != 0) {
		if (!polyBSP.rootNode->plane) {
			// Early out test, only solid node on the stack,
			// must be contained.
			collisionList->contained = true;
			return true;
		}

		// Test the box against the BSP tree.
		polyBSP.box(*transform,box);

		if (!collisionList->size()) {
			if (hitSolidNode) {
				// Hit a solid node, but no planes on the solid were
				// intersected, must be contained.
				collisionList->contained = true;
				return true;
			}
			return false;
		}
		collisionList->stepVector.set(0,0,stepHeight);
		return true;
	}
	return false;
}

ITRCollision::ITRPolyBSPClip::Node* ITRCollision::collideBox(int nodeIndex)
{
	if (nodeIndex < 0) {
	   ITRGeometry::BSPLeafWrap leafWrap
	   	(const_cast<ITRGeometry*>(geometry),-(nodeIndex+1));
		if (leafWrap.isSolid()) {
			AssertFatal(polyBSP.nodeList.size() < 400,
				"ITRCollision::collideBox: Out of nodes");
			polyBSP.nodeList.increment();
			PolyBSPClip::Node& pnode = polyBSP.nodeList.last();
			pnode.nodeId = nodeIndex;
			pnode.plane = 0;
			pnode.planeId = -1;
			pnode.front = pnode.back = 0;
			return &pnode;
		}
	}
	else {
		const ITRGeometry::BSPNode& node = geometry->nodeList[nodeIndex];
		const TPlaneF& plane = geometry->planeList[node.planeIndex];
		float distance = m_dot(plane,center) + plane.d;

		if (fabs(distance) < radius) {
			AssertFatal(polyBSP.nodeList.size() < 400,
				"ITRCollision::collideBox: Out of nodes");
			ITRPolyBSPClip::Node *fn = collideBox(node.front);
			ITRPolyBSPClip::Node *bn = collideBox(node.back);
			if (fn || bn) {
				polyBSP.nodeList.increment();
				PolyBSPClip::Node& pnode = polyBSP.nodeList.last();
				pnode.front = fn;
				pnode.back = bn;
				pnode.nodeId = nodeIndex;
				pnode.plane = &plane;
				pnode.planeId = node.planeIndex;
				return &pnode;
			}
		}
		else {
			if (distance >= .0f)
				return collideBox(node.front);
			else
				return collideBox(node.back);
		}
	}
	return 0;
}

void ITRCollision::ITRPolyBSPClip::collide(const Node* pnode,const Stack& poly)
{
	// Collide with all the faces
	cptr->hitSolidNode = true;
	for (int i = poly.start; i < poly.end; i++) {
		Edge& edge = (*poly.edge)[i];
		for (int f = 0; f < 2; f++) {
			Face& face = faceList[edge.face[f]];
			if (face.planeId >= 0) {
				int sIndex = cptr->pickSurface(pnode->nodeId,face.planeId);
				if (sIndex >= 0) {
					// Push the collision face
					cptr->collisionList->increment();
					CollisionSurface& info = cptr->collisionList->last();
					info.part = 0;
					info.distance = 1.0f;
					info.normal = *face.plane;
					if (face.planeFront)
						info.normal.neg();
					info.position = vertexList[edge.vertex[0]].point;
					info.surface = sIndex;
					info.material = 0;
					if (cptr->materialList) {
						const ITRGeometry::Surface& surface =
							cptr->geometry->surfaceList[info.surface];
						if (surface.material != ITRGeometry::Surface::NullMaterial)
							info.material = int(&(*cptr->materialList)[surface.material]);
					}

					// Clear the id to indicate it's been exported
					face.planeId = -1;
				}
			}
		}

		// Calculate a step height
		for (int v = 0; v < 2; v++) {
			PolyBSPClip::Vertex& vr = vertexList[edge.vertex[v]];
			if (!vr.step) {
				vr.step = true;
				Point3F vp = vr.point - cptr->transform->p;
				float d1 = m_dot(cptr->stepVector,vp);
				if (d1 > cptr->stepHeight)
					cptr->stepHeight = d1;
			}
		}
	}
}

int ITRCollision::pickSurface(int nodeIndex,int planeIndex)
{
	ITRGeometry::BSPLeafWrap leafWrap
		(const_cast<ITRGeometry*>(geometry),-(nodeIndex+1));

	// Early out test
	UInt8* pbegin = const_cast<UInt8*>(&geometry->bitList[leafWrap.getSurfaceIndex()]);
	if (!*pbegin && !leafWrap.getSurfaceCount())
		return -1;

	// Find the first surface on the plane
	UInt8* pend = pbegin + leafWrap.getSurfaceCount();
	for (ITRCompressedBitVector::iterator itr(pbegin, pend); ++itr; ) {
		int sInd = *itr;
		if (geometry->surfaceList[sInd].planeIndex == planeIndex)
			return sInd;
	}
	return -1;
}
#endif


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Test all the collision planes on the stack with the planes
// bounding this node.
//
void ITRCollision::collideLeaf(int nodeIndex)
{
   ITRGeometry::BSPLeafWrap leafWrap(const_cast<ITRGeometry*>(geometry),
                                     -(nodeIndex+1));
	if (leafWrap.isSolid()) {
		UInt8* pbegin = const_cast<UInt8*>(&geometry->bitList[leafWrap.getSurfaceIndex()]);
		// Early out test
		if (!*pbegin && !leafWrap.getSurfaceCount())
			return;
		UInt8* pend   = pbegin + leafWrap.getSurfaceCount();

		// See if any intersected planes bound this leaf.
		for (int i = 0; i < planeStack.size(); i++) {
			int planeIndex = planeStack[i].planeIndex;

			// Test against all the surfaces in the leaf
			for (ITRCompressedBitVector::iterator itr(pbegin, pend); ++itr; ) 
         {
            int   sInd = *itr;
            
            // Test our flag array to see if we've already added this surface.  
            if ( foundSurfaces != NULL && foundSurfaces->test(sInd) )
               continue;
            
				const ITRGeometry::Surface& surface = geometry->surfaceList[sInd];
				if (surface.planeIndex == planeIndex)
					if (collide(surface,sInd))
               {
                  if ( foundSurfaces != NULL )
                     foundSurfaces->set(sInd);
						// Only return a single surface per plane.
						// Should probably only hit one anyway.
						      //  actually we want'em all  break;
               }
			}
		}
	}
}


//----------------------------------------------------------------------------

bool ITRCollision::collide(const ITRGeometry::Surface& surface,int sIndex)
{
	// Test the current sphere against all the polygons on
	// the surface.
	const TPlaneF& plane = geometry->planeList[surface.planeIndex];
 	float dist = plane.distance(center);
	if (fabs(dist) < radius) {
		// Point and radius on the plane
		SphereF ps;
		ps.center = plane;
		ps.center *= (dist > 0.0f)? -dist: dist;
		ps.center += center;
		// ps.radius is radius squared, which is as the
		// collide function expects.
		ps.radius = radius * radius - dist * dist;

		//
		Point3F normal = plane;
		if (!surface.planeFront) {
			normal.neg();
			dist = -dist;
		}

		// Is the point inside a poly?
		if (collide(surface,normal,ps)) {
			collisionList->increment();
			CollisionSurface& info = collisionList->last();
			info.part = 0;
			info.surface = sIndex;
			info.distance = dist;
			info.normal = normal;
			if (materialList && surface.material != ITRGeometry::Surface::NullMaterial)
				info.material = int(&(*materialList)[surface.material]);
			else
				info.material = 0;
			info.position = ps.center;
			return true;
		}
	}
	return false;
}


//----------------------------------------------------------------------------

bool ITRCollision::collide(const ITRGeometry::Surface& surface,
	const Point3F& normal,const SphereF& sphere)
{
	// Expects sphere radius value to actually be the radius squared.
	// See if the sphere intersects the surface poly.  The sphere center
	// is on the plane of the poly.

   // Assumes polys have at most 32 vertices

   // Method of check:  sphere intersects (convex) poly iff
   //   1.  sphere center is inside poly
   //   2.  sphere center is inside a rectangle formed by an edge
   //       of the poly and a perpendicular width of length radius
   //   3.  sphere center is within radius of a poly vertex

   // Here we first check to see if sphere center is outside of poly
   // extended by radius on each edge.  This is expected to reject
   // most cases as not colliding quickly.  More detailed loop then follows.
   // If collisions were the norm, then integrating the loops may be faster.

   Point3F lvec;
	const ITRGeometry::Vertex* vb = &geometry->vertexList[surface.vertexIndex];
	const ITRGeometry::Vertex* vp = &vb[surface.vertexCount - 1];
	const Point3F* ep;
	const Point3F* sp;

	sp = &geometry->point3List[vp->pointIndex];
	vp = &vb[0];
   int i;
   UInt32 bitIndex = 0x0001;
   UInt32 dotBits = 0x0000;
	for (i = 0; i < surface.vertexCount; i++)
	{
		ep = &geometry->point3List[(vp++)->pointIndex];

		Point3F lnormal;
      lvec = *ep - *sp;
		m_cross(lvec,normal,&lnormal);

		Point3F pvec = sphere.center - *sp;
		float dot = m_dot(pvec,lnormal);
		float len2 = m_dot(lnormal,lnormal);

      // Preserve sign value and do test using squares --
      // Done this way to avoid sqrts.
      if (dot<0.0f)
      {
         if (dot*dot > sphere.radius * len2)
            // sphere center outside poly by more than radius of sphere
            return false;
         else
            dotBits |= bitIndex; // dot product is negative; remember this...
                                 // it means sphere center is outside poly edge
      }
		sp = ep;
      bitIndex <<= 1; // shift bit index
   }

   // sphere center inside poly if dotBits = 0
   if (!dotBits)
      return true;

	sp = &geometry->point3List[vb[surface.vertexCount-1].pointIndex];
   bool spChecked = false, epChecked;

	vp = &vb[0];
   bitIndex = 0x0001;
	for (i = 0; i < surface.vertexCount; i++)
	{
		ep = &geometry->point3List[(vp++)->pointIndex];
      epChecked = false;

      if (dotBits & bitIndex) // sphere center outside this edge,
      {                       // so within radius of edge
         lvec = *ep - *sp;
         float vecDot = m_dot(sphere.center,lvec);

         Point3F vertVec;
         if (vecDot < m_dot(*sp,lvec))
         {
            if (!spChecked)
            {
               // is sphere within range of sp
               vertVec = sphere.center - *sp;
               if (m_dot(vertVec,vertVec) < sphere.radius)
                  return true;
            }
         }
         else if (vecDot > m_dot(*ep,lvec))
         {
            // is sphere within range of ep
            vertVec = sphere.center - *ep;
            if (m_dot(vertVec,vertVec) < sphere.radius)
               return true;
            epChecked=true;
         }
         else
            // inside rect on edge of poly
            return true;
      }

		sp = ep;
      spChecked = epChecked;
      bitIndex <<= 1; // shift bit index
  	}
	return false;
}


//----------------------------------------------------------------------------

void
ITRCollision::findSurface(const ITRGeometry::BSPLeafWrap& leafWrap,
                          const Point3F&                  pos,
                          CollisionSurface*               info)
{
	SphereF ps;
	ps.center = pos;
	ps.radius = 0.0f;
	int planeIndex = info->surface;

	// Test against all the surfaces in the leaf
	UInt8* pbegin = const_cast<UInt8*>(&geometry->bitList[leafWrap.getSurfaceIndex()]);
	UInt8* pend   = pbegin + leafWrap.getSurfaceCount();
	for (ITRCompressedBitVector::iterator itr(pbegin, pend); ++itr; ) {
		const ITRGeometry::Surface& surface = geometry->surfaceList[*itr];
		if (surface.planeIndex == planeIndex) {
			const TPlaneF& plane = geometry->planeList[surface.planeIndex];

			Point3F normal = plane;
			if (!surface.planeFront)
				normal.neg();

			// Could speed this writing a point in poly test instead of
			// using the sphere one.
			if (collide(surface,normal,ps)) {
				if (surface.material != ITRGeometry::Surface::NullMaterial)
					info->material = int(&(*materialList)[surface.material]);
				else
					info->material = 0;
				info->surface = *itr;
				return;
			}
		}
	}
}


