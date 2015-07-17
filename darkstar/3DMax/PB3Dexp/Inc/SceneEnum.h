#ifndef SCENEENUM_H_
#define SCENEENUM_H_

#include <Max.h>
#include <Stdmat.h>

//--------------------------------------------------------------

struct PBVert
{
	float x, y, z;
};

struct PBPoly
{
	int v[3];
};

struct Shape
{
	INode *node;
	PBVert *verts;
	PBPoly *polys;
	int num_verts;
	int num_polys;
};

#define MAX_SHAPES 500

class SceneEnumProc : public ITreeEnumProc 
{
	FILE *fp;
	Shape shapes[ MAX_SHAPES ];
	int num_shapes;

public:
	SceneEnumProc(IScene *scene);
	~SceneEnumProc();

	int		numShapes() { return num_shapes; }
	int		callback( INode *node );
	void		processNodes( FILE *fp );
	void 		getNode( Shape *e, FILE *fp );
};

//--------------------------------------------------------------
#endif
