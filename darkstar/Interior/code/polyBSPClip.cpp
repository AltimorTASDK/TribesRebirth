//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#include "polyBSPClip.h"

//----------------------------------------------------------------------------

PolyBSPClip::PolyBSPClip()
{
	vertexList.reserve(100);
	faceList.reserve(200);
	stack[0].reserve(100);
	stack[1].reserve(100);
	stack[2].reserve(100);
}


//----------------------------------------------------------------------------

void PolyBSPClip::box(const TMat3F transform,const Box3F& box)
{
	sideCount = 0;

	// Box is assumed to be axis aligned in the source space.
	// Transform into geometry space
	Point3F xvec,yvec,zvec,min;
	transform.getRow(0,&xvec);
	xvec *= box.len_x();
	transform.getRow(1,&yvec);
	yvec *= box.len_y();
	transform.getRow(2,&zvec);
	zvec *= box.len_z();
	m_mul(box.fMin,transform,&min);

	// Initial vertices
	vertexList.setSize(8);
	vertexList[0].point = min;
	vertexList[1].point = min + yvec;
	vertexList[2].point = min + xvec + yvec;
	vertexList[3].point = min + xvec;
	vertexList[4].point = vertexList[0].point + zvec;
	vertexList[5].point = vertexList[1].point + zvec;
	vertexList[6].point = vertexList[2].point + zvec;
	vertexList[7].point = vertexList[3].point + zvec;
   int i;
	for (i = 0; i < 8; i++) {
		vertexList[i].side = 0;
		vertexList[i].step = false;
	}

	// Initial faces
	faceList.setSize(6);
	for (int f = 0; f < 6; f++) {
		Face& face = faceList[f];
		face.vertex = 0;
		face.plane = 0;
		face.planeId = -1;
	}

	// Initial edges
	stack[0].setSize(12);
	Edge* edge = stack[0].begin();
	for (i = 0; i < 4; i++) {
		int n = (i == 3)? 0: i + 1;
		int p = (i == 0)? 3: i - 1;
		edge->vertex[0] = i;
		edge->vertex[1] = n;
		edge->face[0] = i;
		edge->face[1] = 4;
		edge++;
		edge->vertex[0] = 4 + i;
		edge->vertex[1] = 4 + n;
		edge->face[0] = i;
		edge->face[1] = 5;
		edge++;
		edge->vertex[0] = i;
		edge->vertex[1] = 4 + i;
		edge->face[0] = i;
		edge->face[1] = p;
		edge++;
	}

	// Starting stack
	stack[1].setSize(0);
	stack[2].setSize(0);
	Stack poly;
	poly.edge = &stack[0];
	poly.start = 0;
	poly.end = stack[0].size();
	split(rootNode,poly);
}


//----------------------------------------------------------------------------

void PolyBSPClip::split(const Node* node,const Stack& poly)
{
	// New front and back stacks
	Stack front;
	front.edge = (poly.edge == &stack[2])? &stack[0]: poly.edge + 1;
	front.start = front.edge->size();

	Stack back;
	back.edge = (front.edge == &stack[2])? &stack[0]: front.edge + 1;
	back.start = back.edge->size();

	const TPlaneF& plane = *node->plane;
	int startVertex = vertexList.size();

	// New face
	faceList.increment();
	Face& face = faceList.last();
	face.planeId = node->planeId;
	face.plane = node->plane;
	face.vertex = 0;

	// Test & clip all the edges
	int sideBase = ++sideCount << 1;
	for (int i = poly.start; i < poly.end; i++) {
		Edge& edge = (*poly.edge)[i];

		Vertex& v0 = vertexList[edge.vertex[0]];
		if (v0.side < sideBase)
			v0.side = sideBase + ((plane.whichSide(v0.point) >= 0)? 0: 1);
		Vertex& v1 = vertexList[edge.vertex[1]];
		if (v1.side < sideBase)
			v1.side = sideBase + ((plane.whichSide(v1.point) >= 0)? 0: 1);

		if (v0.side != v1.side) {
			int s = v0.side - sideBase;
			intersect(plane,v0.point,v1.point);

			// Split the edge into each stack
			front.edge->increment();
			Edge& e0 = front.edge->last();
			back.edge->increment();
			Edge& e1 = back.edge->last();
			e0.vertex[0] = edge.vertex[s];
			e1.vertex[0] = edge.vertex[s ^ 1];
			e0.vertex[1] = e1.vertex[1] = vertexList.size() - 1;
			e0.face[0] = e1.face[0] = edge.face[0];
			e0.face[1] = e1.face[1] = edge.face[1];

			// Add new edges on the plane
			for (int f = 0; f < 2; f++) {
				Face& face = faceList[edge.face[f]];
				if (face.vertex < startVertex)
					face.vertex = vertexList.size() - 1;
				else {
					front.edge->increment();
					Edge& e0 = front.edge->last();
					e0.vertex[0] = face.vertex;
					e0.vertex[1] = vertexList.size() - 1;
					e0.face[0] = edge.face[f];
					e0.face[1] = faceList.size() - 1;
					back.edge->push_back(e0);
				}
			}
		}
		else
			if (v0.side == sideBase)
				front.edge->push_back(edge);
			else
				back.edge->push_back(edge);
	}

	front.end = front.edge->size();
	back.end = back.edge->size();

	// Recurse front & back
	if (node->front && front.start != front.end) {
		face.planeFront = true;
		if (node->front->plane)
			split(node->front,front);
		else
			collide(node->front,front);
	}
	if (node->back && back.start != back.end) {
		face.planeFront = false;
		if (node->back->plane)
			split(node->back,back);
		else
			collide(node->back,back);
	}

	// Pop off the face
	faceList.decrement();
}


//----------------------------------------------------------------------------

void PolyBSPClip::collide(const Node* node,const Stack& poly)
{
	// Collide with all the faces
	for (int i = poly.start; i < poly.end; i++) {
		Edge& edge = (*poly.edge)[i];
		for (int f = 0; f < 2; f++) {
			Face& face = faceList[edge.face[f]];
			if (face.planeId >= 0) {
				// Clear the id to indicate it's been exported
				face.planeId = -1;

			}
		}
	}
}


//----------------------------------------------------------------------------

bool PolyBSPClip::intersect(const TPlaneF& plane,const Point3F& sp,const Point3F& ep)
{
	// If den == 0 then the line and plane are parallel.
	float den;
	Point3F dt = ep - sp;
	if ((den = plane.x * dt.x + plane.y * dt.y + plane.z * dt.z) == 0)
		return false;

	vertexList.increment();
	Vertex& v = vertexList.last();
	float s = -(plane.x * sp.x + plane.y * sp.y + plane.z * sp.z + plane.d) / den;
	v.point.x = sp.x + dt.x * s;
	v.point.y = sp.y + dt.y * s;
	v.point.z = sp.z + dt.z * s;
	v.side = 0;
	v.step = false;
	return true;
}
