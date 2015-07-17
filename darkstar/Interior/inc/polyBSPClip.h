//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#ifndef _POLYBSPCLIP_H_
#define _POLYBSPCLIP_H_

#include <ml.h>
#include <tplane.h>


//----------------------------------------------------------------------------

class PolyBSPClip
{
public:
	// Polyhedron
	struct Vertex {
		Point3F point;
		int side;
		bool step;
	};
	struct Edge {
		int vertex[2];
		int face[2];
	};
	struct Face {
		int planeId;
		int vertex;
		const TPlaneF* plane;
		bool planeFront;
	};
	typedef Vector<Edge> EdgeList;
	typedef Vector<Face> FaceList;
	typedef Vector<Vertex> VertexList;

	// BSP Nodes
	// Terminal nodes have null plane ptr.
	struct Node {
		const TPlaneF* plane;
		Node* front;
		Node* back;
		int nodeId;
		int planeId;
	};
	typedef Vector<Node> NodeList;
	typedef Vector<TPlaneF> PlaneList;

	// Edge stack
	struct Stack {
		EdgeList* edge;
		int start;
		int end;
	};

	//
	EdgeList stack[3];
	VertexList vertexList;
	FaceList faceList;
	NodeList nodeList;
	int sideCount;
	Node* rootNode;

	//
	PolyBSPClip();
	void box(const TMat3F transform,const Box3F& box);
	void split(const Node* node,const Stack& poly);
	virtual void collide(const Node* node,const Stack& poly);
	bool intersect(const TPlaneF& plane,const Point3F& sp,const Point3F& ep);
};


#endif
