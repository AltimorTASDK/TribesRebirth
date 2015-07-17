//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#ifndef _TPOLY_H_
#define _TPOLY_H_

#include <tvector.h>
#include <tplane.h>


//---------------------------------------------------------------------------

struct TPolyVertex {
	enum Flag {
		ClippedEdge = 0x1,
	};
	int flags;
	Point3F point;
	TPolyVertex();
	void set(TPolyVertex& start,TPolyVertex& end,floatscale) {
		point.x = start.point.x + (end.point.x - start.point.x) * scale;
		point.y = start.point.y + (end.point.y - start.point.y) * scale;
		point.z = start.point.z + (end.point.z - start.point.z) * scale;
	}
};

inline TPolyVertex::TPolyVertex()
{
	flags = 0;
}


//---------------------------------------------------------------------------
// The vertex class V for the tpoly template should be derived
// from struct TPolyVertex.
//
template <class V>
class TPoly
{
public:
	typedef TPlaneF Plane;
	typedef V Vertex;
	typedef Vector<V> VertexList;
	//
	TPoly();
	~TPoly();
	//
	VertexList vertexList;
	//
	Plane::Side whichSide(Plane&);
	bool split(Plane& plane,TPoly* front,TPoly* back);
	bool clip(Plane& plane,TPoly* front,bool onPlane = true);
	bool clip(Plane& plane,bool onPlane = true);
	bool anyClippedEdges();
};



//---------------------------------------------------------------------------

template <class V>
TPoly<V>::TPoly()
{
}	

template <class V>
TPoly<V>::~TPoly()
{
}

//---------------------------------------------------------------------------

template <class V>
bool TPoly<V>::clip(Plane& plane,bool onPlane)
{
	TPoly tmp;
	clip(plane,&tmp,onPlane);
	*this = tmp;
	return tmp.vertexList.size() != 0;
}


//---------------------------------------------------------------------------

template <class V>
TPoly<V>::Plane::Side TPoly<V>::whichSide(Plane& plane)
{
	if (!vertexList.size())
		return Plane::Undefined;

	// Find first point not on the plane
	VertexList::iterator end = vertexList.end();
	VertexList::iterator itr = vertexList.begin();
	Plane::Side side = Plane::OnPlane;
	for (; itr != end; itr++)
		if ((side = plane.whichSide((*itr).point)) != Plane::OnPlane)
			break;

	// Make sure the rest are on the same side
	for (; itr != end; itr++) {
		Plane::Side nside = plane.whichSide((*itr).point);
		if (nside != side && nside != Plane::OnPlane)
			// Crosses the plane
			return Plane::Intersect;
	}
	return side;
}

//---------------------------------------------------------------------------

template <class V>
bool TPoly<V>::anyClippedEdges()
{
	for (VertexList::iterator itr = vertexList.begin();
			itr != vertexList.end(); itr++)
		if ((*itr).flags & TPolyVertex::ClippedEdge)
			return true;
	return false;
}


