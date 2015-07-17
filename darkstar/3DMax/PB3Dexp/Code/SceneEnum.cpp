#include "SceneEnum.h"

#include <istdplug.h>
#include <animtbl.h>
#include <dummy.h>
#include <decomp.h>
#include <modstack.h>
#include "pb3deres.h"

//#define SDUMP

#define VERSION "0000"

//--------------------------------------------------------------

static TriObject *getTriObj( INode *pNode, Object *ob, int time, bool *deleteIt )
{
	TriObject * tri = 0;
	if ( ob->CanConvertToType(triObjectClassID)) 
	{
		tri = (TriObject *)ob->ConvertToType( time, triObjectClassID);
		*deleteIt = (tri && (tri != ob));
	}
	else
	{
		// if the object can't convert to a tri-mesh, eval world state to
		// get an object that can:
		const ObjectState &os = pNode->EvalWorldState( time );
		
		if ( os.obj->CanConvertToType(triObjectClassID) ) 
			tri = (TriObject *)os.obj->ConvertToType( time, triObjectClassID );
			
		*deleteIt = (tri && (tri != os.obj));
	}
	
	return tri;
}

void SceneEnumProc::getNode( Shape *shp, FILE *fp )
{
	Object * ob		= shp->node->GetObjectRef();
	// we don't want dummys here:
	if ( !ob || ob->ClassID() == dummyClassID )
		return;
		
	bool del_tri;
	TriObject *tri = getTriObj( shp->node, ob, 0, &del_tri );
	
	if ( tri )
	{
		Matrix3 totalMat = shp->node->GetObjTMAfterWSM( 0 );
		Mesh &mesh = tri->mesh;
		shp->verts = new PBVert[ shp->num_verts = mesh.numVerts ];
		shp->polys = new PBPoly[ shp->num_polys = mesh.numFaces ];
		
		PBPoly *poly = shp->polys;
		
	   for ( int i = 0; i < mesh.numFaces; i++, poly++ )
	   {
	      Face &maxface = mesh.faces[ i ];

			for ( int j = 0; j < 3; j++ )
			{
				int vi = poly->v[j] = maxface.v[j];
				Point3 vt, res;
				
		      vt.x = mesh.verts[ vi ][0];
		      vt.y = mesh.verts[ vi ][1];
		      vt.z = mesh.verts[ vi ][2];

				// pre-multiply the point by it's node's total transform, 
				//  since we won't be saving off any transform information.
				res = vt * totalMat;

				PBVert *vert = &shp->verts[ vi ];
				vert->x = res.x;
				vert->y = res.y;
				vert->z = res.z;
			}
		}
	
		if ( del_tri )
			tri->DeleteMe();
	}
}			


//--------------------------------------------------------------

void SceneEnumProc::processNodes( FILE *fp )
{
	Shape *shp;
	PBPoly *p;
	PBVert *v;
	int i, j, k;
	
	fprintf( fp, "PB3D%s\n", VERSION );

	// gather all node info first	
	for ( i = 0, shp = shapes; i < num_shapes; i++, shp++ )
		getNode( shp, fp );
	
#ifdef SDUMP
	fprintf( fp, "\n// Num Shapes:\n" );
#endif	
	fprintf( fp, "%d\n", num_shapes );
		
	for ( i = 0, shp = shapes; i < num_shapes; i++, shp++ )
	{
#ifdef SDUMP
		fprintf( fp, "\n// Shape %d:\n", i );
#endif
		fprintf( fp, "%s\n", shp->node->GetName() );
#ifdef SDUMP
		fprintf( fp, "\n// Num Verts:\n" );
#endif
		fprintf( fp, "%d\n", shp->num_verts );
#ifdef SDUMP
		fprintf( fp, "\n// Verts:\n" );
#endif
		for ( v = shp->verts, j = 0; j < shp->num_verts; j++, v++ )
			fprintf( fp, "%8.5f, %8.5f, %8.5f\n", v->x, v->y, v->z );
			
#ifdef SDUMP
		fprintf( fp, "\n// Num Polys:\n" );
#endif
		fprintf( fp, "%d\n", shp->num_polys );
			
#ifdef SDUMP
		fprintf( fp, "\n// Polys:\n" );
#endif
		for ( p = shp->polys, k = 0; k < shp->num_polys; k++, p++ )
			fprintf( fp, "%d, %d, %d\n", p->v[0], p->v[1], p->v[2] );
	}
}

//--------------------------------------------------------------
// callback for EnumTree:

int SceneEnumProc::callback(INode *pNode) 
{
	const char *name = pNode->GetName();
	ObjectState os = pNode->EvalWorldState(0);
	
	// we are only interested in nodes which have meshes on them
	if ( os.obj->CanConvertToType(triObjectClassID) ) 
		if ( strnicmp( name, "camera", 6 ) )	// exclude cameras
		{
			if ( num_shapes >= MAX_SHAPES )
				return TREE_ABORT;
				
			shapes[ num_shapes++ ].node = pNode;
		}
			
	return TREE_CONTINUE;	// Keep on enumeratin'!
}

//--------------------------------------------------------------

SceneEnumProc::SceneEnumProc(IScene *scene) 
{
   num_shapes = 0;
	scene->EnumTree(this);
}

SceneEnumProc::~SceneEnumProc() 
{
}

//--------------------------------------------------------------