//---------------------------------------------------------------------------
// Split the poly along the given plane.
// Returns true if the poly was actually split, if it wasn't the poly
// will copied into either in the front or back side.
//
template <class V>
bool TPoly<V>::split(Plane& plane,TPoly* front,TPoly* back)
{
	// Make sure the given poly is not degenerate.
	if (front)
		front->vertexList.clear();
	if (back)
	 	back->vertexList.clear();
	if (vertexList.size() < 3)
		return false;

	// Assume both back & front are degenerate polygons.
	bool back_degen = true,front_degen = true;

	// Perform the splitting.  The polygon is closed by starting with
	// the last point.
	int start = vertexList.size() - 1;
	Plane::Side psign = plane.whichSide(vertexList[start].point);

	V vertex;
	float time;
	for (int end = 0; end < vertexList.size(); end++)
	{
		// This assumes that whichSide() continues to return
		// -1 for outside, 0 on the plane and 1 for inside.
		Plane::Side csign = plane.whichSide(vertexList[end].point);

		switch(psign * 3 + csign) {
			case -4:		// P-, C-
			case -1:		// P., C-
				if (back)
					back->vertexList.push_back(vertexList[end]);
				back_degen = false;
				break;
			case 4:		// P+, C+
			case 1:		// P., C+
				if (front)
					front->vertexList.push_back(vertexList[end]);
				front_degen = false;
				break;
			case -3:		// P-, C.
			case 0:		// P., C.
			case 3:		// P+, C.
				if (back)
					back->vertexList.push_back(vertexList[end]);
				if (front)
					front->vertexList.push_back(vertexList[end]);
				break;
			case -2:		// P-, C+
				if (plane.intersect(vertexList[start].point,
						vertexList[end].point,&time)) {
					vertex.set(vertexList[start],vertexList[end],time);
					if (back)
						back->vertexList.push_back(vertex);
					if (front) {
						front->vertexList.push_back(vertex);
						front->vertexList.push_back(vertexList[end]);
					}
					front_degen = false;
				}
				break;

			case 2:		// P+, C-
				if (plane.intersect(vertexList[start].point,
						vertexList[end].point,&time)) {
					vertex.set(vertexList[start],vertexList[end],time);
					if (front)
						front->vertexList.push_back(vertex);
					if (back) {
						back->vertexList.push_back(vertex);
						back->vertexList.push_back(vertexList[end]);
					}
					back_degen = false;
				}
				break;
		}

		psign = csign;
		start = end;
	}

	// Check for degeneracy.
	// This check should be a little more complete.
	if (front && front->vertexList.size() < 3)
		front_degen = true;
	if (back && back->vertexList.size() < 3)
		back_degen = true;

	if (front_degen && back_degen) {
		// If both front and back are degenerate then I assume that
		// it is on the plane and default to the front half space.
		if (back)
			back->vertexList.clear();
		if (front) {
			front->vertexList.setSize(vertexList.size());
			for (int i = 0; i < vertexList.size(); i++)
				front->vertexList[i] = vertexList[i];
		}
	}
	else {
		if (front && front_degen)
			front->vertexList.clear();
		if (back && back_degen)
			back->vertexList.clear();
	}


	if (front && back)
		return front->vertexList.size() && back->vertexList.size();
	else
		if (front)
			return front->vertexList.size() != 0;
		else
			if (back)
				return back->vertexList.size() != 0;
	return false;
}


//---------------------------------------------------------------------------
// Clip the poly along the given plane.
// Only keeps the inside portion.  Returns true if anything was
// put in the output poly.  Pass onPlane as true to consider
// polys on the plane to be considered inside.
//
template <class V>
bool TPoly<V>::clip(Plane& plane,TPoly* front,bool onPlane)
{
	// Make sure the given poly is not degenerate.
	front->vertexList.clear();
	if (vertexList.size() < 3)
		return false;

	// Assume both back & front are degenerate polygons.
	bool back_degen = true,front_degen = true;

	// Perform the splitting.  The polygon is closed by starting with
	// the last point.
	int start = vertexList.size() - 1;
	Plane::Side psign = plane.whichSide(vertexList[start].point);

	V vertex;
	float time;
	for (int end = 0; end < vertexList.size(); end++)
	{
		// This assumes that whichSide() continues to return
		// -1 for outside, 0 on the plane and 1 for inside.
		Plane::Side csign = plane.whichSide(vertexList[end].point);

		switch(psign * 3 + csign) {
			case -4:		// P-, C-
			case -1:		// P., C-
				back_degen = false;
				break;
			case 4:		// P+, C+
			case 1:		// P., C+
				front->vertexList.push_back(vertexList[end]);
				front_degen = false;
				break;
			case 0:		// P., C.
				front->vertexList.push_back(vertexList[end]);
				front->vertexList.last().flags |= TPolyVertex::ClippedEdge;
				break;
			case -3:		// P-, C.
			case 3:		// P+, C.
				front->vertexList.push_back(vertexList[end]);
				break;
			case -2:		// P-, C+
				if (plane.intersect(vertexList[start].point,
						vertexList[end].point,&time)) {
					vertex.set(vertexList[start],vertexList[end],time);
					front->vertexList.push_back(vertex);
					front->vertexList.last().flags |= TPolyVertex::ClippedEdge;
					front->vertexList.push_back(vertexList[end]);
					front_degen = false;
				}
				break;

			case 2:		// P+, C-
				if (plane.intersect(vertexList[start].point,
						vertexList[end].point,&time)) {
					vertex.set(vertexList[start],vertexList[end],time);
					front->vertexList.push_back(vertex);
					back_degen = false;
				}
				break;
		}

		psign = csign;
		start = end;
	}

	// Check for degeneracy.
	// This check should be a little more complete.
	if (front->vertexList.size() < 3)
		front_degen = true;

	if (front_degen)
		if (back_degen && onPlane) {
			// If both front and back are degenerate then I assume that
			// it is on the plane.
			front->vertexList.setSize(vertexList.size());
			for (int i = 0; i < vertexList.size(); i++)
				front->vertexList[i] = vertexList[i];
		}
		else
			front->vertexList.clear();

	return front->vertexList.size();
}


#endif

